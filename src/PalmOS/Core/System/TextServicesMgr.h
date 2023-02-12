/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TextServicesMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for Text Services Manager. This provides the caller with
 *		an API for interacting with various text services, including front-end
 *		processors (FEPs), which are sometimes known as input methods.
 *
 *****************************************************************************/

#ifndef __TEXTSERVICESMGR_H__
#define __TEXTSERVICESMGR_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <SystemMgr.h>

#ifndef USE_TSM_TRAPS
	#if (EMULATION_LEVEL == EMULATION_NONE)
		#define	USE_TSM_TRAPS	1
	#else
		#define	USE_TSM_TRAPS	0
	#endif
#endif

#if USE_TSM_TRAPS
	#define TSM_TRAP(sel) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapTsmDispatch, sel)
#else
	#define TSM_TRAP(tsmSelectorNum)
#endif

/***********************************************************************
 * Public constants
 ***********************************************************************/

// Feature Creators and numbers, for use with the FtrGet() call.
#define	tsmFtrCreator					sysFileCTextServices

// Selector used with call to FtrGet(tsmFtrCreator, xxx) to get the
// Text Services Manager flags.
#define	tsmFtrNumFlags		0

// Flags returned by FtrGet(tsmFtrCreator, tsmFtrNumFlags) call.
#define	tsmFtrFlagsHasFep	0x00000001L		// Bit set if FEP is installed.

// Selectors for routines found in the Text Services manager. The order
// of these selectors MUST match the jump table in TextServicesMgr.c.
typedef UInt16 TsmSelector;

#define	tsmGetFepMode				0
#define	tsmSetFepMode				1
#define	tsmHandleEvent				2
#define	tsmInit						3	// new in 4.0
#define	tsmDrawMode					4	// new in 4.0
#define	tsmGetSystemFep			5	// new in 4.0
#define	tsmSetSystemFep			6	// new in 4.0
#define	tsmGetCurrentFep			7	// new in 4.0
#define	tsmSetCurrentFep			8	// new in 4.0
#define	tsmGetSystemFepCreator	9	// new in 5.0
#define	tsmSetSystemFepCreator	10	// new in 5.0
#define	tsmGetCurrentFepCreator	11	// new in 5.0
#define	tsmSetCurrentFepCreator	12	// new in 5.0
#define 	tsmFepHandleEvent			13	// new in 5.0
#define 	tsmFepMapEvent				14	// new in 5.0
#define 	tsmFepTerminate			15	// new in 5.0
#define 	tsmFepReset					16	// new in 5.0
#define 	tsmFepCommitAction		17	// new in 5.0
#define 	tsmFepOptionsList			18	// new in 5.0	
																	

#define	tsmMaxSelector		tsmFepOptionsList

// Input mode - used with TsmGet/SetFepMode.
typedef UInt16 TsmFepModeType;

#define tsmFepModeDefault	((TsmFepModeType)0)
#define tsmFepModeOff		((TsmFepModeType)1)
#define tsmFepModeCustom	((TsmFepModeType)128)


/***********************************************************************
 * Public types
 ***********************************************************************/

/***********************************************************************
 * Public routines
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Return the current mode for the active FEP. The <nullParam> parameter
// is unused and must be set to NULL.
TsmFepModeType TsmGetFepMode(void* nullParam)
		TSM_TRAP(tsmGetFepMode);

// Set the mode for the active FEP to be <inNewMode>. The previous mode
// is returned. The <nullParam> parameter is unused and must be set
// to NULL.
TsmFepModeType TsmSetFepMode(void* nullParam, TsmFepModeType inNewMode)
		TSM_TRAP(tsmSetFepMode);

#ifdef __cplusplus 
}
#endif

#endif  // __TEXTSERVICESMGR_H__
