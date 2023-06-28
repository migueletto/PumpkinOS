ifeq ($(ROOT),)
$(error Missing ROOT parameter)
endif

BIN=$(ROOT)/bin
SRC=$(ROOT)/src
VFS=$(ROOT)/vfs
TOOLS=$(ROOT)/tools
LIBPIT=$(SRC)/libpit

SYSNAME=pit
VERSION=1.0

ifeq ($(BITS),32)
SYS_SIZE=1
else ifeq ($(BITS),64)
SYS_SIZE=2
else
$(error Missing BITS parameter (must be either 32 or 64))
endif

MACHINE := $(shell uname -m)

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
else ifeq ($(MACHINE),ppc64le)
SYS_ENDIAN=LITTLE_ENDIAN
SYS_CPU=3
# ppc64 would be BIG_ENDIAN 4?
else
$(error Unknown CPU $(MACHINE))
endif

HOSTCC=gcc

ifeq ($(OSNAME),)
OSNAME := $(shell uname -o)
endif

ifeq ($(OSNAME),GNU/Linux)
SYS_OS=1
EXTLIBS=-lrt -ldl
SOEXT=.so
LUAPLAT=linux
OS=Linux
OSDEFS=-m$(BITS) -DLINUX -DSOEXT=\"$(SOEXT)\"
CC=gcc
else ifeq ($(OSNAME),Msys)
SYS_OS=2
EXTLIBS=-lwsock32 -lws2_32
SOEXT=.dll
LUAPLAT=mingw
OS=Windows
OSDEFS=-m$(BITS) -DWINDOWS -DWINDOWS$(BITS) -DSOEXT=\"$(SOEXT)\"
CC=gcc
else ifeq ($(OSNAME),Serenity)
ifeq ($(SERENITY),)
$(error Missing SERENITY parameter (must point to SerenityOS home directory))
endif
SYS_OS=3
EXTLIBS=
SOEXT=.so
LUAPLAT=linux
OS=Serenity
OSDEFS=-m$(BITS) -DSERENITY -DSOEXT=\"$(SOEXT)\" -I$(SERENITY)/Build/x86_64/Root/usr/include -D_GNU_SOURCE
CC=$(SERENITY)/Toolchain/Local/x86_64/bin/x86_64-pc-serenity-gcc
else
$(error Unknown OS $(OSNAME))
endif

SYSDEFS=-DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) -DSYS_ENDIAN=$(SYS_ENDIAN)
CFLAGS=-Wall -Wno-unknown-pragmas -fsigned-char -Wno-multichar -g -fPIC $(OSDEFS) -I$(LIBPIT) -DSYSTEM_NAME=\"$(SYSNAME)\" -DSYSTEM_VERSION=\"$(VERSION)\" -DSYSTEM_OS=\"$(OS)\" $(CUSTOMFLAGS) $(SYSDEFS)
