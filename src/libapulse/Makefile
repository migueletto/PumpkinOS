include ../common.mak

SOURCE=libapulse.c
LIBPULSE=-lpulse -lpulse-simple

PROGRAM=$(BIN)/libapulse$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBPULSE)

clean:
	rm -f $(PROGRAM) $(OBJS)
