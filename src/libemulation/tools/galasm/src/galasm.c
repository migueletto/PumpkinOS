/******************************************************************************
** GALasm.c
*******************************************************************************
**
** description:
**
** This file contains the GAL-assembler.
**
******************************************************************************/



/********************************* includes **********************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "galasm.h"


/********************************** defines **********************************/

#define SUFFIX_NON              0       /* possible suffixes */
#define SUFFIX_T                1
#define SUFFIX_R                2
#define SUFFIX_E                3
#define SUFFIX_CLK              4
#define SUFFIX_APRST            5
#define SUFFIX_ARST             6

#define DUMMY_OLMC11            24
#define DUMMY_OLMC12            25


/******************************** variables **********************************/
 



/* Diese Arrays geben an, in welche Spalte der ent-  */
/* sprechende Pin eingekoppelt (bzw. rückgekoppelt)  */
/* wird. Für die invertierende Einkopplung ist 1 zu  */
/* addieren, um die entsprechende Spalte zu erhalten */
/* -1 heißt: keine Einkopplung auf Matrix vorhanden  */


/* A possible translation (courtesy of Babelfish)
 * These arrays indicate, into which column the appropriate pin
 * becomes linked (and/or jerk-coupled). For inverting linking 
 * is to be added 1, in order to receive the appropriate column -1 hei_t
 * : no linking on matrix available.
 */

/* Possible interpretation (by me) 
 * These arrays maps the pins to the fuse matrix. Each integer
 * represents the number of the linked column or -1 if that pin
 * dosn't link to any column . For inverted signals, the right column
 * number can be calculated by adding one to the given number.
 */

/* GAL16V8 */

int     PinToFuse16Mode1[20]    = {  2,  0,  4,  8, 12, 16, 20, 24, 28, -1,
                                    30, 26, 22, 18, -1, -1, 14, 10,  6, -1 };

int     PinToFuse16Mode2[20]    = {  2,  0,  4,  8, 12, 16, 20, 24, 28, -1,
                                    30, -1, 26, 22, 18, 14, 10,  6, -1, -1 };

int     PinToFuse16Mode3[20]    = { -1,  0,  4,  8, 12, 16, 20, 24, 28, -1,
                                    -1, 30, 26, 22, 18, 14, 10,  6,  2, -1 };

/* GAL20V8 */

int     PinToFuse20Mode1[24]    = {  2, 0, 4, 8,12,16,20,24,28,32,36,-1,
                                    38,34,30,26,22,-1,-1,18,14,10, 6,-1 };

int     PinToFuse20Mode2[24]    = {  2, 0, 4, 8,12,16,20,24,28,32,36,-1,
                                    38,34,-1,30,26,22,18,14,10,-1, 6,-1 };

int     PinToFuse20Mode3[24]    = { -1, 0, 4, 8,12,16,20,24,28,32,36,-1,
                                    -1,38,34,30,26,22,18,14,10, 6, 2,-1 };


/* GAL22V10 */

int     PinToFuse22V10[24]      = {  0, 4, 8,12,16,20,24,28,32,36,40,-1,
                                    42,38,34,30,26,22,18,14,10, 6, 2,-1 };


/* GAL20RA10 */

int     PinToFuse20RA10[24]     = { -1, 0, 4, 8,12,16,20,24,28,32,36,-1,
                                    -1,38,34,30,26,22,18,14,10, 6, 2,-1 };


/* These arrays show which row is connected to */
/* which OLMC */

int     ToOLMC[8]         = { 56, 48, 40, 32, 24, 16, 8, 0 };

int     ToOLMC22V10[12]   = { 122, 111, 98, 83, 66, 49, 34, 21, 10, 1, 0, 131 };

int     ToOLMC20RA10[10]  = { 72, 64, 56, 48, 40, 32, 24, 16, 8, 0 };

/* this array shows the size of the */
/* 22V10-OLMCs ( AR and SP = 1 )    */

int     OLMCSize22V10[12] = { 9, 11, 13, 15, 17, 17, 15, 13, 11, 9, 1, 1 };

/* The last two entries of the 22V10 arrays are for the   */
/* AR and SP rows of the 22V10 GAL. This rows are not     */
/* connected to an OLMC. But to keep the assembler as     */
/* simple as possible, I introduced  two "OLMCs" for the  */
/* AR and SP. The assembler treads them like real OLMCs.  */
/* So don't become confused when OLMC number 11 and 12    */
/* are used as inputs, outputs, pins... by the assembler. */
/* These two OLMCs are just dummy-OLMCs.                  */


UBYTE   PinNames[24][10];
UBYTE   PinDecNeg[24];
UBYTE   ModeErrorStr[] = "Mode  x:  Pin xx";
UBYTE   *pinnames;
int    	modus;

int     linenum;
UBYTE   *actptr, *buffend;


struct  JedecStruct     Jedec;

struct  Pin             actPin;
struct  GAL_OLMC        OLMC[12];

UBYTE   *fbuff;



/******************************************************************************
** int AssemblePldFile(char *file)
*******************************************************************************
** input:   file  The file to be assembled 
**
** output:  0:    successful
**          else: error
**
** remarks: This function does assemble a *.pld file.
******************************************************************************/
 
