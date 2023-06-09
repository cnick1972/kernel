#include "cpu.h"
#include "gdt.h"
#include "idt.h"


int i86_cpu_initialize() {
    i86_gdt_initialize();
    i86_idt_initialize(0x8);

    return 0;
}

void i86_cpu_shutdown() {
    
}


