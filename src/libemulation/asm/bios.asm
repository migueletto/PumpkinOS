;	skeletal cbios for first level of CP/M 2.0 alteration
;
iobyte:	equ	$0003		;intel i/o byte
cdisk:	equ	$0004		;current disk number 0=a,... l5=p
nsects:	equ	($-ccp)/128	;warm start sector count
;
ndrv:	equ	4		;number of disk drives

include	"floppy.inc"
include	"hd.inc"
include	"io.inc"

;
;	jump vector for individual subroutines
;
	jp	boot	;cold start
wboote:	jp	wboot	;warm start
	jp	const	;console status
	jp	conin	;console character in
	jp	conout	;console character out
	jp	dolist	;list character out
	jp	punch	;punch character out
	jp	reader	;reader character in
	jp	home	;move head to home position
	jp	seldsk	;select disk
	jp	SETTRK	;set track number
	jp	setsect	;set sector number
	jp	setdma	;set dma address
	jp	read	;read disk
	jp	write	;write disk
	jp	listst	;return list status
	jp	sectran	;sector translate
;
;	fixed data tables for two drives
;
dpbase:
;	disk Parameter header for disk 0
	dw	skew0, $0000
	dw	$0000, $0000
	dw	dirbf, dpblk0
	dw	$0000, all00
;	disk Parameter header for disk 1
	dw	skew0, $0000
	dw	$0000, $0000
	dw	dirbf, dpblk0
	dw	$0000, all01
;	disk parameter header for disk 2
	dw	skew0, $0000
	dw	$0000, $0000
	dw	dirbf, dpblk2
	dw	$0000, all02
;	disk parameter header for disk 3
	dw	skew0, $0000
	dw	$0000, $0000
	dw	dirbf, dpblk2
	dw	$0000, all03
;

skew0:	db	 1,  2,  3,  4,  5
	db	 6,  7,  8,  9, 10
	db	11, 12, 13, 14, 15
	db	16, 17, 18, 19, 20
	db	21, 22, 23, 24, 25
	db	26, 27, 28, 29, 30
	db	31, 32

;skew6:	db	 1,  7, 13, 19, 25
;	db	 5, 11, 17, 23,  3
;	db	 9, 15, 21,  2,  8
;	db	14, 20, 26,  6, 12
;	db	18, 24,  4, 10, 16
;	db	22

dpblk0:	;disk 0 parameter block
	dw	sptf		;sectors per track
	db	bshf		;block shift factor
	db	blmf		;block mask
	db	exmf		;extent mask
	dw	dsmf		;disk size-1
	dw	drmf		;directory max
	db	$C0		;dir alloc 0
	db	$00		;dir alloc 1
	dw	0		;dir check size
	dw	offf		;reserved tracks
;
dpblk2:	;disk 2 parameter block
	dw	spth		;sectors per track
	db	bshh		;block shift factor
	db	blmh		;block mask
	db	exmh		;extent mask
	dw	dsmh		;disk size-1
	dw	drmh		;directory max
	db	$C0		;dir alloc 0
	db	$00		;dir alloc 1
	dw	0		;dir check size
	dw	offh		;reserved tracks
;
;	end of fixed tables
;
;	individual subroutines to perform each function
boot:	
	call	INITERM
	; IOBYTE:
	; Bits:   6,7     4,5     2,3     0,1
	; Device: LIST    PUNCH   READER  CONSOLE
	; Values:
	; 00      TTY:    TTY:    TTY:    TTY:
	; 01      CRT:    PTP:    PTR:    CRT:
	; 10      LPT:    UP1:    UR1:    BAT:
	; 11      UL1:    UP2:    UR2:    UC1:
	ld	a, $95		; LPT PTP PTR CRT = 10 01 01 01 = $95
	ld	(iobyte),a	;set the iobyte
	ld	hl, welcome
	call	prmsg
	xor	a		;zero in the accum
	ld	(cdisk),a	;select disk zero
	jp	gocpm		;initialize and go to cp/m
