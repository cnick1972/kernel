#include <irq.h>
#include <pic.h>
#include <x86.h>
#include <stddef.h>
#include <stdio.h>
#include <isr.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void x86_IRQ_Handler(Registers* regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    uint8_t pic_isr = x86_PIC_ReadInServiceRegister();
    uint8_t pic_irr = x86_PIC_ReadIrqRequestRegister();

    if (g_IRQHandlers[irq] != NULL)
    {
        // handle IRQ
        g_IRQHandlers[irq](regs);
    }
    else
    {
        kprintf("Unhandled IRQ %d  ISR=%x  IRR=%x...\n", irq, pic_isr, pic_irr);
    }

    // send EOI
    x86_PIC_SendEndOfInterrupt(irq);
}

void x86_IRQ_Initialize()
{
    x86_PIC_Configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    for(int i = 0; i < 16; i++)
        x86_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, x86_IRQ_Handler);

    x86_EnableInterrupts();
}

void x86_IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}