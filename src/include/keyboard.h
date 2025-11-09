#pragma once
#include <isr.h>

/**
 * @brief Keyboard interrupt service routine.
 *
 * @param regs Register snapshot provided by ISR dispatcher.
 */
void keyboard_irq_handler(Registers* regs);
