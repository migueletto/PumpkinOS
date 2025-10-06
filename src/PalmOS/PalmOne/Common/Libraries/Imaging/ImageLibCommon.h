/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Imaging
 */

/**
 * @file 	ImageLibCommon.h
 * @version 1.0
 * @brief	Common defines and error codes to be used with Imaging Library API (ImageLib.h)
 */

#ifndef IMAGELIB_COMMON_H
#define IMAGELIB_COMMON_H

#include <PalmOS.h>
#include <PalmTypes.h>

#define ImageLibCreatorID 'imgL'            /**< Imaging library creator ID */
#define ImageLibTypeID    'libr'            /**< Standard library type */
#define ImageLibName      "ImageLibrary"    /**< Imaging library database name */


/// @name Preview Image Dimensions
/// Preview height and width in standard pixels. On a double density device,
/// the bitmap returned would be twice these dimensions.
/*@{*/
#define kPalmSizeImageWidth     160
#define kPalmSizeImageHeight    120
#define kFullSizeImageWidth     640
#define kFullSizeImageHeight    480
#define kThumbnailImageWidth     40
#define kThumbnailImageHeight    30
#define kPreviewBitmapWidth     (kPalmSizeImageWidth)
#define kPreviewBitmapHeight    (kPalmSizeImageHeight)
/*@}*/

#define kInvalidRecIndex ((UInt16)0xFFFF ) /**< define for invalid record index */

#define kImageFilenameLength    (dmDBNameLength) /**< max filename length */


/// @name Sort Orders
/*@{*/
#define kNoSortOrder (0)
#define kSortByName  (1)
#define kSortByDate  (2)
#define kSortByNameDesc  (3)
#define kSortByDateDesc  (4)
/*@}*/

/// @name Image Operation Codes
/*@{*/
#define kResizeOperation (0)
#define kJPEGOperation   (1)
#define kVFSOperation    (2)
/*@}*/

/** Handle returned by ImageLibIterateImages or ImageLibIterateVFSImages. */
typedef void * ImageIteratorHandle;

/** Passed in when calling ImageLibSetProgressCallback */
typedef void (*ImageLibProgressCallbackType) (Int32 progress, Int32 max, Int8 operation);


// Should this be in HsErrorClases.h? HS should define the error base
#define imageLibErrorClass  (appErrorClass | 0x1000) /**< Imaging library base error number */

/// @name Error Codes
/*@{*/
#define imageLibErrBadParam             (imageLibErrorClass)
#define imageLibErrNoCamera             (imageLibErrorClass + 1)
#define imageLibErrInternalFailure      (imageLibErrorClass + 2)
#define imageLibErrWriteFailure         (imageLibErrorClass + 3)
#define imageLibErrReadFailure          (imageLibErrorClass + 4)
#define imageLibErrBadData              (imageLibErrorClass + 5)
#define imageLibErrBadImageType         (imageLibErrorClass + 6)
#define imageLibErrBadImageDepth        (imageLibErrorClass + 7)
#define imageLibErrBadDimensions        (imageLibErrorClass + 8)
#define imageLibErrInitializationFailed (imageLibErrorClass + 9)
#define imageLibErrBadColorspace        (imageLibErrorClass + 10)
#define imageLibErrBadThumbnailType     (imageLibErrorClass + 11)
#define imageLibErrThumnailExists       (imageLibErrorClass + 12)
#define imageLibErrBadScaleFactor       (imageLibErrorClass + 13)
#define imageLibErrBadBitmapType        (imageLibErrorClass + 14)
#define imageLibErrImageAlreadyDownsized (imageLibErrorClass + 15)
#define imageLibErrImageLocked           (imageLibErrorClass + 16)
/*@}*/

#endif
