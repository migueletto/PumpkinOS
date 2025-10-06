/**
 * \file HsNavCommon.h
 *
 * Common defines shared between 68K HsNav.h and ARM HsNav.h.  The
 * contents of this file will all be integrated into the OS headers of
 * PalmOS 6.x.  The definitions are therefore preceded by the OS header
 * file into which they will be integrated.
 *
 * \license
 *
 * Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsNavCommon.h#5 $
 *
 ****************************************************************/

#ifndef	  __HS_NAV_COMMON_H__
#define	  __HS_NAV_COMMON_H__


#include <Bitmap.h>	// for RGBColorType

#if 0
#pragma mark -------- HS-Specific -------------
#endif

//*********************************************************
// The current version of the navigation API (this is the
//	value set for hsFtrIDNavigationSupported feature)
//*********************************************************
#define hsNavAPIVersion		2


//*********************************************************
// Focus treatment definitions
//*********************************************************
#define	hsNavFocusRingWidth			  6		  // Width in double-density coordinates
#define	hsNavFocusRingNoExtraInfo	  ((Int16)0xFFFF)


enum HsNavFocusColorEnumTag
{
  hsNavFocusColorInsideBottomRightRing	= 0,  // Deprecated (HsNav version 1 focus treatment)
  hsNavFocusColorInsideTopLeftRing = 1,		  // Deprecated (HsNav version 1 focus treatment)
  hsNavFocusColorMiddleRing = 2,			  // Deprecated (HsNav version 1 focus treatment)
  hsNavFocusColorOutsideRing = 3,			  // Deprecated (HsNav version 1 focus treatment)
  hsNavFocusColorSecondaryHighlight = 4,
  hsNavFocusColorRing1 = 5,					  // HsNav version 2 focus treatment.  Innermost ring.
  hsNavFocusColorRing2 = 6,					  // HsNav version 2 focus treatment.
  hsNavFocusColorRing3 = 7,					  // HsNav version 2 focus treatment.
  hsNavFocusColorRing4 = 8,					  // HsNav version 2 focus treatment.
  hsNavFocusColorRing5 = 9,					  // HsNav version 2 focus treatment.
  hsNavFocusColorRing6 = 10					  // HsNav version 2 focus treatment.  Outermost ring.
};
typedef UInt16 HsNavFocusColorEnum;


enum HsNavFocusRingStyleEnumTag
{
  hsNavFocusRingStyleObjectTypeDefault = 0,
  hsNavFocusRingStyleSquare = 1,
  hsNavFocusRingStyleRound = 2,
  hsNavFocusRingStyleHorizontalBars = 3,
  hsNavFocusRingStyleInvalid = 0xFFFF
};
typedef UInt16 HsNavFocusRingStyleEnum;

#if PALMOS_SDK_VERSION < 0x0541

// FrmNavFocusRingStyleEnum's definition should always be exactly the same as
//	HsNavFocusRingStyleEnum's definition since we want their use to be
//	completely interchangeable.  HsNavFocusRingStyleEnum is what the Treo600
//	SDK used and FrmNavFocusRingStyleEnum is what PSI (and Treo650) will use in
//	their SDK.  PSI originally was not going to take focus ring related API
//	but this changed after Treo600 shipped.
enum FrmNavFocusRingStyleEnumTag
{
  frmNavFocusRingStyleObjectTypeDefault = 0,
  frmNavFocusRingStyleSquare = 1,
  frmNavFocusRingStyleRound = 2,
  frmNavFocusRingStyleHorizontalBars = 3,
  frmNavFocusRingStyleInvalid = 0xFFFF
};
typedef UInt16 FrmNavFocusRingStyleEnum;
#define	frmNavFocusRingNoExtraInfo	  ((Int16)0xFFFF)

#endif // PALMOS_SDK_VERSION < 0x0541

#if 0
#pragma mark -------- Incs:Core:UI:UIResources.h -------------
#endif

#define formNavRscType                      'fnav'


#if 0
#pragma mark -------- Incs:Core:System:TextMgr.h ---------------
#endif

//*********************************************************
// Macro for detecting if character is a rocker character
//*********************************************************

// <c> is a rocker key if the event modifier <m> has the command bit set
// and <c> is in the proper range
#define	TxtCharIsRockerKey(m, c)	((((m) & commandKeyMask) != 0) && \
									((((c) >= vchrRockerUp) && ((c) <= vchrRockerCenter))))


#if 0
#pragma mark -------- Incs:Core:CoreTraps.h ----------------
#endif

//*********************************************************
// Selector definitions for Navigation API functions
//*********************************************************
#if PALMOS_SDK_VERSION < 0x0541

#define sysTrapNavSelector						0xA46F

