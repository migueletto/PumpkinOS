/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup Imaging Imaging Library
 *
 * @{
 * @}
 */

/**
 * @file  ImageLib.h
 * @brief Imaging Library.
 *
 * This file contains routines for capturing, storing, iterating, resizing, and
 * manipulating images. The imaging library is a client of both the camera
 * driver and the JPEG library.
 */

/*
 * Copyright (c) 2003 IDEO. All rights reserved.
 * Portions ©2004 palmOne, Inc. All rights reserved.
 *
 * author: Harold Howe
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/68k/libraries/imaging/ImageLib.h#4 $
 *
 ****************************************************************/

#ifndef IMAGELIB_H
#define IMAGELIB_H

#include <PalmOS.h>
#include <PalmTypes.h>
#include <VFSMgr.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/* ImageLibOpen */

/**
 * Library open call for 68k clients of the library. This routine maps to the
 * sysLibTrapOpen trap.
 *
 * It is customary for shared libraries to provide open and close functions.
 * The open and close functions in ImageLib are empty stubs that exist for the
 * sole purpose of adhering to past convention. This function does nothing and
 * will always return errNone. All library initialization is done from
 * PilotMain.
 *
 * This function is not available to ARM clients of the library.
 *
 * @param libRef: IN: Library reference number.
 *
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibOpen(UInt16 libRef) SYS_TRAP(kImageLibTrapOpen );


/****************************************************************/
/* ImageLibClose */

/**
 * Library close function for 68k clients of the library. This routine maps to
 * the sysLibTrapClose trap.
 *
 * This function is an empty stub (see comments for ImageLibOpen). All library
 * cleanup is done from PilotMain.
 *
 * This function is not available to ARM clients of the library.
 *
 * @see ImageLibOpen
 *
 * @param libRef:	IN: Library reference number.
 *
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibClose(UInt16 libRef) SYS_TRAP(kImageLibTrapClose);


/****************************************************************/
/* ImageLibGetVersion */

/**
 * Retrieves the version number of the image library.
 *
 * @param libRef: IN: Library reference number.
 * @param dwVerP: OUT: Pointer to library version variable.
 *
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibGetVersion(UInt16 libRef, UInt32 *dwVerP)
    SYS_TRAP(kImageLibTrapGetVersion);


/****************************************************************/
/* ImageLibCameraExists */

/**
 * Detects presence of camera driver.
 *
 * The imaging library loads the camera driver dynamically. If the driver was
 * found and was loaded successfully, ImageLibCameraExists writes true to
 * existsP, otherwise it writes false.
 *
 * This routine will attempt to load the camera driver if it hasn't
 * already been loaded. The driver will remain loaded until the imaging library
 * is unloaded, or until ImageLibTurnCameraOff is called.
 *
 * @see ImageLibTurnCameraOff
 *
 * @param libRef:	IN: Library reference number.
 * @param existsP:	OUT: true if camera successfully loaded. Otherwise false.
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibCameraExists(UInt16 libRef, Boolean* existsP)
    SYS_TRAP(kImageLibTrapCameraExists);

/****************************************************************/
/* ImageLibImageCapture */

/**
 * Captures an image and stores it in the database.
 *
 * Images are appended to the end of the database.
 *
 * The camera driver always captures 640x480 images. If fullSize is false,
 * the data from the camera is downsampled to 160x120 using a filtering
 * algorithm.
 *
 * Captures are currently done in 16 bit RGB mode. Capturing an image
 * requires at least 600 kB of free space in the dynamic heap (640x480x2).
 *
 * Captured pictures are stored in uncompressed palmOS bitmap format.
 * To compress an image as a JPEG, call ImageLibImageSave.
 *
 * @see ImageLibImageSave
 *
 * @param libRef:			IN: Library reference number.
 * @param fullsize:			IN: true capture a 640x480 image. false to capture 160x120
 * @param playShutterSound:	IN: true to play a camera shutter sound.
 * @param drawImmediately:	IN: true to immediately draw the captured image to the current draw window.
 * @param x:				IN: x location to draw the captured image. Relative to the current draw window. Ignored if drawImmediately is false.
 * @param y:				IN: y location to draw the captured image. Relative to the current draw window. Ignored if drawImmediately is false.
 * @param newRecNoP:		OUT: index of the new image. If the capture fails, kInvalidRecIndex is written to this value. Pass NULL if you don’t care what the new index is.
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibImageCapture(UInt16 libRef,
                         Boolean fullsize,
                         Boolean playShutterSound,
                         Boolean drawImmediately,
                         Coord x,
                         Coord y,
                         UInt16 *newRecNoP)
    SYS_TRAP(kImageLibTrapImageCapture);


/****************************************************************/
/* ImageLibImageSave */

/**
 * Compresses a captured image to JPEG format.
 *
 * Compresses a captured image to JPEG, sets the image name, and sets the
 * category of the picture. After saving, the uncompressed bitmap data is
 * discarded and replaced with the JPEG version of the image.
 *
 * This routine should only be invoke on newly captured images, and it
 * should only be called once per picture.
 *
 * @see ImageLibImageCapture
 *
 * @param libRef:	IN: Library reference number.
 * @param recIndex:	IN: Index of the image to compress.
 * @param name:		IN: Name of the picture
 * @param category:	IN: Category to store the image in
 * @retval Err 0 on success or error code on failure.
 *
 ****************************************************************/
Err ImageLibImageSave(UInt16 libRef,UInt16 recIndex,
                      const char * name, UInt16 category)
    SYS_TRAP(kImageLibTrapImageSave);


/****************************************************************/
/* ImageLibPreviewBitmapCreate */

/**
 * Creates a preview bitmap. If Compresses a captured image to JPEG, sets the image name, and sets the
 * category of the picture. After saving, the uncompressed bitmap data is
 * discarded and replaced with the JPEG version of the image.
 *
 * When called from a 68k application, the return value should be treated
 * as opaque. The bitmap can safely be passed to the Palm OS bitmap routines,
 * but 68k applications should not attempt to dereference members of the bitmap
 * structure directly.
 *
 * Bitmaps created with this function should be disposed of by calling
 * ImageLibBitmapDelete. Do note call BmpDelete or MemPtrFree.
 *
 * If an error occurs, this function returns NULL and writes an error code
 * to errP. If the function returns a non-NULL bitmap value, then *errP will
 * always be 0.
 *
 * This function does not perform an image preview. The resulting bitmap
 * is empty. To fill the bitmap with a preview image, pass it to
 * ImageLibPreviewBitmapFill.
 *
 * @see ImageLibPreviewBitmapFill
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:	IN: Library reference number.
 * @param   errP:	OUT: 0 on success, non-zero on error. This parameter should not be null.
 * @retval  BitmapType On success, the result is a new 160x120 bitmap whose bit depth matches
 *                     the bit depth of the screen. On failure, the result is NULL
 *
 ****************************************************************/
BitmapType* ImageLibPreviewBitmapCreate(UInt16 libRef, Err *errP)
    SYS_TRAP(kImageLibTrapPreviewBitmapCreate);

/****************************************************************/
/* ImageLibPreviewBitmapFill */

/**
 * Fills a bitmap with a preview image from the camera.
 *
 * The imageP argument should represent a bitmap created with
 * ImageLibPreviewBitmapCreate.
 *
 * @see ImageLibPreviewBitmapCreate
 * @see ImageLibBitmapDelete
 * @see ImageLibPreviewStop
 * @see ImageLibPreviewStart
 *
 * @param   libRef:	IN: Library reference number.
 * @param   imageP:	IN,OUT: Bitmap to fill with a preview.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibPreviewBitmapFill(UInt16 libRef, BitmapType* imageP)
    SYS_TRAP(kImageLibTrapPreviewBitmapFill);


/****************************************************************/
/* ImageLibPreviewDraw */

/**
 * Draws a preview image to the current draw window.
 *
 * Snaps a preview image with the camera driver and then draws it to the
 * current draw window at x,y. x and y are relative coordinates.
 *
 * @see ImageLibPreviewStop
 * @see ImageLibPreviewStart
 *
 * @param   libRef:	IN: Library reference number.
 * @param   x:		IN: x location to draw the preview image, relative to the current draw window.
 * @param   y:		IN: y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibPreviewDraw(UInt16 libRef, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapPreviewDraw);



/****************************************************************/
/* ImageLibImageBitmapCreate */

/**
 * Creates a 16 bit bitmap for holding an image from the database. If
 * recIndex is a valid image index, that image is loaded into the resulting
 * bitmap. Otherwise, the result is an empty picture.
 *
 * When called from a 68k application, the return value should be treated
 * as opaque. The bitmap can safely be passed to the Palm OS bitmap routines,
 * but 68k applications should not attempt to dereference members of the bitmap
 * structure directly.
 *
 * Bitmaps created with this function should be disposed of by calling
 * ImageLibBitmapDelete. Do note call BmpDelete or MemPtrFree.
 *
 * If an error occurs, this function returns NULL and writes an error code
 * to errP. If the function returns a non-NULL bitmap value, then *errP will
 * always be 0.
 *
 * If recIndex is kInvalidRecIndex, the result is an empty 160x120 bitmap.
 * If recIndex is not kInvalidRecIndex, the result is a bitmap that contains the
 * given image from the database. The returned bitmap will be 160x120 if the
 * image in the database is 160x120, 320x240, 640x480, or any other image larger
 * than 160x120 with a 4:3 aspect ratio. If the image has a non-standard size,
 * it is downsized to fit in a 160x120 box. The aspect ratio of the picture is
 * preserved. Images that are less than 160x120 are not scaled at all.
 *
 * @see ImageLibBitmapDelete
 * @see ImageLibPreviewStop
 * @see ImageLibPreviewStart
 *
 * @param   libRef:		IN: Library reference number.
 * @param   recIndex:	IN: Index of image to fill bitmap with. Use kInvalidRecIndex to create an empty 160x120, 16 bit bitmap.
 * @param   errP:		OUT: If the function succeeds, 0 is written to errP. Otherwise, a non-zero error code is written. This parameter should not be NULL.
 * @retval  BitmapType* On success, the result is a new bitmap with a bit depth of 16. On error, the result is NULL.
 *
 ****************************************************************/
BitmapType* ImageLibImageBitmapCreate(UInt16 libRef,
                                      UInt16 recIndex,
                                      Err *errP)
    SYS_TRAP(kImageLibTrapImageBitmapCreate);


/****************************************************************/
/* ImageLibImageBitmapFill */

/**
 * Fills a bitmap with an image from the database.
 *
 * The imageP argument should represent a bitmap created with
 * ImageLibImageBitmapCreate.
 *
 * The image from the database is resampled as necessary to fit the
 * dimensions of imageP. When the image is a jpeg, the DCT scaling features of
 * the IJG library are used to get the image size close to the destination size.
 * From there, the image is resampled with a filter to match the dimensions of
 * imageP. If the aspect ratio of imageP does not match the aspect ratio of
 * the picture, the picture is stretched to fit.
 *
 * @see ImageLibImageBitmapCreate
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  index of image to fill bitmap with.
 * @param   imageP:		IN,OUT:  Bitmap to fill.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibImageBitmapFill(UInt16 libRef, UInt16 recIndex, BitmapType* imageP)
    SYS_TRAP(kImageLibTrapImageBitmapFill);

/****************************************************************/
/* ImageLibImageDraw*/

/**
 * Draws an image from the database to the current draw window.
 *
 * Draws an image from the database to the current draw window at x,y. x and y
 * are window relative coordinates.
 *
 * If necessary, the image is downsized to fit in a 160x120 box. The
 * aspect ratio of the picture is preserved.
 *
 * @param   libRef:		IN: Library reference number.
 * @param   recIndex:	IN: index of image to draw.
 * @param   x:			IN: x location to draw the preview image, relative to the current draw window.
 * @param   y:			IN: y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibImageDraw(UInt16 libRef, UInt16 recIndex, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapImageDraw);


/****************************************************************/
/* ImageLibThumbnailBitmapCreate */

/**
 * Creates a 16 bit bitmap for holding an thumbnail image . If
 * recIndex is a valid image index, the thumbnail for that image is loaded into
 * the resulting bitmap. Otherwise, the result is an empty bitmap.
 *
 * When called from a 68k application, the return value should be treated
 * as opaque. The bitmap can safely be passed to the Palm OS bitmap routines,
 * but 68k applications should not attempt to dereference members of the bitmap
 * structure directly.
 *
 * Bitmaps created with this function should be disposed of by calling
 * ImageLibBitmapDelete. Do note call BmpDelete or MemPtrFree.
 *
 * If an error occurs, this function returns NULL and writes an error code
 * to errP. If the function returns a non-NULL bitmap value, then *errP will
 * always be 0.
 *
 * The thumbnail is generated on the fly if the image in the database
 * does not have a thumbnail record.
 *
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  index of image to fill the thumbnail bitmap with. Use kInvalidRecIndex to create an empty 40x30, 16 bit bitmap.
 * @param   errP:		OUT: If the function succeeds, 0 is written to errP. Otherwise, a non-zero error code is written. This parameter should not be NULL.
 * @retval  BitmapType* On success, the result is a new bitmap with a bit depth of 16. On error, the result is NULL.
 *
 ****************************************************************/
BitmapType* ImageLibThumbnailBitmapCreate(UInt16 libRef,
                                          UInt16 recIndex,
                                          Err *errP)
    SYS_TRAP(kImageLibTrapThumbnailBitmapCreate);


/****************************************************************/
/* ImageLibThumbnailBitmapFill*/

/**
 * Fills a bitmap with an thumbnail image from the database.
 *
 * The imageP argument should represent a bitmap created with
 * ImageLibThumbnailBitmapCreate.
 *
 * The image from the database is resampled as necessary to fit the
 * dimensions of imageP. If the aspect ratio of imageP does not match the aspect
 * ratio of the picture, the picture is stretched to fit.
 *
 * @see ImageLibThumbnailBitmapCreate
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN: index of image
 * @param   imageP:		IN,OUT:  Bitmap to fill.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibThumbnailBitmapFill(UInt16 libRef, UInt16 recIndex, BitmapType* imageP)  SYS_TRAP(kImageLibTrapThumbnailBitmapFill);


/****************************************************************/
/* ImageLibThumbnailDraw*/

/**
 * Draws an thumbnail image from the database to the current draw window at x,y.
 * x and y * are window relative coordinates.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  index of image
 * @param   x:			IN:  x location to draw the preview image, relative to the current draw window.
 * @param   y:			IN:  y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibThumbnailDraw(UInt16 libRef, UInt16 recIndex, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapThumbnailDraw);


/****************************************************************/
/* ImageLibBitmapDelete*/

/**
 * Deletes a bitmap created by the library.
 *
 * Use ImageLibBitmapDelete to delete any bitmap that was created and returned
 * by the imaging library (ImageLibPreviewBitmapCreate, ImageLibImageBitmapCreate and ImageLibThumbnailBitmapCreate).
 *
 * @param   libRef:	IN: Library reference number.
 * @param   bmp:	IN: Bitmap to delete.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibBitmapDelete(UInt16 libRef, BitmapType *bmp)
    SYS_TRAP(kImageLibTrapBitmapDelete);

/****************************************************************/
/* ImageLibDeleteImage*/

/**
 * Delete a picture from the database.
 *
 * This routine deletes an image and its thumbnail from the database.
 *
 * Deleting a picture invalidates the record indices of all subsequent
 * images in the database. Records are deleted with DmRemoveRecord. They are
 * not archived.
 *
 * @param   libRef:		IN: Library reference number.
 * @param   recIndex:	IN: index of image to delete.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibDeleteImage(UInt16 libRef, UInt16 recIndex)
    SYS_TRAP(kImageLibTrapDeleteImage);

/****************************************************************/
/* ImageLibResizeImage*/

