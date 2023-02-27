/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: StringMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		String manipulation functions
 *
 *****************************************************************************/

#ifndef __STRINGMGR_H__
#define __STRINGMGR_H__


// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>				// Trap Numbers.
#ifdef PALMOS
#if EMULATION_LEVEL == EMULATION_NONE
#  define _Palm_va_list char *
#else
#  define _Palm_va_list sys_va_list
#  include <stdarg.h>
#endif
#else
#  define _Palm_va_list sys_va_list
#  include <stdarg.h>
#endif

// Max length of string returned by StrIToA, for -2147483647, plus space
// for the terminating null.
#define	maxStrIToALen	12

#ifdef __cplusplus
extern "C" {
#endif

// String Manipulation routines
Char *	StrCopy(Char *dst, const Char *src)
							SYS_TRAP(sysTrapStrCopy);

Char *	StrNCopy(Char *dst, const Char *src, Int16 n)
							SYS_TRAP(sysTrapStrNCopy);

Char *	StrCat(Char *dst, const Char *src)
							SYS_TRAP(sysTrapStrCat);
							
Char *	StrNCat(Char *dst, const Char *src, Int16 n)
							SYS_TRAP(sysTrapStrNCat);
							
UInt16	StrLen(const Char *src)
							SYS_TRAP(sysTrapStrLen);
							
Int16		StrCompareAscii(const Char *s1, const Char *s2)
							SYS_TRAP(sysTrapStrCompareAscii);

Int16		StrCompare(const Char *s1, const Char *s2)
							SYS_TRAP(sysTrapStrCompare);

Int16		StrNCompareAscii(const Char *s1, const Char *s2, Int32 n)
							SYS_TRAP(sysTrapStrNCompareAscii);

Int16		StrNCompare(const Char *s1, const Char *s2, Int32 n)
							SYS_TRAP(sysTrapStrNCompare);

Int16 	StrCaselessCompare(const Char *s1, const Char *s2)
							SYS_TRAP(sysTrapStrCaselessCompare);

Int16		StrNCaselessCompare(const Char *s1, const Char *s2, Int32 n)
							SYS_TRAP(sysTrapStrNCaselessCompare);

Char *	StrToLower(Char *dst, const Char *src)
							SYS_TRAP(sysTrapStrToLower);

Char *	StrIToA(Char *s, Int32 i)
							SYS_TRAP(sysTrapStrIToA);

Char *	StrIToH(Char *s, UInt32 i)
							SYS_TRAP(sysTrapStrIToH);

Char *	StrLocalizeNumber(Char *s, Char thousandSeparator, Char decimalSeparator)
							SYS_TRAP(sysTrapStrLocalizeNumber);

Char *	StrDelocalizeNumber(Char *s, Char thousandSeparator, Char decimalSeparator)
							SYS_TRAP(sysTrapStrDelocalizeNumber);

Char *	StrChr (const Char *str, WChar chr)
							SYS_TRAP(sysTrapStrChr);

Char *	StrStr (const Char *str, const Char *token)
							SYS_TRAP(sysTrapStrStr);

Int32		StrAToI (const Char *str)
							SYS_TRAP(sysTrapStrAToI);
							
Int16 	StrPrintF(Char *s, const Char *formatStr, ...)
							SYS_TRAP(sysTrapStrPrintF);
							
Int16 	StrVPrintF(Char *s, const Char *formatStr, _Palm_va_list arg)
							SYS_TRAP(sysTrapStrVPrintF);

#ifdef __cplusplus 
}
#endif




#endif //__STRINGMGR_H
