#include "elf.h"
#include "io.h"
#include "stdint.h"
#include "string.h"

// kernel.bin(elf) will be loaded in 0x70000
// the entry of running kernel is 0xc0001500
// you can choose any proper address as you wish
#define KERNEL_BIN_BASEADDR 0x70000
// mbr and loader requires 1 and 4 sector to store respectively
#define KERNEL_BIN_SECTOR 5
#define SECTOR_SIZE 512

typedef void (*kentry)();

static inline void wait_disk() {
    while ((inb(0x1F7) & 0xC0) != 0x40)
        ;
}

// read a single sector at @secno to memory addr @dst
static inline void readsect(void* dst, uint32_t secno) {
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
static inline void read_sectors(uintptr_t dst, int count, int start_sec) {
    int i;
    for (i = 0; i < count; i++) {
        readsect((void*)(dst + SECTOR_SIZE * i), start_sec + i);
    }
}

void boot() {
    struct elfhdr* ELFHDR = (struct elfhdr*)KERNEL_BIN_BASEADDR;
    // assuming our kernel is less than 100KB
    read_sectors((uintptr_t)KERNEL_BIN_BASEADDR, 200, KERNEL_BIN_SECTOR);

    // after load kernel.bin, we need to check the validation of it
    if (ELFHDR->e_magic != ELF_MAGIC) {
        goto bad;
    }

    struct proghdr *ph_start, *ph_end;

    // see struct elfhdr in lib/elf.h. e_phoff is the file offset of program
    // header . e_phnum is the number of program header(ph). every ph represents
    // a segment, we need to copy these segments to right places.
    ph_start = (struct proghdr*)(ELFHDR->e_phoff + (uintptr_t)ELFHDR);
    ph_end = ph_start + ELFHDR->e_phnum;

    // see struct proghdr in lib/elf.h . p_va is the virtual addr the segment
    // should map. p_oofset and p_memsz is the file offset and size of this
    // segment respectively.
    for (; ph_start < ph_end; ph_start++) {
        memcpy((void*)ph_start->p_va,
               (void*)((uintptr_t)ELFHDR + ph_start->p_offset),
               ph_start->p_memsz);
    }

    // asm volatile("jmp .");
    // finnaly we can enter the kernel
    ((kentry)(ELFHDR->e_entry))();

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

    /* do nothing */
    while (1)
        ;
}
