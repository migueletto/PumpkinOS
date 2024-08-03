/*
 * $Id: image.c,v 1.81 2004/02/17 01:22:28 chrish Exp $
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

#include "cache.h"
#include "control.h"
#include "debug.h"
#include "document.h"
#include "genericfile.h"
#include "history.h"
#include "hires.h"
#include "list.h"
//#include "loadbar.h"
#include "mainform.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "screen.h"
#include "uncompress.h"
#include "util.h"
#include "metadocument.h"
#include "rotate.h"

#include "image.h"
#include "const.h"
#include "pumpkin.h"
#include "mutex.h"
#include "AppRegistry.h"
#include "storage.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define KEEPALIVE_CACHE  true
#define RELEASE_CACHE    false

#define ImagesType   'Imgs'
static const Char imagesName[] = "Plkr-Images";

/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef enum ShowImagesEnum {
    IMAGE_LIMBO = 0,
    ALL_IMAGES,
    NO_IMAGES
} ShowImagesType;


/***********************************************************************
 *
 *      Local Functions
 *
 ***********************************************************************/
static void DrawInlineImageByHandle( MemHandle imageHandle, const TextContext* tContext, Coord* width ) IMAGE_SECTION;
static Boolean LoadFullScreenImageByHandle( MemHandle imageHandle, const Boolean newPage ) IMAGE_SECTION;

static void ReleaseImageHandle( MemHandle imageHandle, Boolean keepCache ) IMAGE_SECTION;
static LinkedList ReadMultiImageRecord( const Header* imageRecord, UInt16* totalImages ) IMAGE_SECTION;
static void DrawImageText( ImageType* image, Coord* width, Coord* height, const TextContext* tContext ) IMAGE_SECTION;
static void GetImageError( ImageType* image, Char* text ) IMAGE_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static MemHandle        fsImageHandle = NULL;
static ShowImagesType   imageStatus = IMAGE_LIMBO;
static DmOpenRef        imagesRef = NULL;


void ImageInit(void) {
  Err err;

  err = DmCreateDatabase(0, imagesName, ViewerAppID, ImagesType, true);
  if (err == errNone) {
    imagesRef = DmOpenDatabaseByTypeCreator(ImagesType, ViewerAppID, dmModeReadWrite);
  }
}

void ImageFinish(void) {
  LocalID dbID;

  if (imagesRef == NULL) return;

  DmCloseDatabase(imagesRef);

  dbID = DmFindDatabase(0, imagesName);
  if (dbID) {
    DmDeleteDatabase(0, dbID);
  }
  imagesRef = NULL;
}

static BitmapType *ImageLock(ImageType *image) {
  BitmapType *bmp = NULL;
  void *rec;
  UInt32 size;
  UInt16 index;

  if (image && imagesRef) {
    if (image->data.bitmapHandle) {
      rec = MemHandleLock(image->data.bitmapHandle);
    }
    if (image->imgHandle) {
      bmp = MemHandleLock(image->imgHandle);
//debug(1, "XXX", "ImageLock   image %p ref %d bmp %p", image, image->reference, bmp);
    } else if (image->data.bitmapHandle) {
      index = DmFindResource(imagesRef, bitmapRsc, image->reference, NULL);
      if (index == 0xffff) {
//debug(1, "XXX", "ImageLock   image %p ref %d load", image, image->reference);
        size = MemHandleSize(image->data.bitmapHandle);
        image->imgHandle = DmNewResourceEx(imagesRef, bitmapRsc, image->reference, size, rec);
        index = DmFindResource(imagesRef, 0, 0, image->imgHandle);
      }
      image->imgHandle = DmGetResourceIndex(imagesRef, index);
      bmp = MemHandleLock(image->imgHandle);
//debug(1, "XXX", "ImageLock image=%p rec=%p size=%d handle=%p index=%d bmp=%p", image, rec, size, image->imgHandle, index, bmp);
    }
  }

  return bmp;
}

static void ImageUnlock(ImageType *image) {
  if (image && imagesRef) {
//debug(1, "XXX", "ImageUnlock image %p ref %d", image, image->reference);
    if (image->imgHandle) {
      MemHandleUnlock(image->imgHandle);
    }
    if (image->data.bitmapHandle) {
      MemHandleUnlock(image->data.bitmapHandle);
    }
  }
}

