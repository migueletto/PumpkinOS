include ../common.mak

PROGRAM=$(BIN)/libgpio$(SOEXT)

CUSTOMFLAGS=-DBCM2835 -I$(SRC)/libbcm2835

ifeq ($(OS),Linux)
OBJS=libgpio.o gpio.o gpiobcm.o
else
OBJS=libgpio.o gpio.o
endif

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lbcm2835 -lpit

clean:
	rm -f $(PROGRAM) *.o
