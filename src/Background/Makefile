include ../common.mak
include ../commonp.mak

PROGRAM=Background
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: res
	$(PRCDUILD) -f $(PROGRAM).prc -t Bckg -c BOOT -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

res: resource.rcp bg1.bmp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res
