#pragma once
#include <multiboot.h>
#include <stdint.h>

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
void* allocate_physical_page();


page_directory_t initialize_kernel_page_directory();
