include "io.inc"

	org 0100H

bdos:	equ 0005H
width:	equ 256
height:	equ 184
alcols:	equ 12
alrows: equ 4
awidth:	equ (alcols + alcols + 1) * 8
aheight:equ (alrows + alrows) * 8
xmargin:equ 8
ymargin:equ 16
xshield:equ (width - 16*8)/2
yshield:equ height-4*8

debugh	macro v
	push af
	ld a,v
	io_out 7EH
	pop af
endm

debugd	macro v
	push af
	ld a,v
	io_out 7DH
	pop af
endm

debuga	macro v
	push af
	ld a,v
	io_out 7CH
	pop af
endm

set_x	macro x
	ld a,x / 256
	io_out 02H
	ld a,x % 256
	io_out 03H
endm

set_xm	macro addr
	ld a,(addr)
	io_out 03H
	ld a,(addr+1)
	io_out 02H
endm

set_y	macro y
	ld a,y
	io_out 04H
endm

set_ym	macro addr
	ld a,(addr)
	io_out 04H
endm

cls	macro
	ld a,0
	io_out 07H
endm

color	macro c
	ld a,c
	io_out 00H
endm

colorm	macro addr
	ld a,(addr)
	io_out 00H
endm

cursor	macro c
	ld a,c
	io_out 06H
endm

sprite	macro n,on
	push af
	ld a,n+on*128
	io_out 09H
	pop af
endm

	io_in 00H
	ld (oldcolor),a
	cursor 0

	ld hl,player
	ld a,0C0H
	call defdsprite
	ld hl,player
	ld a,0C2H
	call defdsprite
	ld hl,player
	ld a,0C4H
	call defdsprite

	ld hl,missile
	ld a,76H
	call defsprite  ; player missile

	ld hl,missile
	ld a,77H
	call defsprite  ; alien missile

	ld hl,ufo
	ld a,0D8H
	call defdsprite

	ld hl,chars
	ld a,80H
	call defchar
	call defchar
	call defchar
	call defchar
	call defchar
	call defchar
	call defchar
	call defchar
	call defchar

	call resetscore
	call resetaliens
	call resetlives

initgame:
	cls
	color 70H
	sprite 0,0
	sprite 2,0
	sprite 4,0
	sprite 6,0
	sprite 7,0
	sprite 8,0
	ld a,0
	ld (ufoon),a
	ld (ufox),a
	ld (ufox+1),a
	ld (ufoy),a
	ld (ucount),a
	ld (ucount+1),a
	ld (missileon),a
	ld (amissileon),a
	ld (phase),a
	ld (alienx+1),a
	ld (acount),a
	ld (acount+1),a
	ld (mcount),a
	ld (amcount),a
	ld a,1
	ld (dir),a
	ld a,xmargin
	ld (alienx),a
	ld a,ymargin
	ld (alieny),a
	ld a,width/2 - 8
	ld (playerx),a
	ld a,height - 8
	ld (playery),a
	;call drawshield
	call drawline
	call drawlives
	call printscore

loop:
	call drawufo
	call drawplayer
	call drawmissile
	call checkexplosion
	call drawaliens
	call drawamissile
	call checkinput
	jp loop

resetscore:
	ld a,30H
	ld (score),a
	ld (score+1),a
	ld (score+2),a
	ld (score+3),a
	ret

resetlives:
	ld a,3
	ld (lives),a
	ret

resetaliens1:
	ld b,00H
nxtal:
	ld a,(hl)
	ld (de),a
	inc hl
	inc de
	dec b
	jp nz,nxtal
	ret

resetaliens:
	ld a,alcols * alrows
	ld (naliens),a
	ld hl,irow0
	ld de,row0
	call resetaliens1
	ld hl,irow1
	ld de,row1
	call resetaliens1
	ret

drawline:
	set_x 0
	set_y height+4
	ld a,1
	io_out 07H
	set_x (width-1)
	ld a,3
	io_out 07H
	ret

drawlives:
	set_y height+8
	ld a,(lives)
	cp 3
	jp z,lives3
	sprite 4,0
	cp 2
	jp z,lives2
	sprite 2,0
	ret
lives3:
	set_x 18
	sprite 4,1
lives2:
	set_x 0
	sprite 2,1
	ret

drawshield:
	color 60H
	set_x xshield
	set_y yshield
	ld de,shield
	call print
	ret

checkexplosion:
	ld a,(expf)
	or a
	ret z
	ld hl,(expc)
	ld b,0
	ld c,1
	add hl,bc
	ld (expc),hl
	ld a,h
	cp 09H
	ret nz
	ld a,l
	cp 00H
	ret nz
	ld a,0
	ld (expf),a
	ld hl,0000H
	ld (expc),hl
	ld hl,row0
	call eraseexpl
	ld hl,row1
	call eraseexpl
	ret

