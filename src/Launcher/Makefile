include ../common.mak
include ../commonp.mak

PROGRAM=Launcher
PRC=$(VFS)/app_install/$(PROGRAM).prc

ifeq ($(OSNAME),Emscripten)

DLIB=
OBJS=

else

SOURCE=Launcher.c editstr.c editbmp.c editform.c editbin.c editsurf.c
CUSTOMPFLAGS=-I$(LIBPUMPKIN)/tos
OBJS=$(SOURCE:%.c=%.o)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

endif

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

res: resource.rcp
	$(PILRC) resource.rcp
	touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) *.bin res
