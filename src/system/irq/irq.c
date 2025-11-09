#include <irq.h>
#include <pic.h>
#include <x86.h>
#include <stddef.h>
#include <stdio.h>
#include <isr.h>

#define PIC_REMAP_OFFSET        0x20 /**< Offset applied when remapping the PIC. */

/** @brief High-level IRQ handler table (one entry per IRQ line). */
IRQHandler g_IRQHandlers[16];

/**
 * @brief Low-level IRQ dispatcher invoked from the ISR stubs.
 *
 * @param regs Register snapshot captured on interrupt entry.
 */
void x86_IRQ_Handler(Registers* regs)
{
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    uint8_t pic_isr = pic_read_in_service_register();
    uint8_t pic_irr = pic_read_irq_request_register();

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
    pic_send_end_of_interrupt(irq);
}

/**
 * @brief Initialize the PIC and install IRQ handlers into the IDT.
 */
void x86_IRQ_Initialize()
{
    pic_configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8);

    for(int i = 0; i < 16; i++)
        x86_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, x86_IRQ_Handler);

    x86_EnableInterrupts();
}

/**
 * @brief Register a high-level IRQ handler callback.
 *
 * @param irq     IRQ line number (0-15).
 * @param handler Callback to invoke when the IRQ fires.
 */
void x86_IRQ_RegisterHandler(int irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}
