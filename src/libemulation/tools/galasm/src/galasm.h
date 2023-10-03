/******************************************************************************
** GALasm.h
*******************************************************************************
**
** description:
** This file includes some definitions and structures used by GALasm.
**
********************************************************************************/


/****************************** include files ********************************/

#ifndef  __STDIO_H
#include <stdio.h>
#endif

/******************************* definitions *********************************/

#define GAL16V8         1               /* GAL types */
#define GAL20V8         2
#define GAL22V10        3
#define GAL20RA10       4
#define UNKNOWN         5
#define NOT_SPECIFIED   6

#define YES             1
#define NO              0

#define LOW             0
#define HIGH            1
#define INPUT_PIN       0
#define OUTPUT_PIN      1

		                                /* GAL16V8 */
#define LOGIC16         0               /* location of the fuses */
#define XOR16           2048            /* in the JEDEC file     */
#define SIG16           2056
#define AC116           2120
#define PT16            2128
#define SYN16           2192
#define AC016           2193
#define NUMOFFUSES16    2194
                                        /* GAL20V8 */
#define LOGIC20         0               /* location of the fuses */
#define XOR20           2560            /* in the JEDEC file     */
#define SIG20           2568
#define AC120           2632
#define PT20            2640
#define SYN20           2704
#define AC020           2705
#define NUMOFFUSES20    2706

                                        /* GAL22V10 */
#define NUMOFFUSES22V10 5892            /* location of the fuses */
#define XOR22V10        5808            /* in the JEDEC file     */
#define SYN22V10        5809
#define SIG22V10        5828

                                        /* GAL20RA10 */
#define NUMOFFUSES20RA10 3274           /* location of the fuses */
#define XOR20RA10        3200           /* in the JEDEC file     */
#define SIG20RA10        3210


#define LOGIC16_SIZE    2048            /* number of bits for XOR etc. */
#define LOGIC20_SIZE    2560
#define LOGIC22V10_SIZE 5808
#define LOGIC20RA10_SIZE 3200
#define ROW_SIZE_16V8   64
#define ROW_SIZE_20V8   64
#define ROW_SIZE_22V10  132
#define ROW_SIZE_20RA10 80
#define XOR_SIZE        8
#define SIG_SIZE        64
#define AC1_SIZE        8
#define PT_SIZE         64
#define SYN_SIZE        1
#define AC0_SIZE        1
#define ACW_SIZE        82              /* architecture control word (ACW) */

#define MAX_FUSE_ADR16          31      /* addresses of the GALs */
#define SIG_ADR16               32      /* (fuer Fan-Post :-))   */
#define MAX_FUSE_ADR20          39
#define SIG_ADR20               40
#define MAX_FUSE_ADR22V10       43
#define SIG_ADR22V10            44
#define MAX_FUSE_ADR20RA10      39
#define SIG_ADR20RA10           40
#define ACW_ADR                 60
#define SECURITY_ADR            61
#define ERASE_ADR               63

 
                                  /* output's polarity: */
#define ACTIVE_LOW      0             /* pin is high-active */
#define ACTIVE_HIGH     1             /* pin is low-active  */

                                  /* type of the pin: */
#define NOTUSED         0             /* pin not used up to now */
#define NOTCON          0             /* pin not used           */
#define INPUT           2             /* input                  */
#define COMOUT          3             /* combinational output   */
#define TRIOUT          4             /* tristate output        */
#define REGOUT          5             /* register output        */
#define COM_TRI_OUT     6             /* either tristate or     */
                                      /* combinational output   */

                                  /* tristate control: */
#define NO_TRICON       0             /* no tristate control defined       */
#define TRICON          1             /* tristate control defined          */
#define TRI_VCC         2             /* permanent low  impedance          */
#define TRI_GND         3             /* permanent high impedance          */
#define TRI_PRO         4             /* tristate control via product term */


#define NC_PIN          30

#define MODE1           1               /* modes (SYN, AC0) */
#define MODE2           2
#define MODE3           3


#define MAX_SUFFIX_SIZE 6               /* max. string length of a legal */
                                        /* suffix */

#define ENTRY_SIZE      256             /* number of entries per buffer */

#define SIZE_OF_EQUASTRING 80
 
typedef short		BOOL;


typedef void	       *APTR;	    /* 32-bit untyped pointer */

typedef long			LONG;	    /* signed 32-bit quantity */
typedef unsigned long	ULONG;	    /* unsigned 32-bit quantity */
typedef unsigned long	LONGBITS;   /* 32 bits manipulated individually */
typedef short			WORD;	    /* signed 16-bit quantity */
typedef unsigned short	UWORD;	    /* unsigned 16-bit quantity */

