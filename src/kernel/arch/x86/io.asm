
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


global crash_me
crash_me:
    ; div by 0
    mov ecx, 0x1337
    mov eax, 0
    div eax
    ;int 0x50
    ret