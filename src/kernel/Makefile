OSNAME=Kernel

include ../common.mak

PROGRAM=pumpkin

CUSTOMFLAGS=-g -nostartfiles -I$(SRC)/libfatfs -I$(SRC)/font -I$(SRC)/libbcm2835

ifdef RPI
EXTRA_MODULES=$(BIN)/libbcm2835.a
else
EXTRA_MODULES=$(BIN)/liblsdl2.a
endif

LD=ld
OBJCOPY=objcopy

SOURCE=main.c script.c disk.c
OBJS=$(SOURCE:%.c=%.o)
MODULES=$(BIN)/libos.a $(BIN)/libwsurface.a $(EXTRA_MODULES) $(BIN)/libgpio.a $(BIN)/libspi.a $(BIN)/libili9486.a $(BIN)/libpumpkin.a $(BIN)/libpit.a $(BIN)/libfatfs.a float/float.a

$(PROGRAM): $(OBJS) $(MODULES)
	$(CC) -o $(PROGRAM) $(OBJS) $(MODULES)

$(PROGRAM).img: $(PROGRAM).elf
	$(OBJCOPY) -O binary $(PROGRAM).elf $(PROGRAM).img

$(PROGRAM).elf: boot.o $(OBJS) $(MODULES)
	$(LD) boot.o $(OBJS) $(MODULES) -T link.ld -o $(PROGRAM).elf

boot.o: boot.S
	$(CC) -ffreestanding -nostdinc -nostdlib -nostartfiles -c boot.S -o boot.o

clean:
	rm -f boot.o $(OBJS) $(PROGRAM) $(PROGRAM).elf $(PROGRAM).img
