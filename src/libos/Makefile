include ../common.mak
include ../commonp.mak

PROGRAM=libos
SOURCE=libos.c

ifeq ($(OSNAME),Kernel)
STATIC=1
else ifeq ($(OSNAME),Emscripten)
STATIC=2
else
LIBS=-lpumpkin
endif

include ../lib.mak
