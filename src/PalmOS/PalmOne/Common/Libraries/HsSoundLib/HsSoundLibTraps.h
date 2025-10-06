/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 *
 * @file 	HsSoundLibTraps.h
 *
 * @brief	Public 68K include file for the sound library traps for Treo 600
 *			and Treo 650 smartphones.
 *
 * This header file and associated header files support the specific sound
 * functionality of the Treo smartphones. You should use the Palm OS Sound
 * Manager APIs for most of your work.
 *
 * Notes:
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 */


// INCLUDE ONCE
#ifndef __HSSOUNDLIB_TRAPS__H__
#define __HSSOUNDLIB_TRAPS__H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/
#include <PalmTypes.h>

#if 0
#pragma mark -------- Constants --------
#endif

/**
 * @name Sound library traps
 *
 */
/*@{*/
#define kHsSoundLibTrapOpen			  (sysLibTrapOpen)  /**<No definition. */
#define kHsSoundLibTrapClose		  (sysLibTrapClose) /**<No definition. */

#define kHsSoundLibTrapGetVersion     (sysLibTrapCustom + 0) /**<No definition. */
#define kHsSoundLibTrapPortGetInfo	  (sysLibTrapCustom + 1) /**<No definition. */
#define kHsSoundLibTrapPortSetParam	  (sysLibTrapCustom + 2) /**<No definition. */
#define kHsSoundLibTrapSwitchGetPort  (sysLibTrapCustom + 3) /**<No definition. */
#define kHsSoundLibTrapSwitchSetPort  (sysLibTrapCustom + 4) /**<No definition. */
#define kHsSoundLibTrapTonePlay		  (sysLibTrapCustom + 5) /**<No definition. */
#define kHsSoundLibTrapToneStop		  (sysLibTrapCustom + 6) /**<No definition. */
#define kHsSoundLibTrapFormatPlay	  (sysLibTrapCustom + 7) /**<No definition. */
#define kHsSoundLibTrapFormatRecord	  (sysLibTrapCustom + 8) /**<No definition. */
#define kHsSoundLibTrapGetUserVolume  (sysLibTrapCustom + 9) /**<No definition. */
#define kHsSoundLibTrapSetUserVolume  (sysLibTrapCustom + 10) /**<No definition. */
/*@}*/

#endif // INCLUDE ONCE -- __HSSOUNDLIB_TRAPS__H__