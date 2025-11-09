/**
 * @file kernel.c
 * @brief Kernel entry point and top-level initialization.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <stdio.h>
#include <stdint.h>

#include <keyboard.h>
#include <multiboot.h>
#include <memory.h>
#include <meminit.h>
#include <hal.h>
#include <isr.h>
#include <irq.h>
#include <x86.h>
#include <acpi.h>
#include <console.h>
#include <serial.h>
#include <pci.h>

void timer(Registers* regs)
{
    //kprintf(".");
}

void kmain(uint32_t eax, uint32_t ebx)
{
    
    multiboot_store_info((void*) ebx);

    memory_init(multiboot_get_info());

    serial_init();
    hal_init();
    irq_register_handler(0, timer);
    irq_register_handler(1, keyboard_irq_handler);
    isr_register_handler(14, vmm_page_fault_handler);

    multiboot_mmap_entry* mmap;

    mmap = memory_get_mmap();
    
    uint32_t mmap_count = memory_get_mmap_count();
    for(int i = 0; i < mmap_count; i++)
    {
        serial_printf("MEM: region=%d start=0x%016llx length=0x%016llx type=%d\n", i,
                    (unsigned long long)mmap[i].addr,
                    (unsigned long long)mmap[i].len,
                    mmap[i].type);
    }

    pmm_init_allocator(multiboot_get_info()->mem_upper + 1024);
    page_directory_t pd = vmm_initialize_kernel_page_directory();
    x86_reload_page_directory();
    console_init(multiboot_get_info());

    serial_printf("Memory Map: 0x%08x\n", mmap);

// We can now print to the screen

    kclrscr();


    //uint32_t* ptr = 0; 
    uint32_t* ptr = (uint32_t*)find_rsdp();

    serial_printf("framebuffer type: %d\n", multiboot_get_info()->framebuffer_type);

    kprintf("Memory Map address 0x%08x\n", multiboot_get_info()->mmap_addr);

    kprintf("Framebuffer address: 0x%08x%08x\n",
            (uint32_t)(multiboot_get_info()->framebuffer_addr >> 32),
            (uint32_t)multiboot_get_info()->framebuffer_addr);
    kprintf("Framebuffer height: %d\n", multiboot_get_info()->framebuffer_height);
    kprintf("framebuffer type: %d\n", multiboot_get_info()->framebuffer_type);

    pci_enumerate();


end:
    for(;;);    
}
