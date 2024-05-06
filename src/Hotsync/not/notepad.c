/*
 * $Id: notepad.c,v 1.14 2006/10/12 14:21:22 desrod Exp $
 *
 * notepad.c:  Translate Palm NotePad application data formats
 *
 * Copyright (c) 2002, Angus Ainslie
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
#include "pi-notepad.h"


/***********************************************************************
 *
 * Function:    free_NotePad
 *
 * Summary:     Free the memory and filehandle from the record alloc. 
 *
 ***********************************************************************/
void free_NotePad( NotePad_t *a )
{
   if( a->flags & NOTEPAD_FLAG_NAME )
     {
	free(a->name);
     }
   
   if( a->flags & NOTEPAD_FLAG_BODY )
     {
	free(a->data);
     }
   
}


/***********************************************************************
 *
 * Function:    unpack_NotePad
 *
 * Summary:     Unpack the NotePad structure into records we can chew on
 *
 ***********************************************************************/
int unpack_NotePad(NotePad_t *notepad, unsigned char *buffer, size_t len)
{
   unsigned char *start = buffer;
   
   notepad->createDate.sec = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->createDate.min = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->createDate.hour = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->createDate.day = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->createDate.month = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->createDate.year = (unsigned short int) get_short(buffer);
   buffer += 2;

   notepad->createDate.s = (unsigned short int) get_short(buffer);
   buffer += 2;

   notepad->changeDate.sec = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->changeDate.min = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->changeDate.hour = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->changeDate.day = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->changeDate.month = (unsigned short int) get_short(buffer);
   buffer += 2;
   notepad->changeDate.year = (unsigned short int) get_short(buffer);
   buffer += 2;

   notepad->changeDate.s = (unsigned short int) get_short(buffer);
   buffer += 2;

   notepad->flags = (unsigned short int) get_short(buffer);
   buffer += 2;

   
   if( notepad->flags & NOTEPAD_FLAG_ALARM )
     {
	notepad->alarmDate.sec = (unsigned short int) get_short(buffer);
	buffer += 2;
	notepad->alarmDate.min = (unsigned short int) get_short(buffer);
	buffer += 2;
	notepad->alarmDate.hour = (unsigned short int) get_short(buffer);
	buffer += 2;
	notepad->alarmDate.day = (unsigned short int) get_short(buffer);
	buffer += 2;
	notepad->alarmDate.month = (unsigned short int) get_short(buffer);
	buffer += 2;
	notepad->alarmDate.year = (unsigned short int) get_short(buffer);
	buffer += 2;

	notepad->alarmDate.s = (unsigned short int) get_short(buffer);
	buffer += 2;
     }
  
   if( notepad->flags & NOTEPAD_FLAG_NAME )
     {
	notepad->name = strdup((char *) buffer);
   
	buffer += strlen( notepad->name ) + 1;
	
	if( (strlen( notepad->name ) + 1)%2 == 1)
	  buffer++;
	
     }
   else 
     {
	notepad->name = NULL;
     }
   

   if( notepad->flags & NOTEPAD_FLAG_BODY )
     {
	notepad->body.bodyLen = get_long( buffer );
	buffer += 4;
   
	notepad->body.width = get_long( buffer );
	buffer += 4;
   
	notepad->body.height = get_long( buffer );
	buffer += 4;
   
	notepad->body.l1 = get_long( buffer );
	buffer += 4;
   
	notepad->body.dataType = get_long( buffer );
	buffer += 4;

	notepad->body.dataLen = get_long( buffer );
	buffer += 4;
   
	notepad->data = malloc( notepad->body.dataLen );

	if( notepad->data == NULL )
	  {
	     return( 0 );
	  }
	     
	memcpy( notepad->data, buffer, notepad->body.dataLen );

     }
   
   return ( buffer - start );	/* FIXME: return real length */
}


/***********************************************************************
 *
 * Function:    pack_NotePad
 *
 * Summary:     Pack the NotePad records into a structure
 *
 ***********************************************************************/
int pack_NotePad(NotePad_t *notepad, unsigned char *buf, size_t len)
{
   return( 0 );
}


/***********************************************************************
 *
 * Function:    unpack_NotePadAppInfo
 *
 * Summary:     Unpack the NotePad AppInfo block from the structure
 *
 ***********************************************************************/
int unpack_NotePadAppInfo(NotePadAppInfo_t *appinfo, unsigned char *record,
			 size_t len)
{
	int 	i;
	unsigned char *start = record;

	i = unpack_CategoryAppInfo( &appinfo->category, record, len );
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
 * Function:    pack_NotePadAppInfo
 *
 * Summary:     Pack the AppInfo block/record back into the structure
 *
 ***********************************************************************/
int
pack_NotePadAppInfo(NotePadAppInfo_t *appinfo, unsigned char *record,
		 	size_t len)
{
	int 	i;
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

