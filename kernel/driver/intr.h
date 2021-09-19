#ifndef __KERNEL_DRIVER_INTR_H_
#define __KERNEL_DRIVER_INTR_H_

static inline void intr_enable() { asm volatile("sti"); }
static inline void intr_disable() { asm volatile("cli"); }

#endif
