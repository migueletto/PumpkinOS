/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyRmcLib.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       Audio Remote Controller Lib prototype                                *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 00/11/01                                  		   	*
 *       Last Modified: $$Date: 03/06/17 17:21 $ 
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */


#ifndef __RMC_LIB_H__
#define __RMC_LIB_H__

#include <SonyErrorBase.h>
#include <SonySystemResources.h>


#ifndef	RMC_PRV

// Palm common definitions

// BUILDING_APPLICATION
#if BUILDING_APP_OR_LIB || CPU_TYPE != CPU_68K
	// direct link to library code
	#define RMC_LIB_TRAP(trapNum) 
#else
	// else someone else is including this public header file; use traps
	#define RMC_LIB_TRAP(trapNum)	SYS_TRAP(trapNum)
#endif

#define rmcErrParam			(sonyRmcErrorClass | 1)		// invalid parameter
#define rmcErrNotOpen		(sonyRmcErrorClass | 2)		// library is not open
#define rmcErrStillOpen		(sonyRmcErrorClass | 3)		// returned from SampleLibClose() if
																	// the library is still open by others
#define rmcErrNotAvailable	(sonyRmcErrorClass | 4)		// memory error occurred
#define rmcErrRegister		(sonyRmcErrorClass | 5)		// registration related error occurred

#if CPU_TYPE == CPU_68K
//-----------------------------------------------------------------------------
// Rmc library function trap ID's. Each library call gets a trap number:
//   RmcLibTrapXXXX which serves as an index into the library's dispatch table.
//   The constant sysLibTrapCustom is the first available trap number after
//   the system predefined library traps Open,Close,Sleep & Wake.
//
// WARNING!!! The order of these traps MUST match the order of the dispatch
//  table in RmcLibDispatch.c!!!
//-----------------------------------------------------------------------------

typedef enum {
	RmcLibTrapRegister = sysLibTrapCustom,
	RmcLibTrapDisableKeyHandler,
	RmcLibTrapEnableKeyHandler,
	RmcLibTrapGetStatus,
	RmcLibTrapKeyControl,
	RmcLibTrapSwitchAdcVal
} RmcLibTrapNumberEnum;

#endif
typedef enum {
	rmcRegTypeWeak,
	rmcRegTypeStrong
} RmcRegEnum;

typedef enum {
	rmcKeyOther = 0,		// Unknown keys
	rmcKeyPlay,				// Play
	rmcKeyFrPlay,			// FR/Play
	rmcKeyFfPlay,			// FF/Play
	rmcKeyStop,				// Stop
	rmcKeyDown,				// Down
	rmcKeyUp,				// Up
	rmcKeyNum				// Num of all RMC keys
} RmcKeyCodeEnum;

typedef struct {
	UInt32	creatorID;
	UInt32	reserved;
} RmcStatusType;

