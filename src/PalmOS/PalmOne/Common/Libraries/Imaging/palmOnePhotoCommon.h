/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Imaging
 */

/**
 * @file 	palmOnePhotoCommon.h
 * @version 3.0
 * @date 	04/28/2004
 *
 * @brief	Palm Photos API: This file contains all the defines, enumerations, structures needed by #PalmPhoto.h
 *
 */

#ifndef PALMONE_PHOTO_COMMON_H_
#define PALMONE_PHOTO_COMMON_H_

/* Palm OS common definitions */
#include <SystemMgr.h>

/* If we're actually compiling the library code, then we need to
 * eliminate the trap glue that would otherwise be generated from
 * this header file in order to prevent compiler errors in CW Pro 2. */
#ifdef BUILDING_PMP
	#define PMP_LIB_TRAP(trapNum)
#else
	#define PMP_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

/*********************************************************************
 * Type and creator of Sample Library database
 *********************************************************************/

#define		PalmPhotoLibCreatorID			'PMPL'          /**< Palm Photo Library creator id */
#define		PalmPhotoLibTypeID				sysFileTLibrary /**< Standard library type */

/*********************************************************************
 * Internal library name which can be passed to SysLibFind()
 *********************************************************************/

#define		PalmPhotoLibName				"PalmPhotoLib" /**< Palm Photo Library database name */

/*********************************************************************
 * Library versioning
 *********************************************************************/

/**
 * @name Version Numbers
 */
/*@{*/
#define	kPalmPhotoLibVersion1		sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)
#define	kPalmPhotoLibVersion2		sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0)
#define	kPalmPhotoLibVersion3		sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)
#define	kPalmPhotoLibVersion		kPalmPhotoLibVersion3
/*@}*/

/******************************************************************
 * Error codes
 ******************************************************************/

/** Invalid parameter.											*/
#define palmPhotoLibErrParam				(oemErrorClass + 0x100)
/** Library not open.											*/
#define palmPhotoLibErrNotOpen				(palmPhotoLibErrParam | 1)
/** Library is still open.										*/
#define palmPhotoLibErrStillOpen			(palmPhotoLibErrParam | 2)
/** Photos database does not exist on device.					*/
#define	palmPhotoLibErrNotPhotos			(palmPhotoLibErrParam | 3)
/** Could not open Photos database.								*/
#define	palmPhotoLibErrNotOpenPhotos 		(palmPhotoLibErrParam | 4)
/** The album already exists.									*/
#define	palmPhotoLibErrAlbumExist			(palmPhotoLibErrParam | 5)
/** The selected album does not exist.							*/
#define	palmPhotoLibErrAlbumNotExist		(palmPhotoLibErrParam | 6)
/** The album is full.											*/
#define	palmPhotoLibErrAlbumFull			(palmPhotoLibErrParam | 7)
/** The album name is too long.									*/
#define	palmPhotoLibErrAlbumNameTooLong		(palmPhotoLibErrParam | 8)
/** The album cannot be renamed or deleted.						*/
#define	palmPhotoLibErrAlbumReadOnly		(palmPhotoLibErrParam | 9)
/** Insufficient free space in dynamic heap.					*/
#define	palmPhotoLibErrLowHeap				(palmPhotoLibErrParam | 10)
/** Insufficient free space in storage heap.					*/
#define	palmPhotoLibErrLowMemory			(palmPhotoLibErrParam | 11)
/** An image with that name already exists.						*/
#define	palmPhotoLibErrImageExist			(palmPhotoLibErrParam | 12)
/** Specified image does not exist.								*/
#define	palmPhotoLibErrImageNotFound		(palmPhotoLibErrParam | 13)
/** Image format not supported for the attempted operation.		*/
#define	palmPhotoLibErrFormatNotSupported	(palmPhotoLibErrParam | 14)
/** Image is not in the Photos database.						*/
#define	palmPhotoLibErrImageNotInPhotos		(palmPhotoLibErrParam | 15)
/** Invalid path/image name.									*/
#define	palmPhotoLibErrImageInvalidPath		(palmPhotoLibErrParam | 16)
/** User canceled operation.									*/
#define	palmPhotoLibErrUserCancel			(palmPhotoLibErrParam | 17)
/** Unsupported handheld screen color depth.					*/
#define	palmPhotoLibErrScreenDepth			(palmPhotoLibErrParam | 18)
/** Buffer is too small.										*/
#define	palmPhotoLibErrSmallBuffer			(palmPhotoLibErrParam | 19)
/** Unknown or unclassifiable error.							*/
#define	palmPhotoLibErrUnknown				(palmPhotoLibErrParam | 20)
/** Image has no thumbnail.										*/
#define	palmPhotoLibErrNoThumbnail			(palmPhotoLibErrParam | 21)
/** Abort a read or write operation.			New in v2.0.	*/
#define	palmPhotoLibErrAbortReadWrite		(palmPhotoLibErrParam | 22)
/** Rotation angle not supported.				New in v2.0.	*/
#define	palmPhotoLibErrUnsupportedRotation	(palmPhotoLibErrParam | 23)
/** Invalid volume for operation. 				New in v2.0.	*/
#define palmPhotoLibErrInvalidVolume		(palmPhotoLibErrParam | 24)
/** Device has no camera. 						New in v2.0.	*/
#define palmPhotoLibErrNoCamera				(palmPhotoLibErrParam | 25)
/** Library version is too old.									*/
#define	palmPhotoLibErrOldVersion			(palmPhotoLibErrParam | 26)
/** Invalid operation for media protected by DRM 				*/
#define	palmPhotoLibErrProtectedMedia		(palmPhotoLibErrParam | 27)


