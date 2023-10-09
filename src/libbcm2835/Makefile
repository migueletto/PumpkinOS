include ../common.mak

PROGRAM=$(BIN)/libbcm2835$(SOEXT)

OBJS=bcm2835.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN)

clean:
	rm -f $(PROGRAM) *.o
