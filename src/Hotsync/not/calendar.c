/*
 * $Id: calendar.c,v 1.4 2010-01-17 00:38:47 judd Exp $
 *
 * calendar.c - Support for PalmOne Calendar application (CalendarDB-PDat),
 * this is a copy of datebook.c with the calendar fields added.
 * 
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
#include "pi-calendar.h"

#include "pi-debug.h"
#include "debug.h"

#define alarmFlag 	64
#define repeatFlag 	32
#define noteFlag 	16
#define exceptFlag 	8
#define descFlag 	4
#define locFlag 	2

/***********************************************************************
 *
 * Function:    new_CalendarEvent
 *
 * Summary:     Create empty calendar event
 *
 * Parameters:  CalendarEvent_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
new_CalendarEvent(CalendarEvent_t *a)
{
	int i = 0;
	
	a->event = 0;
	a->begin.tm_hour        = 0;
	a->begin.tm_min         = 0;
	a->begin.tm_sec         = 0;
	a->begin.tm_year        = 2000;
	a->begin.tm_mon         = 0;
	a->begin.tm_mday        = 0;
	a->begin.tm_isdst       = -1;
	a->end.tm_hour        = 0;
	a->end.tm_min         = 0;
	a->end.tm_sec         = 0;
	a->end.tm_year        = 2000;
	a->end.tm_mon         = 0;
	a->end.tm_mday        = 0;
	a->end.tm_isdst       = -1;
	a->alarm = 0;
	a->advance = 0;
	a->advanceUnits = 0;
	a->repeatType = 0;
	a->repeatForever = 0;
	a->repeatEnd.tm_hour        = 0;
	a->repeatEnd.tm_min         = 0;
	a->repeatEnd.tm_sec         = 0;
	a->repeatEnd.tm_year        = 2000;
	a->repeatEnd.tm_mon         = 0;
	a->repeatEnd.tm_mday        = 0;
	a->repeatEnd.tm_isdst       = -1;

	a->repeatFrequency = 0;
	a->repeatDay = 0;
	for (i = 0; i < 7; i++) {
		a->repeatDays[i] = 0;
	}
	a->repeatWeekstart = 0;
	a->exceptions = 0;
	a->exception = NULL;
	a->description = NULL;
	a->note = NULL;
	a->location = NULL;
	a->tz = NULL;

	/* initialize the blobs to NULL */
	for (i=0; i<MAX_BLOBS; i++) {
		a->blob[i]=NULL;
	}
}

/***********************************************************************
 *
 * Function:    copy_CalendarEvent
 *
 * Summary: Copy the data from one calendar event to another. The destination
 * calendar event must already be cleared, either by creating new or by
 * calling free_CalendarEvent on it first.
 *
 * Parameters:  CalendarEvent_t*, CalendarEvent_t*
 *
 * Returns:     int -1 on failure (errno will be set), 0 on success
 *
 ***********************************************************************/
int
copy_CalendarEvent(const CalendarEvent_t *source, CalendarEvent_t *dest)
{
	int i = 0;
	
	dest->event = source->event;
	memcpy(&(dest->begin), &(source->begin), sizeof(struct tm));
	memcpy(&(dest->end), &(source->end), sizeof(struct tm));
	dest->alarm = source->alarm;
	dest->advance = source->advance;
	dest->advanceUnits = source->advanceUnits;
	dest->repeatType = source->repeatType;
	dest->repeatForever = source->repeatForever;
	memcpy(&(dest->repeatEnd), &(source->repeatEnd), sizeof(struct tm));

	dest->repeatFrequency = source->repeatFrequency;
	dest->repeatDay = source->repeatDay;
	for (i = 0; i < 7; i++) {
		dest->repeatDays[i] = source->repeatDays[i];
	}
	dest->repeatWeekstart = source->repeatWeekstart;
	dest->exceptions = source->exceptions;
	if(source->exceptions > 0) {
		dest->exception = (struct tm*)malloc(source->exceptions * sizeof(struct tm));
		if(NULL == dest->exception) {
			errno = ENOMEM;
			return -1;
		}
		for(i=0; i<source->exceptions; i++) {
			memcpy(&(dest->exception[i]), &(source->exception[i]), sizeof(struct tm));
		}
	}
	if(NULL != source->description) {
		dest->description = strdup(source->description);
	} else {
		dest->description = NULL;
	}
	if(NULL != source->note) {
		dest->note = strdup(source->note);
	} else {
		dest->note = NULL;
	}
	if(NULL != source->location) {
		dest->location = strdup(source->location);
	} else {
		dest->location = NULL;
	}

	/* copy the blobs */
	for (i=0; i<MAX_BLOBS; i++) {
		if(source->blob[i] != NULL) {
			dest->blob[i] = dup_Blob(source->blob[i]);
			if(NULL == dest->blob[i]) {
				return -1;
			}
		} else {
			dest->blob[i] = NULL;
		}
	}

	if(source->tz != NULL) {
		dest->tz = dup_Timezone(source->tz);
		if(NULL == dest->tz) {
			return -1;
		}
	} else {
		dest->tz = NULL;
	}

	return 0;
}

