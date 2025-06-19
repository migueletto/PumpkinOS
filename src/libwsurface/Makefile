include ../common.mak

PROGRAM=$(BIN)/libwsurface$(SOEXT)

OBJS=libwsurface.o

CUSTOMFLAGS=-I$(SRC)/font

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

clean:
	rm -f $(PROGRAM) *.o
