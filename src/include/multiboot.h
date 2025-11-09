/**
 * @file include/multiboot.h
 * @brief Multiboot2 tag definitions and cached info.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#pragma once

#include <stdint.h>

#define MULTIBOOT_TAG_TYPE_END                   0
#define MULTIBOOT_TAG_TYPE_CMDLINE               1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME      2
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO         4
#define MULTIBOOT_TAG_TYPE_MMAP                  6
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER           8

#define MULTIBOOT_MEMORY_AVAILABLE               1
#define MULTIBOOT_MEMORY_RESERVED                2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE        3
#define MULTIBOOT_MEMORY_NVS                     4
#define MULTIBOOT_MEMORY_BADRAM                  5

typedef struct
{
    uint32_t type;
    uint32_t size;
} multiboot_tag;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
} multiboot_tag_basic_meminfo;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} multiboot_tag_mmap_header;

typedef struct
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} multiboot_mmap_entry;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} multiboot_tag_framebuffer;

/**
 * @brief Cached Multiboot information used by the kernel.
 */
typedef struct
{
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t mmap_length;
    uint32_t mmap_addr; /**< Physical address of the Multiboot memory map. */
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
} multiboot_info;

multiboot_info* multiboot_get_info(void);
void multiboot_store_info(void* multiboot_header);
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED       0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB           1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT      2
