include ../common.mak
include ../commonp.mak

PROGRAM=UTF8
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=utf8.o

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t 0005 -c lang -n UTF8 *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin
