/**
 * @file storage/block_device.c
 * @brief Global block device registry and helpers.
 */

#include <block_device.h>
#include <stdio.h>

#define BLOCK_DEVICE_MAX_COUNT 16

static block_device_t* g_block_devices[BLOCK_DEVICE_MAX_COUNT];
static size_t g_block_device_count;

bool block_device_register(block_device_t* device)
{
    if (device == NULL || device->read == NULL)
    {
        kprintf("BLOCK: reject device registration (invalid descriptor)\n");
        return false;
    }

    if (g_block_device_count >= BLOCK_DEVICE_MAX_COUNT)
    {
        kprintf("BLOCK: registry full, cannot register %s\n", device->name);
        return false;
    }

    g_block_devices[g_block_device_count++] = device;

    kprintf("BLOCK: registered %s sector_size=%u sectors=%llu\n",
            device->name,
            device->sector_size,
            (unsigned long long)device->sector_count);

    return true;
}

block_device_t* block_device_get(size_t index)
{
    if (index >= g_block_device_count)
    {
        return NULL;
    }
    return g_block_devices[index];
}

size_t block_device_count(void)
{
    return g_block_device_count;
}
