include ../common.mak

PROGRAM=libbcm2835

ifeq ($(RPI),1)
CUSTOMFLAGS=-I$(SRC)/libbcm2835
LIBS=-lbcm2835
SOURCE=libspi.c spi_bcm.c
else
SOURCE=libspi.c spi.c
endif

ifeq ($(OSNAME),Kernel)
STATIC=1
endif

include ../lib.mak
