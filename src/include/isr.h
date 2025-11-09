/**
 * @file include/isr.h
 * @brief CPU exception register state and ISR APIs.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once
#include <stdint.h>

typedef struct 
{
    // in the reverse order they are pushed:
    uint32_t ds;                                            // data segment pushed by us
    uint32_t edi, esi, ebp, useless, ebx, edx, ecx, eax;    // pusha
    uint32_t interrupt, error;                              // we push interrupt, error is pushed automatically (or our dummy)
    uint32_t eip, cs, eflags, esp, ss;                      // pushed automatically by CPU
} __attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers* regs);

/**
 * @brief Initialize the CPU exception stubs and enable default gates.
 */
void isr_init(void);

/**
 * @brief Register a high-level handler for a specific interrupt vector.
 *
 * @param interrupt Vector number (0-255).
 * @param handler   Callback to invoke when the interrupt occurs.
 */
void isr_register_handler(int interrupt, ISRHandler handler);
