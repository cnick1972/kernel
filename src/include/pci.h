#pragma once

#include <stdint.h>

typedef struct {
    uint8_t  bus;
    uint8_t  device;
    uint8_t  function;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t  class_code;
    uint8_t  subclass;
    uint8_t  prog_if;
} pci_device_t;

/**
 * @brief Obtain a human-readable name for a PCI base class code.
 *
 * @param class_code PCI class code from the configuration header.
 * @return Pointer to a static string describing the class.
 */
const char* pci_class_name(uint8_t class_code);

/**
 * @brief Obtain a human-readable name for a PCI subclass/prog-if combination.
 *
 * @param class_code PCI class code.
 * @param subclass   PCI subclass code.
 * @param prog_if    Programming interface code.
 * @return Pointer to a static string describing the subclass.
 */
const char* pci_subclass_name(uint8_t class_code, uint8_t subclass, uint8_t prog_if);

/**
 * @brief Enumerate all PCI buses and log discovered functions.
 */
void pci_enumerate(void);
