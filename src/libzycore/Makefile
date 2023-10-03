include ../common.mak

PROGRAM=$(BIN)/libzycore$(SOEXT)

OBJS=Allocator.o ArgParse.o Bitset.o Format.o List.o Memory.o Process.o String.o Synchronization.o Terminal.o Thread.o Vector.o Zycore.o

CUSTOMFLAGS=-I.

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS)

clean:
	rm -f $(PROGRAM) $(OBJS)
