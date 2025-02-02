OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 0xc0100000;

    kernel_virtual_start = .;
    kernel_physical_start = . - 0xC0000000;

    .multiboot ALIGN(4K) : AT(ADDR(.multiboot) - 0xc0000000)
    {
        *(.multiboot)
    }
    
    .text ALIGN(4K) : AT(ADDR(.text) - 0xc0000000)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xc0000000)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0xc0000000)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xc0000000)
    {
        *(COMMON)
        *(.bss)
    }

    kernel_virtual_end = .;
    kernel_physical_end = . - 0xC0000000;
    __end = .;

 
    .pmm ALIGN(4K) : AT(ADDR(.pmm) - 0xc0000000)
    {
        kernel_pmm_virtual_start = .;
        kernel_pmm_physical_start = . - 0xC0000000;
        *(.pmm)
    }

    kernel_pmm_virtual_end = .;
    kernel_pmm_physical_end = . - 0xC0000000;
}