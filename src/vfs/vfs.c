/**
 * @file vfs/vfs.c
 * @brief Minimal virtual filesystem routing layer.
 */

#include <vfs.h>
#include <stdio.h>
#include <memory.h>

typedef struct {
    filesystem_mount_t*            mount;
    const vfs_filesystem_ops_t*    ops;
} vfs_mount_entry_t;

static vfs_mount_entry_t g_vfs_mounts[FILESYSTEM_MAX_MOUNTS];
static size_t g_vfs_mount_count;

void vfs_init(void)
{
    memset(g_vfs_mounts, 0, sizeof(g_vfs_mounts));
    g_vfs_mount_count = 0;
}

static bool vfs_name_equal(const char* a, const char* b)
{
    if (a == NULL || b == NULL)
    {
        return false;
    }

    while (*a != '\0' && *b != '\0')
    {
        if (*a != *b)
        {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

bool vfs_register_mount(filesystem_mount_t* mount,
                        const vfs_filesystem_ops_t* ops)
{
    if (mount == NULL || ops == NULL)
    {
        return false;
    }

    if (g_vfs_mount_count >= FILESYSTEM_MAX_MOUNTS)
    {
        kprintf("VFS: mount table full, cannot add %s\n", mount->name);
        return false;
    }

    for (size_t i = 0; i < g_vfs_mount_count; ++i)
    {
        if (vfs_name_equal(g_vfs_mounts[i].mount->name, mount->name))
        {
            kprintf("VFS: mount name %s already registered\n", mount->name);
            return false;
        }
    }

    g_vfs_mounts[g_vfs_mount_count].mount = mount;
    g_vfs_mounts[g_vfs_mount_count].ops = ops;
    g_vfs_mount_count++;
    return true;
}

const filesystem_mount_t* vfs_get_mount(const char* name)
{
    for (size_t i = 0; i < g_vfs_mount_count; ++i)
    {
        if (vfs_name_equal(g_vfs_mounts[i].mount->name, name))
        {
            return g_vfs_mounts[i].mount;
        }
    }
    return NULL;
}

static const vfs_mount_entry_t* vfs_find_entry(const char* name)
{
    for (size_t i = 0; i < g_vfs_mount_count; ++i)
    {
        if (vfs_name_equal(g_vfs_mounts[i].mount->name, name))
        {
            return &g_vfs_mounts[i];
        }
    }
    return NULL;
}

bool vfs_list_root(const char* mount_name,
                   vfs_dir_entry_callback_t callback,
                   void* context)
{
    const vfs_mount_entry_t* entry = vfs_find_entry(mount_name);
    if (entry == NULL || entry->ops == NULL || entry->ops->list_root == NULL)
    {
        return false;
    }

    return entry->ops->list_root(entry->mount, callback, context);
}

bool vfs_open(const char* mount_name,
              const char* path,
              vfs_file_t* file)
{
    if (file == NULL)
    {
        return false;
    }

    memset(file, 0, sizeof(*file));

    const vfs_mount_entry_t* entry = vfs_find_entry(mount_name);
    if (entry == NULL || entry->ops == NULL || entry->ops->open == NULL)
    {
        return false;
    }

    if (!entry->ops->open(entry->mount, path, file))
    {
        memset(file, 0, sizeof(*file));
        return false;
    }

    file->mount = entry->mount;
    file->ops = entry->ops;
    return true;
}

void vfs_close(vfs_file_t* file)
{
    if (file == NULL)
    {
        return;
    }

    if (file->ops != NULL && file->ops->close != NULL)
    {
        file->ops->close(file);
    }

    memset(file, 0, sizeof(*file));
}

static bool vfs_log_dir_entry(const char* name,
                              uint32_t inode,
                              uint8_t type,
                              void* context)
{
    const char* mount_name = (const char*)context;
    kprintf("    [%s] inode=%u type=%u name=%s\n",
            mount_name ? mount_name : "<mount>",
            inode,
            type,
            name ? name : "<unnamed>");
    return true;
}

void vfs_print_mounts(void)
{
    if (g_vfs_mount_count == 0)
    {
        kprintf("VFS: no mounted filesystems\n");
        return;
    }

    kprintf("VFS: %u mounted filesystem(s)\n", (unsigned)g_vfs_mount_count);

    for (size_t i = 0; i < g_vfs_mount_count; ++i)
    {
        filesystem_mount_t* mount = g_vfs_mounts[i].mount;
        kprintf("VFS: %s -> %s (partition %u) lba=%llu size=%llu fs=%s\n",
                mount->name,
                mount->device ? mount->device->name : "<dev>",
                mount->partition_index,
                (unsigned long long)mount->lba_start,
                (unsigned long long)mount->sector_count,
                filesystem_kind_name(mount->kind));

        if (g_vfs_mounts[i].ops != NULL && g_vfs_mounts[i].ops->list_root != NULL)
        {
            vfs_list_root(mount->name, vfs_log_dir_entry, (void*)mount->name);
        }
    }
}
