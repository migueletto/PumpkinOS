/*
 * $Id: pi-calendar.h,v 1.2 2010-01-16 22:29:35 judd Exp $
 *
 * pi-calendar.h - Support for PalmOne Calendar application (CalendarDB-PDat),
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

#ifndef _PILOT_CALENDAR_H_
#define _PILOT_CALENDAR_H_

#include <time.h>

#include <stdint.h>

#include <pi-appinfo.h>
#include <pi-buffer.h>
#include "pi-location.h"
#include "pi-blob.h"

#ifdef __cplusplus
extern "C" {
#endif
        typedef enum {
                calendar_v1,
        } calendarType;

	enum calendarRepeatType {
		calendarRepeatNone,
		calendarRepeatDaily,
		calendarRepeatWeekly,
		calendarRepeatMonthlyByDay,
		calendarRepeatMonthlyByDate,
		calendarRepeatYearly
	};

	/* This enumeration normally isn't of much use, as you can get just
	   as useful results by taking the value mod 7 to get the day of the
	   week, and div 7 to get the week value, with week 4 (of 0) meaning
	   the last, be it fourth or fifth.
	 */
	enum calendarDayOfMonthType {
		calendar_1stSun, calendar_1stMon, calendar_1stTue, calendar_1stWen, calendar_1stThu,
		calendar_1stFri,
		calendar_1stSat,
		calendar_2ndSun, calendar_2ndMon, calendar_2ndTue, calendar_2ndWen, calendar_2ndThu,
		calendar_2ndFri,
		calendar_2ndSat,
		calendar_3rdSun, calendar_3rdMon, calendar_3rdTue, calendar_3rdWen, calendar_3rdThu,
		calendar_3rdFri,
		calendar_3rdSat,
		calendar_4thSun, calendar_4thMon, calendar_4thTue, calendar_4thWen, calendar_4thThu,
		calendar_4thFri,
		calendar_4thSat,
		calendar_LastSun, calendar_LastMon, calendar_LastTue, calendar_LastWen, calendar_LastThu,
		calendar_LastFri,
		calendar_LastSat
	};

	enum calendarAdvanceTypes { calendar_advMinutes, calendar_advHours, calendar_advDays };

	/**
	 * Times in this structure are assumed to be local unless tz is not null.
	 */
	typedef struct CalendarEvent {
		int event;			/* Is this a timeless event? 				*/
		struct tm begin, end;		/* When does this appointment start and end? 		*/
		int alarm;			/* Should an alarm go off?    				*/
		int advance;			/* How far in advance should it be? 			*/
		int advanceUnits; 		/* What am I measuring the advance in? 			*/
		enum calendarRepeatType repeatType;	/* How should I repeat this appointment, if at all?	*/
		int repeatForever;		/* Do repetitions end at some date?			*/
		struct tm repeatEnd;		/* What date do they end on?  				*/
		int repeatFrequency;		/* Should I skip an interval for each repetition?	*/
		enum calendarDayOfMonthType repeatDay;	/* for repeatMonthlyByDay				*/
		int repeatDays[7];		/* for repeatWeekly 					*/
		int repeatWeekstart;		/* What day did the user decide starts the week?	*/
		int exceptions;			/* How many repetitions are there to be ignored?	*/
		struct tm *exception;		/* What are they?					*/
		char *description;		/* What is the description of this appointment?		*/
		char *note;			/* Is there a note to go along with it?			*/
		char *location;                 /* location of the event */
		Blob_t *blob[MAX_BLOBS];
		Timezone_t *tz;                  /* the timezone information */
          
	} CalendarEvent_t;

	typedef struct CalendarAppInfo {
                calendarType type;
		struct CategoryAppInfo category;
		int startOfWeek;
		uint8_t internal[18];    /* don't know what this is yet */
	} CalendarAppInfo_t;

	extern void new_CalendarEvent
	  PI_ARGS((CalendarEvent_t *event));
	extern void free_CalendarEvent
	  PI_ARGS((CalendarEvent_t *event));
	extern int unpack_CalendarEvent
	    PI_ARGS((CalendarEvent_t *event, const pi_buffer_t *record, calendarType type));
	extern int pack_CalendarEvent
	    PI_ARGS((const CalendarEvent_t *event, pi_buffer_t *record, calendarType type));
	extern int unpack_CalendarAppInfo
	  PI_ARGS((CalendarAppInfo_t *appinfo, pi_buffer_t *buf));
	extern int pack_CalendarAppInfo
	  PI_ARGS((const CalendarAppInfo_t *appinfo, pi_buffer_t *buf));

	extern int copy_CalendarEvent
	PI_ARGS((const CalendarEvent_t *source, CalendarEvent_t *dest));
	
#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_CALENDAR_H_ */

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
