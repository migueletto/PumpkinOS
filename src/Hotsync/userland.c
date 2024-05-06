/*
 * $Id: userland.c,v 1.19 2006/10/12 14:21:22 desrod Exp $ 
 *
 * userland.c: General definitions for userland conduits.
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
//#include <ctype.h>

#include "pi-socket.h"
#include "pi-dlp.h"
#include "pi-source.h"

#include "pi-debug.h"
#include "sys.h"
#include "debug.h"

int plu_connect(char *plu_port) {
	int sd = -1;
	//int newsd;
	int result;
	//struct  SysInfo sys_info;

	if ((sd = pi_socket(PI_AF_PILOT, PI_SOCK_STREAM, PI_PF_DLP)) < 0) {
		debug(DEBUG_ERROR, SYS_DEBUG, "unable to create socket %s", plu_port);
		return -1;
	}

	result = pi_bind(sd, plu_port);

	if (result < 0) {
		debug(DEBUG_ERROR, SYS_DEBUG, "unable to bind to port %s", plu_port);
		pi_close(sd);
		return result;
	}

	debug(DEBUG_INFO, SYS_DEBUG, "listening for incoming connection on %s fd %d", plu_port, sd);

	if (pi_listen(sd, 1) < 0) {
		debug(DEBUG_ERROR, SYS_DEBUG, "Errow listening on %s fd %d", plu_port, sd);
		pi_close(sd);
		return -1;
	}

/*
	newsd = pi_accept_to(sd, 0, 0, plu_timeout);
	if (newsd < 0) {
		if (newsd == PI_ERR_SOCK_TIMEOUT) {
			debug(DEBUG_ERROR, SYS_DEBUG, "timeout accepting connection on %s", plu_port);
		} else {
			debug(DEBUG_ERROR, SYS_DEBUG, "error accepting connection on %s", plu_port);
		}
		pi_close(sd);
		return -1;
	}
	pi_close(sd);
	sd = newsd;

	debug(DEBUG_INFO, SYS_DEBUG, "connected");

	if (dlp_ReadSysInfo(sd, &sys_info) < 0) {
		debug(DEBUG_ERROR, SYS_DEBUG, "error read system info on %s", plu_port);
		pi_close(sd);
		return -1;
	}

	dlp_OpenConduit(sd);
*/
	return sd;
}

int plu_accept(int sd, int plu_timeout) {
	struct  SysInfo sys_info;
	int newsd;

	newsd = pi_accept_to(sd, 0, 0, plu_timeout);
	if (newsd == PI_ERR_SOCK_TIMEOUT) {
		newsd = 0;
	}

	if (newsd > 0) {
		if (dlp_ReadSysInfo(sd, &sys_info) < 0) {
			debug(DEBUG_ERROR, SYS_DEBUG, "error read system info");
			pi_close(newsd);
			newsd = -1;
		} else {
			dlp_OpenConduit(sd);
		}
	}

	return newsd;
}

int plu_close(int sd) {
	pi_close(sd);
	return 0;
}


int plu_findcategory(const struct CategoryAppInfo *info, const char *name, int flags)
{
	int cat_index, match_category;

	match_category = -1;
	for (cat_index = 0; cat_index < 16; cat_index += 1) {
		if (info->name[cat_index][0]) {
			if (flags & PLU_CAT_CASE_INSENSITIVE) {
				if (strncasecmp(info->name[cat_index], name, 15) == 0) {
					match_category = cat_index;
					break;
				}
			} else {
				if (strncmp(info->name[cat_index],name,15) == 0) {
					match_category = cat_index;
					break;
				}
			}
		}
	}

	if ((match_category == -1)  && (flags & PLU_CAT_MATCH_NUMBERS)) {
		//while (isspace((int)(*name))) {
		while (name[0] == ' ' || name[0] == '\t') {
			name++;
		}
		//if (isdigit((int)(*name))) {
		if (name[0] >= '0' && name[0] <= '9') {
			match_category = atoi(name);
		}

		if ((match_category < 0) || (match_category > 15)) {
			match_category = -1;
		}
	}

	if (flags & PLU_CAT_WARN_UNKNOWN) {
		if (match_category == -1) {
			debug(DEBUG_INFO, SYS_DEBUG, "unknown category '%s'%s",
				name,
				(flags & PLU_CAT_DEFAULT_UNFILED) ? ", using 'Unfiled'" : "");
		}
	}

	if (flags & PLU_CAT_DEFAULT_UNFILED) {
		if (match_category == -1) {
			match_category = 0;
		}
	}

	return match_category;
}

int plu_getromversion(int sd, plu_romversion_t *d)
{
	uint32_t ROMversion;

	if ((sd < 0)  || !d) {
		return -1;
	}

	if (dlp_ReadFeature(sd, makelong("psys"), 1, &ROMversion) < 0) {
		return -1;
	}

	d->major =
		(((ROMversion >> 28) & 0xf) * 10) + ((ROMversion >> 24) & 0xf);
	d->minor = ((ROMversion >> 20) & 0xf);
	d->bugfix = ((ROMversion >> 16) & 0xf);
	d->state = ((ROMversion >> 12) & 0xf);
	d->build =
		(((ROMversion >> 8) & 0xf) * 100) +
		(((ROMversion >> 4) & 0xf) * 10) + (ROMversion & 0xf);

	memset(d->name,0,sizeof(d->name));
	snprintf(d->name, sizeof(d->name),"%d.%d.%d", d->major, d->minor, d->bugfix);

	if (d->state != 3) {
		int len = strlen(d->name);
		snprintf(d->name + len, sizeof(d->name) - len,"-%s%d", (
			(d->state == 0) ? "d" :
			(d->state == 1) ? "a" :
			(d->state == 2) ? "b" : "u"), d->build);
	}

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */

