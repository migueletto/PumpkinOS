include ../common.mak
include ../commonp.mak

ifeq ($(S2N),)
$(error S2N variable is not set, use: "make S2N=/some/path")
endif

PROGRAM=S2nHashPlugin
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=S2nHashPlugin.o
CUSTOMPFLAGS=-I$(S2N)

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t plgi -c s2nH -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -L$(S2N)/lib -ls2n -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin
