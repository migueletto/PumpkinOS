/*
 * $Id: grayfont.h,v 1.10 2004/02/10 02:10:44 prussar Exp $
 *
 * The true home of this code is palmfontconv.sourceforge.net.  If you
 * find it elsewhere and improve it, please send a patch to
 * Alexander R. Pruss <ap85@georgetown.edu>.
 *
 * palmfontconv - Copyright (c) 2003, Alexander R. Pruss
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

#ifndef GRAYFONT_H
#define GRAYFONT_H

#include "config.h"
#include "viewer.h"
#include "hires.h"

#define GRAY_FONT_LEFT   'L'
#define GRAY_FONT_RIGHT  'R'
#define GRAY_FONT_NORMAL 'U'

#ifndef GRAYFONT_SECTION
# define GRAYFONT_SECTION
#endif

#ifdef HAVE_GRAY_FONT
/* Start the gray-scale font functions */
void GrayFntStart( void ) GRAYFONT_SECTION;

/* Stop them and clear memory */
void GrayFntStop( void ) GRAYFONT_SECTION;

Err GrayFntDefineFont ( FontID font, void*  fontP ) GRAYFONT_SECTION;

FontID GrayFntGetFont( void ) GRAYFONT_SECTION;

FontID GrayFntSetFont ( FontID font ) GRAYFONT_SECTION;

Int16 GrayFntLineHeight( void ) GRAYFONT_SECTION;

Int16 GrayFntCharHeight( void ) GRAYFONT_SECTION;

Int16 GrayFntWCharWidth ( WChar ch ) GRAYFONT_SECTION;

Int16 GrayFntCharWidth ( Char ch ) GRAYFONT_SECTION;

Int16 GrayFntCharsWidth ( Char const* chars, Int16 length ) GRAYFONT_SECTION;

Int16 GrayFntWidthToOffset ( Char* const chars, UInt16 length, Int16
      pixelWidth, Boolean* leadingEdge, Int16* truncWidth ) GRAYFONT_SECTION;


Char GrayFntSetOrientation ( Char o ) GRAYFONT_SECTION;

void GrayWinDrawCharsGeneral ( Char const* chars, Int16 length, Coord x,
         Coord y, Boolean invert ) GRAYFONT_SECTION;

#define GrayWinDrawChars(c,l,x,y) GrayWinDrawCharsGeneral(c,l,x,y,false)
#define GrayWinDrawInvertedChars(c,l,x,y) GrayWinDrawCharsGeneral(c,l,x,y,true)

void GrayWinDrawChar ( WChar ch, Coord x, Coord y ) GRAYFONT_SECTION;

Boolean GrayFntIsCurrentGray ( void ) GRAYFONT_SECTION;

/* Handle font substitution */
void GrayFntSubstitute ( FontID oldFont, FontID newFont ) GRAYFONT_SECTION;

void GrayFntClearSubstitutionList( void ) GRAYFONT_SECTION;

void GrayFntSetBackgroundErase( Boolean state ) GRAYFONT_SECTION;

extern Coord GrayFntMinLeftKerning( FontID font ) GRAYFONT_SECTION;

extern Coord GrayFntMaxRightOverhang( FontID font ) GRAYFONT_SECTION;

#else /* HAVE_GRAY_FONT */

#include <FntGlue.h>

#define GrayFntMinLeftKerning( a ) 0
#define GrayFntMaxRightOverhang( a ) 0
#define GrayFntStart()
#define GrayFntStop()
#define GrayFntDefineFont  FntDefineFont
#define GrayFntGetFont     FntGetFont
#define GrayFntSetFont     FntSetFont
#define GrayFntLineHeight  FntLineHeight
#define GrayFntCharHeight  FntCharHeight
#define GrayFntCharsWidth  FntCharsWidth
#define GrayFntWCharHeight FntGlueWCharHeight
#define GrayFntWidthToOffset FntWidthToOffset
#define GrayFntSetOrientation( o ) GRAY_FONT_NORMAL
#define GrayWinDrawChars   WinDrawChars
#define GrayWinDrawInvertedChars   WinDrawInvertedChars
#define GrayWinDrawChar    WinDrawChar
#define GrayFntIsCurrentGray() false
#define GrayFntSubstitute( a, b )
#define GrayFntClearSubstitutionList()
#define GrayFntSetBackgroundErase( a )

#define NO_GRAY_FONT_SUBSTITUTION

#endif /* HAVE_GRAY_FONT */

#ifndef NO_GRAY_FONT_SUBSTITUTION
#define FntDefineFont  GrayFntDefineFont
#undef  FntGetFont
#define FntGetFont     GrayFntGetFont
#undef  FntSetFont
#define FntSetFont     GrayFntSetFont
#define FntLineHeight  GrayFntLineHeight
#define FntCharHeight  GrayFntCharHeight
#define FntWCharHeight GrayFntGlueWCharHeight
#define FntCharsWidth  GrayFntCharsWidth
#undef  WinDrawChars
#define WinDrawChars   GrayWinDrawChars
#undef  WinDrawInvertedChars
#define WinDrawInvertedChars   GrayWinDrawInvertedChars
#undef  WinDrawChar
#define WinDrawChar    GrayWinDrawChar
#define TxtGlueCharWidth GrayFntWCharWidth
#undef  FntWidthToOffset 
#define FntWidthToOffset GrayFntWidthToOffset
#undef  FntCharWidth
#define FntCharWidth     GrayFntCharWidth
#endif /* NO_GRAY_FONT_SUBSTITUTION */

#endif /* GRAYFONT_H */

