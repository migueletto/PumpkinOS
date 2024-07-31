/*
 * $Id: grayfont.c,v 1.38 2004/04/18 15:34:48 prussar Exp $
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
#include <FntGlue.h>
#include <TxtGlue.h>
#include "const.h"
#include "os.h"
#include "util.h"
#include "prefsdata.h"
#include "palmbitmap.h"
#include "font.h"
#define NO_GRAY_FONT_SUBSTITUTION
#include "grayfont.h"
#include "../libpit/debug.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define KERNING_FONTVERSION                     2
#define MAX_SCREEN_WIDTH                        500
#define MAX_FONT_ID                             255
#define HIGH_DENSITY_FEATURE_SET_VERSION        4
#define RESOURCE_NAME_ID             'G'
#define RESOURCE_NAME_IDLETTER       0
#define RESOURCE_NAME_ORIENTATION    1
#define RESOURCE_NAME_BITMAP_VERSION 2
#define RESOURCE_NAME_DEPTH          3

#define REMAP_FONTVERSION            0x4000




/***********************************************************************
 *
 *      Internal types
 *
 ***********************************************************************/

typedef struct {
    UInt16 fontVersion;
    UInt16 fontID;
} RemapInfoType;

typedef struct {
    UInt16 firstGlyph;   /* first glyph found in resource */
    UInt16 lastGlyph;    /* last glyph found in resource */
    UInt16 resourceID;   /* the ID of the resource */
    UInt16 reserved;     /* should be zero for now */
} GrayFontBitmapsInfo;

typedef struct {
    UInt16 v;
    UInt16 fontVersion;
    UInt16 firstChar;
    UInt16 lastChar;
    UInt16 fRectWidth;
    UInt16 fRectHeight;
    Int16  ascent;
    Int16  descent;
    Int16  leading;
    UInt16 numberOfBitmapResources;
    UInt16 bitmapResourceTableOffset;
    UInt16 glyphInfoTableOffset;
    Int16  minLeftKerning;
    Int16  maxRightOverhang;
    UInt16 reserved[4];
} GrayFontType;

typedef struct {
    UInt16 offset;
    UInt16 length;
} GrayFontResourceIndexEntry;

typedef struct {
    Int16   leftKerning;
    Int16   advance;
    UInt16  bitmapWidth;
    UInt16  resourceNumber;
    UInt16  positionInResourceIndex;
    UInt16  reserved;
} GrayFontGlyphInfo;

typedef struct {
    MemHandle        handle;
    FontResourceKind kind;
} ResourceCacheEntry;    

typedef struct {
    GrayFontType         header;
    GrayFontGlyphInfo*   glyphList;
    GrayFontBitmapsInfo* bitmapsResourceIndex;
    ResourceCacheEntry*  resourceCache;
} GrayFontInfo;

typedef union {
    DmResType resType;
    Char      string[4];
} ResourceType;



/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void GrayFntDelete ( FontID font ) GRAYFONT_SECTION;
static GrayFontGlyphInfo* GetGlyph ( WChar ch ) GRAYFONT_SECTION;
static UInt16 TempBitmapSize ( UInt16 width, UInt16 height ) GRAYFONT_SECTION;
static void ColorizeBitmapSetColorMap ( RGBColorType* fore, 
    RGBColorType* back, Boolean invert )
    GRAYFONT_SECTION;
static void ColorizeBitmap4To16 ( UInt16 x, UInt16 y, PalmBitmapType* dest,
    PalmBitmapType* src ) GRAYFONT_SECTION;
static void InvertBitmap4 ( PalmBitmapType* dest, PalmBitmapType* src )
    GRAYFONT_SECTION;
static void GrayFntClearCaches( void ) GRAYFONT_SECTION;




/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static ResourceType    resource;
static GrayFontInfo*   grayFont[ MAX_FONT_ID + 1];
static GrayFontInfo*   currentFontPtr = NULL;
static FontID          currentFont;
static Boolean         uses8BitChars;
static Boolean         havePalmHiRes;
static UInt8           substitutionList[ MAX_FONT_ID + 1 ];
static Boolean         active = false;
static PalmBitmapType* tempBitmap = NULL;
static UInt16          tempBitmapSize = 0;
static UInt16          tempBitmapHeaderSize;
static Boolean         fastRenderAvailable = false;
static Boolean         backgroundErase     = true;
static RGBColorType    lastFore = { 0, 0, 0 };
static RGBColorType    lastBack = { 255, 255, 255 };
static UInt16          colorMap[ 16 ] = {
    0xffff, 0xef7d, 0xdefb, 0xce79, 0xbdd7, 0xad55, 0x9cd3, 0x8c51, 0x73ae,
    0x632c, 0x52aa, 0x4228, 0x3186, 0x2104, 0x1082, 0x0000
};



