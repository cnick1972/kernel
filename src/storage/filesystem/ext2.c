/**
 * @file storage/filesystem/ext2.c
 * @brief Minimal EXT2 filesystem reader.
 */

#include <ext2.h>
#include <filesystem.h>
#include <memory.h>
#include <stdio.h>
#include <vfs.h>

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_SIZE   1024
#define EXT2_ROOT_INODE        2
#define EXT2_MAGIC             0xEF53
#define EXT2_S_IFMT            0xF000
#define EXT2_S_IFDIR           0x4000

typedef struct __attribute__((packed)) {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t reserved_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mount_count;
    uint16_t max_mount_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t  uuid[16];
    char     volume_name[16];
    char     last_mounted[64];
} ext2_superblock_t;

typedef struct __attribute__((packed)) {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint32_t reserved[3];
} ext2_group_desc_t;

typedef struct __attribute__((packed)) {
    uint16_t mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint8_t  osd2[12];
} ext2_inode_t;

typedef struct __attribute__((packed)) {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t  name_len;
    uint8_t  file_type;
    char     name[];
} ext2_dir_entry_t;

typedef struct {
    block_device_t* device;
    uint64_t        lba_start;
    uint64_t        sector_count;
    uint32_t        block_size;
    uint32_t        blocks_per_group;
    uint32_t        inodes_per_group;
    uint32_t        inode_size;
    uint32_t        first_data_block;
    uint32_t        group_desc_offset;
    uint32_t        groups_count;
} ext2_fs_t;

typedef struct {
    ext2_fs_t*    fs;
    ext2_inode_t  inode;
    uint32_t      position;
} ext2_file_handle_t;

typedef bool (*ext2_dir_consumer_t)(const filesystem_mount_t* mount,
                                    const ext2_dir_entry_t* entry,
                                    const char* name,
                                    size_t name_len,
                                    void* context);

static bool ext2_read_group_desc(ext2_fs_t* fs,
                                 uint32_t group_index,
                                 ext2_group_desc_t* desc);
static bool ext2_read_inode(ext2_fs_t* fs,
                            uint32_t inode_index,
                            ext2_inode_t* inode_out);
static bool ext2_iterate_directory(ext2_fs_t* fs,
                                   const filesystem_mount_t* mount,
                                   const ext2_inode_t* inode,
                                   ext2_dir_consumer_t consumer,
                                   void* context);
static bool ext2_lookup_child(ext2_fs_t* fs,
                              const filesystem_mount_t* mount,
                              const ext2_inode_t* dir_inode,
                              const char* name,
                              size_t name_len,
                              ext2_inode_t* out_inode);
static bool ext2_resolve_path(ext2_fs_t* fs,
                              const filesystem_mount_t* mount,
                              const char* path,
                              ext2_inode_t* inode_out);
static bool ext2_open_file(const filesystem_mount_t* mount,
                           const char* path,
                           vfs_file_t* file);
static void ext2_close_file(vfs_file_t* file);

const vfs_filesystem_ops_t g_ext2_vfs_ops = {
    .fs_name   = "ext2",
    .list_root = ext2_list_root,
    .open      = ext2_open_file,
    .close     = ext2_close_file,
};

bool ext2_mount(filesystem_mount_t* mount)
{
    if (mount == NULL || mount->kind != FILESYSTEM_KIND_EXT2
        || mount->device == NULL)
    {
        return false;
    }

    uint8_t buffer[EXT2_SUPERBLOCK_SIZE];
    uint8_t* super_ptr = NULL;
    if (!filesystem_read_bytes(mount->device,
                               mount->lba_start,
                               EXT2_SUPERBLOCK_OFFSET,
                               EXT2_SUPERBLOCK_SIZE,
                               buffer,
                               sizeof(buffer),
                               &super_ptr))
    {
        return false;
    }

    const ext2_superblock_t* super = (const ext2_superblock_t*)super_ptr;
    if (super->magic != EXT2_MAGIC)
    {
        kprintf("EXT2: invalid superblock magic on %s\n", mount->name);
        return false;
    }

    if (super->blocks_per_group == 0 || super->inodes_per_group == 0)
    {
        kprintf("EXT2: corrupt superblock on %s\n", mount->name);
        return false;
    }

    ext2_fs_t* fs = kmalloc(sizeof(*fs));
    if (fs == NULL)
    {
        return false;
    }

    fs->device = mount->device;
    fs->lba_start = mount->lba_start;
    fs->sector_count = mount->sector_count;
    fs->block_size = 1024u << super->log_block_size;
    fs->blocks_per_group = super->blocks_per_group;
    fs->inodes_per_group = super->inodes_per_group;
    fs->inode_size = super->inode_size ? super->inode_size : 128;
    fs->first_data_block = super->first_data_block;
    fs->group_desc_offset = (fs->first_data_block + 1u) * fs->block_size;
    fs->groups_count = (super->blocks_count + super->blocks_per_group - 1u)
                     / super->blocks_per_group;

    mount->driver_data = fs;

    kprintf("EXT2: %s block_size=%u blocks=%u inodes=%u\n",
            mount->name,
            fs->block_size,
            super->blocks_count,
            super->inodes_count);
    return true;
}

