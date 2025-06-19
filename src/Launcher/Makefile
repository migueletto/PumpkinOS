include ../common.mak
include ../commonp.mak

PROGRAM=Launcher
PRC=$(VFS)/app_install/$(PROGRAM).prc

ifeq ($(OSNAME),Emscripten)

$(PROGRAM).prc: res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

else ifeq ($(OSNAME),Kernel)

$(PROGRAM).prc: res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

else

SOURCE=Launcher.c editstr.c editbmp.c editform.c editbin.c editsurf.c
CUSTOMPFLAGS=-I$(LIBPUMPKIN)/tos
OBJS=$(SOURCE:%.c=%.o)

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

endif

res: resource.rcp
	$(PILRC) $(PILRCDEFS) resource.rcp
	touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) *.bin res
