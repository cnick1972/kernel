#pragma once

#define GDT_SELECTOR_CODE 0x08 /**< Selector for the kernel code segment. */
#define GDT_SELECTOR_DATA 0x10 /**< Selector for the kernel data segment. */

/**
 * @brief Set up and load the global descriptor table.
 */
void gdt_init(void);
