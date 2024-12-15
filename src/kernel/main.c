#include <stdint.h>
#include "memory.h"
#include "stdio.h"
#include <hal/hal.h>
#include <arch/x86/io.h>
#include <arch/x86/irq.h>
#include "../libs/boot/bootparams.h"

#include <cpuid.h>

extern uint8_t __bss_start;
extern uint8_t __end;

typedef struct {
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
} model;

static int get_model(model* pModel)
{
    uint32_t unused;
    __cpuid(0, unused, pModel->ebx, pModel->ecx, pModel->edx);
    return 0;
}

void timer(Registers* regs)
{
    //printf(".");
}

void keypressed(Registers* regs)
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


void crash_me();

void __attribute__((section(".entry"))) start(BootParams* bootParams)
{
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    clrscr();

    HAL_Initialize();
    
    model myModel;
    get_model(&myModel);

    char buffer[13];
    memcpy(buffer, &myModel, 12);

    printf("Welcome to the kernel\n");
    printf("CPU Model %s\n", buffer);
    printf("Boot drive: %d\n", bootParams->BootDevice);

    printf("Memory regions count: %d\n", bootParams->Memory.RegionCount);


    for(int i = 0; i < bootParams->Memory.RegionCount; i++) {
        printf("MEM: region=%x\tstart=0x%llx\tlength=0x%llx\ttype=%x\n", i,
                                                                      bootParams->Memory.Regions[i].Begin,
                                                                      bootParams->Memory.Regions[i].Length,
                                                                      bootParams->Memory.Regions[i].Type);
    }

    x86_IRQ_RegisterHandler(0, timer);
    x86_IRQ_RegisterHandler(1, keypressed);

end:
    for(;;);
}