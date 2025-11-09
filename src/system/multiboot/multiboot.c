#include <multiboot.h>
#include <memory.h>

/** @brief Cached copy of the Multiboot information block. */
static multiboot_info g_cached_mbi;

void multiboot_store_info(multiboot_info* mbi)
{
    memcpy(&g_cached_mbi, mbi, sizeof(multiboot_info));
}

multiboot_info* multiboot_get_info(void)
{
    return &g_cached_mbi;
}
