EXECUTABLES := i386-elf-gcc i386-elf-ld nasm dd bochs objcopy
OUT_DIR = obj bin

BOCHS = bochs
CC = i386-elf-gcc
LD = i386-elf-ld
NASM = nasm
RM = rm -rf
OBJCOPY = objcopy

IMG = boot.img
BOCHS_CONFIG = bochsrc
CFLAGS = -Wall -O2 -nostdlib -nostdinc -c -fno-builtin -Werror -fno-PIC -fno-stack-protector
INCLUDEPATH = -I./lib -I. -I./kernel/lib
LDFLAGS = -Ttext 0xc0001500 -e main
DDFLAGS = bs=512 conv=notrunc of=bin/$(IMG)

VPATH = kernel kernel/lib lib

KOBJECTS = main.o print.o

K := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $(exec)),,$(error "No $(exec) in PATH")))

bochs: bin/boot.img
	$(BOCHS) -f $(BOCHS_CONFIG)
	# qemu -hdb bin/boot.img
	
bin/$(IMG):dirs bin/mbr.bin bin/loader.bin bin/kernel.bin
	dd if=/dev/zero  count=131040 $(DDFLAGS)
	dd if=bin/mbr.bin count=1 $(DDFLAGS)
	dd if=bin/loader.bin count=4 seek=1 $(DDFLAGS)
	dd if=bin/kernel.bin count=200 seek=5 $(DDFLAGS)


#-------------------objs-------------------
obj/main.o:kernel/main.c kernel/lib/print.h
	$(CC) $< -o $@ $(CFLAGS) $(INCLUDEPATH)

obj/boot.o:boot/boot.c lib/elf.h lib/io.h lib/stdint.h lib/string.h
	$(CC)  $< -o  $@ $(INCLUDEPATH) $(CFLAGS) -Os

obj/loader.o:boot/loader.S
	$(NASM) -o $@ $< -I./boot -f elf

obj/print.o: kernel/lib/print.c kernel/lib/print.h lib/io.h lib/stdint.h
	$(CC) $< -o $@ $(INCLUDEPATH) $(CFLAGS)

#-------------------bins-------------------
bin/mbr.bin:boot/mbr.S
	$(NASM) $< -o $@

bin/loader.bin: obj/loader.o obj/boot.o
	$(LD) -e _start $^  -o bin/loader.elf -Ttext 0x900 -N
	$(OBJCOPY) -S -O binary bin/loader.elf bin/loader.bin

bin/kernel.bin:$(addprefix obj/,$(KOBJECTS))
	$(LD) $^ -o bin/kernel.bin $(LDFLAGS)

$(OUT_DIR):
	@mkdir $(OUT_DIR)

.PHONY:clean dirs cleanobj cleanbin img
dirs: $(OUT_DIR)
cleanobj:
	@$(RM) obj
cleanbin:
	@$(RM) bin
clean: cleanobj cleanbin
img:bin/$(IMG)
