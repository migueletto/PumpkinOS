include ../common.mak

PROGRAM=$(BIN)/libpit$(SOEXT)

CUSTOMFLAGS=-I$(SRC)/font

OBJS=main.o sig.o threadudp.o mutex.o sys.o ptr.o debug.o script.o builtin.o list.o sock.o io.o loadfile.o util.o bytes.o ts.o yuv.o graphic.o vfont.o surface.o timeutc.o media.o xalloc.o endianness.o gps.o vfs.o vfslocal.o filter.o telnet.o ctelnet.o login.o pterm.o bmp.o httpc.o httpd.o template.o findargs.o rgb.o

$(PROGRAM): $(OBJS)
ifeq ($(OSNAME),Android)
	$(CC) -shared -o $(PROGRAM) $(OBJS) $(EXTLIBS) $(OSLDEFS) -Wl,-soname,libpit$(SOEXT) -lm
else
	$(CC) -shared -o $(PROGRAM) $(OBJS) -lpthread $(EXTLIBS) -lm
endif

clean:
	rm -f $(PROGRAM) $(OBJS)
