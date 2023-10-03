%{
// GWP - keep track of version via hand-maintained date stamp.
#define VERSION "9feb2022"

/*
 *  zmac -- macro cross-assembler for the Zilog Z80 microprocessor
 *
 *  Bruce Norskog	4/78
 *
 *  Last modification  1-18-87 by cdk
 *  This assembler is modeled after the Intel 8080 macro cross-assembler
 *  for the Intel 8080 by Ken Borgendale.  The major features are:
 *	1.  Full macro capabilities
 *	2.  Conditional assembly
 *	3.  A very flexible set of listing options and pseudo-ops
 *	4.  Symbol table output
 *	5.  Error report
 *	6.  Elimination of sequential searching
 *	7.  Commenting of source
 *	8.  Facilities for system definition files
 *
 * Revision History:
 *
 * jrp	3-8-82	Converted to run on Vax, updated syntax to conform better
 *		to the Zilog standard.
 *
 * jrp	3-15-82	Added underscore as a character type in the lex table
 *		'numpart' (0x5F).
 *
 *		Changed maximum number of characters in a label to 15
 *		from 7. Note that 'putsymtab' uses this value inside
 *		of a quoted string, so we use 15.
 *
 * jrp	2-15-83	Fixed 'getlocal' to return better local labels. It used
 *		to crash after 6 invocations.
 *
 * jrp	6-7-83	Fixed bug in the ADD IX,... instruction.
 *
 * jrp	5-11-84	Added code to print unused labels out with the symbol table
 *		Also sped up the macro processor by using stdio.
 *
 * jrp 5-22-84	Added include files ala ormac
 *
 * jrp 8-27-84	Added PHASE/DEPHASE commands
 *
 * cdk 9-20-86	Converted to run on a Pyramid.  This meant changing yylval
 *		to be a %union, and then putting in the appropriate
 *		typecasts where ints are pointers are used interchangeably.
 *		The current version still probably won't run on machines where
 *		sizeof(int) != sizeof(char *).
 *		Also changed emit() to use varargs, and got rid of the
 *		old style = in front of yacc action code.
 *			-Colin Kelley  vu-vlsi!colin
 *
 * cdk 10-2-86	Added some more typecasts to keep lint a little happier.
 *		Removed several unused variables.  Changed most vars
 *		declared as char to int, since many of them were being
 *		compared with -1!  I still don't know what's going on with
 *		est[][] being malloc'd and free'd everywhere...it looks pretty
 *		fishy...
 *
 * cdk 1-18-87  Added MIO code to emulate 'mfile' using malloc()'d memory.
 *		This was needed to get the code to work when compiled under
 *		MSC 4.0 on a PC, and it's probably faster anyway.
 *
 * cdk 2-5-87	Added 'cmp' as a synonym for 'cp', 'jmp' as a synonym for
 *		'jp', and added tolerance of accumulator specification for arithmetic
 *		and logical instructions.  (For example, 'or a,12' is now accepted,
 *		same as 'or 12'.)
 *
 * gwp 12-29-08	Changes to allow compilation with modern C compiler and using bison
 *		as the .y to .c converter.  assert, tstate pseudo-ops.
 *		t(), tilo(), tihi() functions.  ==, <=, >=, !=, !, <, > operators.
 *		-c to turn cycle counts off in listing.  Usage, -h and version.
 *
 * gwp 9-26-10	Add ocf() and setocf to track and set op code fetch counts.
 *		Add sett as an alias for tstate
 *
 * gwp 12-30-11	Add undocumented instructions sl1, pfix, pfiy, in (c), out (c),0
 *		bit/set/res (ixy+d),reg and ld/inc/dec ixylh.
 *
 * gwp 2-8-12   Increase MAXIFS massively due to massive lowt macro
 *
 * gwp 2-11-12  Support 32 bit constants.  '%' alias for MOD.  Add defd, dword.
 *		lo(x) and hi(x) for easy low and high byte extraction.  Allow
 *		filenames longer than 15 characters.  All output to "zout" subdirectory
 *		of source file.
 *
 * gwp 2-15-12	Perform multiple passes while equates are changing.  Support
 *		.label for temporary label definitions and _label for file
 *		scoped labels.  Allow '.' within labels.  Assert listing bugfix.
 *
 * gwp 4-27-12	Implement $ prefixed hex constants and double-quoted strings.
 *
 * gwp 6-30-12	Minor changes to allow compilation with gcc.
 *
 * gwp 9-05-12	incbin added.
 *
 * gwp 11-24-12	Fix macro expansion bug when symbol larger than MAXSYMBOLSIZE
 *		due to file name prepending when symbol starts with '_'.
 *
 * gwp 12-04-12	Optional JR promotion and JP demotion errors.  Output a warning
 *		if no execute address given.  Output ".bds" file to enable easy
 *		simple source level debugging.
 *
 * gwp 4-14-13	Parens in expressions, else, .pseudo, full set of C operators
 *		with conventional precedence and various aliases and code
 *		changes to make source similar to zmac 1.3 on internet.
 *
 * gwp 5-5-13	.cmd,.cas,.lcas,.bin output.  dc (both MACRO-80 and EDAS types).
 *		lo, hi renamed to low, high and make unary operators.  Allow
 *		label::, placeholder public, extern declarations.  Bug fixes
 *		in defs, t, ocf, tihi, tilo in phase mode.  Initial support
 *		for -I include directories. 0x hex constants. --mras flag for
 *		limited MRAS compatibility (allows $ in labels, $? to start
 *		labels).
 *
 * gwp 4-6-13	--rel for .rel (Microsoft linker) output and extern, public,
 *		aseg, cseg, dseg in support (big emit + expression revamp).
 *		-I follows C preprocessor convention, output relative to
 *		current directory.  8080 mnemonics, .z80, .8080, -z, -8.
 *		Change .bin to .cim.  Warn on labels not in first column.
 *
 * gwp 8-11-13	Allow $ to start identifiers and do '$' dropping when macro
 *              parsed so we no longer need to drop '$' in identifiers. 
 *              Even $FCB allowed, with warning.  Add --zmac for backwards
 *		compatibility with zmac.  ` now used for joining in macros.
 *		Most reserved words can be used as labels or variables.
 *		Free-form title, name, comment, subttl parsing.  Allow #arg
 *		for macro arguments (in --mras mode).  Support <CR> delimited
 *		files.  Add .ams output.  Integrate documentation (--doc).
 *
 * gwp 3-12-14	Emit jr even if out of range.  z80.lib support.
 *		Warning and bug fixes from Mark Galanger.
 *		Macros can override built-ins and are no longer listed
 *		in symbol table.  A, B, C, D, E, H, L, M, PSW, SP are
 *		pre-defined values which can be used in data statements
 *		(db, dw, dd).  Reserved words can be equated but are only
 *		accessbile in data.  SET can be used in place of DEFL
 *		(MAC and MACRO-80 agree on this).  '=' can be used in place
 *		of EQU. 'maclib file' includes 'file.lib'.  Bug fix in "dw 0,$".
 *		Removed error flagging in expressions which could cause parse
 *		to fail from that point onwards.
 *		expression(ix) equivalent to (ix + expression).
 *		Macro expanded lines didn't go through the line analyzer.
 *		Empty macro arguments (e.g., mac 1,,2)
 *		Implemented rept, irp, irpc, exitm.  Add more detail on phase
 *		errors. '' is an empty string in db/ascii/etc, 0 otherwise.
 *		Almost any name can be used as a macro parameter name.
 *		Allow 'if' in first column.
 *		Fix .rel mode bug in dc, incbin.
 *		Emit .bds output for dc, incbin.
 *		Allow assembly to wrap past end of memory.
 *		"pragma bds" for raw output to .bds file.  Also output equates
 *		to .bds file.
 *		Macro bug fix from Sergey Erokhin.
 *
 * gwp 9-5-16	Allow ' on identifiers for manual prime register tracking.
 *		Improve MRAS support with *LIST, *INCLUDE and Pk=n parameters.
 *
 * gwp 20-9-16	Big MRAS compatibility fixes.  Can do MRAS operator precedence,
 *		proper .operator. tokenization and agressive macro parameter
 *		substituion.  Change Pk=n to -Pk=n.  Add ++, += and variants
 *		for more compact variable adjustment than defl.  First crack
 *		at .tap output for ZX Spectrum support.
 *
 * gwp 13-8-17	Add "import" for simple once-only inclusion of files.
 *		Track full path so relative includes work properly.
 *		Allow push af', pop af' for notational convenience.
 *		Add "bytes" as alias for "dc".  Fix --rel output bugs in
 *		low(), high(), div and mod.
 *
 * gwp 12-3-18	250 baud .cas output and .wav format.  Common blocks.
 *		--oo, --xo, --od to control output.  Delete output on fail.
 *
 * gwp 2-6-18	1000 baud .cas ouput and .mds (MAME/MESS debug script) output.
 *
 * gwp 28-7-18	Double free of output files bug fix from Pedro Gimeno.  Don't
 *		output SYSTEM files if no entry point thanks to Tim Halloran.
 *
 * gwp 6-10-18	Stop quoted inclued file crash on for OSX (and linux?). Thanks
 *		to Paulo Silva. Added "dm" as "ascii" alias for Blair Robins.
 *
 * gwp 5-12-18	Peter Wilson ran into quoted include crash.  Also suggested
 *		import be allowed in column 0 and noted that keywords like
 *		IF and LIST could not be labels even if given colons.
 *
 * gwp 6-12-18	hex output does proper EOF record if no entry address given.
 *		include and some other pseudo-opts can have a label.
 *
 * gwp 3-1-19	Improve jrpromote to handle some edges cases better where
 *		promotion isn't needed as long as shortest code is tried first.
 *
 * gwp 5-1-19	Add cycle count and output bytes to macro invocations.
 *		[ with listing bug fixes added slightly later ]
 *
 * gwp 19-1-19	DRI compatibility enhancements and other bug fixes from
 *		Douglas Miller. TWOCHAR constants ('XY') usable everywhere.
 *		dolopt moved into function to stop LEX stack crash.  Support
 *		labels on cseg/dseg.  Relax paren parsing for LXI instruction.
 *		Recognize TITLE in column 0.  Stop "Not relocatable error" on
 *		"ORG 0".  -i option stops all macro line listing.
 *		--dri mode:
 *		silences warnings about builtin redefinition (for Z80.LIB).
 *		Strips '$' from constants and symbols.
 *		Accepts '*' in column 0 as comment.
 *		All 8080 opcodes can be use in DB and expressions.
 *		Add recognition of DRI $-MACRO directives
 *
 * gwp 20-1-19	Z-80 mnemonics usable as values.  --nmnv turns that off.  def3
 *		Register aliases.  Multiple statements per line roughed in.
 *
 * gwp 20-4-20	Preserve case of symbols in symbol table.  Output hexadecimal
 *		in upper case and show decimal value of symbols.
 *
 * tjh 9-5-20	Add -Dsym to allow definition of symbols on the command line.
 *		ZMAC_ARGS environment variable added to command line.
 *
 * gwp 25-8-20	Fix crash in "out (c),0".  Make "in f,(c)" work.
 *
 * gwp 17-3-21	Stop line buffer overflow when too many "dc" or "incbin" lines
 *		appear contiguously.  --fcal option and Z-180 instructions.
 *
 * gwp 10-4-21	Put code and data indications in .bds output.
 *
 * gwp 9-2-22	Fix --z180 and improve usage message on unknown -- flags.
 */

#if defined(__GNUC__)
#pragma GCC diagnostic error "-Wreturn-type"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#ifdef WIN32
#include <windows.h>	// just for colouring the output
#include <direct.h>		// _mkdir
#ifdef _MSC_VER
#define strdup _strdup
#define unlink _unlink
#define strncasecmp _strnicmp
#endif
#endif

#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__)
#include <unistd.h>	// just for unlink
#endif

#include "zi80dis.h"

#ifdef vax11c
#define unlink(filename) delete(filename)
#endif

#include "mio.h"

/*
 * DEBUG turns on pass reporting.
 * DBUG enables -d to allow yacc/bison yydebug increment (but must be passed
 *	on the command line)
 * Macro debug and Token debug enables.
#define	DEBUG
#define	M_DEBUG	
#define	T_DEBUG
 */

#ifdef DBUG
#define YYDEBUG 1
#endif

#define ITEMTABLESIZE	100000
#define TEMPBUFSIZE	(1000+MAXSYMBOLSIZE)
#define LINEBUFFERSIZE	1000
#define EMITBUFFERSIZE	200
#define MAXSYMBOLSIZE	40
#define IFSTACKSIZE	20
// GWP - I use lots of if's with my lowt macro
#define MAXIFS		65536
#define TITLELEN	50
#define BINPERLINE	16
#define	PARMMAX		25
#define MAXEXP		25
#define SYMMAJIC	07203
#define	NEST_IN		32
#define MAXPASS		32
#define MAXINCPATH	32

int iflist();
int yylex();
int phaseaddr(int addr);
int nextchar();
int getcol();
int skipline(int ac);
int tokenofitem(int deftoken, int keyexclude, int keyinclude);
int getm();
int counterr();
int countwarn();
int convert(char *buf, char *bufend, int *overflow);

void yyerror(char *err)
{}		/* we will do our own error printing */

struct argparse {
	char *arg;	// output buffer for argument
	int argsize;	// size of output buffer
	int (*getch)(struct argparse *); // get next character
	int *peek;	// pointer single item pushback buffer
	int macarg;	// working on macro arguments

	char *user_ptr;	// state for getch
	int user_int;	// state for getch
	int user_peek;	// state for getch

	int didarg;	// internal parsing state
	int numarg;	// internal parsing state
};

int getarg(struct argparse *ap);

struct	item	{
	char	*i_string;
	int	i_value;
	int	i_token;
	int	i_uses;
	int	i_scope;
	int	i_chain;
	int	i_pass;
};

void itemcpy(struct item *dst, struct item *src);
int item_is_verb(struct item *i);
int item_value(struct item *i);
struct item *keyword(char *name);
int keyword_index(struct item *k);

#define SCOPE_NONE	(0)
#define SCOPE_PROGRAM	(1)
#define SCOPE_DATA	(2)
#define SCOPE_COMMON	(3)
#define SCOPE_PUBLIC	(4)
#define SCOPE_EXTERNAL	(8)
#define SCOPE_NORELOC	(16)
#define SCOPE_BUILTIN	(32)	/* abuse */
#define SCOPE_COMBLOCK	(64)
#define SCOPE_TWOCHAR	(128)
#define SCOPE_ALIAS	(256)
#define SCOPE_CMD_P	(512)
#define SCOPE_CMD_D	(1024)

#define SCOPE_SEGMASK	(3)
#define SCOPE_SEG(s)	((s) & SCOPE_SEGMASK)

struct expr {
	int e_value;
	int e_scope;
	int e_token;
	int e_parenthesized;
	struct item *e_item;
	struct expr *e_left;
	struct expr *e_right;
};

#define EXPR_SEG(e)	SCOPE_SEG(e->e_scope)

FILE	*fout,
	*fbuf,
	*fbds,
	*fcmd,
	*fcas,
	*flcas,
	*flnwcas,
	*ftcas,
	*fcim,
	*fams,
	*frel,
	*ftap,
	*fmds,
	*f1500wav,
	*f1000wav,
	*f500wav,
	*f250wav,
	*fin[NEST_IN],
	*now_file ;

char *output_dir = "zout";

struct OutputFile {
	char *suffix;
	char *mode;
	FILE **fpp;
	int system; // A cassette SYSTEM file
	int no_open;
	int wanted; // output file has been explicitly selected
	char *filename;
	int temp;
}
outf[] = {
	{ "lst",	"w",	&fout		},
	{ "hex", 	"w",	&fbuf		},
	{ "bds",	"w",	&fbds		},
	{ "cmd",	"wb",	&fcmd		},
	{ "1500.wav",	"wb",	&f1500wav, 1	}, // must be 1 before 1500.cas
	{ "1500.cas",	"w+b",	&fcas,	   1	},
	{ "1000.wav",	"wb",	&f1000wav, 1	}, // must be 1 before 1000.cas
	{ "1000.cas",	"w+b",	&flnwcas,  1	},
	{ "500.wav",	"wb",	&f500wav,  1	}, // must be 1 before 500.cas
	{ "500.cas",	"w+b",	&flcas,    1	},
	{ "250.wav",	"wb",	&f250wav	}, // must be 1 before 250.cas
	{ "250.cas",	"w+b",	&ftcas		},
	{ "cim",	"wb",	&fcim		},
	{ "ams",	"wb",	&fams		},
	{ "rel",	"wb",	&frel,	0, 1	},
	{ "tap",	"wb",	&ftap		},
	{ "mds",	"w",	&fmds		},
};
#define CNT_OUTF (sizeof outf / sizeof outf[0])

int getoptarg(int argc, char *argv[], int i);
void stop_all_outf();
void clean_outf();
void clean_outf_temp();
void suffix_list(char *sfx_lst, int no_open);

int	pass2;	/*set when pass one completed*/
int	outpass; 	// set when we decide to stop doing passes */
int	passfail;	// set when an error means passes should not continue
int	passretry;	// set when an inconsistency will require another pass
int	dollarsign ;	/* location counter */
int	olddollar ;	/* kept to put out binary */
int	oldothdollar;	// output address of next .cmd/.cas/.lcas block
int	emit_addr;	// where code and data are being placed in memory
int	tstates;	// cumulative T-states
int	ocf;		// cumulative op code fetches
int	llseq;		// local label sequence number
int	mras;		// MRAS semi-compatibility mode
int	trueval = 1;	// Value returned for boolean true
int	zcompat;	// Original zmac compatibility mode
char	modstr[8];	// Replacement string for '?' in labels when MRAS compatible
int	relopt;		// Only output .rel files and length of external symbols
int	driopt;		// DRI assemblers compatibility
int	macopt;		// DRI assemblers $-MACRO et al.
int	comnt;		// DRI assemblers '*' comment line
int	nmnvopt;	// Mnemonics are not treated as predefined values.
char	progname[8];	// Program name for .rel output
int	note_depend;	// Print names of files included
int	separator;	// Character that separates multiple statements.
int	firstcol;
int	logcol;
int	coloncnt;
int first_always_label;
int	full_exprs;	// expression parsing mode allowing almost any identifier
struct item *label, pristine_label; // 
int	list_dollarsign;// flag used to change list output for some operations
int	list_addr;	// address to display for operation if !list_dollarsign

// Include file search path
char	*incpath[MAXINCPATH];
int	incpath_cnt;

/* program counter save for PHASE/DEPHASE */
int	phdollar, phbegin, phaseflag ;

char	*src_name[NEST_IN] ;
int	linein[NEST_IN] ;
int linepeek[NEST_IN];
int	now_in ;


// These first 5 errors are singled out in lsterr1() for reasons I don't
// quite understand.
#define bflag	0	/* balance error */
#define eflag	1	/* expression error */
#define fflag	2	/* syntax error */
#define iflag	3	/* bad digits */
#define mflag	4	/* multiply defined */

#define pflag	5	/* phase error */
#define uflag	6	/* undeclared used */
#define vflag	7	/* value out of range */
#define oflag	8	/* phase/dephase error */
#define aflag	9	/* assert failed */
#define jflag	10	/* JP could be JR */
#define rflag	11	/* expression not relocatable */
#define gflag	12	/* incorrect register */
#define zflag	13	/* invalid instruction */

#define FIRSTWARN	warn_hex
#define	warn_hex	14
#define warn_notimpl	15
#define warn_general	16

#define FLAGS	17	/* number of errors and warnings */

char	err[FLAGS];
int	keeperr[FLAGS];
char	errlet[FLAGS]="BEFIMPUVOAJRGZHNW";
char	*errname[FLAGS]={
	"Balance",
	"Expression",
	"Syntax",
	"Digit",
	"Mult. def.",
	"Phase",
	"Undeclared",
	"Value",
	"Phase/Dephase",
	"Assertion failure",
	"Use JR",
	"Not relocatable",
	"Register usage",
	"Invalid instruction",
	"$hex constant interpreted as symbol",
	"Not implemented",
	"General"
};
char	errdetail[FLAGS][TEMPBUFSIZE * 2];
char	detail[TEMPBUFSIZE * 2];


unsigned char inpbuf[LINEBUFFERSIZE];
unsigned char inpbuf_insert[LINEBUFFERSIZE];
unsigned char *inpptr;

char	linebuf[LINEBUFFERSIZE];
char	*lineptr;
char	*linemax = linebuf+LINEBUFFERSIZE;

char	outbin[BINPERLINE];
char	*outbinp = outbin;
char	*outbinm = outbin+BINPERLINE;

char	outoth[256];
int	outoth_cnt = 0;

unsigned char	emitbuf[EMITBUFFERSIZE];
unsigned char	*emitptr;

char	ifstack[IFSTACKSIZE];
char	*ifptr;
char	*ifstmax = ifstack+IFSTACKSIZE-1;


char	hexadec[] = "0123456789ABCDEF" ;


int	nitems;
int	nbytes;
int	invented;
int	npass;
int	njrpromo;
int	multiline;
int	prev_multiline;


char	tempbuf[TEMPBUFSIZE];
char	*tempmax = tempbuf+TEMPBUFSIZE-1;

char	arg_flag;
struct argparse arg_state;
void	arg_start();
void	arg_reset();
int	str_getch(struct argparse *ap);

int	mras_undecl_ok;
int	mras_param[10];
int	mras_param_set[10];

struct cl_symbol {
	char *name;
	struct cl_symbol *next;
} *cl_symbol_list;

char	inmlex;
char	mlex_list_on;
int	parm_number;
int	exp_number;
char	symlong[] = "Symbol/number too long";
int	raw;

int	disp;

int	param_parse;
#define PARAMTABSIZE (PARMMAX * 2)
struct item paramtab[PARAMTABSIZE];

#define FLOC	PARMMAX
#define TEMPNUM	PARMMAX+1
#define REPNUM	PARMMAX+2
#define MSTART	PARMMAX+3
#define MSTR	PARMMAX+4
#define MARGP	PARMMAX+5
#define MIF	PARMMAX+6

#define PAREXT	7

union exprec {
	char *param;
	int value;
	struct argparse *ap;
};
union exprec	*est;
union exprec	*est2;
union exprec	*expstack[MAXEXP];
int	expptr;

int	floc;
int	mfptr;
FILE	*mfile;

char	*writesyms;


char	*title;
char	titlespace[TITLELEN];
char	*timp;
char	*sourcef;
/* changed to cope with filenames longer than 14 chars -rjm 1998-12-15 */
char	src[1024];
char	bin[1024];
char	listf[1024];
char	oth[1024];

char	copt = 1,	/* cycle counts in listings by default */
	edef = 1,
	eopt = 1,
	fdef = 0,
	fopt = 0,
	gdef = 1,
	gopt = 1,
	iopt = 0 ,	/* list include files */
	jopt = 0,
	JPopt = 0,
	lstoff = 0,
	lston = 0,	/* flag to force listing on */
	mdef = 0,
	mopt = 0,
	nopt = 1 ,	/* line numbers on as default */
	popt = 1,	/* form feed as default page eject */
	sopt = 0,	/* turn on symbol table listing */
	topt = 1,	/* terse, only error count to terminal */
	printer_output = 0, // GWP - printer style output
	z80 = 1,
	saveopt;

char default_jopt, default_JPopt, default_z80 = 1;

char	xeq_flag = 0;
int	xeq;

time_t	now;
int	line;
int	page = 1;

struct stab {
	char	t_name[MAXSYMBOLSIZE+1];
	int	t_value;
	int	t_token;
};

// GWP - support for cycle count tracking (and opens door for efficient .cmd, etc. output)

unsigned char memory[1 << 16];
char memflag[1 << 16];
enum {
	MEM_DATA = 1,
	MEM_INST = 2,
	MEM_T_SET = 4
};
int tstatesum[1 << 16];
int ocfsum[1 << 16];

// GWP - expression handling extensions for .rel output.
void advance_segment(int step);
void expr_reloc_check(struct expr *ex);
void expr_number_check(struct expr *ex);
void expr_scope_same(struct expr *ex1, struct expr *ex2);
void expr_word_check(struct expr *ex);
int is_number(struct expr *ex);
int is_external(struct expr *ex);
struct expr *expr_num(int value);
struct expr *expr_alloc(void);
struct expr *expr_var(struct item *var);
struct expr *expr_mk(struct expr *left, int token, struct expr *right);
struct expr *expr_op(struct expr *left, int token, struct expr *right, int value);
struct expr *expr_op_sc(struct expr *left, int token, struct expr *right, int value);
void expr_free(struct expr *ex);
int can_extend_link(struct expr *ex);
void extend_link(struct expr *ex);
void putrelop(int op);
#define RELOP_BYTE	(1)
#define RELOP_WORD	(2)
#define RELOP_HIGH	(3)
#define RELOP_LOW	(4)
#define RELOP_NOT	(5)
#define RELOP_NEG	(6)
#define RELOP_SUB	(7)
#define RELOP_ADD	(8)
#define RELOP_MUL	(9)
#define RELOP_DIV	(10)
#define RELOP_MOD	(11)
struct item *item_lookup(char *name, struct item *table, int table_size);
struct item *item_substr_lookup(char *name, int token, struct item *table, int table_size);
struct item *locate(char *name);
// Data descriptions for emit()
#define E_CODE		(0)
#define E_DATA		(1)
#define E_CODE8		(2)
#define E_CODE16	(3)
int segment;
#define SEGCHAR(s) " '\"!"[s]
#define SEG_ABS		(0)
#define SEG_CODE	(1)
#define SEG_DATA	(2)
#define SEG_COMMON	(3)
int seg_pos[4]; // may eventually support SEG_COMMON
int seg_size[4];
int rel_main;
int segchange;
struct item *cur_common;
void putout(int value);
int outrec;
int outlen;
unsigned char outbuf[1024 * 1024];
void bookmark();
void listfrombookmark();


/*
 *  push back character
 */
#define NOPEEK (EOF - 1)
int	peekc;
int	nextline_peek;

/* function prototypes */
void error(char *as);
void usage(char *msg, char *param);
void help();
void list_out(int optarg, char *line_str, char type);
void erreport();
void errorprt(int errnum);
void errwarn(int errnum, char *message);
void mlex(char *look);
void popsi();
void suffix(char *str, char *suff);
char *basename(char *filename);
char *getsuffix(char *str);
void outpath(char *out, char *src, char *suff);
void casname(char *out, char *src, int maxlen);
void putm(int c);
void insymtab(char *name);
void outsymtab(char *name);
void compactsymtab();
void putsymtab();
void clear();
void clear_instdata_flags();
void setmem(int addr, int value, int type);
void setvars();
void flushbin();
void flushoth();
void lineout();
void puthex(int byte, FILE *buf);
void putcas(int byte);
void putrelbits(int count, int bits);
void putrel(int byte);
void putrelname(char *str);
void putrelextaddr(int extaddr);
void putrelcmd(int cmd);
void putrelsegref(int scope, int addr);
void flushrel(void);
void lsterr1();
void lsterr2(int lst);
void copyname(char *st1, char *st2);
void next_source(char *sp, int always);
void incbin(char *filename);
void dc(int count, int value);
char *getmraslocal();
void write_tap(int len, int org, unsigned char *data);
void write_250(int low, int high);
void writewavs(int pad250, int pad500, int pad1500);
void reset_import();
int imported(char *filename);
int sized_byteswap(int value);

#define RELCMD_PUBLIC	(0)
#define RELCMD_COMMON	(1)
#define RELCMD_PROGNAME	(2)
#define RELCMD_LIBLOOK	(3)
#define RELCMD_EXTLINK	(4)
#define RELCMD_COMSIZE	(5)
#define RELCMD_EXTCHAIN	(6)
#define RELCMD_PUBVALUE	(7)
#define RELCMD_EXTMINUS	(8)
#define RELCMD_EXTPLUS	(9)
#define RELCMD_DATASIZE	(10)
#define RELCMD_SETLOC	(11)
#define RELCMD_CODESIZE	(13)
#define RELCMD_ENDMOD	(14)
#define RELCMD_ENDPROG	(15)

/*
 *  add a character to the output line buffer
 */
void addtoline(int ac)
{
	/* check for EOF from stdio */
	if (ac == -1)
		ac = 0 ;
	if (lineptr >= linemax)
		error("line buffer overflow");
	*lineptr++ = ac;
}

int get_tstates(unsigned char *inst, int *low, int *high, int *fetch, char *dis)
{
	return zi_tstates(inst, z80, low, high, fetch, dis);
}

/*
 *  put values in buffer for outputing
 */

void emit(int bytes, int desc, struct expr *data, ...)
{
	int type, i, args, dsize;
	va_list ap;

	if (relopt && segchange) {
		segchange = 0;
		putrelcmd(RELCMD_SETLOC);
		putrelsegref(segment, seg_pos[segment]);
	}

	// External references only supported in .rel output.
	if (!relopt && data && (data->e_scope & SCOPE_EXTERNAL)) {
		sprintf(detail, "External references only allowed in .rel output\n");
		errwarn(uflag, detail);
	}

	va_start(ap, data);

	type = desc == E_DATA ? MEM_DATA : MEM_INST;

	// Check emit is not adding instructions to the buffer.
	if (desc != E_DATA && emitptr != emitbuf)
		fprintf(stderr, "internal inconsistency in t-state counting\n");

	dsize = 0;
	args = bytes;
	if (desc == E_DATA) {
		args = 0;
		dsize = bytes;
	}
	else if (desc == E_CODE16)
		dsize = 2;
	else if (desc == E_CODE8)
		dsize = 1;

	for (i = 0; i < args; i++)
	{
		if (emitptr >= &emitbuf[EMITBUFFERSIZE])
			error("emit buffer overflow");
		else {
			int addr = (emit_addr + (emitptr - emitbuf)) & 0xffff;
			*emitptr = va_arg(ap, int);
			if (segment == SEG_CODE) 
				setmem(addr, *emitptr, type);
			putrel(*emitptr);
			putout(*emitptr);
			emitptr++;
		}
	}

	va_end(ap);

	for (i = 0; i < dsize; i++) {
		int addr = (emit_addr + (emitptr - emitbuf)) & 0xffff;
		*emitptr = data->e_value >> (i * 8);
		if (segment == SEG_CODE)
			setmem(addr, *emitptr, type);
		putout(*emitptr);
		emitptr++;
	}

	if (desc != E_DATA)
	{
		int eaddr = emit_addr, low, fetch, addr_after;

		get_tstates(emitbuf, &low, 0, &fetch, 0);

		// emitbuf is OK since this only happens with single emits
		int op = emitbuf[0] & 0xff;

		switch (z80) {
		case 0:
			// 8080 mode, error if Z-80 instructions.
			if (op == 0x08 || op == 0x10 || op == 0x18 ||
			    op == 0x20 || op == 0x28 || op == 0x30 ||
			    op == 0x38 || op == 0xCB || op == 0xD9 ||
			    op == 0xDD || op == 0xED || op == 0xFD)
			{
				errwarn(zflag, "Invalid 8080 instruction");
			}
			break;
		case 1: // Z-80 mode, error if Z-180 instructions
			if (op == 0xED && (
			    (emitbuf[1] & 0xC6) == 0 || // IN0, OUT0
			    (emitbuf[1] & 0xC7) == 4 || // TST r
			    (emitbuf[1] & 0xCF) == 0x4C || // MLT rr
			    emitbuf[1] == 0x64 || // TST m
			    emitbuf[1] == 0x74 || // TSTIO (m)
			    emitbuf[1] == 0x83 || // OTIM
			    emitbuf[1] == 0x93 || // OTIMR
			    emitbuf[1] == 0x8B || // OTDM
			    emitbuf[1] == 0x9B || // OTDMR
			    emitbuf[1] == 0x76)) // SLP
			{
				errwarn(zflag, "Invalid Z-80 instruction");
			}
			break;
		case 2:
			// Z-180 mode, error if undocumented Z-80 instructions
			// So many undocumented Z-80 instructions that I lean
			// on get_states() to answer.
			if (low <= 0)
				errwarn(zflag, "Invalid Z-180 instruction");
			break;
		}

		// Special case to catch promotion of djnz to DEC B JP NZ
		// Even update the tstatesum[] counter though that seems to
		// me to be above and beyond.
		if (emitbuf[0] == 5 && args == 2) {
			tstatesum[eaddr] = tstates;
			ocfsum[eaddr] = ocf;
			memflag[eaddr] |= MEM_T_SET;
			eaddr++;
			tstates += low;
			ocf += fetch;
			low = 10;
			// still 1 fetch
		}

		// Sanity check
		if (low <= 0 && !err[zflag])
		{
			fprintf(stderr, "undefined instruction on %02x %02x (assembler or disassembler broken)\n",
				emitbuf[0], emitbuf[1]);
		}


		// Double setting of both sides of tstatesum[] seems like too
		// much, but must be done in the isolated instruction case:
		// org x ; inc a ; org y

		tstatesum[eaddr] = tstates;
		ocfsum[eaddr] = ocf;
		memflag[eaddr] |= MEM_T_SET;

		// Well, OK, should we default to high or low???
		// Guess it should be whatever makes sense for you
		// to get here which, generally, is the low.

		// low it is.

		tstates += low;
		ocf += fetch;

		addr_after = (emit_addr + (emitptr - emitbuf)) & 0xffff;

		tstatesum[addr_after] = tstates;
		ocfsum[addr_after] = ocf;
		memflag[addr_after] |= MEM_T_SET;
	}

	if (relopt && outpass && dsize > 0) {
		if (dsize == 1) {
			if (is_number(data))
				putrel(data->e_value);
			else if (can_extend_link(data)) {
				extend_link(data);
				putrelop(RELOP_BYTE);
				putrel(0);
			}
			else {
				err[rflag]++;

				putrel(0);
			}
		}
		else if (dsize == 2) {
			int handled = 0;
			if (data->e_scope & SCOPE_EXTERNAL) {
				struct item *var = 0;
				int offset = 0;
				// Simple external reference.
				if (is_external(data))
					var = data->e_item;
				else if (is_external(data->e_left) &&
					data->e_token == '+' &&
					is_number(data->e_right))
				{
					var = data->e_left->e_item;
					offset = data->e_right->e_value;
				}
				else if (is_number(data->e_left) &&
					data->e_token == '+' &&
					is_external(data->e_right))
				{
					offset = data->e_left->e_value;
					var = data->e_right->e_item;
				}
				else if (is_external(data->e_left) &&
					data->e_token == '-' &&
					is_number(data->e_right))
				{
					var = data->e_left->e_item;
					offset = data->e_right->e_value;
				}

				if (var && offset) {
					putrelcmd(data->e_token == '-' ?
						RELCMD_EXTMINUS : RELCMD_EXTPLUS);
					// Theoretically we could put a
					// program or data relative value here...
					putrelsegref(SEG_ABS, offset);
				}

				if (var) {
					if (var->i_chain == 0) {
						putrel(0);
						putrel(0);
					}
					else {
						putrelbits(1, 1);
						putrelextaddr(var->i_chain);
					}
					var->i_chain = (segment << 16) |
						((dollarsign + args) & 0xffff);
					handled = 1;
				}
			}
			else if ((data->e_scope & ~SCOPE_PUBLIC) == 0) {
				// nice constant value
				putrel(data->e_value);
				putrel(data->e_value >> 8);
				handled = 1;
			}
			else if (!(data->e_scope & SCOPE_NORELOC)) {
				// relocatable value
				putrelbits(1, 1);
				putrelbits(2, data->e_scope);
				putrelbits(8, data->e_value);
				putrelbits(8, data->e_value >> 8);
				handled = 1;
			}

			if (!handled) {
				if (can_extend_link(data)) {
					extend_link(data);
					putrelop(RELOP_WORD);
					putrel(0);
					putrel(0);
				}
				else {
					err[rflag]++;
					putrel(data->e_value);
					putrel(data->e_value >> 8);
				}
			}
		}
		else if (dsize == 4) {
			// Only numbers are allowed.
			if (data->e_scope != 0) {
				err[vflag]++;
				if (data->e_scope & SCOPE_NORELOC)
					err[rflag]++;
			}
			for (i = 0; i < dsize; i++)
				putrel(data->e_value >> (i * 8));
		}
		else
			error("internal dsize error");
	}
}

#define ET_NOARG_DISP	(0)
#define ET_NOARG	(1)
#define ET_BYTE		(2)
#define ET_WORD		(5)

void emit1(int opcode, int regvalh, struct expr *data, int type)
{
	if (type == ET_BYTE && (data->e_value < -128 || data->e_value > 255))
		err[vflag]++;

	if (regvalh & 0x10000) {
		switch (type) {
		case ET_NOARG_DISP:
			emit(2, E_CODE, 0, regvalh >> 8, opcode);
			break;
		case ET_BYTE:
			emit(2, E_CODE8, data, regvalh >> 8, opcode);
			break;
		}
	}
	else if (regvalh & 0x8000) {
		switch (type) {
		case ET_NOARG_DISP:
			if (opcode & 0x8000)
				emit(4, E_CODE, 0, regvalh >> 8, opcode >> 8, disp, opcode);
			else
				emit(3, E_CODE, 0, regvalh >> 8, opcode, disp);
			break;
		case ET_NOARG:
			emit(2, E_CODE, 0, regvalh >> 8, opcode);
			break;
		case ET_BYTE:
			emit(3, E_CODE8, data, regvalh >> 8, opcode, disp);
			break;
		case ET_WORD:
			emit(2, E_CODE16, data, regvalh >> 8, opcode);
		}
	} else
		switch(type) {
		case ET_NOARG_DISP:
		case ET_NOARG:
			if (opcode & 0100000)
				emit(2, E_CODE, 0, opcode >> 8, opcode);
			else
				emit(1, E_CODE, 0, opcode);
			break;
		case ET_BYTE:
			emit(1, E_CODE8, data, opcode);
			break;
		case ET_WORD:
			if (opcode & 0100000)
				emit(2, E_CODE16, data, opcode >> 8, opcode);
			else
				emit(1, E_CODE16, data, opcode);
		}
}




void emitdad(int rp1,int rp2)
{
	if (rp1 & 0x8000)
		emit(2, E_CODE, 0, rp1 >> 8, rp2 + 9);
	else
		emit(1, E_CODE, 0, rp2 + 9);
}


void emitjr(int opcode, struct expr *dest)
{
	int disp = dest->e_value - dollarsign - 2;

	if (dest->e_scope & SCOPE_NORELOC)
		err[rflag]++;

	// Can't relative jump to other segments or an external
	// However, without .rel output we default to the code segment
	// for various reasons thus we let "jr 0" (and such) be acceptable
	// in those cases.
	if (((relopt && (dest->e_scope & SCOPE_SEGMASK) != segment) ||
		(dest->e_scope & SCOPE_EXTERNAL) ||
		disp > 127 || disp < -128) && z80)
	{
		if (npass > 1 && jopt) {
			njrpromo++;
			switch (opcode) {
			case 0x10: // DJNZ
				emit(2, E_CODE16, dest, 0x05, 0xC2); // DEC B, JP NZ
				break;
			case 0x18: // JR
				emit(1, E_CODE16, dest, 0xC3); // JP
				break;
			case 0x20: // JR NZ
				emit(1, E_CODE16, dest, 0xC2); // JP NZ
				break;
			case 0x28: // JR Z
				emit(1, E_CODE16, dest, 0xCA); // JP Z
				break;
			case 0x30: // JR NC
				emit(1, E_CODE16, dest, 0xD2); // JP NC
				break;
			case 0x38: // JR C
				emit(1, E_CODE16, dest, 0xDA); // JP C
				break;
			default:
				err[vflag]++;	// shouldn't happen!
				expr_free(dest);
				break;
			}
		}
		else {
			emit(2, E_CODE, 0, opcode, -2);  // branch to itself
			err[vflag]++;
			expr_free(dest);
		}
	}
	else {
		emit(2, E_CODE, 0, opcode, disp);
		expr_free(dest);
	}
}

void checkjp(int op, struct expr *dest)
{
	op &= 0x030;
	// Only applies to Z-80 output and if JP optimization checking is on.
	// JR only has z, nz, nc, c
	// A jump to the current segment might have been optimizable
	if (z80 && JPopt && (op == 0 || op == 010 || op == 020 || op == 030) &&
		(dest->e_scope & (SCOPE_SEGMASK | SCOPE_EXTERNAL)) == segment)
	{
		int disp = dest->e_value - dollarsign - 2;
		if (disp >= -128 && disp <= 127)
			err[jflag]++;
	}
}

/*
 *  put out a byte of binary 
 */
void putbin(int v)
{
	if(!outpass) return;
	*outbinp++ = v;
	if (outbinp >= outbinm) flushbin();

	outoth[outoth_cnt++] = v;
	if (outoth_cnt == 256) flushoth();
}



/*
 *  output one line of binary in INTEL standard form
 */
void flushbin()
{
	char *p;
	int check=outbinp-outbin;

	if (!outpass)
		return;
	nbytes += check;
	if (check) {
		if (fbuf) {
			putc(':', fbuf);
			puthex(check, fbuf);
			puthex(olddollar>>8, fbuf);
			puthex(olddollar, fbuf);
			puthex(0, fbuf);
			check += (olddollar >> 8) + olddollar;
			olddollar += (outbinp-outbin);
			for (p=outbin; p<outbinp; p++) {
				puthex(*p, fbuf);
				check += *p;
			}
			puthex(256-check, fbuf);
			putc('\n', fbuf);
		}
		outbinp = outbin;
	}
}



/*
 *  put out one byte of hex
 */
void puthex(int byte, FILE *buf)
{
	putc(hexadec[(byte >> 4) & 017], buf);
	putc(hexadec[byte & 017], buf);
}

// Case-independent string comparisons.

int ci_strcompare(char *s1, char *s2, int len)
{
	int c1, c2;
	do {
		if (len == 0)
			return 0;

		c1 = *s1++;
		if (c1 >= 'A' && c1 <= 'Z') c1 += 'a' - 'A';
		c2 = *s2++;
		if (c2 >= 'A' && c2 <= 'Z') c2 += 'a' - 'A';
		if (c1 != c2)
			return c1 - c2;

		if (len > 0)
			if (--len == 0)
				return 0;

	} while (c1 && c2);

	return 0;
}

int ci_strcmp(char *s1, char *s2)
{
	return ci_strcompare(s1, s2, -1);
}

void flushoth()
{
	int i, checksum;

	if (!outpass || outoth_cnt == 0)
		return;

	if (fcmd) {
		fprintf(fcmd, "%c%c%c%c", 1, outoth_cnt + 2, oldothdollar, oldothdollar >> 8);
		fwrite(outoth, outoth_cnt, 1, fcmd);
	}

	putcas(0x3c);
	putcas(outoth_cnt);
	putcas(oldothdollar);
	putcas(oldothdollar >> 8);
	checksum = oldothdollar + (oldothdollar >> 8);
	for (i = 0; i < outoth_cnt; i++) {
		putcas(outoth[i]);
		checksum += outoth[i];
		if (fmds)
			fprintf(fmds, "b@$%04x=$%02x\n", oldothdollar + i, outoth[i] & 0xff);
	}
	putcas(checksum);

	oldothdollar += outoth_cnt;
	outoth_cnt = 0;
}

int casbit, casbitcnt = 0;

void putcas(int byte)
{
	if (flcas)
		fputc(byte, flcas);

	if (flnwcas)
		fputc(byte, flnwcas);

	if (fcas) {
		// Buffer 0 stop bit and the 8 data bits.
		casbit = (casbit << 9) | (byte & 0xff);
		casbitcnt += 9;
		while (casbitcnt >= 8) {
			casbitcnt -= 8;
			fputc(casbit >> casbitcnt, fcas);
		}
	}
}

void casname(char *out, char *src, int maxlen)
{
	char *base = basename(src);
	int i;

	out[0] = 'N';
	for (i = 1; i < maxlen; i++)
		out[i] = ' ';

	for (i = 0; *base && i < maxlen; base++) {
		if (ci_strcmp(base, ".z") == 0 || ci_strcmp(base, ".asm") == 0)
			break;

		if (*base >= 'a' && *base <= 'z') {
			*out++ = *base - ('a' - 'A');
			i++;
		}
		else if (*base >= 'A' && *base <= 'Z') {
			*out++ = *base;
			i++;
		}
	}
}

int relbit, relbitcnt = 0;

void putrelbits(int count, int bits)
{
	if (!outpass || !relopt)
		return;

	relbit <<= count;
	relbit |= bits & ((1 << count) - 1);
	relbitcnt += count;

	while (relbitcnt >= 8) {
		relbitcnt -= 8;
		fputc(relbit >> relbitcnt, frel);
	}
}

void putrel(int byte)
{
	// Add 0 bit indicating byte to load at next counter
	putrelbits(1, 0);
	// Add byte to load
	putrelbits(8, byte);
}

void putrelname(char *str)
{
	int len = strlen(str);
	int maxlen = mras ? 7 : relopt;

	// .rel file format can do strings 7 long but for compatibility
	// we restrict them to 6.  I believe this is important because
	// extended link functions require a character when they wish to
	// operate on an external symbol.
	if (len > maxlen)
		len = maxlen;
	putrelbits(3, len);
	while (len-- > 0) {
		int ch = *str++;
		if (ch >= 'a' && ch <= 'z')
			ch -= 'a' - 'A';
		putrelbits(8, ch);
	}
}

void putrelsegref(int scope, int addr)
{
	putrelbits(2, scope);
	putrelbits(8, addr);
	putrelbits(8, addr >> 8);
}

void putrelextaddr(int extaddr)
{
	putrelsegref(extaddr >> 16, extaddr);
}


void putrelcmd(int relcmd)
{
	putrelbits(1, 1);
	putrelbits(2, 0);
	putrelbits(4, relcmd);
}

void flushrel(void)
{
	if (relbitcnt > 0)
		putrelbits(8 - relbitcnt, 0);

	if (relopt)
		fflush(frel);
}

void list_cap_line()
{
	if (multiline) {
		if (lineptr > linebuf)
			lineptr[-1] = separator;
		addtoline('\n');
	}
	addtoline('\0');

	prev_multiline = multiline;
	multiline = 0;
}

/*
 *  put out a line of output -- also put out binary
 */
void list(int optarg)
{
	list_cap_line();

	list_out(optarg, linebuf, ' ');

	lineptr = linebuf;
}

void delayed_list(int optarg)
{
	int delay = iflist() && !mopt;
	FILE *tmp = fout;

	fout = delay ? NULL : tmp;
	list(optarg);
	fout = tmp;
	bookmark(delay);
}

void list_optarg(int optarg, int seg, int type)
{
	if (seg < 0 || !relopt)
		seg = relopt ? segment : SEG_ABS;

	puthex(optarg >> 8, fout);
	puthex(optarg, fout);
	fputc(SEGCHAR(seg), fout);
	if (type)
		fputc(type, fout);
}

void bds_perm(int dollar, int addr, int len)
{
	while (len > 0) {
		int blklen;
		int usage = memflag[addr & 0xffff] & (MEM_INST | MEM_DATA);

		for (blklen = 0; blklen < len; blklen++) {
			int u = memflag[(addr + blklen) & 0xffff] & (MEM_INST | MEM_DATA);
			if (u != usage)
				break;
		}

		int bu = 0;
		if (usage & MEM_INST) bu |= 1;
		if (usage & MEM_DATA) bu |= 2;

		while (blklen > 0) {
			int size = blklen;
			if (size > 255) size = 255;
			fprintf(fbds, "%04x %04x u %02x %02x\n",
				dollar, addr & 0xffff, size, bu);

			addr += size;
			dollar += size;
			len -= size;

			blklen -= size;
		}
	}
}

void list_out(int optarg, char *line_str, char type)
{
	unsigned char *	p;
	int	i;
	int  lst;

	if (outpass) {
		lst = iflist();
		if (lst) {
			lineout();
			if (nopt)
				fprintf(fout, "%4d:", linein[now_in]);

			if (copt)
			{
			    if (emitptr > emitbuf && (memflag[emit_addr] & MEM_INST))
			    {
			        int low, high, fetch;
			        get_tstates(memory + emit_addr, &low, &high, &fetch, 0);

				// Special case to catch promotion of djnz to DEC B JP NZ
				if (memory[emit_addr] == 5 && emitptr - emitbuf == 4) {
					low += 10;
					high += 10;
				}

			    	fprintf(fout, nopt ? "%5d" : "%4d", tstatesum[emit_addr]);

				fprintf(fout, "+%d", low);
				if (low != high)
				    fprintf(fout, "+%d", high - low);
			    }
			    else
			    {
			        fprintf(fout, nopt ? "%5s-" : "%4s-", "");
			    }
			}

			if (nopt || copt)
				fprintf(fout, "\t");

			list_optarg(optarg, -1, type);

			for (p = emitbuf; (p < emitptr) && (p - emitbuf < 4); p++) {
				puthex(*p, fout);
			}
			for (i = 4 - (p-emitbuf); i > 0; i--)
				fputs("  ", fout);

			putc('\t', fout);
			fputs(line_str, fout);
		}

		if (fbds) {
			if (emitptr > emitbuf) {
				fprintf(fbds, "%04x %04x d ", dollarsign, emit_addr);
				for (p = emitbuf; p < emitptr; p++)
					fprintf(fbds, "%02x", *p & 0xff);
				fprintf(fbds, "\n");

				bds_perm(dollarsign, emit_addr, emitptr - emitbuf);
			}
			fprintf(fbds, "%04x %04x s %s", dollarsign, emit_addr, line_str);
		}

		for (p = emitbuf; p < emitptr; p++)
			putbin(*p);

		p = emitbuf+4;
		while (lst && gopt && p < emitptr) {
			lineout();
			if (nopt) putc('\t', fout);
			fputs("      ", fout);
			if (copt) fputs("        ", fout);
			for (i = 0; (i < 4) && (p < emitptr);i++) {
				puthex(*p, fout);
				p++;
			}
			putc('\n', fout);
		}

		lsterr2(lst);
	} else
		lsterr1();

	dollarsign += emitptr - emitbuf;
	emit_addr += emitptr - emitbuf;
	dollarsign &= 0xffff;
	emit_addr &= 0xffff;
	emitptr = emitbuf;
}



/*
 *  keep track of line numbers and put out headers as necessary
 */
void lineout()
{
	if (!printer_output || !fout)
		return;

	if (line == 60) {
		if (popt)
			putc('\014', fout);	/* send the form feed */
		else
			fputs("\n\n\n\n\n", fout);
		line = 0;
	}
	if (line == 0) {
		fprintf(fout, "\n\n%s %s\t%s\t Page %d\n\n\n",
			&timp[4], &timp[20], title, page++);
		line = 4;
	}
	line++;
}


/*
 *  cause a page eject
 */
void eject()
{
	if (!printer_output)
		return;

	if (outpass && iflist()) {
		if (popt) {
			putc('\014', fout);	/* send the form feed */
		} else {
			while (line < 65) {
				line++;
				putc('\n', fout);
			}
		}
	}
	line = 0;
}


/*
 *  space n lines on the list file
 */
void space(int n)
{
	int	i ;
	if (outpass && iflist() && fout)
		for (i = 0; i<n; i++) {
			lineout();
			putc('\n', fout);
		}
}

/*
 *  Error handling - pass 1
 */
void lsterr1()
{
	int i;
	for (i = 0; i <= mflag; i++)
		if (err[i]) {
			if (topt)
				errorprt(i);
			passfail = 1;
			err[i] = 0;
		}
}


/*
 *  Error handling - pass 2.
 */
void lsterr2(int lst)
{
	int i;
	for (i=0; i<FLAGS; i++)
		if (err[i]) {
			if (i < FIRSTWARN)
				passfail = 1;
			if (lst) {
				char *desc = errname[i];
				char *type = i < FIRSTWARN ? " error" : " warning";
				if (errdetail[i][0]) {
					desc = errdetail[i];
					type = "";
				}
				lineout();
				if (fout)
					fprintf(fout, "%c %s%s\n",
						errlet[i], desc, type);
			}
			err[i] = 0;
			keeperr[i]++;
			if (i > mflag && topt)
				errorprt(i);
		}

	if (fout)
		fflush(fout);	/*to avoid putc(har) mix bug*/
}

/*
 *  print diagnostic to error terminal
 */
void errorprt(int errnum)
{
	char *desc = errname[errnum];
	char *type = errnum < FIRSTWARN ? " error" : " warning";
	if (errdetail[errnum][0]) {
		desc = errdetail[errnum];
		type = "";
	}
	fprintf(stderr, "%s(%d) : %s%s",
		src_name[now_in], linein[now_in], desc, type);

	errdetail[errnum][0] = '\0';

	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", linebuf);
	fflush(stderr) ;
}

void errwarn(int errnum, char *message)
{
	if (errnum == uflag && mras_undecl_ok)
		return;

	err[errnum]++;
	strcpy(errdetail[errnum], message);
}

/*
 *  list without address -- for comments and if skipped lines
 */
void list1()
{
	int lst;

	list_cap_line();
	lineptr = linebuf;
	if (outpass) {
		if ((lst = iflist())) {
			lineout();
			if (nopt)
				fprintf(fout, "%4d:\t", linein[now_in]);
			if (copt)
				fprintf(fout, "\t");
			fprintf(fout, "\t\t%s", linebuf);
			lsterr2(lst);
		}
		if (fbds)
			fprintf(fbds, "%04x %04x s %s", dollarsign, emit_addr, linebuf);
	}
	else
		lsterr1();
}

void delayed_list1()
{
	int delay = iflist() && !mopt;
	FILE *tmp = fout;

	fout = delay ? NULL : tmp;
	list1();
	fout = tmp;

	bookmark(delay);
}

/*
 *  see if listing is desired
 */
int iflist()
{
	int problem;

	if (!fout)
		return 0;

	if (inmlex)
		return mlex_list_on;

	if (lston)
		return(1) ;
	if (*ifptr && !fopt)
		return(0);
	if (!lstoff && !expptr)
		return(1);
	problem = countwarn() + counterr();
	if (expptr) {
		if (problem) return(1);
		if (!mopt) return(0);
		if (mopt == 1) return(1);
		return(emitptr > emitbuf);
	}
	if (eopt && problem)
		return(1);
	return(0);
}

void do_equ(struct item *sym, struct expr *val, int call_list);
void do_defl(struct item *sym, struct expr *val, int call_list);

// GWP - This avoids an apparent bug in bison as it tries to start out the
// Not needed under gcc which defines __STDC__ so I guard it to prevent
// warnings.
// yyparse() function with yyparse() ; { }.
#ifndef __STDC__
#define __STDC__
#endif

#define PSTITL	(0)	/* title */
#define PSRSYM	(1)	/* rsym */
#define PSWSYM	(2)	/* wsym */
#define PSINC	(3)	/* include file */
#define PSMACLIB (4)	/* maclib (similar to include) */
#define PSIMPORT (5)	/* import file */
#define PSCMN	(6)	/* common block */

#define SPTITL	(0)	/* title */
#define SPSBTL	(1)	/* sub title */
#define SPNAME	(2)	/* name */
#define SPCOM	(3)	/* comment */
#define SPPRAGMA (4)	/* pragma */

%}

