/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup HiRes Hi-Resolution Timer Library
 *
 * @{
 * @}
 */

/**
 *
 * @file	palmOneHiResTimer.h
 * @version 1.0
 *
 * @brief   Hi-Resolution timer for Palm OS.
 **/

#ifndef _PALMONEHIRESTIMER_H_
#define _PALMONEHIRESTIMER_H_

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>
#include <LibTraps.h>
#include <Common\Libraries\HiResTimerLib\palmOneHiResTimerCommon.h>


/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Opens the Hi-Res Timer library.
 *
 * @param refNum:	IN: Library reference number.
 * @retval Err Error code.
 */
extern Err HRTimeLibOpen(UInt16 refNum)
	SYS_TRAP(sysLibTrapOpen);

/**
 * Closes the Hi-Res Timer library.
 *
 * @param refNum:	IN: Library reference number.
 * @retval Err Error code.
 */
extern Err HRTimeLibClose(UInt16 refNum)
	SYS_TRAP(sysLibTrapClose);

/**
 * Get the frequency of the Hi-Resolution Timer.
 *
 * @param refNum:	IN:  Library reference number.
 * @param ticksP:	OUT: Frequency of the timer.
 * @retval Err Error code.
 */
extern Err HRTimeLibTicksPerSecond(UInt16 refNum, UInt32 *ticksP)
	SYS_TRAP(kHRLibTrapTimerTicksPerSecond);

/**
 * Get the current ticks for Hi-Resolution Timer.
 *
 * @param refNum:		IN:  Library reference number.
 * @param ticksP:		OUT: Current ticks.
 * @param rolloverP:	OUT: Number of rollover since the timer started.
 * @retval Err Error code.
 */
extern Err HRTimeLibGetTime(UInt16 refNum, UInt32 *ticksP, UInt32 *rolloverP)
	SYS_TRAP(kHRLibTrapTimerGetTime);




#ifdef __cplusplus
}
#endif

#endif  // _PALMONEHIRESTIMER_H_
