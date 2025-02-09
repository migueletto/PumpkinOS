include ../common.mak
include ../commonp.mak

CUSTOMFLAGS=-DLUA_COMPAT_5_2 -DLUA_INT_TYPE=1 -DLUA_FLOAT_TYPE=2 -DLUA_USE_POSIX

SOURCE=lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c \
       lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c \
       ltm.c lundump.c lvm.c lzio.c \
       lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c \
       lmathlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c loadlib.c linit.c \
       script_lua.c

ifeq ($(OSNAME),Emscripten)

PROGRAM=$(BIN)/libscriptlua.a
OBJS=$(SOURCE:%.c=%.wasm)
$(PROGRAM): $(OBJS)
	$(EM_AR) -cru $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libscriptlua$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