%union	{
	struct expr *exprptr;
	struct item *itemptr;
	int ival;
	char *cval;
	}

%token <cval> STRING
%token <itemptr> NOOPERAND
%token <itemptr> ARITHC
%token ADD
%token <itemptr> LOGICAL
%token <itemptr> AND
%token <itemptr> OR
%token <itemptr> XOR
%token <ival> ANDAND
%token <ival> OROR
%token <itemptr> BIT
%token CALL
%token <itemptr> INCDEC
%token <itemptr> DJNZ
%token EX
%token <itemptr> IM
%token PHASE
%token DEPHASE
%token <itemptr> TK_IN
%token <itemptr> JR
%token LD
%token <itemptr> TK_OUT
%token <itemptr> PUSHPOP
%token <itemptr> RET
%token <itemptr> SHIFT
%token <itemptr> RST
%token <itemptr> REGNAME
%token <itemptr> IXYLH
%token <itemptr> ACC
%token <itemptr> TK_C
%token <itemptr> RP
%token <itemptr> HL
%token <itemptr> INDEX
%token <itemptr> AF
%token <itemptr> TK_F
%token AFp
%token <itemptr> SP
%token <itemptr> MISCREG
%token <itemptr> COND
%token <itemptr> SPCOND
%token <ival> NUMBER
%token <itemptr> UNDECLARED
%token END
%token ORG
%token ASSERT
%token TSTATE
%token <ival> T
%token <ival> TILO
%token <ival> TIHI
%token SETOCF
%token <ival> OCF
%token <ival> LOW
%token <ival> HIGH
%token DC
%token DEFB
%token DEFD
%token DEFS
%token DEFW
%token DEF3
%token EQU
%token DEFL
%token <itemptr> LABEL
%token <itemptr> EQUATED
%token <itemptr> WASEQUATED
%token <itemptr> DEFLED
%token <itemptr> COMMON
%token <itemptr> MULTDEF
%token <ival> SHL
%token <ival> SHR
%token <ival> LT
%token <ival> EQ
%token <ival> LE
%token <ival> GE
%token <ival> NE
%token <ival> NOT
%token <ival> MROP_ADD MROP_SUB MROP_MUL MROP_DIV MROP_MOD
%token <ival> MROP_AND MROP_OR MROP_XOR
%token <ival> MROP_NE MROP_EQ MROP_LT MROP_GT MROP_LE MROP_GE
%token <ival> MROP_SHIFT MROP_SHL MROP_SHR
%token <ival> MROP_NOT MROP_LOW MROP_HIGH
%token IF_TK
%token <itemptr> IF_DEF_TK
%token ELSE_TK
%token ENDIF_TK
%token <itemptr> ARGPSEUDO
%token <itemptr> INCBIN
%token <itemptr> LIST
%token <itemptr> MINMAX
%token MACRO
%token <itemptr> MNAME
%token ARG
%token ENDM
%token <ival> ONECHAR
%token <ival> TWOCHAR
%token JRPROMOTE
%token JPERROR
%token PUBLIC
%token EXTRN
%token MRAS_MOD
%token <itemptr> SETSEG INSTSET
%token LXI DAD STAX STA SHLD LDAX LHLD LDA MVI MOV
%token <itemptr> INXDCX INRDCR PSW JUMP8 JP CALL8 ALUI8
%token <itemptr> SPECIAL
%token RAWTOKEN LOCAL
// New token types for ZNONSTD support
%token <itemptr> LD_XY ST_XY MV_XY ALU_XY BIT_XY SHIFT_XY INP OUTP JR_COND
%token <itemptr> LDST16 ARITH16
// Tokens for improved macro support
%token REPT IRPC IRP EXITM
%token NUL
%token <itemptr> MPARM
%token <itemptr> TK_IN0 TK_OUT0 MLT TST TSTIO

%type <itemptr> label.part symbol
%type <ival> allreg reg evenreg ixylhreg realreg mem memxy pushable bcdesp bcdehl bcdehlsp mar condition
%type <ival> spcondition
%type <exprptr> noparenexpr parenexpr expression
%type <ival> maybecolon maybeocto
%type <ival> evenreg8 reg8 m pushable8
// Types for improved macro support
%type <cval> locals
%type <ival> varop
%type <itemptr> aliasable

%right '?' ':'
%left OROR
%left ANDAND
%left '|' OR
%left '^' XOR
%left '&' AND
%left '=' EQ NE
%left '<' '>' LT GT LE GE
%left SHL SHR
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' UNARY

%left MROP_ADD MROP_SUB MROP_MUL MROP_DIV MROP_MOD MROP_SHIFT MROP_AND MROP_OR MROP_XOR MROP_NE MROP_EQ MROP_LT MROP_GT MROP_LE MROP_GE MROP_SHL MROP_SHR MROP_NOT MROP_LOW MROP_HIGH

%{
char  *cp;
int  i;

void do_equ(struct item *sym, struct expr *val, int call_list)
{
	expr_reloc_check(val);
	switch(sym->i_token) {
	case UNDECLARED: case WASEQUATED:
		if (sym->i_token == WASEQUATED &&
			(sym->i_value != val->e_value ||
			 ((sym->i_scope ^ val->e_scope) & SCOPE_SEGMASK)))
		{
			if (outpass) {
				if (sym->i_value != val->e_value)
					sprintf(detail, "%s error - %s went from $%04x to $%04x",
						errname[pflag], sym->i_string, sym->i_value, val->e_value);
				else
					sprintf(detail, "%s error - %s changed scope",
						errname[pflag], sym->i_string);
				errwarn(pflag, detail);
			}
			else
				passretry = 1;
		}

		sym->i_token = EQUATED;
		sym->i_pass = npass;
		sym->i_value = val->e_value;
		sym->i_scope |= val->e_scope;
		break;
	default:
		// m80 allows multiple equates as long as the value
		// does not change.  So does newer zmac.
		if (sym->i_value != val->e_value ||
			((sym->i_scope ^ val->e_scope) & SCOPE_SEGMASK))
		{
			err[mflag]++;
			sym->i_token = MULTDEF;
			sym->i_pass = npass;
		}
	}
	sym->i_scope &= ~SCOPE_BUILTIN;
	if (call_list)
		list(val->e_value);
	expr_free(val);
}

void do_defl(struct item *sym, struct expr *val, int call_list)
{
	expr_reloc_check(val);
	switch(sym->i_token) {
	case UNDECLARED: case DEFLED:
		sym->i_token = DEFLED;
		sym->i_pass = npass;
		sym->i_value = val->e_value;
		sym->i_scope = (sym->i_scope & SCOPE_SEGMASK) | val->e_scope;
		break;
	default:
		err[mflag]++;
		sym->i_token = MULTDEF;
		sym->i_pass = npass;
		break;
	}
	if (call_list)
		list(val->e_value);
	expr_free(val);
}

void do_end(struct expr *entry)
{
	if (entry) {
		expr_reloc_check(entry);
		xeq_flag++;
		xeq = entry->e_value & 0xffff;
		rel_main = ((entry->e_scope & SCOPE_SEGMASK) << 16) | xeq;
		expr_free(entry);
	}

// TODO - no reason no to treat END as a simple mras or not.
// At least, give this a try and see how it goes.
//	if (mras) {
		while (expptr)
			popsi();

		peekc = NOPEEK;
		nextline_peek = EOF;
//	}
//	else
//		peekc = 0;

}

void common_block(char *unparsed_id)
{
	struct item *it;
	char *id = unparsed_id;
	char *p;
	int unnamed;

	if (*id == '/') {
		id++;
		for (p = id; *p; p++)
			if (*p == '/')
				*p = '\0';
	}

	unnamed = 1;
	for (p = id; *p; p++)
		if (*p != ' ')
			unnamed = 0;

	id = unnamed ? " " : id;

	it = locate(id);
	switch (it->i_token) {
	case 0:
		nitems++;
	case UNDECLARED:
	case COMMON:
		it->i_value = 0;
		it->i_token = COMMON;
		it->i_pass = npass;
		it->i_scope = SCOPE_COMBLOCK;
		it->i_uses = 0;
		if (!it->i_string)
			it->i_string = strdup(id);
		break;
	default:
		err[mflag]++;
		it->i_token = MULTDEF;
		it->i_pass = npass;
			it->i_string = strdup(id);
		break;
	}

	// Even if we change to the same COMMON block the address is
	// reset back to 0.
	if (relopt) {
		segment = SEG_COMMON;
		segchange = 1;
		dollarsign = seg_pos[SEG_COMMON] = seg_size[SEG_COMMON] = 0;
		// May not be necessary but too much trouble to suppress.
		putrelcmd(RELCMD_COMMON);
		putrelname(it->i_string);
	}

	cur_common = it;
}

int at_least_once; // global to avoid repetition in macro repeat count processing

void dolopt(struct item *itm, int enable)
{
	if (outpass) {
		lineptr = linebuf;
		switch (itm->i_value) {
		case 0:	/* list */
			if (enable < 0) lstoff = 1;
			if (enable > 0) lstoff = 0;
			break;

		case 1:	/* eject */
			if (enable) eject();
			break;

		case 2:	/* space */
			if ((line + enable) > 60) eject();
			else space(enable);
			break;

		case 3:	/* elist */
			eopt = edef;
			if (enable < 0) eopt = 0;
			if (enable > 0) eopt = 1;
			break;

		case 4:	/* fopt */
			fopt = fdef;
			if (enable < 0) fopt = 0;
			if (enable > 0) fopt = 1;
			break;

		case 5:	/* gopt */
			gopt = gdef;
			if (enable < 0) gopt = 1;
			if (enable > 0) gopt = 0;
			break;

		case 6: /* mopt */
			mopt = mdef;
			if (enable < 0) mopt = 0;
			if (enable > 0) mopt = 1;
		}
	}
}

%}

%%

statements:
	/* Empty file! */
|
	statements statement
;


statement:
	label.part '\n'	{ 
		// An identfier without a colon all by itself on a line
		// will be interpreted as a label.  But there's a very
		// good chance it is a misspelling of an instruction or
		// pseudo-op name creating silent errors.  Since the condition
		// is unusual we print a warning.  Unless it is followed by
		// a colon in which case there's no ambiguity.
		if ($1 && !firstcol && coloncnt == 0 && outpass) {
			fprintf(stderr, "%s(%d): warning: '%s' treated as label (instruction typo?)\n",
				src_name[now_in], linein[now_in], $1->i_string);
			fprintf(stderr, "\tAdd a colon or move to first column to stop this warning.\n");
		}

		if ($1) list(dollarsign);
		else  list1();
	}
|
	label.part { list_dollarsign = 1; } operation '\n' {
		list(list_dollarsign ? dollarsign : list_addr);
	}
|
	symbol maybecolon EQU expression '\n' {
		do_equ($1, $4, 1);
		if ($2 == 2)
			$1->i_scope |= SCOPE_PUBLIC;
	}
|
	symbol maybecolon EQU aliasable '\n' {
		do_equ($1, expr_num(keyword_index($4)), 1);
		$1->i_scope |= SCOPE_ALIAS;
		if ($2 == 2)
			$1->i_scope |= SCOPE_PUBLIC;
	}
|
	symbol maybecolon '=' expression '\n' {
		do_defl($1, $4, 1); // TODO: is '=' equate or defl?
		// I don't even recall what assembler I saw that allows '='
		// Not MACR0-80.  Not MRAS.  Not MAC.
		// I choose "defl" since it works so nicely with +=, etc.
		if ($2 == 2)
			$1->i_scope |= SCOPE_PUBLIC;
	}