typedef struct {
    ext2_fs_t*      fs;
    const char*     target;
    size_t          target_len;
    ext2_inode_t*   result_inode;
    bool            found;
} ext2_lookup_context_t;

static bool ext2_lookup_consumer(const filesystem_mount_t* mount,
                                 const ext2_dir_entry_t* entry,
                                 const char* name,
                                 size_t name_len,
                                 void* context)
{
    (void)mount;
    ext2_lookup_context_t* ctx = (ext2_lookup_context_t*)context;
    if (name_len != ctx->target_len)
    {
        return true;
    }

    if (memcmp(name, ctx->target, name_len) != 0)
    {
        return true;
    }

    ctx->found = ext2_read_inode(ctx->fs, entry->inode, ctx->result_inode);
    return false;
}

static bool ext2_lookup_child(ext2_fs_t* fs,
                              const filesystem_mount_t* mount,
                              const ext2_inode_t* dir_inode,
                              const char* name,
                              size_t name_len,
                              ext2_inode_t* out_inode)
{
    if (dir_inode == NULL || out_inode == NULL || name_len == 0)
    {
        return false;
    }

    if ((dir_inode->mode & EXT2_S_IFMT) != EXT2_S_IFDIR)
    {
        return false;
    }

    ext2_lookup_context_t ctx = {
        .fs           = fs,
        .target       = name,
        .target_len   = name_len,
        .result_inode = out_inode,
        .found        = false,
    };

    ext2_iterate_directory(fs, mount, dir_inode, ext2_lookup_consumer, &ctx);
    return ctx.found;
}

static bool ext2_resolve_path(ext2_fs_t* fs,
                              const filesystem_mount_t* mount,
                              const char* path,
                              ext2_inode_t* inode_out)
{
    if (fs == NULL || path == NULL || inode_out == NULL)
    {
        return false;
    }

    ext2_inode_t current;
    if (!ext2_read_inode(fs, EXT2_ROOT_INODE, &current))
    {
        return false;
    }

    const char* cursor = path;
    while (*cursor == '/')
    {
        cursor++;
    }

    if (*cursor == '\0')
    {
        *inode_out = current;
        return true;
    }

    while (*cursor != '\0')
    {
        const char* segment = cursor;
        size_t segment_len = 0;
        while (segment[segment_len] != '\0' && segment[segment_len] != '/')
        {
            segment_len++;
        }

        ext2_inode_t next;
        if (!ext2_lookup_child(fs, mount, &current, segment, segment_len, &next))
        {
            return false;
        }

        current = next;
        cursor += segment_len;
        while (*cursor == '/')
        {
            cursor++;
        }
    }

    *inode_out = current;
    return true;
}

static bool ext2_open_file(const filesystem_mount_t* mount,
                           const char* path,
                           vfs_file_t* file)
{
    if (mount == NULL || file == NULL || path == NULL || *path == '\0')
    {
        return false;
    }

    ext2_fs_t* fs = (ext2_fs_t*)mount->driver_data;
    if (fs == NULL)
    {
        return false;
    }

    ext2_inode_t inode;
    if (!ext2_resolve_path(fs, mount, path, &inode))
    {
        return false;
    }

    ext2_file_handle_t* handle = kmalloc(sizeof(*handle));
    if (handle == NULL)
    {
        return false;
    }

    handle->fs = fs;
    handle->inode = inode;
    handle->position = 0;

    file->fs_private = handle;
    file->size = inode.size;
    file->position = 0;
    file->inode = 0;
    return true;
}

static void ext2_close_file(vfs_file_t* file)
{
    if (file == NULL || file->fs_private == NULL)
    {
        return;
    }

    void* raw = file->fs_private;
    kfree(&raw);
    file->fs_private = NULL;
}

typedef struct {
    vfs_dir_entry_callback_t callback;
    void*                    context;
} ext2_vfs_list_context_t;

static bool ext2_vfs_consumer(const filesystem_mount_t* mount,
                              const ext2_dir_entry_t* entry,
                              const char* name,
                              size_t name_len,
                              void* context)
{
    ext2_vfs_list_context_t* ctx = (ext2_vfs_list_context_t*)context;
    if (ctx->callback != NULL)
    {
        return ctx->callback(name,
                             entry->inode,
                             entry->file_type,
                             ctx->context);
    }

    (void)name_len;
    kprintf("    [%s] inode=%u type=%u name=%s\n",
            mount->name,
            entry->inode,
            entry->file_type,
            name ? name : "<unnamed>");
    return true;
}

