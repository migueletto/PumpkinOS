/*
 * $Id: rotatebitmap.c,v 1.16 2003/11/19 01:51:59 prussar Exp $
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
#include "rotatebitmap.h"

#define DO_ROTATE( shiftX, maskX, maskBits, maskXshift ) \
            if ( clockwise ) { \
                for ( inX = 0 ; inX < width ; inX ++ ) { \
                    inPtr    = src + ( inX >> (shiftX) ); \
                    inShift  = ( inX & (maskX) ) << (maskXshift); \
                    inMask   = (maskBits) >> inShift; \
                    outPtr = dest + inX * ( UInt32 ) destRowBytes; \
                    /* outY = inX */ \
                    for ( outX = height - 1 ; inPtr < srcEnd ; \
                                  outX--, inPtr += srcRowBytes ) { \
                         outShift  = ( outX & (maskX) ) << (maskXshift); \
                         outPtr[ outX >> (shiftX) ] |= ( ( *inPtr & inMask ) << \
                                                inShift ) >> outShift; \
                    } \
                } \
            } \
            else { \
                Int16 inY; \
                for ( inX = 0 ; inX < width ; inX ++ ) { \
                    inPtr     = src + ( inX >> (shiftX) ); \
                    inShift   = ( inX & (maskX) ) << (maskXshift); \
                    inMask    = (maskBits) >> inShift; \
                    outPtr = dest + ( width - 1 - inX ) * \
                                      ( UInt32 ) destRowBytes; \
                    for ( inY = 0 ; inY < height ; \
                                      inY++, inPtr += srcRowBytes ) { \
                         /* outX = inY */ \
                         outShift  = ( inY & (maskX) ) << (maskXshift); \
                         outPtr[ inY >> (shiftX) ] |= ( ( *inPtr & inMask ) << \
                                                         inShift ) >> outShift; \
                    } \
                } \
            }


/* Rotate a 1-, 2-, 4-, 8- or 16-bit bitmap 90 degrees, CW or CCW */
/* The 1-bit version should be maximally optimized as it's used for all
   text in rotated mode on OS 3.5+ if the background is white. */
