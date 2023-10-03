include "floppy.inc"
include "io.inc"

biosl:	equ	380H		; length of the bios (7 sectors on disk)
size:	equ	bios+biosl-ccp	; size of cp/m system
sects:	equ	size/128	; number of sectors to load

	;	begin the load operation 
	im1
	ei
	ld	hl,0038H
	ld	(hl),0EDH
	inc	hl
	ld	(hl),04DH
	ld	hl,0BFFEH
	ld	sp,hl
	ld	bc,2		; b=track 0, c=sector 2
	ld	d,sects		; d=number sectors to load
	ld	hl,ccp		; hl=load address

	LD	A,0		; drive = 0
	iob_out	PIO_DRV		; set drive
	LD	A,0		; track high = 0
	iob_out	PIO_TKH
lsect:	;	load the next sector
	LD	A,B		; track low = b
	iob_out	PIO_TKL
	LD	A,C		; sector = c
	iob_out	PIO_SEC
	LD	A,0
	iob_out	PIO_CMD
next:
	ld	e,128		; 128 bytes to copy
nextb:
	io_in	PIO_DAT		; get one byte from buffer
	ld	(hl),a		; store byte in memory
	inc	hl
	dec	e
	jr	nz,nextb
	dec	d		; sects=sects-1
	jp	z,tobios	; head for the bios

	;	more sectors to load
	inc	c		; sector=sector + 1
	ld	a,c
	cp	sptf+1		; last sector of track?
	jr	c,lsect		; no, go read another

	;	end of track, increment to next track
	ld	c,1		; sector = 1
	inc	b		; track = track + 1
	jr	lsect		; for another group

tobios:
	jp      bios
