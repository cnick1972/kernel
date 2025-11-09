/**
 * @file libk/stdio/kclrscr.c
 * @brief Console clear screen helper.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <stdio.h>
#include <console.h>

/**
 * @brief Clear the active console screen.
 */
void kclrscr()
{
    console_clear();
}
