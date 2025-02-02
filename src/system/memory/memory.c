#include <memory.h>


void* memcpy(void* dst, const void* src, uint16_t num)
{
    uint32_t* u32Dst = (uint32_t *)dst;
    const uint32_t* u32Src = (const uint32_t *)src;

    for (uint16_t i = 0; i < num; i++)
        u32Dst[i] = u32Src[i];

    return dst;
}

void * memset(void * ptr, int value, uint16_t num)
{
    uint32_t* u32Ptr = (uint32_t *)ptr;

    for (uint16_t i = 0; i < num; i++)
        u32Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num)
{
    const uint32_t* u32Ptr1 = (const uint32_t *)ptr1;
    const uint32_t* u32Ptr2 = (const uint32_t *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u32Ptr1[i] != u32Ptr2[i])
            return 1;

    return 0;
}