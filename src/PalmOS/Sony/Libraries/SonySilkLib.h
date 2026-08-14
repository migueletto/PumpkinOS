/******************************************************************************
 *                                                                            *
 *                 (C) Copyright 2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonySilkLib.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Virtual Silk Lib API                                                 *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/11/01                                  		   	*
 *       Last Modified: $Date: 03/07/23 10:41 $ 
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */


#ifndef __SILK_LIB_H__
#define __SILK_LIB_H__

#include <SonyErrorBase.h>


#if CPU_TYPE != CPU_68K
	#define SILK_LIB_TRAP(trapNum) 	// direct link to library code
#else
	#define SILK_LIB_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif


/* ------------------------------------- */
/*				Constant def					  */
/* ------------------------------------- */
/*		SilkMgr Errors		*/
#define silkLibErrParam					(sonySilkErrorClass | 1)	// invalid parameter
#define silkLibErrNotOpen				(sonySilkErrorClass | 2)	// library is not open
#define silkLibErrStillOpen			(sonySilkErrorClass | 3)	// returned from SilkLibClose() if
																					// the library is still opened by others
#define silkLibErrNotAvailable		(sonySilkErrorClass | 4)	// memory error occurred
#define silkLibErrResizeDisabled		(sonySilkErrorClass | 5)	// cannot resize

#define vskErrParam						silkLibErrParam
#define vskErrNotOpen					silkLibErrNotOpen
#define vskErrStillOpen					silkLibErrStillOpen
#define vskErrNotAvailable				silkLibErrNotAvailable
#define vskErrResizeDisabled			silkLibErrResizeDisabled
#define vskErrSlkwNotFound				(sonySilkErrorClass | 6)
#define vskErrSlkwOpenFailed			(sonySilkErrorClass | 7)
#define vskErrSlkwCloseFailed			(sonySilkErrorClass | 8)
#define vskErrSlkwStartFailed			(sonySilkErrorClass | 9)
#define vskErrSlkwStopFailed			(sonySilkErrorClass | 10)
#define vskErrSlkwLoadFailed			(sonySilkErrorClass | 11)
#define vskErrFuncNotAvailable		(sonySilkErrorClass | 12)
//#define vskErr							(sonySilkErrorClass | )
// up to										(sonySilkErrorClass | 15)



#define silkLibAPIVertion				(0x00000003)
#define vskAPIVertion					silkLibAPIVertion
#define vskVersionNum1					(0x00010000)
#define vskVersionNum2					(0x00020000)
#define vskVersionNum3					(0x00030000)

/*		stateType			*/
#define vskStateResize					(0)
	#define silkResizeNormal			(0)
	#define silkResizeToStatus			(1)
	#define silkResizeMax				(2)
	#define vskResizeMax					silkResizeNormal
	#define vskResizeMin					silkResizeToStatus
	#define vskResizeNone				silkResizeMax

#define vskStateEnable					(1)
#define vskStateResizeDirection			(5)
/*The state definition of vskStateEnable and vskStateResizeDirection*/
	#define vskResizeDisable    		(0)
	#define vskResizeVertically     	(1<<0)
	#define vskResizeHorizontally    	(1<<1)

#define vskStateSilkPlugInAvailable		(6)
	#define vskSilkPlugInNotAvailable 	(0)
	#define vskSilkPlugInAvailable    	(1)



/*		slkwType				*/
#define vskSlkwTypeSilk					(0)
#define vskSlkwTypeStatus				(1)

#if CPU_TYPE == CPU_68K
/* ------------------------------------- */
/*				Type def							  */
/* ------------------------------------- */
typedef enum {
	silkLibTrapResizeDispWin = sysLibTrapCustom,
	silkLibTrapEnableResize,
	silkLibTrapDisableResize,
	silkLibTrapGetAPIVersion,
	silkLibLastTrap
} SilkLibTrapNumberEnum;

typedef enum {
	VskTrapGetAPIVersion = silkLibTrapGetAPIVersion,
	VskTrapSetCurrentSlkw,
	VskTrapGetCurrentSlkw,
	VskTrapSetState,
	VskTrapGetState,
	VskTrapEnablePalmSilk,
	VskTrapGetPalmSilkEnabled,
	VskTrapTimerWrite,
	VskTrapDoCommand,
	VskTrapSetDrawWindow,
	VskTrapRestoreDrawWindow,
	VskLastTrap
} VskTrapNumberEnum;

#endif

