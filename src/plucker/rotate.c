/*
 * $Id: rotate.c,v 1.30 2004/04/24 17:28:21 prussar Exp $
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

#include <PalmOS.h>
#include <BmpGlue.h>
#include <TxtGlue.h>
#include "resourceids.h"
#include "viewer.h"
#include "hires.h"
#include "prefsdata.h"
#include "util.h"
#include "os.h"
#include "rotatebitmap.h"
#include "const.h"
#include "fullscreenform.h"

#define NO_GRAY_FONT_SUBSTITUTION
#include "rotate.h"
#include "../libpit/debug.h"

#ifdef BUILD_ARMLETS
#include <PceNativeCall.h>
#include "armlets/rotatebitmap.h"
#endif

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
/* 0 */  Int16 fontType;
/* 2 */  Int16 firstChar;
/* 4 */  Int16 lastChar;
/* 6 */  Int16 maxWidth;
/* 8 */  Int16 kernMax;
/* a */  Int16 nDescent;
/* c */  Int16 fRectWidth;
/* e */  Int16 fRectHeight;
/* 10 */ Int16 owTLoc;
/* 12 */ Int16 ascent;
/* 14 */ Int16 descent;
/* 16 */ Int16 leading;
/* 18 */ Int16 rowWords;
} PalmFontType;



typedef struct {
    Int8 offset;
    Int8 width;
} PalmFontCharInfoType;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void GlueDrawPixel( Coord x, Coord y ) ROTATE_SECTION;
static void GlueErasePixel( Coord x, Coord y ) ROTATE_SECTION;
static UInt8 GetMaxBitDepthForBitmap( UInt32 numPixels ) ROTATE_SECTION;
static void RotDrawCharsGeneral( Char* string, Int16 length,
    Coord x, Coord y, Boolean invert ) ROTATE_SECTION;
static void RotateXY( Coord x, Coord y ) ROTATE_SECTION;
static void Rotatedxdy( Coord dx, Coord dy ) ROTATE_SECTION;
static void RotateRectangle( RectangleType* rect ) ROTATE_SECTION;
static Boolean InitializeRotation( void ) ROTATE_SECTION;
static Boolean InitializeOppositeRotation( void ) ROTATE_SECTION;
static void DrawRotatedText35( Char* string, Int16 length, Coord x,
       Coord y, Boolean invert, Coord charTop, Coord height )
       ROTATE_SECTION;
static void DrawRotatedTextPre35( Char* string, Int16 length, Coord x,
       Coord y, Boolean invert, Coord charTop, Coord height )
       ROTATE_SECTION;



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Coord  rotatedX;
static Coord  rotatedY;
static Coord  rotateddx;
static Coord  rotateddy;
static Coord  deltaX;
static Coord  deltaY;
static RectangleType  rotatedRect;

static Coord   rangeTopY;
static Coord   rangeExtentY;
static Boolean haveRange = false;
static Int16   multiplierY;
static Int16   multiplierX;
static Boolean clockwise;
#ifdef BUILD_ARMLETS
static void*         RotateBitmapArmlet = NULL;
static MemHandle     armChunkH = NULL;
#endif



/* Rotate coordinates.  Many of the functions below use this.
   This returns the results in the file-wide variables rotatedX
   and rotatedY. */
static void RotateXY
        (
        Coord x,
        Coord y
        )
{
    /* uncomment the commented terms if you want more general rotations
    than by 90 degrees */
    rotatedX = deltaX + y * multiplierY;
    rotatedY = deltaY + x * multiplierX;
}



/* Rotate differences.  Results returned in rotateddx and
   rotateddy. */
static void Rotatedxdy
         (
         Coord dx,
         Coord dy
         )
{
    /* uncomment the commented terms if you want more general rotations
    than by 90 degrees */
    rotateddx = dy * multiplierY;
    rotateddy = dx * multiplierX;
}



/* Rotate a rectangle */
static void RotateRectangle
      (
      RectangleType* rect
      )
{
    if ( clockwise )
        RotateXY( rect->topLeft.x, rect->topLeft.y + rect->extent.y - 1 );
    else
        RotateXY( rect->topLeft.x + rect->extent.x - 1, rect->topLeft.y );
    rotatedRect.topLeft.x = rotatedX;
    rotatedRect.topLeft.y = rotatedY;
    rotatedRect.extent.x  = rect->extent.y;
    rotatedRect.extent.y  = rect->extent.x;
}



