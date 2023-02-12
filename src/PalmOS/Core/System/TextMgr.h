/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TextMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header file for Text Manager.
 *
 *****************************************************************************/

#ifndef __TEXTMGR_H__
#define __TEXTMGR_H__

#include <IntlMgr.h>
#include <Chars.h>

/***********************************************************************
 * Public types & constants
 ***********************************************************************/

// See PalmLocale.h for encoding constants of type CharEncodingType, and
// character encoding names.
typedef UInt8 CharEncodingType;

// Transliteration operations for the TxtTransliterate call. We don't use
// an enum, since each character encoding contains its own set of special
// transliteration operations (which begin at translitOpCustomBase).
typedef UInt16 TranslitOpType;

// Standard transliteration operations.
#define	translitOpStandardBase	0			// Beginning of standard operations.

#define	translitOpUpperCase		0
#define	translitOpLowerCase		1
#define	translitOpReserved2		2
#define	translitOpReserved3		3

// Custom transliteration operations (defined in CharXXXX.h encoding-specific
// header files.
#define	translitOpCustomBase		1000		// Beginning of char-encoding specific ops.

#define	translitOpPreprocess		0x8000	// Mask for pre-process option, where
														// no transliteration actually is done.

// Structure used to maintain state across calls to TxtConvertEncoding, for
// proper handling of source or destination encodings with have modes.
#define	kTxtConvertStateSize		32

typedef struct {
	UInt8		ioSrcState[kTxtConvertStateSize];
	UInt8		ioDstState[kTxtConvertStateSize];
} TxtConvertStateType;

// Character encoding assumed for substitution text by TxtConvertEncoding
#define textSubstitutionEncoding	charEncodingUTF8

// Flag to OR with the charEncodingType that is passed to TxtConvertEncoding
#define charEncodingDstBestFitFlag	0x80

// Flags available in the sysFtrNumCharEncodingFlags feature attribute.
#define	charEncodingOnlySingleByte	0x00000001
#define	charEncodingHasDoubleByte	0x00000002
#define	charEncodingHasLigatures	0x00000004
#define	charEncodingRightToLeft		0x00000008

// Various byte attribute flags. Note that multiple flags can be
// set, thus a byte could be both a single-byte character, or the first
// byte of a multi-byte character.
#define	byteAttrFirst				0x80	// First byte of multi-byte char.
#define	byteAttrLast				0x40	// Last byte of multi-byte char.
#define	byteAttrMiddle				0x20	// Middle byte of muli-byte char.
#define	byteAttrSingle				0x01	// Single byte.

// Some double-byte encoding combinations. Every byte in a stream of
// double-byte data must be either a single byte, a single/low byte,
// or a high/low byte.
#define byteAttrSingleLow		(byteAttrSingle | byteAttrLast)
#define byteAttrHighLow			(byteAttrFirst | byteAttrLast)

// Character attribute flags. These replace the old flags defined in
// CharAttr.h, but are bit-compatible.
#define	charAttr_XA		0x0200 	// extra alphabetic
#define	charAttr_XS		0x0100 	// extra space
#define	charAttr_BB		0x0080 	// BEL, BS, etc.
#define	charAttr_CN		0x0040 	// CR, FF, HT, NL, VT
#define	charAttr_DI		0x0020 	// '0'-'9'
#define	charAttr_LO		0x0010 	// 'a'-'z' and lowercase extended chars.
#define	charAttr_PU		0x0008 	// punctuation
#define	charAttr_SP		0x0004 	// space
#define	charAttr_UP		0x0002 	// 'A'-'Z' and uppercase extended chars.
#define	charAttr_XD		0x0001 	// '0'-'9', 'A'-'F', 'a'-'f'

// Various sets of character attribute flags.
#define	charAttrPrint				(charAttr_DI|charAttr_LO|charAttr_PU|charAttr_SP|charAttr_UP|charAttr_XA)
#define	charAttrSpace				(charAttr_CN|charAttr_SP|charAttr_XS)
#define	charAttrAlNum				(charAttr_DI|charAttr_LO|charAttr_UP|charAttr_XA)
#define	charAttrAlpha				(charAttr_LO|charAttr_UP|charAttr_XA)
#define	charAttrCntrl				(charAttr_BB|charAttr_CN)
#define	charAttrGraph				(charAttr_DI|charAttr_LO|charAttr_PU|charAttr_UP|charAttr_XA)
#define	charAttrDelim				(charAttr_SP|charAttr_PU)

// Remember that sizeof(0x0D) == 2 because 0x0D is treated like an int. The
// same is true of sizeof('a'), sizeof('\0'), and sizeof(chrNull). For this
// reason it's safest to use the sizeOf7BitChar macro to document buffer size
// and string length calcs. Note that this can only be used with low-ascii
// characters, as anything else might be the high byte of a double-byte char.
#define	sizeOf7BitChar(c)	1