/* Set a map for colorizing a bitmap */
static void ColorizeBitmapSetColorMap
      (
      RGBColorType*  fore,
      RGBColorType*  back,
      Boolean        invert
      )
{
    UInt16  i;
    if ( invert ) {
        RGBColorType* temp;
        temp = fore;
        fore = back;
        back = temp;
    }
    if ( fore->r == lastFore.r && fore->g == lastFore.g &&
         fore->b == lastFore.b && back->r == lastBack.r &&
         back->g == lastBack.g && back->b == lastBack.b ) {
        return;
    }
    else {
        lastFore = *fore;
        lastBack = *back;
    }
    for ( i = 0 ; i < 16 ; i++ ) {
         UInt8  r;
         UInt8  g;
         UInt8  b;
         UInt16 rgb;
         r = ( ( ( 15 - i ) * back->r + i * fore->r ) / 15 ) >>
                    ( 8 - RED_BITS );
         g = ( ( ( 15 - i ) * back->g + i * fore->g ) / 15 ) >>
                    ( 8 - GREEN_BITS );
         b = ( ( ( 15 - i ) * back->b + i * fore->b ) / 15 ) >>
                    ( 8 - BLUE_BITS );

         rgb = ( r << ( BLUE_BITS + GREEN_BITS ) ) | ( g << BLUE_BITS ) | b;

         colorMap[ i ] = rgb;
    }
}



/* Colorize a 4-bit bitmap into a 16-bit bitmap */
/* Input bitmap: type 1 or 3.  Output: type 2 or 3, 16 bit */
/* No colormaps permitted */
void ColorizeBitmap4To16
      (
      UInt16            topLeftX,
      UInt16            topLeftY,
      PalmBitmapType*   destBitmap,
      PalmBitmapType*   srcBitmap
      )
{
    UInt16  x;
    UInt16  y;
    UInt8*  in;
    UInt16* out;
    UInt8*  src;
    UInt16* dest;
    UInt16  destRowWords;
    UInt16  srcRowBytes;
    UInt16  width;
    UInt16  height;
    Boolean scanLineCompress;

    if ( srcBitmap->flags & palmCompressed )
        scanLineCompress = true;
    else
        scanLineCompress = false;
    srcRowBytes        = srcBitmap->rowBytes;
    width              = srcBitmap->width;
    height             = srcBitmap->height;
    destBitmap->pixelSize = 16;
    destRowWords          = destBitmap->rowBytes / 2;

    if ( srcBitmap->version < 3 ) {
        /* note that sizeof( PalmBitmapTypeV1 ) is the same as
           sizeof( PalmBitmapTypeV2 ) */
        src = ( UInt8* )srcBitmap + sizeof( PalmBitmapTypeV1 );
    }
    else {
        src = ( UInt8* )srcBitmap + sizeof( PalmBitmapTypeV3 );
    }
    if ( destBitmap->version < 3 ) {
        dest = ( UInt16* )( ( UInt8* )destBitmap +
                              sizeof( PalmBitmapTypeV2 ) );
    }
    else {
        dest = ( UInt16* )( ( UInt8* )destBitmap +
                              sizeof( PalmBitmapTypeV3 ) );
    }

    if ( ! scanLineCompress ) {
        UInt16 srcPaddingBytes;
        UInt16 destIncrementWords;
        UInt16 srcDataBytes;

        srcDataBytes       = ( width + 1 ) / 2;
        srcPaddingBytes    = srcRowBytes - srcDataBytes;
        destIncrementWords = destRowWords - width;

        in  = src;
        out = dest + topLeftY * destRowWords + topLeftX;

        for ( y = height ; 0 < y ; y -- ) {
            for ( x = srcDataBytes ; 0 < x ; x -- ) {
                 *out++ = colorMap[ *in >> 4 ];
                 /* The next operation may overwrite the next
                    row if width is odd, but the output bitmap
                    is allocated with two spare bytes so this
                    is not a problem */
                 *out++ = colorMap[ *in & 0xF ];
                 in++;
            }
            in  += srcPaddingBytes;
            out += destIncrementWords;
        }
    }
    else {
        UInt16* lastRow;
        UInt16  srcRowNibbles;

        srcRowNibbles = 2 * srcRowBytes;

        /* To prevent crashes due to bad fonts */
        lastRow = dest;
        /* skip length data */
        if ( srcBitmap->version < 3 )
            in = src + 2;
        else
            in = src + 4;
        for ( y = 0 ; y < height ; y++ ) {
            UInt16* startOut;
            out      = dest + ( topLeftY + y ) * destRowWords + topLeftX;
            startOut = out;
            x        = 0;
            while ( x < srcRowNibbles ) {
                UInt8   octupleMarker;
                UInt8   mask;

                octupleMarker = *in++;
                mask          = 0x80;
                do {
                    if ( mask & octupleMarker ) {
                        if ( x < width ) {
                            *out++ = colorMap[ *in >> 4 ];
                            *out++ = colorMap[ *in & 0xF ];
                        }
                        in++;
                    }
                    else if ( x < width ) {
                        *out++ = lastRow[ x ];
                        *out++ = lastRow[ x + 1 ];
                    }
                    mask >>= 1;
                    x     += 2;
                } while ( mask != 0 && x < srcRowNibbles );
            }
            lastRow = startOut;
        }
    }
}




