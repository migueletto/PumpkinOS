/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SelDay.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines the date picker month object's  structures 
 *   and routines.
 *
 *****************************************************************************/

#ifndef	__SELDAY_H__
#define	__SELDAY_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Day.h>

#define daySelectorMinYear  firstYear
#define daySelectorMaxYear  lastYear

#ifdef __cplusplus
extern "C" {
#endif

extern Boolean SelectDayV10 (Int16 *month, Int16 *day, Int16 *year, 
	const Char *title)
			SYS_TRAP(sysTrapSelectDayV10);

extern Boolean SelectDay (const SelectDayType selectDayBy, Int16 *month, 
	Int16 *day, Int16 *year, const Char *title)
			SYS_TRAP(sysTrapSelectDay);

#ifdef __cplusplus 
}
#endif

#endif //__SELDAY_H__
