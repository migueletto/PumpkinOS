include ../common.mak
include ../commonp.mak

ZIG=zig
ZIGLIB=zig-out/lib/libzigtest.so

PROGRAM=zigtest
PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB)
	$(PRCDUILD) -f $(PROGRAM).prc -t CmdE -c ZigT -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(ZIGLIB)
	cp $(ZIGLIB) $(DLIB)

$(ZIGLIB): main.zig build.zig
	$(ZIG) build -DOS=$(OS)
	touch $(ZIGLIB)

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin $(ZIGLIB)
