OSNAME=Kernel

include ../common.mak

PROGRAM=pumpkin

CUSTOMFLAGS=-g -I$(SRC)/libfatfs -I$(SRC)/font -I$(SRC)/libbcm2835

ifeq ($(RPI),1)
RPI_MODULES=$(BIN)/libbcm2835.a
endif

SOURCE=main.c script.c disk.c
OBJS=$(SOURCE:%.c=%.o)
MODULES=$(BIN)/libos.a $(BIN)/libwsurface.a $(BIN)/libgpio.a $(BIN)/libspi.a $(BIN)/libili9486.a $(BIN)/liblsdl2.a $(BIN)/libpumpkin.a $(BIN)/libpit.a $(BIN)/libfatfs.a $(RPI_MODULES)

$(PROGRAM): $(OBJS) $(MODULES)
#	$(CC) -static -nostdlib -o $(PROGRAM) $(OBJS) $(MODULES)
	$(CC) -o $(PROGRAM) $(OBJS) $(MODULES) -lSDL2 -lm

clean:
	rm -f $(OBJS) $(PROGRAM)
