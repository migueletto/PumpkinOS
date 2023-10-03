#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "pwindow.h"
#include "filter.h"
#include "computer.h"
#include "z80disasm.h"

typedef struct {
  char *name;
  int args;
} opcode_t;

#define A_0         0   // No arguments
#define A_8         1   // 8-bit number
#define A_16        2   // 16-bit number
#define A_0B     0xff   // No arguments, backskip over last opcode byte
#define A_8P    0x100   // 8-bit number preceding last opcode byte
#define A_8R    0x101   // 8-bit relative address
#define A_8X2   0x102   // Two 8-bit numbers

#define undefined "undefined"

static opcode_t major[] = {
    { "nop",        A_0}, /*00 */
    { "ld   bc,%02X%02Xh",  A_16},     /* 01 */
    { "ld   (bc),a",    A_0}, /*02 */
    { "inc  bc",        A_0}, /*03 */
    { "inc  b",     A_0}, /*04 */
    { "dec  b",     A_0}, /*05 */
    { "ld   b,%02Xh",   A_8}, /*06 */
    { "rlca",       A_0}, /*07 */

    { "ex   af,af'",    A_0}, /*08 */
    { "add  hl,bc",     A_0}, /*09 */
    { "ld   a,(bc)",    A_0}, /*0a */
    { "dec  bc",        A_0}, /*0b */
    { "inc  c",     A_0}, /*0c */
    { "dec  c",     A_0}, /*0d */
    { "ld   c,%02Xh",   A_8}, /*0e */
    { "rrca",       A_0}, /*0f */

    { "djnz %04Xh",     A_8R},     /* 10 */
    { "ld   de,%02X%02Xh",  A_16},     /* 11 */
    { "ld   (de),a",    A_0}, /*12 */
    { "inc  de",        A_0}, /*13 */
    { "inc  d",     A_0}, /*14 */
    { "dec  d",     A_0}, /*15 */
    { "ld   d,%02Xh",   A_8}, /*16 */
    { "rla",        A_0}, /*17 */

    { "jr   %04Xh",     A_8R},     /* 18 */
    { "add  hl,de",     A_0}, /*19 */
    { "ld   a,(de)",    A_0}, /*1a */
    { "dec  de",        A_0}, /*1b */
    { "inc  e",     A_0}, /*1c */
    { "dec  e",     A_0}, /*1d */
    { "ld   e,%02Xh",   A_8}, /*1e */
    { "rra",        A_0}, /*1f */

    { "jr   nz,%04Xh",  A_8R},     /* 20 */
    { "ld   hl,%02X%02Xh",  A_16},     /* 21 */
    { "ld   (%02X%02Xh),hl",A_16},     /* 22 */
    { "inc  hl",        A_0}, /*23 */
    { "inc  h",     A_0}, /*24 */
    { "dec  h",     A_0}, /*25 */
    { "ld   h,%02Xh",   A_8}, /*26 */
    { "daa",        A_0}, /*27 */

    { "jr   z,%04Xh",   A_8R},     /* 28 */
    { "add  hl,hl",     A_0}, /*29 */
    { "ld   hl,(%02X%02Xh)",A_16},     /* 2a */
    { "dec  hl",        A_0}, /*2b */
    { "inc  l",     A_0}, /*2c */
    { "dec  l",     A_0}, /*2d */
    { "ld   l,%02Xh",   A_8}, /*2e */
    { "cpl",        A_0}, /*2f */

    { "jr   nc,%04Xh",  A_8R},     /* 30 */
    { "ld   sp,%02X%02Xh",  A_16},     /* 31 */
    { "ld   (%02X%02Xh),a", A_16},     /* 32 */
    { "inc  sp",        A_0}, /*33 */
    { "inc  (hl)",      A_0}, /*34 */
    { "dec  (hl)",      A_0}, /*35 */
    { "ld   (hl),%02Xh",    A_8}, /*36 */
    { "scf",        A_0}, /*37 */

    { "jr   c,%04Xh",   A_8R},     /* 38 */
    { "add  hl,sp",     A_0}, /*39 */
    { "ld   a,(%02X%02Xh)", A_16},     /* 3a */
    { "dec  sp",        A_0}, /*3b */
    { "inc  a",     A_0}, /*3c */
    { "dec  a",     A_0}, /*3d */
    { "ld   a,%02Xh",   A_8}, /*3e */
    { "ccf",        A_0}, /*3f */

    { "ld   b,b",       A_0}, /*40 */
    { "ld   b,c",       A_0}, /*41 */
    { "ld   b,d",       A_0}, /*42 */
    { "ld   b,e",       A_0}, /*43 */
    { "ld   b,h",       A_0}, /*44 */
    { "ld   b,l",       A_0}, /*45 */
    { "ld   b,(hl)",    A_0}, /*46 */
    { "ld   b,a",       A_0}, /*47 */

    { "ld   c,b",       A_0}, /*48 */
    { "ld   c,c",       A_0}, /*49 */
    { "ld   c,d",       A_0}, /*4a */
    { "ld   c,e",       A_0}, /*4b */
    { "ld   c,h",       A_0}, /*4c */
    { "ld   c,l",       A_0}, /*4d */
    { "ld   c,(hl)",    A_0}, /*4e */
    { "ld   c,a",       A_0}, /*4f */

    { "ld   d,b",       A_0}, /*50 */
    { "ld   d,c",       A_0}, /*51 */
    { "ld   d,d",       A_0}, /*52 */
    { "ld   d,e",       A_0}, /*53 */
    { "ld   d,h",       A_0}, /*54 */
    { "ld   d,l",       A_0}, /*55 */
    { "ld   d,(hl)",    A_0}, /*56 */
    { "ld   d,a",       A_0}, /*57 */

    { "ld   e,b",       A_0}, /*58 */
    { "ld   e,c",       A_0}, /*59 */
    { "ld   e,d",       A_0}, /*5a */
    { "ld   e,e",       A_0}, /*5b */
    { "ld   e,h",       A_0}, /*5c */
    { "ld   e,l",       A_0}, /*5d */
    { "ld   e,(hl)",    A_0}, /*5e */
    { "ld   e,a",       A_0}, /*5f */

    { "ld   h,b",       A_0}, /*60 */
    { "ld   h,c",       A_0}, /*61 */
    { "ld   h,d",       A_0}, /*62 */
    { "ld   h,e",       A_0}, /*63 */
    { "ld   h,h",       A_0}, /*64 */
    { "ld   h,l",       A_0}, /*65 */
    { "ld   h,(hl)",    A_0}, /*66 */
    { "ld   h,a",       A_0}, /*67 */

    { "ld   l,b",       A_0}, /*68 */
    { "ld   l,c",       A_0}, /*69 */
    { "ld   l,d",       A_0}, /*6a */
    { "ld   l,e",       A_0}, /*6b */
    { "ld   l,h",       A_0}, /*6c */
    { "ld   l,l",       A_0}, /*6d */
    { "ld   l,(hl)",    A_0}, /*6e */
    { "ld   l,a",       A_0}, /*6f */

    { "ld   (hl),b",    A_0}, /*70 */
    { "ld   (hl),c",    A_0}, /*71 */
    { "ld   (hl),d",    A_0}, /*72 */
    { "ld   (hl),e",    A_0}, /*73 */
    { "ld   (hl),h",    A_0}, /*74 */
    { "ld   (hl),l",    A_0}, /*75 */
    { "halt",       A_0}, /*76 */
    { "ld   (hl),a",    A_0}, /*77 */

    { "ld   a,b",       A_0}, /*78 */
    { "ld   a,c",       A_0}, /*79 */
    { "ld   a,d",       A_0}, /*7a */
    { "ld   a,e",       A_0}, /*7b */
    { "ld   a,h",       A_0}, /*7c */
    { "ld   a,l",       A_0}, /*7d */
    { "ld   a,(hl)",    A_0}, /*7e */
    { "ld   a,a",       A_0}, /*7f */

    { "add  a,b",       A_0}, /*80 */
    { "add  a,c",       A_0}, /*81 */
    { "add  a,d",       A_0}, /*82 */
    { "add  a,e",       A_0}, /*83 */
    { "add  a,h",       A_0}, /*84 */
    { "add  a,l",       A_0}, /*85 */
    { "add  a,(hl)",    A_0}, /*86 */
    { "add  a,a",       A_0}, /*87 */

    { "adc  a,b",       A_0}, /*88 */
    { "adc  a,c",       A_0}, /*89 */
    { "adc  a,d",       A_0}, /*8a */
    { "adc  a,e",       A_0}, /*8b */
    { "adc  a,h",       A_0}, /*8c */
    { "adc  a,l",       A_0}, /*8d */
    { "adc  a,(hl)",    A_0}, /*8e */
    { "adc  a,a",       A_0}, /*8f */

    { "sub  b",     A_0}, /*90 */
    { "sub  c",     A_0}, /*91 */
    { "sub  d",     A_0}, /*92 */
    { "sub  e",     A_0}, /*93 */
    { "sub  h",     A_0}, /*94 */
    { "sub  l",     A_0}, /*95 */
    { "sub  (hl)",      A_0}, /*96 */
    { "sub  a",     A_0}, /*97 */

    { "sbc  a,b",       A_0}, /*98 */
    { "sbc  a,c",       A_0}, /*99 */
    { "sbc  a,d",       A_0}, /*9a */
    { "sbc  a,e",       A_0}, /*9b */
    { "sbc  a,h",       A_0}, /*9c */
    { "sbc  a,l",       A_0}, /*9d */
    { "sbc  a,(hl)",    A_0}, /*9e */
    { "sbc  a,a",       A_0}, /*9f */

    { "and  b",     A_0}, /*a0 */
    { "and  c",     A_0}, /*a1 */
    { "and  d",     A_0}, /*a2 */
    { "and  e",     A_0}, /*a3 */
    { "and  h",     A_0}, /*a4 */
    { "and  l",     A_0}, /*a5 */
    { "and  (hl)",      A_0}, /*a6 */
    { "and  a",     A_0}, /*a7 */

    { "xor  b",     A_0}, /*a8 */
    { "xor  c",     A_0}, /*a9 */
    { "xor  d",     A_0}, /*aa */
    { "xor  e",     A_0}, /*ab */
    { "xor  h",     A_0}, /*ac */
    { "xor  l",     A_0}, /*ad */
    { "xor  (hl)",      A_0}, /*ae */
    { "xor  a",     A_0}, /*af */

    { "or   b",     A_0}, /*b0 */
    { "or   c",     A_0}, /*b1 */
    { "or   d",     A_0}, /*b2 */
    { "or   e",     A_0}, /*b3 */
    { "or   h",     A_0}, /*b4 */
    { "or   l",     A_0}, /*b5 */
    { "or   (hl)",      A_0}, /*b6 */
    { "or   a",     A_0}, /*b7 */

    { "cp   b",     A_0}, /*b8 */
    { "cp   c",     A_0}, /*b9 */
    { "cp   d",     A_0}, /*ba */
    { "cp   e",     A_0}, /*bb */
    { "cp   h",     A_0}, /*bc */
    { "cp   l",     A_0}, /*bd */
    { "cp   (hl)",      A_0}, /*be */
    { "cp   a",     A_0}, /*bf */

    { "ret  nz",        A_0}, /*c0 */
    { "pop  bc",        A_0}, /*c1 */
    { "jp   nz,%02X%02Xh",  A_16},     /* c2 */
    { "jp   %02X%02Xh", A_16},     /* c3 */
    { "call nz,%02X%02Xh",  A_16},     /* c4 */
    { "push bc",        A_0}, /*c5 */
    { "add  a,%02Xh",   A_8}, /*c6 */
    { "rst  0",     A_0}, /*c7 */

    { "ret  z",     A_0}, /*c8 */
    { "ret",        A_0}, /*c9 */
    { "jp   z,%02X%02Xh",   A_16},     /* ca */
    { NULL,          0 },        /* cb */
    { "call z,%02X%02Xh",   A_16},     /* cc */
    { "call %02X%02Xh", A_16},     /* cd */
    { "adc  a,%02Xh",   A_8}, /*ce */
    { "rst  8",     A_0}, /*cf */

    { "ret  nc",        A_0}, /*d0 */
    { "pop  de",        A_0}, /*d1 */
    { "jp   nc,%02X%02Xh",  A_16},     /* d2 */
    { "out  (%02Xh),a", A_8}, /*d3 */
    { "call nc,%02X%02Xh",  A_16},     /* d4 */
    { "push de",        A_0}, /*d5 */
    { "sub  %02Xh",     A_8}, /*d6 */
    { "rst  10h",       A_0}, /*d7 */

    { "ret  c",     A_0}, /*d8 */
    { "exx",        A_0}, /*d9 */
    { "jp   c,%02X%02Xh",   A_16},     /* da */
    { "in   a,(%02Xh)", A_8}, /*db */
    { "call c,%02X%02Xh",   A_16},     /* dc */
    { NULL,          1 },        /* dd */
    { "sbc  a,%02Xh",   A_8}, /*de */
    { "rst  18h",       A_0}, /*df */

    { "ret  po",        A_0}, /*e0 */
    { "pop  hl",        A_0}, /*e1 */
    { "jp   po,%02X%02Xh",  A_16},     /* e2 */
    { "ex   (sp),hl",   A_0}, /*e3 */
    { "call po,%02X%02Xh",  A_16},     /* e4 */
    { "push hl",        A_0}, /*e5 */
    { "and  %02Xh",     A_8}, /*e6 */
    { "rst  20h",       A_0}, /*e7 */

    { "ret  pe",        A_0}, /*e8 */
    { "jp   (hl)",      A_0}, /*e9 */
    { "jp   pe,%02X%02Xh",  A_16},     /* ea */
    { "ex   de,hl",     A_0}, /*eb */
    { "call pe,%02X%02Xh",  A_16},     /* ec */
    { NULL,          2 },        /* ed */
    { "xor  %02Xh",     A_8}, /*ee */
    { "rst  28h",       A_0}, /*ef */

    { "ret  p",     A_0}, /*f0 */
    { "pop  af",        A_0}, /*f1 */
    { "jp   p,%02X%02Xh",   A_16},     /* f2 */
    { "di",         A_0}, /*f3 */
    { "call p,%02X%02Xh",   A_16},     /* f4 */
    { "push af",        A_0}, /*f5 */
    { "or   %02Xh",     A_8}, /*f6 */
    { "rst  30h",       A_0}, /*f7 */

    { "ret  m",     A_0}, /*f8 */
    { "ld   sp,hl",     A_0}, /*f9 */
    { "jp   m,%02X%02Xh",   A_16},     /* fa */
    { "ei",         A_0}, /*fb */
    { "call m,%02X%02Xh",   A_16},     /* fc */
    { NULL,          3},        /* fd */
    { "cp   %02Xh",     A_8}, /*fe */
    { "rst  38h",       A_0}, /*ff */
};