#define NavSelectorFrmCountObjectsInNavOrder	0x0
#define NavSelectorFrmGetNavOrder            	0x1
#define NavSelectorFrmSetNavOrder           	0x2
#define NavSelectorFrmGetNavEntry				0x3
#define NavSelectorFrmSetNavEntry	       		0x4
#define NavSelectorFrmGetNavState		  		0x5
#define NavSelectorFrmSetNavState				0x6
#define NavSelectorFrmNavDrawFocusRing			0x7
#define NavSelectorFrmNavRemoveFocusRing		0x8
#define NavSelectorFrmNavGetFocusRingInfo		0x9

#endif // PALMOS_SDK_VERSION < 0x0541

#define NavSelectorFrmNavObjectTakeFocus		0xA


#if 0
#pragma mark ------- Incs:Core:System:ErrorBase.h --------
#endif

//*********************************************************
// Error class and codes for UI Library
//*********************************************************
#if PALMOS_SDK_VERSION < 0x0541

#define uilibErrorClass             0x3A00  // UI Library (Forms, Controls, etc)

#define uilibErrInvalidParam		  (uilibErrorClass | 1)
#define uilibErrCurrentFocusInvalid	  (uilibErrorClass | 2)
#define	uilibErrObjectFocusModeOff	  (uilibErrorClass | 3)
#define uilibErrObjectNotFound		  (uilibErrorClass | 4)
#define uilibErrNoNavInfoForForm	  (uilibErrorClass | 5)
#define uilibErrInvalidFocusObject	  (uilibErrorClass | 6)
#define uilibErrFormDoesNotHaveFocus  (uilibErrorClass | 7)

#endif // PALMOS_SDK_VERSION < 0x0541


#if 0
#pragma mark -------- Incs:Core:UI:Form.h -----------
#endif

//*********************************************************
// Nav Flags (used with navFlags field of FrmNavHeaderType)
//*********************************************************
#if PALMOS_SDK_VERSION < 0x0541

typedef UInt32	FrmNavHeaderFlagsType;

#define	kFrmNavHeaderFlagsObjectFocusStartState		  0x00000001
#define	kFrmNavHeaderFlagsAppFocusStartState		  0x00000002
#define kFrmNavHeaderFlagsAutoGenerated				  0x80000000

#define kFrmNavHeaderFlagsStartStateMask			  0x00000003
#define	kFrmNavHeaderFlagsDefaultStartStateValue	  0x00000000
#define	kFrmNavHeaderFlagsObjectFocusStartStateValue  0x00000001
#define	kFrmNavHeaderFlagsAppFocusStartStateValue	  0x00000002
#define kFrmNavHeaderFlagsInvalidStartStateValue	  0x00000003


//*********************************************************
// Object Flags (used with objectFlags field of
//	FrmNavOrderEntryType)
//*********************************************************
typedef UInt16	FrmNavObjectFlagsType;

#define kFrmNavObjectFlagsSkip					0x0001
#define	kFrmNavObjectFlagsForceInteractionMode	0x0002
#define	kFrmNavObjectFlagsIsBigButton			0x8000


//*********************************************************
// Nav State Flags (used with stateFlags parameter of
//	FrmGetNavState and FrmSetNavState API functions)
//*********************************************************
typedef UInt32	FrmNavStateFlagsType;

#define kFrmNavStateFlagsInteractionMode		0x00000001
#define kFrmNavStateFlagsObjectFocusMode		0x00000002


//*********************************************************
// The current version of the navigation structures
//  (FrmNavOrderEntryType and FrmNavHeaderType)
//*********************************************************
#define kFrmNavInfoVersion						1

#endif // PALMOS_SDK_VERSION < 0x0541


#if 0
#pragma mark -------- Incs:Core:System:SystemMgr.h -----------
#endif

#if PALMOS_SDK_VERSION < 0x0541

#define sysFtrNumFiveWayNavVersion		32			// version of the 5-way nav if any

#endif // PALMOS_SDK_VERSION < 0x0541


#if 0
#pragma mark --------  Public Structures   ----------------
#endif

#if PALMOS_SDK_VERSION < 0x0541

//*********************************************************
// Structures used with Navigation API functions
//*********************************************************

typedef struct FrmNavOrderEntryTag
{
    UInt16				  objectID;
    FrmNavObjectFlagsType objectFlags;
    UInt16				  aboveObjectID;
    UInt16				  belowObjectID;
} FrmNavOrderEntryType;


typedef struct FrmNavHeaderTag
{
    UInt16				  version;				  // This is version 1
	UInt16				  numberOfObjects;
	UInt16				  headerSizeInBytes;	  // 20 for the version 1 structure
	UInt16				  listElementSizeInBytes; // 8 for the version 1 structure
    FrmNavHeaderFlagsType navFlags;
    UInt16				  initialObjectIDHint;
    UInt16				  jumpToObjectIDHint;
    UInt16				  bottomLeftObjectIDHint;
	UInt16				  padding1;
} FrmNavHeaderType;

#endif // PALMOS_SDK_VERSION < 0x0541

#endif	  // __HS_NAV_COMMON_H__


