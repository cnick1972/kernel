/**
 * @file include/syscall.h
 * @brief System call dispatcher interfaces.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define SYSCALL_INTERRUPT_VECTOR 0x80
#define SYSCALL_MAX              512

typedef uint32_t (*syscall_handler_t)(uint32_t arg0,
                                      uint32_t arg1,
                                      uint32_t arg2,
                                      uint32_t arg3,
                                      uint32_t arg4);

#define SYSCALL_NR_EXIT        1
#define SYSCALL_NR_FORK        2
#define SYSCALL_NR_READ        3
#define SYSCALL_NR_WRITE       4
#define SYSCALL_NR_OPEN        5
#define SYSCALL_NR_CLOSE       6
#define SYSCALL_NR_GETPID     20
#define SYSCALL_NR_GETPPID    64
#define SYSCALL_NR_STAT       106
#define SYSCALL_NR_LSTAT      107
#define SYSCALL_NR_FSTAT      108
#define SYSCALL_NR_GETDENTS   141

void syscall_init(void);
bool syscall_register(uint32_t number,
                      syscall_handler_t handler,
                      const char* name);
