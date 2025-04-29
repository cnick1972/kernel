#include <memory.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <meminit.h>
#include <x86.h>

#define ALLIGNMENT 8

#define PAGE_DIRECTORY_VIRTUAL_ADDRESS 0xfffff000



typedef uint32_t * page_directory_t;
typedef uint32_t * page_table_t;

page_directory_t pd = (page_directory_t) PAGE_DIRECTORY_VIRTUAL_ADDRESS;

void* ptrHeapStart = (void*)0xd0000000;
uint32_t HeapSize = 4096;

typedef struct block_t
{
    size_t size;
    struct block_t *next;
    int free;
} block_t;
 

#define BLOCK_SIZE (sizeof(block_t))

static block_t *free_list = NULL;

static size_t align_up(size_t value, size_t alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

static void split_block(block_t* block, size_t size)
{
    if(block->size  >= size + BLOCK_SIZE + ALLIGNMENT) {
        block_t* new_block = (block_t*)((uint8_t*)block + BLOCK_SIZE + size);
        new_block->size = block->size - size - BLOCK_SIZE;
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

void* kmalloc(size_t size)
{
    size = align_up(size, ALLIGNMENT);

    if(!free_list) {
        free_list = (block_t *)ptrHeapStart;
        free_list->size = HeapSize - BLOCK_SIZE;
        free_list->next = NULL;
        free_list->free = 1;
    }

    block_t* current = free_list;
    while(current) {
        if(current->free && current->size >= size)
        {
            split_block(current, size);
            current->free = 0;
            return (uint8_t*)current + BLOCK_SIZE;
        }
        current = current->next;
    }
    return NULL;
}

void _kfree(void **ptr) {
    if (!ptr && !*ptr) {
        return;
    }

    block_t *block = (block_t *)((uint8_t *)*ptr - BLOCK_SIZE);
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
    kprintf("here 0x%08x\n", *ptr);
    *ptr = NULL;
    kprintf("here 0x%08x\n", *ptr);
}

void debug_heap() {
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