/*
 * $Id: todo.c,v 1.28 2006/11/22 22:52:25 adridg Exp $
 *
 * todo.c:  Translate Palm ToDo application data formats
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

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "pi-macros.h"
#include "pi-todo.h"

/* Maximum length of Description and Note fields */
#define DescMaxLength 256
#define NoteMaxLength 4096


/***********************************************************************
 *
 * Function:    free_ToDo
 *
 * Summary:     Free the memory and filehandle from the record alloc. 
 *
 * Parameters:  ToDo_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_ToDo(ToDo_t *todo)
{

	if (todo->description != NULL) {
		free(todo->description);
		todo->description = NULL;
	}

	if (todo->note != NULL) {
		free(todo->note);
		todo->note = NULL;
	}
}


/***********************************************************************
 *
 * Function:    unpack_ToDo
 *
 * Summary:     Unpack the ToDo structure from buffer into records 
 *              we can chew on.
 *
 * Parameters:  ToDo_t*, pi_buffer_t * of buffer, todo type
 *
 * Returns:     -1 on fail, 0 on success
 *
 ***********************************************************************/
int
unpack_ToDo(ToDo_t *todo, const pi_buffer_t *buf, todoType type)
{
	unsigned long d;
	int ofs;

	/* Note: There are possible timezone conversion problems related to
	   the use of the due member of a struct ToDo. As it is kept in
	   local (wall) time in struct tm's, the timezone of the Palm is
	   irrelevant, _assuming_ that any UNIX program keeping time in
	   time_t's converts them to the correct local time. If the Palm is
	   in a different timezone than the UNIX box, it may not be simple
	   to deduce that correct (desired) timezone.

	   The easiest solution is to keep apointments in struct tm's, and
	   out of time_t's. Of course, this might not actually be a help if
	   you are constantly darting across timezones and trying to keep
	   appointments.
	   -- KJA */

	if (type != todo_v1)
		return -1;

	if (buf == NULL || buf->data == NULL || buf->used < 3)
		return -1;

	d = (unsigned short int) get_short(buf->data);
	if (d != 0xffff) {
		todo->due.tm_year = (d >> 9) + 4;
		todo->due.tm_mon = ((d >> 5) & 15) - 1;
		todo->due.tm_mday = d & 31;
		todo->due.tm_hour = 0;
		todo->due.tm_min = 0;
		todo->due.tm_sec = 0;
		todo->due.tm_isdst = -1;
		mktime(&todo->due);
		todo->indefinite = 0;
	} else {
		todo->indefinite = 1;	/* todo->due is invalid */
	}

	todo->priority = get_byte(buf->data + 2);
	if (todo->priority & 0x80) {
		todo->complete = 1;
		todo->priority &= 0x7f;
	} else {
		todo->complete = 0;
	}

	ofs = 3;

	if (buf->used - ofs < 1)
		return -1;

	todo->description = strdup((char *) buf->data + ofs);

	ofs += strlen(todo->description) + 1;

	if (buf->used - ofs < 1) {
		free(todo->description);
		todo->description = 0;
		return -1;
	}
	todo->note = strdup((char *) buf->data + ofs);

	return 0;
}


/***********************************************************************
 *
 * Function:    pack_ToDo
 *
 * Summary:     Pack the ToDo records into a structure
 *
 * Parameters:  ToDo_t*, pi_buffer_t *buf of record, record type
 *
 * Returns:     -1 on error, 0 on success.
 *
 ***********************************************************************/
int
pack_ToDo(const ToDo_t *todo, pi_buffer_t *buf, todoType type)
{
	int pos;
	size_t destlen = 3;

	if (todo == NULL || buf == NULL)
		return -1;

	if (type != todo_v1)
		return -1;

	if (todo->description)
		destlen += strlen(todo->description);
	destlen++;
	if (todo->note)
		destlen += strlen(todo->note);
	destlen++;

	pi_buffer_expect (buf, destlen);
	buf->used = destlen;	

	if (todo->indefinite) {
		buf->data[0] = 0xff;
		buf->data[1] = 0xff;
	} else {
		set_short(buf->data,
			  ((todo->due.tm_year - 4) << 9) | ((todo->due.tm_mon +
							  1) << 5) | todo->
			  due.tm_mday);
	}
	buf->data[2] = todo->priority;
	if (todo->complete) {
		buf->data[2] |= 0x80;
	}

	pos = 3;
	if (todo->description) {
		strcpy((char *) buf->data + pos, todo->description);
		pos += strlen(todo->description) + 1;
	} else {
		buf->data[pos++] = 0;
	}

	if (todo->note) {
		strcpy((char *) buf->data + pos, todo->note);
		pos += strlen(todo->note) + 1;
	} else {
		buf->data[pos++] = 0;
	}

	return 0;
}


/***********************************************************************
 *
 * Function:    unpack_ToDoAppInfo
 *
 * Summary:     Unpack the ToDo AppInfo block from the structure
 *
 * Parameters:  ToDoAppInfo_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
unpack_ToDoAppInfo(ToDoAppInfo_t *appinfo, const unsigned char *record, size_t len)
{
	int i;
	unsigned char *start = (unsigned char *)record;

	appinfo->type = todo_v1;

	i = unpack_CategoryAppInfo(&appinfo->category, record, len);
	if (!i)
		return 0;
	record += i;
	len -= i;
	if (len < 4)
		return 0;
	appinfo->dirty = get_short(record);
	record += 2;
	appinfo->sortByPriority = get_byte(record);
	record += 2;
	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_ToDoAppInfo
 *
 * Summary:     Pack the AppInfo block/record back into the structure
 *
 * Parameters:  ToDoAppInfo_t*, char* to record, record length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_ToDoAppInfo(const ToDoAppInfo_t *appinfo, unsigned char *record, size_t len)
{
	int i;
	unsigned char *start = record;

	i = pack_CategoryAppInfo(&appinfo->category, record, len);
	if (!record)
		return i + 4;
	if (!i)
		return 0;
	record += i;
	len -= i;
	if (len < 4)
		return 0;
	set_short(record, appinfo->dirty);
	set_byte(record + 2, appinfo->sortByPriority);
	set_byte(record + 3, 0);	/* gapfill */
	record += 4;

	return (record - start);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
