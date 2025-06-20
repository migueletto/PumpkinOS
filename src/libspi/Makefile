include ../common.mak

OBJS=libspi.o spi.o

ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libspi.a
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libspi$(SOEXT)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) -lpit

endif

clean:
	rm -f $(PROGRAM) *.o
