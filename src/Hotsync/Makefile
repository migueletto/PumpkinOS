include ../common.mak
include ../commonp.mak

PROGRAM=Hotsync
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=hotsync.o userland.o \
     cmp.o debug.o dlp.o \
     inet.o md5.o net.o padp.o pi-buffer.o pi-file.o serial.o \
     slp.o socket.o sys.o syspkt.o threadsafe.o srmserial.o unixserial.o utils.o

CUSTOMPFLAGS=-I. -DHAVE_CONFIG_H

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Hots -n Hotsync *.bin

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: resource.rcp hotsync.bmp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(POBJS) $(OBJS) *.bin res
