/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Imaging
 */

/**
 * @file 	palmOnePhoto.h
 * @version 3.0
 * @date 	04/28/2004
 *
 * @brief Palm Photos API: This API is used to access Palm Photos database and manipulate images.
 *
 * The database API provides users with the ability to:
 *  - Get the list of albums.
 *  - Create, delete and rename albums.
 *  - Get the list of photos in each album.
 *  - Add photos to albums.
 *
 * The image manipulation API provides users with the ability to:
 *  - Create and capture images and video clips.
 *  - Read and write image data.
 *  - Convert images to different formats.
 *  - Display images.
 *  - Scale, rotate and crop images.
 *
 */

#ifndef PALMONE_PHOTO_H_
#define PALMONE_PHOTO_H_

/* Include the common define */
#include <Common/Libraries/Imaging/palmOnePhotoCommon.h>

/* Palm OS common definitions */
#include <SystemMgr.h>



/******************************************************************
 * Callback functions
 ******************************************************************/

/**
 * File streaming callback.
 * During read operations, *sizeP contains the size of the buffer
 * passed to the callback. Setting sizeP before returning in this
 * case is simply ingored. If the user doesn't read all the data
 * in the buffer, the data left will be lost.
 * During write operations, *sizeP contains the max size of the
 * buffer used for writing. sizeP is set to the amount of data to
 * write before the callback returns. Setting sizeP to 0 means
 * no more data to write and the write process should stop.
 * The read/write calllback can be stopped at any time by the
 * callback returning the error code palmPhotoLibErrAbortReadWrite.
 *
 * @param bufferP:		IN: Pointer to the buffer containing image data.
 * @param sizeP:		IN: Size of the buffer.
 * @param userDataP:	IN: Pointer to user data set in PalmPhotoReadWriteParam.
 * @retval Err Error code.
 */
typedef Err (*PalmPhotoReadWriteCallback) (
	void 	*bufferP,
	UInt32 	*sizeP,
	void 	*userDataP
);

/**
 * Selection filter callback.
 * This function is called during a selection if provided as a
 * parameter. Given an image handle, the filter decides whether
 * or not and image should be selected. For example, a user might
 * want to select only JPEG images.
 *
 * @param imageH:		IN: Handle of the image to filter.
 * @param userDataP:	IN: Pointer to user data set in PalmPhotoSelectionParam.
 * @retval Boolean True if ok to select image, false otherwise.
 */
typedef Boolean (*PalmPhotoFilterCallback) (
	PalmPhotoHandle	imageH,
	void 			*userDataP
);

/**
 * Display callback.
 * This function is called during display image if provided as a
 * parameter. If the function returns false, the display process
 * will be canceled.
 *
 * @param userDataP:	IN: Pointer to user data set in PalmPhotoDisplayParam.
 * @retval Boolean True if the display process should continue, false otherwise.
 */
typedef	Boolean	(*PalmPhotoDisplayCallback) (
	void 			*userDataP
);

/**
 * Capture callback.
 * This function is called during capture image if provided as a
 * parameter. If the function returns false, the capture process
 * will be canceled.
 * New in v2.0.
 *
 * @param userDataP:	IN: Pointer to user data set in PalmPhotoCaptureParamV2.
 * @retval Boolean True if the capture process should continue, false otherwise.
 */
typedef	Boolean	(*PalmPhotoCaptureCallbackV2) (
	void 			*userDataP
);


/******************************************************************
 * Callback function parameters
 ******************************************************************/

/**
 * @brief Photo selection dialog parameters.
 *
 * This structure is used for the selection dialog and direct image selection.
 * When using the PMPSelectImages API, selectionCount contains the maximum
 * number of images that can be selected.  If selectionCount is set to
 * PALM_PHOTO_SELECT_ALL, the range would be [offset, end].	If albumID is set
 * to PALM_PHOTO_ALBUM_ALL, all images can be selected.
 *
 * NOTE: The API allocates memory inside of selectedImages. It is the caller's
 * responsibility to free that memory via PalmPhotoFreeSelections().
 */
