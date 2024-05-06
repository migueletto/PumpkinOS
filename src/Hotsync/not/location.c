/*
 * $Id: location.c,v 1.1 2009/02/22 08:09:01 nicholas Exp $
 *
 * location.c:  Translate Pilot location data formats
 * (c) 2008, Jon Schewe
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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "pi-macros.h"
#include "pi-location.h"

#include "pi-debug.h"
#include "debug.h"

/***********************************************************************
 *
 * Function:    new_Timezone
 *
 * Summary:     Create empty timzone
 *
 * Parameters:  Timezone_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
new_Timezone(Timezone_t *a)
{
	a->offset = 0;
	a->t2 = 0;
	a->dstStart.dayOfWeek = 0;
	a->dstStart.weekOfMonth = 0;
	a->dstStart.month = 0;
	a->dstStart.unknown = 0;
	a->dstEnd.dayOfWeek = 0;
	a->dstEnd.weekOfMonth = 0;
	a->dstEnd.month = 0;
	a->dstEnd.unknown = 0;
	a->dstObserved = 0;
	a->t4 = 0;
	a->unknown = 0;
	a->name = NULL;
}

/***********************************************************************
 *
 * Function:    new_Locatio
 *
 * Summary:     Create empty location
 *
 * Parameters:  Location_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
new_Location(Location_t *a)
{
	new_Timezone(&(a->tz));
	a->unknownExists = 0;
	a->unknown = 0;
	a->latitude.degrees = 0;
	a->latitude.minutes = 0;
	a->latitude.direction = 0;
	a->longitude.degrees = 0;
	a->longitude.minutes = 0;
	a->longitude.direction = 0;
	a->note = NULL;
}

/***********************************************************************
 *
 * Function:    copy_Timezone
 *
 * Summary: Copy the data from one timezone to another. The destination
 * timezone event must already be cleared, either by creating new or by
 * calling free_Timezone on it first.
 *
 * Parameters:  Timezone_t*, Timezone_t*
 *
 * Returns:     int -1 on failure (errno will be set), 0 on success
 *
 ***********************************************************************/
int
copy_Timezone(const Timezone_t *source, Timezone_t *dest)
{

	dest->offset = source->offset;
	dest->t2 = source->t2;
	dest->dstStart.dayOfWeek = source->dstStart.dayOfWeek;
	dest->dstStart.weekOfMonth = source->dstStart.weekOfMonth;
	dest->dstStart.month = source->dstStart.month;
	dest->dstStart.unknown = source->dstStart.unknown;
	dest->dstEnd.dayOfWeek = source->dstEnd.dayOfWeek;
	dest->dstEnd.weekOfMonth = source->dstEnd.weekOfMonth;
	dest->dstEnd.month = source->dstEnd.month;
	dest->dstEnd.unknown = source->dstEnd.unknown;
	dest->dstObserved = source->dstObserved;
	dest->t4 = source->t4;
	dest->unknown = source->unknown;
	if(NULL != source->name) {
		dest->name = strdup(source->name);
	} else {
		dest->name = NULL;
	}
	
	return 0;
}

/***********************************************************************
 *
 * Function:    copy_Location
 *
 * Summary: Copy the data from one location to another. The destination
 * location event must already be cleared, either by creating new or by
 * calling free_Location on it first.
 *
 * Parameters:  Location_t*, Location_t*
 *
 * Returns:     int -1 on failure (errno will be set), 0 on success
 *
 ***********************************************************************/
int
copy_Location(const Location_t *source, Location_t *dest)
{
	int retval;
	retval = copy_Timezone(&(source->tz), &(dest->tz));
	if(0 != retval) {
		return retval;
	}
	dest->unknownExists = source->unknownExists;
	dest->unknown = source->unknown;
	dest->latitude.degrees = source->latitude.degrees;
	dest->latitude.minutes = source->latitude.minutes;
	dest->latitude.direction = source->latitude.direction;
	dest->longitude.degrees = source->longitude.degrees;
	dest->longitude.minutes = source->longitude.minutes;
	dest->longitude.direction = source->longitude.direction;
	if(NULL != source->note) {
		dest->note = strdup(source->note);
	} else {
		dest->note = NULL;
	}
	
	return 0;
}

/***********************************************************************
 *
 * Function:    free_Location
 *
 * Summary:	Free the members of a location structure
 *
 * Parameters:  Location_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Location(Location_t *loc)
{
	free_Timezone(&(loc->tz));
	
	if(loc->note != NULL) {
		free(loc->note);
		loc->note = NULL;
	}

}

/***********************************************************************
 *
 * Function:    dup_Timezone
 *
 * Summary:     Allocate memory for a new timezone that is a duplicate of this one and copy the data into it
 *
 * Parameters:  Timezone_t*
 *
 * Returns:     Timezone_t* or NULL if there isn't enough memory and errno is set to ENOMEM
 *
 ***********************************************************************/
Timezone_t*
dup_Timezone(const Timezone_t *tz)
{
	Timezone_t *retval = (Timezone_t*)malloc(sizeof(Timezone_t));
	if(NULL == retval) {
		errno = ENOMEM;
		return NULL;
	}
	retval->offset = tz->offset;
	retval->t2 = tz->t2;
	memcpy(&(retval->dstStart), &(tz->dstStart), sizeof(DST_t));
	memcpy(&(retval->dstEnd), &(tz->dstEnd), sizeof(DST_t));
	retval->dstObserved = tz->dstObserved;
	retval->t4 = tz->t4;
	retval->unknown = tz->unknown;
	if(NULL != tz->name) {
		retval->name = strdup(tz->name);
	} else {
		retval->name = NULL;
	}
	
	return retval;
}

/***********************************************************************
 *
 * Function:    free_Timezone
 *
 * Summary:	Free the members of a timezone structure
 *
 * Parameters:  Timezone_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Timezone(Timezone_t *tz) {
	if(tz->name != NULL) {
		free(tz->name);
		tz->name = NULL;
	}
}

/***********************************************************************
 *
 * Function:    unpack_DST
 *
 * Summary:     Fill in the daylight savings time structure based on the raw record 
 *		data
 *
 * Parameters:  DST_t*, pi_buffer_t *buf
 *
 * Returns:     -1 on error, 0 on success
 *
 ***********************************************************************/
int
unpack_DST(DST_t *dst, const pi_buffer_t *buf) {
	return unpack_DST_p(dst, buf->data, 0);
}
/**
   Does the work for unpack_DST given a position in buf
*/
int
unpack_DST_p(DST_t *dst, const unsigned char *data, const size_t position) {
	uint8_t byte;
	
	byte = get_byte(data+position);
	switch(byte) {
	case 0x00:
		dst->dayOfWeek = sunday;
		break;
	case 0x01:
		dst->dayOfWeek = monday;
		break;
	case 0x02:
		dst->dayOfWeek = tuesday;
		break;
	case 0x03:
		dst->dayOfWeek = wednesday;
		break;
	case 0x04:
		dst->dayOfWeek = thursday;
		break;
	case 0x05:
		dst->dayOfWeek = friday;
		break;
	case 0x06:
		dst->dayOfWeek = saturday;
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Illegal value found in day of week: 0x%02X", byte);
		return -1;
	}

	byte = get_byte(data+position+1);
	switch(byte) {
	case 0x00:
		dst->weekOfMonth = first;
		break;
	case 0x01:
		dst->weekOfMonth = second;
		break;
	case 0x02:
		dst->weekOfMonth = third;
		break;
	case 0x03:
		dst->weekOfMonth = fourth;
		break;
	case 0x04:
		dst->weekOfMonth = last;
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Illegal value found in week: 0x%02Xd", byte);
		return -1;
	}

	byte = get_byte(data+position+2);
	switch(byte) {
	case 0x00:
		dst->month = none;
		break;
	case 0x01:
		dst->month = january;
		break;
	case 0x02:
		dst->month = february;
		break;
	case 0x03:
		dst->month = march;
		break;
	case 0x04:
		dst->month = april;
		break;
	case 0x05:
		dst->month = may;
		break;
	case 0x06:
		dst->month = june;
		break;
	case 0x07:
		dst->month = july;
		break;
	case 0x08:
		dst->month = august;
		break;
	case 0x09:
		dst->month = september;
		break;
	case 0x0a:
		dst->month = october;
		break;
	case 0x0b:
		dst->month = november;
		break;
	case 0x0c:
		dst->month = december;
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Illegal value found in month: 0x%02Xd", byte);
		return -1;
	}

	dst->unknown = get_byte(data+position+3);
	switch(dst->unknown) {
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		/*case 0x04:*/
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Bad value for DST.unknown: 0x%02x", dst->unknown);
		return -1;
	}
  
	return 0;
}

/***********************************************************************
 *
 * Function:    unpack_timezone
 *
 * Summary:     Fill in the timezone structure based on the raw record 
 *		data
 *
 * Parameters:  Timezone_t*, pi_buffer_t *buf, optional position in buf
 *
 * Returns:     -1 on error, number of bytes read on success
 *
 ***********************************************************************/
int
unpack_Timezone(Timezone_t *tz, const pi_buffer_t *buf) {
	return unpack_Timezone_p(tz, buf->data, 0);
}
/**
 * Does the work of unpack_Timezone given a position in buf.
 * 
 * Returns:     -1 on error, number of bytes read on success
 */
int
unpack_Timezone_p(Timezone_t *tz, const unsigned char *data, const size_t position) {
	uint8_t byte;
	size_t localPosition = position;
  
	tz->offset = get_short(data+localPosition);
	localPosition += 2;
  
	tz->t2 = get_byte(data + localPosition);
	++localPosition;
	switch(tz->t2) {
	case 0x00:
		break;
	case 0x01:
		break;
	case 0x02:
		break;
	case 0x03:
		break;
	default:
		return -1;
	}

	if(unpack_DST_p(&(tz->dstStart), data, localPosition) != 0) {
		return -1;
	}
	localPosition += 4;

	if(unpack_DST_p(&(tz->dstEnd), data, localPosition) != 0) {
		return -1;
	}
	localPosition += 4;
  
	byte = get_byte(data+localPosition);
	++localPosition;
	switch(byte) {
	case 0x3c:
		/* dst observed */
		tz->dstObserved = 1;
		break;
	case 0x00:
		/* dst not observed */
		tz->dstObserved = 0;
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Illegal value in dst_observed 0x%02X", byte);
		return -1;
	}
  
	tz->t4 = get_byte(data+localPosition);
	++localPosition;

	tz->unknown = get_byte(data+localPosition);
	++localPosition;
	switch(tz->unknown) {
	case 0x80:
	case 0x00:
		break;
	default:
		debug(DEBUG_ERROR, SYS_DEBUG, "Bad value for unknown 0x%02X", tz->unknown);
		return -1;
	}

	if(0x00 == data+localPosition) {
		tz->name = NULL;
		++localPosition;
	} else {
		tz->name = strdup((char *)(data+localPosition));
		localPosition += strlen(tz->name) + 1;
	}

	return localPosition;
}

/***********************************************************************
 *
 * Function:    unpack_Location
 *
 * Summary:     Fill in the location structure based on the raw record 
 *		data
 *
 * Parameters:  Location_t*, pi_buffer_t *buf
 *
 * Returns:     -1 on error, 0 on success
 *
 ***********************************************************************/