/* Invert a 4-bit bitmap */
/* Input bitmap: type 1 or 3.  Output bitmap: same type */
/* No colormaps permitted */
void InvertBitmap4
      (
      PalmBitmapType*   destBitmap,
      PalmBitmapType*   srcBitmap
      )
{
    UInt16  x;
    UInt16  y;
    UInt8*  in;
    UInt8*  out;
    UInt8*  src;
    UInt8*  dest;
    UInt16  width;
    UInt16  height;
    UInt16  headerSize;
    width     = srcBitmap->width;
    height    = srcBitmap->height;

    if ( srcBitmap->version < 3 ) {
        /* note that sizeof( PalmBitmapTypeV1 ) is the same as
           sizeof( PalmBitmapTypeV2 ) */
        headerSize = sizeof( PalmBitmapTypeV1 );
    }
    else {
        headerSize = sizeof( PalmBitmapTypeV3 );

    }
    MemMove( destBitmap, srcBitmap, headerSize );
    src  = ( UInt8* )srcBitmap + headerSize;
    dest = ( UInt8* )destBitmap + headerSize;

    if ( ! ( srcBitmap->flags & palmCompressed ) ) {
        UInt16  rowBytes;

        rowBytes  = srcBitmap->rowBytes;
        for ( y = 0 ; y < height ; y ++ ) {
            in  = ( UInt8* ) src + y * rowBytes;
            out = ( UInt8* ) dest + y * rowBytes;
            for ( x = 0 ; x < width ; x += 2 ) {
                 *out = ( ( 15 - ( *in >> 4 ) ) << 4 ) | ( 15 - ( *in & 0xF ) );
                 in++;
                 out++;
            }
        }
    }
    else {
        UInt32 length;

        if ( 3 <= srcBitmap->version ) {
            length = ( ( UInt32 )( ( ( UInt16* )src )[ 0 ] ) << 16 ) |
                       ( UInt32 )( ( ( UInt16* )src )[ 1 ] );
            MemMove( dest, src, 4 );
            src  += 4;
            dest += 4;
        }
        else {
            length = ( ( UInt16* )src )[ 0 ];
            MemMove( dest, src, 2 );
            src  += 2;
            dest += 2;
        }
        while ( 0 < length ) {
            UInt8  octupleMarker;
            UInt8  mask;
            octupleMarker = *src;
            mask          = 0x80;
            *dest++       = *src++;
            length--;
            while ( 0 < length && 0 != mask ) {
                if ( mask & octupleMarker ) {
                    *dest = ( ( 15 - ( *src >> 4 ) ) << 4 ) | ( 15 - ( *src & 0xF ) );
                    src++;
                    dest++;
                    length--;
                }
                mask >>= 1;
            }
        }
    }
}



static void GrayFntClearCaches( void )
{
    UInt16        i;
    UInt16        j;
    GrayFontInfo* p;
    for ( i = 0 ; i <= MAX_FONT_ID ; i++ ) {
        p = grayFont[ i ];
        if ( p != NULL ) {
            for ( j = 0 ; j < p->header.numberOfBitmapResources ; j++ ) {
                if ( p->resourceCache[ j ].handle != NULL ) {
                    ReleaseFontResource( p->resourceCache[ j ].handle,
                                         p->resourceCache[ j ].kind );
                    p->resourceCache[ j ].handle = NULL;
                }
            }
        }
    }
}



static void GrayFntDelete
       (
       FontID font
       )
{
   GrayFontInfo* p;
   p = grayFont[ font ];
   if ( ! active || p == NULL ) {
       return;
   }
   if ( substitutionList[ currentFont ] == font ) {
       currentFont    = stdFont;
       currentFontPtr = NULL;
       FntSetFont( currentFont );
   }
   if ( p->glyphList != NULL )
       SafeMemPtrFree( grayFont[ font ]->glyphList );
   if ( p->bitmapsResourceIndex != NULL )
       SafeMemPtrFree( grayFont[ font ]->bitmapsResourceIndex );
   if ( p->resourceCache != NULL ) {
       UInt16 i;
       for ( i = 0 ; i < p->header.numberOfBitmapResources ; i++ ) {
           if ( p->resourceCache[ i ].handle != NULL ) {
               ReleaseFontResource( p->resourceCache[ i ].handle,
                                    p->resourceCache[ i ].kind );
           }
       }
       MemPtrFree( p->resourceCache );
   }
   SafeMemPtrFree( p );
   grayFont[ font ] = NULL;
}



