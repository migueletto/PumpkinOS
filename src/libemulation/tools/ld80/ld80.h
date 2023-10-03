#define VERSION     "0.5-dg-1"

#define	NAMELEN		8
#define	MAX_SEGMENTS	128
#define MAX_COMMONS	(MAX_SEGMENTS-3)

/* exit codes */
#define E_SUCCESS	0	/* no error */
#define E_USAGE		1	/* command line error, incorrect usage */
#define E_INPUT		2	/* unprocessable input file */
#define E_RESOURCE	3	/* out of resources */

/* output file formats */
#define	F_IHEX		0	/* Intel HEX */
#define	F_BIN00		1	/* binary, gaps filled with 00 */
#define	F_BINFF		2	/* binary, gaps filled with FF */
#define F_CMD		3	/* TRS-80 CMD file format */

/* segment types */
#define	T_ABSOLUTE	0x00
#define	T_RELOCATABLE	0x10
#define	T_MASK		0x03
#define	T_SPECIAL	0
#define	T_CODE		1
#define	T_DATA		2
#define	T_COMMON	3

struct object_item {
	int type;	/* absolute, special, code, data, common */
	union {
		unsigned char absolute_byte;
		unsigned short relative_word;
		struct {
			int control;
#			define C_ENTRY_SYMBOL	0
#			define C_SELECT_COMMON	1
#			define C_PROGNAME	2
#			define C_LIBSEARCH	3
#			define C_EXTENSION	4
#			define C_COMMON_SIZE	5
#			define C_CHAIN_EXTERNAL	6
#			define C_ENTRY_POINT	7
#			define C_EXT_MINUS_OFF	8
#			define C_EXT_PLUS_OFF	9
#			define C_DATA_SIZE	10
#			define C_SET_LC		11
#			define C_CHAIN_ADDRESS	12
#			define C_PROG_SIZE	13
#			define C_END_PROGRAM	14
#			define C_END_FILE	15
			int A_t;
			unsigned short A_value;
			int B_len;
			unsigned char B_name[NAMELEN+1];
		} special;
	} v;
};

struct loc {
	struct section *section;
	int offset;
};

struct fixup {
	struct fixup *next;
	int lc;
	struct loc at;
};

struct section {
	struct section *next;
	int base;	/* -1 = undefinded */
	int lc;		/* current lc */
	unsigned char *buffer;
	int len;	/* final lc - start */
	char *filename;
	struct fixup *fixups;
	struct node *nodehead, *nodetail;
	char module_name[NAMELEN+1];
	struct segment *segment;
};

struct segment {
	int type;	/* absolute, code, data, common */
	struct section *secs;
	int default_base;
	int uncommon;
	int maxsize;
	char common_name[NAMELEN+1];
};

struct symbol {
	struct loc at;
	int value;
#define	UNDEFINED	(-1)
	char name[NAMELEN+1];
};

struct node {
	struct node *next;
	struct loc at;
	int nodenum;
	int type;
#	define	N_OPERAND	0
#	define	N_BYTE		1
#	define	N_WORD		2
#	define	N_HIGH		3
#	define	N_LOW		4
#	define	N_NOT		5
#	define	N_UNARYMINUS	6
#	define	N_MINUS		7
#	define	N_PLUS		8
#	define	N_MULT		9
#	define	N_DIV		10
#	define	N_MOD		11
#	define	N_EXTERNAL	N_OPERAND+128
#	define	N_EXTMINUS	N_MINUS+128
#	define	N_EXTPLUS	N_PLUS+128
	int value;
	struct symbol *symbol;
};

extern int warn_extchain, debug;
extern int fatalerror;
extern struct segment *segv;
extern unsigned char usage_map[];
#define MARK_BYTE(a)	usage_map[(a)/8] |= 1 << ((a)%8)
#define MARKED(a)	(usage_map[(a)/8] & 1 << ((a)%8))
#define MARKED32(a)	(((uint32_t*)usage_map)[(a)/32] == 0xffffffff)
#define UNMARKED32(a)	(((uint32_t*)usage_map)[(a)/32] == 0)

#ifdef WINHACK
#define __attribute__(x)
#endif

void die(int, const char*, ...) __attribute__ ((__noreturn__));
void *calloc_or_die(size_t, size_t);

int read_object_file(char *, int);

void set_base_address(int, char *, int, int);
void mark_uncommon(char *);
void add_item(struct object_item *, char *);
void relocate_sections(void);
void dump_sections(void);
void init_section(void);
void join_sections(int);
void print_map(FILE *);

struct symbol *find_symbol(char *);
int add_symbol(char *, int, struct section *);
struct symbol *get_symbol(char *);
int init_symbol(int);
void clear_symbol(void);
void dump_symbols(void);
void set_symbols(void);
void print_symbol_table(FILE *);

void add_fixup(struct section *, struct section *, int);
void set_fixups(void);
void resolve_externals(void);
void convert_chain_to_nodes(char *, int, struct section *);
void process_nodes(void);
struct node *add_node(struct section *, int, int);

int do_out(FILE *, int, int);

extern int optget_ind;
extern int optget(int argc, char **argv, char *options, char **arg);

#ifdef NEED_HSEARCH

typedef struct {
	char *key;
	void *data;
} ENTRY;

typedef enum { ENTER, FIND } ACTION;

extern int hcreate(size_t);
extern ENTRY *hsearch(ENTRY item, ACTION action);
extern void hdestroy(void);

#endif

#ifdef WINHACK

// These #defines remove some compiler complaints in VS2008 Express.
// It is possible they otherwise break compilation.

#define strdup _strdup
#define unlink _unlink

#endif
