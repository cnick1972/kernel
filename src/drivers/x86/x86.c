#include <x86.h>

#define UNUSED_PORT     0x80 /**< Standard dummy port for I/O wait. */

/**
 * @brief Briefly stall the CPU to allow I/O operations to settle.
 */
void x86_iowait()
{
    x86_outb(UNUSED_PORT, 0);
}
