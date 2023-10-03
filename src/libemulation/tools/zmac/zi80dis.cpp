//
// zi80dis.cpp - extract disassembly and timing information for Z-80 and 8080 instructions
//
// Standard Z-80 mnemonics are used with extensions for undocumented opcodes.
//		sl1				cb28 - cb2f
//		pfix			DD preceeding DD or FD
//		pfiy			FD preceeding DD of FD
//		ixh,ixl			low, high bytes of IX register
//		iyh,iyl			low, high bytes of IY register
//		ednop			ED with no effect
//		in (c)			ed70
//		out (c),0		ed71
//		srop (ix+d),r	undocumented IX/IY shift/rotate instructions
//
// We don't know what the undocumented Z180 instructions do.  For now
// you're best to turn off undoc when setting Z180 mode.  Though we do
// know that the I[XY][HL] instructions do not work on the Z180.

#include <stdio.h>
#include <string.h>

#include "zi80dis.h"

// TODO: The 8080 extensions are bogus; dunno if they're 8085A or what, exactly.
// Not harmful, but wonder where they come from?
// Answer: they're 8085 undocumented instructions: http://electronicerror.blogspot.ca/2007/08/undocumented-flags-and-instructions.html

static char undefined[] = "undefined";
static char ednop[] = "ednop";
static char pfix[] = "pfix";

struct Opcode {
	const char	*name;
	int			args;
	int			tstates;
	int			tstates8080;
	int			tstates180;
};

// Opcode args if name == 0
// 0 - CB
// 1 - DD|FD
// 2 - ED
// 3 - DD|FD CB

// other Opcode args
#define BC		(4)		/* byte constant */
#define WC		(5)		/* word constant */
#define ADDR	(6)		/* 2 byte memory address */
#define JR		(7)		/* 1 byte relative jump */
#define IND		(8)		/* IX/IY index offset */
#define IND_1	(9)		/* IX/IY index offset and literal byte */
#define CALL	(10)	/* 2 byte memory address as done by call */
#define RST		(11)	/* rst call, addr is inst & 0x38 */
#define PORT	(12)	/* I/O port */
#define UNBR	(256)	/* bit to indicate processor flow goes elsewhere */
#define UNDOC	(512)	/* undocumented instruction */
#define NOEFF	(1024)	/* DD/FD prefix has no effect */
#define MR		(2048)	/* Reads memory */
#define MW		(4096)	/* Writes memory */
#define BT		(8192)	/* Byte operation */
#define WD		(16384)	/* Word operation */
#define IN		(32768)	/* I/O in */
#define OT		(65536)	/* I/O out */

#define FLGMSK	(~255)

// TODO - add flag for repeated instructions

#define VT(low, high)	(((low) << 8) | (high))

