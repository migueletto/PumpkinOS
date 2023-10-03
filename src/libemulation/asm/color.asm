exit:	equ	0000h
bdos:	equ	0005h
arg:	equ	0080h

	org	0100h

	ld hl, arg
	ld a, (hl)
	cp 3
	jp nz, exit
	inc hl
	inc hl

	ld a, (hl)
	cp 30h
	jp c, exit
	cp 38h
	jp nc, exit
	ld (fg), a
	inc hl

	ld a, (hl)
	cp 30h
	jp c, exit
	cp 38h
	jp nc, exit
	ld (bg), a

	ld c, 9
	ld de, color
	call bdos
	jp exit

color:	db 27, '[', '3'
fg:	db 0
	db ';4'
bg:	db 0
	db 'm$'