/**
 * @deprecated
 *
 * Resizes a fullsize picture in the database to 160x120 (palm size). The original
 * fullsize image is replaced with the palm size version.
 *
 * If the original image is a full size 640x480 jpeg, the image is
 * downsampled using the DCT scaling features of the IJG library. This means
 * that the JPEG is decompressed directly to 160x120, and then compressed again
 * and saved back to the database.
 *
 * If the original image is not a 640x480 jpeg, it is scaled as much as
 * possible using the IJG scaling features. After decompression, the picture is
 * resampled with a filter to get its dimensions to 160x120.
 *
 * ImageLibResizeImage is deprecated. Use ImageLibResize instead.
 *
 * @param   libRef:		IN: Library reference number.
 * @param   recIndex:	IN: index of image to resize.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibResizeImage(UInt16 libRef, UInt16 recIndex)
    SYS_TRAP(kImageLibTrapResizeImage);


/****************************************************************/
/* ImageLibGetImageDatabases*/

/**
 * Return the references to the database that are used to store pictures.
 *
 * The image library opens each database when it is loaded, and closes them when it is unloaded.
 * Do not close the database references that are returned by this function because the references are still being used by
 * the imaging library.
 *
 * The data in each database is stored in little endian format.
 *
 * The main database contains one record for each image in the database.
 * Each record has a fixed size and contains information about the picture. The
 * image database contains the actual JPEG data for each image. The main
 * database acts as a master table and the images database is a child table.
 *
 * The notes database is not currently used.
 *
 * The category of an image is determined by the category of its record in
 * the main database. Categories are not used in the image table.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   dbMainP:	OUT: Reference to the main database. Pass NULL for this parameter if you don’t want to retrieve it.
 * @param   dbNoteP:	OUT: Reference to the note database. Pass NULL for this parameter if you don’t want to retrieve it.
 * @param   dbImageP:	OUT: Reference to the images database.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetImageDatabases(UInt16 libRef, DmOpenRef *dbMainP,
                              DmOpenRef *dbNoteP, DmOpenRef *dbImageP)
    SYS_TRAP(kImageLibTrapGetImageDatabases);


/****************************************************************/
/* ImageLibGetImageInfo*/

/**
 * Retrieves information about an image stored in the database.
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   recIndex:		IN:  Index of the picture
 * @param   name:			OUT: Buffer to receive the name of the image. The buffer should be at least 32
                                 characters long (31 + null terminator). Pass NULL for this parameter if you don't want to retrieve it.
 * @param   fullSizeP:		OUT: true if image is 640x480, false if palm size. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   createdP:		OUT: Timestamp when picture was creatd. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   sizeP:			OUT: Size of the image data in bytes. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   categoryP:		OUT: Category that that picture is assigned to. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   categoryName:	OUT: Text description of the picture's category (ie 'Unfiled'). Pass NULL for this parameter if you don't want to retrieve it.
 * @param   lockedP:		OUT: true if image is locked.
 * @param   uidP:			OUT: Unique ID of the image.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetImageInfo(UInt16 libRef, UInt16 recIndex, Char* name,
                         Boolean* fullSizeP, UInt32* createdP, UInt32* sizeP,
                         UInt16* categoryP, Char* categoryName, Boolean *lockedP, UInt32 *uidP) SYS_TRAP(kImageLibTrapGetImageInfo);

/****************************************************************/
/* ImageLibSetImageInfo*/

/**
 * Assigns information about an image stored in the database.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  Index of the picture
 * @param   name:		IN:  Name of the picture. Pass NULL for this parameter if you don't want to set it.
 * @param   categoryP:	OUT: Category that that picture is assigned to. Pass NULL for this parameter if you don't want to set it.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibSetImageInfo(UInt16 libRef, UInt16 recIndex,
                         const char* name, UInt16* categoryP)
    SYS_TRAP(kImageLibTrapSetImageInfo);


/****************************************************************/
/* ImageLibCopyImageToVFS*/

/**
 * Copies an image from the database to a VFS volume.
 *
 * Copies an image from the database to a VFS volume, overwriting any previous
 * file that existed on the volume with the same name.
 *
 * volRefNum should be obtained via a call to VFSVolumeEnumerate
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  Index of the picture
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture.jpg")
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibCopyImageToVFS(UInt16 libRef, UInt16 recIndex,
                           UInt16 volRefNum, const char* fileName)
    SYS_TRAP(kImageLibTrapCopyImageToVFS);


/****************************************************************/
/* ImageLibCopyImageToVFS*/

/**
 * Copies an image from a VFS volume to the database.
 *
 * volRefNum should be obtained via a call to VFSVolumeEnumerate
 *
 * If a picture with a matching name and category already exists in the
 * database, that image is <b>not</b> overwritten. After the call, there will
 * be two images with the same name and category.
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   volRefNum:		IN:  Ref number of the VFS volume
 * @param   fileName:		IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   name:			IN:  Name of the image in the database (ie "picture001")
 * @param   category:		IN:  Category that the image should be assigned to
 * @param   newRecIndexP:	OUT: Record index of the newly stored picture. Pass NULL if you don't care what the new index is.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibCopyImageFromVFS(UInt16 libRef, UInt16 volRefNum,
                             const char* fileName, const char * name,
                             UInt16 category, UInt16 *newRecIndexP)
    SYS_TRAP(kImageLibTrapCopyImageFromVFS);


/****************************************************************/
/* ImageLibImageBitmapCreateFromVFS */

/**
 * Creates a 16 bit bitmap for holding an image from a VFS volume. The library
 * decompresses the image from the volume and stores it in the result.
 *
 * When called from a 68k application, the return value should be treated
 * as opaque. The bitmap can safely be passed to the Palm OS bitmap routines,
 * but 68k applications should not attempt to dereference members of the bitmap
 * structure directly.
 *
 * Bitmaps created with this function should be disposed of by calling
 * ImageLibBitmapDelete. Do note call BmpDelete or MemPtrFree.
 *
 * If an error occurs, this function returns NULL and writes an error code
 * to errP. If the function returns a non-NULL bitmap value, then *errP will
 * always be 0.
 *
 * The picture is always resized to fit in a 160x120 box. If the image needs
 * to be resized, its aspect ratio is maintained.
 *
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture.jpg")
 * @param   errP:		OUT: If the function succeeds, 0 is written to errP. Otherwise, a non-zero error code is written. This parameter should not be NULL.
 * @retval  BitmapType* On success, the result is a new bitmap with a bit depth of 16. On error, the result is NULL.
 *
 ****************************************************************/
BitmapType* ImageLibImageBitmapCreateFromVFS(UInt16 libRef, UInt16 volRefNum,
                                             const char* fileName, Err *errP)
    SYS_TRAP(kImageLibTrapImageBitmapCreateFromVFS);


/****************************************************************/
/* ImageLibImageBitmapFillFromVFS */

/**
 *
 * Fills a 16 bit bitmap with an image from a VFS volume. The library
 * decompresses the image from the volume and stores it in the supplied bitmap.
 * The image is resized to fit the dimensions of the supplied bitmap.
 *
 * The imageP argument should represent a bitmap created with
 * ImageLibImageBitmapCreate or ImageLibImageBitmapCreateFromVFS.
 *
 * The image is resampled as necessary to fit the
 * dimensions of imageP. When the image is a jpeg, the DCT scaling features of
 * the IJG library are used to get the image size close to the destination size.
 * From there, the image is resampled with a filter to match the dimensions of
 * imageP. If the aspect ratio of imageP does not match the aspect ratio of
 * the picture, the picture is stretched to fit.
 *
 * @see ImageLibImageBitmapCreate
 * @see ImageLibImageBitmapCreateFromVFS
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:   IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   imageP:		IN,OUT:  Bitmap to fill.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibImageBitmapFillFromVFS(UInt16 libRef, UInt16 volRefNum,
                                   const char* fileName, BitmapType* imageP)
    SYS_TRAP(kImageLibTrapImageBitmapFillFromVFS);


/****************************************************************/
/* ImageLibImageDrawFromVFS*/

