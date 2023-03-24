include ../common.mak
include ../commonp.mak

PROGRAM=Navegador
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=main.o navegador.o map.o loadmap.o gps.o log.o misc.o serial.o \
     network.o format.o sat.o trip.o ddb.o list.o compass.o \
     fill.o sound.o file.o symbol.o object.o point.o route.o tracks.o \
     datum.o error.o thin.o ndebug.o scroll.o http.o gpc.o label.o \
     mapcolor.o trig.o font.o gpslib.o gad.o protocol.o astro.o \
     buttons.o display.o

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Navg -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit -lm

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res
