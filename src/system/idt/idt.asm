;-----------------------------------------------------------------------------
; @file system/idt/idt.asm
; @brief Assembly helper for loading the IDT.
; @copyright Copyright 2025 Chris Nicholson <chris@cnick.org.uk>
; @history 2025-02-14: Converted header to assembler comment style.
;-----------------------------------------------------------------------------

[bits 32]

;void __attribute__((cdecl)) idt_load(IDTDescriptor* idtDescriptor)
global idt_load
idt_load:

    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame

    mov eax, [ebp + 8]
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret
