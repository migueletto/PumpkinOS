include ../common.mak
include ../commonp.mak

PROGRAM=WebServer
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=WebServer.o

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c WebS -n WebServer *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: resource.rcp webserver.lua
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res
