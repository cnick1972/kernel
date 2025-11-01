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

const char* pci_class_name(uint8_t class_code);
const char* pci_subclass_name(uint8_t class_code, uint8_t subclass, uint8_t prog_if);
void pci_enumerate(void);