/***********************************************************************
 *
 * Function:    free_CalendarEvent
 *
 * Summary:     Frees members of the calendar event structure
 *
 * Parameters:  CalendarEvent_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_CalendarEvent(CalendarEvent_t *a)
{
	int i;
	
	if (a->exception != NULL) {
		free(a->exception);
		a->exception = NULL;
	}

	if (a->description != NULL) {
		free(a->description);
		a->description = NULL;
	}

	if (a->note != NULL) {
		free(a->note);
		a->note = NULL;
	}

	if(a->location != NULL) {
		free(a->location);
		a->location = NULL;
	}

	if(NULL != a->tz) {
		free_Timezone(a->tz);
		free(a->tz);
	}

	for(i=0; i<MAX_BLOBS; ++i) {
		if(NULL != a->blob[i]) {
			free_Blob(a->blob[i]);
			
			free(a->blob[i]);
			a->blob[i] = NULL;
		}
	}
}

/***********************************************************************
 *
 * Function:    unpack_CalendarEvent
 *
 * Summary:     Fill in the calendar event structure based on the raw 
 *		record data
 *
 * Parameters:  CalendarEvent_t*, pi_buffer_t * of buffer, calendarType
 *
 * Returns:     -1 on fail, 0 on success
 *
 ***********************************************************************/
