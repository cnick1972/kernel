#include <stdio.h>

void kputs(const char* str)
{
    while(*str)
    {
        kputc(*str);
        str++;
    }
}

