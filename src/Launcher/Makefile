include ../common.mak
include ../commonp.mak

PROGRAM=Launcher
SOURCE=Launcher.c editstr.c editbmp.c editform.c editbin.c editsurf.c
CUSTOMPFLAGS=-I$(LIBPUMPKIN)/tos

ifeq ($(OSNAME),Emscripten)

OBJS=$(SOURCE:%.c=%.wasm)
EMSC=$(SRC)/emscripten
VFS=$(EMSC)/vfs
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).dlib: $(OBJS)
	$(EM_CC) -o $(PROGRAM).dlib -s SIDE_MODULE $(OBJS)

else

OBJS=$(SOURCE:%.c=%.o)
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

endif

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

res: resource.rcp
	$(PILRC) resource.rcp
	touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) *.bin res
