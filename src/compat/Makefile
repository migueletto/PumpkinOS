include ../common.mak
include ../commonp.mak

PROGRAM=CompatDB
PRC=$(VFS)/app_install/$(PROGRAM).prc

CompatDB.prc:
	@echo Building $(PROGRAM).prc
	@$(PRCBUILD) -f $(PROGRAM).prc -t regt -c psys -n $(PROGRAM) *.dat
	@echo Installing $(PROGRAM).prc
	@cp $(PROGRAM).prc $(PRC)


clean:
	@rm -f $(PROGRAM).prc