static GrayFontGlyphInfo* GetGlyph
      (
      WChar ch
      )
{
    UInt16 index;
    if ( ch < currentFontPtr->header.firstChar ||
         currentFontPtr->header.lastChar < ch  ||
         currentFontPtr->glyphList[ ch -
                    currentFontPtr->header.firstChar ].resourceNumber == 0
       )
        index = currentFontPtr->header.lastChar -
                    currentFontPtr->header.firstChar;
    else
        index = ch - currentFontPtr->header.firstChar;
    return &( currentFontPtr->glyphList[ index ] );
}



/* Start the gray-scale font functions */
void GrayFntStart( void )
{
    UInt16  i;
    UInt32  charEncoding;
    UInt32  version;
    Err     err;
    if ( grayFont != NULL )
        GrayFntStop();
    for ( i = 0 ; i <= MAX_FONT_ID ; i++ ) {
        grayFont[ i ]         = NULL;
        substitutionList[ i ] = i;
    }
    currentFont    = FntGetFont();
    currentFontPtr = NULL;
    err = FtrGet( sysFtrCreator, sysFtrNumEncoding, &charEncoding );
    if ( err == errNone )
        uses8BitChars = ( charEncoding <= charEncodingPalmLatin );
    else
        uses8BitChars = true;
    err = FtrGet( sysFtrCreator, sysFtrNumWinVersion, &version );
    havePalmHiRes = ( HIGH_DENSITY_FEATURE_SET_VERSION <= version );
    resource.string[ RESOURCE_NAME_IDLETTER ] = RESOURCE_NAME_ID;
    resource.string[ RESOURCE_NAME_ORIENTATION ] = GRAY_FONT_NORMAL;
    resource.string[ RESOURCE_NAME_BITMAP_VERSION ] =
        havePalmHiRes ? '3' : '1';
    resource.string[ RESOURCE_NAME_DEPTH ] = '4';
    active = true;
}



/* Stop them and clear memory */
void GrayFntStop( void )
{
    UInt16 i;
    if ( active ) {
        for ( i = 0 ; i <= MAX_FONT_ID ; i++ ) {
            GrayFntDelete( i );
            grayFont[ i ] = NULL;
        }
        active = false;
    }
    if ( tempBitmap != NULL ) {
        SafeMemPtrFree( tempBitmap );
        tempBitmap     = NULL;
        tempBitmapSize = 0;
    }
}



/* size of temporary bitmap for font */
static UInt16 TempBitmapSize
        (
        UInt16 width,
        UInt16 height
        )
{
    UInt16  rowBytes;
    UInt16  extra;
    if ( havePalmHiRes )
        tempBitmapHeaderSize = sizeof( PalmBitmapTypeV3 );
    else
        tempBitmapHeaderSize = sizeof( PalmBitmapTypeV2 );
    if ( 16 <= GetMaxBitDepth() ) {
        rowBytes = width * 2;
        extra    = 2; /* the colorization algorithm needs extra space */
    }
    else {
        rowBytes = ( width + 1 ) / 2;
        if ( rowBytes % 2 != 0 )
            rowBytes++;
        extra    = 0;
    }
    return tempBitmapHeaderSize + extra + height * rowBytes;
}



