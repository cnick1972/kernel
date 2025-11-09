/**
 * @file libk/stdio/kputs.c
 * @brief Console string output helper.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <stdio.h>

/**
 * @brief Output a null-terminated string to the active console.
 *
 * @param str String to write.
 */
void kputs(const char* str)
{
    while(*str)
    {
        kputc(*str);
        str++;
    }
}
