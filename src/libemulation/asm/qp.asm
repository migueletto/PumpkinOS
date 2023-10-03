; QTERM customization

include "io.inc"

; 1. Modem input status: 0110 - 011F
; QTERM calls here to check modem input status. Return with the zero flag
; set if no character is available, or with the zero flag clear if a char
; is available. Generally this can be an input from the usart / sio / dart
; status port followed by an 'and'.

	org 110H
	io_in 23H
	or a
	ret
	ds      120H - $

; 2. Read modem character: 0120 - 012F
; This gets a character from the modem input port once the input status has
; decided it's there. Return the character in the a register. Generally this
; can be an input from the usart / sio / dart data port.

	org 120H
	io_in 22H
	ret
	ds      130H - $

; 3. Modem output status: 0130 - 013F
; Check if the modem output port can accept another character. Return with the
; zero flag set if the output port can't receive a character, or with the zero
; flag clear if the output port is ready. Generally this can be an input from
; the usart / sio / dart status port followed by an 'and'.

	org 130H
	ld a,1
	or a
	ret
	ds      140H - $

; 4. Write modem character: 0140 - 014F
; Send the character in the a register to the modem output port. This will only
; be called after the output status routine has returned a non-zero status.
; Generally this can be an output to the usart / sio / dart data port.

	org 140H
	io_out 22H
	ret
	ds      200H - $

; 15. Moveto: 0200 - 022E
; QTERM requires the ability to move the cursor around the screen. It calls
; this subroutine with the required coordinates in hl: where h is the row,
; and l the column to move to. The top left hand corner of the screen is 0,0;
; and the bottom right corner is 23,79. This subroutine will have to do
; terminal output: at 0109H is a routine that prints a character in the c
; register, and at 010CH is a routine to print a decimal number in hl (mainly
; for the use of vt100 and vt220 compatibles). Note that the above two
; subroutines may destroy all registers, so appropriate action should be
; taken if needed.

	org 200H
	push hl
	ld h,0		; hl = col
	add hl,hl
	add hl,hl
	add hl,hl	; hl = col * 8
	ld a,h
	iob_out 02H
	ld a,l
	iob_out 03H
	pop hl
	ld l,h
	ld h,0		; hl = row
	add hl,hl
	add hl,hl
	add hl,hl	; hl = row * 8
	ld a,l
	iob_out 04H
	ret