/**
 * Draws an image from from a VFS volume to the current draw window at x,y.
 * x and y are window relative coordinates.
 *
 * If necessary, the image is downsized to fit in a 160x120 box. The
 * aspect ratio of the picture is preserved.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   x:			IN:  x location to draw the image, relative to the current draw window.
 * @param   y:			IN:  y location to draw the image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibImageDrawFromVFS(UInt16 libRef, UInt16 volRefNum,
                             const char* fileName, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapImageDrawFromVFS);


/****************************************************************/
/* ImageLibThumbnailBitmapCreateFromVFS */

/**
 * Creates a 16 bit bitmap for holding an thumbnail image for a VFS JPEG.
 *
 * When called from a 68k application, the return value should be treated
 * as opaque. The bitmap can safely be passed to the Palm OS bitmap routines,
 * but 68k applications should not attempt to dereference members of the bitmap
 * structure directly.
 *
 * Bitmaps created with this function should be disposed of by calling
 * ImageLibBitmapDelete. Do note call BmpDelete or MemPtrFree.
 *
 * If an error occurs, this function returns NULL and writes an error code
 * to errP. If the function returns a non-NULL bitmap value, then *errP will
 * always be 0.
 *
 * The thumbnail is generated on the fly from VFS file. For large images,
 * this operation can be slow.
 *
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   errP:		OUT: If the function succeeds, 0 is written to errP. Otherwise, a non-zero error code is written. This parameter should not be NULL.
 * @retval  BitmapType* On success, the result is a new bitmap with a bit depth of 16. On error, the result is NULL.
 *
 ****************************************************************/
BitmapType* ImageLibThumbnailBitmapCreateFromVFS (UInt16 libRef,
                                                  UInt16 volRefNum,
                                                  const char* fileName,
                                                  Err *errP)
    SYS_TRAP(kImageLibTrapThumbnailBitmapCreateFromVFS);


/****************************************************************/
/* ImageLibImageBitmapFillFromVFS */

/**
 * Fills a bitmap with a thumbnail image from a VFS volume. *
 *
 * The imageP argument should represent a bitmap created with
 * ImageLibThumbnailBitmapCreate or ImageLibThumbnailBitmapCreateFromVFS.
 *
 * The image is resampled as necessary to fit the
 * dimensions of imageP.
 *
 * @see ImageLibThumbnailBitmapCreate
 * @see ImageLibThumbnailBitmapCreateFromVFS
 * @see ImageLibBitmapDelete
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   imageP:		IN,OUT:  Bitmap to fill.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibThumbnailBitmapFillFromVFS(UInt16 libRef, UInt16 volRefNum,
                                       const char* fileName, BitmapType* imageP)
    SYS_TRAP(kImageLibTrapThumbnailBitmapFillFromVFS);

/****************************************************************/
/* ImageLibThumbnailDrawFromVFS*/

/**
 * Draws an thumbnail image from a VFS image to the current draw window at x,y.
 * x and y * are window relative coordinates.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   volRefNum:	IN:  Ref number of the VFS volume
 * @param   fileName:	IN:  Filename of the image (ie "/DCIM/picture001.jpg")
 * @param   x:			IN:  x location to draw the preview image, relative to the current draw window.
 * @param   y:			IN:  y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibThumbnailDrawFromVFS(UInt16 libRef, UInt16 volRefNum, const char* fileName, Coord x, Coord y)   SYS_TRAP(kImageLibTrapThumbnailDrawFromVFS);


/****************************************************************/
/* ImageLibDoAttachImageDialog*/

/**
 * Displays the attach image dialog.
 *
 * Displays the attach image dialog. Allows to user to select a picture from a
 * list. After choosing an image, the selection is written to the output
 * parameters recIndexP, volRefNum, and filename.
 *
 * If the user cancels the dialog, the return value is still 0. The values
 * in recIndexP and volRefNumP indicate that the dialog was cancelled.
 *
 * @param   libRef:				IN:  Library reference number.
 * @param   recIndexP:			OUT: If the user selects an image from the database, recIndexP will contain the index of the image. If the user selects an image from a VFS volume, or they cancel the dialog, or an error occurs, recIndexP will contain kInvalidRecIndex.
 * @param   volRefNumP:			OUT: If the user selects an image from the VFS system, volRefNumP will contain the volume ref number of the selected image. In all other cases, volRefNumP will contain vfsInvalidVolRef.
 * @param   fileName:			OUT: If the user selects an image from the VFS system, the full path of the file will be copied into fileNameP. fileName should be capable of holding the largest possible VFS filename. If the user does not select a VFS file, fileName[0] will be \0.
 * @param   initialCategory:	IN:  The initial category that should be highlighted in the category dropdown. Pass 0 for unfiled. Pass dmAllCategories to show all pictures from the database.
 * @param   showConfirmation:	IN:  If true, a confirmation dialog will be displayed that shows the image in full size. If false, the confirmation dialog is skipped.
 * @param   internalImagesOnly:	IN:  If true, images from VFS volumes are not displayed to the user.
 * @param   dialogTitle:		IN:  The string to display as the title of the dialog. Pass null to display a localized default.
 * @param   btnTitle:			IN:  The string to display as the title of the OK button. Pass null to display a localized default.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibDoAttachImageDialog(UInt16 libRef,
                                UInt16 *recIndexP,
                                UInt16 *volRefNumP,
                                char* fileName,
                                UInt16 initialCategory,
                                Boolean showConfirmation,
                                Boolean internalImagesOnly,
                                const char *dialogTitle,
                                const char * btnTitle)
    SYS_TRAP(kImageLibTrapDoAttachImageDialog);


/****************************************************************/
/* ImageLibCopyImageToBuffer*/

/**
 * Copies a JPEG from the image database to a chunk of memory at bufP. The
 * allocated memory block must be large enough to hold the entire blob of JPEG
 * data. The size of the JPEG can be determined with ImageLibGetImageInfo.
 *
 * Fullsize images may requires that bufP be larger than 64 kB. Use
 * FtrPtrNew or MemChunkNew to create buffers larger than 64 kB. Otherwise,
 * consider using ImageLibCopyImageToFile instead.
 *
 * @see ImageLibGetImageInfo
 * @see ImageLibCopyImageToFile
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  Index of the picture
 * @param   bufP:		IN,OUT: Pointer to the block of memory that should receive the image.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibCopyImageToBuffer (UInt16 libRef, UInt16 recIndex, MemPtr bufP)
    SYS_TRAP(kImageLibTrapCopyImageToBuffer     );


/****************************************************************/
/* ImageLibCopyImageToFile*/

/**
 * Copies a JPEG from the image database to a Palm OS file stream. The library
 * writes to the file stream at its current postion. When done, the library
 * leaves the file handle pointing to the end of the file. To see the jpeg data,
 * the file handle should be rewound to its original location.
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   recIndex:	IN:  Index of the picture
 * @param   fileH:		IN:  Handle to an open file stream.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibCopyImageToFile (UInt16 libRef, UInt16 recIndex, FileHand fileH)
    SYS_TRAP(kImageLibTrapCopyImageToFile);


/****************************************************************/
/* ImageLibInsertImageFromBuffer*/

/**
 * Inserts a JPEG blob into the image database, storing it with the supplied
 * name and category.
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   bufP:			IN:  Pointer to a JPEG image in memory.
 * @param   size:			IN:  Size of the source image in bytes.
 * @param   name:			IN:  Name of the new image (ie "Picture001")
 * @param   category:		IN:  The category to store the image in. Pass 0 for unfiled.
 * @param   newRecIndexP:	OUT: Index of the newly created image. Pass NULL if you don’t care about this value.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibInsertImageFromBuffer(UInt16 libRef, MemPtr bufP, UInt32 size,
                                  const char * name, UInt16 category,
                                  UInt16 *newRecIndexP)
    SYS_TRAP(kImageLibTrapInsertImageFromBuffer );


/****************************************************************/
/* ImageLibInsertImageFromBuffer*/

