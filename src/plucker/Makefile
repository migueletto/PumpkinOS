include ../common.mak
include ../commonp.mak

PROGRAM=Plucker
PRC=$(VFS)/app_install/$(PROGRAM).prc
ZLIB_OBJS=zlib/inflate.o zlib/zutil.o zlib/adler32.o zlib/infblock.o zlib/infutil.o zlib/inftrees.o zlib/crc32.o zlib/infcodes.o zlib/inffast.o
OBJS=viewer.o os.o screen.o metadocument.o doclist.o control.o bookmarkform.o resultform.o emailform.o libraryform.o timeout.o util.o externalform.o mainform.o statusform.o prefsform.o prefsgeneral.o prefsbrowsing.o prefslibrary.o prefsautoscroll.o prefshardcopy.o prefsbutton.o prefscontrol.o prefsgesture.o loadbar.o ramfile.o genericfile.o vfsfile.o cache.o detailsform.o searchform.o categoryform.o fontform.o bookmark.o session.o document.o image.o history.o search8.o search.o prefsdata.o anchor.o paragraph.o uncompress.o keyboard.o keyboardform.o list.o link.o renamedocform.o hardcopyform.o font.o table.o fullscreenform.o hires.o rotate.o rotatebitmap.o xlit.o DIA.o resize.o $(ZLIB_OBJS)
CUSTOMPFLAGS=-I. -I./zlib -DNEWGCC -DERROR_CHECK_LEVEL=ERROR_CHECK_FULL -DHAVE_CONFIG_H

all: $(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Plkr -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: viewer.rcp
	$(PILRC) -I . -I ./fonts -I ./icons -L en viewer.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res PyPlucker/*.pyc PyPlucker/helper/*.pyc