/********************************************************************
 * API Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#endif	//RMC_PRV


#if CPU_TYPE == CPU_68K
//--------------------------------------------------
// Standard library open, close, sleep and wake functions
//--------------------------------------------------
#ifndef	RMC_PRV

extern Err RmcLibOpen(UInt16 refNum)
				RMC_LIB_TRAP(sysLibTrapOpen);
				
extern Err RmcLibClose(UInt16 refNum)
				RMC_LIB_TRAP(sysLibTrapClose);

extern Err RmcLibSleep(UInt16 refNum)
				RMC_LIB_TRAP(sysLibTrapSleep);

extern Err RmcLibWake(UInt16 refNum)
				RMC_LIB_TRAP(sysLibTrapWake);

//--------------------------------------------------
// Custom library API functions
//--------------------------------------------------
#endif	//RMC_PRV

typedef void (*RmcKeyHandleProcPtr)(struct _KeyDownEventType *keyDown);

#ifndef	RMC_PRV

extern Err RmcRegister(UInt16 refNum, RmcRegEnum type,
					RmcKeyHandleProcPtr keyHandler, UInt32 creatorID )
				RMC_LIB_TRAP(RmcLibTrapRegister);
	
extern Err RmcDisableKeyHandler(UInt16 refNum)
				RMC_LIB_TRAP(RmcLibTrapDisableKeyHandler);

extern Err RmcEnableKeyHandler(UInt16 refNum)
				RMC_LIB_TRAP(RmcLibTrapEnableKeyHandler);

extern Err RmcGetStatus(UInt16 refNum, RmcStatusType *status)
				RMC_LIB_TRAP(RmcLibTrapGetStatus);

Err RmcKeyRates(UInt16 refNum, Boolean set,
					 UInt16* initDelayP, UInt16* periodP)
				RMC_LIB_TRAP(RmcLibTrapKeyControl);

extern Err RmcLibSwitchAdcVal(UInt16 refNum,
					Boolean refORset, Boolean* adcDirectValueSwitchP)
				RMC_LIB_TRAP(RmcLibTrapSwitchAdcVal);


#endif	//RMC_PRV

#else //CPU_TYPE != CPU_68K

#ifndef	RMC_PRV
//--------------------------------------------------
// Standard library open, close, sleep and wake functions
//--------------------------------------------------

extern Err RmcLibOpen(void)
				RMC_LIB_TRAP(sysLibTrapOpen);
				
extern Err RmcLibClose(void)
				RMC_LIB_TRAP(sysLibTrapClose);

extern Err RmcLibSleep(void)
				RMC_LIB_TRAP(sysLibTrapSleep);

extern Err RmcLibWake(void)
				RMC_LIB_TRAP(sysLibTrapWake);

//--------------------------------------------------
// Custom library API functions
//--------------------------------------------------
#endif	//RMC_PRV

typedef void (*RmcKeyHandleProcPtr)(struct _KeyDownEventType *keyDown);

#ifndef	RMC_PRV

extern Err RmcRegister(RmcRegEnum type,
					RmcKeyHandleProcPtr keyHandler, UInt32 creatorID )
				RMC_LIB_TRAP(RmcLibTrapRegister);
	
extern Err RmcDisableKeyHandler(void)
				RMC_LIB_TRAP(RmcLibTrapDisableKeyHandler);

extern Err RmcEnableKeyHandler(void)
				RMC_LIB_TRAP(RmcLibTrapEnableKeyHandler);

extern Err RmcGetStatus(RmcStatusType *status)
				RMC_LIB_TRAP(RmcLibTrapGetStatus);

Err RmcKeyRates(Boolean set,
					 UInt16* initDelayP, UInt16* periodP)
				RMC_LIB_TRAP(RmcLibTrapKeyControl);

extern Err RmcLibSwitchAdcVal(Boolean refORset,
					 Boolean* adcDirectValueSwitchP )
				RMC_LIB_TRAP(RmcLibTrapSwAdcVal);
#endif	//RMC_PRV

#endif // CPU_TYPE == CPU_68K


#ifndef	RMC_PRV
//--------------------------------------------------
// Macros
//--------------------------------------------------

#define	ADMaxPlay		3372
#define	ADMinPlay		3235

#define	ADMaxFrPlay		3167
#define	ADMinFrPlay		3030

#define	ADMaxFfPlay		2566
#define	ADMinFfPlay		2430

#define	ADMaxStop		2048
#define	ADMinStop		1938

#define	ADMaxDown		1911
#define	ADMinDown		1802

#define	ADMaxUp			1761
#define	ADMinUp			1665


#define	GetRmcKey(keyCode)	\
			( (keyCode <= ADMaxPlay		)? \
			( (keyCode >= ADMinPlay		)? (rmcKeyPlay): \
			( (keyCode <= ADMaxFrPlay 	)? \
			( (keyCode >= ADMinFrPlay 	)? (rmcKeyFrPlay): \
			( (keyCode <= ADMaxFfPlay 	)? \
			( (keyCode >= ADMinFfPlay 	)? (rmcKeyFfPlay): \
			( (keyCode <= ADMaxStop    )? \
			( (keyCode >= ADMinStop    )? (rmcKeyStop): \
			( (keyCode <= ADMaxDown    )? \
			( (keyCode >= ADMinDown    )? (rmcKeyDown): \
			( (keyCode <= ADMaxUp      )? \
			( (keyCode >= ADMinUp      )? (rmcKeyUp): (rmcKeyOther) ): \
			(rmcKeyOther))): \
			(rmcKeyOther))): \
			(rmcKeyOther))): \
			(rmcKeyOther))): \
			(rmcKeyOther))): \
			(rmcKeyOther))

#define	isRmcKeyPlay(keyCode)	((keyCode <= ADMaxPlay)? \
											((keyCode >= ADMinPlay)? (true): (false)):\
											(false))
#define	isRmcKeyFrPlay(keyCode)	((keyCode <= ADMaxFrPlay)? \
											((keyCode >= ADMinFrPlay)? (true): (false)):\
											(false))
#define	isRmcKeyFfPlay(keyCode)	((keyCode <= ADMaxFfPlay)? \
											((keyCode >= ADMinFfPlay)? (true): (false)):\
											(false))
#define	isRmcKeyStop(keyCode)	((keyCode <= ADMaxStop)? \
											((keyCode >= ADMinStop)? (true): (false)):\
											(false))
#define	isRmcKeyDown(keyCode)	((keyCode <= ADMaxDown)? \
											((keyCode >= ADMinDown)? (true): (false)):\
											(false))
#define	isRmcKeyUp(keyCode)		((keyCode <= ADMaxUp)? \
											((keyCode >= ADMinUp)? (true): (false)):\
											(false))
#endif	//RMC_PRV


#ifdef __cplusplus 
}
#endif


#endif	// __RMC_LIB_H__