int
unpack_Location(Location_t *loc, const pi_buffer_t *buf)
{
	size_t localPosition = 0;
  
	localPosition = unpack_Timezone_p(&(loc->tz), buf->data, localPosition);
	if(localPosition < 0) {
		return -1;
	}

	loc->unknownExists = 0;
	  
	/* unpack latitude */
	loc->latitude.degrees = get_short(buf->data+localPosition);
	loc->latitude.minutes = get_short(buf->data+localPosition+2);
	if(loc->latitude.degrees > 90 || loc->latitude.degrees < -90
	   || loc->latitude.minutes > 60 || loc->latitude.minutes < -60) {
		loc->unknownExists = 1;
		loc->unknown = get_byte(buf->data+localPosition);
		++localPosition;

		loc->latitude.degrees = get_short(buf->data+localPosition);
		localPosition += 2;
		loc->latitude.minutes = get_short(buf->data+localPosition);
		localPosition += 2;
	} else {
		localPosition += 4;
	}
		
	loc->longitude.degrees = get_short(buf->data+localPosition);
	localPosition += 2;
	loc->longitude.minutes = get_short(buf->data+localPosition);
	localPosition += 2;

	/* now make latitude and longitude easy to read */
	loc->latitude.direction = south;
	loc->longitude.direction = west;
	if(loc->latitude.minutes < 0) {
		loc->latitude.direction = north;
		loc->latitude.minutes = -1 * loc->latitude.minutes;
	}
	if(loc->latitude.degrees < 0) {
		loc->latitude.direction = north;
		loc->latitude.degrees = -1 * loc->latitude.degrees;
	}
	if(loc->longitude.minutes < 0) {
		loc->longitude.direction = east;
		loc->longitude.minutes = -1 * loc->longitude.minutes;
	}
	if(loc->longitude.degrees < 0) {
		loc->longitude.direction = east;
		loc->longitude.degrees = -1 * loc->longitude.degrees;
	}


	if(0x00 == buf->data[localPosition]) {
		loc->note = NULL;
		++localPosition;
	} else {
		loc->note = strdup((char *)(buf->data+localPosition));
		localPosition += strlen(loc->note) + 1;
	}

	return 0;
}


/***********************************************************************
 *
 * Function:    pack_DST
 *
 * Summary:     append raw DST record data to buf based on the 
 *		DST structure
 *
 * Parameters:  DST_t*, pi_buffer_t *buf of record, record type
 *
 * Returns:     -1 on error, 0 on success.
 *
 ***********************************************************************/
int
pack_DST(const DST_t *dst, pi_buffer_t *buf)
{
	size_t offset;
	
	if (dst == NULL || buf == NULL)
		return -1;

	offset = buf->used;
	
	pi_buffer_expect(buf, buf->used + 4);
	buf->used = buf->used + 4;
	
	switch(dst->dayOfWeek) {
	case sunday:
		set_byte(buf->data+offset, 0x00);
		break;
	case monday:
		set_byte(buf->data+offset, 0x01);
		break;
	case tuesday:
		set_byte(buf->data+offset, 0x02);
		break;
	case wednesday:
		set_byte(buf->data+offset, 0x03);
		break;
	case thursday:
		set_byte(buf->data+offset, 0x04);
		break;
	case friday:
		set_byte(buf->data+offset, 0x05);
		break;
	case saturday:
		set_byte(buf->data+offset, 0x06);
		break;
	default:
		return -1;
		
	}

	switch(dst->weekOfMonth) {
	case first:
		set_byte(buf->data+offset+1, 0x00);
		break;
	case second:
		set_byte(buf->data+offset+1, 0x01);
		break;
	case third:
		set_byte(buf->data+offset+1, 0x02);
		break;
	case fourth:
		set_byte(buf->data+offset+1, 0x03);
		break;
	case last:
		set_byte(buf->data+offset+1, 0x04);
		break;
	default:
		return -1;

	}

	switch(dst->month) {
	case none:
		set_byte(buf->data+offset+2, 0x00);
		break;
	case january:
		set_byte(buf->data+offset+2, 0x01);
		break;
	case february:
		set_byte(buf->data+offset+2, 0x02);
		break;
	case march:
		set_byte(buf->data+offset+2, 0x03);
		break;
	case april:
		set_byte(buf->data+offset+2, 0x04);
		break;
	case may:
		set_byte(buf->data+offset+2, 0x05);
		break;
	case june:
		set_byte(buf->data+offset+2, 0x06);
		break;
	case july:
		set_byte(buf->data+offset+2, 0x07);
		break;
	case august:
		set_byte(buf->data+offset+2, 0x08);
		break;
	case september:
		set_byte(buf->data+offset+2, 0x09);
		break;
	case october:
		set_byte(buf->data+offset+2, 0x0a);
		break;
	case november:
		set_byte(buf->data+offset+2, 0x0b);
		break;
	case december:
		set_byte(buf->data+offset+2, 0x0c);
		break;
	default:
		return -1;
	}

	set_byte(buf->data+offset+3, dst->unknown);
		
	return 0;
}