eraseexpl:
	ld b,00H
nxtexpl:
	ld a,(hl)
	cp a,88H
	jp nz,notexpl
	ld a,' '
	ld (hl),a
notexpl:
	inc hl
	dec b
	jp nz, nxtexpl
	ret

checkinput:
	io_in SIOA_C
	and 1
	jp z,nokey
	io_in SIOA_D
	cp a,'a'
	jp z,moveleft
	cp a,'d'
	jp z,moveright
	cp a,' '
	jp z,playerfire
	cp a,3
	jp nz,nokey
ctrlc:
	sprite 0,0
	sprite 2,0
	sprite 4,0
	sprite 6,0
	sprite 7,0
	sprite 8,0
	colorm oldcolor
	cls
	cursor 1
	jp 0        ; exit

playerfire:
	ld a,(missileon)
	or a
	ret nz
	ld a,(playerx)
	add 4
	ld (missilex),a
	ld a,(playery)
	or a
	sbc 4
	ld (missiley),a
	ld a,1
	ld (missileon),a

nokey:
	ret

moveleft:
	ld a,(playerx)
	or a
	sbc 4
	ld (playerx),a
	ret

moveright:
	ld a,(playerx)
	add 4
	ld (playerx),a
	ret

drawmissile:
	ld a,(missileon)
	or a
	ret z
	ld a,(mcount)
	inc a
	ld (mcount),a
	and 0FFH
	ret nz
	set_xm missilex
	set_ym missiley
	call checkhit
	ld a,(missiley)
	cp 8
	jp c,endmissile
	or a
	sbc 4
	ld (missiley),a
	sprite 6,1
	ret

hit2:
	ld a,(score)
	inc a
	ld (score),a
	ld a,30H
	ret

hit1:
	ld a,(score+1)
	inc a
	cp 3AH
	call z,hit2
	ld (score+1),a
	ld a,30H
	ret

hit:
	ld a,(score+2)
	inc a
	cp 3AH
	call z,hit1
	ld (score+2),a
	call printscore
	ld a,(naliens)
	dec a
	ld (naliens),a
	or a
	jp z,nextwave

endmissile:
	ld a,0
	ld (missileon),a
	sprite 6,0
	ret

nextwave:
	ld a,0
	ld (missileon),a
	ld a,(wave)
	inc a
	ld (wave),a
	sprite 6,0
	call resetaliens
	pop bc
	jp initgame

checkhit:
	ld a,(alienx)
	ld b,a
	ld a,(missilex)
	add 4
	or a
	sbc b
	ret c
	cp awidth
	ret nc
	ld (hitx),a
	ld a,(alieny)
	ld b,a
	ld a,(missiley)
	dec a
	or a
	sbc b
	ret c
	cp aheight
	ld (hity),a
	ret nc
	or a
	srl a
	srl a
	srl a  ; a = a / 8
	or a
	sla a
	sla a
	sla a
	sla a
	sla a  ; a = a * 32
	ld hl,row0
	ld b,0
	ld c,a
	or a
	adc hl,bc
	ld a,(hitx)
	or a
	srl a
	srl a
	srl a
	ld b,0
	ld c,a
	or a
	adc hl,bc
	ld a,(hl)
	cp ' '
	ret z
	cp 88H 
	ret z
	ld a,88H
	ld (hl),a
	ld b,(32 * (alrows + alrows)) / 256
	ld c,(32 * (alrows + alrows)) % 256
	or a
	adc hl,bc
	ld (hl),a
	ld hl,0000H
	ld (expc),hl
	ld a,1
	ld (expf),a
	pop bc
	jp hit

drawamissile:
	ld a,(amissileon)
	or a
	ret z
	ld a,(amcount)
	inc a
	ld (amcount),a
	and 0FFH
	ret nz
	set_xm amissilex
	set_ym amissiley
	ld a,(amissiley)
	cp height
	jp nc,endamissile
	or a
	adc 4
	ld (amissiley),a
	sprite 7,1
	ret

endamissile:
	ld a,0
	ld (amissileon),a
	sprite 7,0
	ld a,(playerx)
	ld b,a
	ld a,(amissilex)
	add 4
	cp b
	ret c
	or a
	sbc 16
	cp b
	ret nc
	call explode
	ld a,(lives)
	dec a
	ld (lives),a
	or a
	jp z,endgame
	call drawlives
	ret

explode:
	sprite 0,0
	ld c,5
	color 10H
explode1:
	set_xm playerx
	set_ym playery
	ld a,88H
	io_out SIOA_D
	io_out SIOA_D
	call delay
	set_xm playerx
	set_ym playery
	ld a,' '
	io_out SIOA_D
	io_out SIOA_D
	call delay
	dec c
	jp nz,explode1
	ret

