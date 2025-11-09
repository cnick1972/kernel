/**
 * @file include/vga.h
 * @brief VGA console control declarations.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

#include <stdint.h>

/**
 * @brief Render a character to the VGA text console.
 *
 * @param c Character to render.
 */
void vga_putchar(char c);

/**
 * @brief Clear the VGA text console and reset the cursor position.
 */
void vga_clear(void);
