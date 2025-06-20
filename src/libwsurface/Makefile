include ../common.mak

OBJS=libwsurface.o

CUSTOMFLAGS=-I$(SRC)/font

ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libwsurface.a
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libwsurface$(SOEXT)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

endif

clean:
	rm -f $(PROGRAM) *.o