// Maximum size a single WChar character will occupy in a text string.
#define	maxCharBytes				3

// Text manager error codes.
#define	txtErrUknownTranslitOp				(txtErrorClass | 1)
#define	txtErrTranslitOverrun				(txtErrorClass | 2)
#define	txtErrTranslitOverflow				(txtErrorClass | 3)
#define	txtErrConvertOverflow				(txtErrorClass | 4)
#define	txtErrConvertUnderflow				(txtErrorClass | 5)
#define	txtErrUnknownEncoding				(txtErrorClass | 6)
#define	txtErrNoCharMapping					(txtErrorClass | 7)
#define	txtErrTranslitUnderflow				(txtErrorClass | 8)
#define	txtErrMalformedText					(txtErrorClass | 9)
#define	txtErrUnknownEncodingFallbackCopy	(txtErrorClass | 10)

/***********************************************************************
 * Public macros
 ***********************************************************************/

#define	TxtCharIsSpace(ch)		((TxtCharAttr(ch) & charAttrSpace) != 0)
#define	TxtCharIsPrint(ch)		((TxtCharAttr(ch) & charAttrPrint) != 0)
#define	TxtCharIsDigit(ch)		((TxtCharAttr(ch) & charAttr_DI) != 0)
#define	TxtCharIsAlNum(ch)		((TxtCharAttr(ch) & charAttrAlNum) != 0)
#define	TxtCharIsAlpha(ch)		((TxtCharAttr(ch) & charAttrAlpha) != 0)
#define	TxtCharIsCntrl(ch)		((TxtCharAttr(ch) & charAttrCntrl) != 0)
#define	TxtCharIsGraph(ch)		((TxtCharAttr(ch) & charAttrGraph) != 0)
#define	TxtCharIsLower(ch)		((TxtCharAttr(ch) & charAttr_LO) != 0)
#define	TxtCharIsPunct(ch)		((TxtCharAttr(ch) & charAttr_PU) != 0)
#define	TxtCharIsUpper(ch)		((TxtCharAttr(ch) & charAttr_UP) != 0)
#define	TxtCharIsHex(ch)			((TxtCharAttr(ch) & charAttr_XD) != 0)
#define	TxtCharIsDelim(ch)		((TxtCharAttr(ch) & charAttrDelim) != 0)

// <c> is a hard key if the event modifier <m> has the command bit set
// and <c> is either in the proper range or is the calculator character.
#define	TxtCharIsHardKey(m, c)	((((m) & commandKeyMask) != 0) && \
								((((c) >= hardKeyMin) && ((c) <= hardKeyMax)) || ((c) == calcChr)))

// <c> is a virtual character if the event modifier <m> has the command
// bit set. WARNING!!! This macro is only safe to use on Palm OS 3.5 or
// later. With earlier versions of the OS, use TxtGlueCharIsVirtual()
// in PalmOSGlue.lib
#define	TxtCharIsVirtual(m, c)	(((m) & commandKeyMask) != 0)

#define	TxtPreviousCharSize(inText, inOffset)	TxtGetPreviousChar((inText), (inOffset), NULL)
#define	TxtNextCharSize(inText, inOffset)		TxtGetNextChar((inText), (inOffset), NULL)


/***********************************************************************
 * Public routines
 ***********************************************************************/

#ifndef STRIP_FUNCTION_HEADERS

