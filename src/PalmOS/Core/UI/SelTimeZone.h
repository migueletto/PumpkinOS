/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelTimeZone.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines select time zone structures and routines.
 *
 *****************************************************************************/

#ifndef	__SELTIMEZONE_H__
#define	__SELTIMEZONE_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <DateTime.h>
#include <LocaleMgr.h>		// LmLocaleType

#ifdef __cplusplus
extern "C" {
#endif

extern Boolean SelectTimeZone(Int16 *ioTimeZoneP, LmLocaleType* ioLocaleInTimeZoneP,
				const Char *titleP, Boolean showTimes, Boolean anyLocale)
						SYS_TRAP(sysTrapSelectTimeZone);

#ifdef __cplusplus 
}
#endif

#endif // __SELTIMEZONE_H__
