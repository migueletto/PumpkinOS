/*
 * $Id: hires.h,v 1.38 2004/04/29 01:52:05 prussar Exp $
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

#ifndef PLUCKER_HIRES_H
#define PLUCKER_HIRES_H

#include "prefsdata.h"

#include "viewer.h"

#ifdef HAVE_HANDERA_SDK
#include <Vga.h>
#endif

typedef enum {
    unknownHiRes = 0x00,
    noHiRes      = 0x01,
    sonyHiRes    = 0x02,  /* Sony HiRes OS5 devices are handled by palmHiRes */
    handeraHiRes = 0x04,
    palmHiRes    = 0x08
} HiRes;

#define NOINIT   false
#define INIT     true
#define NATIVE   kCoordinatesNative
#define STANDARD kCoordinatesStandard

#ifdef HAVE_HIRES

extern void TranslateHiResPenEvents ( EventType* event ) HIRES_SECTION;
extern Boolean ConvertImageToV3( BitmapType** imagePtr )
            HIRES_SECTION;
extern void GeneralWinDrawBitmap( BitmapType* bitmap, Coord x, Coord y )
            HIRES_SECTION;
extern void SetHiResFunctions( void ) HIRES_SECTION;
extern Err HiResInitialize ( void ) HIRES_SECTION;
extern HiRes HiResType ( void ) HIRES_SECTION;
extern void HiResAdjustCurrentToNative ( Coord* i ) HIRES_SECTION;
extern void HiResAdjust ( Coord* i, UInt16 type ) HIRES_SECTION;
extern void HiResAdjustBounds ( RectangleType* r, UInt16 type ) HIRES_SECTION;
extern UInt16 PalmGetDensity( void ) HIRES_SECTION;
extern UInt16 SonyHiResRefNum ( void ) HIRES_SECTION;
extern Int16 PalmTopLeftX( void ) HIRES_SECTION;
extern Int16 PalmTopLeftY( void ) HIRES_SECTION;
extern Int16 PalmExtentX( void ) HIRES_SECTION;
extern Int16 PalmExtentY( void ) HIRES_SECTION;
extern UInt16 PalmGetCoordinateSystem( void ) HIRES_SECTION;
extern UInt16 PalmSetCoordinateSystem( UInt16 coordSys ) HIRES_SECTION;
extern UInt16 PalmStandardCoordFrmAlert( UInt16 alertID ) HIRES_SECTION;
extern UInt16 PalmStandardCoordFrmCustomAlert( UInt16 alertID, const Char* s1,
           const Char* s2, const Char* s3 ) HIRES_SECTION;
extern Int16 NonPalmHiResTopLeftX( void ) HIRES_SECTION;
extern Int16 NonPalmHiResTopLeftY( void ) HIRES_SECTION;
extern Int16 NonPalmHiResExtentX( void ) HIRES_SECTION;
extern Int16 NonPalmHiResExtentY( void ) HIRES_SECTION;
extern void HanderaConvertFontList( FontID* oldFontList,
           FontID* newFontList ) HIRES_SECTION;
extern UInt16 HiResFontImage( FontID font, UInt16 stdImage,
           UInt16 palmHalfImage, UInt16 nonPalmDoubleImage ) HIRES_SECTION;
extern FontID HiResFont( FontID font ) HIRES_SECTION;
extern void HiResAdjustNativeToCurrent( Coord* x ) HIRES_SECTION;

/* If type is set to unknownHiRes, it is treated as a valid hires setting up
 * until the point that it is proven NOT to be valid. That is why it is not
 * included in the IsHiResTypeNone() definition. See comments in screen.c */
#define IsHiResTypeNone( type )          ( ( type & noHiRes ) != 0 )
#define IsHiResTypePalm( type )          ( ( type & palmHiRes ) != 0 )
#define IsHiResTypeSony( type )          ( ( type & sonyHiRes ) != 0 )
#define IsHiResTypeHandera( type )       ( ( type & handeraHiRes ) != 0 )

