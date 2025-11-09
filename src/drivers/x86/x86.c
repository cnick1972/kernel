/**
 * @file drivers/x86/x86.c
 * @brief C helpers that complement the x86 assembly support routines.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <x86.h>

#define X86_UNUSED_PORT 0x80 /**< Standard dummy port for I/O wait. */

/**
 * @brief Briefly stall the CPU to allow I/O operations to settle.
 */
void x86_iowait()
{
    x86_outb(X86_UNUSED_PORT, 0);
}
