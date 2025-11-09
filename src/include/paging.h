#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE_BYTES 4096
#define PAGE_OFFSET_BITS 12
#define KERNEL_PAGE_TABLE_NUMBER 768

typedef uint32_t* page_directory_t;
typedef uint32_t* page_table_t;

enum page_permissions_t {READ_ONLY, READ_WRITE};
enum page_privilege_t {SUPERVISOR, USER};
enum page_size_t {FOUR_KB, FOUR_MB};

extern void * PageDirectoryVirtualAddress;
extern void * PageDirectoryPhysicalAddress;
