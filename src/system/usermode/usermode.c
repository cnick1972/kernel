/**
 * @file system/usermode/usermode.c
 * @brief User-mode transition helpers.
 */

#include <usermode.h>
#include <stdint.h>
#include <stddef.h>

extern void usermode_trampoline(uint32_t entry, uint32_t stack_top);

static uint8_t g_user_stack[8192] __attribute__((aligned(16)));

void usermode_enter(void (*entry)(void))
{
    if (entry == NULL)
    {
        return;
    }

    uint32_t user_stack = (uint32_t)(g_user_stack + sizeof(g_user_stack));
    usermode_trampoline((uint32_t)entry, user_stack);
}