/* Initialize rotation functions; returns true if there is rotation
   to be done. */
static Boolean InitializeRotation( void )
{
    switch ( Prefs()->rotate ) {
        case ROTATE_PLUS90:
            multiplierX = -1;
            multiplierY = 1;
            if ( IsFullscreenformActive() ) {
                deltaX = 0;
                deltaY = MaxExtentY() - 1;
            }
            else {
                deltaX = TopLeftX();
                deltaY = TopLeftY() + ExtentY() - 1;
            }
            clockwise = false;
            return true;
        case ROTATE_MINUS90:
            multiplierX = 1;
            multiplierY = -1;
            if ( IsFullscreenformActive() ) {
                deltaX = MaxExtentX() - 1;
                deltaY = 0;
            }
            else {
                deltaX = TopLeftX() + ExtentX() - 1;
                deltaY = TopLeftY();
            }
            clockwise = true;
            return true;
        default:
            return false;
    }
}



/* Initialize rotation functions; returns true if there is rotation
   to be done. */
static Boolean InitializeOppositeRotation( void )
{
    switch ( Prefs()->rotate ) {
        case ROTATE_PLUS90:
            multiplierX = 1;
            multiplierY = -1;
            if ( IsFullscreenformActive() ) {
                deltaX = MaxExtentY() - 1;
                deltaY = 0;
            }
            else {
                deltaX = TopLeftY() + ExtentY() - 1;
                deltaY = -TopLeftX();
            }
            clockwise = true;
            return true;
        case ROTATE_MINUS90:
            multiplierX = -1;
            multiplierY = 1;
            if ( IsFullscreenformActive() ) {
                deltaX = 0;
                deltaY = MaxExtentX() - 1;
            }
            else {
                deltaX = -TopLeftY();
                deltaY = TopLeftX() + ExtentX() - 1;
            }
            clockwise = false;
            return true;
        default:
            return false;
    }
}



/* supply function missing pre 3.5! */
static void GlueDrawPixel
    (
    Coord   x,
    Coord   y
    )
{
  static RectangleType rect = { { 0, 0 }, { 1, 1 } };
  rect.topLeft.x = x;
  rect.topLeft.y = y;
  WinDrawRectangle( &rect, 0 );
}



/* supply function missing pre 3.5! */
static void GlueErasePixel
    (
    Coord   x,
    Coord   y
    )
{
  static RectangleType rect = { { 0, 0 }, { 1, 1 } };
  rect.topLeft.x = x;
  rect.topLeft.y = y;
  WinEraseRectangle( &rect, 0 );
}



/* Get the highest screen depth possible in the
   context of a <= 64000 byte bitmap.  If the background is white, this
   is 1.  Return 0 if no depth works. */
static UInt8 GetMaxBitDepthForBitmap
    (
    UInt32 numPixels
    )
{
    UInt8 bitmapDepth;
    
    bitmapDepth = Prefs()->screenDepth;

    while ( 64000l * 8 < numPixels * bitmapDepth ) {
        bitmapDepth >>= 1;
    }

    return bitmapDepth;
}



/* Draw rotated text on OS 3.5+ */
void DrawRotatedText35
    (
    Char*   string,   /* string to draw */
    Int16   length,   /* length */
    Coord   x,        /* location */
    Coord   y,
    Boolean invert,   /* do we invert? */
    Coord   charTop,  /* y-offset to start drawing at */
    Coord   height    /* y-offset to draw up to but not including */
    )
{
    Int16            width;
    WinHandle        newWindow;
    Err              err;
    RectangleType    bounds;
    BitmapType*      srcBitmap;
    BitmapType*      destBitmap;
    UInt16           srcRowBytes;
    UInt16           destRowBytes;
    Boolean          clockwise;
    Boolean          convertedSrc;
    BitmapType*      oldSrcBitmap;
    WinHandle        oldWindow;
    UInt8            bitmapDepth;

    clockwise = ( Prefs()->rotate == ROTATE_MINUS90 );

    width = FntCharsWidth( string, length );

    if ( HaveWhiteBackground() ) {
        bitmapDepth = 1;
    }
    else {
        bitmapDepth = GetMaxBitDepthForBitmap( ( UInt32 ) width *
                          ( height - charTop ) );
        if ( 0 == bitmapDepth )
            return;
    }

    srcBitmap = BmpCreate( width, height - charTop, bitmapDepth, NULL, &err );
    if ( err != errNone )
        return;
    destBitmap = BmpCreate( height - charTop, width, bitmapDepth, NULL, &err );
    if ( err != errNone ) {
        BmpDelete( srcBitmap );
        return;
    }

    oldSrcBitmap  = srcBitmap;
    convertedSrc  = ConvertImageToV3( &srcBitmap );

    newWindow = WinCreateBitmapWindow( srcBitmap, &err );

    if ( err != errNone ) {
        BmpDelete( srcBitmap );
        BmpDelete( destBitmap );
        if ( convertedSrc )
            BmpDelete( oldSrcBitmap );
        return;
    }

    bounds.topLeft.x = 0;
    bounds.topLeft.y = 0;
    bounds.extent.x  = width;
    bounds.extent.y  = height - charTop;
    oldWindow = WinSetDrawWindow( newWindow );
    PalmSetCoordinateSystem( NATIVE );
    WinSetBounds( newWindow, &bounds );

    if ( invert )
        WinDrawInvertedChars( string, length, 0, -charTop );
    else
        WinDrawChars( string, length, 0, -charTop );

    WinDeleteWindow( newWindow, false );
    WinSetDrawWindow( oldWindow );

    BmpGlueGetDimensions( srcBitmap, NULL, NULL, &srcRowBytes );
    BmpGlueGetDimensions( destBitmap, NULL, NULL, &destRowBytes );

#ifdef BUILD_ARMLETS
    if ( RotateBitmapArmlet != NULL ) {
        ArmRotateType armData;

        armData.width        = width;
        armData.height       = height - charTop;
        armData.dest         = BmpGetBits( destBitmap );
        armData.destRowBytes = destRowBytes;
        armData.src          = BmpGetBits( srcBitmap );
        armData.srcRowBytes  = srcRowBytes;
        armData.clockwise    = clockwise;
        armData.bitDepth     = bitmapDepth;

        PceNativeCall( RotateBitmapArmlet, &armData );
    }
    else
#endif
        RotateBitmap( width, height - charTop,
            BmpGetBits( destBitmap ), destRowBytes,
            BmpGetBits( srcBitmap ), srcRowBytes,
            clockwise, bitmapDepth );

    bounds.topLeft.x = x;
    bounds.topLeft.y = y + charTop;

    RotateRectangle( &bounds );

    GeneralWinDrawBitmap( destBitmap, rotatedRect.topLeft.x,
        rotatedRect.topLeft.y );

    BmpDelete( srcBitmap );
    BmpDelete( destBitmap );
    if ( convertedSrc )
        BmpDelete( oldSrcBitmap );

    return;
}



