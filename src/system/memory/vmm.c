#include <meminit.h>
#include <stdbool.h>
#include <memory.h>
#include <stdio.h>
#include <isr.h>
#include <x86.h>


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
    uint32_t entry = ((uint32_t) physical_address & 0xFFFFF000); //(uint32_t) physical_address;
    entry |= (global ? (1u << 8) : 0);
    entry |= (cache_disabled ? ( 1u << 4) : 0);
    entry |= (write_through ? (1u << 3) : 0);
    entry |= (privilege << 2);
    entry |= (permissions << 1);
    entry |= (present ? 1u : 0);

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
    uint32_t pde = make_page_directory_entry((void*) &PageDirectoryPhysicalAddress, 
                                             FOUR_KB, 
                                             false, 
                                             false, 
                                             SUPERVISOR, 
                                             READ_WRITE, 
                                             true);

    pd[1023] = pde;

    void* page_table_physical_address = (void*)allocate_physical_page();
    pd[KERNEL_PAGE_TABLE_NUMBER] = make_page_directory_entry((void*) page_table_physical_address, 
                                                             FOUR_KB, 
                                                             false, 
                                                             false, 
                                                             SUPERVISOR, 
                                                             READ_WRITE, 
                                                             true);
                                                             
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
            //kprintf("Page Directory Entry %d: 0x%08x\n", i, entry);
            num++;
        }
    }
    return num;
}

void PageFaulthandler(Registers* regs)
{
    

    uint32_t cr2 = x86_CR2_register();

   

    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;

    // do i have a page directory entry for the memory requested?
    uint32_t directoryEntry = cr2 >> 22;
    page_table_t pt;

    if(!get_present_from_pde(pd[directoryEntry]))
    {
        
        // allocate a memory block for the required pde
        uintptr_t newPage = allocate_physical_page();
        uint32_t pde = make_page_directory_entry((void*) newPage, 
                        FOUR_KB, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);

        pd[directoryEntry] = pde;

        pt = (page_table_t) page_table_virtual_address(directoryEntry);
        memset(pt, 0, PAGE_SIZE_BYTES);
    }
    else
    {
        pt = (page_table_t) page_table_virtual_address(directoryEntry);
    }

    uint32_t tbentry = (cr2 >> 12) & 0x3ff;

    if(!get_present_from_pte(pt[tbentry]))
    {

        uintptr_t newPage = allocate_physical_page();
    
        pt[tbentry] = make_page_table_entry((void*)newPage, 
                        false, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);
    }
    x86_ReloadPageDirectory();
    //kprintf("In handler 0x%08x\n", cr2);
}

bool MapPhysicalToVirtual(uint8_t* pAddress, uint8_t* vAddress)
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;
    
    // This procedure will map a physical address to a virtual address
    // Step 1 - Check is the requested virtual address is available
    // Step 1.1 see if there is a page directory for the request virtual address

    uint32_t directoryEntry = (uint32_t)vAddress >> 22;
    page_table_t pt;

    if(!get_present_from_pde(pd[directoryEntry]))
    {
        uintptr_t newPage = allocate_physical_page();
        uint32_t pde = make_page_directory_entry((void*) newPage, 
                                FOUR_KB, 
                                false, 
                                false, 
                                SUPERVISOR, 
                                READ_WRITE, 
                                true);

        pd[directoryEntry] = pde;
        pt = (page_table_t) page_table_virtual_address(directoryEntry);
        memset(pt, 0, PAGE_SIZE_BYTES);
    }
    else
    {
        pt = (page_table_t) page_table_virtual_address(directoryEntry);
    }

    uint32_t tbentry = ((uint32_t)vAddress >> 12) & 0x3ff;
    
    if(!get_present_from_pte(pt[tbentry]))
    {
        pt[tbentry] = make_page_table_entry(pAddress, 
                        false, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);
    }
    //x86_ReloadPageDirectory();


    return false;

}


bool Map4MBPhysicalToVirtual(uint8_t* pAddress, uint8_t* vAddress, size_t size)
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;

    uint32_t directoryEntry = (uint32_t)vAddress / 1024 / 4096;

    int entries = size / 4;
    if(size % 4)
        entries++;

    for(int i = 0; i < entries; i++)
    {
        uint32_t entry = (uint32_t)pAddress + (0x400000 * i) & 0xffc00000;
        entry = entry + 0x83;
    
        pd[directoryEntry + i] = entry;

    }    

    x86_ReloadPageDirectory();

    return false;
}