void RotateBitmap
    (
    Int16   width,
    Int16   height,
    UInt8*  dest,
    UInt16  destRowBytes,
    UInt8*  src,
    UInt16  srcRowBytes,
    Boolean clockwise,
    UInt8   bitDepth
    )
{
    Int16  inX;
    Int16  outX;
    UInt8  inMask;
    UInt8  inShift;
    UInt8  outShift;
    UInt8* inPtr;
    UInt8* outPtr;
    UInt8* srcEnd;

    if ( width == 0 || height == 0 )
        return;

    if ( bitDepth < 8 ) {
        UInt32 length;
        UInt8* p;

        length = ( UInt32 ) destRowBytes * width;
        p      = dest;

        while ( length-- )
            *p++ = 0;
    }

    srcEnd = src + ( UInt32 ) srcRowBytes * height;

    switch ( bitDepth ) {
        case 1: {
            UInt16 wholeBytes;
            UInt16 remainingBits;
            Int16  i;
            UInt8  outMask;
            UInt8  theByte;

            wholeBytes    = width >> 3;
            remainingBits = width & 7;

            if ( clockwise ) {
                outPtr  = dest + ( ( height - 1 ) >> 3 );
                outMask = 0x80 >> ( ( height - 1 ) & 7 );
                for ( inPtr = src ; inPtr < srcEnd ; inPtr += srcRowBytes ) {
                     UInt8* startOutPtr;
                     UInt8* startInPtr;
                     startOutPtr = outPtr;
                     startInPtr  = inPtr;
                     for ( i = wholeBytes ; 0 < i ; i-- ) {
                          theByte = *inPtr;
                          if ( theByte & 0x80 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x40 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x20 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x10 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x08 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x04 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x02 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          if ( theByte & 0x01 )
                              *outPtr |= outMask;
                          outPtr += destRowBytes;
                          inPtr++;
                     }
                     if ( 0 < remainingBits ) {
                         inMask  = 0x80;
                         theByte = *inPtr;
                         for ( i = remainingBits ; 0 < i ; i-- ) {
                              if ( theByte & inMask )
                                  *outPtr |= outMask;
                              outPtr += destRowBytes;
                              inMask >>= 1;
                         }
                     }
                     outMask <<= 1;
                     if ( outMask == 0 ) {
                         outPtr  = startOutPtr - 1;
                         outMask = 1;
                     }
                     else {
                         outPtr = startOutPtr;
                     }
                     inPtr = startInPtr;
                }
            }
            else {
                outPtr  = dest + destRowBytes * ( width - 1 );
                outMask = 0x80;
                for ( inPtr = src; inPtr < srcEnd ; inPtr += srcRowBytes ) {
                     UInt8* startOutPtr;
                     UInt8* startInPtr;
                     startOutPtr = outPtr;
                     startInPtr  = inPtr;
                     theByte = *inPtr;
                     inMask = 0x80;
                     for ( i = wholeBytes ; 0 < i ; i-- ) {
                          theByte = *inPtr;
                          if ( theByte & 0x80 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x40 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x20 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x10 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x08 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x04 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x02 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          if ( theByte & 0x01 )
                              *outPtr |= outMask;
                          outPtr -= destRowBytes;
                          inPtr++;
                     }
                     if ( 0 < remainingBits ) {
                         theByte = *inPtr;
                         inMask  = 0x80;
                         for ( i = remainingBits ; 0 < i ; i-- ) {
                              if ( theByte & inMask )
                                  *outPtr |= outMask;
                              outPtr -= destRowBytes;
                              inMask >>= 1;
                         }
                     }
                     outMask >>= 1;
                     if ( outMask == 0 ) {
                         outPtr  = startOutPtr + 1;
                         outMask = 0x80;
                     }
                     else {
                         outPtr = startOutPtr;
                     }
                     inPtr = startInPtr;
                }
            }
            break;
        }

        case 2:
            DO_ROTATE( 2, 3, 0x80 + 0x40, 1 );
            break;

        case 4:
            DO_ROTATE( 1, 1, 0x80 + 0x40 + 0x20, 2 );
            break;

        case 8:
            if ( clockwise ) {
                for ( inX = 0 ; inX < width ; inX ++ ) {
                    inPtr  = src + inX;
                    outPtr = dest + inX * ( UInt32 )destRowBytes + height - 1;
                    /* outY = inX */
                    for ( ; inPtr < srcEnd ; outPtr--, inPtr += srcRowBytes ) {
                         *outPtr = *inPtr;
                    }
                }
            }
            else {
                for ( inX = 0 ; inX < width ; inX ++ ) {
                    inPtr  = src + inX;
                    outPtr = dest + ( width - 1 - inX ) *
                                      ( UInt32 )destRowBytes;
                    for ( ; inPtr < srcEnd; outPtr++, inPtr += srcRowBytes ) {
                         /* outX = inY */
                         *outPtr = *inPtr;
                    }
                }
            }
            break;

        case 16:
            if ( clockwise ) {
                for ( inX = 0 ; inX < width ; inX ++ ) {
                    inPtr  = src + inX * 2;
                    outPtr = dest + inX * ( UInt32 ) destRowBytes +
                                 ( height - 1 ) * 2;
                    /* outY = inX */
                    for ( ; inPtr < srcEnd ;
                                      outPtr -= 2, inPtr += srcRowBytes ) {
                         * ( UInt16* ) outPtr = * ( UInt16* ) inPtr;
                    }
                }
            }
            else {
                for ( inX = 0 ; inX < width ; inX ++ ) {
                    inPtr  = src + inX * 2;
                    outPtr = dest + ( width - 1 - inX ) *
                                        ( UInt32 ) destRowBytes;
                    for ( ; inPtr < srcEnd ;
                                      outPtr += 2, inPtr += srcRowBytes ) {
                         /* outX = inY */
                         * ( UInt16* ) outPtr = * ( UInt16* ) inPtr;
                    }
                }
            }
            break;
    }
}

