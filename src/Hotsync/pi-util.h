/*
 * $Id: pi-util.h,v 1.17 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-util.h: Header for utility routines
 *
 * Copyright (c) 2000, Helix Code Inc.
 *
 * Author: JP Rosevear <jpr@helixcode.com>
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
 *
 */

#ifndef _PILOT_UTIL_H_
#define _PILOT_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pi-args.h"

/* pi_mktag Turn a sequence of characters into a long (er.. 32 bit quantity)
            like those used on the PalmOS device to identify creators and
            similar.

   pi_untag Given a 32 bit identifier, unpack it into the 5-byte char array
	    buf so it is suitable for printing.

   Both of these macros are deprecated for runtime use, but for calculating
   compile-time constants pi_mktag is ok.
*/
#define pi_mktag(c1,c2,c3,c4) (((c1)<<24)|((c2)<<16)|((c3)<<8)|(c4))
#define pi_untag(buf,tag) { buf[0]=(tag >> 24) & 0xff; \
	buf[1]=(tag >> 16) & 0xff; \
	buf[2]=(tag >> 8) & 0xff; \
	buf[3]=(tag) & 0xff; \
	buf[4]=0; }


	/** @brief Read the PILOTRATE environment variable
	 *
	 * If the PILOTRATE environment variable is set, read it. It should
	 * be a speed value. If the first letter is an 'H', then it means we
	 * want to use this speed even if it's higher than the highest speed
	 * published by the device.
	 *
	 * @param establishrate On return, PILOTRATE value or -1 if environment variable not set
	 * @param establishhighrate On return, 1 if speed prefixed with 'H', 0 otherwise
	 */
	extern void get_pilot_rate
		PI_ARGS((int *establishrate, int *establishhighrate));

	extern int convert_ToPilotChar_WithCharset
		PI_ARGS((const char *charset, const char *text, int bytes,
		     char **ptext, const char *pi_charset));

	extern int convert_ToPilotChar
		PI_ARGS((const char *charset, const char *text, int bytes,
		     char **ptext));

	extern int convert_FromPilotChar_WithCharset
		PI_ARGS((const char *charset, const char *ptext, int bytes,
		     char **text, const char *pi_charset));

	extern int convert_FromPilotChar
		PI_ARGS((const char *charset, const char *ptext, int bytes,
		     char **text));

	/** @brief Convert a milliseconds timeout value to an absolute timespec
	 *
	 * @param timeout Timeout value from now, in milliseconds
	 * @param ts Ptr to a timespec structure to fill. Contains the absolute time on return.
	 */
	extern void pi_timeout_to_timespec
		PI_ARGS((int timeout, struct timespec *ts));

	/** @brief Convert an absolute time to a timeout value from now (in milliseconds)
	 *
	 * The returned timeout will be a negative if we passed the absolute
	 * time already
	 *
	 * @param ts Timespec with an absolute time
	 * @return Timeout value in milliseconds (negative if expired)
	 */
	extern int pi_timespec_to_timeout
		PI_ARGS((const struct timespec *ts));

	/** @brief Checks if an absolute timeout is expired
	 *
	 * @param ts Absolute time defining the timeout time
	 * @return Non-zero if expired
	 */
	extern int pi_timeout_expired
		PI_ARGS((const struct timespec *ts));

#ifdef __cplusplus
}
#endif
#endif
