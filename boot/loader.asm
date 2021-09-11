;%1(type)，%2(base)，%3(limit)
;详见段描述符格式
%macro BUILD_GDT_DESC 3
	dw	%3 & 0xffff , %2 & 0xffff
	db  (%2 >> 16)  & 0xff, (%1 | 0x90)
	db  ((%3 >> 16) & 0x0f) | 0xc0
	db  (%2 >> 24) & 0xff
%endmacro

DESC_CODE_TYPE equ 0x08
DESC_DATA_TYPE equ 0x02
DESC_VIDEO_TYPE equ 0x02
LIMIT_OF_FLAT equ 0xfffff
LIMIT_OF_VIDEO equ 0x7
LOADER_BASE_ADDR equ 0x900
VIDEO_BASE_ADDR equ 0xb8000
KERNEL_SPACE_ADDR equ 0xc0000000

[bits 16]
;SECTION LOADER vstart=LOADER_BASE_ADDR
section .text
global _start
_start:
	;position the cursor on the fifth line
	mov ah,2
	mov bh,0
	mov dh,4
	mov dl,0
	int 0x10

	;将0x900以下作为栈空间，方便后续的c内核代码使用
	mov sp,LOADER_BASE_ADDR
	mov bp,LOADER_BASE_ADDR

	mov cx,strlen1
	mov bx,160
	mov si,LOADER_STR

print_str_real:
	mov al,[si]
	mov [gs:bx],al
	inc si
	add bx,2 
	loop print_str_real

	;通过键盘控制器打开a20
wait_not_busy1:
	mov dx,0x64
	in  al,dx
	test al,0x2
	jnz wait_not_busy1
	mov al,0xd1
	out dx,al

wait_not_busy2:
	in  al,dx
	test al,0x2
	jnz wait_not_busy2
	mov al,0xdf
	mov dx,0x60
	out dx,al

	
	;加载gdt
	lgdt [GDTR]

	;将CR0的第0位PE置为1
	mov eax,cr0
	or  eax,1
	mov cr0,eax

	;刷新流水线，cs。8=1<<3
	jmp dword 8:p_mode

[bits 32]
p_mode:
	;更改段选择子的值(选中数据段)
	mov ax,2<<3
	mov ds,ax
	mov es,ax
	mov ss,ax

	mov ax,3<<3
	mov gs,ax
	
	mov si,P_MODE_STR
	mov bx,320			;第三行
	mov cx,strlen2	

	call print_str_p

	;设置页表与页目录表
	%include "page.asm"		

;-----------------------------------------------------
;将显存的基址、GDT_BASE、栈指针放入内核空间，
;虽然这些都在1MB内存以下，不放入3GB以上目前也可以访问
;-----------------------------------------------------
	;修改显示段的段描述符
	sgdt [GDTR]
	mov ebx,[GDTR+2]
	add dword [ebx+24+4],KERNEL_SPACE_ADDR
	;修改GDT_BASE
	add dword [GDTR+2],KERNEL_SPACE_ADDR
	;栈指针放入内核空间
	add esp,KERNEL_SPACE_ADDR
	
;-----------------------------------------------------
; 准备开启分页
;-----------------------------------------------------
	;将页目录地址赋给cr3
	mov eax,PAGE_DIR_TABLE_ADDR
	mov cr3,eax
	;打开cr0的pg位
	mov eax,cr0
	or eax,0x80000000
	mov cr0,eax
	;重新加载gdt
	lgdt [GDTR]

	mov si,PAGING_STR
	mov bx,480
	mov cx,strlen3
	call print_str_p

	mov esp,0xc009f000
	mov ebp,0xc009f000
	;jump to c function boot. see boot/boot.c
	extern boot
	call boot

;si:字符串起始地址
;bx:字符输出位置
;cx:循环次数
print_str_p:
	mov al,[si]
	mov [gs:bx],al
	inc si
	add bx,2
	loop print_str_p
	ret

;代码段和数据段都采用平坦模式，为了方便输出字符，
;单独设置了个显示段，采用非平坦
align 4
GDT:
NULL_SEG_DESC:
	dd 0,0			;第一个取空
CODE_SEG_DESC:
	BUILD_GDT_DESC 	DESC_CODE_TYPE,0,LIMIT_OF_FLAT
DATA_SEG_DESC:
	BUILD_GDT_DESC DESC_DATA_TYPE,0,LIMIT_OF_FLAT
VIDEO_SEG_DESC:
	BUILD_GDT_DESC DESC_VIDEO_TYPE,VIDEO_BASE_ADDR,LIMIT_OF_VIDEO

GDTR:
	dw $-GDT-1	
	dd GDT

LOADER_STR:
	db "load loader succ!"
	strlen1 equ $ - LOADER_STR
P_MODE_STR:
	db "enter protect mode succ!"
	strlen2 equ $ - P_MODE_STR
PAGING_STR:
	db "memery paging succ!"
	strlen3 equ $ - PAGING_STR