|
	symbol DEFL expression '\n' {
		do_defl($1, $3, 1);
	}
|
	symbol varop '=' expression '\n' {
		do_defl($1, expr_mk(expr_var($1), $2, $4), 1);
	}
|
	symbol '+' '+' '\n' {
		do_defl($1, expr_mk(expr_var($1), '+', expr_num(1)), 1);
	}
|
	symbol '-' '-' '\n' {
		do_defl($1, expr_mk(expr_var($1), '-', expr_num(1)), 1);
	}
|
	symbol MINMAX expression ',' expression '\n' {
		int val3 = $3->e_value;
		int val5 = $5->e_value;
		expr_reloc_check($3);
		expr_reloc_check($5);
		expr_scope_same($3, $5);
		switch ($1->i_token) {
		case UNDECLARED: case DEFLED:
			$1->i_token = DEFLED;
			$1->i_pass = npass;
			$1->i_scope |= $3->e_scope;
			if ($2->i_value)	/* max */
				list($1->i_value = (val3 > val5? val3:val5));
			else list($1->i_value = (val3 < val5? val3:val5));
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
			$1->i_pass = npass;
			list($1->i_value);
		}
		expr_free($3);
		expr_free($5);
	}
|
	IF_TK expression '\n' {
		expr_number_check($2);
		if (ifptr >= ifstmax)
			error("Too many ifs");
		else
			*++ifptr = !($2->e_value);

		saveopt = fopt;
		fopt = 1;
		list($2->e_value);
		fopt = saveopt;
		expr_free($2);
	}
|
	// IF_DEF_TK UNDECLARED '\n' might work, but probably would define the symbol
	IF_DEF_TK arg_on ARG arg_off '\n' {
		struct item *ip = locate(tempbuf);
		int declared = ip && ip->i_pass == npass;
		int value = declared == $1->i_value;

		if (ifptr >= ifstmax)
			error("Too many ifs");
		else
			*++ifptr = !value;

		saveopt = fopt;
		fopt = 1;
		list(value);
		fopt = saveopt;
	}
|
	ELSE_TK '\n' {
		/* FIXME: it would be nice to spot repeated ELSEs, but how? */
		*ifptr = !*ifptr;
		saveopt = fopt;
		fopt = 1;
		list1();
		fopt = saveopt;
	}
|
	ENDIF_TK '\n' {
		if (ifptr == ifstack) err[bflag]++;
		else --ifptr;
		list1();
	}
|
	label.part END '\n' {
		list(dollarsign);
		do_end(NULL);
	}
|
	label.part END expression '\n' {
		list($3->e_value);
		do_end($3);
	}
|
	label.part DEFS expression '\n' {
		expr_number_check($3);
		if ($3->e_value < 0) err[vflag]++;
		if ($3->e_value > 0) {
			if (!phaseflag) {
				list(dollarsign);
				flushbin();
				flushoth();
				dollarsign += $3->e_value;
				olddollar += $3->e_value;
				oldothdollar += $3->e_value;
				emit_addr += $3->e_value;
				advance_segment($3->e_value);
				putrelcmd(RELCMD_SETLOC);
				putrelsegref(segment, seg_pos[segment]);
			}
			else
				dc($3->e_value, 0);
		}
		else
			list1();

		expr_free($3);
	}
|
	label.part DEFS expression ',' expression '\n' {
		expr_number_check($3);
		expr_number_check($5);
		if ($3->e_value < 0) err[vflag]++;
		if ($5->e_value < -128 || $5->e_value > 127) err[vflag]++;
		if ($3->e_value > 0) {
			dc($3->e_value, $5->e_value);
		}
		else
			list1();

		expr_free($3);
		expr_free($5);
	}
|
	label.part DC ONECHAR '\n' { emit(1, E_DATA, expr_num($3 | 0x80)); list(dollarsign); }
|
	label.part DC TWOCHAR '\n' { emit(1, E_DATA, expr_num($3)); emit(1, E_DATA, expr_num(($3 >> 8) | 0x80)); list(dollarsign); }
|
	label.part DC STRING '\n'
		{
			for (cp = $3; *cp != '\0'; cp++)
				if (!cp[1])
					emit(1, E_DATA, expr_num(*cp | 0x80));
				else
					emit(1, E_DATA, expr_num(*cp));

			list(dollarsign);
		}
|
	label.part DC expression ',' expression '\n'
		{
			expr_number_check($3);
			expr_number_check($5);
			dc($3->e_value, $5->e_value);
			expr_free($3);
			expr_free($5);
		}
|
	label.part ARGPSEUDO arg_on ARG arg_off '\n' {
		list1();
		switch ($2->i_value) {

		case PSTITL:	/* title */
			lineptr = linebuf;
			cp = tempbuf;
			title = titlespace;
			while ((*title++ = *cp++) && (title < &titlespace[TITLELEN]));
			*title = 0;
			title = titlespace;
			break;

		case PSRSYM:	/* rsym */
			if (pass2) break;
			insymtab(tempbuf);
			break;

		case PSWSYM:	/* wsym */
			writesyms = malloc(strlen(tempbuf)+1);
			strcpy(writesyms, tempbuf);
			break;
		case PSINC:	/* include file */
			if (mras && !strchr(tempbuf, '.')) {
				strcat(tempbuf, ".asm");
			}
			next_source(tempbuf, 1);
			break ;
		case PSIMPORT:	/* import file */
			next_source(tempbuf, 0);
			break;
		case PSMACLIB:
			strcat(tempbuf, ".lib");
			next_source(tempbuf, 1);
			break;
		case PSCMN:
			common_block(tempbuf);
			break;
		}
	}
|
	label.part ARGPSEUDO arg_on arg_off '\n' {
		if ($2->i_value == PSCMN) {
			common_block(" ");
		}
		else {
			fprintf(stderr, "Missing argument of '%s'\n", $2->i_string);
			err[fflag]++;
		}
		list1();
	}
|
	label.part INCBIN arg_on ARG arg_off '\n' {
		incbin(tempbuf);
	}
|
	SPECIAL { raw = 1; } RAWTOKEN {
		int quote = 0;
		char *p, *q;
		switch ($1->i_value) {
		case SPTITL:
			cp = tempbuf;
			title = titlespace;
			if (*cp == '\'' || *cp == '"')
				quote = *cp++;
			while ((*title++ = *cp++) && (title < &titlespace[TITLELEN]));
			if (quote && title > titlespace + 1 && title[-2] == quote)
				title[-2] = '\0';
			title = titlespace;
			list1();
			break;
		case SPSBTL:
			err[warn_notimpl]++;
			list1();
			break;
		case SPNAME:
			// Drop surrounding ('') if present
			p = tempbuf;
			q = strchr(tempbuf, '\0') - 1;
			if (*p == '(' && *q == ')' && q > p) p++, q--;
			if (*p == '\'' && *q == '\'' && q > p) p++, q--;
			q[1] = '\0';
			strncpy(progname, p, sizeof progname);
			progname[sizeof progname - 1] = '\0';
			list1();
			break;
		case SPCOM:
			quote = *tempbuf;
			list1();
			for (;;) {
				raw = 1;
				yychar = yylex();
				list1();
				if (yychar == 0)
					break;
				if (*tempbuf == quote) {
					yychar = yylex();
					break;
				}
			}
			break;

		case SPPRAGMA:
			if (strncmp(tempbuf, "bds", 3) == 0 && fbds && outpass) {
				fprintf(fbds, "%s\n", tempbuf + 4);
			}
			if (strncmp(tempbuf, "mds", 3) == 0 && fmds && outpass) {
				fprintf(fmds, "%s\n", tempbuf + 4);
			}
			list1();
			break;
		}
	}
|
	LIST '\n' {
		dolopt($1, 1); }
|
	LIST mras_undecl_on expression mras_undecl_off '\n' {
		int enable = $3->e_value;

		if (mras) {
			if (ci_strcmp(tempbuf, "on") == 0)
				enable = 1;
			else if (ci_strcmp(tempbuf, "off") == 0)
				enable = -1;
			else {
				fprintf(stderr, "LIST not given 'on' or 'off'\n");
				err[fflag]++;
				list(dollarsign);
				goto dolopt_done;
			}
		}
		else {
			expr_number_check($3);
			expr_free($3);
		}

		dolopt($1, enable);

	dolopt_done: ;
	}
|
	JRPROMOTE expression '\n' {
		expr_number_check($2);
		jopt = !!$2->e_value;
		list1();
		expr_free($2);
	}
|
	JPERROR expression '\n' {
		expr_number_check($2);
		JPopt = !!$2->e_value;
		list1();
		expr_free($2);
	}
|
	PUBLIC public.list '\n' {
		list1();
	}
|
	EXTRN extrn.list '\n' {
		list1();
	}
|
	MRAS_MOD '\n' {
		char *p = strchr(modstr, '\0') - 1;
		for (; p >= modstr; p--) {
			(*p)++;
			if (*p < 'Z')
				break;
			*p = 'A';
		}
		list1();
	}
|
	label.part SETSEG '\n' {
		if (relopt && segment != $2->i_value) {
			segment = $2->i_value;
			segchange = 1;
			dollarsign = seg_pos[$2->i_value];
		}
		list1();
	}
|
	INSTSET '\n' {
		z80 = $1->i_value;
		list1();
	}
|
	UNDECLARED MACRO { param_parse = 1; } parm.list '\n' {
		param_parse = 0;
		$1->i_token = MNAME;
		$1->i_pass = npass;
		$1->i_value = mfptr;
		if (keyword($1->i_string) && !driopt) {
			sprintf(detail, "Macro '%s' will override the built-in '%s'",
				$1->i_string, $1->i_string);
			errwarn(warn_general, detail);
		}
#ifdef M_DEBUG
		fprintf (stderr, "%s(%d) [UNDECLARED MACRO %s]\n",
			src_name[now_in], linein[now_in], $1->i_string);
#endif
		list1();
	}
	locals {
		mlex_list_on += !iopt;
		mfseek(mfile, (long)mfptr, 0);
		mlex($7);
		mlex_list_on -= !iopt;
		parm_number = 0;
	}
|
	label.part MNAME al { arg_state.macarg = 1; } arg.list '\n' {
#ifdef M_DEBUG
		fprintf (stderr, "%s(%d) [MNAME %s]\n",
			src_name[now_in], linein[now_in], $2->i_string);
#endif
		$2->i_uses++ ;
		arg_reset();
		parm_number = 0;
		delayed_list(dollarsign);
		expptr++;
		est = est2;
		est2 = NULL; // GWP - this may leak, but it avoids double-free crashes
		est[FLOC].value = floc;
		est[TEMPNUM].value = exp_number++;
		est[MIF].param = ifptr;
		est[REPNUM].value = 0;
		est[MSTR].param = NULL;
		floc = $2->i_value;
		mfseek(mfile, (long)floc, 0);
	}
|
	label.part REPT expression al '\n' {
		expr_reloc_check($3);
		// MRAS compat would require treating the repeat count
		// as a byte value with 0 == 256.
		at_least_once = $3->e_value > 0;

		if (at_least_once)
			delayed_list1();
		else
			list1();

		arg_reset();
	}
	locals {
		int pos = mfptr;
		mfseek(mfile, (long)mfptr, 0);
		mlex($7);
		parm_number = 0;

		if (at_least_once) {
			expptr++;
			est = est2;
			est2 = NULL;
			est[FLOC].value = floc;
			est[TEMPNUM].value = exp_number++;
			est[MIF].param = ifptr;
			est[REPNUM].value = $3->e_value - 1;
			est[MSTART].value = pos;
			est[MSTR].param = NULL;
			floc = pos;
			mfseek(mfile, (long)floc, 0);
		}
	}
|
	label.part IRPC parm.single ',' { parm_number = 0; } al arg.element arg_off '\n'
	{
		at_least_once = est2[0].param[0];

		if (at_least_once)
			delayed_list1();
		else
			list1();
	}
	locals {
		int pos = mfptr;

		mfseek(mfile, (long)mfptr, 0);
		mlex($11);

		parm_number = 0;

		if (at_least_once) {
			expptr++;
			est = est2;
			est2 = NULL;
			est[FLOC].value = floc;
			est[TEMPNUM].value = exp_number++;
			est[MIF].param = ifptr;
			est[REPNUM].value = 0;
			est[MSTART].value = pos;
			est[MSTR].param = est[0].param;
			est[0].param = malloc(2);
			est[0].param[0] = est[MSTR].param[0];
			est[0].param[1] = '\0';
			floc = pos;
			mfseek(mfile, (long)floc, 0);
		}
	}
|
	label.part IRP parm.single ',' { parm_number = 0; } al arg.element arg_off '\n'
	{
		// if the sub list is not empty
		at_least_once = est2[0].param[0] && est2[0].param[0] != ';'
			&& est2[0].param[0] != '\n';

		if (at_least_once)
			delayed_list1();
		else
			list1();
	}
	locals {
		int pos = mfptr;
		mfseek(mfile, (long)mfptr, 0);
		mlex($11);

		parm_number = 0;
		if (at_least_once) {
			expptr++;
			est = est2;
			est2 = NULL;
			est[FLOC].value = floc;
			est[TEMPNUM].value = exp_number++;
			est[MIF].param = ifptr;
			est[REPNUM].value = -1;
			est[MSTART].value = pos;
			est[MSTR].param = NULL;

			est[MARGP].ap = malloc(sizeof *est[MARGP].ap);
			est[MARGP].ap->arg = malloc(TEMPBUFSIZE);
			est[MARGP].ap->argsize = TEMPBUFSIZE;
			est[MARGP].ap->getch = str_getch;
			est[MARGP].ap->user_ptr = est[0].param;
			est[MARGP].ap->user_int = 0;
			est[MARGP].ap->user_peek = -1;
			est[MARGP].ap->peek = &est[MARGP].ap->user_peek;
			est[MARGP].ap->macarg = 0;
			est[MARGP].ap->didarg = 0;
			est[MARGP].ap->numarg = 0;

			est[0].param = est[MARGP].ap->arg;
			getarg(est[MARGP].ap);

			floc = pos;
			mfseek(mfile, (long)floc, 0);
		}
	}
|
	label.part EXITM '\n' {
		// XXX - popsi() is not safe, There is type-specific cleanup.
		//  But for now...
		// popsi() must be made safe as others use it.
		list1();
		popsi();
	}
|
	error {
		err[fflag]++;
		arg_reset();
		parm_number = 0;
		param_parse = 0;

		if (est2)
		{
			int i;
			for (i=0; i<PARMMAX; i++) {
				if (est2[i].param) {
#ifdef M_DEBUG
	fprintf (stderr, "[Freeing2 arg%u(%p)]\n", i, est2[i].param),
#endif
					free(est2[i].param);
				}
			}
			free(est2);
			est2 = NULL;
		}

		while(yychar != '\n' && yychar != '\0') yychar = yylex();
		list(dollarsign);
		yyclearin;yyerrok;
	}
;

maybecolon:
	/* empty */ { $$ = 0; }
|
	':' { $$ = 1; }
|
	':' ':' { $$ = 2; }
;

label.part:
	/*empty*/
	 {	label = $$ = NULL;	}
|
	symbol maybecolon {
		coloncnt = $2;
		itemcpy(&pristine_label, $1);
		label = coloncnt == 0 ? $1 : NULL;
		$1->i_scope |= segment;
		if ($2 == 2)
			$1->i_scope |= SCOPE_PUBLIC;

		if ($1->i_string[0] != '.')
			llseq++;

		switch($1->i_token) {
		case UNDECLARED:
			if (pass2) {
				sprintf(detail, "%s error - label '%s' not declared",
					errname[pflag], $1->i_string);
				errwarn(pflag, detail);
			}
			else {
				$1->i_token = LABEL;
				$1->i_pass = npass;
				$1->i_value = dollarsign;
			}
			break;
		case LABEL:
			if (!pass2) {
				$1->i_token = MULTDEF;
				$1->i_pass = npass;
				err[mflag]++;
			} else if ($1->i_value != dollarsign) {
				// XXX - perhaps only allow retrys if JR promotions are in play?
				if (outpass) {
					if (!passfail) {
						sprintf(detail, "%s error - label '%s' changed from $%04x to $%04x",
							errname[pflag], $1->i_string, $1->i_value, dollarsign);
						errwarn(pflag, detail);
					}
				}
				else {
					$1->i_value = dollarsign;
					passretry = 1;
				}
			}
			break;
		default:
			err[mflag]++;
			$1->i_token = MULTDEF;
			$1->i_pass = npass;
		}
	}
;

public.list:
	public.part
|
	public.list ',' public.part
;

public.part:
	symbol {
		if (!($1->i_scope & SCOPE_COMBLOCK))
			$1->i_scope |= SCOPE_PUBLIC;
		// Just being "public" does not #ifdef define a symbol.
		if (pass2) {
			if ($1->i_token == UNDECLARED) {
				sprintf(detail, "'%s' %s", $1->i_string, errname[uflag]);
				errwarn(uflag, detail);
			}
		}
	}
;

extrn.list:
	extrn.part
|
	extrn.list ',' extrn.part
;

extrn.part:
	symbol {
		if (pass2 && $1->i_scope != SCOPE_NONE && !($1->i_scope & SCOPE_EXTERNAL)) {
			fprintf(stderr, "Label scope change\n");
			err[fflag]++;
		}
		$1->i_pass = npass; // external labels seen as defined for #ifdef
		$1->i_scope |= SCOPE_EXTERNAL;
		if (pass2) {
			if ($1->i_token != UNDECLARED) {
				fprintf(stderr, "External label defined locally.\n");
				err[fflag]++;
			}
		}
	}
;

varop:
	'+' { $$ = '+'; } |
	'-' { $$ = '-'; } |
	'*' { $$ = '*'; } |
	'/' { $$ = '/'; } |
	'%' { $$ = '%'; } |
	'&' { $$ = '&'; } |
	'|' { $$ = '|'; } |
	'^' { $$ = '^'; } |
	ANDAND { $$ = ANDAND; } |
	OROR { $$ = OROR; } |
	SHL { $$ = SHL; } |
	SHR { $$ = SHR; } |
// and MROP variants just to be nice.
	MROP_ADD { $$ = '+'; } |
	MROP_SUB { $$ = '-'; } |
	MROP_MUL { $$ = '*'; } |
	MROP_DIV { $$ = '/'; } |
	MROP_MOD { $$ = '%'; } |
	MROP_AND { $$ = '&'; } |
	MROP_OR  { $$ = '|'; } |
	MROP_XOR { $$ = '^'; } |
	MROP_SHL { $$ = MROP_SHL; } |
	MROP_SHR { $$ = MROP_SHR; } |
	MROP_SHIFT { $$ = MROP_SHIFT; }
;

operation:
	NOOPERAND
		{ emit1($1->i_value, 0, 0, ET_NOARG); }
|	
	NOOPERAND expression
		{
			// XXX - maybe splitting out CPI is better?
			if (!z80 && $1->i_value == 0166641)
				emit1(0376, 0, $2, ET_BYTE);
			else
				err[fflag]++;
		}
|
	SHIFT
		{
			if (!z80 && ($1->i_value & 070) < 020)
				emit(1, E_CODE, 0, 007 | ($1->i_value & 070));
			else
				err[fflag]++;
		}
|
	JP expression
	{
		if ($2->e_parenthesized)
			errwarn(warn_general, "JP (addr) is equivalent to JP addr");

		if (z80 || $1->i_value == 0303) {
			checkjp(0, $2);
			emit(1, E_CODE16, $2, 0303);
		}
		else
			// can't optimize jump on plus
			emit(1, E_CODE16, $2, 0362);
	}
|
	CALL expression
		{	emit(1, E_CODE16, $2, 0315);	}
|
	RST	expression
	{
		// accepts rst 0-7 or rst 0,8,16,...,56
		int vec = $2->e_value;
		expr_number_check($2);
		if ((vec > 7 || vec < 0) && (vec & ~(7 << 3)))
			err[vflag]++;
		if (vec > 7) vec >>= 3;
		emit(1, E_CODE, 0, $1->i_value + ((vec & 7) << 3));
		expr_free($2);
	}
|
	ALUI8 expression
		{ emit1($1->i_value, 0, $2, ET_BYTE); }
|
	ALU_XY dxy
		{
			emit(3, E_CODE, 0, $1->i_value >> 8, $1->i_value, disp);
		}
|
	ADD expression
		{ emit1(0306, 0, $2, ET_BYTE); }
|
	ADD ACC ',' expression
		{ emit1(0306, 0, $4, ET_BYTE); }
|
	ARITHC expression
		{ emit1(0306 + ($1->i_value & 070), 0, $2, ET_BYTE); }
|
	ARITHC ACC ',' expression
		{ emit1(0306 + ($1->i_value & 070), 0, $4, ET_BYTE); }
|
	LOGICAL expression
		{
			// CP is CALL P in 8080
			if (!z80 && $1->i_value == 0270 && $1->i_string[1] == 'p')
				emit(1, E_CODE16, $2, 0364);
			else
				emit1(0306 | ($1->i_value & 070), 0, $2, ET_BYTE);
		}
|
	AND expression
		{ emit1(0306 | ($1->i_value & 070), 0, $2, ET_BYTE); }
|
	OR expression
		{ emit1(0306 | ($1->i_value & 070), 0, $2, ET_BYTE); }
|
	XOR expression
		{ emit1(0306 | ($1->i_value & 070), 0, $2, ET_BYTE); }
|
	TST expression
		{ emit(2, E_CODE8, $2, 0xED, 0x60 | $1->i_value); }
|
	LOGICAL ACC ',' expression	/* -cdk */
		{ emit1(0306 | ($1->i_value & 070), 0, $4, ET_BYTE); }
|
	AND ACC ',' expression	/* -cdk */
		{ emit1(0306 | ($1->i_value & 070), 0, $4, ET_BYTE); }
|
	OR ACC ',' expression	/* -cdk */
		{ emit1(0306 | ($1->i_value & 070), 0, $4, ET_BYTE); }
|
	XOR ACC ',' expression	/* -cdk */
		{ emit1(0306 | ($1->i_value  & 070), 0, $4, ET_BYTE); }
|
	ADD allreg
		{ emit1(0200 + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	ADD ACC ',' allreg
		{ emit1(0200 + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	ADD m
		{ emit(1, E_CODE, 0, 0206); }
|
	ARITHC allreg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	ARITHC ACC ',' allreg
		{ emit1($1->i_value + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	ARITHC m
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	LOGICAL allreg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	LOGICAL m
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	AND allreg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	OR allreg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	XOR allreg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	TST reg
		{ emit1($1->i_value + ($2 << 3), $2, 0, ET_NOARG_DISP); }
|
	LOGICAL ACC ',' allreg		/* -cdk */
		{ emit1($1->i_value + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	AND ACC ',' allreg		/* -cdk */
		{ emit1($1->i_value + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	OR ACC ',' allreg		/* -cdk */
		{ emit1($1->i_value + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	XOR ACC ',' allreg		/* -cdk */
		{ emit1($1->i_value + ($4 & 0377), $4, 0, ET_NOARG_DISP); }
|
	SHIFT reg
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG_DISP); }
|
	SHIFT_XY dxy
		{
			emit(4, E_CODE, 0, $1->i_value >> 24, $1->i_value >> 16, disp, $1->i_value);
		}
|
	SHIFT memxy ',' realreg
		{ emit1($1->i_value + ($4 & 0377), $2, 0, ET_NOARG_DISP); }
|
	INCDEC	allreg
		{ emit1($1->i_value + (($2 & 0377) << 3), $2, 0, ET_NOARG_DISP); }
|
	INRDCR	reg8
		{ emit1($1->i_value + (($2 & 0377) << 3), $2, 0, ET_NOARG_DISP); }
|
	ARITHC HL ',' bcdehlsp
		{ if ($1->i_value == 0210)
				emit(2,E_CODE,0,0355,0112+$4);
			else
				emit(2,E_CODE,0,0355,0102+$4);
		}
|
	ADD mar ',' bcdesp
		{ emitdad($2,$4); }
|
	ADD mar ',' mar
		{
			if ($2 != $4) {
				fprintf(stderr,"ADD mar, mar error\n");
				err[gflag]++;
			}
			emitdad($2,$4);
		}
|
	DAD evenreg8 { emitdad(040, $2); }
|
	ARITH16 bcdesp
		{
			emit(2, E_CODE, 0, $1->i_value >> 8, $1->i_value | $2);
		}
|
	ARITH16 mar
		{
			int dst = $1->i_value >> 8;
			int reg = $2 >> 8;
			if (!reg) reg = 0xed;

			if (dst != reg) {
				if (dst == 0xed)
					fprintf(stderr, "dadc/dsbc cannot take ix or iy\n");
				else if (dst == 0xdd)
					fprintf(stderr, "dadx cannot take hl or iy\n");
				else
					fprintf(stderr, "dady cannot take hl or ix\n");
				err[gflag]++;
			}
			emit(2, E_CODE, 0, $1->i_value >> 8, $1->i_value | $2);
		}
|
	INCDEC evenreg
		{ emit1((($1->i_value & 1) << 3) + ($2 & 0377) + 3, $2, 0, ET_NOARG); }
|
	INXDCX evenreg8
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG); }
|
	MLT bcdehl
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG); }
|
	PUSHPOP pushable
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG); }
|
	PUSHPOP pushable8
		{ emit1($1->i_value + ($2 & 0377), $2, 0, ET_NOARG); }
|
	BIT expression
		{
			if (strcmp($1->i_string, "set") == 0 && label) {
				// Clear error that label definition will have been set
				err[mflag] = 0;
				itemcpy(label, &pristine_label);
				do_defl(label, $2, 0);
				list_dollarsign = 0;
				list_addr = label->i_value;
			}
			else {
				err[fflag]++;
			}
		}
|
	BIT expression ',' reg
		{
			int bit = $2->e_value;
			expr_number_check($2);
			expr_free($2);
			if (bit < 0 || bit > 7)
				err[vflag]++;
			emit1($1->i_value + ((bit & 7) << 3) + ($4 & 0377), $4, 0, ET_NOARG_DISP);
		}
|
	BIT_XY expression ',' dxy
		{
			int bit = $2->e_value;
			expr_number_check($2);
			expr_free($2);
			if (bit < 0 || bit > 7)
				err[vflag]++;
			emit(4, E_CODE, 0, $1->i_value >> 24, $1->i_value >> 16, disp,
				$1->i_value | (bit << 3));
		}
|
	BIT expression ',' memxy ',' realreg
		{
			int bit = $2->e_value;
			expr_number_check($2);
			expr_free($2);
			if (bit < 0 || bit > 7)
				err[vflag]++;
			emit1($1->i_value + ((bit & 7) << 3) + ($6 & 0377), $4, 0, ET_NOARG_DISP);
		}
|
	JP condition ',' expression
	{
		checkjp($2, $4);
		emit(1, E_CODE16, $4, 0302 + $2);
	}
|
	JUMP8 expression
	{
		checkjp($1->i_value, $2);
		emit(1, E_CODE16, $2, $1->i_value);
	}
|
	JP '(' mar ')'
		{ emit1(0351, $3, 0, ET_NOARG); }
|
	JP mar // allow JP HL|IX|IY without brackets.
		{ emit1(0351, $2, 0, ET_NOARG); }
|
	CALL condition ',' expression
		{ emit(1, E_CODE16, $4, 0304 + $2); }
|
	CALL8 expression
		{ emit(1, E_CODE16, $2, $1->i_value); }
|
	JR expression
		{ emitjr(030,$2); }
|
	JR spcondition ',' expression
		{ emitjr($1->i_value + $2, $4); }
|
	JR_COND expression
		{ emitjr($1->i_value, $2); }
|
	DJNZ expression
		{ emitjr($1->i_value, $2); }
|
	RET
		{ emit(1, E_CODE, 0, $1->i_value); }
|
	RET condition
		{ emit(1, E_CODE, 0, 0300 + $2); }
|
	LD allreg ',' allreg
		{
			// Many constraints on byte access to IX/IY registers.
			if (($2 | $4) >> 16) {
				int a = $2;
				int b = $4;

				// Only ixh,ixh; ixh,ixl; ixl,ixh; ixl,ixl allowed.
				if (a >> 16 && b >> 16) {
					if (a >> 8 != b >> 8) {
						fprintf(stderr, "LD cannot move between ix and iy\n");
						err[gflag]++;
					}
				}
				else {
					int c = b >> 16 ? a : b;
					// No access to h, l, (hl), (ix), (iy)
					if (c == 4 || c == 5 || (c & 0xff) == 6) {
						fprintf(stderr, "LD cannot combine i/xy/lh and h,l,(hl),(ix) or (iy).\n");
						err[gflag]++;
					}
				}
			}

			if (($2 & 0377) == 6 && ($4 & 0377) == 6) {
				fprintf(stderr,"LD reg, reg error: can't do memory to memory\n");
				err[gflag]++;
			}
			emit1(0100 + (($2 & 7) << 3) + ($4 & 7), $2 | $4, 0, ET_NOARG_DISP);
		}
|
	LD_XY realreg ',' dxy
		{
			emit(3, E_CODE, 0, $1->i_value >> 8, $1->i_value | ($2 << 3), disp);
		}
|
	ST_XY realreg ',' dxy
		{
			emit(3, E_CODE, 0, $1->i_value >> 8, $1->i_value | $2, disp);
		}
|
	MOV reg8 ',' reg8
		{
			if ($2 == 6 && $4 == 6) err[gflag]++;
			emit1(0100 + (($2 & 7) << 3) + ($4 & 7),$2 | $4, 0, ET_NOARG_DISP);
		}
|
	LD allreg ',' noparenexpr
		{ emit1(6 + (($2 & 0377) << 3), $2, $4, ET_BYTE); }
|
	MV_XY expression ',' dxy
		{
			emit(3, E_CODE8, $2, $1->i_value >> 8, $1->i_value, disp);
		}
|
	MVI reg8 ',' expression
		{ emit1(6 + (($2 & 0377) << 3), $2, $4, ET_BYTE); }
|
	LD allreg ',' '(' RP ')'
		{	if ($2 != 7) {
				fprintf(stderr,"LD reg, (RP) error\n");
				err[gflag]++;
			}
			else emit(1, E_CODE, 0, 012 + $5->i_value);
		}
|
	LDAX realreg
		{
			if ($2 != 0 && $2 != 2) err[gflag]++;
			emit(1, E_CODE, 0, 012 + ($2 << 3));
		}
|
	LD allreg ',' parenexpr
		{
			if ($2 != 7) {
				fprintf(stderr,"LD reg, (expr) error: A only valid destination\n");
				err[gflag]++;
			}
			else {
				expr_word_check($4);
				emit(1, E_CODE16, $4, 072);
			}
		}
|
	LDA expression
		{
			expr_word_check($2);
			emit(1, E_CODE16, $2, 072);
		}
|
	LD '(' RP ')' ',' ACC
		{ emit(1, E_CODE, 0, 2 + $3->i_value); }
|
	STAX realreg
		{
			if ($2 != 0 && $2 != 2) err[gflag]++;
			emit(1, E_CODE, 0, 2 + ($2 << 3));
		}
|
	LD parenexpr ',' ACC
		{
			expr_word_check($2);
			emit(1, E_CODE16, $2, 062);
		}
|
	STA expression
		{
			expr_word_check($2);
			emit(1, E_CODE16, $2, 062);
		}
|
	LD allreg ',' MISCREG
		{
			if ($2 != 7) {
				fprintf(stderr,"LD reg, MISCREG error: A only valid destination\n");
				err[gflag]++;
			}
			else emit(2, E_CODE, 0, 0355, 0127 + $4->i_value);
		}
|
	LD MISCREG ',' ACC
		{ emit(2, E_CODE, 0, 0355, 0107 + $2->i_value); }
|
	LD evenreg ',' noparenexpr
		{
			expr_word_check($4);
			emit1(1 + ($2 & 060), $2, $4, ET_WORD);
		}
|
	LXI evenreg8 ',' expression
		{
			expr_word_check($4);
			emit1(1 + ($2 & 060), $2, $4, ET_WORD);
		}
|
	LD evenreg ',' parenexpr
		{
			expr_word_check($4);
			if (($2 & 060) == 040)
				emit1(052, $2, $4, ET_WORD);
			else
				emit(2, E_CODE16, $4, 0355, 0113 + $2);
		}
|
	LHLD expression
		{
			expr_word_check($2);
			emit1(052, 040, $2, ET_WORD);
		}
|
	LD parenexpr ',' evenreg
		{
			expr_word_check($2);
			if (($4 & 060) == 040)
				emit1(042, $4, $2, ET_WORD);
			else
				emit(2, E_CODE16, $2, 0355, 0103 + $4);
		}
|
	SHLD expression
		{
			expr_word_check($2);
			emit1(042, 040, $2, ET_WORD);
		}
|
	LD evenreg ',' mar
		{
			if ($2 != 060) {
				fprintf(stderr,"LD evenreg error\n");
				err[gflag]++;
			}
			else
				emit1(0371, $4, 0, ET_NOARG);
		}
|
	LDST16 expression
		{
			expr_word_check($2);
			emit(2, E_CODE16, $2, $1->i_value >> 8, $1->i_value);
		}
|
	EX RP ',' HL
		{
			if ($2->i_value != 020) {
				fprintf(stderr,"EX RP, HL error\n");
				err[gflag]++;
			}
			else
				emit(1, E_CODE, 0, 0353);
		}
|
	EX AF ',' AFp
		{ emit(1, E_CODE, 0, 010); }
|
	EX '(' SP ')' ',' mar
		{ emit1(0343, $6, 0, ET_NOARG); }
|
	TK_IN realreg ',' parenexpr
		{
			if ($2 != 7) {
				fprintf(stderr,"IN reg, (expr) error\n");
				err[gflag]++;
			}
			else	{
				if ($4->e_value < 0 || $4->e_value > 255)
					err[vflag]++;
				emit(1, E_CODE8, $4, $1->i_value);
			}
		}
|
	TK_IN0 realreg ',' parenexpr
		{
			if ($4->e_value < 0 || $4->e_value > 255)
				err[vflag]++;
			emit(2, E_CODE8, $4, $1->i_value >> 8,
				$1->i_value | ($2 << 3));
		}
|
	TK_IN0 parenexpr
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;
			emit(2, E_CODE8, $2, $1->i_value >> 8,
				$1->i_value | (6 << 3));
		}
|
	TK_IN expression
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;
			emit(1, E_CODE8, $2, $1->i_value);
		}
|
	TK_IN realreg ',' '(' TK_C ')'
		{ emit(2, E_CODE, 0, 0355, 0100 + ($2 << 3)); }
|
	INP realreg
		{ emit(2, E_CODE, 0, 0355, 0100 + ($2 << 3)); }
|
	TK_IN TK_F ',' '(' TK_C ')'
		{ emit(2, E_CODE, 0, 0355, 0160); }
|
	TK_IN '(' TK_C ')'
		{ emit(2, E_CODE, 0, 0355, 0160); }
|
	TK_OUT parenexpr ',' ACC
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;
			emit(1, E_CODE8, $2, $1->i_value);
		}
|
	TK_OUT0 parenexpr ',' realreg
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;
			emit(2, E_CODE8, $2, $1->i_value >> 8,
				$1->i_value | ($4 << 3));
		}
