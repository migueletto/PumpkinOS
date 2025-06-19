include ../common.mak

PROGRAM=$(BIN)/libili9486$(SOEXT)

CUSTOMFLAGS=-I$(SRC)/font

OBJS=libili9486.o ili9486.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
