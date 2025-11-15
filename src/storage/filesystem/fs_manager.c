/**
 * @file storage/filesystem/fs_manager.c
 * @brief Filesystem probing stubs for FAT16, EXT2, and CDFS.
 */

#include <filesystem.h>
#include <stdio.h>
#include <memory.h>
#include <ext2.h>
#include <vfs.h>

static const char FAT16_SIGNATURE[8] = { 'F', 'A', 'T', '1', '6', ' ', ' ', ' ' };
static const char ISO9660_MAGIC[5]   = { 'C', 'D', '0', '0', '1' };

static filesystem_mount_t g_mounts[FILESYSTEM_MAX_MOUNTS];
static size_t g_mount_count;

static inline uint32_t fs_sector_size(const block_device_t* device)
{
    return (device && device->sector_size) ? device->sector_size : 512;
}

static bool fs_read(block_device_t* device,
                    uint64_t lba,
                    uint32_t sector_count,
                    void* buffer)
{
    if (device == NULL || device->read == NULL)
    {
        return false;
    }

    return device->read(device, lba, sector_count, buffer);
}

bool filesystem_read_bytes(block_device_t* device,
                           uint64_t lba_start,
                           uint32_t byte_offset,
                           uint32_t byte_length,
                           uint8_t* scratch,
                           size_t scratch_size,
                           uint8_t** view_out)
{
    uint32_t sector_size = fs_sector_size(device);
    if (sector_size == 0)
    {
        sector_size = 512;
    }

    uint64_t start_sector = lba_start + (byte_offset / sector_size);
    uint32_t sector_offset = byte_offset % sector_size;
    uint32_t total_bytes = sector_offset + byte_length;
    uint32_t sectors_needed = (total_bytes + sector_size - 1) / sector_size;
    size_t bytes_needed = (size_t)sectors_needed * sector_size;

    if (bytes_needed > scratch_size)
    {
        return false;
    }

    if (!fs_read(device, start_sector, sectors_needed, scratch))
    {
        return false;
    }

    if (view_out != NULL)
    {
        *view_out = scratch + sector_offset;
    }

    return true;
}

static filesystem_kind_t filesystem_probe_fat16(block_device_t* device,
                                                uint64_t lba_start,
                                                uint64_t sector_count)
{
    (void)sector_count;

    uint8_t buffer[2048];
    uint8_t* bpb = NULL;
    if (!filesystem_read_bytes(device, lba_start, 0, 512, buffer, sizeof(buffer), &bpb))
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    if (bpb[510] != 0x55 || bpb[511] != 0xAA)
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    uint16_t bytes_per_sector = (uint16_t)bpb[11] | ((uint16_t)bpb[12] << 8);
    uint16_t fatsz16 = (uint16_t)bpb[22] | ((uint16_t)bpb[23] << 8);

    if (bytes_per_sector == 0 || fatsz16 == 0)
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    if (memcmp(&bpb[54], FAT16_SIGNATURE, sizeof(FAT16_SIGNATURE)) != 0)
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    return FILESYSTEM_KIND_FAT16;
}

static filesystem_kind_t filesystem_probe_ext2(block_device_t* device,
                                               uint64_t lba_start,
                                               uint64_t sector_count)
{
    (void)sector_count;

    uint8_t buffer[4096];
    uint8_t* super = NULL;
    if (!filesystem_read_bytes(device, lba_start, 1024, 1024, buffer, sizeof(buffer), &super))
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    uint16_t magic = (uint16_t)super[56] | ((uint16_t)super[57] << 8);
    if (magic != 0xEF53)
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    return FILESYSTEM_KIND_EXT2;
}

static filesystem_kind_t filesystem_probe_cdfs(block_device_t* device,
                                               uint64_t lba_start,
                                               uint64_t sector_count)
{
    (void)sector_count;

    uint8_t buffer[4096];
    uint8_t* pvd = NULL;
    if (!filesystem_read_bytes(device, lba_start, 16u * 2048u, 2048, buffer, sizeof(buffer), &pvd))
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    if (pvd[0] == 0x01 && memcmp(&pvd[1], ISO9660_MAGIC, sizeof(ISO9660_MAGIC)) == 0)
    {
        return FILESYSTEM_KIND_CDFS;
    }

    return FILESYSTEM_KIND_UNKNOWN;
}

