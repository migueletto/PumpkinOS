/*
 * $Id: pi-header.c,v 1.25 2006/10/12 14:21:22 desrod Exp $
 *
 * pi-header.c:  Splash for the version/etc.
 *
 * Copyright (c) 2000, David A. Desrosiers
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "pi-version.h"
#include "pi-header.h"

void print_splash(const char *progname)
{
        char 	*patchlevel = "";

	fprintf(stderr,"   DEPRECATED: The application is calling print_splash()\n");
#ifdef PILOT_LINK_PATCH
        patchlevel = PILOT_LINK_PATCH;
#endif
	printf("   .--------------------------------------------.\n"
	       "   | (c) Copyright 1996-2006, pilot-link team   |\n"
	       "   |   Join the pilot-link lists to help out.   |\n"
	       "   `--------------------------------------------'\n"
	       "   This is %s, from pilot-link version %d.%d.%d%s\n\n"
	       "   Build target..: %s\n"
	       "   Build date....: %s %s\n\n", progname,
		PILOT_LINK_VERSION, PILOT_LINK_MAJOR, PILOT_LINK_MINOR,
		patchlevel, HOST_OS, __DATE__, __TIME__);

	printf("   pilot-link %d.%d.%d%s is covered under the GPL/LGPL\n",
                PILOT_LINK_VERSION, PILOT_LINK_MAJOR, PILOT_LINK_MINOR,
                patchlevel);

	printf("   See the file COPYING under docs for more info.\n\n"
	       "   Please use --help for more detailed options.\n");

}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