/***********************************************************************
 *
 * Function:    pack_Timezone
 *
 * Summary:     Append the raw Timezone record to buf based on the 
 *		Timezone structure
 *
 * Parameters:  Timezone_t*, pi_buffer_t *buf of record, record type
 *
 * Returns:     -1 on error, 0 on success.
 *
 ***********************************************************************/
int
pack_Timezone(const Timezone_t *tz, pi_buffer_t *buf)
{

	size_t offset;
	
	if (tz == NULL || buf == NULL)
		return -1;
	
	offset = buf->used;
	pi_buffer_expect(buf, buf->used + 3);
	buf->used = buf->used + 3;

	set_short(buf->data+offset, tz->offset);
	set_byte(buf->data+offset+2, tz->t2);

	pack_DST(&(tz->dstStart), buf);
	pack_DST(&(tz->dstEnd), buf);

	offset = buf->used;
	pi_buffer_expect(buf, buf->used + 3);
	buf->used = buf->used + 3;

	if(tz->dstObserved) {
		set_byte(buf->data+offset, 0x3c);
	} else {
		set_byte(buf->data+offset, 0x00);
	}
	set_byte(buf->data+offset, tz->t4);

	if(NULL != tz->name) {
		offset = buf->used;
		pi_buffer_expect(buf, buf->used + strlen(tz->name)+1);
		buf->used = buf->used + strlen(tz->name)+1;

		strcpy((char *)(buf->data+offset), tz->name);
	}
		
	return 0;
}

/***********************************************************************
 *
 * Function:    pack_Location
 *
 * Summary:     Append the raw Location record to buf based on the 
 *		Location structure
 *
 * Parameters:  Location_t*, pi_buffer_t *buf of record, record type
 *
 * Returns:     -1 on error, 0 on success.
 *
 ***********************************************************************/
int
pack_Location(const Location_t *loc, pi_buffer_t *buf)
{
	size_t offset;
	
	if (loc == NULL || buf == NULL)
		return -1;

	pack_Timezone(&(loc->tz), buf);

	if(loc->unknownExists) {
		offset = buf->used;
		pi_buffer_expect(buf, buf->used + 1);
		buf->used = buf->used+1;
		set_byte(buf->data+offset, loc->unknown);
	}

	offset = buf->used;
	pi_buffer_expect(buf, buf->used+8);
	buf->used = buf->used+8;

	if(loc->latitude.direction == north) {
		set_short(buf->data+offset, -1 * loc->latitude.degrees);
		set_short(buf->data+offset+2, -1 * loc->latitude.minutes);
	} else {
		set_short(buf->data+offset, loc->latitude.degrees);
		set_short(buf->data+offset+2, loc->latitude.minutes);
	}
	if(loc->longitude.direction == east) {
		set_short(buf->data+offset+4, -1 * loc->longitude.degrees);
		set_short(buf->data+offset+6, -1 * loc->longitude.minutes);
	} else {
		set_short(buf->data+offset+4, loc->longitude.degrees);
		set_short(buf->data+offset+6, loc->longitude.minutes);
	}

	if(NULL != loc->note) {
		offset = buf->used;
		pi_buffer_expect(buf, buf->used + strlen(loc->note)+1);
		buf->used = buf->used + strlen(loc->note)+1;

		strcpy((char *)(buf->data+offset), loc->note);
	} else {
		offset = buf->used;
		pi_buffer_expect(buf, buf->used + 1);
		set_byte(buf->data+offset, 0);
		buf->used = buf->used + 1;
	}
		

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