/* Draw rotated text on OS < 3.5 */
void DrawRotatedTextPre35
    (
    Char*   string,   /* string to draw */
    Int16   length,   /* length */
    Coord   x,        /* location */
    Coord   y,
    Boolean invert,   /* do we invert? */
    Coord   charTop,  /* y-offset to start drawing at */
    Coord   height    /* y-offset to draw up to but not including */
    )
{
    RectangleType     rect;
    PalmFontType*     font;
    PalmFontCharInfoType* owTable;
    Int16             dx;
    Int16             dy;
    Int16             width;
    UInt16            charX;
    Int16             rowWords;
    UInt16*           glyph;
    UInt16*           bitmapLocationTable;
    UInt16            charNum;
    Int16             charWidth;
    UInt16*           glyphPlusCharTop;
    WChar             ch;

    
    font = ( PalmFontType* ) FntGetFontPtr();
    if ( font->owTLoc < 0 )
        return;

    owTable = ( PalmFontCharInfoType* ) ( ( UInt8* ) &( font->owTLoc )
                                        + 2 * ( font->owTLoc ) );

    height   = font->fRectHeight;
    rowWords = font->rowWords;

    glyph = ( UInt16* ) ( ( ( UInt8* ) font ) + sizeof( PalmFontType ) );
    bitmapLocationTable = glyph + rowWords * height;

    rect.topLeft.y = y + charTop;
    rect.extent.y  = height - charTop;

    glyphPlusCharTop = glyph + rowWords * charTop;

    while ( 0 < length ) {
        Boolean missing;

        charWidth  = TxtGlueGetNextChar( string, 0, &ch );
        string    += charWidth;
        length    -= charWidth;

        if ( ch < font->firstChar ||
            font->lastChar < ch )
            ch = font->lastChar;
        charNum = ch - font->firstChar;

        charX = bitmapLocationTable[ charNum ];

        if ( -1 == owTable[ charNum ].width ) {
            width   = FntWCharWidth( ch );
            missing = true;
        }
        else
            missing = false;

        width = owTable[ charNum ].width;

        rect.topLeft.x   = x;
        rect.extent.x    = width;

        RotateRectangle( &rect );
        if ( ! invert )
            WinEraseRectangle( &rotatedRect, 0 );
        else
            WinDrawRectangle( &rotatedRect, 0 );

        if ( ! missing ) {

            Rotatedxdy( 0, 1 );

            for ( dx = 0 ; dx < width ; dx++ ) {
                UInt16*  column;
                UInt16   mask;
                UInt16   offset;

                column = glyphPlusCharTop + ( charX + dx ) / 16;
                mask   = 1 << ( 15 - ( ( charX + dx ) & 15 ) );
                offset = 0;
                RotateXY( x + dx, y + charTop );

                for ( dy = charTop ; dy < height ; dy++ ) {
                    if ( ( column[ offset ] & mask ) != 0 ) {
                        if ( ! invert )
                            GlueDrawPixel( rotatedX, rotatedY );
                        else
                            GlueErasePixel( rotatedX, rotatedY );
                    }
                    offset   += rowWords;
                    rotatedX += rotateddx;
                    /* rotatedY += rotateddy; */  /* rotateddy is always zero */
                }
            }
        }

        x += width;
    }
}



/* Draw a rotated character or string at (x,y). */
/* On OS3.5+ this calls RotBitmapMinus90 and hence only supports minus
   90 degree rotation. */
static void RotDrawCharsGeneral
    (
    Char*   string,   /* string to draw */
    Int16   length,   /* length */
    Coord   x,        /* location */
    Coord   y,
    Boolean invert    /* do we invert? */
    )
{
    Coord   charTop;
    Coord   height;

#ifdef HAVE_GRAY_FONT
    if ( GrayFntIsCurrentGray() ) {
        if ( Prefs()->rotate == ROTATE_MINUS90 )
            GrayFntSetOrientation( GRAY_FONT_RIGHT );
        else
            GrayFntSetOrientation( GRAY_FONT_LEFT );
        RotateXY( x, y );
        GrayWinDrawCharsGeneral( string, length, rotatedX, rotatedY, invert );
        return;
    }
#endif

    height  = FntCharHeight();
    charTop = 0;

    if ( haveRange ) {
        /* some clipping to speed things up */
        if ( y < rangeTopY ) {
            charTop = rangeTopY - y;
            if ( height <= charTop )
                return;
        }
        if ( rangeTopY + rangeExtentY < y + height ) {
            height = ( rangeTopY + rangeExtentY ) - y;
            if ( height <= charTop )
                return;
        }
    }

    if ( Support35() ) {
        DrawRotatedText35( string, length, x, y, invert, charTop, height );
    }
    else {
        DrawRotatedTextPre35( string, length, x, y, invert, charTop, height );
    }
}




/* We include replacements for a number of PalmOS functions, and
   a few Plucker ones, that can be used instead of the originals.
   Input coordinates, the ones that the rest of Plucker works with, 
   are "logical coordinates".  The replacement functions rotate these
   into "screen coordinates".  If a function returns, say, a rectangle,
   it is rotated into logical coordinates.  Thus, the caller does not
   need to worry about rotation. */

/* Draw a string, rotating as needed */
void RotDrawChars
      (
      Char* string,
      Int16 length,
      Coord x,
      Coord y
      )
{
    if ( ! InitializeRotation() ) {
#ifdef HAVE_GRAY_FONT
        GrayFntSetOrientation( GRAY_FONT_NORMAL );
        GrayWinDrawChars( string, length, x, y );
#else
//debug(1, "XXX", "WinDrawChars(\"%.*s\", %d, %d)", length, string, x, y);
        WinDrawChars( string, length, x, y );
#endif
        return;
    }
    RotDrawCharsGeneral( string, length, x, y, false );
}