Err GrayFntDefineFont
       (
       FontID font,
       void*  fontP
       )
{
    UInt16 numGlyphs;
    UInt16 newTempSize;
    UInt16 verticalLayoutSize;
    UInt16 width;
    UInt16 height;
    UInt16 i;
    GrayFontInfo* newFont;

    GrayFntDelete( font );
    if ( * ( UInt8* ) fontP & 0x80 ) {
        return FntDefineFont( font, fontP );
    }
    else if ( * ( UInt16* ) fontP == REMAP_FONTVERSION ) {
        FontID mapToFontID;
        /* Note: multiple layers of substitution are not permitted and
           have undefined behavior, including possible crash. */
        mapToFontID              = ( ( RemapInfoType* )fontP )->fontID;
        substitutionList[ font ] = mapToFontID;
        if ( currentFont == font ) {
            currentFontPtr = grayFont[ mapToFontID ];
        }
        return errNone;
    }
    if ( ! Support35() ) {
        return errOSVersionTooLow;
    }
    grayFont[ font ] = SafeMemPtrNew( sizeof( GrayFontInfo ) );
    newFont = grayFont[ font ];
    if ( newFont == NULL ) {
        return memErrNotEnoughSpace;
    }
    MemMove( &( newFont->header ), fontP, sizeof( GrayFontType ) );
    numGlyphs = newFont->header.lastChar - newFont->header.firstChar + 1;
    newFont->bitmapsResourceIndex = NULL;
    newFont->glyphList = SafeMemPtrNew( sizeof( GrayFontGlyphInfo ) *
                                                numGlyphs );
    if ( newFont->glyphList == NULL ) {
        GrayFntDelete( font );
        return memErrNotEnoughSpace;
    }
    newFont->bitmapsResourceIndex =
        SafeMemPtrNew( sizeof( GrayFontBitmapsInfo ) *
                      newFont->header.numberOfBitmapResources );
    if ( newFont->bitmapsResourceIndex == NULL ) {
        GrayFntDelete( font );
        return memErrNotEnoughSpace;
    }
    newFont->resourceCache =
        MemPtrNew( sizeof( ResourceCacheEntry ) *
                      newFont->header.numberOfBitmapResources );
    if ( newFont->resourceCache == NULL ) {
        GrayFntDelete( font );
        return memErrNotEnoughSpace;
    }

    MemMove( newFont->glyphList,
             ( UInt8* ) fontP + newFont->header.glyphInfoTableOffset,
             sizeof( GrayFontGlyphInfo ) * numGlyphs );

    MemMove( newFont->bitmapsResourceIndex,
             ( UInt8* ) fontP +
                  newFont->header.bitmapResourceTableOffset,
             sizeof( GrayFontBitmapsInfo ) *
                    newFont->header.numberOfBitmapResources );

    if ( substitutionList[ currentFont ] == font ) {
        currentFontPtr = newFont;
    }

    width  = newFont->header.fRectWidth;
    height = newFont->header.fRectHeight;
    if ( width <= MAX_SCREEN_WIDTH && height <= MAX_SCREEN_WIDTH &&
         16 <= GetMaxBitDepth() && Have68K() ) {
        newTempSize         = TempBitmapSize( MAX_SCREEN_WIDTH, height );
        verticalLayoutSize  = TempBitmapSize( height, MAX_SCREEN_WIDTH );
        fastRenderAvailable = true;
    }
    else {
        newTempSize         = TempBitmapSize( width, height );
        verticalLayoutSize  = TempBitmapSize( height, height );
        fastRenderAvailable = false;
    }

    if ( newTempSize < verticalLayoutSize )
        newTempSize = verticalLayoutSize;
    if ( tempBitmapSize < newTempSize || tempBitmap == NULL ) {
        if ( tempBitmap != NULL ) {
            SafeMemPtrFree( tempBitmap );
            tempBitmap     = NULL;
            tempBitmapSize = 0;
        }
        tempBitmap = SafeMemPtrNew( newTempSize );
        if ( tempBitmap != NULL ) {
            tempBitmapSize = newTempSize;
            MemSet( tempBitmap, tempBitmapSize, 0 );
            if ( havePalmHiRes ) {
                (( PalmBitmapTypeV3* )tempBitmap)->size
                    = sizeof( PalmBitmapTypeV3 );
                (( PalmBitmapTypeV3* )tempBitmap)->density
                    = PalmGetDensity();
                tempBitmap->version = 3;
            }
            else {
                tempBitmap->version = 2;
            }
        }
    }
    if ( tempBitmap == NULL ) {
        fastRenderAvailable = false;
    }
    for ( i = 0 ; i < newFont->header.numberOfBitmapResources ; i++ )
        newFont->resourceCache[ i ].handle = NULL;

    return errNone;
}




FontID GrayFntGetFont( void )
{
    return currentFont;
}



FontID GrayFntSetFont
        (
        FontID font
        )
{
    FontID    oldFont;
    oldFont        = currentFont;
    currentFont    = font;
    currentFontPtr = grayFont[ substitutionList[ font ] ];
    if ( grayFont[ font ] == NULL ) {
        FntSetFont( substitutionList[ font ] );
    }
    return oldFont;
}



Int16 GrayFntLineHeight( void )
{
    if ( currentFontPtr == NULL )
        return FntLineHeight();
    else {
        Coord height;
        height = currentFontPtr->header.leading +
                   currentFontPtr->header.fRectHeight;
        HiResAdjustNativeToCurrent( &height );
        return height;
    }
}



