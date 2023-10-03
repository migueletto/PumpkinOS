#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ld80.h"

//#define	overlapping_common(x)	(!uncommon(x))
#define	A_FIND			0
#define	A_ENTER			1
#define	BNAME			(item->v.special.B_name)
#define AVALUE			(item->v.special.A_value)
#define	ATYPE			(item->v.special.A_t)

struct segment *segv;
static struct section *secs[4];
static int current_section_t = T_CODE;
unsigned char *aseg;
unsigned char usage_map[0x10000/8];
static int overlap = 0;
static char module_name[NAMELEN+1];

static int uncommon(int, char *);
static int base_address(int, char *);
static struct segment *search_segment(int, char *, int);

static
void add_section(int type, char *common_name,
		int start, int len, char *filename)
{
	struct section **pp, *p = calloc_or_die(1, sizeof(*p));
	struct segment *sp;

#ifdef DEBUG
	if (debug)
	printf("add_section(type=%d, %s, start=%.4x, len=%.4x, %s)=%p\n",
		type, type == T_COMMON ? common_name : "<null>",
		start, len, filename, p);
#endif

	p->len = len;
	p->filename = filename;
	strncpy(p->module_name, module_name, NAMELEN);
	p->base = start;
	p->buffer = (type == T_ABSOLUTE) ? aseg : calloc_or_die(len, 1);

	/* find row */
	sp = search_segment(type, common_name, A_ENTER);
	/* find column */
	for (pp=&sp->secs; *pp; pp=&((*pp)->next)) /* EMPTY */;
	/* add new section */
	*pp = p;
	p->segment = sp;
	if (sp->maxsize < len)
		sp->maxsize = len;/* size of largest section */

	secs[current_section_t = type] = p;
	return;
}

static
int to_chain(struct object_item *item, char *filename)
{
	if (AVALUE != 0x0000) return 1;
	if (ATYPE != T_ABSOLUTE) return 1;
	if (!warn_extchain) return 0;
	fprintf(stderr,
"* \n"
"* Warning! The definition of 'L80 compatible' object format is ambigous.\n");
	if (MARKED(0) && MARKED(1)) {
		fprintf(stderr,
"* In module %s of file %s ld80 detected such a construction\n"
"* There is no way to figure out if value of external symbol %s\n"
"* should be placed at absolute location 0000H or not.\n"
"* If yes, try to use of 'DW %s/1' instead of simple 'DW %s'.\n"
"* \n",
			module_name, filename, BNAME, BNAME, BNAME);
	}
	else {
		fprintf(stderr,
"* ld80 guesses that module %s in file %s refers\n"
"* to external symbol %s, but actually does not use it.\n"
"* \n",
			module_name, filename, BNAME);
	}
	return 0;
}

