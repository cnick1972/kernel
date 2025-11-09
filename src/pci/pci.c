/**
 * @file pci/pci.c
 * @brief PCI bus enumeration logic.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <pci.h>
#include <x86.h>
#include <stdio.h>
#include <stdbool.h>

#define PCI_CONFIG_ADDRESS 0xCF8   /**< PCI configuration address register */
#define PCI_CONFIG_DATA    0xCFC   /**< PCI configuration data register */

/**
 * @brief Build the configuration address used with PCI configuration mechanism #1.
 *
 * @param bus      Target PCI bus number.
 * @param device   Target device number on the bus.
 * @param function Target function number within the device.
 * @param offset   Register offset within the configuration space.
 * @return Encoded configuration address suitable for CONFIG_ADDRESS.
 */
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

/**
 * @brief Read a 32-bit value from PCI configuration space.
 *
 * @param bus      Target PCI bus number.
 * @param device   Target device number on the bus.
 * @param function Target function number within the device.
 * @param offset   DWORD-aligned register offset.
 * @return 32-bit value read from configuration space.
 */
static uint32_t pci_read_config_dword(uint8_t bus,
                                      uint8_t device,
                                      uint8_t function,
                                      uint8_t offset)
{
    uint32_t address = pci_make_address(bus, device, function, offset);
    x86_outl(PCI_CONFIG_ADDRESS, address);
    return x86_inl(PCI_CONFIG_DATA);
}

/**
 * @brief Read a 16-bit value from PCI configuration space (word-aligned access).
 *
 * @param bus      Target PCI bus number.
 * @param device   Target device number on the bus.
 * @param function Target function number within the device.
 * @param offset   Register offset (word-aligned).
 * @return 16-bit value read from configuration space.
 */
static uint16_t pci_read_config_word(uint8_t bus, uint8_t device,
                                     uint8_t function, uint8_t offset)
{
    uint32_t value = pci_read_config_dword(bus, device, function, offset & ~0x3);
    return (uint16_t)(value >> ((offset & 0x2) * 8));
}

/**
 * @brief Read an 8-bit value from PCI configuration space.
 *
 * @param bus      Target PCI bus number.
 * @param device   Target device number on the bus.
 * @param function Target function number within the device.
 * @param offset   Register offset.
 * @return 8-bit value read from configuration space.
 */
static uint8_t pci_read_config_byte(uint8_t bus, uint8_t device,
                                    uint8_t function, uint8_t offset)
{
    uint32_t value = pci_read_config_dword(bus, device, function, offset & ~0x3);
    return (uint8_t)(value >> ((offset & 0x3) * 8));
}

/**
 * @brief Emit a human-readable description of a PCI function, if present.
 *
 * @param bus      Target PCI bus number.
 * @param device   Target device number on the bus.
 * @param function Target function number within the device.
 */
static void pci_log_function(uint8_t bus, uint8_t device, uint8_t function)
{
    pci_device_t dev = {
        .bus        = bus,
        .device     = device,
        .function   = function,
        .vendor_id  = 0xFFFF,
        .device_id  = 0,
        .class_code = 0,
        .subclass   = 0,
        .prog_if    = 0,
    };

    dev.vendor_id = pci_read_config_word(bus, device, function, 0x00);
    if (dev.vendor_id != 0xFFFF)
    {
        dev.device_id  = pci_read_config_word(bus, device, function, 0x02);
        dev.class_code = pci_read_config_byte(bus, device, function, 0x0B);
        dev.subclass   = pci_read_config_byte(bus, device, function, 0x0A);
        dev.prog_if    = pci_read_config_byte(bus, device, function, 0x09);

        const char* class_str = pci_class_name(dev.class_code);
        const char* subclass_str = pci_subclass_name(dev.class_code, dev.subclass, dev.prog_if);

        kprintf("PCI %02x:%02x.%u vendor=%04x device=%04x [%s / %s, prog-if %02x]\n",
                dev.bus, dev.device, dev.function,
                dev.vendor_id, dev.device_id,
                class_str, subclass_str, dev.prog_if);
    }
}

/**
 * @brief Probe a PCI device slot and enumerate all available functions.
 *
 * @param bus    Target PCI bus number.
 * @param device Device number on the bus to probe.
 */
static void pci_scan_device(uint8_t bus, uint8_t device)
{
    uint8_t header = pci_read_config_byte(bus, device, 0, 0x0E);
    bool multi_function = (header & 0x80) != 0;
    
    pci_log_function(bus, device, 0);

    for (uint8_t function = 1; multi_function && function < 8; function++)
    {
        pci_log_function(bus, device, function);
    }
}

/**
 * @brief Enumerate the full PCI bus hierarchy and log discovered devices.
 */
void pci_enumerate(void)
{
    for (uint16_t bus = 0; bus < 256; bus++)
        for (uint8_t device = 0; device < 32; device++)
            pci_scan_device(bus, device);
}
