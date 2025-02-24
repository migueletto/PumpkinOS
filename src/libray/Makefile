include ../common.mak

ifeq ($(RAYLIB),)
$(error Missing RAYLIB parameter (RAYLIB must point to your raylib/src directory))
endif

PROGRAM=$(BIN)/libray$(SOEXT)
CUSTOMFLAGS=-I$(RAYLIB)
OBJS=libray.o

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -L$(RAYLIB) -lraylib -lpit

clean:
	rm -f $(PROGRAM) $(OBJS)
