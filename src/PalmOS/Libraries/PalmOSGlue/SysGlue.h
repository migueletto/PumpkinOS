/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SysGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for SysGlue library routines.
 *
 *		Glue providing system manager utility routines & bug-fixed routines.
 *
 *****************************************************************************/

#ifndef __SYSGLUE_H__
#define __SYSGLUE_H__

#include <PalmTypes.h>

// On the Simulator, we always assume that the trap (actually the routine) exists,
// since we're also assuming that the user is linking against the latest Palm OS
// core code, which will have a routine for every implemented trap. Since
// SysGetTrapAddress doesn't work on the Simulator, this is the best we can do.

#if (EMULATION_LEVEL == EMULATION_NONE)
#define	SysGlueTrapExists(trapNum)	(SysGlueGetTrapAddress(trapNum) != SysGlueGetTrapAddress(sysTrapSysUnimplemented))
#else
#define	SysGlueTrapExists(trapNum)	true
#endif

#ifdef __cplusplus
	extern "C" {
#endif

void* SysGlueGetTrapAddress(UInt16 trapNum);

#ifdef __cplusplus
	}
#endif

#endif	// __SYSGLUE_H__
