include ../common.mak

CUSTOMFLAGS=-I$(SRC)/font

SOURCE=main.c sig.c sys.c threadudp.c mutex.c ctype.c bsearch.c printf.c string.c ptr.c debug.c script.c builtin.c list.c sock.c io.c loadfile.c util.c bytes.c ts.c yuv.c graphic.c vfont.c surface.c time.c timeutc.c media.c xalloc.c endianness.c gps.c vfs.c vfslocal.c filter.c telnet.c ctelnet.c login.c pterm.c bmp.c httpc.c httpd.c template.c findargs.c rgb.c

ifeq ($(OSNAME),Emscripten)

PROGRAM=$(BIN)/libpit$(SOEXT)
OBJS=$(SOURCE:%.c=%.wasm)
$(PROGRAM): $(OBJS)
	$(EM_CC) -o $(PROGRAM) -s SIDE_MODULE $(OBJS)

else

PROGRAM=$(BIN)/libpit$(SOEXT)
OBJS=$(SOURCE:%.c=%.o)
$(PROGRAM): $(OBJS)
ifeq ($(OSNAME),Android)
	$(CC) -shared -o $(PROGRAM) $(OBJS) $(EXTLIBS) $(OSLDEFS) -Wl,-soname,libpit$(SOEXT) -lm
else
	$(CC) -shared -o $(PROGRAM) $(OBJS) -lpthread $(EXTLIBS) -lm
endif

endif

clean:
	rm -f $(PROGRAM) $(OBJS)
