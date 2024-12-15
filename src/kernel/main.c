#include <stdint.h>
#include <cpuid.h>
#include "memory.h"
#include "stdio.h"
#include "string.h"
#include <hal/hal.h>
#include <arch/x86/io.h>
#include <arch/x86/irq.h>
#include <arch/x86/kybrd.h>
#include "../libs/boot/bootparams.h"


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

void crash_me();

enum KEYCODE getch() {
    enum KEYCODE key = KEY_UNKNOWN;
    while(key == KEY_UNKNOWN)
        key = kkybrd_get_last_key ();
	kkybrd_discard_last_key ();
	return key;  
}

void cmd() {
    printf("\nCommand>");
}

void get_cmd(char* buf, int n) {
    cmd();

    enum KEYCODE key = KEY_UNKNOWN;
    bool BufChar;

    int i = 0;

    while(i < n) {
        BufChar = true;

        key = getch();

        if(key == KEY_RETURN)
            break;

        if(BufChar) {
            char c = kkybrd_key_to_ascii (key);
            if(c != 0) {
                putc(c);
                buf[i++] = c;
            }
        }
    }
    buf[i] = '\0';

}

bool run_cmd(char* cmd_buf) {

    if(strcmp(cmd_buf, "exit") == 0) {
        clrscr();
        printf("Goodbye");
        return true;
    }
    else if(strcmp(cmd_buf, "cls") == 0) {
        clrscr();
    }
    else if(strcmp(cmd_buf, "help") == 0) {
		puts("\nOS Development Series Keyboard Programming Demo");
		puts("\nwith a basic Command Line Interface (CLI)\n\n");
		puts("Supported commands:\n");
		puts(" - exit: quits and halts the system\n");
		puts(" - cls: clears the display\n");
		puts(" - help: displays this message\n");
    }
    else
    {
        printf("\nUnknown Command");
    }
    return false;
}

void run() {
    char cmd_buf[100];

    while(1) {
        get_cmd(cmd_buf, 98);

        if(run_cmd(cmd_buf))
            break;
    }
}

void __attribute__((section(".entry"))) start(BootParams* bootParams)
{
    memset(&__bss_start, 0, (&__end) - (&__bss_start));

    clrscr();

    HAL_Initialize();
    x86_IRQ_RegisterHandler(0, timer);
    kkbrd_install(1);

    char buffer[13] = "Unknown\0";
    
    if(x86_cpuid()) {
        model myModel;
        get_model(&myModel);

        memset(&buffer, 0, 13);
        memcpy(buffer, &myModel, 12);
    }

    printf("Welcome to the kernel\n");
    printf("CPU Model: %s\n", buffer);
    printf("Boot drive: %d\n", bootParams->BootDevice);

// print the computer memory, this will need fixing for older machines

    printf("Memory regions count: %d\n", bootParams->Memory.RegionCount);
    for(int i = 0; i < bootParams->Memory.RegionCount; i++) {
        printf("MEM: region=%x\tstart=0x%llx\tlength=0x%llx\ttype=%x\n", i,
                                                                      bootParams->Memory.Regions[i].Begin,
                                                                      bootParams->Memory.Regions[i].Length,
                                                                      bootParams->Memory.Regions[i].Type);
    }
    printf("\nPress any key to continue...\n");
    getch();
    clrscr();

    run();

end:
    for(;;);
}