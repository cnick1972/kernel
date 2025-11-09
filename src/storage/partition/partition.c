/**
 * @file storage/partition/partition.c
 * @brief Partition discovery scaffolding.
 */

#include <partition.h>
#include <stdio.h>
#include <memory.h>

#define PARTITION_TABLE_ENTRIES 4

typedef struct __attribute__((packed)) {
    uint8_t  status;
    uint8_t  chs_first[3];
    uint8_t  type;
    uint8_t  chs_last[3];
    uint32_t lba_first;
    uint32_t sector_count;
} mbr_partition_entry_t;

static bool partition_read_sector(block_device_t* device,
                                  uint64_t lba,
                                  uint32_t count,
                                  void* buffer)
{
    if (device == NULL || device->read == NULL)
    {
        return false;
    }

    return device->read(device, lba, count, buffer);
}

void partition_scan_device(block_device_t* device)
{
    if (device == NULL)
    {
        return;
    }

    uint8_t sector[512];
    if (!partition_read_sector(device, 0, 1, sector))
    {
        kprintf("PART: %s -> unable to read MBR\n", device->name);
        return;
    }

    if (sector[510] != 0x55 || sector[511] != 0xAA)
    {
        kprintf("PART: %s -> invalid MBR signature\n", device->name);
        return;
    }

    partition_info_t found[PARTITION_TABLE_ENTRIES];
    size_t found_count = 0;

    for (uint8_t i = 0; i < PARTITION_TABLE_ENTRIES; ++i)
    {
        const mbr_partition_entry_t* entry =
            (const mbr_partition_entry_t*)(sector + 446 + (i * sizeof(mbr_partition_entry_t)));

        if (entry->type == 0 || entry->sector_count == 0)
        {
            continue;
        }

        partition_info_t part = {
            .index        = i,
            .lba_start    = entry->lba_first,
            .sector_count = entry->sector_count,
            .fs_kind      = filesystem_probe(device,
                                             entry->lba_first,
                                             entry->sector_count),
        };

        if (found_count < PARTITION_TABLE_ENTRIES)
        {
            found[found_count++] = part;
        }
    }

    if (found_count == 0)
    {
        kprintf("PART: %s -> no partitions detected\n", device->name);
        return;
    }

    kprintf("PART: %s partition list:\n", device->name);
    for (size_t idx = 0; idx < found_count; ++idx)
    {
        const partition_info_t* part = &found[idx];
        kprintf("  slot %u lba=%llu size=%llu fs=%s\n",
                part->index,
                (unsigned long long)part->lba_start,
                (unsigned long long)part->sector_count,
                filesystem_kind_name(part->fs_kind));
    }
}
