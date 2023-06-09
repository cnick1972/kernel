#include <hal.h>
#include "cpu.h"
#include "idt.h"


int hal_initialize() {
    i86_cpu_initialize();
    return 0;
}

int hal_shutdown() {
    i86_cpu_shutdown();
    return 0;
}

void geninterrupt(int n) {
    _geninterrupt(n);
}