#include <meminit.h>
#include <stdbool.h>
#include <stdio.h>

#define KERNEL_PAGE_TABLE_NUMBER 768
#define PAGE_OFFSET_BITS 12
#define PAGE_SIZE_BYTES 4096

uint32_t make_page_directory_entry(void* page_table_physical_address,
                                   enum page_size_t page_size,
                                   bool cache_disabled,
                                   bool write_through,
                                   enum page_privilege_t privilege,
                                   enum page_permissions_t permissions,
                                   bool present)
{
    uint32_t entry = (uint32_t) page_table_physical_address;
    entry |= (page_size << 7);
    entry |= (cache_disabled << 4);
    entry |= (write_through << 3);
    entry |= (privilege << 2);
    entry |= (permissions << 1);
    entry |= present;

    return entry;
}

bool get_present_from_pde(uint32_t pde)
{
    return pde & 0x1;
}

uint32_t make_page_table_entry(void* physical_address,
                            bool global,
                            bool cache_disabled,
                            bool write_through,
                            enum page_privilege_t privilege,
                            enum page_permissions_t permissions,
                            bool present)
{
    uint32_t entry = (uint32_t) physical_address; //(uint32_t) physical_address;
    entry |= global << 8;
    entry |= cache_disabled << 4;
    entry |= write_through << 3;
    entry |= privilege << 2;
    entry |= permissions << 1;
    entry |= present;

    return entry;
}

bool get_present_from_pte(uint32_t pte)
{
    return pte & 0x1;
}

void* page_table_virtual_address(uint16_t page_table_number)
{
    uint32_t virtual_address = 0xffc00000;

    virtual_address |= (page_table_number << PAGE_OFFSET_BITS);

    return (void*) virtual_address;
}

page_directory_t initialize_kernel_page_directory()
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;


    kprintf("Page Directory Virtual Address: 0x%08x\n", pd);
    kprintf("Page Directory Physical Address: 0x%08x\n", (void*) &PageDirectoryPhysicalAddress);

    uint32_t pde = make_page_directory_entry((void*) &PageDirectoryPhysicalAddress, 
                                             FOUR_KB, 
                                             false, 
                                             false, 
                                             SUPERVISOR, 
                                             READ_WRITE, 
                                             true);

    pd[1023] = pde;

    void* page_table_physical_address = allocate_physical_page();

    kprintf("Page Table Physical Address: 0x%08x\n", (void*) page_table_physical_address);

    

    uint32_t pde2 = make_page_directory_entry((void*) page_table_physical_address, 
                                                             FOUR_KB, 
                                                             false, 
                                                             false, 
                                                             SUPERVISOR, 
                                                             READ_WRITE, 
                                                             true);


   
    pd[KERNEL_PAGE_TABLE_NUMBER] = pde2;

    kprintf("Page address: 0x%08x\n", pde2);
                                                             
    page_table_t pt = (page_table_t) page_table_virtual_address(KERNEL_PAGE_TABLE_NUMBER);
    for(uint16_t i = 0; i < 1024; i++)
    {
        void* page_physical_address = (void*) (i * PAGE_SIZE_BYTES);
        pt[i] = make_page_table_entry(page_physical_address, 
                                             false, 
                                             false, 
                                             false, 
                                             SUPERVISOR, 
                                             READ_WRITE, 
                                             true);
    }

    return pd;
}

uint32_t num_present_pages(page_directory_t pd)
{
    uint32_t num = 0;
    for(int i = 0; i < 1024; i++) {
        uint32_t entry = pd[i];
        bool present = entry & 0x1;

        if(present) {
            kprintf("Page Directory Entry %d: 0x%08x\n", i, entry);
            num++;
        }
    }
    return num;
}