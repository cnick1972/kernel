#include <memory.h>


/**
 * @brief Copy a block of memory.
 */
void* memcpy(void* dst, const void* src, size_t num)
{
    uint8_t* dst_bytes = (uint8_t *)dst;
    const uint8_t* src_bytes = (const uint8_t *)src;

    for (size_t i = 0; i < num; i++)
        dst_bytes[i] = src_bytes[i];

    return dst;
}

/**
 * @brief Fill memory with a constant byte value.
 */
void * memset(void * ptr, int value, size_t num)
{
    uint8_t* bytes = (uint8_t *)ptr;

    for (size_t i = 0; i < num; i++)
        bytes[i] = (uint8_t)value;

    return ptr;
}

/**
 * @brief Compare two memory regions.
 */
int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    const uint8_t* first_bytes = (const uint8_t *)ptr1;
    const uint8_t* second_bytes = (const uint8_t *)ptr2;

    for (size_t i = 0; i < num; i++)
        if (first_bytes[i] != second_bytes[i])
            return 1;

    return 0;
}
