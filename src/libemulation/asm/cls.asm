bdos:	equ	0005h

	org	0100h

	ld c, 9
	ld de, cls
	call bdos
	jp 0000h

cls:	db 27, '[', '2', 'J', 27, '[', 'H', '$'