/**
 * Insert an image into the database from a Palm OS file stream. The file stream
 * is read from its current position to the end of the stream. When the routine
 * is finished, it leaves the stream open and performs a FileSeek back to the
 * original position.
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   fileH:			IN:  Handle to an open file stream that contains a JPEG starting at its current position.
 * @param   name:			IN:  Name of the new image (ie "Picture001")
 * @param   category:		IN:  The category to store the image in. Pass 0 for unfiled.
 * @param   newRecIndexP:	OUT: Index of the newly created image. Pass NULL if you don’t care about this value.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibInsertImageFromFile(UInt16 libRef, FileHand fileH,
                               const char * name, UInt16 category,
                               UInt16 *newRecIndexP)
    SYS_TRAP(kImageLibTrapInsertImageFromFile   );


/****************************************************************/
/* ImageLibIterateImages*/

/**
 * Creates an iterator for working with a collection of images in the database.
 * When finished, the iterator refers to a data structure that contains images
 * The iterator can be used to step through images, draw images, sort images, and search for
 * images by name.
 *
 * The returned iterator handle can be passed to other iterator routines
 * that take an iterator handle. The handle should be freed by calling
 * ImageLibIterFree.
 *
 * @see ImageLibIterFree
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   category:		IN:  The category to search. Pass 0 for unfiled. Pass dmAllCategories to iterate through all images in the databse.
 * @param   sortType:		IN:  Initial sort order of the datastructure (kNoSortOrder, kSortByName, or kSortByDate)
 * @param   imageCountP:	OUT: Number of images in the iterator data structure. Pass NULL if you don't care about this value.
 * @param   errP:			OUT: 0 on success, non-zero on error. This parameter should not be null.
 * @retval  ImageIteratorHandle Opaque iterator handle.
 *
 ****************************************************************/
ImageIteratorHandle ImageLibIterateImages(UInt16 libRef,
                                          UInt16 category,
                                          UInt16 sortType,
                                          UInt16 *imageCountP,
                                          Err *errP)
    SYS_TRAP(kImageLibTrapIterateImages);

/****************************************************************/
/* ImageLibIterateVFSImages*/

/**
 * Creates an iterator for working with a collection of images on a
 * VFS volume. When finished, the iterator refers to a data structure that
 * contains images. The iterator can be used to step through images, draw
 * images, sort images, and search for images by name.
 *
 * The returned iterator handle can be passed to other iterator routines
 * that take an iterator handle. The handle should be freed by calling
 * ImageLibIterFree.
 *
 * @see ImageLibIterFree
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   volRefNum:		IN:  VFS volume to search.
 * @param   path:			IN:  Directory to start searching in.
 * @param   recurseBelow:	IN:  true to recurse in all subdirectories of path. false to search in path only.
 * @param   sortType:		IN:  Initial sort order of the datastructure (kNoSortOrder, kSortByName, or kSortByDate)
 * @param   imageCount:		OUT: Number of images in the iterator data structure. Pass NULL if you don't care about this value.
 * @param   errP:			OUT: 0 on success, non-zero on error. This parameter should not be null.
 * @retval  ImageIteratorHandle Opaque iterator handle.
 *
 ****************************************************************/
ImageIteratorHandle ImageLibIterateVFSImages(UInt16 libRef,
                                             UInt16 volRefNum,
                                             const char *path,
                                             Boolean recurseBelow,
                                             UInt16 sortType,
                                             UInt16 *imageCount, Err *errP)
    SYS_TRAP(kImageLibTrapIterateVFSImages)  ;


/****************************************************************/
/* ImageLibIterSort*/

/**
 * Sorts the images in an iterator chain. Note that this only reorganizes images
 * in the iterator data structure. This does not alter the way images are stored
 * in the database or on a card.
 * images, sort images, and search for images by name.
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * @see ImageLibIterateImages
 * @see ImageLibIterateVFSImages
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   iter:		IN:  Handle to an iterator.
 * @param   sortType:	IN:  New sort order of the datastructure (kNoSortOrder, kSortByName, kSortByDate, kSortByNameDesc, or kSortByDateDesc)
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterSort (UInt16 libRef,ImageIteratorHandle iter, UInt16 sortType )
    SYS_TRAP(kImageLibTrapIterSort);

/****************************************************************/
/* ImageLibIterGetImageInfo*/
/**
 * Retrieves information about a single image in an iterator chain.
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * Pass NULL for any OUT parameter that you don't care about.
 *
 * @see ImageLibIterateImages
 * @see ImageLibIterateVFSImages
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   iter:		IN:  Handle to an iterator.
 * @param   index:		IN:  Index of the image in the iterator. [0,imageCount)
 * @param   name:		OUT: Buffer to receive the name of the image. For VFS images, this is the full path and filename of the image (ie "/dcim/picture001.jpg"). For internal images, this is the name of the picture (ie "Picture001")
 * @param   nameLen:	IN:  Size in bytes of the name buffer.
 * @param   categoryP:	OUT: Category of the picture if it is stored in the image database. 0xFF if it is a VFS picture.
 * @param   recIndexP:	OUT: Record index of the picture if it is stored in the image database. kInvalidRecIndex if it is a VFS picture.
 * @param   volRefNumP:	OUT: VFS volume ref number of the picture if it is stored on a VFS volume. vfsInvalidVolRef if it is stored in the internal database.
 * @param   createdP:	OUT: Timestamp when picture was creatd. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   sizeP:		OUT: Size of the image data in bytes. Pass NULL for this parameter if you don't want to retrieve it.
 * @param   path:		OUT: Directory that the picture is stored in if it is a VFS image (ie /DCIM). Empty string if it is an internal image.
 * @param   pathLen:	IN:  Size in bytes of the path buffer.
 * @param   lockedP:	OUT: true if the image is locked. False otherwise. VFS images are never locked.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterGetImageInfo(UInt16 libRef, ImageIteratorHandle iter,
                             UInt16 index, char *name, UInt16 nameLen,
                             UInt16 *categoryP, UInt16 *recIndexP,
                             UInt16 *volRefNumP, UInt32* createdP,
                             UInt32* sizeP, char *path, UInt16 pathLen,
                             Boolean *lockedP)
    SYS_TRAP(kImageLibTrapIterGetImageInfo)  ;


/****************************************************************/
/* ImagelibIterGetCount*/

/**
 * Retrieves the number of images stored in the iterator data structure.
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * Iterator routines that take an index should ensure that the index is
 * less than the value returned in imageCountP.
 *
 * @see ImageLibIterateImages
 * @see ImageLibIterateVFSImages
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   iter:			IN:  Handle to an iterator.
 * @param   imageCountP:	OUT: Number of images in the iterator data structure.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImagelibIterGetCount(UInt16 libRef, ImageIteratorHandle iter,
                         UInt16 *imageCountP)
    SYS_TRAP(kImagelibTrapIterGetCount) ;

/****************************************************************/
/* ImageLibIterDeleteImage*/

/**
 * Deletes an image (either from the database or from a VFS volume)
 *
 * Permanently deletes an image given a reference to the image in an iterator.
 * Given an iterator and an index to an image in the iterator, this routine
 * determines whether the image resides in the internal database or on a VFS
 * volume. If it resides on a VFS volume, the routine attempts to delete it by
 * calling VFSFileDelete. If it is an internal image, the picture is nuked by
 * calling ImageLibDeleteImage. After the image is deleted, it is removed from
 * the iterator chain. All subsequent images in the iterator data structure move
 * down one spot. The count of images in the iterator chain is decremented by
 * one.
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * Deleting an image invalidates the iterator index for all subsequent
 * images in the iterator chain. If you need to delete a range of images in an
 * iterator, it is best to loop from back to front.
 *
 * @param   libRef:  IN:  Library reference number.
 * @param   iter:    IN:  Handle to an iterator.
 * @param   index:   IN:  Index of the image to delete. This is an index into the iterator, not an index to the image database.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterDeleteImage(UInt16 libRef,ImageIteratorHandle iter,
                            UInt16 index)
    SYS_TRAP(kImageLibTrapIterDeleteImage) ;

/****************************************************************/
/* ImageLibIterRemoveImage*/

