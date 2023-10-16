include ../common.mak
include ../commonp.mak

PROGRAM=MicroUI
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=main.o microui.o

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c McUI -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res
