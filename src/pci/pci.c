#include <pci.h>
#include <x86.h>
#include <stdio.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static inline uint32_t pci_make_address(uint8_t bus,
                                        uint8_t device,
                                        uint8_t function,
                                        uint8_t offset)
{
    return (uint32_t)(1u << 31)
         | ((uint32_t)bus << 16)
         | ((uint32_t)device << 11)
         | ((uint32_t)function << 8)
         | (offset & 0xFC);
}

static uint32_t pci_read_config_dword(uint8_t bus,
                                      uint8_t device,
                                      uint8_t function,
                                      uint8_t offset)
{
    uint32_t address = pci_make_address(bus, device, function, offset);
    x86_outl(PCI_CONFIG_ADDRESS, address);
    return x86_inl(PCI_CONFIG_DATA);
}

static uint16_t pci_read_config_word(uint8_t bus, uint8_t device,
                                     uint8_t function, uint8_t offset)
{
    uint32_t value = pci_read_config_dword(bus, device, function, offset & ~0x3);
    return (uint16_t)(value >> ((offset & 0x2) * 8));
}

static uint8_t pci_read_config_byte(uint8_t bus, uint8_t device,
                                    uint8_t function, uint8_t offset)
{
    uint32_t value = pci_read_config_dword(bus, device, function, offset & ~0x3);
    return (uint8_t)(value >> ((offset & 0x3) * 8));
}

static void pci_log_function(uint8_t bus, uint8_t device, uint8_t function)
{
    uint16_t vendor = pci_read_config_word(bus, device, function, 0x00);
    if (vendor == 0xFFFF)
        return;

    pci_device_t dev = {
        .bus        = bus,
        .device     = device,
        .function   = function,
        .vendor_id  = vendor,
        .device_id  = pci_read_config_word(bus, device, function, 0x02),
        .class_code = pci_read_config_byte(bus, device, function, 0x0B),
        .subclass   = pci_read_config_byte(bus, device, function, 0x0A),
        .prog_if    = pci_read_config_byte(bus, device, function, 0x09),
    };

    kprintf("PCI %02x:%02x.%u vendor=%04x device=%04x class=%02x%02x prog-if=%02x\n",
            dev.bus, dev.device, dev.function,
            dev.vendor_id, dev.device_id,
            dev.class_code, dev.subclass, dev.prog_if);
}

static void pci_scan_device(uint8_t bus, uint8_t device)
{
    pci_log_function(bus, device, 0);

    uint8_t header = pci_read_config_byte(bus, device, 0, 0x0E);
    if ((header & 0x80) == 0)
        return;

    for (uint8_t function = 1; function < 8; function++)
        pci_log_function(bus, device, function);
}

void pci_enumerate(void)
{
    for (uint16_t bus = 0; bus < 256; bus++)
        for (uint8_t device = 0; device < 32; device++)
            pci_scan_device(bus, device);
}
