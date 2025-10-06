/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Camera
 */

/**
 *
 * @file	palmOneCamera.h
 * @version	3.0
 *
 * @brief	Public 68K include file for camera support for Treo 650 devices.
 * 			This version of the library adds functionality for Zire72, and Treo 650.
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

#ifndef __palmOneCameraMgr_H__
#define __palmOneCameraMgr_H__

// Palm OS common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>
#include <LibTraps.h>
#include <Common/Libraries/CameraMgr/palmOneCameraCommon.h>

#define CAM_LIB_TRAP(trapNum) SYS_TRAP(trapNum)

#ifdef __cplusplus
extern "C" {
#endif

/// Standard library open routine.
///
/// Library should always be loaded with SysLibLoad()(not SysLibFind())
/// before calling CamLibOpen(). See note at top of file.
///
/// @param refNum:	IN: Library reference number.
/// @retval Err Error code.
extern Err CamLibOpen(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapOpen);

/// Closes the Camera Manager.
///
/// Library should always be unloaded with SysLibRemove() after
/// CamLibClose() is called. See note at top of file.
///
/// @param refNum:	IN: Library reference number.
/// @retval Err Error code.
extern Err CamLibClose(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapClose);

/// Standard library sleep function.
///
/// @param refNum:	IN: Library reference number.
/// @retval Err Error code.
extern Err CamLibSleep(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapSleep);

/// Standard library wake function.
///
/// @param refNum:	IN: Library reference number.
/// @retval Err Error code.
extern Err CamLibWake(UInt16 refNum)
	CAM_LIB_TRAP(sysLibTrapWake);

/// Checks the version of the Camera Manager Library.
///
/// @param refNum:		IN: Library reference number.
/// @param sdkVersion:	IN: The version the application expects.
/// @param libVersionP:	OUT: The actual version of the library.
/// @retval Err Error code.
extern Err CamLibGetVersion(UInt16 refNum, UInt32 sdkVersion, UInt32* libVersionP)
	CAM_LIB_TRAP(kCamLibTrapGetVersion);

/// Check or set a Camera property.
///
/// @param refNum:		IN: Library reference number.
/// @param cmdId:		IN: The control command used to check or set the camera property.
/// @param parameterP:	IN: Pointer to a parameter based on the control command type.
/// @retval Err Error code.
extern Err CamLibControl(UInt16 refNum, CamLibControlType cmdId, void *parameterP)
	CAM_LIB_TRAP(kCamLibTrapControl);

#ifdef __cplusplus
}
#endif

#endif // __PalmCameraLib_H__
