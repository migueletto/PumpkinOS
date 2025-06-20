include ../common.mak

ifeq ($(RPI),1)
CUSTOMFLAGS=-DBCM2835 -I$(SRC)/libbcm2835
RPI_LIBS=-lbcm2835
RPI_OBJS=gpiobcm.o
endif

ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libgpio.a
OBJS=libgpio.o gpiomem.o gpiosys.o $(RPI_OBJS)
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else ifeq ($(OSNAME),GNU/Linux)

PROGRAM=$(BIN)/libgpio$(SOEXT)
OBJS=libgpio.o gpiomem.o gpiosys.o $(RPI_OBJS)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) $(RPI_LIBS) -lpit

else

PROGRAM=$(BIN)/libgpio$(SOEXT)
OBJS=libgpio.o gpiomem.o
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

endif

clean:
	rm -f $(PROGRAM) *.o