int
unpack_CalendarEvent(CalendarEvent_t *a, const pi_buffer_t *buf, calendarType type)
{
	int 	iflags,
		j,
		destlen,
		i,
		result;
	unsigned char *p2;
	unsigned long d;

	destlen = 8;

       if (type != calendar_v1)
                return -1;

	if (buf == NULL || buf->data == NULL || buf->used < destlen) {
		return -1;
	}

	a->begin.tm_hour 	= get_byte(buf->data);
	a->begin.tm_min 	= get_byte(buf->data + 1);
	a->begin.tm_sec 	= 0;
	d = (unsigned short int) get_short(buf->data + 4);
	a->begin.tm_year 	= (d >> 9) + 4;
	a->begin.tm_mon 	= ((d >> 5) & 15) - 1;
	a->begin.tm_mday 	= d & 31;
	a->begin.tm_isdst 	= -1;
	a->end = a->begin;

	a->end.tm_hour = get_byte(buf->data + 2);
	a->end.tm_min = get_byte(buf->data + 3);

	if (get_short(buf->data) == 0xffff) {
		a->event 		= 1;
		a->begin.tm_hour 	= 0;
		a->begin.tm_min 	= 0;
		a->end.tm_hour 		= 0;
		a->end.tm_min 		= 0;
	} else {
		a->event = 0;
	}

	mktime(&a->begin);
	mktime(&a->end);

	iflags = get_byte(buf->data + 6);

	/* buf->data+7 is gapfill */

	p2 = (unsigned char *) buf->data + 8;

	if (iflags & alarmFlag) {
		a->alarm 	= 1;
		a->advance 	= get_byte(p2);
		p2 += 1;
		a->advanceUnits = get_byte(p2);
		p2 += 1;

	} else {
		a->alarm 	= 0;
		a->advance 	= 0;
		a->advanceUnits = 0;
	}

	if (iflags & repeatFlag) {
		int 	i,
			on;

		a->repeatType = (enum calendarRepeatType) get_byte(p2);
		p2 += 2;
		d = (unsigned short int) get_short(p2);
		p2 += 2;
		if (d == 0xffff)
			a->repeatForever = 1;	/* repeatEnd is invalid */
		else {
			a->repeatEnd.tm_year 	= (d >> 9) + 4;
			a->repeatEnd.tm_mon 	= ((d >> 5) & 15) - 1;
			a->repeatEnd.tm_mday 	= d & 31;
			a->repeatEnd.tm_min 	= 0;
			a->repeatEnd.tm_hour 	= 0;
			a->repeatEnd.tm_sec 	= 0;
			a->repeatEnd.tm_isdst 	= -1;
			mktime(&a->repeatEnd);
			a->repeatForever = 0;
		}
		a->repeatFrequency = get_byte(p2);
		p2++;
		on = get_byte(p2);
		p2++;
		a->repeatDay = (enum calendarDayOfMonthType) 0;
		for (i = 0; i < 7; i++)
			a->repeatDays[i] = 0;

		if (a->repeatType == calendarRepeatMonthlyByDay)
			a->repeatDay = (enum calendarDayOfMonthType) on;
		else if (a->repeatType == calendarRepeatWeekly)
			for (i = 0; i < 7; i++)
				a->repeatDays[i] = !!(on & (1 << i));
		a->repeatWeekstart = get_byte(p2);
		p2++;
		p2++;
	} else {
		int 	i;

		a->repeatType 		= (enum calendarRepeatType) 0;
		a->repeatForever 	= 1;	/* repeatEnd is invalid */
		a->repeatFrequency 	= 0;
		a->repeatDay 		= (enum calendarDayOfMonthType) 0;
		for (i = 0; i < 7; i++)
			a->repeatDays[i] = 0;
		a->repeatWeekstart 	= 0;
	}

	if (iflags & exceptFlag) {
		a->exceptions = get_short(p2);
		p2 += 2;
		a->exception = malloc(sizeof(struct tm) * a->exceptions);

		for (j = 0; j < a->exceptions; j++, p2 += 2) {
			d = (unsigned short int) get_short(p2);
			a->exception[j].tm_year 	= (d >> 9) + 4;
			a->exception[j].tm_mon 		= ((d >> 5) & 15) - 1;
			a->exception[j].tm_mday 	= d & 31;
			a->exception[j].tm_hour 	= 0;
			a->exception[j].tm_min 		= 0;
			a->exception[j].tm_sec 		= 0;
			a->exception[j].tm_isdst 	= -1;
			mktime(&a->exception[j]);
		}

	} else {
		a->exceptions 	= 0;
		a->exception 	= 0;
	}

	if (iflags & descFlag) {
		a->description = strdup((char *)p2);
		p2 += strlen((char *)p2) + 1;
	} else
		a->description = 0;

	if (iflags & noteFlag) {
		a->note = strdup((char *)p2);
		p2 += strlen((char *)p2) + 1;
	} else {
		a->note = 0;
	}

	if (iflags & locFlag) {
		a->location = strdup((char *)p2);
		p2 += strlen((char *)p2) + 1;
	} else {
		a->location = 0;
	}

	/* initialize the blobs to NULL */
	for (i=0; i<MAX_BLOBS; ++i) {
		a->blob[i]=NULL;
	}

	if(p2 - buf->data < buf->used) {
		uint8_t blob_count;
		
		/* read the blobs */
		a->tz = NULL;
		for(blob_count = 0; buf->used - (p2 - buf->data) > 6; ++blob_count) {
			if(blob_count >= MAX_BLOBS) {
				/* too many blobs were found */
				debug(DEBUG_ERROR, SYS_DEBUG, "found more than %d blobs: %d", MAX_BLOBS, blob_count);
				return -1;
			}

			a->blob[blob_count] = (Blob_t *)malloc(sizeof(Blob_t));
			result = unpack_Blob_p(a->blob[blob_count], p2, 0);
			if(-1 == result) {
				return -1;
			} else {
				p2 += result;
			}
			
			/* if it's a timezone blob store it */
			if (0 == memcmp(a->blob[blob_count]->type, BLOB_TYPE_CALENDAR_TIMEZONE_ID, 4)) {
				int result;
				if(NULL != a->tz) {
					debug(DEBUG_INFO, SYS_DEBUG, "found more than one timezone blob! Freeing the previous one and starting again");
					free_Timezone(a->tz);
					free(a->tz);
				}
				a->tz = (Timezone_t *)malloc(sizeof(Timezone_t));
				result = unpack_Timezone_p(a->tz, a->blob[blob_count]->data, 0);
				if(-1 == result) {
					debug(DEBUG_ERROR, SYS_DEBUG, "error unpacking timezone blob");
					return -1;
				} else if(result != a->blob[blob_count]->length) {
					debug(DEBUG_ERROR, SYS_DEBUG, "read the wrong number of bytes for a timezone expected %d but was %d", a->blob[blob_count]->length, result);
					return -1;
				}
						
			}
		}
		if(p2 - buf->data < buf->used) {
			debug(DEBUG_ERROR, SYS_DEBUG, "extra data found %ld bytes", (buf->used - (p2 - buf->data)));
			return -1;
		}
	} else {
		a->tz = NULL;
	}
	
	return 0;
}

