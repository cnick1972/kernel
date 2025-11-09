#include <meminit.h>
#include <memory.h>
#include <multiboot.h>


/** @brief Cached Multiboot memory map entries. */
static multiboot_mmap_entry g_memory_mmap_entries[32];
/** @brief Number of valid entries currently cached. */
static uint32_t g_memory_mmap_count;

#define MAX_MMAP_ENTRIES (sizeof(g_memory_mmap_entries) / sizeof(g_memory_mmap_entries[0]))

/**
 * @brief Cache the Multiboot-supplied memory map for later use.
 */
void memory_init(multiboot_info* mbi)
{
    uint32_t entries = mbi->mmap_length / sizeof(multiboot_mmap_entry);
    if(entries > MAX_MMAP_ENTRIES)
    {
        entries = MAX_MMAP_ENTRIES;
    }
    g_memory_mmap_count = entries;
    memcpy(g_memory_mmap_entries,
           (void*)(uintptr_t)(mbi->mmap_addr + 0xc0000000),
           entries * sizeof(multiboot_mmap_entry));
}

uint32_t memory_get_mmap_count()
{
    return g_memory_mmap_count;
}

multiboot_mmap_entry* memory_get_mmap()
{
    return g_memory_mmap_entries;
}
