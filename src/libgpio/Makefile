include ../common.mak

PROGRAM=$(BIN)/libgpio$(SOEXT)

ifeq ($(RPI),yes)
CUSTOMFLAGS=-DBCM2835 -I$(SRC)/libbcm2835
OBJS=libgpio.o gpiomem.o gpiosys.o gpiobcm.o
LIBS=-lbcm2835
else ifeq ($(OS),Linux)
OBJS=libgpio.o gpiomem.o gpiosys.o
else
OBJS=libgpio.o gpiomem.o
endif

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(LIB) $(LIBS) -lpit

clean:
	rm -f $(PROGRAM) *.o
