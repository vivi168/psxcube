GCCRS_INCOMPLETE_AND_EXPERIMENTAL_COMPILER_DO_NOT_USE := 1
TOOLCHAIN := mipsel-none-elf-

ifeq ($(PSYQ_PATH),)
$(error PSYQ SDK path is not set)
endif

ELF := MAIN.ELF
EXE := MAIN.EXE
ISO := cube.bin

CC := $(TOOLCHAIN)gcc
CCRS := $(TOOLCHAIN)gccrs
LD := $(TOOLCHAIN)ld
OBJCOPY := $(TOOLCHAIN)objcopy

INC := -I$(PSYQ_PATH)/include
LIB := -L$(PSYQ_PATH)/lib -lgpu -lgte -lcd -letc -lsn -lsnd -lspu -lcard -lpad -lc2 -lapi -lextra

RSFLAGS := -g -O2 -G0 -nostdlib -mno-unaligned-access -msoft-float -Wall -Wextra $(INC)
CFLAGS := -g -O2 -G0 -ffreestanding -nostdlib -mno-unaligned-access -msoft-float -Wall -Wextra $(INC)
LDFLAGS := -T linker.ld

SRC := stdafx.c input.c io.c main.c mesh.c renderer.c linalg.c camera.c noise.c terrain.c hashmap.c
RS_SRC := test.rs

OBJ := $(SRC:.c=.o)
RS_OBJ := $(RS_SRC:.rs=.o)

all: $(ISO)

$(ISO): $(EXE)
	mkpsxiso -y mkpsxiso.xml

$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(OBJ) $(RS_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIB)

%.o: %.rs
	$(CCRS) -c $(RSFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(ELF) $(EXE) $(OBJ) $(RS_OBJ) $(ISO)

run:
	cd log && pcsx-redux -bios openbios.bin -fastboot -interpreter -debugger -iso ../$(ISO) -run -logfile development.log

install:
	cp $(ISO) $(INSTALL_PATH)

.PHONY: all clean install
