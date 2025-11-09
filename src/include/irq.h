#pragma once
#include <isr.h>

typedef void (*IRQHandler)(Registers* regs);

/**
 * @brief Initialize the PIC and hook IRQ handlers into the IDT.
 */
void irq_init(void);

/**
 * @brief Register a high-level handler for a hardware IRQ.
 *
 * @param irq      IRQ line number (0-15).
 * @param handler  Callback to execute when the IRQ fires.
 */
void irq_register_handler(int irq, IRQHandler handler);
