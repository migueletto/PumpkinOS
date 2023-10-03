;Configuration file for XMODEM 2.X by M. Eberhard
;Must be named XMODEM.CFG and be on CP/M's default disk.
;This file is extremely verbose, for demonstration.
;Obviously, you can trim it WAY down.

;How this CFG file works:

;(These commands may also be on the command line. Command
;line options override .CFG file options.) Type XMODEM with
;no options for more help.

; Anything after a semicolon on a line is a comment. Tabs,
; spaces, carriage returns, and line feeds are generally
; ignored (though not directly following a slash and not
; between 2 digits of a hex value).

;/C specifies reception with checksums, rather than with
;   CRC error checking. (Transmit error checking is set by
;   the other end.)

;/E specifies a RDR: port that returns with Z set when no
;   character is waiting (for the /X1 option)

;The following /I commands install 8080 code that gets used
;with the /X3 option. /I options are available ib XMODEM
;version 2.4 and later only.

;/I0 hh hh... specifies up to 8 bytes of 8080 code that
;   runs once during initialization, once the .CFG file
;   and command line have both been parsed. (USeful for
;   setting baud rates, etc.)

;/I1 hh hh... specifies up to 8 bytes of 8080 code for
;   the Tx byte routine. The chr to send is in register c.

;/I2 hh hh... specifies up to 8 bytes of 8080 code for
;   the Rx status routine. The routine should return with
;   Z cleared if a chr is waiting.

;/I3 hh hh... specifies up to 8 bytes of 8080 code for the Rx
;   data routine. The chr should be returned in register a.

;/M causes the following message to be printed on the console

;/O option specifies a port initialization sequence

;   The first byte is a port address, and all subsequent
;   bytes are sent to that port.

;   Note that more than one /O option may be specified, so
;   that you can set up the UART and also e.g. the baud rate.

;/P option specifies a custom serial port, for the X2 option

;   Byte 1 = status port address
;   Byte 2 = data port address
;   Byte 3 = 00 if port ready bits are active low
;          = 01 if port ready bits are active high
;   Byte 4 = bit bask for receiver ready bit
;   Byte 5 = bit mask for transmitter ready bit

;/Q suppresses pacifiers during data transfer, useful
;   when the transfer port is also the console

;/X0 uses the CON: port for data transfers
;/X1 uses the RDR:/PUN: port for data transfers (default)
;/X2 uses custom serial port (defined by /P) for data transfers
;/X3 uses patched ports (defined by /I) for data transfers
;(/X3 available in vers. 2.4 and later)

; RC2014:  /Z5 seems to be the best setting
/Z5	;specify a 2 MHz CPU (any integer from 1 to 9)

; RC2014:  Standard full monty likes /X0
/X0 uses the CON: port for data transfers

;--------------------------------------------------------------
;Example port configurations (Un-comment the one you will use.)

;--------------------------------------------------------------
;MITS 88-SIO (No initialization required)
;
;/MDirect I/O is configured for 88-SIO
;/P 00 01 00 01 80	;88-SIO (no init needed)

;--------------------------------------------------------------
;MITS 88-2SIO (Typical Motorola 6850 ports)
;
;/MDirect I/O is configured for 88-2SIO Port A
;/X2			;use custom port defined here
;/P 10 11 01 01 02	;Port A
;/O 10 03 15		;8 data, 1 stop, no parity


;--------------------------------------------------------------
;Compupro Interfacer/Interfacer II
;Control bits are set by jumpers. The board will
;XOR whatever you write to the control port with the
;DIP switch setting. This assumes all these jumpers
;are set to their '0' position
;
;/MDirect I/O is configured for Interfacer port A
;/X2			;use custom port defined here
;/P 01 00 01 02 01	;Port A
;/O 01 AC		;8 data, no parity, controls high, no ints
;
;/MDirect I/O is configured for Interfacer port B
;/X2			;use custom port defined here
;/P 03 02 01 02 01	;Port B
;/O 03 AC		;8 data, no parity, controls high, no ints

