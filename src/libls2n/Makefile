include ../common.mak

PROGRAM=$(BIN)/libls2n.so

OBJS=libls2n.o

# set S2N variable to s2n-tls location
CUSTOMFLAGS=-I$(S2N)/api

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -L$(S2N)/lib -lpit -ls2n

clean:
	rm -f $(PROGRAM) *.o
