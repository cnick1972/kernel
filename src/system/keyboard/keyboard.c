#include <irq.h>
#include <stdio.h>
#include <x86.h>




void keyb_int_handler(Registers* regs)
{
    uint8_t scancode;

    scancode = x86_inb(0x60);
    kprintf("Scancode %x\n", scancode);
    
}