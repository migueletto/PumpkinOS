; I2CZ80.ASM
;
;	TO USE: First edit this file filling in answers for your own
;		equipment.  Then assemble with ASM.COM or equivalent
;		assembler.  Then use MLOAD to merge into the main file:
;
;		MLOAD IMP.COM=IMP.COM,I2R4-x.HEX

INCLUDE "io.inc"

YES:	EQU	0FFH
NO:	EQU	0

LF:	EQU	10	; ^J = Linefeed
CR:	EQU	13	; ^M = Carriage return

	ORG	0100H

	DS	3	; Skip the data area below

; These routines and equates are at the beginning of the program so
; they can be patched by a monitor or overlay file without re-assembling
; the program.

MSPEED:	 DB	9	; 0=110 1=300 2=450 3=600 4=710 5=1200		103H
			; 6=2400 7=4800 8=9600 9=19200 default
HS2400:	 DB	NO	; Yes=2400 bps highest speed			104H
HS1200:	 DB	NO	; Yes=1200 bps highest speed			105H
RACAL:	 DB	NO	; Yes=Racal-Vadic 1200V or 2400V or 2400PA	106H
PROMODM: DB	NO	; Yes=Prometheus ProModem 1200 bps		107H
RESVD1:	 DB	NO	; Reserved for special modems			108H
RESVD2:	 DB	NO	; Reserved for special modems			109H

CLEAR:	 DB	12	; Clear screen character (ESC not needed)	10AH
CLOCK:	 DB	40	; Clock speed in MHz x10, 25.5 MHz max. 	10BH
			; 20=2 MHh, 37=3.68 MHz, 40=4 MHz, etc.
BYTDLY:	 DB	0	; 0=0 delay  1=10ms  5=50 ms - 9=90 ms		10CH
			;   default time to send character in ter-
			;   minal mode file transfer for slow BBS
CRDLY:	 DB	0	; 0=0 delay 1=100 ms 5=500 ms - 9=900 ms	10DH
			;   default time for extra wait after CRLF
			;   in terminal mode file transfer
NOFCOL:	 DB	5	; Number of directory columns shown		10EH
TCHPUL:	 DB	'T'	; T=tone, P=Pulse (Hayes 2400 modems)		10FH

ADDLFD:	 DB	NO	; Yes=add LF after CR to send file in terminal	110H
			;   mode (normally added by remote echo)
CONVRUB: DB	YES	; Yes=convert rub to backspace			111H
CRCDFLT: DB	YES	; Yes=default to CRC checking			112H
IGNRCTL: DB	NO	; Yes=CTL-chars above ^M not displayed		113H

EXTCHR:	 DB	'['-40H	; ESC = preceeds local control character	114H
EXITCHR: DB	'E'	; Exit character				115H
FILESND: DB	'F'	; Send file when in terminal mode		116H
NOCONCT: DB	'N'	; Disconnect from phone line			117H
LOGCHR:	 DB	'L'	; Send logon					118H
LSTCHR:	 DB	'P'	; Toggle printer				119H
UNSAVCH: DB	'R'	; Close input text buffer			11AH
SAVECHR: DB	'Y'	; Open input text buffer			11BH
CLEARS:	 DB	'Z'	; Clears screen, terminal mode			11CH
BRKCHR:	 DB	'Q'	; Send a break tone				11DH
NODTR:	 DB	NO	; YES if no DTR and need ATH0 to disconnect	11EH

; Handles in/out ports for data and status

IMDCTL1: JP   RCVCTL            ; In modem control port                 11FH
         DB    0,0,0,0,0,0,0    ; Spares if needed                      122H

IMDTXE:  JP   RCVCTL            ;                                       129H
         DB    0,0,0,0,0,0,0    ;                                       12CH

IMDDATP: JP   RCVDATP           ;in modem data port                     133H
         DB    0,0,0,0,0,0,0    ;                                       146H

OMDDATP: JP   SNDDATP           ; Out modem data port                   13DH
         DB    0,0,0,0,0,0,0    ; Spares if needed                      140H

