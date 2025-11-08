#include <multiboot.h>
#include <memory.h>

/** @brief Cached copy of the Multiboot information block. */
multiboot_info g_mbi;

void StoreMultiboot(multiboot_info* mbi)
{
    memcpy(&g_mbi, mbi, sizeof(multiboot_info));
}

multiboot_info* GetMultiboot()
{
    return &g_mbi;
}
