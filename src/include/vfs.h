/**
 * @file include/vfs.h
 * @brief Virtual filesystem abstraction layer.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <filesystem.h>

typedef bool (*vfs_dir_entry_callback_t)(const char* name,
                                         uint32_t inode,
                                         uint8_t type,
                                         void* context);

struct vfs_file;
typedef struct vfs_file vfs_file_t;

typedef struct {
    const char* fs_name;
    bool (*list_root)(const filesystem_mount_t* mount,
                      vfs_dir_entry_callback_t callback,
                      void* context);
    bool (*open)(const filesystem_mount_t* mount,
                 const char* path,
                 vfs_file_t* file);
    void (*close)(vfs_file_t* file);
} vfs_filesystem_ops_t;

struct vfs_file {
    const filesystem_mount_t*     mount;
    const vfs_filesystem_ops_t*   ops;
    uint32_t                      size;
    uint32_t                      position;
    uint32_t                      inode;
    void*                         fs_private;
};

void vfs_init(void);
bool vfs_register_mount(filesystem_mount_t* mount,
                        const vfs_filesystem_ops_t* ops);
const filesystem_mount_t* vfs_get_mount(const char* name);
bool vfs_list_root(const char* mount_name,
                   vfs_dir_entry_callback_t callback,
                   void* context);
bool vfs_open(const char* mount_name,
              const char* path,
              vfs_file_t* file);
void vfs_close(vfs_file_t* file);
void vfs_print_mounts(void);