/* Determine if Palm OS image compression can be used by the device */
Boolean NotSupportedImageCompression
    (
    BitmapType* bitmap    /* pointer to image structure */
    )
{
    BitmapCompressionType   compressionType;

    if ( imageStatus != IMAGE_LIMBO )
        return ( imageStatus == NO_IMAGES );

    compressionType = BmpGlueGetCompressionType( bitmap );
    if ( compressionType == BitmapCompressionTypeNone || 
         SupportCompressionType( compressionType ) ) {
        imageStatus = ALL_IMAGES;
    }
    else {
        imageStatus = NO_IMAGES;
    }

    return ( imageStatus == NO_IMAGES );
}



void GetImageMetrics
    (
    const Int16 reference,
    Coord*      width,
    Coord*      height
    )
{
    MemHandle   imageHandle;
    ImageType*  image;

    THROW_IF( width == NULL || height == NULL, dmErrInvalidParam );

    imageHandle = GetImageHandle( reference );
    image = LockImage( imageHandle );
    if ( image->err != errNone || imageStatus == NO_IMAGES ||
         image == NULL || ! ShowImages( GetHistoryCurrent() ) ) {
        DrawImageText( image, width, height, NULL );
    }
    else {
        *width  = image->width;
        *height = image->height;
    }
//debug(1, "XXX", "GetImageMetrics         image %p ref %d type %d %dx%d", image, image->reference, image->type, image->width, image->height);

    if ( ! HasCacheNode( IMAGEHANDLE, image->reference ) &&
         image->type != MULTIIMAGE ) {
        UnlockImage( image );
        FreeImageHandle( &imageHandle );
        return;
    }

    UnlockImage( image );
}



void DrawInlineImage
    (
    const Int16         reference,  /* bitmap's record ID */
    const TextContext*  tContext,   /* pointer to text context */
    Coord*              width       /* upon return, set to the width of the image */
    )
{
    MemHandle imageHandle;
    ImageType*     image;

    imageHandle = GetImageHandle( reference );
    DrawInlineImageByHandle( imageHandle, tContext, width );

    image = LockImage( imageHandle );
    if ( ! HasCacheNode( IMAGEHANDLE, image->reference ) &&
         image->type != MULTIIMAGE ) {
        MemHandleUnlock( imageHandle );
        FreeImageHandle( &imageHandle );
        return;
    }

    UnlockImage( image );
}



static void DrawInlineImageByHandle
    (
    MemHandle           imageHandle,
    const TextContext*  tContext,   /* pointer to text context */
    Coord*              width       /* upon return, set to the width of the image */
    )
{
    ImageType*  image;

    image = LockImage( imageHandle );
    if ( image->err != errNone || imageStatus == NO_IMAGES ||
          image == NULL ||
         ( ! ShowImages( GetHistoryCurrent() ) &&
           tContext->writeMode != WRITEMODE_NO_DRAW ) ) {
        DrawImageText( image, width, NULL, tContext );
        UnlockImage( image );
        return;
    }

    if ( tContext->writeMode == WRITEMODE_DRAW_CHAR ) {
        if ( image->type == MULTIIMAGE ) {
            MultiImageNodeType* node;

            *width = 0;
            node = ListFirst( image->data.multiList );
            while ( node != NULL ) {
                TextContext subTContext;
                Coord       subWidth;

                MemMove( &subTContext, tContext, sizeof( TextContext ) );
                subTContext.cursorX += node->position.topLeft.x;
                MSG( _( "image #%ld node position: %dx%d+%d+%d\n",
                      node->reference,
                      node->position.topLeft.x, node->position.topLeft.y,
                      node->position.extent.x, node->position.extent.y ) );
                subTContext.cursorY -= image->height - node->position.topLeft.y
                    - node->position.extent.y;
                DrawInlineImageByHandle( node->imageHandle, &subTContext,
                    &subWidth );
                node = ListNext( image->data.multiList, node );
            }
        }
        else {
            MSG( _( "image #%ld drawing position: %dx%d+%d+%d\n",
                  image->reference, tContext->cursorX,
                  (Int16)tContext->cursorY - image->height, image->width,
                  image->height ) );
            UInt32 ymax = TopLeftY() + ExtentY();
            UInt32 xmax = TopLeftX() + ExtentX();
            if (tContext->cursorY - image->height < ymax && tContext->cursorY >= 0 &&
                tContext->cursorX < xmax && tContext->cursorX + image->width >= 0) {
              RotDrawBitmap( image->bitmap, tContext->cursorX, tContext->cursorY - image->height );
            }
        }
    }

    *width = image->width;

    MSG( _( "image #%ld (type %d)\n", image->reference, image->type ) );

    UnlockImage( image );
}



