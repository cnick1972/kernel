/**
 * @file include/hal.h
 * @brief Hardware abstraction layer public API.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

/**
 * @brief Initialize the hardware abstraction layer (GDT, IDT, ISR, IRQ).
 */
void hal_init(void);