/**
 * Removes an image from an iterator chain without deleting it from its
 * original location.
 *
 * Removes an image from an iterator chain without deleting it from its
 * original location. Once an image is removed, it can no longer be accessed via
 * the iterator. However, the image is still accessible from its original location
 * (either in the image database or on a VFS card).
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * Removing an image invalidates the iterator index for all subsequent
 * images in the iterator chain. If you need to remove a range of images in an
 * iterator, it is best to loop from back to front.
 *
 * @param   libRef:  IN:  Library reference number.
 * @param   iter:    IN:  Handle to an iterator.
 * @param   index:   IN:  Index of the image to remove. This is an index into the iterator, not an index to the image database.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterRemoveImage(UInt16 libRef, ImageIteratorHandle iter,
                            UInt16 index)
     SYS_TRAP(kImageLibTrapIterRemoveImage) ;


/****************************************************************/
/* ImageLibIterFree*/

/**
 * Frees the memory associated with an iterator chain.
 *
 * The iterator handle should be created with ImageLibIterateImages or
 * ImageLibIterateVFSImages.
 *
 * Once the iterator has been freed, it should not be used anymore.
 *
 * @param   libRef:  IN:  Library reference number.
 * @param   iter:    IN:  Handle to an iterator.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterFree (UInt16 libRef, ImageIteratorHandle iter)
    SYS_TRAP(kImageLibTrapIterFree);


/****************************************************************/
/* ImageLibIterDrawImage*/

/**
 * Draws an image in an iterator chain to the current draw window at x,y.
 * x and y are window relative coordinates.
 *
 * If the image resides in the internal database, this routine calls
 * ImageLibImageDraw to draw it. If it is a VFS image the call is dispatched to
 * ImageLibImageDrawFromVFS
 *
 * @param   libRef:  IN:  Library reference number.
 * @param   iter:    IN:  Handle to an iterator.
 * @param   index:   IN:  Index of the image to draw. This is an index into the iterator, not an index to the image database.
 * @param   x:       IN:  x location to draw the preview image, relative to the current draw window.
 * @param   y:       IN:  y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterDrawImage(UInt16 libRef, ImageIteratorHandle iter,
                          UInt16 index, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapIterDrawImage);


/****************************************************************/
/* ImageLibIterDrawThumbnail*/

/**
 * Draws a thumbnail for an image in an iterator chain to the current draw
 * window at x,y. x and y are window relative coordinates.
 *
 * If the image resides in the internal database, this routine calls
 * ImageLibThumbnailDraw to draw it. If it is a VFS image the call is dispatched to
 * ImageLibThumbnailDrawFromVFS
 *
 * @param   libRef:  IN:  Library reference number.
 * @param   iter:    IN:  Handle to an iterator.
 * @param   index:   IN:  Index of the image to draw. This is an index into the iterator, not an index to the image database.
 * @param   x:       IN:  x location to draw the preview image, relative to the current draw window.
 * @param   y:       IN:  y location to draw the preview image, relative to the current draw window.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterDrawThumbnail (UInt16 libRef, ImageIteratorHandle iter,
                               UInt16 index, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapIterDrawThumbnail);

/****************************************************************/
/* ImageLibIterSearch*/

/**
 * Searches for an image in an iterator chain by name. To match, a picture's
 * name must the name string starting in the first character. If partical match
 * is true, then the entire string must match. Case is ignored for all
 * comparisons. Pattern matching and wildcards are not supported.
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   iterH:        IN:  Handle to an iterator.
 * @param   name:         IN:  String to search for.
 * @param   partialMatch: IN:  If true, only the beginning of a picture name must match. If false, the entire name must match.
 * @param   index:        OUT: Contains the index of the first picture that matches the search string. kInvalidRecIndex if no match found.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterSearch(UInt16 libRef, ImageIteratorHandle iterH,
                       const char *name, Boolean partialMatch,
                       UInt16 *index)
    SYS_TRAP(kImageLibTrapIterSearch);

/****************************************************************/
/* ImageLibSetProgressCallback*/

/**
 * Sets a progress callback function. The image library will invoke this routine
 * periodically during long processing, such as compressing or decompressing a
 * jpeg or resizing an image.
 *
 * All of the APIs that decompress or resize images use the callback
 * mechanism. ImageLibImageSave also invokes it.
 *
 * max is the total number of operations that must be performed. For a JPEG
 * compression or decompression, this equates to the number of horizontal
 * scan lines in the image. The progress value will range from 0 to max.
 * The imaging library will invoked the callback repeatedly. On each pass, the
 * progress variable will increase by some unspecified amount. The operation
 * argument determines the type of operation being performed. Possible values
 * are kResizeOperation, kJPEGOperation, and kVFSOperation.
 *
 * The callback function should have the following signature:
 *
 * @code void callback(Int32 progress, Int32 max, Int8 operation);
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   callback:	IN:  Address of the callback function. Pass NULL to reset the callback to nothing.
 * @retval  void
 *
 ****************************************************************/
void ImageLibSetProgressCallback(UInt16 libRef,
                                 ImageLibProgressCallbackType callback)
    SYS_TRAP(kImageLibTrapSetProgressCallback);


/****************************************************************/
/* ImageLibCopyImage*/

/**
 * Creates a copy of an image in the database.
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   recIndex:     IN:  Index of the image to copy.
 * @param   newName:      IN:  Name of the newly copied image.
 * @param   newRecIndexP: OUT: Index of the newly copied image. Pass NULL if you don't care about this value.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibCopyImage(UInt16 libRef, UInt16 recIndex,
                      const char *newName, UInt16 *newRecIndexP)
    SYS_TRAP(kImageLibTrapCopyImage);


/****************************************************************/
/* ImageLibFindImageByID*/

/**
 * Determines the record index of an image in the database given a unique ID.
 * The image is found by calling DmFindRecordByID on the main database.
 *
 * If you need to store a permanent reference to a picture in the
 * database, it is best to store its unique ID. You can obtain the unique ID
 * of an image by calling ImageLibGetImageInfo. To access an image by unique
 * ID, call ImageLibFindImageByID to retrieve the record index, and then use
 * the record index to access the picture.
 *
 * @see ImageLibGetImageInfo
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   uniqueID:     IN:  Unique ID of the image to find.
 * @param   recIndexP:    OUT: Record index of the image.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibFindImageByID (UInt16 libRef, UInt32 uniqueID, UInt16 *recIndexP)
    SYS_TRAP(kImageLibTrapFindImageByID);


/****************************************************************/
/* ImageLibPreviewStart*/

/**
 * Turns on the direct draw preview mode of the camera.
 *
 * This routine serves as a front end to the camera driver's
 * CameraLibPreviewStart function. After being called, the camera will display
 * preview images to the supplied window at the given coordinates.
 *
 * @see ImageLibPreviewStop
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   windowH:      IN:  Window handle where preview images should be drawn. This should generally be a screen window.
 * @param   x:            IN:  x location to draw the preview image. Relative to windowH.
 * @param   y:            IN:  y location to draw the preview image. Relative to windowH.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibPreviewStart (UInt16 libRef, void * windowH, Coord x, Coord y)
    SYS_TRAP(kImageLibTrapPreviewStart);

/****************************************************************/
/* ImageLibPreviewStop*/

/**
 * Turns off the direct draw preview mode of the camera.
 *
 * This routine serves as a front end to the camera driver's
 * CameraLibPreviewStop function. It halts previews that were initiated by a
 * call to ImageLibPreviewStart.
 *
 * @see ImageLibPreviewStart
 *
 * @param   libRef:       IN:  Library reference number.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibPreviewStop  (UInt16 libRef)
    SYS_TRAP(kImageLibTrapPreviewStop);

/****************************************************************/
/* ImageLibDrawJPEG*/

