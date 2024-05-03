include ../common.mak
include ../commonp.mak

PROGRAM=SpaceTrader
PRC=$(VFS)/app_install/$(PROGRAM).prc
OBJS=AppHandleEvent.o Bank.o BuyEquipEvent.o BuyShipEvent.o Cargo.o CmdrStatusEvent.o Draw.o Encounter.o Field.o Fuel.o Global.o Math.o Merchant.o Money.o OtherEvent.o QuestEvent.o SellEquipEvent.o ShipEvent.o ShipPrice.o ShiptypeInfoEvent.o Shipyard.o Skill.o SpecialEvent.o SystemInfoEvent.o Traveler.o WarpFormEvent.o
CUSTOMPFLAGS=-DBPP8

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c STra -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(PROGRAM).dlib
	cp $(PROGRAM).dlib $(DLIB)

$(PROGRAM).dlib: $(OBJS)
	$(CC) -shared -o $(PROGRAM).dlib $(OBJS) -L$(BIN) -lpumpkin -lpit

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc $(PROGRAM).dlib $(PROGRAM) $(OBJS) *.bin res
