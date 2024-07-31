/*
 * $Id: image.h,v 1.28 2004/02/04 16:09:06 chrish Exp $
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

#ifndef PLUCKER_IMAGE_H
#define PLUCKER_IMAGE_H

#include "document.h"
#include "list.h"

#include "viewer.h"

typedef enum {
    UNKNOWN = 0,
    MULTIIMAGE,
    DIRECT,
    UNCOMPRESSED,
    OPTIMIZED,
    CACHE_STORAGE,
    VERSION3_BMP,
    WINDOW_HANDLE,
} ImageTypeEnum;

typedef struct {
    Int32         reference;
    ImageTypeEnum type;
    MemHandle     recordHandle;
    union {
        MemHandle   bitmapHandle; /* if type == COMPRESSED || CACHE_STORAGE */
        LinkedList  multiList;    /* if type == MULTIIMAGE */
        BitmapType* bitmap;       /* if type == DIRECT || OPTIMIZED || V3_BMP */
        WinHandle   window;       /* if type == WINDOW_HANDLE ( < palm 3.5) */
    } data;
    Coord         width;
    Coord         height;
    UInt8         pixelDepth;
    UInt16        totalImages;
    Err           err;
    /* Before the image can be used, it must be locked with LockImage(). It's
     * only during this time that this last BitmapType pointer will be valid
     * and available for use. Subsequently, be sure to call UnlockImage() when
     * you're done. */
    BitmapType*   bitmap;
    MemHandle     imgHandle;
} ImageType;

typedef struct {
    Int16 columns;
    Int16 rows;
} MultiImageRecordType;

typedef struct {
    Int32         reference;
    MemHandle     imageHandle;
    ImageType*    image;
    RectangleType position;
    Int16         column;
    Int16         row;
} MultiImageNodeType;


extern void ImageInit(void);
extern void ImageFinish(void);

extern Boolean NotSupportedImageCompression( BitmapType* bitmap ) IMAGE_SECTION;
extern void GetImageMetrics( const Int16 reference, Coord* width, Coord* height ) IMAGE_SECTION;
extern void DrawInlineImage( const Int16 reference, const TextContext* tContext, Coord* width ) IMAGE_SECTION;
extern Boolean LoadFullScreenImage( Header* record, const Boolean newPage ) IMAGE_SECTION;
extern MemHandle GetFullscreenImageHandle( void ) IMAGE_SECTION;
extern void SetImageHandleType( MemHandle imageHandle, ImageTypeEnum type, void* bitmapData ) IMAGE_SECTION;
extern ImageType* LockImage( MemHandle imageHandle ) IMAGE_SECTION;
extern MemHandle UnlockImage( ImageType* image ) IMAGE_SECTION;
extern Boolean OptimizeImage_None( ImageType* image ) IMAGE_SECTION;
extern Boolean OptimizeImage_OS35( ImageType* image ) IMAGE_SECTION;
extern Boolean ShowImages( Int16 reference ) IMAGE_SECTION;
extern void ShowImagesOn( Int16 reference ) IMAGE_SECTION;
extern void ShowImagesOff( Int16 reference ) IMAGE_SECTION;
extern void SaveImageInStorageCache( MemHandle imageHandle ) IMAGE_SECTION;
extern void FreeImageHandle( MemHandle* imageHandle ) IMAGE_SECTION;
extern MemHandle GetImageHandle( const Int16 reference ) IMAGE_SECTION;
extern void ReleaseFsImageHandle( void ) IMAGE_SECTION;

#ifdef HAVE_IMODE
extern void DisplayImode ( DmOpenRef* plkrImodeDB, TextContext* tContext,
                 UInt16 index, Int16* width, Int16* height ) IMAGE_SECTION;
#endif

PLKR_GLOBAL Boolean (*OptimizeImage)( ImageType* image );

#endif

