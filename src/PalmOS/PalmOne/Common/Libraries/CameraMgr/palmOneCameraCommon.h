/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Camera Camera Library
 * @brief		This library provides camera support for Zire 72 and Treo 650
 * 				devices.
 *
 * Developers who want to provide camera support to their application on Zire 72
 * and Treo 650 devices should use Camera Manager Library API instead of the old
 * Camera Library.
 *
 * @{
 * @}
 */
/**
 * @ingroup Camera
 */

/**
 * @file	palmOneCameraCommon.h
 * @version	3.0
 * @brief	Public 68K common header file for camera support for Zire 72 and Treo 650 devices.
 *
 * This file contains the library constants and error codes used in the APIs.
 * <hr>
 */

#ifndef _PALMONECAMERAMGRCOMMON_H_
#define _PALMONECAMERAMGRCOMMON_H_

#define		kCamLibType		'libr'				/**< Default library type */
#define		kCamLibCreator	'camL'				/**< Camera Manager library creator ID */
#define		kCamLibName		"CameraLib-camL"	/**< Camera Manager library name */


/**
 * @name Library Versions
 */
/*@{*/
#define		kCamLibVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)
#define		kCamLibVersion2	sysMakeROMVersion(2, 0, 0, sysROMStageRelease, 0)
#define		kCamLibVersion3	sysMakeROMVersion(3, 0, 0, sysROMStageRelease, 0)
#define		kCamLibVersion	kCamLibVersion3
/*@}*/


/**
 * @name Features
 */
/*@{*/
#define		kCamLibFtrCaptureVersion	0		/**< Feature id for capture h/w. */
#define		kCamLibCaptureHwrVersion	0x10	/**< Feature value if capture h/w present. */
#define		kCamLibFtrVersionNum		0		/**< Feature version if camera is present. */
/*@}*/


/**
 * @name Error Codes
 */
/*@{*/
#define kCamLibErrorClass			(oemErrorClass + 0x100)
#define kCamLibErrBadParam			(kCamLibErrorClass | 0x01)/**< Invalid parameter. */
#define	kCamLibErrNoMemory			(kCamLibErrorClass | 0x02)/**< Memory error. */
#define kCamLibErrNotOpen			(kCamLibErrorClass | 0x03)/**< Library is not open. */
#define kCamLibErrStillOpen			(kCamLibErrorClass | 0x04)/**< Returned from CamLibClose() if the library is still open. */
#define	kCamLibErrInternal			(kCamLibErrorClass | 0x05)/**< Internal error. */
#define	kCamLibErrNotSupported		(kCamLibErrorClass | 0x06)/**< Unsupported function. */
#define	kCamLibErrNotCompatible		(kCamLibErrorClass | 0x07)/**< Bad Version. */
#define	kCamLibErrNoPower			(kCamLibErrorClass | 0x08)/**< Camera not powered. */
#define kCamLibErrAllFramesLocked	(kCamLibErrorClass | 0x09)/**< All video frames are locked. */
/*@}*/


/**
 * @name Control Commands
 */
