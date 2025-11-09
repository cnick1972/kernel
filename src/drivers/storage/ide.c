/**
 * @file drivers/storage/ide.c
 * @brief Minimal IDE controller initialization using PCI discovery data.
 */

#include <ide.h>
#include <x86.h>
#include <stdio.h>
#include <memory.h>
#include <stddef.h>
#include <block_device.h>
#include <partition.h>

#define IDE_CHANNEL_PRIMARY   0
#define IDE_CHANNEL_SECONDARY 1
#define IDE_CHANNEL_COUNT     2

#define ATA_CTL_SRST 0x04 /**< Soft-reset controllers when set. */
#define ATA_CTL_nIEN 0x02 /**< Disable interrupts when set. */

#define ATA_SR_ERR 0x01
#define ATA_SR_DRQ 0x08
#define ATA_SR_DF  0x20
#define ATA_SR_DRDY 0x40
#define ATA_SR_BSY 0x80

#define ATA_CMD_IDENTIFY         0xEC
#define ATA_CMD_IDENTIFY_PACKET  0xA1
#define ATA_CMD_READ_SECTORS     0x20

#define ATA_REG_DATA          0x00
#define ATA_REG_ERROR         0x01
#define ATA_REG_SECTOR_COUNT  0x02
#define ATA_REG_LBA_LOW       0x03
#define ATA_REG_LBA_MID       0x04
#define ATA_REG_LBA_HIGH      0x05
#define ATA_REG_DRIVE_HEAD    0x06
#define ATA_REG_STATUS        0x07
#define ATA_REG_COMMAND       0x07

typedef enum {
    IDE_DEVICE_NONE = 0,
    IDE_DEVICE_ATA,
    IDE_DEVICE_ATAPI,
} ide_device_type_t;

typedef struct {
    uint16_t command_base;
    uint16_t control_base;
    uint16_t bus_master_base;
    uint8_t  irq;
    bool     present;
} ide_channel_t;

typedef struct ide_controller ide_controller_t;

typedef struct {
    bool              present;
    ide_device_type_t type;
    uint8_t           channel_index;
    uint8_t           drive_select;
    uint32_t          unit_number;
    ide_channel_t*    channel;
    ide_controller_t* controller;
    char              model[41];
    block_device_t    block;
    uint64_t          total_sectors;
    uint32_t          sector_size;
} ide_drive_info_t;

struct ide_controller {
    bool             in_use;
    uint8_t          bus;
    uint8_t          device;
    uint8_t          function;
    uint16_t         vendor_id;
    uint16_t         device_id;
    uint8_t          prog_if;
    ide_channel_t    channels[IDE_CHANNEL_COUNT];
    ide_drive_info_t drives[IDE_CHANNEL_COUNT][2];
};

static ide_controller_t g_ide_controllers[IDE_MAX_CONTROLLERS];
static uint8_t g_ide_controller_count;
static uint32_t g_ide_drive_count;

static bool ide_block_device_read(block_device_t* device,
                                  uint64_t lba,
                                  uint32_t sector_count,
                                  void* buffer);

static bool ide_pio_read(ide_drive_info_t* drive,
                         uint64_t lba,
                         uint32_t sector_count,
                         void* buffer);

static bool ide_wait_not_busy(uint16_t io_base);
static bool ide_wait_for_drq(uint16_t io_base);
static void ide_select_drive(uint16_t io_base, uint16_t control_base, uint8_t drive);

static uint16_t ide_default_command_base(uint8_t channel)
{
    return (channel == IDE_CHANNEL_PRIMARY) ? 0x1F0 : 0x170;
}

static uint16_t ide_default_control_base(uint8_t channel)
{
    return (channel == IDE_CHANNEL_PRIMARY) ? 0x3F6 : 0x376;
}

static inline uint16_t ide_mask_io_bar(uint32_t raw_value)
{
    if ((raw_value & 0x1) == 0) {
        return 0;
    }

    return (uint16_t)(raw_value & 0xFFFFFFFCu);
}

