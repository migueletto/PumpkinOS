include ../common.mak

PROGRAM=$(BIN)/libray$(SOEXT)
CUSTOMFLAGS=-I$(RAYLIB)
OBJS=libray.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) $(RAYLIB)/libraylib.a -lpit -lGL -lX11 -lm

clean:
	rm -f $(PROGRAM) $(OBJS)
