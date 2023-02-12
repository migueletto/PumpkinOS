/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IntlGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *         	Header file for IntlGlue library routines.
 *
 * IntlGlue provides compatibility for applications that wish to make
 *	calls to Text Manager, but which might actually be running on devices
 *	with roms that do not have this support available, in which case the glue
 *	code executes the standard (Latin only) routines instead. This library
 *	can only be used on post-1.0 roms!!!
 *
 * Written by TransPac Software, Inc.
 *
 *****************************************************************************/

#ifndef __INTLGLUE_H__
#define __INTLGLUE_H__

#include <IntlMgr.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef _BUILDING_TXTLATIN
extern void
	TxtLatinByteAttr(), TxtLatinCharAttr(), TxtLatinCharXAttr(),
	TxtLatinCharSize(), TxtLatinCharWidth(),
	TxtLatinGetPreviousChar(), TxtLatinGetNextChar(), TxtLatinGetChar(),
	TxtLatinSetNextChar(), TxtLatinReplaceStr(), TxtLatinParamString(),
	TxtLatinCharBounds(), TxtLatinGetTruncationOffset(), TxtLatinFindString(),
	TxtLatinWordBounds(), TxtLatinGetWordWrapOffset(), TxtLatinCharEncoding(),
	TxtLatinStrEncoding(), TxtLatinMaxEncoding(), TxtLatinEncodingName(),
	TxtLatinNameToEncoding(), TxtLatinTransliterate(), TxtLatinCharIsValid(),
	TxtLatinCaselessCompare(), TxtLatinCompare(), TxtLatinPrepFindString();
#endif

// IntlGlueGetRoutineAddress must be passed the desired selector (from the
// list in IntlMgr.h), just as with IntlGetRoutineAddress.  You must also
// pass the corresponding latinSymbol from the list above.  Being passed
// as an argument to IntlGlueGetRoutineAddress is the only useful use for
// these symbols -- don't try to do anything else with them. 
//
// If the Int'l Mgr (and the appropriate Text Mgr routines) exists, then
// the result will be the same as calling IntlGetRoutineAddress() with the
// appropriate routine selector. If Text Mgr support is not available, then
// this call returns the address of the corresponding glue code routine.
//
// Note that the address returned is only valid so long as the application
// stays locked in memory, thus this routine should be called at or after
// your StartApplication routine, and only used up to the point where your
// application terminates.
//
// (In previous SDKs, there was an IntlGlueGetRoutineAddress function which
// took different parameters.  It was never documented, and has been
// replaced by this one.)

void *IntlGlueGetRoutineAddress(IntlSelector selector, const void *latinSymbol);

#ifdef __cplusplus
	}
#endif

#endif // __INTLGLUE_H__
