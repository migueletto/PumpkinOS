include ../common.mak

PROGRAM=libemulation
SOURCE=libemulation.c z80.c cz80.c modem.c disk.c rom.c z80disasm.c coco.c m6809.c dasm6809.c vdg.c cas.c spectrum.c spectrumcas.c zx81.c zx81cas.c trs80.c wd1793.c m6502.c apple2.c
CUSTOMFLAGS=-I$(SRC)/font
EXTDEPS=tools.flag
EXTCLEANDEPS=clean_tools.flag

include ../lib.mak

tools.flag:
	cd tools && make
	touch tools.flag

clean_tools.flag:
	cd tools && make clean
	rm -f tools.flag
