ifeq ($(ROOT),)
$(error Missing ROOT parameter)
endif

BIN=$(ROOT)/bin
SRC=$(ROOT)/src
VFS=$(ROOT)/vfs
LIBPIT=$(SRC)/libpit

SYSNAME=pit
VERSION=1.0

MACHINE := $(shell uname -m)
OSNAME  := $(shell uname -o)

ifeq ($(BITS),32)
SYS_SIZE=1
else ifeq ($(BITS),64)
SYS_SIZE=2
else
$(error Missing BITS parameter (must be either 32 or 64))
endif

ifeq ($(findstring arm,$(MACHINE)),arm)
SYS_CPU=1
SYS_ENDIAN=LITTLE_ENDIAN
else ifeq ($(MACHINE),x86_64)
SYS_ENDIAN=LITTLE_ENDIAN
SYS_CPU=2
else ifeq ($(MACHINE),x86_32)
SYS_ENDIAN=LITTLE_ENDIAN
SYS_CPU=2
else ifeq ($(MACHINE),i686)
SYS_ENDIAN=LITTLE_ENDIAN
SYS_CPU=2
else ifeq ($(MACHINE),i386)
SYS_ENDIAN=LITTLE_ENDIAN
SYS_CPU=2
else
$(error Unknown CPU $(MACHINE))
endif

ifeq ($(OSNAME),GNU/Linux)
SYS_OS=1
EXTLIBS=-lrt -ldl
SOEXT=.so
LUAPLAT=linux
OS=Linux
OSDEFS=-m$(BITS) -DLINUX -DSOEXT=\"$(SOEXT)\"
else ifeq ($(OSNAME),Msys)
SYS_OS=2
EXTLIBS=-lwsock32 -lws2_32
SOEXT=.dll
LUAPLAT=mingw
OS=Windows
OSDEFS=-m$(BITS) -DWINDOWS -DWINDOWS$(BITS) -DSOEXT=\"$(SOEXT)\"
else
$(error Unknown OS $(OSNAME))
endif

CC=gcc

SYSDEFS=-DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) -DSYS_ENDIAN=$(SYS_ENDIAN)
CFLAGS=-Wall -Wno-unknown-pragmas -fsigned-char -g -fPIC -I$(LIBPIT) -DSYSTEM_NAME=\"$(SYSNAME)\" -DSYSTEM_VERSION=\"$(VERSION)\" -DSYSTEM_OS=\"$(OS)\" $(CUSTOMFLAGS) $(SYSDEFS) $(OSDEFS)
