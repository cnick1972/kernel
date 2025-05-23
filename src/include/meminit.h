#pragma once
#include <multiboot.h>
#include <stdint.h>
#include <stdbool.h>
#include <isr.h>
#include <stdio.h>

enum page_permissions_t {READ_ONLY, READ_WRITE};
enum page_privilege_t {SUPERVISOR, USER};
enum page_size_t {FOUR_KB, FOUR_MB};

typedef uint32_t * page_directory_t;
typedef uint32_t * page_table_t;

extern void * PageDirectoryVirtualAddress;
extern void * PageDirectoryPhysicalAddress;


void init_memory(multiboot_info* mbi);
uint32_t get_mmap_count();
multiboot_mmap_entry* get_mmap();

// Physical Memory manager interface functions
// These are located in pmm.c

uint32_t init_pmm_allocator(uint32_t memsize);
uintptr_t allocate_physical_page();
void mark_unavailable(uint32_t page_number);
void mark_free(uint32_t page_number) ;


page_directory_t initialize_kernel_page_directory();

uint32_t make_page_directory_entry(void* page_table_physical_address,
    enum page_size_t page_size,
    bool cache_disabled,
    bool write_through,
    enum page_privilege_t privilege,
    enum page_permissions_t permissions,
    bool present);

uint32_t make_page_table_entry(void* physical_address,
    bool global,
    bool cache_disabled,
    bool write_through,
    enum page_privilege_t privilege,
    enum page_permissions_t permissions,
    bool present);


uint32_t num_present_pages(page_directory_t pd);
void* page_table_virtual_address(uint16_t page_table_number);

void PageFaulthandler(Registers* regs);
bool MapPhysicalToVirtual(uint8_t* pAddress, uint8_t* vAddress);
bool Map4MBPhysicalToVirtual(uint8_t* pAddress, uint8_t* vAddress, size_t size);