int AssemblePldFile(char *file, struct Config *cfg)
{ 
    UBYTE   chr;
    UBYTE   *bool_start, *oldptr;
    char    prevOp;
    char    suffix_strn[MAX_SUFFIX_SIZE];
    int     i = 0, j, k, l = 0, n, m;
    int     max_chr, pass, pin_num, bool_linenum;
    int     actOLMC, row_offset, newline, oldline;
    int     suffix, start_row, max_row, num_of_olmcs;
	int		gal_type;
	int		num_of_pins;
	int  	num_of_col,fsize;

        {
            fsize = FileSize(file);

		if((fbuff = malloc(fsize)))
            {
                if ((ReadFile(file, fsize, (char *)fbuff)))
                {
                    actptr  = fbuff;
                    buffend = fbuff+fsize;
                    linenum = 1;

                    for (n = 0; n < sizeof(Jedec); n++)
                    {                            /* init. JEDEC structure */
                        if (n < LOGIC22V10_SIZE)
                            Jedec.GALLogic[n] = 1;         /* set fuses */
                        //else
                            //Jedec.GALLogic[n] = 0;         /* clear ACW... */
                    }



                    for (n = 0; n < 12; n++)
                    {                              /* clear OLMC structure */
                        OLMC[n].Active   = 0;
                        OLMC[n].PinType  = 0;
                        OLMC[n].TriCon   = 0;
                        OLMC[n].Clock    = 0;
                        OLMC[n].ARST     = 0;
                        OLMC[n].APRST    = 0;
                        OLMC[n].FeedBack = 0;
                    }


                    /*** get type of GAL ***/

                    if (strncmp((char *)actptr, "GAL16V8", (size_t)7) == 0)
                    {
                        num_of_olmcs = 8;                  /* number of OLMCs */
                        num_of_pins  = 20;                 /* number of pins  */
                        num_of_col   = MAX_FUSE_ADR16 + 1; /* number of col.  */
                        gal_type     = GAL16V8;

                        if ((*(actptr+7L) != ' ')  &&	/* Only ' ', newline and tab are valid 	*/
                            (*(actptr+7L) != 0x0A) &&   /* after the GAL's name					*/
                            (*(actptr+7L) != 0x09))		/* Any other char will produce an error */
                        {
                          AsmError(1, 0);
                            return(-1);
                        }
                    }
                    else
                    if (strncmp((char *)actptr, "GAL20V8", (size_t)7) == 0)
                    {
                        num_of_olmcs = 8;                 /* num of OLMCs */
                        num_of_pins  = 24;                /* num of pins  */
                        num_of_col   = MAX_FUSE_ADR20 + 1;/* num of col   */
                        gal_type     = GAL20V8;

                        if ((*(actptr+7L) != ' ')  &&
                            (*(actptr+7L) != 0x0A) &&
                            (*(actptr+7L) != 0x09))
                        {
                              AsmError(1, 0);
                            return(-1);
                        }
                    }
                    else
                    if (strncmp((char *)actptr, "GAL20RA10", (size_t)9) == 0)
                    {
                        num_of_olmcs = 10;
                        num_of_pins  = 24;
                        num_of_col   = MAX_FUSE_ADR20RA10 + 1;
                        gal_type     = GAL20RA10;

                        if ((*(actptr+9L) != ' ')  &&
                            (*(actptr+9L) != 0x0A) &&
                            (*(actptr+9L) != 0x09) )
                        {
                                AsmError(1, 0);
                            return(-1);
                        }
                    }
                    else
                    if (strncmp((char *)actptr, "GAL22V10", (size_t)8) == 0)
                    {
                        num_of_olmcs = 10;
                        num_of_pins  = 24;
                        num_of_col   = MAX_FUSE_ADR22V10 + 1;
                        gal_type     = GAL22V10;

                        if ((*(actptr+8L) != ' ')  &&
                            (*(actptr+8L) != 0x0A) &&
                            (*(actptr+8L) != 0x09))
                        {
                              AsmError(1, 0);
                            return(-1);
                        }
                    }
                    else
                    {
                        AsmError(1, 0);
                        return(-1);
                    }


                                /*** get the leading 8 bytes of the second ***/
                                /*** line as signature                     ***/


                if (GetNextLine())                   /* end of file? */
                {
                    AsmError(2, 0);                  /* yes, then error */
                    return(-1);
                }
                                            /* store signature in the */
                n = m = 0;                  /* JEDEC structure        */


                                        /* end of signature: after eight */
                                        /* characters, CR or TAB         */


                while((*actptr != 0x0A) && (*actptr != 0x09) && (n < 8))
                {
                    chr = *actptr;

                    for (m = 0; m < 8; m++)
                    Jedec.GALSig[n*8 + m] = (chr >> (7 - m)) & 0x1;

                    actptr++;                   /* increment pointer and */
                    n++;                        /* character-counter     */

                    if (actptr>buffend)         /* end of file ?   */
                    {                           /* yes, then error */

                        AsmError(2, 0);
                        return(-1);
                    }
                }


                                /*** get name of pins ***/

                                        /* clear flags for negations */
                                        /* in the pin declaration    */

                for (n = 0; n < 24; PinDecNeg[n++] = 0);



                pinnames = &PinNames[0][0]; /*assembler: pin names in PinNames*/

                               /* set flag 'not assembled' */

                GetNextLine();

            for (n = 0; n < num_of_pins; n++)
            {
                if (GetNextChar())              /* unexpected end of file? */
                {                               /* yes, then error         */
                    AsmError(2, 0);
                    return(-1);
                }

                m = 0;

                chr = *actptr;                  /* get character */

                if (IsNEG(chr))                 /* is there a negation? */
                {
                    max_chr      = 10;
                    PinDecNeg[n] = 1;           /* yes, then set flag */
                }
                else
                    max_chr = 9;

                if (!(isalpha(chr) || isdigit(chr) || IsNEG(chr)))
                {
                    AsmError(5, 0);             /* is character a legal */
                    return(-1);                 /* one?                 */
                }

                k = 0;

                while (isalpha(chr) || isdigit(chr) || IsNEG(chr))
                {
                    if (IsNEG(chr) && k != 0)    /* check position of '/' */
                    {
                        AsmError(10, 0);         /* must be at the beginning */
                        return(-1);              /* of the pin name          */
                    }

                    k = 1;

                    actptr++;

                    if (IsNEG(chr) && (!(isalpha(*actptr) || isdigit(*actptr))))
                    {
                        AsmError(3, 0);
                        return(-1);
                    }

                    *(pinnames + n*10 + m) = chr;

                    m++;

                    chr = *actptr;

                    if (m == max_chr)          /* check number of characters */
                    {                          /* in this pinname            */
                        AsmError(4, 0);
                        return(-1);            /* error: too many char. */
                    }

                }

                *(pinnames+n*10+m) = 0;         /* mark end of string */

                for (l = 0; l < n; l++)         /* pin name twice? */
                {
                    if (strcmp((char *)pinnames+l*10, "NC"))
                    {
                        i = j = 0;

                        if (IsNEG(*(pinnames+l*10)))  /* skip negation sign */
                            i = 1;

                        if (IsNEG(*(pinnames+n*10)))
                            j = 1;

                        if (!strcmp((char *)(pinnames+l*10+i),
                                    (char *)(pinnames+n*10+j)))
                        {
                            AsmError(9, 0);       /* pin name defined twice */
                            return(-1);
                        }
                    }
                }
                                                  /* is GND at the GND-pin? */
                if (!strcmp((char *)(pinnames + n*10), "GND"))
                {
                    if (n+1 != num_of_pins/2)
                    {
                        AsmError(6, 0);
                        return(-1);
                    }
                }

                if (n + 1 == num_of_pins/2)
                {
                    if (strcmp((char *)(pinnames + n*10), "GND"))
                    {
                        AsmError(8, 0);
                        return(-1);
                    }
                }
                                                /* is VCC at the VCC pin? */
                if (!strcmp((char *)(pinnames + n*10), "VCC"))
                {
                    if (n+1 != num_of_pins)
                    {
                        AsmError(6, 0);
                        return(-1);
                    }
                }

                if (n + 1 == num_of_pins)
                {
                    if (strcmp((char *)(pinnames + n*10), "VCC"))
                    {
                        AsmError(7, 0);
                        return(-1);
                    }
                }
                                        /* AR and SP are key words for 22V10 */
                                        /* they are not allowed in the pin */
                                        /* declaration */
                if (gal_type == GAL22V10)
                {
                    if (!strcmp((char *)(pinnames+n*10), "AR"))
                    {
                        AsmError(18, 0);
                        return(-1);
                    }

                    if (!strcmp((char *)(pinnames + n*10), "SP"))
                    {
                        AsmError(18, 0);
                        return(-1);
                    }
                }

            }


/* Boolean-Equations auswerten:
   Dabei werden die Boolean-Equations zweimal untersucht. Beim ersten
   Durchlauf werden die OLMC-Pins ausgewertet und die OLMC-Struktur ge-
   füllt. Mit Hilfe dieser Struktur läßt sich auf dem notwendigen Modus
   (1, 2 oder 3) schließen. Beim zweiten Durchlauf wird dann die
   Fuse-Matrix erstellt.
*/

/* Babelfish translation (slightly adapted):
   Boolean Equations evaluate:
   The Boolean Equations is twice examined.
   With the first run the OLMC pins are evaluated and the OLMC structure is filled.
   With the help of this structure the correct mode (1, 2 or 3) will be 
   calculated. With the second run the Fuse matrix is then provided.
*/
    if (GetNextChar())                  /* end of file? */
    {
        AsmError(2, 0);
        return(-1);
    }

                                        /* are there any equations? */
    if (!strncmp((char *)actptr, "DESCRIPTION", (size_t)11))
    {
        AsmError(33, 0);              /* no, then error */
        return(-1);
    }


    bool_start   = actptr;            /* set pointer to the beginning of    */
    bool_linenum = linenum;           /* the equations and save line number */


    for (pass = 0; pass < 2; pass++)  /* this is a two-pass-assembler */
    {
	printf("Assembler Phase %d for \"%s\"\n", (pass+1), file);

        if (pass)                       /* 2. pass? => make ACW and get */
        {                               /* the mode for 16V8,20V8 GALs  */
            modus = 0;
                                /*** GAL16V8, GAL20V8 ***/

            if (gal_type == GAL16V8 || gal_type == GAL20V8)
            {
                for (n = 0; n < 8; n++)            /* examine all OLMCs */
                {
                    if (OLMC[n].PinType == REGOUT) /* is there a registered  */
                    {                              /* OLMC?, then GAL's mode */
                        modus = MODE3;             /* is mode 3              */

                        Jedec.GALSYN = 0;      /* set SYN and AC0 for mode 3 */
                        Jedec.GALAC0 = 1;

                        break;
                    }
                }


                if (!modus)                     /* still no mode? */
                {
                    for (n = 0; n < 8; n++)    /* examine all OLMCs */
                    {
                        if (OLMC[n].PinType == TRIOUT) /* is there a tristate */
                        {                              /* OLMC?, then GAL's   */
                            modus = MODE2;             /* mode is mode 2      */

                            Jedec.GALSYN = 1;      /* set SYN, AC0 for mode 2 */
                            Jedec.GALAC0 = 1;

                            break;
                        }
                    }
                }


                if (!modus)           /* still no mode? */
                {
                                      /* if there is a violation of mode 1, */
                                      /* then use automatically mode 2      */
                    for (n = 0; n < 8; n++)
                    {
                        if (OLMC[n].PinType == INPUT)
                        {
                            if (gal_type == GAL16V8)
                            {
                                pin_num = n + 12;

                                if (pin_num == 15 || pin_num == 16)
                                {
                                    modus = MODE2;      /* mode 2 */

                                    Jedec.GALSYN = 1;   /* set SYN, AC0 bit */
                                    Jedec.GALAC0 = 1;

                                    break;
                                }
                            }

                            if (gal_type == GAL20V8)
                            {
                                pin_num = n + 15;

                                if (pin_num == 18 || pin_num == 19)
                                {
                                    modus = MODE2;      /* mode 2 */

                                    Jedec.GALSYN = 1;   /* set SYN, AC0 bit */
                                    Jedec.GALAC0 = 1;

                                    break;
                                }
                            }
                        }

                        /* output and feedback? then mode 2 */

                        if (OLMC[n].PinType == COM_TRI_OUT && OLMC[n].FeedBack)
                        {
                            modus = MODE2;      /* mode 2 */

                            Jedec.GALSYN = 1;   /* set SYN,  AC0 bit */
                            Jedec.GALAC0 = 1;

                            break;
                        }
                    }
                }

                if (!modus)             /* if there is still no mode */
                {                       /* defined, use mode 1 */
                    modus = MODE1;

                    Jedec.GALSYN = 1;       /* set SYN and AC0 bit */
                    Jedec.GALAC0 = 0;
                }



                /* If GAL's mode is mode 1, use all OLMCs which type is   */
                /* not defined explicitly as combinational outputs.       */
                /* If GAL's mode is mode 2 or 3, use all OLMCs which type */
                /* is not defined explicitly as tristate output which is  */
                /* always enabled */


                for (n = 0; n < 8; n++)         /* examine all OLMCs */
                {
                    if (OLMC[n].PinType == COM_TRI_OUT) /* is OLMC's type */
                    {                                   /* definded expl. */
                        if (modus == MODE1)  /* mode 1? then comb. output */
                            OLMC[n].PinType = COMOUT;
                        else
                        {
                            OLMC[n].PinType = TRIOUT;   /* mode 2, 3? then */
                                                        /* tri. output     */

                            OLMC[n].TriCon  = TRI_VCC;  /* tristate control */
                                                        /* = TRUE           */
                        }
                    }
                }
        
                /* make ACW; (SYN and AC0 are */
                /* defined already) */

                for (n = 0; n < PT_SIZE; n++)   /* set product term disable */
                    Jedec.GALPT[n] = 1;

                                        /* get AC1 bits */
                for (n = 0; n < AC1_SIZE; n++)
                {
                    if (OLMC[n].PinType == INPUT || OLMC[n].PinType == TRIOUT)
                        Jedec.GALAC1[AC1_SIZE - 1 - n] = 1;
                }

                for (n = 0; n < XOR_SIZE; n++)         /* get XOR bits */
                {
                    if (((OLMC[n].PinType == COMOUT) ||
                         (OLMC[n].PinType == TRIOUT) ||
                         (OLMC[n].PinType == REGOUT)) &&
                         (OLMC[n].Active  == ACTIVE_HIGH))
                            Jedec.GALXOR[XOR_SIZE - 1 - n] = 1;
                }

            }

            /*** GAL22V10 ***/
            if (gal_type == GAL22V10)
            {
                for (n = 0; n < 10; n++)
                {
                    if (OLMC[n].PinType == COM_TRI_OUT)  /* output can be */
                        OLMC[n].PinType = TRIOUT;        /* tristate or   */
                                                         /* register      */

                    if (((OLMC[n].PinType == COMOUT) ||
                        (OLMC[n].PinType == TRIOUT) ||
                        (OLMC[n].PinType == REGOUT)) &&
                        (OLMC[n].Active  == ACTIVE_HIGH))
                        Jedec.GALXOR[9 - n] = 1;

                    /* get AC1 bits (S1) */
                    if (OLMC[n].PinType == INPUT || OLMC[n].PinType == TRIOUT)
                        Jedec.GALS1[9 - n] = 1;
                }

            }

            /*** GAL20RA10 ***/
            if (gal_type == GAL20RA10)
            {
                                                /* get XOR bits (S0) */
                for (n = 0; n < 10; n++)
                {
                    if (OLMC[n].PinType == COM_TRI_OUT) /* output can be */
                        OLMC[n].PinType = TRIOUT;       /* tristate or   */
                                                        /* register      */

                    if (((OLMC[n].PinType == COMOUT) ||
                         (OLMC[n].PinType == TRIOUT) ||
                         (OLMC[n].PinType == REGOUT)) &&
                         (OLMC[n].Active  == ACTIVE_HIGH))
                        Jedec.GALXOR[9 - n] = 1;
                }
            }
        }

	if(pass)
		printf("GAL %s; Operation mode %d; Security fuse %s\n", GetGALName(gal_type), modus, cfg->JedecSecBit ? "on" : "off");

        actptr  = bool_start;
        linenum = bool_linenum;
        newline = linenum;

        goto label1;            /* Shit, don't blame me for the gotos.       */
                                /* I know, goto is a very bad command in C   */
                                /* and in the most other languages. But it   */
                                /* is very hard to remove them in this case. */

loop1:

        if (GetNextChar())                          /* end of file? */
        {
            AsmError(2, 0);
            return(-1);
        }

        suffix = SUFFIX_NON;

        if (*actptr == '.')         /* is there a suffix?       */
        {                           /* yes, then get the string */
            actptr++;

            if (gal_type == GAL22V10 &&
                (actPin.p_Pin == 24 || actPin.p_Pin == 25))
            {
                AsmError(39, 0);                /* no suffix allowed at */
                return(-1);                     /* AR and SP */
            }


            n = 0;
            while (isalpha(*actptr))            /* copy suffix string into */
            {                                   /* suffix array            */
                if (n < MAX_SUFFIX_SIZE)
                    suffix_strn[n++] = *actptr++;
                else
                {                               /* string too long, then */
                    AsmError(13, 0);            /* unknown suffix        */
                    return(-1);
                }
            }

            suffix_strn[n] = 0;                 /* mark end of string */

                 if (suffix_strn[0] == 'T')				suffix = SUFFIX_T;
            else if (suffix_strn[0] == 'R')				suffix = SUFFIX_R;
            else if (suffix_strn[0] == 'E')				suffix = SUFFIX_E;
            else if (!strcmp(&suffix_strn[0], "CLK"))	suffix = SUFFIX_CLK;
            else if (!strcmp(&suffix_strn[0], "ARST"))	suffix = SUFFIX_ARST;
            else if (!strcmp(&suffix_strn[0], "APRST"))	suffix = SUFFIX_APRST;
            else
            {
                AsmError(13, 0);    /* unknown suffix */
                return(-1);
            }

            /* check whether suffix is */
            /* allowed or not */
            if (gal_type != GAL20RA10)
            {
                switch (suffix)
                {
                    case SUFFIX_CLK:
                        AsmError(34, 0);            /* no .CLK allowed */
                        return(-1);
                        break;

                    case SUFFIX_ARST:
                        AsmError(35, 0);            /* .ARST is not allowed */
                        return(-1);
                        break;

                    case SUFFIX_APRST:
                        AsmError(36, 0);            /* .APRST is not allowed */
                        return(-1);
                        break;
                }
            }

            if (GetNextChar())                      /* end of file? */
            {
                AsmError(2, 0);
                return(-1);
            }
        }

        actOLMC = (int)actPin.p_Pin;                /* save offset of OLMC */

        if (gal_type == GAL16V8)
            actOLMC -= 12;
        else
        if (gal_type == GAL20V8)
            actOLMC -= 15;
        else
            actOLMC -= 14;


        row_offset = 0;                     /* offset for OR at OLMC*/
        prevOp     = 0;                     /* previous operator */

        if (!pass)                      /* is this pass 1? */
        {                               /* is pin a OLMC pin? */
            if (((gal_type == GAL16V8) &&
                 (actPin.p_Pin >= 12)  && (actPin.p_Pin <= 19)) ||
                ((gal_type == GAL20V8) &&
                 (actPin.p_Pin >= 15) && (actPin.p_Pin <= 22)) ||
                ((gal_type == GAL22V10) &&
                 (actPin.p_Pin >= 14) && (actPin.p_Pin <= DUMMY_OLMC12)) ||
                ((gal_type == GAL20RA10) &&
                 (actPin.p_Pin >= 14) && (actPin.p_Pin <= 23)))
            {

                switch (gal_type)           /* get OLMC number */
                {
                    case GAL16V8:
                        n = actPin.p_Pin - 12;
                        break;

                    case GAL20V8:
                        n = actPin.p_Pin - 15;
                        break;

                    case GAL22V10:
                    case GAL20RA10:
                        n = actPin.p_Pin - 14;
                        break;
                }



                switch (suffix)
                {

                    case SUFFIX_R:                /* output definition */
                    case SUFFIX_T:
                    case SUFFIX_NON:

                        if (!OLMC[n].PinType || OLMC[n].PinType == INPUT)
                        {

                            if (actPin.p_Neg)        /* get pin's activation */
                                OLMC[n].Active = ACTIVE_LOW;
                            else
                                OLMC[n].Active = ACTIVE_HIGH;

                            if (suffix == SUFFIX_T)
                                OLMC[n].PinType = TRIOUT;  /* tri. output */

                            if (suffix == SUFFIX_R)
                                OLMC[n].PinType = REGOUT;  /* reg. output */

                            if (suffix == SUFFIX_NON)    /* type of output is */
                                OLMC[n].PinType = COM_TRI_OUT; /* not defined */
                                                               /* explicitly  */
                        }
                        else
                        {
                            if (gal_type == GAL22V10 && (n == 10 || n == 11))
                            {
                                AsmError(40, 0);  /* AR or SP is defined */
                                return(-1);       /* twice               */
                            }
                            else
                            {
                                AsmError(16, 0);  /* pin is defined twice as */
                                return(-1);       /* output                  */
                            }
                        }

                        break;



                case SUFFIX_E:

                    if (actPin.p_Neg)       /* negation of the trisate */
                    {                       /* control is not allowed */
                        AsmError(19, 0);
                        return(-1);
                    }

                    if (OLMC[n].TriCon)     /* tri. control twice? */
                    {                       /* yes, then error */
                        AsmError(22, 0);
                        return(-1);
                    }

                    OLMC[n].TriCon = TRICON; /* set the flag that there is */
                                             /* a tri. control equation    */

                    if (!OLMC[n].PinType || OLMC[n].PinType == INPUT)
                    {
                        AsmError(17, 0);     /* the sequence must be output  */
                        return(-1);          /* followed by the tri. control */
                    }


                    if (OLMC[n].PinType == REGOUT &&
                        (gal_type == GAL16V8 || gal_type == GAL20V8))
                    {
                        AsmError(23, 0);  /* GAL16V8/20V8: tristate control */
                        return(-1);       /* for reg. output is not allowed */
                    }


                    if (OLMC[n].PinType == COM_TRI_OUT)
                    {                                   /* no tristate .T? */
                        AsmError(24, 0);                /* then error      */
                        return(-1);
                    }

                    break;



               case SUFFIX_CLK:

                    if (actPin.p_Neg)           /* negation of the .CLK   */
                    {                           /* control is not allowed */
                        AsmError(19, 0);
                        return(-1);
                    }

                    if (OLMC[n].PinType == NOTUSED)
                    {
                        AsmError(42, 0);        /* sequence must be: output */
                        return(-1);             /* def., .CLK definition */
                    }

                    if (OLMC[n].Clock)
                    {                           /* is .CLK defined twice? */
                        AsmError(45, 0);        /* yes, then error */
                        return(-1);
                    }

                    OLMC[n].Clock = 1;          /* set flag that there is */
                                                /* a .CLK equation        */
                    if (OLMC[n].PinType != REGOUT)
                    {
                        AsmError(48, 0);        /* no .CLK allowed when     */
                        return(-1);             /* output is not registered */
                    }

                    break;
        


                case SUFFIX_ARST:

                    if (actPin.p_Neg)
                    {                           /* negation of the .ARST  */
                        AsmError(19, 0);        /* control is not allowed */
                        return(-1);
                    }

                    if (OLMC[n].PinType == NOTUSED)
                    {
                        AsmError(43, 0);        /* sequence must be: output */
                        return(-1);             /* def., .ARST definition   */
                    }

                    if (OLMC[n].ARST)
                    {                           /* is .ARST defined twice? */
                        AsmError(46, 0);        /* yes, then error         */
                        return(-1);
                    }

                    OLMC[n].ARST = 1;           /* set flag that there is */
                                                /* a .ARST equation       */
                    if (OLMC[n].PinType != REGOUT)
                    {
                        AsmError(48, 0);        /* no .CLK allowed when     */
                        return(-1);             /* output is not registered */
                    }

                    break;



                case SUFFIX_APRST:

                    if (actPin.p_Neg)
                    {                           /* negation of the .APRST */
                        AsmError(19, 0);        /* control is not allowed */
                        return(-1);
                    }

                    if (OLMC[n].PinType == NOTUSED)
                    {
                        AsmError(44, 0);        /* sequence must be: output */
                        return(-1);             /* def., .APRST definition  */
                    }

                    if (OLMC[n].APRST)
                    {                           /* is .APRST defined twice? */
                        AsmError(47, 0);        /* yes, then error          */
                        return(-1);
                    }

                    OLMC[n].APRST = 1;          /* set flag that there is */
                                                /* a .APRST equation      */
                    if (OLMC[n].PinType != REGOUT)
                    {
                        AsmError(48, 0);        /* no .CLK allowed when     */
                        return(-1);             /* output is not registered */
                    }

                    break;
                }

            }
            else
            {
                AsmError(15, 0);                /* pin can't be programmed */
                return(-1);                     /* as output               */
            }
        }

	start_row = max_row = 0;

        switch (gal_type)
        {                                       /* get first the row of the */
            case GAL16V8:                       /* OLMC and the number of   */
            case GAL20V8:                       /* rows which areavailable  */
                start_row = ToOLMC[actOLMC];
                max_row   = 8;
                break;

            case GAL22V10:
                start_row = ToOLMC22V10[actOLMC];
                max_row   = OLMCSize22V10[actOLMC];
                break;

            case GAL20RA10:
                start_row = ToOLMC20RA10[actOLMC];
                max_row   = 8;
                break;
        }

        if (*actptr != '=')                     /* '=' ?          */
        {                                       /* no, then error */
            AsmError(14, 0);
            return(-1);
        }
loop2:
        actptr++;

        if (GetNextChar())
        {                                       /* end of file?    */
            AsmError(2, 0);                     /* yes, then error */
            return(-1);
        }

        oldptr = actptr;                            /* save pointer */

        IsPinName(pinnames, num_of_pins);

        if (gal_type == GAL22V10 && !actPin.p_Pin)
        {                               /* AR and SP is not allowed */
            Is_AR_SP(oldptr);           /* in terms of an equation  */

            if (actPin.p_Pin)
            {                           /* when used, then error */
                AsmError(31, 0);
                return(-1);
            }
        }

        if (!actPin.p_Pin)
        {                                   /* pin name?      */
            AsmError(11, 0);                /* no, then error */
            return(-1);
        }

        if (actPin.p_Pin == NC_PIN)
        {               /* NC used as pin name? */
            AsmError(12, 0);                          /* yes, then error */
            return(-1);
        }


        if (IsNEG(*(pinnames+(long)((actPin.p_Pin - 1)*10))))
            actPin.p_Neg = !actPin.p_Neg;       /* consider negation in the */
                                                /* pin declartion           */

        oldline = linenum;

        if (GetNextChar())
        {                                       /* end of file?    */
            AsmError(2, 0);                     /* yes, then error */
            return(-1);
        }

        newline = linenum;
        linenum = oldline;

        if (!pass)                              /* is this pass 1?*/
        {
            if (((gal_type == GAL16V8) &&       /* is this pin an OLMC pin? */
                 (actPin.p_Pin >= 12) && (actPin.p_Pin <= 19)) ||
                ((gal_type == GAL20V8) &&
                 (actPin.p_Pin >= 15) && (actPin.p_Pin <= 22)) ||
                ((gal_type == GAL22V10) &&
                 (actPin.p_Pin >= 14) && (actPin.p_Pin <= DUMMY_OLMC12)) ||
                ((gal_type == GAL20RA10) &&
                 (actPin.p_Pin >= 14) && (actPin.p_Pin <= 23)) )
            {


                switch (gal_type)                        /* get OLMC number */
                {
                    case GAL16V8:
                        n = actPin.p_Pin - 12;
                        break;

                    case GAL20V8:
                        n = actPin.p_Pin - 15;
                        break;

                    case GAL22V10:
                    case GAL20RA10:
                        n = actPin.p_Pin - 14;
                        break;
                }



                if (!OLMC[n].PinType)            /* is OLMC's type already */
                    OLMC[n].PinType = INPUT;     /* defined? no, then use  */
                                                 /* it as input            */
                OLMC[n].FeedBack = YES; /* if a OLMC pin is used within an */
            }                           /* equation, a feedback is needed  */
        }

                              /* in pass 2 we have to make the fuse matrix */
        if (pass)
        {
            switch (gal_type)                /* get row offset */
            {
                case GAL16V8:
                case GAL20V8:

                    if (suffix == SUFFIX_E)   /* when tristate control use */
                        row_offset = 0;       /* first row (=> offset = 0) */
                    else
                        if (!row_offset)  /*is offset of rows still equal 0?*/
                            if (modus != MODE1 &&
                                OLMC[actOLMC].PinType != REGOUT)
                                row_offset = 1;    /* then init. row-offset */

                    break;

                case GAL22V10:

                    if (suffix == SUFFIX_E)    /* enable is the first row */
                        row_offset = 0;        /* of the OLMC             */
                    else
                    {
                        if (actOLMC == 10 || actOLMC == 11)
                            row_offset = 0;     /* AR, SP?, then no offset */
                        else
                            if (!row_offset)     /* output starts at the     */
                                row_offset = 1;  /* second row => offset = 1 */
                    }

                    break;

                case GAL20RA10:
                    switch (suffix)
                    {
                        case SUFFIX_E:         /* enable is the first row */
                            row_offset = 0;    /* of the OLMC             */
                            break;

                        case SUFFIX_CLK:       /* Clock is the second row */
                            row_offset = 1;    /* of the OLMC             */
                            break;

                        case SUFFIX_ARST:      /* AReset is the third row */
                            row_offset = 2;    /* of the OLMC             */
                            break;

                        case SUFFIX_APRST:     /* APreset is the fourth row */
                            row_offset = 3;    /* of the OLMC               */
                            break;

                        default:                  /* output equation starts */
                            if (row_offset <= 3)  /* at the fifth row       */
                                row_offset = 4;
                    }
                    break;
            }

            pin_num = actPin.p_Pin;


            /* is there a valuation of GAL's mode? */

            if (gal_type == GAL16V8 || gal_type == GAL20V8)
            {

                if (modus == MODE2)               /* valuation of mode 2? */
                {
                    if (gal_type == GAL16V8 && (pin_num == 12 || pin_num == 19))
                    {
                        AsmError(20, 0);
                        return(-1);
                    }

                    if (gal_type == GAL20V8 && (pin_num == 15 || pin_num == 22))
                    {
                        AsmError(21, 0);
                        return(-1);
                    }
                }

                if (modus == MODE3)                /* valuation of mode 3? */
                {
                    if (gal_type == GAL16V8 && (pin_num == 1 || pin_num == 11))
                    {
                        AsmError(26, 0);
                        return(-1);
                    }

                    if (gal_type == GAL20V8 && (pin_num == 1 || pin_num == 13))
                    {
                        AsmError(27, 0);
                        return(-1);
                    }
                }
            }

            if (gal_type == GAL20RA10)       /* valuation of 20RA10? */
            {
                if (pin_num == 1)               /* pin 1 is reserved for */
                {                               /* /PL (preload)         */
                    AsmError(37, 0);
                    return(-1);
                }

                if (pin_num == 13)
                {                               /* pin 13 is reserved for */
                    AsmError(38, 0);            /* /OE (output enable)    */
                    return(-1);
                }
            }

            /* if GND, set row equal 0 */
            if (pin_num == num_of_pins || pin_num == num_of_pins/2)
            {
                if (actPin.p_Neg)
                {                         /* /VCC and /GND are not allowed */
                    AsmError(25, 0);
                    return(-1);
                }

                if (!prevOp && !IsAND(*actptr) && !IsOR(*actptr))
                {
                    if (pin_num == num_of_pins/2)
                    {
                        m = (start_row + row_offset) * num_of_col;
                                                      /* set row equal 0 */
                        for (n = m; n < m+num_of_col; Jedec.GALLogic[n++] = 0);

                    }
                }
                else
                {
                    AsmError(28, 0);
                    return(-1);
                }
            }
            else
            {

                if (suffix == SUFFIX_E || suffix == SUFFIX_CLK ||
                    suffix == SUFFIX_ARST || suffix == SUFFIX_APRST ||
                    (gal_type == GAL22V10 && (actOLMC == 10 || actOLMC == 11)))
                {

                    if (IsOR(prevOp))
                    {                           /* max. one product term   */
                        AsmError(29, 0);        /* for CLK, ARST, APRST, E */
                        return(-1);             /* and 22V10: AR, SP       */
                    }

                    SetAND(start_row + row_offset, pin_num, actPin.p_Neg, gal_type);
                }
                else
                {
                    if (IsOR(prevOp))
                    {                             /* OR operation? yes, then */
                        row_offset++;             /* take the next row       */

                        if (row_offset == max_row)
                        {        /* too many ORs?*/
                            AsmError(30, 0);
                            return(-1);
                        }
                    }
                                                           /* set ANDs */
                    SetAND(start_row + row_offset, pin_num, actPin.p_Neg, gal_type);
                }
            }


                                            /* are there any more terms? */
            if (!IsOR(*actptr) && !IsAND(*actptr) && suffix != SUFFIX_E &&
                suffix != SUFFIX_CLK && suffix != SUFFIX_ARST &&
                suffix != SUFFIX_APRST)
            {
                                                    /* no?, then set unused */
                row_offset++;                       /* rows of the OLMX     */
                                                    /* equal 0              */
                if (row_offset != max_row)
                {
                    m = (start_row + row_offset) * num_of_col;

                    for (n = m; n < m + (max_row - row_offset)*num_of_col; n++)
                        Jedec.GALLogic[n] = 0;

                }
            }
        }

        linenum = newline;

        if (IsOR(*actptr) || IsAND(*actptr))
        {
            prevOp = *actptr;
            goto loop2;
        }

        if (strncmp((char *)actptr, "DESCRIPTION", (size_t)11))
        {

label1:
            linenum = newline;

            oldptr = actptr;

            IsPinName(pinnames, num_of_pins);

            if (gal_type == GAL22V10 && !actPin.p_Pin)
            {                                       /* no pin name? then  */
                Is_AR_SP(oldptr);                   /* check whether name */
                                                    /* is AR or SP        */
                if (actPin.p_Pin && actPin.p_Neg)
                {                                   /* but no negation of */
                    AsmError(32, 0);                /* AR or SP           */
                    return(-1);
                }
            }
              
            if (!actPin.p_Pin)
            {                                       /* pin name?      */
                AsmError(11, 0);                    /* no, then error */
                return(-1);
            }

            if (actPin.p_Pin == NC_PIN)
            {                                       /* NC used as pin name? */
                AsmError(12, 0);                    /* yes, then error      */
                return(-1);
            }

            if (IsNEG(*(pinnames+(long)((actPin.p_Pin-1)*10))))
                actPin.p_Neg = !actPin.p_Neg; /* negation at pin declaration */

            goto loop1;

        }

    }


                        /* set fuse matrix of unused OLMCs and of OLMCs */
                        /* which are programmed as input equal 0        */

    for (n = 0; n < num_of_olmcs; n++)
    {
        if (OLMC[n].PinType == NOTUSED || OLMC[n].PinType == INPUT)
        {
		int i = 0;

            switch (gal_type)
            {                           /* get first row of the     */
                case GAL16V8:           /* OLMC and the number of   */
                case GAL20V8:           /* rows which are available */
                    l = ToOLMC[n];
                    i = 8;
                    break;

                case GAL22V10:
                    l = ToOLMC22V10[n];
                    i = OLMCSize22V10[n];
                    break;

                case GAL20RA10:
                    l = ToOLMC20RA10[n];
                    i = 8;
                    break;
            }

            l = l * num_of_col;

            m = l + i * num_of_col;

            for (k = l; k < m; k++)
                Jedec.GALLogic[k] = 0;
        }
    }


    if (gal_type == GAL22V10)
    {                                   /* if AR or SP is not defined,   */
                                        /* set corresponding row equal 0 */
        if (!OLMC[10].PinType)                  /* set row of AR equal 0 */
            for (n = 0; n < num_of_col; Jedec.GALLogic[n++] = 0);

        if (!OLMC[11].PinType)                         /* set row of SP equal 0 */
            for (n = 5764; n < 5764 + num_of_col; Jedec.GALLogic[n++] = 0);
    }


    if (gal_type == GAL20RA10)
    {                                           /* set unused CLK, ARST */
                                                /* and APRST equal 0    */
        for (n = 0; n < num_of_olmcs; n++)      /* examine all OLMCs    */
        {
            if (OLMC[n].PinType != NOTUSED)     /* is OLMC used? */
            {
                if (OLMC[n].PinType == REGOUT && !OLMC[n].Clock)
                {
                    AsmError(41, n + 14);       /* register output        */
                    return(-1);                 /* needs clock definition */
                }

                if (!OLMC[n].Clock)
                {                                      /* is clock unused? */
                    l = (ToOLMC20RA10[n] + 1) * num_of_col;  /* then clear */
                                                             /* the row    */
                    for (k = l; k < l + num_of_col; k++)
                        Jedec.GALLogic[k] = 0;
                }

                if (OLMC[n].PinType == REGOUT)
                {
                    if (!OLMC[n].ARST)
                    {                  /* is ARST unused? */
                        l = (ToOLMC20RA10[n] + 2) * num_of_col;

                        for (k = l; k < l + num_of_col; k++)
                            Jedec.GALLogic[k] = 0;
                    }

                    if (!OLMC[n].APRST)
                    {                               /* is APRST unused? */
                        l = (ToOLMC20RA10[n] + 3) * num_of_col;

                        for (k = l; k < l + num_of_col; k++)
                            Jedec.GALLogic[k] = 0;
                    }
                }

            }

        }

    }


/* now the JEDEC structure is ready */
/* (be happy, it was a hard task)   */



                     /* set flag, so that we can see that  */
                                      /* this file is assembled succesfully */

					free(fbuff);



                                      /*** now make the selected files ***/

					/* Obtain the filename without the extension */
					{
						char *base; int l;


						if((base = GetBaseName(file)))
						{
							#define extman(p,l,a,b,c) { p[l-2] = a; base[l-1] = b; base[l-0] = c; }

							l = (strlen(base) - 1);

							base[l-3] = '.';

							extman(base,l,'j','e','d');	
							WriteJedecFile(base, gal_type, cfg);

							extman(base,l,'f','u','s');	
    	                    if(cfg->GenFuse) WriteFuseFile(base, gal_type);

							extman(base,l,'p','i','n');	
        	                if(cfg->GenPin ) WritePinFile (base, gal_type);

							extman(base,l,'c','h','p');	
            	            if(cfg->GenChip) WriteChipFile(base, gal_type);

							free(base);

						}
						else
						{
							ErrorReq(2);
							return(-2);
						}

                        return(0);                 /* there was no error */

					}
                }
                else
                {
                    ErrorReq(3);                          /* read error */
					free(fbuff);
                    return(-2);
                }
            }
            else
            {
                ErrorReq(2);                            /* no more free memory */
                return(-2);
            }
        }
}





