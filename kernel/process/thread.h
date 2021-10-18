#ifndef __KERNEL_PROCESS_THREAD_H_
#define __KERNEL_PROCESS_THREAD_H_
#include "stdint.h"

typedef void (*thread_func)(void*);

enum task_status {
    TASK_RUNNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DEAD,
};

//详见vectors.asm
//在调用intr_all_handler之前，我们的栈信息就如此,栈顶到栈底的数据依次如下
struct intr_stack {
    uint32_t vec_no;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    // cpu从低特权级进入高特权级时,还会压入以下字段
    uint32_t err_code;
    void (*eip)();
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};

struct thread_stack {
    //根据ABI规则，被调用函数有义务保持这四个寄存器
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    //第一次调用时,eip就是thread.c中的kernel_thread函数
    //调用kernel_thread不是使用call，而是当esp指向kernel_thread
    //时，进行ret，这时cpu会把kernel_thread(即下面的eip)当做retaddr.
    //这时esp就指向了unused_retaddr，栈中的function和func_arg正好可以作为
    // kernel_thread的两个参数
    void (*eip)(thread_func func, void* func_arg);
    //以下第一次由CPU调度时使用
    //返回地址，这里起到占位的作用
    void(*unused_retaddr);
    //下面两个是eip这个函数的两个参数
    thread_func function;
    void* func_arg;
};

// 进程或线程的PCB
// PCB占据一整块，我们将该线程栈的栈顶设为这块的最高处
// task_struct放在该块的最低处
struct task_struct {
    //栈顶的位置
    uintptr_t self_kstack;
    //线程的状态
    enum task_status status;
    //线程的优先级
    uint8_t priority;
    //线程的名字
    char name[16];
    //栈的边界标记，用于检测栈是否溢出
    //如果栈顶向下增长导致stack_magic改变说明溢出
    uint32_t stack_magic;
};

struct task_struct* thread_start(char* name, int prio, thread_func function,
                                 void* func_arg);

#endif