typedef struct _PalmPhotoSelectionParam
{
	UInt16					albumID;		/**< IN: Album ID for selection.					*/
	UInt16					reserved;       /**< reserved for future use                        */
	UInt32					offset;			/**< IN: Range start.								*/
	Int32					selectionCount;	/**< IN: Number of images to select.				*/
	PalmPhotoFilterCallback	filterCallback;	/**< IN: Callback to filter the selections.			*/
	void					*userDataP;		/**< IN: User data to be passed during callback.	*/
	PalmPhotoSelections		selectedImages;	/**< OUT: Struct containing handle array.			*/
} PalmPhotoSelectionParam;


/**
 * @brief Photo image manipulation param.
 *
 * This parameter is used to set the callback for reading and writing
 * data of an image. The imageFormat provides the user a way to convert
 * images. For example, reading a JPEG image and setting imageFormat to
 * RGB565 should convert the JPEG to RGB565.
 */
typedef struct _PalmPhotoReadWriteParam
{
	PalmPhotoFileFormat			imageFormat;	/**< IN: Memory file image format.	*/
	UInt16						reserved;       /**< reserved for future use        */
	PalmPhotoReadWriteCallback	rwCallback;		/**< IN: Read/Write callback.		*/
	void						*userDataP;		/**< IN: User data.					*/
} PalmPhotoReadWriteParam;


/**
 * @brief Photo image display parameters.
 *
 * The bounds might be ignored by the application.
 * New in v2.0: If playing a video, winH and rect are ignored.
 */
typedef struct _PalmPhotoDisplayParam
{
	WinHandle					winH;	 			/**< IN: WinHandle.				*/
	RectangleType				rect;				/**< IN: Display rectangle.		*/
	PalmPhotoDisplayCallback	displayCallback;	/**< IN: display callback.		*/
	void						*userDataP;			/**< IN: User data.				*/
} PalmPhotoDisplayParam;

/**
 * @brief Photo image create parameters.
 *
 * Set imageInfo can determines the type of the file.  To create a
 * JPEG image, set imageInfo.fileFormat to palmPhotoJPEGFileFormat.
 */

typedef struct _PalmPhotoCreateParam
{
	PalmPhotoFileLocation	fileLocation;	/**< IN: file location of new file.		*/
	PalmPhotoImageInfo		imageInfo;		/**< IN: file information.				*/
} PalmPhotoCreateParam;

/**
 * @brief Photo/video image capture parameters.
 *
 * The callback function allows the calling application to cancel the image
 * capture operation.  The user may cancel the operation by interacting with
 * the capture application.
 * New in v2.0.
 */

typedef struct _PalmPhotoCaptureParamV2
{
	PalmPhotoFileLocation		fileLocation;		/**< IN: file location of new file.	*/
													/**  	(Note: only VFSFile 		*/
													/**  	supported for video)        */
	PalmPhotoImageInfo			imageInfo;			/**< IN: file information.			*/
	PalmPhotoCaptureCallbackV2	captureCallback;	/**< IN: capture callback.			*/
	void						*userDataP;			/**< IN: User data.					*/

} PalmPhotoCaptureParamV2;


/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************
 * Standard library open, close, sleep and wake functions
 ******************************************************************/

/// Standard library open function
/// @param refNum: IN: library reference number
/// @retval Err Error code.
extern Err PalmPhotoLibOpen(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapOpen);

/// Standard library close function
/// @param refNum: IN: library reference number
/// @retval Err Error code.
extern Err PalmPhotoLibClose(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapClose);

/// Standard library sleep function
/// @param refNum: IN: library reference number
/// @retval Err Error code.
extern Err PalmPhotoLibSleep(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapSleep);

/// Standard library wake function
/// @param refNum: IN: library reference number
/// @retval Err Error code.
extern Err PalmPhotoLibWake(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapWake);


/******************************************************************
 * Custom library API functions
 ******************************************************************/

/******************************************************************
 * Photo API v1.0 functions
 ******************************************************************/

/**
 * Get the number of albums in Palm Photos.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:	IN: Library reference number.
 * @retval			UInt16 Album count.
 */
extern UInt16  PalmPhotoAlbumCount(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapBase + 5);

/**
 * Get an album ID given the album index.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:	IN: Library reference number.
 * @param index:	IN: Album index.
 * @retval			UInt16 Album ID. If album doesn't exist, return PALM_PHOTO_ALBUM_ALL.
 */
