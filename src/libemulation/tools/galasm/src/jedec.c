/******************************************************************************
** JEDEC.c
*******************************************************************************
**
** description:
**
** This file contains some functions to save GAL data in JEDEC format.
**
******************************************************************************/

#include "galasm.h"
#include <stdlib.h>

static size_t WriteOutput(void *buf, size_t size, size_t nmemb, FILE *out);

/******************************************************************************
** FileChecksum()
*******************************************************************************
** input:   buff    ActBuffer structure of file buffer
**
** output:          16 bit checksum of the JEDEC structure
**
** remarks: This function calculates the JEDEC file checksum. The start and
**          the end of the area for which the checksum should be calculated
**          must be marked by <STX> and <ETX>!.
******************************************************************************/

int FileChecksum(struct ActBuffer buff)
{
    int checksum;


    checksum = 0;

    while (*buff.Entry != 0x2)                  /* search for <STX> */
        IncPointer(&buff);

    while (*buff.Entry != 0x3)
    {                                           /* search for <ETX> and */
        checksum += *buff.Entry;                /* add values           */

        IncPointer(&buff);
    }

    checksum += 0x3;                              /* add <ETX> too */

    return(checksum);                             /* ready */
}





/******************************************************************************
** FuseChecksum()
*******************************************************************************
** input:   galtype     type of GAL
**
** output:              16 bit checksum of the JEDEC structure
**
** remarks: This function does calculate the fuse checksum of the JEDEC
**          structure.
******************************************************************************/

int FuseChecksum(int galtype)
{
    int     checksum, byte, n;
    BYTE    *ptr, *ptrXOR, *ptrS1;



    ptr    = &Jedec.GALLogic[0] - 1L;
    ptrXOR = &Jedec.GALXOR[0];
    ptrS1  = &Jedec.GALS1[0];

    n = checksum = byte = 0;

    for (;;)
    {

        if (galtype == GAL16V8)
        {
            if (n == XOR16)
            {
                ptr = &Jedec.GALXOR[0];
            }
            else
            {
                if (n == XOR16+8)
                {
                    ptr = &Jedec.GALSig[0];
                }
                else
                {
                    if (n == NUMOFFUSES16)
                        break;
                    else
                        ptr++;
                }
            }
        }


        if (galtype == GAL20V8)
        {
            if (n == XOR20)
            {
                ptr = &Jedec.GALXOR[0];
            }
            else
            {
                if (n == XOR20+8)
                {
                    ptr = &Jedec.GALSig[0];
                }
                else
                {
                    if (n == NUMOFFUSES20)
                        break;
                    else
                        ptr++;
                }
            }
        }


        if (galtype == GAL22V10)
        {
            if (n >= XOR22V10 && n < XOR22V10 + 20)
            {
                if (!(n % 2))
                    ptr = ptrXOR++;
                else
                    ptr = ptrS1++;
            }
            else
            {
                if (n == SIG22V10)
                    ptr = &Jedec.GALSig[0] - 1L;

                if (n == SIG22V10 + SIG_SIZE)
                    break;
                else
                    ptr++;
            }
        }


        if (galtype == GAL20RA10)
        {
            if (n == XOR20RA10)
            {
                ptr = &Jedec.GALXOR[0];
            }
            else
            {
                if (n == SIG20RA10 + SIG_SIZE)
                    break;
                else
                    ptr++;
            }
        }


        byte |= (*ptr << (n+8) % 8);

        if (!((n + 9)%8))
        {
            checksum += byte;
            byte = 0;
        }

        n++;
    }

    checksum += byte;

    return(checksum);
}





/******************************************************************************
** MakeJedecBuff()
*******************************************************************************
** input:   buff     ActBuffer structure for the ram buffer
**          galtype  type of GAL
**			cfg		 pointer to the config structure
**
** output:  0:  o.k.
**         -1:  not enough free memory
**
** remarks: generates the JEDEC file in a ram buffer
******************************************************************************/
 
