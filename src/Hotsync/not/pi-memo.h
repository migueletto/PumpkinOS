/*
 * $Id: pi-memo.h,v 1.16 2006/11/22 22:52:25 adridg Exp $
 *
 * pi-memo.h: Support for the Palm Memos application
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

#ifndef _PILOT_MEMO_H_		/* -*- C++ -*- */
#define _PILOT_MEMO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-appinfo.h"
#include "pi-buffer.h"

	typedef enum {
		memo_v1,
	} memoType;
  
	typedef struct Memo {
		char *text;
	} Memo_t;

	typedef struct MemoAppInfo {
		memoType type;
		struct CategoryAppInfo category;
		/* New for 2.0 memo application, 0 is manual, 1 is
		   alphabetical.
		 */
		int sortByAlpha;	

	} MemoAppInfo_t;

	extern void free_Memo PI_ARGS((struct Memo *));
	extern int unpack_Memo
	    PI_ARGS((struct Memo *, const pi_buffer_t *record, memoType type));
	extern int pack_Memo
	    PI_ARGS((const struct Memo *, pi_buffer_t *record, memoType type));
	extern int unpack_MemoAppInfo
	    PI_ARGS((struct MemoAppInfo *, const unsigned char *AppInfo,
		     size_t len));
	extern int pack_MemoAppInfo
	    PI_ARGS((const struct MemoAppInfo *, unsigned char *AppInfo,
		     size_t len));

#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_MEMO_H_ */
