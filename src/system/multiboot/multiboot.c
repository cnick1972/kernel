#include <multiboot.h>
#include <memory.h>

multiboot_info g_mbi;

void StoreMultiboot(multiboot_info* mbi)
{
    memcpy(&g_mbi, mbi, sizeof(multiboot_info));
}

multiboot_info* GetMultiboot()
{
    return &g_mbi;
}