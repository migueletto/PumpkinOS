/*----------------------------------------------------------------------*/
/* M6809 Disassembler, code by Bruno Vedder 13 Jun 2002			*/
/* Feel free to re-use this code, or use this desassembly as long as 	*/
/* you give proper credit.You use this program as your own risk, no 	*/
/* waranties ... 							*/
/* RESET* (OPC 3E) undocumented instruction is supported.		*/
/*----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "dasm6809.h"

#define MNEMONIC_LEN     10      /* Mnemonic string lenght   */
#define DESCRIPTION_LEN  16      /* Mnemonic description len  */
#define MNEMO_HELP_LEN   32      /* Help on mnemonic effects   */
#define DUMP_STR_LEN     32      /* Lenght of a return string of Dump     */
#define OPERAND_STR_LEN  32      /* Lenght of a return string of operand list   */

/*---------- Link Adressing mode to its display function ----------*/

#define INHERENT      (int (*)(void))AM_Inherent      /* Adressing mode INHERENT   */
#define IMMEDIAT_8    (int (*)(void))AM_Immediat_8    /* Immediat 8 Bit    */
#define IMMEDIAT_16   (int (*)(void))AM_Immediat_16   /* Immediate 16 Bit    */
#define BRANCH_REL_8  (int (*)(void))AM_Branch_Rel_8  /* Branch relative 8 bit. +127 -128 bytes move */
#define BRANCH_REL_16 (int (*)(void))AM_Branch_Rel_16 /* Branch relative 16 bit. +32767 -32768 bytes move */
#define DIRECT        (int (*)(void))AM_Direct        /* Via DP register for msb      */
#define EXTENDED      (int (*)(void))AM_Extended      /* in C =  *(u8*)         */
#define INDEXED       (int (*)(void))AM_Indexed       /* Indexed mode */
#define ILLEGAL       (int (*)(void))AM_Illegal       /* Illegal addressing mode */

/*---------- Defines used for Inherent Adressing Mode ----------*/

#define OP_EXG    0x1e      /* EXG Opcode   */
#define OP_TFR    0x1f      /* TFR Opcode   */
#define OP_PSHS   0x34      /* PSHS Opcode  */
#define OP_PULS   0x35      /* PULS Opcode  */
#define OP_PSHU   0x36      /* PSHU Opcode  */
#define OP_PULU   0x37      /* PULU Opcode  */
#define OP_CWAI   0x3c      /* CWAI Opcode  */
#define OP_SWI    0x3f      /* SWIx Opcode  */

/*---------------  Typedef and structures       ---------------*/

typedef unsigned char u8;      /* unsigned 8 bit */
typedef unsigned short u16;    /* unsigned 16 bit */

typedef struct M6809CPU {
  u8 D[2];          /* A and B register = D 16 bit register */
  u8 DP;            /* Direct Page Register */
  u8 CC;            /* Condition Code register */
  u16 X;            /* X register */
  u16 Y;            /* Y register */
  u16 U;            /* User Stack register */
  u16 S;            /* System Stack register */
  u16 PC;           /* Program Counter register */
  u8 (*ReadMem8)(struct M6809CPU *cpu, u16 Adr);   /* Funcs use to make byte read operation.    */
  u16 (*ReadMem16)(struct M6809CPU *cpu, u16 Adr); /* Add decoding, I/O map are performed here. */
  void *data;
} M6809CPU;

typedef struct {
  u8 Opcode;        /* Operande Opcode */
  u8 Clock;         /* Number of cycle duration */
  u8 Size;          /* Opcode+Operande size */
  int (*AddrMode)();                 /* Addressing mode dep. operand decoder */
  char Mnemonic[MNEMONIC_LEN];       /* Mnemonic string */
  char Description[DESCRIPTION_LEN]; /* Mnemonic short description */
  char MnemoHelp[MNEMO_HELP_LEN];    /* More complete description */
} M6809Opcode;

/*----- Register table used by IndexRegister, and InherentRegister Functions -----*/

static char *Register16bit[4] = {"X","Y","U","S"};
static char *Registers[16] = {"D","X","Y","U","S","PC","??","??","A","B","CC","DP","??","??","??","??"};

/*----- Each functions display instruction operands depending on the Addressing mode -----*/

