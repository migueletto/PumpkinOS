/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup HiRes
 */

/**
 *
 * @file	palmOneHiResTimerCommon.h
 * @version 1.0
 * @brief Hi-Resolution timer for PalmOS.
 */

#ifndef _PALMONEHIRESTIMERCOMMON_H_
#define _PALMONEHIRESTIMERCOMMON_H_

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>


/***********************************************************************
 * Type and creator of the Library
 ***********************************************************************/

#define		kHiResTimeLibType		'libr'				/**< Hi-Res timer library database type */
#define		kHiResTimeLibCreator	'pHRT'				/**< Hi-Res timer library database creator */

/***********************************************************************
 * Internal library name which can be passed to SysLibFind()
 ***********************************************************************/

#define		kHiResTimeLibName		"HRTimer-Lib"   /**< Hi-Res timer library name */

/***********************************************************************
 * Library versioning
 ***********************************************************************/

#define		kHiResTimeLibVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0) /**< Hi-Res timer library version 1 */
#define		kHiResTimeLibVersion2	sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0) /**< Hi-Res timer library version 2 */
#define		kHiResTimeLibVersion	kCamLibVersion2 /**< Hi-Res timer library version 2 */

/***********************************************************************
 * HiRes Timer result codes
 ***********************************************************************/
/** Hi-Res timer base error number */
#define kHiResTimeErrorClass			(oemErrorClass + 0x100)
/** Bad Parameter */
#define kHiResTimeLibErrBadParam		(kHiResTimeErrorClass | 0x01)
/** Library is not open */
#define kHiResTimeLibErrNotOpen			(kHiResTimeErrorClass | 0x02)
/** Returned from HRTimeLibClose() if the library is still open */
#define kHiResTimeLibErrStillOpen		(kHiResTimeErrorClass | 0x03)
/** Fucntion not availabe - If returned by HRTimeLibOpen, there is no HiRes Timer available. */
#define kHiResTimeLibErrNotAvailable	(kHiResTimeErrorClass | 0x04)

/***********************************************************************
 * Library trap
 ***********************************************************************/

/**
 * @name Function Trap Numbers
 */
/*@{*/
#define kHRLibTrapTimerTicksPerSecond 	(sysLibTrapCustom)
#define kHRLibTrapTimerGetTime			(sysLibTrapCustom+1)
/*@}*/

#endif  // _PALMONEHIRESTIMERCOMMON_H_