/* Draw a string, rotating as needed */
void RotDrawInvertedChars
      (
      Char* string,
      Int16 length,
      Coord x,
      Coord y
      )
{
    if ( ! InitializeRotation() ) {
#ifdef HAVE_GRAY_FONT
        GrayFntSetOrientation( GRAY_FONT_NORMAL );
        GrayWinDrawInvertedChars( string, length, x, y );
#else
        WinDrawInvertedChars( string, length, x, y );
#endif
        return;
    }
    RotDrawCharsGeneral( string, length, x, y, true );
}



void RotSetClip
     (
     RectangleType* rect
     )
{
  if ( ! InitializeRotation() ) {
      WinSetClip( rect );
      return;
  }
    RotateRectangle( rect );
    WinSetClip( &rotatedRect );
}



void RotEraseRectangle
      (
      RectangleType* rect,
      UInt16         cornerDiam
      )
{
  if ( ! InitializeRotation() ) {
      WinEraseRectangle( rect, cornerDiam );
      return;
  }
    RotateRectangle( rect );
    WinEraseRectangle( &rotatedRect, cornerDiam );
}



void RotInvertRectangle
      (
      RectangleType* rect,
      UInt16 cornerDiam
      )
{
  if ( ! InitializeRotation() ) {
      WinInvertRectangle( rect, cornerDiam );
      return;
  }
    RotateRectangle( rect );
    WinInvertRectangle( &rotatedRect, cornerDiam );
}



void RotDrawRectangle
      (
      RectangleType* rect,
      UInt16 cornerDiam
      )
{
  if ( ! InitializeRotation() ) {
      WinDrawRectangle( rect, cornerDiam );
      return;
  }
    RotateRectangle( rect );
    WinDrawRectangle( &rotatedRect, cornerDiam );
}



void RotDrawRectangleFrame
      (
      FrameType      frame,
      RectangleType* rect
      )
{
  if ( ! InitializeRotation() ) {
      WinDrawRectangleFrame( frame, rect );
      return;
  }
    RotateRectangle( rect );
    WinDrawRectangleFrame( frame, &rotatedRect );
}



void RotDrawGrayRectangleFrame
      (
      FrameType      frame,
      RectangleType* rect
      )
{
  if ( ! InitializeRotation() ) {
      WinDrawGrayRectangleFrame( frame, rect );
      return;
  }
    RotateRectangle( rect );
    WinDrawGrayRectangleFrame( frame, &rotatedRect );
}



WinHandle RotCreateOffscreenWindow
       (
       Coord width,
       Coord height,
       WindowFormatType format,
       UInt16* error
       )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return WinCreateOffscreenWindow( width, height, format, error );
    else
        return WinCreateOffscreenWindow( height, width, format, error );
}



/* take deltas in two coordinate directions, rotate,
   and return a WinDirectionType. */
static WinDirectionType dxdyToRotatedDirection
        (
        Coord dx,
        Coord dy
        )
{
    Rotatedxdy( dx, dy );
    if ( rotateddx < 0 )
        return winLeft;
    if ( 0 < rotateddx )
        return winRight;
    if ( rotateddy < 0 )
        return winUp;
    return winDown;
}



/* Is our present rotation state and OS version supportive of
   RotDrawBitmap()? */
Boolean RotSupportDrawBitmapNow( void )
{
   return ( Prefs()->rotate == ROTATE_ZERO || Support35() );
}



