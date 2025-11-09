#pragma once

#include <stdint.h>

/**
 * @brief Initialize and remap the legacy PIC controllers.
 *
 * @param offsetPIC1 Vector offset for the master PIC.
 * @param offsetPIC2 Vector offset for the slave PIC.
 */
void pic_configure(uint8_t master_offset, uint8_t slave_offset);

/**
 * @brief Notify the PIC that an interrupt has been fully serviced.
 *
 * @param irq IRQ line that has completed.
 */
void pic_send_end_of_interrupt(int irq);

/**
 * @brief Mask all IRQ lines (disable the PIC).
 */
void pic_disable();

/**
 * @brief Mask (disable) a specific IRQ line.
 *
 * @param irq IRQ line to mask.
 */
void pic_mask_irq(int irq);

/**
 * @brief Unmask (enable) a specific IRQ line.
 *
 * @param irq IRQ line to unmask.
 */
void pic_unmask_irq(int irq);

/**
 * @brief Read the PIC's Interrupt Request Register (IRR).
 *
 * @return Combined 16-bit IRR value.
 */
uint16_t pic_read_irq_request_register();

/**
 * @brief Read the PIC's In-Service Register (ISR).
 *
 * @return Combined 16-bit ISR value.
 */
uint16_t pic_read_in_service_register();
