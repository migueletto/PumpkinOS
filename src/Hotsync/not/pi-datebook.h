/*
 * $Id: pi-datebook.h,v 1.23 2006/11/22 22:52:25 adridg Exp $
 *
 * pi-datebook.h - Support for Palm "Classic" Datebook application
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

#ifndef _PILOT_DATEBOOK_H_
#define _PILOT_DATEBOOK_H_

#include <time.h>
#include "pi-appinfo.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		datebook_v1,
	} datebookType;

	extern char *DatebookAlarmTypeNames[];
	extern char *DatebookRepeatTypeNames[];

	enum alarmTypes { advMinutes, advHours, advDays };

	enum repeatTypes {
		repeatNone,
		repeatDaily,
		repeatWeekly,
		repeatMonthlyByDay,
		repeatMonthlyByDate,
		repeatYearly
	};

	/* This enumeration normally isn't of much use, as you can get just
	   as useful results by taking the value mod 7 to get the day of the
	   week, and div 7 to get the week value, with week 4 (of 0) meaning
	   the last, be it fourth or fifth.
	 */
	enum DayOfMonthType {
		dom1stSun, dom1stMon, dom1stTue, dom1stWen, dom1stThu,
		dom1stFri,
		dom1stSat,
		dom2ndSun, dom2ndMon, dom2ndTue, dom2ndWen, dom2ndThu,
		dom2ndFri,
		dom2ndSat,
		dom3rdSun, dom3rdMon, dom3rdTue, dom3rdWen, dom3rdThu,
		dom3rdFri,
		dom3rdSat,
		dom4thSun, dom4thMon, dom4thTue, dom4thWen, dom4thThu,
		dom4thFri,
		dom4thSat,
		domLastSun, domLastMon, domLastTue, domLastWen, domLastThu,
		domLastFri,
		domLastSat
	};

	typedef struct Appointment {
		int event;			/* Is this a timeless event? 				*/
		struct tm begin, end;		/* When does this appointment start and end? 		*/
		int alarm;			/* Should an alarm go off?    				*/
		int advance;			/* How far in advance should it be? 			*/
		int advanceUnits; 		/* What am I measuring the advance in? 			*/
		enum repeatTypes repeatType;	/* How should I repeat this appointment, if at all?	*/
		int repeatForever;		/* Do repetitions end at some date?			*/
		struct tm repeatEnd;		/* What date do they end on?  				*/
		int repeatFrequency;		/* Should I skip an interval for each repetition?	*/
		enum DayOfMonthType repeatDay;	/* for repeatMonthlyByDay				*/
		int repeatDays[7];		/* for repeatWeekly 					*/
		int repeatWeekstart;		/* What day did the user decide starts the week?	*/
		int exceptions;			/* How many repetitions are their to be ignored?	*/
		struct tm *exception;		/* What are they?					*/
		char *description;		/* What is the description of this appointment?		*/
		char *note;			/* Is there a note to go along with it?			*/
	} Appointment_t;

	typedef struct AppointmentAppInfo {
		struct CategoryAppInfo category;
		int startOfWeek;
	} AppointmentAppInfo_t;

	extern void free_Appointment
	  PI_ARGS((struct Appointment *));
	extern int unpack_Appointment
	    PI_ARGS((struct Appointment *, const pi_buffer_t *record, datebookType type));
	extern int pack_Appointment
	    PI_ARGS((const struct Appointment *, pi_buffer_t *record, datebookType type));
	extern int unpack_AppointmentAppInfo
	  PI_ARGS((struct AppointmentAppInfo *, const unsigned char *AppInfo,
		     size_t len));
	extern int pack_AppointmentAppInfo
	  PI_ARGS((const struct AppointmentAppInfo *, unsigned char *AppInfo,
		     size_t len));

#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_DATEBOOK_H_ */
