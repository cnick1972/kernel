/**
 * @file system/tss/tss.c
 * @brief Task state segment initialization.
 */

#include <tss.h>
#include <gdt.h>
#include <memory.h>

typedef struct __attribute__((packed)) {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} tss_entry_t;

static tss_entry_t g_tss;

void tss_set_kernel_stack(uint32_t kernel_stack_top)
{
    g_tss.esp0 = kernel_stack_top;
}

void tss_init(uint32_t kernel_stack_top)
{
    memset(&g_tss, 0, sizeof(g_tss));
    g_tss.ss0 = GDT_SELECTOR_DATA;
    g_tss.esp0 = kernel_stack_top;
    g_tss.cs = GDT_SELECTOR_USER_CODE;
    g_tss.ss = g_tss.ds = g_tss.es = g_tss.fs = g_tss.gs = GDT_SELECTOR_USER_DATA;
    g_tss.iomap_base = sizeof(tss_entry_t);

    gdt_set_entry_raw(GDT_INDEX_TSS,
                      (uint32_t)&g_tss,
                      sizeof(tss_entry_t),
                      GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_TSS_AVAILABLE,
                      GDT_FLAG_32BIT);

    uint16_t selector = GDT_SELECTOR_TSS;
    __asm__ volatile("ltr %0" : : "r"(selector));
}
