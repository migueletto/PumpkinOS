include ../common.mak
include ../commonp.mak

PROGRAM=AddressBook
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=AddrCustom.o AddrDetails.o AddrDialList.o AddrEdit.o AddressAutoFill.o Address.o AddressDB.o AddressLookup.o AddressTransfer.o AddrList.o AddrNote.o AddrPrefs.o AddrTools.o AddrView.o
CUSTOMPFLAGS=-DPUMPKIN_APP -fvisibility=hidden

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c addr -n "Address Book" resources/*.dat $(DLIB)
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)
	strip $(DLIB)

$(PROGRAM).dlib: $(OBJS) $(LIBAPP)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) $(LIBAPP)

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) $(DLIB)