/**
 * Decompresses a blob of JPEG data and draws it to the current draw window.
 * The picture is drawn at left,top, which are relative to the draw window.
 *
 * If the image needs to be resized, its aspect ratio is maintained. To
 * determine how an image will be resized beforehand, call
 * ImageLibGetDimensionsJPEG.
 *
 * @see ImageLibGetDimensionsJPEG
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   jpegDataP:    IN:  Pointer to a JPEG image in memory.
 * @param   jpegSize:     IN:  Size of the source image in bytes.
 * @param   left:         IN:  x location to draw the image, relative to the current draw window.
 * @param   top:          IN:  y location to draw the image, relative to the current draw window.
 * @param   fitToScreen:  IN:  If true, the JPEG is resized to fit in a box defined by width and height. If false, image is not resized.
 * @param   width:        IN:  Max width of drawn image. Ignored if fitToScreen is false.
 * @param   height:       IN:  Max height of drawn image. Ignored if fitToScreen is false.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibDrawJPEG(UInt16 libRef, MemPtr jpegDataP, UInt32 jpegSize,
                     Coord left, Coord top, Boolean fitToScreen,
                     Coord width, Coord height)
    SYS_TRAP(kImageLibTrapDrawJPEG);

/****************************************************************/
/* ImageLibTurnCameraOff*/

/**
 * Deactivates the camera hardware by unloading the camera driver.
 *
 * @param   libRef:       IN:  Library reference number.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibTurnCameraOff(UInt16 libRef)
    SYS_TRAP(kImageLibTrapTurnCameraOff);

/****************************************************************/
/* ImageLibSetImageID*/

/**
 * Sets the media and container IDs for an image in the database.
 *
 * The media ID and container ID should both be 21 bytes in size. The image
 * library copies 21 bytes from the supplied buffers into the main record for the
 * image.
 *
 * @see ImageLibGetImageID
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   recIndex:		IN:  Index of image.
 * @param   mediaIDP:		IN:  Media ID.
 * @param   containderIDP:	IN: Container ID.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibSetImageID(UInt16 libRef, UInt16 recIndex,
                      const char* mediaIDP, const char *containerIDP)
    SYS_TRAP(kImageLibTrapSetImageID);

/****************************************************************/
/* ImageLibGetImageID*/

/**
 * Gets the media and container IDs for an image in the database.
 *
 * The media ID and container ID should both be at least 21 bytes in size.
 * The library copies 21 bytes to the supplied buffers.
 *
 * @see ImageLibSetImageID
 *
 * @param   libRef:			IN:  Library reference number.
 * @param   recIndex:		IN:  Index of image.
 * @param   mediaIDP:		IN:  Media ID.
 * @param   containderIDP:	IN: Container ID.
 * @retval  Err     0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetImageID(UInt16 libRef, UInt16 recIndex,
                      char* mediaIDP, char *containerIDP)
    SYS_TRAP(kImageLibTrapGetImageID);

/****************************************************************/
/* ImageLibGetDimensions*/

/**
 * Retrieves the dimensions of an image in the database.
 *
 * This routine allows clients of the library to query the dimensions of an image
 * in the database. If fitToDimensions is false, the routine copies the full
 * dimensions of the image to widthP and heightP. The initial values of widthP
 * and heightP are ignored. If fitToDimensions is true, the routine calculates
 * how big the image will be after it is downsized to fit in a box determined
 * by the initial values of widthP and heightP.
 *
 * @param   libRef:				IN:  Library reference number.
 * @param   recIndex:			IN:  Index of image.
 * @param   widthP:				IN,OUT: On output, contains the width of the image. On input, if fitToDimensions is true, determines the max width that the image should be.
 * @param   heightP:			IN,OUT: On output, contains the height of the image. On input, if fitToDimensions is true, determines the max height that the image should be.
 * @param   fitToDimensions:	IN: If true, tells the library to return the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetDimensions     (UInt16 libRef, UInt16 recIndex,
                               Coord *width, Coord *height,
                               Boolean fitToDimensions)
    SYS_TRAP(kImageLibTrapGetDimensions    );


/****************************************************************/
/* ImageLibGetDimensionsVFS*/

/**
 * Retrieves the dimensions of an image on a VFS volume.
 *
 * This routine allows clients of the library to query the dimensions of an image
 * that resides on a VFS volume. The routine follows the same semantics as
 * ImageLibGetDimensions.
 *
 * @see ImageLibGetDimensions
 *
 * @param   libRef:				IN:  Library reference number.
 * @param   volRefNum:			IN:  Ref number of the VFS volume
 * @param   fileName:			IN:  Filename of the image (ie "/DCIM/picture.jpg")
 * @param   widthP:				IN,OUT: On output, contains the width of the image. On input, if fitToDimensions is true, determines the max width that the image should be.
 * @param   heightP:			IN,OUT: On output, contains the height of the image. On input, if fitToDimensions is true, determines the max height that the image should be.
 * @param   fitToDimensions:	IN: If true, tells the library to return the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetDimensionsVFS  (UInt16 libRef, UInt16 volRefNum,
                               const char* fileName,  Coord *width,
                               Coord *height, Boolean fitToDimensions)
    SYS_TRAP(kImageLibTrapGetDimensionsVFS );


/****************************************************************/
/* ImageLibIterGetDimensions*/

/**
 * Retrieves the dimensions of an image in an iterator chain.
 *
 * This routine allows clients of the library to query the dimensions of an image
 * in an iterator chain. The routine follows the same semantics as
 * ImageLibGetDimensions.
 *
 * @see ImageLibGetDimensions
 *
 * @param   libRef:				IN:  Library reference number.
 * @param   iter:				IN:  Iterator handle
 * @param   index:				IN:  Index of the image in the iterator. [0,imageCount)
 * @param   widthP:				IN,OUT: On output, contains the width of the image. On input, if fitToDimensions is true, determines the max width that the image should be.
 * @param   heightP:			IN,OUT: On output, contains the height of the image. On input, if fitToDimensions is true, determines the max height that the image should be.
 * @param   fitToDimensions:	IN: If true, tells the library to return the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterGetDimensions (UInt16 libRef, ImageIteratorHandle iter,
                               UInt16 index, Coord *width, Coord *height,
                               Boolean fitToDimensions)
    SYS_TRAP(kImageLibTrapIterGetDimensions);


/****************************************************************/
/* ImageLibGetDimensionsJPEG*/

/**
 * Retrieves the dimensions of a blob of JPEG data in memory.
 *
 * This routine allows clients of the library to query the dimensions of an
 * arbitrary blob of JPEG data. The routine follows the same semantics as
 * ImageLibGetDimensions.
 *
 * @see ImageLibGetDimensions
 *
 * @param   libRef:				IN:  Library reference number.
 * @param   jpegDataP:			IN:  Pointer to a JPEG image in memory.
 * @param   jpegSize:			IN:  Size of the source image in bytes.
 * @param   widthP:				IN,OUT: On output, contains the width of the image. On input, if fitToDimensions is true, determines the max width that the image should be.
 * @param   heightP:			IN,OUT: On output, contains the height of the image. On input, if fitToDimensions is true, determines the max height that the image should be.
 * @param   fitToDimensions:	IN: If true, tells the library to return the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibGetDimensionsJPEG (UInt16 libRef, MemPtr jpegDataP, UInt32 jpegSize,
                               Coord *width, Coord *height,
                               Boolean fitToDimensions)
    SYS_TRAP(kImageLibTrapGetDimensionsJPEG);


/****************************************************************/
/* ImageLibResize*/

