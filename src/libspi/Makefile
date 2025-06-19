include ../common.mak

PROGRAM=$(BIN)/libspi$(SOEXT)

OBJS=libspi.o spi.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

clean:
	rm -f $(PROGRAM) *.o
