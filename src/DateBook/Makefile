include ../common.mak
include ../commonp.mak

PROGRAM=DateBook
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=DateAgenda.o DateAlarm.o Datebook.o DateDay.o DateDB.o DateDisplay.o DateGlobals.o DateMonth.o DatePref.o DateTransfer.o DateWeek.o ToDoDB.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c date -n "Date Book" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) $(DLIB)
