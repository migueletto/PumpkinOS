include ../common.mak
include ../commonp.mak

PROGRAM=MineHunt
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=MineApp.o
CUSTOMPFLAGS=-DPUMPKIN_APP

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c mine -n MineHunt *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS)

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) *.bin res
