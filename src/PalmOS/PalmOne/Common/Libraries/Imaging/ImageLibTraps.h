/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Imaging
 */

/**
 * @file 	ImageLibTraps.h
 * @version 1.0
 * @brief	Function trap numbers for the Imaging Library API.
 */

#ifndef IMAGELIBTRAPS_H
#define IMAGELIBTRAPS_H


#include <PalmTypes.h>

/**
 * @name Function Trap Numbers
 */
/*@{*/
#define kImageLibTrapOpen                          (sysLibTrapOpen      )
#define kImageLibTrapClose                         (sysLibTrapClose     )

#define kImageLibTrapGetVersion                    (sysLibTrapCustom    )
#define kImageLibTrapCameraExists                  (sysLibTrapCustom + 1)
#define kImageLibTrapImageCapture                  (sysLibTrapCustom + 2)
#define kImageLibTrapImageSave                     (sysLibTrapCustom + 3)
#define kImageLibTrapPreviewBitmapCreate           (sysLibTrapCustom + 4)
#define kImageLibTrapPreviewBitmapFill             (sysLibTrapCustom + 5)
#define kImageLibTrapPreviewDraw                   (sysLibTrapCustom + 6)
#define kImageLibTrapImageBitmapCreate             (sysLibTrapCustom + 7)
#define kImageLibTrapImageBitmapFill               (sysLibTrapCustom + 8)
#define kImageLibTrapImageDraw                     (sysLibTrapCustom + 9)
#define kImageLibTrapThumbnailBitmapCreate         (sysLibTrapCustom + 10)
#define kImageLibTrapThumbnailBitmapFill           (sysLibTrapCustom + 11)
#define kImageLibTrapThumbnailDraw                 (sysLibTrapCustom + 12)
#define kImageLibTrapBitmapDelete                  (sysLibTrapCustom + 13)
#define kImageLibTrapDeleteImage                   (sysLibTrapCustom + 14)
#define kImageLibTrapResizeImage                   (sysLibTrapCustom + 15)
#define kImageLibTrapGetImageDatabases             (sysLibTrapCustom + 16)
#define kImageLibTrapGetImageInfo                  (sysLibTrapCustom + 17)
#define kImageLibTrapSetImageInfo                  (sysLibTrapCustom + 18)

#define kImageLibTrapCopyImageToVFS                (sysLibTrapCustom + 19)
#define kImageLibTrapCopyImageFromVFS              (sysLibTrapCustom + 20)
#define kImageLibTrapImageBitmapCreateFromVFS      (sysLibTrapCustom + 21)
#define kImageLibTrapImageBitmapFillFromVFS        (sysLibTrapCustom + 22)
#define kImageLibTrapImageDrawFromVFS              (sysLibTrapCustom + 23)
#define kImageLibTrapThumbnailBitmapCreateFromVFS  (sysLibTrapCustom + 24)
#define kImageLibTrapThumbnailBitmapFillFromVFS    (sysLibTrapCustom + 25)
#define kImageLibTrapThumbnailDrawFromVFS          (sysLibTrapCustom + 26)


#define kImageLibTrapDoInfoDialog                  (sysLibTrapCustom + 27)
#define kImageLibTrapDoAttachImageDialog           (sysLibTrapCustom + 28)
#define kImageLibTrapSetNoteFont                   (sysLibTrapCustom + 29)
#define kImageLibTrapGetNoteFont                   (sysLibTrapCustom + 30)
#define kImageLibTrapDoNoteDialog                  (sysLibTrapCustom + 31)

#define kImageLibTrapCopyImageToBuffer             (sysLibTrapCustom + 32)
#define kImageLibTrapCopyImageToFile               (sysLibTrapCustom + 33)
#define kImageLibTrapInsertImageFromBuffer         (sysLibTrapCustom + 34)
#define kImageLibTrapInsertImageFromFile           (sysLibTrapCustom + 35)
#define kImageLibTrapLockImage                     (sysLibTrapCustom + 36)
#define kImageLibTrapIterateImages                 (sysLibTrapCustom + 37)
#define kImageLibTrapIterateVFSImages              (sysLibTrapCustom + 38)
#define kImageLibTrapIterSort                      (sysLibTrapCustom + 39)
#define kImageLibTrapIterGetImageInfo              (sysLibTrapCustom + 40)
#define kImagelibTrapIterGetCount                  (sysLibTrapCustom + 41)
#define kImageLibTrapIterDeleteImage               (sysLibTrapCustom + 42)
#define kImageLibTrapIterRemoveImage               (sysLibTrapCustom + 43)
#define kImageLibTrapIterFree                      (sysLibTrapCustom + 44)
#define kImageLibTrapIterDrawImage                 (sysLibTrapCustom + 45)
#define kImageLibTrapIterDrawThumbnail             (sysLibTrapCustom + 46)
#define kImageLibTrapIterSearch                    (sysLibTrapCustom + 47)
#define kImageLibTrapSetProgressCallback           (sysLibTrapCustom + 48)
#define kImageLibTrapCopyImage                     (sysLibTrapCustom + 49)
#define kImageLibTrapFindImageByID                 (sysLibTrapCustom + 50)
#define kImageLibTrapPreviewStart                  (sysLibTrapCustom + 51)
#define kImageLibTrapPreviewStop                   (sysLibTrapCustom + 52)
#define kImageLibTrapDrawJPEG                      (sysLibTrapCustom + 53)
#define kImageLibTrapTurnCameraOff                 (sysLibTrapCustom + 54)
#define kImageLibTrapSetImageID                    (sysLibTrapCustom + 55)
#define kImageLibTrapGetImageID                    (sysLibTrapCustom + 56)

#define kImageLibTrapGetDimensions                 (sysLibTrapCustom + 57)
#define kImageLibTrapGetDimensionsVFS              (sysLibTrapCustom + 58)
#define kImageLibTrapIterGetDimensions             (sysLibTrapCustom + 59)
#define kImageLibTrapGetDimensionsJPEG             (sysLibTrapCustom + 60)

#define kImageLibTrapResize                        (sysLibTrapCustom + 61)
#define kImageLibTrapResizeVFS                     (sysLibTrapCustom + 62)
#define kImageLibTrapIterResize                    (sysLibTrapCustom + 63)
#define kImageLibTrapResizeJPEG                    (sysLibTrapCustom + 64)

#define kImageLibTrapIterRefresh                   (sysLibTrapCustom + 65)
#define kImageLibTrapSetJPEGQuality                (sysLibTrapCustom + 66)
#define kImageLibTrapGetJPEGQuality                (sysLibTrapCustom + 67)
#define kImageLibTrapGetAveragePictureSize         (sysLibTrapCustom + 68)
#define kImageLibTrapCheckLockedImages             (sysLibTrapCustom + 69)
/*@}*/

#endif
