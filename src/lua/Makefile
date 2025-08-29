include ../common.mak
include ../commonp.mak

PROGRAM=libscriptlua
SOURCE=lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c \
       lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c \
       ltm.c lundump.c lvm.c lzio.c \
       lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c \
       lmathlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c loadlib.c linit.c \
       script_lua.c
CUSTOMFLAGS=-DLUA_COMPAT_5_2 -DLUA_INT_TYPE=1 -DLUA_FLOAT_TYPE=2 -DLUA_USE_POSIX

ifeq ($(OSNAME),Emscripten)
STATIC=2
endif

include ../lib.mak
