OSNAME=Kernel

include ../common.mak

PROGRAM=pumpkin

CUSTOMFLAGS=-g -I$(SRC)/libfatfs

SOURCE=main.c script.c disk.c
OBJS=$(SOURCE:%.c=%.o)
MODULES=$(BIN)/libos.a $(BIN)/liblsdl2.a $(BIN)/libpumpkin.a $(BIN)/libpit.a $(BIN)/libfatfs.a

$(PROGRAM): $(OBJS) $(MODULES)
#	$(CC) -static -nostdlib -o $(PROGRAM) $(OBJS) $(MODULES)
	$(CC) -o $(PROGRAM) $(OBJS) $(MODULES) -lSDL2 -lm

clean:
	rm -f $(OBJS) $(PROGRAM)
