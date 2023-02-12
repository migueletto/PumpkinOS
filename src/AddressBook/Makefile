include ../common.mak
include ../commonp.mak

PROGRAM=AddressBook
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=AddrCustom.o AddrDetails.o AddrDialList.o AddrEdit.o AddressAutoFill.o Address.o AddressDB.o AddressLookup.o AddressTransfer.o AddrList.o AddrNote.o AddrPrefs.o AddrTools.o AddrView.o SysDebug.o

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c addr -n "Address Book" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) $(DLIB)
