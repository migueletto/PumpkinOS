/******************************************************************************
 *
 * Copyright (c) 1999-2003 PalmSource, Inc. All rights reserved.
 *
 * File: FntGlue.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Glue providing compatibility for applications that wish
 *		to make calls to the Font Mgr, but which might actually be running
 *		on a system which does not support newer calls.
 *
 *****************************************************************************/

#ifndef __FNTGLUE_H__
#define __FNTGLUE_H__

#include <Font.h>

enum fontDefaults {
	defaultSmallFont = 0,
	defaultLargeFont,
	defaultBoldFont,
	defaultSystemFont
};
typedef enum fontDefaults FontDefaultType;

#ifdef __cplusplus
	extern "C" {
#endif

FontID FntGlueGetDefaultFontID(FontDefaultType inFontType);

Boolean FntGlueTruncateString(char* iDstString, const char* iSrcString, FontID iFont,
	Coord iMaxWidth, Boolean iAddEllipsis);
	
Int16 FntGlueWCharWidth(WChar iChar);

Int16 FntGlueWidthToOffset(const Char* charsP, UInt16 length, Int16 pixelWidth,
			Boolean* leadingEdge, Int16* truncWidth);

#ifdef __cplusplus
	}
#endif

#endif	// __FNTGLUE_H__
