/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: AttentionMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Attention Manager
 *
 *****************************************************************************/

#ifndef __ATTENTION_MGR_H__
#define __ATTENTION_MGR_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <Rect.h>
#include <SysEvent.h>
#include <Event.h>

/************************************************************
 * Attention Manager result codes
 * (attnErrorClass is defined in ErrorBase)
 *************************************************************/
#define	attnErrMemory			(attnErrorClass | 1)	// ran out of memory


/************************************************************
 * Attention Indicator bounds
 *************************************************************/
#define kAttnIndicatorLeft		0
#define kAttnIndicatorTop		0
#define kAttnIndicatorWidth	16
#define kAttnIndicatorHeight	15
	

/************************************************************
 * Constants used for list view drawing.
 *
 * Applications should use the following constants to format 
 * the display of information in attention manager list view.
 *
 * The application's small icon should be drawn centered within
 * the first kAttnListMaxIconWidth pixels of the drawing bounds.
 *
 * Two lines of text information describing the attention should 
 * then be drawn left justified starting at kAttnListTextOffset 
 * from the left edge of the drawing bounds.
 *************************************************************/
#define	kAttnListMaxIconWidth	15
#define	kAttnListTextOffset		17
			
			
/********************************************************************
 * Attention Manager Structures
 ********************************************************************/

typedef UInt32 AttnFlagsType;

#define kAttnFlagsSoundBit				((AttnFlagsType)0x0001)
#define kAttnFlagsLEDBit				((AttnFlagsType)0x0002)
#define kAttnFlagsVibrateBit			((AttnFlagsType)0x0004)
#define kAttnFlagsCustomEffectBit	((AttnFlagsType)0x0008)
	// Note: More bits can be defined if/when hardware capability increases

#define kAttnFlagsAllBits				((AttnFlagsType)0xFFFF)


// The following are passed to AttnGetAttention() and AttnUpdate to specify
// overrides from the user settings for an attention request.
#define kAttnFlagsUseUserSettings	((AttnFlagsType)0x00000000)

#define kAttnFlagsAlwaysSound			(kAttnFlagsSoundBit)
#define kAttnFlagsAlwaysLED			(kAttnFlagsLEDBit)
#define kAttnFlagsAlwaysVibrate		(kAttnFlagsVibrateBit)
#define kAttnFlagsAlwaysCustomEffect (kAttnFlagsCustomEffectBit)
#define kAttnFlagsEverything			(kAttnFlagsAllBits)

#define kAttnFlagsNoSound				(kAttnFlagsSoundBit<<16)
#define kAttnFlagsNoLED					(kAttnFlagsLEDBit<<16)
#define kAttnFlagsNoVibrate			(kAttnFlagsVibrateBit<<16)
#define kAttnFlagsNoCustomEffect		(kAttnFlagsCustomEffectBit<<16)
#define kAttnFlagsNothing				(kAttnFlagsAllBits<<16)


// The following are used to interpret the feature.
#define kAttnFtrCreator					'attn'
#define kAttnFtrCapabilities			0			// Read to determine device capabilities and user settings.

#define kAttnFlagsUserWantsSound		(kAttnFlagsSoundBit)
#define kAttnFlagsUserWantsLED		(kAttnFlagsLEDBit)
#define kAttnFlagsUserWantsVibrate	(kAttnFlagsVibrateBit)
#define kAttnFlagsUserWantsCustomEffect (kAttnFlagsCustomEffectBit)	// Always false
#define kAttnFlagsUserSettingsMask	(kAttnFlagsAllBits)

#define kAttnFlagsHasSound				(kAttnFlagsSoundBit<<16)
#define kAttnFlagsHasLED				(kAttnFlagsLEDBit<<16)
#define kAttnFlagsHasVibrate			(kAttnFlagsVibrateBit<<16)
#define kAttnFlagsHasCustomEffect	(kAttnFlagsCustomEffectBit<<16)	// Always true
#define kAttnFlagsCapabilitiesMask	(kAttnFlagsAllBits<<16)


typedef UInt16 AttnLevelType;
	#define kAttnLevelInsistent		((AttnLevelType)0)
	#define kAttnLevelSubtle			((AttnLevelType)1)

typedef UInt16 AttnCommandType;
	#define kAttnCommandDrawDetail	((AttnCommandType)1)
	#define kAttnCommandDrawList		((AttnCommandType)2)
	#define kAttnCommandPlaySound		((AttnCommandType)3)
	#define kAttnCommandCustomEffect	((AttnCommandType)4)
	#define kAttnCommandGoThere		((AttnCommandType)5)
	#define kAttnCommandGotIt			((AttnCommandType)6)
	#define kAttnCommandSnooze			((AttnCommandType)7)
	#define kAttnCommandIterate		((AttnCommandType)8)

typedef union AttnCommandArgsTag {
	struct AttnCommandArgsDrawDetailTag{
		RectangleType bounds;
		Boolean firstTime;
		AttnFlagsType flags;
	} drawDetail;
	
	struct AttnCommandArgsDrawListTag {
		RectangleType bounds;
		Boolean firstTime;
		AttnFlagsType flags;
		Boolean selected;
	} drawList;
	
	struct AttnCommandArgsGotItTag {
		Boolean dismissedByUser;
	} gotIt;
	
	struct AttnCommandArgsIterateTag {
		UInt32 iterationData;
	} iterate;
} AttnCommandArgsType;

typedef struct {
	AttnCommandType command;
	UInt32 userData;
	AttnCommandArgsType *commandArgsP;
} AttnLaunchCodeArgsType;

typedef Err AttnCallbackProc (AttnCommandType command, UInt32 userData, AttnCommandArgsType *commandArgsP);

// These details go with the sysNotifyGotUsersAttention notification.
typedef struct {
	AttnFlagsType flags;
} AttnNotifyDetailsType;


/********************************************************************
 * Public Attention Manager Routines
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Err AttnGetAttention (UInt16 cardNo, LocalID dbID, UInt32 userData,
	AttnCallbackProc *callbackFnP, AttnLevelType level, AttnFlagsType flags,
	UInt16 nagRateInSeconds, UInt16 nagRepeatLimit)
		SYS_TRAP(sysTrapAttnGetAttention);

Boolean AttnUpdate (UInt16 cardNo, LocalID dbID, UInt32 userData,
	AttnCallbackProc *callbackFnP, AttnFlagsType *flagsP,
	UInt16 *nagRateInSecondsP, UInt16 *nagRepeatLimitP)
		SYS_TRAP(sysTrapAttnUpdate);

Boolean AttnForgetIt (UInt16 cardNo, LocalID dbID, UInt32 userData)
		SYS_TRAP(sysTrapAttnForgetIt);

UInt16 AttnGetCounts (UInt16 cardNo, LocalID dbID, UInt16 *insistentCountP, UInt16 *subtleCountP)
		SYS_TRAP(sysTrapAttnGetCounts);

void AttnListOpen (void)
		SYS_TRAP(sysTrapAttnListOpen);

void AttnIterate (UInt16 cardNo, LocalID dbID, UInt32 iterationData)
		SYS_TRAP(sysTrapAttnIterate);

Err AttnDoSpecialEffects(AttnFlagsType flags)
		SYS_TRAP(sysTrapAttnDoSpecialEffects);

void AttnIndicatorEnable(Boolean enableIt)
		SYS_TRAP(sysTrapAttnIndicatorEnable);

Boolean AttnIndicatorEnabled(void)
		SYS_TRAP(sysTrapAttnIndicatorEnabled);


#ifdef __cplusplus 
}
#endif

#endif  // __ATTENTION_MGR_H__