#if 0
#define FrmAlert( objID ) \
    ( ( IsHiResTypePalm( HiResType() ) ) ? \
      PalmStandardCoordFrmAlert( objID ) : \
      FrmAlert( objID ) )

#define FrmCustomAlert( alertID, s1, s2, s3 ) \
    ( ( IsHiResTypePalm( HiResType() ) ) ? \
      PalmStandardCoordFrmCustomAlert( alertID, s1, s2, s3 ) : \
      FrmCustomAlert( alertID, s1, s2, s3 ) )
#endif

#ifdef HAVE_SONY_SDK

void HiResCopyRectangle( WinHandle srcWin, WinHandle dstWin, RectangleType* srcRect,
     Coord destX, Coord destY, WinDrawOperation mode ) HIRES_SECTION;
void HiResDrawChars( const char* chars, UInt16 len, Coord x, Coord y )
     HIRES_SECTION;
void HiResDrawBitmap( BitmapType* bitmapP, Coord x, Coord y ) HIRES_SECTION;

#define WinClipRectangle( rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinClipRectangle( SonyHiResRefNum(), rP ) : \
    WinClipRectangle( rP ) )

#define WinCopyRectangle HiResCopyRectangle

#define WinCreateBitmapWindow( bitmapP, error ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinCreateBitmapWindow( SonyHiResRefNum(), bitmapP, error ) : \
    WinCreateBitmapWindow( bitmapP, error ) )

#define WinCreateOffscreenWindow( width, height, format, error ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinCreateOffscreenWindow( SonyHiResRefNum(), width, height, format, error ) : \
    WinCreateOffscreenWindow( width, height, format, error ) )

#define WinCreateWindow( bounds, frame, modal, focusable, error ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinCreateWindow( SonyHiResRefNum(), bounds, frame, modal, focusable, error ) : \
    WinCreateWindow( bounds, frame, modal, focusable, error ) )

#define WinDisplayToWindowPt( extentX, extentY ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDisplayToWindowPt( SonyHiResRefNum(), extentX, extentY ) : \
    WinDisplayToWindowPt( extentX, extentY ) )

#define WinDrawBitmap HiResDrawBitmap

#define WinDrawChar( theChar, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawChar( SonyHiResRefNum(), theChar, x, y ) : \
    WinDrawChar( theChar, x, y ) )

#define WinDrawChars HiResDrawChars

#define WinDrawGrayLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawGrayLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinDrawGrayLine( x1, y1, x2, y2 ) )

#define WinDrawGrayRectangleFrame( frame, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawGrayRectangleFrame( SonyHiResRefNum(), frame, rP ) : \
    WinDrawGrayRectangleFrame( frame, rP ) )

#define WinDrawInvertedChars( chars, len, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawInvertedChars( SonyHiResRefNum(), chars, len, x, y ) : \
    WinDrawInvertedChars( chars, len, x, y ) )

#define WinDrawLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinDrawLine( x1, y1, x2, y2 ) )

#define WinDrawPixel( x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawPixel( SonyHiResRefNum(), x, y ) : \
    WinDrawPixel( x, y ) )

#define WinDrawRectangle( rP, cornerDiam ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawRectangle( SonyHiResRefNum(), rP, cornerDiam ) : \
    WinDrawRectangle( rP, cornerDiam ) )

#define WinDrawRectangleFrame( frame, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawRectangleFrame( SonyHiResRefNum(), frame, rP ) : \
    WinDrawRectangleFrame( frame, rP ) )

#define WinDrawTruncChars( chars, len, x, y, maxWidth ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinDrawTruncChars( SonyHiResRefNum(), chars, len, x, y, maxWidth ) : \
    WinDrawTruncChars( chars, len, x, y, maxWidth ) )

#define WinEraseChars( chars, len, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinEraseChars( SonyHiResRefNum(), chars, len, x, y ) : \
    WinEraseChars( chars, len, x, y ) )

#define WinEraseLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinEraseLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinEraseLine( x1, y1, x2, y2 ) )

#define WinErasePixel( x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinErasePixel( SonyHiResRefNum(), x, y ) : \
    WinErasePixel( x, y ) )