extern UInt16 PalmPhotoGetAlbumID(UInt16 refNum, UInt16 index)
	PMP_LIB_TRAP(sysLibTrapBase + 6);

/**
 * Get an album name given the album ID.
 * The buffer that will hold the album name is allocated by the user.
 *
 * @param refNum:	IN:  Library reference number.
 * @param albumID:	IN:  Album ID
 * @param nameP:	OUT: Buffer that will contain the name of the album.
 * @param bufSize:	IN:  Size of the buffer pointed by nameP.
 * @retval			Err Error code.
 */
extern Err PalmPhotoGetAlbumName(UInt16 refNum, UInt16 albumID, Char *nameP, UInt16 bufSize)
	PMP_LIB_TRAP(sysLibTrapBase + 7);

/**
 * Create a new album.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:	IN:  Library reference number.
 * @param nameP:	IN:  String that contains the name of the new album.
 * @param errP:		OUT: Error code.
 * @retval			UInt16 If successful, returns the ID of the newly created album.
 *					Otherwise, returns PALM_PHOTO_ALBUM_ALL.
 */
extern UInt16 PalmPhotoNewAlbum(UInt16 refNum, const Char *nameP, Err *errP)
	PMP_LIB_TRAP(sysLibTrapBase + 8);

/**
 * Delete an album by album ID.
 *
 * @param refNum:			IN: Library reference number.
 * @param albumID:			IN: ID of the Album to delete.
 * @param moveToUnfiled:	IN: If true, photos from the deleted album are moved to Unfiled.
 *                              Otherwise, they are deleted.
 * @retval					Err Error code.
 */
extern Err PalmPhotoDeleteAlbum(UInt16 refNum, UInt16 albumID, Boolean moveToUnfiled)
	PMP_LIB_TRAP(sysLibTrapBase + 9);

/**
 * Rename an album.
 * The "Unfiled" album can't be renamed.
 * If Photos has "Camera" album, it also can't be renamed.
 *
 * @param refNum:	IN: Library reference number.
 * @param albumID:	IN: ID of the Album to delete.
 * @param nameP:	IN: New album name.
 * @retval			Err Error code.
 */
extern Err PalmPhotoRenameAlbum(UInt16 refNum, UInt16 albumID, const Char *nameP)
	PMP_LIB_TRAP(sysLibTrapBase + 10);

/**
 * Get image count for an album.
 *
 * @param refNum:	IN: Library reference number.
 * @param albumID:	IN: ID of the Album to delete.
 * @retval			Err Error code.
 */
extern UInt16 PalmPhotoImageCount(UInt16 refNum, UInt16 albumID)
	PMP_LIB_TRAP(sysLibTrapBase + 11);

/**
 * Select images within a specific range.
 * The selected images are passed in the selection parameter.
 * To free the memory allocated, use the PalmPhotoFreeSelections() function.
 *
 * @param refNum:			IN: Library reference number.
 * @param photoSelectionP:	IN,OUT: Palm Photo selection parameter. Contains array of selections on return.
 * @retval					Err Error code.
 */
extern Err PalmPhotoSelectImages(UInt16 refNum, PalmPhotoSelectionParam *photoSelectionP)
	PMP_LIB_TRAP(sysLibTrapBase + 12);

/**
 * Select images using the Media Selection/Deletion dialog.
 * The selected images are passed in the selection parameter.
 * To free the memory allocated, use the PalmPhotoFreeSelections() function.
 *
 * @param refNum:			IN: Library reference number.
 * @param photoSelectionP:	IN,OUT: Palm Photo selection parameter. Contains array of selections on return.
 * @param dlgType:			IN: Selection dialog type (eg palmPhotoDlgSelection...)
 * @param handleCard:		IN: If true, the dialog with intercept card notifications.
 * @retval					Err Error code.
 */
extern Err PalmPhotoSelectDlg(UInt16 refNum, PalmPhotoSelectionParam *photoSelectionP, PalmPhotoDlgType dlgType, Boolean handleCard)
	PMP_LIB_TRAP(sysLibTrapBase + 13);