/*@{*/
#define	kCamLibCtrlPowerOn					0x01/**< Powers on and initialize the camera module. No parameter. */
#define	kCamLibCtrlPowerOff					0x02/**< Powers off the camera module. No parameter. */
#define kCamLibCtrlHardwareSettingsDlg		0x03/**< Displays the hardware camera settings dialog. No parameter. */
#define kCamLibCtrlPreviewStart				0x04/**< Starts the camera preview mode. No parameter. */
#define kCamLibCtrlPreviewStop				0x05/**< Stops the camera preview mode. No parameter. */
#define kCamLibCtrlCaptureGetBitmapInfo		0x06/**< Used to determine the captured image properties. Parameter is CamLibBitmapInfoType*. */
#define kCamLibCtrlCapture					0x07/**< Captures the current image. Parameter is CamLibCaptureType*. */
#define kCamLibCtrlStatusGet				0x08/**< Used to determine if a certain camera status is currently in effect. Parameter is CamLibStatusType*. */
#define kCamLibCtrlStatusQuery				0x09/**< Used to determine if a certain camera status is available. Parameter is CamLibStatusType*. */
#define kCamLibCtrlCaptureFormatSet			0x10/**< Sets the capture format. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlCaptureFormatGet			0x11/**< Gets the current capture format. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlCaptureFormatQuery		0x12/**< Queries the capture formats available. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlCaptureSizeSet			0x13/**< Sets the capture size. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlCaptureSizeGet			0x14/**< Gets the current capture size. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlCaptureSizeQuery			0x15/**< Queries the capture sizes available. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlCaptureAreaSet			0x16/**< Sets the capture area. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlCaptureAreaGet			0x17/**< Gets the capture area. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlPreviewSizeSet			0x20/**< Sets the preview size. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlPreviewSizeGet			0x21/**< Gets the preview size. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlPreviewSizeQuery			0x22/**< Queries the preview sizes available. Parameter is CamLibImageSizeType*. */
#define kCamLibCtrlPreviewRectSet			0x23/**< Sets the preview rectangle. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlPreviewRectGet			0x24/**< Gets the preview rectangle. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlPreviewAreaSet			0x26/**< Sets the preview area. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlPreviewAreaGet			0x27/**< Gets the preview area. Parameter is RectangleType*. Low-res value. */
#define kCamLibCtrlWhiteBalanceSet			0x30/**< Sets the white balance. Parameter is CamLibWhiteBalanceType*. */
#define kCamLibCtrlWhiteBalanceGet			0x31/**< Gets the current white balance. Parameter is CamLibWhiteBalanceType*. */
#define kCamLibCtrlWhiteBalanceQuery		0x32/**< Queries the white balances available. Parameter is CamLibWhiteBalanceType*. */
#define kCamLibCtrlLightingSet				0x33/**< Sets the lighting setting. Parameter is CamLibLightingType*. */
#define kCamLibCtrlLightingGet				0x34/**< Gets the current lighting setting. Parameter is CamLibLightingType*. */
#define kCamLibCtrlLightingQuery			0x35/**< Queries the lighting settings available. Parameter is CamLibLightingType*. */
#define kCamLibCtrlZoomSet					0x36/**< Sets the zoom value. Parameter is CamLibZoomType*. */
#define kCamLibCtrlZoomGet					0x37/**< Gets the current zoom value. Parameter is CamLibZoomType*. */
#define kCamLibCtrlZoomQuery				0x38/**< Queries the zoom range. Parameter is CamLibZoomType*. */
#define kCamLibCtrlEffectsSet				0x39/**< Sets the camera effect. Parameter is CamLibEffectsType*. */
#define kCamLibCtrlEffectsGet				0x3A/**< Gets the current camera effect. Parameter is CamLibEffectsType*. */
#define kCamLibCtrlEffectsQuery				0x3B/**< Queries the camera effects available. Parameter is CamLibEffectsType*. */
#define kCamLibCtrlSaturationSet			0x3C/**< Sets the saturation level. Parameter is CamLibSaturationType*. */
#define kCamLibCtrlSaturationGet			0x3D/**< Gets the current saturation level. Parameter is CamLibSaturationType*. */
#define kCamLibCtrlSaturationQuery			0x3E/**< Queries the saturation range. Parameter is CamLibSaturationType*. */
#define kCamLibCtrlExposureSet				0x40/**< Sets the exposure. Parameter is CamLibExposureType*. */
#define kCamLibCtrlExposureGet				0x41/**< Gets the exposure. Parameter is CamLibExposureType*. */
#define kCamLibCtrlExposureQuery			0x42/**< Checks the exposure values available. Parameter is CamLibExposureType*. */
#define kCamLibCtrlContrastSet				0x43/**< Sets the contrast. Parameter is CamLibContrastType*. */
#define kCamLibCtrlContrastGet				0x44/**< Gets the current contrast. Parameter is CamLibContrastType*. */
#define kCamLibCtrlContrastQuery			0x45/**< Checks the contrast values available. Parameter is CamLibContrastType*. */
#define kCamLibCtrlSharpnessSet				0x46/**< Sets the sharpness. Parameter is CamLibSharpnessType*. */
#define kCamLibCtrlSharpnessGet				0x47/**< Gets the sharpness. Parameter is CamLibSharpnessType*. */
#define kCamLibCtrlSharpnessQuery			0x48/**< Queries the sharpness range. Parameter is CamLibSharpnessType*. */
#define kCamLibCtrlStreamFormatSet			0x50/**< Sets the video stream format. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlStreamFormatGet			0x51/**< Gets the video stream format. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlStreamFormatQuery		0x52/**< Queries the video stream formats available. Parameter is CamLibCaptureFormatType*. */
#define kCamLibCtrlStreamSizeSet			0x53/**< Sets the video stream size. Parameter is CamLibCaptureSizeType*. */
#define kCamLibCtrlStreamSizeGet			0x54/**< Gets the video stream size. Parameter is CamLibCaptureSizeType*. */
#define kCamLibCtrlStreamSizeQuery			0x55/**< Queries the video stream sizes available. Parameter is CamLibCaptureSizeType*. */
#define kCamLibCtrlStreamStart				0x56/**< Starts the video stream capture. Parameter is CamLibStreamType*. */
#define kCamLibCtrlStreamStop				0x57/**< Stops the video stream capture. Parameter is CamLibStreamType*. */
#define kCamLibCtrlStreamWaitForFrame		0x58/**< Waits and locks a video frame. Parameter is CamLibStreamType*. */
#define kCamLibCtrlStreamUnlockFrame		0x59/**< Unlocks a video frame. Parameter is CamLibStreamType*. */
#define kCamLibCtrlStreamBufNumSet			0x60/**< Sets the number of video frame buffers. Parameter is CamLibBufNumType*. */
#define kCamLibCtrlStreamBufNumGet			0x61/**< Gets the number of video frame buffers. Parameter is CamLibBufNumType*. */
#define kCamLibCtrlStreamBufNumQuery		0x62/**< Queries the number of video frame buffer range. Parameter is CamLibBufNumType*. */
#define kCamLibCtrlStreamPreviewSet			0x63/**< Sets the stream preview on or off. Parameter is CamLibStreamPreviewType*. */
#define kCamLibCtrlStreamPreviewGet			0x64/**< Gets the stream preview status (on or off). Parameter is CamLibStreamPreviewType*. */
#define kCamLibCtrlStreamPreviewQuery		0x65/**< Queries the stream preview available. Parameter is CamLibStreamPreviewType*. */
/*@}*/

