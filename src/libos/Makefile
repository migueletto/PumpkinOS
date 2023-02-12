include ../common.mak
include ../commonp.mak

PROGRAM=$(BIN)/libos$(SOEXT)
OBJS=libos.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lpumpkin

clean:
	rm -f $(PROGRAM) $(OBJS)
