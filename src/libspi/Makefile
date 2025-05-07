include ../common.mak

ifeq ($(RPI),1)
CUSTOMFLAGS=-I$(SRC)/libbcm2835
LIBS=-lbcm2835
OBJS=libspi.o spi_bcm.o
else
OBJS=libspi.o spi.o
endif

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
