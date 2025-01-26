[bits 32]

MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS)   ; checksum of above, to prove we are multiboot

section .multiboot
align 4
    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM
    dd 0, 0, 0, 0, 0

    dd 0
    dd 800
    dd 600
    dd 32

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .boot   

global _start 
_start:
    ; we need a temp stack

    mov esp, (stack_top - 0xbff00000)
    push eax
    push ebx

    ; identity map the first page table (4 mb)

    mov eax, (page_table_0 - 0xbff00000)
    mov ebx, 0x0 | 3
    mov ecx, 1024
.loop:
    mov dword [eax], ebx
    add eax, 4
    add ebx, 4096
    loop .loop

    ; map the 768 table to physical address 1MB

    mov eax, (page_table_768 - 0xbff00000)
    mov ebx, 0x100000 | 3
    mov ecx, 1024
.loop2:
    mov dword [eax], ebx
    add eax, 4
    add ebx, 4096
    loop .loop2

    ; map the 769 table to the frame buffer address 0xb8000

    mov eax, (page_table_769 - 0xbff00000)
    mov ebx, 0x0 | 3
    mov ecx, 1024
.loop3:
    mov dword [eax], ebx
    add eax, 4
    add ebx, 4096
    loop .loop3

    ; set up directory table

    mov eax, (page_table_0 - 0xbff00000)
    or eax, 3
    mov dword [(initial_page_dir - 0xbff00000)], eax

    mov eax, (page_table_768 - 0xbff00000)
    or eax, 3
    mov dword [(initial_page_dir - 0xbff00000) + (768 * 4)], eax

    mov eax, (page_table_769 - 0xbff00000)
    or eax, 3
    mov dword [(initial_page_dir - 0xbff00000) + (769 * 4)], eax

    mov ecx, (initial_page_dir - 0xbff00000)
    mov cr3, ecx


    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    pop ebx
    pop eax

    jmp higher_half

section .text
global higher_half
higher_half:

    ; indentity page should no longer be required, but I will need to map the frame buffer if I remove it.

    mov dword [initial_page_dir], 0
    invlpg [0]
    add ebx, 0xc0400000

    mov esp, stack_top
    push ebx
    push eax
    xor ebp, ebp


    extern kmain
    call kmain

halt:
    hlt
    jmp halt

section .data
align 4096
global initial_page_dir
initial_page_dir:
    times 1024 dd 0

align 4096
global page_table_0
page_table_0:
    times 1024 dd 0

align 4096
global page_table_768
page_table_768:
    times 1024 dd 0

align 4096
global page_table_769
page_table_769:
    times 1024 dd 0



