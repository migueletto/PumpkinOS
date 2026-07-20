include ../common.mak
include ../commonp.mak

PROGRAM=MinimalRust

ifeq ($(TYPE),)
TYPE=debug
endif

ifeq ($(TYPE),release)
BTYPE=--release
endif

CARGO=cargo
TYPE:=debug
RUSTLIB=target/$(TYPE)/lib$(PROGRAM)$(SOEXT)

PRC=$(VFS)/app_install/$(PROGRAM).prc

$(PROGRAM).prc: $(DLIB) res
	$(PRCBUILD) -f $(PROGRAM).prc -t appl -c MinR -n $(PROGRAM) *.bin
	cp $(PROGRAM).prc $(PRC)

$(DLIB): $(RUSTLIB)
	cp $(RUSTLIB) $(DLIB)

$(RUSTLIB): src/lib.rs
	$(CARGO) build $(BTYPE)

res: resource.rcp
	$(PILRC) resource.rcp; touch res

clean:
	rm -f $(PRC) $(PROGRAM).prc *.bin res $(RUSTLIB)
