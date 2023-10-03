#ifndef NEED_HSEARCH
#include <search.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ld80.h"

static struct symbol *symbol_table;
static int next_symbol, symbol_table_size;

struct symbol *find_symbol(char *name)
{
	struct symbol *sym;
	ENTRY e, *ep;

	sym = get_symbol(name);
	if (sym) return sym;

	if (next_symbol >= symbol_table_size) die(E_RESOURCE,
		"ld80: Symbol table exceeds %d entries. "
		"Use -S option.\n", symbol_table_size);
	sym = symbol_table + next_symbol++;
	strcpy(sym->name, name);
	sym->value = UNDEFINED;

	e.key = strdup(sym->name);
	e.data = (char *)sym;
	ep = hsearch(e, ENTER);
	if (ep == NULL) die(E_RESOURCE, "ld80: Not enough memory\n");
	return sym;
}

int add_symbol(char *name, int offset, struct section *section)
{
	struct symbol *sym;

	sym = find_symbol(name);
	if (sym->at.section) {
		fprintf(stderr,"ld80: Symbol %s is redefined\n", name);
		fatalerror = 1;
	}
	sym->at.offset = offset;
	sym->at.section = section;
	sym->value = 0;

	return 1;
}

struct symbol *get_symbol(char *name)
{
	ENTRY e, *ep;

	e.key = name;
	ep = hsearch(e, FIND);
	return (struct symbol *)(ep ? ep->data : NULL);
}

void set_symbols(void)
{
	int i;
	struct symbol *s;

	for (i=0; i<symbol_table_size; i++) {
		s = symbol_table + i;
		if (s->at.section == NULL) break;
		s->value = s->at.section->base + s->at.offset;
	}
}

int init_symbol(int n)
{
	symbol_table = calloc(n, sizeof(struct symbol));
	if (symbol_table == NULL || hcreate(n) == 0)
		die(E_RESOURCE, "ld80: Not enough memory for %d symbols\n", n);
	next_symbol = 0;
	symbol_table_size = n;
	return n;
}

void clear_symbol(void)
{
	free(symbol_table);
	hdestroy();
}

#ifdef	DEBUG
void dump_symbols(void)
{
	int i;

	for (i=0; i<next_symbol; i++) {
		printf("name=%-8s section=%p offset=%.4x value=%.4x\n",
			symbol_table[i].name, symbol_table[i].at.section,
			symbol_table[i].at.offset, symbol_table[i].value);
	}
}
#endif

#define	SYM(x)	(*((struct symbol **)(x)))

/*
static
int by_value(const void *a, const void *b)
{
	return SYM(a)->value - SYM(b)->value;
}
*/

static
int by_name(const void *a, const void *b)
{
	return strcmp(SYM(a)->name, SYM(b)->name);
}

void print_symbol_table(FILE *f)
{
	int i;
	struct symbol *s, **slist, **sp;

	slist = calloc_or_die(next_symbol, sizeof(*slist));
	for (i=0; i<next_symbol; i++) slist[i] = symbol_table+i;
	qsort((void*)slist, next_symbol, sizeof(*slist), by_name);

	fprintf(f,
"Symbol   Value Module   File\n"
"======================================================================\n");

	for (i=0,sp=slist; i<next_symbol; i++, sp++) {
		s = *sp;
		if (s->at.section) fprintf(f,"%-8s %.4x  %-8s %s\n",
				s->name, s->value, s->at.section->module_name,
				s->at.section->filename);
		else fprintf(f,"%-8s *** UNDEFINED ***\n", s->name);
	}
}
