include ../common.mak
include ../commonp.mak

PROGRAM=JsSyntax
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=jssyntax.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t plgi -c JsSc -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin
