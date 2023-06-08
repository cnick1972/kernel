OUTPUT_FORMAT("binary")

ENTRY(_ProMode)

SECTIONS
{
    .text 0x100000 : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
