/*
 * $Id: pi-threadsafe.h,v 1.6 2008/11/06 10:45:33 desrod Exp $
 *
 * pi-threadsafe.h: utilities for thread-safe behavior
 *
 * Copyright (c) 2005, Florent Pillet.
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
 * * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
 */

#ifndef _PILOT_THREADSAFE_H
#define _PILOT_THREADSAFE_H

#include <stdint.h>

#if HAVE_PTHREAD

	#include <pthread.h>

	#define PI_THREADSAFE 1

	#define PI_MUTEX_DECLARE(mutex_name)	pthread_mutex_t mutex_name
	#define PI_MUTEX_DEFINE(mutex_name)	pthread_mutex_t mutex_name = PTHREAD_MUTEX_INITIALIZER

	typedef pthread_mutex_t pi_mutex_t;

#else
	/* when not in thread-safe mode, we still use dummy variables the
	   code will simply do nothing */
	#define PI_THREADSAFE 0

	#define PI_MUTEX_DECLARE(mutex_name)	int mutex_name

	/* dummy declaration for the code to compile */
	#define	PI_MUTEX_DEFINE(mutex_name)	int mutex_name = 0

        /* ditto from above */
	typedef int pi_mutex_t;
#endif

extern int pi_mutex_lock(pi_mutex_t *mutex);

extern int pi_mutex_trylock(pi_mutex_t *mutex);

extern int pi_mutex_unlock(pi_mutex_t *mutex);

extern uint32_t pi_thread_id(void);

#endif
