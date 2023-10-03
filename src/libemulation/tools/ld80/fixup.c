#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ld80.h"

int undefined = 0;
int nodenum = 0;
static struct node *tos;

static void push(struct node *);
static struct node *pop(void);

void add_fixup(struct section *loc, struct section *target, int offset)
{
	struct fixup *f = calloc_or_die(1, sizeof(*f));

	f->lc = loc->lc;
	f->at.section = target;
	f->at.offset = offset;
#ifdef DEBUG
	if (debug) printf("add_fixup: f=%p at %p:%.4x to %p:%.4x base=%.4x\n",
		f,loc,f->lc,f->at.section,f->at.offset,loc->base);
#endif
	f->next = loc->fixups;
	loc->fixups = f;
}

static
struct fixup *get_fixup(int lc, struct section *section)
{
	struct fixup **fpp, *fp;

	/* linear search on fixup chain */
	for (fpp=&section->fixups, fp=*fpp; fp; fpp=&fp->next,fp=fp->next) {
		if (fp->lc == lc) break;	/* gotcha */
	}
	if (*fpp == NULL) return NULL;		/* not found */
	*fpp = fp->next;			/* remove from chain */
	return fp;
}

struct node *add_node(struct section *section, int offset, int type)
{
	struct node *n = calloc_or_die(1, sizeof(*n));

	if (section->nodetail) section->nodetail->next = n;
	else section->nodehead = n;
	section->nodetail = n;
	n->at.offset = offset;
	n->type = type;
	n->nodenum = nodenum++;
	return n;
}

void convert_chain_to_nodes(char *name, int offset, struct section *section)
{
	struct node *n;
	struct fixup *f;

#ifdef DEBUG
	if (debug) printf("ext_chain: %s at %p:%.4x\n", name, section, offset);
#endif

	while (1) {
		n = add_node(section, offset, N_EXTERNAL);
		n->symbol = find_symbol(name);

		n = add_node(section, offset, N_WORD);

		f = get_fixup(offset, section);
		if (f==NULL) break;	/* end of chain */
		offset = f->at.offset;
		section = f->at.section;
		free(f);
	}
}

void set_fixups(void)
{
	struct segment *segp;
	struct section *sp;
	struct fixup *f, *ff;
	unsigned char *p;

	for (segp=segv; segp<=segv+T_COMMON || segp->secs; segp++) {
		for (sp=segp->secs; sp; sp=sp->next) {/* all sections */
			for (f=sp->fixups; f; ff=f, f=f->next, free(ff)) {
				p = sp->buffer + f->lc;
				*((unsigned short *)p) = (unsigned short)
					(f->at.section->base + f->at.offset);
#ifdef DEBUG
				if (debug) printf("fixup: f=%p sec=%p loc=%.4x "
					"target=%p rel=%.4x abs=%.4x\n", f, sp,
					f->lc, f->at.section, f->at.offset,
					f->at.section->base + f->at.offset);
#endif
			}
		}
	}
}

void resolve_externals(void)
{
	struct segment *segp;
	struct section *sp;
	struct node *n;
	struct symbol *s;

	for (segp=segv; segp<=segv+T_COMMON || segp->secs; segp++) {
		for (sp=segp->secs; sp; sp=sp->next) {/* all sections */
			for (n=sp->nodehead; n; n=n->next) {
				if (n->type != N_EXTERNAL) continue;
				n->type = N_OPERAND;
				s=get_symbol(n->symbol->name);
				if (s==NULL || s->value==UNDEFINED) {
					fprintf(stderr,
						"ld80: Undefined symbol %s\n",
						n->symbol->name);
					undefined++;
					fatalerror = 1;
					n->value = 0;
				}
				else {
					n->value = s->value;
				}
#ifdef DEBUG
				if (debug) printf("resolve: %s=%.4x\n",
					n->symbol->name, n->value);
#endif
			}
		}
	}
	
}

#define NODE(x)  (*((struct node **)(x)))

static
int by_offset(const void *a, const void *b)
{
	int i;
	i = NODE(a)->at.offset - NODE(b)->at.offset;
	if (i) return i;
	return NODE(a)->nodenum - NODE(b)->nodenum;
}

