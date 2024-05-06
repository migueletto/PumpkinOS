/*
 * $Id: pi-hinote.h,v 1.9 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-hinote.h: HiNote application macros (deprecated)
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
 
#ifndef _PILOT_HINOTE_H_	/* -*- C++ -*- */
#define _PILOT_HINOTE_H_

#include "pi-appinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct HiNoteNote {
		int flags;
		int level;
		char *text;
	} HiNoteNote_t;

	typedef struct HiNoteAppInfo {
		struct CategoryAppInfo category;
		unsigned char reserved[48];
	} HiNoteAppInfo_t;

	extern void free_HiNoteNote PI_ARGS((struct HiNoteNote *));
	extern int unpack_HiNoteNote
	    PI_ARGS((struct HiNoteNote *, unsigned char *record, int len));
	extern int pack_HiNoteNote
	    PI_ARGS((struct HiNoteNote *, unsigned char *record, int len));
	extern int unpack_HiNoteAppInfo
	    PI_ARGS((struct HiNoteAppInfo *, unsigned char *AppInfo,
		     size_t len));
	extern int pack_HiNoteAppInfo
	    PI_ARGS((struct HiNoteAppInfo *, unsigned char *AppInfo,
		     size_t len));

#ifdef __cplusplus
}
#endif				/*__cplusplus*/
#endif				/* _PILOT_HINOTE_H_ */
