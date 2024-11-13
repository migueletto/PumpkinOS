include ../common.mak
include ../commonp.mak

PROGRAM=Launcher
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=Launcher.o editstr.o editbmp.o editform.o editbin.o editsurf.o

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Lnch -n Launcher *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) *.bin res