static
void sort_nodes(struct section *sp)
{
	int i,nodeno;
	struct node *n, **nlist;

	for (nodeno=0,n=sp->nodehead; n; n=n->next) nodeno++;
	if (nodeno==0) return;

	nlist = calloc_or_die(nodeno, sizeof(*nlist));
	for (i=0,n=sp->nodehead; n; n=n->next, i++) nlist[i] = n;
	qsort((void*)nlist, nodeno, sizeof(*nlist), by_offset);
	for (i=0; i<(nodeno-1); i++) nlist[i]->next = nlist[i+1];
	nlist[nodeno-1]->next = NULL;

	sp->nodehead = nlist[0];
	sp->nodetail = nlist[nodeno-1];
	free(nlist);
	return;
}

void process_nodes(void)
{
	struct segment *segp;
	struct section *sp;
	struct node *n;
	void *p;

	/* nested for cycles get all sections */
	for (segp=segv; segp<=segv+T_COMMON || segp->secs; segp++)
			for (sp=segp->secs; sp; sp=sp->next) {
		sort_nodes(sp);
		while ((n=sp->nodehead) != NULL) { /* get all nodes */
			sp->nodehead = n->next;
//printf("  node: offset=%.4x type=%d value=%.4x name=%s\n",
//n->at.offset, n->type, n->value, n->symbol ? n->symbol->name : "");
			switch (n->type) {
			case N_OPERAND:
				if (n->at.section)
					n->value += n->at.section->base;
				if (tos && tos->at.offset==n->at.offset &&
						tos->type==N_EXTPLUS) {
					tos->value += n->value;
					tos->type = N_OPERAND;
				}
				else {
					push(n);
					n = NULL;
				}
				break;
			case N_BYTE:
				free(n);
				n = pop();
				p = sp->buffer + n->at.offset; /* cast removal needed for mac, too */
#ifdef DEBUG
				if (debug) printf("byte sp=%p offset=%.4x "
					"value=%.2x\n",
					sp, n->at.offset, n->value&0xff);
#endif
				*((unsigned char*)p) = n->value;
				break;
			case N_WORD:
				free(n);
				n = pop();
				p = sp->buffer + n->at.offset; /* cast removal needed for mac, too */
#ifdef DEBUG
				if (debug) printf("word sp=%p offset=%.4x "
					"value=%.4x\n",
					sp, n->at.offset, n->value&0xffff);
#endif
				*((short*)p) = n->value;
				break;
			case N_NOT:
				tos->value = ~tos->value;
				break;
			case N_UNARYMINUS:
				tos->value = -tos->value;
				break;
			case N_HIGH:
				tos->value >>= 8;
				/* FALL THROUGH */
			case N_LOW:
				tos->value &= 0xff;
				break;
			case N_PLUS:
				free(n);
				n = pop();
				tos->value += n->value;
				break;
			case N_MINUS:
				free(n);
				n = pop();
				tos->value -= n->value;
				break;
			case N_MULT:
				free(n);
				n = pop();
				tos->value *= n->value;
				break;
			case N_DIV:
				free(n);
				n = pop();
				tos->value /= n->value;
				break;
			case N_MOD:
				free(n);
				n = pop();
				tos->value %= n->value;
				break;
			case N_EXTPLUS:
			case N_EXTMINUS:
				if (n->at.section)
					n->value += n->at.section->base;
				if (n->type==N_EXTMINUS) {
					n->value = -n->value;
					n->type = N_EXTPLUS;
				}
				push(n);
				n = NULL;
				break;
			case N_EXTERNAL:
				die(E_INPUT,
					"ld80: Unresolved external node\n");
			default:
				die(E_INPUT, "ld80: Unknown node type %d\n",
					n->type);
			} /* switch */
			if (n) free(n);
//if (tos) printf("tos: at=%p:%.4x type=%d value=%.4x symbol=%s\n", tos->at.section,
//tos->at.offset, tos->type, tos->value, tos->symbol ? tos->symbol->name : "");
//else printf("empty\n");
		} /* while */
		if (tos) {
			die(E_INPUT,"ld80: Expression evaluation error\n");
//printf("tos: at=%p:%.4x type=%d value=%.4x symbol=%s\n", tos->at.section,
//tos->at.offset, tos->type, tos->value, tos->symbol ? tos->symbol->name : "");
		}
	} /* for for */
}

static void push(struct node *n)
{
	n->next = tos;
	tos = n;
}

static struct node *pop(void)
{
	struct node *retval = tos;
	if (tos) tos = tos->next;
	return retval;
}