Boolean LoadFullScreenImage
    (
    Header*         record, /* pointer to record */
    const Boolean   newPage /* true if the page is from the history */
    )
{
    MemHandle imageHandle;
    Boolean   result;

    imageHandle = GetImageHandle( record->uid );
    result = LoadFullScreenImageByHandle( imageHandle, newPage );
    return result;
}



static Boolean LoadFullScreenImageByHandle
    (
    MemHandle       imageHandle,
    const Boolean   newPage /* true if the page is from the history */
    )
{
    ImageType* image;
    Boolean    result;

    image = LockImage( imageHandle );
    result = true;
    if ( image == NULL  || image->err != errNone ) {
        Char errorText[ 255 ];

        GetImageError( image, errorText );
        FrmCustomAlert( errImageError, errorText, NULL, NULL );
        result = false;
    }
    else if ( image->type == MULTIIMAGE ) {
        MultiImageNodeType* node;

        node = ListFirst( image->data.multiList );
        while ( node != NULL ) {
            result = LoadFullScreenImageByHandle( node->imageHandle, newPage );
            node = ( result ) ? ListNext( image->data.multiList, node ) : NULL;
        }
    }

    UnlockImage( image );

    /* Although this assignment may be made multiple times when dealing with a
       MULTIIMAGE, always the latest one will prevail. Once the original call
       to LoadFullScreenImageByHandle is complete, fsImageHandle will either be
       a single image (when only one is present) or the original MULTIIMAGE
       imageHandle. */
    fsImageHandle = imageHandle;

    return result;
}



MemHandle GetFullscreenImageHandle( void )
{
    return fsImageHandle;
}



