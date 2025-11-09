/**
 * @file system/idt/idt.c
 * @brief Interrupt Descriptor Table management.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <idt.h>
#include <stdint.h>
#include <binary.h>
#include <kerndef.h>

/**
 * @brief Interrupt descriptor table entry layout.
 */
typedef struct
{
    uint16_t    BaseLow;          /**< Handler address bits 0-15. */
    uint16_t    SegmentSelector;  /**< Code segment selector for the handler. */
    uint8_t     Reserved;         /**< Unused, must be zero. */
    uint8_t     Flags;            /**< Gate type, privilege, and presence bits. */
    uint16_t    BaseHigh;         /**< Handler address bits 16-31. */
} __attribute__((packed)) idt_entry_t;

/**
 * @brief Pseudo-descriptor consumed by the LIDT instruction.
 */
typedef struct 
{
    uint16_t    Limit;            /**< Size of the IDT in bytes minus one. */
    idt_entry_t*   Ptr;           /**< Linear address of the first entry. */
} __attribute__((packed)) idt_descriptor_t;

/**
 * @brief Global interrupt descriptor table.
 */
static idt_entry_t idt_entries[256];

/**
 * @brief Descriptor referencing the active IDT.
 */
static idt_descriptor_t idt_descriptor = { sizeof(idt_entries) - 1, idt_entries };

/**
 * @brief Assembly helper that loads the IDT descriptor using LIDT.
 *
 * @param idtDescriptor Pointer to the descriptor describing the IDT.
 */
void KERNEL_CDECL idt_load(idt_descriptor_t* descriptor);

/**
 * @brief Configure a single IDT gate.
 *
 * @param interrupt         Vector number to configure.
 * @param base              Pointer to the handler function.
 * @param segmentDescriptor Code segment selector for the handler.
 * @param flags             Gate attributes (type and privilege).
 */
void idt_set_gate(int interrupt, void* base, uint16_t segment_selector, uint8_t flags)
{
    idt_entries[interrupt].BaseLow            = ((uint32_t)base) & 0xFFFF;
    idt_entries[interrupt].SegmentSelector    = segment_selector;
    idt_entries[interrupt].Reserved           = 0;
    idt_entries[interrupt].Flags              = flags;
    idt_entries[interrupt].BaseHigh           = ((uint32_t)base >> 16) & 0xFFFF;
}

/**
 * @brief Mark an IDT entry as present.
 *
 * @param interrupt Vector number to enable.
 */
void idt_enable_gate(int interrupt)
{
    FLAG_SET(idt_entries[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Mark an IDT entry as not present.
 *
 * @param interrupt Vector number to disable.
 */
void idt_disable_gate(int interrupt)
{
    FLAG_UNSET(idt_entries[interrupt].Flags, IDT_FLAG_PRESENT);
}

/**
 * @brief Load the IDT descriptor into the processor.
 */
void idt_init(void)
{
    idt_load(&idt_descriptor);
}
