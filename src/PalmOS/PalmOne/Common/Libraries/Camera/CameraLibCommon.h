/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 *@ingroup Camera
 */

/**
 * @file	CameraLibCommon.h
 * @brief	Public 68K common header file for camera support for Treo 600 devices.
 *
 * This file contains the library constants and error codes used in the APIs.
 * <hr>
 */

#ifndef CAMERA_LIB_COMMON_H
#define CAMERA_LIB_COMMON_H

#define kCameraLibType		sysFileTLibrary	/**< Default library type */
#define kCameraLibCreator	'HsCa'			/**< Camera library creator ID */
#define kCameraLibName		"CameraLib"		/**< Camera library database name */


/**
 * @name Image Format
 */
/*@{*/
#define kCameraImageFormatRGB1		  1 /**<Whatever RGB1 Format is*/
#define kCameraImageFormatRGB4		  2 /**<Whatever RGB4 Format is*/
#define kCameraImageFormatRGB8		  3 /**<Whatever RGB8 Format is*/
#define kCameraImageFormatRGB16		  4 /**<Whatever RGB16 Format is*/
#define kCameraImageFormatRGB24		  5 /**<Whatever RGB24 Format is*/
#define kCameraImageFormatRGB32		  6 /**<Whatever RGB32 Format is*/
#define kCameraImageFormatYCrCb		  7 /**<Whatever RGBYCrCb Format is*/
#define kCameraImageLittleEndian	  0x8000 /**<Whatever LittleEndian Format is*/
#define kCameraImageFormatYCbCr		  kCameraImageFormatYCrCb /**< not used */
/*@}*/

/**
 * @name Error Codes
 */
/*@{*/
#define errCam						  0xaf00
#define	errCamNoFrame 				  (errCam + 0)
#define errInPreviewMode			  (errCam + 1)
#define errCamOverflow				  (errCam + 2)
#define errInvalidAddress			  (errCam + 3)
#define errCamWriteFailed			  (errCam + 4)
/*@}*/

/**
 * @name Function Traps
 */
/*@{*/
#define kCameraLibTrapOpen					sysLibTrapOpen
#define kCameraLibTrapClose					sysLibTrapClose
#define kCameraLibTrapSleep					sysLibTrapSleep
#define kCameraLibTrapWake					sysLibTrapWake
#define kCameraLibTrapCameraLibCapture		(sysLibTrapCustom)
#define kCameraLibTrapCameraLibPreview		(sysLibTrapCustom+1)
#define kCameraLibTrapCameraLibPreviewStart	(sysLibTrapCustom+2)
#define kCameraLibTrapCameraLibPreviewStop	(sysLibTrapCustom+3)
#define kCameraLibTrapCameraLibRegisterGet	(sysLibTrapCustom+4)
#define kCameraLibTrapCameraLibRegisterSet	(sysLibTrapCustom+5)
/*@}*/

#endif
