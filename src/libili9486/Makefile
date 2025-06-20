include ../common.mak

CUSTOMFLAGS=-I$(SRC)/font

OBJS=libili9486.o ili9486.o

ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libili9486.a
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libili9486$(SOEXT)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
