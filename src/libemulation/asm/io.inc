SIOA_D:	EQU	20H
SIOA_C:	EQU	21H
SIOB_D:	EQU	22H
SIOB_C:	EQU	23H
SIOC_D:	EQU	24H
SIOC_C:	EQU	25H

DSKDRV:	EQU	0 ; set drive
DSKTKH:	EQU	1 ; set track high
DSKTKL:	EQU	2 ; set track low
DSKSEC:	EQU	3 ; set sector
DSKCMD:	EQU	4 ; disk command/status
DSKDAT:	EQU	5 ; disk data

PIO_DRV: EQU	40H + DSKDRV
PIO_TKH: EQU	40H + DSKTKH
PIO_TKL: EQU	40H + DSKTKL
PIO_SEC: EQU	40H + DSKSEC
PIO_CMD: EQU	40H + DSKCMD
PIO_DAT: EQU	40H + DSKDAT

ADDR_PORT:	EQU	0FFFFH
ADDR_DATA:	EQU	0FFFEH

xio_out	macro port
	out (port),a
	endm

xio_in	macro port
	in a,(port)
	endm

; version that preserves A register
io_out	macro port
	ld (ADDR_DATA),a
	push af
	ld a,port
	ld (ADDR_PORT),a
	pop af
	halt
	ei
	endm

; version that does not preserve A register
iob_out	macro port
	ld (ADDR_DATA),a
	ld a,port
	ld (ADDR_PORT),a
	halt
	ei
	endm

io_in	macro port
	ld a,port
	or 80H
	ld (ADDR_PORT),a
	halt
	ei
	ld a,(ADDR_DATA)
	endm
