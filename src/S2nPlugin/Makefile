include ../common.mak
include ../commonp.mak

ifeq ($(S2N),)
$(error S2N variable is not set, use: "make S2N=/some/path/s2n-tls")
endif

PROGRAM=S2nPlugin
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=S2nPlugin.o
CUSTOMPFLAGS=-I$(S2N)

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t plgi -c s2nC -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(S2N)/lib/libs2n.a
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(S2N)/lib/libs2n.a -L$(BIN) -lcrypto -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin
