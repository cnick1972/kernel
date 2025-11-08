#pragma once

#include <stdint.h>

/**
 * @brief Initialize and remap the legacy PIC controllers.
 *
 * @param offsetPIC1 Vector offset for the master PIC.
 * @param offsetPIC2 Vector offset for the slave PIC.
 */
void x86_PIC_Configure(uint8_t offsetPIC1, uint8_t offsetPIC2);

/**
 * @brief Notify the PIC that an interrupt has been fully serviced.
 *
 * @param irq IRQ line that has completed.
 */
void x86_PIC_SendEndOfInterrupt(int irq);

/**
 * @brief Mask all IRQ lines (disable the PIC).
 */
void x86_PIC_Disable();

/**
 * @brief Mask (disable) a specific IRQ line.
 *
 * @param irq IRQ line to mask.
 */
void x86_PIC_Mask(int irq);

/**
 * @brief Unmask (enable) a specific IRQ line.
 *
 * @param irq IRQ line to unmask.
 */
void x86_PIC_Unmask(int irq);

/**
 * @brief Read the PIC's Interrupt Request Register (IRR).
 *
 * @return Combined 16-bit IRR value.
 */
uint16_t x86_PIC_ReadIrqRequestRegister();

/**
 * @brief Read the PIC's In-Service Register (ISR).
 *
 * @return Combined 16-bit ISR value.
 */
uint16_t x86_PIC_ReadInServiceRegister();
