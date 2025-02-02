#include <meminit.h>
#include <memory.h>
#include <multiboot.h>


multiboot_mmap_entry g_mmap[32];
uint32_t g_mmap_count;

void init_memory(multiboot_info* mbi)
{
    g_mmap_count = mbi->mmap_length / sizeof(multiboot_mmap_entry);
    memcpy(g_mmap, (uint32_t*)(mbi->mmap_addr + 0xc0000000), mbi->mmap_length);
}

uint32_t get_mmap_count()
{
    return g_mmap_count;
}

multiboot_mmap_entry* get_mmap()
{
    return g_mmap;
}