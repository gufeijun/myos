#include "thread.h"

#include "memory.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096
#define MAGIC_LOVER 0x686A6C

static void kernel_thread(thread_func function, void* func_arg) {
    function(func_arg);
}

void thread_create(struct task_struct* pthread, thread_func function,
                   void* func_arg) {
    //给线程栈空间预留intr_stack栈和thread_stack的空间
    pthread->self_kstack -= sizeof(struct intr_stack);
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack =
        (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi =
        kthread_stack->edi = 0;
}

void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    if (name) strcpy(pthread->name, name);
    pthread->status = TASK_RUNNING;
    pthread->priority = prio;
    //指向该块的最顶端
    pthread->self_kstack = (uintptr_t)pthread + PG_SIZE;
    pthread->stack_magic = MAGIC_LOVER;
}

struct task_struct* thread_start(char* name, int prio, thread_func function,
                                 void* func_arg) {
    //给PCB分配一页
    struct task_struct* thread = kmalloc_page(1);
    if (!thread) return NULL;
    init_thread(thread, name, prio);
    thread_create(thread, function, func_arg);
    /* the 4KB of PCB looks like this:
     *  |  intr_stack  |  the end of the page
     *  | thread_stack |
     *  |     ...      |
     *  |  task_struct |  the start of the page
     */
    asm volatile(
        "movl %0,%%esp;"
        "pop %%ebp;pop %%ebx;pop %%edi;pop %%esi;"
        "ret;" ::"g"(thread->self_kstack)
        : "memory");
    return thread;
}