#ifdef __cplusplus
	extern "C" {
#endif


// Return back byte attribute (first, last, single, middle) for <inByte>.

UInt8 TxtByteAttr(UInt8 inByte)
		INTL_TRAP(intlTxtByteAttr);

// Return back the standard attribute bits for <inChar>.

UInt16 TxtCharAttr(WChar inChar)
		INTL_TRAP(intlTxtCharAttr);

// Return back the extended attribute bits for <inChar>.

UInt16 TxtCharXAttr(WChar inChar)
		INTL_TRAP(intlTxtCharXAttr);

// Return the size (in bytes) of the character <inChar>. This represents
// how many bytes would be required to store the character in a string.

UInt16 TxtCharSize(WChar inChar)
		INTL_TRAP(intlTxtCharSize);

// Return the width (in pixels) of the character <inChar>. You should
// use FntWCharWidth or FntGlueWCharWidth instead of this routine.

Int16 TxtCharWidth(WChar inChar)
		INTL_TRAP(intlTxtCharWidth);

// Load the character before offset <inOffset> in the <inText> text. Return
// back the size of the character.

UInt16 TxtGetPreviousChar(const Char *inText, UInt32 inOffset, WChar *outChar)
		INTL_TRAP(intlTxtGetPreviousChar);

// Load the character at offset <inOffset> in the <inText> text. Return
// back the size of the character.

UInt16 TxtGetNextChar(const Char *inText, UInt32 inOffset, WChar *outChar)
		INTL_TRAP(intlTxtGetNextChar);

// Return the character at offset <inOffset> in the <inText> text.

WChar TxtGetChar(const Char *inText, UInt32 inOffset)
		INTL_TRAP(intlTxtGetChar);

// Set the character at offset <inOffset> in the <inText> text, and
// return back the size of the character.

UInt16 TxtSetNextChar(Char *ioText, UInt32 inOffset, WChar inChar)
		INTL_TRAP(intlTxtSetNextChar);

// Replace the substring "^X" (where X is 0..9, as specified by <inParamNum>)
// with the string <inParamStr>. If <inParamStr> is NULL then don't modify <ioStr>.
// Make sure the resulting string doesn't contain more than <inMaxLen> bytes,
// excluding the terminating null. Return back the number of occurances of
// the substring found in <ioStr>.

UInt16 TxtReplaceStr(Char *ioStr, UInt16 inMaxLen, const Char *inParamStr, UInt16 inParamNum)
		INTL_TRAP(intlTxtReplaceStr);

// Allocate a handle containing the result of substituting param0...param3
// for ^0...^3 in <inTemplate>, and return the locked result. If a parameter
// is NULL, replace the corresponding substring in the template with "".

Char *TxtParamString(const Char *inTemplate, const Char *param0,
			const Char *param1, const Char *param2, const Char *param3)
		INTL_TRAP(intlTxtParamString);

// Return the bounds of the character at <inOffset> in the <inText>
// text, via the <outStart> & <outEnd> offsets, and also return the
// actual value of character at or following <inOffset>.

WChar TxtCharBounds(const Char *inText, UInt32 inOffset, UInt32 *outStart, UInt32 *outEnd)
		INTL_TRAP(intlTxtCharBounds);

// Return the appropriate byte position for truncating <inText> such that it is
// at most <inOffset> bytes long.

UInt32 TxtGetTruncationOffset(const Char *inText, UInt32 inOffset)
		INTL_TRAP(intlTxtGetTruncationOffset);

// Search for <inTargetStr> in <inSourceStr>. If found return true and pass back
// the found position (byte offset) in <outPos>, and the length of the matched
// text in <outLength>.

Boolean TxtFindString(const Char *inSourceStr, const Char *inTargetStr,
			UInt32 *outPos, UInt16 *outLength)
		INTL_TRAP(intlTxtFindString);

// Find the bounds of the word that contains the character at <inOffset>.
// Return the offsets in <*outStart> and <*outEnd>. Return true if the
// word we found was not empty & not a delimiter (attribute of first char
// in word not equal to space or punct).

Boolean TxtWordBounds(const Char *inText, UInt32 inLength, UInt32 inOffset,
			UInt32 *outStart, UInt32 *outEnd)
		INTL_TRAP(intlTxtWordBounds);

// Return the offset of the first break position (for text wrapping) that
// occurs at or before <iOffset> in <iTextP>. Note that this routine will
// also add trailing spaces and a trailing linefeed to the break position,
// thus the result could be greater than <iOffset>.

UInt32 TxtGetWordWrapOffset(const Char *iTextP, UInt32 iOffset)
		INTL_TRAP(intlTxtGetWordWrapOffset);

// Return the minimum (lowest) encoding required for <inChar>. If we
// don't know about the character, return encoding_Unknown.

CharEncodingType TxtCharEncoding(WChar inChar)
		INTL_TRAP(intlTxtCharEncoding);

// Return the minimum (lowest) encoding required to represent <inStr>.
// This is the maximum encoding of any character in the string, where
// highest is unknown, and lowest is ascii.

CharEncodingType TxtStrEncoding(const Char *inStr)
		INTL_TRAP(intlTxtStrEncoding);

// Return the higher (max) encoding of <a> and <b>.

CharEncodingType TxtMaxEncoding(CharEncodingType a, CharEncodingType b)
		INTL_TRAP(intlTxtMaxEncoding);

// Return a pointer to the 'standard' name for <inEncoding>. If the
// encoding is unknown, return a pointer to an empty string.

const Char *TxtEncodingName(CharEncodingType inEncoding)
		INTL_TRAP(intlTxtEncodingName);

// Map from a character set name <iEncodingName> to a CharEncodingType.
// If the character set name is unknown, return charEncodingUnknown.

CharEncodingType TxtNameToEncoding(const Char* iEncodingName)
		INTL_TRAP(intlTxtNameToEncoding);

// Transliterate <inSrcLength> bytes of text found in <inSrcText>, based
// on the requested <inOp> operation. Place the results in <outDstText>,
// and set the resulting length in <ioDstLength>. On entry <ioDstLength>
// must contain the maximum size of the <outDstText> buffer. If the
// buffer isn't large enough, return an error (note that outDestText
// might have been modified during the operation). Note that if <inOp>
// has the preprocess bit set, then <outDstText> is not modified, and
// <ioDstLength> will contain the total space required in the destination
// buffer in order to perform the operation. 

Err TxtTransliterate(const Char *inSrcText, UInt16 inSrcLength, Char *outDstText,
			UInt16 *ioDstLength, TranslitOpType inOp)
		INTL_TRAP(intlTxtTransliterate);

// Convert <*ioSrcBytes> of text from <srcTextP> between the <srcEncoding>
// and <dstEncoding> character encodings. If <dstTextP> is not NULL, write
// the resulting bytes to the buffer, and always return the number of
// resulting bytes in <*ioDstBytes>. Update <*srcBytes> with the number of
// bytes from the beginning of <*srcTextP> that were successfully converted.
// When the routine is called with <srcTextP> pointing to the beginning of
// a string or text buffer, <newConversion> should be true; if the text is
// processed in multiple chunks, either because errors occurred or due to
// source/destination buffer size constraints, then subsequent calls to
// this routine should pass false for <newConversion>. The TxtConvertStateType
// record maintains state information so that if the source or destination
// character encodings have state or modes (e.g. JIS), processing a single
// sequence of text with multiple calls will work correctly.

// When an error occurs due to an unconvertable character, the behavior of
// the routine will depend on the <substitutionStr> parameter. If it is NULL,
// then <*ioSrcBytes> will be set to the offset of the unconvertable character,
// <ioDstBytes> will be set to the number of successfully converted resulting
// bytes, and <dstTextP>, in not NULL, will contain conversion results up to
// the point of the error. The routine will return an appropriate error code,
// and it is up to the caller to either terminate conversion or skip over the
// unconvertable character and continue the conversion process (passing false
// for the <newConversion> parameter in subsequent calls to TxtConvertEncoding).
// If <substitutionStr> is not NULL, then this string is written to the
// destination buffer when an unconvertable character is encountered in the
// source text, and the source character is skipped. Processing continues, though
// the error code will still be returned when the routine terminates. Note that
// if a more serious error occurs during processing (e.g. buffer overflow) then
// that error will be returned even if there was an earlier unconvertable character.
// Note that the substitution string must use the destination character encoding.

Err TxtConvertEncoding(Boolean newConversion, TxtConvertStateType* ioStateP,
			const Char* srcTextP, UInt16* ioSrcBytes, CharEncodingType srcEncoding,
			Char* dstTextP, UInt16* ioDstBytes, CharEncodingType dstEncoding,
			const Char* substitutionStr, UInt16 substitutionLen)
		INTL_TRAP(intlTxtConvertEncoding);

// Return true if <inChar> is a valid (drawable) character. Note that we'll
// return false if it is a virtual character code.

Boolean TxtCharIsValid(WChar inChar)
		INTL_TRAP(intlTxtCharIsValid);

// Compare the first <s1Len> bytes of <s1> with the first <s2Len> bytes
// of <s2>. Return the results of the comparison: < 0 if <s1> sorts before
// <s2>, > 0 if <s1> sorts after <s2>, and 0 if they are equal. Also return
// the number of bytes that matched in <s1MatchLen> and <s2MatchLen>
// (either one of which can be NULL if the match length is not needed).
// This comparison is "caseless", in the same manner as a find operation,
// thus case, character size, etc. don't matter.

Int16 TxtCaselessCompare(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen,
			const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen)
		INTL_TRAP(intlTxtCaselessCompare);

// Compare the first <s1Len> bytes of <s1> with the first <s2Len> bytes
// of <s2>. Return the results of the comparison: < 0 if <s1> sorts before
// <s2>, > 0 if <s1> sorts after <s2>, and 0 if they are equal. Also return
// the number of bytes that matched in <s1MatchLen> and <s2MatchLen>
// (either one of which can be NULL if the match length is not needed).

Int16 TxtCompare(const Char *s1, UInt16 s1Len, UInt16 *s1MatchLen,
			const Char *s2, UInt16 s2Len, UInt16 *s2MatchLen)
		INTL_TRAP(intlTxtCompare);

#ifdef __cplusplus
	}
#endif

#endif // !STRIP_FUNCTION_HEADERS

#endif // __TEXTMGR_H__
