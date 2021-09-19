#include "idt.h"

#include "print.h"
#include "stdint.h"

#define IDT_DESC_CNT 0x21

extern uintptr_t intr_entry_table[IDT_DESC_CNT];

struct idt_desc {
    uint16_t low_16_bits;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attr;
    uint16_t high_16_bits;
} idt_descs[IDT_DESC_CNT];

void intr_all_handler(uint8_t intr_no) {
    if (intr_no == 0x27 || intr_no == 0x3f) {
        return;
    }
    //在此处理所有中断
    printf("%s", "g");
    return;
}

static void setup_idt_desc(struct idt_desc* desc, uintptr_t base,
                           uint8_t attr) {
    desc->low_16_bits = (base & 0xFFFF);
    desc->selector = (1 << 3);
    desc->dcount = 0;
    desc->attr = attr;
    desc->high_16_bits = (base >> 16) & 0xFFFF;
}

static void idt_descs_init() {
    uint32_t i;
    for (i = 0; i < IDT_DESC_CNT; i++) {
        setup_idt_desc(&idt_descs[i], (uintptr_t)intr_entry_table[i],
                       (1 << 7) + 0xe);
    }
}

void idt_init() {
    idt_descs_init();
    uint64_t idt_operand =
        ((sizeof(idt_descs) - 1) | ((uint64_t)((uintptr_t)idt_descs << 16)));
    asm volatile("lidt %0" : : "m"(idt_operand));
}