Int16 GrayFntCharHeight( void )
{
    if ( currentFontPtr == NULL )
        return FntCharHeight();
    else {
        Coord height;
        height = currentFontPtr->header.fRectHeight;
        HiResAdjustNativeToCurrent( &height );
        return height;
    }
}



Int16 GrayFntWCharWidth
       (
       WChar ch
       )
{
    if ( currentFontPtr == NULL )
        return FntGlueWCharWidth( ch );
    else {
        Coord width;
        width = GetGlyph( ch )->advance;
        HiResAdjustNativeToCurrent( &width );
        return width;
    }
}



Int16 GrayFntCharWidth
       (
       Char ch
       )
{
    if ( currentFontPtr == NULL )
        return FntCharWidth( ch );
    else {
        Coord width;
        width = GetGlyph( ( UInt8 )ch )->advance;
        HiResAdjustNativeToCurrent( &width );
        return width;
    }
}



Int16 GrayFntCharsWidth
       (
       Char const* chars,
       Int16 length
       )
{
    Int16  width;
    UInt8 const* p;
    if ( currentFontPtr == NULL )
        return FntCharsWidth( chars, length );
    width = 0;
    if ( uses8BitChars ) {
        p = ( UInt8* )chars;
        while ( 0 < length-- ) {
            width += GetGlyph( *p++ )->advance;
        }
    }
    else {
        UInt32 inOffset;
        inOffset = 0;
        while ( inOffset < length ) {
            WChar  ch;
            inOffset += TxtGlueGetNextChar( chars, inOffset, &ch );
            if ( length < inOffset )
                break;
            width += GetGlyph( ch )->advance;
        }
    }
    HiResAdjustNativeToCurrent( &width );
    return width;
}



Char GrayFntSetOrientation
      (
      Char o
      )
{
    Char oldOrientation;
    oldOrientation = resource.string[ RESOURCE_NAME_ORIENTATION ];
    if ( oldOrientation != o ) {
        resource.string[ RESOURCE_NAME_ORIENTATION ] = o;
        GrayFntClearCaches();
    }
    return oldOrientation;
}



Coord GrayFntMinLeftKerning
       (
       FontID font
       )
{
    GrayFontInfo* fontPtr;
    fontPtr = grayFont[ substitutionList[ font ] ];
    if ( fontPtr == NULL ||
         fontPtr->header.fontVersion < KERNING_FONTVERSION )
        return 0;
    else
        return fontPtr->header.minLeftKerning;
}



Coord GrayFntMaxRightOverhang
       (
       FontID font
       )
{
    GrayFontInfo* fontPtr;
    fontPtr = grayFont[ substitutionList[ font ] ];
    if ( fontPtr == NULL ||
         fontPtr->header.fontVersion < KERNING_FONTVERSION )
        return 0;
    else
        return fontPtr->header.maxRightOverhang;
}




void GrayFntSetBackgroundErase
      (
      Boolean state
      )
{
    backgroundErase = state;
}



