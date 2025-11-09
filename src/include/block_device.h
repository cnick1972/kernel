/**
 * @file include/block_device.h
 * @brief Generic block device abstraction used by storage subsystems.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BLOCK_DEVICE_MAX_NAME 12 /**< Maximum length for block device identifiers. */

typedef struct block_device block_device_t;

/**
 * @brief Block device read callback signature.
 *
 * @param device       Target block device.
 * @param lba          Starting logical block address.
 * @param sector_count Number of sectors to read.
 * @param buffer       Output buffer (must hold sector_count * sector_size bytes).
 * @return true if the read succeeded, false otherwise.
 */
typedef bool (*block_device_read_fn)(block_device_t* device,
                                     uint64_t lba,
                                     uint32_t sector_count,
                                     void* buffer);

struct block_device {
    char                   name[BLOCK_DEVICE_MAX_NAME];
    uint32_t               sector_size;
    uint64_t               sector_count;
    block_device_read_fn   read;
    void*                  driver_data;
};

/**
 * @brief Register a block device with the global storage layer.
 *
 * @param device Device descriptor with callbacks populated.
 * @return true if the device was registered successfully.
 */
bool block_device_register(block_device_t* device);

/**
 * @brief Retrieve a registered block device by index.
 *
 * @param index Index within the registration table.
 * @return Pointer to the block device or NULL if out of range.
 */
block_device_t* block_device_get(size_t index);

/**
 * @brief Obtain the number of registered block devices.
 *
 * @return Count of devices currently registered.
 */
size_t block_device_count(void);
