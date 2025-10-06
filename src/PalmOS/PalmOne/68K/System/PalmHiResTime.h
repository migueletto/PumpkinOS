/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmHiResTime.h
 * @version 1.0
 * @date 	10/07/2003
 *
 * @brief Exported High Resolution timer definitions.
 * 
 *
 * <hr>
 */


 
#ifndef __PalmHiResTime_H__
#define __PalmHiResTime_H__

// Palm OS common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>

// To Define when building the library
#if (CPU_TYPE != CPU_68K) || (defined BUILDING_CAM_LIB)
	#define ARM_NATIVECODE			/**<		*/
	//#define CAM_LIB_TRAP(trapNum)		/**<		*/
#else
	#include <LibTraps.h>
	//#define CAM_LIB_TRAP(trapNum) SYS_TRAP(trapNum)		/**<		*/
#endif

/**
 * @name Type and creator of the Library
 *
 */
/*@{*/
#define		kHiResTimeLibType		'libr'			/**< database type */
#define		kHiResTimeLibCreator	'pHRT'				/**< database creator */
/*@}*/

/** Internal library name which can be passed to SysLibFind() */
#define		kHiResTimeLibName		"HRTimer-Lib"

/**
 * @name Library versionning
 *
 */
/*@{*/
#define		kHiResTimeLibVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)	/**<		*/
#define		kHiResTimeLibVersion	kCamLibVersion1			/**<		*/
/*@}*/

/** HiRes Timer result codes */
#define kHiResTimeErrorClass			(oemErrorClass + 0x100)

/**
 * @name Library trap
 *
 */
/*@{*/
#define kHRLibTrapTimerTicksPerSecond 	(sysLibTrapCustom)			/**<		*/
#define kHRLibTrapTimerGetTime			(sysLibTrapCustom+1)		/**<		*/
/*@}*/

/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARM_NATIVECODE

/**
 * @brief Standard library open function
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err HRTimeLibOpen(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapOpen);

/**
 * @brief Standard library close function
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err HRTimeLibClose(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapClose);

/**
 * @brief Custom library API functions
 *
 * @param refNum:	IN:  
 * @param *ticksP:	IN:  
 * @retval Err error code.
 **/
extern Err HRTimeLibTicksPerSecond(UInt16 refNum, UInt32 *ticksP )
	CAM_LIB_TRAP(kHRLibTrapTimerTicksPerSecond);
	
/**
 * @brief
 *
 * @param refNum:	IN:  
 * @param *ticksP:	IN:  
 * @param *rolloverP:	IN:  
 * @retval Err error code.
 **/
extern Err HRTimeLibGetTime(UInt16 refNum, UInt32 *ticksP, UInt32 *rolloverP)
	CAM_LIB_TRAP(kHRLibTrapTimerGetTime);

#else
/**
 * @brief Currently only used by 68K app.  ARM code can directly call to the HAL
 *	   functions.  The HAL function prototypes are included here.
 *
 * @param *ticksP:	IN:  
 * @retval Err error code.
 **/
Err HRTimeLibTicksPerSecond( UInt32 *ticksP );

/**
 * @brief
 *
 * @param *ticksP:	IN:  
 * @param *rolloverP:	IN:  
 * @retval Err error code.
 **/
Err HRTimeLibGetTime( UInt32 *ticksP, UInt32 *rolloverP );

#endif


#ifdef __cplusplus
}
#endif

#endif // __PalmHiResTime_H__
