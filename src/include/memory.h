#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define kfree(a) _kfree((void**)&a)

void* memcpy(void* dst, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);

void* kmalloc(size_t size);
void _kfree(void **ptr);
void debug_heap();