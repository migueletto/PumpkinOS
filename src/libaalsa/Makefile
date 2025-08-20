include ../common.mak

SOURCE=libaalsa.c
LIBSDL=-lasound

PROGRAM=$(BIN)/libaalsa$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBALSA)

clean:
	rm -f $(PROGRAM) $(OBJS)