/**
 * Free memory allocated during the photos selection.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:			IN: Library reference number.
 * @param photoSelectionP:	IN,OUT: Palm Photo selection parameter.
 */
extern void PalmPhotoFreeSelections(UInt16 refNum, PalmPhotoSelectionParam *photoSelectionP)
	PMP_LIB_TRAP(sysLibTrapBase + 14);

/**
 * Create a new image.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:		IN:  Library reference number.
 * @param createParamP:	IN:  Palm Photo create parameter.
 * @param errP:			OUT: Error code.
 * @retval				PalmPhotoHandle The newly created photo handle or NULL if unsuccessful.
 */
extern PalmPhotoHandle PalmPhotoCreateImage(UInt16 refNum, const PalmPhotoCreateParam *createParamP, Err *errP)
	PMP_LIB_TRAP(sysLibTrapBase + 15);

/**
 * Open an existing image.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:		IN:  Library reference number.
 * @param photoFileP:	IN:  Palm Photo file location.
 * @param errP:			OUT: Error code.
 * @retval				PalmPhotoHandle The opened photo handle or NULL if unsuccessful.
 */
extern PalmPhotoHandle PalmPhotoOpenImage(UInt16 refNum, const PalmPhotoFileLocation *photoFileP, Err *errP)
	PMP_LIB_TRAP(sysLibTrapBase + 16);

/**
 * Close an image.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:	IN: Library reference number.
 * @param imageH:	IN: Image handle.
 */
extern void PalmPhotoCloseImage(UInt16 refNum, PalmPhotoHandle imageH)
	PMP_LIB_TRAP(sysLibTrapBase + 17);

/**
 * Get an image information.
 *
 * @param refNum:	IN:  Library reference number.
 * @param imageH:	IN:  Image handle.
 * @param infoP:	OUT: Image info structure.
 * @retval			Err Error code.
 */
extern Err PalmPhotoGetImageInfo(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoImageInfo *infoP)
	PMP_LIB_TRAP(sysLibTrapBase + 18);

/**
 * Get an image extra information size.
 *
 * @param refNum:	IN: Library reference number.
 * @param imageH:	IN: Image handle.
 * @param infoType:	IN: Extra info type to retrieve the size from.
 * @retval			UInt32 The size of the extra information of the specified type.
 */
extern UInt32 PalmPhotoGetImageExtraInfoSize (UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoExtraInfoType infoType)
	PMP_LIB_TRAP(sysLibTrapBase + 19);

/**
 * Get an image extra information.
 *
 * Note: new info types (palmPhotoExtraInfoSound and palmPhotoExtraInfoUID) added in 3.0.
 *
 * @param refNum:		IN:  Library reference number.
 * @param imageH:		IN:  Image handle.
 * @param extraInfoP:	OUT: Structure that will hold the extra info requested.
 * @retval				Err Error code.
 */
extern Err PalmPhotoGetImageExtraInfo(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoExtraInfoParam *extraInfoP)
	PMP_LIB_TRAP(sysLibTrapBase + 20);

/**
 * Set an image extra information.
 * When user set palmPhotoExtraInfoLocation, if the file name contains illegal
 * characters, the function will remove them. If the file name has an incorrect
 * file extension, the function will add the correct one.
 * For example, if the format is Jpeg, "*22.j" will be converted to "22.jpg".
 *
 * @param refNum:		IN: Library reference number.
 * @param imageH:		IN: Image handle.
 * @param extraInfoP:	IN: Structure that the extra info the extra info to set.
 * @retval				Err Error code.
 */
extern Err PalmPhotoSetImageExtraInfo(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoExtraInfoParam *extraInfoP)
	PMP_LIB_TRAP(sysLibTrapBase + 21);

/**
 * Add an image to a Palm Photos album.
 * A new image is created and added to the Photos database, in the specified album.
 * The image will be re-encoded in JPEG format at the specified quality.
 * If album doesn't exist, the image will be added to the "Unfiled" album.
 * Passing albumID of PALM_PHOTO_ALBUM_ALL results in the image being added
 * to the "Unfiled" album.
 * For compatibility only; new development should use the V2 version.
 *
 * @param refNum:		IN:  Library reference number.
 * @param albumID:		IN:  Album ID for the image.
 * @param imageH:		IN:  Image handle.
 * @param addedImageH:	OUT: The new image added to the album.
 * @param imageQuality:	IN:  Image quality.
 * @retval				Err Error code.
 */
