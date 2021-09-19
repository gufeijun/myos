#ifndef __KERNEL_DEBUG_DEBUG_H_
#define __KERNEL_DEBUG_DEBUG_H_

void panic_spin(char* filename, int line, const char* func,
                const char* condition);

//变参宏，用__VA_ARGS__表示这些变参。__FILE__代表文件名，__LINE__代表行号，__func__代表函数名，
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) \
    if (!(CONDITION)) PANIC(#CONDITION);
#endif
// CONDITION前加了个#号，这时PANIC获得参数是将CONDITION变成一个字符串char*

#endif