void add_item(struct object_item *item, char *filename)
{
	struct section *p;
	struct segment *sp;
	int t = 1;
	struct node *n;
	unsigned char *b;
	int base;

	switch (item->type) {
	/********************** body *************************/
	case T_ABSOLUTE:
		if ((p=secs[current_section_t]) == NULL) die(E_INPUT,
			"ld80: No segment to fill\n");
		if (p->lc >= p->len) die(E_INPUT, "ld80: Segment overloaded\n");
		p->buffer[p->lc] = item->v.absolute_byte;
		if (current_section_t == T_ABSOLUTE) {
			if (MARKED(p->lc)) {
				if (!overlap++) fprintf(stderr,
						"ld80: Overlapping ASEG in "
						"file %s\n", filename);
				fatalerror = 1;
			}
			else MARK_BYTE(p->lc);
		}
		p->lc++;
		break;
	case T_RELOCATABLE|T_CODE:
	case T_RELOCATABLE|T_DATA:
	case T_RELOCATABLE|T_COMMON:
		p=secs[current_section_t];
		add_fixup(p, secs[item->type&T_MASK], item->v.relative_word);
		if (current_section_t == T_ABSOLUTE) {
			MARK_BYTE(p->lc);
			MARK_BYTE(p->lc+1);
		}
		p->lc+=2;
		break;
	case T_RELOCATABLE|T_SPECIAL:
		switch (item->v.special.control) {
		case C_COMMON_SIZE:	/* 5 */
#if 0
			if (overlapping_common(BNAME) &&
				adjust_section(BNAME, AVALUE))
					break;	/* resized */
#endif
			/* new segment. we create it */
			t++;
			/* FALL THROUGH */
		case C_DATA_SIZE:	/* 10 */
			t++;
			/* FALL THROUGH */
		case C_PROG_SIZE:	/* 13 */
			add_section(t, (char *)BNAME, base_address(t,(char *)BNAME),
				AVALUE, filename);
			if (uncommon(t,(char *)BNAME)) {
				base = base_address(current_section_t, (char *)BNAME);
				if (base >= 0)
					set_base_address(current_section_t,
						(char *)BNAME, base + AVALUE, 0);
			}
			break;
		case C_SET_LC:		/* 11 */
			current_section_t = ATYPE;
			if ((p=secs[current_section_t]) == NULL)
				die(E_INPUT, "ld80: No segment to org\n");
			p->lc = AVALUE;
			overlap=0;
			break;
		case C_SELECT_COMMON:	/* 1 */
			sp = search_segment(T_COMMON, (char *)BNAME, A_FIND);
			if (sp==NULL || sp->secs==NULL) die(E_INPUT,
				"ld80: No common segment %s\n", BNAME);
			for (p=sp->secs; p->next; p=p->next) /* EMPTY */;
			secs[T_COMMON] = p;
			break;
		case C_EXTENSION:	/* 4 */
			b = BNAME;
			switch (b[0]) {
			case 'A':	/* operator */
				add_node(secs[current_section_t],
					secs[current_section_t]->lc,
					b[1]);
				break;
			case 'B':	/* external operand */
				n = add_node(secs[current_section_t],
					secs[current_section_t]->lc,
					N_EXTERNAL);
				n->symbol =
					find_symbol((char *)BNAME+1);
				break;
			case 'C':	/* base+offset operand */
				n = add_node(secs[current_section_t],
					secs[current_section_t]->lc,
					N_OPERAND);
				n->at.section = secs[b[1]];
				n->value = b[3]*256 + b[2];
				break;
			default:
				die(E_INPUT, "ld80: Unimplemented extension "
					"type 0x%.2x\n", b[0]);
			}
			break;
		case C_EXT_PLUS_OFF:	/* 9 */
		case C_EXT_MINUS_OFF:	/* 8 */
				n = add_node(secs[current_section_t],
					secs[current_section_t]->lc,
					item->v.special.control ==
					C_EXT_PLUS_OFF ?
					N_EXTPLUS : N_EXTMINUS);
				n->at.section = secs[ATYPE];
				n->value = AVALUE;
				break;
		case C_LIBSEARCH:	/* 3 */
			fprintf(stderr,
				"ld80: Library search is unimplemented. "
				"Use -l option.\n");
			break;
		/********************** tail *************************/
		case C_CHAIN_EXTERNAL:	/* 6 */
			if (to_chain(item, filename))
				convert_chain_to_nodes((char *)BNAME, AVALUE,
					secs[ATYPE]);
			break;
		case C_ENTRY_POINT:	/* 7 */
			add_symbol((char *)BNAME, AVALUE, secs[ATYPE]);
			break;
		case C_CHAIN_ADDRESS:	/* 12 */
			die(E_INPUT, "ld80: Address chain is unimplemented."
				" Contact the author.\n");
		case C_END_PROGRAM:	/* 14 */
		case C_END_FILE:	/* 15 */
			break;
		/********************** head *************************/
		case C_ENTRY_SYMBOL:	/* 0 */
					/* NEVER CALLED */
			break;
		case C_PROGNAME:	/* 2 */
			strncpy(module_name,(char *)BNAME,NAMELEN);
			add_section(T_ABSOLUTE,NULL,0x0000,0x10000,filename);
			break;
		}
	}
}