/******************************************************************************
** SetAND()
*******************************************************************************
** input:   row         row in which the AND should be set
**          pinnum      pin which should be ANDed
**          negation    0: pin without negation
**                      1: pin with negation sign (/)
**
** output:  none
**
** remarks: sets an AND (=0) in the fuse matrix
******************************************************************************/
 
void SetAND(int row, int pinnum, int negation, int gal_type)
{
	int column = 0;
	int numofcol = 0;

    switch (gal_type)
    {
        case GAL16V8:
            if (modus == MODE1)
                column = PinToFuse16Mode1[pinnum - 1];
            if (modus == MODE2)
                column = PinToFuse16Mode2[pinnum - 1];
            if (modus == MODE3)
                column = PinToFuse16Mode3[pinnum - 1];

			numofcol = MAX_FUSE_ADR16	+ 1;
            break;

        case GAL20V8:
            if (modus == MODE1)
                column  =  PinToFuse20Mode1[pinnum - 1];
            if (modus == MODE2)
                column  =  PinToFuse20Mode2[pinnum - 1];
            if (modus == MODE3)
                column  =  PinToFuse20Mode3[pinnum - 1];

			numofcol = MAX_FUSE_ADR20 + 1;
            break;

        case GAL22V10:
            column  =  PinToFuse22V10[pinnum - 1];

                                    /* is it a registered OLMC pin?   */
                                    /* yes, then correct the negation */
            if ((pinnum >= 14 && pinnum <= 23) && !Jedec.GALS1[23 - pinnum])
            {
                negation = negation ? 0 : 1;
            }

			numofcol = MAX_FUSE_ADR22V10 + 1;
            break;

        case GAL20RA10:
            column  =  PinToFuse20RA10[pinnum - 1];
			numofcol = MAX_FUSE_ADR20RA10	+ 1;
	        break;
    }

    Jedec.GALLogic[row*numofcol + column + negation] = 0;

}





