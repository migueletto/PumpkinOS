/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Graffiti.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header for the Graffiti interface
 *
 *****************************************************************************/

#ifndef __GRAFFITI_H__
#define __GRAFFITI_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <Rect.h>

/*------------------------------------------------------------------------------
 * Match info structure. Returned by GrfMatch and GrfMatchGlyph  
 *-----------------------------------------------------------------------------*/
typedef struct {
	UInt8	glyphID;							/* glyph ID of this match */
	UInt8	unCertainty;	            /* unCertainty of this match (0 most certain) */
	} GrfMatchType;	


#define grfMaxMatches	4
typedef struct {
	UInt16				numMatches;	      /* number of matches returned in this structure */
	GrfMatchType 	match[grfMaxMatches];	
	} GrfMatchInfoType;
typedef	GrfMatchInfoType*	GrfMatchInfoPtr;


//----------------------------------------------------------------------------
// Escape codes preceding special sequences in the dictionary or macros
//----------------------------------------------------------------------------
// In dictionary or macros preceding virtual key event sequences. These are always 
// 13 byte sequences that have ASCII encoded values for the ascii code, keyCode,
//   and modifiers: 
//   grfVirtualSequence, ascii,   keyCode,  modifiers.
//         1 byte        4 bytes   4 bytes   4 bytes          
#define	grfVirtualSequence	0x01

// In dictionary to tell us about temp shift state changes.
#define	grfShiftSequence		0x02

// In dictionary/macros to hide special features
#define	grfSpecialSequence	0x03


// Determine if a string has a sequence
#define HasVirtualSequence(s)		(s[0] == grfVirtualSequence)
#define HasSpecialSequence(s)		(s[0] == grfSpecialSequence)


/*------------------------------------------------------------------------------
 * Temp shift states, returned by GrfGetState
 *-----------------------------------------------------------------------------*/
#define	grfTempShiftPunctuation				1
#define	grfTempShiftExtended				2
#define	grfTempShiftUpper					3
#define	grfTempShiftLower					4


/*------------------------------------------------------------------------------
 * Macro (aka Shortcut) related constants/macros
 * Use the definitions in ShortcutLib.h instead!
 *-----------------------------------------------------------------------------*/

// Char indicating a seqeunce of characters to expand.
#define	grfExpansionSequence	'@'

// Chars indicating what to expand into
#define	expandDateChar			'D'
#define	expandTimeChar			'T'
#define	expandStampChar			'S'	//	This follows 'D' or 'T' for the sake
													//	of the mnemonic name.
#define HasExpansionSequence(s)	(s[0] == grfExpansionSequence)

// max shortcut name length
#define grfNameLength	8				// eight letters possible (don't forget CR)

// index which is not a shortcut
#define grfNoShortCut	0xffff

// Flags for the sysFtrCreator/sysFtrNumInputAreaFlags feature.
#define grfFtrInputAreaFlagDynamic				0x00000001
#define	grfFtrInputAreaFlagLiveInk				0x00000002
#define	grfFtrInputAreaFlagCollapsible			0x00000004
#define grfFtrInputAreaFlagLandscape            0x00000008
#define grfFtrInputAreaFlagReversePortrait      0x00000010
#define grfFtrInputAreaFlagReverseLandscape     0x00000020
#define grfFtrInputAreaFlagLefthanded           0x00000040


/************************************************************
 * Graffiti result codes
 *************************************************************/
#define	grfErrBadParam					(grfErrorClass | 1)
#define	grfErrPointBufferFull			(grfErrorClass | 2)
#define	grfErrNoGlyphTable				(grfErrorClass | 3)
#define	grfErrNoDictionary				(grfErrorClass | 4)
#define	grfErrNoMapping					(grfErrorClass | 5)
#define	grfErrMacroNotFound				(grfErrorClass | 6)
#define	grfErrDepthTooDeep				(grfErrorClass | 7)
#define	grfErrMacroPtrTooSmall			(grfErrorClass | 8)
#define	grfErrNoMacros					(grfErrorClass | 9)

#define	grfErrMacroIncomplete			(grfErrorClass | 129)  // (grfWarningOffset+1)
#define	grfErrBranchNotFound			(grfErrorClass | 130)  // (grfWarningOffset+2)

