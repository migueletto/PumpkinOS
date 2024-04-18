include ../common.mak

PROGRAM=$(BIN)/liblsdl2$(SOEXT)

OBJS=liblsdl2.o

ifeq ($(OSNAME),Msys)
#LIBSDL=-lSDL2 -lversion -limm32 -lgdi32 -lsetupapi -lwinmm -lole32 -loleaut32 -lSDL2_mixer
LIBSDL=-lSDL2 -lversion -limm32 -lgdi32 -lsetupapi -lwinmm -lole32 -loleaut32
endif

ifeq ($(OSNAME),GNU/Linux)
#LIBSDL=-lSDL2 -lSDL2_mixer
LIBSDL=-lSDL2
endif

ifeq ($(OSNAME),serenity)
#LIBSDL=-lSDL2 -lSDL2_mixer
LIBSDL=-lSDL2
endif

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBSDL)

clean:
	rm -f $(PROGRAM) $(OBJS)
