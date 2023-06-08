
;*******************************************************
;
;	Stage3.asm
;		A basic 32 bit binary kernel running
;
;	OS Development Series
;*******************************************************

;org	0x100000			; Kernel starts at 1 MB

bits	32				; 32 bit code

extern _kernel_entry


	jmp	Entry				; jump to entry point

%include "stdio.inc"

msg db  0x0A, 0x0A, "                       - OS Development Series -"
    db  0x0A, 0x0A, "                     MOS 32 Bit Kernel Executing", 0x0A, 0

Entry:

	;-------------------------------;
	;   Set registers		;
	;-------------------------------;

	mov	ax, 0x10		; set data segments to data selector (0x10)
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov fs, ax
	mov gs, ax
	mov	esp, 90000h		; stack begins from 90000h
	mov ebp, esp
	push ebp

	;---------------------------------------;
	;   Clear screen and print success	;
	;---------------------------------------;

	call	ClrScr32
	mov	ebx, msg
	call	Puts32

	;---------------------------------------;
	;   Stop execution			;
	;---------------------------------------;

	call _kernel_entry

	cli
	hlt


