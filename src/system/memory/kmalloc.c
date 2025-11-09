#include <memory.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <serial.h>

#include <stdio.h>

#define HEAP_ALIGNMENT 8

static void* heap_virtual_start = (void*)0xd0000000;
static uint32_t heap_region_size = 0x1000;

/**
 * @brief Header prepended to each heap allocation.
 */
typedef struct block_t
{
    size_t size;           /**< Payload size in bytes. */
    struct block_t *next;  /**< Next block in the free list. */
    int free;              /**< Non-zero if block is free. */
} block_t;
 

#define BLOCK_SIZE (sizeof(block_t))

static block_t *free_list = NULL;

/**
 * @brief Round value up to the requested alignment.
 */
static size_t align_up(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

/**
 * @brief Split a free block if it is large enough to satisfy a request.
 */
static void split_block(block_t* block, size_t size)
{
    if(block->size  >= size + BLOCK_SIZE + HEAP_ALIGNMENT) {
        block_t* new_block = (block_t*)((uint8_t*)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

/**
 * @brief Allocate heap memory.
 */
void* kmalloc(size_t size)
{
    size = align_up(size, HEAP_ALIGNMENT);

    SerialPrintf("Here 1\n");

    if(!free_list) {
        SerialPrintf("Here 2\n");
        free_list = (block_t *)heap_virtual_start;
        free_list->size = heap_region_size - BLOCK_SIZE;
        free_list->next = NULL;
        free_list->free = 1;
    }

    block_t* current = free_list;

    SerialPrintf("Here 3\n");
    while(current) {
        if(current->free && current->size >= size)
        {
            
            SerialPrintf("Here 4\n");
            split_block(current, size);
            current->free = 0;
            return (uint8_t*)current + BLOCK_SIZE;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Free heap memory previously obtained from kmalloc.
 */
void kfree(void **ptr_handle)
{
    if (!ptr_handle || !*ptr_handle) {
        return;
    }

    block_t *block = (block_t *)((uint8_t *)*ptr_handle - BLOCK_SIZE);
    block->free = 1;

    block_t *current = free_list;
    while (current) {
        if (current->free && current->next && current->next->free) {
            current->size += BLOCK_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
    //kprintf("here 0x%08x\n", *ptr);
    *ptr_handle = NULL;
    //kprintf("here 0x%08x\n", *ptr);
}

/**
 * @brief Dump the current heap state for debugging.
 */
void kheap_debug_dump(void)
{
    kprintf("Heap layout:\n");

    block_t *current = free_list;
    int index = 0;
    while (current) {
        kprintf(" Block %d: addr=0x%08x size=%d free=%d\n",
            index,
            (void *)current,
            current->size,
            current->free);

        current = current->next;
        index++;
    }
}
