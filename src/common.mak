ifeq ($(ROOT),)
ROOT=../..
endif

BIN=$(ROOT)/bin
SRC=$(ROOT)/src
VFS=$(ROOT)/vfs
TOOLS=$(ROOT)/tools
LIBPIT=$(SRC)/libpit
LIBPUMPKIN=$(SRC)/libpumpkin
STUBS=syscalls_stubs

SYSNAME=pit
VERSION=1.0

ifeq ($(MACHINE),)
MACHINE := $(shell uname -m)
endif

ifeq ($(findstring arm,$(MACHINE)),arm)
SYS_CPU=1
SYS_ENDIAN=1
ifeq ($(BITS),)
BITS=32
endif
else ifeq ($(MACHINE),aarch64)
SYS_CPU=1
SYS_ENDIAN=1
ifeq ($(BITS),)
BITS=32
endif
MBITS=
else ifeq ($(MACHINE),x86_64)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=64
endif
MBITS=
else ifeq ($(MACHINE),x86_32)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
MBITS=-m$(BITS)
endif
else ifeq ($(MACHINE),i686)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
endif
MBITS=-m$(BITS)
else ifeq ($(MACHINE),i386)
SYS_ENDIAN=1
SYS_CPU=2
ifeq ($(BITS),)
BITS=32
endif
MBITS=-m$(BITS)
else ifeq ($(MACHINE),ppc64le)
SYS_ENDIAN=1
SYS_CPU=3
# ppc64 would be BIG_ENDIAN 4?
else
$(error Unknown CPU $(MACHINE))
endif

ifeq ($(BITS),32)
SYS_SIZE=1
else ifeq ($(BITS),64)
SYS_SIZE=2
else
$(error Missing BITS parameter (must be either 32 or 64))
endif

AR=ar
NM=nm -j -u

ifneq (,$(wildcard /boot/firmware/config.txt))
RPI=4
RPI_DEFS=-DRPI=$(RPI)
endif

ifeq ($(OSNAME),)
OSNAME := $(shell uname -o)
endif

REAL_OSNAME := $(shell uname -o)

ifeq ($(OSNAME),GNU/Linux)
SYS_OS=1
EXTLIBS=-lrt -ldl
SOEXT=.so
LUAPLAT=linux
OS=Linux
OSDEFS=$(MBITS) -DLINUX $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\"
CC=gcc
FC=gfortran

else ifeq ($(OSNAME),Msys)
SYS_OS=2
EXTLIBS=-lwsock32 -lws2_32
SOEXT=.dll
LUAPLAT=mingw
OS=Windows
OSDEFS=$(MBITS) -DWINDOWS -DWINDOWS$(BITS) -DSOEXT=\"$(SOEXT)\"
ifeq ($(REAL_OSNAME),GNU/Linux)
CC=x86_64-w64-mingw32-gcc-win32
CPP=x86_64-w64-mingw32-g++-win32
WINDRES=x86_64-w64-mingw32-windres
else
CC=gcc
CPP=g++
WINDRES=windres
endif

else ifeq ($(OSNAME),Serenity)
ifeq ($(SERENITY),)
$(error Missing SERENITY parameter (must point to SerenityOS home directory))
endif
SYS_OS=3
EXTLIBS=
SOEXT=.so
LUAPLAT=linux
OS=Serenity
OSDEFS=$(MBITS) -DSERENITY -DSOEXT=\"$(SOEXT)\" -I$(SERENITY)/Build/x86_64/Root/usr/include -D_GNU_SOURCE
CC=$(SERENITY)/Toolchain/Local/x86_64/bin/x86_64-pc-serenity-gcc

else ifeq ($(OSNAME),Android)
ifeq ($(NDK),)
$(error You must define the NDK environment variable)
endif
SYS_OS=4
ifeq ($(BITS),32)
ARM_ARCH=--target=armv7-none-linux-androideabi26 -march=armv7-a -mthumb
else
ARM_ARCH=--target=aarch64-none-linux-android26
endif
OSLDEFS=$(ARM_ARCH) --gcc-toolchain=$(NDK) --sysroot=$(NDK)/sysroot -fPIC -Wl,--exclude-libs,libgcc.a -Wl,--exclude-libs,libgcc_real.a -Wl,--exclude-libs,libatomic.a -static-libstdc++ -Wl,--build-id -Wl,--fatal-warnings -Wl,--exclude-libs,libunwind.a -Wl,--no-undefined -Qunused-arguments
EXTLIBS=-llog -latomic
SOEXT=.so
LUAPLAT=linux
OS=Android
OSDEFS=$(ARM_ARCH) --gcc-toolchain=$(NDK) --sysroot=$(NDK)/sysroot -fdata-sections -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -D_FORTIFY_SOURCE=2 -DANDROID -DSOEXT=\"$(SOEXT)\"
CC=$(NDK)/bin/clang

else ifeq ($(OSNAME),Emscripten)
EMSC=$(SRC)/emscripten
BIN=$(EMSC)
VFS=$(EMSC)/vfs
SYS_OS=5
SOEXT=.wasm
LUAPLAT=linux
OS=Emscripten
OSDEFS=$(MBITS) -DEMSCRIPTEN -DSOEXT=\"$(SOEXT)\" -pthread

else ifeq ($(OSNAME),Kernel)
SYS_OS=6
SOEXT=.a
LUAPLAT=linux
OS=Linux
OSDEFS=$(MBITS) -DKERNEL $(RPI_DEFS) -DSOEXT=\"$(SOEXT)\"
PILRCDEFS=-D KERNEL
CC=gcc

else
$(error Unknown OS $(OSNAME))
endif

EM_CC=emcc
EM_AR=emar

ifeq ($(TYPE),release)
OPTIMIZATION=-O2
STRIP=strip
else
OPTIMIZATION=-g
STRIP=touch
endif

SYSDEFS=-DSYS_CPU=$(SYS_CPU) -DSYS_SIZE=$(SYS_SIZE) -DSYS_OS=$(SYS_OS) -DSYS_ENDIAN=$(SYS_ENDIAN)
CPPFLAGS=-Wall -Wno-unknown-pragmas -fsigned-char -Wno-multichar $(OPTIMIZATION) -fPIC -fno-stack-protector -I$(LIBPIT) -DSYSTEM_NAME=\"$(SYSNAME)\" -DSYSTEM_VERSION=\"$(VERSION)\" -DSYSTEM_OS=\"$(OS)\" -DAPPNAME="\"$(APPNAME)\"" -DAPPID=\'$(APPID)\' $(CUSTOMFLAGS) $(SYSDEFS) $(OSDEFS) -DLOGTRAP_SYS
CFLAGS=$(CPPFLAGS) -ffreestanding -std=c99
HOSTCFLAGS=-Wall
FFLAGS=-fPIC $(OPTIMIZATION)

ifneq ($(REAL_OSNAME),Msys)
COLOR_GREEN=\033[0;1;32m
COLOR_CYAN=\033[0;1;36m
COLOR_RED=\033[0;1;31m
COLOR_END=\033[0m
endif

ifneq ($(HOSTCC),)
%.o: %.c
	@echo Compiling $<
	@$(HOSTCC) $(HOSTCFLAGS) -c -o $@ $<
else
%.o: %.c
	@echo Compiling $<
	@$(CC) $(CFLAGS) -c -o $@ $<
endif

%.wasm : %.c
	@echo Compiling $<
	@$(EM_CC) $(CFLAGS) -c -o $@ $<

%.fo: %.f90
	@echo Compiling $<
	@$(FC) $(FFLAGS) -c -o $@ $<