#define WinEraseRectangle( rP, cornerDiam ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinEraseRectangle( SonyHiResRefNum(), rP, cornerDiam ) : \
    WinEraseRectangle( rP, cornerDiam ) )

#define WinEraseRectangleFrame( frame, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinEraseRectangleFrame( SonyHiResRefNum(), frame, rP ) : \
    WinEraseRectangleFrame( frame, rP ) )

#define WinFillLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinFillLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinFillLine( x1, y1, x2, y2 ) )

#define WinFillRectangle( rP, cornerDiam ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinFillRectangle( SonyHiResRefNum(), rP, cornerDiam ) : \
    WinFillRectangle( rP, cornerDiam ) )

#define WinGetClip( rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetClip( SonyHiResRefNum(), rP ) : \
    WinGetClip( rP ) )

#define WinGetDisplayExtent( extentX, extentY ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetDisplayExtent( SonyHiResRefNum(), extentX, extentY ) : \
    WinGetDisplayExtent( extentX, extentY ) )

#define WinGetFramesRectangle( frame, rP, obscuredRect ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetFramesRectangle( SonyHiResRefNum(), frame, rP, obscuredRect ) : \
    WinGetFramesRectangle( frame, rP, obscuredRect ) )

#define WinGetPixel( x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetPixel( SonyHiResRefNum(), x, y ) : \
    WinGetPixel( x, y ) )

#define WinGetPixelRGB( x, y, rgbP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetPixelRGB( SonyHiResRefNum(), x, y, rgbP ) : \
    WinGetPixelRGB( x, y, rgbP ) )

#define WinGetDrawWindowBounds( rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetWindowBounds( SonyHiResRefNum(), rP ) : \
    WinGetDrawWindowBounds( rP ) )

#define WinGetWindowExtent( extentX, extentY ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetWindowExtent( SonyHiResRefNum(), extentX, extentY ) : \
    WinGetWindowExtent( extentX, extentY ) )

#define WinGetWindowFrameRect( winHandle, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinGetWindowFrameRect( SonyHiResRefNum(), winHandle, rP ) : \
    WinGetWindowFrameRect( winHandle, rP ) )

#define WinInvertChars( chars, len, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinInvertChars( SonyHiResRefNum(), chars, len, x, y ) : \
    WinInvertChars( chars, len, x, y ) )

#define WinInvertLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinInvertLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinInvertLine( x1, y1, x2, y2 ) )

#define WinInvertPixel( x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinInvertPixel( SonyHiResRefNum(), x, y ) : \
    WinInvertPixel( x, y ) )

#define WinInvertRectangle( rP, cornerDiam ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinInvertRectangle( SonyHiResRefNum(), rP, cornerDiam ) : \
    WinInvertRectangle( rP, cornerDiam ) )

#define WinInvertRectangleFrame( frame, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinInvertRectangleFrame( SonyHiResRefNum(), frame, rP ) : \
    WinInvertRectangleFrame( frame, rP ) )

#define WinPaintBitmap( bitmapP, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintBitmap( SonyHiResRefNum(), bitmapP, x, y ) : \
    WinPaintBitmap( bitmapP, x, y ) )

#define WinPaintChar( theChar, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintChar( SonyHiResRefNum(), theChar, x, y ) : \
    WinPaintChar( theChar, x, y ) )

#define WinPaintChars( chars, len, x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintChars( SonyHiResRefNum(), chars, len, x, y ) : \
    WinPaintChars( chars, len, x, y ) )

#define WinPaintLine( x1, y1, x2, y2 ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintLine( SonyHiResRefNum(), x1, y1, x2, y2 ) : \
    WinPaintLine( x1, y1, x2, y2 ) )

#define WinPaintPixel( x, y ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintPixel( SonyHiResRefNum(), x, y ) : \
    WinPaintPixel( x, y ) )

#define WinPaintPixels( numPoints, pts ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintPixel( SonyHiResRefNum(), numPoints, pts ) : \
    WinPaintPixel( numPoints, pts ) )

