#include <meminit.h>
#include <memory.h>
#include <stdio.h>

#define BLOCK_SIZE          4096
#define PAGE_SIZE           4096
#define PAGE_SIZE_DWORDS    1024
#define PAGES_PER_BYTE      8
#define PAGE_OFFSET_BITS    12

extern uint32_t kernel_pmm_virtual_start;    // location of pmm bitmap
extern uint32_t kernel_pmm_physical_end;
extern uint32_t kernel_pmm_physical_start;
extern uint8_t kernel_physical_start;
extern uint8_t kernel_physical_end;

static	uint32_t	pmm_memory_size = 0;
static	uint32_t	pmm_used_blocks=0;
static	uint32_t	pmm_max_blocks = 0;


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
static	uint32_t*	pmm_bitmap = &kernel_pmm_virtual_start;

uint32_t bitmapsize;
uint32_t free_pages = 0;

void mark_free(uint32_t page_number) 
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

void mark_unavailable(uint32_t page_number)
{
    uint32_t index = page_number >> 5;
    uint32_t bit = page_number & 0b11111; 
    uint32_t value = pmm_bitmap[index];

    if((value & (1 << bit)) == 1)
    {
        free_pages--;
    }

    value &= ~(1 << bit);
    pmm_bitmap[index] = value;
}

uint32_t page_number(uint32_t address)
{
    return address >> PAGE_OFFSET_BITS;
}

uint32_t round_up_to_nearest_page_start(uint32_t address)
{
    if((address & 0xfffff000) != address)
    {
        address &= 0xfffff000;
        address += 0x00001000;
    }
    return address;
}

uint32_t round_down_to_nearest_page_start(uint32_t address)
{
    return address &= 0xfffff000;
}


/**
 * @brief 
 * 
 * @param memsize the size of the physical memory in KB
 * @return uint32_t The size if the physical memory bitmap
 */
uint32_t init_pmm_allocator(uint32_t memsize)
{
    kprintf("Mem size: 0x%08x\n", memsize * 1024);
    pmm_memory_size = memsize;
    pmm_max_blocks = (pmm_memory_size * 1024) / BLOCK_SIZE;
    pmm_used_blocks = pmm_max_blocks;

    // calculate how big the bitmap is

    bitmapsize = pmm_max_blocks / PAGES_PER_BYTE;

    
    //Set all blocks to unavailable, the free block will be switch on later
    memset(pmm_bitmap, 0x00, pmm_max_blocks / PAGES_PER_BYTE);

    multiboot_mmap_entry* mmap = get_mmap();
    uint8_t memory_entries = get_mmap_count();

    for(int i = 0; i < memory_entries; i++)
    {
        if(mmap[i].type == 1)
        {
            uint32_t first_addr = mmap[i].addr_low;
            uint32_t one_past_last_address = first_addr + mmap[i].len_low;
            uint32_t first_full_page = page_number(round_up_to_nearest_page_start(first_addr));
            uint32_t one_past_last_full_page = page_number(round_down_to_nearest_page_start(one_past_last_address));

            for(uint32_t j = first_full_page; j < one_past_last_full_page; j++)
            {
                if(j > PAGE_SIZE_DWORDS)
                {
                    mark_free(j);
                }
            }
        }
        else
        {

        }
    }

    uint32_t first_partial_page = page_number(round_down_to_nearest_page_start((uint32_t)&kernel_physical_start));
    uint32_t one_past_last_partial_page = page_number(round_up_to_nearest_page_start((uint32_t)&kernel_pmm_physical_end));

    for(uint32_t i = first_partial_page; i < one_past_last_partial_page; i++)
    {
        mark_unavailable(i);
    }
 
    return free_pages;
}

void* allocate_physical_page()
{
    for(uint32_t index = 0; index < bitmapsize; index++)
    {
        if(pmm_bitmap[index] != 0)
        {
            // there is at least one free page in this chunk
            for(uint8_t bit = 0; bit < 32; bit++)
            {
                if((pmm_bitmap[index] & (1 << bit)) != 0)
                {
                    uint32_t page_number = index * 32 + bit;
                    mark_unavailable(page_number);
                    void* page_start = (void*) (page_number << PAGE_OFFSET_BITS);
                    return page_start;
                }
            }
        }
    }

    return 0;
}