extern Err PalmPhotoAddImage(UInt16 refNum, UInt16 albumID, PalmPhotoHandle imageH, PalmPhotoHandle* addedImageH, PalmPhotoQualityType imageQuality)
	PMP_LIB_TRAP(sysLibTrapBase + 22);

/**
 * Delete an image.
 *
 * @param refNum:		IN: Library reference number.
 * @param imageH:		IN: Image handle.
 * @retval				Err Error code.
 */
extern Err PalmPhotoDeleteImage(UInt16 refNum, PalmPhotoHandle imageH)
	PMP_LIB_TRAP(sysLibTrapBase + 23);

/**
 * Read image data given an image handle.
 * The image format specifies what type of buffer the user expects to copy
 * read data into. For example, to convert a JPEG image to RBG565 data, open
 * a JPEG image, and set readParamP->imageFormat to palmPhotoRGB565FileFormat.
 * The callback function should copy the image data from *bufferP into a buffer
 * referenced within readParamP->userDataP.
 *
 * @param refNum:		IN: Library reference number.
 * @param imageH:		IN: Image handle.
 * @param readParamP:	IN,OUT: Read parameters.
 * @retval				Err Error code.
 */
extern Err PalmPhotoReadImage(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoReadWriteParam *readParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 24);

/**
 * Write image data given an image handle.
 * The writeParamP->imageFormat parameter specifies the source format the calling
 * application is providing to the API to write into storage.  The only valid source
 * formats are RGB565 and RGB888.  For example, to convert an RGB565 image to a JPEG,
 * you would create a JPEG image and pass its handle in imageH; then you would set
 * writeParamP->imageFormat to palmPhotoRGB565FileFormat.  The actual RGB image data
 * would be referenced by a pointer inside writeParamP->userDataP.  The callback
 * function will pass the RGB data to the API by copying it into *bufferP.
 *
 * @param refNum:		IN: Library reference number.
 * @param imageH:		IN,OUT: Image handle.
 * @param writeParamP:	IN: Write parameters.
 * @retval				Err Error code.
 */
extern Err PalmPhotoWriteImage(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoReadWriteParam *writeParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 25);

/**
 * Display an image or play a video.
 * A still image referenced by imageH will be drawn in the window and
 * rectangle specified within *displayParamP.  If the window handle is NULL, the
 * current draw window is used.  A video referenced by imageH will be played in
 * the video player, and the window and rectangle will be ignored.
 *
 * The image is displayed "shrink to fit" mode, where 100% of the image
 * is represented on the handheld display. (See also PalmPhotoDisplayImageToSizeV3
 * for "best fit" scaling.)
 *
 * If the callback function returns false, the Photo API will halt
 * the draw or play operation.
 *
 * @param refNum:			IN: Library reference number.
 * @param imageH:			IN: Image handle.
 * @param displayParamP:	IN: Display parameter.
 * @retval					Err Error code.
 */
extern Err PalmPhotoDisplayImage(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoDisplayParam *displayParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 26);

/**
 * Get an image thumbnail size.
 *
 * @param refNum:	IN:  Library reference number.
 * @param imageH:	IN:  Image handle.
 * @param widthP:	OUT: Width of the thumbnail.
 * @param heightP:	OUT: Height of the thumbnail.
 * @retval			Err Error code.
 */
extern Err PalmPhotoGetThumbSize(UInt16 refNum, PalmPhotoHandle imageH, UInt16 *widthP, UInt16 *heightP)
	PMP_LIB_TRAP(sysLibTrapBase + 27);

/**
 * Checks the version of the Palm Photo Library.
 *
 * @param refNum:		IN:  Library reference number.
 * @param sdkVersion:	IN:  The version the application expects.
 * @param libVersionP:	OUT: The actual version of the library.
 * @retval				Err Error code.
 */
extern Err PalmPhotoLibGetVersion(UInt16 refNum, UInt32 sdkVersion, UInt32* libVersionP)
	PMP_LIB_TRAP(sysLibTrapBase + 29);


