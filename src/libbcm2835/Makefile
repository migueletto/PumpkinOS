include ../common.mak

PROGRAM=libbcm2835
SOURCE=bcm2835.c

ifeq ($(OSNAME),Kernel)
STATIC=1
CUSTOMFLAGS=-DBAREMETAL
endif

include ../lib.mak
