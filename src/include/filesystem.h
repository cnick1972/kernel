/**
 * @file include/filesystem.h
 * @brief Filesystem probing/abstraction interfaces.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <block_device.h>

typedef enum {
    FILESYSTEM_KIND_UNKNOWN = 0,
    FILESYSTEM_KIND_FAT16,
    FILESYSTEM_KIND_EXT2,
    FILESYSTEM_KIND_CDFS,
} filesystem_kind_t;

#define FILESYSTEM_MOUNT_MAX_NAME 16
#define FILESYSTEM_MAX_MOUNTS     8

typedef struct {
    char               name[FILESYSTEM_MOUNT_MAX_NAME];
    filesystem_kind_t  kind;
    block_device_t*    device;
    uint64_t           lba_start;
    uint64_t           sector_count;
    uint8_t            partition_index;
    void*              driver_data;
} filesystem_mount_t;

/**
 * @brief Convert a filesystem kind enum into a printable label.
 *
 * @param kind Filesystem kind.
 * @return Static string describing the filesystem kind.
 */
const char* filesystem_kind_name(filesystem_kind_t kind);

/**
 * @brief Probe a slice of a block device for a supported filesystem.
 *
 * @param device       Target block device.
 * @param lba_start    Starting LBA of the region to probe.
 * @param sector_count Number of sectors available in the region.
 * @return Filesystem kind detected (or FILESYSTEM_KIND_UNKNOWN).
 */
filesystem_kind_t filesystem_probe(block_device_t* device,
                                   uint64_t lba_start,
                                   uint64_t sector_count);

/**
 * @brief Register a mounted filesystem instance for a partition.
 *
 * @param device         Parent block device.
 * @param partition_idx  Partition slot index on the device.
 * @param lba_start      Partition start LBA.
 * @param sector_count   Partition length in sectors.
 * @param kind           Filesystem type detected on the partition.
 * @return true if the mount succeeded.
 */
bool filesystem_mount_partition(block_device_t* device,
                                uint8_t partition_idx,
                                uint64_t lba_start,
                                uint64_t sector_count,
                                filesystem_kind_t kind);

/**
 * @brief Retrieve a mounted filesystem descriptor by index.
 *
 * @param index Mount index.
 * @return Pointer to the mount descriptor or NULL if out of range.
 */
filesystem_mount_t* filesystem_get_mount(size_t index);

/**
 * @brief Obtain the number of mounted filesystems.
 *
 * @return Count of mounted filesystems.
 */
size_t filesystem_mount_count(void);

/**
 * @brief Read an arbitrary byte range from a block device partition.
 *
 * @param device       Target block device.
 * @param lba_start    Partition starting LBA.
 * @param byte_offset  Offset from the start of the partition in bytes.
 * @param byte_length  Number of bytes to read.
 * @param scratch      Caller-provided scratch buffer receiving the data.
 * @param scratch_size Size in bytes of the scratch buffer.
 * @param view_out     Optional pointer within scratch adjusted for offset.
 * @return true if the byte range was read successfully.
 */
bool filesystem_read_bytes(block_device_t* device,
                           uint64_t lba_start,
                           uint32_t byte_offset,
                           uint32_t byte_length,
                           uint8_t* scratch,
                           size_t scratch_size,
                           uint8_t** view_out);
