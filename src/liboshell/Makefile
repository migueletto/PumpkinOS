include ../common.mak

PROGRAM=$(BIN)/liboshell$(SOEXT)

OBJS=liboshell.o oshell.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lutil

clean:
	rm -f $(PROGRAM) $(OBJS)
