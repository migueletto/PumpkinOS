CFLAGS=-Wall
TOOLS=../tools
ZMAC=$(TOOLS)/zmac/zmac --od . --oo lst,hex,cim
COM=basic.com cls.com color.com bye.com

all: disk0.dsk

# CP/M

disk0.h: mkdisk0h disk0.dsk
	./mkdisk0h disk0.dsk > disk0.h

disk0.dsk: mkdisk bootdr.bin cpm.bin biosdr.bin $(COM)
	./mkdisk disk0.dsk 77 2 bootdr.bin cpm.bin biosdr.bin ./tpa/* $(COM)

# max size: 128 bytes
bootdr.bin: bootdr.asm boot.asm memory.inc dr.inc io.inc floppy.inc
	$(ZMAC) bootdr.asm
	mv bootdr.cim bootdr.bin

# max size: 44*128 = 5632 bytes
cpm.bin: cpm.asm memory.inc dr.inc
	$(ZMAC) cpm.asm
	mv cpm.cim cpm.bin

# max size: 7*128 = 896 bytes
biosdr.bin: biosdr.asm bios.asm memory.inc dr.inc io.inc floppy.inc hd.inc cterm.inc cdisk.inc
	$(ZMAC) biosdr.asm
	mv biosdr.cim biosdr.bin

mkdisk0h: mkdisk0h.o
	$(CC) -o mkdisk0h mkdisk0h.o

mkdisk: mkdisk.o
	$(CC) -o mkdisk mkdisk.o

mkhd: mkhd.o
	$(CC) -o mkhd mkhd.o

i2cz80.hex: i2cz80.asm
	$(ZMAC) i2cz80.asm

patch.hex: patch.asm
	$(ZMAC) patch.asm

# TPA

basic.com: basic.asm memory.inc io.inc dr.inc cterm.inc
	$(ZMAC) basic.asm
	mv basic.cim basic.com

bye.com: bye.asm
	$(ZMAC) bye.asm
	mv bye.cim bye.com

cls.com: cls.asm
	$(ZMAC) cls.asm
	mv cls.cim cls.com

color.com: color.asm
	$(ZMAC) color.asm
	mv color.cim color.com

term.com: term.asm
	$(ZMAC) term.asm
	mv term.cim term.com

term2.com: term2.asm
	$(ZMAC) term2.asm
	mv term2.cim term2.com

sprite.com: sprite.asm
	$(ZMAC) sprite.asm
	mv sprite.cim sprite.com

invaders.com: invaders.asm
	$(ZMAC) invaders.asm
	mv invaders.cim invaders.com

# other

clean:
	rm -f *.lst *.hex *.bin *.o *.exe mkdisk mkdisk0h mkhd disk0.dsk disk0.h $(COM)