void init_section(void)
{
	int i;

	aseg = calloc_or_die(1, 0x10000);
	segv = calloc_or_die(MAX_SEGMENTS, sizeof(*segv));
	segv[T_ABSOLUTE].type = T_ABSOLUTE;
	segv[T_CODE].type = T_CODE;
	segv[T_CODE].uncommon = 1;
	segv[T_DATA].type = T_DATA;
	segv[T_DATA].uncommon = 1;
	segv[T_DATA].default_base = -1;
	for (i=T_COMMON; i<MAX_SEGMENTS; i++) {
		segv[i].type = -1;
		segv[i].default_base = -1;
	}
}

static
struct segment *search_segment(int type, char *common_name, int action)
{
	struct segment *s;

	if (type < T_COMMON) return segv+type;
	for (s=segv+T_COMMON; s->type==T_COMMON; s++)
		if (!strcmp(common_name, s->common_name)) return s;
	if (action == A_FIND) return NULL;
	if (s-segv > MAX_SEGMENTS)
		die(E_RESOURCE, "ld80: Too many common segments\n");
	strncpy(s->common_name, common_name, NAMELEN);
	s->type = T_COMMON;
	return s;
}

void mark_uncommon(char *common_name)
{
	search_segment(T_COMMON, common_name, A_ENTER)->uncommon = 1;
}

static
int uncommon(int type, char *common_name)
{
	struct segment *a;

	a = search_segment(type, common_name, A_FIND);
	return a ? a->uncommon : 0;
}

void set_base_address(int type, char *common_name, int base, int align)
{
	if (align) {
		base = -base;	/* alignment is represented by negative value */
		if (base == 0) base = -1;
	}
	search_segment(type, common_name, A_ENTER)->default_base = base;
}

static
int base_address(int type, char *common_name)
{
	struct segment *a;

	a = search_segment(type, common_name, A_FIND);
	return a ? a->default_base : -1;
}

#ifdef	DEBUG
static
int dump_line(unsigned char *b, int virt_add, int len)
{
	int i;

	printf("\t%.4x: ",virt_add);
	for (i=0; i<virt_add%16; i++) printf("   ");
	for (i=0; !i || (virt_add%16 && len); i++, len--, virt_add++) {
//		printf("%.2X%c", *b++, MARKED(virt_add)?'.':' ');
		printf("%.2X%c", *b++, virt_add%16==7 ? '-' : ' ');
	}
	putchar('\n');
	return i;
}

static
void hexdump(unsigned char *b, int virt_add, int len)
{
	int i, may_discard=0, j=0;

	while (len>0) {
		if (may_discard && !memcmp(b, b-16, 16)) {
			i = 16;
			if (j++==0) printf("\t...\n");
		}
		else {
			i = dump_line(b, virt_add, len);
			j = 0;
		}
		may_discard = i==16;
		len -= i;
		virt_add += i;
		b += i;
	}
	printf("\t%.4x:\n",virt_add);
}

void dump_sections(void)
{
	char *segname[4] = {"absolute","code","data","common"};
	struct segment *segp;
	struct section *sp;
	struct node *n;

	printf("\nSection dump:\n");
	for (segp=segv; segp<=segv+T_COMMON || segp->secs; segp++) {
		char *sname = segp->type >= 0 && segp->type < 4 ? segname[segp->type] : "<unused>";
		printf("seg=%p type=%d(%s) name=/%s/ %s def.base=%.4x "
			"max.size=%.4x\n",
			segp, segp->type, sname,
			segp->common_name, segp->uncommon ? "concatenated" :
			"overlapped", segp->default_base, segp->maxsize);
		for (sp=segp->secs; sp; sp=sp->next) {
			printf("  sec=%p base=%.4x lc=%.4x "
				"len=%.4x module=%s:%s\n", sp,
				sp->base, sp->lc, sp->len,
				sp->filename, sp->module_name);
			for (n=sp->nodehead; n; n=n->next) {
				printf("    node: offset=%.4x type=%d "
					"value=%.4x name=%s\n",
					n->at.offset, n->type,
					n->value,
					n->symbol ? n->symbol->name : "");
			}
			if (segp->type!=T_ABSOLUTE || sp->next==NULL)
				if (sp->buffer) hexdump(sp->buffer,
					sp->base==-1 ? 0 : sp->base, sp->len);
		}
		printf("-------------------------------\n");
	}
}
#endif	/* ifdef DEBUG */

