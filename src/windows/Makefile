include ../common.mak
include ../commonp.mak

RESAMPLE=$(SRC)/libresample
OBJS=main.o drop.o midi.o $(RESAMPLE)/resample.o $(RESAMPLE)/resamplesubs.o $(RESAMPLE)/filterkit.o
RES=icon.res version.res
PROGRAM=$(ROOT)/pumpkin.exe
CUSTOMPFLAGS=-I$(SRC)/libresample

$(PROGRAM): $(OBJS) $(RES)
	g++ -mwindows -o $(PROGRAM) $(OBJS) $(RES) -L$(BIN) -lpit -lpumpkin -lgdi32 -lole32 -luuid -lwinmm

drop.o: drop.cpp
	g++ $(CFLAGS) -c -o drop.o drop.cpp

icon.res: icon.rc
	windres icon.rc -O coff -o icon.res

version.res: version.rc
	windres version.rc -O coff -o version.res

clean:
	rm -f $(OBJS) $(RES) $(PROGRAM)
