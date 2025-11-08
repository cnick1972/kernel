#include <meminit.h>
#include <memory.h>
#include <multiboot.h>


/** @brief Cached Multiboot memory map entries. */
multiboot_mmap_entry g_mmap[32];
/** @brief Number of valid entries currently cached. */
uint32_t g_mmap_count;

#define MAX_MMAP_ENTRIES (sizeof(g_mmap) / sizeof(g_mmap[0]))

/**
 * @brief Cache the Multiboot-supplied memory map for later use.
 */
void init_memory(multiboot_info* mbi)
{
    uint32_t entries = mbi->mmap_length / sizeof(multiboot_mmap_entry);
    if(entries > MAX_MMAP_ENTRIES)
    {
        entries = MAX_MMAP_ENTRIES;
    }
    g_mmap_count = entries;
    memcpy(g_mmap, (void*)(uintptr_t)(mbi->mmap_addr + 0xc0000000), entries * sizeof(multiboot_mmap_entry));
}

uint32_t get_mmap_count()
{
    return g_mmap_count;
}

multiboot_mmap_entry* get_mmap()
{
    return g_mmap;
}
