    global _geninterrupt

_geninterrupt:
    mov al, byte [esp]
    mov byte [genint+1], al
    jmp genint
genint:
    int 0
    ret
