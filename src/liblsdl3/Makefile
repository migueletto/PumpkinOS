ifeq ($(SDL3),)
$(error Missing SDL3 parameter (SDL3 must point to your SDL3 root directory))
endif

include ../common.mak

PROGRAM=liblsdl3
SOURCE=liblsdl3.c

#MIXER_LIBS=-lSDL3_mixer
#MIXER_FLAGS=-DSDL_MIXER
#OPENGL_FLAGS=-DSDL_OPENGL

ifeq ($(OSNAME),Kernel)
STATIC=1
else ifeq ($(OSNAME),Emscripten)
STATIC=2
CUSTOMFLAGS=-s USE_SDL=3
else ifeq ($(OSNAME),Msys)
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS)
LIBS=-lSDL3 -lversion -limm32 -lgdi32 -lsetupapi -lwinmm -lole32 -loleaut32 $(MIXER_LIBS)
else
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS) -I$(SDL3)/include
LIBS=-L$(SDL3)/build -lSDL3 $(MIXER_LIBS)
endif

include ../lib.mak
