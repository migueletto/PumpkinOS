include ../common.mak

CUSTOMFLAGS=-I$(SRC)/font -I$(SRC)/libfatfs -ffreestanding

SOURCE=main.c sig.c sys.c malloc.o strtol.c strtoul.c ctype.c bsearch.c printf.c string.c ptr.c debug.c builtin.c list.c sock.c io.c loadfile.c util.c bytes.c ts.c yuv.c graphic.c vfont.c surface.c time.c timeutc.c media.c xalloc.c endianness.c gps.c vfs.c vfslocal.c filter.c telnet.c ctelnet.c login.c pterm.c bmp.c findargs.c rgb.c average.c

ifeq ($(OSNAME),Emscripten)

PROGRAM=$(BIN)/libpit.a
OBJS=$(SOURCE:%.c=%.wasm) script.wasm threadptr.wasm mutex.wasm httpc.wasm httpd.wasm template.wasm
$(PROGRAM): $(OBJS)
	$(EM_AR) -cru $(PROGRAM) $(OBJS)

else ifeq ($(OSNAME),Kernel)

PROGRAM=$(BIN)/libpit.a
OBJS=$(SOURCE:%.c=%.o) threadnull.o mutexnull.o dlm.o
$(PROGRAM): $(OBJS)
	$(AR) -cr $(PROGRAM) $(OBJS)

else

PROGRAM=$(BIN)/libpit$(SOEXT)
OBJS=$(SOURCE:%.c=%.o) script.o threadudp.o mutex.o httpc.o httpd.o template.o
$(PROGRAM): $(OBJS)
ifeq ($(OSNAME),Android)
	$(CC) -shared -o $(PROGRAM) $(OBJS) $(EXTLIBS) $(OSLDEFS) -Wl,-soname,libpit$(SOEXT) -lm
else
	$(CC) -shared -o $(PROGRAM) $(OBJS) -lpthread $(EXTLIBS) -lm
endif

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
