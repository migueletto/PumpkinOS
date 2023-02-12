include ../common.mak
include ../commonp.mak

PROGRAM=ToDoList
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=ToDo.o ToDoDB.o ToDoRsc.o ToDoTransfer.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c todo -n "To Do List" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) $(DLIB)
