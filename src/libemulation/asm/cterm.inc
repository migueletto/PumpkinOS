INITERM: RET

CHKTTY: io_in	SIOA_C
        OR      A
	RET

GETTTY:
	CALL	CHKTTY
	JR	Z,GETTTY
	io_in	SIOA_D
        RET                    ; char ready in A

PUTTTY:
	;push af
	;LD	A,0
	;io_out	06H
	;pop af
	io_out	SIOA_D         ; output char in A
	;push af
	;LD	A,1
	;io_out	06H
	;pop af
        RET

CHKSER: io_in	SIOB_C
        OR      A
	RET

GETSER: CALL	CHKSER
	JR	Z,GETSER
	io_in	SIOB_D
        RET                    ; char ready in A

PUTSER: io_out  SIOB_D         ; output char in A
        RET