/******************************************************************************
** IsPinName()
*******************************************************************************
** input:   *pinnames   pointer to the pinnames array
**          numofpins   number of pins (20 or 24, depends on the type of GAL)
**
**  global: actptr          pointer to the first character of the pinname
**          actPin.p_Pin:   number of pin or NC_PIN; 0: no pin
**          actPin.p_Neg:   pinname with '/' = 1;  without '/' = 0
**
** output:  none
**
** remarks: This function tests whether actptr points to a pinname or not
******************************************************************************/
 
void IsPinName(UBYTE *pinnames, int numofpins)
{
    int     i, k, n;
    UBYTE   *oldactptr;


    actPin.p_Neg = 0;                   /* install structure for pin */
    actPin.p_Pin = 0;

    if (IsNEG(*actptr))
    {                                   /* negation? */
        actptr++;
        actPin.p_Neg = 1;
    }

    n = 0;                                /* get length of pin name */

    oldactptr = actptr;

    while (isalpha(*actptr) || isdigit(*actptr))
    {
        actptr++;
        n++;
    }

    if (n) {
        if ((n == 2 ) && !strncmp((char *)oldactptr, "NC", (size_t)2))
            actPin.p_Pin = NC_PIN;                      /* NC pin*/
        else
            for (k = 0; k < numofpins; k++)
            {                           /* examine whole list of pin names */
                i = 0;

                if (IsNEG(*(pinnames+k*10)))
                    i = 1;

                                         /* are the string sizes equal? */
                if (n == strlen((char *)(pinnames+k*10+i)))
                    if (!(strncmp((char *)oldactptr, (char *)(pinnames+k*10+i),
                          (size_t)n)))   /* yes, then compare these strings */
                {
                    actPin.p_Pin = k + 1;
                    break;
                }
            }
    }
}





