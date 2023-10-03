SETDRV: ; A=drive
        io_out     PIO_DRV
        RET

SETTRK: ; BC=track
        LD      A,B             ; track high = B
        io_out     PIO_TKH
SETTKL:
        LD      A,C             ; track low = C
        io_out     PIO_TKL
        RET

SETSEC: ; A=sector
        io_out     PIO_SEC
        RET

SETCMD: ; A=cmd
        io_out     PIO_CMD
        RET

DSKSTA: ; returns disk status in A
        io_in      PIO_CMD
        RET

RDBYTE: ; read in A one byte from buffer
        io_in      PIO_DAT
        RET

WRBYTE: ; write A to buffer
        io_out     PIO_DAT
        RET
