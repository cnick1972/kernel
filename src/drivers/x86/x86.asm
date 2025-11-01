[bits 32]
[section .text]

;void        ASMCALL x86_outb(uint16_t port, uint8_t value)
global x86_outb
x86_outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

;uint8_t     ASMCALL x86_inb(uint16_t port)
global x86_inb
x86_inb:
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret

;void        ASMCALL x86_outw(uint16_t port, uint16_t value
global x86_outw
x86_outw:
    mov dx, [esp + 4]
    mov ax, [esp + 8]
    out dx, ax
    ret

;uint16_t    ASMCALL x86_inw(uint16_t port)
global x86_inw
x86_inw:
    mov dx, [esp + 4]
    xor eax, eax
    in ax, dx
    ret

;void        ASMCALL x86_outl(uint16_t port, uint32_t value)
global x86_outl
x86_outl:
    mov dx, [esp + 4]
    mov eax, [esp + 8]
    out dx,eax
    ret

;uint32_t    ASMCALL x86_inl(uint16_t port)

global x86_inl
x86_inl:
    mov dx, [esp + 4]
    xor eax, eax
    in eax, dx
    ret

;void        ASMCALL x86_Panic()
global x86_Panic
x86_Panic:
    cli
    hlt


;uint8_t     ASMCALL x86_EnableInterrupts()
global x86_EnableInterrupts
x86_EnableInterrupts:
    sti
    ret

;uint8_t     ASMCALL x86_DisableInterrupts()
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


;void        ASMCALL x86_ReloadPageDirectory()
global x86_ReloadPageDirectory
x86_ReloadPageDirectory:
    cli             ; disable interrupts, we don't want to be interrupted here
    mov eax, cr3
    mov cr3, eax
    sti             ; re-enable interrupts   
    ret


;uint32_t    ASMCALL x86_CR2_register()
global x86_CR2_register
x86_CR2_register:
    cli                 ; disable interrupts
    mov eax, cr2        ; read CR2 into EAX
    sti                 ; re-enable interrupts
    ret
