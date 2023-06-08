OUTPUT_FORMAT("binary")

STARTUP(build/kernel/src/asm/entry.asm.o)

SECTIONS
{
    .text 0x100000 : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