void GrayWinDrawCharsGeneral
      (
      Char const* chars,
      Int16   length,
      Coord   x,
      Coord   y,
      Boolean invert
      )
{
    Coord*    toIncrement;
    Int16     incrementSign;
    UInt32    inOffset;
    Boolean   drawBeforePosition;
    Boolean   doMapColors;
    Boolean   doInvert;
    MemHandle bitmapHandle   = NULL;
    UInt16    prevCoordSys;
    Boolean   doFastRender;
    Coord     bitmapTopLeftX;
    Coord     bitmapTopLeftY;
    Coord     width;
    Coord     oldX;
    Coord     oldY;
    Coord     firstKern;
    Coord     lastOverhang;
    WChar     ch;
    WinDrawOperation  oldOperation = winPaint;
    Boolean           doKern;

    if ( currentFontPtr == NULL ) {
        if ( invert )
            WinDrawInvertedChars( chars, length, x, y );
        else
            WinDrawChars( chars, length, x, y );
        return;
    }
    if ( length <= 0 )
        return;

    HiResAdjustCurrentToNative( &x );
    HiResAdjustCurrentToNative( &y );

    prevCoordSys   = PalmSetCoordinateSystem( NATIVE );

    doFastRender   = false;
    width          = 0;
    lastOverhang   = 0;
    if ( GrayFntMinLeftKerning( currentFont ) < 0 ||
         0 < GrayFntMaxRightOverhang( currentFont )  ) {
        oldOperation = WinSetDrawMode( winOverlay );
        doKern       = true;
    }
    else {
        oldOperation = WinSetDrawMode( winPaint );
        doKern       = false;
        if ( fastRenderAvailable && 16 == Prefs()->screenDepth ) {
            width = GrayFntCharsWidth( chars, length );
            doFastRender = ( width <= MAX_SCREEN_WIDTH );
        }
    }

    bitmapTopLeftX = 0;
    bitmapTopLeftY = 0;

    TxtGlueGetNextChar( chars, 0, &ch );
    firstKern = GetGlyph( ch )->leftKerning;

    switch ( resource.string[ RESOURCE_NAME_ORIENTATION ] )
    {
        case GRAY_FONT_LEFT:
            y++;
            toIncrement        = &y;
            incrementSign      = -1;
            drawBeforePosition = true;
            if ( doFastRender ) {
                bitmapTopLeftX = x;
                bitmapTopLeftY = y - width;
                x              = 0;
                y              = width - firstKern;
                tempBitmap->width    = currentFontPtr->header.fRectHeight;
                tempBitmap->height   = width - firstKern;
                tempBitmap->rowBytes = ( tempBitmap->width + 1 ) / 2 * 4;
            }
            break;
        case GRAY_FONT_RIGHT:
            toIncrement        = &y;
            incrementSign      = 1;
            drawBeforePosition = false;
            x -= currentFontPtr->header.fRectHeight;
            if ( doFastRender ) {
                bitmapTopLeftX = x;
                bitmapTopLeftY = y + firstKern;
                x              = 0;
                y              = 0;
                tempBitmap->width    = currentFontPtr->header.fRectHeight;
                tempBitmap->height   = width - firstKern;
                tempBitmap->rowBytes = ( tempBitmap->width + 1 ) / 2 * 4;
            }
            break;
        default:
            toIncrement        = &x;
            incrementSign      = 1;
            drawBeforePosition = false;
            if ( doFastRender ) {
                bitmapTopLeftX = x;
                bitmapTopLeftY = y + firstKern;
                x              = 0;
                y              = 0;
                tempBitmap->width  = width - firstKern;
                tempBitmap->height = currentFontPtr->header.fRectHeight;
                tempBitmap->rowBytes = ( tempBitmap->width + 1 ) / 2 * 4;
            }
            break;
    }
    doMapColors = false;
    if ( 8 <= Prefs()->screenDepth && 16 <= GetMaxBitDepth() ) {
        RGBColorType fore;
        RGBColorType back;
        WinSetTextColorRGB( NULL, &fore );
        WinSetBackColorRGB( NULL, &back );
        if ( doFastRender || 
             ( ( fore.r != 0 || fore.g != 0 || fore.b != 0 ||
                 back.r != 255 || back.g != 255 || back.b != 255 ) &&
                                               tempBitmap != NULL ) ) {
            ColorizeBitmapSetColorMap( &fore, &back, invert );
            if ( havePalmHiRes ) {
                ( ( PalmBitmapTypeV2* )tempBitmap )->flags =
                                              palmHasTransparency;
                ( ( PalmBitmapTypeV3* )tempBitmap )->pixelFormat =
                    palmPixelFormat565;
                tempBitmap->version = 3;
                ( ( PalmBitmapTypeV3* )tempBitmap )->transparentValue =
                    colorMap[ 0 ];
            }
            else {
                tempBitmap->version = 2;
            }
            doMapColors = true;
        }
    }
    doInvert    = false;
    if ( invert && tempBitmap != NULL && ! doMapColors ) {
        doInvert = true;
        if ( havePalmHiRes ) {
            ( ( PalmBitmapTypeV3* )tempBitmap )->pixelFormat =
                palmPixelFormatIndexed;
            tempBitmap->version = 3;
        }
        else {
            tempBitmap->version = 2;
        }
    }

    inOffset           = 0;
    while ( inOffset < length ) {
        GrayFontGlyphInfo*     glyph;
        UInt16                 resourceIndex;

        inOffset += TxtGlueGetNextChar( chars, inOffset, &ch );
        if ( length < inOffset )
            break;
        glyph = GetGlyph( ch );

        resourceIndex = glyph->resourceNumber - 1;
        if ( currentFontPtr->resourceCache[ resourceIndex ].handle != NULL ) {
            bitmapHandle = currentFontPtr->resourceCache[ resourceIndex ].handle;
        }
        else {
            UInt16   resourceID;
            resourceID = currentFontPtr->bitmapsResourceIndex[
                            resourceIndex ].resourceID;
            bitmapHandle = GetFontResource( resource.resType, resourceID,
                               &( currentFontPtr->resourceCache[
                                                      resourceIndex ].kind ) );
            currentFontPtr->resourceCache[ resourceIndex ].handle = bitmapHandle;
        }

        oldX = x;
        oldY = y;
        *toIncrement += incrementSign * glyph->leftKerning;
        if ( drawBeforePosition ) {
            /* adjust drawing position for CCW rotation */
            *toIncrement += incrementSign * glyph->bitmapWidth;
        }
        if ( bitmapHandle != NULL && (((uint64_t)bitmapHandle)) >> 56 != 0xff) {
            PalmBitmapType* bitmap;
            RectangleType   rect;
            UInt8*          bitmapResource;

            bitmapResource = MemHandleLock( bitmapHandle );
            bitmap = ( PalmBitmapType* ) ( bitmapResource +
                        ( ( GrayFontResourceIndexEntry* )bitmapResource )[
                                 glyph->positionInResourceIndex ].offset );
            if ( doKern && ( backgroundErase || invert ) ) {
                Int16  charRectWidth;
                charRectWidth = glyph->bitmapWidth - lastOverhang + 
                                    glyph->leftKerning;
                switch ( resource.string[ RESOURCE_NAME_ORIENTATION ] ) {
                    case GRAY_FONT_LEFT:
                        rect.topLeft.x = x;
                        rect.topLeft.y = y;
                        rect.extent.x  = currentFontPtr->header.fRectHeight;
                        rect.extent.y  = charRectWidth;
                        break;
                    case GRAY_FONT_RIGHT:
                        rect.topLeft.x = x;
                        rect.topLeft.y = y + lastOverhang - glyph->leftKerning;
                        rect.extent.x  = currentFontPtr->header.fRectHeight;
                        rect.extent.y  = charRectWidth;
                        break;
                    default:
                        rect.topLeft.x = x + lastOverhang - glyph->leftKerning;
                        rect.topLeft.y = y;
                        rect.extent.x  = charRectWidth;
                        rect.extent.y  = currentFontPtr->header.fRectHeight;
                        break;
                }
                if ( invert )
                    WinDrawRectangle( &rect, 0 );
                else
                    WinEraseRectangle( &rect, 0 );
            }
            if ( doInvert ) {
                InvertBitmap4( tempBitmap, bitmap );
                if ( doKern ) {
                    if ( tempBitmap->version < 3 ) {
                        tempBitmap->version = 2;
                        ( ( PalmBitmapTypeV2* )tempBitmap )->transparentIndex = 15;
                    }
                    else {
                        ( ( PalmBitmapTypeV3* )tempBitmap )->transparentValue = 15;
                    }
                }
                else
                    tempBitmap->flags &= ~palmHasTransparency;
                WinPaintBitmap( ( BitmapType* )tempBitmap, x, y );
            }
            else if ( doMapColors ) {
                Coord outputX;
                Coord outputY;
                if ( doFastRender ) {
                    outputX = x;
                    outputY = y;
                }
                else {
                    tempBitmap->width    = bitmap->width;
                    tempBitmap->height   = bitmap->height;
                    tempBitmap->rowBytes = 2 * bitmap->width;
                    outputX              = 0;
                    outputY              = 0;
                }
                ColorizeBitmap4To16( outputX, outputY, tempBitmap, bitmap );
                if ( ! doFastRender ) {
                    WinPaintBitmap( ( BitmapType* )tempBitmap, x, y );
                }
            }
            else {
                WinPaintBitmap( ( BitmapType* )bitmap, x, y );
            }
            MemHandleUnlock( bitmapHandle );
        }
        x = oldX;
        y = oldY;
        lastOverhang = glyph->bitmapWidth - glyph->advance + glyph->leftKerning;
        *toIncrement += incrementSign *  glyph->advance;
    }
    if ( doFastRender ) {
        WinDrawBitmap( ( BitmapType* )tempBitmap, bitmapTopLeftX,
            bitmapTopLeftY );
    }

    if ( doKern ) {
        WinSetDrawMode( oldOperation );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}




void GrayWinDrawChar
      (
      WChar ch,
      Coord x,
      Coord y
      )
{
    Char  line[80];
    Int16 length;
    if ( currentFontPtr == NULL ) {
        WinDrawChar( ch, x, y );
        return;
    }
    length = TxtGlueSetNextChar( line, 0, ch );
    GrayWinDrawChars( line, length, x, y );
}



Boolean GrayFntIsCurrentGray( void )
{
    return currentFontPtr != NULL;
}



/* Handle font substitution */
void GrayFntSubstitute
      (
      FontID oldFont,
      FontID newFont
      )
{
    if ( grayFont[ oldFont ] != NULL ) {
        GrayFntDelete( oldFont );
    }
    substitutionList[ oldFont ] = newFont;
    if ( currentFont == oldFont )
        GrayFntSetFont( currentFont );
}



void GrayFntClearSubstitutionList( void )
{
    Int16 i;
    for ( i = 0 ; i <= MAX_FONT_ID ; i++ )
         substitutionList[ i ] = i;
    GrayFntSetFont( currentFont );
}

