/*
 * $Id: palmbitmap.h,v 1.3 2003/11/30 15:50:27 prussar Exp $
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
 *
 */


/* This header file defines various structures for Palm bitmaps.
   There is no guarantee that OS functions will generate bitmaps
   in these formats, but they accept bitmaps in these formats
   (with the correct version numbers). */

#ifndef PLUCKER_PALMBITMAP_H
#define PLUCKER_PALMBITMAP_H

#define palmDirectColor              0x04
#define palmHasTransparency          0x20
#define palmHasColorTable            0x40  /* constants for bitmap flags */
#define palmCompressed               0x80


#define RED_BITS                     5
#define GREEN_BITS                   6
#define BLUE_BITS                    5
#define MAX_RED                      ( ( 1 << RED_BITS ) - 1 )
#define MAX_GREEN                    ( ( 1 << GREEN_BITS ) - 1 )
#define MAX_BLUE                     ( ( 1 << BLUE_BITS ) - 1 )

typedef enum {
        palmPixelFormatIndexed = 0,
        palmPixelFormat565,
        palmPixelFormat565LE,
        palmPixelFormatIndexedLE,
} PalmPixelFormatType;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
} PalmBitmapType;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt16 reserved[4];
} PalmBitmapTypeV0;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
        UInt16 nextDepthOffset;
        UInt16 reserved[2];
} PalmBitmapTypeV1;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
        UInt16 nextDepthOffset;
        UInt8  transparentIndex;
        UInt8  compressionType;
        UInt16 reserved;
} PalmBitmapTypeV2;

typedef struct {
       Int16  width;
       Int16  height;
       UInt16 rowBytes;
       UInt8  flags;
       UInt8  reserved0;
       UInt8  pixelSize;
       UInt8  version;
       UInt8  size;
       UInt8  pixelFormat;
       UInt8  unused;
       UInt8  compressionType;
       UInt16 density;
       UInt32 transparentValue;
       UInt32 nextBitmapOffset;
} PalmBitmapTypeV3;

typedef enum {
    palmBitmapCompressionTypeScanLine = 0,
    palmBitmapCompressionTypeRLE,
    palmBitmapCompressionTypePackBits,
    palmBitmapCompressionTypeEnd,
    palmBitmapCompressionTypeBest = 0x64,
    palmBitmapCompressionTypeNone = 0xFF
} PalmBitmapCompressionType;

typedef struct {
        UInt16 numEntries;
        /* RGBColorType entry[]; */
} PalmColorTableType;

typedef struct {
    UInt8 redBits;
    UInt8 greenBits;
    UInt8 blueBits;
    UInt8 reserved;
    RGBColorType transparentColor;
} PalmBitmapDirectInfoType;

#endif /* PALM_BITMAP_H */

