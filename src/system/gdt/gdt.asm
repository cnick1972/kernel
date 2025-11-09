;-----------------------------------------------------------------------------
; @file system/gdt/gdt.asm
; @brief Assembly helper for loading the GDT.
; @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
; @history 2025-02-14: Converted header to assembler comment style.
;-----------------------------------------------------------------------------

[bits 32]

; void __attribute__((cdecl)) gdt_load(GDTDescriptor* descriptor, uint16_t codeSegment, uint16_t dataSegment);
global gdt_load
gdt_load:
    
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame
    
    ; load gdt
    mov eax, [ebp + 8]
    lgdt [eax]

    ; reload code segment
    mov eax, [ebp + 12]
    push eax
    push .reload_cs
    retf

.reload_cs:

    ; reload data segments
    mov ax, [ebp + 16]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax,
    mov ss, ax

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret
