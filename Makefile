TOOLCHAIN := mipsel-none-elf-

ifeq ($(PSYQ_PATH),)
$(error PSYQ SDK path is not set)
endif

ELF := MAIN.ELF
EXE := MAIN.EXE
ISO := cube.bin

CC := $(TOOLCHAIN)gcc
LD := $(TOOLCHAIN)ld
OBJCOPY := $(TOOLCHAIN)objcopy

INC := -I$(PSYQ_PATH)/include
LIB := -L$(PSYQ_PATH)/lib -lgpu -lgte -lcd -letc -lsn -lsnd -lspu -lcard -lpad -lc2 -lapi -lextra

CFLAGS := -g -O2 -G0 -ffreestanding -nostdlib -mno-unaligned-access -Wall -Wextra $(INC)
LDFLAGS := -T linker.ld

SRC := stdafx.c input.c io.c main.c mesh.c renderer.c linalg.c

OBJ := $(SRC:.c=.o)

all: $(ISO)

$(ISO): $(EXE)
	mkpsxiso -y mkpsxiso.xml

$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIB)

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(ELF) $(EXE) $(OBJ) $(ISO)

run:
	cd log && pcsx-redux -bios openbios.bin -fastboot -interpreter -debugger -iso ../$(ISO) -run -logfile development.log

.PHONY: all clean
