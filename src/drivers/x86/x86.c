#include <x86.h>

#define UNUSED_PORT     0x80

void x86_iowait()
{
    x86_outb(UNUSED_PORT, 0);
}