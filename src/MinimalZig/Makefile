include ../common.mak
include ../commonp.mak

PROGRAM=MinimalZig

ZIG=zig
ZIGLIB=zig-out/lib/lib$(PROGRAM)$(SOEXT)

PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res
	$(PRCDUILD) -f $(PROGRAM).prc -t appl -c MinZ -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(ZIGLIB)
	cp $(ZIGLIB) $(DLIB)

$(ZIGLIB): main.zig ../libpumpkin/pumpkin.zig
	$(ZIG) build
	touch $(ZIGLIB)

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res $(ZIGLIB)
