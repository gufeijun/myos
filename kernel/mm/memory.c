#include "memory.h"

#include "bitmap.h"
#include "stdint.h"
#include "string.h"

// TODO implement of free

#define PG_SIZE 4096

// 内核栈顶在0xc009f000
// 内核pcb放在0xc009e000
// 我们总共预留了1MB~4MB这3MB的空间
// 1MB~2MB区域存放了256个页表
// 2MB~2MB+192KB存放bitmap
// 每128KB的bitmap可以表示4GB的内存空间，按照下列的代码
// 32位系统我们总共最多需要表示4GB的物理空间和2GB的虚拟空间，所以192
#define BITMAP_BASE (1 << 21)

//内核堆区起始虚拟地址3GB+4MB
#define K_HEAP_START 0xc0400000

#define MEM_SIZE_ADDR 0x900

#define MASK 0x03ff  // 1111111111b
#define PDE_IDX(addr) ((addr >> 22) & MASK)
#define PTE_IDX(addr) ((addr >> 12) & MASK)

//给定一个addr，得到该地址对应的pde的虚拟地址
//高10位全为1，即代表pd的最后一个表项，保存pd的物理地址
//中间10位全为1，又是pd最后一个表项，即代表了pd所在的这个内存块
//最后加上12位的偏移即可得到对应的pde虚拟地址
#define PDE_VADDR(addr) ((uint32_t*)((0xfffff000) + PDE_IDX(vaddr) * 4))

//给定一个addr，得到该地址对应的pte的虚拟地址
//这个和上同理
#define PTE_VADDR(addr)                                      \
    ((uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + \
                 PTE_IDX(vaddr) * 4))
#ifndef NULL
#define NULL ((void*)0)
#endif

static inline uint32_t get_mem_size() { return *(uint32_t*)MEM_SIZE_ADDR; }

//物理内存池
struct paddr_pool {
    uint8_t* bitmap;
    uint32_t bitmap_len;
    uint32_t phy_addr_start;
    uint32_t size;
};

static struct paddr_pool kernel_paddr_pool, user_paddr_pool;
static struct vaddr_pool kernel_vaddr_pool;  //用于给内核分配虚拟地址

//获取@pg_cnt个虚拟页, 失败返回NULL
static void* vaddr_get(enum pool_flag flag, uint32_t pg_cnt) {
    int index;
    if (flag == PF_USER) {
        // TODO
        return NULL;
    }
    index = bitmap_scan_and_set(kernel_vaddr_pool.bitmap,
                                kernel_vaddr_pool.bitmap_len, 0, pg_cnt);
    return index == -1
               ? NULL
               : (void*)(kernel_vaddr_pool.vaddr_start + index * PG_SIZE);
}

// page allocation
static void* palloc(struct paddr_pool* m_pool) {
    int index;
    index = bitmap_scan_and_set(m_pool->bitmap, m_pool->bitmap_len, 0, 1);
    return index == -1 ? NULL
                       : (void*)(m_pool->phy_addr_start + index * PG_SIZE);
}

//页表中添加虚拟地址@_vaddr到物理地址@_page_phyaddr的映射
static void page_table_add(uintptr_t vaddr, uintptr_t page_phyaddr) {
    uint32_t* pde = PDE_VADDR(vaddr);
    uint32_t* pte = PTE_VADDR(vaddr);
    // 直接访问*pte，因为pd中还不存在对应的表项，会出现page_fault
    // 判断pde的P位是否存在，不存在则创建一个pde
    if (!(*pde & 1)) {
        uintptr_t pde_phyaddr = (uintptr_t)palloc(&kernel_paddr_pool);
        *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
        memset((void*)((uintptr_t)pte & 0xfffff000), 0, PG_SIZE);
    }
    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
}

void* malloc_page(enum pool_flag flag, uint32_t pg_cnt) {
    // 1、申请虚拟地址
    // 2、申请物理地址
    // 3、在页表中添加映射
    void* vaddr_start = vaddr_get(PF_KERNEL, pg_cnt);
    uintptr_t vaddr = (uintptr_t)vaddr_start;
    if (!vaddr_start) return NULL;
    struct paddr_pool* mem_pool =
        flag == PF_KERNEL ? &kernel_paddr_pool : &user_paddr_pool;
    while (pg_cnt--) {
        void* paddr = palloc(mem_pool);
        if (!paddr) return NULL;
        page_table_add(vaddr, (uintptr_t)paddr);
        vaddr += PG_SIZE;
    }
    return vaddr_start;
}

void* kmalloc_page(uint32_t pg_cnt) {
    void* addr = malloc_page(PF_KERNEL, pg_cnt);
    if (addr) {
        memset(addr, 0, PG_SIZE * pg_cnt);
    }
    return addr;
}

static void mem_pool_init() {
    // used_mem就是我们目前可以畅快使用的物理内存起始地址:4MB向上
    uint32_t used_mem = 1 << 22;
    uint32_t free_mem = get_mem_size() - used_mem;
    uint16_t all_free_pages = free_mem / PG_SIZE;
    //物理地址空间对半分给内核以及用户
    uint16_t kernel_free_pages = all_free_pages / 2;
    uint16_t user_free_pages = all_free_pages - kernel_free_pages;

    uint32_t kbm_length = kernel_free_pages / 8;  //内核空间需要bitmap的字节数
    uint32_t ubm_length = user_free_pages / 8;  //用户空间需要bitmap的字节数

    kernel_paddr_pool.phy_addr_start = used_mem;
    user_paddr_pool.phy_addr_start = used_mem + kernel_free_pages * PG_SIZE;

    kernel_paddr_pool.size = kernel_free_pages * PG_SIZE;
    user_paddr_pool.size = user_free_pages * PG_SIZE;

    kernel_paddr_pool.bitmap_len = kbm_length;
    user_paddr_pool.bitmap_len = ubm_length;

    kernel_paddr_pool.bitmap = (uint8_t*)BITMAP_BASE;
    user_paddr_pool.bitmap = (uint8_t*)(BITMAP_BASE + kbm_length);

    bitmap_zero(kernel_paddr_pool.bitmap, kbm_length * 8);
    bitmap_zero(user_paddr_pool.bitmap, ubm_length * 8);

    //内核虚拟地址位图大小就等于物理地址位图大小
    kernel_vaddr_pool.bitmap_len = kbm_length;
    //位图放在用户物理内存池bitmap的后面
    kernel_vaddr_pool.bitmap = (void*)(BITMAP_BASE + kbm_length + ubm_length);
    kernel_vaddr_pool.vaddr_start = K_HEAP_START;
    bitmap_zero(kernel_vaddr_pool.bitmap, kernel_vaddr_pool.bitmap_len * 8);
}

void mem_init() { mem_pool_init(); }
