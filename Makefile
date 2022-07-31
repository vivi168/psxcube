TOOLCHAIN := mipsel-none-elf-

ifeq ($(PSYQ),)
$(error PSYQ SDK path is not set)
endif

ELF := MAIN.ELF
EXE := MAIN.EXE
ISO := cube.bin

CC := $(TOOLCHAIN)gcc
LD := $(TOOLCHAIN)ld
OBJCOPY := $(TOOLCHAIN)objcopy

INC := -I$(PSYQ)/include

LIB := \
	$(PSYQ)/lib/libcard.a \
	$(PSYQ)/lib/libpress.a \
	$(PSYQ)/lib/libgpu.a \
	$(PSYQ)/lib/libgs.a \
	$(PSYQ)/lib/libgte.a \
	$(PSYQ)/lib/libcd.a \
	$(PSYQ)/lib/libetc.a \
	$(PSYQ)/lib/libsn.a \
	$(PSYQ)/lib/libsnd.a \
	$(PSYQ)/lib/libspu.a \
	$(PSYQ)/lib/libmath.a \
	$(PSYQ)/lib/libcomb.a \
	$(PSYQ)/lib/libcard.a \
	$(PSYQ)/lib/libtap.a \
	$(PSYQ)/lib/libsio.a \
	$(PSYQ)/lib/libpad.a \
	$(PSYQ)/lib/libc2.a \
	$(PSYQ)/lib/libapi.a

CFLAGS := -g -O3 -G0 -ffreestanding $(INC)
CFLAGS += -Dmodern_toolchain
LDFLAGS := -g -T linker.ld

SRC := input.c io.c main.c mesh.c renderer.c

OBJ := $(SRC:.c=.o)

all: $(ISO)

$(ISO): $(EXE)
	mkpsxiso -y mkpsxiso.xml

$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(OBJ)
	$(LD) -o $@ $(LDFLAGS) $^ $(LIB)

.c.o:
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(ELF) $(EXE) $(OBJ) $(ISO)

.PHONY: all clean