;
wboot:	;simplest case is to read the disk until all sectors loaded
	ld	sp, $80		;use space below buffer for stack
	ld	c, 0		;select disk 0
	call	seldsk
	call	home		;go to track 00
;
	ld	b, nsects	;b counts * of sectors to load
	ld	c, 0		;c has the current track number
	ld	d, 2		;d has the next sector to read
;	note that we begin by reading track 0, sector 2 since sector 1
;	contains the cold start loader, which is skipped in a warm start
	ld	hl, ccp		;base of cp/m (initial load point)
load1:	;load	one more sector
	push	bc		;save sector count, current track
	push	de		;save next sector to read
	push	hl		;save dma address
	ld	c, d		;get sector address to register C
	call	setsect		;set sector address from register C
	pop	bc		;recall dma address to b, C
	push	bc		;replace on stack for later recall
	call	setdma		;set dma address from b, C
;
;	drive set to 0, track set, sector set, dma address set
	call	read
	cp	$00		;any errors?
	jp	nz,wboot	;retry the entire boot if an error occurs
;
;	no error, move to next sector
	pop	hl		;recall dma address
	ld	de, 128		;dma=dma+128
	add	hl,de		;new dma address is in h, l
	pop	de		;recall sector address
	pop	bc	;recall number of sectors remaining, and current trk
	dec	b		;sectors=sectors-1
	jp	z,gocpm		;transfer to cp/m if all have been loaded
;
;	more	sectors remain to load, check for track change
	inc	d
	ld	a,d		;sector=sptf?, if so, change tracks
	cp	sptf+1
	jp	c,load1		;carry generated if sector<sptf
;
;	end of	current track,	go to next track
	ld	d, 1		;begin with first sector of next track
	inc	c		;track=track+1
;
;	save	register state, and change tracks
	push	bc
	push	de
	push	hl
	ld	b, 0		;assume track < 256
	call	SETTRK		;track address set from register c
	pop	hl
	pop	de
	pop	bc
	jp	load1		;for another sector
;
;	end of	load operation, set parameters and go to cp/m
gocpm:
	ld	a, $0c3		;c3 is a jmp instruction
	ld	(0),a		;for jmp to wboot
	ld	hl, wboote	;wboot entry point
	ld	(1),hl		;set address field for jmp at 0
;
	ld	(5),a		;for jmp to bdos
	ld	hl, bdos	;bdos entry point
	ld	(6),hl		;address field of Jump at 5 to bdos
;
	ld	bc, $80		;default dma address is 80h
	call	setdma
;
	ei			;enable the interrupt system
	ld	a,(cdisk)	;get current disk number
	ld	c, a		;send to the ccp
	jp	ccp		;go to cp/m for further processing

include "cterm.inc"

; CONSOLE       TTY     CRT     BAT     UC1
sttab:	dw	CHKSER, CHKTTY, noop,   noop
intab:	dw	GETSER, GETTTY, noop,   noop
outtab:	dw	PUTSER, PUTTTY, noop,   noop

; READER        TTY     PTR     UR1     UR2
rdrtab:	dw	noop,   noop,   noop,   noop

; PUNCH         TTY     PTP     UP1     UP2
puntab:	dw	noop,   noop,   noop,   noop

; LIST          TTY     CRT     LPT     UL1
lsttab:	dw	noop,   noop,   noop,   noop
dlstab:	dw	PUTSER, PUTTTY, noop,   noop

noop:	xor a
	ret

reader:	;read character into register A from reader device
	;strip parity bit ?
	ld	hl,rdrtab
	push	af
	ld	a,(iobyte)
	and	$0c
	srl	a
	jr	jiob

punch:	;punch character from register C
	ld	a, c
	ld	hl,puntab
	push	af
	ld	a,(iobyte)
	and	$30
	srl	a
	srl	a
	srl	a
	jr	jiob

conio:
	push	af
	ld	a,(iobyte)
	and	$03
	sla	a
jiob:
	add	l
	ld	l,a
	ld	a,(hl)
	inc	hl
	ld	h,(hl)
	ld	l,a
	pop	af
	jp	(hl)	

