include ../common.mak
include ../commonp.mak

PROGRAM=Vexed
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=vexed.o gfx.o game.o globals.o sound.o system.o intro.o pack.o levelpackform.o ini.o selectlevelform.o congrats.o webbrowsermanager.o
CUSTOMPFLAGS=-DHIRES -I./vexpdb

VPOBJS=vexpdb/main.o vexpdb/ini.o
VEXPDB=vexpdb/vexpdb

LEVELS=levelpacks/ChildrensPack.pdb \
       levelpacks/ClassicLevels.pdb \
       levelpacks/ClassicIILevels.pdb \
       levelpacks/ConfusionPack.pdb \
       levelpacks/ImpossiblePack.pdb \
       levelpacks/PanicPack.pdb \
       levelpacks/TwisterLevels.pdb \
       levelpacks/VarietyPack.pdb \
       levelpacks/VarietyIIPack.pdb \
       levelpacks/Variety3Pack.pdb \
       levelpacks/Variety4Pack.pdb \
       levelpacks/Variety5Pack.pdb \
       levelpacks/Variety6Pack.pdb \
       levelpacks/Variety7Pack.pdb \
       levelpacks/Variety8Pack.pdb \
       levelpacks/Variety9Pack.pdb \
       levelpacks/Variety10Pack.pdb \
       levelpacks/Variety11Pack.pdb \
       levelpacks/Variety12Pack.pdb

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
