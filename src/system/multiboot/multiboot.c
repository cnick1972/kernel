#include <multiboot.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>

#define ALIGN_UP(value, align) (((value) + ((align) - 1)) & ~((align) - 1))
#define KERNEL_VIRTUAL_BASE 0xC0000000u

static multiboot_info g_multiboot_info;

static uint32_t virtual_to_physical(uintptr_t address)
{
    if(address >= KERNEL_VIRTUAL_BASE)
    {
        return (uint32_t)(address - KERNEL_VIRTUAL_BASE);
    }
    return (uint32_t)address;
}

void multiboot_store_info(void* multiboot_header)
{
    memset(&g_multiboot_info, 0, sizeof(g_multiboot_info));

    uintptr_t base = (uintptr_t) multiboot_header;
    uint32_t total_size = *((uint32_t*) base);
    uintptr_t tag_ptr = base + 8;

    while(tag_ptr < (base + total_size))
    {
        multiboot_tag* tag = (multiboot_tag*) tag_ptr;

        switch(tag->type)
        {
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            {
                multiboot_tag_basic_meminfo* mem = (multiboot_tag_basic_meminfo*) tag;
                g_multiboot_info.mem_lower = mem->mem_lower;
                g_multiboot_info.mem_upper = mem->mem_upper;
                break;
            }

            case MULTIBOOT_TAG_TYPE_MMAP:
            {
                multiboot_tag_mmap_header* mmap = (multiboot_tag_mmap_header*) tag;
                uint32_t entries_size = mmap->size - sizeof(multiboot_tag_mmap_header);
                uintptr_t entries_virtual = (uintptr_t)mmap + sizeof(multiboot_tag_mmap_header);
                g_multiboot_info.mmap_length = entries_size;
                g_multiboot_info.mmap_addr = virtual_to_physical(entries_virtual);
                break;
            }

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            {
                multiboot_tag_framebuffer* fb = (multiboot_tag_framebuffer*) tag;
                g_multiboot_info.framebuffer_addr = fb->framebuffer_addr;
                g_multiboot_info.framebuffer_pitch = fb->framebuffer_pitch;
                g_multiboot_info.framebuffer_width = fb->framebuffer_width;
                g_multiboot_info.framebuffer_height = fb->framebuffer_height;
                g_multiboot_info.framebuffer_bpp = fb->framebuffer_bpp;
                g_multiboot_info.framebuffer_type = fb->framebuffer_type;
                break;
            }

            case MULTIBOOT_TAG_TYPE_END:
                return;

            default:
                break;
        }

        tag_ptr += ALIGN_UP(tag->size, 8);
    }
}

multiboot_info* multiboot_get_info(void)
{
    return &g_multiboot_info;
}
