#include "print.h"

#include "io.h"
#include "stdarg.h"
#include "stdint.h"
#include "string.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_CHARACTORS ((VGA_WIDTH) * (VGA_HEIGHT))
#define VIDEO_BASE_ADDR 0xb8000
#define VIDEO_Nth_LINE_ADDR(n) (VIDEO_FIRST_LINE_ADDR + (n - 1) * VGA_WIDTH * 2)
#define VIDEO_FIRST_LINE_ADDR VIDEO_BASE_ADDR
#define VIDEO_LAST_LINE_ADDR (VIDEO_Nth_LINE_ADDR(VGA_HEIGHT))

// in VGA text video mode, each page of sceen can hold
// 2000 characters. we use a uint16_t type to describe the pos.
static inline uint16_t __get_cursor_pos() {
    uint16_t pos = 0;
    // get low 8 bits of cursor postion
    outb(0x3d4, 0xf);
    pos |= inb(0x3d5);
    // get high 8 bits of cursor postion
    outb(0x3d4, 0xe);
    pos |= ((uint16_t)inb(0x3d5)) << 8;
    return pos;
}

static inline void __set_char(uint16_t pos, uint8_t ch, uint8_t attr) {
    uint8_t* addr = (uint8_t*)(VIDEO_BASE_ADDR + pos * 2);
    *addr++ = ch;
    *addr = attr;
}

static inline void __scroll_lines(int n) {
    if (n > VGA_HEIGHT) n = VGA_HEIGHT;
    size_t size = 2 * VGA_CHARACTORS - n * 2 * VGA_WIDTH;
    memcpy((void*)VIDEO_FIRST_LINE_ADDR, (void*)VIDEO_Nth_LINE_ADDR(n + 1),
           size);
    memset((void*)(VIDEO_LAST_LINE_ADDR - (n - 1) * VGA_WIDTH * 2), 0,
           n * 2 * VGA_WIDTH);
    // set the attribute of the first character fo last line(cursor), making
    // cursor visible
    // *(uint8_t*)(VIDEO_LAST_LINE_ADDR + 1) = 0x07;
    __set_char(VGA_CHARACTORS - VGA_WIDTH, 0, 0x7);
}

static inline void __set_cursor_pos(uint16_t pos) {
    // when cursor goes out of screen, the screen scrolls up several lines
    if (pos >= VGA_CHARACTORS) {
        __scroll_lines((pos - VGA_CHARACTORS) / VGA_WIDTH + 1);
        pos = VGA_CHARACTORS - VGA_WIDTH + pos % VGA_WIDTH;
    }
    // set low 8 bits of cursor postion
    outb(0x3d4, 0xf);
    outb(0x3d5, (uint8_t)(pos & 0xFF));
    // set high 8 bits of cursor postion
    outb(0x3d4, 0xe);
    outb(0x3d5, (uint8_t)((pos >> 8) & 0xFF));
}

void put_char(char ch) {
    if (ch == '\0') return;
    uint16_t pos = __get_cursor_pos();
    // when encountering CR or LF, the cursor wraps
    if (ch == '\n' || ch == '\r') {
        if (pos >= VGA_CHARACTORS - VGA_WIDTH) {
            __scroll_lines(1);
            __set_cursor_pos(VGA_CHARACTORS - VGA_WIDTH);
            return;
        }
        __set_cursor_pos(pos - pos % VGA_WIDTH + VGA_WIDTH);
        return;
    }
    __set_char(pos, ch, 0x07);
    pos++;
    __set_cursor_pos(pos);
}

// TODO Depending on put_char will increase the number of times we call
// __get_cursor_pos and __set_cursor_pos. we should better rewrite it.
void put_str(char* str) {
    while (*str) {
        put_char(*str);
        str++;
    }
}

void put_int(int num) {
    // 32-bit integers have up to 10 digits indecimal
    char buf[11];
    int i = 10;
    if (num < 0) {
        put_char('-');
        num = 0 - num;
    }
    while (num != 0) {
        buf[--i] = num % 10 + '0';
        num /= 10;
    }
    buf[10] = 0;
    put_str(buf + i);
}

void printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

loop:
    if (*fmt != '%') {
        if (*fmt == '\0') goto end;
        put_char(*fmt++);
        goto loop;
    }
    switch (*(++fmt)) {
        case 'd':
            put_int(va_arg(args, int));
            break;
        case 'c':
            put_char(va_arg(args, int));
            break;
        case 's':
            put_str(va_arg(args, char*));
            break;
        case '\0':
            goto end;
        case '%':
            put_char('%');
            break;
        default:
            // can't recognize, using '0' to replace
            put_char('0');
    }
    fmt++;
    goto loop;
end:
    va_end(args);
}
