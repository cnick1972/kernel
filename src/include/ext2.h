/**
 * @file include/ext2.h
 * @brief EXT2 filesystem interfaces.
 */

#pragma once

#include <stdbool.h>

#include <filesystem.h>
#include <vfs.h>

bool ext2_mount(filesystem_mount_t* mount);
bool ext2_list_root(const filesystem_mount_t* mount,
                    vfs_dir_entry_callback_t callback,
                    void* context);

extern const vfs_filesystem_ops_t g_ext2_vfs_ops;