const:	;console status, return 0ffh if character ready, 00h if not
	ld	hl,sttab
	jr	conio

conin:	;console character into register A
	ld	hl,intab
	jr	conio

conout:	;console character output from register C
	ld	a, c
	ld	hl,outtab
	jr	conio

lstio:
	push	af
	ld	a,(iobyte)
	and	$c0
	srl	a
	srl	a
	srl	a
	srl	a
	srl	a
	jr	jiob

dolist:	;list character from register c
	ld	a, c
	ld	hl,dlstab
	jr	lstio

listst:	;return list status (0 if not ready, 1 if ready)
	ld	hl,lsttab
	jr	lstio
;
;	i/o drivers for the disk follow
;	for now, we will simply store the parameters away for use
;	in the read and write	subroutines

include "cdisk.inc"

home:	;move to the track 0 position of current drive
	ld	bc, 0		;select track 0
	jp	SETTRK
;
seldsk:	;select disk given by register c
	ld	hl,$0000	;error return code
	ld	a, c
	cp	ndrv
	ret	nc		;no carry if greater
;	disk number is in the proper range

        ld      c, a
        call    SETDRV

;	compute proper disk Parameter header address
	ld	l, a		;l=disk number
	add	hl,hl		;*2
	add	hl,hl		;*4
	add	hl,hl		;*8
	add	hl,hl		;*16 (size of each header)
	ld	de,dpbase
	add	hl,de		;hl=,dpbase (diskno*16)
	ret

setsect:	;set sector given by register c
	ld      a, c
        jp      SETSEC

sectran:
	ex	de,hl		;hl=.trans
	add	hl,bc		;hl=.trans (sector)
	ld	l, (hl)		;l=trans (sector)
	ld	h, 0		;hl=trans (sector)
	ret			;with value in hl
;
setdma:	;set	dma address given by registers b and c
	ld	(dma),bc
	ret
;

read:	;perform read operation
	;return a 00h in register a if the operation completes
	;properly, and 0lh if an error occurs during the read
	ld	a, 0		;read sector
	call	SETCMD
	call	DSKSTA		;get status
	or	a
	jr	nz,read1	;skip copy in case of disk error
	push	hl
	push	bc
	ld	hl,(dma)	;address where data will be writen to
	ld	b,128
nextbr: call	RDBYTE		;get one byte from buffer
	ld	(hl),a		;write one byte to memory
	inc	hl
	dec	b
	jr	nz,nextbr
	pop	bc
	pop	hl
	xor	a		;success
read1:	ret
;

write:	;perform a write operation
	;return a 00h in register a if the operation completes
	;properly, and 0lh if an error occurs during the write
	push	hl
	push	bc
	ld	hl,(dma)	;address where data will be read from
	ld	b,128
nextbw: ld	a,(hl)		;read one byte from memory
	call    WRBYTE		;write one byte to buffer
	inc	hl
	dec	b
	jp	nz,nextbw
	pop	bc
	pop	hl
	ld	a, 1		;write sector
	call	SETCMD
	call	DSKSTA		;get status
	ret

prmsg:	;print message at hl to 0
	ld	a,(hl)
	or	a	;zero?
	ret	z
	push	hl
	ld	c,a
	call	conout
	pop	hl
	inc	hl
	jp	prmsg
welcome:db	"CZ80", 13, 10, 0
;
;	the remainder of the cbios is reserved uninitialized
;	data area, and does not need to be a Part of the
;	system	memory image (the space must be available,
;	however, between"begdat" and"enddat").
;
;
;	scratch ram area for bdos use
begdat:	equ	$	 	;beginning of data area
dma:	ds	2		;transfer address
dirbf:	ds	128	 	;scratch directory area
all00:	ds	alocf	 	;allocation vector 0
all01:	ds	alocf	 	;allocation vector 1
all02:	ds	aloch	 	;allocation vector 2
all03:	ds	aloch	 	;allocation vector 3
;
enddat:	equ	$	 	;end of data area
datsiz:	equ	$-begdat	;size of data area
