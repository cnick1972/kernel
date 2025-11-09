#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Copy a block of memory.
 *
 * @param dst Destination buffer.
 * @param src Source buffer.
 * @param num Number of bytes to copy.
 * @return Pointer to dst.
 */
void* memcpy(void* dst, const void* src, size_t num);

/**
 * @brief Fill a block of memory with a value.
 *
 * @param ptr   Destination buffer.
 * @param value Byte value to write.
 * @param num   Number of bytes to write.
 * @return Pointer to ptr.
 */
void* memset(void* ptr, int value, size_t num);

/**
 * @brief Compare two memory blocks.
 *
 * @param ptr1 First buffer.
 * @param ptr2 Second buffer.
 * @param num  Number of bytes to compare.
 * @return 0 if equal, non-zero otherwise.
 */
int memcmp(const void* ptr1, const void* ptr2, size_t num);

/**
 * @brief Allocate heap memory.
 *
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory or NULL.
 */
void* kmalloc(size_t size);

/**
 * @brief Free heap memory previously allocated via kmalloc.
 *
 * @param ptr_handle Pointer-to-pointer returned by kmalloc (cleared on return).
 */
void kfree(void **ptr_handle);

/**
 * @brief Dump the current heap layout to the console.
 */
void kheap_debug_dump(void);
