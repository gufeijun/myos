KERNEL_START_SECTOR equ 5
KERNEL_BIN_BASE_ADDR equ 0x70000		;将kernel.bin加载到0x70000

 	mov eax,KERNEL_START_SECTOR
	mov ebx,KERNEL_BIN_BASE_ADDR
	mov ecx,200		;读取的扇区数

	mov esi,eax
	mov di,cx
	;设置读取的扇区数
	mov dx,0x1f2
	mov al,cl
	out dx,al
	mov eax,esi
	;写入LBA地址
	mov dx,0x1f3
	out dx,al
	mov cl,8
	shr eax,cl
	mov dx,0x1f4
	out dx,al
	shr eax,cl
	mov dx,0x1f5
	out dx,al
	shr eax,cl
	and al,0x0f
	or  al,0xe0
	mov dx,0x1f6
	out dx,al
	mov dx,0x1f7
	mov al,0x20
	out dx,al
.not_ready:
	nop
	in  al,dx
	and al,0x88
	cmp al,0x08
	jnz .not_ready
	mov ax,di
	mov dx,256
	mul dx
	mov cx,ax
	mov dx,0x1f0
.go_on_read:
	in  ax,dx
	mov [ebx],ax
	add ebx,2
	loop .go_on_read
