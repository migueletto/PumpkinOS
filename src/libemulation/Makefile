include ../common.mak

PROGRAM=$(BIN)/libemulation$(SOEXT)

OBJS=libemulation.o z80.o cz80.o modem.o disk.o rom.o z80disasm.o coco.o m6809.o dasm6809.o vdg.o cas.o spectrum.o spectrumcas.o zx81.o zx81cas.o trs80.o wd1793.o m6502.o apple2.o

CUSTOMFLAGS=-I$(SRC)/font

$(PROGRAM): $(OBJS)
	$(CC) -shared -o $(PROGRAM) $(OBJS) -L$(BIN) -lpit
	cd tools && make

clean:
	rm -f $(PROGRAM) *.o
	cd tools && make clean