/* ------------------------------------- */
/*				API Prototypes			 		  */
/* ------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

#if CPU_TYPE != CPU_68K
// new APIs
extern Err VskOpen(void);
extern Err VskClose(void);
extern Err VskSleep(void);
extern Err VskWake(void);
extern UInt32 VskGetAPIVersion(void);
extern Err VskSetCurrentSlkw(UInt16 slkwType, UInt32 creator);
extern Err VskGetCurrentSlkw(UInt16 slkwType, UInt32 *creatorP);
extern Err VskSetState(UInt16 stateType, UInt16 state);
extern Err VskGetState(UInt16 stateType, UInt16 *stateP);
extern Err VskEnablePalmSilk(Boolean enable);
extern Err VskGetPalmSilkEnabled(Boolean *graffiti, Boolean *penButton);
extern Err VskTimerWrite(UInt16 slkwType, UInt32 interval/*msec*/);
extern Err VskDoCommand(UInt32 creator, UInt16 command,
								UInt32 data1, UInt32 data2);
extern Err VskSetDrawWindow(UInt16 slkwType);
extern Err VskRestoreDrawWindow(UInt16 slkwType);

#else

extern Err SilkLibOpen(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapOpen);
				
extern Err SilkLibClose(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapClose);

extern Err SilkLibSleep(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapSleep);

extern Err SilkLibWake(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapWake);

extern Err SilkLibResizeDispWin(UInt16 refNum, UInt8 win)
				SILK_LIB_TRAP(silkLibTrapResizeDispWin);

extern Err SilkLibEnableResize(UInt16 refNum)
				SILK_LIB_TRAP(silkLibTrapEnableResize);

extern Err SilkLibDisableResize(UInt16 refNum)
				SILK_LIB_TRAP(silkLibTrapDisableResize);

extern UInt32 SilkLibGetAPIVersion(UInt16 refNum)
				SILK_LIB_TRAP(silkLibTrapGetAPIVersion);

// new APIs
extern Err VskOpen(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapOpen);

extern Err VskClose(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapClose);

extern Err VskSleep(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapSleep);

extern Err VskWake(UInt16 refNum)
				SILK_LIB_TRAP(sysLibTrapWake);

// this will be removed
//extern Err VskResize(UInt16 refNum, UInt8 reqSize);

// this will be removed
//extern Err VskEnableResize(UInt16 refNum);

// this will be removed
//extern Err VskDisableResize(UInt16 refNum);

extern UInt32 VskGetAPIVersion(UInt16 refNum)
				SILK_LIB_TRAP(VskTrapGetAPIVersion);

extern Err VskSetCurrentSlkw(UInt16 refNum, UInt16 slkwType, UInt32 creator)
				SILK_LIB_TRAP(VskTrapSetCurrentSlkw);

extern Err VskGetCurrentSlkw(UInt16 refNum, UInt16 slkwType, UInt32 *creatorP)
				SILK_LIB_TRAP(VskTrapGetCurrentSlkw);

extern Err VskSetState(UInt16 refNum, UInt16 stateType, UInt16 state)
				SILK_LIB_TRAP(VskTrapSetState);

extern Err VskGetState(UInt16 refNum, UInt16 stateType, UInt16 *stateP)
				SILK_LIB_TRAP(VskTrapGetState);

extern Err VskEnablePalmSilk(UInt16 refNum, Boolean enable)
				SILK_LIB_TRAP(VskTrapEnablePalmSilk);

extern Err VskGetPalmSilkEnabled(UInt16 refNum, Boolean *graffiti, Boolean *penButton)
				SILK_LIB_TRAP(VskTrapGetPalmSilkEnabled);

extern Err VskTimerWrite(UInt16 refNum, UInt16 slkwType, UInt32 interval/*msec*/)
				SILK_LIB_TRAP(VskTrapTimerWrite);

extern Err VskDoCommand(UInt16 refNum, UInt32 creator, UInt16 command,
								UInt32 data1, UInt32 data2)
				SILK_LIB_TRAP(VskTrapDoCommand);

extern Err VskSetDrawWindow(UInt16 refNum, UInt16 slkwType)
				SILK_LIB_TRAP(VskTrapSetDrawWindow);

extern Err VskRestoreDrawWindow(UInt16 refNum, UInt16 slkwType)
				SILK_LIB_TRAP(VskTrapRestoreDrawWindow);
#endif	//CPU_TYPE != CPU_68K

#ifdef __cplusplus 
}
#endif


#endif	// __SLK_LIB_H__
