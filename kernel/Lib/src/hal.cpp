#include <hal.h>


int hal_initialize() {
    return 0;
}

int hal_shutdown() {
    return 0;
}

void geninterrupt(int n) {
    _geninterrupt(n);
}