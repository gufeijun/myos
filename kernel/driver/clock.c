#include "clock.h"

#include "io.h"
#include "print.h"

#define IRQ0_FREQUENCY 1000
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY

#define IO_TIMER1 0x40
#define TIMER_MODE 0x43
#define TIMER_SEL0 0x00
#define TIMER_RATEGEN 0x04
#define TIMER_16BIT 0x30

/**
 * 初始化PIT 8253，更改中断频率。
 */
void clock_init() {
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, (uint8_t)COUNTER0_VALUE);
    outb(IO_TIMER1, (uint8_t)COUNTER0_VALUE >> 8);
}
