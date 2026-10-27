# additional package(s) to install (example for Ubuntu):
# sudo apt install libsdl3-dev

# for Msys on Windows, download the mingw SDL3 package and unpack it. Example:
# https://github.com/libsdl-org/SDL/releases/download/release-3.4.18/SDL3-devel-3.4.18-mingw.tar.gz
# then point SDL3 variable to the unpacked directory:
# make SDL3=/some/dir/SDL3-3.4.18

include ../common.mak

PROGRAM=liblsdl3
SOURCE=liblsdl3.c

#MIXER_LIBS=-lSDL3_mixer
#MIXER_FLAGS=-DSDL_MIXER
#OPENGL_FLAGS=-DSDL_OPENGL

ifeq ($(SDL3),)
# use SDL3 from system packages
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS)
LIBS=-lSDL3 $(MIXER_LIBS) $(OPENGL_LIBS)

else
# use SDL3 from custom installation
$(info Using custom SDL3 at $(SDL3))

ifeq ($(OSNAME),Msys)
# on Windows with SDL3 mingw package in $(SDL3)
CUSTOMFLAGS=$(MIXER_FLAGS) -I$(SDL3)/x86_64-w64-mingw32/include
LIBS=-L$(SDL3)/x86_64-w64-mingw32/bin -lSDL3 $(MIXER_LIBS)
else
# on Linux with SDL3 compiled from source in $(SDL3)
CUSTOMFLAGS=$(MIXER_FLAGS) $(OPENGL_FLAGS) -I$(SDL3)/include
LIBS=-L$(SDL3)/build -lSDL3 $(MIXER_LIBS)
endif

endif

include ../lib.mak
