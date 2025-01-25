OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .multiboot ALIGN(4K) :
    {
        *(.multiboot)
    }

    .note.gnu.build-id ALIGN(4K) :
    {
        *(.note.gnu.build-id)
    }

    .boot ALIGN(4K) :
    {
        *(.boot)
    }

    . += 0xbff00000;
    
    .text ALIGN(4K) : AT(ADDR(.text) - 0xbff00000)
    {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xbff00000)
    {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0xbff00000)
    {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xbff00000)
    {
        *(COMMON)
        *(.bss)
    }

    __end = .;
}