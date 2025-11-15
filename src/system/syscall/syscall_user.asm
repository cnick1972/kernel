[bits 32]

global usermode_syscall

; uint32_t usermode_syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
usermode_syscall:
    ; on entry:
    ;   [esp+4]  = eax (syscall number)
    ;   [esp+8]  = ebx
    ;   [esp+12] = ecx
    ;   [esp+16] = edx
    push ebp
    mov ebp, esp

    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    int 0x80

    mov esp, ebp
    pop ebp
    ret
