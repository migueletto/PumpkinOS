/*
 * $Id: memo.c,v 1.27 2006/11/22 22:52:25 adridg Exp $
 *
 * memo.c:  Translate Pilot memopad data formats
 *
 * Copyright (c) 1996, Kenneth Albanowski
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
#include "pi-memo.h"

/***********************************************************************
 *
 * Function:    free_Memo
 *
 * Summary:     Frees all record data associated with the Memo database
 *
 * Parameters:  Memo_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void 
free_Memo(Memo_t *memo)
{
	if (memo->text != NULL) {
		free(memo->text);
		memo->text = NULL;
	}
}


/***********************************************************************
 *
 * Function:    unpack_Memo
 *
 * Summary:     Unpack the memo structure from the buffer
 *
 * Parameters:  Memo_t*, char* to buffer, length
 *
 * Returns:     Length in bytes of the buffer allocated
 *
 ***********************************************************************/
int
unpack_Memo(Memo_t *memo, const pi_buffer_t *record, memoType type)
{
	if (type != memo_v1)
		/* Don't support anything else yet */
		return -1;
	if (record == NULL || record->data == NULL || record->used < 1)
		return -1;
	memo->text = strdup((char *) record->data);
	return 0;
}


/***********************************************************************
 *
 * Function:    pack_Memo
 *
 * Summary:     Pack the memo structure into the buffer allocated
 *
 * Parameters:  Memo_t*, char* to buffer, length of buffer
 *
 * Returns:     buffer length
 *
 ***********************************************************************/
int
pack_Memo(const Memo_t *memo, pi_buffer_t *record, memoType type)
{
	size_t destlen = (memo->text ? strlen(memo->text) : 0) + 1;

	if (type != memo_v1)
		/* Don't support anything else yet */
		return -1;
	if (record == NULL)
		return -1;

	pi_buffer_expect(record, destlen);
	record->used = destlen;

	if (memo->text)
		strcpy((char *) record->data, memo->text);
	else
		record->data[0] = 0;

	return 0;
}


/***********************************************************************
 *
 * Function:    unpack_MemoAppInfo
 *
 * Summary:     Unpack the memo AppInfo block structure
 *
 * Parameters:  MemoAppInfo_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
unpack_MemoAppInfo(struct MemoAppInfo *appinfo, const unsigned char *record,
			 size_t len)
{
	int 	i = unpack_CategoryAppInfo(&appinfo->category, record, len);
	unsigned char *start = (unsigned char *)record;

	appinfo->type = memo_v1;

	if (!i)
		return i;
	record += i;
	len -= i;
	if (len >= 4) {
		record += 2;
		appinfo->sortByAlpha = get_byte(record);
		record += 2;
	} else {
		appinfo->sortByAlpha = 0;
	}
	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_MemoAppInfo
 *
 * Summary:     Pack the memo AppInfo block structure
 *
 * Parameters:  MemoAppInfo_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
pack_MemoAppInfo(const MemoAppInfo_t *appinfo, unsigned char *record, size_t len)
{
	int 	i;
	unsigned char *start = record;
	
	i = pack_CategoryAppInfo(&appinfo->category, record, len);
	if (!record)
		return i + 4;
	if (i == 0)				/* category pack failed */
		return 0;
	record += i;
	len -= i;
	if (len < 4)
		return (record - start);
	set_short(record, 0);			/* gapfill new for 2.0 	*/
	record += 2;
	set_byte(record, appinfo->sortByAlpha);	/* new for 2.0 		*/
	record++;
	set_byte(record, 0);			/* gapfill new for 2.0 	*/
	record++;
	
	return (record - start);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */

