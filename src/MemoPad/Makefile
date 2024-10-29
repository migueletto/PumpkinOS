include ../common.mak
include ../commonp.mak

PROGRAM=MemoPad
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=MemoDB.o MemoMain.o MemoRsc.o MemoTransfer.o
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c memo -n "Memo Pad" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)
	strip $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(STUBS).o
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(STUBS).o

$(STUBS).o: $(STUBS).c
	$(CC) $(CFLAGS) -c $(STUBS).c

$(STUBS).c: $(OBJS)
	$(NM) $(OBJS) | sort | uniq > $(STUBS).txt
	$(LIBPUMPKIN)/gen_stubs.sh > $(STUBS).c

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) $(DLIB) $(STUBS).c $(STUBS).o $(STUBS).txt
