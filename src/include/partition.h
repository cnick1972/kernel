/**
 * @file include/partition.h
 * @brief Partition discovery APIs for block devices.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <block_device.h>
#include <filesystem.h>

typedef struct {
    uint8_t          index;        /**< Partition table entry index. */
    uint64_t         lba_start;    /**< First LBA belonging to the partition. */
    uint64_t         sector_count; /**< Length in sectors. */
    filesystem_kind_t fs_kind;     /**< Filesystem type (once probed). */
} partition_info_t;

/**
 * @brief Scan a block device for partitions and associated filesystems.
 *
 * @param device Target block device.
 */
void partition_scan_device(block_device_t* device);
