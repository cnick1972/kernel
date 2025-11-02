#pragma once
#include <stdint.h>

typedef enum
{
    IDT_FLAG_GATE_TASK              = 0x5,
    IDT_FLAG_GATE_16BIT_INT         = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP        = 0x7,
    IDT_FLAG_GATE_32BIT_INT         = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP        = 0xF,

    IDT_FLAG_RING0                  = (0 << 5),
    IDT_FLAG_RING1                  = (1 << 5),
    IDT_FLAG_RING2                  = (2 << 5),
    IDT_FLAG_RING3                  = (3 << 5),

    IDT_FLAG_PRESENT                = 0x80,

} IDT_FLAGS;

/**
 * @brief Install an entry in the interrupt descriptor table.
 *
 * @param interrupt         Vector number to configure.
 * @param base              Pointer to the handler function.
 * @param segmentDescriptor Code segment selector to use.
 * @param flags             Gate attributes (type and privilege).
 */
void x86_IDT_SetGate(int interrupt, void* base, uint16_t segmentDescriptor, uint8_t flags);

/**
 * @brief Mark an IDT entry as present.
 *
 * @param interrupt Vector number to enable.
 */
void x86_IDT_EnableGate(int interrupt);

/**
 * @brief Mark an IDT entry as not present.
 *
 * @param interrupt Vector number to disable.
 */
void x86_IDT_DisableGate(int interrupt);

/**
 * @brief Load the IDT descriptor into the processor.
 */
void x86_IDT_Initialize();
