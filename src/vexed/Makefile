include ../common.mak
include ../commonp.mak

PROGRAM=Vexed
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=vexed.o gfx.o game.o globals.o sound.o system.o intro.o pack.o levelpackform.o ini.o selectlevelform.o congrats.o webbrowsermanager.o
CUSTOMPFLAGS=-DHIRES -I./vexpdb

VPOBJS=vexpdb/main.o vexpdb/ini.o
VEXPDB=vexpdb/vexpdb

LEVELS=levelpacks/Childrens\ Pack.pdb \
       levelpacks/Classic\ Levels.pdb \
       levelpacks/Classic\ II\ Levels.pdb \
       levelpacks/Confusion\ Pack.pdb \
       levelpacks/Impossible\ Pack.pdb \
       levelpacks/Panic\ Pack.pdb \
       levelpacks/Twister\ Levels.pdb \
       levelpacks/Variety\ Pack.pdb \
       levelpacks/Variety\ II\ Pack.pdb \
       levelpacks/Variety\ 3\ Pack.pdb \
       levelpacks/Variety\ 4\ Pack.pdb \
       levelpacks/Variety\ 5\ Pack.pdb \
       levelpacks/Variety\ 6\ Pack.pdb \
       levelpacks/Variety\ 7\ Pack.pdb \
       levelpacks/Variety\ 8\ Pack.pdb \
       levelpacks/Variety\ 9\ Pack.pdb \
       levelpacks/Variety\ 10\ Pack.pdb \
       levelpacks/Variety\ 11\ Pack.pdb \
       levelpacks/Variety\ 12\ Pack.pdb

all: $(PROGRAM).prc levels

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c Vexd -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: vexed.rcp
	$(PILRC) vexed.rcp; touch res

levels: $(VEXPDB) $(LEVELS)
	touch levels

$(VEXPDB): $(VPOBJS)
	$(CC) -o $(VEXPDB) $(VPOBJS)

%.pdb : %.ini
	$(VEXPDB) "$<"

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res levels $(VEXPDB) $(VPOBJS) $(LEVELS)