/** Camera Manager library control structure */
typedef UInt16 CamLibControlType;

/**
 * @name Camera Status
 */
/*@{*/
#define kCamLibStatusPoweredOn			0x01/**< Is the camera powered on? */
#define kCamLibStatusInPreview			0x02/**< Is the camera in preview mode? */
#define kCamLibStatusInStillCapture		0x04/**< Is the camera in still capture? */
#define kCamLibStatusInVideoCapture		0x08/**< Is the camera in video capture? */
/*@}*/

/** @brief Container for the camera status kCamLibStatus* */
typedef struct CamLibStatusTag
{
	UInt32 status;	/**< Camera status */
} CamLibStatusType;

/** @brief Bitmap info structure for every format except PalmBMPV4.  Data starts after the parameters. */
typedef struct CamLibBitmapInfoTag
{
	UInt16 width;			/**< Width of the bitmap. */
	UInt16 height;			/**< Height of the bitmap. */
	UInt32 numBytes;		/**< Number or Bytes to be streamed. */
	UInt8  bitsPerPixel;	/**< Number of bits per pixel. */
	UInt8  reserved0;		/**< For packing. */
	UInt16 reserved1;		/**< For packing. */
} CamLibBitmapInfoType;

/******************************************************************************/
/* CamLibCaptureCallbackFunc */
/**
   @brief Image capture callback function pointer

   @param bufP		I/O     Pointer to the image buffer
   @param size		IN      Size of the buffer
   @param userDataP	OUT     Pointer to the user data

   @return			Error code or zero if no error occurs

*******************************************************************************/
typedef Err CamLibCaptureCallbackFunc(void *bufP, UInt32 size, void *userDataP);