AMDRCV:  AND	01H		;					147H
	 RET			;					149H

CMDRCV:  CP	01H		;					14AH
	 RET			;					14CH

AMDSND:  LD	A,80H		;					14DH
	 RET			;					14FH

CMDSND:  CP	80H		;					150H
	 RET			;					152H

AMDTXE:  AND	80H		;					153H
	 RET			;					155H

CMDTXE:  CP	80H		;					156H
	 RET			;					158H

; Special exit vector, used by some computers to reset interrupt vectors

JEXITVEC:RET			;					159H
	 DB	0,0		;					15AH

; Jump vectors needed by each overlay

JGOODBYE:JP	GOODBYE		; Disconnects modem by dropping DTR	15CH
JINITMOD:JP	INITMOD		; Initializes modem, autosets baudrate	15FH
JSTUPR:  JP	STUPR		; SET routine to change baudrate	162H
JSYSVR:  JP	SYSVR		; Signon message			165H

; "AT" command strings, can be replaced in individual overlay if needed

JSTRNGA: DS	3		; 1200 bps "AT" string			168H
JSTRNG1: DS	3		; 2400 bps "AT" string			16BH

; Next fourteen lines should not be changed by user overlay as these go
; to specific locations in the main program, not in the overlay.

JCMDSPL:  DS	3		; Allows entry of baudrate on CMD line	16EH
JCRLF:	  DS	3		; Turns up one new line on display	171H
JDIAL:	  DS	3		; Start of dialing routine		174H
JDSCONT:  DS	3		; Terminates modem use			177H
JGOLST:   DS	3		; Printer routine, needed by Apple //e	17AH
JILPRT:   DS	3		; Prints an inline string, 0 to end	17DH
JINBUF:   DS	3		; Stores a keybd string for comparison	180H
JINLNCP:  DS	3		; Inline "compare strings" routine	183H
JINMDM:   DS	3		; Max .1 sec wait for modem character	186H
JRCVRSP:  DS	3		; For 3801 I/O use (TV-803)		189H
JSNDCHR:  DS	3		; Sends a character to the modem	18CH
JSNDSTR:  DS	3		; Sends a string to the modem, $ to end 18FH
JTIMER:   DS	3		; .1 second timer (amount in 'B' reg.)	192H
JBREAK:   DS	3		; Break routine 			195H
JNEW2:	  DB	0,0,0		; For future needs			198H

MANUAL:	  DB	0		; For manual selection flag		19BH
J300:	  JP	OK300		; Sets baudrate to 300 baud		19CH
J1200:	  JP	OK1200		; Sets baudrate to 1200 bps		19FH
J2400:	  JP	OK2400		; Sets baudrate to 2400 bps		1A2H

LOGPTR:	  DW	LOGON		; Pointer to display LOGON message	1A5H

SYSVR:	  CALL	JILPRT		; Display the following line
	  DB	'CZ80',CR,LF,0
	  RET

LOGON:	  DB	'This is a CZ80 computer',CR,LF,0

GOODBYE:
INITMOD:
STUPR:
OK300:
OK1200:
OK2400:
	RET

RCVCTL:
	;LD	A,(LFBUF)
	;OR	A
	;JP	Z,RCVCTL1
	;OR	01H
	;RET
RCVCTL1:
	io_in SIOB_C
	RET

RCVDATP:
	;LD	A,(LFBUF)
	;PUSH	AF
	;LD	A,0
	;LD	(LFBUF),A
	;POP	AF
	;OR	A
	;JP	NZ,RCVDATP1	; return saved LF
	io_in SIOB_D		; return char
	;CP	LF
	;JP	NZ,RCVDATP1
	;LD	(LFBUF),A	; save LF for later
	;LD	A,CR		; change LF to CR
RCVDATP1:
	RET

SNDDATP:
	;CP	CR
	;JP	NZ,SNDDATP1
	;LD	A,LF		; change CR to LF
SNDDATP1:
	io_out SIOB_D
	RET

;LFBUF:	DB	0
;	END
