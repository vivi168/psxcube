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

LIB := \
	$(PSYQ_PATH)/lib/libcard.a \
	$(PSYQ_PATH)/lib/libpress.a \
	$(PSYQ_PATH)/lib/libgpu.a \
	$(PSYQ_PATH)/lib/libgs.a \
	$(PSYQ_PATH)/lib/libgte.a \
	$(PSYQ_PATH)/lib/libcd.a \
	$(PSYQ_PATH)/lib/libetc.a \
	$(PSYQ_PATH)/lib/libsn.a \
	$(PSYQ_PATH)/lib/libsnd.a \
	$(PSYQ_PATH)/lib/libspu.a \
	$(PSYQ_PATH)/lib/libmath.a \
	$(PSYQ_PATH)/lib/libcomb.a \
	$(PSYQ_PATH)/lib/libcard.a \
	$(PSYQ_PATH)/lib/libtap.a \
	$(PSYQ_PATH)/lib/libsio.a \
	$(PSYQ_PATH)/lib/libpad.a \
	$(PSYQ_PATH)/lib/libc2.a \
	$(PSYQ_PATH)/lib/libapi.a \
	$(PSYQ_PATH)/lib/extra.a

CFLAGS := -g -O2 -G0 -ffreestanding -nostdlib -mno-unaligned-access -Wall -Wextra $(INC)
CFLAGS += -Dmodern_toolchain
LDFLAGS := -T linker.ld

SRC := input.c io.c main.c mesh.c renderer.c

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

.PHONY: all clean
