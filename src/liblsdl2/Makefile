# additional package(s) to install (example for Ubuntu):
# sudo apt install libsdl2-dev

# for Msys on Windows, download the mingw SDL2 package and unpack it. Example:
# https://github.com/libsdl-org/SDL/releases/download/release-2.32.10/SDL2-devel-2.32.10-mingw.tar.gz
# then point SDL2 variable to the unpacked directory:
# make SDL2=/some/dir/SDL2-2.32.10

include ../common.mak

PROGRAM=liblsdl2
SOURCE=liblsdl2.c

#MIXER_LIBS=-lSDL2_mixer
#MIXER_FLAGS=-DSDL_MIXER
OPENGL_FLAGS=-DSDL_OPENGL
OPENGL_LIBS=-lGL

ifeq ($(SDL2),)
# use SDL2 from system packages
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS)
LIBS=-lSDL2 $(MIXER_LIBS) $(OPENGL_LIBS)

else
# use SDL2 from custom installation
$(info Using custom SDL2 at $(SDL2))

ifeq ($(OSNAME),Msys)
# on Windows with SDL2 mingw package in $(SDL2)
CUSTOMFLAGS=$(MIXER_FLAGS) -I$(SDL2)/x86_64-w64-mingw32/include
LIBS=-L$(SDL2)/x86_64-w64-mingw32/bin -lSDL2 $(MIXER_LIBS)
else
# on Linux with SDL2 compiled from source in $(SDL2)
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS) -I$(SDL2)/include
LIBS=-L$(SDL2)/build -lSDL2 $(MIXER_LIBS)
endif

endif

include ../lib.mak
