/******************************************************************************
 *
 * Copyright (c) 1994-1999 Palm Computing, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: CharAttr.h
 *
 * Description:
 *        This file defines character classification and character
 *        conversion macros
 *
 * History:
 *		April 21, 1995	Created by Art Lamb
 *
 *****************************************************************************/

#ifndef __CHARATTR_H__
#define __CHARATTR_H__

#define NON_INTERNATIONAL

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>					// Trap Numbers.

// Remember that sizeof(0x0D) == 2 because 0x0D is treated like an int. The
// same is true of sizeof('a'), sizeof('\0'), and sizeof(chrNull). For this
// reason it's safest to use the sizeOf7BitChar macro to document buffer size
// and string length calcs. Note that this can only be used with low-ascii
// characters, as anything else might be the high byte of a double-byte char.

#define	sizeOf7BitChar(c)	1

#ifdef NON_INTERNATIONAL
#define sizeofchar(c)	sizeof((char) (c))
#define lastAsciiChr			0x00FF
#else
#define	sizeofchar(c)	_Obsolete__use_sizeOf7BitChar
#define	lastAsciiChr	_Obsolete__lastAsciiChr_does_not_work_for_Japanese
#endif

// Character attribute code bits.

#define _XA		0x0200 	// extra alphabetic
#define _XS		0x0100 	// extra space
#define _BB		0x0080 	// BEL, BS, etc.
#define _CN		0x0040 	// CR, FF, HT, NL, VT
#define _DI		0x0020 	// '0'-'9'
#define _LO		0x0010 	// 'a'-'z' and lowercase extended chars.
#define _PU		0x0008 	// punctuation
#define _SP		0x0004 	// space
#define _UP		0x0002 	// 'A'-'Z' and uppercase extended chars.
#define _XD		0x0001 	// '0'-'9', 'A'-'F', 'a'-'f'


// These macros have all been deprecated and replaced by corresponding TxtCharXXXX
// macros found in TextMgr.h. The main problem is that these all assume 8-bit character
// codes, and thus won't work with Shift JIS and other multi-byte encodings.

// Character classification macros.

#ifdef NON_INTERNATIONAL
#define IsAscii(c)			(c <= 255)
#define IsAlNum(attr,c)		(attr[(UInt8)(c)] & (_DI|_LO|_UP|_XA))
#define IsAlpha(attr,c)		(attr[(UInt8)(c)] & (_LO|_UP|_XA))
#define IsCntrl(attr,c)		(attr[(UInt8)(c)] & (_BB|_CN))
#define IsDigit(attr,c)		(attr[(UInt8)(c)] & _DI)
#define IsGraph(attr,c)		(attr[(UInt8)(c)] & (_DI|_LO|_PU|_UP|_XA))
#define IsLower(attr,c)		(attr[(UInt8)(c)] & _LO)
#define IsPrint(attr,c)		(attr[(UInt8)(c)] & (_DI|_LO|_PU|_SP|_UP|_XA))
#define IsPunct(attr,c)		(attr[(UInt8)(c)] & _PU)
#define IsSpace(attr,c)		(attr[(UInt8)(c)] & (_CN|_SP|_XS))
#define IsUpper(attr,c)		(attr[(UInt8)(c)] & _UP)
#define IsHex(attr,c)		(attr[(UInt8)(c)] & _XD)
#define IsDelim(attr,c)		(attr[(UInt8)(c)] & _SP|_PU)
#else
#define IsAscii(c)			_Obsolete__use_TxtCharIsValid
#define IsAlNum(attr,c)		_Obsolete__use_TxtCharIsAlNum
#define IsAlpha(attr,c)		_Obsolete__use_TxtCharIsAlpha
#define IsCntrl(attr,c)		_Obsolete__use_TxtCharIsCntrl
#define IsDigit(attr,c)		_Obsolete__use_TxtCharIsDigit
#define IsGraph(attr,c)		_Obsolete__use_TxtCharIsGraph
#define IsLower(attr,c)		_Obsolete__use_TxtCharIsLower
#define IsPrint(attr,c)		_Obsolete__use_TxtCharIsPrint
#define IsPunct(attr,c)		_Obsolete__use_TxtCharIsPunct
#define IsSpace(attr,c)		_Obsolete__use_TxtCharIsSpace
#define IsUpper(attr,c)		_Obsolete__use_TxtCharIsUpper
#define IsHex(attr,c)		_Obsolete__use_TxtCharIsHex
#define IsDelim(attr,c)		_Obsolete__use_TxtCharIsDelim
#endif

// This macro is deprecated because it relies on character code ranges, versus checking
// to ensure that the keydown event has the command bit set in the modifiers field. Use
// the TxtCharIsHardKey macro found in TextMgr.h.

#ifdef NON_INTERNATIONAL
#define ChrIsHardKey(c)		((((c) >= hardKeyMin) && ((c) <= hardKeyMax)) || ((c) == calcChr))
#else
#define ChrIsHardKey(c)		_Obsolete__use_TxtCharIsHardKey
#endif

#ifdef __cplusplus
extern "C" {
#endif

// In 3.1 and later versions of Palm OS, these routines have all been replaced by new
// Text Manager routines found in TextMgr.h

#ifdef NON_INTERNATIONAL
const UInt16 *GetCharAttr (void)
			SYS_TRAP(sysTrapGetCharAttr);

const UInt8 *GetCharSortValue (void)
			SYS_TRAP(sysTrapGetCharSortValue);

const UInt8 *GetCharCaselessValue (void)
			SYS_TRAP(sysTrapGetCharCaselessValue);
#else
#define	GetCharAttr()				_Obsolete__use_TxtCharIs_macros
#define	GetCharSortValue()		_Obsolete__use_TxtCompare
#define	GetCharCaselessValue()	_Obsolete__use_TxtCaselessCompare
#endif

#ifdef __cplusplus 
}
#endif


#endif  /* __CHARATTR_H__ */

