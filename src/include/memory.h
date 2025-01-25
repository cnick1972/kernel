#pragma once
#include <multiboot.h>
#include <stdint.h>


void init_memory(multiboot_info* mbi);
uint32_t get_mmap_count();
multiboot_mmap_entry* get_mmap();

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);