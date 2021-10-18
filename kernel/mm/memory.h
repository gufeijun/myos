#ifndef __KERNEL_MM_MEMORY_H_
#define __KERNEL_MM_MEMORY_H_

#include "stdint.h"

struct vaddr_pool {
    uint8_t* bitmap;
    uint32_t bitmap_len;
    uint32_t vaddr_start;  //虚拟地址的起始地址
};

enum pool_flag {
    PF_KERNEL,
    PF_USER,
};

#define PG_P_1 1
#define PG_P_0 0
#define PG_RW_R 0
#define PG_RW_W 2
#define PG_US_S 0
#define PG_US_U 4

void mem_init();
void* kmalloc_page(uint32_t pg_cnt);

#endif