static uint16_t ide_decode_command_bar(uint32_t bar_value, uint8_t channel)
{
    uint16_t addr = ide_mask_io_bar(bar_value);
    if (addr == 0) {
        return ide_default_command_base(channel);
    }
    return addr;
}

static uint16_t ide_decode_control_bar(uint32_t bar_value, uint8_t channel)
{
    uint16_t addr = ide_mask_io_bar(bar_value);
    if (addr == 0) {
        return ide_default_control_base(channel);
    }
    return (uint16_t)(addr + 2); // device control resides at base + 2
}

static uint16_t ide_decode_bus_master_bar(uint32_t bar_value, uint8_t channel)
{
    uint16_t base = ide_mask_io_bar(bar_value);
    if (base == 0) {
        return 0;
    }
    return (uint16_t)(base + (channel == IDE_CHANNEL_SECONDARY ? 8 : 0));
}

static void ide_soft_reset_channel(const ide_channel_t* channel)
{
    if (!channel->present || channel->control_base == 0) {
        return;
    }

    x86_outb(channel->control_base, ATA_CTL_SRST | ATA_CTL_nIEN);
    x86_iowait();
    x86_outb(channel->control_base, ATA_CTL_nIEN);

    for (int i = 0; i < 4; ++i) {
        x86_iowait();
    }
}

static void ide_configure_channel(ide_controller_t* ctrl,
                                  uint8_t channel,
                                  uint32_t bar_cmd,
                                  uint32_t bar_ctl,
                                  uint32_t bar_busmaster,
                                  uint8_t irq)
{
    ide_channel_t* ch = &ctrl->channels[channel];
    ch->command_base    = ide_decode_command_bar(bar_cmd, channel);
    ch->control_base    = ide_decode_control_bar(bar_ctl, channel);
    ch->bus_master_base = ide_decode_bus_master_bar(bar_busmaster, channel);
    ch->irq             = irq;
    ch->present         = true;

    ide_soft_reset_channel(ch);

    kprintf("IDE: %02x:%02x.%u %s channel cmd=0x%04x ctrl=0x%04x bm=0x%04x irq=%u\n",
            ctrl->bus,
            ctrl->device,
            ctrl->function,
            channel == IDE_CHANNEL_PRIMARY ? "primary" : "secondary",
            ch->command_base,
            ch->control_base,
            ch->bus_master_base,
            ch->irq);
}

static void ide_format_drive_name(char* buffer, size_t length, uint32_t index)
{
    if (buffer == NULL || length == 0)
    {
        return;
    }

    if (length < 4)
    {
        buffer[0] = '\0';
        return;
    }

    buffer[0] = 'h';
    buffer[1] = 'd';
    size_t pos = 2;

    char digits[10];
    size_t digit_count = 0;
    do
    {
        digits[digit_count++] = (char)('0' + (index % 10));
        index /= 10;
    } while (index != 0 && digit_count < sizeof(digits));

    while (digit_count > 0 && pos < length - 1)
    {
        buffer[pos++] = digits[--digit_count];
    }

    buffer[pos] = '\0';
}