MemHandle GetImageHandle
    (
    const Int16 reference
    )
{
    MemHandle  imageHandle;
    ImageType* image;
    Header*    record;

    /* Check if we already have a valid imageHandle stored in cache */
    imageHandle = LoadFromCache( IMAGEHANDLE, reference );
    if ( imageHandle != NULL )
        return imageHandle;

    ErrTry {
        imageHandle = SafeMemHandleNew( sizeof( ImageType ) );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        FrmAlert( warnInsufficientMemory );
        return NULL;
    } ErrEndCatch

    image = MemHandleLock( imageHandle );
    MemSet( image, sizeof( ImageType ), 0 );

    image->reference    = reference;
    image->recordHandle = GetRecordHandle( reference );
    record = MemHandleLock( image->recordHandle );

    /* By default this can be set to 1. Will be adjusted if need be */
    image->totalImages = 1;

    image->err = errNone;
    switch ( record->type ) {
        case DATATYPE_TBMP_COMPRESSED:
            image->type = UNCOMPRESSED;
            ErrTry {
                image->data.bitmapHandle = Uncompress( record );
            }
            ErrCatch( err ) {
                FrmAlert( warnInsufficientMemory );
                image->err = err;
            } ErrEndCatch
            break;

        case DATATYPE_TBMP:
            image->type = DIRECT;
            image->data.bitmap = (BitmapType*) ( record + 1 );
            break;

        case DATATYPE_MULTIIMAGE:
            image->type = MULTIIMAGE;
            image->data.multiList = ReadMultiImageRecord( record,
                &image->totalImages );
            if ( image->data.multiList == NULL )
                image->err = dmErrMemError;
            break;

        default:
            image->err = errBadImageType;
            break;
    }
//debug(1, "XXX", "GetImageHandle          image %p ref %d type %d", image, image->reference, image->type);

    if ( image->err == errNone ) {
        if ( image->type == MULTIIMAGE ) {
            /* If we have a multi-record image, then we go into a controlled
               recursive loop that scans each image. The intention is at this
               point serves two purposes... */
            MultiImageNodeType* node;
            Coord               column;
            Coord               row;
            ImageType*          subImage;
            PointType           placeImage;
            //LoadBarType*        loadBar;

            image->width = image->height = 0;
            column = row = 0;
            placeImage.x = placeImage.y = 0;
            //loadBar = LoadBarNew( image->totalImages );

            node = ListFirst( image->data.multiList );
            while ( node != NULL ) {
                //LoadBarNextStep( loadBar );

                /* ...first it populates node->imageHandle. This is useful for
                   quick successive references of the sub-images directly... */
                node->imageHandle = GetImageHandle( node->reference );
                subImage = LockImage( node->imageHandle );
                if ( image == NULL || subImage->err != errNone ) {
                    image->err = subImage->err;
                    UnlockImage( subImage );
                    break;
                }

                /* ...next we try to strategically position the individual
                   subImage within its own unique place within the master image
                   display area... */
                if ( row < node->row ) {
                    placeImage.y += subImage->height;
                    placeImage.x = 0;
                }
                node->position.extent.x = subImage->width;
                node->position.extent.y = subImage->height;
                node->position.topLeft.x = placeImage.x;
                node->position.topLeft.y = placeImage.y - subImage->height;
                placeImage.x += subImage->width;

                /* ...then we discover the width, height, and pixelDepth and
                   store it at the base-level ImageType handle (the one that
                   was the original MULTIIMAGE type). This is useful for any
                   future requests to this image that only care to know how big
                   the whole master image is. */
                if ( column < node->column ) {
                    column = node->column;
                    image->width += subImage->width;
                }
                if ( row < node->row ) {
                    row = node->row;
                    image->height += subImage->height;
                }
                image->pixelDepth = subImage->pixelDepth;

                MSG( _( "Placing image #%ld @ %dx%d+%d+%d (%dx%d)\n",
                      node->reference,
                      node->position.topLeft.x,
                      node->position.topLeft.y,
                      node->position.extent.x,
                      node->position.extent.y,
                      image->width, image->height ) );

                /* Pretty cool, eh? :) */

                UnlockImage( subImage );
                node = ListNext( image->data.multiList, node );
            }
            //LoadBarFree( loadBar );
        }
        else {
            /* Locking at this point is only necessary to ensure we have
               a valid image->bitmap */
            LockImage( imageHandle );
            if ( image != NULL && image->err == errNone ) {
                if ( NotSupportedImageCompression( image->bitmap ) ) {
                    image->err = errBadImageCompression;
                }
                else {
                    BmpGlueGetDimensions( image->bitmap, &image->width,
                        &image->height, NULL );
//debug(1, "XXX", "BmpGlueGetDimensions    image %p ref %d type %d dim %d,%d", image, image->reference, image->type, image->width, image->height);
                    image->pixelDepth = BmpGlueGetBitDepth( image->bitmap );
                    if ( GetMaxBitDepth() < image->pixelDepth )
                        image->err = errImageTooHighBitDepth;
                    else
                        OptimizeImage( image );
                }
            }
            UnlockImage( image );
        }
    }

    AddPtrToCache( IMAGEHANDLE, reference, imageHandle, FreeImageHandle );
#if 0

    /* If our image is either optimized or uncompressed, its data is
       currently being stored within the precious dynamic heap. Transfer it off
       to the storage heap handled by the cache */
    if ( image->type == OPTIMIZED ||
         image->type == UNCOMPRESSED ||
         image->type == VERSION3_BMP )
        SaveImageInStorageCache( imageHandle );
#endif

    /* Unfortunatly, writing to cache has to preceed converting the image to
       v3. Short of mucking around with the internals of palm bitmaps and
       potentially causing a crash on a future OS, we end up with a garbled
       mess if we first try to convert to v3, then save to cache :( 
       Thus, we do the conversion at display time.  This shouldn't waste
       much CPU time, probably.  After all, the conversion is done by
       ARM code in the OS since this is only for OS5+. */
/*
    if ( image->type != UNKNOWN && image->type != MULTIIMAGE ) {
        if ( ConvertImageHandleToV3( imageHandle ) )
            MSG( _( "Converted to a version3 bitmap\n" ) );
    }
*/

    MemHandleUnlock( image->recordHandle );
    MemHandleUnlock( imageHandle );

    return imageHandle;
}



