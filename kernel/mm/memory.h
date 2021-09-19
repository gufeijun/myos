#ifndef __KERNEL_MM_MEMORY_H_
#define __KERNEL_MM_MEMORY_H_

#include "stdint.h"

struct vaddr_mgr {
    uint8_t* vaddr_bitmap;
    uint32_t vaddr_start;  //虚拟地址的起始地址
};

#endif
