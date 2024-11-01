include ../common.mak
include ../commonp.mak

PROGRAM=Hello
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: res.flag
	$(PRCDUILD) -f $(PROGRAM).prc -t ctos -c Helo -n $(PROGRAM) *.bin

res.flag: resource.rcp hello.ttp
	$(PILRC) resource.rcp; touch res.flag

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res.flag

