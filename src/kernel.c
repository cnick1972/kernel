#include <stdio.h>
#include <stdint.h>

#include <multiboot.h>
#include <memory.h>
#include <meminit.h>
#include <hal.h>
#include <isr.h>
#include <irq.h>
#include <x86.h>



void timer(Registers* regs)
{
    kprintf(".");
}

void keyboard(Registers* regs)
{
    kprintf("keyboard\n");
}

void kmain(uint32_t eax, uint32_t ebx)
{

    kclrscr();
    multiboot_info* mbi = (multiboot_info*) ebx;

    StoreMultiboot(mbi);
    kprintf("Memory Map address 0x%08x\n", mbi->mmap_addr);

    init_memory(GetMultiboot());

    HAL_Initialize();
    x86_IRQ_RegisterHandler(0, timer);
    x86_IRQ_RegisterHandler(1, keyboard);


    multiboot_mmap_entry* mmap;

    mmap = get_mmap();
    uint32_t mmap_count = get_mmap_count();
    for(int i = 0; i < mmap_count; i++)
    {
        kprintf("MEM: region=%d start=0x%08x length=0x%08x type=%d\n", i, 
                                                                        mmap[i].addr_low, mmap[i].len_low, mmap[i].type);
    }

    init_pmm_allocator(mbi->mem_upper + 1024);
    page_directory_t pd = initialize_kernel_page_directory();
    kprintf("Address of page directory: 0x%08x\n", pd);
    kprintf("%i present pages\n", num_present_pages(pd));
    x86_ReloadPageDirectory();


end:
    for(;;);    

}