;--------------------------------------------------------------
;Vector Graphic Bitstreamer/Bitstreamer II, Imsai SIO-2
;(typical Intel 8251 ports)
;
;/MDirect I/O is configured for Bitstreamer port A
;/X2			;use custom port defined here
;/P 03 02 01 02 01	;Port A
;/O 03 AA 40 4E 27	;8 bits, no parity, ports enabled, etc.
;
;/MDirect I/O is configured for Bitstreamer port B
;/X2			;use custom port defined here
;/P 05 04 01 02 01	;Port B
;/O 05 AA 40 4E 27	;8 bits, no parity, ports enabled, etc.
;
;--------------------------------------------------------------
;Cromemco TU-ART
;
;/MDirect I/O is configured for TU-ART port A
;/X2			;use custom port defined here
;/P 20 21 01 40 80	;Port A
;/O 22 01 		;reset UART
;/O 23 00		;disable interrupts
;/O 20 C0		;9600 baud low byte
;/O 22 00		;9600 baud high bit
;
;/MDirect I/O is configured for TU-ART port B
;/X2			;use custom port defined here
;/P 50 51 01 40 80	;Port B
;/O 52 01 		;reset UART
;/O 53 00		;disable interrupts
;/O 50 a0		;38.4 Kbaud low byte
;/O 52 10		;38.4 Kbaud high bit

;--------------------------------------------------------------
;CCS 2719 serial ports (Typical Zilog DART ports)
;
;/MDirect I/O is configured for CCS 2719 Port A
;/X2			;use custom port defined here
;(typical Z80 DART & Z80 CTC configuration)
;/P 55 54 01 01 04			;Port A
;/O 55 48 01 00 03 C1 04 44 05 EA	;DART: 8 bits, 1 stop, no parity
;/O 50 47 03				;CTC: 38.4K baud
;**/O 50 47 0C				  ;<alternate> CTC: 9600 baud
;**/O 50 47 60				  ;<alternate> CTC: 1200 baud
;
;/MDirect I/O is configured for CCS 2719 Port B
;/X2			;use custom port defined here
;/P 57 56 01 01 04			;Port B
;/O 57 48 01 00 03 C1 04 44 05 EA	;DART: 8 bits, 1 stop, no parity
;/O 51 47 03				;CTC: 38.4K baud
;**/O 51 47 0C				  ;<alternate> CTC: 9600 baud
;**/O 51 47 60				  ;<alternate> CTC: 1200 baud

;--------------------------------------------------------------
;CCS 2810 serial port (which is also the console)
;
;/MDirect I/O is configured for the 2810's serial port, which is also
;/Mthe console. No progress messages will be printed during transfer.
;/X2			;use custom port defined here
;/Q			;Quiet mode, because this is also CON:
;/P 25 20 01 01 10	;2810 serial port
;/O 24 0F		;modem control
;/O 23 83		;Baud rate divisor access
;/O 21 00		;High baud rate
;/O 20 0C		;Low baud rate, 9600 baud
;**/O 20 03		  ;<alternate> 38.4K Baud
;**/O 20 60		  ;<alternate> 1200 baud
;/O 23 03		;Line control
;/O 21 00		;Ints disabled
;/O 25 00		;handshake lines active

;--------------------------------------------------------------
;Processor Technology 3P+S's serial port
;(No initialization required)
;
;/Direct I/O via the 3P+S serial port
;/X2			;use custom port defined here
;/P 00 01 01 40 80

;--------------------------------------------------------------
;Processor Technology Sol-20 (with SOLOS ROM) or
;Subsystem B (with CUTER ROM)
;
;/MUSing Solos/Cuter serial I/O
;/X3			;Use the (following) patched I/O
;/I1 48 3E 01 CD 1C C0	;Tx: call AOUT with a=1 for serial port
;/I2 3E 01 CD 22 C0	;Rx status: call AINP with a=1 for serial port
;/I3			;Rx data: no Rx data routine, since AOUT got
;			;the data. (this line is not actually required)

;--------------------------------------------------------------
;Poly-88 with POLEX ROM
;
;/MUsing POLEX for serial I/O
;/X3			;Use the (following) patched I/O
;/I0 3E 1F CD 18 F4     ;Set baud rate to 9600
;/I1 CD 12 F4		;Call PXSOUT to transit
;/I2 CD 0C F4		;Call PXSTA for status
;/I3 CD 0F F4		;call PXSIN for data
