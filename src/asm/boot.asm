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

section .text   

global _start 
_start:
    ; we need a temp stack

    mov esp, (stack_top - 0xc0000000)
    push eax
    push ebx

    ; identity map the first page table (4 mb)
    ; update page directory page 0 to point to first 4Mb

    mov dword [(initial_page_dir - 0xc0000000)], 0x00000083

    ;  create a page directory entry point 0xc0000000 at 0x0
    mov dword [(initial_page_dir - 0xc0000000) + (768 * 4)], 0x00000083

    ; move address of initial_page_dir to the cr3 register
    mov ecx, (initial_page_dir - 0xc0000000)
    mov cr3, ecx

    ; enable PSE so we can use 4MB pages
    mov ecx, cr4
    or ecx, 0x00000010
    mov cr4, ecx

    ; enable paging
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    pop ebx
    pop eax

    ; jump to highr half
    lea ecx, higher_half
    jmp ecx


section .text
global higher_half
higher_half:
    ; indentity page should no longer be required, but I will need to map the frame buffer if I remove it.

    mov dword [initial_page_dir], 0
    invlpg [0]

    ; correctly re-enable the stack to use virtual addressing
    mov esp, stack_top

    add ebx, 0xc0000000
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




