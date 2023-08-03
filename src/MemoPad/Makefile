include ../common.mak
include ../commonp.mak

PROGRAM=MemoPad
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=MemoDB.o MemoMain.o MemoRsc.o MemoTransfer.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c memo -n "Memo Pad" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(OSLDEFS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) $(DLIB)
