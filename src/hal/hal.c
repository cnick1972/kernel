#include <hal.h>
#include <gdt.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>

/**
 * @brief Initialize core processor structures and interrupt plumbing.
 */
void hal_init()
{
    gdt_init();
    idt_init();
    isr_init();
    irq_init();
}