gameover:
	color 70H
	set_y 24
	set_x ((width-72)/2)
	ld de,msggameover
	call print
	ret

presskey:
	color 70H
	set_y 32
	set_x ((width-16*8)/2)
	ld de,msgpress
	call print
waitkey:
	io_in SIOA_C
	or a
	jp z,waitkey
	io_in SIOA_D
	and 0DFH
	cp 3
	jp z,ctrlc
	cp 'S'
	jp nz,waitkey
	ret

printscore:
	color 70H
	set_x (width-17*8)
	set_y height+8
	ld de,mscore
	call print
	ret

drawplayer:
	set_xm playerx
	set_ym playery
	sprite 0,1
	ret

drawufo:
	ld hl,(ucount)
	ld b,0
	ld c,1
	add hl,bc
	ld (ucount),hl
	ld a,h
	cp 00H
	ret nz
	ld a,l
	cp 40H
	ret nz
	ld hl,0000H
	ld (ucount),hl
	ld a,(ufox)
	or a
	jp nz,drawufo1
	call rnd
	and 01H
	jp z,drawufo2
	ld (ufoon),a
drawufo1:
	ld a,(ufoon)
	or a
	jp z,drawufo2
	set_xm ufox
	set_ym ufoy
	sprite 8,1
drawufo2:
	ld a,(ufox)
	inc a
	ld (ufox),a
	or a
	ret nz
	ld (ufoon),a
	sprite 8,0
	ret

drawaliens:
	ld hl,(acount)
	ld b,0
	ld c,1
	add hl,bc
	ld (acount),hl
	ld a,h
	cp 08H
	ret nz
	ld a,l
	cp 00H
	ret nz
	ld hl,0000H
	ld (acount),hl
	ld a,(dir)
	and 1
	jp z,left
	ld a,(alienx)
	cp width - awidth - xmargin
	jp c,keepright
	ld a,0
	ld (dir),a
	call aliendown
	jp move
keepright:
	add 8
	ld (alienx),a
	jp move
left:
	ld a,(alienx)
	cp xmargin
	jp nz,keepleft
	ld a,1
	ld (dir),a
	call aliendown
	jp move
keepleft:
	or a
	sbc 8
	ld (alienx),a
move:
	ld a,(phase)
	and 1
	jp z,phase0
	ld hl,row1
	jp move1
phase0:
	ld hl,row0
move1:
	xor 1
	ld (phase),a
	call drawrows
	call alienfire
	ret

aliendown:
	ld a,(alienymax)
	cp height - 16
	jp c,down
	pop hl
endgame:
	call gameover
	call presskey
	call resetscore
	call resetaliens
	call resetlives
	jp initgame
down:
	ld a,(alieny)
	add 8
	ld (alieny),a
	ret

alienfire:
	ld a,(amissileon)
	or a
	ret nz
	ld a,(naliens)
	ld b,a
	call rnd
	and 3FH
afire1:
	cmp b
	jp c,afire0
	or a
	sbc b
	jp afire1
afire0:
	ld b,a  ; b = index of firing alien
	ld hl,row0
afire2:
	ld a,(hl)
	or a
	jp z,afire4
	cp ' '
	jp z,afire4
	cp 88H
	jp z,afire4
	ld a,b
	or a
	jp z,afire3
	dec b
afire4:
	inc hl
	jp afire2
afire3:
	ld bc,row0
	or a
	sbc hl,bc    ; hl = index into row0 of firing alien
	ld c,l
	ld a,l
	and 0E0H
	or a
	srl a
	srl a
	ld b,a
	ld a,(alieny)
	add b
	add 8
	ld (amissiley),a
	ld a,c
	and 1FH
	or a
	sla a
	sla a
	sla a
	ld c,a
	ld a,(alienx)
	add c
	ld (amissilex),a
	ld a,1
	ld (amissileon),a
	ret

delay:	ld a,50H
delay1:
	call delay2
	dec a
	jp nz,delay1
	ret

delay2:
	ld b,0FFH
delay3:
	dec b
	jp nz,delay3
	ret

defdsprite:
	io_out 08H
	ld b,16
nexts:
	ld a,(hl)
	io_out 08H
	inc hl
	dec b
	jp nz,nexts
	ret

defsprite:
	io_out 08H
	ld b,8
	jp nexts

defchar:
	io_out 0AH
	ld b,8
	push af
nextc:
	ld a,(hl)
	io_out 0AH
	inc hl
	dec b
	jp nz,nextc
	pop af
	inc a
	ret

drawrow:
	set_xm alienx
	ld b,alcols + alcols - 1
nexta:
	ld a,(hl)
	io_out SIOA_D
	cp ' '
	jp z,notalien
	ld a,e
	ld (alienymax),a