/******************************************************************
 * Photo API v2.0 functions
 ******************************************************************/

/**
 * Return a scaled image.
 * New in v2.0.
 *
 * @param refNum:			IN:  Library reference number.
 * @param imageH:			IN:  Image handle.
 * @param scaledImageHP:	OUT: The new image scaled.
 * @param width:			IN:  New image width (must be <= existing width).
 * @param height:			IN:  New image height (must be <= existing height).
 * @retval					Err Error code.
 */
extern Err PalmPhotoScaleImageV2(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoHandle *scaledImageHP, UInt32 width, UInt32 height)
	PMP_LIB_TRAP(sysLibTrapBase + 30);

/**
 * Return a rotated image.
 * New in v2.0.
 *
 * @param refNum:			IN:  Library reference number.
 * @param imageH:			IN:  Image handle.
 * @param rotatedImageHP:	OUT: The new image rotated.
 * @param rotation:			IN:  Rotation angle (rotation is counter-clockwise).
 * @retval					Err Error code.
 */
extern Err PalmPhotoRotateImageV2(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoHandle *rotatedImageHP, Int16 rotation)
	PMP_LIB_TRAP(sysLibTrapBase + 31);

/**
 * Return a cropped image.
 * New in v2.0.
 *
 * @param refNum:			IN:  Library reference number.
 * @param imageH:			IN:  Image handle.
 * @param croppedImageHP:	OUT: The new image cropped.
 * @param cropRectP:		IN:  Cropping rectangle.
 * @retval					Err  Error code.
 */
extern Err PalmPhotoCropImageV2(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoHandle *croppedImageHP, RectangleType *cropRectP)
	PMP_LIB_TRAP(sysLibTrapBase + 32);

/**
 * Convert an image.
 * New in v2.0.
 *
 * @param refNum:			IN:  Library reference number.
 * @param imageH:			IN:  Image handle.
 * @param convertedImageHP:	OUT: The new image converted.
 * @param createParamP:		IN:  File information and location for the new image.
 * @retval					Err  Error code.
 */
extern Err PalmPhotoConvertImageV2(UInt16 refNum, PalmPhotoHandle imageH, PalmPhotoHandle *convertedImageHP, const PalmPhotoCreateParam *createParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 33);

/**
 * Get the error code from the last Photo API function that executed.
 * Sets the Photo API error code to errNone, if the library is open.
 * If the library is not open, it does not set the error code and
 * returns palmPhotoLibErrNotOpen.
 * New in v2.0.
 *
 * @param refNum:	IN: Library reference number.
 * @retval			Err Error code.
 */
extern Err  PalmPhotoGetLastErrV2( UInt16 refNum )
	PMP_LIB_TRAP(sysLibTrapBase + 34);

/**
 * Get the number of albums in Palm Photos residing on the specified volume.
 * If volumeRef is 0, then the location is on the handheld.
 * Otherwise, it is in VFS.
 *
 * @param refNum:		IN: Library reference number.
 * @param volumeRef:	IN: Volume to search for photo albums.
 * @retval				UInt16 Album count.
 */
extern UInt16  PalmPhotoAlbumCountV2(UInt16 refNum, UInt16 volumeRef)
	PMP_LIB_TRAP(sysLibTrapBase + 35);

/**
 * Get an album ID given the album index and volume.
 * If volumeRef is 0, then the location is on the handheld.
 * Otherwise, it is in VFS.
 *
 * @param refNum:		IN: Library reference number.
 * @param volumeRef:	IN: Volume to search for photo album.
 * @param index:		IN: Album index.
 * @retval				UInt16 Album ID. If album doesn't exist, return PALM_PHOTO_ALBUM_ALL.
 */
extern UInt16 PalmPhotoGetAlbumIDV2(UInt16 refNum, UInt16 volumeRef, UInt16 index)
	PMP_LIB_TRAP(sysLibTrapBase + 36);