typedef signed char		BYTE;	    /* signed 8-bit quantity */
typedef unsigned char	UBYTE;	    /* unsigned 8-bit quantity */

typedef unsigned char  *STRPTR;     /* string pointer (NULL terminated) */

#ifndef TRUE
#define TRUE		1
#endif
#ifndef FALSE
#define FALSE		0
#endif
#ifndef NULL
#define NULL		0L
#endif


/******************************** structures *********************************/

                                        /* this structure is used to store   */
                                        /* GALasm's configuration             */
struct  Config 
{ 
	BOOL GenFuse;          /* generate fuse file?        */
    BOOL GenChip;          /* generate chip file?        */
    BOOL GenPin;           /* generate pin file?         */
    BOOL JedecSecBit;      /* set security bit in JEDEC? */
    BOOL JedecFuseChk;     /* calc. fuse checksum?       */ /* azummo: if false, file checksum will be generated */
};


                                        /* this structure is used to store */
                                        /* the fuses in a kind of JEDEC    */
                                        /* format                          */

struct  JedecStruct { BYTE GALLogic[5808];      /*max. size of fuse matrix */
                      BYTE GALXOR  [10];        /* XOR bits                */
                      BYTE GALSig  [64];        /* signature               */
                      BYTE GALAC1  [8];         /* AC1 bits                */
                      BYTE GALPT   [64];        /* product term disable    */
                      BYTE GALSYN;              /* SYN bit                 */
                      BYTE GALAC0;              /* AC0 bit                 */
                      BYTE GALS1   [10];        /* S1 bits for 22V10       */
                    };
                    

                                        /* used to store infos about a pin */
struct  Pin             { BYTE p_Neg;        /* polarity of pin */
                          BYTE p_Pin;        /* pin number      */
                        };

                                        /* used to store infos about an OLMC */

struct  GAL_OLMC        { BYTE Active;       /* output's polarity           */
                          BYTE PinType;      /* type of pin (input,...)     */
                          BYTE TriCon;       /* user def. tristate control? */
                          BYTE Clock;        /* user def. clock equation?   */
                          BYTE ARST;         /* user def. ARST equation?    */
                          BYTE APRST;        /* user def. APRST equation?   */
                          BYTE FeedBack;     /* is there a feedback?        */
                        };

                                        /* this structure is used to store  */
                                        /* some datas in a chained list     */
                                        /* e.g. the coded equations for the */
                                        /* optimizer                        */

struct  Buffer  { struct Buffer *Next;
                  struct Buffer *Prev;
                  UBYTE  Entries[ENTRY_SIZE];   /* data area */
                };
                                                    
                                        /* used to store results and     */
                                        /* parameters of functions       */
                                        /* which deal with chained lists */

struct  ActBuffer  { struct Buffer *ThisBuff;  /* pointer to current buffer */
                     UBYTE       *Entry;       /* pointer to data area      */
                     UBYTE       *BuffEnd;     /* pointer to the end of the */
                   };                          /* buffer                    */






/********************************** globals **********************************/

extern struct JedecStruct Jedec;
extern char *ErrorArray[];
extern char *AsmErrorArray[];

/*************************** function declartions ****************************/


/*GALasm .c */
void  SetAND(int row, int pinnum, int negation, int gal_type);
void  IsPinName(UBYTE *pinnames, int numofpins);
int   GetNextChar(void);
int   GetNextLine(void);
void  AsmError(int errornum, int pinnum);
void  WriteChipFile(char *filename, int gal_type);
void  WritePinFile(char *filename, int gal_type);
void  WriteFuseFile(char *filename, int gal_type);
void  WriteSpaces(FILE *fp, int numof);
void  WriteRow(FILE *fp, int row, int num_of_col);
int   IsOR(char);
int   IsAND(char);
int   IsNEG(char);
void  Is_AR_SP(UBYTE *ptr);


/* support.c */
char *GetBaseName(char *filename);
int FileSize(char *filename);
int   ReadFile(char *filename, int filesize, char *filebuff);
int   AddByte(struct ActBuffer *buff, UBYTE code);
int   AddString(struct ActBuffer *buff, UBYTE *strnptr);
void  IncPointer(struct ActBuffer *buff);
void  DecPointer(struct ActBuffer *buff);
void  FreeBuffer(struct Buffer *buff);
char *GetGALName(int galtype);
void  ErrorReq(int errornum);

/* Jedec.c */
int   FileChecksum(struct ActBuffer buff);
int   FuseChecksum(int galtype);
int   MakeJedecBuff(struct ActBuffer buff, int galtype, struct Config *cfg);
void  WriteJedecFile(char *filename, int galtype, struct Config *cfg);


/* EOF */
