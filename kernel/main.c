#include "init.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

void haha(void* arg) { printf("%s\n", "hello world\n"); }

int main() {
    init();
    // char* str = "hello wolrd\n";
    // thread_start("haha", 1, haha, (void*)str);
    while (1)
        ;
    return 0;
}
