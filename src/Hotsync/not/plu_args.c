/*
 * $Id: plu_args.c,v 1.9 2006/10/12 14:21:21 desrod Exp $ 
 *
 * plu_args.c: common popt argument processing routines
 *
 * Copyright (C) 2004 by Adriaan de Groot <groot@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include "pi-userland.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi-header.h"

char *plu_port = NULL;
int plu_timeout = 0;
int plu_quiet = 0;
static char *badoption_help = NULL;


#define ARGUMENT_BAD_OPTION	17227

static void callback(poptContext pc,
	int reason,
	const struct poptOption *opt,
	const char *arg,
	void *data)
{
	static int have_complained_already = 0;
	switch(opt->val) {
	case 'v' :
		//print_splash(poptGetInvocationName(pc));
		exit(0);
		break;
	case ARGUMENT_BAD_OPTION:
		if (!have_complained_already) {
			fprintf(stderr,"   WARNING: You are using deprecated options. %s\n\n", badoption_help ? "Use these instead:" : "See --help for more.");
			if (badoption_help) fprintf(stderr,"%s",badoption_help);
			have_complained_already=1;
		}
		break;
	}
}

struct poptOption plu_common_options[] = {
	{ NULL, 0, POPT_ARG_CALLBACK, callback, 0, NULL, NULL},
	{ "port",    'p', POPT_ARG_STRING, &plu_port,  0 , "Use device <port> to communicate with Palm", "<port>"},
	{ "timeout",    't', POPT_ARG_INT, &plu_timeout,  0 , "Use timeout <timeout> seconds", "<timeout>"},
	{ "version",  0 , POPT_ARG_NONE,    NULL, 'v', "Display version information", NULL},
	{ "quiet",   'q', POPT_ARG_NONE,  &plu_quiet,  0 , "Suppress 'Hit HotSync button' message", NULL},
	{ "bad-option",0, POPT_ARG_NONE | POPT_ARGFLAG_DOC_HIDDEN, NULL, ARGUMENT_BAD_OPTION, NULL, NULL },
	POPT_TABLEEND
} ;


void plu_badoption(poptContext pc, int optc)
{
	fprintf(stderr, "%s: %s\n",
		poptBadOption(pc, POPT_BADOPTION_NOALIAS),
		poptStrerror(optc));

        poptPrintUsage(pc, stderr, 0);
	exit(1);
}


void plu_popt_alias(poptContext pc,
	const char *alias_long,
	char alias_short,
	const char *expansion)
{
	struct poptAlias alias = {
		alias_long,
		alias_short,
		0,
		NULL
	} ;

	poptParseArgvString(expansion,&alias.argc,&alias.argv);
	poptAddAlias(pc,alias,0);
}


void plu_set_badoption_help(const char *h)
{
	if (badoption_help) free(badoption_help);
	badoption_help = NULL;
	if (h) badoption_help = strdup(h);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
