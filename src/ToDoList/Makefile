include ../common.mak
include ../commonp.mak

PROGRAM=ToDoList
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=ToDo.o ToDoDB.o ToDoRsc.o ToDoTransfer.o
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c todo -n "To Do List" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)
	strip $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(LIBAPP)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(LIBAPP)

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) $(DLIB)