static int AM_Inherent      (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Immediat_8    (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Immediat_16   (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Branch_Rel_8  (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Branch_Rel_16 (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Direct        (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Extended      (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Indexed       (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);
static int AM_Illegal       (M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr);

/*----- Decode Register, Disassemble and Dump function prototypes -----*/

static u8 *IndexRegister(u8 postbyte);
static u8 *InherentRegister(u8 postbyte);
static u8 Dasm6809(M6809CPU *Cpu,u16 Adr,char *dst_str,u8 *nb_cycles);
static void HexDump(M6809CPU *Cpu,u16 Add,u8 nb_byte,char *dst);
static void WritePSHSRegister(char *dst_str,u8 byte);
static void WritePSHURegister(char *dst_str,u8 byte);
static void WritePULSRegister(char *dst_str,u8 byte);
static void WritePULURegister(char *dst_str,u8 byte);
static void WriteCWAIRegister(char *dst_str,u8 byte);

/*-------------------------------------------------------------------------------*/
/*  OPCODE  CLOCK   SIZE  ADD MODE  MNEMO.  ACTION,    HELP   */ 
/*-------------------------------------------------------------------------------*/
static M6809Opcode OpcodeTable [256] = {
  {0x00,  6,  2,  DIRECT,    "NEG ",  "d=-d",    "Negate"},
  {0x01,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x02,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x03,  6,  2,  DIRECT,    "COM ",  "d=~d",    "Complement"},
  {0x04,  6,  2,  DIRECT,    "LSR ",  "d=->{C,d,0}",  "Logical Shift Right"},
  {0x05,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x06,  6,  2,  DIRECT,    "ROR ",  "d=->{C,d}",  "Rotate Right"},
  {0x07,  6,  2,  DIRECT,    "ASR ",  "d=d/2",  "Arithmetic Shift Right"},
  {0x08,  6,  2,  DIRECT,    "LSL ",  "d={C,d,0}<-",  "Logical Shift Left"},
  {0x09,  6,  2,  DIRECT,    "ROL ",  "d=->{C,d}",  "Rotate Right"},
  {0x0A,  6,  2,  DIRECT,    "DEC ",  "d=d-1",  "Decremente"},
  {0x0B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x0C,  6,  2,  DIRECT,    "INC ",  "d=d+1",  "Incremente"},
  {0x0D,  6,  2,  DIRECT,    "TST ",  "s",    "Test"},
  {0x0E,  3,  2,  DIRECT,    "JMP ",  "PC=EAs",  "Jump"},
  {0x0F,  6,  2,  DIRECT,    "CLR ",  "d=0",    "Clear"},

  {0x10,  0,  1,  ILLEGAL,  "Prefix","0x10 Prefix",  "Illegal or Unsupported."},
  {0x11,  0,  1,  ILLEGAL,  "Prefix","0x11 Prefix",  "Illegal or Unsupported."},
  {0x12,  2,  1,  INHERENT,  "NOP ",  "Nop",    "No Operation"},
  {0x13,  2,  1,  INHERENT,  "SYNC",  "(min ~s=2)",  "Sync. to interrupt"},
  {0x14,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x15,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x16,  5,  3,  BRANCH_REL_16,  "LBRA ","PC=nn",  "Long Branch Always"},
  {0x17,  9,  3,  BRANCH_REL_16,  "LBSR ","-[S]=PC,LBRA",  "Long Branch Sub Routine"},
  {0x18,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x19,  2,  1,  INHERENT,  "DAA",  "a=BCD format",  "Decimal Adjust Acc. "},
  {0x1A,  3,  2,  IMMEDIAT_8,  "ORCC ","CC=CCvn",  "Inclusive OR CCR"},
  {0x1B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x1C,  3,  2,  IMMEDIAT_8,  "ANDCC ","CC=CC&s",  "Logical AND with CCR"},
  {0x1D,  2,  1,  INHERENT,  "SEX",  "D=B",    "Sign Extend"},
  {0x1E,  8,  2,  INHERENT,  "EXG ",  "r1<->r2",  "Exchange (r1 size=r2)"},
  {0x1F,  7,  2,  INHERENT,  "TFR ",  "r2=r1",  "Transfer (r1 size<=r2)"},

  {0x20,  3,  2,  BRANCH_REL_8,  "BRA ",  "PC=m",    "Branch Always"},
  {0x21,  3,  2,  BRANCH_REL_8,  "BRN ",  "Nop",    "Branch Never "},
  {0x22,  3,  2,  BRANCH_REL_8,  "BHI ",  "If CvZ=0",  "Branch if Higher"},
  {0x23,  3,  2,  BRANCH_REL_8,  "BLS ",  "If CvZ=1",  "Branch if Lower/Same"},
  {0x24,  3,  2,  BRANCH_REL_8,  "BCC ",  "If C=0",  "Branch if Carry Clear"},
  {0x25,  3,  2,  BRANCH_REL_8,  "BCS ",  "If C=1",  "Branch if Carry Set"},
  {0x26,  3,  2,  BRANCH_REL_8,  "BNE ",  "If Z=0",  "Branch if Not Equal"},
  {0x27,  3,  2,  BRANCH_REL_8,  "BEQ ",  "If Z=1",  "Branch if EQual"},
  {0x28,  3,  2,  BRANCH_REL_8,  "BVC ",  "If V=0",  "Branch if overflow Clear"},
  {0x29,  3,  2,  BRANCH_REL_8,  "BVS ",  "If V=1",  "Branch if overflow Set"},
  {0x2A,  3,  2,  BRANCH_REL_8,  "BPL ",  "If N=0",  "Branch if Plus"},
  {0x2B,  3,  2,  BRANCH_REL_8,  "BMI ",  "If N=1",  "Branch if Minus"},
  {0x2C,  3,  2,  BRANCH_REL_8,  "BGE ",  "If NxV=0",  "Branch if Great/Equal"},
  {0x2D,  3,  2,  BRANCH_REL_8,  "BLT ",  "If NxV=1",  "Branch if Less Than"},
  {0x2E,  3,  2,  BRANCH_REL_8,  "BGT ",  "If Zv{NxV}=0",  "Branch if Greater Than"},
  {0x2F,  3,  2,  BRANCH_REL_8,  "BLE ",  "If Zv{NxV}=1",  "Branch if Less/Equal"},

  {0x30,  4,  2,  INDEXED,  "LEAX ","p=EAs(X=0-3)",  "Load Effective Address"},
  {0x31,  4,  2,  INDEXED,  "LEAY ","p=EAs(Y=0-3)",  "Load Effective Address"},
  {0x32,  4,  2,  INDEXED,  "LEAS ","p=EAs(S=0-3)",  "Load Effective Address"},
  {0x33,  4,  2,  INDEXED,  "LEAU ","p=EAs(U=0-3)",  "Load Effective Address"},
  {0x34,  5,  2,  INHERENT,  "PSHS ", "-[S]={r,...}",  "Push reg(s) (not S)"},
  {0x35,  5,  2,  INHERENT,  "PULS ","{r,...}=[S]+",  "Pull reg(s) (not S)"},
  {0x36,  5,  2,  INHERENT,  "PSHU ", "-[U]={r,...}",  "Push reg(s) (not U)"},
  {0x37,  5,  2,  INHERENT,  "PULU ","{r,...}=[U]+",  "Pull reg(s) (not U)"},
  {0x38,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x39,  5,  1,  INHERENT,  "RTS",  "PC=[S]+",  "Return from Subroutine"},
  {0x3A,  3,  1,  INHERENT,  "ABX",  "X=X+B",  "Add to Index Register"},
  {0x3B,  6,  1,  INHERENT,  "RTI",  "{regs}=[S]+",  "Return from Interrupt"},
  {0x3C,  21,  2,  INHERENT,  "CWAI ","CC=0 +wait Int","Clear CC and wait interrupt"},
  {0x3D,  11,  1,  INHERENT,  "MUL",  "D=A*B",  "Multiply"},
  {0x3E,  1,  1,  INHERENT,  "RESET*","Jmp [FFFE]",  "Restart (Undocumented)"},
  {0x3F,  19,  1,  INHERENT,  "SWI1",  "-[S]={regs}",  "Software Interrupt 1"},

  {0x40,  2,  1,  INHERENT,  "NEGA",  "a= -a",  "Negate accumulator"},
  {0x41,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x42,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x43,  7,  1,  INHERENT,  "COMA",  "a=~a",    "Complement accumulator"},
  {0x44,  2,  1,  INHERENT,  "LSRA",  "d=->{C,d,0}",  "Logical Shift Right"},
  {0x45,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x46,  2,  1,  INHERENT,  "RORA",  "a=->{C,a}",  "Rotate Right acc."},
  {0x47,  2,  1,  INHERENT,  "ASRA",  "a=a>>1",  "Arithmetic Shift Right"},
  {0x48,  2,  1,  INHERENT,  "ASLA",  "a=a<<1",  "Arithmetic Shift Left"},
  {0x49,  2,  1,  INHERENT,  "ROLA",  "a={C,a}<-",  "Rotate Left acc."},
  {0x4A,  2,  1,  INHERENT,  "DECA",  "a=a-1",  "Decrement accumulator"},
  {0x4B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x4C,  2,  1,  INHERENT,  "INCA",  "a=a+1",  "Increment accumulator"},
  {0x4D,  2,  1,  INHERENT,  "TSTA",  "a",    "Test accumulator"},
  {0x4E,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x4F,  2,  1,  INHERENT,  "CLRA",  "a = 0",  "Clear accumulator"},

  {0x50,  2,  1,  INHERENT,  "NEGB",  "b= -b",  "Negate accumulator"},
  {0x51,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x52,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x53,  7,  1,  INHERENT,  "COMB",  "b=~b",    "Complement accumulator"},
  {0x54,  2,  1,  INHERENT,  "LSRB",  "d=->{C,d,0}",  "Logical Shift Right"},
  {0x55,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x56,  2,  1,  INHERENT,  "RORB",  "b=->{C,b}",  "Rotate Right acc."},
  {0x57,  2,  1,  INHERENT,  "ASRB",  "b=b>>1",  "Arithmetic Shift Right"},
  {0x58,  2,  1,  INHERENT,  "ASLB",  "b=b<<1",  "Arithmetic Shift Left"},
  {0x59,  2,  1,  INHERENT,  "ROLB",  "b={C,b}<-",  "Rotate Left acc."},
  {0x5A,  2,  1,  INHERENT,  "DECB",  "b=b-1",  "Decrement accumulator"},
  {0x5B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x5C,  2,  1,  INHERENT,  "INCB",  "b=b+1",  "Increment accumulator"},
  {0x5D,  2,  1,  INHERENT,  "TSTB",  "b",    "Test accumulator"},
  {0x5E,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x5F,  2,  1,  INHERENT,  "CLRB",  "b = 0",  "Clear accumulator"},

  {0x60,  6,  2,  INDEXED,  "NEG ",  "s=~s",    "Negate"},
  {0x61,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x62,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x63,  6,  2,  INDEXED,  "COM ",  "d=~d",    "Complement"},
  {0x64,  6,  2,  INDEXED,  "LSR ",  "d=d/2",  "Logical Shift Right"},
  {0x65,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x66,  6,  2,  INDEXED,  "ROR ",  "d=->{C,d}",  "Rotate Right"},
  {0x67,  6,  2,  INDEXED,  "ASR ",  "d=d/2",  "Arithmetic Shift Right"},
  {0x68,  6,  2,  INDEXED,  "LSL ",  "d={C,d,0}<-",  "Logical Shift Left"},
  {0x69,  6,  2,  INDEXED,  "ROL ",  "d={C,d}<-",  "Rotate left"},
  {0x6A,  6,  2,  INDEXED,  "DEC ",  "d=d-1",  "Decrement"},
  {0x6B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x6C,  6,  2,  INDEXED,  "INC ",  "d=d+1",  "Increment"},
  {0x6D,  6,  2,  INDEXED,  "TST ",  "s",    "Test"},
  {0x6E,  3,  2,  INDEXED,  "JMP ",  "PC=EAs",  "Jump"},
  {0x6F,  6,  2,  INDEXED,  "CLR ",  "d=0",    "Clear"},

  {0x70,  7,  3,  EXTENDED,  "NEG ",  "s=~s",  "Negate"},
  {0x71,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x72,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},  
  {0x73,  7,  3,  EXTENDED,  "COM ",  "s=~s",    "Complete"},
  {0x74,  7,  3,  EXTENDED,  "LSR ",  "s=s/2",  "Logical Shift Right"},
  {0x75,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x76,  7,  3,  EXTENDED,  "ROR ",  "d=->{C,d}",  "Rotate Right"},
  {0x77,  7,  3,  EXTENDED,  "ASR ",  "d=d/2",  "Arithmetic Shift Right"},
  {0x78,  7,  3,  EXTENDED,  "LSL ",  "d=d*2",  "Logical Shift Left"},
  {0x79,  7,  3,  EXTENDED,  "ROL ",  "d={C,d}<-",  "Rotate Left"},
  {0x7A,  7,  3,  EXTENDED,  "DEC ",  "d=d-1",  "Decrement"},
  {0x7B,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x7C,  7,  3,  EXTENDED,  "INC ",  "d=d+1",  "Increment"},
  {0x7D,  7,  3,  EXTENDED,  "TST ",  "s",    "Test"},
  {0x7E,  3,  3,  EXTENDED,  "JMP ",  "PC=EAs",  "Jump"},
  {0x7F,  7,  3,  EXTENDED,  "CLR ",  "s=0",    "Clear"},

  {0x80,  2,  2,  IMMEDIAT_8,  "SUBA ","a=a-s",  "Substract."},
  {0x81,  2,  2,  IMMEDIAT_8,  "CMPA ","a-s",    "Compare."},  
  {0x82,  2,  2,  IMMEDIAT_8,  "SBCA ","a=a-s-C",  "Subtract with Carry"},  
  {0x83,  4,  3,  IMMEDIAT_16,  "SUBd ","D=D-s",  "Subtract Double acc."},
  {0x84,  2,  2,  IMMEDIAT_8,  "ANDA ","a=a&s",  "Logical AND"},
  {0x85,  2,  2,  IMMEDIAT_8,  "BITA ","a&s",    "Bit Test accumulator"},
  {0x86,  2,  2,  IMMEDIAT_8,  "LDA ",  "a=s",    "Load accumulator"},
  {0x87,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0x88,  2,  2,  IMMEDIAT_8,  "EORA ","a=axs",  "Logical Exclusive OR"},
  {0x89,  2,  2,  IMMEDIAT_8,  "ADCA ","a=a+s+C",  "Add with Carry"},
  {0x8A,  2,  2,  IMMEDIAT_8,  "ORA ",  "a=avs",  "Logical inclusive OR"},
  {0x8B,  2,  2,  IMMEDIAT_8,  "ADDA ","a=a+s",  "Add"},
  {0x8C,  4,  3,  IMMEDIAT_16,  "CMPX ","X-s",    "Compare"},
  {0x8D,  7,  2,  BRANCH_REL_8,  "BSR ",  "-[S]=PC,BRA",  "Branch to Subroutine"},
  {0x8E,  3,  3,  IMMEDIAT_16,  "LDX ","X=s",    "Load Index register"},
  {0x8F,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},

  {0x90,  4,  2,  DIRECT,  "SUBA ",  "a=a-s",  "Substract"},
  {0x91,  4,  2,  DIRECT,  "CMPA ",  "a-s",    "Compare"},  
  {0x92,  4,  2,  DIRECT,  "SBCA ",  "a=a-s-C",  "Subtract with Carry"},  
  {0x93,  6,  2,  DIRECT,  "SUBd ",  "D=D-s",  "Subtract Double acc."},
  {0x94,  4,  2,  DIRECT,  "ANDA ",  "a=a&s",  "Logical AND"},
  {0x95,  4,  2,  DIRECT,  "BITA ",  "a&s",    "Bit Test accumulator"},
  {0x96,  4,  2,  DIRECT,  "LDA ",    "a=s",    "Load accumulator"},
  {0x97,  4,  2,  DIRECT,  "STA ",    "d=a",    "Store accumultor"},
  {0x98,  4,  2,  DIRECT,  "EORA ",  "a=axs",  "Logical Exclusive OR"},
  {0x99,  4,  2,  DIRECT,  "ADCA ",  "a=a+s+C",  "Add with Carry"},
  {0x9A,  4,  2,  DIRECT,  "ORA ",    "a=avs",  "Logical Inclusive OR "},
  {0x9B,  4,  2,  DIRECT,  "ADDA ",  "a=a+s",  "Add"},
  {0x9C,  6,  2,  DIRECT,  "CMPX ",  "a-s",    "Compare"},
  {0x9D,  7,  2,  DIRECT,  "JSR ",    "-[S]=PC,JMP",  "Jump Sub Routine"},
  {0x9E,  5,  2,  DIRECT,  "LDX ",    "X=s",    "Load X"},
  {0x9F,  5,  2,  DIRECT,  "STX ",    "s=X",    "Store X"},

  {0xA0,  4,  2,  INDEXED,  "SUBA ","a=a-s",  "Substract"},
  {0xA1,  4,  2,  INDEXED,  "CMPA ","a-s",    "Compare"},  
  {0xA2,  4,  2,  INDEXED,  "SBCA ","a=a-s-C",  "Substract with Carry"},  
  {0xA3,  6,  2,  INDEXED,  "SUBD ","D=D-s",  "Substract Double Accumulator"},
  {0xA4,  4,  2,  INDEXED,  "ANDA ","a=a&s",  "Logical AND"},
  {0xA5,  4,  2,  INDEXED,  "BITA ","a&s",    "Bit Test accumulator"},
  {0xA6,  4,  2,  INDEXED,  "LDA ",  "a=s",    "Load Accumulator"},
  {0xA7,  4,  2,  INDEXED,  "STA ",  "d=a",    "Store Accumulator"},
  {0xA8,  4,  2,  INDEXED,  "EORA ","a=axs",  "Logical Exclusive OR"},
  {0xA9,  4,  2,  INDEXED,  "ADCA ","a=a+s+C",  "Add with Carry"},
  {0xAA,  4,  2,  INDEXED,  "ORA ",  "a=avs",  "Logical Inclusive OR"},
  {0xAB,  4,  2,  INDEXED,  "ADDA ","a=a+s",  "Add"},
  {0xAC,  6,  2,  INDEXED,  "CMPX ","X-s",    "Compare"},
  {0xAD,  7,  2,  INDEXED,  "JSR ",  "-[S]=PC,JMP",  "Jump Sub Routine"},
  {0xAE,  5,  2,  INDEXED,  "LDX ",  "X=s",    "Load X"},
  {0xAF,  5,  2,  INDEXED,  "STX ",  "d=X",    "Store X"},

  {0xB0,  5,  3,  EXTENDED,  "SUBA ","a=a-s",  "Substract"},
  {0xB1,  5,  3,  EXTENDED,  "CMPA ","a-s",    "Compare"},  
  {0xB2,  5,  3,  EXTENDED,  "SBCA ","a=a-s-C",  "Substract with Carry"},  
  {0xB3,  7,  3,  EXTENDED,  "SUBD ","D=D-s",  "Subtract Double acc."},
  {0xB4,  5,  3,  EXTENDED,  "ANDA ","a=a&s",  "Logical AND"},
  {0xB5,  5,  3,  EXTENDED,  "BITA ","a&s ",    "Bit Test accumulator"},
  {0xB6,  5,  3,  EXTENDED,  "LDA ",  "a=s",    "Load accumulator"},
  {0xB7,  5,  3,  EXTENDED,  "STA ",  "d=a",    "Store Accumulator"},
  {0xB8,  5,  3,  EXTENDED,  "EORA ","a=axs",  "Logical Exclusive OR"},
  {0xB9,  5,  3,  EXTENDED,  "ADCA ","a=a+s+C",  "Add with Carry"},
  {0xBA,  5,  3,  EXTENDED,  "ORA ",  "a=avs",  "Logical Inclusive OR"},
  {0xBB,  5,  3,  EXTENDED,  "ADDA ","a=a+s",  "Add"},
  {0xBC,  7,  3,  EXTENDED,  "CMPX ","X-s",    "Compare"},
  {0xBD,  8,  3,  EXTENDED,  "JSR ",  "Unknown",  "Jump Sub Routine"},
  {0xBE,  6,  3,  EXTENDED,  "LDX ",  "X=s",    "Load Index Register"},
  {0xBF,  6,  3,  EXTENDED,  "STX ",  "d=X",    "Store Index Register"},

  {0xC0,  2,  2,  IMMEDIAT_8,  "SUBB ","b=b-s",  "Substract."},
  {0xC1,  2,  2,  IMMEDIAT_8,  "CMPB ","b-s",    "Compare."},  
  {0xC2,  2,  2,  IMMEDIAT_8,  "SBCB ","b=b-s-C",  "Subtract with Carry"},  
  {0xC3,  4,  3,  IMMEDIAT_16,  "ADDd ","D=D+s",  "Add to Double acc. "},
  {0xC4,  2,  2,  IMMEDIAT_8,  "ANDB ","b=b&s",  "Logical AND"},
  {0xC5,  2,  2,  IMMEDIAT_8,  "BITB ","b&s",    "Bit Test accumulator"},
  {0xC6,  2,  2,  IMMEDIAT_8,  "LDB ",  "b=s",    "Load accumulator"},
  {0xC7,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0xC8,  2,  2,  IMMEDIAT_8,  "EORB ","b=bxs",  "Logical Exclusive OR"},
  {0xC9,  2,  2,  IMMEDIAT_8,  "ADCB ","b=b+s+C",  "Add with Carry"},
  {0xCA,  2,  2,  IMMEDIAT_8,  "ORB ",  "b=bvs",  "Logical inclusive OR"},
  {0xCB,  2,  2,  IMMEDIAT_8,  "ADDB ","b=b+s",  "Add"},
  {0xCC,  3,  3,  IMMEDIAT_16,  "LDD ",  "D=s",    "Load Double acc."},
  {0xCD,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},
  {0xCE,  3,  3,  IMMEDIAT_16,  "LDU ",  "U=s",    "Load User stack ptr"},
  {0xCF,  1,  1,  ILLEGAL,  "???",  "Unknown",  "Illegal or Unsupported."},

  {0xD0,  4,  2,  DIRECT,  "SUBB ",  "b=b-s",  "Substract"},
  {0xD1,  4,  2,  DIRECT,  "CMPB ",  "b-s",    "Compare"},  
  {0xD2,  4,  2,  DIRECT,  "SBCB ",  "b=b-s-C",  "Subtract with Carry"},  
  {0xD3,  6,  2,  DIRECT,  "ADDD ",  "D=D+s",  "Add to Double acc"},
  {0xD4,  4,  2,  DIRECT,  "ANDB",    "b=b&s",  "Logical AND."},
  {0xD5,  4,  2,  DIRECT,  "BITB ",  "b&s",    "Test"},
  {0xD6,  4,  2,  DIRECT,  "LDB ",    "b=s",    "Load Accumulator"},
  {0xD7,  4,  2,  DIRECT,  "STB ",    "s=b",    "Store Accumulator"},
  {0xD8,  4,  2,  DIRECT,  "EORB ",  "b=bxs",  "Logical Exclusive OR"},
  {0xD9,  4,  2,  DIRECT,  "ADCB ",  "b=b+s+C",  "Add with Carry"},
  {0xDA,  4,  2,  DIRECT,  "ORB ",    "b=bvs",  "Logical Inclusive OR."},
  {0xDB,  4,  2,  DIRECT,  "ADDB ",  "b=b+s",  "Add"},
  {0xDC,  5,  2,  DIRECT,  "LDD ",    "D=s",    "Load Double acc"},
  {0xDD,  5,  2,  DIRECT,  "STD ",    "s=D",    "Store Double acc"},
  {0xDE,  5,  2,  DIRECT,  "LDU ",    "U=s",    "Load User stack ptr"},
  {0xDF,  5,  2,  DIRECT,  "STU ",    "s=U",    "Store User stacl ptr"},

  {0xE0,  4,  2,  INDEXED,  "SUBB ","b=b-s",  "Substract"},
  {0xE1,  4,  2,  INDEXED,  "CMPB ","b-s",    "Compare"},  
  {0xE2,  4,  2,  INDEXED,  "SBCB ","b=b-s-C",  "Substract with Carry"},  
  {0xE3,  6,  2,  INDEXED,  "ADDD ","D=D+s",  "Add to Double acc."},
  {0xE4,  4,  2,  INDEXED,  "ANDB ","b=b&s",  "Logical AND"},
  {0xE5,  4,  2,  INDEXED,  "BITB ","b&s",    "Bit Test accumulator"},
  {0xE6,  4,  2,  INDEXED,  "LDB ",  "b=s",    "Load Accumulator"},
  {0xE7,  4,  2,  INDEXED,  "STB ",  "d=s",    "Store Accumulator"},
  {0xE8,  4,  2,  INDEXED,  "EORB ","b=bxs",  "Logical Exclusive OR"},
  {0xE9,  4,  2,  INDEXED,  "ADCB ","b=b+s+C",  "Add with Carry"},
  {0xEA,  4,  2,  INDEXED,  "ORB ",  "b=bvs",  "Logical Inclusive OR"},
  {0xEB,  4,  2,  INDEXED,  "ADDB ","b=b+s",  "Add"},
  {0xEC,  5,  2,  INDEXED,  "LDD ",  "D=s",    "Load Double Accumulator"},
  {0xED,  5,  2,  INDEXED,  "STD ",  "d=D",    "Store Double Accumulator"},
  {0xEE,  5,  2,  INDEXED,  "LDU ",  "U=s",    "Load User Stack ptr"},
  {0xEF,  5,  2,  INDEXED,  "STU ",  "s=U",    "Store User Stack ptr"},

  {0xF0,  5,  3,  EXTENDED,  "SUBB ","b=b-s",  "Substract"},
  {0xF1,  5,  3,  EXTENDED,  "CMPB ","b-s",    "Compare"},  
  {0xF2,  5,  3,  EXTENDED,  "SBCB ","b=b-s-C",  "Substract with Carry"},  
  {0xF3,  7,  3,  EXTENDED,  "ADDD ","D=D+s",  "Add to Double accumulator"},
  {0xF4,  5,  3,  EXTENDED,  "ANDB ","b=b&s",  "Logical AND"},
  {0xF5,  5,  3,  EXTENDED,  "BITB ","b&s",    "Bit Test accumulator"},
  {0xF6,  5,  3,  EXTENDED,  "LDB ",  "b=s",    "Load Accumulator"},
  {0xF7,  5,  3,  EXTENDED,  "STB ",  "d=s",    "Store Accumulator"},
  {0xF8,  5,  3,  EXTENDED,  "EORB ","b=bxs",  "Logical Exclusive OR"},
  {0xF9,  5,  3,  EXTENDED,  "ADCB ","b=b+s+C",  "Add with Carry"},
  {0xFA,  5,  3,  EXTENDED,  "ORB ",  "b=bvs",  "Logical Inclusive OR"},
  {0xFB,  5,  3,  EXTENDED,  "ADDB ","b=b+s",  "Add"},
  {0xFC,  6,  3,  EXTENDED,  "LDD ",  "D=s",    "Load Double Accumulator"},
  {0xFD,  6,  3,  EXTENDED,  "STD ",  "d=D",    "Store Double Accumulator"},
  {0xFE,  6,  3,  EXTENDED,  "LDU ",  "U=s",    "Load User Stack ptr"},
  {0xFF,  6,  3,  EXTENDED,  "STU ",  "U=a",    "Store User Stack ptr"},
};
/*-------------------------------------------------------------------------------*/
/*  OPCODE  CLOCK   SIZE  ADD MODE  MNEMO.  ACTION,    HELP   */
/*-------------------------------------------------------------------------------*/
/*   All Opcodes are prefixed by 0x10  value. Prefix included in opcode size  */
/*-------------------------------------------------------------------------------*/
static M6809Opcode OpcodeTable10 [256] = {
  {0x0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x10,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x11,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x12,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x13,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x14,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x15,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x16,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x17,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x18,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x19,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x20,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x21,  5,  4,  BRANCH_REL_16,  "LBRN ","Nop",    "Long Branch Never"},
  {0x22,  5,  4,  BRANCH_REL_16,  "LBHI ","If CvZ=0",  "Long Branch if Higher"},
  {0x23,  5,  4,  BRANCH_REL_16,  "LBLS ","If CvV=1",  "Long Branch if Lower/Same"},
  {0x24,  5,  4,  BRANCH_REL_16,  "LBCC ","If C=0",  "Long Branch if Carry Clear"},
  {0x25,  5,  4,  BRANCH_REL_16,  "LBCS ","If C=1",  "Long Branch if Carry Set"},
  {0x26,  5,  4,  BRANCH_REL_16,  "LBNE ","If Z=0",  "Long Branch if Not Equal"},
  {0x27,  5,  4,  BRANCH_REL_16,  "LBEQ ","If Z=1",  "Long Branch if Equal"},
  {0x28,  5,  4,  BRANCH_REL_16,  "LBVC ","If V=0",  "Long Branch if Overflow Clr"},
  {0x29,  5,  4,  BRANCH_REL_16,  "LBVS ","If V=1",  "Long Branch if Overflow Set"},
  {0x2a,  5,  4,  BRANCH_REL_16,  "LBPL ","If N=0",  "Long Branch if Plus"},
  {0x2b,  5,  4,  BRANCH_REL_16,  "LBMI ","If N=1",  "Long Branch if Minus"},
  {0x2c,  5,  4,  BRANCH_REL_16,  "LBGE ","If NxV=0",  "Long Branch if Great/Equal"},
  {0x2d,  5,  4,  BRANCH_REL_16,  "LBLT ","If NxV=1",  "Long Branch if Less Than"},
  {0x2e,  5,  4,  BRANCH_REL_16,  "LBGT ","If Zv{NxV}=0",  "Long Branch if Greater Than"},
  {0x2f,  5,  4,  BRANCH_REL_16,  "LBLE ","If Zv{NxV}=1",  "Long Branch if Less/Equal"},
  
  {0x30,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x31,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x32,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x33,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x34,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x35,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x36,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x37,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x38,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x39,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3f,  20,  2,  INHERENT,  "SWI2 ","Swi2",    "Software interrupt 2"},

  {0x40,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x41,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x42,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x43,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x44,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x45,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x46,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x47,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x48,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x49,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x50,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x51,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x52,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x53,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x54,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x55,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x56,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x57,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x58,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x59,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x60,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x61,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x62,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x63,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x64,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x65,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x66,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x67,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x68,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x69,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x70,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x71,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x72,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x73,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x74,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x75,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x76,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x77,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x78,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x79,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x80,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x81,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x82,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x83,    5,  4,  IMMEDIAT_16,  "CMPD ","D-s",    "Compare Double Accumulator"},
  {0x84,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x85,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x86,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x87,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x88,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x89,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8c,   5,  4,  IMMEDIAT_16,  "CMPY ","D-s",    "Compare "},
  {0x8d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8e,   4,  4,  IMMEDIAT_16,  "LDY ","Y=s",    "Load Y"},
  {0x8f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x90,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x91,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x92,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x93,   7,  3,  DIRECT,    "CMPD ","D-s",    "Compare Double Accumulator"},
  {0x94,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x95,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x96,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x97,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x98,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x99,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9c,   7,  3,  DIRECT,    "CMPY ","D-s",    "Compare "},
  {0x9d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9e,   6,  3,  DIRECT,    "LDY ","Y=s",    "Load Y"},
  {0x9f,   6,  3,  DIRECT,    "STY ","d=Y",    "Store Y"},

  {0xa0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa3,   7,  3,  INDEXED,  "CMPD ","D-s",    "Compare Double Accumulator"},
  {0xa4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xaa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xab,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xac,   7,  3,  INDEXED,  "CMPY ","D-s",    "Compare "},
  {0xad,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xae,    6,  3,  INDEXED,  "LDY ","Y=s",    "Load Y"},
  {0xaf,   6,  3,  INDEXED,  "STY ","d=Y",    "Store Y"},

  {0xb0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb3,   8,  4,  EXTENDED,  "CMPD ","D-s",    "Compare Double Accumulator"},
  {0xb4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xba,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbc,   8,  4,  EXTENDED,  "CMPY ","D-s",    "Compare "},
  {0xbd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbe,   7,  4,  EXTENDED,  "LDY ","Y=s",    "Load Y"},
  {0xbf,   7,  4,  EXTENDED,  "STY ","d=Y",    "Store Y"},

  {0xc0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xca,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xce,   4,  4,  IMMEDIAT_16,  "LDS ","S=d",    "Load S"},
  {0xcf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xd0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xda,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xde,   6,  3,  DIRECT,    "LDS ","S=s",    "Load S"},
  {0xdf,   6,  3,  DIRECT,    "STS ","d=S",    "Store S"},

  {0xe0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xea,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xeb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xec,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xed,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xee,   6,  3,  INDEXED,  "LDS ","S=s",    "Load S"},
  {0xef,   6,  3,  INDEXED,  "STS ","d=S",    "Store S"},

  {0xf0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfe,   7,  4,  EXTENDED,  "LDS ","S=s",    "Load S"},
  {0xff,   7,  4,  EXTENDED,  "STS ","d=S",    "Store S"},
};
/*-------------------------------------------------------------------------------*/
/*  OPCODE  CLOCK   SIZE  ADD MODE  MNEMO.  ACTION,    HELP   */
/*-------------------------------------------------------------------------------*/
/*   All Opcodes are prefixed by 0x11  value. Prefix included in opcode size  */
/*-------------------------------------------------------------------------------*/
static M6809Opcode OpcodeTable11 [256] = {
  {0x0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  
  {0x10,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x11,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x12,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x13,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x14,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x15,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x16,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x17,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x18,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x19,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x1f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  
  {0x20,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x21,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x22,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x23,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x24,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x25,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x26,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x27,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x28,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x29,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x2f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x30,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x31,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x32,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x33,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x34,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x35,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x36,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x37,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x38,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x39,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x3f,  20,  2,  INHERENT,  "SWI3 ","sftint",  "Software interrupt"},

  {0x40,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x41,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x42,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x43,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x44,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x45,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x46,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x47,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x48,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x49,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x4f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x50,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x51,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x52,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x53,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x54,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x55,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x56,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x57,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x58,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x59,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x5f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x60,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x61,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x62,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x63,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x64,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x65,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x66,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x67,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x68,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x69,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x6f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x70,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x71,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x72,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x73,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x74,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x75,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x76,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x77,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x78,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x79,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7c,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x7f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x80,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x81,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x82,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x83,   5,  4,  IMMEDIAT_16,  "CMPU ","U-s",    "Compare"},
  {0x84,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x85,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x86,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x87,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x88,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x89,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8c,   5,  4,  IMMEDIAT_16,  "CMPS ","S-s",    "Compare"},
  {0x8d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x8f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0x90,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x91,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x92,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x93,   7,  3,  DIRECT,    "CMPU ","U-s",    "Compare"},
  {0x94,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x95,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x96,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x97,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x98,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x99,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9a,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9b,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9c,   7,  3,  DIRECT,    "CMPS ","S-s",    "Compare"},
  {0x9d,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9e,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0x9f,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  
  {0xa0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa3,   7,  3,  INDEXED,  "CMPU ","U-s",    "Compare"},
  {0xa4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xa9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xaa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xab,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xac,   7,  3,  INDEXED,  "CMPS ","S-s",    "Compare"},
  {0xad,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xae,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xaf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xb0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb3,   8,  4,  EXTENDED,  "CMPU ","U-s",    "Compare"},
  {0xb4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xb9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xba,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbc,   8,  4,  EXTENDED,  "CMPS ","S-s",    "Compare"},
  {0xbd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbe,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xbf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xc0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xc9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xca,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xce,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xcf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xd0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xd9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xda,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xde,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xdf,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xe0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xe9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xea,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xeb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xec,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xed,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xee,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xef,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},

  {0xf0,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf1,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf2,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf3,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf4,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf5,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf6,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf7,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf8,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xf9,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfa,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfb,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfc,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfd,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xfe,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
  {0xff,   1,  2,  ILLEGAL,  "???","Unknown",  "Illegal or Unsupported."},
};

/*----------------------------------------------------------------------*/
/*	CPU willuse this functions to access memory. They contain here	*/
/* code for eventual bank switching or I/O mapping. They act like a BUS	*/
/* These functions are provided and attached to CPU 'object' by user.	*/
/*----------------------------------------------------------------------*/

/* CPU Read Function */
static u8 RdMem8(M6809CPU *cpu, u16 Adr) {
  return m6809_getbyte(cpu->data, Adr);
}

/* CAREFULL WITH INDIANESS */
static u16 RdMem16(M6809CPU *cpu, u16 Adr) {
  return (RdMem8(cpu, Adr) << 8) | RdMem8(cpu, Adr+1);
}

/*----------------------------------------------------------------------*/
/*	Disassemble One Opcode.						*/
/*	CPU is the cpu structure that will be taen for disassembly.	*/
/*	Adr is the address to disassemble from. 			*/
/*	dst_str is a string were the disassembly can wrote is output.	*/
/*	The func return the size of the disassembled opcode.		*/
/*----------------------------------------------------------------------*/
static u8 Dasm6809(M6809CPU *CPU,u16 Adr,char *dst_str,u8 * cycle_nbr) {
  u8 Opcode,Opsize,Inst_Clk;
  M6809Opcode *CurrentOpcodeTable;
  char Operands [OPERAND_STR_LEN];
  char Dump     [DUMP_STR_LEN];
  char Mnemo    [MNEMONIC_LEN];

  Operands[0]='\0';		/* Clear Operands string */
  Dump 	[0]='\0';
  Mnemo 	[0]='\0';
  Opcode = CPU->ReadMem8 (CPU, Adr);	/* Read byte */
  sprintf (dst_str,"%.4X: ",Adr);	    /* Write Address */

  switch (Opcode) {
	case 0x10:
		Opcode = CPU->ReadMem8 (CPU, Adr+1);			/* Read Real Opcode */
		CurrentOpcodeTable = OpcodeTable10;		/* Use Table of 10h prefixed opcode*/
		/* Write operands in string: Call funct associated to addressing mode */
		Opsize = CurrentOpcodeTable[Opcode].AddrMode (CPU,&CurrentOpcodeTable[Opcode],Adr+1,Operands,&Inst_Clk);
		HexDump (CPU,Adr,Opsize,Dump);				/* Dump byte(s)*/
		strcpy (Mnemo,CurrentOpcodeTable[Opcode].Mnemonic); /* Write Mnemonic */

	break;
	case 0x11:
		Opcode = CPU->ReadMem8 (CPU, Adr+1);			/* Read Real Opcode */
		CurrentOpcodeTable = OpcodeTable11;		/* Use Table of 11h prefixed opcode*/
		/* Write operands in string: Call funct associated to addressing mode */
		Opsize = CurrentOpcodeTable[Opcode].AddrMode (CPU,&CurrentOpcodeTable[Opcode],Adr+1,Operands,&Inst_Clk);
		HexDump (CPU,Adr,Opsize,Dump);				/* Dump byte(s)*/	
		strcpy (Mnemo,CurrentOpcodeTable[Opcode].Mnemonic);	/* Write Mnemonic */	
	break;
	default:
		CurrentOpcodeTable = OpcodeTable;
		/* Write operands in string: Call funct associated to addressing mode */
		Opsize=CurrentOpcodeTable[Opcode].AddrMode (CPU,&CurrentOpcodeTable[Opcode],Adr,Operands,&Inst_Clk);
		HexDump (CPU,Adr,Opsize,Dump);				/* Dump byte(s)*/
		strcpy (Mnemo,CurrentOpcodeTable[Opcode].Mnemonic); 	/* Write Mnemonic */
	break;	
	}
  strcat (dst_str,Dump);
  strcat (dst_str,Mnemo);
  strcat (dst_str,Operands); 	/* Add Operands at the end of line */
  //strcat (dst_str,"\t\t\t"); 	/* Add Mnemonic help :-)*/
  //strcat (dst_str,CurrentOpcodeTable[Opcode].MnemoHelp);
  strcat (dst_str,"\n");		/* CR\LF at the end of line */
  return (Opsize);
}

/*-----------------------------------------------------------------------------
	This function is used to decode registers used with the Operands
using xRRxxxxx Post Byte Register Bit.Register are encoded like this: X11XXXXX
which is an index in Register16bit Table.
-----------------------------------------------------------------------------*/
static u8 *IndexRegister(u8 postbyte) {
  return (u8 *)Register16bit [(postbyte>>5)&0x03];
}
/*-----------------------------------------------------------------------------
	This function is used to decode registers used with the
EXG,TFR Operands. Four lowest bits are significatives. Register are 
encoded like this: XXXX1111 which is an index in Register Table.
-----------------------------------------------------------------------------*/
static u8 *InherentRegister(u8 postbyte) {
  return (u8 *)(Registers [postbyte & 0xf]);
}
/*-----------------------------------------------------------------------------
	This function is used to decode registers used with the
CWAI instruction.
-----------------------------------------------------------------------------*/
static void WriteCWAIRegister(char *dst_str,u8 byte) {
  char ValBuf [4];
  sprintf (ValBuf,"#%.2X",byte);
  strcat (dst_str,ValBuf);
}

/*-----------------------------------------------------------------------------
	Those functions are used to write register pushed/pulled
in right order. PSHS never Pushes S, PULU never Pull U.
For info: each register is represented by a bit in byte.
if byte =0, we got some datas, or an illegal combinaison.
-----------------------------------------------------------------------------*/
static void WritePSHSRegister(char *dst_str,u8 byte) {
  if (byte == 0) {strcat (dst_str,"??");return;}
  if (byte & 0x80) strcat (dst_str,",PC");
  if (byte & 0x40) strcat (dst_str,",U");
  if (byte & 0x20) strcat (dst_str,",Y");
  if (byte & 0x10) strcat (dst_str,",X");
  if (byte & 0x08) strcat (dst_str,",DP");
  if (byte & 0x04) strcat (dst_str,",B");
  if (byte & 0x02) strcat (dst_str,",A");
  if (byte & 0x01) strcat (dst_str,",CC");
}

static void WritePSHURegister(char *dst_str,u8 byte) {
  if (byte == 0) {strcat (dst_str,"??");return;}
  if (byte & 0x80) strcat (dst_str,",PC");
  if (byte & 0x40) strcat (dst_str,",S");
  if (byte & 0x20) strcat (dst_str,",Y");
  if (byte & 0x10) strcat (dst_str,",X");
  if (byte & 0x08) strcat (dst_str,",DP");
  if (byte & 0x04) strcat (dst_str,",B");
  if (byte & 0x02) strcat (dst_str,",A");
  if (byte & 0x01) strcat (dst_str,",CC");
}

static void WritePULSRegister(char *dst_str,u8 byte) {
  if (byte == 0) {strcat (dst_str,"??");return;}
  if (byte & 0x01) strcat (dst_str,",CC");
  if (byte & 0x02) strcat (dst_str,",A");
  if (byte & 0x04) strcat (dst_str,",B");
  if (byte & 0x08) strcat (dst_str,",DP");
  if (byte & 0x10) strcat (dst_str,",X");
  if (byte & 0x20) strcat (dst_str,",Y");
  if (byte & 0x40) strcat (dst_str,",U");
  if (byte & 0x80) strcat (dst_str,",PC");
}

static void WritePULURegister(char *dst_str,u8 byte) {
  if (byte == 0) {strcat (dst_str,"??");return;}
  if (byte & 0x01) strcat (dst_str,",CC");
  if (byte & 0x02) strcat (dst_str,",A");
  if (byte & 0x04) strcat (dst_str,",B");
  if (byte & 0x08) strcat (dst_str,",DP");
  if (byte & 0x10) strcat (dst_str,",X");
  if (byte & 0x20) strcat (dst_str,",Y");
  if (byte & 0x40) strcat (dst_str,",S");
  if (byte & 0x80) strcat (dst_str,",PC");
}
/*-----------------------------------------------------------------------------
	Dump nb_byte byte from Add, via CPU->ReadMem8, in dst string.
-----------------------------------------------------------------------------*/
static void HexDump(M6809CPU *Cpu,u16 Add,u8 nb_byte,char *dst) {
  switch (nb_byte) {
	case 1:
		sprintf (dst,"%.2X              ",Cpu->ReadMem8 (Cpu, Add));
	break;
	case 2:
		sprintf (dst,"%.2X %.2X           ",Cpu->ReadMem8 (Cpu, Add),Cpu->ReadMem8 (Cpu, Add+1));
	break;
	case 3:
		sprintf (dst,"%.2X %.2X %.2X        ",Cpu->ReadMem8 (Cpu, Add),Cpu->ReadMem8 (Cpu, Add+1),Cpu->ReadMem8 (Cpu, Add+2));
	break;
	case 4:
		sprintf (dst,"%.2X %.2X %.2X %.2X     ",Cpu->ReadMem8 (Cpu, Add),Cpu->ReadMem8 (Cpu, Add+1),Cpu->ReadMem8 (Cpu, Add+2),Cpu->ReadMem8 (Cpu, Add+3));	
	break;
	case 5:
		sprintf (dst,"%.2X %.2X %.2X %.2X %.2X  ",Cpu->ReadMem8 (Cpu, Add),Cpu->ReadMem8 (Cpu, Add+1),Cpu->ReadMem8 (Cpu, Add+2),Cpu->ReadMem8 (Cpu, Add+3),Cpu->ReadMem8 (Cpu, Add+4));	
	break;
	
	default:
		sprintf (dst,"find instruction with len of : %d chars !",nb_byte);
	break;
	}
}
/*-----------------------------------------------------------------------------
	Those functions are used to write instruction operands with specific
	addressing mode. They are attached to an opcode via a pointer in the
	opcode table. All have the same prototype:
	Input: 	Cpu is the concerned ... CPU :-)
		Op is the opcode descriptor, in right table (maybe prefixed).
		Add is the Address of the instruction opcode. (prefix shunted)
		Operands is a string for returning string with Operand disassembly.
		Cycle nbr is a pointer to an u8 to return the number of inst cycle.
	Output: return an int equal to the opcode Size.
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Inherent addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Inherent(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u8 byte;
  if (Op->Size != 1) {	/* Mean that instruction have operands -> EXG,TFR,PSHS,PULS,PSHU,PULU */
	  byte = CPU->ReadMem8 (CPU, Add+1);
	if (Op->Opcode==OP_EXG || Op->Opcode==OP_TFR) {
		sprintf (Operands,"%s,%s",InherentRegister (byte>>4),InherentRegister (byte &0xf));			
  } else {
		switch (Op->Opcode) {
			case OP_PSHS:
				WritePSHSRegister (Operands,byte);
			break;
			case OP_PSHU:
				WritePSHURegister (Operands,byte);
			break;
			case OP_PULS:
				WritePULSRegister (Operands,byte);
			break;
			case OP_PULU:
				WritePULURegister (Operands,byte);
			break;
			case OP_CWAI:
				WriteCWAIRegister (Operands,byte);
			break;
			case OP_SWI:	/* No Operand for prefixed swi2-swi3 Instructions .*/
			break;
			default:
				printf ("Unknown Inherent adressing mode instruction %d.\n ",Op->Opcode);
			break;
			}
		}
	}
  *Cycles_nbr = Op->Clock;	/* Store in Cycles_nbr, cycles total */
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Immediat_8 addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Immediat_8(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u8 byte;
  byte = CPU->ReadMem8(CPU, Add+1);	/* Get next byte:Immediate 8b value */
  sprintf (Operands,"#$%.2X",byte);		
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Immediat_16 addressing mode.
-----------------------------------------------------------------------------*/

static int AM_Immediat_16(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u16 word;
  word =CPU->ReadMem16(CPU, Add+1);	/* Get next byte:Immediate 8b value */
  sprintf (Operands,"#$%.4X",word);		
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Branch_rel_8 addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Branch_Rel_8(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u8 byte;
  u16 word;
  byte = CPU->ReadMem8(CPU, Add+1);	/* Get next byte:Immediate 8b value */
  if (byte<127) {
	  word = Add + 2 + byte;	/* +2 = from byte after Inst + 8bit value */
	} else {
	  word = Add + 2 - (256 - byte);
	}
  sprintf (Operands,"$%.4X",word);		
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Branch_Rel_16 addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Branch_Rel_16(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u16 word;
  word = CPU->ReadMem16(CPU, Add+1);	/* Get next byte:Immediate 8b value */
  if (word<32767) {
	  word = Add + 3 + word;	/* +3 = from byte after Inst + 16bit value */
	} else {
	  word = Add + 3 - (65536 - word);
	}
  sprintf (Operands,"$%.4X",word);		
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Direct addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Direct(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u8 byte;
  byte = CPU->ReadMem8(CPU, Add+1);	/* Get next byte:Immediate 8b value */
  sprintf (Operands,"$%.2X",byte);		
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Extended addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Extended(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u16 word;
  word = CPU->ReadMem16 (CPU, Add+1);
  sprintf (Operands,"$%.4X",word);
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Illegal addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Illegal(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  Add++;
  Operands [0]='\0';
  CPU->ReadMem8 (CPU, Add);
  sprintf (Operands,"??");
  *Cycles_nbr = Op->Clock;
  return (Op->Size);
}
/*
This doc come from: 
-------------------
Description Of The Motorola 6809 Instruction Set
#FILE: m6809.html
#REV: 1.1
#DATE: 01/06/95
#AUTHOR: Paul D. Burgin
  +------------------------------------------------------------------------+
  |          INDEX ADDRESSING POST BYTE REGISTER BIT ASSIGNMENTS           |
  +-------------------------------+--------------------------------+-------+
  |    POST BYTE REGISTER BIT     |                                |  Add  |
  +---+---+---+---+---+---+---+---+          INDEXED MODE          +---+---+
  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |                                | ~ | # |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 0 | R | R | F | F | F | F | F |      (+/- 4 bit offset),R      | 1 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | 0 | 0 | 0 | 0 | 0 |               ,R+              | 2 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 0 | 0 | 0 | 1 |               ,R++             | 3 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | 0 | 0 | 0 | 1 | 0 |               ,-R              | 2 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 0 | 0 | 1 | 1 |               ,--R             | 3 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 0 | 1 | 0 | 0 |               ,R               | 0 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 0 | 1 | 0 | 1 |             (+/- B),R          | 1 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 0 | 1 | 1 | 0 |             (+/- A),R          | 1 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | X | X | X | 0 | 1 | 1 | 1 |              Illegal           | u | u |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 1 | 0 | 0 | 0 |      (+/- 7 bit offset),R      | 1 | 1 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 1 | 0 | 0 | 1 |      (+/- 15 bit offset),R     | 4 | 2 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | X | X | X | 1 | 0 | 1 | 0 |              Illegal           | u | u |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | R | R | I | 1 | 0 | 1 | 1 |             (+/- D),R          | 4 | 0 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | X | X | I | 1 | 1 | 0 | 0 |      (+/- 7 bit offset),PC     | 1 | 1 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | X | X | I | 1 | 1 | 0 | 1 |      (+/- 15 bit offset),PC    | 5 | 2 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | X | X | X | 1 | 1 | 1 | 0 |              Illegal           | u | u |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+
  | 1 | 0 | 0 | 1 | 1 | 1 | 1 | 1 |             [address]          | 5 | 2 |
  +---+---+---+---+---+---+---+---+--------------------------------+---+---+

    Key
    ===

    ~ Additional clock cycles.
    # Additional post bytes.
    u Undefined.
    X Don't Care.
    F Offset.
    I Indirect field.
        0 = Non indirect
        1 = Indirect (add 3 cycles)
    R Register field.
       00 = X
       01 = Y
       10 = U
       11 = S */
/*-----------------------------------------------------------------------------
	Return in string decoded operands, using Indexed addressing mode.
-----------------------------------------------------------------------------*/
static int AM_Indexed(M6809CPU *CPU,M6809Opcode *Op,u16 Add,char *Operands,u8 *Cycles_nbr) {
  u8 extrabyte=0;
  u8 postbyte,byte;
  u16 word;
  char signe;

  postbyte = CPU->ReadMem8 (CPU, Add+1);
  if (!(postbyte & 0x80)) { /*	(+/- 4 bit offset),R -> CONSTANT OFFSET FROM REGISTER	*/
	  byte = (postbyte & 0x1f);
	  if (byte & 0x10) {byte=0x20-byte;signe ='-';}	
	  else signe ='+';
	  sprintf (Operands,"%c$%.2X,%s",signe,byte,IndexRegister(postbyte));	
	} else {
	  switch (postbyte & 0x1f) {
		case 0x00:	/* ,R+ -> POSTINCREMENT FROM REGISTER */ 
			sprintf (Operands,",%s+",IndexRegister(postbyte));	
		break;
		case 0x01:	/* ,R++ -> POSTINCREMENT FROM REGISTER */ 
			sprintf (Operands,",%s++ ",IndexRegister(postbyte));	
		break;
		case 0x02:	/* ,-R -> PRE DECREMENT FROM REGISTER */ 
			sprintf (Operands,",-%s",IndexRegister(postbyte));	
		break;
		case 0x03:	/* ,--R -> PRE DECREMENT FROM REGISTER */ 
			sprintf (Operands,",--%s",IndexRegister(postbyte));	
		break;
		case 0x04:	/* ,R ->  FROM REGISTER */ 
			sprintf (Operands,",%s",IndexRegister(postbyte));	
		break;
		case 0x05:	/* B,R -> ACCUMULATOR OFFSET FROM REGISTER */ 
			sprintf (Operands,"B,%s",IndexRegister(postbyte));	
		break;
		case 0x06:	/* A,R -> ACCUMULATOR OFFSET FROM REGISTER */ 
			sprintf (Operands,"A,%s",IndexRegister(postbyte));	
		break;
		case 0x07:	/* ILLEGAL ADRESSING MODE */ 
			sprintf (Operands,"??");	
		break;	
		case 0x08:	/* (+/- 7 bit offset),R Display sign ?*/ 
			byte = CPU->ReadMem8 (CPU, Add+2);
			if (byte>127) {signe = '-';byte=0x0100-byte;}
			else signe = '+';
			sprintf (Operands,"%c$%2.X,%s",signe,byte,IndexRegister (postbyte));	
			extrabyte = 1;
		break;
		case 0x09:	/* (+/- 15 bit offset),R  */ 
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"$%.4X,%s",word,IndexRegister (postbyte));	
			extrabyte = 2;
		break;
		case 0x0a:	/* ILLEGAL ADRESSING MODE ?? */ 
			sprintf (Operands,"??");	
		break;
		case 0x0b:	/* D,R ->  FROM REGISTER */ 
			sprintf (Operands,"D,%s",IndexRegister(postbyte));	
		break;
		case 0x0c:	/* (+/- 7 bit offset),PC */ 
			byte = CPU->ReadMem8 (CPU, Add+2);
			if (byte>127) {signe = '-';byte=0x0100-byte;}
			else signe = '+';
			sprintf (Operands,"%c$%2.X,PC",signe,byte);	
			extrabyte = 1;
		break;
		case 0x0d:	/* (+/- 15 bit offset),PC */ 
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"$%.4X,PC",word);	
			extrabyte = 2;
		break;
		case 0x0e:	/* ILLEGAL ADRESSING MODE */ 
			sprintf (Operands,"??");
		break;
		case 0x0f:	/* $XXXX,R */ 
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"$%.4X,%s",word,IndexRegister(postbyte));	
			extrabyte = 2;
		break;
		case 0x10:	/* ADRESSING MODE [,R+] */ 
			sprintf (Operands,"[,%s+]",IndexRegister (postbyte));	
		break;
		case 0x11:	/* ADRESSING MODE [,R++] */ 
			sprintf (Operands,"[,%s++]",IndexRegister (postbyte));	
		break;
		case 0x12:	/* ADRESSING MODE [,-R] */ 
		sprintf (Operands,"[,-%s]",IndexRegister (postbyte));	
		break;
		case 0x13:	/* ADRESSING MODE [,--R] */ 
			sprintf (Operands,"[,--%s]",IndexRegister (postbyte));	
		break;
		case 0x14:	/* ADRESSING MODE [,R] */ 
			sprintf (Operands,"[,%s]",IndexRegister (postbyte));	
		break;
		case 0x15:	/* ILLEGAL ADRESSING MODE OR [B,R] ?*/ 
			sprintf (Operands,"[B,%s]",IndexRegister (postbyte));	
		break;
		case 0x16:	/* ILLEGAL ADRESSING MODE OR [A,R] ?*/ 
			sprintf (Operands,"[A,%s]",IndexRegister (postbyte));	
		break;
		case 0x17:	/* ILLEGAL ADRESSING MODE */ 
			sprintf (Operands,"??");	
		break;
		case 0x18:	/* (+/- 7 bit offset),R */ 
			byte = CPU->ReadMem8 (CPU, Add+2);
			if (byte>127) {signe = '-';byte=0x0100-byte;}
			else signe = '+';
			sprintf (Operands,"[%c$%2.X,%s]",signe,byte,IndexRegister (postbyte));	
			extrabyte = 1;
		break;
		case 0x19:	/* (+/- 15 bit offset),R */ 
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"[$%.4X,%s]",word,IndexRegister (postbyte));	
			extrabyte = 2;
		break;
		case 0x1a:	/* ILLEGAL ADRESSING MODE */ 
			sprintf (Operands,"??");	
		break;
		case 0x1b:	/* [D,R] ->  FROM REGISTER */ 
			sprintf (Operands,"[D,%s]",IndexRegister(postbyte));	
		break;
		case 0x1c:	/* [(+/- 7 bit offset),PCR]  */ 
			byte = CPU->ReadMem8 (CPU, Add+2);
			if (byte>127) {signe = '-';byte=0x0100-byte;}
			else signe = '+';
			sprintf (Operands,"[%c$%.2X,PC]",signe,byte);	
			extrabyte = 1;
		break;
		case 0x1d:	/* [(+/- 15 bit offset),PCR]  */ 
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"[$%.4X,PC]",word);	
			extrabyte = 2;
		break;

		case 0x1e:	/* ILLEGAL ADRESSING MODE */ 
			sprintf (Operands,"??");	
		break;
		case 0x1f:	/* [XXXX]*/
			word = CPU->ReadMem16 (CPU, Add+2);
			sprintf (Operands,"[$%.4X]",word);	
			extrabyte = 2;
		break;
		default:
			sprintf (Operands,"UNSUPPORTED INDEXED ADRESSING MODE");	
		break;		
		}		
	}
  *Cycles_nbr = Op->Clock;

  return (Op->Size+extrabyte);
}

void m6809_dasm(void *data, int addr, char *buf) {
  M6809CPU CPU;
  u8 Cycles;

  CPU.ReadMem8 = &RdMem8;
  CPU.ReadMem16 = &RdMem16;
  CPU.data = data;

  Dasm6809(&CPU, addr, buf, &Cycles);
}
