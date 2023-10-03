#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ld80.h"

#define	S(special_type)	(1<<(special_type))
#define	RELOC		(1<<16)
#define	ABS		(1<<19)

#define	HEAD_ELEMENT	(S(0)|S(2))
#define	PRGNAME_ELEMENT	S(2)
#define	ENTRY_ELEMENT	S(0)
#define	BODY_ELEMENT	(S(1)|S(3)|S(4)|S(5)|S(8)|S(9)|S(10)|S(11)|S(13)|\
			RELOC|ABS)
#define	TAIL_ELEMENT	(S(6)|S(7)|S(12)|S(14))
#define	EOF_ELEMENT	(S(15))

#define	HAS_A		1
#define	HAS_B		2
#define	ALIGN		4
#define	BLANK		8
static int special_attrib[16] = {
	HAS_B, HAS_B|BLANK, HAS_B, HAS_B, HAS_B,	/* 0-4 */
	HAS_A|HAS_B|BLANK, HAS_A|HAS_B, HAS_A|HAS_B,	/* 5-7 */
	HAS_A, HAS_A, HAS_A, HAS_A, HAS_A, HAS_A,	/* 8-13 */
	HAS_A|ALIGN, 					/* 14 */
	0,						/* 15 */
};

#define byte_read() bit_read(8)
void dump_item(struct object_item *);

static FILE *objectfile;
static char *objfilename;
static int libsearch;
static int bitpos;

static
int get_bit(void)
{
	int retval;
	static int bytebuf;

	if (bitpos%8 == 0) {
		bytebuf = fgetc(objectfile);
		if (bytebuf == EOF) die(E_INPUT, "ld80: Unexpected EOF "
			"on input file %s\n", objfilename);
	}
	retval = (bytebuf & 0x80)!=0;
	bytebuf <<= 1;
	bitpos++;
	return retval;
}

static
int bit_read(int n)
{
	int retval = 0;

	while (n--) retval = (retval << 1) | get_bit();
	return retval;
}

static
int word_read(void)
{
	int low, high;
	low = byte_read();
	high = byte_read();
	return (high<<8) + low;
}

static
unsigned long read_item(struct object_item *item)
{
	int i;

	i = bit_read(1);
	if (i==0) {	/* absolute */
		item->type = T_ABSOLUTE;
		i = byte_read();
		item->v.absolute_byte = i;
		return ABS;
	}

	/* relocatable */
	i = bit_read(2);
	item->type = T_RELOCATABLE | i;
	if (i != 0) {	/* relative */
		i = word_read();
		item->v.relative_word = i;
		return RELOC;
	}

	/* special link item */
	i = bit_read(4);
	item->v.special.control = i;
	if (special_attrib[item->v.special.control] & HAS_A) {
		i = bit_read(2);
		item->v.special.A_t = i;
		i = word_read();
		item->v.special.A_value = i;
	}
	if (special_attrib[item->v.special.control] & HAS_B) {
		int len,j;

		i = bit_read(3);
		len = i ? i : 8;
		item->v.special.B_len = len;
		for (j=0; j<len; j++) {
			i = byte_read();
			item->v.special.B_name[j] = i;
		}
		if (item->v.special.B_name[0]==' ' &&
			special_attrib[item->v.special.control] & BLANK) j=0;
		item->v.special.B_name[j] = '\0';
	}
	else {
		item->v.special.B_name[0] = '\0';
		item->v.special.B_len = 0;
	}
	if (special_attrib[item->v.special.control] & ALIGN && bitpos%8) {
		bit_read(8 - bitpos%8);
	}
	return S(item->v.special.control);
}

static
int read_item_buffered(struct object_item *item, unsigned long accepted)
{
	static unsigned long entry_type;
	static struct object_item itembuf;
	if (!entry_type) entry_type = read_item(&itembuf);
	if (entry_type & accepted) {
		memcpy((void*)item, (void*)&itembuf, sizeof(*item));
		entry_type = 0;
		return 1;
	}
	else return 0;
}

