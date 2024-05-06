/*
 * $Id: pi-appinfo.h,v 1.15 2006/11/22 22:52:25 adridg Exp $
 *
 * pi-appinfo.h: AppInfo header block definitions
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
  
#ifndef _PILOT_APPINFO_H_	/* -*- C++ -*- */
#define _PILOT_APPINFO_H_

#include "pi-args.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct CategoryAppInfo {
		unsigned int renamed[16]; 	/* Boolean array of categories with changed names */
		char name[16][16];		/* 16 categories of 15 characters+nul each */
		unsigned char ID[16];

		/* Each category gets a unique ID, for sync tracking
		   purposes. Those from the Palm are between 0 & 127. Those
		   from the PC are between 128 & 255. I'm not sure what role
		   lastUniqueID plays. */
		unsigned char lastUniqueID;	
	} CategoryAppInfo_t;

	extern int unpack_CategoryAppInfo
	    PI_ARGS((CategoryAppInfo_t *, const unsigned char *AppInfo, size_t len));
	extern int pack_CategoryAppInfo
	    PI_ARGS((const CategoryAppInfo_t *, unsigned char *AppInfo, size_t len));

#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_APPINFO_H_ */
