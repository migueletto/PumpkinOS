include ../common.mak

ifeq ($(OSNAME),Msys)
ifeq ($(UNICORN),)
$(error Missing UNICORN parameter (UNICORN must point to your Unicorn root directory))
else
UNICORN_INC=-I$(UNICORN)/include
UNICORN_LIB=-L$(UNICORN)/bin
endif
endif

SOURCE=ucarm.c
APPNAME=UnicornArm
APPID=ucAr
APPTYPE=plgi

CUSTOMPFLAGS=-I$(SRC)/libpumpkin/emulation -I$(SRC)/libpumpkin/emulation/arm $(UNICORN_INC)
EXTRALIBS=$(UNICORN_LIB) -lunicorn

include ../commonp.mak
include ../app.mak
