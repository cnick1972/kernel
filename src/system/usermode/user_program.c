/**
 * @file system/usermode/user_program.c
 * @brief Simple demo routine executed in user mode.
 */

#include <stdint.h>
#include <syscall.h>

extern uint32_t syscall_write(uint32_t fd, const void* buffer, uint32_t count);

void user_program_start(void)
{
    const char* msg = "Hello from user mode via syscall write!\n";
    uint32_t len = 42;
    syscall_write(1, msg, len);

    for (;;)
    {
        __asm__ volatile("pause");
    }
}
