include ../common.mak
include ../commonp.mak

PROGRAM=Launcher
RCP=resource.rcp
APPNAME=Launcher
APPID=Lnch

ifeq ($(OSNAME),Kernel)
STATIC=1
else ifeq ($(OSNAME),Emscripten)
STATIC=2
else
SOURCE=Launcher.c editstr.c editbmp.c editform.c editbin.c editsurf.c
CUSTOMPFLAGS=-I$(LIBPUMPKIN)/tos
endif

include ../app.mak
