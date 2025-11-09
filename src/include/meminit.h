#pragma once
#include <multiboot.h>
#include <stdint.h>
#include <stdbool.h>
#include <isr.h>
#include <stdio.h>
#include <paging.h>

/**
 * @brief Cache the Multiboot memory map and make it accessible.
 */
void memory_init(multiboot_info* mbi);

/**
 * @brief Retrieve the number of entries in the cached memory map.
 */
uint32_t memory_get_mmap_count();

/**
 * @brief Get a pointer to the cached Multiboot memory map.
 */
multiboot_mmap_entry* memory_get_mmap();

// Physical Memory manager interface functions (implemented in pmm.c)

uint32_t pmm_init_allocator(uint32_t memsize);
uintptr_t pmm_allocate_page();
void pmm_mark_page_reserved(uint32_t page_number);
void pmm_mark_page_free(uint32_t page_number);

page_directory_t vmm_initialize_kernel_page_directory();

uint32_t vmm_make_page_directory_entry(void* page_table_physical_address,
    enum page_size_t page_size,
    bool cache_disabled,
    bool write_through,
    enum page_privilege_t privilege,
    enum page_permissions_t permissions,
    bool present);

uint32_t vmm_make_page_table_entry(void* physical_address,
    bool global,
    bool cache_disabled,
    bool write_through,
    enum page_privilege_t privilege,
    enum page_permissions_t permissions,
    bool present);

uint32_t vmm_count_present_pages(page_directory_t page_directory);
void* vmm_page_table_virtual_address(uint16_t page_table_number);

void vmm_page_fault_handler(Registers* regs);
bool vmm_map_physical_to_virtual(uint8_t* physical_address, uint8_t* virtual_address);
bool vmm_map_4mb_physical_to_virtual(uint8_t* physical_address, uint8_t* virtual_address, size_t size);