int MakeJedecBuff(struct ActBuffer buff, int galtype, struct Config *cfg)
{
    UBYTE   mystrng[16];
    struct  ActBuffer buff2;
    int     n, m, bitnum, bitnum2, flag;
    int     MaxFuseAdr = 0, RowSize = 0, XORSize = 0;


    switch (galtype)
    {
         case  GAL16V8:
                 MaxFuseAdr = MAX_FUSE_ADR16; /* This variables are defined  */
                 RowSize    = ROW_SIZE_16V8;  /* both globally AND locally!  */
                 XORSize    = 8;              /* All assignments concern to  */
                 break;                       /* the locale variables!!!!    */

         case GAL20V8:
                 MaxFuseAdr = MAX_FUSE_ADR20;
                 RowSize    = ROW_SIZE_20V8;
                 XORSize    = 8;
                 break;

         case GAL22V10:
                 MaxFuseAdr = MAX_FUSE_ADR22V10;
                 RowSize    = ROW_SIZE_22V10;
                 XORSize    = 10;
                 break;

         case GAL20RA10:
                 MaxFuseAdr = MAX_FUSE_ADR20RA10;
                 RowSize    = ROW_SIZE_20RA10;
                 XORSize    = 10;
                 break;
    }



    buff2 = buff;

    if (!cfg->JedecFuseChk)
        if (AddString(&buff, (UBYTE *)"\2\n"))          /* <STX> */
            return(-1);

	/*** make header of JEDEC file ***/
    if (AddString(&buff, (UBYTE *)"Used Program:   GALasm 2.1\n"))
        return(-1);

    if (AddString(&buff, (UBYTE *)"GAL-Assembler:  GALasm 2.1\n"))
        return(-1);

    if (galtype == GAL16V8)
    {
        if (AddString(&buff, (UBYTE *)"Device:         GAL16V8\n\n"))
            return(-1);
    }

    if (galtype == GAL20V8)
    {
        if (AddString(&buff, (UBYTE *)"Device:         GAL20V8\n\n"))
            return(-1);
    }

    if (galtype == GAL20RA10)
    {
        if (AddString(&buff, (UBYTE *)"Device:         GAL20RA10\n\n"))
            return(-1);
    }

    if (galtype == GAL22V10)
    {
        if (AddString(&buff, (UBYTE *)"Device:         GAL22V10\n\n"))
            return(-1);
    }


    if (AddString(&buff, (UBYTE *)"*F0\n"))     /* default value of fuses */
        return(-1);

    if (cfg->JedecSecBit)
    {   /* Security-Bit */
        if (AddString(&buff, (UBYTE *)"*G1\n"))
            return(-1);
    }
    else
        if (AddString(&buff, (UBYTE *)"*G0\n"))
            return(-1);


    if (galtype == GAL16V8)                       /* number of fuses */
        if (AddString(&buff, (UBYTE *)"*QF2194\n"))
            return(-1);

    if (galtype == GAL20V8)
        if (AddString(&buff, (UBYTE *)"*QF2706\n"))
            return(-1);
      
    if (galtype == GAL20RA10)
        if (AddString(&buff, (UBYTE *)"*QF3274\n"))
            return(-1);

    if (galtype == GAL22V10)
        if (AddString(&buff, (UBYTE *)"*QF5892\n"))
            return(-1);

	/*** make fuse-matrix ***/

    bitnum = bitnum2 = flag = 0;

    for (m = 0; m < RowSize; m++)
    {
        flag = 0;

        bitnum2 = bitnum;

        for (n = 0; n <= MaxFuseAdr; n++)
        {
            if (Jedec.GALLogic[bitnum2])
            {
                flag = 1;
                break;
            }

            bitnum2++;
        }

        if (flag)
        {
            sprintf((char *)&mystrng[0], "*L%04d ", bitnum);

            if (AddString(&buff, (UBYTE *)&mystrng[0]))
                return(-1);

            for (n=0; n<=MaxFuseAdr; n++)
            {
                if (AddByte(&buff, (UBYTE)(Jedec.GALLogic[bitnum] + '0')))
                    return(-1);
                bitnum++;
            }

            if (AddByte(&buff, (UBYTE)'\n'))
                return(-1);
        }
        else
            bitnum = bitnum2;
    }

    if (!flag)
        bitnum = bitnum2;

                                                /*** XOR-Bits ***/
    sprintf((char *)&mystrng[0], "*L%04d ", bitnum);    /* add fuse adr. */
    if (AddString(&buff, (UBYTE *)&mystrng[0]))
        return(-1);

    for (n = 0; n < XORSize; n++)
    {
        if (AddByte(&buff, (UBYTE)(Jedec.GALXOR[n] + '0')))
            return(-1);
        bitnum++;

        if (galtype == GAL22V10)
        {                                           /*** S1 of 22V10 ***/
            if (AddByte(&buff, (UBYTE)(Jedec.GALS1[n] + '0')))
                return(-1);
            bitnum++;
        }
    }

    if (AddByte(&buff, (UBYTE)'\n'))
        return(-1);


                                                /*** Signature ***/
    sprintf((char *)&mystrng[0], "*L%04d ", bitnum);

    if (AddString(&buff, (UBYTE *)&mystrng[0]))
        return(-1);

    for (n = 0; n < SIG_SIZE; n++)
    {
        if (AddByte(&buff, (UBYTE)(Jedec.GALSig[n] + '0')))
            return(-1);
        bitnum++;
    }

    if (AddByte(&buff, (UBYTE)'\n'))
        return(-1);



    if ((galtype == GAL16V8) || (galtype == GAL20V8))
    {
                                                        /*** AC1-Bits ***/
        sprintf((char *)&mystrng[0], "*L%04d ", bitnum);
        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);

        for (n = 0; n < AC1_SIZE; n++)
        {
            if (AddByte(&buff, (UBYTE)(Jedec.GALAC1[n] + '0')))
                return(-1);
            bitnum++;
        }

        if (AddByte(&buff, (UBYTE)'\n'))
            return(-1);


                                                        /*** PT-Bits ***/
        sprintf((char *)&mystrng[0], "*L%04d ", bitnum);
        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);

        for (n = 0; n < PT_SIZE; n++)
        {
            if (AddByte(&buff, (UBYTE)(Jedec.GALPT[n] + '0')))
                return(-1);
            bitnum++;
        }

        if (AddByte(&buff, (UBYTE)'\n'))
            return(-1);


                                                        /*** SYN-Bit ***/
        sprintf((char *)&mystrng[0], "*L%04d ", bitnum);

        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);

        if (AddByte(&buff, (UBYTE)(Jedec.GALSYN + '0')))
            return(-1);

        if (AddByte(&buff, (UBYTE)'\n'))
            return(-1);

        bitnum++;


                                                        /*** AC0-Bit ***/
        sprintf((char *)&mystrng[0], "*L%04d ", bitnum);

        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);

        if (AddByte(&buff, (UBYTE)(Jedec.GALAC0 + '0')))
            return(-1);

        if (AddByte(&buff, (UBYTE)'\n'))
            return(-1);

    }



