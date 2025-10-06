/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Camera
 *
 */

/**
 * @file	CameraLib.h
 *
 * @brief	Public 68K include file for camera support for Treo 600 devices.
 *
 * Notes:
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 */

#ifndef __CAMERALIB68K_H__
#define	__CAMERALIB68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>
#include <Common/HsCommon.h>


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/// Standard library open routine.
/// Library should always be loaded with SysLibLoad()(not SysLibFind())
/// before calling CameraLibOpen(). See note at top of file.
///
/// @param refnum:	IN: Library reference number.
/// @retval Err Error code.
Err CameraLibOpen(UInt16 refnum)
			SYS_TRAP(kCameraLibTrapOpen);

/// Closes the Camera library.
/// Library should always be unloaded with SysLibRemove() after
/// CameraLibClose() is called. See note at top of file.
///
/// @param refnum:	IN: Library reference number.
/// @retval Err Error code.
Err CameraLibClose(UInt16 refnum)
			SYS_TRAP(kCameraLibTrapClose);

/// Standard library sleep function.
///
/// @param refnum:	IN: Library reference number.
/// @retval Err Error code.
Err CameraLibSleep (UInt16 refnum)
			SYS_TRAP(kCameraLibTrapSleep);

/// Standard library wake function.
///
/// @param refnum:	IN: Library reference number.
/// @retval Err Error code.
Err CameraLibWake (UInt16 refnum)
			SYS_TRAP(kCameraLibTrapWake);


/// Returns a full image (640X480) in the specified color space.
/// This function is not really necessary, since CameraLibPreview could be used instead. This function might be deprecated in the future.
///
/// @param refnum:	IN: Library reference number.
/// @param buffer:	IN: Buffer to hold the image.
/// @param format:	IN: Format of the image you want returned.
/// @retval Err Error code.
Err CameraLibCapture (UInt16 refnum, void * buffer, UInt16 format)
			SYS_TRAP(kCameraLibTrapCameraLibCapture);

/// Returns an image using the window width and height (pixels) you specify.
/// If the width and height are less the dimensions of a full camera
/// image (640x480) a simple pixel sub-sampling algorithm is used to decimate
/// the image.
///
/// @param refnum:	IN: Library reference number.
/// @param buffer:	IN: Buffer to hold the image.
/// @param format:	IN: Format of the image you want returned.
/// @param width:	IN: The width in pixels of the image you want captured.
/// @param height:	IN: The height in pixels of the image you want captured.
/// @retval Err Error code.
Err CameraLibPreview (UInt16 refnum, void * buffer, UInt16 format, UInt32 width, UInt32 height)
			SYS_TRAP(kCameraLibTrapCameraLibPreview);

/// Puts the camera library in preview mode and establishes a preview window.
/// While in this mode, QQVGA images are continuously copied to the LCD
/// screen of the device. Call CameraLibPreviewStop to exit preview mode.
///
/// @param refnum:	IN: Library reference number.
/// @param windowH:	IN: Window handle of the PalmOS window that the image
/// 					is to be displayed in.
/// @param x:		IN: Window-relative coordinate of the left edge of the image.
/// @param y:		IN: Window-relative coordinate of the top edge of the image.
/// @retval Err Error code.
Err CameraLibPreviewStart (UInt16 refnum, WinHandle windowH, UInt32 x, UInt32 y)
			SYS_TRAP(kCameraLibTrapCameraLibPreviewStart);

/// Stops preview mode.
/// @see CameraLibPreviewStart.
///
/// @param refnum:	IN: Library reference number.
/// @retval Err Error code.
Err CameraLibPreviewStop (UInt16 refnum)
			SYS_TRAP(kCameraLibTrapCameraLibPreviewStop);

/// Gets the current setting of the camera register.
///
/// @param refnum:	IN: Library reference number.
/// @param reg:		IN: Register number.
/// @param valueP:	OUT: Pointer to where register data will be stored.
/// @retval Err Error code.
Err CameraLibRegisterGet (UInt16 refnum, UInt32 reg, UInt32 * valueP)
			SYS_TRAP(kCameraLibTrapCameraLibRegisterGet);

/// Sets the value of the camera register.
///
/// @param refnum:	IN: Library reference number.
/// @param reg:		IN: Register number.
/// @param value:	IN: The data value you want to set the register to.
/// @retval Err Error code.
Err CameraLibRegisterSet (UInt16 refnum, UInt32 reg, UInt32 value)
			SYS_TRAP(kCameraLibTrapCameraLibRegisterSet);


#ifdef __cplusplus
}
#endif

#endif 	//__CAMERALIB68K_H__