/* Move an image from the dynamic heap to the storage heap within the cache */
void SaveImageInStorageCache
    (
    MemHandle imageHandle
    )
{
    ImageType* image;
    UInt32     bmpHeaderSize;
    UInt32     bmpDataSize;
    MemHandle  cacheHandle;
    UInt8*     cachePtr;
    Boolean    separateData;

    image = LockImage( imageHandle );

    if ( image == NULL || image->err != errNone )
        return;

//debug(1, "XXX", "SaveImageInStorageCache image %p ref %d type %d", image, image->reference, image->type);
    if ( Support40() ) {
        BmpGetSizes( image->bitmap, &bmpDataSize, &bmpHeaderSize );
        separateData = true;
    }
    else if ( Support35() ) {
        bmpHeaderSize = BmpSize( image->bitmap );
        bmpDataSize   = BmpBitsSize( image->bitmap );

        /* If we're dealing with a non-indirect bitmap then BmpSize() will
           return not only the header but the databits with its value. Easy to
           adjust. */
        if ( bmpDataSize < bmpHeaderSize )
            bmpHeaderSize -= bmpDataSize;
        separateData = true;
    }
    else {
        /* for OS <3.5, our bitmaps are not optimized and hence are
           portable since they come right from the distiller */
        bmpHeaderSize  = PortableBmpSize( image->bitmap );
        bmpDataSize    = 0;
        separateData   = false;
    }

    cacheHandle = AllocateCacheRecord( IMAGEHANDLE,
        image->reference, bmpHeaderSize + bmpDataSize );

    if ( cacheHandle == NULL ) {
        UnlockImage( image );
        return;
    }

    cachePtr = MemHandleLock( cacheHandle );

    DmWrite( cachePtr, 0, image->bitmap, bmpHeaderSize );
    if ( separateData )
        DmWrite( cachePtr, bmpHeaderSize, BmpGetBits( image->bitmap ),
            bmpDataSize );

    MemHandleUnlock( cacheHandle );

    UnlockImage( image );

    SetImageHandleType( imageHandle, CACHE_STORAGE, cacheHandle );
}



void SetImageHandleType
    (
    MemHandle     imageHandle,
    ImageTypeEnum type,
    void*         bitmapData
    )
{
    ImageType* image;

    ReleaseImageHandle( imageHandle, KEEPALIVE_CACHE );
    image = MemHandleLock( imageHandle );
//debug(1, "XXX", "SetImageHandleType      image %p ref %d type %d -> %d", image, image->reference, image->type, type);
    image->type = type;
    switch ( image->type ) {
        case DIRECT:
        case OPTIMIZED:
        case VERSION3_BMP:
            image->data.bitmap = (BitmapType*) bitmapData;
            break;

        case UNCOMPRESSED:
        case CACHE_STORAGE:
            image->data.bitmapHandle = (MemHandle) bitmapData;
            break;

        default:
            break;
    }
    MemHandleUnlock( imageHandle );
}



ImageType* LockImage
    (
    MemHandle imageHandle
    )
{
    ImageType* image;
    Boolean    actualImage;

    if ( imageHandle == NULL )
        return NULL;

    image = MemHandleLock( imageHandle );
    image->bitmap = NULL;

    actualImage = false;

    if ( image->err == errNone ) {
//debug(1, "XXX", "LockImage               image %p ref %d type %d", image, image->reference, image->type);
        switch ( image->type ) {
            case DIRECT:
            case OPTIMIZED:
            case VERSION3_BMP:
                image->bitmap = image->data.bitmap;
                actualImage = true;
                break;

            case UNCOMPRESSED:
            case CACHE_STORAGE:
/*
                if ( image->data.bitmapHandle != NULL ) {
                    image->bitmap = MemHandleLock( image->data.bitmapHandle );
                }
*/
                image->bitmap = ImageLock(image);
                actualImage = true;
                break;

            case WINDOW_HANDLE:
            case MULTIIMAGE:
                /* Do Nothing */
                break;

            default:
                image->err = errBadImageType;
                break;
        }
    }

    if ( actualImage && image->bitmap == NULL && image->err == errNone )
        /* Aw crap. Although we cannot confirm it, we probably just ran
           out of space on either the dynamic or storage heap :( */
        image->err = dmErrMemError;

    return image;
}



