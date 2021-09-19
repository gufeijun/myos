EMPTY					:=
SPACE					:= ${EMPTY} ${EMPTY}
MKDIRS 					:= ./obj/ ./obj/boot/ ./bin
LOADER_BASE_ADDR 		:= 0x900
IMG 					:= boot.img

EXECUTABLES				:= i386-elf-gcc i386-elf-ld nasm dd bochs objcopy
T 						:= $(foreach exec,$(EXECUTABLES),\
        				$(if $(shell which $(exec)),,$(error "No $(exec) in PATH!")))

K_DIR_SRC 				:= ./lib/ ./kernel/ ./kernel/lib/ ./kernel/driver/ ./kernel/init/ ./kernel/debug/ ./kernel/mm/
K_DIR_INC 				:= ./lib/ ./kernel/lib/	./kernel/driver/ ./kernel/init/	./kernel/debug/ ./kernel/mm/
DIR_BIN					:= ./bin/
DIR_OBJ					:= ./obj/
DIR_OBJ_BOOT			:= ./obj/boot/
BOOT_DIR_SRC			:= ./boot/
BOOT_DIR_INC			:= ./lib/
BOOT_DIR_INC_ASM		:= ./boot/

NASM 					:= nasm
LD 						:= i386-elf-ld
CC 						:= i386-elf-gcc
OBJCOPY 				:= objcopy
RM 						:= rm -rf
BOCHS 					:= bochs

OBJCP_FLAGS				:= -S -O binary
INCLUDE_PATH			:= ${foreach n,${K_DIR_INC},-I${n}}
CFLAGS 					:= -Wall -O2 -nostdlib -nostdinc \
						-fno-builtin  -fno-stack-protector \
						-fno-PIC -Werror  ${INCLUDE_PATH}
LFLAGS 					:=
LDFLAGS 				:= -Ttext 0xc0001500 -e main
DDFLAGS = bs=512 conv=notrunc of=bin/$(IMG)

vpath 		%.asm		${BOOT_DIR_INC_ASM}
vpath 		%.bin 		${DIR_BIN}
vpath 		%.h 		${BOOT_DIR_INC}
vpath 		%.o 		${DIR_OBJ_BOOT}
vpath  		%.elf 		${BOOT_BIN}
vpath 		%.c  		${BOOT_DIR_SRC}
vpath 		%.img 		${DIR_BIN}

.PHONY:all
all:dirs boot.img

boot.img:mbr.bin loader.bin kernel.bin
	dd if=/dev/zero  count=131040 $(DDFLAGS)
	dd if=bin/mbr.bin count=1 $(DDFLAGS)
	dd if=bin/loader.bin count=4 seek=1 $(DDFLAGS)
	dd if=bin/kernel.bin count=200 seek=5 $(DDFLAGS)
	@echo "create bin/boot.img success!"
	@echo "run 'make bochs' to start os!"

# -------------------------------------------------------------------
#  mbr
mbr.bin:mbr.asm
	${NASM} $< -o ${DIR_BIN}$@

# -------------------------------------------------------------------
#  loader
NASMFLAG				:= -I${BOOT_DIR_INC_ASM} -f elf
loader.bin:loader.elf
	$(OBJCOPY) ${OBJCP_FLAGS} ${DIR_BIN}$< ${DIR_BIN}$@
loader.elf:loader.o boot.o
	$(LD) ${DIR_OBJ_BOOT}loader.o ${DIR_OBJ_BOOT}boot.o  -o ${DIR_BIN}loader.elf -e _start -Ttext ${LOADER_BASE_ADDR} -N
loader.o:loader.asm page.asm
	${NASM} -o ${DIR_OBJ_BOOT}$@ $<  ${NASMFLAG} 
boot.o: boot.c elf.h stdint.h io.h string.h
	${CC} -c -o ${DIR_OBJ_BOOT}$@ $< ${CFLAGS} -Os

# -------------------------------------------------------------------
# kernel
SRC_WITH_PATH			:= ${foreach n,${K_DIR_SRC},${wildcard ${n}*.c}}
SRC 					:= ${notdir ${SRC_WITH_PATH}}

OBJ_WITH_PATH			:= ${patsubst %.c,${DIR_OBJ}%.o,${SRC}}
OBJ 					:= ${notdir ${OBJ_WITH_PATH}}

VPATH 					:= ${subst ${SPACE},:,${K_DIR_SRC}}:${DIR_BIN}
vpath 		%.c 		${subst ${SPACE},:,${K_DIR_SRC}}
vpath 		%.o			${DIR_OBJ}
vpath 		%.d 		${DIR_OBJ}
vpath 		%.h 		${K_DIR_INC}
vpath 		%.asm		./kernel/driver


kernel.bin:${OBJ:.o=.d} ${OBJ} vectors.o
	${LD} ${OBJ_WITH_PATH} ${DIR_OBJ}vectors.o -o ${DIR_BIN}$@ ${LDFLAGS}

vectors.o:vectors.asm
	${NASM} -o ${DIR_OBJ}$@ -f elf $<

%.o:%.c
	${CC} ${CFLAGS} -c -o ${DIR_OBJ}$@ $<

%.d:%.c
	${CC} ${CFLAGS} -MM -MT "$(subst .c,.o,${notdir $<}) $(subst .c,.d,${notdir $<})" -MF "$(subst .c,.d,${DIR_OBJ}${notdir $<})" $<
	
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),dirs)
ifneq ($(MAKECMDGOALS),bochs)
-include ${OBJ_WITH_PATH:.o=.d}
endif
endif
endif

.PHONY:clean
clean:
	-rm -rf ${DIR_BIN}
	-rm -rf ${DIR_OBJ}

.PHONY:dirs
dirs:${MKDIRS}
${MKDIRS}:
	@mkdir $@
.PHONY:bochs
bochs:all
	${BOCHS} -f bochsrc
