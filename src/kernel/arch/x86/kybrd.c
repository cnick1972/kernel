#include "kybrd.h"
#include "irq.h"
#include "io.h"
#include "../../stdio.h"


void x86_kybrd_irq(Registers* regs)
{
    unsigned char scancode;
    scancode = x86_inb(0x60);
    if(scancode & 0x80)
    {

    }
    else
    {
        printf("Key Pressed %x\n", scancode);
    }
}

void kkbrd_install(int i)
{
    x86_IRQ_RegisterHandler(1, x86_kybrd_irq);
}

