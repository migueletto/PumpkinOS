include "io.inc"

	org 0100h
	ld a,90H
	io_out 08H
	ld hl,sprite
	ld b,16
next:
	ld a,(hl)
	io_out 08H
	inc hl
	dec b
	jp nz,next
	ld a,10H
	io_out 09H
	jp 0

sprite:
	db 00111100b
	db 01111110b
	db 11111111b
	db 11111111b
	db 11111111b
	db 11111111b
	db 01111110b
	db 00111100b
	db 00111100b
	db 01111110b
	db 11111111b
	db 11100111b
	db 11100111b
	db 11111111b
	db 01111110b
	db 00111100b