/** @brief Capture settings structure.  The callback is called with userDataP passed. */
typedef struct CamLibCaptureTag
{
	CamLibCaptureCallbackFunc *callbackP;	/**< Callback function. */
	void *userDataP;						/**< User data pointer. */
} CamLibCaptureType;

/**
 * @brief Video stream structure.  Used for capturing a video stream.
 *
 * When used with WaitfForFram, bufP will contain the full frame buffer.
 */
typedef struct CamLibStreamTag
{
	void	*lock;			/**< Reserved. */
	void	*bufP;			/**< Pointer to base of frame buffer content. */
	UInt32	size;			/**< Size in bytes of frame buffer. */
	UInt32	frameTime;		/**< Time-stamp when frame was captured. */
	UInt32	frameNum;		/**< Number of frames (relative to start) in stream. */
} CamLibStreamType;


/** Video Stream Capture buffers */
#define kCamLibBufNumManual		0x01

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibBufNumType;

/**
 * @brief Settings Capabilities structure.
 * For settings without manual values, only type is used.
 */
typedef struct CamLibSettingTag
{
	UInt32 type;		/**< Setting type. */
	Int32 value;		/**< For manual settings only. */
	Int32 minValue;		/**< Used for Get/Query only and is valid for manual value. */
	Int32 maxValue;		/**< Used for Get/Query only and is valid for manual value. */
} CamLibSettingType;

/**
 * @name Capture Formats
 */
/*@{*/
#define kCamLibCaptureDataFormatRGB565				0x0002/**< Capture format in 16-bit RGB. */
#define kCamLibCaptureDataFormatYUV422				0x0004/**< Capture format in 16-bit YUV422. */
#define kCamLibCaptureDataFormatYCbCr422			0x0008/**< Capture format in 16-bit YCbCr422. */
#define kCamLibCaptureDataFormatYUV422Planar		0x0010/**< Capture format in 16-bit YUV422 Planar. */
#define kCamLibCaptureDataFormatYCbCr422Planar		0x0020/**< Capture format in 16-bit YCbCr422 Planar. */
#define kCamLibCaptureDataFormatYUV420				0x0040/**< Capture format in 16-bit YUV420. */
#define kCamLibCaptureDataFormatYCbCr420			0x0080/**< Capture format in 16-bit YCbCr420. */
#define kCamLibCaptureDataFormatYUV420Planar		0x0100/**< Capture format in 16-bit YUV420 Planar. */
#define kCamLibCaptureDataFormatYCbCr420Planar		0x0200/**< Capture format in 16-bit YCbCr420 Planar. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibCaptureFormatType;

/**
 * @name Image Resolution
 */
/*@{*/
#define kCamLibImageSizeQQVGA				0x02/**< 160 x 120 High Resolution. */
#define kCamLibImageSizeQVGA				0x04/**< 320 x 240 High Resolution. */
#define kCamLibImageSizeVGA					0x08/**< 640 x 480 High Resolution. */
#define kCamLibImageSizeSXGA				0x10/**< 1280x 960 High Resolution. */
#define kCamLibImageSizeQCIF				0x20/**< 177 x 144 High Resolution. */
#define kCamLibImageSizeCIF					0x40/**< 354 x 288 High Resolution. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibImageSizeType;

/**
 * @name White Balance Settings
 */
