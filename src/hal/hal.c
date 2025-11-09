/**
 * @file hal/hal.c
 * @brief Initializes core hardware abstraction layer components.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

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