/******************************************************************************
** Is_AR_SP()
*******************************************************************************
** input:   *ptr    pointer to the first character of the pinname
**
** output:  none
**          global  actPin.p_Pin: 23: AR, 24: SP, 0: no AR, SP
**                  actPin.p_Neg: pinname with '/' = 1;  without '/' = 0
**
** remarks: This function tests whether actptr points to a AR or SP
******************************************************************************/

void Is_AR_SP(UBYTE *ptr)
{
    int     n;
    UBYTE   *oldptr;


    actPin.p_Neg = 0;                     /* install structure for pin */
    actPin.p_Pin = 0;

    if (IsNEG(*ptr))
    {                    /* negation? */
        ptr++;
        actPin.p_Neg = 1;
    }

    n = 0;                                /* get length of pin name */

    oldptr = ptr;

    while (isalpha(*ptr) || isdigit(*ptr))
    {
        ptr++;
        n++;
    }
                        /* assign AR to "OLMC 11" ("pin 24") and */
    if (n)              /* assign SP to "OLMC 12" ("pin 25")     */
    {
        if ((n == 2 ) && !strncmp((char *)oldptr, "AR", (size_t)2))
            actPin.p_Pin = DUMMY_OLMC11;

        if ((n == 2 ) && !strncmp((char *)oldptr, "SP", (size_t)2))
            actPin.p_Pin = DUMMY_OLMC12;
    }
}