/***********************************************************************
 *
 * Function:    pack_CalendarEvent
 *
 * Summary:	Fill in the raw calendar event record data based on the 
 *		calendar event structure.
 *
 * Parameters:  CalendarEvent_t*, pi_buffer_t*, calendarType
 *
 * Returns:     -1 on error (bad arguments, mostyle) or 0 on success.
 *              The buffer is sized to accomodate the required data.
 *
 ***********************************************************************/
int
pack_CalendarEvent(const CalendarEvent_t *a, pi_buffer_t *buf, calendarType type)
{
	int 	iflags,
		destlen = 8;
	char 	*pos;

       if (type != calendar_v1)
                return -1;

	if (a == NULL || buf == NULL)
		return -1;

	if (a->alarm)
		destlen += 2;
	if (a->repeatType)
		destlen += 8;
	if (a->exceptions)
		destlen += 2 + 2 * a->exceptions;
	if (a->note)
		destlen += strlen(a->note) + 1;
	if (a->description)
		destlen += strlen(a->description) + 1;
	if (a->location)
		destlen += strlen(a->location) + 1;

	pi_buffer_expect (buf, destlen);
	buf->used = destlen;	

	set_byte(buf->data, a->begin.tm_hour);
	set_byte(buf->data + 1, a->begin.tm_min);
	set_byte(buf->data + 2, a->end.tm_hour);
	set_byte(buf->data + 3, a->end.tm_min);
	set_short(buf->data + 4,
		  ((a->
		    begin.tm_year - 4) << 9) | ((a->begin.tm_mon +
						 1) << 5) | a->begin.
		  tm_mday);

	if (a->event) {
		set_long(buf->data, 0xffffffff);
	}

	iflags = 0;

	pos = (char *) buf->data + 8;

	if (a->alarm) {
		iflags |= alarmFlag;

		set_byte(pos, a->advance);
		set_byte(pos + 1, a->advanceUnits);
		pos += 2;
	}

	if (a->repeatType) {
		int 	i, 
			on;

		iflags |= repeatFlag;

		if (a->repeatType == calendarRepeatMonthlyByDay)
			on = a->repeatDay;
		else if (a->repeatType == calendarRepeatWeekly) {
			on = 0;
			for (i = 0; i < 7; i++)
				if (a->repeatDays[i])
					on |= 1 << i;
		} else
			on = 0;

		set_byte(pos, a->repeatType);
		set_byte(pos + 1, 0);
		pos += 2;

		if (a->repeatForever)
			set_short(pos, 0xffff);
		else
			set_short(pos,
				  ((a->
				    repeatEnd.tm_year -
				    4) << 9) | ((a->repeatEnd.tm_mon +
						 1) << 5) | a->repeatEnd.
				  tm_mday);

		pos += 2;

		set_byte(pos, a->repeatFrequency);
		pos++;
		set_byte(pos, on);
		pos++;
		set_byte(pos, a->repeatWeekstart);
		pos++;
		set_byte(pos, 0);
		pos++;
	}

	if (a->exceptions) {
		int 	i;

		iflags |= exceptFlag;

		set_short(pos, a->exceptions);
		pos += 2;

		for (i = 0; i < a->exceptions; i++, pos += 2)
			set_short(pos,
				  ((a->
				    exception[i].tm_year -
				    4) << 9) | ((a->exception[i].tm_mon +
						 1) << 5) | a->
				  exception[i].tm_mday);
	}

	if (a->description != NULL) {
		iflags |= descFlag;

		strcpy(pos, a->description);
		pos += strlen(pos) + 1;
	}

	if (a->note != NULL) {
		iflags |= noteFlag;

		strcpy(pos, a->note);
		pos += strlen(pos) + 1;
	}

	if (a->location != NULL) {
		iflags |= locFlag;

		strcpy(pos, a->location);
		pos += strlen(pos) + 1;
	}

	set_byte(buf->data + 6, iflags);
	set_byte(buf->data + 7, 0);	/* gapfill */

	/* Calendar stuff */
	uint8_t blob_index;

	//write out the blobs
	for(blob_index = 0; blob_index < MAX_BLOBS; ++blob_index) {
		if(NULL != a->blob[blob_index]) {
			pack_Blob(a->blob[blob_index], buf);
		}
	}
	
	return 0;
}