const char* filesystem_kind_name(filesystem_kind_t kind)
{
    switch (kind)
    {
        case FILESYSTEM_KIND_FAT16: return "FAT16";
        case FILESYSTEM_KIND_EXT2:  return "EXT2";
        case FILESYSTEM_KIND_CDFS:  return "CDFS";
        default:                    return "UNKNOWN";
    }
}

filesystem_kind_t filesystem_probe(block_device_t* device,
                                   uint64_t lba_start,
                                   uint64_t sector_count)
{
    if (device == NULL || device->read == NULL || sector_count == 0)
    {
        return FILESYSTEM_KIND_UNKNOWN;
    }

    filesystem_kind_t kind = filesystem_probe_fat16(device, lba_start, sector_count);
    if (kind != FILESYSTEM_KIND_UNKNOWN)
    {
        return kind;
    }

    kind = filesystem_probe_ext2(device, lba_start, sector_count);
    if (kind != FILESYSTEM_KIND_UNKNOWN)
    {
        return kind;
    }

    return filesystem_probe_cdfs(device, lba_start, sector_count);
}

static void filesystem_format_mount_name(char* dest,
                                         size_t dest_len,
                                         const char* device_name,
                                         uint8_t partition_idx)
{
    if (dest == NULL || dest_len == 0)
    {
        return;
    }

    if (device_name == NULL)
    {
        device_name = "hd";
    }

    if (dest_len < 4)
    {
        dest[0] = '\0';
        return;
    }

    int written = snprintf(dest, dest_len, "%sp%u", device_name, partition_idx + 1);
    if (written < 0)
    {
        dest[0] = '\0';
    }
}

filesystem_mount_t* filesystem_get_mount(size_t index)
{
    if (index >= g_mount_count)
    {
        return NULL;
    }
    return &g_mounts[index];
}

size_t filesystem_mount_count(void)
{
    return g_mount_count;
}

static const vfs_filesystem_ops_t* filesystem_ops_for_kind(filesystem_kind_t kind)
{
    switch (kind)
    {
        case FILESYSTEM_KIND_EXT2:
            return &g_ext2_vfs_ops;
        default:
            return NULL;
    }
}

bool filesystem_mount_partition(block_device_t* device,
                                uint8_t partition_idx,
                                uint64_t lba_start,
                                uint64_t sector_count,
                                filesystem_kind_t kind)
{
    if (device == NULL || kind == FILESYSTEM_KIND_UNKNOWN || sector_count == 0)
    {
        return false;
    }

    if (g_mount_count >= FILESYSTEM_MAX_MOUNTS)
    {
        kprintf("FS: mount table full, skipping %s partition %u\n",
                device->name, partition_idx);
        return false;
    }

    filesystem_mount_t* mount = &g_mounts[g_mount_count];
    memset(mount, 0, sizeof(*mount));
    mount->device = device;
    mount->kind = kind;
    mount->lba_start = lba_start;
    mount->sector_count = sector_count;
    mount->partition_index = partition_idx;
    filesystem_format_mount_name(mount->name, sizeof(mount->name),
                                 device->name, partition_idx);

    bool mounted = false;
    switch (kind)
    {
        case FILESYSTEM_KIND_EXT2:
            mounted = ext2_mount(mount);
            break;
        case FILESYSTEM_KIND_FAT16:
        case FILESYSTEM_KIND_CDFS:
        default:
            mounted = false;
            break;
    }

    if (!mounted)
    {
        memset(mount, 0, sizeof(*mount));
        return false;
    }

    const vfs_filesystem_ops_t* ops = filesystem_ops_for_kind(kind);
    if (ops != NULL)
    {
        if (!vfs_register_mount(mount, ops))
        {
            kprintf("FS: failed to register mount %s with VFS\n", mount->name);
        }
    }

    kprintf("FS: mounted %s (partition %u) as %s [%s]\n",
            device->name,
            partition_idx,
            mount->name,
            filesystem_kind_name(kind));

    g_mount_count++;
    return true;
}
