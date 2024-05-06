/*
 * $Id: pi-versamail.h,v 1.7 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-versamail.h: Palm VersaMail application support (replaced Palm Mail 
 *                 application)
 *
 * Copyright (c) 2005, Florent Pillet.
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
 * * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
 */

#ifndef _PILOT_VERSAMAIL_H_
#define _PILOT_VERSAMAIL_H_

#include <time.h>
#include "pi-args.h"
#include "pi-appinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

	struct VersaMail {
		unsigned long imapuid;
		struct tm date;
		unsigned int category;
		unsigned int accountNo;
		unsigned int unknown1;
		unsigned int download;
		unsigned int mark;
		unsigned int unknown2;
		unsigned int reserved1;
		unsigned int reserved2;
		unsigned int read;
		unsigned int msgSize;
		unsigned int attachmentCount;
		char *messageUID;
		char *to;
		char *from;
		char *cc;
		char *bcc;
		char *subject;
		char *dateString;
		char *body;
		char *replyTo;
		void *unknown3;
		unsigned int unknown3length;
	};

	struct VersaMailAppInfo {
		struct CategoryAppInfo category;
	};

	extern int unpack_VersaMail
	    PI_ARGS((struct VersaMail *, char *record, size_t len));

	extern int pack_VersaMail
	    PI_ARGS((struct VersaMail *a, char *buffer, size_t len));

	extern void free_VersaMail PI_ARGS((struct VersaMail *));

	extern void free_VersaMailAppInfo PI_ARGS((struct VersaMailAppInfo *));
	extern int unpack_VersaMailAppInfo PI_ARGS((struct VersaMailAppInfo *,
		unsigned char *AppInfo, size_t len));

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_VERSAMAIL_H_ */