MemHandle UnlockImage
    (
    ImageType* image
    )
{
    MemHandle imageHandle;

    if ( image == NULL )
        return NULL;

//debug(1, "XXX", "UnlockImage             image %p ref %d type %d", image, image->reference, image->type);
    if ( image->err == errNone ) {
        switch ( image->type ) {
            case UNCOMPRESSED:
            case CACHE_STORAGE:
                //MemHandleUnlock( image->data.bitmapHandle );
                ImageUnlock(image);
                break;

            default:
                break;
        }
    }
    image->bitmap = NULL;

    imageHandle = MemPtrRecoverHandle( image );
    MemHandleUnlock( imageHandle );

    return imageHandle;
}



static void ReleaseImageHandle
    (
    MemHandle imageHandle,
    Boolean   keepCache
    )
{
    ImageType* image;

    image = MemHandleLock( imageHandle );

    switch ( image->type ) {
        case MULTIIMAGE:
            MSG( _( "Releasing MULTIIMAGE handle\n" ) );
            ListRelease( image->data.multiList );
            break;

        case UNCOMPRESSED:
            MSG( _( "Releasing UNCOMPRESSED handle\n" ) );
            break;

        case OPTIMIZED:
            MSG( _( "Releasing OPTIMIZED handle\n" ) );
            PortableBmpDelete( image->data.bitmap );
            image->data.bitmap = NULL;
            break;

        case VERSION3_BMP:
            MSG( _( "Releasing VERSION3_BMP handle\n" ) );
            BmpDelete( image->data.bitmap );
            image->data.bitmap = NULL;
            /* By setting type to VERSION3_BMP, we indicate that we created
               this bitmap ourselves by using the image in the storage cache as
               base data.  Since we no longer need our bitmap, we still need to
               clear out the cache so... FALLTHROUGH*/

        case CACHE_STORAGE:
            if ( ! keepCache ) {
                MSG( _( "Releasing CACHE_STORAGE handle\n" ) );
                RemoveCacheRecord( IMAGEHANDLE, image->reference );
            }
            break;


        default:
            break;
    }
    MemHandleUnlock( imageHandle );
}



void FreeImageHandle
    (
    MemHandle* imageHandle
    )
{
    ImageType* image;

    if ( *imageHandle == NULL )
        return;

    /* Lock imageHandle one last time to free any remaining artifacts */
    image = MemHandleLock( *imageHandle );
//debug(1, "XXX", "FreeImageHandle         image %p ref %d type %d", image, image->reference, image->type);
    MSG( _( "freeing image refID %ld (type %d)\n", image->reference,
          image->type ) );
    if ( image->recordHandle != NULL ) {
        FreeRecordHandle( &image->recordHandle );
    }
    MemHandleUnlock( *imageHandle );
    ReleaseImageHandle( *imageHandle, RELEASE_CACHE );

    /* Remove imageHandle from memory */
    SafeMemHandleFree( *imageHandle );
    *imageHandle = NULL;
}



void ReleaseFsImageHandle( void )
{
    ImageType*          imagePtr;
    ImageType*          subImagePtr;
    MultiImageNodeType* node;

    /* Reset/RemoveCache will take care of CACHE_STORAGE */
    if ( fsImageHandle != NULL ) {
        imagePtr = MemHandleLock( fsImageHandle );
        if ( HasCacheNode( IMAGEHANDLE, imagePtr->reference ) ) {
            MemHandleUnlock( fsImageHandle );
        }
        else if ( imagePtr->type == MULTIIMAGE ) {
            node = ListFirst( imagePtr->data.multiList );
            while ( node != NULL ) {
                subImagePtr = MemHandleLock( node->imageHandle );
                if ( subImagePtr->type != CACHE_STORAGE ) {
                    MemHandleUnlock( node->imageHandle );
                    FreeImageHandle( &(node->imageHandle) );
                }
                node = ListNext( imagePtr->data.multiList, node );
            }
            MemHandleUnlock( fsImageHandle );
            FreeImageHandle( &fsImageHandle );
        }
        else if ( imagePtr->type != CACHE_STORAGE ) {
            MemHandleUnlock( fsImageHandle );
            FreeImageHandle( &fsImageHandle );
        }
        fsImageHandle = NULL;
    }
}