int read_module(void)
{
	struct object_item progname_item;
	struct object_item item;
	struct symbol *s;
	int load_this = !libsearch;

	if (!read_item_buffered(&progname_item, PRGNAME_ELEMENT)) die(E_INPUT,
		"ld80: Module has no name in object file %s\n", objfilename);

	while (read_item_buffered(&item, ENTRY_ELEMENT)) {
		s = get_symbol((char *)item.v.special.B_name);
		if (s && s->value==UNDEFINED) {
			s->value = 0;
			load_this = 1;
		}
	}
	if (load_this) {
#ifdef DEBUG
		if (debug>1) dump_item(&progname_item);
#endif
		add_item(&progname_item, objfilename);
	}
	else if (debug) printf("Module %s:%s skipped\n",
			objfilename, progname_item.v.special.B_name);

	while (read_item_buffered(&item, BODY_ELEMENT|TAIL_ELEMENT)) {
		if (load_this) {
#ifdef DEBUG
			if (debug>1) dump_item(&item);
#endif
			add_item(&item, objfilename);
		}
	}

	return !read_item_buffered(&item, EOF_ELEMENT);
}

int read_object_file(char *filename, int lib)
{
	int modcnt = 0;

	bitpos = 0;

	objectfile=fopen(filename,"rb");
	if (objectfile==NULL) die(E_USAGE,
		"ld80: Cannot open object file %s: %s\n",
		filename, strerror(errno));

	objfilename = filename;
	libsearch = lib;
	while(read_module()) modcnt++;
	fclose(objectfile);
	return modcnt;
}

#ifdef	DEBUG

static char *stypes[] = {
	"special","code","data","common",
};
static char *atypes[] = {
	"absolute","code","data","common",
};
static char *operators[] = {
	"",
	"BYTE",
	"WORD",
	"HIGH",
	"LOW",
	"NOT",
	"unary -",
	"-",
	"+",
	"*",
	"/",
	"MOD",
};

void dump_item(struct object_item *item)
{
	printf("<< ");
	switch (item->type) {
	case T_ABSOLUTE:
		printf("absolute %.2x\n",item->v.absolute_byte);
		break;
	case T_RELOCATABLE|T_CODE:
	case T_RELOCATABLE|T_DATA:
	case T_RELOCATABLE|T_COMMON:
		printf("%s relative %.4x\n",
			stypes[item->type&T_MASK],
			item->v.relative_word);
		break;
	case T_RELOCATABLE|T_SPECIAL:
		switch (item->v.special.control) {
		case C_ENTRY_SYMBOL:
			printf("Entry symbol %s\n",item->v.special.B_name);
			break;
		case C_SELECT_COMMON:
			printf("Select common %s\n",item->v.special.B_name);
			break;
		case C_PROGNAME:
			printf("Program name %s\n",item->v.special.B_name);
			break;
		case C_LIBSEARCH:
			printf("Library search %s\n",item->v.special.B_name);
			break;
		case C_EXTENSION:
			printf("Extension: ");
			switch (item->v.special.B_name[0]) {
			int operator;
			case 'A':
				operator = item->v.special.B_name[1];;
				if (operator > 11) die(E_INPUT,
					"ld80: Unknown operator %.2x\n",
					operator);
				printf(" operator \"%s\"\n",
					operators[operator]);
				break;
			case 'B':
				printf(" operand %s (external)\n",
					item->v.special.B_name+1);
				break;
			case 'C':
				printf(" operand %.4x (%s)\n",
					(item->v.special.B_name[3] << 8) +
					item->v.special.B_name[2],
					atypes[item->v.special.B_name[1]]);
				break;
			default:
				die(E_INPUT, "ld80: Unknown extension %.2x\n",
					item->v.special.B_name[0]);
			}
			break;
		case C_COMMON_SIZE:
			printf("Common %s size %.4x (%s)\n",
				item->v.special.B_name,
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_CHAIN_EXTERNAL:
			printf("Chain external symbol %s to %.4x (%s)\n",
				item->v.special.B_name,
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_ENTRY_POINT:
			printf("Entry point %s is %.4x (%s)\n",
				item->v.special.B_name,
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_EXT_MINUS_OFF:
			printf("External minus offset %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_EXT_PLUS_OFF:
			printf("External plus offset %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_DATA_SIZE:
			printf("Data size is %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_SET_LC:
			printf("Set location counter to %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_CHAIN_ADDRESS:
			printf("Chain address %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_PROG_SIZE:
			printf("Code size is %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_END_PROGRAM:
			printf("Program end is %.4x (%s)\n",
				item->v.special.A_value,
				atypes[item->v.special.A_t]);
			break;
		case C_END_FILE:
			printf("End of file\n");
			break;
		default:
			die(E_INPUT, "ld80: Unknown control type %x\n",
				item->v.special.control);
		}
		break;
	default:
		die(E_INPUT, "ld80: Illegal item type %x\n",item->type);
	}
}
#endif
