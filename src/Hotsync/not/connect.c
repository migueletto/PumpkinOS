/*
 * $Id: connect.c,v 1.60 2006/10/12 14:21:22 desrod Exp $
 *
 * connect.c:  Palm Serial, USB, IR connection routines
 *
 * Copyright (c) 2001, David A. Desrosiers
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#include <unistd.h>
#include <sys/stat.h>

#include "pi-socket.h"
#include "pi-dlp.h"
#include "pi-header.h"

/***********************************************************************
 *
 * Function:    pilot_connect  [DEPRECATED]
 *
 * Summary:     Connect to a Palm device.
 *
 * Parameters:  port. Communications port through which the Palm device is
 *              connected.
 *
 * Returns:     Socket descriptor of type 'client_sd', if successful.
 *		Returns 1, if the connection can not be established.
 *
 *  'port' is allowed to be NULL for the pi_bind call because pi_bind will
 *  check your PILOTPORT environment variable.  If port is NULL and the
 *  attempt to connect fails for any reason, we simply assume the user didn't
 *  provide one, because pilot_connect doesn't have a way of knowing, for the
 *  time being.
 *
 * This function is deprecated because it doesn't really belong in libpisock.
 * It will remain at least until it has been properly replaced.
 *
 ***********************************************************************/
int
pilot_connect(const char *port)
{
	int 	sd	= -1, 	/* Socket, formerly parent/client_socket */
		result;

	struct  SysInfo sys_info;

	fprintf(stderr, "\n");
	fprintf(stderr,"   DEPRECATED: The application is calling pilot_connect()\n");
	if ((sd = pi_socket(PI_AF_PILOT,
			PI_SOCK_STREAM, PI_PF_DLP)) < 0) {
		fprintf(stderr, "\n   Unable to create socket '%s'\n", port);
		return -1;
	}

	result = pi_bind(sd, port);

	if (result < 0) {
		if (port == NULL)
			fprintf(stderr, "   No port specified\n");
		else
			fprintf(stderr, "   Unable to bind to port: %s\n", port);

		fprintf(stderr, "   Please use --help for more information\n\n");
		return result;
	}

	if (isatty(fileno(stdout))) {
		printf("\n   Listening for incoming connection on %s... ",
			port);
		fflush(stdout);
	}

	if (pi_listen(sd, 1) < 0) {
		fprintf(stderr, "\n   Error listening on %s\n", port);
		pi_close(sd);
		return -1;
	}

	sd = pi_accept(sd, 0, 0);
	if (sd < 0) {
		fprintf(stderr, "\n   Error accepting data on %s\n", port);
		pi_close(sd);
		return -1;
	}

	if (isatty(fileno(stdout))) {
		printf("connected!\n\n");
	}

	if (dlp_ReadSysInfo(sd, &sys_info) < 0) {
		fprintf(stderr, "\n   Error read system info on %s\n", port);
		pi_close(sd);
		return -1;
	}

	dlp_OpenConduit(sd);
	return sd;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
