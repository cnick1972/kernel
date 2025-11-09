/**
 * @file system/memory/pmm.c
 * @brief Physical memory manager implementation.
 * @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
 */

#include <meminit.h>
#include <memory.h>

#define BLOCK_SIZE          PAGE_SIZE_BYTES
#define PAGE_SIZE           PAGE_SIZE_BYTES
#define PAGE_SIZE_DWORDS    1024
#define PAGES_PER_BYTE      8

extern uint32_t kernel_pmm_virtual_start;    // location of pmm bitmap
extern uint32_t kernel_pmm_physical_end;
extern uint32_t kernel_pmm_physical_start;
extern uint8_t kernel_physical_start;
extern uint8_t kernel_physical_end;

static uint32_t pmm_memory_size = 0;
static uint32_t pmm_used_blocks = 0;
static uint32_t pmm_max_blocks = 0;


/**
 * @brief The pmm_bitmap is a area of memory that control the use of physical memory.
 * The size of the bitmap is calculated by dividing the physical memory into 4096 byte pages, then each bit is assigned to that page
 * for example, if the OS has 134217728 byte of memory (128MB) that would be 32768 pages, each byte of the bitmap can control 8 pages
 * so the bitmap would need to be 4096 byte in size.
 * 
 * So Pages 0 - 7 of memory would be mem location 0x00000000 - 0x00007fff.  IF page 2 (0x00002000-0x00002fff) is available, then bit 2 of the first byte would be set 
 * to 1, if it was unavailabe, then the bit would be set to 0
 * 
 */
static uint32_t* pmm_bitmap = &kernel_pmm_virtual_start;

static uint32_t bitmap_size_bytes;
static uint32_t bitmap_dwords;
static uint32_t free_pages = 0;

/**
 * @brief Mark a page frame as free in the bitmap.
 */
void pmm_mark_page_free(uint32_t page_number) 
{
    uint32_t index = page_number >> 5;
    uint32_t bit = page_number & 0b11111;
    uint32_t value = pmm_bitmap[index];

    if((value & (1 << bit)) == 0)
    {
        free_pages++;
    }

    value |= (1 << bit);
    pmm_bitmap[index] = value; 
}

/**
 * @brief Mark a page frame as unavailable/reserved.
 */
void pmm_mark_page_reserved(uint32_t page_number)
{
    uint32_t index = page_number >> 5;
    uint32_t bit = page_number & 0b11111;
    uint32_t mask = 1u << bit;
    uint32_t value = pmm_bitmap[index];

    if(value & mask)
    {
        if(free_pages > 0)
            free_pages--;
    }

    pmm_bitmap[index] = value & ~mask;
}

static uint32_t page_number_from_address(uint32_t address)
{
    return address >> PAGE_OFFSET_BITS;
}

static uint32_t round_up_to_nearest_page_start(uint32_t address)
{
    if((address & 0xfffff000) != address)
    {
        address &= 0xfffff000;
        address += 0x00001000;
    }
    return address;
}

static uint32_t round_down_to_nearest_page_start(uint32_t address)
{
    return address &= 0xfffff000;
}


/**
 * @brief Initialize the physical memory manager bitmap.
 *
 * @param memsize Physical memory size reported by Multiboot (KB).
 * @return Number of free pages discovered.
 */
uint32_t pmm_init_allocator(uint32_t memsize)
{
    //kprintf("Mem size: 0x%08x\n", memsize * 1024);
    pmm_memory_size = memsize;
    pmm_max_blocks = (pmm_memory_size * 1024) / BLOCK_SIZE;
    pmm_used_blocks = pmm_max_blocks;

    // calculate how big the bitmap is

    bitmap_size_bytes = pmm_max_blocks / PAGES_PER_BYTE;
    bitmap_dwords = (bitmap_size_bytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);
    
    //Set all blocks to unavailable, the free block will be switch on later
    memset(pmm_bitmap, 0x00, pmm_max_blocks / PAGES_PER_BYTE);

    multiboot_mmap_entry* mmap = memory_get_mmap();
    uint8_t memory_entries = memory_get_mmap_count();

    for(int i = 0; i < memory_entries; i++)
    {
        if(mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            uint64_t region_start = mmap[i].addr;
            uint64_t region_end = region_start + mmap[i].len;

            if(region_start >= 0x100000000ULL)
            {
                continue;
            }

            if(region_end > 0x100000000ULL)
            {
                region_end = 0x100000000ULL;
            }

            uint32_t first_addr = (uint32_t)region_start;
            uint32_t one_past_last_address = (uint32_t)region_end;
            uint32_t first_full_page = page_number_from_address(round_up_to_nearest_page_start(first_addr));
            uint32_t one_past_last_full_page = page_number_from_address(round_down_to_nearest_page_start(one_past_last_address));

            for(uint32_t j = first_full_page; j < one_past_last_full_page; j++)
            {
                if(j > PAGE_SIZE_DWORDS)
                {
                    pmm_mark_page_free(j);
                }
            }
        }
    }

    uint32_t first_partial_page = page_number_from_address(round_down_to_nearest_page_start((uint32_t)&kernel_physical_start));
    uint32_t one_past_last_partial_page = page_number_from_address(round_up_to_nearest_page_start((uint32_t)&kernel_pmm_physical_end));

    for(uint32_t i = first_partial_page; i < one_past_last_partial_page; i++)
    {
        pmm_mark_page_reserved(i);
    }
 
    return free_pages;
}

/**
 * @brief Allocate a free 4 KiB page frame.
 *
 * @return Physical address of the page or 0 if none available.
 */
uintptr_t pmm_allocate_page()
{
    for(uint32_t index = 0; index < bitmap_dwords; index++)
    {
        if(pmm_bitmap[index] != 0)
        {
            // there is at least one free page in this chunk
            for(uint8_t bit = 0; bit < 32; bit++)
            {
                if((pmm_bitmap[index] & (1 << bit)) != 0)
                {
                    uint32_t page_number = index * 32 + bit;
                    pmm_mark_page_reserved(page_number);
                    uintptr_t page_start = (uintptr_t) (page_number << PAGE_OFFSET_BITS);
                    return page_start;
                }
            }
        }
    }

    return 0;
}