/***********************************************************************
 *
 * Function:    unpack_CalendarAppInfo
 *
 * Summary:     Fill in the app info structure based on the raw app 
 *		info data
 *
 * Parameters:  CalendarAppInfo_t*, char* to record, record length
 *
 * Returns:     0 on error, the length of the data used from the 
 *		buffer otherwise
 *
 ***********************************************************************/
int
unpack_CalendarAppInfo(CalendarAppInfo_t *ai, pi_buffer_t *buf)
{
	int 		i;
	int 		len;
	unsigned char 	*record;
	int		used;

	len = buf->used;
	record = buf->data;
	used = unpack_CategoryAppInfo(&ai->category, record, len);
	if (!used)
		return 0;
	record += used;
	len -= used;
	if (len < 2)
		return 0;
	ai->startOfWeek = get_byte(record);
	// alignment byte
	record += 2;
	used += 2;

	for(i=0; i<18; ++i) {
		ai->internal[i] = get_byte(record);
		record++;
		used++;
	}
	ai->type = calendar_v1;

	return used;
}

/***********************************************************************
 *
 * Function:    pack_CalendarAppInfo
 *
 * Summary:     Fill in the raw app info record data based on the app 
 *		info structure
 *
 * Parameters:  AppointmentAppInfo*, char* to buffer, buffer length
 *
 * Returns:     The length of the buffer required if record is NULL,
 *		or 0 on error, the length of the data used from the 
 *		buffer otherwise
 *
 ***********************************************************************/
int
pack_CalendarAppInfo(const CalendarAppInfo_t *ai, pi_buffer_t *buf)
{
	int 	i;
	int 	len;
	unsigned char *record;

	if (!buf) {
		return 298;
	}

	/* AppInfo size should be 298, 300 will do */
	len = 300;
	pi_buffer_expect(buf, 300);
	buf->used = pack_CategoryAppInfo(&ai->category, buf->data, buf->allocated);
	if (!buf->used)
		return 0;
	record 	= buf->data + buf->used;
	len 	-= buf->used;
	if (len < 2)
		return 0;
	set_short(record, 0);
	set_byte(record, ai->startOfWeek);
	record += 2;
	buf->used += 2;

	for(i=0; i<18; ++i) {
		set_byte(record, ai->internal[i]);
		record ++;
		buf->used ++;
	}
		    
	return (record - buf->data);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