/******************************************************************************
** GetNextChar()
*******************************************************************************
** input:   none
**
** output:  0: character found, actptr points to it
**          1: no character found
**
** remarks: searchs the next character which is no comment, space, TAB, LF
******************************************************************************/

int GetNextChar(void)
{

    for(;;)
    {
        switch (*actptr)
        {
            case 0x0A:                           /* LineFeed */
                actptr++;
                linenum++;
                break;

            case ' ':                            /* space */
            case 0x09:                           /* TAB   */
                actptr++;
                break;

            case ';':                            /* comment found?         */
                if (GetNextLine())               /* then skip rest of line */
                    return(0);
                break;

            default:
                                                 /* was there a character? */
                if (*actptr > ' ' && *actptr <= '~')
                    return(0);
                else
                    actptr++;
        }

        if (actptr > buffend)                    /* end of file? */
            return(1);
    }
}




 
/******************************************************************************
** GetNextLine()
*******************************************************************************
** input:   none
**
** output:  0: line found, actptr points to this line
**          1: end of file reached
**
** remarks: gets pointer to next line
******************************************************************************/

int GetNextLine(void)
{

    for(;;)
    {
        if (*actptr == 0x0A)
        {
            actptr++;
            linenum++;
            return(0);
        }

        if (actptr > buffend)                       /* end of file? */
            return(1);

        actptr++;
    }
}






