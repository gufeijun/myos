MBR_BASE_ADDR equ 0x7c00
LOADER_BASE_ADDR equ 0x900
LOADER_SECTOR_START equ 1
LOADER_SECTOR_COUNT equ 4

SECTION MBR vstart=MBR_BASE_ADDR
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov fs,ax
	mov sp,MBR_BASE_ADDR

	mov ax,0xb800	
	mov gs,ax
	

	mov cx,2000
	mov bx,0
clean_screen:
	mov word [gs:bx],0x0700
	add bx,2
	loop clean_screen


	mov cx,strlen
	mov bx,0
	mov si,MBR_STR
print_char:
	mov al,[si]
	mov [gs:bx],al
	inc si
	add bx,2
	loop print_char
	
	xor ecx,ecx
	mov si,LOADER_SECTOR_START
	mov cx,LOADER_SECTOR_COUNT
	mov bx,LOADER_BASE_ADDR

;cx待读取的扇区数，si是扇区地址，bx是dst
read_sectors:
	;等待磁盘就绪
	mov dx,0x1F7
	in  al,dx
	and al,0xc0		;查看最高两位
	cmp al,0x40
	jnz read_sectors

	;指定读取的扇区数
	mov al,cl
	mov dx,0x1F2
	out dx,al

	;写入LBA地址
	mov ax,si
	mov dx,0x1F3
	out dx,al
	mov al,ah
	mov dx,0x1F4
	out dx,al
	mov dx,0x1F5
	mov al,0
	out dx,al
	mov dx,0x1F6
	mov al,0xe0
	out dx,al

	;发送读命令
	mov dx,0x1F7
	mov al,0x20
	out dx,al


	shl cx,8		;循环cx*256次，512/2，每个扇区512B，每次读2B
wait_disk_ready:
	nop
	mov dx,0x1F7
	in  al,dx
	and al,0x88
	cmp al,0x08
	jnz wait_disk_ready

	mov dx,0x1F0
	in  ax,dx
	mov [bx],ax
	add bx,2
	loop wait_disk_ready
	jmp LOADER_BASE_ADDR

MBR_STR:
	db "load mbr succ!"
	strlen equ $-MBR_STR

	times 510-($-$$) db 0
	dw 0xaa55
