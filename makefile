EXECUTABLES := i386-elf-gcc i386-elf-ld nasm dd bochs
OUT_DIR = obj bin

BOCHS = bochs
CC = i386-elf-gcc
LD = i386-elf-ld
NASM = nasm
RM = rm -rf

IMG = boot.img
BOCHS_CONFIG = bochsrc
CFLAGS = -Wall -O2 -nostdlib -nostdinc
LDFLAGS = -Ttext 0xc0001500 -e main
DDFLAGS = bs=512 conv=notrunc of=$(IMG)

VPATH = kernel

K := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $(exec)),,$(error "No $(exec) in PATH")))

bochs:dirs boot.img
	$(BOCHS) -f $(BOCHS_CONFIG)
	# qemu -hda boot.img
	
$(IMG):bin/mbr.bin bin/loader.bin bin/kernel.bin
	dd if=/dev/zero  count=131040 $(DDFLAGS)
	dd if=$< count=1 $(DDFLAGS)
	dd if=bin/loader.bin count=4 seek=1 $(DDFLAGS)
	dd if=bin/kernel.bin count=20 seek=5 $(DDFLAGS)


#-------------------objs-------------------
obj/main.o:kernel/main.c
	$(CC) $< -o $@ $(CFLAGS)

#-------------------bins-------------------
bin/mbr.bin:boot/mbr.S
	$(NASM) $< -o $@

bin/loader.bin:boot/loader.S
	$(NASM) $< -o $@ -I./boot

bin/kernel.bin:obj/main.o
	$(LD) $^ -o bin/kernel.bin $(LDFLAGS)

$(OUT_DIR):
	@mkdir $(OUT_DIR)

.PHONY:clean dirs cleanobj cleanbin
dirs: $(OUT_DIR)
cleanobj:
	@$(RM) obj
cleanbin:
	@$(RM) bin
clean: cleanobj cleanbin