/*@{*/
#define kCamLibWhiteBalanceAuto				0x02/**< Automatically determines the correct white balance for capturing and previewing images.*/
#define kCamLibWhiteBalanceIndoor			0x04/**< Sets the white balance for indoor capturing and previewing of images */
#define kCamLibWhiteBalanceIncandescent		0x04/**< Sets the white balance for indoor capturing and previewing of images */
#define kCamLibWhiteBalanceOutdoor			0x08/**< Sets the white balance for outdoor capturing and previewing of images */
#define kCamLibWhiteBalanceSunny			0x08/**< Sets the white balance for outdoor capturing and previewing of images */
#define kCamLibWhiteBalanceExtra			0x10/**< Sets the white balance for flourescent indoor capturing and previewing of images */
#define kCamLibWhiteBalanceFluorescent		0x10/**< Sets the white balance for flourescent indoor capturing and previewing of images */
#define kCamLibWhiteBalanceCloudy			0x20/**< Sets the white balance for cloudy lighting during capture and preview of images */
#define kCamLibWhiteBalanceShade			0x40/**< Sets the white balance for shady lighting during capture and preview of images */
#define kCamLibWhiteBalanceTwilight			0x80/**< Sets the white balance for twilight lighting during capture and preview of images */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibWhiteBalanceType;

/**
 * @name Exposure Level Settings
 */
/*@{*/
#define kCamLibExposureManual			0x01/**<Sets the exposure level manually through an additional
                                                 passed parameter with kCamLibCtrlExposureSet. For manual
                                                 values, all values are x1000 of real exposure i.e. exp 1.5 is 1500. */
#define kCamLibExposureAuto				0x02/**<Sets the exposure level automatically.*/
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibExposureType;

/**
 * @name Contrast Setting
 */
/*@{*/
#define kCamLibContrastManual			0x01/**< Sets the contrast value manually. Use kCamLibCtrlContrastGet
to determine the minimum and maximum values allowed and the unit of
measurement. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibContrastType;

/**
 * @name Zoom Setting
 */
/*@{*/
#define kCamLibZoomManual				0x01/**< Sets the zoom value manually.
All values here are x1000 of real zoom i.e. x2 is 2000. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibZoomType;

/**
 * @name Lighting Settings
 */
/*@{*/
#define kCamLibLightingNormal			0x02/**< The camera is set for normal light. */
#define kCamLibLightingLow				0x04/**< The camera is set for low light. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibLightingType;

/**
 * @name Special Effects
 */
/*@{*/
#define kCamLibEffectsNormal			0x02/**< No special effects */
#define kCamLibEffectsBlackWhite		0x04/**< Balck & white effect */
#define kCamLibEffectsSepia				0x08/**< Sepia effect */
#define kCamLibEffectsBluish			0x10/**< Bluish effect */
#define kCamLibEffectsNegative			0x20/**< Negative effect */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibEffectsType;

/**
 * @name Saturation Level
 */
/*@{*/
#define kCamLibSaturationManual			0x01/**< Sets the saturation level manually */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibSaturationType;

/**
 * @name Sharpness Level
 */
/*@{*/
#define kCamLibSharpnessManual			0x01/**< Sets the sharpness level manually. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibSharpnessType;

/**
 * @name Stream Preview Settings
 */
/*@{*/
#define kCamLibStreamPreviewOn			0x01/**< Stream preview on. */
#define kCamLibStreamPreviewOff			0x02/**< Stream preview off. */
/*@}*/

/** @see CamLibSettingTag */
typedef struct CamLibSettingTag CamLibStreamPreviewType;

/**
 * @name Function Traps
 */
/*@{*/
#define kCamLibTrapGetVersion		(sysLibTrapCustom + 0)
#define kCamLibTrapControl			(sysLibTrapCustom + 1)
/*@}*/

#endif  // _CAMERAMGRCOMMON_H_

