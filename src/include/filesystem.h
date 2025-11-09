/**
 * @file include/filesystem.h
 * @brief Filesystem probing/abstraction interfaces.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <block_device.h>

typedef enum {
    FILESYSTEM_KIND_UNKNOWN = 0,
    FILESYSTEM_KIND_FAT16,
    FILESYSTEM_KIND_EXT2,
    FILESYSTEM_KIND_CDFS,
} filesystem_kind_t;

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
 * This is a stub that will eventually delegate to specific FAT16/EXT2/CDFS
 * implementations.
 *
 * @param device       Target block device.
 * @param lba_start    Starting LBA of the region to probe.
 * @param sector_count Number of sectors available in the region.
 * @return Filesystem kind detected (or FILESYSTEM_KIND_UNKNOWN).
 */
filesystem_kind_t filesystem_probe(block_device_t* device,
                                   uint64_t lba_start,
                                   uint64_t sector_count);
