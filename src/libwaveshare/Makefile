include ../common.mak

PROGRAM=$(BIN)/libwaveshare$(SOEXT)

OBJS=libwaveshare.o ili9486.o ads7846.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit -lm

clean:
	rm -f $(PROGRAM) *.o
