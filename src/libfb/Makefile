include ../common.mak

PROGRAM=$(BIN)/libfb$(SOEXT)

OBJS=libfb.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
