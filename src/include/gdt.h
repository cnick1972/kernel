#pragma once

#define x86_GDT_CODE_SEGMENT 0x08 /**< Selector for the kernel code segment. */
#define x86_GDT_DATA_SEGMENT 0x10 /**< Selector for the kernel data segment. */

/**
 * @brief Set up and load the global descriptor table.
 */
void x86_GDT_Initialize();
