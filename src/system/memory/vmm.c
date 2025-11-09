#include <meminit.h>
#include <paging.h>
#include <stdbool.h>
#include <memory.h>
#include <isr.h>
#include <x86.h>


#define KERNEL_PAGE_TABLE_NUMBER 768 /**< Kernel virtual address PDE index. */
#define PAGE_OFFSET_BITS 12
#define PAGE_SIZE_BYTES 4096

/**
 * @brief Construct a page directory entry.
 */
uint32_t vmm_make_page_directory_entry(void* page_table_physical_address,
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

/**
 * @brief Construct a page table entry.
 */
uint32_t vmm_make_page_table_entry(void* physical_address,
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

/**
 * @brief Compute the recursive-mapping virtual address of a page table.
 */
void* vmm_page_table_virtual_address(uint16_t page_table_number)
{
    uint32_t virtual_address = 0xffc00000;

    virtual_address |= (page_table_number << PAGE_OFFSET_BITS);

    return (void*) virtual_address;
}

/**
 * @brief Create the initial kernel page directory and identity mappings.
 */
page_directory_t vmm_initialize_kernel_page_directory()
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;
    uint32_t pde = vmm_make_page_directory_entry((void*) &PageDirectoryPhysicalAddress, 
                                             FOUR_KB, 
                                             false, 
                                             false, 
                                             SUPERVISOR, 
                                             READ_WRITE, 
                                             true);

    pd[1023] = pde;

    void* page_table_physical_address = (void*)pmm_allocate_page();
    pd[KERNEL_PAGE_TABLE_NUMBER] = vmm_make_page_directory_entry((void*) page_table_physical_address, 
                                                             FOUR_KB, 
                                                             false, 
                                                             false, 
                                                             SUPERVISOR, 
                                                             READ_WRITE, 
                                                             true);
                                                             
    page_table_t pt = (page_table_t) vmm_page_table_virtual_address(KERNEL_PAGE_TABLE_NUMBER);
    for(uint16_t i = 0; i < 1024; i++)
    {
        void* page_physical_address = (void*) (i * PAGE_SIZE_BYTES);
        pt[i] = vmm_make_page_table_entry(page_physical_address, 
                                             false, 
                                             false, 
                                             false, 
                                             SUPERVISOR, 
                                             READ_WRITE, 
                                             true);
    }

    return pd;
}

/**
 * @brief Count the number of present page-directory entries.
 */
uint32_t vmm_count_present_pages(page_directory_t pd)
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

/**
 * @brief Demand-map page fault handler.
 */
void vmm_page_fault_handler(Registers* regs)
{
    

    uint32_t cr2 = x86_read_cr2();

   

    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;

    // do i have a page directory entry for the memory requested?
    uint32_t directory_entry = cr2 >> 22;
    page_table_t pt;

    if(!get_present_from_pde(pd[directory_entry]))
    {
        
        // allocate a memory block for the required pde
        uintptr_t new_page = pmm_allocate_page();
        uint32_t pde = vmm_make_page_directory_entry((void*) new_page, 
                        FOUR_KB, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);

        pd[directory_entry] = pde;

        pt = (page_table_t) vmm_page_table_virtual_address(directory_entry);
        memset(pt, 0, PAGE_SIZE_BYTES);
    }
    else
    {
        pt = (page_table_t) vmm_page_table_virtual_address(directory_entry);
    }

    uint32_t table_entry = (cr2 >> 12) & 0x3ff;

    if(!get_present_from_pte(pt[table_entry]))
    {

        uintptr_t new_page = pmm_allocate_page();
    
        pt[table_entry] = vmm_make_page_table_entry((void*)new_page, 
                        false, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);
    }
    x86_reload_page_directory();
    //kprintf("In handler 0x%08x\n", cr2);
}

/**
 * @brief Establish a 4 KiB mapping between a physical and virtual address.
 */
bool vmm_map_physical_to_virtual(uint8_t* physical_address, uint8_t* virtual_address)
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;
    
    // This procedure will map a physical address to a virtual address
    // Step 1 - Check is the requested virtual address is available
    // Step 1.1 see if there is a page directory for the request virtual address

    uint32_t directory_entry = (uint32_t)virtual_address >> 22;
    page_table_t pt;
    bool mapped_new_entry = false;

    if(!get_present_from_pde(pd[directory_entry]))
    {
        uintptr_t new_page = pmm_allocate_page();
        if(new_page == 0)
        {
            return false;
        }
        uint32_t pde = vmm_make_page_directory_entry((void*) new_page, 
                                FOUR_KB, 
                                false, 
                                false, 
                                SUPERVISOR, 
                                READ_WRITE, 
                                true);

        pd[directory_entry] = pde;
        pt = (page_table_t) vmm_page_table_virtual_address(directory_entry);
        memset(pt, 0, PAGE_SIZE_BYTES);
        mapped_new_entry = true;
    }
    else
    {
        pt = (page_table_t) vmm_page_table_virtual_address(directory_entry);
    }

    uint32_t table_entry = ((uint32_t)virtual_address >> 12) & 0x3ff;
    
    if(!get_present_from_pte(pt[table_entry]))
    {
        pt[table_entry] = vmm_make_page_table_entry(physical_address, 
                        false, 
                        false, 
                        false, 
                        SUPERVISOR, 
                        READ_WRITE, 
                        true);
        mapped_new_entry = true;
    }
    //x86_reload_page_directory();


    return mapped_new_entry;

}


/**
 * @brief Map a contiguous region using 4 MiB pages.
 */
bool vmm_map_4mb_physical_to_virtual(uint8_t* physical_address, uint8_t* virtual_address, size_t size)
{
    page_directory_t pd = (page_directory_t) &PageDirectoryVirtualAddress;

    uint32_t directory_entry = (uint32_t)virtual_address / 1024 / 4096;

    int entries_required = size / 4;
    if(size % 4)
        entries_required++;

    for(int i = 0; i < entries_required; i++)
    {
        uint32_t entry = (uint32_t)physical_address + (0x400000 * i);
        entry &= 0xffc00000;
        entry |= 0x83;
    
        pd[directory_entry + i] = entry;

    }    

    x86_reload_page_directory();

    return true;
}