/*  if (cfg->JedecFuseChk)
    {*/                                   /* add fuse-checksum */
        sprintf((char *)&mystrng[0], "*C%04x\n", FuseChecksum(galtype));

        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);
/*    }*/


    if (AddString(&buff, (UBYTE *)"*\n"))                 /* closing '*' */
        return(-1);


    if(!cfg->JedecFuseChk)
    {
        if (AddByte(&buff, (UBYTE)0x3))                     /* <ETX> */
            return(-1);

        sprintf((char *)&mystrng[0], "%04x\n", FileChecksum(buff2));

        if (AddString(&buff, (UBYTE *)&mystrng[0]))
            return(-1);
    }


    return(0);
}


/******************************************************************************
** WriteJedecFile()
*******************************************************************************
** input:   galtype     type of GAL
**          cfg			configuration structure
**
** output:  none
**
** remarks: generats the JEDEC file out of the JEDEC structure
******************************************************************************/
 
void WriteJedecFile(char *filename, int galtype, struct Config *cfg)
{
    struct  ActBuffer       mybuff;
    struct  Buffer          *first_buff;
    UBYTE   *filebuffer, *filebuffer2;
    long    result;
	FILE *fp;

	if(!(first_buff = (struct Buffer *) calloc(sizeof(struct Buffer),1)))
    {
        ErrorReq(2);                                /* out of memory? */
        return;
    }

    mybuff.ThisBuff = first_buff;
    mybuff.Entry    = (UBYTE *)(&first_buff->Entries[0]);
    mybuff.BuffEnd  = (UBYTE *)first_buff + (long)sizeof(struct Buffer);



    if (MakeJedecBuff(mybuff, galtype, cfg))
    {                                       /* put JEDEC in ram-buffer */
        FreeBuffer(first_buff);             /* error?                  */
        ErrorReq(2);
        return;
    }

    if ((fp = fopen(filename, "w")))
    {
        for (;;)
        {
            filebuffer = filebuffer2 = mybuff.Entry;

            while (filebuffer2 < mybuff.BuffEnd)
            {                                   /* get size of buffer */
                if (!*filebuffer2)
                    break;
                filebuffer2++;
            }
                                                /* save buffer */
			/* DHH - 24-Oct-2012: ensure lines are terminated with CRLF */
			result = WriteOutput(filebuffer,(size_t) 1, (size_t) (filebuffer2 - filebuffer),fp);

            if (result != (filebuffer2 - filebuffer))
            {                                   /* write error? */
                fclose(fp);
                FreeBuffer(first_buff);
                ErrorReq(13);
                return;
            }

            if (!mybuff.ThisBuff->Next)         /* more buffers here? */
                break;                          /* no, then cancel */

            mybuff.ThisBuff = mybuff.ThisBuff->Next;
            mybuff.Entry    = (UBYTE *)(&mybuff.ThisBuff->Entries[0]);
            mybuff.BuffEnd  = (UBYTE *)mybuff.ThisBuff + (long)sizeof(struct Buffer);
        }

        fclose(fp);
    }
    else
    {
        FreeBuffer(first_buff);                     /* error?, then cancel*/
        ErrorReq(13);                               /* can't open file */
        return;
    }

    FreeBuffer(first_buff);
}

/*
 * DHH - 24-Oct-2012
 * This function works like fwrite, but outputs newlines as CRLF.
 * My GAL programmer (Wellon VP-190) doesn't like JEDEC files
 * with bare newlines in them.
 */
static size_t WriteOutput(void *buf_, size_t size, size_t nmemb, FILE *out)
{
	unsigned char *buf = (unsigned char *) buf_;
	size_t i;

	for (i = 0; i < size*nmemb; i++) {
		unsigned char byte = buf[i];
		if (byte == '\n') {
			fwrite("\r\n", 1, 2, out);
		} else {
			fwrite(&byte, 1, 1, out);
		}
	}

	return nmemb;
}
