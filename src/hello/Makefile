include ../common.mak
include ../commonp.mak

PROGRAM=Hello
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PRC): $(PROGRAM).prc
	cp $(PROGRAM).prc $(PRC)

$(PROGRAM).prc: res
	$(PRCDUILD) -f $(PROGRAM).prc -t ctos -c Helo -n $(PROGRAM) *.bin

res: resource.rcp hello.ttp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res

