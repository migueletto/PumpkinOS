/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: InsPoint.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines insertion point routines.
 *
 *****************************************************************************/

#ifndef __INSPOINT_H__
#define __INSPOINT_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

// Blink interval is half of a second
#define		insPtBlinkInterval	(sysTicksPerSecond / 2)		
#define		insPtWidth				2

#ifdef __cplusplus
extern "C" {
#endif

extern void InsPtInitialize (void)
							SYS_TRAP(sysTrapInsPtInitialize);

extern void InsPtSetLocation (const Int16 x, const Int16 y)
							SYS_TRAP(sysTrapInsPtSetLocation);

extern void InsPtGetLocation (Int16 *x, Int16 *y)
							SYS_TRAP(sysTrapInsPtGetLocation);

extern void InsPtEnable (Boolean enableIt)
							SYS_TRAP(sysTrapInsPtEnable);

extern Boolean InsPtEnabled (void)
							SYS_TRAP(sysTrapInsPtEnabled);

extern void InsPtSetHeight (const Int16 height)
							SYS_TRAP(sysTrapInsPtSetHeight);

extern Int16 InsPtGetHeight (void)
							SYS_TRAP(sysTrapInsPtGetHeight);

extern void InsPtCheckBlink (void)
							SYS_TRAP(sysTrapInsPtCheckBlink);

#ifdef __cplusplus 
}
#endif

#endif //__INSPOINT_H__
