/**
 * @file include/ide.h
 * @brief Basic IDE controller discovery/initialization interfaces.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Maximum number of IDE controllers the kernel tracks concurrently.
 */
#define IDE_MAX_CONTROLLERS 4

/**
 * @brief Descriptor describing an IDE function discovered on the PCI bus.
 */
typedef struct {
    uint8_t  bus;           /**< PCI bus number. */
    uint8_t  device;        /**< PCI device number. */
    uint8_t  function;      /**< PCI function number. */
    uint16_t vendor_id;     /**< PCI vendor identifier. */
    uint16_t device_id;     /**< PCI device identifier. */
    uint8_t  prog_if;       /**< Programming interface byte. */
    uint32_t bar[6];        /**< Raw BAR values from configuration space. */
    uint8_t  interrupt_line; /**< Routed IRQ line reported by PCI config space. */
} ide_pci_descriptor_t;

/**
 * @brief Initialize an IDE controller described by a PCI function.
 *
 * @param desc Descriptor populated during PCI enumeration.
 */
void ide_controller_init_from_pci(const ide_pci_descriptor_t* desc);