void RotDrawBitmap
       (
       BitmapType* bitmap,
       Coord       x,
       Coord       y
       )
{
    Coord       height;
    Coord       width;
    UInt16      rowBytes;
    UInt16      destRowBytes;
    UInt8       bitDepth;
    BitmapType* rotatedBitmap;
    Err         err;
    ColorTableType* colorTable;

    if ( ! InitializeRotation() ) {
        GeneralWinDrawBitmap( bitmap, x, y );
        return;
    }
    if ( ! Support35() )
        return;
    BmpGlueGetDimensions( bitmap, &width, &height, &rowBytes );
    bitDepth      = BmpGlueGetBitDepth( bitmap );
    colorTable    = BmpGetColortable( bitmap );
    rotatedBitmap = BmpCreate( height, width, bitDepth, colorTable, &err );
    if ( err != errNone )
        return;
    BmpGlueGetDimensions( rotatedBitmap, NULL, NULL, &destRowBytes );
#ifdef BUILD_ARMLETS
    if ( RotateBitmapArmlet != NULL ) {
        ArmRotateType armData;

        armData.width        = width;
        armData.height       = height;
        armData.dest         = BmpGetBits( rotatedBitmap );
        armData.destRowBytes = destRowBytes;
        armData.src          = BmpGetBits( bitmap );
        armData.srcRowBytes  = rowBytes;
        armData.clockwise    = clockwise;
        armData.bitDepth     = bitDepth;

        PceNativeCall( RotateBitmapArmlet, &armData );
    }
    else
#endif
        RotateBitmap( width, height,
            BmpGetBits( rotatedBitmap ), destRowBytes,
            BmpGetBits( bitmap ), rowBytes,
            clockwise, bitDepth );
    if ( clockwise )
        RotateXY( x, y + height - 1 );
    else
        RotateXY( x + width - 1, y );
    GeneralWinDrawBitmap( rotatedBitmap, rotatedX, rotatedY );
    BmpDelete( rotatedBitmap );
}



void RotScrollRectangle
       (
       RectangleType* rP,
       WinDirectionType direction,
       Coord distance,
       RectangleType* vacatedP
       )
{
    RectangleType rect;
    if ( ! InitializeRotation() ) {
        WinScrollRectangle( rP, direction, distance, vacatedP );
        return;
    }
    switch ( direction ) {
        case winUp:
             direction = dxdyToRotatedDirection( 0, -1 );
             break;
        case winDown:
             direction = dxdyToRotatedDirection( 0, 1 );
             break;
        case winLeft:
             direction = dxdyToRotatedDirection( -1, 0 );
             break;
        case winRight:
             direction = dxdyToRotatedDirection( 1, 0 );
             break;
    }

    rect = *rP;

    if ( haveRange ) {
        RectangleType zero = { { 0, 0 }, { 0, 0 } };
        rect = *rP;
        if ( rect.topLeft.y < rangeTopY ) {
            rect.extent.y -= ( rangeTopY - rect.topLeft.y );
            if ( rect.extent.y <= 0 ) {
                *vacatedP = zero;
                return;
            }
            rect.topLeft.y = rangeTopY;
        }
        if ( rangeTopY + rangeExtentY < rect.topLeft.y + rect.extent.y ) {
            rect.extent.y = ( rangeTopY + rangeExtentY ) - rect.topLeft.y;
            if ( rect.extent.y <= 0 ) {
                *vacatedP = zero;
                return;
            }
        }
    }
    RotateRectangle( &rect );
    WinClipRectangle( &rotatedRect );
    WinScrollRectangle( &rotatedRect, direction, distance, vacatedP );
    if ( vacatedP != NULL ) {
        InitializeOppositeRotation();
        RotateRectangle( vacatedP );
        *vacatedP = rotatedRect;
    }
}



Coord RotExtentX( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return ExtentX();
    else
        return ExtentY();
}



Coord RotExtentY( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return ExtentY();
    else
        return ExtentX();
}



Coord RotTopLeftX( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return TopLeftX();
    else
        return 0;
}



Coord RotTopLeftY( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return TopLeftY();
    else
        return 0;
}



void RotateRectangleInPlace
      (
      RectangleType* rect
      )
{
    if ( ! InitializeRotation() )
        return;
    RotateRectangle( rect );
    *rect = rotatedRect;
}



void RotDrawLine
      (
      Coord x,
      Coord y,
      Coord x2,
      Coord y2
      )
{
    if ( ! InitializeRotation() ) {
        WinDrawLine( x, y, x2, y2 );
        return;
    }
    RotateXY( x, y );
    x = rotatedX;
    y = rotatedY;
    RotateXY( x2, y2 );
    WinDrawLine( x, y, rotatedX, rotatedY );
}



