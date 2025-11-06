include ../common.mak

PROGRAM=libgpio

ifneq ($(RPI),)
CUSTOMFLAGS=-DBCM2835 -I$(SRC)/libbcm2835
LIBS=-lbcm2835
RPI_SOURCE=gpiobcm.c
endif

ifeq ($(OSNAME),Kernel)
STATIC=1
SOURCE=libgpio.c gpiomem.c gpiosys.c $(RPI_SOURCE)
else ifeq ($(OSNAME),GNU/Linux)
SOURCE=libgpio.c gpiomem.c gpiosys.c $(RPI_SOURCE)
else
SOURCE=libgpio.c gpiomem.c
endif

include ../lib.mak
