/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: OverlayMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Public header for routines that support overlays.
 *
 *****************************************************************************/

#ifndef	__OVERLAYMGR_H__
#define	__OVERLAYMGR_H__

// Include elementary types
#include <PalmTypes.h>
#include <DataMgr.h>
#include <LocaleMgr.h>				// For LmLocaleType

/***********************************************************************
 * Overlay Manager constants
 **********************************************************************/

#define	omOverlayRscType		'ovly'	// Overlay desc resource type
#define	omOverlayRscID			1000		// Overlay desc resource ID

#define	omFtrCreator			'ovly'	// For get/set of Overlay features.
#define	omFtrShowErrorsFlag	0			// Boolean - True => display overlay errors.
#define	omFtrDefaultLocale	1			// LmLocaleType record => default locale to
													// try with stripped bases & no valid overlay.
													
// OmFindOverlayDatabase called with stripped base, and no appropriate overlay was found.
#define	omErrBaseRequiresOverlay	(omErrorClass | 1)

// OmOverlayDBNameToLocale or OmLocaleToOverlayDBName were passed an unknown locale.
#define	omErrUnknownLocale			(omErrorClass | 2)

// OmOverlayDBNameToLocale was passed a poorly formed string.
#define	omErrBadOverlayDBName		(omErrorClass | 3)

// OmGetIndexedLocale was passed an invalid index.
#define	omErrInvalidLocaleIndex		(omErrorClass | 4)	

// OmSetSystemLocale was passed an invalid locale (doesn't correspond to available
// system overlay).
#define	omErrInvalidLocale			(omErrorClass | 5)

// OmSetSystemLocale was passed a locale that referenced an invalid system overlay
// (missing one or more required resources)
#define	omErrInvalidSystemOverlay	(omErrorClass | 6)

// OmGetNextSystemLocale was called, but there were no more valid system
// locales to return.
#define	omErrNoNextSystemLocale		(omErrorClass | 7)

/***********************************************************************
 * Selectors & macros used for calling Overlay Manager routines
 **********************************************************************/

#ifdef DIRECT_OVERLAY_CALLS
	#define	OMDISPATCH_TRAP(omSelectorNum)
#else
	#define	OMDISPATCH_TRAP(sel) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapOmDispatch, sel)
#endif

// Selectors used for getting to the right Overlay Manager routine via
// the OmDispatch trap.

#define omInit							0
#define omOpenOverlayDatabase		1
#define omLocaleToOverlayDBName	2
#define omOverlayDBNameToLocale	3
#define omGetCurrentLocale			4
#define omGetIndexedLocale			5
#define omGetSystemLocale			6
#define omSetSystemLocale			7
#define omGetRoutineAddress		8
#define omGetNextSystemLocale		9

#define omMaxSelector				omGetNextSystemLocale

typedef UInt16 OmSelector;

/***********************************************************************
 * Overlay Manager types
 **********************************************************************/


typedef LmLocaleType OmLocaleType;

// Structure passed to OmGetNextSystemLocale.
typedef struct {
	DmSearchStateType	searchState;
	DmOpenRef			systemDBRef;
	UInt16				systemDBCard;
	Char 					systemDBName[dmDBNameLength];
	Int16					systemDBNameLen;
	LmLocaleType		curLocale;
	Boolean				didNoOverlaySystem;
	Boolean				foundSystem;
	UInt8					reserved[6];
} OmSearchStateType;
	
/***********************************************************************
 * Overlay Manager routines
 **********************************************************************/

#ifdef __cplusplus
	extern "C" {
#endif

// Return in <overlayDBName> an overlay database name that's appropriate
// for the base name <baseDBName> and the locale <targetLocale>. If the
// <targetLocale> param in NULL, use the current locale. The <overlayDBName>
// buffer must be at least dmDBNameLength bytes.
Err OmLocaleToOverlayDBName(const Char *baseDBName, const LmLocaleType *targetLocale,
									Char *overlayDBName)
			OMDISPATCH_TRAP(omLocaleToOverlayDBName);

// Given the name of an overlay database in <overlayDBName>, return back
// the overlay in overlayLocale. If the name isn't an overlay name,
// return omErrBadOverlayDBName.
Err OmOverlayDBNameToLocale(const Char *overlayDBName, LmLocaleType *overlayLocale)
			OMDISPATCH_TRAP(omOverlayDBNameToLocale);

// Return the current locale in <currentLocale>. This may not be the same as
// the system locale, which will take effect after the next reset.
void OmGetCurrentLocale(LmLocaleType *currentLocale)
			OMDISPATCH_TRAP(omGetCurrentLocale);

//	Return the nth valid system locale in <theLocale>. Indexes are zero-based,
// and omErrInvalidLocaleIndex will be returned if <localeIndex> is out of
// bounds. Note that OmGetNextSystemLocale should be used on Palm OS 4.0 or
// later, since OmGetIndexedLocale can be slow on ROMs with more than few
// valid system locales.
Err OmGetIndexedLocale(UInt16 localeIndex, LmLocaleType *theLocale)
			OMDISPATCH_TRAP(omGetIndexedLocale);

// Return the system locale in <systemLocale>. This may not be the same as
// the current locale. WARNING!!! This routine should only be used in very
// special situations; typically OmGetCurrentLocale should be used to determine
// the "active" locale.
void OmGetSystemLocale(LmLocaleType *systemLocale)
			OMDISPATCH_TRAP(omGetSystemLocale);

//	Set the post-reset system locale to be <systemLocale>. Return omErrInvalidLocale if
// the passed locale doesn’t correspond to a valid System.prc overlay.
Err OmSetSystemLocale(const LmLocaleType *systemLocale)
			OMDISPATCH_TRAP(omSetSystemLocale);

// Return back the address of the routine indicated by <inSelector>. If
// <inSelector> isn't a valid routine selector, return back NULL.
void *OmGetRoutineAddress(OmSelector inSelector)
			OMDISPATCH_TRAP(omGetRoutineAddress);

// NEW in 4.0. Return back the next valid system locale in <oLocaleP>. The first
// time the routine is called, <iNewSearch> must be true. When there are no more
// valid system locales, omErrInvalidLocaleIndex will be returned. This routine
// should be used in place of OmGetIndexedLocale on Palm OS 4.0 or later, since
// it's much faster.
Err OmGetNextSystemLocale(Boolean iNewSearch, OmSearchStateType* ioStateInfoP, LmLocaleType* oLocaleP)
			OMDISPATCH_TRAP(omGetNextSystemLocale);
			

#ifdef __cplusplus
	}
#endif

#endif