static opcode_t minor[6][256] = {
    {
      { "rlc  b",     A_0 },      /* cb00 */
      { "rlc  c",     A_0 },      /* cb01 */
      { "rlc  d",     A_0 },      /* cb02 */
      { "rlc  e",     A_0 },      /* cb03 */
      { "rlc  h",     A_0 },      /* cb04 */
      { "rlc  l",     A_0 },      /* cb05 */
      { "rlc  (hl)",      A_0 },      /* cb06 */
      { "rlc  a",     A_0 },      /* cb07 */

      { "rrc  b",     A_0},      /* cb08 */
      { "rrc  c",     A_0},      /* cb09 */
      { "rrc  d",     A_0},      /* cb0a */
      { "rrc  e",     A_0},      /* cb0b */
      { "rrc  h",     A_0},      /* cb0c */
      { "rrc  l",     A_0},      /* cb0d */
      { "rrc  (hl)",      A_0},      /* cb0e */
      { "rrc  a",     A_0},      /* cb0f */

      { "rl   b",     A_0},      /* cb10 */
      { "rl   c",     A_0},      /* cb11 */
      { "rl   d",     A_0},      /* cb12 */
      { "rl   e",     A_0},      /* cb13 */
      { "rl   h",     A_0},      /* cb14 */
      { "rl   l",     A_0},      /* cb15 */
      { "rl   (hl)",      A_0},      /* cb16 */
      { "rl   a",     A_0},      /* cb17 */

      { "rr   b",     A_0},      /* cb18 */
      { "rr   c",     A_0},      /* cb19 */
      { "rr   d",     A_0},      /* cb1a */
      { "rr   e",     A_0},      /* cb1b */
      { "rr   h",     A_0},      /* cb1c */
      { "rr   l",     A_0},      /* cb1d */
      { "rr   (hl)",      A_0},      /* cb1e */
      { "rr   a",     A_0},      /* cb1f */

      { "sla  b",     A_0},      /* cb20 */
      { "sla  c",     A_0},      /* cb21 */
      { "sla  d",     A_0},      /* cb22 */
      { "sla  e",     A_0},      /* cb23 */
      { "sla  h",     A_0},      /* cb24 */
      { "sla  l",     A_0},      /* cb25 */
      { "sla  (hl)",      A_0},      /* cb26 */
      { "sla  a",     A_0},      /* cb27 */

      { "sra  b",     A_0},      /* cb28 */
      { "sra  c",     A_0},      /* cb29 */
      { "sra  d",     A_0},      /* cb2a */
      { "sra  e",     A_0},      /* cb2b */
      { "sra  h",     A_0},      /* cb2c */
      { "sra  l",     A_0},      /* cb2d */
      { "sra  (hl)",      A_0},      /* cb2e */
      { "sra  a",     A_0},      /* cb2f */

      { "slia b",     A_0},      /* cb30 [undoc] */
      { "slia c",     A_0},      /* cb31 [undoc] */
      { "slia d",     A_0},      /* cb32 [undoc] */
      { "slia e",     A_0},      /* cb33 [undoc] */
      { "slia h",     A_0},      /* cb34 [undoc] */
      { "slia l",     A_0},      /* cb35 [undoc] */
      { "slia (hl)",      A_0},      /* cb36 [undoc] */
      { "slia a",     A_0},      /* cb37 [undoc] */

      { "srl  b",     A_0},      /* cb38 */
      { "srl  c",     A_0},      /* cb39 */
      { "srl  d",     A_0},      /* cb3a */
      { "srl  e",     A_0},      /* cb3b */
      { "srl  h",     A_0},      /* cb3c */
      { "srl  l",     A_0},      /* cb3d */
      { "srl  (hl)",      A_0},      /* cb3e */
      { "srl  a",     A_0},      /* cb3f */

      { "bit  0,b",       A_0},      /* cb40 */
      { "bit  0,c",       A_0},      /* cb41 */
      { "bit  0,d",       A_0},      /* cb42 */
      { "bit  0,e",       A_0},      /* cb43 */
      { "bit  0,h",       A_0},      /* cb44 */
      { "bit  0,l",       A_0},      /* cb45 */
      { "bit  0,(hl)",    A_0},      /* cb46 */
      { "bit  0,a",       A_0},      /* cb47 */

      { "bit  1,b",       A_0},      /* cb48 */
      { "bit  1,c",       A_0},      /* cb49 */
      { "bit  1,d",       A_0},      /* cb4a */
      { "bit  1,e",       A_0},      /* cb4b */
      { "bit  1,h",       A_0},      /* cb4c */
      { "bit  1,l",       A_0},      /* cb4d */
      { "bit  1,(hl)",    A_0},      /* cb4e */
      { "bit  1,a",       A_0},      /* cb4f */

      { "bit  2,b",       A_0},      /* cb50 */
      { "bit  2,c",       A_0},      /* cb51 */
      { "bit  2,d",       A_0},      /* cb52 */
      { "bit  2,e",       A_0},      /* cb53 */
      { "bit  2,h",       A_0},      /* cb54 */
      { "bit  2,l",       A_0},      /* cb55 */
      { "bit  2,(hl)",    A_0},      /* cb56 */
      { "bit  2,a",       A_0},      /* cb57 */

      { "bit  3,b",       A_0},      /* cb58 */
      { "bit  3,c",       A_0},      /* cb59 */
      { "bit  3,d",       A_0},      /* cb5a */
      { "bit  3,e",       A_0},      /* cb5b */
      { "bit  3,h",       A_0},      /* cb5c */
      { "bit  3,l",       A_0},      /* cb5d */
      { "bit  3,(hl)",    A_0},      /* cb5e */
      { "bit  3,a",       A_0},      /* cb5f */

      { "bit  4,b",       A_0},      /* cb60 */
      { "bit  4,c",       A_0},      /* cb61 */
      { "bit  4,d",       A_0},      /* cb62 */
      { "bit  4,e",       A_0},      /* cb63 */
      { "bit  4,h",       A_0},      /* cb64 */
      { "bit  4,l",       A_0},      /* cb65 */
      { "bit  4,(hl)",    A_0},      /* cb66 */
      { "bit  4,a",       A_0},      /* cb67 */

      { "bit  5,b",       A_0},      /* cb68 */
      { "bit  5,c",       A_0},      /* cb69 */
      { "bit  5,d",       A_0},      /* cb6a */
      { "bit  5,e",       A_0},      /* cb6b */
      { "bit  5,h",       A_0},      /* cb6c */
      { "bit  5,l",       A_0},      /* cb6d */
      { "bit  5,(hl)",    A_0},      /* cb6e */
      { "bit  5,a",       A_0},      /* cb6f */

      { "bit  6,b",       A_0},      /* cb70 */
      { "bit  6,c",       A_0},      /* cb71 */
      { "bit  6,d",       A_0},      /* cb72 */
      { "bit  6,e",       A_0},      /* cb73 */
      { "bit  6,h",       A_0},      /* cb74 */
      { "bit  6,l",       A_0},      /* cb75 */
      { "bit  6,(hl)",    A_0},      /* cb76 */
      { "bit  6,a",       A_0},      /* cb77 */

      { "bit  7,b",       A_0},      /* cb78 */
      { "bit  7,c",       A_0},      /* cb79 */
      { "bit  7,d",       A_0},      /* cb7a */
      { "bit  7,e",       A_0},      /* cb7b */
      { "bit  7,h",       A_0},      /* cb7c */
      { "bit  7,l",       A_0},      /* cb7d */
      { "bit  7,(hl)",    A_0},      /* cb7e */
      { "bit  7,a",       A_0},      /* cb7f */

      { "res  0,b",       A_0},      /* cb80 */
      { "res  0,c",       A_0},      /* cb81 */
      { "res  0,d",       A_0},      /* cb82 */
      { "res  0,e",       A_0},      /* cb83 */
      { "res  0,h",       A_0},      /* cb84 */
      { "res  0,l",       A_0},      /* cb85 */
      { "res  0,(hl)",    A_0},      /* cb86 */
      { "res  0,a",       A_0},      /* cb87 */

      { "res  1,b",       A_0},      /* cb88 */
      { "res  1,c",       A_0},      /* cb89 */
      { "res  1,d",       A_0},      /* cb8a */
      { "res  1,e",       A_0},      /* cb8b */
      { "res  1,h",       A_0},      /* cb8c */
      { "res  1,l",       A_0},      /* cb8d */
      { "res  1,(hl)",    A_0},      /* cb8e */
      { "res  1,a",       A_0},      /* cb8f */

      { "res  2,b",       A_0},      /* cb90 */
      { "res  2,c",       A_0},      /* cb91 */
      { "res  2,d",       A_0},      /* cb92 */
      { "res  2,e",       A_0},      /* cb93 */
      { "res  2,h",       A_0},      /* cb94 */
      { "res  2,l",       A_0},      /* cb95 */
      { "res  2,(hl)",    A_0},      /* cb96 */
      { "res  2,a",       A_0},      /* cb97 */

      { "res  3,b",       A_0},      /* cb98 */
      { "res  3,c",       A_0},      /* cb99 */
      { "res  3,d",       A_0},      /* cb9a */
      { "res  3,e",       A_0},      /* cb9b */
      { "res  3,h",       A_0},      /* cb9c */
      { "res  3,l",       A_0},      /* cb9d */
      { "res  3,(hl)",    A_0},      /* cb9e */
      { "res  3,a",       A_0},      /* cb9f */

      { "res  4,b",       A_0},      /* cba0 */
      { "res  4,c",       A_0},      /* cba1 */
      { "res  4,d",       A_0},      /* cba2 */
      { "res  4,e",       A_0},      /* cba3 */
      { "res  4,h",       A_0},      /* cba4 */
      { "res  4,l",       A_0},      /* cba5 */
      { "res  4,(hl)",    A_0},      /* cba6 */
      { "res  4,a",       A_0},      /* cba7 */

      { "res  5,b",       A_0},      /* cba8 */
      { "res  5,c",       A_0},      /* cba9 */
      { "res  5,d",       A_0},      /* cbaa */
      { "res  5,e",       A_0},      /* cbab */
      { "res  5,h",       A_0},      /* cbac */
      { "res  5,l",       A_0},      /* cbad */
      { "res  5,(hl)",    A_0},      /* cbae */
      { "res  5,a",       A_0},      /* cbaf */

      { "res  6,b",       A_0},      /* cbb0 */
      { "res  6,c",       A_0},      /* cbb1 */
      { "res  6,d",       A_0},      /* cbb2 */
      { "res  6,e",       A_0},      /* cbb3 */
      { "res  6,h",       A_0},      /* cbb4 */
      { "res  6,l",       A_0},      /* cbb5 */
      { "res  6,(hl)",    A_0},      /* cbb6 */
      { "res  6,a",       A_0},      /* cbb7 */

      { "res  7,b",       A_0},      /* cbb8 */
      { "res  7,c",       A_0},      /* cbb9 */
      { "res  7,d",       A_0},      /* cbba */
      { "res  7,e",       A_0},      /* cbbb */
      { "res  7,h",       A_0},      /* cbbc */
      { "res  7,l",       A_0},      /* cbbd */
      { "res  7,(hl)",    A_0},      /* cbbe */
      { "res  7,a",       A_0},      /* cbbf */

      { "set  0,b",       A_0},      /* cbc0 */
      { "set  0,c",       A_0},      /* cbc1 */
      { "set  0,d",       A_0},      /* cbc2 */
      { "set  0,e",       A_0},      /* cbc3 */
      { "set  0,h",       A_0},      /* cbc4 */
      { "set  0,l",       A_0},      /* cbc5 */
      { "set  0,(hl)",    A_0},      /* cbc6 */
      { "set  0,a",       A_0},      /* cbc7 */

      { "set  1,b",       A_0},      /* cbc8 */
      { "set  1,c",       A_0},      /* cbc9 */
      { "set  1,d",       A_0},      /* cbca */
      { "set  1,e",       A_0},      /* cbcb */
      { "set  1,h",       A_0},      /* cbcc */
      { "set  1,l",       A_0},      /* cbcd */
      { "set  1,(hl)",    A_0},      /* cbce */
      { "set  1,a",       A_0},      /* cbcf */

      { "set  2,b",       A_0},      /* cbd0 */
      { "set  2,c",       A_0},      /* cbd1 */
      { "set  2,d",       A_0},      /* cbd2 */
      { "set  2,e",       A_0},      /* cbd3 */
      { "set  2,h",       A_0},      /* cbd4 */
      { "set  2,l",       A_0},      /* cbd5 */
      { "set  2,(hl)",    A_0},      /* cbd6 */
      { "set  2,a",       A_0},      /* cbd7 */

      { "set  3,b",       A_0},      /* cbd8 */
      { "set  3,c",       A_0},      /* cbd9 */
      { "set  3,d",       A_0},      /* cbda */
      { "set  3,e",       A_0},      /* cbdb */
      { "set  3,h",       A_0},      /* cbdc */
      { "set  3,l",       A_0},      /* cbdd */
      { "set  3,(hl)",    A_0},      /* cbde */
      { "set  3,a",       A_0},      /* cbdf */

      { "set  4,b",       A_0},      /* cbe0 */
      { "set  4,c",       A_0},      /* cbe1 */
      { "set  4,d",       A_0},      /* cbe2 */
      { "set  4,e",       A_0},      /* cbe3 */
      { "set  4,h",       A_0},      /* cbe4 */
      { "set  4,l",       A_0},      /* cbe5 */
      { "set  4,(hl)",    A_0},      /* cbe6 */
      { "set  4,a",       A_0},      /* cbe7 */

      { "set  5,b",       A_0},      /* cbe8 */
      { "set  5,c",       A_0},      /* cbe9 */
      { "set  5,d",       A_0},      /* cbea */
      { "set  5,e",       A_0},      /* cbeb */
      { "set  5,h",       A_0},      /* cbec */
      { "set  5,l",       A_0},      /* cbed */
      { "set  5,(hl)",    A_0},      /* cbee */
      { "set  5,a",       A_0},      /* cbef */

      { "set  6,b",       A_0},      /* cbf0 */
      { "set  6,c",       A_0},      /* cbf1 */
      { "set  6,d",       A_0},      /* cbf2 */
      { "set  6,e",       A_0},      /* cbf3 */
      { "set  6,h",       A_0},      /* cbf4 */
      { "set  6,l",       A_0},      /* cbf5 */
      { "set  6,(hl)",    A_0},      /* cbf6 */
      { "set  6,a",       A_0},      /* cbf7 */

      { "set  7,b",       A_0},      /* cbf8 */
      { "set  7,c",       A_0},      /* cbf9 */
      { "set  7,d",       A_0},      /* cbfa */
      { "set  7,e",       A_0},      /* cbfb */
      { "set  7,h",       A_0},      /* cbfc */
      { "set  7,l",       A_0},      /* cbfd */
      { "set  7,(hl)",    A_0},      /* cbfe */
      { "set  7,a",       A_0}      /* cbff */
    },
    {
      { undefined, A_0B},     /* dd00 */
      { undefined, A_0B},     /* dd01 */
      { undefined, A_0B},     /* dd02 */
      { undefined, A_0B},     /* dd03 */
      { undefined, A_0B},     /* dd04 */
      { undefined, A_0B},     /* dd05 */
      { undefined, A_0B},     /* dd06 */
      { undefined, A_0B},     /* dd07 */

      { undefined, A_0B},     /* dd08 */
      { "add  ix,bc",     A_0},      /* dd09 */
      { undefined, A_0B},     /* dd0a */
      { undefined, A_0B},     /* dd0b */
      { undefined, A_0B},     /* dd0c */
      { undefined, A_0B},     /* dd0d */
      { undefined, A_0B},     /* dd0e */
      { undefined, A_0B},     /* dd0f */

      { undefined, A_0B},     /* dd10 */
      { undefined, A_0B},     /* dd11 */
      { undefined, A_0B},     /* dd12 */
      { undefined, A_0B},     /* dd13 */
      { undefined, A_0B},     /* dd14 */
      { undefined, A_0B},     /* dd15 */
      { undefined, A_0B},     /* dd16 */
      { undefined, A_0B},     /* dd17 */

      { undefined, A_0B},     /* dd18 */
      { "add  ix,de",     A_0},      /* dd19 */
      { undefined, A_0B},     /* dd1a */
      { undefined, A_0B},     /* dd1b */
      { undefined, A_0B},     /* dd1c */
      { undefined, A_0B},     /* dd1d */
      { undefined, A_0B},     /* dd1e */
      { undefined, A_0B},     /* dd1f */

      { undefined, A_0B},     /* dd20 */
      { "ld   ix,%02X%02Xh",  A_16},     /* dd21 */
      { "ld   (%02X%02Xh),ix",A_16},     /* dd22 */
      { "inc  ix",        A_0},      /* dd23 */
      { "inc  ixh",       A_0},      /* dd24 [undoc] */
      { "dec  ixh",       A_0},      /* dd25 [undoc] */
      { "ld   ixh,%02Xh", A_8},      /* dd26 [undoc] */
      { undefined, A_0B},     /* dd27 */

      { undefined, A_0B},     /* dd28 */
      { "add  ix,ix",     A_0},      /* dd29 */
      { "ld   ix,(%02X%02Xh)",A_16},     /* dd2a */
      { "dec  ix",        A_0},      /* dd2b */
      { "inc  ixl",       A_0},      /* dd2c [undoc] */
      { "dec  ixl",       A_0},      /* dd2d [undoc] */
      { "ld   ixl,%02Xh", A_8},      /* dd2e [undoc] */
      { undefined, A_0B},     /* dd2f */

      { undefined, A_0B},     /* dd30 */
      { undefined, A_0B},     /* dd31 */
      { undefined, A_0B},     /* dd32 */
      { undefined, A_0B},     /* dd33 */
      { "inc  (ix+%02Xh)",    A_8},      /* dd34 */
      { "dec  (ix+%02Xh)",    A_8},      /* dd35 */
      { "ld   (ix+%02Xh),%02Xh",A_8X2},  /* dd36 */
      { undefined, A_0B},     /* dd37 */

      { undefined, A_0B},     /* dd38 */
      { "add  ix,sp",     A_0},      /* dd39 */
      { undefined, A_0B},     /* dd3a */
      { undefined, A_0B},     /* dd3b */
      { undefined, A_0B},     /* dd3c */
      { undefined, A_0B},     /* dd3d */
      { undefined, A_0B},     /* dd3e */
      { undefined, A_0B},     /* dd3f */

      { undefined, A_0B},     /* dd40 */
      { undefined, A_0B},     /* dd41 */
      { undefined, A_0B},     /* dd42 */
      { undefined, A_0B},     /* dd43 */
      { "ld   b,ixh",     A_0},      /* dd44 [undoc] */
      { "ld   b,ixl",     A_0},      /* dd45 [undoc] */
      { "ld   b,(ix+%02Xh)",  A_8},      /* dd46 */
      { undefined, A_0B},     /* dd47 */

      { undefined, A_0B},     /* dd48 */
      { undefined, A_0B},     /* dd49 */
      { undefined, A_0B},     /* dd4a */
      { undefined, A_0B},     /* dd4b */
      { "ld   c,ixh",     A_0},      /* dd4c [undoc] */
      { "ld   c,ixl",     A_0},      /* dd4d [undoc] */
      { "ld   c,(ix+%02Xh)",  A_8},      /* dd4e */
      { undefined, A_0B},     /* dd4f */

      { undefined, A_0B},     /* dd50 */
      { undefined, A_0B},     /* dd51 */
      { undefined, A_0B},     /* dd52 */
      { undefined, A_0B},     /* dd53 */
      { "ld   d,ixh",     A_0},      /* dd54 [undoc] */
      { "ld   d,ixl",     A_0},      /* dd55 [undoc] */
      { "ld   d,(ix+%02Xh)",  A_8},      /* dd56 */
      { undefined, A_0B},     /* dd57 */

      { undefined, A_0B},     /* dd58 */
      { undefined, A_0B},     /* dd59 */
      { undefined, A_0B},     /* dd5a */
      { undefined, A_0B},     /* dd5b */
      { "ld   e,ixh",     A_0},      /* dd5c [undoc] */
      { "ld   e,ixl",     A_0},      /* dd5d [undoc] */
      { "ld   e,(ix+%02Xh)",  A_8},      /* dd5e */
      { undefined, A_0B},     /* dd5f */

      { "ld   ixh,b",     A_0},      /* dd60 [undoc] */
      { "ld   ixh,c",     A_0},      /* dd61 [undoc] */
      { "ld   ixh,d",     A_0},      /* dd62 [undoc] */
      { "ld   ixh,e",     A_0},      /* dd63 [undoc] */
      { "ld   ixh,ixh",   A_0},      /* dd64 [undoc] */
      { "ld   ixh,ixl",   A_0},      /* dd65 [undoc] */
      { "ld   h,(ix+%02Xh)",  A_8},      /* dd66 */
      { "ld   ixh,a",     A_0},      /* dd67 [undoc] */

      { "ld   ixl,b",     A_0},      /* dd68 [undoc] */
      { "ld   ixl,c",     A_0},      /* dd69 [undoc] */
      { "ld   ixl,d",     A_0},      /* dd6a [undoc] */
      { "ld   ixl,e",     A_0},      /* dd6b [undoc] */
      { "ld   ixl,ixh",   A_0},      /* dd6c [undoc] */
      { "ld   ixl,ixl",   A_0},      /* dd6d [undoc] */
      { "ld   l,(ix+%02Xh)",  A_8},      /* dd6e */
      { "ld   ixl,a",     A_0},      /* dd6f [undoc] */

      { "ld   (ix+%02Xh),b",  A_8},      /* dd70 */
      { "ld   (ix+%02Xh),c",  A_8},      /* dd71 */
      { "ld   (ix+%02Xh),d",  A_8},      /* dd72 */
      { "ld   (ix+%02Xh),e",  A_8},      /* dd73 */
      { "ld   (ix+%02Xh),h",  A_8},      /* dd74 */
      { "ld   (ix+%02Xh),l",  A_8},      /* dd75 */
      { undefined, A_0B},     /* dd76 */
      { "ld   (ix+%02Xh),a",  A_8},      /* dd77 */

      { undefined, A_0B},     /* dd78 */
      { undefined, A_0B},     /* dd79 */
      { undefined, A_0B},     /* dd7a */
      { undefined, A_0B},     /* dd7b */
      { "ld   a,ixh",     A_0},      /* dd7c [undoc] */
      { "ld   a,ixl",     A_0},      /* dd7d [undoc] */
      { "ld   a,(ix+%02Xh)",  A_8},      /* dd7e */
      { undefined, A_0B},     /* dd7f */

      { undefined, A_0B},     /* dd80 */
      { undefined, A_0B},     /* dd81 */
      { undefined, A_0B},     /* dd82 */
      { undefined, A_0B},     /* dd83 */
      { "add  a,ixh",     A_0},      /* dd84 [undoc] */
      { "add  a,ixl",     A_0},      /* dd85 [undoc] */
      { "add  a,(ix+%02Xh)",  A_8},      /* dd86 */
      { undefined, A_0B},     /* dd87 */

      { undefined, A_0B},     /* dd88 */
      { undefined, A_0B},     /* dd89 */
      { undefined, A_0B},     /* dd8a */
      { undefined, A_0B},     /* dd8b */
      { "adc  a,ixh",     A_0},      /* dd8c [undoc] */
      { "adc  a,ixl",     A_0},      /* dd8d [undoc] */
      { "adc  a,(ix+%02Xh)",  A_8},      /* dd8e */
      { undefined, A_0B},     /* dd8f */

      { undefined, A_0B},     /* dd90 */
      { undefined, A_0B},     /* dd91 */
      { undefined, A_0B},     /* dd92 */
      { undefined, A_0B},     /* dd93 */
      { "sub  ixh",       A_0},      /* dd94 [undoc] */
      { "sub  ixl",       A_0},      /* dd95 [undoc] */
      { "sub  (ix+%02Xh)",    A_8},      /* dd96 */
      { undefined, A_0B},     /* dd97 */

      { undefined, A_0B},     /* dd98 */
      { undefined, A_0B},     /* dd99 */
      { undefined, A_0B},     /* dd9a */
      { undefined, A_0B},     /* dd9b */
      { "sbc  ixh",       A_0},      /* dd9c [undoc] */
      { "sbc  ixl",       A_0},      /* dd9d [undoc] */
      { "sbc  a,(ix+%02Xh)",  A_8},      /* dd9e */
      { undefined, A_0B},     /* dd9f */

      { undefined, A_0B},     /* dda0 */
      { undefined, A_0B},     /* dda1 */
      { undefined, A_0B},     /* dda2 */
      { undefined, A_0B},     /* dda3 */
      { "and  ixh",       A_0},      /* dda4 [undoc] */
      { "and  ixl",       A_0},      /* dda5 [undoc] */
      { "and  (ix+%02Xh)",    A_8},      /* dda6 */
      { undefined, A_0B},     /* dda7 */

      { undefined, A_0B},     /* dda8 */
      { undefined, A_0B},     /* dda9 */
      { undefined, A_0B},     /* ddaa */
      { undefined, A_0B},     /* ddab */
      { "xor  ixh",       A_0},      /* ddac [undoc] */
      { "xor  ixl",       A_0},      /* ddad [undoc] */
      { "xor  (ix+%02Xh)",    A_8},      /* ddae */
      { undefined, A_0B},     /* ddaf */

      { undefined, A_0B},     /* ddb0 */
      { undefined, A_0B},     /* ddb1 */
      { undefined, A_0B},     /* ddb2 */
      { undefined, A_0B},     /* ddb3 */
      { "or   ixh",       A_0},      /* ddb4 [undoc] */
      { "or   ixl",       A_0},      /* ddb5 [undoc] */
      { "or   (ix+%02Xh)",    A_8},      /* ddb6 */
      { undefined, A_0B},     /* ddb7 */

      { undefined, A_0B},     /* ddb8 */
      { undefined, A_0B},     /* ddb9 */
      { undefined, A_0B},     /* ddba */
      { undefined, A_0B},     /* ddbb */
      { "cp   ixh",       A_0},      /* ddbc [undoc] */
      { "cp   ixl",       A_0},      /* ddbd [undoc] */
      { "cp   (ix+%02Xh)",    A_8},      /* ddbe */
      { undefined, A_0B},     /* ddbf */

      { undefined, A_0B},     /* ddc0 */
      { undefined, A_0B},     /* ddc1 */
      { undefined, A_0B},     /* ddc2 */
      { undefined, A_0B},     /* ddc3 */
      { undefined, A_0B},     /* ddc4 */
      { undefined, A_0B},     /* ddc5 */
      { undefined, A_0B},     /* ddc6 */
      { undefined, A_0B},     /* ddc7 */

      { undefined, A_0B},     /* ddc8 */
      { undefined, A_0B},     /* ddc9 */
      { undefined, A_0B},     /* ddca */
      { NULL,            4},        /* ddcb */
      { undefined, A_0B},     /* ddcc */
      { undefined, A_0B},     /* ddcd */
      { undefined, A_0B},     /* ddce */
      { undefined, A_0B},     /* ddcf */

      { undefined, A_0B},     /* ddd0 */
      { undefined, A_0B},     /* ddd1 */
      { undefined, A_0B},     /* ddd2 */
      { undefined, A_0B},     /* ddd3 */
      { undefined, A_0B},     /* ddd4 */
      { undefined, A_0B},     /* ddd5 */
      { undefined, A_0B},     /* ddd6 */
      { undefined, A_0B},     /* ddd7 */

      { undefined, A_0B},     /* ddd8 */
      { undefined, A_0B},     /* ddd9 */
      { undefined, A_0B},     /* ddda */
      { undefined, A_0B},     /* dddb */
      { undefined, A_0B},     /* dddc */
      { undefined, A_0B},     /* dddd */
      { undefined, A_0B},     /* ddde */
      { undefined, A_0B},     /* dddf */

      { undefined, A_0B},     /* dde0 */
      { "pop  ix",        A_0},      /* dde1 */
      { undefined, A_0B},     /* dde2 */
      { "ex   (sp),ix",   A_0},      /* dde3 */
      { undefined, A_0B},     /* dde4 */
      { "push ix",        A_0},      /* dde5 */
      { undefined, A_0B},     /* dde6 */
      { undefined, A_0B},     /* dde7 */

      { undefined, A_0B},     /* dde8 */
      { "jp   (ix)",      A_0},      /* dde9 */
      { undefined, A_0B},     /* ddea */
      { undefined, A_0B},     /* ddeb */
      { undefined, A_0B},     /* ddec */
      { undefined, A_0B},     /* dded */
      { undefined, A_0B},     /* ddee */
      { undefined, A_0B},     /* ddef */

      { undefined, A_0B},     /* ddf0 */
      { undefined, A_0B},     /* ddf1 */
      { undefined, A_0B},     /* ddf2 */
      { undefined, A_0B},     /* ddf3 */
      { undefined, A_0B},     /* ddf4 */
      { undefined, A_0B},     /* ddf5 */
      { undefined, A_0B},     /* ddf6 */
      { undefined, A_0B},     /* ddf7 */

      { undefined, A_0B},     /* ddf8 */
      { "ld   sp,ix",     A_0},      /* ddf9 */
      { undefined, A_0B},     /* ddfa */
      { undefined, A_0B},     /* ddfb */
      { undefined, A_0B},     /* ddfc */
      { undefined, A_0B},     /* ddfd */
      { undefined, A_0B},     /* ddfe */
      { undefined, A_0B}     /* ddff */
    },
    {
      { undefined, A_0},      /* ed00 */
      { undefined, A_0},      /* ed01 */
      { undefined, A_0},      /* ed02 */
      { undefined, A_0},      /* ed03 */
      { undefined, A_0},      /* ed04 */
      { undefined, A_0},      /* ed05 */
      { undefined, A_0},      /* ed06 */
      { undefined, A_0},      /* ed07 */

      { undefined, A_0},      /* ed08 */
      { undefined, A_0},      /* ed09 */
      { undefined, A_0},      /* ed0a */
      { undefined, A_0},      /* ed0b */
      { undefined, A_0},      /* ed0c */
      { undefined, A_0},      /* ed0d */
      { undefined, A_0},      /* ed0e */
      { undefined, A_0},      /* ed0f */

      { undefined, A_0},      /* ed10 */
      { undefined, A_0},      /* ed11 */
      { undefined, A_0},      /* ed12 */
      { undefined, A_0},      /* ed13 */
      { undefined, A_0},      /* ed14 */
      { undefined, A_0},      /* ed15 */
      { undefined, A_0},      /* ed16 */
      { undefined, A_0},      /* ed17 */

      { undefined, A_0},      /* ed18 */
      { undefined, A_0},      /* ed19 */
      { undefined, A_0},      /* ed1a */
      { undefined, A_0},      /* ed1b */
      { undefined, A_0},      /* ed1c */
      { undefined, A_0},      /* ed1d */
      { undefined, A_0},      /* ed1e */
      { undefined, A_0},      /* ed1f */

      { undefined, A_0},      /* ed20 [vavasour emt] */
      { undefined, A_0},      /* ed21 [vavasour emt] */
      { undefined, A_0},      /* ed22 */
      { undefined, A_0},      /* ed23 */
      { undefined, A_0},      /* ed24 */
      { undefined, A_0},      /* ed25 */
      { undefined, A_0},      /* ed26 */
      { undefined, A_0},      /* ed27 */

      /* xtrs emulator traps; not real Z80 instructions */
      { "emt_system",     A_0},      /* ed28 */
      { "emt_mouse",      A_0},      /* ed29 */
      { "emt_getdir",     A_0},      /* ed2a */
      { "emt_setdir",     A_0},      /* ed2b */
      { undefined, A_0},      /* ed2c */
      { undefined, A_0},      /* ed2d */
      { undefined, A_0},      /* ed2e */
      { "emt_debug",      A_0},      /* ed2f */

      { "emt_open",       A_0},      /* ed30 */
      { "emt_close",      A_0},      /* ed31 */
      { "emt_read",       A_0},      /* ed32 */
      { "emt_write",      A_0},      /* ed33 */
      { "emt_lseek",      A_0},      /* ed34 */
      { "emt_strerror",   A_0},      /* ed35 */
      { "emt_time",       A_0},      /* ed36 */
      { "emt_opendir",    A_0},      /* ed37 */

      { "emt_closedir",   A_0},      /* ed38 */
      { "emt_readdir",    A_0},      /* ed39 */
      { "emt_chdir",      A_0},      /* ed3a */
      { "emt_getcwd",     A_0},      /* ed3b */
      { "emt_misc",       A_0},      /* ed3c */
      { "emt_ftruncate",  A_0},      /* ed3d */
      { "emt_opendisk",   A_0},      /* ed3e */
      { "emt_closedisk",  A_0},      /* ed3f */
      /* end xtrs emulator traps */

      { "in   b,(c)",     A_0},      /* ed40 */
      { "out  (c),b",     A_0},      /* ed41 */
      { "sbc  hl,bc",     A_0},      /* ed42 */
      { "ld   (%02X%02Xh),bc",A_16},     /* ed43 */
      { "neg",        A_0},      /* ed44 */
      { "retn",       A_0},      /* ed45 */
      { "im   0",     A_0},      /* ed46 */
      { "ld   i,a",       A_0},      /* ed47 */

      { "in   c,(c)",     A_0},      /* ed48 */
      { "out  (c),c",     A_0},      /* ed49 */
      { "adc  hl,bc",     A_0},      /* ed4a */
      { "ld   bc,(%02X%02Xh)",A_16},     /* ed4b */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed4c [undoc] */
      { "reti",       A_0},      /* ed4d */
      { undefined, A_0},      /* ed4e */
      { "ld   r,a",       A_0},      /* ed4f */

      { "in   d,(c)",     A_0},      /* ed50 */
      { "out  (c),d",     A_0},      /* ed51 */
      { "sbc  hl,de",     A_0},      /* ed52 */
      { "ld   (%02X%02Xh),de",A_16},     /* ed53 */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed54 [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed55 [undoc] */
      { "im   1",     A_0},      /* ed56 */
      { "ld   a,i",       A_0},      /* ed57 */

      { "in   e,(c)",     A_0},      /* ed58 */
      { "out  (c),e",     A_0},      /* ed59 */
      { "adc  hl,de",     A_0},      /* ed5a */
      { "ld   de,(%02X%02Xh)",A_16},     /* ed5b */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed5c [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed5d [undoc] */
      { "im   2",     A_0},      /* ed5e */
      { "ld   a,r",       A_0},      /* ed5f */

      { "in   h,(c)",     A_0},      /* ed60 */
      { "out  (c),h",     A_0},      /* ed61 */
      { "sbc  hl,hl",     A_0},      /* ed62 */
      { "ld   (%02X%02Xh),hl\t;undoc equiv",A_16},
      /* ed63 [semi-documented] */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed64 [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed65 [undoc] */
      { "im\t0\t\t;undoc equiv",A_0},    /* ed66 [undoc] */
      { "rrd",        A_0},      /* ed67 */

      { "in   l,(c)",     A_0},      /* ed68 */
      { "out  (c),l",     A_0},      /* ed69 */
      { "adc  hl,hl",     A_0},      /* ed6a */
      { "ld   hl,(%02X%02Xh)\t;undoc equiv",A_16},
      /* ed6b [semi-documented] */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed6c [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed6d [undoc] */
      { undefined, A_0},      /* ed6e */
      { "rld",        A_0},      /* ed6f */

      { "in   (c)",       A_0},      /* ed70 [undoc] */
      { "out  (c),0",     A_0},      /* ed71 [undoc] */
      { "sbc  hl,sp",     A_0},      /* ed72 */
      { "ld   (%02X%02Xh),sp",A_16},     /* ed73 */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed74 [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed75 [undoc] */
      { "im\t1\t\t;undoc equiv",A_0},    /* ed76 [undoc] */
      { undefined, A_0},      /* ed77 */

      { "in   a,(c)",     A_0},      /* ed78 */
      { "out  (c),a",     A_0},      /* ed79 */
      { "adc  hl,sp",     A_0},      /* ed7a */
      { "ld   sp,(%02X%02Xh)",A_16},     /* ed7b */
      { "neg\t\t\t;undoc equiv",A_0},    /* ed7c [undoc] */
      { "ret\t\t\t;undoc equiv",A_0},    /* ed7d [undoc] */
      { "im\t2\t\t;undoc equiv",A_0},    /* ed7e [undoc] */
      { undefined, A_0},      /* ed7f */

      { undefined, A_0},      /* ed80 */
      { undefined, A_0},      /* ed81 */
      { undefined, A_0},      /* ed82 */
      { undefined, A_0},      /* ed83 */
      { undefined, A_0},      /* ed84 */
      { undefined, A_0},      /* ed85 */
      { undefined, A_0},      /* ed86 */
      { undefined, A_0},      /* ed87 */

      { undefined, A_0},      /* ed88 */
      { undefined, A_0},      /* ed89 */
      { undefined, A_0},      /* ed8a */
      { undefined, A_0},      /* ed8b */
      { undefined, A_0},      /* ed8c */
      { undefined, A_0},      /* ed8d */
      { undefined, A_0},      /* ed8e */
      { undefined, A_0},      /* ed8f */

      { undefined, A_0},      /* ed90 */
      { undefined, A_0},      /* ed91 */
      { undefined, A_0},      /* ed92 */
      { undefined, A_0},      /* ed93 */
      { undefined, A_0},      /* ed94 */
      { undefined, A_0},      /* ed95 */
      { undefined, A_0},      /* ed96 */
      { undefined, A_0},      /* ed97 */

      { undefined, A_0},      /* ed98 */
      { undefined, A_0},      /* ed99 */
      { undefined, A_0},      /* ed9a */
      { undefined, A_0},      /* ed9b */
      { undefined, A_0},      /* ed9c */
      { undefined, A_0},      /* ed9d */
      { undefined, A_0},      /* ed9e */
      { undefined, A_0},      /* ed9f */

      { "ldi",        A_0},      /* eda0 */
      { "cpi",        A_0},      /* eda1 */
      { "ini",        A_0},      /* eda2 */
      { "outi",       A_0},      /* eda3 */
      { undefined, A_0},      /* eda4 */
      { undefined, A_0},      /* eda5 */
      { undefined, A_0},      /* eda6 */
      { undefined, A_0},      /* eda7 */

      { "ldd",        A_0},      /* eda8 */
      { "cpd",        A_0},      /* eda9 */
      { "ind",        A_0},      /* edaa */
      { "outd",       A_0},      /* edab */
      { undefined, A_0},      /* edac */
      { undefined, A_0},      /* edad */
      { undefined, A_0},      /* edae */
      { undefined, A_0},      /* edaf */

      { "ldir",       A_0},      /* edb0 */
      { "cpir",       A_0},      /* edb1 */
      { "inir",       A_0},      /* edb2 */
      { "otir",       A_0},      /* edb3 */
      { undefined, A_0},      /* edb4 */
      { undefined, A_0},      /* edb5 */
      { undefined, A_0},      /* edb6 */
      { undefined, A_0},      /* edb7 */

      { "lddr",       A_0},      /* edb8 */
      { "cpdr",       A_0},      /* edb9 */
      { "indr",       A_0},      /* edba */
      { "otdr",       A_0},      /* edbb */
      { undefined, A_0},      /* edbc */
      { undefined, A_0},      /* edbd */
      { undefined, A_0},      /* edbe */
      { undefined, A_0},      /* edbf */

      { undefined, A_0},      /* edc0 */
      { undefined, A_0},      /* edc1 */
      { undefined, A_0},      /* edc2 */
      { undefined, A_0},      /* edc3 */
      { undefined, A_0},      /* edc4 */
      { undefined, A_0},      /* edc5 */
      { undefined, A_0},      /* edc6 */
      { undefined, A_0},      /* edc7 */

      { undefined, A_0},      /* edc8 */
      { undefined, A_0},      /* edc9 */
      { undefined, A_0},      /* edca */
      { undefined, A_0},      /* edcb */
      { undefined, A_0},      /* edcc */
      { undefined, A_0},      /* edcd */
      { undefined, A_0},      /* edce */
      { undefined, A_0},      /* edcf */

      { undefined, A_0},      /* edd0 */
      { undefined, A_0},      /* edd1 */
      { undefined, A_0},      /* edd2 */
      { undefined, A_0},      /* edd3 */
      { undefined, A_0},      /* edd4 */
      { undefined, A_0},      /* edd5 */
      { undefined, A_0},      /* edd6 */
      { undefined, A_0},      /* edd7 */

      { undefined, A_0},      /* edd8 */
      { undefined, A_0},      /* edd9 */
      { undefined, A_0},      /* edda */
      { undefined, A_0},      /* eddb */
      { undefined, A_0},      /* eddc */
      { undefined, A_0},      /* eddd */
      { undefined, A_0},      /* edde */
      { undefined, A_0},      /* eddf */

      { undefined, A_0},      /* ede0 */
      { undefined, A_0},      /* ede1 */
      { undefined, A_0},      /* ede2 */
      { undefined, A_0},      /* ede3 */
      { undefined, A_0},      /* ede4 */
      { undefined, A_0},      /* ede5 */
      { undefined, A_0},      /* ede6 */
      { undefined, A_0},      /* ede7 */

      { undefined, A_0},      /* ede8 */
      { undefined, A_0},      /* ede9 */
      { undefined, A_0},      /* edea */
      { undefined, A_0},      /* edeb */
      { undefined, A_0},      /* edec */
      { undefined, A_0},      /* eded */
      { undefined, A_0},      /* edee */
      { undefined, A_0},      /* edef */

      { undefined, A_0},      /* edf0 */
      { undefined, A_0},      /* edf1 */
      { undefined, A_0},      /* edf2 */
      { undefined, A_0},      /* edf3 */
      { undefined, A_0},      /* edf4 */
      { undefined, A_0},      /* edf5 */
      { undefined, A_0},      /* edf6 */
      { undefined, A_0},      /* edf7 */

      { undefined, A_0},      /* edf8 */
      { undefined, A_0},      /* edf9 */
      { undefined, A_0},      /* edfa */
      { undefined, A_0},      /* edfb */
      { undefined, A_0},      /* edfc */
      { undefined, A_0},      /* edfd */
      { undefined, A_0},      /* edfe */
      { undefined, A_0}      /* edff */
    },
    {
      { undefined, A_0B},     /* fd00 */
      { undefined, A_0B},     /* fd01 */
      { undefined, A_0B},     /* fd02 */
      { undefined, A_0B},     /* fd03 */
      { undefined, A_0B},     /* fd04 */
      { undefined, A_0B},     /* fd05 */
      { undefined, A_0B},     /* fd06 */
      { undefined, A_0B},     /* fd07 */

      { undefined, A_0B},     /* fd08 */
      { "add  iy,bc",     A_0},      /* fd09 */
      { undefined, A_0B},     /* fd0a */
      { undefined, A_0B},     /* fd0b */
      { undefined, A_0B},     /* fd0c */
      { undefined, A_0B},     /* fd0d */
      { undefined, A_0B},     /* fd0e */
      { undefined, A_0B},     /* fd0f */

      { undefined, A_0B},     /* fd10 */
      { undefined, A_0B},     /* fd11 */
      { undefined, A_0B},     /* fd12 */
      { undefined, A_0B},     /* fd13 */
      { undefined, A_0B},     /* fd14 */
      { undefined, A_0B},     /* fd15 */
      { undefined, A_0B},     /* fd16 */
      { undefined, A_0B},     /* fd17 */

      { undefined, A_0B},     /* fd18 */
      { "add  iy,de",     A_0},      /* fd19 */
      { undefined, A_0B},     /* fd1a */
      { undefined, A_0B},     /* fd1b */
      { undefined, A_0B},     /* fd1c */
      { undefined, A_0B},     /* fd1d */
      { undefined, A_0B},     /* fd1e */
      { undefined, A_0B},     /* fd1f */

      { undefined, A_0B},     /* fd20 */
      { "ld   iy,%02X%02Xh",  A_16},     /* fd21 */
      { "ld   (%02X%02Xh),iy",A_16},     /* fd22 */
      { "inc  iy",        A_0},      /* fd23 */
      { undefined, A_0B},     /* fd24 */
      { undefined, A_0B},     /* fd25 */
      { undefined, A_0B},     /* fd26 */
      { undefined, A_0B},     /* fd27 */

      { undefined, A_0B},     /* fd28 */
      { "add  iy,iy",     A_0},      /* fd29 */
      { "ld   iy,(%02X%02Xh)",A_16},     /* fd2a */
      { "dec  iy",        A_0},      /* fd2b */
      { undefined, A_0B},     /* fd2c */
      { undefined, A_0B},     /* fd2d */
      { undefined, A_0B},     /* fd2e */
      { undefined, A_0B},     /* fd2f */

      { undefined, A_0B},     /* fd30 */
      { undefined, A_0B},     /* fd31 */
      { undefined, A_0B},     /* fd32 */
      { undefined, A_0B},     /* fd33 */
      { "inc  (iy+%02Xh)",    A_8},      /* fd34 */
      { "dec  (iy+%02Xh)",    A_8},      /* fd35 */
      { "ld   (iy+%02Xh),%02Xh",A_8X2},  /* fd36 */
      { undefined, A_0B},     /* fd37 */

      { undefined, A_0B},     /* fd38 */
      { "add  iy,sp",     A_0},      /* fd39 */
      { undefined, A_0B},     /* fd3a */
      { undefined, A_0B},     /* fd3b */
      { undefined, A_0B},     /* fd3c */
      { undefined, A_0B},     /* fd3d */
      { undefined, A_0B},     /* fd3e */
      { undefined, A_0B},     /* fd3f */

      { undefined, A_0B},     /* fd40 */
      { undefined, A_0B},     /* fd41 */
      { undefined, A_0B},     /* fd42 */
      { undefined, A_0B},     /* fd43 */
      { undefined, A_0B},     /* fd44 */
      { undefined, A_0B},     /* fd45 */
      { "ld   b,(iy+%02Xh)",  A_8},      /* fd46 */
      { undefined, A_0B},     /* fd47 */

      { undefined, A_0B},     /* fd48 */
      { undefined, A_0B},     /* fd49 */
      { undefined, A_0B},     /* fd4a */
      { undefined, A_0B},     /* fd4b */
      { undefined, A_0B},     /* fd4c */
      { undefined, A_0B},     /* fd4d */
      { "ld   c,(iy+%02Xh)",  A_8},      /* fd4e */
      { undefined, A_0B},     /* fd4f */

      { undefined, A_0B},     /* fd50 */
      { undefined, A_0B},     /* fd51 */
      { undefined, A_0B},     /* fd52 */
      { undefined, A_0B},     /* fd53 */
      { undefined, A_0B},     /* fd54 */
      { undefined, A_0B},     /* fd55 */
      { "ld   d,(iy+%02Xh)",  A_8},      /* fd56 */
      { undefined, A_0B},     /* fd57 */

      { undefined, A_0B},     /* fd58 */
      { undefined, A_0B},     /* fd59 */
      { undefined, A_0B},     /* fd5a */
      { undefined, A_0B},     /* fd5b */
      { undefined, A_0B},     /* fd5c */
      { undefined, A_0B},     /* fd5d */
      { "ld   e,(iy+%02Xh)",  A_8},      /* fd5e */
      { undefined, A_0B},     /* fd5f */

      { undefined, A_0B},     /* fd60 */
      { undefined, A_0B},     /* fd61 */
      { undefined, A_0B},     /* fd62 */
      { undefined, A_0B},     /* fd63 */
      { undefined, A_0B},     /* fd64 */
      { undefined, A_0B},     /* fd65 */
      { "ld   h,(iy+%02Xh)",  A_8},      /* fd66 */
      { undefined, A_0B},     /* fd67 */

      { undefined, A_0B},     /* fd68 */
      { undefined, A_0B},     /* fd69 */
      { undefined, A_0B},     /* fd6a */
      { undefined, A_0B},     /* fd6b */
      { undefined, A_0B},     /* fd6c */
      { undefined, A_0B},     /* fd6d */
      { "ld   l,(iy+%02Xh)",  A_8},      /* fd6e */
      { undefined, A_0B},     /* fd6f */

      { "ld   (iy+%02Xh),b",  A_8},      /* fd70 */
      { "ld   (iy+%02Xh),c",  A_8},      /* fd71 */
      { "ld   (iy+%02Xh),d",  A_8},      /* fd72 */
      { "ld   (iy+%02Xh),e",  A_8},      /* fd73 */
      { "ld   (iy+%02Xh),h",  A_8},      /* fd74 */
      { "ld   (iy+%02Xh),l",  A_8},      /* fd75 */
      { undefined, A_0B},     /* fd76 */
      { "ld   (iy+%02Xh),a",  A_8},      /* fd77 */

      { undefined, A_0B},     /* fd78 */
      { undefined, A_0B},     /* fd79 */
      { undefined, A_0B},     /* fd7a */
      { undefined, A_0B},     /* fd7b */
      { undefined, A_0B},     /* fd7c */
      { undefined, A_0B},     /* fd7d */
      { "ld   a,(iy+%02Xh)",  A_8},      /* fd7e */
      { undefined, A_0B},     /* fd7f */

      { undefined, A_0B},     /* fd80 */
      { undefined, A_0B},     /* fd81 */
      { undefined, A_0B},     /* fd82 */
      { undefined, A_0B},     /* fd83 */
      { undefined, A_0B},     /* fd84 */
      { undefined, A_0B},     /* fd85 */
      { "add  a,(iy+%02Xh)",  A_8},      /* fd86 */
      { undefined, A_0B},     /* fd87 */

      { undefined, A_0B},     /* fd88 */
      { undefined, A_0B},     /* fd89 */
      { undefined, A_0B},     /* fd8a */
      { undefined, A_0B},     /* fd8b */
      { undefined, A_0B},     /* fd8c */
      { undefined, A_0B},     /* fd8d */
      { "adc  a,(iy+%02Xh)",  A_8},      /* fd8e */
      { undefined, A_0B},     /* fd8f */

      { undefined, A_0B},     /* fd90 */
      { undefined, A_0B},     /* fd91 */
      { undefined, A_0B},     /* fd92 */
      { undefined, A_0B},     /* fd93 */
      { undefined, A_0B},     /* fd94 */
      { undefined, A_0B},     /* fd95 */
      { "sub  (iy+%02Xh)",    A_8},      /* fd96 */
      { undefined, A_0B},     /* fd97 */

      { undefined, A_0B},     /* fd98 */
      { undefined, A_0B},     /* fd99 */
      { undefined, A_0B},     /* fd9a */
      { undefined, A_0B},     /* fd9b */
      { undefined, A_0B},     /* fd9c */
      { undefined, A_0B},     /* fd9d */
      { "sbc  a,(iy+%02Xh)",  A_8},      /* fd9e */
      { undefined, A_0B},     /* fd9f */

      { undefined, A_0B},     /* fda0 */
      { undefined, A_0B},     /* fda1 */
      { undefined, A_0B},     /* fda2 */
      { undefined, A_0B},     /* fda3 */
      { undefined, A_0B},     /* fda4 */
      { undefined, A_0B},     /* fda5 */
      { "and  (iy+%02Xh)",    A_8},      /* fda6 */
      { undefined, A_0B},     /* fda7 */

      { undefined, A_0B},     /* fda8 */
      { undefined, A_0B},     /* fda9 */
      { undefined, A_0B},     /* fdaa */
      { undefined, A_0B},     /* fdab */
      { undefined, A_0B},     /* fdac */
      { undefined, A_0B},     /* fdad */
      { "xor  (iy+%02Xh)",    A_8},      /* fdae */
      { undefined, A_0B},     /* fdaf */

      { undefined, A_0B},     /* fdb0 */
      { undefined, A_0B},     /* fdb1 */
      { undefined, A_0B},     /* fdb2 */
      { undefined, A_0B},     /* fdb3 */
      { undefined, A_0B},     /* fdb4 */
      { undefined, A_0B},     /* fdb5 */
      { "or   (iy+%02Xh)",    A_8},      /* fdb6 */
      { undefined, A_0B},     /* fdb7 */

      { undefined, A_0B},     /* fdb8 */
      { undefined, A_0B},     /* fdb9 */
      { undefined, A_0B},     /* fdba */
      { undefined, A_0B},     /* fdbb */
      { undefined, A_0B},     /* fdbc */
      { undefined, A_0B},     /* fdbd */
      { "cp   (iy+%02Xh)",    A_8},      /* fdbe */
      { undefined, A_0B},     /* fdbf */

      { undefined, A_0B},     /* fdc0 */
      { undefined, A_0B},     /* fdc1 */
      { undefined, A_0B},     /* fdc2 */
      { undefined, A_0B},     /* fdc3 */
      { undefined, A_0B},     /* fdc4 */
      { undefined, A_0B},     /* fdc5 */
      { undefined, A_0B},     /* fdc6 */
      { undefined, A_0B},     /* fdc7 */

      { undefined, A_0B},     /* fdc8 */
      { undefined, A_0B},     /* fdc9 */
      { undefined, A_0B},     /* fdca */
      { NULL,            5},        /* fdcb */
      { undefined, A_0B},     /* fdcc */
      { undefined, A_0B},     /* fdcd */
      { undefined, A_0B},     /* fdce */
      { undefined, A_0B},     /* fdcf */

      { undefined, A_0B},     /* fdd0 */
      { undefined, A_0B},     /* fdd1 */
      { undefined, A_0B},     /* fdd2 */
      { undefined, A_0B},     /* fdd3 */
      { undefined, A_0B},     /* fdd4 */
      { undefined, A_0B},     /* fdd5 */
      { undefined, A_0B},     /* fdd6 */
      { undefined, A_0B},     /* fdd7 */

      { undefined, A_0B},     /* fdd8 */
      { undefined, A_0B},     /* fdd9 */
      { undefined, A_0B},     /* fdda */
      { undefined, A_0B},     /* fddb */
      { undefined, A_0B},     /* fddc */
      { undefined, A_0B},     /* fddd */
      { undefined, A_0B},     /* fdde */
      { undefined, A_0B},     /* fddf */

      { undefined, A_0B},     /* fde0 */
      { "pop  iy",        A_0},      /* fde1 */
      { undefined, A_0B},     /* fde2 */
      { "ex   (sp),iy",   A_0},      /* fde3 */
      { undefined, A_0B},     /* fde4 */
      { "push iy",        A_0},      /* fde5 */
      { undefined, A_0B},     /* fde6 */
      { undefined, A_0B},     /* fde7 */

      { undefined, A_0B},     /* fde8 */
      { "jp   (iy)",      A_0},      /* fde9 */
      { undefined, A_0B},     /* fdea */
      { undefined, A_0B},     /* fdeb */
      { undefined, A_0B},     /* fdec */
      { undefined, A_0B},     /* fded */
      { undefined, A_0B},     /* fdee */
      { undefined, A_0B},     /* fdef */

      { undefined, A_0B},     /* fdf0 */
      { undefined, A_0B},     /* fdf1 */
      { undefined, A_0B},     /* fdf2 */
      { undefined, A_0B},     /* fdf3 */
      { undefined, A_0B},     /* fdf4 */
      { undefined, A_0B},     /* fdf5 */
      { undefined, A_0B},     /* fdf6 */
      { undefined, A_0B},     /* fdf7 */

      { undefined, A_0B},     /* fdf8 */
      { "ld   sp,iy",     A_0},      /* fdf9 */
      { undefined, A_0B},     /* fdfa */
      { undefined, A_0B},     /* fdfb */
      { undefined, A_0B},     /* fdfc */
      { undefined, A_0B},     /* fdfd */
      { undefined, A_0B},     /* fdfe */
      { undefined, A_0B}     /* fdff */
    },
    {
      { "ld   b,rlc (ix+%02Xh)",  A_8P}, /* ddcb..00 [undoc] */
      { "ld   c,rlc (ix+%02Xh)",  A_8P}, /* ddcb..01 [undoc] */
      { "ld   d,rlc (ix+%02Xh)",  A_8P}, /* ddcb..02 [undoc] */
      { "ld   e,rlc (ix+%02Xh)",  A_8P}, /* ddcb..03 [undoc] */
      { "ld   h,rlc (ix+%02Xh)",  A_8P}, /* ddcb..04 [undoc] */
      { "ld   l,rlc (ix+%02Xh)",  A_8P}, /* ddcb..05 [undoc] */
      { "rlc  (ix+%02Xh)",        A_8P}, /* ddcb..06 */
      { "ld   a,rlc (ix+%02Xh)",  A_8P}, /* ddcb..07 [undoc] */

      { "ld   b,rrc (ix+%02Xh)",  A_8P}, /* ddcb..08 [undoc] */
      { "ld   c,rrc (ix+%02Xh)",  A_8P}, /* ddcb..09 [undoc] */
      { "ld   d,rrc (ix+%02Xh)",  A_8P}, /* ddcb..0a [undoc] */
      { "ld   e,rrc (ix+%02Xh)",  A_8P}, /* ddcb..0b [undoc] */
      { "ld   h,rrc (ix+%02Xh)",  A_8P}, /* ddcb..0c [undoc] */
      { "ld   l,rrc (ix+%02Xh)",  A_8P}, /* ddcb..0d [undoc] */
      { "rrc  (ix+%02Xh)",        A_8P}, /* ddcb..0e */
      { "ld   a,rrc (ix+%02Xh)",  A_8P}, /* ddcb..0f [undoc] */

      { "ld   b,rl (ix+%02Xh)",   A_8P}, /* ddcb..10 [undoc] */
      { "ld   c,rl (ix+%02Xh)",   A_8P}, /* ddcb..11 [undoc] */
      { "ld   d,rl (ix+%02Xh)",   A_8P}, /* ddcb..12 [undoc] */
      { "ld   e,rl (ix+%02Xh)",   A_8P}, /* ddcb..13 [undoc] */
      { "ld   h,rl (ix+%02Xh)",   A_8P}, /* ddcb..14 [undoc] */
      { "ld   l,rl (ix+%02Xh)",   A_8P}, /* ddcb..15 [undoc] */
      { "rl   (ix+%02Xh)",        A_8P}, /* ddcb..16 */
      { "ld   a,rl (ix+%02Xh)",   A_8P}, /* ddcb..17 [undoc] */

      { "ld   b,rr (ix+%02Xh)",   A_8P}, /* ddcb..18 [undoc] */
      { "ld   c,rr (ix+%02Xh)",   A_8P}, /* ddcb..19 [undoc] */
      { "ld   d,rr (ix+%02Xh)",   A_8P}, /* ddcb..1a [undoc] */
      { "ld   e,rr (ix+%02Xh)",   A_8P}, /* ddcb..1b [undoc] */
      { "ld   h,rr (ix+%02Xh)",   A_8P}, /* ddcb..1c [undoc] */
      { "ld   l,rr (ix+%02Xh)",   A_8P}, /* ddcb..1d [undoc] */
      { "rr   (ix+%02Xh)",        A_8P}, /* ddcb..1e */
      { "ld   a,rr (ix+%02Xh)",   A_8P}, /* ddcb..1f [undoc] */

      { "ld   b,sla (ix+%02Xh)",  A_8P}, /* ddcb..20 [undoc] */
      { "ld   c,sla (ix+%02Xh)",  A_8P}, /* ddcb..21 [undoc] */
      { "ld   d,sla (ix+%02Xh)",  A_8P}, /* ddcb..22 [undoc] */
      { "ld   e,sla (ix+%02Xh)",  A_8P}, /* ddcb..23 [undoc] */
      { "ld   h,sla (ix+%02Xh)",  A_8P}, /* ddcb..24 [undoc] */
      { "ld   l,sla (ix+%02Xh)",  A_8P}, /* ddcb..25 [undoc] */
      { "sla  (ix+%02Xh)",        A_8P}, /* ddcb..26 */
      { "ld   a,sla (ix+%02Xh)",  A_8P}, /* ddcb..27 [undoc] */

      { "ld   b,sra (ix+%02Xh)",  A_8P}, /* ddcb..28 [undoc] */
      { "ld   c,sra (ix+%02Xh)",  A_8P}, /* ddcb..29 [undoc] */
      { "ld   d,sra (ix+%02Xh)",  A_8P}, /* ddcb..2a [undoc] */
      { "ld   e,sra (ix+%02Xh)",  A_8P}, /* ddcb..2b [undoc] */
      { "ld   h,sra (ix+%02Xh)",  A_8P}, /* ddcb..2c [undoc] */
      { "ld   l,sra (ix+%02Xh)",  A_8P}, /* ddcb..2d [undoc] */
      { "sra  (ix+%02Xh)",        A_8P}, /* ddcb..2e */
      { "ld   a,sra (ix+%02Xh)",  A_8P}, /* ddcb..2f [undoc] */

      { "ld   b,slia (ix+%02Xh)", A_8P}, /* ddcb..30 [undoc] */
      { "ld   c,slia (ix+%02Xh)", A_8P}, /* ddcb..31 [undoc] */
      { "ld   d,slia (ix+%02Xh)", A_8P}, /* ddcb..32 [undoc] */
      { "ld   e,slia (ix+%02Xh)", A_8P}, /* ddcb..33 [undoc] */
      { "ld   h,slia (ix+%02Xh)", A_8P}, /* ddcb..34 [undoc] */
      { "ld   l,slia (ix+%02Xh)", A_8P}, /* ddcb..35 [undoc] */
      { "slia (ix+%02Xh)",        A_8P}, /* ddcb..36 [undoc] */
      { "ld   a,slia (ix+%02Xh)", A_8P}, /* ddcb..37 [undoc] */

      { "ld   b,srl (ix+%02Xh)",  A_8P}, /* ddcb..38 [undoc] */
      { "ld   c,srl (ix+%02Xh)",  A_8P}, /* ddcb..39 [undoc] */
      { "ld   d,srl (ix+%02Xh)",  A_8P}, /* ddcb..3a [undoc] */
      { "ld   e,srl (ix+%02Xh)",  A_8P}, /* ddcb..3b [undoc] */
      { "ld   h,srl (ix+%02Xh)",  A_8P}, /* ddcb..3c [undoc] */
      { "ld   l,srl (ix+%02Xh)",  A_8P}, /* ddcb..3d [undoc] */
      { "srl  (ix+%02Xh)",        A_8P}, /* ddcb..3e */
      { "ld   a,srl (ix+%02Xh)",  A_8P}, /* ddcb..3f [undoc] */

      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..40 [undoc] */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..41 [undoc] */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..42 [undoc] */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..43 [undoc] */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..44 [undoc] */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..45 [undoc] */
      { "bit  0,(ix+%02Xh)",  A_8P},          /* ddcb..46 */
      { "bit  0,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..47 [undoc] */

      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..48 [undoc] */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..49 [undoc] */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..4a [undoc] */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..4b [undoc] */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..4c [undoc] */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..4d [undoc] */
      { "bit  1,(ix+%02Xh)",  A_8P},          /* ddcb..4e */
      { "bit  1,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..4f [undoc] */

      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..50 [undoc] */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..51 [undoc] */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..52 [undoc] */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..53 [undoc] */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..54 [undoc] */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..55 [undoc] */
      { "bit  2,(ix+%02Xh)",  A_8P},          /* ddcb..56 */
      { "bit  2,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..57 [undoc] */

      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..58 [undoc] */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..59 [undoc] */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..5a [undoc] */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..5b [undoc] */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..5c [undoc] */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..5d [undoc] */
      { "bit  3,(ix+%02Xh)",  A_8P},          /* ddcb..5e */
      { "bit  3,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..5f [undoc] */

      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..60 [undoc] */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..61 [undoc] */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..62 [undoc] */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..63 [undoc] */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..64 [undoc] */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..65 [undoc] */
      { "bit  4,(ix+%02Xh)",  A_8P},          /* ddcb..66 */
      { "bit  4,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..67 [undoc] */

      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..68 [undoc] */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..69 [undoc] */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..6a [undoc] */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..6b [undoc] */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..6c [undoc] */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..6d [undoc] */
      { "bit  5,(ix+%02Xh)",  A_8P},          /* ddcb..6e */
      { "bit  5,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..6f [undoc] */

      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..70 [undoc] */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..71 [undoc] */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..72 [undoc] */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..73 [undoc] */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..74 [undoc] */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..75 [undoc] */
      { "bit  6,(ix+%02Xh)",  A_8P},          /* ddcb..76 */
      { "bit  6,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..77 [undoc] */

      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..78 [undoc] */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..79 [undoc] */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..7a [undoc] */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..7b [undoc] */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..7c [undoc] */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..7d [undoc] */
      { "bit  7,(ix+%02Xh)",  A_8P},          /* ddcb..7e */
      { "bit  7,(ix+%02Xh)\t;undoc equiv", A_8P}, /* ddcb..7f [undoc] */

      { "ld   b,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..80 [undoc] */
      { "ld   c,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..81 [undoc] */
      { "ld   d,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..82 [undoc] */
      { "ld   e,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..83 [undoc] */
      { "ld   h,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..84 [undoc] */
      { "ld   l,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..85 [undoc] */
      { "res  0,(ix+%02Xh)",      A_8P}, /* ddcb..86 */
      { "ld   a,res 0,(ix+%02Xh)",    A_8P}, /* ddcb..87 [undoc] */

      { "ld   b,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..88 [undoc] */
      { "ld   c,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..89 [undoc] */
      { "ld   d,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..8a [undoc] */
      { "ld   e,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..8b [undoc] */
      { "ld   h,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..8c [undoc] */
      { "ld   l,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..8d [undoc] */
      { "res  1,(ix+%02Xh)",      A_8P}, /* ddcb..8e */
      { "ld   a,res 1,(ix+%02Xh)",    A_8P}, /* ddcb..8f [undoc] */

      { "ld   b,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..90 [undoc] */
      { "ld   c,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..91 [undoc] */
      { "ld   d,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..92 [undoc] */
      { "ld   e,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..93 [undoc] */
      { "ld   h,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..94 [undoc] */
      { "ld   l,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..95 [undoc] */
      { "res  2,(ix+%02Xh)",      A_8P}, /* ddcb..96 */
      { "ld   a,res 2,(ix+%02Xh)",    A_8P}, /* ddcb..97 [undoc] */

      { "ld   b,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..98 [undoc] */
      { "ld   c,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..99 [undoc] */
      { "ld   d,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..9a [undoc] */
      { "ld   e,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..9b [undoc] */
      { "ld   h,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..9c [undoc] */
      { "ld   l,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..9d [undoc] */
      { "res  3,(ix+%02Xh)",      A_8P}, /* ddcb..9e */
      { "ld   a,res 3,(ix+%02Xh)",    A_8P}, /* ddcb..9f [undoc] */

      { "ld   b,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a0 [undoc] */
      { "ld   c,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a1 [undoc] */
      { "ld   d,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a2 [undoc] */
      { "ld   e,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a3 [undoc] */
      { "ld   h,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a4 [undoc] */
      { "ld   l,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a5 [undoc] */
      { "res  4,(ix+%02Xh)",      A_8P}, /* ddcb..a6 */
      { "ld   a,res 4,(ix+%02Xh)",    A_8P}, /* ddcb..a7 [undoc] */

      { "ld   b,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..a8 [undoc] */
      { "ld   c,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..a9 [undoc] */
      { "ld   d,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..aa [undoc] */
      { "ld   e,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..ab [undoc] */
      { "ld   h,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..ac [undoc] */
      { "ld   l,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..ad [undoc] */
      { "res  5,(ix+%02Xh)",      A_8P}, /* ddcb..ae */
      { "ld   a,res 5,(ix+%02Xh)",    A_8P}, /* ddcb..af [undoc] */

      { "ld   b,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b0 [undoc] */
      { "ld   c,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b1 [undoc] */
      { "ld   d,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b2 [undoc] */
      { "ld   e,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b3 [undoc] */
      { "ld   h,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b4 [undoc] */
      { "ld   l,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b5 [undoc] */
      { "res  6,(ix+%02Xh)",      A_8P}, /* ddcb..b6 */
      { "ld   a,res 6,(ix+%02Xh)",    A_8P}, /* ddcb..b7 [undoc] */

      { "ld   b,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..b8 [undoc] */
      { "ld   c,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..b9 [undoc] */
      { "ld   d,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..ba [undoc] */
      { "ld   e,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..bb [undoc] */
      { "ld   h,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..bc [undoc] */
      { "ld   l,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..bd [undoc] */
      { "res  7,(ix+%02Xh)",      A_8P}, /* ddcb..be */
      { "ld   a,res 7,(ix+%02Xh)",    A_8P}, /* ddcb..bf [undoc] */

      { "ld   b,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c0 [undoc] */
      { "ld   c,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c1 [undoc] */
      { "ld   d,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c2 [undoc] */
      { "ld   e,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c3 [undoc] */
      { "ld   h,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c4 [undoc] */
      { "ld   l,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c5 [undoc] */
      { "set  0,(ix+%02Xh)",      A_8P}, /* ddcb..c6 */
      { "ld   a,set 0,(ix+%02Xh)",    A_8P}, /* ddcb..c7 [undoc] */

      { "ld   b,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..c8 [undoc] */
      { "ld   c,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..c9 [undoc] */
      { "ld   d,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..ca [undoc] */
      { "ld   e,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..cb [undoc] */
      { "ld   h,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..cc [undoc] */
      { "ld   l,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..cd [undoc] */
      { "set  1,(ix+%02Xh)",      A_8P}, /* ddcb..ce */
      { "ld   a,set 1,(ix+%02Xh)",    A_8P}, /* ddcb..cf [undoc] */

      { "ld   b,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d0 [undoc] */
      { "ld   c,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d1 [undoc] */
      { "ld   d,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d2 [undoc] */
      { "ld   e,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d3 [undoc] */
      { "ld   h,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d4 [undoc] */
      { "ld   l,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d5 [undoc] */
      { "set  2,(ix+%02Xh)",      A_8P}, /* ddcb..d6 */
      { "ld   a,set 2,(ix+%02Xh)",    A_8P}, /* ddcb..d7 [undoc] */

      { "ld   b,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..d8 [undoc] */
      { "ld   c,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..d9 [undoc] */
      { "ld   d,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..da [undoc] */
      { "ld   e,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..db [undoc] */
      { "ld   h,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..dc [undoc] */
      { "ld   l,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..dd [undoc] */
      { "set  3,(ix+%02Xh)",      A_8P}, /* ddcb..de */
      { "ld   a,set 3,(ix+%02Xh)",    A_8P}, /* ddcb..df [undoc] */

      { "ld   b,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e0 [undoc] */
      { "ld   c,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e1 [undoc] */
      { "ld   d,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e2 [undoc] */
      { "ld   e,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e3 [undoc] */
      { "ld   h,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e4 [undoc] */
      { "ld   l,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e5 [undoc] */
      { "set  4,(ix+%02Xh)",      A_8P}, /* ddcb..e6 */
      { "ld   a,set 4,(ix+%02Xh)",    A_8P}, /* ddcb..e7 [undoc] */

      { "ld   b,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..e8 [undoc] */
      { "ld   c,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..e9 [undoc] */
      { "ld   d,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..ea [undoc] */
      { "ld   e,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..eb [undoc] */
      { "ld   h,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..ec [undoc] */
      { "ld   l,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..ed [undoc] */
      { "set  5,(ix+%02Xh)",      A_8P}, /* ddcb..ee */
      { "ld   a,set 5,(ix+%02Xh)",    A_8P}, /* ddcb..ef [undoc] */

      { "ld   b,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f0 [undoc] */
      { "ld   c,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f1 [undoc] */
      { "ld   d,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f2 [undoc] */
      { "ld   e,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f3 [undoc] */
      { "ld   h,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f4 [undoc] */
      { "ld   l,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f5 [undoc] */
      { "set  6,(ix+%02Xh)",      A_8P}, /* ddcb..f6 */
      { "ld   a,set 6,(ix+%02Xh)",    A_8P}, /* ddcb..f7 [undoc] */

      { "ld   b,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..f8 [undoc] */
      { "ld   c,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..f9 [undoc] */
      { "ld   d,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..fa [undoc] */
      { "ld   e,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..fb [undoc] */
      { "ld   h,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..fc [undoc] */
      { "ld   l,set 7,(ix+%02Xh)",    A_8P}, /* ddcb..fd [undoc] */
      { "set  7,(ix+%02Xh)",      A_8P}, /* ddcb..fe */
      { "ld   a,set 7,(ix+%02Xh)",    A_8P} /* ddcb..ff [undoc] */
    },
    {
      { "ld   b,rlc (iy+%02Xh)",  A_8P}, /* fdcb..00 [undoc] */
      { "ld   c,rlc (iy+%02Xh)",  A_8P}, /* fdcb..01 [undoc] */
      { "ld   d,rlc (iy+%02Xh)",  A_8P}, /* fdcb..02 [undoc] */
      { "ld   e,rlc (iy+%02Xh)",  A_8P}, /* fdcb..03 [undoc] */
      { "ld   h,rlc (iy+%02Xh)",  A_8P}, /* fdcb..04 [undoc] */
      { "ld   l,rlc (iy+%02Xh)",  A_8P}, /* fdcb..05 [undoc] */
      { "rlc  (iy+%02Xh)",        A_8P}, /* fdcb..06 */
      { "ld   a,rlc (iy+%02Xh)",  A_8P}, /* fdcb..07 [undoc] */

      { "ld   b,rrc (iy+%02Xh)",  A_8P}, /* fdcb..08 [undoc] */
      { "ld   c,rrc (iy+%02Xh)",  A_8P}, /* fdcb..09 [undoc] */
      { "ld   d,rrc (iy+%02Xh)",  A_8P}, /* fdcb..0a [undoc] */
      { "ld   e,rrc (iy+%02Xh)",  A_8P}, /* fdcb..0b [undoc] */
      { "ld   h,rrc (iy+%02Xh)",  A_8P}, /* fdcb..0c [undoc] */
      { "ld   l,rrc (iy+%02Xh)",  A_8P}, /* fdcb..0d [undoc] */
      { "rrc  (iy+%02Xh)",        A_8P}, /* fdcb..0e */
      { "ld   a,rrc (iy+%02Xh)",  A_8P}, /* fdcb..0f [undoc] */

      { "ld   b,rl (iy+%02Xh)",   A_8P}, /* fdcb..10 [undoc] */
      { "ld   c,rl (iy+%02Xh)",   A_8P}, /* fdcb..11 [undoc] */
      { "ld   d,rl (iy+%02Xh)",   A_8P}, /* fdcb..12 [undoc] */
      { "ld   e,rl (iy+%02Xh)",   A_8P}, /* fdcb..13 [undoc] */
      { "ld   h,rl (iy+%02Xh)",   A_8P}, /* fdcb..14 [undoc] */
      { "ld   l,rl (iy+%02Xh)",   A_8P}, /* fdcb..15 [undoc] */
      { "rl   (iy+%02Xh)",        A_8P}, /* fdcb..16 */
      { "ld   a,rl (iy+%02Xh)",   A_8P}, /* fdcb..17 [undoc] */

      { "ld   b,rr (iy+%02Xh)",   A_8P}, /* fdcb..18 [undoc] */
      { "ld   c,rr (iy+%02Xh)",   A_8P}, /* fdcb..19 [undoc] */
      { "ld   d,rr (iy+%02Xh)",   A_8P}, /* fdcb..1a [undoc] */
      { "ld   e,rr (iy+%02Xh)",   A_8P}, /* fdcb..1b [undoc] */
      { "ld   h,rr (iy+%02Xh)",   A_8P}, /* fdcb..1c [undoc] */
      { "ld   l,rr (iy+%02Xh)",   A_8P}, /* fdcb..1d [undoc] */
      { "rr   (iy+%02Xh)",        A_8P}, /* fdcb..1e */
      { "ld   a,rr (iy+%02Xh)",   A_8P}, /* fdcb..1f [undoc] */

      { "ld   b,sla (iy+%02Xh)",  A_8P}, /* fdcb..20 [undoc] */
      { "ld   c,sla (iy+%02Xh)",  A_8P}, /* fdcb..21 [undoc] */
      { "ld   d,sla (iy+%02Xh)",  A_8P}, /* fdcb..22 [undoc] */
      { "ld   e,sla (iy+%02Xh)",  A_8P}, /* fdcb..23 [undoc] */
      { "ld   h,sla (iy+%02Xh)",  A_8P}, /* fdcb..24 [undoc] */
      { "ld   l,sla (iy+%02Xh)",  A_8P}, /* fdcb..25 [undoc] */
      { "sla  (iy+%02Xh)",        A_8P}, /* fdcb..26 */
      { "ld   a,sla (iy+%02Xh)",  A_8P}, /* fdcb..27 [undoc] */

      { "ld   b,sra (iy+%02Xh)",  A_8P}, /* fdcb..28 [undoc] */
      { "ld   c,sra (iy+%02Xh)",  A_8P}, /* fdcb..29 [undoc] */
      { "ld   d,sra (iy+%02Xh)",  A_8P}, /* fdcb..2a [undoc] */
      { "ld   e,sra (iy+%02Xh)",  A_8P}, /* fdcb..2b [undoc] */
      { "ld   h,sra (iy+%02Xh)",  A_8P}, /* fdcb..2c [undoc] */
      { "ld   l,sra (iy+%02Xh)",  A_8P}, /* fdcb..2d [undoc] */
      { "sra  (iy+%02Xh)",        A_8P}, /* fdcb..2e */
      { "ld   a,sra (iy+%02Xh)",  A_8P}, /* fdcb..2f [undoc] */

      { "ld   b,slia (iy+%02Xh)", A_8P}, /* fdcb..30 [undoc] */
      { "ld   c,slia (iy+%02Xh)", A_8P}, /* fdcb..31 [undoc] */
      { "ld   d,slia (iy+%02Xh)", A_8P}, /* fdcb..32 [undoc] */
      { "ld   e,slia (iy+%02Xh)", A_8P}, /* fdcb..33 [undoc] */
      { "ld   h,slia (iy+%02Xh)", A_8P}, /* fdcb..34 [undoc] */
      { "ld   l,slia (iy+%02Xh)", A_8P}, /* fdcb..35 [undoc] */
      { "slia (iy+%02Xh)",        A_8P}, /* fdcb..36 [undoc] */
      { "ld   a,slia (iy+%02Xh)", A_8P}, /* fdcb..37 [undoc] */

      { "ld   b,srl (iy+%02Xh)",  A_8P}, /* fdcb..38 [undoc] */
      { "ld   c,srl (iy+%02Xh)",  A_8P}, /* fdcb..39 [undoc] */
      { "ld   d,srl (iy+%02Xh)",  A_8P}, /* fdcb..3a [undoc] */
      { "ld   e,srl (iy+%02Xh)",  A_8P}, /* fdcb..3b [undoc] */
      { "ld   h,srl (iy+%02Xh)",  A_8P}, /* fdcb..3c [undoc] */
      { "ld   l,srl (iy+%02Xh)",  A_8P}, /* fdcb..3d [undoc] */
      { "srl  (iy+%02Xh)",        A_8P}, /* fdcb..3e */
      { "ld   a,srl (iy+%02Xh)",  A_8P}, /* fdcb..3f [undoc] */

      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..40 [undoc] */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..41 [undoc] */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..42 [undoc] */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..43 [undoc] */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..44 [undoc] */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..45 [undoc] */
      { "bit  0,(iy+%02Xh)",  A_8P},          /* fdcb..46 */
      { "bit  0,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..47 [undoc] */

      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..48 [undoc] */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..49 [undoc] */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..4a [undoc] */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..4b [undoc] */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..4c [undoc] */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..4d [undoc] */
      { "bit  1,(iy+%02Xh)",  A_8P},          /* fdcb..4e */
      { "bit  1,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..4f [undoc] */

      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..50 [undoc] */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..51 [undoc] */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..52 [undoc] */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..53 [undoc] */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..54 [undoc] */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..55 [undoc] */
      { "bit  2,(iy+%02Xh)",  A_8P},          /* fdcb..56 */
      { "bit  2,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..57 [undoc] */

      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..58 [undoc] */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..59 [undoc] */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..5a [undoc] */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..5b [undoc] */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..5c [undoc] */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..5d [undoc] */
      { "bit  3,(iy+%02Xh)",  A_8P},          /* fdcb..5e */
      { "bit  3,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..5f [undoc] */

      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..60 [undoc] */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..61 [undoc] */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..62 [undoc] */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..63 [undoc] */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..64 [undoc] */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..65 [undoc] */
      { "bit  4,(iy+%02Xh)",  A_8P},          /* fdcb..66 */
      { "bit  4,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..67 [undoc] */

      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..68 [undoc] */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..69 [undoc] */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..6a [undoc] */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..6b [undoc] */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..6c [undoc] */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..6d [undoc] */
      { "bit  5,(iy+%02Xh)",  A_8P},          /* fdcb..6e */
      { "bit  5,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..6f [undoc] */

      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..70 [undoc] */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..71 [undoc] */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..72 [undoc] */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..73 [undoc] */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..74 [undoc] */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..75 [undoc] */
      { "bit  6,(iy+%02Xh)",  A_8P},          /* fdcb..76 */
      { "bit  6,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..77 [undoc] */

      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..78 [undoc] */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..79 [undoc] */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..7a [undoc] */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..7b [undoc] */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..7c [undoc] */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..7d [undoc] */
      { "bit  7,(iy+%02Xh)",  A_8P},          /* fdcb..7e */
      { "bit  7,(iy+%02Xh)\t;undoc equiv", A_8P}, /* fdcb..7f [undoc] */

      { "ld   b,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..80 [undoc] */
      { "ld   c,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..81 [undoc] */
      { "ld   d,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..82 [undoc] */
      { "ld   e,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..83 [undoc] */
      { "ld   h,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..84 [undoc] */
      { "ld   l,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..85 [undoc] */
      { "res  0,(iy+%02Xh)",      A_8P}, /* fdcb..86 */
      { "ld   a,res 0,(iy+%02Xh)",    A_8P}, /* fdcb..87 [undoc] */

      { "ld   b,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..88 [undoc] */
      { "ld   c,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..89 [undoc] */
      { "ld   d,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..8a [undoc] */
      { "ld   e,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..8b [undoc] */
      { "ld   h,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..8c [undoc] */
      { "ld   l,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..8d [undoc] */
      { "res  1,(iy+%02Xh)",      A_8P}, /* fdcb..8e */
      { "ld   a,res 1,(iy+%02Xh)",    A_8P}, /* fdcb..8f [undoc] */

      { "ld   b,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..90 [undoc] */
      { "ld   c,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..91 [undoc] */
      { "ld   d,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..92 [undoc] */
      { "ld   e,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..93 [undoc] */
      { "ld   h,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..94 [undoc] */
      { "ld   l,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..95 [undoc] */
      { "res  2,(iy+%02Xh)",      A_8P}, /* fdcb..96 */
      { "ld   a,res 2,(iy+%02Xh)",    A_8P}, /* fdcb..97 [undoc] */

      { "ld   b,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..98 [undoc] */
      { "ld   c,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..99 [undoc] */
      { "ld   d,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..9a [undoc] */
      { "ld   e,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..9b [undoc] */
      { "ld   h,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..9c [undoc] */
      { "ld   l,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..9d [undoc] */
      { "res  3,(iy+%02Xh)",      A_8P}, /* fdcb..9e */
      { "ld   a,res 3,(iy+%02Xh)",    A_8P}, /* fdcb..9f [undoc] */

      { "ld   b,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a0 [undoc] */
      { "ld   c,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a1 [undoc] */
      { "ld   d,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a2 [undoc] */
      { "ld   e,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a3 [undoc] */
      { "ld   h,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a4 [undoc] */
      { "ld   l,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a5 [undoc] */
      { "res  4,(iy+%02Xh)",      A_8P}, /* fdcb..a6 */
      { "ld   a,res 4,(iy+%02Xh)",    A_8P}, /* fdcb..a7 [undoc] */

      { "ld   b,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..a8 [undoc] */
      { "ld   c,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..a9 [undoc] */
      { "ld   d,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..aa [undoc] */
      { "ld   e,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..ab [undoc] */
      { "ld   h,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..ac [undoc] */
      { "ld   l,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..ad [undoc] */
      { "res  5,(iy+%02Xh)",      A_8P}, /* fdcb..ae */
      { "ld   a,res 5,(iy+%02Xh)",    A_8P}, /* fdcb..af [undoc] */

      { "ld   b,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b0 [undoc] */
      { "ld   c,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b1 [undoc] */
      { "ld   d,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b2 [undoc] */
      { "ld   e,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b3 [undoc] */
      { "ld   h,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b4 [undoc] */
      { "ld   l,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b5 [undoc] */
      { "res  6,(iy+%02Xh)",      A_8P}, /* fdcb..b6 */
      { "ld   a,res 6,(iy+%02Xh)",    A_8P}, /* fdcb..b7 [undoc] */

      { "ld   b,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..b8 [undoc] */
      { "ld   c,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..b9 [undoc] */
      { "ld   d,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..ba [undoc] */
      { "ld   e,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..bb [undoc] */
      { "ld   h,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..bc [undoc] */
      { "ld   l,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..bd [undoc] */
      { "res  7,(iy+%02Xh)",      A_8P}, /* fdcb..be */
      { "ld   a,res 7,(iy+%02Xh)",    A_8P}, /* fdcb..bf [undoc] */

      { "ld   b,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c0 [undoc] */
      { "ld   c,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c1 [undoc] */
      { "ld   d,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c2 [undoc] */
      { "ld   e,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c3 [undoc] */
      { "ld   h,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c4 [undoc] */
      { "ld   l,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c5 [undoc] */
      { "set  0,(iy+%02Xh)",      A_8P}, /* fdcb..c6 */
      { "ld   a,set 0,(iy+%02Xh)",    A_8P}, /* fdcb..c7 [undoc] */

      { "ld   b,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..c8 [undoc] */
      { "ld   c,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..c9 [undoc] */
      { "ld   d,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..ca [undoc] */
      { "ld   e,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..cb [undoc] */
      { "ld   h,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..cc [undoc] */
      { "ld   l,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..cd [undoc] */
      { "set  1,(iy+%02Xh)",      A_8P}, /* fdcb..ce */
      { "ld   a,set 1,(iy+%02Xh)",    A_8P}, /* fdcb..cf [undoc] */

      { "ld   b,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d0 [undoc] */
      { "ld   c,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d1 [undoc] */
      { "ld   d,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d2 [undoc] */
      { "ld   e,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d3 [undoc] */
      { "ld   h,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d4 [undoc] */
      { "ld   l,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d5 [undoc] */
      { "set  2,(iy+%02Xh)",      A_8P}, /* fdcb..d6 */
      { "ld   a,set 2,(iy+%02Xh)",    A_8P}, /* fdcb..d7 [undoc] */

      { "ld   b,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..d8 [undoc] */
      { "ld   c,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..d9 [undoc] */
      { "ld   d,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..da [undoc] */
      { "ld   e,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..db [undoc] */
      { "ld   h,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..dc [undoc] */
      { "ld   l,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..dd [undoc] */
      { "set  3,(iy+%02Xh)",      A_8P}, /* fdcb..de */
      { "ld   a,set 3,(iy+%02Xh)",    A_8P}, /* fdcb..df [undoc] */

      { "ld   b,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e0 [undoc] */
      { "ld   c,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e1 [undoc] */
      { "ld   d,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e2 [undoc] */
      { "ld   e,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e3 [undoc] */
      { "ld   h,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e4 [undoc] */
      { "ld   l,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e5 [undoc] */
      { "set  4,(iy+%02Xh)",      A_8P}, /* fdcb..e6 */
      { "ld   a,set 4,(iy+%02Xh)",    A_8P}, /* fdcb..e7 [undoc] */

      { "ld   b,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..e8 [undoc] */
      { "ld   c,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..e9 [undoc] */
      { "ld   d,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..ea [undoc] */
      { "ld   e,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..eb [undoc] */
      { "ld   h,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..ec [undoc] */
      { "ld   l,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..ed [undoc] */
      { "set  5,(iy+%02Xh)",      A_8P}, /* fdcb..ee */
      { "ld   a,set 5,(iy+%02Xh)",    A_8P}, /* fdcb..ef [undoc] */

      { "ld   b,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f0 [undoc] */
      { "ld   c,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f1 [undoc] */
      { "ld   d,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f2 [undoc] */
      { "ld   e,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f3 [undoc] */
      { "ld   h,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f4 [undoc] */
      { "ld   l,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f5 [undoc] */
      { "set  6,(iy+%02Xh)",      A_8P}, /* fdcb..f6 */
      { "ld   a,set 6,(iy+%02Xh)",    A_8P}, /* fdcb..f7 [undoc] */

      { "ld   b,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..f8 [undoc] */
      { "ld   c,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..f9 [undoc] */
      { "ld   d,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..fa [undoc] */
      { "ld   e,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..fb [undoc] */
      { "ld   h,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..fc [undoc] */
      { "ld   l,set 7,(iy+%02Xh)",    A_8P}, /* fdcb..fd [undoc] */
      { "set  7,(iy+%02Xh)",      A_8P}, /* fdcb..fe */
      { "ld   a,set 7,(iy+%02Xh)",    A_8P} /* fdcb..ff [undoc] */
    }
};

#define getb(addr) c->getb(c, addr)

static int arglen(int x) {
  return (char)(x & 0xFF);
}

int z80_disasm(computer_t *c, uint16_t pc) {
  opcode_t code;
  uint32_t addr, i, j;

  addr = pc;
  i = getb(pc++);
  if (major[i].name == NULL) {
    j = major[i].args;
    i = getb(pc++);
    if (minor[j][i].name == NULL) {
      /* dd cb or fd cb; offset comes *before* instruction */
      j = minor[j][i].args;
      pc++; /* skip over offset */
      i = getb(pc++);
    }
    code = minor[j][i];
  } else {
    code = major[i];
  }

  fprintf(stderr, "%04X:  ", addr);
  for (i = 0; i < ((pc + arglen(code.args) - addr) & 0xffff); i++) {
    fprintf(stderr, "%02X ", getb(addr + i));
  }

  for (; i < 4; i++) {
    fprintf(stderr, "   ");
  }
  fprintf(stderr, " ");

  switch (code.args) {
    case A_16: /* 16-bit number */
      fprintf(stderr, code.name, getb(pc + 1), getb(pc));
      break;
    case A_8X2: /* Two 8-bit numbers */
      fprintf(stderr, code.name, getb(pc), getb(pc + 1));
      break;
    case A_8:  /* One 8-bit number */
      fprintf(stderr, code.name, getb(pc));
      break;
    case A_8P: /* One 8-bit number before last opcode byte */
      fprintf(stderr, code.name, getb(pc - 2));
      break;
    case A_0:  /* No args */
    case A_0B: /* No args, backskip over last opcode byte */
      fprintf(stderr, code.name, 0);
      break;
    case A_8R: /* One 8-bit relative address */
      fprintf(stderr, code.name, (pc + 1 + (char)getb(pc)) & 0xffff);
      break;
  }

  pc += arglen(code.args);

  return pc;  /* return the location of the next instruction */
}