/**
 * Create a new album in the selected volume.
 * If volumeRef is 0, then the location is on the handheld.
 * Otherwise, it is in VFS.
 *
 * @param refNum:		IN: Library reference number.
 * @param volumeRef:	IN: Volume to search for photo album.
 * @param nameP:		IN: String that contains the name of the new album.
 * @retval				UInt16 If successful, returns the ID of the newly created album.
 *                             Otherwise, returns PALM_PHOTO_ALBUM_ALL.
 */
extern UInt16 PalmPhotoNewAlbumV2(UInt16 refNum, UInt16 volumeRef, const Char *nameP)
	PMP_LIB_TRAP(sysLibTrapBase + 37);

/**
 * Free memory allocated during the photos selection.
 *
 * @param refNum:			IN: Library reference number.
 * @param photoSelectionP:	IN,OUT: Palm Photo selection parameter.
 * @retval					Err Error code.
 */
extern Err PalmPhotoFreeSelectionsV2(UInt16 refNum, PalmPhotoSelectionParam *photoSelectionP)
	PMP_LIB_TRAP(sysLibTrapBase + 38);

/**
 * Create a new image.
 *
 * @param refNum:		IN: Library reference number.
 * @param createParamP:	IN: Palm Photo create parameter.
 * @retval				PalmPhotoHandle The newly created photo handle or NULL if unsuccessful.
 */
extern PalmPhotoHandle PalmPhotoCreateImageV2(UInt16 refNum, const PalmPhotoCreateParam *createParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 39);

/**
 * Capture a new still image or video clip using the built-in camera.
 *
 * New functionality added in 3.0:
 * Can capture video as well as still images.
 *     (For video, height and width in captureParam.imageInfo
 *	   must match a supported resolution as returned by
 *     PalmPhotoGetSupportedCaptureResolutionsV3. Location
 *	   in captureParam.fileLocation must be a VFSFile.)
 * Can limit size (in bytes) of captured media.
 *	  (Set captureParam.imageInfo.fileSize to the desired
 *    limit. If fileSize = 0, no constraint s placed on
 *    ithe file size.)
 *
 * @param refNum:			IN: Library reference number.
 * @param captureParamP:	IN: Palm Photo capture parameter.
 * @retval					PalmPhotoHandle The newly created photo handle or NULL if unsuccessful.
 */
extern PalmPhotoHandle PalmPhotoCaptureImageV2(UInt16 refNum, const PalmPhotoCaptureParamV2 *captureParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 40);

/**
 * Open an existing image.
 *
 * @param refNum:		IN: Library reference number.
 * @param photoFileP:	IN: Palm Photo file location.
 * @retval				PalmPhotoHandle The opened photo handle or NULL if unsuccessful.
 */
extern PalmPhotoHandle PalmPhotoOpenImageV2(UInt16 refNum, const PalmPhotoFileLocation *photoFileP)
	PMP_LIB_TRAP(sysLibTrapBase + 41);

/**
 * Close an image.
 *
 * @param refNum:	IN: Library reference number.
 * @param imageH:	IN: Image handle.
 * @retval			Err Error code.
 */
extern Err PalmPhotoCloseImageV2(UInt16 refNum, PalmPhotoHandle imageH)
	PMP_LIB_TRAP(sysLibTrapBase + 42);

/**
 * Add an image to a Palm Photos album.
 * The image is added to the indicated Photos album.  If the album
 * resides on a different volume than imageH, then the image is copied
 * into the new volume.  A handle to the image in the designated album is
 * returned in addedImageH.
 * No image conversion takes place.  The content of the image in the album is
 * identical to that of the original image.
 * If album doesn't exist, the image will be added to the "Unfiled" album.
 * Passing albumID of PALM_PHOTO_ALBUM_ALL results in the image being added
 * to the "Unfiled" album.
 *
 * @param refNum:		IN:  Library reference number.
 * @param albumID:		IN:  Album ID for the image.
 * @param imageH:		IN:  Image handle.
 * @param addedImageH:	OUT: The new image added to the album.
 * @retval				Err Error code.
 */
extern Err PalmPhotoAddImageV2(UInt16 refNum, UInt16 albumID, PalmPhotoHandle imageH, PalmPhotoHandle* addedImageH)
	PMP_LIB_TRAP(sysLibTrapBase + 43);

/******************************************************************
 * Photo API v3 .0 functions
 ******************************************************************/

