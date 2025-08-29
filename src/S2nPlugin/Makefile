ifeq ($(S2N),)
$(error Missing S2N parameter (S2N must point to your s2n root directory))
endif

PROGRAM=S2nPlugin
SOURCE=S2nPlugin.c
CUSTOMPFLAGS=-I$(S2N)
APPID=s2nC
APPTYPE=plgi
LIBS=$(S2N)/lib/libs2n.a -lcrypto -lpumpkin

include ../common.mak
include ../commonp.mak
include ../app.mak
