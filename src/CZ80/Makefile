include ../common.mak
include ../commonp.mak

PROGRAM=CZ80
CREATOR=Cz80
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=main.o
CUSTOMPFLAGS=-I$(SRC)/libemulation

$(PROGRAM).prc: $(PROGRAM).dlib res
	cp $(PROGRAM).dlib $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c $(CREATOR) -n $(PROGRAM) *.bin
	rm -f $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lemulation -lpit

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(OBJS) *.bin res
