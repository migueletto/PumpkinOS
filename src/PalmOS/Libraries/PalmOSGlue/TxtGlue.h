/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TxtGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for TxtGlue library routines.
 *
 * TxtGlue provides compatibility for applications that wish to make
 *	calls to Text Manager, but which might actually be running on devices
 *	with roms that do not have this support available, in which case the glue
 *	code executes the standard (Latin only) routines instead. This library
 *	can only be used on post-1.0 roms!!!
 *
 *****************************************************************************/

#ifndef __TXTGLUE_H__
#define __TXTGLUE_H__

#include <TextMgr.h>

// Duplicate versions of all of the macros in TextMgr.h that call the library
// routines instead:

#define TxtGlueCharIsSpace(ch)		((TxtGlueCharAttr(ch) & charAttrSpace) != 0)
#define TxtGlueCharIsPrint(ch)		((TxtGlueCharAttr(ch) & charAttrPrint) != 0)
#define TxtGlueCharIsDigit(ch)      ((TxtGlueCharAttr(ch) & charAttr_DI) != 0)
#define TxtGlueCharIsAlNum(ch)		((TxtGlueCharAttr(ch) & charAttrAlNum) != 0)
#define TxtGlueCharIsAlpha(ch)		((TxtGlueCharAttr(ch) & charAttrAlpha) != 0)
#define TxtGlueCharIsCntrl(ch)		((TxtGlueCharAttr(ch) & charAttrCntrl) != 0)
#define TxtGlueCharIsGraph(ch)		((TxtGlueCharAttr(ch) & charAttrGraph) != 0)
#define TxtGlueCharIsLower(ch)      ((TxtGlueCharAttr(ch) & charAttr_LO) != 0)
#define TxtGlueCharIsPunct(ch)      ((TxtGlueCharAttr(ch) & charAttr_PU) != 0)
#define TxtGlueCharIsUpper(ch)      ((TxtGlueCharAttr(ch) & charAttr_UP) != 0)
#define TxtGlueCharIsHex(ch)        ((TxtGlueCharAttr(ch) & charAttr_XD) != 0)
#define TxtGlueCharIsDelim(ch)		((TxtGlueCharAttr(ch) & charAttrDelim) != 0)

#define	TxtGluePreviousCharSize(inText, inOffset)	TxtGlueGetPreviousChar((inText), (inOffset), NULL)
#define	TxtGlueNextCharSize(inText, inOffset)		TxtGlueGetNextChar((inText), (inOffset), NULL)

#ifdef __cplusplus
	extern "C" {
#endif

UInt8 TxtGlueByteAttr(UInt8 inByte);

UInt16 TxtGlueCharAttr(WChar inChar);

UInt16 TxtGlueCharXAttr(WChar inChar);

UInt16 TxtGlueCharSize(WChar inChar);

Int16 TxtGlueCharWidth(WChar inChar);

UInt16 TxtGlueGetPreviousChar(const Char* inText, UInt32 inOffset, WChar* outChar);

UInt16 TxtGlueGetNextChar(const Char* inText, UInt32 inOffset, WChar* outChar);

WChar TxtGlueGetChar(const Char* inText, UInt32 inOffset);

UInt16 TxtGlueReplaceStr(Char* ioStr, UInt16 inMaxLen, const Char* inParamStr, UInt16 inParamNum);

UInt16 TxtGlueSetNextChar(Char* ioText, UInt32 inOffset, WChar inChar);

WChar TxtGlueCharBounds(const Char* inText, UInt32 inOffset, UInt32* outStart, UInt32* outEnd);

Boolean TxtGlueFindString(const Char* inSourceStr, const Char* inTargetStr,
			UInt32* outPos, UInt16* outLength);

Boolean TxtGlueWordBounds(const Char* inText, UInt32 inLength, UInt32 inOffset,
			UInt32* outStart, UInt32* outEnd);

CharEncodingType TxtGlueCharEncoding(WChar inChar);

CharEncodingType TxtGlueStrEncoding(const Char* inStr);

CharEncodingType TxtGlueMaxEncoding(CharEncodingType a, CharEncodingType b);

const Char* TxtGlueEncodingName(CharEncodingType inEncoding);

Err TxtGlueTransliterate(const Char* inSrcText, UInt16 inSrcLength, Char* outDstText,
			UInt16* ioDstLength, TranslitOpType inOp);

void TxtGlueUpperStr(Char* ioString, UInt16 inMaxLength);

void TxtGlueLowerStr(Char* ioString, UInt16 inMaxLength);

WChar TxtGlueUpperChar(WChar inChar);

WChar TxtGlueLowerChar(WChar inChar);

UInt32 TxtGlueGetTruncationOffset(const Char* inText, UInt32 inOffset);

Boolean TxtGlueTruncateString(Char* ioStringP, UInt16 inMaxWidth);

Boolean TxtGlueCharIsValid(WChar inChar);

Int16 TxtGlueCaselessCompare(const Char* s1, UInt16 s1Len, UInt16* s1MatchLen,
			const Char* s2, UInt16 s2Len, UInt16* s2MatchLen);

Int16 TxtGlueCompare(const Char* s1, UInt16 s1Len, UInt16* s1MatchLen,
			const Char* s2, UInt16 s2Len, UInt16* s2MatchLen);

WChar TxtGlueGetNumericSpaceChar(void);

WChar TxtGlueGetHorizEllipsisChar(void);

Boolean TxtGlueCharIsVirtual(UInt16 inModifiers, WChar inChar);

Char* TxtGlueStripSpaces(Char* ioStr, Boolean leading, Boolean trailing);

Char* TxtGlueParamString(const Char* inTemplate, const Char* param0,
			const Char* param1, const Char* param2, const Char* param3);

void TxtGluePrepFindString(const Char* inSrcP, Char* outDstP, UInt16 inDstSize);

Err TxtGlueConvertEncoding(Boolean newConversion, TxtConvertStateType* ioStateP,
			const Char* srcTextP, UInt16* ioSrcBytes, CharEncodingType srcEncoding,
			Char* dstTextP, UInt16* ioDstBytes, CharEncodingType dstEncoding,
			const Char* substitutionStr, UInt16 substitutionLen);

#ifdef __cplusplus
	}
#endif

#endif	// __TXTGLUE_H__
