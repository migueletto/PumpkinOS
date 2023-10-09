include ../common.mak

PROGRAM=$(BIN)/libspi$(SOEXT)

CUSTOMFLAGS=-DBCM2835 -I$(SRC)/libbcm2835

OBJS=libspi.o spi.o

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lbcm2835 -lpit

clean:
	rm -f $(PROGRAM) *.o
