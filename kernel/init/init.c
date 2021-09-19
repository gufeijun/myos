#include "init.h"

#include "clock.h"
#include "idt.h"
#include "intr.h"
#include "pic.h"
#include "print.h"

void init() {
    pic_init();
    put_str("pic init succ!\n");
    idt_init();
    put_str("idt init succ!\n");
    clock_init();
    put_str("clock init succ!\n");
    // intr_enable();
}
