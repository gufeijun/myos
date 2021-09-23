;功能：设置页目录表、页表以及开启分页
;我们将1MB物理内存起始处依次开始存放页目录表和页表

PAGE_DIR_TABLE_ADDR equ 0x100000	;页目录表存放在1MB处
PAGE_TABLE_START_ADDR equ 0x101000	;第一个页表存放处
;页目录表项的user位
PG_US_U equ 4
PG_US_S equ 0
;页目录表项的RW位，为1则可读可写
PG_RW_RW equ 2
PG_RW_R equ 0
PG_P equ 1

setup_page:
	mov ecx,1024
	mov eax,PAGE_DIR_TABLE_ADDR
.clean_dir_page:		;将页目录表清0
	mov dword [eax],0
	add eax,4
	loop .clean_dir_page

;------------------------------------------------------	
; 第一步：初始化页目录表。仅初始化3个表项。每一个页目录表项代表4MB内存
; 1、将虚拟0~4MB -> 物理0~4B。页目录表的第0个表项
; 2、将虚拟3G~3G+4MB -> 物理0-4MB。页目录表的第768个表项
; 3、将最后一个表项指向自己这个内存块，方便以后修改自己
; 我们的内核将放在0~1MB处，虚拟地址3G以上是内核空间
; 1和2都指向同一个页表
;------------------------------------------------------	

;分别初始化上述的三个页目录表项
.create_page_dir_table_entry:
	;页目录表高20位为物理页地址，低12位为属性
	mov eax,PAGE_TABLE_START_ADDR	
	or eax,PG_US_U | PG_RW_RW | PG_P
	mov [PAGE_DIR_TABLE_ADDR],eax			;1
	mov [PAGE_DIR_TABLE_ADDR+768*4],eax		;2
	sub eax,0x1000
	mov [PAGE_DIR_TABLE_ADDR+1023*4],eax	;3

;------------------------------------------------------	
; 第二步：初始化第0个页表，保证映射到0~4MB这1024个块中
;------------------------------------------------------	
.create_page_table_entry:
	mov eax,PAGE_TABLE_START_ADDR	
	mov ecx,1024
	mov esi,0
.loop_init_first_page_table:
	or esi,PG_US_U | PG_RW_RW | PG_P
	mov [eax],esi
	add eax,4
	add esi,0x1000
	loop .loop_init_first_page_table

;------------------------------------------------------	
; 第三步：将3G+4MB~4GB的虚拟地址空间映射到固定位置，方便未来
; 进程间共享。共1020MB，即需255个页目录表项以及页表，对应于
; 页目录表的769~1023这255个表项，但1023指向了本身，所以我们只能
; 使用到254个表项，内核空间实际是1GB-4MB。
;------------------------------------------------------	
.create_kernel_space_entry:
	mov eax,PAGE_TABLE_START_ADDR + 0x1000	
	or eax,PG_US_U | PG_RW_RW | PG_P
	mov ebx,769 * 4 + PAGE_DIR_TABLE_ADDR
	mov ecx,254	
.loop_init_kernel_space:
	mov [ebx],eax
	add eax,0x1000
	add ebx,4
	loop .loop_init_kernel_space
