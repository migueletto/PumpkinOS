include ../common.mak

#CUSTOMFLAGS=-DSDL_OPENGL

SOURCE=liblsdl2.c

ifeq ($(OSNAME),Emscripten)

CUSTOMFLAGS=-s USE_SDL=2

PROGRAM=$(BIN)/liblsdl2.a
OBJS=$(SOURCE:%.c=%.wasm)
$(PROGRAM): $(OBJS)
	$(EM_AR) -cru $(PROGRAM) $(OBJS)

else

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

PROGRAM=$(BIN)/liblsdl2$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit $(LIBSDL)

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
