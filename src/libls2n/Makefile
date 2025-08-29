ifeq ($(S2N),)
$(error Missing S2N parameter (S2N must point to your s2n root directory))
endif

PROGRAM=libls2n
SOURCE=libls2n.c
CUSTOMFLAGS=-I$(S2N)/api
LIBS=-L$(S2N)/lib -ls2n

include ../common.mak
include ../lib.mak
