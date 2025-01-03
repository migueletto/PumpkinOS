include "io.inc"

	org 0100h

wait_key:
	io_in SIOA_C
	and 1
	jp nz,got_key
	io_in SIOB_C
	and 1
	jp z,wait_key
	io_in SIOB_D
  io_out SIOA_D
	jp wait_key
got_key:
	io_in SIOA_D
	cp 3
	jp z,exit
	cp 13
	jp nz,notcr
  io_out SIOB_D
	ld a,10
notcr:
  io_out SIOB_D
	jp wait_key
exit:
	jp 0
