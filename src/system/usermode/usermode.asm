[bits 32]

%define USER_CS 0x1B
%define USER_DS 0x23

global usermode_trampoline

; void usermode_trampoline(uint32_t entry, uint32_t stack_top);
usermode_trampoline:
    mov eax, [esp + 4]    ; entry point
    mov edx, [esp + 8]    ; user stack pointer

    mov bx, USER_DS
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    push dword USER_DS
    push edx
    push dword 0x202      ; EFLAGS with IF enabled
    push dword USER_CS
    push eax
    iretd
