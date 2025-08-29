include ../common.mak

ifeq ($(OSNAME),GNU/Linux)
SOURCE=oshell.c
endif
ifeq ($(OSNAME),Msys)
SOURCE=wshell.c
endif

PROGRAM=liboshell
LIBS=-lutil

include ../lib.mak