|
	TK_OUT expression
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;
			emit(1, E_CODE8, $2, $1->i_value);
		}
|
	TK_OUT '(' TK_C ')' ',' realreg
		{ emit(2, E_CODE, 0, 0355, 0101 + ($6 << 3)); }
|
	OUTP realreg
		{ emit(2, E_CODE, 0, 0355, 0101 + ($2 << 3)); }
|
	TK_OUT '(' TK_C ')' ',' expression
		{
			expr_number_check($6);
			if ($6->e_value != 0) {
				fprintf(stderr, "Can only output 0 to port C with OUT\n");
				err[vflag]++;
			}
			expr_free($6);

			emit(2, E_CODE, 0, 0355, 0101 + (6 << 3));
		}
|
	TSTIO parenexpr
		{
			if ($2->e_value < 0 || $2->e_value > 255)
				err[vflag]++;

			emit(2, E_CODE8, $2, $1->i_value >> 8, $1->i_value);
		}
|
	IM expression
		{
			int im = $2->e_value;
			expr_number_check($2);
			expr_free($2);
			if (im > 2 || im < 0)
				err[vflag]++;
			else
				emit(2, E_CODE, 0, $1->i_value >> 8, $1->i_value + ((im + (im > 0)) << 3));
		}
|
	PHASE expression
		{
			expr_number_check($2);
			if (phaseflag) {
				err[oflag]++;
			} else {
				phaseflag = 1;
				phdollar = dollarsign;
				dollarsign = $2->e_value;
				phbegin = dollarsign;
			}
			expr_free($2);
		}
|
	DEPHASE
		{
			if (!phaseflag) {
				err[oflag]++;
			} else {
				phaseflag = 0;
				dollarsign = phdollar + dollarsign - phbegin;
			}
		}
|
	ORG expression
		{
			expr_reloc_check($2);
			// Cannot org to the other segment (but absolute values are OK)
			if (relopt && segment &&
				($2->e_scope & SCOPE_SEGMASK) &&
				($2->e_scope & SCOPE_SEGMASK) != segment)
			{
				err[rflag]++;
			}
			if (phaseflag) {
				err[oflag]++;
				dollarsign = phdollar + dollarsign - phbegin;
				phaseflag = 0;
			}
			if ($2->e_value-dollarsign) {
				flushbin();
				flushoth();
				olddollar = $2->e_value;
				oldothdollar = $2->e_value;
				dollarsign = $2->e_value;
				emit_addr = $2->e_value;
				seg_pos[segment] = dollarsign;
				if (seg_pos[segment] > seg_size[segment]) {
					seg_size[segment] = seg_pos[segment];
					if (segment == SEG_COMMON && cur_common)
						cur_common->i_value = seg_size[segment];
				}
				putrelcmd(RELCMD_SETLOC);
				putrelsegref(segment, seg_pos[segment]);
				segchange = 0;
			}
			expr_free($2);
		}
|
	ASSERT expression
		{
			list_dollarsign = 0;
			list_addr = $2->e_value;
			expr_number_check($2);
			if (outpass && !$2->e_value)
			{
				err[aflag]++;
			}
			expr_free($2);
		}
|
	TSTATE expression
		{
			list_dollarsign = 0;
			list_addr = $2->e_value;
			expr_number_check($2);
			tstates = $2->e_value;
			tstatesum[emit_addr] = tstates;
			expr_free($2);
		}
|
	SETOCF expression
		{
			list_dollarsign = 0;
			list_addr = $2->e_value;
			expr_number_check($2);
			ocf = $2->e_value;
			ocfsum[emit_addr] = ocf;
			expr_free($2);
		}
|
	DEFB { full_exprs = 1; } db.list { full_exprs = 0; }
|
	DEFW { full_exprs = 1; } dw.list { full_exprs = 0; }
|
	DEF3 { full_exprs = 1; } d3.list { full_exprs = 0; }
|
	DEFD { full_exprs = 1; } dd.list { full_exprs = 0; }
|
	ENDM
;

parm.list:
|
	parm.element
|
	parm.list ',' parm.element
;

parm.single: { param_parse = 1; } parm.element { param_parse = 0; };

maybeocto: { $$ = 0; } | '#' { $$ = 1; };

parm.element:
	maybeocto MPARM
		{
			if (parm_number >= PARMMAX)
				error("Too many parameters");
			$2->i_value = parm_number++;
			$2->i_scope = $1;
			$2->i_chain = 0;
		}
;

locals: local_decls {
		static char macpush[LINEBUFFERSIZE];
		// Because of locals the parser has to look ahead.
		// We'll have buffered that as we usually do so just a
		// matter of picking that up and cancelling any look-ahead.
		*lineptr = '\0';
		strcpy(macpush, linebuf);
		lineptr = linebuf;
		peekc = NOPEEK;
		yychar = YYEMPTY;
		$$ = macpush;
	}
;

local_decls:
|
	local_decls LOCAL { param_parse = 1; } local.list '\n' { param_parse = 0; list1(); }
;

local.list:
|
	local.element
|
	local.list ',' local.element
;

local.element:
	MPARM
		{
			if (parm_number >= PARMMAX)
				error("Too many parameters");
			$1->i_value = parm_number++;
			$1->i_scope = 0;
			$1->i_chain = 1;
		}
;

arg.list:
	/* empty */
|
	arg.element
|
	arg.list ',' arg.element
;


arg.element:
	ARG
		{
			cp = malloc(strlen(tempbuf)+1);
#ifdef M_DEBUG
			fprintf (stderr, "[Arg%u(%p): %s]\n", parm_number, cp, tempbuf);
#endif
			est2[parm_number++].param = cp;
			strcpy(cp, tempbuf);
		}
|
	'%' { arg_flag = 0; } expression
		{
			arg_flag = 1;
			expr_reloc_check($3);
			sprintf(tempbuf, "%d", $3->e_value);
			est2[parm_number++].param = strdup(tempbuf);
			expr_free($3);
		}
;

allreg:
	reg
|
	ixylhreg
;
reg:
	realreg
|
	mem
;
ixylhreg:
	IXYLH
		{
			$$ = $1->i_value;
		}
;
reg8:
	realreg
|
	m
;
m:
	COND { if ($1->i_value != 070) err[gflag]++; $$ = 6; }
;
realreg:
	REGNAME
		{
			$$ = $1->i_value;
		}
|
	ACC
		{
			$$ = $1->i_value;
		}
|
	TK_C
		{
			$$ = $1->i_value;
		}
;
mem:
	'(' HL ')'
		{
			$$ = 6;
		}
|
	memxy
;
memxy:
	'(' INDEX dxy ')'
		{
			$$ = ($2->i_value & 0177400) | 6;
		}
|
	'(' INDEX ')'
		{
			disp = 0;
			$$ = ($2->i_value & 0177400) | 6;
		}
|
	dxy '(' INDEX ')'
		{
			$$ = ($3->i_value & 0177400) | 6;
		}
;
dxy:	expression
		{
			expr_number_check($1);
			disp = $1->e_value;
			expr_free($1);
			if (disp > 127 || disp < -128)
				err[vflag]++;
		}
;
evenreg:
	bcdesp
|
	mar
;
evenreg8:
	realreg	{ if ($1 & 1) err[gflag]++; $$ = $1 << 3; }
|
	SP { $$ = $1->i_value; }
;
pushable:
	RP
		{
			$$ = $1->i_value;
		}
|
	AF
		{
			$$ = $1->i_value;
		}
|
	AFp
		{
			$$ = 060;
		}
|
	mar
;
pushable8:
	realreg	{ if (($1 & 1) && $1 != 7) err[gflag]++; $$ = ($1 & 6) << 3; }
|
	PSW { $$ = $1->i_value; }
;
bcdesp:
	RP
		{
			$$ = $1->i_value;
		}
|
	SP
		{
			$$ = $1->i_value;
		}
;
bcdehlsp:
	bcdesp
|
	HL
		{
			$$ = $1->i_value;
		}
;
bcdehl:
	RP
		{
			$$ = $1->i_value;
		}
|
	HL
		{
			$$ = $1->i_value;
		}
;
mar:
	HL
		{
			$$ = $1->i_value;
		}
|
	INDEX
		{
			$$ = $1->i_value;
		}
;
aliasable: REGNAME | ACC | TK_C | AF | PSW | RP | HL | INDEX | SP | COND;

condition:
	spcondition
|
	COND
		{
			$$ = $1->i_value;
		}
;
spcondition:
	SPCOND
		{
			$$ = $1->i_value;
		}
|
	TK_C
		{	$$ = 030;	}
;
db.list:
	db.list.element
|
	db.list ',' db.list.element
;
db.list.element:
	STRING
		{
			cp = $1;
			while (*cp != '\0')
				emit(1,E_DATA,expr_num(*cp++));
		}
|
	expression
		{
			// Allow a single "TWOCHAR" value...
			if ($1->e_scope & SCOPE_TWOCHAR) {
				emit(1, E_DATA, expr_num($1->e_value & 0xff));
				emit(1, E_DATA, expr_num($1->e_value >> 8));
			}
			else {
				if (is_number($1) && ($1->e_value < -128 || $1->e_value > 255)) {
					err[vflag]++;
				}

				emit(1, E_DATA, $1);
			}
		}
;


dw.list:
	dw.list.element
|
	dw.list ',' dw.list.element
;


dw.list.element:
	expression
		{
			if ($1->e_value < -32768 || $1->e_value > 65535) {
				err[vflag]++;
			}
			emit(2, E_DATA, $1);
		}
;

d3.list:
	d3.list.element
|
	d3.list ',' d3.list.element
;


d3.list.element:
	expression
		{
			if ($1->e_value < -0x800000 || $1->e_value > 0xffffff) {
				err[vflag]++;
			}
			emit(3, E_DATA, $1);
		}
;

dd.list:
	dd.list.element
|
	dd.list ',' dd.list.element
;


dd.list.element:
	expression
		{
			// Can't check overflow as I only have 32 bit ints.
			emit(4, E_DATA, $1);
		}
;

expression:
	parenexpr
|
	noparenexpr
;

parenexpr:
	'(' expression ')'
		{	$$ = $2; $$->e_parenthesized = 1;	}
;

noparenexpr:
/*
  This rule commented out because it doesn't actually produce expression
  errors but instead stops parsing.  That's because it doesn't clear the
  parse error with yyerrok.  I tried to add that but couldn't come up with
  a recovery that works in all cases.  In fact, it often infinite loops.
	error
		{
			err[eflag]++;
			$$ = expr_num(0);
		}
|
*/
	LABEL
		{
			$$ = expr_var($1);
			$1->i_uses++;
		}
|
	NUMBER
		{
			$$ = expr_num($1);
		}
|
	ONECHAR
		{
			$$ = expr_num($1);
		}
|
	TWOCHAR
		{
			$$ = expr_num($1);
			// Mark as twochar for db.list hackery
			$$->e_scope |= SCOPE_TWOCHAR;
		}
|
	EQUATED
		{
			$$ = expr_var($1);
		}
|
	WASEQUATED
		{
			$$ = expr_var($1);
		}
|
	DEFLED
		{
			$$ = expr_var($1);
		}
|
	COMMON
		{
			$$ = expr_var($1);
		}
|
	'$'
		{
			$$ = expr_num(dollarsign + emitptr - emitbuf);
			$$->e_scope = segment;
		}
|
	UNDECLARED
		{
			int chkext = 1;
			$$ = expr_alloc();
			$$->e_token = 'u';
			$$->e_item = $1;
			$$->e_scope = $1->i_scope;
			$$->e_value = 0;

			if (!nmnvopt) {
				// Allow use of JMP, RET, etc. as values.
				struct item *i = keyword($1->i_string);
				if (item_is_verb(i)) {
					chkext = 0;
					$1 = i;
					$$->e_item = i;
					$$->e_value = item_value(i);
				}
			}

			if (chkext && !($1->i_scope & SCOPE_EXTERNAL)) {
				sprintf(detail, "'%s' %s", $1->i_string, errname[uflag]);
				// Would be nice to add to list of undeclared errors
				errwarn(uflag, detail);
			}
		}
|
	MULTDEF
		{
			$$ = expr_alloc();
			$$->e_token = 'm';
			$$->e_item = $1;
			$$->e_scope = $1->i_scope;
			// Use the current value.  Harmless enough as this
			// will normally error out yet vital to allow
			// "var set var+1" to function.
			$$->e_value = $1->i_value;
		}
|
	NUL { raw = 2; } RAWTOKEN
		{
			$$ = expr_num(tempbuf[0] ? -1 : 0);
		}
|	expression '+' expression { $$ = expr_mk($1, '+', $3); }
|	expression '-' expression { $$ = expr_mk($1, '-', $3); }
|	expression '/' expression { $$ = expr_mk($1, '/', $3); }
|	expression '*' expression { $$ = expr_mk($1, '*', $3); }
|	expression '%' expression { $$ = expr_mk($1, '%', $3); }
|	expression '&' expression { $$ = expr_mk($1, '&', $3); }
|	expression AND expression { $$ = expr_mk($1, '&', $3); }
|	expression '|' expression { $$ = expr_mk($1, '|', $3); }
|	expression OR expression  { $$ = expr_mk($1, '|', $3); }
|	expression '^' expression { $$ = expr_mk($1, '^', $3); }
|	expression XOR expression { $$ = expr_mk($1, '^', $3); }
|	expression SHL expression { $$ = expr_mk($1, SHL, $3); }
|	expression SHR expression { $$ = expr_mk($1, SHR, $3); }
|	expression '<' expression { $$ = expr_mk($1, '<', $3); }
|	expression LT  expression { $$ = expr_mk($1, '<', $3); }
|	expression '=' expression { $$ = expr_mk($1, '=', $3); }
|	expression EQ  expression { $$ = expr_mk($1, '=', $3); }
|	expression '>' expression { $$ = expr_mk($1, '>', $3); }
|	expression GT  expression { $$ = expr_mk($1, '>', $3); }
|	expression LE  expression { $$ = expr_mk($1, LE, $3); }
|	expression NE  expression { $$ = expr_mk($1, NE, $3); }
|	expression GE  expression { $$ = expr_mk($1, GE, $3); }
|	expression ANDAND expression { $$ = expr_mk($1, ANDAND, $3); }
|	expression OROR expression { $$ = expr_mk($1, OROR, $3); }

|	expression MROP_ADD expression { $$ = expr_mk($1, '+', $3); }
|	expression MROP_SUB expression { $$ = expr_mk($1, '-', $3); }
|	expression MROP_DIV expression { $$ = expr_mk($1, '/', $3); }
|	expression MROP_MUL expression { $$ = expr_mk($1, '*', $3); }
|	expression MROP_MOD expression { $$ = expr_mk($1, '%', $3); }
|	expression MROP_AND expression { $$ = expr_mk($1, '&', $3); }
|	expression MROP_OR expression  { $$ = expr_mk($1, '|', $3); }
|	expression MROP_XOR expression { $$ = expr_mk($1, '^', $3); }
|	expression MROP_SHL expression { $$ = expr_mk($1, SHL, $3); }
|	expression MROP_SHR expression { $$ = expr_mk($1, SHR, $3); }
|	expression MROP_LT expression { $$ = expr_mk($1, '<', $3); }
|	expression MROP_EQ expression { $$ = expr_mk($1, '=', $3); }
|	expression MROP_GT expression { $$ = expr_mk($1, '>', $3); }
|	expression MROP_LE expression { $$ = expr_mk($1, LE, $3); }
|	expression MROP_NE expression { $$ = expr_mk($1, NE, $3); }
|	expression MROP_GE expression { $$ = expr_mk($1, GE, $3); }
|
	expression MROP_SHIFT expression
		{
			if ($3->e_value < 0) {
				$3->e_value = -$3->e_value;
				$$ = expr_mk($1, SHR, $3);
			}
			else
				$$ = expr_mk($1, SHL, $3);
		}
|
	expression '?' expression ':' expression
		{
			expr_number_check($1);
			if ($1->e_value) {
				$$ = $3;
				expr_free($5);
			}
			else {
				$$ = $5;
				expr_free($3);
			}
			expr_free($1);
		}
|
	'[' expression ']'
		{	$$ = $2;	}
|
	'~' expression
		{	$$ = expr_op($2, '~', 0, ~$2->e_value);	}
|
	MROP_NOT expression
		{	$$ = expr_op($2, '~', 0, ~$2->e_value);	}
|
	'!' expression
		{	$$ = expr_op($2, '!', 0, !$2->e_value * trueval);	}
|
	'+' expression %prec UNARY
		{	$$ = $2; /* no effect */	}
|
	MROP_ADD expression
		{	$$ = $2; /* no effect */	}
|
	'-' expression %prec UNARY
		{	$$ = expr_op($2, '-', 0, -$2->e_value);	}
|
	MROP_SUB expression
		{	$$ = expr_op($2, '-', 0, -$2->e_value);	}
|
	T expression %prec UNARY
		{
			expr_reloc_check($2);
			$$ = expr_num(tstatesum[phaseaddr($2->e_value)]);
			expr_free($2);
		}
|
	TILO expression %prec UNARY
		{
			int low;
			expr_reloc_check($2);
			get_tstates(memory + phaseaddr($2->e_value), &low, 0, 0, 0);
			$$ = expr_num(low);
			expr_free($2);
		}
|
	TIHI expression %prec UNARY
		{
			int high;
			expr_reloc_check($2);
			get_tstates(memory + phaseaddr($2->e_value), 0, &high, 0, 0);
			$$ = expr_num(high);
			expr_free($2);
		}
|
	OCF expression %prec UNARY
		{
			expr_reloc_check($2);
			$$ = expr_num(ocfsum[phaseaddr($2->e_value)]);
			expr_free($2);
		}
|
	LOW expression %prec UNARY
		{
			$$ = expr_op($2, LOW, 0, $2->e_value & 0xff);
		}
|
	MROP_LOW expression
		{
			$$ = expr_op($2, LOW, 0, $2->e_value & 0xff);
		}
|
	HIGH expression %prec UNARY
		{
			$$ = expr_op($2, HIGH, 0, ($2->e_value >> 8) & 0xff);
		}
|
	MROP_HIGH expression
		{
			$$ = expr_op($2, HIGH, 0, ($2->e_value >> 8) & 0xff);
		}
;

symbol:
	UNDECLARED
|
	LABEL
|
	MULTDEF
|
	EQUATED
|
	WASEQUATED
|
	DEFLED
|
	COMMON
;


al:
	{ int i;
		if (expptr >= MAXEXP)
			error("Macro expansion level too deep");
		est2 = (union exprec *) malloc((PARMMAX + PAREXT) * sizeof *est2);
		expstack[expptr] = est2;
		for (i=0; i<PARMMAX; i++)
			est2[i].param = 0;
		arg_start();
	}
;


arg_on:
	{	arg_start();	}
;

arg_off:
	{	arg_reset();	}
;

mras_undecl_on:
	{	if (mras) mras_undecl_ok = 1; }
;

mras_undecl_off:
	{	if (mras) mras_undecl_ok = 0; }
;

%%
/*extern int	yylval;*/

#define F_END	0
#define OTHER	1
#define SPACE	2
#define DIGIT	3
#define LETTER	4
#define STARTER 5
#define DOLLAR	6


/*
 *  This is the table of character classes.  It is used by the lexical
 *  analyser. (yylex())
 */
char	charclass[] = {
	F_END,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	SPACE,	OTHER,	OTHER,	OTHER,	SPACE,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,
	SPACE,	OTHER,	OTHER,	OTHER,	DOLLAR,	OTHER,	OTHER,	OTHER,	//  !"#$%&'
	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	STARTER,OTHER,	// ()*+,-./
	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	// 01234567
	DIGIT,	DIGIT,	OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	STARTER,// 89:;<=>?
	STARTER,LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// @ABCDEFG
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// HIJKLMNO
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// PQRSTUVW
	LETTER, LETTER, LETTER, OTHER,	OTHER,	OTHER,	OTHER,	LETTER,	// XYZ[\]^_
	OTHER,	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// `abcdefg
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// hijklmno
	LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER, LETTER,	// pqrstuvw
	LETTER, LETTER, LETTER, OTHER,	OTHER,	OTHER,	OTHER,	OTHER,	// xyz{|}~
};
#define CHARCLASS(x) charclass[(x) & 0xff]


/*
 *  the following table tells which characters are parts of numbers.
 *  The entry is non-zero for characters which can be parts of numbers.
 */
char	numpart[] = {
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',
	'8',	'9',	0,	0,	0,	0,	0,	0,
	0,	'A',	'B',	'C',	'D',	'E',	'F',	0,
	'H',	0,	0,	0,	0,	0,	0,	'O',
	0,	'Q',	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	'a',	'b',	'c',	'd',	'e',	'f',	0,
	'h',	0,	0,	0,	0,	0,	0,	'o',
	0,	'q',	0,	0,	0,	0,	0,	0,
	'x',	0,	0,	0,	0,	0,	0,	0,
	0};




/*
 *  the following table is a list of assembler mnemonics;
 *  for each mnemonic the associated machine-code bit pattern
 *  and symbol type are given.
 *
 *  The i_uses field is overloaded to indicate the possible uses for
 *  a token.
 */

#define VERB	(1)	/* opcode or psuedo-op */
#define I8080	(2)	/* used in 8080 instructions */
#define Z80	(4)	/* used in Z80 instructions */
#define UNDOC	(8)	/* used only in undocumented instructions */
#define TERM	(16)	/* can appear in expressions (not all marked) */
#define ZNONSTD	(32)	/* non-standard Z-80 mnemonic (probably TDL or DRI) */
#define COL0	(64)	/* will always be recognized in logical column 0 */
#define MRASOP	(128)	/* dig operator out of identifiers */
#define Z180	(256)	/* used in Z180 (HD64180) instructions */