void RotDrawGrayLine
      (
      Coord x,
      Coord y,
      Coord x2,
      Coord y2
      )
{
    if ( ! InitializeRotation() ) {
        WinDrawGrayLine( x, y, x2, y2 );
        return;
    }
    RotateXY( x, y );
    x = rotatedX;
    y = rotatedY;
    RotateXY( x2, y2 );
    WinDrawGrayLine( x, y, rotatedX, rotatedY );
}



Coord RotMaxExtentX( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return MaxExtentX();
    else
        return MaxExtentY();
}




Coord RotMaxExtentY( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return MaxExtentY();
    else
        return MaxExtentX();
}




/* only render characters with top<= y < top+extent */
void RotCharClipRange
        (
        Coord top,     /* clip off parts of chars above this */
        Coord extent   /* only show this much vertically from top */
        )
{
    rangeTopY    = top;
    rangeExtentY = extent;
    haveRange    = true;
}



void RotCharClearClipRange( void )
{
    haveRange    = false;
}



Coord RotGetScrollValue( void )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return GetScrollValue();
    else {
        Coord  extent;
        UInt16 prevCoordSys;
        prevCoordSys = PalmSetCoordinateSystem( NATIVE );
        extent       = RotExtentY() - FntLineHeight();
        PalmSetCoordinateSystem( prevCoordSys );
        return extent;
    }
}



/* rotate coordinates: screen to logical */
void RotFromScreenXY
    (
    Coord* x,
    Coord* y
    )
{
    if ( ! InitializeOppositeRotation() )
        return;
    RotateXY( *x, *y );
    *x = rotatedX;
    *y = rotatedY;
}




/* rotate arrow keys */
void RotateArrowKeys
    (
    WChar *key
    )
{
    if ( Prefs()->rotate == ROTATE_ZERO )
        return;

    if ( Prefs()->rotate == ROTATE_PLUS90 ) {
        switch ( *key ) {
            case chrPageDown:
                 *key = chrPageUp;
                 break;
            case chrPageUp:
                 *key = chrPageDown;
                 break;
            case chrDownArrow:
                 *key = chrLeftArrow;
                 break;
            case chrUpArrow:
                 *key = chrRightArrow;
                 break;
            case chrLeftArrow:
                 *key = chrUpArrow;
                 break;
            case chrRightArrow:
                 *key = chrDownArrow;
                 break;
            default:
                 break;
        }
    }
    else {
        switch ( *key ) {
            case chrDownArrow:
                 *key = chrRightArrow;
                 break;
            case chrUpArrow:
                 *key = chrLeftArrow;
                 break;
            case chrLeftArrow:
                 *key = chrDownArrow;
                 break;
            case chrRightArrow:
                 *key = chrUpArrow;
                 break;
            default:
                 break;
        }
    }
}



#ifdef HAVE_JOGDIAL
Boolean RotIsJogdialUp
        (
        WChar key
        )
{
  if ( Prefs()->rotate != ROTATE_PLUS90 )
      return IsJogdialUp( key );
  else
      return IsJogdialDown( key );
}



Boolean RotIsJogdialDown
        (
        WChar key
        )
{
  if ( Prefs()->rotate != ROTATE_PLUS90 )
      return IsJogdialDown( key );
  else
      return IsJogdialUp( key );
}



Boolean RotIsJogdialPushUp
        (
        WChar key
        )
{
  if ( Prefs()->rotate != ROTATE_PLUS90 )
      return IsJogdialPushUp( key );
  else
      return IsJogdialPushDown( key );
}



Boolean RotIsJogdialPushDown
        (
        WChar key
        )
{
  if ( Prefs()->rotate != ROTATE_PLUS90 )
      return IsJogdialPushDown( key );
  else
      return IsJogdialPushUp( key );
}
#endif



#ifdef BUILD_ARMLETS
/* enable rotation armlet */
void RotOpenArmlet( void )
{
    if ( Prefs()->rotate != ROTATE_ZERO && SupportArmlets() ) {
        armChunkH = DmGetResource( ArmletResourceType, armRotateBitmap );
        if ( armChunkH != NULL )
            RotateBitmapArmlet = MemHandleLock( armChunkH );
    }
}



/* disable rotation armlet */
void RotCloseArmlet( void )
{
    if ( armChunkH != NULL ) {
        MemHandleUnlock( armChunkH );
        DmReleaseResource( armChunkH );
        armChunkH = NULL;
    }
    RotateBitmapArmlet = NULL;
}
#endif