/******************************************************************************
** IsOR()
*******************************************************************************
** input:   none
**
** output:  1: chr is a OR
**          0: chr is no OR
**
** remarks: checks whether or not chr is a OR sign or not
******************************************************************************/

int IsOR(char chr)
{
    if (chr == '+' || chr == '#')
        return(1);
    else
        return(0);
}





/******************************************************************************
** IsAND()
*******************************************************************************
** input:   none
**
** output:  1: chr is a AND
**          0: chr is no AND
**
** remarks: checks whether or not chr is a AND sign or not
******************************************************************************/

int IsAND(char chr)
{
    if (chr == '*' || chr == '&')
        return(1);
    else
        return(0);
}





/******************************************************************************
** IsNEG()
*******************************************************************************
** input:   none
**
** output:  1: chr is a negation sign
**          0: chr is no negation sign
**
** remarks: checks whether or not chr is a negation sign or not
******************************************************************************/

int IsNEG(char chr)
{
    if (chr == '/' || chr == '!')
        return(1);
    else
        return(0);
}





/********************************************************/
/* the following routines are for the creation of the   */
/* documentation files                                  */
/********************************************************/



int GetPinNum(int gal_type)
{
	if(gal_type == GAL16V8) return(20);

	return(24);
}


/******************************************************************************
** WriteChipFile(char *filename, int gal_type)
*******************************************************************************
** input:   gal type
**			filename 
**
** output:  none
**
** remarks: make chip file
******************************************************************************/

void WriteChipFile(char *filename, int gal_type)
{
    FILE    *fp;
    int     n;

	int num_of_pins = GetPinNum(gal_type);

        if ((fp = fopen(filename, (char *)"w")))
        {

            fprintf(fp, "\n\n");

            WriteSpaces(fp, 31);

            if (gal_type == GAL16V8)
                fprintf(fp, " GAL16V8\n\n");

            if (gal_type == GAL20V8)
                fprintf(fp, " GAL20V8\n\n");

            if (gal_type == GAL22V10)
                fprintf(fp, " GAL22V10\n\n");

            if (gal_type == GAL20RA10)
                fprintf(fp, "GAL20RA10\n\n");


            WriteSpaces(fp, 26);

            fprintf(fp,"-------\\___/-------\n");

            for (n = 0; n < num_of_pins/2; n++)
            {

                WriteSpaces(fp, 25 - (int)strlen((char *)(pinnames+n*10)));

                fprintf(fp,"%s | %2d           %2d | %s\n", pinnames + n*10,
                        n+1, num_of_pins-n, pinnames+(num_of_pins-n-1)*10);

                if (n < num_of_pins/2 - 1)
                {
                    WriteSpaces(fp, 26);

                    fprintf(fp, "|                 |\n");
                }
            }

            WriteSpaces(fp, 26);

            fprintf(fp, "-------------------\n");

            if (fclose(fp) == EOF)
            {
                ErrorReq(8);                   /* can't close file */
                return;
            }
        }
}



/******************************************************************************
** WritePinFile(char é*filename, int gal_type)
*******************************************************************************
** input:   gal type
**			filename
**
** output:  none
**
** remarks: make pin file
******************************************************************************/

void WritePinFile(char *filename, int gal_type)
{
    FILE    *fp;
    int     k, n, flag;

	int num_of_pins = GetPinNum(gal_type);

        if ((fp = fopen(filename, (char *)"w")))
        {
            fprintf(fp, "\n\n");

            fprintf(fp, " Pin # | Name     | Pin Type\n");

            fprintf(fp, "-----------------------------\n");

            for (n = 1; n <= num_of_pins; n++)
            {
                fprintf(fp,"  %2d   | ",n);

                fprintf(fp,"%s",pinnames+(n-1)*10);

                WriteSpaces(fp, 9-(int)strlen((char *)(pinnames+(n-1)*10)));

                flag = 0;

                if (n == num_of_pins/2)
                {
                    fprintf(fp,"| GND\n");
                    flag = 1;
                }

                if (n == num_of_pins)
                {
                    fprintf(fp,"| VCC\n\n");
                    flag = 1;
                }


                if (gal_type == GAL16V8 || gal_type == GAL20V8)
                {

                    if (modus == MODE3 && n == 1)
                    {
                        fprintf(fp, "| Clock\n");
                        flag = 1;
                    }

                    if (modus == MODE3)
                    {
                        if (gal_type == GAL16V8 && n == 11)
                        {
                            fprintf(fp, "| /OE\n");
                            flag = 1;
                        }

                        if (gal_type == GAL20V8 && n == 13)
                        {
                            fprintf(fp, "| /OE\n");
                            flag = 1;
                        }
                    }
                }

                if (gal_type == GAL22V10 && n == 1)
                {
                    fprintf(fp, "| Clock/Input\n");
                    flag = 1;
                }

                                                        /* OLMC pin?*/

                if ((gal_type == GAL16V8   && n >= 12 && n <= 19) ||
                    (gal_type == GAL20V8   && n >= 15 && n <= 22) ||
                    (gal_type == GAL20RA10 && n >= 14 && n <= 23) ||
                    (gal_type == GAL22V10  && n >= 14 && n <= 23))
                {


                    if (gal_type == GAL16V8)
                        k = n - 12;
                    else
                        if (gal_type == GAL20V8)
                            k = n - 15;
                        else
                            k = n - 14;

                    if (OLMC[k].PinType != INPUT)
                        if (OLMC[k].PinType)
                            fprintf(fp,"| Output\n");
                        else
                            fprintf(fp,"| NC\n");
                    else
                        fprintf(fp,"| Input\n");
                }
                else
                {
                    if (!flag)
                        fprintf(fp,"| Input\n");
                }
            }

            if (fclose(fp) == EOF)
            {
                ErrorReq(8);                           /* can't close file */
                return;
            }
        }
        else
        {
            ErrorReq(13);
            return;
        }

}

