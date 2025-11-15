/**
 * @file include/gdt.h
 * @brief Global Descriptor Table selectors and APIs.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

#include <stdint.h>

#define GDT_SELECTOR_CODE       0x08 /**< Kernel code segment selector. */
#define GDT_SELECTOR_DATA       0x10 /**< Kernel data segment selector. */
#define GDT_SELECTOR_USER_CODE  0x1B /**< User-mode code segment selector (RPL=3). */
#define GDT_SELECTOR_USER_DATA  0x23 /**< User-mode data segment selector (RPL=3). */
#define GDT_SELECTOR_TSS        0x28 /**< Task state segment selector. */

#define GDT_INDEX_TSS           5

typedef enum
{
    GDT_ACCESS_CODE_READABLE                = 0x02,
    GDT_ACCESS_DATA_WRITEABLE               = 0x02,

    GDT_ACCESS_CODE_CONFORMING              = 0x04,
    GDT_ACCESS_DATA_DIRECTION_NORMAL        = 0x00,
    GDT_ACCESS_DATA_DIRECTION_DOWN          = 0x04,

    GDT_ACCESS_DATA_SEGMENT                 = 0x10,
    GDT_ACCESS_CODE_SEGMENT                 = 0x18,

    GDT_ACCESS_DESCRIPTOR_TSS               = 0x00,
    GDT_ACCESS_TSS_AVAILABLE                = 0x09,

    GDT_ACCESS_RING0                        = 0x00,
    GDT_ACCESS_RING1                        = 0x20,
    GDT_ACCESS_RING2                        = 0x40,
    GDT_ACCESS_RING3                        = 0x60,

    GDT_ACCESS_PRESENT                      = 0x80,

} GDT_ACCESS;

typedef enum
{
    GDT_FLAG_64BIT                          = 0x20,
    GDT_FLAG_32BIT                          = 0x40,
    GDT_FLAG_16BIT                          = 0x00,

    GDT_FLAG_GRANULARITY_1B                 = 0x00,
    GDT_FLAG_GRANULARITY_4K                 = 0x80,
} GDT_FLAGS;

/**
 * @brief Set up and load the global descriptor table.
 */
void gdt_init(void);

/**
 * @brief Overwrite a GDT entry at runtime.
 *
 * @param index  Index within the GDT.
 * @param base   Segment base address.
 * @param limit  Segment limit (bytes).
 * @param access Access byte.
 * @param flags  Flags nibble (upper four bits of flags/limit byte).
 */
void gdt_set_entry_raw(int index,
                       uint32_t base,
                       uint32_t limit,
                       uint8_t access,
                       uint8_t flags);
