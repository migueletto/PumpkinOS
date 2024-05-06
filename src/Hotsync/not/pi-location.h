/*
 * $Id: pi-location.h,v 1.1 2009/02/22 08:08:59 nicholas Exp $
 *
 * pi-location.h: Support for Palm Location databases (locLCusLocationDB, locLDefLocationDB and part of CalendarDB-PDat)
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
  
#ifndef _PILOT_LOCATION_H_
#define _PILOT_LOCATION_H_

#include <stdint.h>

#include <pi-appinfo.h>
#include <pi-buffer.h>

#ifdef __cplusplus
extern "C" {
#endif
	enum DayOfWeek {
		sunday, monday, tuesday, wednesday, thursday, friday, saturday
	};

	enum WeekOfMonth {
		first, second, third, fourth, last
	};

	enum Month {
		none, january, february, march, april, may, june, july, august,
		september, october, november, december
	};

	typedef struct {
		enum DayOfWeek dayOfWeek;
		enum WeekOfMonth weekOfMonth;
		enum Month month;
		uint8_t unknown;
      } DST_t;

	enum CompassDirection { north, east, south, west };
	typedef struct {
		int16_t degrees; /* negative is N/E */
		int16_t minutes; /* negative is N/E */
		enum CompassDirection direction;
      } EarthPoint_t;

	typedef struct {
		int16_t offset; /* Offset from GMT */
		uint8_t t2; /* 00 - 03 */
		DST_t dstStart;
		DST_t dstEnd;
		uint8_t dstObserved;
		uint8_t t4;
		uint8_t unknown; /* always 0x80 or 0x00 */
		char* name; /* the name of the entry, null terminated, max 21 plus null */
      } Timezone_t;

	typedef struct {
		Timezone_t tz;
		uint8_t unknownExists;
		uint8_t unknown;  /* byte that sometimes exists before the lat/lon */
		EarthPoint_t latitude;
		EarthPoint_t longitude;
		char* note; /* the note for this timezone, null terminated string or just null */
      } Location_t;
	
	extern void new_Timezone
	  PI_ARGS((Timezone_t *));
	extern void new_Location
	  PI_ARGS((Location_t *));
  
	extern void free_Timezone
	PI_ARGS((Timezone_t *tz));
	extern void free_Location
	PI_ARGS((Location_t *loc));
	
	extern int unpack_DST
	PI_ARGS((DST_t *dst, const pi_buffer_t *buf));
	extern int unpack_DST_p
	PI_ARGS((DST_t *dst, const unsigned char *data, const size_t position));
	extern int unpack_Timezone
	PI_ARGS((Timezone_t *tz, const pi_buffer_t *buf));
	extern int unpack_Timezone_p
	PI_ARGS((Timezone_t *tz, const unsigned char *data, const size_t position));
	extern int unpack_Location
	PI_ARGS((Location_t *tz, const pi_buffer_t *buf));

	extern int pack_DST
	PI_ARGS((const DST_t *dst, pi_buffer_t *buf));
	extern int pack_Timezone
	PI_ARGS((const Timezone_t *tz, pi_buffer_t *buf));
	extern int pack_Location
	PI_ARGS((const Location_t *tz, pi_buffer_t *buf));

	extern Timezone_t *dup_Timezone
	PI_ARGS((const Timezone_t *tz));

	extern int copy_Timezone
	PI_ARGS((const Timezone_t *source, Timezone_t *dest));

	extern int copy_Location
	PI_ARGS((const Location_t *source, Location_t *dest));
	
#ifdef __cplusplus
};
#endif

#endif				/* _PILOT_LOCATION_H_ */

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