/******************************************************************************
** WriteRow()
*******************************************************************************
** input:   *fp     pointer to the file handle
**          row     number of row which should be written
**
** output:  none
**
** remarks: writes a row of an OLMC to the file characterized by the file
**          handle fp
******************************************************************************/
 
void WriteRow(FILE *fp, int row, int num_of_col)
{
    int col;

    fprintf(fp, "\n%3d ", row);                 /* print row number */

    for (col = 0; col < num_of_col; col++)
    {                                           /* print fuses of */
        if (!((col) % 4))                       /* a row          */
            fprintf(fp, " ");

        if (Jedec.GALLogic[row*num_of_col + col])
            fprintf(fp, "-");
        else
            fprintf(fp, "x");
    }
}

/******************************************************************************
** WriteChipFile(char *filename, int gal_type)
*******************************************************************************
** input:   gal type
**			filename
**
** output:  none
**
** remarks: make fuse file
******************************************************************************/

void WriteFuseFile(char *filename, int gal_type)
{
    FILE    *fp;
    int     row, pin, n, numofOLMCs, numofrows, olmc;

	int num_of_col = 0;

	switch(gal_type)
	{
		case GAL16V8: 	num_of_col = MAX_FUSE_ADR16	+ 1;
		case GAL20V8: 	num_of_col = MAX_FUSE_ADR20 + 1;
		case GAL20RA10: num_of_col = MAX_FUSE_ADR20RA10	+ 1;
		case GAL22V10:  num_of_col = MAX_FUSE_ADR22V10 + 1;
	}

        if ((fp = fopen(filename, (char *)"w")))
        {
            if (gal_type == GAL16V8)
            {
                pin = 19;
                numofOLMCs = 8;
            }
            else
                if (gal_type == GAL20V8)
                {
                    pin = 22;
                    numofOLMCs = 8;
                }
                else
                {                                  /* 22V10, 20RA10 */
                    pin = 23;
                    numofOLMCs = 10;
                }



            row = 0;

            for (olmc = 0; olmc < numofOLMCs; olmc++)
            {

                if (gal_type == GAL22V10 && olmc == 0)
                {                               /* AR when 22V10 */
                    fprintf(fp, "\n\nAR");
                    WriteRow(fp, row, num_of_col);
                    row++;
                }

                if (gal_type == GAL22V10)             /* get number of rows */
                    numofrows = OLMCSize22V10[olmc];  /* of an OLMC         */
                else
                    numofrows = 8;



                fprintf(fp, "\n\nPin %2d = ", pin);             /* print pin */

                fprintf(fp, "%s", pinnames + (pin - 1)*10);

                WriteSpaces(fp, 13-(int)strlen((char *)(pinnames+(pin-1)*10)));


                if (gal_type == GAL16V8)
                    fprintf(fp, "XOR = %1d   AC1 = %1d", Jedec.GALXOR[19-pin],
                            Jedec.GALAC1[19 - pin]);
                else
                    if (gal_type == GAL20V8)
                        fprintf(fp, "XOR = %1d   AC1 = %1d",
                                Jedec.GALXOR[22 - pin], Jedec.GALAC1[22 - pin]);
                    else
                        if (gal_type == GAL22V10)
                            fprintf(fp, "S0 = %1d   S1 = %1d",
                                    Jedec.GALXOR[23 - pin],
                                    Jedec.GALAC1[23 - pin]);
                        else
                            if (gal_type == GAL20RA10)
                                fprintf(fp, "S0 = %1d",
                                        Jedec.GALXOR[23 - pin]);



                for (n = 0; n < numofrows; n++)
                {                           /* print all fuses of an OLMC */
                    WriteRow(fp, row, num_of_col);
                    row++;
                }


                if (gal_type == GAL22V10 && olmc == 9)
                {                                        /* SP when 22V10 */
                    fprintf(fp, "\n\nSP");
                    WriteRow(fp, row, num_of_col);
                }

                pin--;
            }


            fprintf(fp, "\n\n");

            if (fclose(fp) == EOF)
            {
                ErrorReq(8);                            /* can't close file */
                return;
            }
        }
        else
        {
            ErrorReq(13);
            return;
        }

}

/******************************************************************************
** WriteSpaces()
*******************************************************************************
** input:   *fp         pointer to the file handle of the file
**          numof       number of spaces to be written to the file
**
** output:  none
**
** remarks: write "numof" spaces to the file characterized by *fp
******************************************************************************/

void WriteSpaces(FILE *fp, int numof)
{
    int n;

    for (n = 0; n < numof; n++)
        fprintf(fp, " ");

}



/******************************************************************************
** AsmError()
*******************************************************************************
** input:   errornum    number of error to be printed
**          pinnum      = 0: print "Error in line linnum:" ...
**                      > 0: print "Pin pinnum:" ...
**
** output:  none
**
** remarks: print error messages of the GAL-assembler and free
**          the memory allocated by the file buffer
******************************************************************************/
 
void AsmError(int errornum, int pinnum)
{
    free(fbuff);

    if (!pinnum)
        printf("Error in line %d: ", linenum);
    else
		printf("Error, pin %d: ", pinnum);

	printf("%s\n", AsmErrorArray[errornum]);
} 
 
/******************************************************************************
** main 
*******************************************************************************
** 
** options:
** 
** -s Enable security fuse
** -c Disable .chp file output
** -f Disable .fus file output
** -p Disable .pin file output
** -a Restrict checksum to the fuse array only
**
**
** 
******************************************************************************/


int main(int argc, char *argv[])
{
	int rc;
  	char *p;

	struct Config cfg;

	cfg.GenFuse 		= TRUE;
	cfg.GenChip 		= TRUE;
	cfg.GenPin 	 	= TRUE;
	cfg.JedecSecBit 	= FALSE;
	cfg.JedecFuseChk 	= FALSE;

	p = argv[1];

	printf( "GALasm 2.1, Portable GAL Assembler\n"
			"Copyright (c) 1998-2003 Alessandro Zummo. All Rights Reserved\n"
			"Original sources Copyright (c) 1991-96 Christian Habermann\n\n");


  	while(argc > 1 && (p[0] == '-' || (isalpha(p[1]) && (argc != 2)))) 
	{
    	switch(p[1]) 
		{
      		case 's':
			case 'S':
				cfg.JedecSecBit = TRUE;
			break;

      		case 'c':
			case 'C':
				cfg.GenChip = FALSE;
			break;

      		case 'f':
			case 'F':
				cfg.GenFuse = FALSE;
			break;

      		case 'p':
			case 'P':
				cfg.GenPin = FALSE;
			break;

      		case 'a':
			case 'A':
				cfg.JedecFuseChk = TRUE;
			break;

			case 'h':
			case 'H':
			case '?':
	    		printf("Usage:\nGALasm [-scfpa] <filename>\n");
				printf(	"-s Enable security fuse\n"
						"-c Do not create the .chp file\n"
						"-f Do not create the .fus file\n"
						"-p Do not create the .pin file\n"
						"-a Restrict checksum to the fuse array only\n");
				return(0);

      		case '-': 
      		case '\0':
			argc--;
			argv++;
			goto opt_done;

      		default:
				goto usage;
    	}
		

		if(!isalpha(p[2]))
		{
	    	argc--;
    		argv++;

			p = argv[1];
		}
		else
			p++;
  	}


	opt_done:

  	if(argc != 2) 
	{
		usage:
    		printf("Usage:\nGALasm [-scfpa] <filename>\n");
			printf("Type GALasm -h for help\n");
		return(5);
  	}


	rc = AssemblePldFile(argv[1], &cfg);

	if(rc != 0)
		printf("Assembling failed.\n");
	else
		printf("Assembling successfully completed.\n");	

	return(rc);
}
