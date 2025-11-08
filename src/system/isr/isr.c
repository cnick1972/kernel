#include <isr.h>
#include <idt.h>
#include <gdt.h>
#include <x86.h>
#include <stdio.h>
#include <stddef.h>
#include <kerndef.h>

/** @brief High-level ISR handler table (vector-indexed). */
ISRHandler g_ISRHandlers[256];

/** @brief Human-readable names for processor exceptions. */
static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
};

/** @brief Assembly stub that populates the IDT with ISR entries. */
void x86_ISR_InitializeGates();

/**
 * @brief Initialize default CPU exception handlers and enable IDT gates.
 */
void x86_ISR_Initialize()
{
    x86_ISR_InitializeGates();
    for(int i = 0; i < 256; i++)
        x86_IDT_EnableGate(i);
    x86_IDT_DisableGate(0x80);
}

/**
 * @brief Central ISR dispatcher invoked from assembly stubs.
 *
 * @param regs Register snapshot captured on interrupt entry.
 */
void KERNEL_CDECL x86_ISR_Handler(Registers* regs)
{
    if(g_ISRHandlers[regs->interrupt] != NULL)
        g_ISRHandlers[regs->interrupt](regs);
    else if(regs->interrupt >= 32)
        kprintf("Unhandled interrupt %d\n", regs->interrupt);

    else
    {
        kprintf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        
        kprintf("  eax=%x  ebx=%x  ecx=%x  edx=%x  esi=%x  edi=%x\n",
               regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);

        kprintf("  esp=%x  ebp=%x  eip=%x  eflags=%x  cs=%x  ds=%x  ss=%x\n",
               regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss);

        kprintf("  interrupt=%x  errorcode=%x\n", regs->interrupt, regs->error);

        kprintf("KERNEL PANIC!\n");
        x86_Panic();        
    }
}

/**
 * @brief Register a high-level ISR callback and enable its gate.
 *
 * @param interrupt Vector number (0-255).
 * @param handler   Callback to execute when the interrupt fires.
 */
void x86_ISR_RegisterHandler(int interrupt, ISRHandler handler)
{
    g_ISRHandlers[interrupt] = handler;
    x86_IDT_EnableGate(interrupt);
}