bool ext2_list_root(const filesystem_mount_t* mount,
                    vfs_dir_entry_callback_t callback,
                    void* context)
{
    if (mount == NULL || mount->kind != FILESYSTEM_KIND_EXT2
        || mount->driver_data == NULL)
    {
        return false;
    }

    ext2_fs_t* fs = (ext2_fs_t*)mount->driver_data;
    ext2_inode_t root_inode;
    if (!ext2_read_inode(fs, EXT2_ROOT_INODE, &root_inode))
    {
        kprintf("EXT2: failed to read root inode on %s\n", mount->name);
        return false;
    }

    ext2_vfs_list_context_t ctx = {
        .callback = callback,
        .context  = context,
    };

    return ext2_iterate_directory(fs, mount, &root_inode, ext2_vfs_consumer, &ctx);
}

static bool ext2_read_group_desc(ext2_fs_t* fs,
                                 uint32_t group_index,
                                 ext2_group_desc_t* desc)
{
    if (group_index >= fs->groups_count)
    {
        return false;
    }

    uint32_t offset = fs->group_desc_offset + group_index * sizeof(*desc);
    uint8_t buffer[1024];
    uint8_t* view = NULL;
    if (!filesystem_read_bytes(fs->device,
                               fs->lba_start,
                               offset,
                               sizeof(*desc),
                               buffer,
                               sizeof(buffer),
                               &view))
    {
        return false;
    }

    memcpy(desc, view, sizeof(*desc));
    return true;
}

static bool ext2_read_inode(ext2_fs_t* fs,
                            uint32_t inode_index,
                            ext2_inode_t* inode_out)
{
    if (inode_index < 1)
    {
        return false;
    }

    uint32_t adjusted = inode_index - 1;
    uint32_t group = adjusted / fs->inodes_per_group;
    uint32_t index_in_group = adjusted % fs->inodes_per_group;

    ext2_group_desc_t desc;
    if (!ext2_read_group_desc(fs, group, &desc))
    {
        return false;
    }

    uint64_t inode_table_offset = (uint64_t)desc.inode_table * fs->block_size;
    uint64_t inode_offset = inode_table_offset + (uint64_t)index_in_group * fs->inode_size;

    uint8_t buffer[1024];
    uint8_t* view = NULL;
    if (!filesystem_read_bytes(fs->device,
                               fs->lba_start,
                               (uint32_t)inode_offset,
                               fs->inode_size,
                               buffer,
                               sizeof(buffer),
                               &view))
    {
        return false;
    }

    size_t copy_len = fs->inode_size < sizeof(*inode_out) ? fs->inode_size : sizeof(*inode_out);
    memset(inode_out, 0, sizeof(*inode_out));
    memcpy(inode_out, view, copy_len);
    return true;
}

static bool ext2_iterate_directory(ext2_fs_t* fs,
                                   const filesystem_mount_t* mount,
                                   const ext2_inode_t* inode,
                                   ext2_dir_consumer_t consumer,
                                   void* context)
{
    if (inode == NULL)
    {
        return false;
    }

    if (fs->block_size > 4096)
    {
        kprintf("EXT2: block size %u too large to enumerate\n", fs->block_size);
        return false;
    }

    uint8_t block_buffer[4096];
    uint8_t* view = NULL;

    for (int i = 0; i < 12; ++i)
    {
        uint32_t block = inode->block[i];
        if (block == 0)
        {
            continue;
        }

        uint32_t block_offset = block * fs->block_size;
        if (!filesystem_read_bytes(fs->device,
                                   fs->lba_start,
                                   block_offset,
                                   fs->block_size,
                                   block_buffer,
                                   sizeof(block_buffer),
                                   &view))
        {
            kprintf("EXT2: failed to read block %u on %s\n", block, mount->name);
            continue;
        }

        uint32_t offset = 0;
        while (offset + sizeof(ext2_dir_entry_t) <= fs->block_size)
        {
            ext2_dir_entry_t* entry = (ext2_dir_entry_t*)(view + offset);
            if (entry->rec_len == 0)
            {
                break;
            }

            if (entry->inode != 0 && entry->name_len > 0)
            {
                char name[256];
                size_t name_len = (entry->name_len < sizeof(name) - 1)
                                  ? entry->name_len
                                  : (sizeof(name) - 1);
                memcpy(name, entry->name, name_len);
                name[name_len] = '\0';

                if (consumer != NULL)
                {
                    if (!consumer(mount, entry, name, name_len, context))
                    {
                        return true;
                    }
                }
            }

            offset += entry->rec_len;
            if (entry->rec_len == 0)
            {
                break;
            }
        }
    }

    return true;
}
