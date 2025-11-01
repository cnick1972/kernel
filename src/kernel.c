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
    
    multiboot_info* mbi = (multiboot_info*) ebx;

    StoreMultiboot(mbi);
//    kprintf("Memory Map address 0x%08x\n", mbi->mmap_addr);

//    kprintf("Framebuffer address: 0x%08x\n",mbi->framebuffer_addr);
//    kprintf("Framebuffer height: %d\n", mbi->framebuffer_height);
//    kprintf("framebuffer type: %d\n", mbi->framebuffer_type);

    init_memory(GetMultiboot());
    InitSerial();
    SerialPrintf("Hello World\n");

    HAL_Initialize();
    x86_IRQ_RegisterHandler(0, timer);
    x86_IRQ_RegisterHandler(1, keyb_int_handler);
    x86_ISR_RegisterHandler(14, PageFaulthandler);

    multiboot_mmap_entry* mmap;

    mmap = get_mmap();
    uint32_t mmap_count = get_mmap_count();
    for(int i = 0; i < mmap_count; i++)
    {
        SerialPrintf("MEM: region=%d start=0x%08x length=0x%08x type=%d\n", i, 
                    mmap[i].addr_low, mmap[i].len_low, mmap[i].type);
    }

    init_pmm_allocator(mbi->mem_upper + 1024);
    page_directory_t pd = initialize_kernel_page_directory();
    x86_ReloadPageDirectory();
    console_init(mbi);

// We can now print to the screen

    kclrscr();


    uint32_t* ptr = 0; 
    ptr = (uint32_t*)find_rsdp();

    SerialPrintf("framebuffer type: %d\n", mbi->framebuffer_type);

    kprintf("Memory Map address 0x%08x\n", mbi->mmap_addr);

    kprintf("Framebuffer address: 0x%08x\n",mbi->framebuffer_addr);
    kprintf("Framebuffer height: %d\n", mbi->framebuffer_height);
    kprintf("framebuffer type: %d\n", mbi->framebuffer_type);

    pci_enumerate();


end:
    for(;;);    
}