notalien:
	inc hl
	dec b
	jp nz,nexta
	ld c,32 - (alcols + alcols - 1)
	add hl,bc
	ld a,0DH
	io_out SIOA_D
	ld a,e
	add 8
	ld e,a
	ret

linefeed:
	ld a,0AH
	io_out SIOA_D
	ret

drawrows:
	set_ym alieny
	ld a,(alieny)
	ld e,a
	colorm (colors+0)
	call drawrow     ; space
	call linefeed
	call drawrow     ; aliens
	call linefeed
	call drawrow     ; space
	call linefeed
	colorm (colors+1)
	call drawrow     ; aliens
	call linefeed
	call drawrow     ; space
	call linefeed
	colorm (colors+2)
	call drawrow     ; aliens
	call linefeed
	call drawrow     ; space
	call linefeed
	colorm (colors+3)
	call drawrow     ; aliens
	ret

print:
	ld c, 9
	call bdos
	ret

rnd:
	io_in 60H
	ret

msggameover:
	db 'Game over$'
msgpress:
	db 'Press S to start$'

mscore:
	db 'Wave '
wave:
	db '0'
	db ' Score '
score:
	db '0000$'

oldcolor:
	db 0

missileon:
	db 0
missilex:
	db 0,0
missiley:
	db 0
mcount:
	db 0
hitx:	db 0
hity:	db 0
expf:	db 0
expc:	db 0,0

playerx:
	db 0,0
playery:
	db 0
lives:
	db 0

ufox:
	db 0,0
ufoy:
	db 0
ucount:
	db 0,0
ufoon:
	db 0

alienx:
	db 0,0
alieny:
	db 0
alienymax:
	db 0
naliens:
	db 0

amissileon:
	db 0
amissilex:
	db 0,0
amissiley:
	db 0
amcount:
	db 0

acount:
	db 0,0
dir:
	db 0
phase:
	db 0

row0:	ds 256
row1:	ds 256

irow0:
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,80H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,82H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,84H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,86H,20H,0,0,0,0,0,0,0

irow1:
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,81H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,83H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,85H,20H,0,0,0,0,0,0,0
	db 20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,20H,0,0,0,0,0,0,0
	db 20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,87H,20H,0,0,0,0,0,0,0

shield:
	db 0B0H,0B0H,0B0H,0B0H,20H,20H,0B0H,0B0H,0B0H,0B0H,20H,20H,0B0H,0B0H,0B0H,0B0H,'$'

colors:
	db 10H,20H,30H,50H

player:
	db 00000000b
	db 00000000b
	db 00000011b
	db 00000011b
	db 01111111b
	db 11111111b
	db 11111111b
	db 11111111b

	db 00000000b
	db 00000000b
	db 11000000b
	db 11000000b
	db 11111110b
	db 11111111b
	db 11111111b
	db 11111111b

ufo:
	db 00000000b
	db 00000111b
	db 00111111b
	db 01111111b
	db 11010101b
	db 11111111b
	db 00111001b
	db 00010000b

	db 00000000b
	db 11100000b
	db 11111100b
	db 11111110b
	db 10101011b
	db 11111111b
	db 10011100b
	db 00001000b

missile:
	db 00000000b
	db 00001000b
	db 00010000b
	db 00001000b
	db 00010000b
	db 00001000b
	db 00010000b
	db 00000000b

chars:
	; alien 0
	db 00111100b
	db 01111110b
	db 11111111b
	db 11011011b
	db 11111111b
	db 00100100b
	db 01000010b
	db 00100100b

	db 00111100b
	db 01111110b
	db 11111111b
	db 11011011b
	db 11111111b
	db 00100100b
	db 01011010b
	db 10100101b

	; alien 1
	db 01000010b
	db 00100100b
	db 00111100b
	db 01011010b
	db 11111111b
	db 10100101b
	db 10100101b
	db 00111100b

	db 01000010b
	db 00100100b
	db 00111100b
	db 01011010b
	db 11111111b
	db 10100101b
	db 10100101b
	db 01000010b

	; alien 2
        db 00011000b
        db 01111110b
        db 11111111b
        db 10011001b
        db 11111111b
        db 01100110b
        db 10011001b
        db 01000010b

        db 00011000b
        db 01111110b
        db 11111111b
        db 10011001b
        db 11111111b
        db 01100110b
        db 01011010b
        db 10000001b

	; alien 3
	db 01000010b
	db 00100100b
	db 00111100b
	db 01011010b
	db 11111111b
	db 10100101b
	db 10100101b
	db 01100110b

	db 01000010b
	db 00100100b
	db 10111101b
	db 11011011b
	db 11111111b
	db 00100100b
	db 00100100b
	db 01100110b

	; explosion
        db 00010000b
        db 01010010b
        db 00110100b
        db 11111000b
        db 00011111b
        db 00101100b
        db 01001010b
        db 00001000b
