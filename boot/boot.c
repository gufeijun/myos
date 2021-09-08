#include "elf.h"
#include "io.h"
#include "string.h"

// kernel.bin(elf) will be loaded in 0x70000
// you can choose any proper address as you wish
#define KERNEL_BIN_BASEADDR 0x70000
struct elfhdr* ELFHDR = (struct elfhdr*)KERNEL_BIN_BASEADDR;
// mbr and loader requires 1 and 4 sector to store respectively
#define KERNEL_BIN_SECTOR 5

#define SECTOR_SIZE 512

static inline void wait_disk() {
    while ((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

// read a single sector at @secno to memory addr @dst
static void readsect(void* dst, uint32_t secno) {
    wait_disk();

    // write the number of sectors to be read into 0x1F2(Feature register)
    outb(0x1F2, 1);
    // write LBA addr into LBA low,mid,high and Device register
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    outb(0x1F7, 0x20);  // 0x1F7 is Command register, 0x20 = write command

    wait_disk();

    // read four bytes per time
    insl(0x1F0, dst, SECTOR_SIZE / 4);
}

// read @count sectors from @start_sec to addr @dest
static void read_sectors(uintptr_t dst, int count, int start_sec) {
    int i;
    for (i = 0; i < count; i++) {
        readsect((void*)(dst + SECTOR_SIZE * i), start_sec + i);
    }
}

void boot() {
    // assuming our kernel is less than 100KB
    read_sectors((uintptr_t)KERNEL_BIN_BASEADDR, 200, KERNEL_BIN_SECTOR);

    // after load kernel, we need to check the validation of it
    if (*(uint32_t*)(ELFHDR->e_elf) != ELF_MAGIC) {
        goto bad;
    }

    // TODO parse ELF

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1)
        ;
}
