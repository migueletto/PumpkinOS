/*
 * $Id: xlit.h,v 1.4 2004/03/02 04:08:39 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef PLUCKER_XLIT_H
#define PLUCKER_XLIT_H

#include "viewer.h"

#ifdef SUPPORT_TRANSLITERATION

#define TRANSLITERATION_FLAG_8BIT                   1
#define TRANSLITERATION_FLAG_OUT_OF_RANGE_NO_CHANGE 2
#define TRANSLITERATION_FLAG_SYMMETRIC              4
#define TRANSLITERATION_NAME_LENGTH                 59

typedef struct {
    UInt32 version; /* currently only 1 is supported */
    UInt32 flags;   /* currently TRANSLITERATION_FLAG_8BIT must be set */
    Char   name[ TRANSLITERATION_NAME_LENGTH + 1 ];
    UInt16 firstGlyph;
    UInt16 lastGlyph;
    WChar  outOfRangeMapTo;
    UInt16 dataOffset;
} TransliterationHeader;

extern void OpenTransliterations( void ) XLIT_SECTION;

extern void CloseTransliterations( void ) XLIT_SECTION;

/* Set up xlatTable in accordance with transliteration number
   xlitNum.  Return value: Is the transliteration symmetric.
   *multibyteCharFixP is set to true iff the transliteration
   specifies a value to transliterate multibyte chars to, and
   if so then *multibyteCharFixTo is set to that value. */
extern Boolean SetTransliteration ( UInt16 xlitNum, Char* xlatTable, Boolean*
    multibyteCharFixP, WChar* multibyteCharFixTo ) XLIT_SECTION;

/* Setup transliteration popup on search form */
extern void SetupXlitPopup( void ) XLIT_SECTION;

#else /* SUPPORT_TRANSLITERATION */

#define OpenTransliterations()
#define CloseTransliterations()
#define SetupXlitPopup()

#endif /* SUPPORT_TRANSLITERATION */

#endif /* PLUCKER_XLIT_H */