/******************************************************************
 * Constants and Types
 ******************************************************************/

/** Media formats */

/** JPEG file format.											*/
#define	palmPhotoJPEGFileFormat				(0x0001)
/** Raw RGB 888 file format.									*/
#define	palmPhotoRGB888FileFormat			(0x0002)
/** Raw RGB 565 file format.									*/
#define	palmPhotoRGB565FileFormat			(0x0003)
/** GIF 8-bit indexed color format				New in v2.0.	*/
#define	palmPhotoGIFFileFormat				(0x0004)
/** BMP bitmap format							New in v2.0.	*/
#define	palmPhotoBMPFileFormat				(0x0005)
/** TIFF bitmap format							New in v2.0.	*/
#define	palmPhotoTIFFFileFormat				(0x0006)
/** PNG bitmap format							New in v2.0.	*/
#define	palmPhotoPNGFileFormat				(0x0007)
/** M-JPEG video format							New in v2.0.	*/
#define	palmPhotoMJPEGFileFormat			(0x1001)
/** MPEG1 video format							New in v2.0.	*/
#define	palmPhotoMPEG1FileFormat			(0x1002)
/** MPEG4 video format							New in v2.0.	*/
#define	palmPhotoMPEG4FileFormat			(0x1003)

/** AVI file with ADPCM audio + MPEG4 video		New in v2.0.	*/
#define palmPhotoFormat_AVI_ADPCM_MPEG4		(0x1113)
/** AVI file with MP3 audio + MPEG4 video		New in v2.0.	*/
#define palmPhotoFormat_AVI_MP3_MPEG4		(0x1123)
/** ASF file with ADPCM audio + MPEG4 video		New in v2.0.	*/
#define palmPhotoFormat_ASF_ADPCM_MPEG4		(0x1213)
/** ASF file with MP3 audio + MPEG4 video		New in v2.0.	*/
#define palmPhotoFormat_ASF_MP3_MPEG4	 	(0x1223)
/** 3GP file with AMR audio + MPEG4 video 		New in v3.0.	*/
#define palmPhotoFormat_3GP_AMR_MPEG4	 	(0x1333)
/** 3GP file with AMR audio + H.263 video 		New in v3.0.	*/
#define	palmPhotoFormat_3GP_AMR_H263		(0x1334)
/** 3G2 file with QCELP audio + MPEG4 video 	New in v3.0.	*/
#define	palmPhotoFormat_3G2_QCELP_MPEG4		(0x1443)
/** 3G2 file with QCELP audio + H.263 video 	New in v3.0.	*/
#define	palmPhotoFormat_3G2_QCELP_H263		(0x1444)

/** Default still capture format.				New in v3.0.	*/
#define palmPhotoPreferredImageFormat		(0xFFFD)
/** Default video capture format.				New in v3.0.	*/
#define palmPhotoPreferredVideoFormat		(0xFFFE)

/** Unsupported file format.									*/
#define	palmPhotoUnsupportedFormat 			(0xFFFF)