static bool ide_pio_read(ide_drive_info_t* drive,
                         uint64_t lba,
                         uint32_t sector_count,
                         void* buffer)
{
    if (drive == NULL || buffer == NULL || sector_count == 0)
    {
        return false;
    }

    if (drive->type != IDE_DEVICE_ATA)
    {
        return false;
    }

    if (drive->sector_size != 512)
    {
        return false;
    }

    if ((lba + sector_count) >= (1ull << 28))
    {
        return false;
    }

    ide_channel_t* channel = drive->channel;
    if (channel == NULL || !channel->present)
    {
        return false;
    }

    uint16_t* target = (uint16_t*)buffer;

    while (sector_count > 0)
    {
        uint8_t chunk = (sector_count > 127) ? 127 : (uint8_t)sector_count;
        if (chunk == 0)
        {
            chunk = 1;
        }

        ide_select_drive(channel->command_base, channel->control_base, drive->drive_select);

        if (!ide_wait_not_busy(channel->command_base))
        {
            return false;
        }

        x86_outb(channel->command_base + ATA_REG_SECTOR_COUNT, chunk);
        x86_outb(channel->command_base + ATA_REG_LBA_LOW,  (uint8_t)(lba & 0xFF));
        x86_outb(channel->command_base + ATA_REG_LBA_MID,  (uint8_t)((lba >> 8) & 0xFF));
        x86_outb(channel->command_base + ATA_REG_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
        x86_outb(channel->command_base + ATA_REG_DRIVE_HEAD,
                 (uint8_t)(0xE0 | (drive->drive_select << 4) | ((lba >> 24) & 0x0F)));
        x86_outb(channel->command_base + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

        for (uint8_t s = 0; s < chunk; ++s)
        {
            if (!ide_wait_not_busy(channel->command_base))
            {
                return false;
            }

            if (!ide_wait_for_drq(channel->command_base))
            {
                return false;
            }

            for (int word = 0; word < 256; ++word)
            {
                *target++ = x86_inw(channel->command_base + ATA_REG_DATA);
            }
        }

        sector_count -= chunk;
        lba += chunk;
    }

    return true;
}

static bool ide_block_device_read(block_device_t* device,
                                  uint64_t lba,
                                  uint32_t sector_count,
                                  void* buffer)
{
    if (device == NULL || buffer == NULL || sector_count == 0)
    {
        return false;
    }

    ide_drive_info_t* drive = (ide_drive_info_t*)device->driver_data;
    if (drive == NULL || !drive->present || drive->type != IDE_DEVICE_ATA)
    {
        return false;
    }

    return ide_pio_read(drive, lba, sector_count, buffer);
}

static void ide_register_drive(ide_controller_t* ctrl,
                               ide_channel_t* channel,
                               uint8_t channel_index,
                               uint8_t drive_select,
                               ide_device_type_t type,
                               const char* model,
                               uint64_t total_sectors)
{
    if (ctrl == NULL || channel == NULL)
    {
        return;
    }

    ide_drive_info_t* drive = &ctrl->drives[channel_index][drive_select];
    memset(drive, 0, sizeof(*drive));

    drive->present       = true;
    drive->type          = type;
    drive->channel_index = channel_index;
    drive->drive_select  = drive_select;
    drive->controller    = ctrl;
    drive->channel       = channel;
    drive->unit_number   = g_ide_drive_count++;
    drive->total_sectors = total_sectors;
    drive->sector_size   = 512;

    if (model != NULL)
    {
        size_t i;
        for (i = 0; i < sizeof(drive->model) - 1 && model[i] != '\0'; ++i)
        {
            drive->model[i] = model[i];
        }
        drive->model[i] = '\0';
    }

    ide_format_drive_name(drive->block.name, sizeof(drive->block.name), drive->unit_number);
    drive->block.sector_size  = drive->sector_size;
    drive->block.sector_count = drive->total_sectors;
    drive->block.read         = ide_block_device_read;
    drive->block.driver_data  = drive;

    if (!block_device_register(&drive->block))
    {
        kprintf("IDE: failed to register block interface for drive %u\n", drive->unit_number);
        drive->present = false;
        return;
    }

    partition_scan_device(&drive->block);
}

static const char* ide_channel_name(uint8_t channel)
{
    return (channel == IDE_CHANNEL_PRIMARY) ? "primary" : "secondary";
}

static const char* ide_drive_name(uint8_t drive)
{
    return (drive == 0) ? "master" : "slave";
}

static bool ide_wait_not_busy(uint16_t io_base)
{
    for (uint32_t i = 0; i < 100000; ++i) {
        uint8_t status = x86_inb(io_base + 7);
        if ((status & ATA_SR_BSY) == 0) {
            return true;
        }
    }
    return false;
}

static bool ide_wait_for_drq(uint16_t io_base)
{
    for (uint32_t i = 0; i < 100000; ++i) {
        uint8_t status = x86_inb(io_base + 7);
        if (status & ATA_SR_ERR) {
            return false;
        }
        if (status & ATA_SR_DRQ) {
            return true;
        }
    }
    return false;
}

static void ide_select_drive(uint16_t io_base, uint16_t control_base, uint8_t drive)
{
    if (control_base != 0) {
        x86_outb(control_base, ATA_CTL_nIEN);
    }

    x86_outb(io_base + 6, (uint8_t)(0xA0 | (drive << 4)));
    for (int i = 0; i < 4; ++i) {
        x86_iowait();
    }
}

static bool ide_issue_identify(uint16_t io_base,
                               uint16_t control_base,
                               uint8_t drive,
                               uint8_t command,
                               uint16_t* buffer)
{
    ide_select_drive(io_base, control_base, drive);

    x86_outb(io_base + 1, 0x00);
    x86_outb(io_base + 2, 0x00);
    x86_outb(io_base + 3, 0x00);
    x86_outb(io_base + 4, 0x00);
    x86_outb(io_base + 5, 0x00);

    x86_outb(io_base + 7, command);

    x86_iowait();
    x86_iowait();

    uint8_t status = x86_inb(io_base + 7);
    if (status == 0) {
        return false;
    }

    if (!ide_wait_not_busy(io_base)) {
        return false;
    }

    status = x86_inb(io_base + 7);
    if (status & (ATA_SR_ERR | ATA_SR_DF)) {
        return false;
    }

    if (!ide_wait_for_drq(io_base)) {
        return false;
    }

    for (int i = 0; i < 256; ++i) {
        buffer[i] = x86_inw(io_base);
    }

    return true;
}

static void ide_extract_model(const uint16_t* identify_words, char* out, size_t out_len)
{
    if (out_len == 0) {
        return;
    }

    size_t idx = 0;
    for (int word = 27; word <= 46 && idx + 1 < out_len; ++word) {
        uint16_t val = identify_words[word];
        char hi = (char)(val >> 8);
        char lo = (char)(val & 0xFF);

        if (hi == '\0') hi = ' ';
        if (lo == '\0') lo = ' ';

        out[idx++] = hi;
        if (idx + 1 < out_len) {
            out[idx++] = lo;
        }
    }

    if (idx >= out_len) {
        idx = out_len - 1;
    }
    out[idx] = '\0';

    for (int i = (int)idx - 1; i >= 0; --i) {
        if (out[i] == ' ' || out[i] == '\0') {
            out[i] = '\0';
        } else {
            break;
        }
    }
}

static ide_device_type_t ide_identify_device(const ide_channel_t* channel,
                                             uint8_t drive,
                                             char* model,
                                             size_t model_len,
                                             uint64_t* total_sectors_out)
{
    if (model_len > 0) {
        model[0] = '\0';
    }

    if (total_sectors_out != NULL) {
        *total_sectors_out = 0;
    }

    if (channel == NULL || !channel->present || channel->command_base == 0) {
        return IDE_DEVICE_NONE;
    }

    uint16_t identify_words[256];
    memset(identify_words, 0, sizeof(identify_words));

    if (ide_issue_identify(channel->command_base,
                           channel->control_base,
                           drive,
                           ATA_CMD_IDENTIFY,
                           identify_words))
    {
        ide_extract_model(identify_words, model, model_len);
        if (total_sectors_out != NULL)
        {
            uint64_t sectors_28 =
                ((uint64_t)identify_words[61] << 16) |
                (uint64_t)identify_words[60];
            *total_sectors_out = sectors_28;
        }
        return IDE_DEVICE_ATA;
    }

    memset(identify_words, 0, sizeof(identify_words));
    if (ide_issue_identify(channel->command_base,
                           channel->control_base,
                           drive,
                           ATA_CMD_IDENTIFY_PACKET,
                           identify_words))
    {
        ide_extract_model(identify_words, model, model_len);
        if (total_sectors_out != NULL)
        {
            *total_sectors_out = 0;
        }
        return IDE_DEVICE_ATAPI;
    }

    return IDE_DEVICE_NONE;
}

static void ide_probe_drive(ide_controller_t* ctrl,
                            ide_channel_t* channel,
                            uint8_t channel_index,
                            uint8_t drive)
{
    if (!channel->present || channel->command_base == 0) {
        return;
    }

    char model[41];
    uint64_t total_sectors = 0;
    ide_device_type_t type = ide_identify_device(channel,
                                                 drive,
                                                 model,
                                                 sizeof(model),
                                                 &total_sectors);
    const char* chan_name = ide_channel_name(channel_index);
    const char* drive_name = ide_drive_name(drive);

    if (type == IDE_DEVICE_NONE) {
        kprintf("IDE: %02x:%02x.%u %s %s no device detected\n",
                ctrl->bus,
                ctrl->device,
                ctrl->function,
                chan_name,
                drive_name);
        return;
    }

    const char* type_str = (type == IDE_DEVICE_ATAPI) ? "ATAPI" : "ATA";
    const char* model_str = (model[0] != '\0') ? model : "unknown";

    kprintf("IDE: %02x:%02x.%u %s %s %s model=\"%s\"\n",
            ctrl->bus,
            ctrl->device,
            ctrl->function,
            chan_name,
            drive_name,
            type_str,
            model_str);

    ide_register_drive(ctrl, channel, channel_index, drive, type, model, total_sectors);
}

static void ide_scan_devices(ide_controller_t* ctrl)
{
    for (uint8_t channel = 0; channel < IDE_CHANNEL_COUNT; ++channel) {
        ide_probe_drive(ctrl, &ctrl->channels[channel], channel, 0);
        ide_probe_drive(ctrl, &ctrl->channels[channel], channel, 1);
    }
}

void ide_controller_init_from_pci(const ide_pci_descriptor_t* desc)
{
    if (desc == NULL) {
        return;
    }

    if (g_ide_controller_count >= IDE_MAX_CONTROLLERS) {
        kprintf("IDE: ignoring %02x:%02x.%u -> controller list full\n",
                desc->bus,
                desc->device,
                desc->function);
        return;
    }

    ide_controller_t* ctrl = &g_ide_controllers[g_ide_controller_count++];
    ctrl->in_use   = true;
    ctrl->bus      = desc->bus;
    ctrl->device   = desc->device;
    ctrl->function = desc->function;
    ctrl->vendor_id = desc->vendor_id;
    ctrl->device_id = desc->device_id;
    ctrl->prog_if   = desc->prog_if;

    kprintf("IDE: initializing controller %02x:%02x.%u vendor=%04x device=%04x (prog-if %02x)\n",
            ctrl->bus,
            ctrl->device,
            ctrl->function,
            ctrl->vendor_id,
            ctrl->device_id,
            ctrl->prog_if);

    ide_configure_channel(ctrl,
                          IDE_CHANNEL_PRIMARY,
                          desc->bar[0],
                          desc->bar[1],
                          desc->bar[4],
                          desc->interrupt_line);

    ide_configure_channel(ctrl,
                          IDE_CHANNEL_SECONDARY,
                          desc->bar[2],
                          desc->bar[3],
                          desc->bar[4],
                          desc->interrupt_line);

    kprintf("IDE: controller ready %02x:%02x.%u vendor=%04x device=%04x\n",
            ctrl->bus,
            ctrl->device,
            ctrl->function,
            ctrl->vendor_id,
            ctrl->device_id);

    ide_scan_devices(ctrl);
}