static LinkedList ReadMultiImageRecord
    (
    const Header* imageRecord,
    UInt16*       totalImages
    )
{
    LinkedList           list;
    MultiImageNodeType*  node;
    MultiImageRecordType* multiImageRecord;
    UInt8*               images;
    UInt16               i;
    UInt16               thisColumn = 1;
    UInt16               thisRow = 1;
    Int16                tmp;

    list = ListCreate();

    ErrTry {
        multiImageRecord = SafeMemPtrNew( imageRecord->size );
    }
    ErrCatch( UNUSED_PARAM(err) ) {
        FrmAlert( warnInsufficientMemory );
        return NULL;
    } ErrEndCatch

    MemMove( multiImageRecord, (UInt8*) imageRecord + sizeof( Header ),
        imageRecord->size );

    *totalImages = ( imageRecord->size - sizeof( MultiImageRecordType ) ) / 2;
    images      = (UInt8*) multiImageRecord + sizeof( MultiImageRecordType );

    MSG( _( "multiImageRecord->rows: %d\n", multiImageRecord->rows ) );
    MSG( _( "multiImageRecord->columns: %d\n", multiImageRecord->columns ) );
    MSG( _( "totalImages: %d\n", *totalImages ) );

    for ( i = 0; i < *totalImages; i++ ) {
        ErrTry {
            node = SafeMemPtrNew( sizeof( MultiImageNodeType ) );
        }
        ErrCatch( UNUSED_PARAM(err) ) {
            FrmAlert( warnInsufficientMemory );
            SafeMemPtrFree( multiImageRecord );
            ListRelease( list );
            return NULL;
        } ErrEndCatch
        MemSet( node, sizeof( MultiImageNodeType ), 0 );
        /* The record's references are Int16 and we need to */
        /* move these into a Int32 in the structure.        */
        /* So we copy it to a temporary Int16 first.        */
        MemMove( &tmp, images + ( i * 2 ), sizeof( UInt16 ) );
        node->reference = tmp;
        node->column = thisColumn++;
        node->row = thisRow;
        if ( multiImageRecord->columns < thisColumn ) {
            thisColumn = 1;
            thisRow++;
        }
        MSG( _( "found refID %ld (+%d+%d) at %d\n", node->reference,
            node->column, node->row, i ) );
        ListAppend( list, node );
    }

    SafeMemPtrFree( multiImageRecord );
    return list;
}



/* Optimize an image based upon screendepth and OS */
Boolean OptimizeImage_None
    (
    ImageType* image
    )
{
    return false;
}



