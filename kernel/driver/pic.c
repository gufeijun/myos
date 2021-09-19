#include "pic.h"

#include "io.h"

#define PIC_MASTR 0x20
#define PIC_SLAVE 0xa0

void pic_init() {
    outb(PIC_MASTR, 0x11);
    outb(PIC_MASTR + 1, 0x20);

    outb(PIC_MASTR + 1, 0x04);
    outb(PIC_MASTR + 1, 0x01);

    outb(PIC_SLAVE, 0x11);
    outb(PIC_SLAVE + 1, 0x28);

    outb(PIC_SLAVE + 1, 0x02);
    outb(PIC_SLAVE + 1, 0x01);

    outb(PIC_MASTR + 1, 0xfe);
    outb(PIC_SLAVE + 1, 0xff);
}
