#ifndef __LIB_IO_H_
#define __LIB_IO_H_
#include "stdint.h"

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a"(data) : "d"(port));
    return data;
}

static inline void insl(uint32_t port, void *addr, int cnt) {
    asm volatile(
        "cld;"
        "repne; insl;"
        : "=D"(addr), "=c"(cnt)
        : "d"(port), "0"(addr), "1"(cnt)
        : "memory", "cc");
}

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" ::"a"(data), "d"(port));
}

static inline void outw(uint16_t port, uint16_t data) {
    asm volatile("outw %0, %1" ::"a"(data), "d"(port));
}

#endif