#define grfErrGenericHWRErrBase			(grfErrorClass | 16)
#define grfErrNoHWRInstalled			(grfErrGenericHWRErrBase)


/************************************************************
 * Graffiti interface procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {  
#endif


#ifdef BUILDING_GRAFFITI_EXT
#define GRF_TRAP(x)
#else
#define GRF_TRAP  SYS_TRAP
#endif


//-----------------------------------------------------------------
// High Level Calls
//------------------------------------------------------------------
Err		GrfInit (void)
				GRF_TRAP(sysTrapGrfInit);

Err		GrfFree (void)
				GRF_TRAP(sysTrapGrfFree);

Err		GrfBeginStroke(const PointType *startPtP, const RectangleType *boundsP, Boolean liveInk)
				GRF_TRAP(sysTrapGrfBeginStroke);

Err		GrfProcessStroke(const PointType *startPtP, const PointType *endPtP, Boolean upShift)
				GRF_TRAP(sysTrapGrfProcessStroke);

Err		GrfFieldChange(Boolean resetState, UInt16 *characterToDelete)
				GRF_TRAP(sysTrapGrfFieldChange);

Err		GrfGetState(Boolean *capsLockP, Boolean *numLockP, 
					UInt16 *tempShiftP, Boolean *autoShiftedP)
				GRF_TRAP(sysTrapGrfGetState);

Err		GrfSetState(Boolean capsLock, Boolean numLock, 
					Boolean upperShift)
				GRF_TRAP(sysTrapGrfSetState);


//-----------------------------------------------------------------
// Mid Level Calls
//------------------------------------------------------------------

Err 		GrfFlushPoints (void)
				GRF_TRAP(sysTrapGrfFlushPoints);

Err 		GrfAddPoint (PointType *pt)
				GRF_TRAP(sysTrapGrfAddPoint);

Err 		GrfInitState(void)
				GRF_TRAP(sysTrapGrfInitState);

Err 		GrfCleanState(void)
				GRF_TRAP(sysTrapGrfCleanState);

Err 		GrfMatch (UInt16 *flagsP, void *dataPtrP, UInt16 *dataLenP,
					UInt16 *uncertainLenP, GrfMatchInfoPtr matchInfoP)
				GRF_TRAP(sysTrapGrfMatch);

Err 		GrfGetMacro(Char *nameP, UInt8 *macroDataP,
					UInt16 *dataLenP)
				SYS_TRAP(sysTrapGrfGetMacro);

Err 		GrfGetAndExpandMacro(Char *nameP, UInt8 *macroDataP,
					UInt16 *dataLenP)
				SYS_TRAP(sysTrapGrfGetAndExpandMacro);


//-----------------------------------------------------------------
// Low Level Calls
//------------------------------------------------------------------
Err 		GrfFilterPoints (void)
				GRF_TRAP(sysTrapGrfFilterPoints);

Err 		GrfGetNumPoints(UInt16 *numPtsP)
				GRF_TRAP(sysTrapGrfGetNumPoints);

Err 		GrfGetPoint(UInt16 index, PointType *pointP)
				GRF_TRAP(sysTrapGrfGetPoint);

Err 		GrfFindBranch(UInt16 flags)
				GRF_TRAP(sysTrapGrfFindBranch);

Err 		GrfMatchGlyph (GrfMatchInfoPtr matchInfoP,
					Int16 maxUnCertainty, UInt16 maxMatches)
				GRF_TRAP(sysTrapGrfMatchGlyph);

Err 		GrfGetGlyphMapping (UInt16 glyphID, UInt16 *flagsP,
					void *dataPtrP, UInt16 *dataLenP, UInt16 *uncertainLenP)
				GRF_TRAP(sysTrapGrfGetGlyphMapping);

Err 		GrfGetMacroName(UInt16 index, Char *nameP)
				SYS_TRAP(sysTrapGrfGetMacroName);

Err 		GrfDeleteMacro(UInt16 index)
				SYS_TRAP(sysTrapGrfDeleteMacro);

Err 		GrfAddMacro(const Char *nameP, UInt8 *macroDataP,
					UInt16 dataLen)
				SYS_TRAP(sysTrapGrfAddMacro);




#ifdef __cplusplus
}
#endif


#endif //__SYSEVTMGR_H__
