global x86_outb
x86_outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global x86_inb
x86_inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret


global x86_outw
x86_outw:
    [bits 32]
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

global x86_inw
x86_inw:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

global x86_outl
x86_outl:
    [bits 32]
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx,eax
    ret

global x86_inl
x86_inl:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

global x86_Panic
x86_Panic:
    cli
    hlt

global x86_EnableInterrupts
x86_EnableInterrupts:
    sti
    ret

global x86_DisableInterrupts
x86_DisableInterrupts:
    cli
    ret


;void        ASMCALL x86_InvalidatePage(uint8_t page)
global x86_InvalidatePage
x86_InvalidatePage:
    mov eax, [esp + 4]
    invlpg [eax]
    ret


;void        ASMCALL x86_ReloadPageDirectory();
global x86_ReloadPageDirectory
x86_ReloadPageDirectory:
    cli
    mov eax, cr3
    mov cr3, eax
    sti
    ret

global x86_CR2_register
x86_CR2_register:
    cli
    mov eax, cr2
    sti
    ret
