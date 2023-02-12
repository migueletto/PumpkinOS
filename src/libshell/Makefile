include ../common.mak

PROGRAM=$(BIN)/libshell$(SOEXT)

OBJS=libshell.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