/** @name Old Constant Names */
/** Old (2.0) constant names for compatibility					*/
/*@{*/
#define	palmPhotoAVIM4AFileFormat			palmPhotoFormat_AVI_ADPCM_MPEG4
#define	palmPhotoAVIM43FileFormat			palmPhotoFormat_AVI_MP3_MPEG4
#define	palmPhotoASFM4AFileFormat			palmPhotoFormat_ASF_ADPCM_MPEG4
#define	palmPhotoASFM43FileFormat			palmPhotoFormat_ASF_MP3_MPEG4
/*@}*/

/** Palm Photo file format.										*/
typedef UInt16 PalmPhotoFileFormat;



/** Capture resolution supported by device camera */

/** SUBQCIF (128x96)							New in v3.0.	*/
#define palmPhotoCaptureResSUBQCIF		(0x00000001)
/** QQVGA (160x120)							 	New in v3.0.	*/
#define palmPhotoCaptureResQQVGA		(0x00000002)
/** QCIF (176x144)							 	New in v3.0.	*/
#define palmPhotoCaptureResQCIF			(0x00000004)
/** QVGA (320x240)							 	New in v3.0.	*/
#define palmPhotoCaptureResQVGA			(0x00000008)
/** CIF (352x288)							 	New in v3.0.	*/
#define palmPhotoCaptureResCIF			(0x00000010)
/** VGA (640x480)							 	New in v3.0.	*/
#define palmPhotoCaptureResVGA			(0x00000020)
/** SVGA (800x600)							 	New in v3.0.	*/
#define palmPhotoCaptureResSVGA			(0x00000040)
/** XVGA (1024x768)							 	New in v3.0.	*/
#define palmPhotoCaptureResXVGA			(0x00000080)
/** SXGA (1280x960)							 	New in v3.0.	*/
#define palmPhotoCaptureResSXGA			(0x00000100)

/** Unknown dimensions (caller must use CameraManager)		 	*/
#define palmPhotoCaptureResOther		(0x80000000)

/** None (no camera or camera disabled)							*/
#define palmPhotoCaptureResNone			(0x00000000)

/** Used with palmPhotoCaptureRes defines */
typedef UInt32 PalmPhotoCaptureResolution;

/** File stream on the device.									*/
#define palmPhotoStreamLocation				(0x0000)
/** VFS file.													*/
#define palmPhotoVFSLocation				(0x0001)
/** Memory file. Read-Only images.								*/
#define palmPhotoMemoryLocation				(0x0002)

/** Palm Photo file location type.								*/
typedef UInt16  PalmPhotoLocationType;

/** Unique ID 													*/
typedef UInt8 PalmPhotoUID[21];

/** File location.infoP points to a PalmPhotoFileLocation.		*/
#define palmPhotoExtraInfoLocation			(0x0000)
/** Date.infoP points to a DateType.							*/
#define palmPhotoExtraInfoDate				(0x0001)
/** Album.infoP points to a (Char *).							*/
#define palmPhotoExtraInfoAlbum				(0x0002)
/** Notes.infoP points to a (Char *).							*/
#define palmPhotoExtraInfoNotes				(0x0003)
/** Notes.infoP points to a PalmPhotoFileLocation. New in v3.0.	*/
#define palmPhotoExtraInfoSound 			(0x0004)
/** Notes.infoP points to a PalmPhotoUID.		   New in v3.0. */
#define palmPhotoExtraInfoImageUID 			(0x0005)
/** Notes.infoP points to a PalmPhotoUID.		   New in v3.0. */
#define palmPhotoExtraInfoSoundUID			(0x0006)
/** Notes.infoP points to a PalmPhotoUID.		   New in v3.0. */
#define palmPhotoExtraInfoCompositeUID		(0x0007)

/** Palm Photo extra information type.							*/
typedef UInt16 PalmPhotoExtraInfoType;


/** Mutiple selection dialog.									*/
#define	palmPhotoDlgSelection				(0x0000)
/** Single selection dialog.									*/
#define	palmPhotoDlgSingleSelection			(0x0001)
/** Deletion dialog.											*/
#define	palmPhotoDlgDeletion				(0x0002)

/** Palm Photo dialog type.										*/
typedef UInt16 PalmPhotoDlgType;


/** Select still image types									*/
#define	palmPhotoMediaTypePhoto				(0x0001)
/** Select video types											*/
#define	palmPhotoMediaTypeVideo				(0x0002)
/** All media types (in 3.0, limited to photos and video).		*/
#define	palmPhotoMediaTypeAll				(0xFFFF)

/** Palm Photo media type (used in PalmPhotoSelectDlgV3).		*/
typedef UInt16 PalmPhotoMediaType;