/**
 * Resizes an image in the database to any arbitrary size.
 *
 * This routine resizes an image in the database to any size. The routine
 * can maintain the existing aspect ratio of the image, or it can stretch it
 * to fit any size.
 *
 * @param   libRef:					IN:  Library reference number.
 * @param   recIndex:				IN:  Index of image.
 * @param   widthP:					IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   heightP:				IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   maintainAspectRatio:	IN: If true, tells the library to maintain the aspect ratio of the image. In this case, the input values of widthP and heightP serve as maximum guidelines. If false, the image is stretched to fit the input values of widthP and heightP. the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibResize (UInt16 libRef, UInt16 recIndex,
                    Coord *widthP, Coord *heightP, Boolean maintainAspectRatio)
    SYS_TRAP(kImageLibTrapResize     );

/****************************************************************/
/* ImageLibResizeVFS*/

/**
 * Resizes an image on a VFS volume to any arbitrary size.
 *
 * This routine resizes a VFS image to any size. The routine follows the same
 * semantics as ImageLibResize.
 *
 * @see ImageLibResize
 *
 * @param   libRef:					IN:  Library reference number.
 * @param   volRefNum:				IN:  Ref number of the VFS volume
 * @param   fileName:				IN:  Filename of the image (ie "/DCIM/picture.jpg")
 * @param   widthP:					IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   heightP:				IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   maintainAspectRatio:	IN: If true, tells the library to maintain the aspect ratio of the image. In this case, the input values of widthP and heightP serve as maximum guidelines. If false, the image is stretched to fit the input values of widthP and heightP. the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibResizeVFS(UInt16 libRef, UInt16 volRefNum, const char* fileName,
                      Coord *widthP, Coord *heightP,
                      Boolean maintainAspectRatio)
    SYS_TRAP(kImageLibTrapResizeVFS  );


/****************************************************************/
/* ImageLibIterResize*/

/**
 * Resizes an image in an iterator chain.
 *
 * This routine resizes an image in an iterator chain to any size. The routine
 * follows the same semantics as ImageLibResize.
 *
 * @see ImageLibResize
 *
 * @param   libRef:					IN:  Library reference number.
 * @param   iter:					IN:  Iterator handle
 * @param   index:					IN:  Index of the image in the iterator. [0,imageCount)
 * @param   widthP:					IN/OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   heightP:				IN/OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   maintainAspectRatio:	IN: If true, tells the library to maintain the aspect ratio of the image. In this case, the input values of widthP and heightP serve as maximum guidelines. If false, the image is stretched to fit the input values of widthP and heightP. the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterResize (UInt16 libRef, ImageIteratorHandle iter, UInt16 index,
                        Coord *widthP, Coord *heightP,
                        Boolean maintainAspectRatio)
    SYS_TRAP(kImageLibTrapIterResize );



/****************************************************************/
/* ImageLibResizeJPEG*/

/**
 * Resizes a JPEG in memory.
 *
 * This routine resizes a JPEG in memory to any size. The routine
 * follows the same semantics as ImageLibResize, except that it creates a new
 * JPEG in memory as a result.
 *
 * @see ImageLibResize
 *
 * @param   libRef:					IN:  Library reference number.
 * @param   jpegInputP:				IN:  Pointer to a JPEG image in memory.
 * @param   inputSize:				IN:  Size of the source image in bytes.
 * @param   widthP:					IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   heightP:				IN,OUT: On output, contains the new width of the image. On input, if maintainAspectRatio is true, determines the max width that the image should be. If maintainAspectRatio is false, determines the exact width that the image should be after resizing.
 * @param   maintainAspectRatio:	IN: If true, tells the library to maintain the aspect ratio of the image. In this case, the input values of widthP and heightP serve as maximum guidelines. If false, the image is stretched to fit the input values of widthP and heightP. the dimensions that an image would be if it was forced to draw in a box bounded by widthP and heightP.
 * @param   outputPP:				OUT: Pointer to the newly sized JPEG. The caller is responsible for freeing this memory.
 * @param   outputSizeP:			OUT: Size in bytes of the newly sized JPEG.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibResizeJPEG (UInt16 libRef, MemPtr jpegInputP, UInt32 inputSize,
                        Coord *widthP, Coord *heightP,
                        Boolean maintainAspectRatio, MemPtr *outputPP,
                        UInt32 *outputSizeP)
    SYS_TRAP(kImageLibTrapResizeJPEG);



/****************************************************************/
/* ImageLibIterRefresh*/

/**
 * Refreshes the information for an image in an iterator chain. If the image is
 * stored in the image database, this routine updates the name, recIndex,
 * category, size, timestamp, and locked status of the image. If it is a VFS
 * picture, this routine updates the timestamp and size of the picture.
 *
 * Call this function when an iterator refers to an image that has changed
 * since the iterator was created. For example, if you downsize or resize an
 * iternal image, call ImageLibIterRefresh to update the iterator information
 * for that image.
 *
 * If you rename a VFS picture that is referenced by an iterator chain, you
 * can pass its new name for the newVFSName argument. Pass the full path and
 * filename of the image. If you pass a string for this argument, the name of
 * file in the iterator chain will be updated. Pass NULL for the newVFSName
 * parameter if the iterator image refers to an internal picture or a VFS
 * picture that whose name has not been changed (If you don't know what this is
 * about, pass NULL).
 *
 * @param   libRef:		IN:  Library reference number.
 * @param   iter:		IN:  Handle to an iterator.
 * @param   index:		IN:  Index of the image in the iterator. [0,imageCount)
 * @param   newVFSName:	IN:  If picture is a VFS file, pass its new name for this parameters
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibIterRefresh(UInt16 libRef, ImageIteratorHandle iter, UInt16 index,
                        const char *newVFSName)
    SYS_TRAP(kImageLibTrapIterRefresh);

/****************************************************************/
/* ImageLibSetJPEGQuality*/

/**
 * Sets the compression factor for all JPEG compression operations performed
 * by the imaging library. The compression factor shoule be a value in the range
 * of 1-99.
 *
 * @param   libRef:       IN:  Library reference number.
 * @param   quality:      IN:  new compression quality.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibSetJPEGQuality(UInt16 libRef, Int32 value)
    SYS_TRAP(kImageLibTrapSetJPEGQuality);

/****************************************************************/
/* ImageLibGetJPEGQuality*/

/**
 * Returns the compression factor used for JPEG compression operations.
 *
 * @param   libRef:       IN:  Library reference number.
 * @retval  Int32 Compression value. [1,99]
 *
 ****************************************************************/
Int32 ImageLibGetJPEGQuality(UInt16 libRef)
    SYS_TRAP(kImageLibTrapGetJPEGQuality);



/****************************************************************/
/* ImageLibLockImage*/

/**
 * Returns the compression factor used for JPEG compression operations.
 *
 * Once an image is locked, it is locked forever and can't be unlocked.
 *
 * @param   libRef:              IN:   Library reference number.
 * @param   recIndex:            IN:   Index of the image to compress.
 * @retval  Err 0 on success, non-zero on error.
 *
 ****************************************************************/
Err ImageLibLockImage(UInt16 libRef, UInt16 recIndex)
    SYS_TRAP(kImageLibTrapLockImage);


/****************************************************************/
/* ImageLibGetAveragePictureSize*/

/**
 * Returns the average size that a picture will be at the current compression factor.
 *
 * @param   libRef:        IN:   Library reference number.
 * @param   fullsize:      IN:   True to return average of last 20 fullsize captures. False to return average of palm size captures.
 * @retval  Int32 0 Average size.
 *
 ****************************************************************/
Int32 ImageLibGetAveragePictureSize(UInt16 libRef, Boolean fullsize)
    SYS_TRAP(kImageLibTrapGetAveragePictureSize);


/****************************************************************/
/* ImageLibCheckLockedImages*/

/**
 * Verifies the locked images match the given phone number. If they don't, all locked images
 * are purged from the database.
 *
 * @param libRef:		IN: Library reference number.
 * @param phoneNumber:	IN: String containing the phone number to verify against.
 ****************************************************************/
void ImageLibCheckLockedImages(UInt16 libRef, const Char *phoneNumber)
    SYS_TRAP(kImageLibTrapCheckLockedImages);


#ifdef __cplusplus
}
#endif


#endif
