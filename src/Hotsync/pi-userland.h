/*
 * $Id: pi-userland.h,v 1.8 2006/10/17 13:24:07 desrod Exp $
 *
 * userland.h: General definitions for userland conduits.
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
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef PALM_USERLAND_H
#define PALM_USERLAND_H

//#include <popt.h>
#include <stdlib.h>
#include "pi-appinfo.h"

/*
 * This file defines general stuff for conduits -- common option processing,
 * perhaps some utility functions, etc. It prescribes how some of the code
 * of a conduit should look like, so as to preserve uniformity of options
 * and handling.
 *
 * - Each conduit should start its options table with
 *   USERLAND_RESERVED_OPTIONS.  This sets up the standard options --port,
 *   --version, --quiet as well as popt autohelp.
 * - If an error is found while processing options, call plu_badoption. 
 *   This produces a standard error message.
 * - If no error is found, call plu_connect() instead of pilot_connect(). 
 *   This does the same as pilot_connect, but obeys --quiet and produces
 *   output on stderr only if there _is_ an error.
 */


#if 0
/***********************************************************************
 *
 * Option-handling functions.
 *
 ***********************************************************************/

/*
 * These are definitions for popt support in userland. Every conduit's popt
 * table should start with USERLAND_RESERVED_OPTIONS to insert the standard
 * options into it. Also enables autohelp.
 */

#define USERLAND_RESERVED_OPTIONS \
	{NULL,0,POPT_ARG_INCLUDE_TABLE,plu_common_options,0,"Options common to all conduits.",NULL}, \
	POPT_AUTOHELP

/*
 * Complain about a bad (ie. nonexistent) option and exit();
 */

extern void plu_badoption(poptContext pc, int optc);


/*
 * Add an alias to a popt context; remember to use --bad-option in the alias
 * to add a complaint about deprecated options. Do not pass in both a long
 * and a short option in one go, use two calls for that.
 */
void plu_popt_alias(poptContext pc,
	const char *alias_long,
	char alias_short,
	const char *expansion);

/*
 * Set explanation of what options to use in response to an alias that
 * contains --bad-option.
 */
void plu_set_badoption_help(const char *help);
#endif

/***********************************************************************
 *
 * Connection functions.
 *
 ***********************************************************************/

/*
 * Connect to the Pilot specified by any --port option, respecting the quiet
 * flag as well. This is basically pilot_connect(), but marginally cleaner.
 */

extern int plu_connect(char *plu_port);
extern int plu_accept(int sd, int plu_timeout);
extern int plu_close(int sd);


/***********************************************************************
 *
 * Things to do once you're connected to the handheld.
 *
 ***********************************************************************/

/*
 * Look up a category name. Argument @p info is the category part of the
 * AppInfo block for the database, while @p name is the category to look up. 
 * Returns the index of the category if found (0..15) or -1 if not.
 *
 * The flags passed to findcategory are a bitwise or of enums; the meanings
 * are:
 *
 *   NOFLAGS          : Match case-sensitive, return -1 if not found, do not
 *                      match numbers as category numbers, do not complain.
 *   CASE_INSENSITIVE : Match in a case-insensitive fashion.
 *   DEFAULT_UNFILED  : Return 0 (unfiled) instead of -1 on no-match.
 *   MATCH_NUMBERS    : Match number strings 0 .. 15 as categories 0 .. 15.
 *   WARN_UNKNOWN     : Complain on stderr if category not found.
 */
typedef enum {
	PLU_CAT_NOFLAGS = 0,
	PLU_CAT_CASE_INSENSITIVE = 0x0001,
	PLU_CAT_DEFAULT_UNFILED = 0x0002,
	PLU_CAT_MATCH_NUMBERS = 0x0004,
	PLU_CAT_WARN_UNKNOWN = 0x0008
	} plu_findcategory_flags_t;

extern int plu_findcategory(const struct CategoryAppInfo *info, const char *name, int flags);

typedef struct {
	/* Numeric parts of the ROM version */
	int major,
		minor,
		bugfix,
		build,
		state;
	/* Textual representation of same: xxx.xx.xx-xxxxx */
	char name[16];
} plu_romversion_t;

/*
 * Retrieve the ROM version from the Palm; returns -1 on failure, 0
 * otherwise and fills the fields of the @p d structure.
 */
extern int plu_getromversion(int sd, plu_romversion_t *d);


/***********************************************************************
 *
 * File-handling functions on the PC.
 *
 ***********************************************************************/

/*
 * Function:	protect_files
 *
 * Summary:     Adjust output file name so as to not overwrite an exsisting
 *              file.
 *
 * Parameters:  name        <-> buffer for filename
 *              extension   --> file extension to add to name
 *              namelength  --> size of buffer
 *
 * Returns:     1 file name protected (and stored in buffer name)
 *              0 no alernate name found
 *             -1 other failure
 *
 */
int plu_protect_files(char *name, const char *extension, const size_t namelength);

/*
 * We need to be able to refer to the table of common options.
 */

//extern struct poptOption plu_common_options[];


#endif

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
