/**
 * @file system/syscall/syscall.c
 * @brief System call dispatcher and default handlers.
 */

#include <syscall.h>
#include <isr.h>
#include <idt.h>
#include <gdt.h>
#include <stdio.h>
#include <memory.h>
#include <vfs.h>
#include <kerndef.h>

extern void KERNEL_CDECL x86_ISR128(void);

typedef struct {
    syscall_handler_t handler;
    const char*       name;
} syscall_entry_t;

static syscall_entry_t g_syscalls[SYSCALL_MAX];

static uint32_t syscall_unimplemented(uint32_t arg0,
                                      uint32_t arg1,
                                      uint32_t arg2,
                                      uint32_t arg3,
                                      uint32_t arg4)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    return (uint32_t)-1;
}

static void syscall_isr_handler(Registers* regs)
{
    uint32_t number = regs->eax;
    if (number >= SYSCALL_MAX || g_syscalls[number].handler == NULL)
    {
        regs->eax = (uint32_t)-1;
        return;
    }

    syscall_handler_t handler = g_syscalls[number].handler;
    regs->eax = handler(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}

bool syscall_register(uint32_t number,
                      syscall_handler_t handler,
                      const char* name)
{
    if (number >= SYSCALL_MAX || handler == NULL)
    {
        return false;
    }

    g_syscalls[number].handler = handler;
    g_syscalls[number].name = name;
    return true;
}

static uint32_t sys_write(uint32_t fd,
                          uint32_t buffer_ptr,
                          uint32_t count,
                          uint32_t unused3,
                          uint32_t unused4)
{
    (void)unused3;
    (void)unused4;
    if (buffer_ptr == 0 || count == 0)
    {
        return (uint32_t)-1;
    }

    const char* buffer = (const char*)buffer_ptr;
    if (fd == 1 || fd == 2)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            char ch = buffer[i];
            if (ch == '\0')
            {
                break;
            }
            kputc(ch);
        }
        return count;
    }

    return (uint32_t)-1;
}

static uint32_t sys_vfs_list(uint32_t dirfd,
                             uint32_t dirp,
                             uint32_t count,
                             uint32_t unused3,
                             uint32_t unused4)
{
    (void)dirfd;
    (void)dirp;
    (void)count;
    (void)unused3;
    (void)unused4;
    vfs_print_mounts();
    return 0;
}

void syscall_init(void)
{
    memset(g_syscalls, 0, sizeof(g_syscalls));

    idt_set_gate(SYSCALL_INTERRUPT_VECTOR,
                 x86_ISR128,
                 GDT_SELECTOR_CODE,
                 IDT_FLAG_RING3 | IDT_FLAG_GATE_32BIT_INT | IDT_FLAG_PRESENT);
    idt_enable_gate(SYSCALL_INTERRUPT_VECTOR);
    isr_register_handler(SYSCALL_INTERRUPT_VECTOR, syscall_isr_handler);

    for (uint32_t i = 0; i < SYSCALL_MAX; ++i)
    {
        g_syscalls[i].handler = syscall_unimplemented;
        g_syscalls[i].name = "unimplemented";
    }

    syscall_register(SYSCALL_NR_WRITE, sys_write, "write");
    syscall_register(SYSCALL_NR_GETDENTS, sys_vfs_list, "getdents");

    kprintf("Syscall: initialized vector 0x%02x with %u slots\n",
            SYSCALL_INTERRUPT_VECTOR,
            (unsigned)SYSCALL_MAX);
}
