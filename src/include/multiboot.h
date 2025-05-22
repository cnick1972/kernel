#pragma once

#include <stdint.h>

typedef struct
{
    uint32_t tabsize;
    uint32_t strsize;
    uint32_t addr;
    uint32_t reserved;
} __attribute__((packed)) multiboot_aout_symbol_table;

typedef struct 
{
    uint32_t size;
    uint32_t num;
    uint32_t addr;
    uint32_t shndx;
} __attribute__((packed)) multiboot_elf_section_header_table;

typedef struct
{
    uint32_t flags;

    uint32_t mem_lower;
    uint32_t mem_upper;

    uint32_t boot_device;

    uint32_t cmdline;

    uint32_t mods_count;
    uint32_t mods_addr;

    union
    {
    multiboot_aout_symbol_table aout_sym;
    multiboot_elf_section_header_table elf_sec;
    } u;

    uint32_t mmap_length;
    uint32_t mmap_addr;

    uint32_t drives_length;
    uint32_t drives_addr;

    uint32_t config_table;

    uint32_t bootloader_name;

    uint32_t apm_table;

    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;

    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
    uint8_t framebuffer_type;

    union
    {
        struct
        {
            uint32_t framebuffer_palette_addr;
            uint16_t framebuffer_palette_num_colors;
        };
        struct
        {
            uint8_t framebuffer_red_field_position;
            uint8_t framebuffer_red_mask_size;
            uint8_t framebuffer_green_field_position;
            uint8_t framebuffer_green_mask_size;
            uint8_t framebuffer_blue_field_position;
            uint8_t framebuffer_blue_mask_size;
        };
    };   
} __attribute__((packed)) multiboot_info;

typedef struct
{
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
    uint32_t type;
} __attribute__((packed))  multiboot_mmap_entry;




#define MULTIBOOT_TAG_TYPE_END                  0
#define MULTIBOOT_TAG_TYPE_CMDLINE              1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME     2
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER          8
#define MULTIBOOT_TAG_TYPE_MMAP                 6

typedef struct {
    uint32_t    type;
    uint32_t    size;
} multiboot_tag;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint16_t reserved;
} multiboot_tag_framebuffer;






multiboot_info* GetMultiboot();
void StoreMultiboot(multiboot_info* mbi);