void relocate_sections(void)
{
	struct section *sp;
	struct segment *segp;
	int loc = 0;	/* default CSEG location */
	int m;

	for (segp=segv+T_CODE; segp<segv+T_COMMON || segp->secs; segp++) {
		if (segp->uncommon) for (sp=segp->secs; sp; sp=sp->next) {
			if (sp->base < 0 ) {
				m = -sp->base;
				if (loc % m) loc += m - loc % m;
				sp->base = loc;
			}
			loc = sp->base + sp->len;
			if (loc > 0xffff)
				die(E_INPUT, "ld80: Program too large\n");
		}
		else {
			if (segp->default_base < 0 ) {
				m = -segp->default_base;
				if (loc % m) loc += m - loc % m;
				segp->default_base = loc;
			}
			else loc = segp->default_base;
			for (sp=segp->secs; sp; sp=sp->next) sp->base = loc;
			loc = segp->default_base + segp->maxsize;
		}
	}
}

void join_sections(int nodata)
{
	int i, len;
	char *segname[4] = {"abs","code","data","common"};
	struct segment *segp;
	struct section *sp;
	unsigned char *p;

	for (segp=segv+T_CODE;
			nodata ? (segp<segv+T_DATA) :
			(segp<segv+T_COMMON || segp->secs);
			segp++) {
		for (sp=segp->secs; sp; sp=sp->next) {
			overlap = 0;
			for (i=sp->base, p=sp->buffer, len=sp->len;
					len; i++, p++, len--) {
				aseg[i] = *p;
				if (!overlap && MARKED(i)) {
					fprintf(stderr,
						"ld80: Overlapping %s segment "
						"%s in file %s\n",
						segname[segp->type],
						sp->segment->type == T_COMMON ?
						sp->segment->common_name[0] ?
						sp->segment->common_name :
						"//" : "",
						sp->filename);
					overlap++;
					fatalerror = 1;
				}
				else MARK_BYTE(i);
			}
			free(sp->buffer);
			sp->buffer = NULL;
		}
	}
}

#define SECT(x)  (*((struct section **)(x)))

static
int by_base(const void *a, const void *b)
{
	int i;
	i = SECT(a)->base - SECT(b)->base;
	if (i) return i;
	return strcmp(SECT(a)->module_name, SECT(b)->module_name);
}

void print_map(FILE *f)
{
	char segtype[] = "APDC", buf[NAMELEN+8];
	struct segment *segp;
	struct section **slist, *sp;
	int secno = 0, i = 0;

	for (segp=segv+T_CODE; segp<=segv+T_COMMON || segp->secs; segp++)
		for (sp=segp->secs; sp; sp=sp->next)
			secno++;
	if (secno == 0) return;

	slist = calloc_or_die(secno, sizeof(*slist));
	for (segp=segv+T_CODE; segp<=segv+T_COMMON || segp->secs; segp++)
		for (sp=segp->secs; sp; sp=sp->next)
			slist[i++] = sp;
	qsort((void*)slist, secno, sizeof(*slist), by_base);

	fprintf(f,
"Addr  Length Typ Name       Module   File\n"
"=====================================================================\n");

	for (i=0; i<secno; i++) {
		sp = slist[i];
		if (sp->len == 0) continue;
		if (sp->segment->type == T_COMMON)
			sprintf(buf,"C  /%s/", sp->segment->common_name);
		else sprintf(buf,"%c  -",segtype[sp->segment->type]);
		fprintf(f,"%.4x   %.4x   %-13s %-8s %s\n",
			sp->base, sp->len, buf, sp->module_name, sp->filename);
	}
	fputc('\n',f);
}
