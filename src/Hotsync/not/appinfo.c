/*
 * $Id: appinfo.c,v 1.19 2006/11/22 22:52:25 adridg Exp $
 *
 * appinfo.c:  Translate Pilot category info
 *
 * Copyright (c) 1996, 1997, Kenneth Albanowski
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

#include "pi-macros.h"
#include "pi-appinfo.h"

/***********************************************************************
 *
 * Function:    unpack_CategoryAppInfo
 *
 * Summary:     Unpack the AppInfo block from record into the structure
 *
 * Parameters:  CategoryAppInfo_t*, char* to record, record length
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
int
unpack_CategoryAppInfo(CategoryAppInfo_t *ai, const unsigned char *record, size_t len)
{
	int 	i,
		rec;

	if (len < 2 + 16 * 16 + 16 + 4)
		return 0;
	rec = get_short(record);
	for (i = 0; i < 16; i++) {
		if (rec & (1 << i))
			ai->renamed[i] = 1;
		else
			ai->renamed[i] = 0;
	}
	record += 2;
	for (i = 0; i < 16; i++) {
		memcpy(ai->name[i], record, 16);
		record += 16;
	}
	memcpy(ai->ID, record, 16);
	record += 16;
	ai->lastUniqueID = get_byte(record);
	record += 4;
	return 2 + 16 * 16 + 16 + 4;
}

/***********************************************************************
 *
 * Function:    pack_CategoryAppInfo
 *
 * Summary:     Pack the AppInfo structure 
 *
 * Parameters:  CategoryAppInfo_t*, char* to record, record length
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
int
pack_CategoryAppInfo(const CategoryAppInfo_t *ai, unsigned char *record, size_t len)
{
	int 	i,
		rec;
	
	unsigned char *start = record;

	if (!record) {
		return 2 + 16 * 16 + 16 + 4;
	}
	if (len < (2 + 16 * 16 + 16 + 4))
		return 0;	/* not enough room */
	rec = 0;
	for (i = 0; i < 16; i++) {
		if (ai->renamed[i])
			rec |= (1 << i);
	}
	set_short(record, rec);
	record += 2;
	for (i = 0; i < 16; i++) {
		memcpy(record, ai->name[i], 16);
		record += 16;
	}
	memcpy(record, ai->ID, 16);
	record += 16;
	set_byte(record, ai->lastUniqueID);
	record++;
	set_byte(record, 0);		/* gapfill */
	set_short(record + 1, 0);	/* gapfill */
	record += 3;

	return (record - start);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
