ifeq ($(RAYLIB),)
$(error Missing RAYLIB parameter (RAYLIB must point to your raylib root directory))
endif

PROGRAM=libray
SOURCE=libray.c
CUSTOMFLAGS=-I$(RAYLIB)/build/raylib/include
LIBS=-L$(RAYLIB)/build/raylib -lraylib

include ../common.mak
include ../lib.mak
