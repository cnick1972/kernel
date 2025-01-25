#include <stdio.h>
#include <stdint.h>

#include <multiboot.h>
#include <memory.h>
#include <hal.h>
#include <isr.h>
#include <irq.h>
#include <x86.h>

extern uint8_t __end;

void timer(Registers* regs)
{
    //printf(".");
}


void kmain(uint32_t eax, uint32_t ebx)
{
    multiboot_info* mbi = (multiboot_info*) ebx;

    StoreMultiboot(mbi);
    init_memory(mbi);

    HAL_Initialize();
    x86_IRQ_RegisterHandler(0, timer);

    multiboot_mmap_entry* mmap;

    mmap = get_mmap();
    uint32_t mmap_count = get_mmap_count();
    
    kclrscr();
    for(int i = 0; i < mmap_count; i++)
    {
        kprintf("MEM: region=%d start=0x%08x length=0x%08x type=%d\n", i, 
                                                                        mmap[i].addr_low, mmap[i].len_low, mmap[i].type);
    }
    kprintf("The end 0x%08x\n", &__end);

    int j = 33;
    kprintf("j is %d, at location 0x%08x\n", j, &j);

end:
    for(;;);    

}