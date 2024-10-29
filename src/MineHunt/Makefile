include ../common.mak
include ../commonp.mak

PROGRAM=MineHunt
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=MineApp.o
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c mine -n MineHunt *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)
	strip $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(STUBS).o
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS).o

$(STUBS).o: $(STUBS).c
	$(CC) $(CFLAGS) -c $(STUBS).c

$(STUBS).c: $(OBJS)
	$(NM) $(OBJS) > $(STUBS).txt
	$(LIBPUMPKIN)/gen_stubs.sh > $(STUBS).c

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res $(STUBS).c $(STUBS).o $(STUBS).txt