/** Highest quality compressed image.							*/
#define	palmPhotoHighestQuality				(0xFFFF)
/** High quality compressed image.								*/
#define	palmPhotoHighQuality				(0xFFFE)
/** Normal quality compressed image.							*/
#define	palmPhotoMediumQuality				(0xFFFD)
/** Low quality compressed image.								*/
#define	palmPhotoLowQuality					(0xFFFC)
/** Lowest quality compressed image.							*/
#define	palmPhotoLowestQuality				(0xFFFB)

/** Palm Photo image quality type.								*/
typedef UInt16 PalmPhotoQualityType;


/** Maximum file path length.									*/
#define	PALM_PHOTO_MAX_PATH					(255)
/** Used during selection to select maximum number of photos.	*/
#define	PALM_PHOTO_SELECT_ALL				((Int32)0xFFFFFFFF)
/** Used to select all the albums at once.						*/
#define	PALM_PHOTO_ALBUM_ALL				((UInt16)0xFFFF)
/** Used to select all the albums on the handheld at once.		*/
#define	PALM_PHOTO_ALBUM_ALL_HANDHELD		((UInt16)0xFFFF)
/** Used to select all the albums on the VFS card at once.		*/
#define	PALM_PHOTO_ALBUM_ALL_VFS			((UInt16)0xFFFE)
/** Maximum album name length on device. (Increased for v3.0.)  */
#define PALM_PHOTO_ALBUM_MAX_NAME			(255)


/******************************************************************
 * Structures
 ******************************************************************/

/**
 * @brief Photo file location.
 *
 * This structure is used to open, create and query location
 * information of an image.
 */
typedef struct _PalmPhotoFileLocation
{
	PalmPhotoLocationType	fileLocationType;	/**< File location type.		*/
	UInt16					reserved;           /**< reserved for future use    */
	union
	{
		struct {
			Char	name[dmDBNameLength];		/**< File name on the device.	*/
			UInt32	type;						/**< Type of the stream.		*/
			UInt32	creator;					/**< Creator of the stream.		*/
		} StreamFile;                           /**< File format: stream        */

		struct {
			Char	name[PALM_PHOTO_MAX_PATH+1];/**< Full path name.					*/
			UInt16	volumeRef;					/**< Volume where the file is located.	*/
			UInt16	reserved;                   /**< reserved for future use    */
		} VFSFile;                              /**< File format: VFS           */

		struct {
			void	*bufferP;					/**< Buffer holding the image.	*/
			UInt32	bufferSize;					/**< Size of the buffer.		*/
		} MemoryFile;                           /**< File format: Memory        */

	} file;										/**< Image location.			*/

} PalmPhotoFileLocation;

/**
 * @brief Photo information.
 *
 * This structure contains the basic image information.
 */
typedef struct _PalmPhotoImageInfo
{
	UInt32 					width;			/**< Width in pixels of the image.	*/
	UInt32 					height;			/**< Height in pixels of the image.	*/
	UInt32 					bitsPerPixel;	/**< Color depth of the image.		*/
	UInt32 					filesize;		/**< File size, in bytes.			*/
	PalmPhotoFileFormat 	fileFormat;		/**< File format, e.g., JPEG, GIF.	*/
	PalmPhotoQualityType	imageQuality;	/**< Image quality.					*/
} PalmPhotoImageInfo;


/**
 * @brief Photo image extra info parameters.
 *
 * Gets extra information on an image.
 */
typedef struct _PalmPhotoExtraInfo
{
	PalmPhotoExtraInfoType	infoType; 		/**< IN: Image info type.						*/
	UInt16					reserved;       /**< reserved for future use                    */
	void					*infoP;			/**< OUT: Points to the information selected.	*/
	UInt32					infoSize;		/**< OUT: Size of the buffer pointed by infoP.	*/
} PalmPhotoExtraInfoParam;

/** Photo handle.												*/
typedef struct _PalmPhotoType * PalmPhotoHandle;

/**
 * @brief Photo selections.
 *
 * This strucutre is used during image selections, either by using
 * the PalmPhotoSelectDlg() function of the PalmPhotoSelect() function.
 */
typedef struct _PalmPhotoSelections
{
	UInt32					imageCount;		/**< Number of images.			*/
	PalmPhotoHandle 		*imageH;		/**< Array of image handles.	*/
} PalmPhotoSelections;

#endif // PALMONE_PHOTO_COMMON_H_