#define WinPaintRectangle( rP, cornerDiam ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintRectangle( SonyHiResRefNum(), rP, cornerDiam ) : \
    WinPaintRectangle( rP, cornerDiam ) )

#define WinPaintRectangleFrame( frame, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinPaintRectangleFrame( SonyHiResRefNum(), frame, rP ) : \
    WinPaintRectangleFrame( frame, rP ) )

#define WinRestoreBits( winHandle, destX, destY ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinRestoreBits( SonyHiResRefNum(), winHandle, destX, destY ) : \
    WinRestoreBits( winHandle, destX, destY ) )

#define WinSaveBits( sourceP, error ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinSaveBits( SonyHiResRefNum(), sourceP, error ) : \
    WinSaveBits( sourceP, error ) )

#define WinScreenMode( operation, widthP, heightP, depthP, enableColorP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinScreenMode( SonyHiResRefNum(), operation, widthP, heightP, depthP, enableColorP ) : \
    WinScreenMode( operation, widthP, heightP, depthP, enableColorP ) )

#define WinScrollRectangle( rP, direction, distance, vacatedP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinScrollRectangle( SonyHiResRefNum(), rP, direction, distance, vacatedP ) : \
    WinScrollRectangle( rP, direction, distance, vacatedP ) )

#define WinSetClip( rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinSetClip( SonyHiResRefNum(), rP ) : \
    WinSetClip( rP ) )

#define WinSetDrawWindowBounds( winHandle, rP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinSetDrawWindowBounds( SonyHiResRefNum(), winHandle, rP ) : \
    WinSetDrawWindowBounds( winHandle, rP ) )

#define WinWindowToDisplayPt( extentX, extentY ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRWinWindowToDisplayPt( SonyHiResRefNum(), extentX, extentY ) : \
    WinWindowToDisplayPt( extentX, extentY ) )

#define BmpBitsSize( bitmapP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRBmpBitsSize( SonyHiResRefNum(), bitmapP ) : \
    BmpBitsSize( bitmapP ) )

#define BmpSize( bitmapP ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRBmpSize( SonyHiResRefNum(), bitmapP ) : \
    BmpSize( bitmapP ) )

#define BmpCreate( width, height, depth, colorTableP, error ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRBmpCreate( SonyHiResRefNum(), width, height, depth, colorTableP, error ) : \
    BmpCreate( width, height, depth, colorTableP, error ) )

#define FntGetFont() \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRFntGetFont( SonyHiResRefNum() ) : \
    FntGetFont( ) )

#define FntSetFont( font ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRFntSetFont( SonyHiResRefNum(), font ) : \
    FntSetFont( font ) )

#define FontSelect( font ) \
    ( IsHiResTypeSony( HiResType() ) ? \
    HRFontSelect( SonyHiResRefNum(), font ) : \
    FontSelect( font ) )

#endif /* HAVE_SONY_SDK */

#else

#define TranslateHiResPenEvents( a )
#define ConvertImageToV3( a )           false
#define SetHiResFunctions()             SetStandardFunctions()
#define HiResInitialize()               errNoHiRes
#define HiResType()                     noHiRes
#define HiResStop()                     (void) errNoHiRes
#define HiResFont( a )                  a
#define HiResAdjust( a, b )
#define HiResAdjustCurrentToNative( a )  
#define HiResAdjustBounds( a, b )
#define SonyHiResRefNum()               NULL
#define PalmGetCoordinateSystem()       STANDARD
#define PalmSetCoordinateSystem( a ) StandardCoordinateSystem()
#define HanderaConvertFontList( a, b )
#define HiResFontImage( f, a, b, c )    a
#define IsHiResTypeNone( type )         true
#define IsHiResTypePalm( type )         false
#define IsHiResTypeSony( type )         false
#define IsHiResTypeHandera( type )      false
#define GeneralWinDrawBitmap            WinDrawBitmap
#define PalmGetDensity()                72
#define HiResAdjustNativeToCurrent( a )

#endif /* HAVE_HIRES */

PLKR_GLOBAL Err (*HiResStop)( void );

#endif

