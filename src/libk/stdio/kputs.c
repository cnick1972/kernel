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
