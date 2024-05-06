/*
 * $Id: hinote.c,v 1.22 2006/10/12 14:21:22 desrod Exp $
 *
 * hinote.c:  Translate Hi-Note data formats
 *
 * Copyright 1997 Bill Goodman
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

#include "pi-hinote.h"

/***********************************************************************
 *
 * Function:    free_HiNoteNote
 *
 * Summary:     frees HiNoteNote_t members
 *
 * Parameters:  HiNoteNote_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_HiNoteNote(HiNoteNote_t *hinote)
{
	if (hinote->text != NULL) {
		free(hinote->text);
		hinote->text = NULL;
	}
}

/***********************************************************************
 *
 * Function:    unpack_HiNoteNote
 *
 * Summary:     Unpack a HiNote record
 *
 * Parameters:  HiNoteNote_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_HiNoteNote(HiNoteNote_t *hinote, unsigned char *buffer, int len)
{
	if (len < 3)
		return 0;

	hinote->flags 	= buffer[0];
	hinote->level 	= buffer[1];
	hinote->text 	= strdup((char *) &buffer[2]);

	return strlen((char *) &buffer[2]) + 3;
}

/***********************************************************************
 *
 * Function:    pack_HiNoteNote
 *
 * Summary:     Pack a HiNote record
 *
 * Parameters:  HiNoteNote_t*
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
int
pack_HiNoteNote(HiNoteNote_t *hinote, unsigned char *buffer, int len)
{
	int 	destlen;

	destlen = 3;
	if (hinote->text)
		destlen += strlen(hinote->text);

	if (!buffer)
		return destlen;
	if (len < destlen)
		return 0;

	buffer[0] = hinote->flags;
	buffer[1] = hinote->level;

	if (hinote->text)
		strcpy((char *) &buffer[2], hinote->text);
	else {
		buffer[2] = 0;
	}
	return destlen;
}


/***********************************************************************
 *
 * Function:    unpack_HiNoteAppInfo
 *
 * Summary:     Unpack the HiNote AppInfo block
 *
 * Parameters:  HiNoteAppInfo_t*, char* to record, record length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_HiNoteAppInfo(HiNoteAppInfo_t *appinfo, unsigned char *record, size_t len)
{
	int i;
	unsigned char *start;

	start = record;
	i = unpack_CategoryAppInfo(&appinfo->category, record, len);
	if (!i)
		return i;
	record += i;
	len -= i;
	if (len < 48)
		return 0;
	for (; i < 48; i++)
		appinfo->reserved[i] = *record++;
	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_HiNoteAppInfo
 *
 * Summary:     Pack the HiNote AppInfo block
 *
 * Parameters:  HiNoteAppInfo_t*, char* record, length of record
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
pack_HiNoteAppInfo(HiNoteAppInfo_t *appinfo, unsigned char *record, size_t len)
{
	int i;
	unsigned char *start = record;

	i = pack_CategoryAppInfo(&appinfo->category, record, len);
	if (i == 0)		/* category pack failed */
		return 0;
	if (!record)
		return i + 48;
	record += i;
	len -= i;
	if (len < 48)
		return (record - start);
	for (; i < 48; i++)
		*record++ = appinfo->reserved[i];

	return (record - start);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
