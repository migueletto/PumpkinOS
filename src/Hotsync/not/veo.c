/*
 * $Id: veo.c,v 1.7 2006/10/12 14:21:23 desrod Exp $
 *
 * veo.c:  Translate veo traveler data formats
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

#include "pi-macros.h"
#include "pi-veo.h"

#include "pi-debug.h"
#include "debug.h"

/***********************************************************************
 *
 * Function:    free_Veo
 *
 * Summary:     Free the memory and filehandle from the record alloc. 
 *
 ***********************************************************************/
void
free_Veo( Veo_t *veo )
{
}

/***********************************************************************
 *
 * Function:    unpack_Veo
 *
 * Summary:     Unpack the Veo structure into records we can chew on
 *
 ***********************************************************************/
int
unpack_Veo(Veo_t *veo, unsigned char *buffer, size_t len)
{
   unsigned char *start = buffer;
   
   /* consume unknown */
   buffer += 1;
   veo->quality = (unsigned char) get_byte(buffer);
   buffer += 1;
   veo->resolution = (unsigned char) get_byte(buffer);
   buffer += 1;
   /* consume 12 more unknowns */
   buffer += 12;
   veo->picnum = (unsigned long int) get_long(buffer);
   buffer += 4;
   veo->day = (unsigned short int) get_short(buffer);
   buffer += 2;
   veo->month = (unsigned short int) get_short(buffer);
   buffer += 2;
   veo->year = (unsigned short int) get_short(buffer);
   buffer += 2;

   if( veo->resolution == 0 )
     {
	veo->width = 640;
	veo->height = 480;
     }
   else if( veo->resolution == 1 )
     {
	veo->width = 320;
	veo->height = 240;
     }
   else 
     debug(DEBUG_ERROR, SYS_DEBUG, "veo.c: unknown resolution" );
	
   return ( buffer - start );	/* FIXME: return real length */
}


/***********************************************************************
 *
 * Function:    pack_Veo
 *
 * Summary:     Pack the Veo records into a structure
 *
 ***********************************************************************/
int pack_Veo(Veo_t *veo, unsigned char *buf, size_t len)
{
   return( 0 );
}


/***********************************************************************
 *
 * Function:    unpack_VeoAppInfo
 *
 * Summary:     Unpack the Veo AppInfo block from the structure
 *
 ***********************************************************************/
int unpack_VeoAppInfo(struct VeoAppInfo *appinfo, unsigned char *record,
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
 * Function:    pack_VeoAppInfo
 *
 * Summary:     Pack the AppInfo block/record back into the structure
 *
 ***********************************************************************/
int
pack_VeoAppInfo(struct VeoAppInfo *appinfo, unsigned char *record, size_t len)
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