/**
 * Check for presence of an enabled camera.
 *
 * @param refNum:			IN: Library reference number.
 * @retval					Boolean false if no camera or camera is disabled.
 *
 */
extern Boolean PalmPhotoIsCameraAvailableV3(UInt16 refNum)
	PMP_LIB_TRAP(sysLibTrapBase + 44);

/**
 * Returns resolutions that are supported by the still or video camera.
 * Returns zero (palmPhotoCaptureResNone) if no camera, or camera disabled.
 *
 * @param refNum:			IN: Library reference number.
 * @param mediaType:		IN: palmPhotoMediaTypePhoto or palmPhotoMediaTypeVideo
 * @retval					PalmPhotoCaptureResolution Flags (bit is set for each resolution that camera supports).
 *
 */
extern PalmPhotoCaptureResolution PalmPhotoGetSupportedCaptureResolutionsV3(UInt16 refNum, PalmPhotoMediaType mediaType)
	PMP_LIB_TRAP(sysLibTrapBase + 45);

/**
 * Select images using the Photo Selection/Deletion dialog, with
 * optional camera support.
 * The selected images are passed in the selection parameter.
 * To free the memory allocated, use the PalmPhotoFreeSelections() function.
 *
 * @param refNum:			IN:Library reference number.
 * @param photoSelectionP:	IN,OUT: Palm Photo selection parameter. Contains array of selections on return.
 * @param captureParamP:	IN: Palm Photo capture parameter. Set to null to suppress the camera.
 *							    Camera only supported for single select dialog type.
 *							    Camera only supported when media type is photo OR video (not all)
 * @param dlgType:			IN: Selection dialog type (eg palmPhotoDlgSelection...)
 * @param mediaType:		IN: Dialog displays only photos, only videos, or all media types.
 * @param handleCard:		IN: If true, the dialog with intercept card notifications.
 * @retval					Err Error code.
 */
extern Err PalmPhotoSelectDlgV3(UInt16 refNum, PalmPhotoSelectionParam *photoSelectionP, const PalmPhotoCaptureParamV2 *captureParamP,
			PalmPhotoDlgType dlgType, PalmPhotoMediaType mediaType, Boolean handleCard)
	PMP_LIB_TRAP(sysLibTrapBase + 46);


/**
 * Display an image or play a video.
 * A still image referenced by imageH will be drawn in the window and
 * rectangle specified within *displayParamP.  If the window handle is NULL, the
 * current draw window is used.  A video referenced by imageH will be played in
 * the video player, and the window and rectangle will be ignored.
 *
 * The image is displayed in "best fit" mode, scaled exactly so that the
 * rectangle is completely filled, then cropping anything outside the
 * rectangle. (See also PalmPhotoDisplayImage for shrink-to-fit scaling.)
 *
 * If the callback function returns false, the Photo API will halt
 * the draw or play operation.
 *
 * New in v3.0.
 *
 * @param refNum:			IN: Library reference number.
 * @param imageH:			IN: Image handle.
 * @param displayParamP:	IN: Display parameter.
 * @retval					Err Error code.
 */
extern Err PalmPhotoDisplayImageToSizeV3(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoDisplayParam *displayParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 47);

/**
 * Display thumbnail for still image or video.
 *
 * Thumbnail for the image or video referenced by imageH will be drawn
 * in the window and rectangle specified within *displayParamP. (In the
 * case of video, displays the first frame.) Use PalmPhotoGetThumbSize()
 * first to get the thumbnail size. If the window handle is NULL,
 * the current draw window is used.
 *
 * If the callback function returns false, the Photo API will halt
 * the draw or play operation.
 *
 * New in v3.0.
 *
 * @param refNum:			IN: Library reference number.
 * @param imageH:			IN: Image handle.
 * @param displayParamP:	IN: Display parameter.
 * @retval					Err Error code.
 */
extern Err PalmPhotoDisplayThumbnailV3(UInt16 refNum, PalmPhotoHandle imageH, const PalmPhotoDisplayParam *displayParamP)
	PMP_LIB_TRAP(sysLibTrapBase + 53);



#ifdef __cplusplus
}
#endif

#endif /* PALMONE_PHOTO_H_ */
