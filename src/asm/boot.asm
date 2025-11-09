;-----------------------------------------------------------------------------
; @file asm/boot.asm
; @brief Bootloader entry point that sets up paging and jumps into the higher-half kernel.
; @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
; @history 2025-02-14: Converted header to assembler comment style.
;-----------------------------------------------------------------------------

[bits 32]

MB2_MAGIC        equ  0xE85250D6
MB2_ARCH_I386    equ  0
KERNEL_VIRTUAL_BASE equ 0xC0000000

section .multiboot
align 8
mb2_header_start:
    dd MB2_MAGIC
    dd MB2_ARCH_I386
    dd mb2_header_end - mb2_header_start
    dd -(MB2_MAGIC + MB2_ARCH_I386 + (mb2_header_end - mb2_header_start))

align 8
framebuffer_tag:
    dw 5                       ; MULTIBOOT_HEADER_TAG_FRAMEBUFFER
    dw 0
    dd framebuffer_tag_end - framebuffer_tag
    dd 1680
    dd 1050
    dd 32
framebuffer_tag_end:

align 8
info_req_tag:
    dw 4                       ; MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST
    dw 0
    dd info_req_tag_end - info_req_tag
    dd 6                       ; MULTIBOOT_TAG_TYPE_MMAP
info_req_tag_end:

align 8
end_tag:
    dw 0
    dw 0
    dd 8

mb2_header_end:

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text   

extern kernel_physical_start

global _start 
_start:
    ; we need a temp stack

    mov esp, (stack_top - 0xc0000000)
    push eax
    push ebx

    global PageDirectoryPhysicalAddress
    PageDirectoryPhysicalAddress equ (PageDirectoryVirtualAddress - KERNEL_VIRTUAL_BASE)

    ; move address of initial_page_dir to the cr3 register
    mov ecx, (PageDirectoryVirtualAddress - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx

    ; identity map the first page table (4 mb)
    ; update page directory page 0 to point to first 4Mb
    mov dword [(PageDirectoryVirtualAddress - KERNEL_VIRTUAL_BASE)], 0x00000083

    ;  create a page directory entry point 0xc0000000 at kernel physical base
    mov eax, kernel_physical_start
    and eax, 0xFFC00000
    or  eax, 0x00000083
    mov dword [(PageDirectoryVirtualAddress - KERNEL_VIRTUAL_BASE) + (768 * 4)], eax

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

    mov dword [PageDirectoryVirtualAddress], 0
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
global PageDirectoryVirtualAddress
PageDirectoryVirtualAddress:
    times 1024 dd 0

section .pmm
global physical_memory_bitmap
physical_memory_bitmap:
    times 0x20000 db 0x00
