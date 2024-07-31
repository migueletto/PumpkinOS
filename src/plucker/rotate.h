/*
 * $Id: rotate.h,v 1.10 2003/08/11 02:31:57 prussar Exp $
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

#ifndef PLUCKER_ROTATE_H
#define PLUCKER_ROTATE_H

#include "viewer.h"
#include "prefsdata.h"
#include "hires.h"
#ifdef HAVE_JOGDIAL
#include "jogdial.h"
#endif
#include "grayfont.h"

#ifdef HAVE_ROTATE

/* Draw a string, rotating as needed */
extern void RotDrawChars( Char* string, Int16 length, Coord x, Coord y )
    ROTATE_SECTION;

/* Draw a string, rotating as needed */
extern void RotDrawInvertedChars( Char* string, Int16 length, Coord x,
    Coord y ) ROTATE_SECTION;

extern void RotSetClip( RectangleType* rect ) ROTATE_SECTION;

extern void RotEraseRectangle( RectangleType* rect, UInt16 cornerDiam )
     ROTATE_SECTION;

extern void RotInvertRectangle( RectangleType* rect, UInt16 cornerDiam )
     ROTATE_SECTION;

extern void RotDrawRectangle( RectangleType* rect, UInt16 cornerDiam )
     ROTATE_SECTION;

extern void RotDrawRectangleFrame( FrameType frame, RectangleType* rect )
     ROTATE_SECTION;

extern void RotDrawGrayRectangleFrame( FrameType frame, RectangleType* rect )
     ROTATE_SECTION;

extern void RotScrollRectangle( RectangleType* rP, WinDirectionType direction,
       Coord distance, RectangleType* vacatedP ) ROTATE_SECTION;

extern Boolean RotSupportDrawBitmapNow( void ) ROTATE_SECTION;

extern void RotDrawBitmap ( BitmapType* bitmap, Coord x, Coord y )
       ROTATE_SECTION;

extern WinHandle RotCreateOffscreenWindow ( Coord width, Coord height,
       WindowFormatType format, UInt16* error ) ROTATE_SECTION;

extern Coord RotExtentX( void ) ROTATE_SECTION;

extern Coord RotExtentY( void ) ROTATE_SECTION;

extern Coord RotTopLeftX( void ) ROTATE_SECTION;

extern Coord RotTopLeftY( void ) ROTATE_SECTION;

extern void RotateRectangleInPlace( RectangleType* rect ) ROTATE_SECTION;

extern void RotDrawLine( Coord x, Coord y, Coord dx, Coord dy )
    ROTATE_SECTION;

extern void RotDrawGrayLine( Coord x, Coord y, Coord dx, Coord dy )
    ROTATE_SECTION;

extern Coord RotMaxExtentX( void ) ROTATE_SECTION;

extern Coord RotMaxExtentY( void ) ROTATE_SECTION;

/* only render characters with top<= y < top+extent */
extern void RotCharClipRange( Coord top, Coord extent ) ROTATE_SECTION;

extern void RotCharClearClipRange( void ) ROTATE_SECTION;

extern Coord RotGetScrollValue( void ) ROTATE_SECTION;

/* rotate coordinates */
extern void RotFromScreenXY( Coord* x, Coord* y ) ROTATE_SECTION;

#ifdef BUILD_ARMLETS
extern void RotOpenArmlet( void ) ROTATE_SECTION;

extern void RotCloseArmlet( void ) ROTATE_SECTION;
#else
#define RotOpenArmlet()
#define RotCloseArmlet()
#endif

/* rotate arrow keys */
extern void RotateArrowKeys( WChar* key ) ROTATE_SECTION;

extern Boolean RotIsJogdialUp( WChar key ) ROTATE_SECTION;

extern Boolean RotIsJogdialDown( WChar key ) ROTATE_SECTION;

extern Boolean RotIsJogdialPushUp( WChar key ) ROTATE_SECTION;

extern Boolean RotIsJogdialPushDown( WChar key ) ROTATE_SECTION;

#define RotSelect(zero,plus,minus) ( Prefs()->rotate == ROTATE_ZERO ? ( zero ) : \
                                     ( Prefs()->rotate == ROTATE_PLUS90 ? ( plus ) : \
                                       ( minus ) ) )

#else /* HAVE_ROTATE */

#define RotDrawChars         WinDrawChars
#define RotDrawInvertedChars WinDrawInvertedChars
#define RotSetClip           WinSetClip
#define RotEraseRectangle    WinEraseRectangle
#define RotInvertRectangle   WinInvertRectangle
#define RotDrawRectangle     WinDrawRectangle
#define RotDrawRectangleFrame WinDrawRectangleFrame
#define RotDrawGrayRectangleFrame WinDrawGrayRectangleFrame
#define RotScrollRectangle   WinScrollRectangle
#define RotCreateOffscreenWindow WinCreateOffscreenWindow
#define RotExtentX           ExtentX
#define RotExtentY           ExtentY
#define RotTopLeftX          TopLeftX
#define RotTopLeftY          TopLeftY
#define RotateRectangleInPlace(rP)
#define RotDrawLine          WinDrawLine
#define RotDrawGrayLine      WinDrawGrayLine
#define RotMaxExtentX        MaxExtentX
#define RotMaxExtentY        MaxExtentY
#define RotGetScrollValue    GetScrollValue
#define RotCharClipRange(top,extent)
#define RotCharClearClipRange()
#define RotFromScreenXY(x,y)
#define RotOpenArmlet()
#define RotCloseArmlet()
#define RotateArrowKeys(kP)
#define RotDrawBitmap        WinDrawBitmap
#define RotSupportDrawBitmapNow() true

#define RotSelect(zero,plus,minus)  zero

#ifdef HAVE_JOGDIAL
#define RotIsJogdialUp       IsJogdialUp
#define RotIsJogdialDown     IsJogdialDown
#define RotIsJogdialPushUp   IsJogdialPushUp
#define RotIsJogdialPushDown IsJogdialPushDown
#endif

#endif /* HAVE_ROTATE */

#endif /* PLUCKER_ROTATE_H */
