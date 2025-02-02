#pragma once
#include <multiboot.h>
#include <stdint.h>


void init_memory(multiboot_info* mbi);
uint32_t get_mmap_count();
multiboot_mmap_entry* get_mmap();