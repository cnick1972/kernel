/**
 * @file system/syscall/syscall_user.c
 * @brief User-mode syscall stubs.
 */

#include <syscall.h>

extern uint32_t usermode_syscall(uint32_t eax,
                                 uint32_t ebx,
                                 uint32_t ecx,
                                 uint32_t edx);

uint32_t syscall_write(uint32_t fd, const void* buffer, uint32_t count)
{
    return usermode_syscall(SYSCALL_NR_WRITE, fd, (uint32_t)buffer, count);
}