/* Optimize an image based upon screendepth and OS */
Boolean OptimizeImage_OS35
    (
    ImageType* image
    )
{
    BitmapType* tempBitmapPtr;
    Err         err;
    WinHandle   tempWindow;
    WinHandle   screenWindow;
    MemHandle   imageHandle;

    MSG( _( "Optimizing Image...\n" ) );
    MSG( _( "Before: %dx%dx%d (%ld bytes)\n", image->width, image->height,
        image->pixelDepth, (UInt32) BmpBitsSize( image->bitmap ) ) );
//debug(1, "XXX", "OptimizeImage_OS35      image %p ref %d type %d", image, image->reference, image->type);

    if ( image->pixelDepth <= Prefs()->screenDepth ) {
        MSG( _( "Image cannot be optimized further\n" ) );
        return false;
    }

    /* Set a new depth for the optimized image. If the original image's depth
       is greater than the current screenDepth, bring it down to match the
       display */
    if ( Prefs()->screenDepth < image->pixelDepth )
        image->pixelDepth = Prefs()->screenDepth;

    tempBitmapPtr = PortableBmpCreate( image->width, image->height,
        image->pixelDepth, NULL, &err );
    if ( tempBitmapPtr == NULL || err != errNone ) {
        MSG( _( "BmpCreate() failed! Error %d\n", err ) );
        return false;
    }

    tempWindow = WinCreateBitmapWindow( tempBitmapPtr, &err );
    if ( tempWindow == NULL || err != errNone ) {
        PortableBmpDelete( tempBitmapPtr );
        MSG( _( "WinCreateBitmapWindow() failed! Error %d\n", err ) );
        return false;
    }

    screenWindow = WinSetDrawWindow( tempWindow );
    WinDrawBitmap( image->bitmap, 0, 0 );
    WinSetDrawWindow( screenWindow );
    WinDeleteWindow( tempWindow, false );
    imageHandle = UnlockImage( image );

    SetImageHandleType( imageHandle, OPTIMIZED, tempBitmapPtr );

    LockImage( imageHandle );
    BmpGlueGetDimensions( image->bitmap, &image->width, &image->height, NULL );
    image->pixelDepth = BmpGlueGetBitDepth( image->bitmap );
    MSG( _( "After: %dx%dx%d (%ld bytes)\n", image->width, image->height,
        image->pixelDepth, (UInt32) BmpBitsSize( image->bitmap ) ) );

    return true;
}



/* Determine if images should be rendered for this record */
Boolean ShowImages
    (
    Int16 reference /* record number */
    )
{
    return ! GetBitStatus( SHOW_IMAGES_ID, reference );
}



/* Turn on image rendering for this record */
void ShowImagesOn
    (
    Int16 reference /* record number */
    )
{
    SetBitStatus( SHOW_IMAGES_ID, reference, false );
}



/* Turn off image rendering for this record */
void ShowImagesOff
    (
    Int16 reference /* record number */
    )
{
    SetBitStatus( SHOW_IMAGES_ID, reference, true );
}



static void DrawImageText
    (
    ImageType*         image,
    Coord*             width,   /* Pointer to width of text */
    Coord*             height,  /* Pointer to height of text */
    const TextContext* tContext /* NULL if not drawing to the screen */
    )
{
    Char drawText[ 255 ];

    if ( image->err != errNone ) {
        Char errorText[ 255 ];

        GetImageError( image, errorText );
        StrPrintF( drawText, "[img error: %s]", errorText );
    }
    else {
        StrPrintF( drawText, "[img]" );
    }

    if ( width != NULL )
        *width  = FntCharsWidth( drawText, StrLen( drawText ) );
    if ( height != NULL )
        *height = FntCharHeight();

    if ( tContext != NULL )
        DrawText( drawText, StrLen( drawText ), tContext );
}


static void GetImageError
    (
    ImageType* image,
    Char*      text
    )
{
    switch ( image->err ) {
        case errBadImageType:
            StrPrintF( text, "Bad image type" );
            break;

        case errImageTooHighBitDepth:
            StrPrintF( text, "Too high bit depth" );
            break;

        case errBadImageCompression:
            StrPrintF( text, "Bad image compression" );
            break;

        case dmErrMemError:
            StrPrintF( text, "Low dynamic memory" );
            break;

        case errZLibMemError:
            StrPrintF( text, "Zlib memory error" );
            break;

        default:
            StrPrintF( text, "0x%x", image->err );
    }
}



#ifdef HAVE_IMODE
/* Draws an I-mode icon */
void DisplayImode
    (
    DmOpenRef*   plkrImodeDB,
    TextContext* tContext,
    UInt16       index,
    Int16*       width,
    Int16*       height
    )
{
    MemHandle     imageHandle;
    BitmapType*   imagePtr;
    Coord         imageWidth;
    Coord         imageHeight;

    imageHandle = DmQueryRecord( plkrImodeDB, index );

    if ( imageHandle != NULL ) {
      imagePtr = MemHandleLock( imageHandle );

      BmpGlueGetDimensions( imagePtr, &imageWidth,
                &imageHeight, NULL );
      *width  = imageWidth;
      *height = imageHeight;

      RotDrawBitmap( imagePtr, tContext->cursorX,
             tContext->cursorY - imageHeight );

    MemHandleUnlock( imageHandle );
    FreeRecordHandle( &imageHandle );
    }
}
#endif

