/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: IMCUtils.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *      Routines to handle Internet Mail Consortium specs
 *
 *****************************************************************************/

#ifndef __IMCUTILS_H__
#define __IMCUTILS_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#ifndef EOF
#define EOF	0xffff
#endif


// Constants for some common IMC spec values.
#define parameterDelimeterChr			';'
#define valueDelimeterChr				':'
#define groupDelimeterChr				'.'
#define paramaterNameDelimiterChr	'='
#define endOfLineChr 0x0D
#define imcLineSeparatorString		"\015\012"
#define imcFilenameLength				32
#define imcUnlimitedChars				0xFFFE		// 64K, minus 1 character for null

// These are for functions called to handle input and output.  These are currently used
// to allow disk based or obx based transfers
typedef UInt16 GetCharF (const void *);
typedef void PutStringF(void *, const Char * const stringP);

#ifdef __cplusplus
extern "C" {
#endif

// maxChars does NOT include trailing null, buffer may be 1 larger.
// use imcUnlimitedChars if you don't want a max.
extern Char * ImcReadFieldNoSemicolon(void *inputStream, 
	GetCharF inputFunc, UInt16 *c, const UInt16 maxChars)
							SYS_TRAP(sysTrapImcReadFieldNoSemicolon);

// maxChars does NOT include trailing null, buffer may be 1 larger.
// use imcUnlimitedChars if you don't want a max.
extern Char * ImcReadFieldQuotablePrintable(void *inputStream, GetCharF inputFunc, UInt16 *c, 
	const Char stopAt, const Boolean quotedPrintable, const UInt16 maxChars)
							SYS_TRAP(sysTrapImcReadFieldQuotablePrintable);
	
extern void ImcReadPropertyParameter(void *inputStream, GetCharF inputFunc,
										UInt16 *cP, Char *nameP, Char *valueP)
							SYS_TRAP(sysTrapImcReadPropertyParameter);
	
extern void ImcSkipAllPropertyParameters(void *inputStream, GetCharF inputFunc, 
	UInt16 *cP, Char *identifierP, Boolean *quotedPrintableP)
							SYS_TRAP(sysTrapImcSkipAllPropertyParameters);
	
extern void ImcReadWhiteSpace(void *inputStream, GetCharF inputFunc, 
	const UInt16 *const charAttrP, UInt16 *c)
							SYS_TRAP(sysTrapImcReadWhiteSpace);
	
extern void ImcWriteQuotedPrintable(void *outputStream, PutStringF outputFunc, 
	const Char *stringP, const Boolean noSemicolons)
							SYS_TRAP(sysTrapImcWriteQuotedPrintable);
	
extern void ImcWriteNoSemicolon(void *outputStream, PutStringF outputFunc, 
	const Char * const stringP)
							SYS_TRAP(sysTrapImcWriteNoSemicolon);
	
extern Boolean ImcStringIsAscii(const Char * const stringP)
							SYS_TRAP(sysTrapImcStringIsAscii);

#ifdef __cplusplus 
}
#endif

#endif	// _IMC_UTILS_H
