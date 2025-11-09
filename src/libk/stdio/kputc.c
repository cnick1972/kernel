/**
 * @file libk/stdio/kputc.c
 * @brief Console character output helper.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <stdio.h>
#include <console.h>

/**
 * @brief Output a character to the active console.
 *
 * @param c Character to write.
 */
void kputc(const char c)
{
    console_putc(c);
}