static Opcode z_major[256] = {
	{ "nop",			0,				 4,  4,  3 }, /* 00 */
	{ "ld bc,%s",		WC|WD,			10, 10,  9 }, /* 01 */
	{ "ld (bc),a",		BT|MR,			 7,  7,  7 }, /* 02 */
	{ "inc bc",			WD,				 6,  5,  4 }, /* 03 */
	{ "inc b",			BT,				 4,  5,  4 }, /* 04 */
	{ "dec b",			BT,				 4,  5,  4 }, /* 05 */
	{ "ld b,%s",		BC|BT,			 7,  7,  6 }, /* 06 */
	{ "rlca",			BT,				 4,  4,  3 }, /* 07 */

	{ "ex af,af'",		WD,				 4,  4,  4 }, /* 08 */
	{ "add hl,bc",		WD,				11, 10,  7 }, /* 09 */
	{ "ld a,(bc)",		BT|MR,			 7,  7,  6 }, /* 0a */
	{ "dec bc",			WD,				 6,  5,  4 }, /* 0b */
	{ "inc c",			BT,				 4,  5,  4 }, /* 0c */
	{ "dec c",			BT,				 4,  5,  4 }, /* 0d */
	{ "ld c,%s",		BC|BT,			 7,  7,  6 }, /* 0e */
	{ "rrca",			BT,				 4,  4,  3 }, /* 0f */

	{ "djnz %s",		JR,		 VT(8, 13),  4, VT(7, 9) }, /* 10 */
	{ "ld de,%s",		WC|WD,			10, 10,  9 }, /* 11 */
	{ "ld (de),a",		BT|MW,			 7,  7,  7 }, /* 12 */
	{ "inc de",			WD,				 6,  5,  4 }, /* 13 */
	{ "inc d",			BT,				 4,  5,  4 }, /* 14 */
	{ "dec d",			BT,				 4,  5,  4 }, /*15 */
	{ "ld d,%s",		BC|BT,			 7,  7,  6 }, /* 16 */
	{ "rla",			BT,				 4,  4,  3 }, /* 17 */

	{ "jr %s",			JR | UNBR,		12,  4,  8 }, /* 18 */
	{ "add hl,de",		WD,				11, 10,  7 }, /* 19 */
	{ "ld a,(de)",		BT|MW,			 7,  7,  6 }, /* 1a */
	{ "dec de",			WD,				 6,  5,  4 }, /* 1b */
	{ "inc e",			BT,				 4,  5,  4 }, /* 1c */
	{ "dec e",			BT,				 4,  5,  4 }, /* 1d */
	{ "ld e,%s",		BC|BT,			 7,  7,  6 }, /* 1e */
	{ "rra",			BT,				 4,  4,  3 }, /* 1f */

	{ "jr nz,%s",		JR,		 VT(7, 12),  4, VT(6, 8) }, /* 20 */
	{ "ld hl,%s",		WC|WD,			10, 10,  9 }, /* 21 */
	{ "ld (%s),hl",		ADDR|WD|MW,		16, 16, 16 }, /* 22 */
	{ "inc hl",			WD,				 6,  5,  4 }, /* 23 */
	{ "inc h",			BT,				 4,  5,  4 }, /* 24 */
	{ "dec h",			BT,				 4,  5,  4 }, /* 25 */
	{ "ld h,%s",		BC|BT,			 7,  7,  6 }, /* 26 */
	{ "daa",			BT,				 4,  4,  4 }, /* 27 */

	{ "jr z,%s",		JR,		 VT(7, 12),  4, VT(6, 8) }, /* 28 */
	{ "add hl,hl",		WD,				11, 10,  7 }, /* 29 */
	{ "ld hl,(%s)",		ADDR|WD|MW,		16, 16, 15 }, /* 2a */
	{ "dec hl",			WD,				 6,  5,  4 }, /* 2b */
	{ "inc l",			BT,				 4,  5,  4 }, /* 2c */
	{ "dec l",			BT,				 4,  5,  4 }, /* 2d */
	{ "ld l,%s",		BC|BT,			 7,  7,  6 }, /* 2e */
	{ "cpl",			BT,				 4,  4,  4 }, /* 2f */

	{ "jr nc,%s",		JR,		 VT(7, 12),  4, VT(6, 8) }, /* 30 */
	{ "ld sp,%s",		WC|WD,			10, 10,  9 }, /* 31 */
	{ "ld (%s),a",		ADDR|BT|MW,		13, 13, 13 }, /* 32 */
	{ "inc sp",			WD,				 6,  5,  4 }, /* 33 */
	{ "inc (hl)",		BT|MR|MW,		11, 10, 10 }, /* 34 */
	{ "dec (hl)",		BT|MR|MW,		11, 10, 10 }, /* 35 */
	{ "ld (hl),%s",		BC|WD|MW,		10, 10,  9 }, /* 36 */
	{ "scf",			0,				 4,  4,  3 }, /* 37 */

	{ "jr c,%s",		JR,		 VT(7, 12),  4, VT(6, 8) }, /* 38 */
	{ "add hl,sp",		WD,				11, 10,  7 }, /* 39 */
	{ "ld a,(%s)",		ADDR|BT|MR,		13, 13, 12 }, /* 3a */
	{ "dec sp",			WD,				 6,  5,  4 }, /* 3b */
	{ "inc a",			BT,				 4,  5,  4 }, /* 3c */
	{ "dec a",			BT,				 4,  5,  4 }, /* 3d */
	{ "ld a,%s",		BC|BT,			 7,  7,  6 }, /* 3e */
	{ "ccf",			0,				 4,  4,  3 }, /* 3f */

	{ "ld b,b",			BT,				 4,  5,  4 }, /* 40 */
	{ "ld b,c",			BT,				 4,  5,  4 }, /* 41 */
	{ "ld b,d",			BT,				 4,  5,  4 }, /* 42 */
	{ "ld b,e",			BT,				 4,  5,  4 }, /* 43 */
	{ "ld b,h",			BT,				 4,  5,  4 }, /* 44 */
	{ "ld b,l",			BT,				 4,  5,  4 }, /* 45 */
	{ "ld b,(hl)",		BT|MR,			 7,  7,  6 }, /* 46 */
	{ "ld b,a",			BT,				 4,  5,  4 }, /* 47 */

	{ "ld c,b",			BT,				 4,  5,  4 }, /* 48 */
	{ "ld c,c",			BT,				 4,  5,  4 }, /* 49 */
	{ "ld c,d",			BT,				 4,  5,  4 }, /* 4a */
	{ "ld c,e",			BT,				 4,  5,  4 }, /* 4b */
	{ "ld c,h",			BT,				 4,  5,  4 }, /* 4c */
	{ "ld c,l",			BT,				 4,  5,  4 }, /* 4d */
	{ "ld c,(hl)",		BT|MR,			 7,  7,  6 }, /* 4e */
	{ "ld c,a",			BT,				 4,  5,  4 }, /* 4f */

	{ "ld d,b",			BT,				 4,  5,  4 }, /* 50 */
	{ "ld d,c",			BT,				 4,  5,  4 }, /* 51 */
	{ "ld d,d",			BT,				 4,  5,  4 }, /* 52 */
	{ "ld d,e",			BT,				 4,  5,  4 }, /* 53 */
	{ "ld d,h",			BT,				 4,  5,  4 }, /* 54 */
	{ "ld d,l",			BT,				 4,  5,  4 }, /* 55 */
	{ "ld d,(hl)",		BT|MR,			 7,  7,  6 }, /* 56 */
	{ "ld d,a",			BT,				 4,  5,  4 }, /* 57 */

	{ "ld e,b",			BT,				 4,  5,  4 }, /* 58 */
	{ "ld e,c",			BT,				 4,  5,  4 }, /* 59 */
	{ "ld e,d",			BT,				 4,  5,  4 }, /* 5a */
	{ "ld e,e",			BT,				 4,  5,  4 }, /* 5b */
	{ "ld e,h",			BT,				 4,  5,  4 }, /* 5c */
	{ "ld e,l",			BT,				 4,  5,  4 }, /* 5d */
	{ "ld e,(hl)",		BT|MR,			 7,  7,  6 }, /* 5e */
	{ "ld e,a",			BT,				 4,  5,  4 }, /* 5f */

	{ "ld h,b",			BT,				 4,  5,  4 }, /* 60 */
	{ "ld h,c",			BT,				 4,  5,  4 }, /* 61 */
	{ "ld h,d",			BT,				 4,  5,  4 }, /* 62 */
	{ "ld h,e",			BT,				 4,  5,  4 }, /* 63 */
	{ "ld h,h",			BT,				 4,  5,  4 }, /* 64 */
	{ "ld h,l",			BT,				 4,  5,  4 }, /* 65 */
	{ "ld h,(hl)",		BT|MR,			 7,  7,  6 }, /* 66 */
	{ "ld h,a",			BT,				 4,  5,  4 }, /* 67 */

	{ "ld l,b",			BT,				 4,  5,  4 }, /* 68 */
	{ "ld l,c",			BT,				 4,  5,  4 }, /* 69 */
	{ "ld l,d",			BT,				 4,  5,  4 }, /* 6a */
	{ "ld l,e",			BT,				 4,  5,  4 }, /* 6b */
	{ "ld l,h",			BT,				 4,  5,  4 }, /* 6c */
	{ "ld l,l",			BT,				 4,  5,  4 }, /* 6d */
	{ "ld l,(hl)",		BT|MR,			 7,  7,  6 }, /* 6e */
	{ "ld l,a",			BT,				 4,  5,  4 }, /* 6f */

	{ "ld (hl),b",		BT|MW,			 7,  7,  7 }, /* 70 */
	{ "ld (hl),c",		BT|MW,			 7,  7,  7 }, /* 71 */
	{ "ld (hl),d",		BT|MW,			 7,  7,  7 }, /* 72 */
	{ "ld (hl),e",		BT|MW,			 7,  7,  7 }, /* 73 */
	{ "ld (hl),h",		BT|MW,			 7,  7,  7 }, /* 74 */
	{ "ld (hl),l",		BT|MW,			 7,  7,  7 }, /* 75 */
	{ "halt",			0,				 4,  7,  3 }, /* 76 */
	{ "ld (hl),a",		BT|MW,			 7,  7,  7 }, /* 77 */

	{ "ld a,b",			BT,				 4,  5,  4 }, /* 78 */
	{ "ld a,c",			BT,				 4,  5,  4 }, /* 79 */
	{ "ld a,d",			BT,				 4,  5,  4 }, /* 7a */
	{ "ld a,e",			BT,				 4,  5,  4 }, /* 7b */
	{ "ld a,h",			BT,				 4,  5,  4 }, /* 7c */
	{ "ld a,l",			BT,				 4,  5,  4 }, /* 7d */
	{ "ld a,(hl)",		BT|MR,			 7,  7,  7 }, /* 7e */
	{ "ld a,a",			BT,				 4,  5,  4 }, /* 7f */

	{ "add a,b",		BT,				 4,  4,  4 }, /* 80 */
	{ "add a,c",		BT,				 4,  4,  4 }, /* 81 */
	{ "add a,d",		BT,				 4,  4,  4 }, /* 82 */
	{ "add a,e",		BT,				 4,  4,  4 }, /* 83 */
	{ "add a,h",		BT,				 4,  4,  4 }, /* 84 */
	{ "add a,l",		BT,				 4,  4,  4 }, /* 85 */
	{ "add a,(hl)",		BT|MR,			 7,  7,  6 }, /* 86 */
	{ "add a,a",		BT,				 4,  4,  4 }, /* 87 */

	{ "adc a,b",		BT,				 4,  4,  4 }, /* 88 */
	{ "adc a,c",		BT,				 4,  4,  4 }, /* 89 */
	{ "adc a,d",		BT,				 4,  4,  4 }, /* 8a */
	{ "adc a,e",		BT,				 4,  4,  4 }, /* 8b */
	{ "adc a,h",		BT,				 4,  4,  4 }, /* 8c */
	{ "adc a,l",		BT,				 4,  4,  4 }, /* 8d */
	{ "adc a,(hl)",		BT|MR,			 7,  7,  6 }, /* 8e */
	{ "adc a,a",		BT,				 4,  4,  4 }, /* 8f */

	{ "sub b",			BT,				 4,  4,  4 }, /* 90 */
	{ "sub c",			BT,				 4,  4,  4 }, /* 91 */
	{ "sub d",			BT,				 4,  4,  4 }, /* 92 */
	{ "sub e",			BT,				 4,  4,  4 }, /* 93 */
	{ "sub h",			BT,				 4,  4,  4 }, /* 94 */
	{ "sub l",			BT,				 4,  4,  4 }, /* 95 */
	{ "sub (hl)",		BT|MR,			 7,  7,  6 }, /* 96 */
	{ "sub a",			BT,				 4,  4,  4 }, /* 97 */

	{ "sbc a,b",		BT,				 4,  4,  4 }, /* 98 */
	{ "sbc a,c",		BT,				 4,  4,  4 }, /* 99 */
	{ "sbc a,d",		BT,				 4,  4,  4 }, /* 9a */
	{ "sbc a,e",		BT,				 4,  4,  4 }, /* 9b */
	{ "sbc a,h",		BT,				 4,  4,  4 }, /* 9c */
	{ "sbc a,l",		BT,				 4,  4,  4 }, /* 9d */
	{ "sbc a,(hl)",		BT|MR,			 7,  7,  6 }, /* 9e */
	{ "sbc a,a",		BT,				 4,  4,  4 }, /* 9f */

	{ "and b",			BT,				 4,  4,  4 }, /* a0 */
	{ "and c",			BT,				 4,  4,  4 }, /* a1 */
	{ "and d",			BT,				 4,  4,  4 }, /* a2 */
	{ "and e",			BT,				 4,  4,  4 }, /* a3 */
	{ "and h",			BT,				 4,  4,  4 }, /* a4 */
	{ "and l",			BT,				 4,  4,  4 }, /* a5 */
	{ "and (hl)",		BT|MR,			 7,  7,  6 }, /* a6 */
	{ "and a",			BT,				 4,  4,  4 }, /* a7 */

	{ "xor b",			BT,				 4,  4,  4 }, /* a8 */
	{ "xor c",			BT,				 4,  4,  4 }, /* a9 */
	{ "xor d",			BT,				 4,  4,  4 }, /* aa */
	{ "xor e",			BT,				 4,  4,  4 }, /* ab */
	{ "xor h",			BT,				 4,  4,  4 }, /* ac */
	{ "xor l",			BT,				 4,  4,  4 }, /* ad */
	{ "xor (hl)",		BT|MR,			 7,  7,  6 }, /* ae */
	{ "xor a",			BT,				 4,  4,  4 }, /* af */

	{ "or b",			BT,				 4,  4,  4 }, /* b0 */
	{ "or c",			BT,				 4,  4,  4 }, /* b1 */
	{ "or d",			BT,				 4,  4,  4 }, /* b2 */
	{ "or e",			BT,				 4,  4,  4 }, /* b3 */
	{ "or h",			BT,				 4,  4,  4 }, /* b4 */
	{ "or l",			BT,				 4,  4,  4 }, /* b5 */
	{ "or (hl)",		BT|MR,			 7,  7,  6 }, /* b6 */
	{ "or a",			BT,				 4,  4,  4 }, /* b7 */

	{ "cp b",			BT,				 4,  4,  4 }, /* b8 */
	{ "cp c",			BT,			 	 4,  4,  4 }, /* b9 */
	{ "cp d",			BT,				 4,  4,  4 }, /* ba */
	{ "cp e",			BT,				 4,  4,  4 }, /* bb */
	{ "cp h",			BT,				 4,  4,  4 }, /* bc */
	{ "cp l",			BT,				 4,  4,  4 }, /* bd */
	{ "cp (hl)",		BT|MR,			 7,  7,  6 }, /* be */
	{ "cp a",			BT,				 4,  4,  4 }, /* bf */

	{ "ret nz",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* c0 */
	{ "pop bc",			WD|MR,			10, 10,  9 }, /* c1 */
	{ "jp nz,%s",		ADDR,			10, 10, VT(6, 9) }, /* c2 */
	{ "jp %s",			ADDR | UNBR,	10, 10,  9 }, /* c3 */
	{ "call nz,%s",		CALL,   VT(10, 17), VT(11, 17), VT(6, 16) }, /* c4 */
	{ "push bc",		WD|MW,			11, 11, 11 }, /* c5 */
	{ "add a,%s",		BC|BT,			 7,  7,  6 }, /* c6 */
	{ "rst %s",			RST,			11, 11, 11 }, /* c7 */

	{ "ret z",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* c8 */
	{ "ret",			WD | UNBR,		10, 10,  9 }, /* c9 */
	{ "jp z,%s",		ADDR,			10, 10, VT(6, 9) }, /* ca */
	{ 0,				0,				 0, 10,  0 }, /* cb */
	{ "call z,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* cc */
	{ "call %s",		CALL,			17, 17, 16 }, /* cd */
	{ "adc a,%s",		BC|BT,			 7,  7,  6 }, /* ce */
	{ "rst %s",			RST,			11, 11, 11 }, /* cf */

	{ "ret nc",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* d0 */
	{ "pop de",			WD|MR,			10, 10,  9 }, /* d1 */
	{ "jp nc,%s",		ADDR,			10, 10, VT(6, 9) }, /* d2 */
	{ "out (%s),a",		PORT|OT,		11, 10, 10 }, /* d3 */
	{ "call nc,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* d4 */
	{ "push de",		WD|MW,			11, 11, 11 }, /* d5 */
	{ "sub %s",			BC|BT,			 7,  7,  6 }, /* d6 */
	{ "rst %s",			RST,			11, 11, 11 }, /* d7 */

	{ "ret c",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* d8 */
	{ "exx",			WD,				 4, 10,  3 }, /* d9 */
	{ "jp c,%s",		ADDR,			10, 10, VT(6, 9) }, /* da */
	{ "in a,(%s)",		PORT|IN,		11, 10,  9 }, /* db */
	{ "call c,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* dc */
	{ 0,				1,				 0, 17,  0 }, /* dd */
	{ "sbc a,%s",		BC|BT,			 7,  7,  6 }, /* de */
	{ "rst %s",			RST,			11, 11, 11 }, /* df */

	{ "ret po",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* e0 */
	{ "pop hl",			WD|MR,			10, 10,  9 }, /* e1 */
	{ "jp po,%s",		ADDR,			10, 10, VT(6, 9) }, /* e2 */
	{ "ex (sp),hl",		WD|MR|MW,		19, 18, 16 }, /* e3 */
	{ "call po,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* e4 */
	{ "push hl",		WD|MR,			11, 11, 11 }, /* e5 */
	{ "and %s",			BC|BT,			 7,  7,  6 }, /* e6 */
	{ "rst %s",			RST,			11, 11, 11 }, /* e7 */

	{ "ret pe",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* e8 */
	{ "jp (hl)",		0 | UNBR,		 4,  5,  3 }, /* e9 */
	{ "jp pe,%s",		ADDR,			10, 10, VT(6, 9) }, /* ea */
	{ "ex de,hl",		WD,				 4,  4,  3 }, /* eb */
	{ "call pe,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* ec */
	{ 0,				2,				 0, 17,  0 }, /* ed */
	{ "xor %s",			BC|BT,			 7,  7,  6 }, /* ee */
	{ "rst %s",			RST,			11, 11, 11 }, /* ef */

	{ "ret p",			0,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* f0 */
	{ "pop af",			WD|MR,			10, 10,  9 }, /* f1 */
	{ "jp p,%s",		ADDR,			10, 10, VT(6, 9) }, /* f2 */
	{ "di",				0,				 4,  4,  3 }, /* f3 */
	{ "call p,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* f4 */
	{ "push af",		WD|MW,			11, 11, 11 }, /* f5 */
	{ "or %s",			BC|BT,			 7,  7,  6 }, /* f6 */
	{ "rst %s",			RST,			11, 11, 11 }, /* f7 */

	{ "ret m",			WD,		 VT(5, 11), VT(5, 11), VT(5, 10) }, /* f8 */
	{ "ld sp,hl",		WD,				 6,  5,  4 }, /* f9 */
	{ "jp m,%s",		ADDR,			10, 10, VT(6, 9) }, /* fa */
	{ "ei",				0,				 4,  4,  3 }, /* fb */
	{ "call m,%s",		CALL,	VT(10, 17), VT(11, 17), VT(6, 16) }, /* fc */
	{ 0,				1,				 0, 17,  0 }, /* fd */
	{ "cp %s",			BC|BT,			 7,  7,  6 }, /* fe */
	{ "rst %s",			RST,			11, 11, 11 }, /* ff */
};

static Opcode z_minor[3][256] = {
  {							/* cb */
	{ "rlc b",			BT,				 8, 0,  7 }, /* cb00 */
	{ "rlc c",			BT,				 8, 0,  7 }, /* cb01 */
	{ "rlc d",			BT,				 8, 0,  7 }, /* cb02 */
	{ "rlc e",			BT,				 8, 0,  7 }, /* cb03 */
	{ "rlc h",			BT,				 8, 0,  7 }, /* cb04 */
	{ "rlc l",			BT,				 8, 0,  7 }, /* cb05 */
	{ "rlc (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb06 */
	{ "rlc a",			BT,				 8, 0,  7 }, /* cb07 */

	{ "rrc b",			BT,				 8, 0,  7 }, /* cb08 */
	{ "rrc c",			BT,				 8, 0,  7 }, /* cb09 */
	{ "rrc d",			BT,				 8, 0,  7 }, /* cb0a */
	{ "rrc e",			BT,				 8, 0,  7 }, /* cb0b */
	{ "rrc h",			BT,				 8, 0,  7 }, /* cb0c */
	{ "rrc l",			BT,				 8, 0,  7 }, /* cb0d */
	{ "rrc (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb0e */
	{ "rrc a",			BT,				 8, 0,  7 }, /* cb0f */

	{ "rl b",			BT,				 8, 0,  7 }, /* cb10 */
	{ "rl c",			BT,				 8, 0,  7 }, /* cb11 */
	{ "rl d",			BT,				 8, 0,  7 }, /* cb12 */
	{ "rl e",			BT,				 8, 0,  7 }, /* cb13 */
	{ "rl h",			BT,				 8, 0,  7 }, /* cb14 */
	{ "rl l",			BT,				 8, 0,  7 }, /* cb15 */
	{ "rl (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb16 */
	{ "rl a",			BT,				 8, 0,  7 }, /* cb17 */

	{ "rr b",			BT,				 8, 0,  7 }, /* cb18 */
	{ "rr c",			BT,				 8, 0,  7 }, /* cb19 */
	{ "rr d",			BT,				 8, 0,  7 }, /* cb1a */
	{ "rr e",			BT,				 8, 0,  7 }, /* cb1b */
	{ "rr h",			BT,				 8, 0,  7 }, /* cb1c */
	{ "rr l",			BT,				 8, 0,  7 }, /* cb1d */
	{ "rr (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb1e */
	{ "rr a",			BT,				 8, 0,  7 }, /* cb1f */

	{ "sla b",			BT,				 8, 0,  7 }, /* cb20 */
	{ "sla c",			BT,				 8, 0,  7 }, /* cb21 */
	{ "sla d",			BT,				 8, 0,  7 }, /* cb22 */
	{ "sla e",			BT,				 8, 0,  7 }, /* cb23 */
	{ "sla h",			BT,				 8, 0,  7 }, /* cb24 */
	{ "sla l",			BT,				 8, 0,  7 }, /* cb25 */
	{ "sla (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb26 */
	{ "sla a",			BT,				 8, 0,  7 }, /* cb27 */

	{ "sra b",			BT,				 8, 0,  7 }, /* cb28 */
	{ "sra c",			BT,				 8, 0,  7 }, /* cb29 */
	{ "sra d",			BT,				 8, 0,  7 }, /* cb2a */
	{ "sra e",			BT,				 8, 0,  7 }, /* cb2b */
	{ "sra h",			BT,				 8, 0,  7 }, /* cb2c */
	{ "sra l",			BT,				 8, 0,  7 }, /* cb2d */
	{ "sra (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb2e */
	{ "sra a",			BT,				 8, 0,  7 }, /* cb2f */

	{ "sl1 b",			UNDOC|BT,		 8, 0,  0 }, /* cb30 */
	{ "sl1 c",			UNDOC|BT,		 8, 0,  0 }, /* cb31 */
	{ "sl1 d",			UNDOC|BT,		 8, 0,  0 }, /* cb32 */
	{ "sl1 e",			UNDOC|BT,		 8, 0,  0 }, /* cb33 */
	{ "sl1 h",			UNDOC|BT,		 8, 0,  0 }, /* cb34 */
	{ "sl1 l",			UNDOC|BT,		 8, 0,  0 }, /* cb35 */
	{ "sl1 (hl)",		UNDOC|BT|MR|MW,	15, 0,  0 }, /* cb36 */
	{ "sl1 a",			UNDOC|BT,		 8, 0,  0 }, /* cb37 */

	{ "srl b",			BT,				 8, 0,  7 }, /* cb38 */
	{ "srl c",			BT,				 8, 0,  7 }, /* cb39 */
	{ "srl d",			BT,				 8, 0,  7 }, /* cb3a */
	{ "srl e",			BT,				 8, 0,  7 }, /* cb3b */
	{ "srl h",			BT,				 8, 0,  7 }, /* cb3c */
	{ "srl l",			BT,				 8, 0,  7 }, /* cb3d */
	{ "srl (hl)",		BT|MR|MW,		15, 0, 13 }, /* cb3e */
	{ "srl a",			BT,				 8, 0,  7 }, /* cb3f */

	{ "bit 0,b",		BT,				 8, 0,  6 }, /* cb40 */
	{ "bit 0,c",		BT,				 8, 0,  6 }, /* cb41 */
	{ "bit 0,d",		BT,				 8, 0,  6 }, /* cb42 */
	{ "bit 0,e",		BT,				 8, 0,  6 }, /* cb43 */
	{ "bit 0,h",		BT,				 8, 0,  6 }, /* cb44 */
	{ "bit 0,l",		BT,				 8, 0,  6 }, /* cb45 */
	{ "bit 0,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb46 */
	{ "bit 0,a",		BT,				 8, 0,  6 }, /* cb47 */

	{ "bit 1,b",		BT,				 8, 0,  6 }, /* cb48 */
	{ "bit 1,c",		BT,				 8, 0,  6 }, /* cb49 */
	{ "bit 1,d",		BT,				 8, 0,  6 }, /* cb4a */
	{ "bit 1,e",		BT,				 8, 0,  6 }, /* cb4b */
	{ "bit 1,h",		BT,				 8, 0,  6 }, /* cb4c */
	{ "bit 1,l",		BT,				 8, 0,  6 }, /* cb4d */
	{ "bit 1,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb4e */
	{ "bit 1,a",		BT,				 8, 0,  6 }, /* cb4f */

	{ "bit 2,b",		BT,				 8, 0,  6 }, /* cb50 */
	{ "bit 2,c",		BT,				 8, 0,  6 }, /* cb51 */
	{ "bit 2,d",		BT,				 8, 0,  6 }, /* cb52 */
	{ "bit 2,e",		BT,				 8, 0,  6 }, /* cb53 */
	{ "bit 2,h",		BT,				 8, 0,  6 }, /* cb54 */
	{ "bit 2,l",		BT,				 8, 0,  6 }, /* cb55 */
	{ "bit 2,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb56 */
	{ "bit 2,a",		BT,				 8, 0,  6 }, /* cb57 */

	{ "bit 3,b",		BT,				 8, 0,  6 }, /* cb58 */
	{ "bit 3,c",		BT,				 8, 0,  6 }, /* cb59 */
	{ "bit 3,d",		BT,				 8, 0,  6 }, /* cb5a */
	{ "bit 3,e",		BT,				 8, 0,  6 }, /* cb5b */
	{ "bit 3,h",		BT,				 8, 0,  6 }, /* cb5c */
	{ "bit 3,l",		BT,				 8, 0,  6 }, /* cb5d */
	{ "bit 3,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb5e */
	{ "bit 3,a",		BT,				 8, 0,  6 }, /* cb5f */

	{ "bit 4,b",		BT,				 8, 0,  6 }, /* cb60 */
	{ "bit 4,c",		BT,				 8, 0,  6 }, /* cb61 */
	{ "bit 4,d",		BT,				 8, 0,  6 }, /* cb62 */
	{ "bit 4,e",		BT,				 8, 0,  6 }, /* cb63 */
	{ "bit 4,h",		BT,				 8, 0,  6 }, /* cb64 */
	{ "bit 4,l",		BT,				 8, 0,  6 }, /* cb65 */
	{ "bit 4,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb66 */
	{ "bit 4,a",		BT,				 8, 0,  6 }, /* cb67 */

	{ "bit 5,b",		BT,				 8, 0,  6 }, /* cb68 */
	{ "bit 5,c",		BT,				 8, 0,  6 }, /* cb69 */
	{ "bit 5,d",		BT,				 8, 0,  6 }, /* cb6a */
	{ "bit 5,e",		BT,				 8, 0,  6 }, /* cb6b */
	{ "bit 5,h",		BT,				 8, 0,  6 }, /* cb6c */
	{ "bit 5,l",		BT,				 8, 0,  6 }, /* cb6d */
	{ "bit 5,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb6e */
	{ "bit 5,a",		BT,				 8, 0,  6 }, /* cb6f */

	{ "bit 6,b",		BT,				 8, 0,  6 }, /* cb70 */
	{ "bit 6,c",		BT,				 8, 0,  6 }, /* cb71 */
	{ "bit 6,d",		BT,				 8, 0,  6 }, /* cb72 */
	{ "bit 6,e",		BT,				 8, 0,  6 }, /* cb73 */
	{ "bit 6,h",		BT,				 8, 0,  6 }, /* cb74 */
	{ "bit 6,l",		BT,				 8, 0,  6 }, /* cb75 */
	{ "bit 6,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb76 */
	{ "bit 6,a",		BT,				 8, 0,  6 }, /* cb77 */

	{ "bit 7,b",		BT,				 8, 0,  6 }, /* cb78 */
	{ "bit 7,c",		BT,				 8, 0,  6 }, /* cb79 */
	{ "bit 7,d",		BT,				 8, 0,  6 }, /* cb7a */
	{ "bit 7,e",		BT,				 8, 0,  6 }, /* cb7b */
	{ "bit 7,h",		BT,				 8, 0,  6 }, /* cb7c */
	{ "bit 7,l",		BT,				 8, 0,  6 }, /* cb7d */
	{ "bit 7,(hl)",		BT|MR|MW,		12, 0,  9 }, /* cb7e */
	{ "bit 7,a",		BT,				 8, 0,  6 }, /* cb7f */

	{ "res 0,b",		BT,				 8, 0,  7 }, /* cb80 */
	{ "res 0,c",		BT,				 8, 0,  7 }, /* cb81 */
	{ "res 0,d",		BT,				 8, 0,  7 }, /* cb82 */
	{ "res 0,e",		BT,				 8, 0,  7 }, /* cb83 */
	{ "res 0,h",		BT,				 8, 0,  7 }, /* cb84 */
	{ "res 0,l",		BT,				 8, 0,  7 }, /* cb85 */
	{ "res 0,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cb86 */
	{ "res 0,a",		BT,				 8, 0,  7 }, /* cb87 */

	{ "res 1,b",		BT,				 8, 0,  7 }, /* cb88 */
	{ "res 1,c",		BT,				 8, 0,  7 }, /* cb89 */
	{ "res 1,d",		BT,				 8, 0,  7 }, /* cb8a */
	{ "res 1,e",		BT,				 8, 0,  7 }, /* cb8b */
	{ "res 1,h",		BT,				 8, 0,  7 }, /* cb8c */
	{ "res 1,l",		BT,				 8, 0,  7 }, /* cb8d */
	{ "res 1,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cb8e */
	{ "res 1,a",		BT,				 8, 0,  7 }, /* cb8f */

	{ "res 2,b",		BT,				 8, 0,  7 }, /* cb90 */
	{ "res 2,c",		BT,				 8, 0,  7 }, /* cb91 */
	{ "res 2,d",		BT,				 8, 0,  7 }, /* cb92 */
	{ "res 2,e",		BT,				 8, 0,  7 }, /* cb93 */
	{ "res 2,h",		BT,				 8, 0,  7 }, /* cb94 */
	{ "res 2,l",		BT,				 8, 0,  7 }, /* cb95 */
	{ "res 2,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cb96 */
	{ "res 2,a",		BT,				 8, 0,  7 }, /* cb97 */

	{ "res 3,b",		BT,				 8, 0,  7 }, /* cb98 */
	{ "res 3,c",		BT,				 8, 0,  7 }, /* cb99 */
	{ "res 3,d",		BT,				 8, 0,  7 }, /* cb9a */
	{ "res 3,e",		BT,				 8, 0,  7 }, /* cb9b */
	{ "res 3,h",		BT,				 8, 0,  7 }, /* cb9c */
	{ "res 3,l",		BT,				 8, 0,  7 }, /* cb9d */
	{ "res 3,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cb9e */
	{ "res 3,a",		BT,				 8, 0,  7 }, /* cb9f */

	{ "res 4,b",		BT,				 8, 0,  7 }, /* cba0 */
	{ "res 4,c",		BT,				 8, 0,  7 }, /* cba1 */
	{ "res 4,d",		BT,				 8, 0,  7 }, /* cba2 */
	{ "res 4,e",		BT,				 8, 0,  7 }, /* cba3 */
	{ "res 4,h",		BT,				 8, 0,  7 }, /* cba4 */
	{ "res 4,l",		BT,				 8, 0,  7 }, /* cba5 */
	{ "res 4,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cba6 */
	{ "res 4,a",		BT,				 8, 0,  7 }, /* cba7 */

	{ "res 5,b",		BT,				 8, 0,  7 }, /* cba8 */
	{ "res 5,c",		BT,				 8, 0,  7 }, /* cba9 */
	{ "res 5,d",		BT,				 8, 0,  7 }, /* cbaa */
	{ "res 5,e",		BT,				 8, 0,  7 }, /* cbab */
	{ "res 5,h",		BT,				 8, 0,  7 }, /* cbac */
	{ "res 5,l",		BT,				 8, 0,  7 }, /* cbad */
	{ "res 5,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbae */
	{ "res 5,a",		BT,				 8, 0,  7 }, /* cbaf */

	{ "res 6,b",		BT,				 8, 0,  7 }, /* cbb0 */
	{ "res 6,c",		BT,				 8, 0,  7 }, /* cbb1 */
	{ "res 6,d",		BT,				 8, 0,  7 }, /* cbb2 */
	{ "res 6,e",		BT,				 8, 0,  7 }, /* cbb3 */
	{ "res 6,h",		BT,				 8, 0,  7 }, /* cbb4 */
	{ "res 6,l",		BT,				 8, 0,  7 }, /* cbb5 */
	{ "res 6,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbb6 */
	{ "res 6,a",		BT,				 8, 0,  7 }, /* cbb7 */

	{ "res 7,b",		BT,				 8, 0,  7 }, /* cbb8 */
	{ "res 7,c",		BT,				 8, 0,  7 }, /* cbb9 */
	{ "res 7,d",		BT,				 8, 0,  7 }, /* cbba */
	{ "res 7,e",		BT,				 8, 0,  7 }, /* cbbb */
	{ "res 7,h",		BT,				 8, 0,  7 }, /* cbbc */
	{ "res 7,l",		BT,				 8, 0,  7 }, /* cbbd */
	{ "res 7,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbbe */
	{ "res 7,a",		BT,				 8, 0,  7 }, /* cbbf */

	{ "set 0,b",		BT,				 8, 0,  7 }, /* cbc0 */
	{ "set 0,c",		BT,				 8, 0,  7 }, /* cbc1 */
	{ "set 0,d",		BT,				 8, 0,  7 }, /* cbc2 */
	{ "set 0,e",		BT,				 8, 0,  7 }, /* cbc3 */
	{ "set 0,h",		BT,				 8, 0,  7 }, /* cbc4 */
	{ "set 0,l",		BT,				 8, 0,  7 }, /* cbc5 */
	{ "set 0,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbc6 */
	{ "set 0,a",		BT,				 8, 0,  7 }, /* cbc7 */

	{ "set 1,b",		BT,				 8, 0,  7 }, /* cbc8 */
	{ "set 1,c",		BT,				 8, 0,  7 }, /* cbc9 */
	{ "set 1,d",		BT,				 8, 0,  7 }, /* cbca */
	{ "set 1,e",		BT,				 8, 0,  7 }, /* cbcb */
	{ "set 1,h",		BT,				 8, 0,  7 }, /* cbcc */
	{ "set 1,l",		BT,				 8, 0,  7 }, /* cbcd */
	{ "set 1,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbce */
	{ "set 1,a",		BT,				 8, 0,  7 }, /* cbcf */

	{ "set 2,b",		BT,				 8, 0,  7 }, /* cbd0 */
	{ "set 2,c",		BT,				 8, 0,  7 }, /* cbd1 */
	{ "set 2,d",		BT,				 8, 0,  7 }, /* cbd2 */
	{ "set 2,e",		BT,				 8, 0,  7 }, /* cbd3 */
	{ "set 2,h",		BT,				 8, 0,  7 }, /* cbd4 */
	{ "set 2,l",		BT,				 8, 0,  7 }, /* cbd5 */
	{ "set 2,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbd6 */
	{ "set 2,a",		BT,				 8, 0,  7 }, /* cbd7 */

	{ "set 3,b",		BT,				 8, 0,  7 }, /* cbd8 */
	{ "set 3,c",		BT,				 8, 0,  7 }, /* cbd9 */
	{ "set 3,d",		BT,				 8, 0,  7 }, /* cbda */
	{ "set 3,e",		BT,				 8, 0,  7 }, /* cbdb */
	{ "set 3,h",		BT,				 8, 0,  7 }, /* cbdc */
	{ "set 3,l",		BT,				 8, 0,  7 }, /* cbdd */
	{ "set 3,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbde */
	{ "set 3,a",		BT,				 8, 0,  7 }, /* cbdf */

	{ "set 4,b",		BT,				 8, 0,  7 }, /* cbe0 */
	{ "set 4,c",		BT,				 8, 0,  7 }, /* cbe1 */
	{ "set 4,d",		BT,				 8, 0,  7 }, /* cbe2 */
	{ "set 4,e",		BT,				 8, 0,  7 }, /* cbe3 */
	{ "set 4,h",		BT,				 8, 0,  7 }, /* cbe4 */
	{ "set 4,l",		BT,				 8, 0,  7 }, /* cbe5 */
	{ "set 4,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbe6 */
	{ "set 4,a",		BT,				 8, 0,  7 }, /* cbe7 */

	{ "set 5,b",		BT,				 8, 0,  7 }, /* cbe8 */
	{ "set 5,c",		BT,				 8, 0,  7 }, /* cbe9 */
	{ "set 5,d",		BT,				 8, 0,  7 }, /* cbea */
	{ "set 5,e",		BT,				 8, 0,  7 }, /* cbeb */
	{ "set 5,h",		BT,				 8, 0,  7 }, /* cbec */
	{ "set 5,l",		BT,				 8, 0,  7 }, /* cbed */
	{ "set 5,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbee */
	{ "set 5,a",		BT,				 8, 0,  7 }, /* cbef */

	{ "set 6,b",		BT,				 8, 0,  7 }, /* cbf0 */
	{ "set 6,c",		BT,				 8, 0,  7 }, /* cbf1 */
	{ "set 6,d",		BT,				 8, 0,  7 }, /* cbf2 */
	{ "set 6,e",		BT,				 8, 0,  7 }, /* cbf3 */
	{ "set 6,h",		BT,				 8, 0,  7 }, /* cbf4 */
	{ "set 6,l",		BT,				 8, 0,  7 }, /* cbf5 */
	{ "set 6,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbf6 */
	{ "set 6,a",		BT,				 8, 0,  7 }, /* cbf7 */

	{ "set 7,b",		BT,				 8, 0,  7 }, /* cbf8 */
	{ "set 7,c",		BT,				 8, 0,  7 }, /* cbf9 */
	{ "set 7,d",		BT,				 8, 0,  7 }, /* cbfa */
	{ "set 7,e",		BT,				 8, 0,  7 }, /* cbfb */
	{ "set 7,h",		BT,				 8, 0,  7 }, /* cbfc */
	{ "set 7,l",		BT,				 8, 0,  7 }, /* cbfd */
	{ "set 7,(hl)",		BT|MR|MW,		15, 0, 13 }, /* cbfe */
	{ "set 7,a",		BT,				 8, 0,  7 }, /* cbff */
  },
  {							/* dd fd */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd00 fd00 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd01 fd01 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd02 fd02 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd03 fd03 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd04 fd04 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd05 fd05 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd06 fd06 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd07 fd07 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd08 fd08 */
	{ "add ix,bc",		WD,				15, 0, 10 }, /* dd09 fd09 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0a fd0a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0b fd0b */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0c fd0c */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0d fd0d */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0e fd0e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd0f fd0f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd10 fd10 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd11 fd11 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd12 fd12 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd13 fd13 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd14 fd14 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd15 fd15 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd16 fd16 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd17 fd17 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd18 fd18 */
	{ "add ix,de",		WD,				15, 0, 10 }, /* dd19 fd19 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1a fd1a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1b fd1b */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1c fd1c */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1d fd1d */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1e fd1e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd1f fd1f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd20 fd20 */
	{ "ld ix,%s",		WC|WD,			14, 0, 12 }, /* dd21 fd21 */
	{ "ld (%s),ix",		ADDR|WD|MW,		20, 0, 19 }, /* dd22 fd22 */
	{ "inc ix",			WD,				10, 0,  7 }, /* dd23 fd23 */
	{ "inc ixh",		UNDOC|BT,		 8 }, /* dd24 fd24 */
	{ "dec ixh",		UNDOC|BT,		 8 }, /* dd25 fd25 */
	{ "ld ixh,%s",		UNDOC|BC|BT,	11 }, /* dd26 fd26 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd27 fd27 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd28 fd28 */
	{ "add ix,ix",		WD,				15, 0, 10 }, /* dd29 fd29 */
	{ "ld ix,(%s)",		ADDR|WD|MR,		20, 0, 18 }, /* dd2a fd2a */
	{ "dec ix",			WD,				10, 0,  7 }, /* dd2b fd2b */
	{ "inc ixl",		UNDOC|BT,		 8 }, /* dd2c fd2c */
	{ "dec ixl",		UNDOC|BT,		 8 }, /* dd2d fd2d */
	{ "ld ixl,%s",		UNDOC|BC|BT,	11 }, /* dd2e fd2e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd2f fd2f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd30 fd30 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd31 fd31 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd32 fd32 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd33 fd33 */
	{ "inc (ix%s)",		IND|BT|MR|MW,	23, 0, 18 }, /* dd34 fd34 */
	{ "dec (ix%s)",		IND|BT|MR|MW,	23, 0, 18 }, /* dd35 fd35 */
	{ "ld (ix%s),%s",	IND_1|BT|MW,	19, 0, 15 }, /* dd36 fd36 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd37 fd37 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd38 fd38 */
	{ "add ix,sp",		WD,				15, 0, 10 }, /* dd39 fd39 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3a fd3a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3b fd3b */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3c fd3c */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3d fd3d */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3e fd3e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd3f fd3f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd40 fd40 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd41 fd41 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd42 fd42 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd43 fd43 */
	{ "ld b,ixh",		UNDOC|BT,		 8 }, /* dd44 fd44 */
	{ "ld b,ixl",		UNDOC|BT,		 8 }, /* dd45 fd45 */
	{ "ld b,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd46 fd46 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd47 fd47 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd48 fd48 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd49 fd49 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd4a fd4a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd4b fd4b */
	{ "ld c,ixh",		UNDOC|BT,		 8 }, /* dd4c fd4c */
	{ "ld c,ixl",		UNDOC|BT,		 8 }, /* dd4d fd4d */
	{ "ld c,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd4e fd4e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd4f fd4f */
	
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd50 fd50 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd51 fd51 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd52 fd52 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd53 fd53 */
	{ "ld d,ixh",		UNDOC|BT,		 8 }, /* dd54 fd54 */
	{ "ld d,ixl",		UNDOC|BT,		 8 }, /* dd55 fd55 */
	{ "ld d,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd56 fd56 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd57 fd57 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd58 fd58 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd59 fd59 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd5a fd5a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd5b fd5b */
	{ "ld e,ixh",		UNDOC|BT,		 8 }, /* dd5c fd5c */
	{ "ld e,ixl",		UNDOC|BT,		 8 }, /* dd5d fd5d */
	{ "ld e,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd5e fd5e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd5f fd5f */
	
	{ "ld ixh,b",		UNDOC|BT,		 8 }, /* dd60 fd60 */
	{ "ld ixh,c",		UNDOC|BT,		 8 }, /* dd61 fd61 */
	{ "ld ixh,d",		UNDOC|BT,		 8 }, /* dd62 fd62 */
	{ "ld ixh,e",		UNDOC|BT,		 8 }, /* dd63 fd63 */
	{ "ld ixh,ixh",		UNDOC|BT,		 8 }, /* dd64 fd64 */
	{ "ld ixh,ixl",		UNDOC|BT,		 8 }, /* dd65 fd65 */
	{ "ld h,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd66 fd66 */
	{ "ld ixh,a",		UNDOC|BT,		 8 }, /* dd67 fd67 */

	{ "ld ixl,b",		UNDOC|BT,		 8 }, /* dd68 fd68 */
	{ "ld ixl,c",		UNDOC|BT,		 8 }, /* dd69 fd69 */
	{ "ld ixl,d",		UNDOC|BT,		 8 }, /* dd6a fd6a */
	{ "ld ixl,e",		UNDOC|BT,		 8 }, /* dd6b fd6b */
	{ "ld ixl,ixh",		UNDOC|BT,		 8 }, /* dd6c fd6c */
	{ "ld ixl,ixl",		UNDOC|BT,		 8 }, /* dd6d fd6d */
	{ "ld l,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd6e fd6e */
	{ "ld ixl,a",		UNDOC|BT,		 8 }, /* dd6f fd6f */
	
	{ "ld (ix%s),b",	IND|BT|MW,		19, 0, 15 }, /* dd70 fd70 */
	{ "ld (ix%s),c",	IND|BT|MW,		19, 0, 15 }, /* dd71 fd71 */
	{ "ld (ix%s),d",	IND|BT|MW,		19, 0, 15 }, /* dd72 fd72 */
	{ "ld (ix%s),e",	IND|BT|MW,		19, 0, 15 }, /* dd73 fd73 */
	{ "ld (ix%s),h",	IND|BT|MW,		19, 0, 15 }, /* dd74 fd74 */
	{ "ld (ix%s),l",	IND|BT|MW,		19, 0, 15 }, /* dd75 fd75 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd76 fd76 */
	{ "ld (ix%s),a",	IND|BT|MW,		19, 0, 15 }, /* dd77 fd77 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd78 fd78 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd79 fd79 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd7a fd7a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd7b fd7b */
	{ "ld a,ixh",		UNDOC|BT,		 8 }, /* dd7c fd7c */
	{ "ld a,ixl",		UNDOC|BT,		 8 }, /* dd7d fd7d */
	{ "ld a,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd7e fd7e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd7f fd7f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd80 fd80 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd81 fd81 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd82 fd82 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd83 fd83 */
	{ "add a,ixh",		UNDOC|BT,		 8 }, /* dd84 fd84 */
	{ "add a,ixl",		UNDOC|BT,		 8 }, /* dd85 fd85 */
	{ "add a,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd86 fd86 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd87 fd87 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd88 fd88 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd89 fd89 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd8a fd8a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd8b fd8b */
	{ "adc a,ixh",		UNDOC|BT,		 8 }, /* dd8c fd8c */
	{ "adc a,ixl",		UNDOC|BT,		 8 }, /* dd8d fd8d */
	{ "adc a,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd8e fd8e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd8f fd8f */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd90 fd90 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd91 fd91 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd92 fd92 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd93 fd93 */
	{ "sub ixh",		UNDOC|BT,		 8 }, /* dd94 fd94 */
	{ "sub ixl",		UNDOC|BT,		 8 }, /* dd95 fd95 */
	{ "sub (ix%s)",		IND|BT|MR,		19, 0, 14 }, /* dd96 fd96 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd97 fd97 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd98 fd98 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd99 fd99 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd9a fd9a */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd9b fd9b */
	{ "sbc ixh",		UNDOC|BT,		 8 }, /* dd9c fd9c */
	{ "sbx ixl",		UNDOC|BT,		 8 }, /* dd9d fd9d */
	{ "sbc a,(ix%s)",	IND|BT|MR,		19, 0, 14 }, /* dd9e fd9e */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dd9f fd9f */
	
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda0 fda0 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda1 fda1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda2 fda2 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda3 fda3 */
	{ "and ixh",		UNDOC|BT,		 8 }, /* dda4 fda4 */
	{ "and ixl",		UNDOC|BT,		 8 }, /* dda5 fda5 */
	{ "and (ix%s)",		IND|BT|MR,		19, 0, 14 }, /* dda6 fda6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda7 fda7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda8 fda8 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dda9 fda9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddaa fdaa */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddab fdab */
	{ "xor ixh",		UNDOC|BT,		 8 }, /* ddac fdac */
	{ "xor ixl",		UNDOC|BT,		 8 }, /* ddad fdad */
	{ "xor (ix%s)",		IND|BT|MR,		19, 0, 14 }, /* ddae fdae */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddaf fdaf */
	
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb0 fdb0 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb1 fdb1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb2 fdb2 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb3 fdb3 */
	{ "or ixh",			UNDOC|BT,		 8 }, /* ddb4 fdb4 */
	{ "or ixl",			UNDOC|BT,		 8 }, /* ddb5 fdb5 */
	{ "or (ix%s)",		IND|BT|MR,		19, 0, 14 }, /* ddb6 fdb6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb7 fdb7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb8 fdb8 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddb9 fdb9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddba fdba */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddbb fdbb */
	{ "cp ixh",			UNDOC|BT,		 8 }, /* ddbc fdbc */
	{ "cp ixl",			UNDOC|BT,		 8 }, /* ddbd fdbd */
	{ "cp (ix%s)",		IND|BT|MR,		19, 0, 14 }, /* ddbe fdbe */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddbf fdbf */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc0 fdc0 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc1 fdc1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc2 fdc2 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc3 fdc3 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc4 fdc4 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc5 fdc5 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc6 fdc6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc7 fdc7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc8 fdc8 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddc9 fdc9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddca fdca */
	{ "dd cb",			3,				 0 }, /* ddcb fdcb */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddcc fdcc */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddcd fdcd */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddce fdce */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddcf fdcf */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd0 fdd0 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd1 fdd1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd2 fdd2 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd3 fdd3 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd4 fdd4 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd5 fdd5 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd6 fdd6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd7 fdd7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd8 fdd8 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddd9 fdd9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddda fdda */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dddb fddb */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dddc fddc */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dddd fddd */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddde fdde */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dddf fddf */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde0 fde0 */
	{ "pop ix",			WD|MR,			14, 0, 12 }, /* dde1 fde1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde2 fde2 */
	{ "ex (sp),ix",		WD,				23, 0, 19 }, /* dde3 fde3 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde4 fde4 */
	{ "push ix",		WD|MW,			15, 0, 14 }, /* dde5 fde5 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde6 fde6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde7 fde7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* dde8 fde8 */
	{ "jp (ix)",		0 | UNBR,		 8, 0,  6 }, /* dde9 fde9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddea fdea */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* eb */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddec fdec */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* dded fded */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddee fdee */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddef fdef */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf0 fdf0 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf1 fdf1 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf2 fdf2 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf3 fdf3 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf4 fdf4 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf5 fdf5 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf6 fdf6 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf7 fdf7 */

	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddf8 fdf8 */
	{ "ld sp,ix",		WD,				10, 0,  7 }, /* ddf9 fdf9 */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddfa fdfa */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddfb fdfb */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddfc fdfc */
	{ pfix,				UNDOC | NOEFF, 	 4 }, /* ddfd fdfd */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddfe fdfe */
	{ pfix,				UNDOC | NOEFF,	 4 }, /* ddff fdff */
  },
  {						/* ed */
	{ ednop,			UNDOC,			 8 }, /* ed00 */
	{ ednop,			UNDOC,			 8 }, /* ed01 */
	{ ednop,			UNDOC,			 8 }, /* ed02 */
	{ ednop,			UNDOC,			 8 }, /* ed03 */
	{ ednop,			UNDOC,			 8 }, /* ed04 */
	{ ednop,			UNDOC,			 8 }, /* ed05 */
	{ ednop,			UNDOC,			 8 }, /* ed06 */
	{ ednop,			UNDOC,			 8 }, /* ed07 */

	{ ednop,			UNDOC,			 8 }, /* ed08 */
	{ ednop,			UNDOC,			 8 }, /* ed09 */
	{ ednop,			UNDOC,			 8 }, /* ed0a */
	{ ednop,			UNDOC,			 8 }, /* ed0b */
	{ ednop,			UNDOC,			 8 }, /* ed0c */
	{ ednop,			UNDOC,			 8 }, /* ed0d */
	{ ednop,			UNDOC,			 8 }, /* ed0e */
	{ ednop,			UNDOC,			 8 }, /* ed0f */

	{ ednop,			UNDOC,			 8 }, /* ed10 */
	{ ednop,			UNDOC,			 8 }, /* ed11 */
	{ ednop,			UNDOC,			 8 }, /* ed12 */
	{ ednop,			UNDOC,			 8 }, /* ed13 */
	{ ednop,			UNDOC,			 8 }, /* ed14 */
	{ ednop,			UNDOC,			 8 }, /* ed15 */
	{ ednop,			UNDOC,			 8 }, /* ed16 */
	{ ednop,			UNDOC,			 8 }, /* ed17 */

	{ ednop,			UNDOC,			 8 }, /* ed18 */
	{ ednop,			UNDOC,			 8 }, /* ed19 */
	{ ednop,			UNDOC,			 8 }, /* ed1a */
	{ ednop,			UNDOC,			 8 }, /* ed1b */
	{ ednop,			UNDOC,			 8 }, /* ed1c */
	{ ednop,			UNDOC,			 8 }, /* ed1d */
	{ ednop,			UNDOC,			 8 }, /* ed1e */
	{ ednop,			UNDOC,			 8 }, /* ed1f */

	{ ednop,			UNDOC,			 8 }, /* ed20 */
	{ ednop,			UNDOC,			 8 }, /* ed21 */
	{ ednop,			UNDOC,			 8 }, /* ed22 */
	{ ednop,			UNDOC,			 8 }, /* ed23 */
	{ ednop,			UNDOC,			 8 }, /* ed24 */
	{ ednop,			UNDOC,			 8 }, /* ed25 */
	{ ednop,			UNDOC,			 8 }, /* ed26 */
	{ ednop,			UNDOC,			 8 }, /* ed27 */

	{ ednop,			UNDOC,			 8 }, /* ed28 */
	{ ednop,			UNDOC,			 8 }, /* ed29 */
	{ ednop,			UNDOC,			 8 }, /* ed2a */
	{ ednop,			UNDOC,			 8 }, /* ed2b */
	{ ednop,			UNDOC,			 8 }, /* ed2c */
	{ ednop,			UNDOC,			 8 }, /* ed2d */
	{ ednop,			UNDOC,			 8 }, /* ed2e */
	{ ednop,			UNDOC,			 8 }, /* ed2f */

	{ ednop,			UNDOC,			 8 }, /* ed30 */
	{ ednop,			UNDOC,			 8 }, /* ed31 */
	{ ednop,			UNDOC,			 8 }, /* ed32 */
	{ ednop,			UNDOC,			 8 }, /* ed33 */
	{ ednop,			UNDOC,			 8 }, /* ed34 */
	{ ednop,			UNDOC,			 8 }, /* ed35 */
	{ ednop,			UNDOC,			 8 }, /* ed36 */
	{ ednop,			UNDOC,			 8 }, /* ed37 */

	{ ednop,			UNDOC,			 8 }, /* ed38 */
	{ ednop,			UNDOC,			 8 }, /* ed39 */
	{ ednop,			UNDOC,			 8 }, /* ed3a */
	{ ednop,			UNDOC,			 8 }, /* ed3b */
	{ ednop,			UNDOC,			 8 }, /* ed3c */
	{ ednop,			UNDOC,			 8 }, /* ed3d */
	{ ednop,			UNDOC,			 8 }, /* ed3e */
	{ ednop,			UNDOC,			 8 }, /* ed3f */

	{ "in b,(c)",		IN,				12, 0,  9 }, /* ed40 */
	{ "out (c),b",		OT,				12, 0, 10 }, /* ed41 */
	{ "sbc hl,bc",		WD,				15, 0, 10 }, /* ed42 */
	{ "ld (%s),bc",		ADDR|WD|MW,		20, 0, 19 }, /* ed43 */
	{ "neg",			BT,				 8, 0,  6 }, /* ed44 */
	{ "retn",			WD | UNBR,		14, 0, 12 }, /* ed45 */
	{ "im 0",			0,				 8, 0,  6 }, /* ed46 */
	{ "ld i,a",			BT,				 9, 0,  6 }, /* ed47 */

	{ "in c,(c)",		IN,				12, 0,  9 }, /* ed48 */
	{ "out (c),c",		OT,				12, 0, 10 }, /* ed49 */
	{ "adc hl,bc",		WD,				15, 0, 10 }, /* ed4a */
	{ "ld bc,(%s)",		ADDR|WD|MW,		20, 0, 18 }, /* ed4b */
	{ "neg",			UNDOC|BT,		 8 }, /* ed4c */
	{ "reti",			WD | UNBR,		14, 0, 12 }, /* ed4d */
	{ "im 0",			UNDOC,			 8 }, /* ed4e */
	{ "ld r,a",			BT,				 9, 0,  6 }, /* ed4f */

	{ "in d,(c)",		IN,				12, 0,  9 }, /* ed50 */
	{ "out (c),d",		OT,				12, 0, 10 }, /* ed51 */
	{ "sbc hl,de",		WD,				15, 0, 10 }, /* ed52 */
	{ "ld (%s),de",		ADDR|WD|MW,		20, 0, 19 }, /* ed53 */
	{ "neg",			UNDOC|BT,		 8 }, /* ed54 */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed55 */
	{ "im 1",			0,				 8, 0,  6 }, /* ed56 */
	{ "ld a,i",			BT,				 9, 0,  6 }, /* ed57 */

	{ "in e,(c)",		IN,				12, 0,  9 }, /* ed58 */
	{ "out (c),e",		OT,				12, 0, 10 }, /* ed59 */
	{ "adc hl,de",		WD,				15, 0, 10 }, /* ed5a */
	{ "ld de,(%s)",		ADDR|WD|MR,		20, 0, 18 }, /* ed5b */
	{ "neg",			UNDOC|BT,		 8 }, /* ed5c */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed5d */
	{ "im 2",			0,				 8, 0,  6 }, /* ed5e */
	{ "ld a,r",			BT,				 9, 0,  6 }, /* ed5f */

	{ "in h,(c)",		IN,				12, 0,  9 }, /* ed60 */
	{ "out (c),h",		OT,				12, 0, 10 }, /* ed61 */
	{ "sbc hl,hl",		WD,				15, 0, 10 }, /* ed62 */
	{ "ld (%s),hl",		ADDR|WD|MW,		20, 0, 19 }, /* ed63 */
	{ "neg",			UNDOC|BT,		 8 }, /* ed64 */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed65 */
	{ "im 0",			UNDOC,			 8 }, /* ed66 */
	{ "rrd",			BT|MR|MW,		18, 0, 16 }, /* ed67 */

	{ "in l,(c)",		IN,				12, 0,  9 }, /* ed68 */
	{ "out (c),l",		OT,				12, 0, 10 }, /* ed69 */
	{ "adc hl,hl",		WD,				15, 0, 10 }, /* ed6a */
	{ "ld hl,(%s)",		ADDR|WD|MR,		20, 0, 18 }, /* ed6b */
	{ "neg",			UNDOC|BT,		 8 }, /* ed6c */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed6d */
	{ "im 0",			UNDOC,			 8 }, /* ed6e */
	{ "rld",			BT,				18, 0, 16 }, /* ed6f */

	{ "in (c)",			UNDOC|IN,		12 }, /* ed70 */
	{ "out (c),0",		UNDOC|OT,		12 }, /* ed71 */
	{ "sbc hl,sp",		WD,				15, 0, 10 }, /* ed72 */
	{ "ld (%s),sp",		ADDR|WD|MW,		20, 0, 19 }, /* ed73 */
	{ "neg",			UNDOC|BT,		 8 }, /* ed74 */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed75 */
	{ "im 1",			UNDOC,			 8 }, /* ed76 */
	{ ednop,			UNDOC,			 8 }, /* ed77 */

	{ "in a,(c)",		IN,				12, 0,  9 }, /* ed78 */
	{ "out (c),a",		OT,				12, 0, 10 }, /* ed79 */
	{ "adc hl,sp",		WD,				15, 0, 10 }, /* ed7a */
	{ "ld sp,(%s)",		ADDR|WD|MR,		20, 0, 18 }, /* ed7b */
	{ "neg",			UNDOC|BT,		 8 }, /* ed7c */
	{ "retn",			UNDOC|WD|UNBR,	14 }, /* ed7d */
	{ "im 2",			UNDOC,			 8 }, /* ed7e */
	{ ednop,			UNDOC,			 8 }, /* ed7f */

	{ ednop,			UNDOC,			 8 }, /* ed80 */
	{ ednop,			UNDOC,			 8 }, /* ed81 */
	{ ednop,			UNDOC,			 8 }, /* ed82 */
	{ ednop,			UNDOC,			 8 }, /* ed83 */
	{ ednop,			UNDOC,			 8 }, /* ed84 */
	{ ednop,			UNDOC,			 8 }, /* ed85 */
	{ ednop,			UNDOC,			 8 }, /* ed86 */
	{ ednop,			UNDOC,			 8 }, /* ed87 */

	{ ednop,			UNDOC,			 8 }, /* ed88 */
	{ ednop,			UNDOC,			 8 }, /* ed89 */
	{ ednop,			UNDOC,			 8 }, /* ed8a */
	{ ednop,			UNDOC,			 8 }, /* ed8b */
	{ ednop,			UNDOC,			 8 }, /* ed8c */
	{ ednop,			UNDOC,			 8 }, /* ed8d */
	{ ednop,			UNDOC,			 8 }, /* ed8e */
	{ ednop,			UNDOC,			 8 }, /* ed8f */

	{ ednop,			UNDOC,			 8 }, /* ed90 */
	{ ednop,			UNDOC,			 8 }, /* ed91 */
	{ ednop,			UNDOC,			 8 }, /* ed92 */
	{ ednop,			UNDOC,			 8 }, /* ed93 */
	{ ednop,			UNDOC,			 8 }, /* ed94 */
	{ ednop,			UNDOC,			 8 }, /* ed95 */
	{ ednop,			UNDOC,			 8 }, /* ed96 */
	{ ednop,			UNDOC,			 8 }, /* ed97 */

	{ ednop,			UNDOC,			 8 }, /* ed98 */
	{ ednop,			UNDOC,			 8 }, /* ed99 */
	{ ednop,			UNDOC,			 8 }, /* ed9a */
	{ ednop,			UNDOC,			 8 }, /* ed9b */
	{ ednop,			UNDOC,			 8 }, /* ed9c */
	{ ednop,			UNDOC,			 8 }, /* ed9d */
	{ ednop,			UNDOC,			 8 }, /* ed9e */
	{ ednop,			UNDOC,			 8 }, /* ed9f */

	{ "ldi",			BT|MR|MW,		16, 0, 12 }, /* eda0 */
	{ "cpi",			BT|MR,			16, 0, 12 }, /* eda1 */
	{ "ini",			BT|MW|IN,		16, 0, 12 }, /* eda2 */
	{ "outi",			BT|MR|OT,		16, 0, 12 }, /* eda3 */
	{ ednop,			UNDOC,			 8 }, /* eda4 */
	{ ednop,			UNDOC,			 8 }, /* eda5 */
	{ ednop,			UNDOC,			 8 }, /* eda6 */
	{ ednop,			UNDOC,			 8 }, /* eda7 */

	{ "ldd",			BT|MR|MW,		16, 0, 12 }, /* eda8 */
	{ "cpd",			BT|MR,			16, 0, 12 }, /* eda9 */
	{ "ind",			BT|MW|IN,		16, 0, 12 }, /* edaa */
	{ "outd",			BT|MR|OT,		16, 0, 12 }, /* edab */
	{ ednop,			UNDOC,			 8 }, /* edac */
	{ ednop,			UNDOC,			 8 }, /* edad */
	{ ednop,			UNDOC,			 8 }, /* edae */
	{ ednop,			UNDOC,			 8 }, /* edaf */

	{ "ldir",			BT|MR|MW,VT(16, 21), 0, VT(14, 16) }, /* edb0 */
	{ "cpir",			BT|MR,	 VT(16, 21), 0, VT(14, 16) }, /* edb1 */
	{ "inir",			BT|MR|IN,VT(16, 21), 0, VT(12, 14) }, /* edb2 */
	{ "otir",			BT|MR|OT,VT(16, 21), 0, VT(12, 14) }, /* edb3 */
	{ ednop,			UNDOC,			 8 }, /* edb4 */
	{ ednop,			UNDOC,			 8 }, /* edb5 */
	{ ednop,			UNDOC,			 8 }, /* edb6 */
	{ ednop,			UNDOC,			 8 }, /* edb7 */

	{ "lddr",			BT|MR|MW,VT(16, 21), 0, VT(14, 16) }, /* edb8 */
	{ "cpdr",			BT|MR,	 VT(16, 21), 0, VT(14, 16) }, /* edb9 */
	{ "indr",			BT|MW|IN,VT(16, 21), 0, VT(12, 14) }, /* edba */
	{ "otdr",			BT|MR|OT,VT(16, 21), 0, VT(12, 14) }, /* edbb */
	{ ednop,			UNDOC,			 8 }, /* edbc */
	{ ednop,			UNDOC,			 8 }, /* edbd */
	{ ednop,			UNDOC,			 8 }, /* edbe */
	{ ednop,			UNDOC,			 8 }, /* edbf */

	{ ednop,			UNDOC,			 8 }, /* edc0 */
	{ ednop,			UNDOC,			 8 }, /* edc1 */
	{ ednop,			UNDOC,			 8 }, /* edc2 */
	{ ednop,			UNDOC,			 8 }, /* edc3 */
	{ ednop,			UNDOC,			 8 }, /* edc4 */
	{ ednop,			UNDOC,			 8 }, /* edc5 */
	{ ednop,			UNDOC,			 8 }, /* edc6 */
	{ ednop,			UNDOC,			 8 }, /* edc7 */

	{ ednop,			UNDOC,			 8 }, /* edc8 */
	{ ednop,			UNDOC,			 8 }, /* edc9 */
	{ ednop,			UNDOC,			 8 }, /* edca */
	{ ednop,			UNDOC,			 8 }, /* edcb */
	{ ednop,			UNDOC,			 8 }, /* edcc */
	{ ednop,			UNDOC,			 8 }, /* edcd */
	{ ednop,			UNDOC,			 8 }, /* edce */
	{ ednop,			UNDOC,			 8 }, /* edcf */

	{ ednop,			UNDOC,			 8 }, /* edd0 */
	{ ednop,			UNDOC,			 8 }, /* edd1 */
	{ ednop,			UNDOC,			 8 }, /* edd2 */
	{ ednop,			UNDOC,			 8 }, /* edd3 */
	{ ednop,			UNDOC,			 8 }, /* edd4 */
	{ ednop,			UNDOC,			 8 }, /* edd5 */
	{ ednop,			UNDOC,			 8 }, /* edd6 */
	{ ednop,			UNDOC,			 8 }, /* edd7 */

	{ ednop,			UNDOC,			 8 }, /* edd8 */
	{ ednop,			UNDOC,			 8 }, /* edd9 */
	{ ednop,			UNDOC,			 8 }, /* edda */
	{ ednop,			UNDOC,			 8 }, /* eddb */
	{ ednop,			UNDOC,			 8 }, /* eddc */
	{ ednop,			UNDOC,			 8 }, /* eddd */
	{ ednop,			UNDOC,			 8 }, /* edde */
	{ ednop,			UNDOC,			 8 }, /* eddf */

	{ ednop,			UNDOC,			 8 }, /* ede0 */
	{ ednop,			UNDOC,			 8 }, /* ede1 */
	{ ednop,			UNDOC,			 8 }, /* ede2 */
	{ ednop,			UNDOC,			 8 }, /* ede3 */
	{ ednop,			UNDOC,			 8 }, /* ede4 */
	{ ednop,			UNDOC,			 8 }, /* ede5 */
	{ ednop,			UNDOC,			 8 }, /* ede6 */
	{ ednop,			UNDOC,			 8 }, /* ede7 */

	{ ednop,			UNDOC,			 8 }, /* ede8 */
	{ ednop,			UNDOC,			 8 }, /* ede9 */
	{ ednop,			UNDOC,			 8 }, /* edea */
	{ ednop,			UNDOC,			 8 }, /* edeb */
	{ ednop,			UNDOC,			 8 }, /* edec */
	{ ednop,			UNDOC,			 8 }, /* eded */
	{ ednop,			UNDOC,			 8 }, /* edee */
	{ ednop,			UNDOC,			 8 }, /* edef */

	{ ednop,			UNDOC,			 8 }, /* edf0 */
	{ ednop,			UNDOC,			 8 }, /* edf1 */
	{ ednop,			UNDOC,			 8 }, /* edf2 */
	{ ednop,			UNDOC,			 8 }, /* edf3 */
	{ ednop,			UNDOC,			 8 }, /* edf4 */
	{ ednop,			UNDOC,			 8 }, /* edf5 */
	{ ednop,			UNDOC,			 8 }, /* edf6 */
	{ ednop,			UNDOC,			 8 }, /* edf7 */

	{ ednop,			UNDOC,			 8 }, /* edf8 */
	{ ednop,			UNDOC,			 8 }, /* edf9 */
	{ ednop,			UNDOC,			 8 }, /* edfa */
	{ ednop,			UNDOC,			 8 }, /* edfb */
	{ ednop,			UNDOC,			 8 }, /* edfc */
	{ ednop,			UNDOC,			 8 }, /* edfd */
	{ ednop,			UNDOC,			 8 }, /* edfe */
	{ ednop,			UNDOC,			 8 }, /* edff */
  }
};

struct OpcodePatch {
	int		op;
	Opcode	code;
};

// TODO: reminder: where do these come from?
// Answer: they're 8085 undocumented instructions: http://electronicerror.blogspot.ca/2007/08/undocumented-flags-and-instructions.html

#if 0
static OpcodePatch change8080[] = {
	{ 0x08, { "sub hl,bc",		WD,				15 } },
	{ 0x10, { "asr hl", 		WD,				 8 } },
	{ 0x18, { "lsl de", 		WD,				 8 } },
	{ 0x20, { undefined, 		0,				 0 } },	// 8085 rim
	{ 0x28, { "ld de,hl+%s",	BC|WD,			13 } },
	{ 0x30, { undefined, 		0,				 0 } },	// 8080 sim
	{ 0x38, { "ld de,sp+%s",	BC|WD,			13 } },
	{ 0xcb, { "rst v,40h",		RST,			11 } },
	{ 0xd9, { "ld (de),hl",		WD|MW,			17 } },
	{ 0xed, { "ld hl,(de)",		WD|MR,			17 } },
	{ 0xdd, { "jp nx5,%s",		ADDR | UNBR,	10 } },
	{ 0xfd, { "jp x5,%s",		ADDR | UNBR,	10 } }
};
#endif

static OpcodePatch change180[] = {
	{ 0x00, { "in0 b,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x08, { "in0 c,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x10, { "in0 d,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x18, { "in0 e,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x20, { "in0 h,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x28, { "in0 l,(%s)",		PORT | IN,		0, 0, 12 } },
	{ 0x30, { "in0 (%s)", 		PORT | IN,		0, 0, 12 } },
	{ 0x38, { "in0 a,(%s)",		PORT | IN,		0, 0, 12 } },

	{ 0x01, { "out0 (%s),b",	PORT | OT,		0, 0, 13 } },
	{ 0x09, { "out0 (%s),c",	PORT | OT,		0, 0, 13 } },
	{ 0x11, { "out0 (%s),d",	PORT | OT,		0, 0, 13 } },
	{ 0x19, { "out0 (%s),e",	PORT | OT,		0, 0, 13 } },
	{ 0x21, { "out0 (%s),h",	PORT | OT,		0, 0, 13 } },
	{ 0x29, { "out0 (%s),l",	PORT | OT,		0, 0, 13 } },
	{ 0x39, { "out0 (%s),a",	PORT | OT,		0, 0, 13 } },

	{ 0x04, { "tst b",			BT,				0, 0,  7 } },
	{ 0x0c, { "tst c",			BT,				0, 0,  7 } },
	{ 0x14, { "tst d",			BT,				0, 0,  7 } },
	{ 0x1c, { "tst e",			BT,				0, 0,  7 } },
	{ 0x24, { "tst h",			BT,				0, 0,  7 } },
	{ 0x2c, { "tst l",			BT,				0, 0,  7 } },
	{ 0x34, { "tst (hl)",		BT | MR,		0, 0, 10 } },
	{ 0x3c, { "tst a",			BT,				0, 0,  7 } },
	{ 0x64, { "tst %s",			BC | BT,		0, 0,  9 } },

	{ 0x4c, { "mlt bc",			BT,				0, 0, 17 } },
	{ 0x5c, { "mlt de",			BT,				0, 0, 17 } },
	{ 0x6c, { "mlt hl",			BT,				0, 0, 17 } },

	{ 0x74, { "tstio (%s)",		PORT | IN,		0, 0, 12 } },
	
	{ 0x76, { "slp",			0,				0, 0,  8 } },

	{ 0x83, { "otim",			BT | MR | OT,	0, 0, 14 } },
	{ 0x8b, { "otdm",			BT | MR | OT,	0, 0, 14 } },
	{ 0x93, { "otimr",			BT | MR | OT,	0, 0, VT(14, 16) } },
	{ 0x9b, { "otdmr",			BT | MR | OT,	0, 0, VT(14, 16) } },
};

Zi80dis::Zi80dis()
{
	m_processor = procZ80;
	m_assemblerable = true;
	m_dollarhex = false;
	m_undoc = false;
}

void Zi80dis::SetProcessor(enum Zi80dis::Processor proc)
{
	m_processor = proc;
}

void Zi80dis::SetAssemblerable(bool assemblerable)
{
	m_assemblerable = assemblerable;
}

void Zi80dis::SetDollarHex(bool dollarhex)
{
	m_dollarhex = dollarhex;
	if (m_dollarhex)
		m_assemblerable = false;
}

void Zi80dis::SetUndocumented(bool undoc)
{
	m_undoc = undoc;
}

bool Zi80dis::IsUndefined()
{
	return m_minT == 0;
}

#define SIGN_EXTEND(byte)	(((byte) & 0x80) ? -(((byte) ^ 255) + 1) : (byte))

void Zi80dis::Disassemble(const unsigned char *mem, int pc, bool memPointsToInstruction)
{
	m_minT = 0;
	m_maxT = 0;
	m_ocf = 0;
	m_min8080T = 0;
	m_max8080T = 0;
	m_min180T = 0;
	m_max180T = 0;
	m_length = 0;

#define FETCH_TO(var) var = memPointsToInstruction	\
		? mem[m_length] 							\
		: mem[(pc + m_length) & 0xffff];			\
	m_length++

	int FETCH_TO(op);
	int op0 = op;
	const Opcode *code = &z_major[op];
	int args;

	if (code->tstates8080 > 256)
	{
		m_min8080T += code->tstates8080 >> 8;
		m_max8080T += code->tstates8080 & 0xff;
	}
	else
	{
		m_min8080T += code->tstates8080;
		m_max8080T = m_min8080T;
	}

	m_ocf++;

	if (m_processor == proc8080)
	{
#if 0
		// Check if there are replacements for 8080 instructions.

		for (int i = 0; i < sizeof(change8080) / sizeof(change8080[0]); i++)
		{
			if (op == change8080[i].op)
			{
				code = &change8080[i].code;
				break;
			}
		}
#endif
		args = code->args;
	}
	else if (!z_major[op].name)
	{
		if (op == 0xED && m_processor == procZ180)
		{
			FETCH_TO(op);

			code = &z_minor[2][op];

			// Check if there are replacements for Z-180 instructions.
			for (int i = 0; i < int(sizeof(change180) / sizeof(change180[0])); i++)
			{
				if (op == change180[i].op)
				{
					code = &change180[i].code;
					break;
				}
			}
			args = code->args;
		}
		else
		{
			args = z_major[op0].args;
			FETCH_TO(op);
			// Check DD/FD prefix specially for no effect.
			if (args == 1 && (z_minor[args][op].args & NOEFF))
			{
				m_length--;
				code = &z_minor[args][op];
			}
			else
			{
				m_ocf++; // book says at most 2 op code fetches even for DDCB (TODO: really?)
				while (!z_minor[args][op].name)
				{
					args = z_minor[args][op].args;
					FETCH_TO(op);
				}
				code = &z_minor[args][op];
			}
		}
	}

	m_neverNext = (code->args & UNBR) != 0;
	m_stableMask = (1 << m_length) - 1;	// opcodes are stable
	m_attrMask = 0;
	m_numArg = 0;

	if (!m_undoc && (code->args & UNDOC))
	{
		strcpy(m_format, m_processor == procZ180 ? "illegal" : undefined);
		return;
	}

	if (code->args & MR) m_attrMask |= attrRead;
	if (code->args & MW) m_attrMask |= attrWrite;
	if (code->args & BT) m_attrMask |= attrByte;
	if (code->args & WD) m_attrMask |= attrWord;
	if (code->args & IN) m_attrMask |= attrIn;
	if (code->args & OT) m_attrMask |= attrOut;
	if (code->name[0] == 'j' && code->name[1] == 'p')
	{
		m_attrMask |= code->name[3] == '(' ? attrJumpReg : attrJump;
	}

	switch (code->args & ~FLGMSK)
	{
	case 3:	/* DD CB or FD CB instructions */
		m_stableMask &= ~4;	// Offset at byte 3 is not stable.
		m_stableMask |= 8;	// Opcode at byte 4 is stable.
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = SIGN_EXTEND(op);
		m_argType[0] = Zi80dis::argIndex;
		FETCH_TO(op);
		code = &z_minor[0][op];
		
		{
			// Find (hl) in normal instruction description.
			const char *left = strchr(code->name, '(');
			if (!left) {
				if (!m_undoc || m_processor == procZ180)
				{
					strcpy(m_format, undefined); // No (hl), don't want undocumented instructions.
				}
				else
				{
					left = strchr(code->name, '\0') - 1;
				}
			}

			if (left)
			{
					sprintf(m_format, "%.*s(ix%%s)", (int)(left - code->name), code->name);
					if (*left != '(' && strncmp(code->name, "bit", 3) != 0)
					{
						strcat(m_format, ",");
						strcat(m_format, left);
					}
					m_minT = 8;
					m_maxT = 8;
					m_min180T = 6;
					m_max180T = 6;
			}
		}
		break;
	case IND:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = SIGN_EXTEND(op);
		m_argType[0] = argIndex;
		strcpy(m_format, code->name);
		break;
	case IND_1:
		m_numArg = 2;
		FETCH_TO(op);
		m_arg[0] = SIGN_EXTEND(op);
		m_argType[0] = argIndex;
		FETCH_TO(op);
		m_arg[1] = op;
		m_argType[1] = argByte;
		strcpy(m_format, code->name);
		break;
	case PORT:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = op;
		m_argType[0] = argPort;
		strcpy(m_format, code->name);
		break;
	case 0:
		strcpy(m_format, code->name);
		break;
	case BC:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = op;
		m_argType[0] = argByte;
		strcpy(m_format, code->name);
		break;
	case WC:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = op;
		FETCH_TO(op);
		m_arg[0] |= op << 8;
		m_argType[0] = argWord;
		strcpy(m_format, code->name);
		break;
	case ADDR:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = op;
		FETCH_TO(op);
		m_arg[0] |= op << 8;
		m_argType[0] = argAddress;
		strcpy(m_format, code->name);
		break;
	case CALL:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = op;
		FETCH_TO(op);
		m_arg[0] |= op << 8;
		m_argType[0] = argCall;
		strcpy(m_format, code->name);
		m_attrMask |= attrCall;
		break;
	case RST:
		m_numArg = 1;
		m_arg[0] = op0 & 0x38;
		m_argType[0] = argRst;
		strcpy(m_format, code->name);
		m_attrMask |= attrRst;
		break;
	case JR:
		m_numArg = 1;
		FETCH_TO(op);
		m_arg[0] = (pc + m_length + SIGN_EXTEND(op)) & 0xffff;
		m_argType[0] = argRelative;
		strcpy(m_format, code->name);
		m_attrMask |= attrBranch;
		break;
	}

	// Translate ix to iy if necessary.
	if (op0 == 0xfd)
	{
		for (int i = 0; m_format[i]; i++)
		{
			if (m_format[i] == 'i' && m_format[i + 1] == 'x')
			{
				m_format[i + 1] = 'y';
			}
		}
	}

	if (code->tstates > 256)
	{
		m_minT += code->tstates >> 8;
		m_maxT += code->tstates & 0xff;
	}
	else
	{
		m_minT += code->tstates;
		m_maxT = m_minT;
	}

	if (code->tstates180 > 256)
	{
		m_min180T += code->tstates180 >> 8;
		m_max180T += code->tstates180 & 0xff;
	}
	else
	{
		m_min180T += code->tstates180;
		m_max180T = m_min180T;
	}
}

void Zi80dis::Format(char *output)
{
	char arg[2][16];

	for (int i = 0; i < m_numArg; i++)
	{
		switch (m_argType[i])
		{
		case argWord:
		case argAddress:
		case argRelative:
		case argCall:
		case argRst:
			if (m_assemblerable)
			{
				sprintf(arg[i], "0%04xh", m_arg[i]);
			}
			else if (m_dollarhex)
			{
				sprintf(arg[i], "$%04x", m_arg[i]);
			}
			else
			{
				sprintf(arg[i], "%04xh", m_arg[i]);
			}
			break;

		case argByte:
		case argPort:
			if (m_assemblerable)
			{
				sprintf(arg[i], "0%02xh", m_arg[i]);
			}
			else if (m_dollarhex)
			{
				sprintf(arg[i], "$%02x", m_arg[i]);
			}
			else
			{
				sprintf(arg[i], "%02xh", m_arg[i]);
			}
			break;

		case argIndex:
			sprintf(arg[i], m_dollarhex ? "%c$%02x" : "%c%02xh",
				(m_arg[i] & 0x80) ? '-' : '+',
				(m_arg[i] & 0x80) ? ((m_arg[i] & 255) ^ 255) + 1 : m_arg[i]);
			break;
		}
	}

	sprintf(output, m_format, arg[0], arg[1]);
}

extern "C" int zi_tstates(const unsigned char *inst, int proc,
	int *low, int *high, int *ocf, char *disasm)
{
	Zi80dis dis;

	dis.SetProcessor((Zi80dis::Processor)proc);
	dis.SetUndocumented(true);
	dis.SetDollarHex(true);
	dis.Disassemble(inst, 0, true);

	if (ocf) *ocf = dis.m_ocf;

	// proc corresponds to  Zi80dis::Processor
	switch (proc) {
	case Zi80dis::procZ80:
		if (low) *low = dis.m_minT;
		if (high) *high = dis.m_maxT;
		break;
	case Zi80dis::proc8080:
		if (low) *low = dis.m_min8080T;
		if (high) *high = dis.m_max8080T;
		break;
	case Zi80dis::procZ180:
		if (low) *low = dis.m_min180T;
		if (high) *high = dis.m_max180T;
		break;
	default:
		break;
	}

	if (disasm)
		dis.Format(disasm);

	return dis.m_length;
}
