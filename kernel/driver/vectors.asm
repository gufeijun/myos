[bits 32]
%define NOP nop
%define ZERO push 0

extern intr_all_handler

section .data
global intr_entry_table
intr_entry_table:
%macro VECTOR 2
section .text
intr_entry_%1:
	%2				;没有ERROR_CODE则push一个4B

	push ds
	push es
	push fs
	push gs
	pushad

	mov  al,0x20
	out  0xa0,al		;往从片发送EOI，代表中断结束
	out  0x20,al		;往主片发送EOI

	push %1
	call intr_all_handler	;调用c的总处理函数
	add  esp,4

	popad
	pop  gs
	pop	 fs
	pop  es
	pop  ds
	add  esp,4
	iretd
section .data
	dd intr_entry_%1

%endmacro

	VECTOR 0x00,ZERO
	VECTOR 0x01,ZERO
	VECTOR 0x02,ZERO
	VECTOR 0x03,ZERO
	VECTOR 0x04,ZERO
	VECTOR 0x05,ZERO
	VECTOR 0x06,ZERO
	VECTOR 0x07,ZERO
	VECTOR 0x08,ZERO
	VECTOR 0x09,ZERO
	VECTOR 0x0a,ZERO
	VECTOR 0x0b,ZERO
	VECTOR 0x0c,ZERO
	VECTOR 0x0d,ZERO
	VECTOR 0x0e,ZERO
	VECTOR 0x0f,ZERO
	VECTOR 0x10,ZERO
	VECTOR 0x11,ZERO
	VECTOR 0x12,ZERO
	VECTOR 0x13,ZERO
	VECTOR 0x14,ZERO
	VECTOR 0x15,ZERO
	VECTOR 0x16,ZERO
	VECTOR 0x17,ZERO
	VECTOR 0x18,ZERO
	VECTOR 0x19,ZERO
	VECTOR 0x1a,ZERO
	VECTOR 0x1b,ZERO
	VECTOR 0x1c,ZERO
	VECTOR 0x1d,ZERO
	VECTOR 0x1e,NOP
	VECTOR 0x1f,ZERO
	VECTOR 0x20,ZERO