struct	item	keytab[] = {
	{"*include",	PSINC,	ARGPSEUDO,	VERB | COL0 },
	{"*list",	0,	LIST,		VERB | COL0 },
	{"*mod",	0,	MRAS_MOD,	VERB },
	{".8080",	0,	INSTSET,	VERB },
	{"a",		7,	ACC,		I8080 | Z80 },
	{"aci",		0316,	ALUI8,		VERB | I8080 },
	{"adc",		0210,	ARITHC,		VERB | I8080 | Z80  },
	{"adcx",	0xdd8e,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"adcy",	0xfd8e,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"add",		0200,	ADD,		VERB | I8080 | Z80  },
	{"addx",	0xdd86,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"addy",	0xfd86,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"adi",		0306,	ALUI8,		VERB | I8080 },
	{"af",		060,	AF,		Z80 },
	{"ana",		0240,	ARITHC,		VERB | I8080},
	{"and",		0240,	AND,		VERB | Z80 | TERM },
	{".and.",	0,	MROP_AND,	TERM | MRASOP },
	{"andx",	0xdda6,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"andy",	0xfda6,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"ani",		0346,	ALUI8,		VERB | I8080 },
	{".ascii",	0,	DEFB,		VERB },
	{".aseg",	SEG_ABS,SETSEG,		VERB },
	{".aset",	0,	DEFL,		VERB },
	{".assert",	0,	ASSERT,		VERB },
	{"b",		0,	REGNAME,	I8080 | Z80 },
	{"bc",		0,	RP,		Z80 },
	{"bit",		0145500,BIT,		VERB | Z80 },
	{"bitx",	0xddcb0046,BIT_XY,	VERB | Z80 | ZNONSTD },
	{"bity",	0xfdcb0046,BIT_XY,	VERB | Z80 | ZNONSTD },
	{".block",	0,	DEFS,		VERB },
	{".byte",	0,	DEFB,		VERB },
	{".bytes",	0,	DC,		VERB },
	{"c",		1,	TK_C,		I8080 | Z80 },
	{"call",	0315,	CALL,		VERB | I8080 | Z80 },
	{"cc",		0334,	CALL8,		VERB | I8080 },
	{"ccd",		0xeda9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"ccdr",	0xedb9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"ccf",		077,	NOOPERAND,	VERB | Z80 },
	{"cci",		0xeda1,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"ccir",	0xedb1,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"cm",		0374,	CALL8,		VERB | I8080 },
	{"cma",		057,	NOOPERAND,	VERB | I8080 },
	{"cmc",		077,	NOOPERAND,	VERB | I8080 },
	{"cmp",		0270,	LOGICAL,	VERB | I8080 },
	{"cmpx",	0xddbe,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"cmpy",	0xfdbe,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"cnc",		0324,	CALL8,		VERB | I8080 },
	{"cnz",		0304,	CALL8,		VERB | I8080 },
	{".comment",	SPCOM,	SPECIAL,	VERB },
	{".common",	PSCMN,	ARGPSEUDO,	VERB },
	{".cond",	0,	IF_TK,		VERB },
	{"cp",		0270,	LOGICAL,	VERB | I8080 | Z80 },
	{"cpd",		0166651,NOOPERAND,	VERB | Z80 },
	{"cpdr",	0166671,NOOPERAND,	VERB | Z80 },
	{"cpe",		0354,	CALL8,		VERB | I8080 },
	{"cpi",		0166641,NOOPERAND,	VERB | I8080 | Z80 },
	{"cpir",	0166661,NOOPERAND,	VERB | Z80 },
	{"cpl",		057,	NOOPERAND,	VERB | Z80 },
	{"cpo",		0344,	CALL8,		VERB | I8080 },
	{".cseg",	SEG_CODE,SETSEG,	VERB },
	{"cz",		0314,	CALL8,		VERB | I8080 },
	{"d",		2,	REGNAME,	I8080 | Z80 },
	{".d3",		0,	DEF3,		VERB },
	{"daa",		0047,	NOOPERAND,	VERB | I8080 | Z80 },
	{"dad",		9,	DAD,		VERB | I8080 },
	{"dadc",	0xed4a,	ARITH16,	VERB | Z80 | ZNONSTD },
	{"dadx",	0xdd09,	ARITH16,	VERB | Z80 | ZNONSTD },
	{"dady",	0xfd09,	ARITH16,	VERB | Z80 | ZNONSTD },
	{".db",		0,	DEFB,		VERB },
	{".dc",		0,	DC,		VERB },
	{"dcr",		5,	INRDCR,		VERB | I8080 },
	{"dcrx",	0xdd35,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"dcry",	0xfd35,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"dcx",		0xb,	INXDCX,		VERB | I8080 },
	{"dcxix",	0xdd2b,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"dcxiy",	0xfd2b,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"de",		020,	RP,		Z80 },
	{"dec",		5,	INCDEC,		VERB | I8080 | Z80 },
	{".def3",	0,	DEF3,		VERB },
	{".defb",	0,	DEFB,		VERB },
	{".defd",	0,	DEFD,		VERB },
	{".defl",	0,	DEFL,		VERB },
	{".defm",	0,	DEFB,		VERB },
	{".defs",	0,	DEFS,		VERB },
	{".defw",	0,	DEFW,		VERB },
	{".dephase",	0,	DEPHASE,	VERB },
	{"di",		0363,	NOOPERAND,	VERB | I8080 | Z80 },
	{"djnz",	020,	DJNZ,		VERB | Z80 },
	{".dm",		0,	DEFB,		VERB },
	{".ds",		0,	DEFS,		VERB },
	{"dsbc",	0xed42,	ARITH16,	VERB | Z80 | ZNONSTD },
	{".dseg",	SEG_DATA,SETSEG,	VERB },
	{".dw",		0,	DEFW,		VERB },
	{".dword",	0,	DEFD,		VERB },
	{"e",		3,	REGNAME,	I8080 | Z80 },
	{"ei",		0373,	NOOPERAND,	VERB | I8080 | Z80 },
	{".eject",	1,	LIST,		VERB },
	{".elist",	3,	LIST,		VERB },
	{".else",	0,	ELSE_TK,	VERB },
	{".end",	0,	END,		VERB },
	{".endc",	0,	ENDIF_TK,	VERB },
	{".endif",	0,	ENDIF_TK,	VERB },
	{".endm", 	0,	ENDM,		VERB },
	{".entry",	0,	PUBLIC,		VERB },
	{"eq",		0,	EQ,		0 },
	{".eq.",	0,	MROP_EQ,	TERM | MRASOP },
	{".equ",	0,	EQU,		VERB },
	{"ex",		0xEB,	EX,		VERB | Z80 },
	{"exaf",	0x08,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{".exitm", 	0,	EXITM,		VERB },
	{".ext",	0,	EXTRN,		VERB },
	{".extern",	0,	EXTRN,		VERB },
	{".extrn",	0,	EXTRN,		VERB },
	{"exx",		0331,	NOOPERAND,	VERB | Z80 },
	{"f",		0,	TK_F,		Z80 },
	{".flist",	4,	LIST,		VERB },
	{"ge",		0,	GE,		0 },
	{".ge.",	0,	MROP_GE,	TERM | MRASOP },
	{".glist",	5,	LIST,		VERB },
	{".global",	0,	PUBLIC,		VERB },
	{"gt",		0,	GT,		0 },
	{".gt.",	0,	MROP_GT,	TERM | MRASOP },
	{"h",		4,	REGNAME,	I8080 | Z80 },
	{"halt",	0166,	NOOPERAND,	VERB | Z80 },
	{"high",	0,	HIGH,		0 },
	{".high.",	0,	MROP_HIGH,	TERM | MRASOP },
	{"hl",		040,	HL,		Z80 },
	{"hlt",		0166,	NOOPERAND,	VERB | I8080 },
	{"i",		0,	MISCREG,	Z80 },
	{".if",		0,	IF_TK,		VERB | COL0 },
	{".ifdef",	1,	IF_DEF_TK,	VERB | COL0 },
	{".ifndef",	0,	IF_DEF_TK,	VERB | COL0 },
	{"im",		0166506,IM,		VERB | Z80 },
	{"im0",		0xed46,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"im1",		0xed56,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"im2",		0xed5e,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{".import",	PSIMPORT,ARGPSEUDO,	VERB | COL0 },
	{"in",		0333,	TK_IN,		VERB | I8080 | Z80 },
	{"in0",		0xED00,	TK_IN0,		VERB | Z180 },
	{"inc",		4,	INCDEC,		VERB | Z80 },
	{".incbin", 	0, 	INCBIN,		VERB },
	{".include",	PSINC,	ARGPSEUDO,	VERB | COL0 },	// COL0 only needed for MRAS mode
	{"ind",		0166652,NOOPERAND,	VERB | Z80 },
	{"indr",	0166672,NOOPERAND,	VERB | Z80 },
	{"ini",		0166642,NOOPERAND,	VERB | Z80 },
	{"inir",	0166662,NOOPERAND,	VERB | Z80 },
	{"inp",		0xed40,	INP,		VERB | Z80 | ZNONSTD },
	{"inr",		4,	INRDCR,		VERB | I8080 },
	{"inrx",	0xdd34,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"inry",	0xfd34,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"inx",		3,	INXDCX,		VERB | I8080 },
	{"inxix",	0xdd23,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"inxiy",	0xfd23,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"irp",		0,	IRP,		VERB },
	{"irpc",	0,	IRPC,		VERB },
	{"ix",		0156440,INDEX,		Z80 },
	{"ixh",		0x1DD04,IXYLH,		Z80 | UNDOC },
	{"ixl",		0x1DD05,IXYLH,		Z80 | UNDOC },
	{"iy",		0176440,INDEX,		Z80 },
	{"iyh",		0x1FD04,IXYLH,		Z80 | UNDOC },
	{"iyl",		0x1FD05,IXYLH,		Z80 | UNDOC },
	{"jc",		0332,	JUMP8,		VERB | I8080 },
	{"jm",		0372,	JUMP8,		VERB | I8080 },
	{"jmp",		0303,	JP,		VERB | I8080 },
	{"jnc",		0322,	JUMP8,		VERB | I8080 },
	{"jnz",		0302,	JUMP8,		VERB | I8080 },
	{"jp",		0362,	JP,		VERB | I8080 | Z80 },
	{"jpe",		0352,	JUMP8,		VERB | I8080 },
	{".jperror",	0,	JPERROR,	VERB },
	{"jpo",		0342,	JUMP8,		VERB | I8080 },
	{"jr",		040,	JR,		VERB | Z80 },
	{"jrc",		0x38,	JR_COND,	VERB | Z80 | ZNONSTD },
	{"jrnc",	0x30,	JR_COND,	VERB | Z80 | ZNONSTD },
	{"jrnz",	0x20,	JR_COND,	VERB | Z80 | ZNONSTD },
	{".jrpromote",	0,	JRPROMOTE,	VERB },
	{"jrz",		0x28,	JR_COND,	VERB | Z80 | ZNONSTD },
	{"jz",		0312,	JUMP8,		VERB | I8080 },
	{"l",		5,	REGNAME,	I8080 | Z80 },
	{"lbcd",	0xed4b,	LDST16,		VERB | Z80 | ZNONSTD },
	{"ld",		0x40,	LD,		VERB | Z80 },
	{"lda",		0x3a,	LDA,		VERB | I8080 },
	{"ldai",	0xed57,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"ldar",	0xed5f,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"ldax",	0xA,	LDAX,		VERB | I8080 },
	{"ldd",		0166650,NOOPERAND,	VERB | Z80 },
	{"lddr",	0166670,NOOPERAND,	VERB | Z80 },
	{"lded",	0xed5b,	LDST16,		VERB | Z80 | ZNONSTD },
	{"ldi",		0166640,NOOPERAND,	VERB | Z80 },
	{"ldir",	0166660,NOOPERAND,	VERB | Z80 },
	{"ldx",		0xdd46,	LD_XY,		VERB | Z80 | ZNONSTD},
	{"ldy",		0xfd46,	LD_XY,		VERB | Z80 | ZNONSTD},
	{"le",		0,	LE,		0 },
	{".le.",	0,	MROP_LE,	TERM | MRASOP },
	{"lhld",	0x2a,	LHLD,		VERB | I8080 },
	{".list",	0,	LIST,		VERB | COL0 }, // COL0 only needed for MRAS
	{"lixd",	0xdd2a,	LDST16,		VERB | Z80 | ZNONSTD },
	{"liyd",	0xfd2a,	LDST16,		VERB | Z80 | ZNONSTD },
	{".local",	0,	LOCAL,		VERB },
	{"low",		0,	LOW,		0 },
	{".low.",	0,	MROP_LOW,	TERM | MRASOP },
	{"lspd",	0xed7b,	LDST16,		VERB | Z80 | ZNONSTD },
	{"lt",		0,	LT,		0 },
	{".lt.",	0,	MROP_LT,	TERM | MRASOP },
	{"lxi",		1,	LXI,		VERB | I8080 },
	{"lxix",	0xdd21,	LDST16,		VERB | Z80 | ZNONSTD },
	{"lxiy",	0xfd21,	LDST16,		VERB | Z80 | ZNONSTD },
	{"m",		070,	COND,		I8080 | Z80 },
	{".maclib",	PSMACLIB,ARGPSEUDO,	VERB },
	{".macro",	0,	MACRO,		VERB },
	{".max",	1,	MINMAX,		VERB },
	{".min",	0,	MINMAX,		VERB },
	{".mlist",	6,	LIST,		VERB },
	{"mlt",		0xED4C,	MLT,		VERB | Z180 },
	{"mod",		0,	'%',		0 },
	{".mod.",	0,	MROP_MOD,	TERM | MRASOP },
	{"mov",		0x40,	MOV,		VERB | I8080 },
	{"mvi",		6,	MVI,		VERB | I8080 },
	{"mvix",	0xdd36,	MV_XY,		VERB | Z80 | ZNONSTD },
	{"mviy",	0xfd36,	MV_XY,		VERB | Z80 | ZNONSTD },
	{".name",	SPNAME,	SPECIAL,	VERB },
	{"nc",		020,	SPCOND,		0 },
	{"ne",		0,	NE,		0 },
	{".ne.",	0,	MROP_NE,	TERM | MRASOP },
	{"neg",		0166504,NOOPERAND,	VERB | Z80 },
	{".nolist",	-1,	LIST,		VERB },
	{"nop",		0,	NOOPERAND,	VERB | I8080 | Z80 },
	{"not",		0,	'~',		0 },
	{".not.",	0,	MROP_NOT,	TERM | MRASOP },
	{"nul",		0,	NUL,		0 },
	{"nv",		040,	COND,		Z80 },
	{"nz",		0,	SPCOND,		Z80 },
	{"ocf",		0,	OCF,		0 },
	{"or",		0260,	OR,		VERB | Z80 | TERM },
	{".or.",	6,	MROP_OR,	TERM | MRASOP },
	{"ora",		0260,	LOGICAL,	VERB | I8080 },
	{".org",	0,	ORG,		VERB },
	{"ori",		0366,	ALUI8,		VERB | I8080 },
	{"orx",		0xddb6,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"ory",		0xfdb6,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"otdm",	0xED8B,	NOOPERAND,	VERB | Z180 },
	{"otdmr",	0xED9B,	NOOPERAND,	VERB | Z180 },
	{"otdr",	0166673,NOOPERAND,	VERB | Z80 },
	{"otim",	0xED83,	NOOPERAND,	VERB | Z180 },
	{"otimr",	0xED93,	NOOPERAND,	VERB | Z180 },
	{"otir",	0166663,NOOPERAND,	VERB | Z80 },
	{"out",		0323,	TK_OUT,		VERB | I8080 | Z80 },
	{"out0",	0xED01,	TK_OUT0,	VERB | Z180 },
	{"outd",	0166653,NOOPERAND,	VERB | Z80 },
	{"outdr",	0166673,NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"outi",	0166643,NOOPERAND,	VERB | Z80 },
	{"outir",	0166663,NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"outp",	0xED41,	OUTP,		VERB | Z80 | ZNONSTD },
	{"p",		060,	COND,		Z80 },
	{".page",	1,	LIST,		VERB },
	{"pchl",	0351,	NOOPERAND,	VERB | I8080 },
	{"pcix",	0xdde9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"pciy",	0xfde9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"pe",		050,	COND,		Z80 },
	{"pfix",	0xdd,	NOOPERAND,	VERB | Z80 | UNDOC },
	{"pfiy",	0xfd,	NOOPERAND,	VERB | Z80 | UNDOC },
	{".phase",	0,	PHASE,		VERB },
	{"po",		040,	COND,		Z80 },
	{"pop",		0301,	PUSHPOP,	VERB | I8080 | Z80 },
	{"popix",	0xdde1,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"popiy",	0xfde1,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"pragma",	SPPRAGMA,SPECIAL,	VERB },
	{"psw", 	060,	PSW,		I8080 },
	{".public",	0,	PUBLIC,		VERB },
	{"push",	0305,	PUSHPOP,	VERB | I8080 | Z80 },
	{"pushix",	0xdde5,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"pushiy",	0xfde5,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"r",		010,	MISCREG,	Z80 },
	{"ral",		027,	NOOPERAND,	VERB | I8080 },
	{"ralr",	0xCB10,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"ralx",	0xddcb0016,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"raly",	0xfdcb0016,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rar",		037,	NOOPERAND,	VERB | I8080 },
	{"rarr",	0xCB18,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"rarx",	0xddcb001e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rary",	0xfdcb001e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rc",		0330,	NOOPERAND,	VERB | I8080 },
	{".read",	PSINC,	ARGPSEUDO,	VERB },
	{"rept",	0,	REPT,		VERB },
	{"res",		0145600,BIT,		VERB | Z80 },
	{"resx",	0xddcb0086,BIT_XY,	VERB | Z80 | ZNONSTD },
	{"resy",	0xfdcb0086,BIT_XY,	VERB | Z80 | ZNONSTD },
	{"ret",		0311,	RET,		VERB | I8080 | Z80 },
	{"reti",	0166515,NOOPERAND,	VERB | Z80 },
	{"retn",	0166505,NOOPERAND,	VERB | Z80 },
	{"rl",		0xCB10,	SHIFT,		VERB | Z80 },
	{"rla",		027,	NOOPERAND,	VERB | Z80 },
	{"rlc",		0xCB00,	SHIFT,		VERB | I8080 | Z80 },
	{"rlca",	07,	NOOPERAND,	VERB | Z80 },
	{"rlcr",	0xCB00,	SHIFT,		VERB | I8080 | Z80 | ZNONSTD },
	{"rlcx",	0xddcb0006,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rlcy",	0xfdcb0006,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rld",		0166557,NOOPERAND,	VERB | Z80 },
	{"rm",		0370,	NOOPERAND,	VERB | I8080 },
	{".rmem",	0,	DEFS,		VERB },
	{"rnc",		0320,	NOOPERAND,	VERB | I8080 },
	{"rnz",		0300,	NOOPERAND,	VERB | I8080 },
	{"rp",		0360,	NOOPERAND,	VERB | I8080 },
	{"rpe",		0350,	NOOPERAND,	VERB | I8080 },
	{"rpo",		0340,	NOOPERAND,	VERB | I8080 },
	{"rr",		0xCB18,	SHIFT,		VERB | Z80 },
	{"rra",		037,	NOOPERAND,	VERB | Z80 },
	{"rrc",		0xCB08,	SHIFT,		VERB | I8080 | Z80 },
	{"rrca",	017,	NOOPERAND,	VERB | Z80 },
	{"rrcr",	0xCB08,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"rrcx",	0xddcb000e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rrcy",	0xfdcb000e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"rrd",		0166547,NOOPERAND,	VERB | Z80 },
	{"rst",		0307,	RST,		VERB | I8080 | Z80 },
	{".rsym",	PSRSYM,	ARGPSEUDO,	VERB },
	{"rz",		0310,	NOOPERAND,	VERB | I8080 },
	{"sbb",		0230,	ARITHC,		VERB | I8080 },
	{"sbc",		0230,	ARITHC,		VERB | Z80 },
	{"sbcd",	0xed43,	LDST16,		VERB | Z80 | ZNONSTD },
	{"sbcx",	0xdd9e,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"sbcy",	0xfd9e,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"sbi",		0336,	ALUI8,		VERB | I8080 },
	{"scf",		067,	NOOPERAND,	VERB | Z80 },
	{"sded",	0xed53,	LDST16,		VERB | Z80 | ZNONSTD },
	{"set",		0145700,BIT,		VERB | Z80 },
	{"setb",	0145700,BIT,		VERB | Z80 | ZNONSTD },
	{".setocf",	0,	SETOCF,		VERB },
	{".sett",	0,	TSTATE,		VERB },
	{"setx",	0xddcb00c6,BIT_XY,	VERB | Z80 | ZNONSTD },
	{"sety",	0xfdcb00c6,BIT_XY,	VERB | Z80 | ZNONSTD },
	{"shl",		0,	SHL,		TERM },
	{".shl.",	0,	MROP_SHL,	TERM | MRASOP },
	{"shld",	0x22,	SHLD,		VERB | I8080 },
	{"shr",		0,	SHR,		TERM },
	{".shr.",	0,	MROP_SHR,	TERM | MRASOP },
	{"sixd",	0xdd22,	LDST16,		VERB | Z80 | ZNONSTD },
	{"siyd",	0xfd22,	LDST16,		VERB | Z80 | ZNONSTD },
	{"sl1",		0xCB30,	SHIFT,		VERB | Z80 | UNDOC },
	{"sla",		0xCB20,	SHIFT,		VERB | Z80 },
	{"slar",	0xCB20,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"slax",	0xddcb0026,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"slay",	0xfdcb0026,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"sll",		0xCB30,	SHIFT,		VERB | Z80 },
	{"slp",		0xED76,	NOOPERAND,	VERB | Z180 },
	{"sp",		060,	SP,		I8080 | Z80 },
	{".space",	2,	LIST,		VERB },
	{"sphl",	0371,	NOOPERAND,	VERB | I8080 },
	{"spix",	0xddf9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"spiy",	0xfdf9,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"sra",		0xCB28,	SHIFT,		VERB | Z80 },
	{"srar",	0xCB28,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"srax",	0xddcb002e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"sray",	0xfdcb002e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"srl",		0xCB38,	SHIFT,		VERB | Z80 },
	{"srlr",	0xCB38,	SHIFT,		VERB | Z80 | ZNONSTD },
	{"srlx",	0xddcb003e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"srly",	0xfdcb003e,SHIFT_XY,	VERB | Z80 | ZNONSTD },
	{"sspd",	0xed73,	LDST16,		VERB | Z80 | ZNONSTD },
	{"sta",		0x32,	STA,		VERB | I8080 },
	{"stai",	0xed47,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"star",	0xed4f,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"stax",	2,	STAX,		VERB | I8080 },
	{"stc",		067,	NOOPERAND,	VERB | I8080 },
	{"stx",		0xdd70,	ST_XY,		VERB | Z80 | ZNONSTD},
	{"sty",		0xfd70,	ST_XY,		VERB | Z80 | ZNONSTD},
	{"sub",		0220,	LOGICAL,	VERB | I8080 | Z80 },
	{".subttl",	SPSBTL,	SPECIAL,	VERB },
	{"subx",	0xdd96,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"suby",	0xfd96,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"sui",		0326,	ALUI8,		VERB | I8080 },
	{"t",		0,	T,		0 },
	{".text",	0,	DEFB,		VERB },
	{"tihi",	0,	TIHI,		0 },
	{"tilo",	0,	TILO,		0 },
	{".title",	SPTITL,	SPECIAL,	VERB | COL0},
	{"tst",		0xED04,	TST,		VERB | Z180 },
	{".tstate",	0,	TSTATE,		VERB },
	{"tstio",	0xED74,	TSTIO,		VERB | Z180 },
	{"v",		050,	COND,		Z80 },
	{".word",	0,	DEFW,		VERB },
	{".wsym",	PSWSYM,	ARGPSEUDO,	VERB },
	{"xchg",	0353,	NOOPERAND,	VERB | I8080 },
	{"xor",		0250,	XOR,		VERB | Z80 | TERM },
	{".xor.",	0,	MROP_XOR,	TERM | MRASOP },
	{"xorx",	0xddae,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"xory",	0xfdae,	ALU_XY,		VERB | Z80 | ZNONSTD },
	{"xra",		0250,	LOGICAL,	VERB | I8080 },
	{"xri",		0356,	ALUI8,		VERB | I8080 },
	{"xthl",	0343,	NOOPERAND,	VERB | I8080 },
	{"xtix",	0xdde3,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"xtiy",	0xfde3,	NOOPERAND,	VERB | Z80 | ZNONSTD },
	{"z",		010,	SPCOND,		Z80 },
	{".z180",	2,	INSTSET,	VERB },
	{".z80",	1,	INSTSET,	VERB },
};

/*
 *  user-defined items are tabulated in the following table.
 */

struct item	itemtab[ITEMTABLESIZE];
struct item	*itemmax = itemtab+ITEMTABLESIZE;

int item_is_verb(struct item *i)
{
	return i && (i->i_uses & VERB) == VERB;
}

int item_value(struct item *i)
{
	int value = i->i_value;

	// Some special cases for 8080 opcode values.
	if (!z80) {
		// CP is CALL P in 8080
		if (i->i_value == 0270 && i->i_string[1] == 'p')
			value = 0364;
		else if (i->i_value == 0166641) // CPI
			value = 0376;
		else if (i->i_token == SHIFT && (i->i_value & 070) < 020)
			value = 7 | (i->i_value & 070);
	}
	else {
		if (i->i_token == JR)
			value = 0x18;
		else if (i->i_token == JP)
			value = 0xC3;

		value = sized_byteswap(value);
	}

	return value;
}

/*
 *  lexical analyser, called by yyparse.
 */
int yylex()
{
	int lex();
	int token = lex();

	if (mras) {
		switch (token)
		{
		// Operator translations because of different precedence
		case '+': token = MROP_ADD; break;
		case '-': token = MROP_SUB; break;
		case '*': token = MROP_MUL; break;
		case '/': token = MROP_DIV; break;
		case '%': token = MROP_MOD; break;
		case '&': token = MROP_AND; break;
		case '|': token = MROP_OR; break;
		case '^': token = MROP_XOR; break;
		case SHL: token = MROP_SHL; break;
		case SHR: token = MROP_SHR; break;
		case LT:  token = MROP_LT; break;
		case EQ:  token = MROP_EQ; break;
		case '>': token = MROP_GT; break;
		case GE:  token = MROP_GE; break;
		case NE:  token = MROP_NE; break;
		case LE:  token = MROP_LE; break;
		case NOT: token = MROP_NOT; break;
		case HIGH: token = MROP_HIGH; break;
		case LOW: token = MROP_LOW; break;
		// Operator translations to completely different operator.
		case '<': token = MROP_SHIFT; break;
		case '!': token = MROP_OR; break;
		default: break;
		// Sadly, AND, OR, XOR and '=' can't be translated unilaterally
		// because they are also opcodes or psuedo-ops.
		}
	}

	return token;
}

int lex()
{
	int c;
	char *p;
	int radix;
	int sep;
	int exclude, include, overflow, token, endc;

	if (arg_flag) {
		yylval.cval = arg_state.arg;
		c = getarg(&arg_state);
		if (c == '\0' || c == '\n' || c == ';')
			c = skipline(c);

		return c;
	}

	if (raw == 2) {
		while (charclass[c = nextchar()] == SPACE)
			;

		*tempbuf = c == '\n' || c == '\0';

		peekc = skipline(c);

		raw = 0;

		return RAWTOKEN;
	}
	else if (raw) {
		int skip = 1;
		p = tempbuf;
		while ((c = nextchar()) != '\n' && c) {
			if (p >= tempmax) {
				*p = '\0';
				printf("was parsing '%s'\n", tempbuf);
				error(symlong);
			}
			if (!skip || charclass[c] != SPACE) {
				*p++ = c;
				skip = 0;
			}
		}
		if (c == 0)
			peekc = c;

		*p-- = '\0';

		while (p >= tempbuf && CHARCLASS(*p) == SPACE)
			*p-- = '\0';

		raw = 0;

		return RAWTOKEN;
	}

	for (;;) switch(charclass[c = nextchar()]) {
	case F_END:
		if (!expptr)
			return 0;

		if (est[MSTR].param) {
			int ch;
			est[REPNUM].value++;
			ch = est[MSTR].param[est[REPNUM].value];
			if (ch) {
				est[0].param[0] = ch;
				floc = est[MSTART].value;
				mfseek(mfile, (long)floc, 0);
				est[TEMPNUM].value = exp_number++;
			}
			else {
				free(est[MSTR].param);
				est[MSTR].param = 0;
				popsi();
			}
		}
		else if (est[REPNUM].value < 0) {
			int arg;
			do {
				switch (getarg(est[MARGP].ap)) {
				case ARG:
					arg = 1;
					break;
				case ',':
					arg = 0;
					break;
				default:
					arg = 2;
					break;
				}
			} while (!arg);

			if (arg == 1) {
				floc = est[MSTART].value;
				mfseek(mfile, (long)floc, 0);
				est[TEMPNUM].value = exp_number++;
			}
			else {
				// XXX - memory leak
				est[0].param = NULL;
				free(est[MARGP].ap);
				popsi();
			}
		}
		else if (est[REPNUM].value-- > 0) {
			floc = est[MSTART].value;
			mfseek(mfile, (long)floc, 0);
			est[TEMPNUM].value = exp_number++;
		}
		else
			popsi();

		continue;

	case SPACE:
		while (charclass[c = nextchar()] == SPACE)
			;
		peekc = c;
		logcol++;
		break;
	case LETTER:
	case STARTER:
	case DIGIT:
	case DOLLAR:
	spectok:
		firstcol = getcol() == 1;

		radix = -1; // might be a number
		p = tempbuf;
		do {
			if (p >= tempmax) {
				*tempmax = '\0';
				printf("was parsing '%s'\n", tempbuf);
				error(symlong);
			}

			if (driopt && c == '$') {
				c = nextchar();
				continue;
			}
			// GWP - don't lowercase
			//*p = (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;
			*p = c;
			if (mras && *p == '?') {
				char *q;

				radix = 0; // can't be a number even if it looks like it

				if (expptr)
					q = getmraslocal();
				else
					for (q = modstr; *q == '@'; q++)
						;

				if (*q) {
					strcpy(p, q);
					p = strchr(p, '\0') - 1;
				}
				else
					*p = '?';
			}
			p++;
			c = nextchar();
		} while	(charclass[c]==LETTER || charclass[c]==DIGIT ||
			charclass[c]==STARTER || charclass[c]==DOLLAR);

		if (driopt && p == tempbuf)
			*p++ = '$'; // reverse overzelous stripping

		*p = '\0';

		// When parsing macro parameters we use a custom symbol table.
		// And, heck, almost anything goes.
		if (param_parse) {
			struct item *param = item_lookup(tempbuf, paramtab, PARAMTABSIZE);
			peekc = c;
			if (param->i_token) {
				sprintf(detail, "%s error.  Macro parameter '%s' repeated",
					errname[fflag], tempbuf);
				errwarn(fflag, detail);
			}

			param->i_token = MPARM;
			param->i_string = malloc(strlen(tempbuf) + 1);
			strcpy(param->i_string, tempbuf);

			yylval.itemptr = param;
			return param->i_token;
		}

		// Special case for AF'
		if (c == '\'' && ci_strcmp(tempbuf, "af") == 0)
			return AFp;

		endc = c;
		peekc = c;

		// Pass off '?' (XXX but, technically, should only be done in expression context)
		if (strcmp(tempbuf, "?") == 0)
			return '?';

		// Pass off '$'
		if (strcmp(tempbuf, "$") == 0)
			return '$';

		// Look ahead at what we have.
		while (charclass[c] == SPACE)
			c = nextchar();

		peekc = c;

		//printf("%d %s\n", logcol, tempbuf);
		// If logcol == 0 then if c == ':' we're a label for sure.
		// If logcol == 1 if c == ':' we're a label, change logcol
		//    otherwise we're op or pseudo
		// If logcol == 0 and c == '\n' or ';' then we're alone so
		//	we give tokenization a chance otherwise label
		// If logcol >= 2 we're in the arguments
		//
		// There is quite a lot of unrealized scope for error
		// detection and helpful warnings.

		 // Default to any tokenization.
		exclude = 0;
		include = 0;

		if (logcol >= 2) {
			// Arguments allow mnemonics and psuedo-ops
			exclude = VERB;
			include = TERM;
		}
		else if (logcol == 0 && first_always_label) {
			exclude = ~TERM;
		}
		else if (logcol <= 1 && c == ':') {
			exclude = ~TERM;
			logcol = 0;
		}
		else if (logcol == 0 && c != ';' && c != '\n') {
			exclude = ~TERM;
			include = COL0;
		}

		logcol++;

		// Look for possible numbers.
		// 0x<hex> $<hex> <hex>h <octal>o <octal>q <binary>b
		// <decimal> <decimal>d
		// Suffix formats must start with 0-9.

		if (radix)
			radix = convert(tempbuf, p, &overflow);

		// If we're in the first logical column and the token starts with
		// '$' then we'll force it to be a label even though it could be
		// a $hex constant. This will allow $FCB as a label.
		// Thus we must also allow symbol lookup a chance to override number
		// parsing if we start with a '$'.

		if (tempbuf[0] == '$') {
			if (logcol == 1 || locate(tempbuf)->i_token) {
				if (radix > 0) {
					sprintf(detail, "warning: $hex constant '%s' interpreted as symbol", tempbuf);
					errwarn(warn_hex, detail);
				}
				radix = 0;
			}
		}

		if (radix > 0) {
			// Might be line skipping time, though.
			if (*ifptr)
				return skipline(c);

			if (overflow) {
				err[iflag]++;
				yylval.ival = 0;
			}
			return NUMBER;
		}

		// Too late to do '$' concatenation of numbers.  But zmac
		// didn't allow for that previously at any rate.
		if (zcompat) {
			char *q = tempbuf;
			// Normal zmac operation requires we ignore $ in identifiers
			for (p = q; *p; p++)
				if (*p != '$')
					*q++ = *p;

			*q = '\0';
			p = q;
		}

		// GWP - boy, this should be a warning or error
		if (p - tempbuf > MAXSYMBOLSIZE) {
			p = tempbuf + MAXSYMBOLSIZE;
			*p = '\0';
		}

		token = tokenofitem(UNDECLARED, exclude, include);

		// Initial "priming" hack

		if (endc == '\'')
		{
			//printf("allowing <<%s%c>> at %d, endc=%c\n", tempbuf, peekc, logcol, endc);
			peekc = NOPEEK;
// A step towards emitting the instructions
#if 0
			// Only if auto-exx on on...
			if (tempbuf[0] == 'a') {
				emit1(8, 0, 0, ET_NOARG);
				list_out(dollarsign, "\tex\taf,af'\n", '*');
			}
			else {
				emit1(0xd9, 0, 0, ET_NOARG);
				list_out(dollarsign, "\texx\n", '*');
			}
#endif
		}

		return token;

	default:
		if (*ifptr)
			return(skipline(c));

		if (mras && getcol() == 1 && c == '*')
			goto spectok;

		switch(c) {
		int corig;
		case ':':
			if (logcol == 1) {
				// Make sure "label:ret", "label: ret",
				// "label: :ret", "label: : ret" work out OK.
				// But stop fooling around once we've done the VERB
				peekc = nextchar();
				if (charclass[peekc] == SPACE)
					logcol--;
			}
			return c;
		case ';':
			return(skipline(c));
		case '\'':
		case '"':
			sep = c;
			p = tempbuf;
			p[1] = 0;
			do	switch(c = nextchar())	{
			case '\0':
			case '\n':
				err[bflag]++;
				goto retstring;
			default:
				if (c == sep && (c = nextchar()) != sep) {
				retstring:
					peekc = c;
					*p = '\0';
					switch (p - tempbuf) {
					case 2:
						p = tempbuf;
						yylval.ival = *p++ & 255;
						yylval.ival |= (*p & 255) << 8;
						return TWOCHAR;
					case 1:
						p = tempbuf;
						yylval.ival = *p++;
						return ONECHAR;
					case 0:
						if (!full_exprs) {
							yylval.ival = 0;
							return NUMBER;
						}
						// fall through
					default:
						yylval.cval = tempbuf;
						return STRING;
					}
				}
				*p++ = c;
			} while (p < tempmax);
			/*
			 *  if we break out here, our string is longer than
			 *  our input line
			 */
			error("string buffer overflow");
		case '<':
			corig = c;
			switch (c = nextchar ()) {
			case '=':
				return LE;
			case '<':
				return SHL;
			case '>':
				return NE;
			default:
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */
		case '>':
			corig = c;
			switch (c = nextchar ()) {
			case '=':
				return GE;
			case '>':
				return SHR;
			default:
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */
		case '!':
			corig = c;
			switch (c = nextchar ()) {
			case '=':
				return NE;
			default:
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */
		case '=':
			corig = c;
			switch (c = nextchar ()) {
			case '=':
				return '=';
			default:
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */

		case '&':
			corig = c;
			if ((c = nextchar()) == '&')
				return ANDAND;
			else {
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */
		case '|':
			corig = c;
			if ((c = nextchar()) == '|')
				return OROR;
			else {
				peekc = c;
				return corig;
			}
			/* break; suppress "unreachable" warning for tcc */
		default:
			return(c);
		}
	}
}

// Convert string to number
// 0x<hex> $<hex> <hex>h <octal>o <octal>q <binary>b
// <decimal> <decimal>d
// Suffix formats must start with 0-9.
//
// Returns:
//	-1	not numeric
//	0	numeric but digit out of range
//	>0	radix of number,  yylval.ival holds value

int convert(char *buf, char *bufend, int *overflow)
{
	int radix = -1, dummy;
	char *d0, *dn;

	if (!bufend)
		bufend = strchr(buf, '\0');

	if (!overflow)
		overflow = &dummy;

	if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X') && buf[2]) {
		radix = 16;
		d0 = buf + 2;
		dn = bufend;
	} else if (buf[0] == '$') {
		radix = 16;
		d0 = buf + 1;
		dn = bufend;
	}
	else if (buf[0] >= '0' && buf[0] <= '9') {
		d0 = buf;
		dn = bufend - 1;
		switch (*dn) {
		case 'O':
		case 'o':
		case 'Q':
		case 'q':
			radix = 8;
			break;
		case 'D':
		case 'd':
			radix = 10;
			break;
		case 'H':
		case 'h':
			radix = 16;
			break;
		case 'B':
		case 'b':
			radix = 2;
			break;
		default:
			radix = 10;
			dn++;
			break;
		}
	}

	// We may have a number on our hands.
	if (radix > 0) {
		*overflow = 0;
		yylval.ival = 0;

		for (; d0 < dn; d0++) {
			unsigned int ovchk = (unsigned int)yylval.ival;
			int c = *d0;
			if (c >= 'a') c -= 'a' - 10;
			else if (c >= 'A') c -= 'A' - 10;
			else c -= '0';
			if (c < 0 || c >= radix) {
				radix = 0;
				break;
			}
			if (ovchk * radix / radix != ovchk)
				*overflow = 1;

			yylval.ival *= radix;
			yylval.ival += c;
		}
	}

	return radix;
}

// Verify keytab is in alphabetical order.
// And that all MRASOP keywords start with '.'

int check_keytab()
{
	int i;
	char *prev;

	for (i = 0; i < sizeof(keytab) / sizeof(keytab[0]); i++) {
		char *next = keytab[i].i_string;
		next += *next == '.';
		if (i != 0) {
			if (strcmp(prev, next) >= 0) {
				printf("keytab error: %s >= %s\n", prev, next);
				return 0;
			}
		}
		prev = next;

		if ((keytab[i].i_uses & MRASOP) && keytab[i].i_string[0] != '.') {
			printf("keytab error: %s does not start with '.'\n",
				keytab[i].i_string);
			return 0;
		}

		// Uncomment to liat all 8080 verbs to assist documentation.
		//if ((keytab[i].i_uses & (VERB | I8080)) == (VERB | I8080))
		//	printf("\tdb\t%s\n", keytab[i].i_string);
		// Uncomment to list all Z-80 verbs to assist documentation.
		//if ((keytab[i].i_uses & (VERB | Z80)) == (VERB | Z80))
		//	printf("%-6s   $%X\n", keytab[i].i_string,
		//		item_value(keytab + i));
		// Uncomment to list all Z-180 verbs to assist documentation.
		//if ((keytab[i].i_uses & (VERB | Z180)) == (VERB | Z180))
		//	printf("%-6s   $%X\n", keytab[i].i_string,
		//		item_value(keytab + i));
	}

	printf("keytab OK\n");

	return 1;
}


struct item *keyword(char *name)
{
	int  r, l, u;
	struct item *ip;

	/*
	 *  binary search
	 */
	l = 0;
	u = (sizeof keytab/sizeof keytab[0])-1;
	while (l <= u) {
		char *key;
		i = (l+u)/2;
		ip = &keytab[i];
		key = ip->i_string;
		r = ci_strcmp(name + (name[0] == '.'), key + (key[0] == '.'));
		if (r == 0) {
			// Do not allow ".foo" to match "foo"
			if (name[0] == '.' && key[0] != '.')
				break;

			return ip;
		}
		if (r < 0)
			u = i-1;
		else
			l = i+1;
	}

	// Check if there's an alias.
	ip = locate(name);
	if (ip && (ip->i_scope & SCOPE_ALIAS))
		return keytab + ip->i_value;

	return 0;
}

int keyword_index(struct item *k)
{
	return k - keytab;
}

// Find 'name' in an item table.  Returns an empty slot if not found.
// Uses case-independent comparisions which are largely wasted as
// there is only 1 case where 'name' has not already been lowercased.

struct item *item_lookup(char *name, struct item *table, int table_size)
{
	struct item *ip;
	/*
	 *  hash into item table
	 */
	int hash = 0;
	char *p = name;
	while (*p) {
		char ch = *p++;
		if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
		hash += ch;
	}
	hash %= table_size;
	ip = &table[hash];

	for (;;) {
		if (ip->i_token == 0)
			break;
		if (ci_strcmp(name, ip->i_string) == 0)
			break;
		if (++ip >= table + table_size)
			ip = table;
	}

	return ip;
}

struct item *locate(char *name)
{
	return item_lookup(name, itemtab, ITEMTABLESIZE);
}

// Return the longest token that matches the start of the given name.
// Currently used for MRAS which will substitute macro parameters inside
// identifiers.
struct item *item_substr_lookup(char *name, int token, struct item *table, int table_size)
{
	struct item *ip = 0;
	int i;

	for (i = 0; i < table_size; i++) {
		unsigned int len;

		if (table[i].i_token != token)
			continue;

		len = strlen(table[i].i_string);
		if (ci_strcompare(name, table[i].i_string, len) == 0) {
			if (!ip || len > strlen(ip->i_string)) {
				ip = table + i;
			}
		}
	}

	return ip;
}

/*
 *  return the token associated with the string pointed to by
 *  tempbuf.  if no token is associated with the string, associate
 *  deftoken with the string and return deftoken.
 *  in either case, cause yylval to point to the relevant
 *  symbol table entry.
 *
 *  Only keys not matching the keyexclude will be returned allowing
 *  context-dependent tokenization.  Unless they match keyinclude.
 */

int tokenofitem(int deftoken, int keyexclude, int keyinclude)
{
	struct item *ip;
	int  i;

#ifdef T_DEBUG
	fputs("'tokenofitem entry'	", stderr) ;
	fputs(tempbuf, stderr) ;
#endif

	// Allow macros to override built-ins
	// Maybe shouldn't be done for identifiers that will undergo
	// '.' and '_' expansion.
	ip = locate(tempbuf);
	if (ip->i_token == MNAME)
		goto found;

	if (full_exprs)
		keyexclude = ~TERM;

	ip = keyword(tempbuf);
	if (ip) {
		if (ip->i_uses & keyinclude)
			goto found;

		if (!(ip->i_uses & keyexclude))
			goto found;
	}

	// This is really my own thing rather than old zmac, but zmac
	// didn't support it and it does depend on '$' crushing a bit.
	if (zcompat) {
	    // '_' prefixed labels are local to the file
	    if (tempbuf[0] == '_') {
		    strcat(tempbuf, "$");
		    strcat(tempbuf, basename(src_name[now_in]));
	    }

	    // '.' prefixed labels are local between labels
	    if (tempbuf[0] == '.') {
		    char *p = tempbuf;
		    while (*p) p++;
		    sprintf(p, "$%d", llseq);
	    }
	}

	ip = locate(tempbuf);

	if (ip->i_token)
		goto found;

	if (!deftoken) {
		i = 0 ;
		goto token_done ;
	}
	if (++nitems > ITEMTABLESIZE-20)
		error("item table overflow");
	ip->i_string = malloc(strlen(tempbuf)+1);
	ip->i_token = deftoken;
	ip->i_uses = 0;
	strcpy(ip->i_string, tempbuf);

found:
	if (*ifptr) {
		if (ip->i_token == ENDIF_TK) {
			i = ENDIF_TK;
			goto token_done ;
		}
		if (ip->i_token == ELSE_TK) {
			/* We must only honour the ELSE if it is not
			   in a nested failed IF/ELSE */
			char forbid = 0;
			char *ifstackptr;
			for (ifstackptr = ifstack; ifstackptr != ifptr; ++ifstackptr) {
				if (*ifstackptr) {
					forbid = 1;
					break;
				}
			}
			if (!forbid) {
				i = ELSE_TK;
				goto token_done;
			}
		}
		if (ip->i_token == IF_TK || ip->i_token == IF_DEF_TK) {
			if (ifptr >= ifstmax)
				error("Too many ifs");
			else *++ifptr = 1;
		}
		i = skipline(' ');
		goto token_done ;
	}
	yylval.itemptr = ip;
	i = ip->i_token;
token_done:
#ifdef T_DEBUG
	fputs("\t'tokenofitem exit'\n", stderr) ;
#endif
	return(i) ;
}

void itemcpy(struct item *dst, struct item *src)
{
	if (dst && src && dst != src) {
		dst->i_string = src->i_string;
		dst->i_value = src->i_value;
		dst->i_token = src->i_token;
		dst->i_uses = src->i_uses;
		dst->i_scope = src->i_scope;
		dst->i_chain = src->i_chain;
		dst->i_pass = src->i_pass;
	}
}

/*
 *  interchange two entries in the item table -- used by custom_qsort
 */
void interchange(int i, int j)
{
	struct item temp;

	itemcpy(&temp, itemtab + i);
	itemcpy(itemtab + i, itemtab + j);
	itemcpy(itemtab + j, &temp);
}



/*
 *  quick sort -- used by compactsymtab to sort the symbol table
 */
void custom_qsort(int m, int n)
{
	int  i, j;

	if (m < n) {
		i = m;
		j = n+1;
		for (;;) {
			do i++; while(strcmp(itemtab[i].i_string,
					itemtab[m].i_string) < 0);
			do j--; while(strcmp(itemtab[j].i_string,
					itemtab[m].i_string) > 0);
			if (i < j) interchange(i, j); else break;
		}
		interchange(m, j);
		custom_qsort(m, j-1);
		custom_qsort(j+1, n);
	}
}

int getcol()
{
	return inpptr - inpbuf;
}

void dri_setmacro(int op) {
	if (op == '-') {
		mopt = 0;
	} else if (op == '+') {
		mopt = 1;
	} else { // '*'
		mopt = 2;
	}
}

int mc_quote;
int mc_first;

void start_multi_check()
{
	mc_quote = -1;
	mc_first = 1;
}

int found_multi(int ch)
{

	if (ch == mc_quote && (mc_quote == '"' || mc_quote == '\''))
		mc_quote = -1;
	else if (mc_quote < 0 && (ch == '\'' || ch == '"' || ch == ';'))
		mc_quote = ch;
	else if (ch == '*' && mc_first)
		mc_quote = '*';

	mc_first = 0;
	if (ch == separator && mc_quote < 0)
		return 1;

	return 0;
}

/*
 *  get the next character
 */
int nextchar()
{
	int c, ch;
	unsigned char *p;
	char *getlocal();

	if (peekc != NOPEEK) {
		c = peekc;
		peekc = NOPEEK;
		return c;
	}

	if (inpptr) {
		int col = getcol();

		// Double nul indicates EOF for macros
		if (expptr && inpptr[0] == '\0' && inpptr[1] == '\0') {
			inpptr = 0;
			return 0;
		}

		if (col == 0) {
			void analyze_inpbuf(void);
			void mras_operator_separate(void);

			if (driopt && inpptr[0] == '*') {
				addtoline(*inpptr++);
				c = skipline(inpptr[0]);
				linein[now_in]++;
				return c;
			}

			if (!expptr && !prev_multiline)
				linein[now_in]++;

			analyze_inpbuf();

			// TODO - I think this code and comnt is unnecessary
			if (driopt && comnt) {
				addtoline(*inpptr++);
				c = skipline(*inpptr);
				linein[now_in]++;
				return c;
			}

			if (macopt) {
				dri_setmacro(macopt);
				addtoline(*inpptr++);
				c = skipline(inpptr[0]);
				linein[now_in]++;
				macopt = 0;
				return c;
			}

			if (mras)
				mras_operator_separate();
		}

		if (inpbuf_insert[col]) {
			c = inpbuf_insert[col];
			inpbuf_insert[col] = '\0';
		}
		else {
			c = *inpptr++;
			addtoline(c);
		}

		if (*inpptr == '\0')
			inpptr = 0;

		return c;
	}

	inpptr = inpbuf;
	logcol = 0;
	p = inpbuf;

	// XXX - should check for input line overflow!

	// If invoking a macro then pull the next line from it.
	if (expptr) {
		start_multi_check();
		for (;;) {
			ch = getm();

			if (ch == '\1') { /* expand argument */
				ch = getm() - 'A';
				if (ch >= 0 && ch < PARMMAX && est[ch].param) {
					strcpy((char *)p, est[ch].param);
					p = (unsigned char *)strchr((char *)p, '\0');
				}
			}
			else if (ch == '\2') {	/*  local symbol  */
				ch = getm() - 'A';
				if (ch >= 0 && ch < PARMMAX && est[ch].param)
					strcpy((char *)p, est[ch].param);
				else
					strcpy((char *)p, getlocal(ch, est[TEMPNUM].value));

				p = (unsigned char *)strchr((char *)p, '\0');
			}
			else {
				if (ch == 0)
					break;

				*p++ = ch;

				if (ch == '\n')
					break;

				if (found_multi(ch)) {
					p[-1] = '\n';
					multiline = 1;
					break;
				}
			}
		}
		*p = '\0';
		p[1] = ch;
	}
	else {
		start_multi_check();

		for (;;) {
			ch = nextline_peek != NOPEEK ? nextline_peek : getc(now_file);
			nextline_peek = NOPEEK;

			if (ch == '\r') {
				nextline_peek = getc(now_file);
				if (nextline_peek == '\n')
					nextline_peek = NOPEEK;

				ch = '\n';
			}

			if (ch == EOF)
				break;

			*p++ = ch;

			if (ch == '\n') 
				break;

			if (found_multi(ch) && !inmlex) {
				p[-1] = '\n';
				multiline = 1;
				break;
			}
		}

		*p = '\0';

		/* if EOF, check for include file */
		if (ch == EOF) {
			if (now_in) {
				fclose(fin[now_in]) ;
				free(src_name[now_in]);
				now_file = fin[--now_in];
				nextline_peek = linepeek[now_in];
			}
			else if (p == inpbuf)
				return 0;
	
			if (linein[now_in] < 0) {
				lstoff = 1;
				linein[now_in] = -linein[now_in];
			} else {
				lstoff = 0 ;
			}

			if (outpass) {
				if (iflist()) {
					lineout();
					fprintf(fout, "**** %s ****\n", src_name[now_in]) ;
				}
				if (fbds)
					fprintf(fbds, "%04x %04x f %s\n", dollarsign, emit_addr, src_name[now_in]);
			}

			if (p != inpbuf) {
				*p++='\n';
				*p = '\0';
			}
			else
				inpptr = 0;
		}
	}

	return nextchar();
}

char *skipspace(char *p)
{
	while (CHARCLASS(*p) == SPACE)
		p++;

	return p;
}

// Look at inpbuf and try to determine what logical column we are starting
// at.  We could put all of the work in here and keep yylex simple but for
// now we share the load.

void analyze_inpbuf(void)
{
	int cc;
	char *p, *q, save;
	char *word1, *word2;
	struct item *ip, *word1item;
	int triplecase = 1;

	macopt = 0;

	// No need to do this when pulling in data for a macro.  In fact,
	// in can be harmful to undef a macro.
	if (inmlex)
		return;

	// Default if we find nothing to override
	logcol = 0;

	// One case to check is when you start with whitespace yet there are
	// 3 columns.  If so then we change logcol to -1 to compensate.
	// If the 2nd column is a VERB.

	// If we start with whitespace then we can give up on triple word case.
	p = (char *)inpbuf;
	if (CHARCLASS(*p) != SPACE)
		triplecase = 0;

	p = skipspace(p);
	word1 = p;

	// Special comment if first non-space char is '*'
	if (driopt) {
		comnt = (*p == '*');
		if (comnt)
			return;
	}

	// Now skip over a token or abort if we don't find one

	cc = CHARCLASS(*p);
	if (cc != LETTER && cc != STARTER && cc != DIGIT && cc != DOLLAR)
		return;

	if (driopt && *p == '$' && (p[1] == '-' || p[1] == '+' || p[1] == '*') &&
		strncasecmp(p + 2, "macro", 5) == 0 &&
		(p[7] == 0 || p[7] == '\n' || CHARCLASS(p[7]) == SPACE))
	{
		macopt = p[1];
	}

	for (;;) {
		cc = CHARCLASS(*p);
		if (cc == LETTER || cc == STARTER || cc == DIGIT || cc == DOLLAR)
			p++;
		else
			break;
	}

	// We could skip space-separated colons now, but if we see a colon
	// both issues have been decided to do that because it is easier.
	if (*p == ':')
		return;

	save = *p;
	*p = '\0';
	word1item = locate(word1);
	*p = save;

	p = skipspace(p);

	// Another token to skip past.
	// But we need to examine it to see if it is a verb.

	cc = CHARCLASS(*p);
	if (cc != LETTER && cc != STARTER && cc != DIGIT && cc != DOLLAR)
		return;

	q = p;
	word2 = p;
	for (;;) {
		cc = CHARCLASS(*p);
		if (cc == LETTER || cc == STARTER || cc == DIGIT || cc == DOLLAR)
			p++;
		else
			break;
	}

	// Now we have a second word we can check for the "name macro" case.
	// Unless we're skipping.
	save = *p;
	*p = '\0';
	if (ci_strcmp(word2, "macro") == 0 && word1item->i_token && !*ifptr)
		word1item->i_token = UNDECLARED;

	*p = save;

	if (!triplecase)
		return;

	// Must have space to skip over
	if (CHARCLASS(*p) != SPACE)
		return;

	// This 2nd token must be a verb.
	cc = *p;
	*p = '\0';
	ip = keyword(q);
	*p = cc;
	if (!ip || !(ip->i_uses & VERB))
		return;

	// Now skip over space.  If there's anything but a comment or end
	// of the line then we've may have 3 logical columns.
	// "ld a, 5" can throw that off, but we've done the verb check.

	p = skipspace(p);

	if (*p != ';' && *p != '\n' && *p != '\0')
		logcol--;
}

void mras_operator_separate(void)
{
	int i, sep;

	// Only do operator separation after macro expansion.
	if (inmlex)
		return;

	// Apply operator separation across the entire line.
	// We could limit this to logical columns if need be,
	// but I imagine MRAS didn't allow "x.mod.y" as a label
	// or macro name.

	// Strings are handled in a simple but largly compatible way.
	// AF' is a problem which can be safely ignored since its appearance
	// will mean no expression is present.
	// My allowed salting of ' after identifiers is another matter.
	// But, again, original MRAS code won't do that.

	memset(inpbuf_insert, 0, strlen((char *)inpbuf));

	sep = '\0';
	for (i = 0; inpbuf[i]; i++) {
		int j;

		if (inpbuf[i] == sep) {
			sep = '\0';
			continue;
		}

		if (inpbuf[i] == '\'' || inpbuf[i] == '"')
			sep = inpbuf[i];

		// Don't do anthing if inside a string constant.
		if (sep)
			continue;

		if (inpbuf[i] == ';')
			break;

		// Slight nod to efficiency.  Assumption is tested in
		// check_keytab().

		if (inpbuf[i] != '.')
			continue;

		for (j = 0; j < sizeof(keytab) / sizeof(keytab[0]); j++) {
			char *p = keytab[j].i_string;
			int len = strlen(p);

			if (!(keytab[j].i_uses & MRASOP))
				continue;

			if (ci_strcompare((char *)inpbuf + i, p, len) == 0) {
				// Only need to add spaces if we're possibly
				// in an identifier.  But no harm if we're not.
				inpbuf_insert[i] = ' ';
				inpbuf_insert[i + len] = ' ';
				i += len - 1;
				break;
			}
		}
	}
}


/*
 *  skip to rest of the line -- comments and if skipped lines
 */
int skipline(int ac)
{
	int  c;

	c = ac;
	while (c != '\n' && c != '\0')
		c = nextchar();
	return('\n');
}

void add_incpath(char *dir)
{
	if (incpath_cnt >= MAXINCPATH) {
		fprintf(stderr, "Sorry, can only handle %d include paths\n", MAXINCPATH);
		exit(1);
	}

	incpath[incpath_cnt++] = strdup(dir);
}

FILE *open_incpath(char *filename, char *mode, char **path_used)
{
	char quote;
	int i;
	char path[1024];
	FILE *fp;

	// Due to the way parsing works the string can be specified
	// without quotes or will allow quotes but include them.  Instead
	// of fooling with the parsing I just strip the quotes.  I think
	// you can still include a file that starts with a single or double
	// quote by quoting it, but that's an awful thing to do to yourself.

	quote = *filename;
	if (quote == '"' || quote == '\'') {
		char *p;
		for (p = filename; *p; p++)
			p[0] = p[1];

		if (p[-2] == quote)
			p[-2] = '\0';
	}

	// First look for included file in same directory as source file.

	strcpy(path, src_name[now_in]);
	*basename(path) = '\0';
	strcat(path, filename);
	fp = fopen(path, mode);
	if (fp) {
		if (path_used)
			*path_used = strdup(path);

		if (note_depend && outpass)
			printf("%s\n", path);
		return fp;
	}

	for (i = 0; i < incpath_cnt; i++) {
		sprintf(path, "%s/%s", incpath[i], filename);
		fp = fopen(path, mode);
		if (fp) {
			if (path_used)
				*path_used = strdup(path);
			if (note_depend && outpass)
				printf("%s\n", path);
			return fp;
		}
	}

	if (note_depend && outpass)
		printf("%s\n", filename);

	fp = fopen(filename, mode);
	if (fp && path_used)
		*path_used = strdup(filename);

	return fp;
}

void version()
{
	fprintf(stderr, "zmac version " VERSION "        http://48k.ca/zmac.html\n");
}

//
// Print out a usage message and exit.
//
void usage(char *msg, char *param)
{
	fprintf(stderr, msg, param);
	fprintf(stderr, "\n");
	version();
	fprintf(stderr, "usage: zmac [-8bcefghijJlLmnopstz] [-I dir] [-Pk=n] [-Dsym] file[.z]\n");
	fprintf(stderr, "other opts: --rel[7] --mras --zmac --dri --nmnv --z180 --fcal --dep --help --doc --version\n");
	fprintf(stderr, "  zmac -h for more detail about options.\n");
	exit(1);
}

void help()
{
	version();

	fprintf(stderr, "   --version\tshow version number\n");
	fprintf(stderr, "   --help\tshow this help message\n");
	fprintf(stderr, "   -8\t\tuse 8080 timings and interpretation of mnemonics\n");
	fprintf(stderr, "   -b\t\tno binary (.hex,.cmd,.cas, etc.) output\n");
	fprintf(stderr, "   -c\t\tno cycle counts in listing\n");
	fprintf(stderr, "   -e\t\terror list only\n");
	fprintf(stderr, "   -f\t\tprint if skipped lines\n");
	fprintf(stderr, "   -g\t\tdo not list extra code\n");
	fprintf(stderr, "   -h\t\tshow this information about options and quit\n");
	fprintf(stderr, "   -i\t\tdo not list include files\n");
	fprintf(stderr, "   -I dir\tadd 'dir' to include file search path\n");
	fprintf(stderr, "   -j\t\tpromote relative jumps to absolute as needed\n");
	fprintf(stderr, "   -J\t\twarn when a jump could be relative\n");
	fprintf(stderr, "   -l\t\tlist to standard output\n");
	fprintf(stderr, "   -L\t\tforce listing of everything\n");
	fprintf(stderr, "   -m\t\tprint macro expansions\n");
	fprintf(stderr, "   -n\t\tput line numbers off\n");
	fprintf(stderr, "   -o file.hex\toutput only named file (multiple -o allowed)\n");
	fprintf(stderr, "   -p\t\tput out four \\n's for eject\n");
	fprintf(stderr, "   -P\t\tformat listing for a printer\n");
	fprintf(stderr, "   -s\t\tdon't produce a symbol list\n");
	fprintf(stderr, "   -t\t\toutput error count instead of list of errors\n");
	fprintf(stderr, "   -z\t\tuse Z-80 timings and interpretation of mnemonics\n");
	fprintf(stderr, "   -Pk=num\tset @@k to num before assembly (e.g., -P4=10)\n");
	fprintf(stderr, "   -Dsym\tset symbol sym to 1 before assembly (e.g., define it)\n");
	fprintf(stderr, "   --od\tdir\tdirectory unnamed output files (default \"zout\")\n");
	fprintf(stderr, "   --oo\thex,cmd\toutput only listed file suffix types\n");
	fprintf(stderr, "   --xo\tlst,cas\tdo not output listed file suffix types\n");
	fprintf(stderr, "   --nmnv\tdo not interpret mnemonics as values in expressions\n");
	fprintf(stderr, "   --z180\tuse Z-180 timings and extended instructions\n");
	fprintf(stderr, "   --dep\tlist files included\n");
	fprintf(stderr, "   --mras\tlimited MRAS/EDAS compatibility\n");
	fprintf(stderr, "   --rel\toutput .rel file only (--rel7 for 7 character symbol names)\n");
	fprintf(stderr, "   --zmac\tcompatibility with original zmac\n");
	fprintf(stderr, "   --fcal\tidentifier in first column is always a label\n");
	fprintf(stderr, "   --doc\toutput documentation as HTML file\n");

	exit(0);
}

int main(int argc, char *argv[])
{
	struct item *ip;
	int  i, j;
	int  files;
	int used_o;
	int used_oo;
	char *zmac_args_env;
#ifdef DBUG
	extern  yydebug;
#endif

	separator = '\\';

	fin[0] = stdin;
	now_file = stdin;
	files = 0;
	used_o = 0;
	used_oo = 0;

	// Special flag for unit testing.
	if (argc > 1 && strcmp(argv[1], "--test") == 0)
		exit(!check_keytab());

	// To avoid typing typical command-line arguments every time we
	// allow ZMAC_ARGS environment variable to augment the command-line.
	zmac_args_env = getenv("ZMAC_ARGS");
	if (zmac_args_env) {
		int new_argc = 0;
		char *arg;
		char **new_argv;
		static char *zmac_args;

		// Overestimate to size of new argv vector.
		new_argv = malloc((argc + strlen(zmac_args_env) + 1) *
			sizeof *new_argv);
		// Save a copy because we (1) mutate it and (2) use it in argv.
		zmac_args = strdup(zmac_args_env);

		if (!new_argv || !zmac_args)
			error("malloc to support ZMAC_ARGS failed");

		memcpy(new_argv, argv, sizeof(*new_argv) * argc);
		new_argc = argc;

		arg = strtok(zmac_args, " \t");
		while (arg != NULL) {
			new_argv[new_argc++] = arg;
			arg = strtok(NULL, " \t");
		}

		argv = new_argv;
		argc = new_argc;
	}

	for (i=1; i<argc; i++) {
		int skip = 0;
		if (strcmp(argv[i], "--mras") == 0) {
			mras = 1;
			trueval = -1;
			continue;
		}

		if (strcmp(argv[i], "--dri") == 0) {
			driopt = 1;
			separator = '!';
			continue;
		}

		if (strcmp(argv[i], "--rel") == 0) {
			relopt = 6;
			continue;
		}

		if (strcmp(argv[i], "--rel7") == 0) {
			relopt = 7;
			continue;
		}

		if (strcmp(argv[i], "--zmac") == 0) {
			zcompat = 1;
			continue;
		}

		if (strcmp(argv[i], "--dep") == 0) {
			note_depend = 1;
			continue;
		}

		if (strcmp(argv[i], "--nmnv") == 0) {
			nmnvopt = 1;
			continue;
		}

		if (strcmp(argv[i], "--fcal") == 0) {
			first_always_label = 1;
			continue;
		}

		if (strcmp(argv[i], "--help") == 0) {
			help();
			continue;
		}

		if (strcmp(argv[i], "--doc") == 0) {
			extern void doc(void);
			doc();
			exit(0);
			continue; // not reached
		}

		if (strcmp(argv[i], "--version") == 0) {
			version();
			exit(0);
			continue; // not reached
		}

		if (strcmp(argv[i], "--z180") == 0) {
			/* Equivalent to .z180 */
			default_z80 = 2;
			continue;
		}

		if (strcmp(argv[i], "--od") == 0) {
			output_dir = argv[i = getoptarg(argc, argv, i)];
			continue;
		}

		if (strcmp(argv[i], "--oo") == 0) {
			if (!used_oo)
				stop_all_outf();

			suffix_list(argv[i = getoptarg(argc, argv, i)], 0);

			used_oo = 1;
			continue;
		}

		if (strcmp(argv[i], "--xo") == 0) {
			suffix_list(argv[i = getoptarg(argc, argv, i)], 1);
			continue;
		}

		if (argv[i][0] == '-' && argv[i][1] == '-')
			usage("Unknown option: %s", argv[i]);

		if (argv[i][0] == '-' && argv[i][1] == 'P' &&
			argv[i][2] >= '0' && argv[i][2] <= '9')
		{
			if (argv[i][3] == '=') {
				int overflow;
				int radix;
				int sign = 1;
				char *str = argv[i] + 4;
				if (*str == '-') {
					sign = -1;
					str++;
				}

				radix = convert(str, NULL, &overflow);
				if (radix <= 0 || overflow)
					usage("bad -Pn= parameter value", 0);

				mras_param[argv[i][2] - '0'] = sign * yylval.ival;
				mras_param_set[argv[i][2] - '0'] = 1;
			}
			else if (argv[i][3] == '\0') {
				mras_param[argv[i][2] - '0'] = -1;
				mras_param_set[argv[i][2] - '0'] = 1;
			}
			else
				usage("-Pn syntax error", 0);

			continue;
		}

		if (argv[i][0] == '-' && argv[i][1] == 'D') {
			struct cl_symbol *sym = malloc(sizeof(struct cl_symbol));
			if (!argv[i][2])
				usage("missing symbol name for -D", 0);

			sym->name = argv[i] + 2;
			sym->next = cl_symbol_list;
			cl_symbol_list = sym;

			continue;
		}

		if (*argv[i] == '-') while (*++argv[i]) {
			switch(*argv[i]) {

			case '8':	/* Equivalent to .8080 */
				default_z80 = 0;
				continue;

			case 'b':	/*  no binary  */
				for (j = 0; j < CNT_OUTF; j++)
					if (strcmp(outf[j].suffix, "lst") != 0)
						outf[j].no_open = 1;
				continue;

			case 'c':	/*  no cycle counts in listing */
				copt-- ;
				continue;

#ifdef DBUG
			case 'd':	/*  debug  */
				yydebug++;
				continue;
#endif

			case 'e':	/*  error list only  */
				eopt = 0;
				edef = 0;
				continue;

			case 'f':	/*  print if skipped lines  */
				fopt++;
				fdef++;
				continue;

			case 'g':	/*  do not list extra code  */
				gopt = 0;
				gdef = 0;
				continue;

			case 'h':
				help();
				continue;

			case 'i':	/* do not list include files */
				iopt = 1 ;
				continue ;

			case 'I':
				if (argv[i][1])
					add_incpath(argv[i] + 1);
				else
					add_incpath(argv[i = getoptarg(argc, argv, i)]);
				skip = 1;
				break;

			case 'l':	/*  list to stdout */
				fout = stdout;
				continue;

			case 'L':	/*  force listing of everything */
				lston++;
				continue;

			case 'j':	// promote relative jumps to absolute as needed
				default_jopt = 1;
				continue;

			case 'J':	// error when JR instructions could replace JP
				default_JPopt = 1;
				continue;

			case 'm':	/*  print macro expansions  */
				mdef++;
				mopt++;
				continue;

			case 'n':	/*  put line numbers off */
				nopt-- ;
				continue;

			case 'o':	/*  select output */
				{
					char *outfile = 0;
					char *sfx;
					int found = 0;

					if (!used_o)
						stop_all_outf();

					if (argv[i][1])
						outfile = argv[i] + 1;
					else
						outfile = argv[i = getoptarg(argc, argv, i)];

					for (sfx = outfile; !found && *sfx; sfx++) {
						if (*sfx != '.')
							continue;

						for (j = 0; j < CNT_OUTF; j++) {
							if (ci_strcmp(sfx + 1, outf[j].suffix) == 0) {
								outf[j].no_open = 0;
								outf[j].wanted = 1;
								outf[j].filename = outfile;
								found = 1;
								break;
							}
						}
					}
					if (!found)
						usage("output file '%s' has unknown suffix", outfile);
				}
				used_o = 1;
				skip = 1;
				break;

			case 'p':	/*  put out four \n's for eject */
				popt-- ;
				continue;

			case 'P':	// GWP - printer style output (headers, page separation, etc.)
				printer_output = 1;
				continue;

			case 's':	/*  don't produce a symbol list  */
				sopt++;
				continue;

			case 't':	/*  output only number of errors */
				topt = 0;
				continue;

			case 'z':	/* Equivalent to .z80 */
				default_z80 = 1;
				continue;

			default:	/*  error  */
				{
					char badopt[2] = { argv[i][0], 0 };
					usage("Unknown option: %s", badopt);
				}
			}
			if (skip)
				break;
		}
		else if (files++ == 0) {
			sourcef = argv[i];
			strcpy(src, sourcef);
			if ((now_file = fopen(src, "r")) == NULL) {
				if (!*getsuffix(src))
					suffix(src, ".z");
				if ((now_file = fopen(src, "r")) == NULL)
					usage("Cannot open source file '%s'", src);
			}
			now_in = 0;
			fin[now_in] = now_file ;
			src_name[now_in] = src ;
		} else if (files)
			usage("Too many arguments", 0);
	}


	if (files == 0)
		usage("No source file", 0);

	// .wav file outputs must ensure their .cas antecedents are generated.
	// And also check for .rel output and set relopt while we're at it.
	for (j = 0; j < CNT_OUTF; j++) {
		char *p;

		if (strcmp(outf[j].suffix, "rel") == 0 && !outf[j].no_open && !relopt)
			relopt = 6;

		p = strchr(outf[j].suffix, '.');
		// Only .wav file that open matter and only if .cas doesn't open.
		if (!p || strcmp(p, ".wav") != 0 || outf[j].no_open || !outf[j + 1].no_open)
			continue;

		outf[j + 1].no_open = 0;
		outf[j + 1].temp = 1;
		if (outf[j].filename) {
			// Replace .wav suffix with .cas.  This is safe for
			// now as only -o can choose a filename and it must end
			// with outf[j]'s suffix to be put in outf[j].
			outf[j + 1].filename = strdup(outf[j].filename);
			p = strrchr(outf[j + 1].filename, '.');
			strcpy(p + 1, "cas");
		}
	}

	if (relopt) {
		for (j = 0; j < CNT_OUTF; j++) {
			if (strcmp(outf[j].suffix, "lst") != 0)
			{
				outf[j].no_open = strcmp(outf[j].suffix, "rel") != 0;
			}
		}
	}

	for (j = 0; j < CNT_OUTF; j++) {
		if (outf[j].no_open || *outf[j].fpp)
			continue;

		if (!outf[j].filename) {
			char suffix[32];
			strcpy(suffix, ".");
			strcat(suffix, outf[j].suffix);
			outpath(oth, sourcef, suffix);
			outf[j].filename = strdup(oth);
		}

		*outf[j].fpp = fopen(outf[j].filename, outf[j].mode);
		if (!*outf[j].fpp) {
			fprintf(stderr, "Cannot create file '%s'", outf[j].filename);
			clean_outf();
			exit(1);
		}
	}

	if (fbds) {
		fprintf(fbds, "binary-debuggable-source\n");
		fprintf(fbds, "%04x %04x f %s\n", dollarsign, emit_addr, src_name[now_in]);
	}

	// Tape header
	for (i = 0; i < 255; i++) {
		if (flcas) fputc(0, flcas);
		if (flnwcas) fputc(0, flnwcas);
		if (fcas) fputc(0x55, fcas);
		if (ftcas) fputc(0, ftcas);
	}
	if (flcas) fputc(0xA5, flcas);
	if (flnwcas) fputc(0xA5, flnwcas);
	if (fcas) fputc(0x7F, fcas);
	if (ftcas) fputc(0xA5, ftcas);

	casname(oth, sourcef, 6);
	putcas(0x55);
	for (i = 0; i < 6; i++)
		putcas(oth[i]);

	if (relopt) {
		strncpy(progname, basename(sourcef), sizeof progname);
		progname[sizeof progname - 1] = '\0';
	}

	// mfopen() is always in-memory not a temporary file.

	mfile = mfopen("does-not-matter","w+b") ;

	/*
	 *  get the time
	 */
	time(&now);
	timp = ctime(&now);
	timp[16] = 0;
	timp[24] = 0;

	title = sourcef;
	/*
	 * pass 1
	 */
#ifdef DEBUG
	fputs("DEBUG-pass 1\n", stderr) ;
#endif
	clear();
	setvars();
	npass = 1;
	outpass = 0;
	yyparse();

	// GWP - errors should stop us, but the listing is very useful.

	pass2++;

	for (npass = 2; npass < MAXPASS; npass++) {
		if (passfail || npass == MAXPASS - 1)
			outpass = 1;

		if (outpass) {
			putrelcmd(RELCMD_PROGNAME);
			putrelname(progname);
		}

		for (ip = itemtab - 1; ++ip < itemmax; ) {
			// Output list of public labels.  m80 will let
			// equates and aseg values be public so we do, too.
			if (outpass && ip->i_token && (ip->i_scope & SCOPE_PUBLIC)) {
				putrelcmd(RELCMD_PUBLIC);
				putrelname(ip->i_string);
			}

			/* reset use count */
			ip->i_uses = 0 ;

			/* set macro names, equated and defined names */
			switch	(ip->i_token) {
			case MNAME:
				ip->i_token = UNDECLARED;
				break;

			case EQUATED:
				ip->i_token = WASEQUATED;
				break;

			case DEFLED:
				if (zcompat)
					ip->i_token = UNDECLARED;
				break;
			}
		}

		if (outpass) {
			// m80 outputs data size as an absolute value, but
			// code size as code segment relative.  Odd, but
			// I'll follow suit.
			putrelcmd(RELCMD_DATASIZE);
			putrelsegref(SEG_ABS, seg_size[SEG_DATA]);

			putrelcmd(RELCMD_CODESIZE);
			putrelsegref(SEG_CODE, seg_size[SEG_CODE]);
		}

		for (ip = itemtab - 1; ++ip < itemmax; ) {
			if (ip->i_token != COMMON)
				continue;

			// TODO: perhaps have WASCOMMON but this will suffice
			ip->i_token = UNDECLARED;

			putrelcmd(RELCMD_COMSIZE);
			putrelsegref(SEG_ABS, ip->i_value);
			putrelname(ip->i_string);
		}

		// In case we hit 'end' inside an included file
		while (now_in > 0) {
			fclose(fin[now_in]);
			free(src_name[now_in]);
			now_file = fin[--now_in];
			nextline_peek = linepeek[now_in];
		}
		setvars();
		clear_instdata_flags();
		fseek(now_file, (long)0, 0);

	#ifdef DEBUG
		fprintf(stderr, "DEBUG- pass %d\n", npass) ;
	#endif

		yyparse();

		if (outpass || passfail)
			break;

		if (!passretry)
			outpass = 1;
	}

	flushbin();
	flushoth();

	if (fbuf)
		putc(':', fbuf);

	if (xeq_flag) {
		if (fbuf) {
			puthex(0, fbuf);
			puthex(xeq >> 8, fbuf);
			puthex(xeq, fbuf);
			puthex(1, fbuf);
			puthex(255-(xeq >> 8)-xeq, fbuf);
		}
		if (fcmd) {
			fprintf(fcmd, "%c%c%c%c", 2, 2, xeq, xeq >> 8);
			fflush(fcmd);
		}
		putcas(0x78);
		putcas(xeq);
		putcas(xeq >> 8);
		if (fmds)
			fprintf(fmds, "pc=$%04x\ng\n", xeq);
	}
	else {
		// SYSTEM cassette files must have an execution address.
		// Without one we simply do not output .cas or .wav SYSTEM.

		int i;
		for (i = 0; i < CNT_OUTF; i++) {
			if (*outf[i].fpp && outf[i].system) {
				fclose(*outf[i].fpp);
				*outf[i].fpp = NULL;
				// unlink is an intended implicit declaration -- silence the gcc warning.
				// Old gcc's don't permit #pragman in a function.
				// Uncomment to suppress the warning.
				//#pragma GCC diagnostic push
				//#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
				unlink(outf[i].filename);
				//#pragma GCC diagnostic pop
				if (outf[i].wanted)
					fprintf(stderr, "Warning: %s not output -- no entry address (forgot \"end label\")\n", outf[i].filename);
			}
		}

		if (fbuf) {
			puthex(0, fbuf);
			puthex(0, fbuf);
			puthex(0, fbuf);
			puthex(1, fbuf);
			puthex(255, fbuf);
		}
	}

	if (fbuf) {
		putc('\n', fbuf);
		fflush(fbuf);
	}

	// "Play Cas" seems to require trailing zeros to work
	// properly.  And we need to output at least one zero byte
	// to flush out the final high speed bits.
	#define CAS_PAD 6
	for (i = 0; i < CAS_PAD; i++)
		putcas(0);

	if (relopt) {
		struct item *ip;
		// Output external symbols and value of public symbols
		for (ip = itemtab; ip < itemmax; ip++) {
			if (ip->i_token == UNDECLARED && (ip->i_scope & SCOPE_EXTERNAL)) {
				putrelcmd(RELCMD_EXTCHAIN);
				// Chain value will have top two bits set appropriately
				putrelextaddr(ip->i_chain);
				putrelname(ip->i_string);
			}
			if (ip->i_scope & SCOPE_PUBLIC)
			{
				putrelcmd(RELCMD_PUBVALUE);
				putrelsegref(ip->i_scope, ip->i_value);
				putrelname(ip->i_string);
			}
		}

		// End module, entry address if any
		putrelcmd(RELCMD_ENDMOD);
		putrelextaddr(rel_main);
		flushrel(); // byte alignment expected after end module

		// End .rel file
		putrelcmd(RELCMD_ENDPROG);
		flushrel();
	}

	if (xeq_flag == 0) {
#if WIN32
		char *username = getenv("USERNAME");
		if (username && strcmp(username, "George") == 0 && !relopt) {
			CONSOLE_SCREEN_BUFFER_INFO inf;
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleScreenBufferInfo(hOut, &inf);
			SetConsoleTextAttribute(hOut, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
			fprintf(stderr, "Warning: no entry address (forgot \"end label\")\n");
			fflush(stderr);
			SetConsoleTextAttribute(hOut, inf.wAttributes);
		}
#endif
	}
	else if (fbds) {
		fprintf(fbds, "%04x e\n", xeq);
	}

	if (fcim || fams || ftap || ftcas) {
		int low = 0;
		int high = sizeof(memory) - 1;
		int chk;
		int filelen;
		int i;
		char leafname[] = "FILENAMEBIN";

		while (low < sizeof(memory) && (memflag[low] & (MEM_INST | MEM_DATA)) == 0)
			low++;

		while (high >= 0 && (memflag[high] & (MEM_INST | MEM_DATA)) == 0)
			high--;

		if (high >= low && fcim)
			fwrite(memory + low, high + 1 - low, 1, fcim);

		filelen = (high + 1) - low;

		// AMSDOS binary file output (A for Amstrad, code from zmac 1.3)
		if (fams) {
			chk = 0;
			putc(0, fams);
			for (i = 0; i < 11; i++) {
				putc(leafname[i], fams);
				chk += leafname[i];
			}
			for (i = 0; i < 6; i++)
				putc(0, fams);

			putc(2, fams); // Unprotected binary
			chk += 2;
			putc(0, fams);
			putc(0, fams);
			putc(low & 0xff, fams);
			chk += low & 0xff;
			putc(low >> 8, fams);
			chk += low >> 8;
			putc(0, fams);
			putc(filelen & 0xff, fams);
			chk += filelen & 0xff;
			putc(filelen >> 8, fams);
			chk += filelen >> 8;
			putc(xeq & 0xff, fams);
			chk += xeq & 0xff;
			putc(xeq >> 8, fams);
			chk += xeq >> 8;
			for (i = 28; i < 64; i++)
				putc(0, fams);

			putc(filelen & 0xff, fams);
			chk += filelen & 0xff;
			putc(filelen >> 8, fams);
			chk += filelen >> 8;
			putc(0, fams); // this would be used if filelen > 64K
			putc(chk & 0xff, fams);
			putc(chk >> 8, fams);

			for (i = 69; i < 128; i++)
				putc(0, fams);

			if (filelen > 0)
				fwrite(memory + low, filelen, 1, fams);

			if (filelen & 0x7f)
				putc(0x1a, fams); // CP/M EOF character
		}

		if (ftap)
			write_tap(filelen, low, memory + low);
 
		if (ftcas)
			write_250(low, high);
	}

	// Output .wav files noting the padding bytes to ignore.
	writewavs(0, CAS_PAD, CAS_PAD);

	if (fbds) {
		struct item *tp;

		for (tp = itemtab; tp < itemmax; tp++) {
			if (tp->i_token == LABEL)
				fprintf(fbds, "%04x a %s\n", tp->i_value, tp->i_string);
			else if (tp->i_token == EQUATED)
				fprintf(fbds, "%04x v %s\n", tp->i_value, tp->i_string);
		}
	}

	if (fout)
		fflush(fout);
	if (writesyms)
		outsymtab(writesyms);
	compactsymtab();
	if (eopt)
		erreport();
	if (!sopt)
		putsymtab();
	if (fout) {
		eject();
		fflush(fout);
	}
	// GWP - some things (like balance errors in macro definitions) do
	// not show up until you use them.  So just in case we print an error
	// count here as not to confuse the programmer who is unlikely to check
	// the listing for errors if none are shown in the command window.
	if (counterr() > 0)
		fprintf(stderr, "%d errors (see listing if no diagnostics appeared here)\n", counterr());
	if (countwarn() > 0)
		fprintf(stderr, "%d warnings (see listing if no diagnostics appeared here)\n", countwarn());

	clean_outf_temp();
	if (counterr() > 0)
		clean_outf();
	exit(counterr() > 0);
}

int getoptarg(int argc, char *argv[], int i)
{
	i++;
	if (i < argc)
		return i;

	usage("missing argument for %s option", argv[i - 1]);
	return i; // not actually reached
}

void stop_all_outf()
{
	int i;
	for (i = 0; i < CNT_OUTF; i++)
		outf[i].no_open = 1;
}

void clean_outf()
{
	int i;

	for (i = 0; i < CNT_OUTF; i++) {
		if (*outf[i].fpp) {
			int size = ftell(*outf[i].fpp);
			fclose(*outf[i].fpp);
			*outf[i].fpp = NULL;
			// Listing file can stick around, but not if empty.
			if (strcmp(outf[i].suffix, "lst") != 0 || size == 0)
				unlink(outf[i].filename);
		}
	}
}

void clean_outf_temp()
{
	int i;
	for (i = 0; i < CNT_OUTF; i++) {
		if (*outf[i].fpp && outf[i].temp) {
			fclose(*outf[i].fpp);
			*outf[i].fpp = NULL;
			unlink(outf[i].filename);
		}
	}
}

// Set output files to open or not using a comma-separated list of suffixes.
// Loops over the outf[] suffix so that "wav" can be used to eliminate all
// .wav files.
void suffix_list(char *sfx_lst, int no_open)
{
	while (sfx_lst) {
		int i;
		char *p = strchr(sfx_lst, ',');
		if (p)
			*p = '\0';

		// Allow prefix '.' in case user puts them in.
		while (*sfx_lst == '.')
			sfx_lst++;

		for (i = 0; i < CNT_OUTF; i++) {
			char *sfx;
			for (sfx = outf[i].suffix; sfx; ) {
				if (ci_strcmp(sfx, sfx_lst) == 0) {
					outf[i].no_open = no_open;
					if (!no_open)
						outf[i].wanted = 1;
				}
				sfx = strchr(sfx, '.');
				if (sfx)
					sfx++;
			}
		}

		sfx_lst = p ? p + 1 : 0;
	}
}

void equate(char *id, int value, int scope)
{
	struct item *it;

	it = locate(id);
	if (!it->i_token) {
		nitems++;
		it->i_value = value;
		it->i_token = EQUATED;
		it->i_pass = npass;
		it->i_scope = scope;
		it->i_uses = 0;
		it->i_string = malloc(strlen(id)+1);
		strcpy(it->i_string, id);
	}

	// This variable test true for ifdef
	// This is a slightly subtle way to ensure it->i_pass == npass
	// Setting it to npass or npass + 1 doesn't always work due to
	// the different contexts in which setvars() is called.
	if (scope & (SCOPE_CMD_D | SCOPE_CMD_P))
		it->i_pass++;
}

/*
 *  set some data values before each pass
 */
void setvars()
{
	int  i;
	struct cl_symbol *sym;

	peekc = NOPEEK;
	inpptr = 0;
	nextline_peek = NOPEEK;
	raw = 0;
	linein[now_in] = 0;
	exp_number = 0;
	emitptr = emitbuf;
	lineptr = linebuf;
	ifptr = ifstack;
	*ifptr = 0;
	dollarsign = 0;
	emit_addr = 0;
	olddollar = 0;
	oldothdollar = 0;
	phaseflag = 0;
	for (i=0; i<FLAGS; i++) err[i] = 0;
	tstates = 0;
	ocf = 0;
	llseq = 0;
	passfail = 0;
	passretry = 0;
	njrpromo = 0;
	jopt = default_jopt;
	JPopt = default_JPopt;
	strcpy(modstr, "@@@@");
	segment = SEG_CODE;
	cur_common = NULL;
	memset(seg_pos, 0, sizeof(seg_pos));
	memset(seg_size, 0, sizeof(seg_size));
	segchange = 0;
	z80 = default_z80;
	outrec = 0;
	outlen = 0;

	param_parse = 0;
	arg_reset();

	mfptr = 0;
	mfseek(mfile, mfptr, 0);

	// TODO - maybe these could be just as well handled lexically
	// like the 8080 opcodes in DRI mode?
	// These are slightly harmful, but not too bad.  Only
	// absolutely necessary for MAC compatibility.  But there's
	// some use in having them available always.

	equate("b", 0, SCOPE_BUILTIN);
	equate("c", 1, SCOPE_BUILTIN);
	equate("d", 2, SCOPE_BUILTIN);
	equate("e", 3, SCOPE_BUILTIN);
	equate("h", 4, SCOPE_BUILTIN);
	equate("l", 5, SCOPE_BUILTIN);
	equate("m", 6, SCOPE_BUILTIN);
	equate("a", 7, SCOPE_BUILTIN);

	equate("sp", 6, SCOPE_BUILTIN);
	equate("psw", 6, SCOPE_BUILTIN);

	// TODO - these are now handled lexically in --dri mode
	// There are a large number of symbols to add to support
	// "LXI H,MOV" and the like.

	// Technically only MRAS has these, but I'll wait and see if
	// anyone actually uses @@0 through @@9.  If so, then we can
	// DEFL them instead of EQU.
	for (i = 0; i < 10; i++) {
		char var[4];
		var[0] = '@';
		var[1] = '@';
		var[2] = '0' + i;
		var[3] = '\0';
		equate(var, mras_param[i], mras_param_set[i] ? SCOPE_CMD_P : SCOPE_BUILTIN);
	}

	for (sym = cl_symbol_list; sym; sym = sym->next)
		equate(sym->name, 1, SCOPE_CMD_D);

	reset_import();
}

//
// Clear out cycle counts and memory.
//

void clear()
{
	int i;

	for (i = 0; i < sizeof(memory) / sizeof(memory[0]); i++)
	{
		memory[i] = 0;
		memflag[i] = 0;
		tstatesum[i] = 0;
	}
}

void clear_instdata_flags()
{
	int i;

	for (i = 0; i < sizeof(memory) / sizeof(memory[0]); i++)
		memflag[i] &= ~(MEM_DATA | MEM_INST);
}

void setmem(int addr, int value, int type)
{
	value &= 0xff;
	memory[addr] = value;
	memflag[addr] |= type;
}

/*
 *  print out an error message and die
 */
void error(char *as)
{
	*linemax = 0;
	if (fout) {
		fprintf(fout, "%s\n", linebuf);
		fflush(fout);
	}
	fprintf(stderr, "%s\n", as) ;
	clean_outf();
	exit(1);
}


/*
 * Compact the symbol table, removing unused, UNDECLARED entries,
 * macros and built-in identifiers.
 */
void compactsymtab()
{
	struct item *tp, *fp;

	if (!nitems)
		return;

	tp = itemtab;
	tp--;
	for (fp = itemtab; fp<itemmax; fp++) {
		if (fp->i_token == UNDECLARED && !(fp->i_scope & SCOPE_EXTERNAL)) {
			nitems--;
			continue;
		}
		if (fp->i_token == 0)
			continue;

		// Don't list macros or internally defined symbols
		if (fp->i_token == MNAME || (fp->i_scope & SCOPE_BUILTIN)) {
			nitems--;
			continue;
		}

		tp++;
		itemcpy(tp, fp);
	}

	tp++;
	tp->i_string = "{";	/* } */

	/*  sort the table */
	custom_qsort(0, nitems-1);
}

/*
 *  output the symbol table
 */
void putsymtab()
{
	int  i, j, k, t, rows;
	char c, seg = ' '; //, c1;
	int numcol = printer_output ? 4 : 1;
	struct item *tp;

	if (!nitems || !fout)
		return;

	title = "**  Symbol Table  **";

	rows = (nitems+numcol-1) / numcol;
	if (rows+5+line > 60)
		eject();
	lineout();
	fprintf(fout,"\n\n\nSymbol Table:\n\n") ;
	line += 4;

	for (i=0; i<rows; i++) {
		for(j=0; j<numcol; j++) {
			k = rows*j+i;
			if (k < nitems) {
				tp = &itemtab[k];
				t = tp->i_token;
				c = ' ' ;
				if (t == EQUATED || t == DEFLED)
					c = '=' ;
				if (t == COMMON)
					c = '/';

				//if (tp->i_uses == 0)
				//	c1 = '+' ;
				//else
				//	c1 = ' ' ;

				// GWP - decided I don't care about uses
				// even if it were accurate.
				// TODO: Should use maxsymbol size in there,
				// but makes output harder to read.

				fprintf(fout, "%-15s%c", tp->i_string, c);

				if (tp->i_scope & SCOPE_ALIAS)
					fprintf(fout, "\"%s\"",
						keytab[tp->i_value].i_string);

				else {
					if (relopt)
						seg = SEGCHAR(tp->i_scope & SCOPE_SEGMASK);

					if (tp->i_value >> 16)
						fprintf(fout, "%08X%c", tp->i_value, seg);
					else if (tp->i_value >> 8)
						fprintf(fout, "%4X%c    ", tp->i_value, seg);
					else
						fprintf(fout, "%02X%c      ", tp->i_value, seg);

					fprintf(fout, " %d", tp->i_value);

					if (tp->i_scope & SCOPE_EXTERNAL)
						fprintf(fout, " (extern)");

					if (tp->i_scope & SCOPE_PUBLIC)
						fprintf(fout, " (public)");

					if (tp->i_scope & SCOPE_CMD_P)
						fprintf(fout, " (command line -P)");

					if (tp->i_scope & SCOPE_CMD_D)
						fprintf(fout, " (command line -D)");
				}
			}
		}
		lineout();
		putc('\n', fout);
	}
}




/*
 *  put out error report
 */
void erreport()
{
	int i, numerr, numwarn;

	if (!fout)
		return;

	if (line > 49) eject();
	lineout();
	numerr = 0;
	for (i=0; i<FIRSTWARN; i++) numerr += keeperr[i];
	numwarn = 0;
	for (i = FIRSTWARN; i < FLAGS; i++) numwarn += keeperr[i];
	if (numerr || numwarn) {
		fputs("\n\n\nError + Warning report:\n\n", fout);
		fprintf(fout, "%6d errors\n", numerr);
		fprintf(fout, "%6d warnings\n", numwarn);
		line += 6;
	} else {
		fputs("\n\n\nStatistics:\n", fout);
		line += 3;
	}

	for (i=0; i<FLAGS; i++)
		if (keeperr[i]) {
			lineout();
			fprintf(fout, "%6d %c -- %s %s\n",
				keeperr[i], errlet[i], errname[i],
				i < FIRSTWARN ? "error" : "warnings");
		}

	if (line > 52) eject();
	lineout();
	fprintf(fout, "\n%6d\tpasses\n", npass);
	fprintf(fout, "%6d\tjr promotions\n", njrpromo);
	fprintf(fout, "%6d\tsymbols\n", nitems);
	fprintf(fout, "%6d\tbytes\n", nbytes);
	line += 4;
	if (mfptr) {
		if (line > 53) eject();
		lineout();
		fprintf(fout, "\n%6d\tmacro calls\n", exp_number);
		fprintf(fout, "%6d\tmacro bytes\n", mfptr);
		fprintf(fout, "%6d\tinvented symbols\n", invented/2);
		line += 3;
	}
}

/*
 * count errors (GWP - added to set exit code)
 */
int counterr()
{
	int i, numerr = 0;
	for (i=0; i<FIRSTWARN; i++) numerr += keeperr[i];
	return numerr;
}

// Count warnings
int countwarn()
{
	int i, numwarn = 0;
	for (i = FIRSTWARN; i < FLAGS; i++)
		numwarn += keeperr[i];
	return numwarn;
}

char *mlook;

int nextmac()
{
	int ch;

	if (mlook) {
		if (*mlook) {
			ch = *mlook++;
			addtoline(ch);
		}
		else
			mlook = 0;
	}

	if (!mlook)
		ch = nextchar();

	return ch;
}

void putm_param_ref(struct item *param);
void putm_str(char *str, int look_for_param);

/*
 *  lexical analyser for macro definition
 */
void mlex(char *look)
{
	char  *p;
	int  c;
	int  t;
	int quote = 0, ampdrop = 0;
	struct item *param, *key;
	char symbuf[TEMPBUFSIZE];
	char *symbufmax = symbuf+TEMPBUFSIZE-1;

	/*
	 *  move text onto macro file, changing formal parameters
	 */
#ifdef	M_DEBUG
	fprintf(stderr,"enter 'mlex' at %d\n", mfptr) ;
#endif
	inmlex++;

	mlook = look;

	c = nextmac();
	for (;;) {
		int octo = 0, amp = 0, caret = 0;

		if (c == '#' || c == '&' || c == '^') {
			int esc = c;
			c = nextmac();
			if (charclass[c] != STARTER && charclass[c] != LETTER) {
				if (esc != '&' || !ampdrop)
					putm(esc);
				ampdrop = 0;
				continue;
			}
			if (esc == '#')
				octo = 1;
			else if (esc == '&')
				amp = 1;
			else
				caret = 1;
		}

		switch(charclass[c]) {

		case DIGIT:
			while (numpart[c]) {
				putm(c);
				c = nextmac();
			}
			continue;

		case STARTER:
		case LETTER:
			t = 0;
			p = symbuf;
			do {
				if (p >= symbufmax) {
					*symbufmax = '\0';
					printf("was parsing '%s' in macro definition\n", tempbuf);
					error(symlong);
				}
				*p++ = c;
				if (t < MAXSYMBOLSIZE)
					tempbuf[t++] = (c >= 'A' && c <= 'Z')  ?
						c+'a'-'A' : c;
				c = nextmac();
			} while	(charclass[c]==LETTER || charclass[c]==DIGIT || charclass[c]==STARTER);

			tempbuf[t] = 0;
			*p++ = '\0';
			p = symbuf;

			key = keyword(tempbuf);
			t = key ? key->i_token : 0;

			// Hmmm, that "item_lookup" is putting crap in the table, yes?
			if (mras)
				param = item_substr_lookup(tempbuf, MPARM, paramtab, PARAMTABSIZE);
			else
				param = item_lookup(tempbuf, paramtab, PARAMTABSIZE);

			// Accept almost anything as a parameter.  But a few
			// keywords will just screw things up royally.
			if (param && t != ENDM && t != REPT && t != IRPC && t != IRP && t != MACRO)
				t = param->i_token;

			// Caret escaping kills all expansion
			// And interpretation of ENDM, etc.  For better or worse.
			if (caret)
				t = 0;

			if (t == MPARM) {
				int pfx = octo || amp || c == '&';
				// # prefix only allowed if declared that way
				if (octo != param->i_scope)
					t = 0;
				else
					octo = 0;

				// Expansion in quoted strings only with prefix character.
				if (quote && !pfx && !zcompat)
					t = 0;

				amp = 0; // parameter substitution, eat '&'
			}
			else if (t && quote)
				t = 0;

			if (ampdrop)
				amp = 0;

			ampdrop = c == '&' && t == MPARM;

			if (octo) putm('#');
			if (amp) putm('&');

			if (t != MPARM) {
				putm(symbuf[0]);
				putm_str(symbuf + 1, mras && !quote);
			}
			else {
				putm_param_ref(param);
				// Only in MRAS will we have a remainder string
				putm_str(symbuf + strlen(param->i_string), mras && !quote);
			}

			if (t == ENDM) {
				if (--inmlex == 0)
					goto done;
			}
			else if (t == REPT || t == IRPC || t == IRP || t == MACRO) {
				inmlex++;
			}

			continue;

		case F_END:
			errwarn(warn_general, "macro definition went until end of file");
			if (expptr) {
				popsi();
				c = nextmac();
				continue;
			}

			goto done;

		default:
			switch (c) {
			case '\n':
				quote = 0;
				list1();
				break;
			case ';':
				if (!quote) {
					while (c != '\n' && c != 0) {
						putm(c);
						c = nextmac();
					}
					continue;
				}
				break;
			case '\'':
			case '"':
				if (c == quote)
					quote = 0;
				else
					quote = c;
				break;
			default:
				break;
			}
			if (c != '\1' && c != '`') putm(c);
			c = nextmac();
		}
	}

	/*
	 *  finish off the file entry
	 */
done:
	while(c != EOF && c != '\n' && c != '\0') c = nextmac();
	inmlex++;
	list1();
	inmlex--;
	// WHY two newlines??
//	putm('\n');
	putm('\n');
	putm(0);

	// TODO - here's where we could save parameter names for MRAS
	for (c = 0; c < PARAMTABSIZE; c++) {
		if (paramtab[c].i_token == MPARM) {
			free(paramtab[c].i_string);
			paramtab[c].i_string = NULL;
			paramtab[c].i_token = 0;
		}
	}
	inmlex = 0;
#ifdef	M_DEBUG
	fprintf(stderr,"exit 'mlex' at %d\n", mfptr) ;
#endif
}

void putm_param_ref(struct item *param)
{
	if (*(param->i_string) == '?' || param->i_chain)
		putm('\2');
	else
		putm('\1');

	putm(param->i_value + 'A');
}

void putm_str(char *str, int look_for_param)
{
	for (; *str; str++) {
		if (look_for_param) {
			struct item *param = item_substr_lookup(str, MPARM, paramtab, PARAMTABSIZE);
			if (param) {
				putm_param_ref(param);
				str += strlen(param->i_string) - 1;
				continue;
			}
		}
		putm(*str);
	}
}

int str_getch(struct argparse *ap)
{
	int ch = ap->user_peek;
	if (ch >= 0) {
		ap->user_peek = -1;
		return ch;
	}
	if (!ap->user_ptr || ap->user_ptr[ap->user_int] == '\0')
		return '\0';

	return ap->user_ptr[ap->user_int++];
}

int arg_getch(struct argparse *ap)
{
	(void)ap; // suppress warning
	return nextchar();
}

void arg_start()
{
	arg_reset();
	arg_flag = 1;
}

void arg_reset()
{
	arg_flag = 0;

	arg_state.arg = tempbuf;
	arg_state.argsize = sizeof tempbuf;
	arg_state.peek = &peekc;
	arg_state.getch = arg_getch;
	arg_state.macarg = 0;
	arg_state.user_ptr = 0;
	arg_state.user_int = 0;
	arg_state.didarg = 0;
	arg_state.numarg = 0;
}

/*
 *  lexical analyser for the arguments of a macro call
 */
int getarg(struct argparse *ap)
{
	int c;
	char *p;
	int quote;
	int depth;

	*ap->arg = 0;
	while (charclass[c = ap->getch(ap)] == SPACE);

	switch(c) {

	case '\0':
		if (!ap->user_ptr)
			popsi(); // a seemingly unlikely case?
	case '\n':
	case ';':
		if (!ap->didarg && ap->numarg) {
			*ap->peek = c;
			ap->didarg = 1;
			ap->numarg++;
			return ARG;
		}
		ap->didarg = 0;
		ap->numarg = 0;
		return c;

	case ',':
		if (!ap->didarg) {
			ap->didarg = 1;
			*ap->peek = c;
			ap->numarg++;
			return ARG;
		}
		ap->didarg = 0;
		return c;

	case '\'':
	case '\"':
		quote = c;
		p = ap->arg;
		if (!zcompat)
			*p++ = c;

		do {
			c = ap->getch(ap);
			if (c == '\0' || c == '\n') {
				*ap->peek = c;
				*p = 0;
				err[bflag]++;
				ap->didarg = 1;
				ap->numarg++;
				return ARG;
			}
			else if (c == quote) {
				if ((c = ap->getch(ap)) != quote) {
					if (!zcompat)
						*p++ = quote;
					*ap->peek = c;
					*p = '\0';
					ap->didarg = 1;
					ap->numarg++;
					return ARG;
				}
			}
			else
				*p++ = c;
		} while (p < ap->arg + ap->argsize - 1);
		ap->arg[ap->argsize - 1] = '\0';
		printf("was parsing macro argument '%s'\n", ap->arg);
		error(symlong);
		return 0; // not reached

	case '<':
		depth = 1;
		p = ap->arg;
		do {
			c = ap->getch(ap);
			if (c == '\0' || c == '\n') {
				*ap->peek = c;
				*p = 0;
				err[bflag]++;
				ap->didarg = 1;
				ap->numarg++;
				return ARG;
			}
			if (c == '>') {
				depth--;
				if (depth == 0) {
					*p = '\0';
					ap->didarg = 1;
					ap->numarg++;
					return ARG;
				}
			}
			else if (c == '<')
				depth++;

			*p++ = c;
		} while (p < ap->arg + ap->argsize - 1);
		ap->arg[ap->argsize - 1] = '\0';
		printf("was parsing macro argument '%s'\n", ap->arg);
		error(symlong);
		return 0; // not reached

	default:  /* unquoted string */
		if (c == '%' && ap->macarg) {
			ap->didarg = 1;
			ap->numarg++;
			return c;
		}

		p = ap->arg;
		*ap->peek = c;

		do {
			c = ap->getch(ap);
			switch(c) {
			case '\0':
			case '\n':
			case '\t':
			case ' ':
			case ',':
				*ap->peek = c;
				*p = '\0';
				ap->didarg = 1;
				ap->numarg++;
				return ARG;
			case '^':
				c = ap->getch(ap);
				switch (c) {
				case ',':
				case '^':
				case ' ':
				case '\t':
					*p++ = c;
					break;
				default:
					*p++ = '^';
					*ap->peek = c;
					break;
				}
				break;
			default:
				*p++ = c;
			}
		} while (p < ap->arg + ap->argsize - 1);
		ap->arg[ap->argsize - 1] = '\0';
		printf("was parsing macro argument '%s'\n", ap->arg);
		error("macro argument too long");
		return 0; // not reached
	}
}


/*
 *  add a suffix to a string
 */
void suffix(char *str, char *suff)
{
	strcpy(getsuffix(str), suff);
}

char *basename(char *filename)
{
	char *base, *p;

	base = filename;
	for (p = filename; *p; p++) {
		if (*p == '/' || *p == '\\') {
			base = p + 1;
		}
	}

	return base;
}

char *getsuffix(char *str)
{
	char *suffix = 0;
	str = basename(str);
	for (; *str; str++) {
		if (*str == '.')
			suffix = str;
	}
	return suffix ? suffix : str;
}

// Construct output file given input path.
// Essentially files for "file.z" are sent to "zout/file.suffix".
// And for "dir/file.z" they are "zout/file.suffix"
// Creates output directory as a side effect.

void outpath(char *out, char *src, char *suff)
{
	static int did_mkdir = 0;

	strcpy(out, output_dir);

	if (!did_mkdir) {
		char *dir = out;
		while (*dir) {
			char *p;
			int ch;
			for (p = dir; *p && *p != '/'
#ifdef WIN32
				 && *p != '\\'
#endif
				 ; p++) { };
			ch = *p;
			*p = '\0';
#ifdef WIN32
			_mkdir(out);
#else
			mkdir(out, 0777);
#endif
			*p = ch;
			dir = p;
			if (ch)
				dir++;
		}
		did_mkdir = 1;
	}

	if (!suff)
		return;

	if (*out)
		strcat(out, "/");
	strcat(out, basename(src));
	suffix(out, suff);
}


/*
 *  put out a byte to the macro file, keeping the offset
 */
void putm(int c)
{
	mfseek(mfile, mfptr, 0);
	mfptr++;
	mfputc(c, mfile);
}



/*
 *  get a byte from the macro file
 */
int getm()
{
	int ch;

	mfseek(mfile, floc, 0);
	floc++;
	ch = mfgetc(mfile);
	if (ch == EOF) {
		ch = 0;
		fprintf(stderr, "bad macro read\n");
	}
	return ch;
}



/*
 *  pop standard input
 */
void popsi()
{
	int  i;

	for (i=0; i<PARMMAX; i++) {
		if (est[i].param) free(est[i].param);
	}
	floc = est[FLOC].value;
	ifptr = est[MIF].param;
	free(est);
	expptr--;
	est = expptr ? expstack[expptr-1] : 0;
	mfseek(mfile, (long)floc, 0);
	if (lineptr > linebuf) lineptr--;

	listfrombookmark();
}



/*
 *  return a unique name for a local symbol
 *  c is the parameter number, n is the macro number.
 */

char *getlocal(int c, int n)
{
	static char local_label[10];

	invented++;
	if (c >= 26)
		c += 'a' - '0';
	sprintf(local_label, "?%c%04d", c+'a', n) ;
	return(local_label);
}

char *getmraslocal()
{
	static char mras_local[32];
	char *p = mras_local + sizeof mras_local - 1;
	int n = est[TEMPNUM].value;

	*p = '\0';
	for (; n > 0; n /= 26)
		*--p = 'A' + n % 26;


	return p;
}


/*
 *  read in a symbol table
 */
void insymtab(char *name)
{
	struct stab *t;
	int  s, i;
	FILE *sfile;

	t = (struct stab *) tempbuf;
	if (!(sfile = fopen(name, "rb")))
		return;
	fread((char *)t, 1, sizeof *t, sfile);
	if (t->t_value != SYMMAJIC)
		return;

	
	s = t->t_token;
	for (i=0; i<s; i++) {
		fread((char *)t, 1, sizeof *t, sfile);
		if (tokenofitem(UNDECLARED, 0, 0) != UNDECLARED)
			continue;
		yylval.itemptr->i_token = t->t_token;
		yylval.itemptr->i_value = t->t_value;
		if (t->t_token == MACRO)
			yylval.itemptr->i_value += mfptr;
	}

	while ((s = fread(tempbuf, 1, TEMPBUFSIZE, sfile)) > 0) {
		mfptr += s;
		mfwrite(tempbuf, 1, s, mfile) ;
	}
	fclose(sfile);
}



/*
 *  write out symbol table
 */
void outsymtab(char *name)
{
	struct stab *t;
	struct item *ip;
	int  i;
	FILE *sfile;

	t = (struct stab *) tempbuf;
	if (!(sfile = fopen(name, "wb")))
		return;
	for (ip=itemtab; ip<itemmax; ip++) {
		if (ip->i_token == UNDECLARED) {
			ip->i_token = 0;
			nitems--;
		}
	}

	copyname(title, (char *)t);
	t->t_value = SYMMAJIC;
	t->t_token = nitems;
	fwrite((char *)t, 1, sizeof *t, sfile);

	for (ip=itemtab; ip<itemmax; ip++) {
		if (ip->i_token != 0) {
			t->t_token = ip->i_token;
			t->t_value = ip->i_value;
			copyname(ip->i_string, (char *)t);
			fwrite((char *)t, 1, sizeof *t, sfile);
		}
	}

	mfseek(mfile, (long)0, 0);
	while((i = mfread(tempbuf, 1, TEMPBUFSIZE, mfile) ) > 0)
		fwrite(tempbuf, 1, i, sfile);

	fclose(sfile);
}



/*
 *  copy a name into the symbol file
 */
void copyname(char *st1, char *st2)
{
	char  *s1, *s2;
	int  i;

	i = (MAXSYMBOLSIZE+2) & ~01;
	s1 = st1;
	s2 = st2;

	while((*s2++ = *s1++)) i--;		/* -Wall-ishness :-) -RJM */
	while(--i > 0) *s2++ = '\0';
}

/* get the next source file */
void next_source(char *sp, int always)
{
	char *path;

	if (!always && imported(sp))
		return;

	if(now_in == NEST_IN -1)
		error("Too many nested includes") ;
	if ((now_file = open_incpath(sp, "r", &path)) == NULL) {
		char ebuf[1024] ;
		sprintf(ebuf,"Can't open include file: %s", sp) ;
		error(ebuf) ;
	}
	if (outpass && iflist()) {
		lineout() ;
		fprintf(fout, "**** %s ****\n", path) ;
	}

	if (outpass && fbds)
		fprintf(fbds, "%04x %04x f %s\n", dollarsign, emit_addr, sp);

	/* save the list control flag with the current line number */
	if (lstoff)
		linein[now_in] = - linein[now_in] ;

	/* no list if include files are turned off */
	lstoff |= iopt ;

	linepeek[now_in] = nextline_peek;
	nextline_peek = NOPEEK;
	/* save the new file descriptor. */
	fin[++now_in] = now_file ;
	/* start with line 0 */
	linein[now_in] = 0 ;
	/* save away the file name */
	src_name[now_in] = path;
}

int phaseaddr(int addr)
{
	if (!phaseflag)
		return addr;

	if (addr < phbegin || addr > dollarsign) {
		err[vflag]++;
		if (pass2)
			fprintf(stderr, "$%04x outside current phase area\n", addr);
		return 0;
	}

	return phdollar + (addr - phbegin);
}

// Include contents of named file as binary data.
void incbin(char *filename)
{
	FILE *fp = open_incpath(filename, "rb", NULL);
	int ch;
	int start = dollarsign;
	int last = start;
	int bds_count;
	int bds_dollar = dollarsign, bds_addr = emit_addr, bds_len;

	if (!fp) {
		char ebuf[1024];
		sprintf(ebuf, "Can't binary include file: %s", filename);
		error(ebuf);
		return;
	}

	addtoline('\0');
	if (outpass && fbds)
		fprintf(fbds, "%04x %04x s %s", dollarsign, emit_addr, linebuf);

	// Avoid emit() because it has a small buffer and it'll spam the listing.
	bds_count = 0;
	bds_len = 0;
	while ((ch = fgetc(fp)) != EOF) {
		if (outpass && fbds) {
			if (bds_count == 0)
				fprintf(fbds, "%04x %04x d ", dollarsign, emit_addr);
			fprintf(fbds, "%02x", ch);
			bds_len++;
			bds_count++;
			if (bds_count == 16) {
				fprintf(fbds, "\n");
				bds_count = 0;
			}
		}

		if (segment == SEG_CODE)
			setmem(emit_addr, ch, MEM_DATA);
		emit_addr++;
		emit_addr &= 0xffff;
		last = dollarsign;
		dollarsign++;
		dollarsign &= 0xffff;

		putbin(ch);
		putrel(ch);
		putout(ch);
	}
	if (outpass && fbds) {
		if (bds_count)
			fprintf(fbds, "\n");

		bds_perm(bds_dollar, bds_addr, bds_len);
	}

	fclose(fp);

	// Do our own list() work as we emit bytes manually.

	if (outpass && iflist()) {
		lineout();

		if (nopt)
			fprintf(fout, "%4d:", linein[now_in]);

		if (copt)
		        fprintf(fout, nopt ? "%5s-" : "%4s-", "");

		if (nopt || copt)
			fprintf(fout, "\t");

		list_optarg(start, -1, 0);
		fprintf(fout, "..");
		list_optarg(last, -1, 0);

		putc('\t', fout);

		fputs(linebuf, fout);

	}
	lineptr = linebuf;
}

void dc(int count, int value)
{
	int start = dollarsign;
	int bds_count;
	int bds_addr = emit_addr, bds_len = count;

	addtoline('\0');
	if (outpass && fbds)
		fprintf(fbds, "%04x %04x s %s", dollarsign, emit_addr, linebuf);

	// Avoid emit() because it has a small buffer and it'll spam the listing.
	bds_count = 0;
	while (count-- > 0) {
		if (outpass && fbds) {
			if (bds_count == 0)
				fprintf(fbds, "%04x %04x d ", dollarsign, emit_addr);
			fprintf(fbds, "%02x", value);
			bds_count++;
			if (bds_count == 16) {
				fprintf(fbds, "\n");
				bds_count = 0;
			}
		}

		if (segment == SEG_CODE)
			setmem(emit_addr, value, MEM_DATA);

		emit_addr++;
		emit_addr &= 0xffff;
		dollarsign++;
		dollarsign &= 0xffff;

		putbin(value);
		putrel(value);
		putout(value);
	}

	if (outpass && fbds) {
		if (bds_count)
			fprintf(fbds, "\n");

		bds_perm(start, bds_addr, bds_len);
	}

	// Do our own list() work as we emit bytes manually.

	if (outpass && iflist()) {
		lineout();

		if (nopt)
			fprintf(fout, "%4d:", linein[now_in]);

		if (copt)
		        fprintf(fout, nopt ? "%5s-" : "%4s-", "");

		if (nopt || copt)
			fprintf(fout, "\t");

		list_optarg(start, -1, 0);
		fprintf(fout, "..");
		list_optarg(dollarsign - 1, -1, 0);
		puthex(value, fout);
		putc('\t', fout);
		fputs(linebuf, fout);
		lsterr2(1);

	}
	else
		lsterr1();

	lineptr = linebuf;
}

#define OUTREC_SEG(rec)		outbuf[rec]
#define OUTREC_ADDR(rec)	((outbuf[rec + 1] << 8) | outbuf[rec + 2])
#define OUTREC_LEN(rec)		outbuf[rec + 3]
#define OUTREC_DATA(rec, len)	outbuf[rec + 4 + len]
#define OUTREC_SIZEOF	5

void new_outrec(void)
{
	OUTREC_LEN(outrec) = outlen;
	outrec += OUTREC_SIZEOF + outlen;

	outlen = 0;
	OUTREC_SEG(outrec) = segment;
	outbuf[outrec + 1] = seg_pos[segment] >> 8;
	outbuf[outrec + 2] = seg_pos[segment];
}

void putout(int value)
{
	int addr = (OUTREC_ADDR(outrec) + outlen) & 0xffff;
	if (OUTREC_SEG(outrec) != segment || addr != seg_pos[segment])
		new_outrec();

	if (pass2 && OUTREC_DATA(outrec, outlen) != value && !passfail) {
		char segname[2];
		if (relopt)
			sprintf(segname, "%c", SEGCHAR(segment));
		else
			segname[0] = '\0';

		sprintf(detail, "%s error - $%04x%s changed from $%02x to $%02x",
			errname[pflag], addr, segname, OUTREC_DATA(outrec, outlen), value);
		errwarn(pflag, detail);

		if (!outpass)
			passretry = 1;
	}
	OUTREC_DATA(outrec, outlen) = value;
	outlen++;

	if (outlen >= 255)
		new_outrec();

	advance_segment(1);
}

struct bookmark {
	int len, rec, tstates, linenum, listed;
	char *line;
}
bookstack[MAXEXP];

int mark;

void bookmark(int delay)
{
	struct bookmark *book;

	if (!outpass)
		return;

	if (mark >= MAXEXP) {
		error("Macro expansion level too deep");
		return;
	}

	book = bookstack + mark++;

	book->len = outlen;
	book->rec = outrec;
	book->tstates = tstates;
	book->linenum = linein[now_in];
	book->line = strdup(linebuf);
	book->listed = !delay;
}

int book_row, book_col, book_addr, book_seg;

void booklist(int seg, int addr, int data, struct bookmark *book)
{
	// Don't list beyond the first 4 bytes if told not to.
	if (!gopt && book_row > 0)
		return;

	if (book_addr == -1) {
		list_optarg(addr, seg, ' ');
		book_seg = seg;
		book_addr = addr;
		book_col = 0;
	}

	if (seg != book_seg || addr != book_addr || book_col < 0 || book_col == 4) {
		if (book_row == 0 && book->line) {
			fprintf(fout, "\t%s", book->line);
			free(book->line);
			book->line = 0;
		}
		else
			fputs("\n", fout);

		lineout();

		book_row++;
		book_col = 0;

		if (!gopt && !book->line)
			return;

		if (nopt) putc('\t', fout);
		if (copt) fputs("        ", fout);

		if (seg != book_seg || addr != book_addr)
			list_optarg(addr, seg, ' ');
		else
			fputs("      ", fout);

		book_seg = seg;
		book_addr = addr;
	}

	if (!gopt && book_row > 0)
		return;

	puthex(data, fout);
	book_addr++;
	book_col++;
}

void listfrombookmark()
{
	int t, n;
	struct bookmark *book;

	if (!outpass)
		return;

	if (mark < 1) {
		//error("Internal delayed listing underflow.");
		fprintf(stderr, "Internal delayed listing underflow at %d.", mark);
		return;
	}

	book = bookstack + --mark;

	// No delayed listing required?  bookstack clean is all we need.
	if (book->listed) {
		if (book->line) {
			free(book->line);
			book->line = 0;
		}
		return;
	}

	book->listed = 1;

	t = tstates - book->tstates;
	lineout(); // call before every output line

	if (nopt)
		fprintf(fout, "%4d:", book->linenum);

	if (copt) {
		if (t) {
			fprintf(fout, nopt ? "%5d+%d" : "%4d+%d", book->tstates, t);
		}
		else {
			fprintf(fout, nopt ? "%5s-" : "%4s-", "");
		}
	}

	if (nopt || copt)
		fprintf(fout, "\t");

	book_row = 0;
	book_col = -1;
	book_addr = -1;
	n = 0;
	while (book->rec <= outrec) {
		int len = book->rec == outrec ? outlen : OUTREC_LEN(book->rec);
		int addr = OUTREC_ADDR(book->rec) + book->len;
		int seg = OUTREC_SEG(book->rec);

		for (; book->len < len; book->len++) {
			booklist(seg, addr++, OUTREC_DATA(book->rec, book->len), book);
			n++;
		}

		book->len = 0;
		book->rec += OUTREC_SIZEOF + len;
	}

	if (book->line) {
		// pad with spaces up to 4 total hex bytes
		for (; n < 4; n++)
			fprintf(fout, "  ");

		fprintf(fout, "\t%s", book->line);
		free(book->line);
		book->line = 0;
	}
	else if (gopt)
		fputs("\n", fout);
}

void advance_segment(int step)
{
	int top = seg_pos[segment] += step;
	seg_pos[segment] &= 0xffff;
	if (top >= 0x10000)
		top = 0xffff;

	if (top > seg_size[segment]) {
		seg_size[segment] = top;
		if (segment == SEG_COMMON && cur_common)
			cur_common->i_value = top;
	}
}

void expr_reloc_check(struct expr *ex)
{
	if (!relopt) return;
	if (ex->e_scope & (SCOPE_EXTERNAL | SCOPE_NORELOC))
		err[rflag]++;
}

void expr_number_check(struct expr *ex)
{
	if (!relopt) return;
	expr_reloc_check(ex);
	if (ex->e_scope & SCOPE_SEGMASK)
		err[rflag]++;
}

void expr_scope_same(struct expr *ex1, struct expr *ex2)
{
	if (!relopt) return;
	if ((ex1->e_scope & SCOPE_SEGMASK) != (ex2->e_scope & SCOPE_SEGMASK))
		err[rflag]++;
}

void expr_word_check(struct expr *ex)
{
	if (ex->e_value < -32768 || ex->e_value > 65535) {
		err[vflag]++;
	}
}

int is_number(struct expr *ex)
{
	return ex && (ex->e_scope & ~SCOPE_PUBLIC) == 0;
}

int is_external(struct expr *ex)
{
	return ex && (ex->e_scope & SCOPE_EXTERNAL) && !ex->e_left && !ex->e_right &&
		ex->e_item;
}

struct expr *expr_alloc(void)
{
	struct expr *ex = malloc(sizeof *ex);

	ex->e_value = 0;
	ex->e_scope = 0;
	ex->e_token = 0;
	ex->e_item = 0;
	ex->e_left = 0;
	ex->e_right = 0;
	ex->e_parenthesized = 0;

	return ex;
}

struct expr *expr_var(struct item *it)
{
	struct expr *ex = expr_alloc();

	ex->e_token = 'v';
	ex->e_item = it;
	ex->e_scope = it->i_scope;
	ex->e_value = it->i_value;

	return ex;
}

struct expr *expr_num(int value)
{
	struct expr *ex = expr_alloc();
	ex->e_value = value;
	ex->e_token = '0';

	return ex;
}

// Build expression and update value based on the operator.
// Could be done inline in the grammar but there is a fair bit of
// repetition and MRAS operators have only made that worse.

struct expr *expr_mk(struct expr *left, int op, struct expr *right)
{
	struct expr *ex;
	int val = 0;
	int sc = 0;

	switch (op) {
	case '+':
		ex = expr_op(left, '+', right, left->e_value + right->e_value);

		// Can't operate on external labels.
		// But we can add constants to any scope.
		if (!((left->e_scope | right->e_scope) & SCOPE_EXTERNAL) &&
			((left->e_scope & SCOPE_SEGMASK) == 0 ||
			(right->e_scope & SCOPE_SEGMASK) == 0))
		{
			ex->e_scope &= ~(SCOPE_NORELOC | SCOPE_SEGMASK);
			ex->e_scope |= (left->e_scope | right->e_scope) & SCOPE_SEGMASK;
		}
		return ex;
	case '-':
		ex = expr_op_sc(left, '-', right, left->e_value - right->e_value);

		// But we can subtract a constant.
		if (!((left->e_scope | right->e_scope) & SCOPE_EXTERNAL) &&
			((right->e_scope & SCOPE_SEGMASK) == 0))
		{
			ex->e_scope &= ~(SCOPE_NORELOC | SCOPE_SEGMASK);
			ex->e_scope |= (left->e_scope & SCOPE_SEGMASK);
		}
		return ex;
	case '/':
		if (!(right->e_scope & SCOPE_EXTERNAL)) {
			if (right->e_value == 0)
				err[eflag]++;
			else
				val = left->e_value / right->e_value;
		}
		break;
	case '*':
		val = left->e_value * right->e_value;
		break;
	case '%':
		if (!(right->e_scope & SCOPE_EXTERNAL)) {
			if (right->e_value == 0)
				err[eflag]++;
			else
				val = left->e_value % right->e_value;
		}
		break;
	case '&':
		val = left->e_value & right->e_value;
		break;
	case '|':
		val = left->e_value | right->e_value;
		break;
	case '^':
		val = left->e_value ^ right->e_value;
		break;
	case SHL:
		val = left->e_value << right->e_value;
		break;
	case SHR:
		val = right->e_value == 0 ? left->e_value : ((left->e_value >> 1) & ((0x7fff << 16) | 0xffff)) >> (right->e_value - 1);
		break;
	case '<':
		val = (left->e_value < right->e_value) * trueval;
		sc = 1;
		break;
	case '=':
		val = (left->e_value == right->e_value) * trueval;
		sc = 1;
		break;
	case '>':
		val = (left->e_value > right->e_value) * trueval;
		sc = 1;
		break;
	case LE:
		val = (left->e_value <= right->e_value) * trueval;
		sc = 1;
		break;
	case NE:
		val = (left->e_value != right->e_value) * trueval;
		sc = 1;
		break;
	case GE:
		val = (left->e_value >= right->e_value) * trueval;
		sc = 1;
		break;
	case ANDAND:
		val = (left->e_value && right->e_value) * trueval;
		break;
	case OROR:
		val = (left->e_value || right->e_value) * trueval;
		break;
	default:
		fprintf(stderr, "internal expression evaluation error!\n");
		clean_outf();
		exit(-1);
		break;
	}

	if (sc)
		return expr_op_sc(left, op, right, val);

	return expr_op(left, op, right, val);
}


// Expression consruction for operators that subtract/compare.
// They produce a valid result if operating on numbers in the same segment.
struct expr *expr_op_sc(struct expr *left, int token, struct expr *right, int value)
{
	struct expr *ex = expr_op(left, token, right, value);

	if (!(ex->e_scope & SCOPE_EXTERNAL) &&
		((left->e_scope ^ right->e_scope) & SCOPE_SEGMASK) == 0)
	{
		// Result relocatable and a simple number
		ex->e_scope &= ~(SCOPE_NORELOC | SCOPE_SEGMASK);
	}

	return ex;
}

struct expr *expr_op(struct expr *left, int token, struct expr *right, int value)
{
	struct expr *ex = expr_alloc();

	ex->e_value = value;
	ex->e_token = token;
	ex->e_left = left;
	ex->e_right = right;

	// Combining two numbers will be fine as long as they're not
	// flagged as external or already not relocatable.  In which case
	// it is up to the particular operator to allow the value
	// to become valid.

	ex->e_scope = left->e_scope;
	if (left->e_scope & SCOPE_SEGMASK)
		ex->e_scope |= SCOPE_NORELOC;
	if (right) {
		ex->e_scope |= right->e_scope;
		if (right->e_scope & SCOPE_SEGMASK)
			ex->e_scope |= SCOPE_NORELOC;
	}

	return ex;
}

void expr_free(struct expr *ex)
{
	if (!ex)
		return;

	expr_free(ex->e_left);
	expr_free(ex->e_right);
	free(ex);
}

int synth_op(struct expr *ex, int gen)
{
	if (!is_number(ex->e_right))
		return 0;

	switch (ex->e_token) {
	case '&':
		if (ex->e_right->e_value == 255) {
			if (gen) {
				extend_link(ex->e_left);
				putrelop(RELOP_LOW);
				return 1;
			}
			return can_extend_link(ex->e_left);
		}
		break;
	case SHR:
		if (ex->e_right->e_value <= 15) {
			if (gen) {
				extend_link(ex->e_left);
				extend_link(expr_num(1 << ex->e_right->e_value));
				putrelop(RELOP_DIV);
			}
			return can_extend_link(ex->e_left);
		}
		break;
	case SHL:
		if (ex->e_right->e_value <= 15) {
			if (gen) {
				extend_link(ex->e_left);
				extend_link(expr_num(1 << ex->e_right->e_value));
				putrelop(RELOP_MUL);
			}
			return can_extend_link(ex->e_left);
		}
		break;
	default:
		break;
	}

	return 0;
}

int link_op(struct expr *ex)
{
	if (!ex)
		return 0;

	switch (ex->e_token) {
	case HIGH: return RELOP_HIGH;
	case LOW: return RELOP_LOW;
	case '~': return RELOP_NOT;
	case '-': return !ex->e_right ? RELOP_NEG : RELOP_SUB;
	case '+': return RELOP_ADD;
	case '*': return RELOP_MUL;
	case '/': return RELOP_DIV;
	case '%': return RELOP_MOD;
	default: return 0;
	}
}

int can_extend_link(struct expr *ex)
{
	if (!ex)
		return 1;

	// If we have a value available then we're good.
	if (!(ex->e_scope & SCOPE_NORELOC)) {
		//printf("HEY!\n");
		//if (ex->e_item && ex->e_item->i_string)
		//	printf("ext link says OK for '%s'\n", ex->e_item->i_string);
		return 1;
	}

	// Might be able to synthesize the operation.
	if (synth_op(ex, 0))
		return 1;

	// Otherwise, the operator must be supported and the children
	// must be linkable.

	return link_op(ex) && can_extend_link(ex->e_left) && can_extend_link(ex->e_right);
}

void extend_link(struct expr *ex)
{
	int op;

	if (!ex)
		return;

	if (synth_op(ex, 1))
		return;

	extend_link(ex->e_left);
	extend_link(ex->e_right);

	op = link_op(ex);
	if (op) {
		putrelop(op);
		return;
	}

	putrelcmd(RELCMD_EXTLINK);

	if (is_external(ex)) {
		char *str = ex->e_item->i_string;
		int len = strlen(str);

		if (len > 6)
			len = 6;

		putrelbits(3, 1 + len);
		putrelbits(8, 'B');
		while (len-- > 0) {
			int ch = *str++;
			if (ch >= 'a' && ch <= 'z')
				ch -= 'a' - 'A';
			putrelbits(8, ch);
		}
	}
	else {
		putrelbits(3, 4);
		putrelbits(8, 'C');
		putrelbits(8, ex->e_scope & SCOPE_SEGMASK);
		putrelbits(8, ex->e_value);
		putrelbits(8, ex->e_value >> 8);
	}
}

void putrelop(int op)
{
	putrelcmd(RELCMD_EXTLINK);

	putrelbits(3, 2);
	putrelbits(8, 'A');
	putrelbits(8, op);
}

void write_tap_block(int type, int len, unsigned char *data)
{
	int i, parity;

	fputc((len + 2) & 0xff, ftap);
	fputc((len + 2) >> 8, ftap);

	fputc(type, ftap);
	parity = type;
	for (i = 0; i < len; i++) {
		fputc(data[i], ftap);
		parity ^= data[i];
	}
	fputc(parity, ftap);
}

// One supposes .tap files could load multiple blocks into memory.
// However, doesn't seem to be a lot of point and we'd have to write
// extra loader code to make it happen.  For now we just load the
// assembled data as one contiguous block with the holes getting
// filled with zeros.

void write_tap(int len, int org, unsigned char *data)
{
	unsigned char block[32], *p, orglo, orghi;
	unsigned char basic_loader[] = {
		239, 34, 34, 175, 58, 249, 192, 176, // LOAD ""CODE:RANDOMIZE USR VAL
		'"', '2', '3', '2', '9', '6', '"', 13 // aka 0x5b00 - start of RAM
	};
	int entry = org;

	if (xeq_flag)
		entry = xeq;

	// .tap file output borrowed heavily from skoolkit's bin2tap.py
	// It loads a short basic program which auto-executes and loads
	// a short machine-language loader that reads the block of code
	// and jumps to it.
	// Constrast this with pasmo which doesn't have the short machine
	// code loader but uses a native code block.  I think that means
	// it can only execute at the beginning of the loaded data.

	p = block;

	*p++ = 0; // Program block
	casname((char *)p, sourcef, 10);
	p += 10;
	*p++ = 4 + sizeof basic_loader; *p++ = 0; // length of BASIC program
	*p++ = 10; *p++ = 0; // run line 10 after loading
	*p++ = 4 + sizeof basic_loader; *p++ = 0; // length of BASIC program

	write_tap_block(0, p - block, block);

	p = block;
	*p++ = 0; *p++ = 10; // line 10
	*p++ = sizeof(basic_loader); *p++ = 0;
	memcpy(p, basic_loader, sizeof basic_loader);
	p += sizeof basic_loader;
	write_tap_block(0xff, p - block, block);

	p = block;

	*p++ = 3; // Code block
	casname((char *)p, sourcef, 10);
	p += 10;
	*p++ = 19; *p++ = 0; // length of loader program
	*p++ = 0; *p++ = 0x5b; // 0x5b00 == 23296 - start of RAM
	*p++ = 0; *p++ = 0; // ?

	write_tap_block(0, p - block, block);

	p = block;

	orglo = org & 0xff;
	orghi = org >> 8;
	/* LD IX,org   */ *p++ = 0xdd; *p++ = 0x21; *p++ = orglo; *p++ = orghi;
	/* LD DE,len   */ *p++ = 0x11; *p++ = len & 0xff; *p++ = len >> 8;
	/* SCF         */ *p++ = 0x37;
	/* SBC A,A     */ *p++ = 0x9f;
	/* LD SP,org   */ *p++ = 0x31; *p++ = orglo; *p++ = orghi;
	/* LD BC,entry */ *p++ = 0x01; *p++ = entry & 0xff; *p++ = entry >> 8;
	/* PUSH BC     */ *p++ = 0xc5;
	/* JP $556     */ *p++ = 0xc3; *p++ = 0x56; *p++ = 0x05;

	write_tap_block(0xff, p - block, block);

	write_tap_block(0xff, len, data);
}

#define WORD(w) (w) & 255, (w) >> 8

void write_250(int low, int high)
{
	int load = low;
	int len = high - low + 1;
	int last;
	int chk;

	if (len <= 0) {
		// Nothing to output.  So we'll just delete the output file.
		int i;
		for (i = 0; i < CNT_OUTF; i++) {
			if (*outf[i].fpp && (*outf[i].fpp == ftcas || *outf[i].fpp == f250wav)) {
				fclose(*outf[i].fpp);
				*outf[i].fpp = NULL;
				unlink(outf[i].filename);
				if (outf[i].wanted)
					fprintf(stderr, "Warning: %s not output -- no code or data\n", outf[i].filename);
			}
		}
		return;
	}

	if (xeq_flag) {
		// Only add relocation if they don't already put their
		// execution address in to $41FE.  This means programs will
		// be unchanged if they seem to be aware of the structure.

		if (low > 0x41FE || high < 0x41FF ||
			memory[0x41FE] != (xeq & 0xff) ||
			memory[0x41FF] != xeq >> 8)
		{
			if (low >= 0x4200 && low <= 0x4200 + 14) {
				// A little too high.  More efficient to
				// just load a bit extra.  Plus we can't
				// easily fit in the "copy up" code.
				low = 0x41FE;
				memory[0x41FE] = xeq;
				memory[0x41FF] = xeq >> 8;
				load = low;
			}
			else if (low < 0x4200) {
				// Moving down.
				int src = 0x4200;
				int dst = low;
				unsigned char relo[] = {
					0x21, WORD(src),	// LD HL,nn
					0x11, WORD(dst),	// LD DE,nn
					0x01, WORD(len),	// LD BC,len
					0xED, 0xB0,		// LDIR
					0xC3, WORD(xeq)		// JP nn
				};
				high++;
				low -= 2;
				memory[low] = src + len;
				memory[low + 1] = (src + len) >> 8;
				memcpy(memory + high, relo, sizeof relo);
				high += sizeof relo - 1;
				load = 0x41FE;
			}
			else {
				// Moving up
				int src = 0x41FE + 2 + 14 + len - 1;
				int dst = low + len - 1;
				unsigned char relo[] = {
					WORD(0x4200),
					0x21, WORD(src),	// LD HL,nn
					0x11, WORD(dst),	// LD DE,nn
					0x01, WORD(len),	// LD BC,len
					0xED, 0xB8,		// LDDR
					0xC3, WORD(xeq)		// JP nn
				};
				low -= sizeof relo;
				memcpy(memory + low, relo, sizeof relo);
				load = 0x41FE;
			}
		}
	}

	len = high + 1 - low;
	last = load + len;
	// Yeah, it is big endian.
	fprintf(ftcas, "%c%c%c%c", load >> 8, load, last >> 8, last);
	fwrite(memory + low, len, 1, ftcas);
	chk = 0;
	for (i = 0; i < len; i++)
		chk += memory[low + i];
	fprintf(ftcas, "%c", -chk);
}

int bitgetbuf;
int bitgetcnt;

void bitget_rewind(FILE *fp)
{
	bitgetcnt = 0;
	fseek(fp, 0, SEEK_SET);
}

int bitget(FILE *fp)
{
	int bit;

	if (bitgetcnt == 0) {
		bitgetbuf = fgetc(fp);
		bitgetcnt = 8;
	}

	bit = !!(bitgetbuf & 0x80);
	bitgetbuf <<= 1;
	bitgetcnt--;

	return bit;
}

void writewavs(int pad250, int pad500, int pad1500)
{
	FILE *cas[] = { ftcas, flcas, flnwcas, fcas };
	FILE *wav[] = { f250wav, f500wav, f1000wav, f1500wav };
	int padbytes[] = { pad250, pad500, pad500, pad1500 };
#define	NFMT (sizeof padbytes / sizeof padbytes[0])
	int bits[NFMT];
	int i, j, k, m;
	unsigned char pulse[][2][13] = {
		{ { 2, 0xff, 2, 0, 42 - 4, 0x80, 0 },
		  { 2, 0xff, 2, 0, 17, 0x80, 2, 0xff, 2, 0, 17, 0x80, 0 } },

		{ { 3, 0xff, 3, 0, 44 - 6, 0x80, 0 },
		  { 3, 0xff, 3, 0, 16, 0x80, 3, 0xff, 3, 0, 16, 0x80, 0 } },

		{ { 3, 0xff, 3, 0, 44 - 6, 0x80, 0 },
		  { 3, 0xff, 3, 0, 16, 0x80, 3, 0xff, 3, 0, 16, 0x80, 0 } },

		{ { 8, 0, 8, 0xff, 0 },
		  { 4, 0, 4, 0xff, 0 } }
	};
	int hz[] = { 11025, 22050, 44100, 22050 };
	int pulse_len[NFMT][2];

	for (i = 0; i < NFMT; i++) {
		for (j = 0; j < 2; j++) {
			pulse_len[i][j] = 0;
			for (k = 0; pulse[i][j][k]; k += 2)
				pulse_len[i][j] += pulse[i][j][k];
		}
	}

	for (i = 0; i < NFMT; i++) {
		if (!cas[i] || !wav[i])
			continue;

		bits[i] = (ftell(cas[i]) - padbytes[i]) * 8;
		if (i == 2 && casbitcnt > 0)
			bits[i] -= 8 - casbitcnt;
	}

	for (i = 0; i < NFMT; i++) {
		int headPad = 10, tailPad = hz[i] / 2;
		int audio_bytes = headPad;
		unsigned char hzlo = hz[i] & 0xff;
		unsigned char hzhi = hz[i] >> 8;

		unsigned char waveHeader[] = {
			'R', 'I', 'F', 'F',
			0, 0, 0, 0,
			'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
			16, 0, 0, 0, // wav information length
			1, 0, // PCM
			1, 0, // Single channel
			hzlo, hzhi, 0, 0, // samples/second
			hzlo, hzhi, 0, 0, // average bytes/second
			1, 0, // block alignment
			8, 0, // bits/sample
			'd', 'a', 't', 'a',
			0, 0, 0, 0
		};
		int waveHeaderSize = sizeof waveHeader;

		if (!cas[i] || !wav[i])
			continue;

		bitget_rewind(cas[i]);
		for (j = 0; j < bits[i]; j++)
			audio_bytes += pulse_len[i][bitget(cas[i])];

		audio_bytes += tailPad;

		waveHeader[waveHeaderSize - 4] = audio_bytes;
		waveHeader[waveHeaderSize - 3] = audio_bytes >> 8;
		waveHeader[waveHeaderSize - 2] = audio_bytes >> 16;
		waveHeader[waveHeaderSize - 1] = audio_bytes >> 24;

		waveHeader[4] = (audio_bytes + 36);
		waveHeader[5] = (audio_bytes + 36) >> 8;
		waveHeader[6] = (audio_bytes + 36) >> 16;
		waveHeader[7] = (audio_bytes + 36) >> 24;

		bitget_rewind(cas[i]);

		fwrite(waveHeader, waveHeaderSize, 1, wav[i]);

		for (j = 0; j < headPad; j++)
			fputc(0x80, wav[i]);

		bitget_rewind(cas[i]);
		for (j = 0; j < bits[i]; j++) {
			int bit = bitget(cas[i]);
			for (k = 0; pulse[i][bit][k]; k += 2)
				for (m = 0; m < pulse[i][bit][k]; m++)
					fputc(pulse[i][bit][k + 1], wav[i]);
		}

		for (j = 0; j < tailPad; j++)
			fputc(0x80, wav[i]);
	}
}

int sized_byteswap(int value)
{
	int swapped = 0;

	for (; value; value = (value >> 8) & 0xffffff) {
		swapped <<= 8;
		swapped |= value & 0xff;
	}

	return swapped;
}

// Tracking whether a file has been imported.

struct import_list {
	struct import_list *next;
	char *filename;
	int imported;
} *imports;

void reset_import()
{
	struct import_list *il;
	for (il = imports; il; il = il->next)
		il->imported = 0;
}

// Returns 1 if filename has been imported.  Marks it as imported.
// Only uses the base name.

int imported(char *filename)
{
	struct import_list *il;
	int ret;
	char *p;

	for (p = filename; *p; p++)
		if (*p == '/' || *p == '\\')
			filename = p + 1;

	for (il = imports; il; il = il->next) {
		if (strcmp(filename, il->filename) == 0)
			break;
	}

	if (!il) {
		il = malloc(sizeof *il);
		il->filename = strdup(filename);
		il->imported = 0;
		il->next = imports;
		imports = il;
	}

	ret = il->imported;
	il->imported = 1;
	return ret;
}
