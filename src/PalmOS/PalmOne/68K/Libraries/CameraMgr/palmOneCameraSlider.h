/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Camera
 *
 *
 */

/**
 * @file 	palmOneCameraSlider.h
 * @version 1.0
 * @brief Public 68K file for camera slider notification for Zire 71 devices.
 *
 * Contains the Slider notification and status information.
 *
 * Any application can register to the notification to be nofity
 * when the slider is opened or closed.
 * Any application can also get the status of the slider by checking
 * the feature.
 * This is specific to the camera slider.
 * The notifyDetailsP param in SysNotifyParamType is null in the notification.
 * Application should check the status of the slider by using the Feature.
 *
 */

#ifndef __PalmCameraSlider_H__
#define __PalmCameraSlider_H__

// Palm OS common definitions
#include <PalmTypes.h>
#include <SystemMgr.h>

/***********************************************************************
 * Slider notification and status
 ***********************************************************************/

/** Version information for the camera slider. */
#define kCamSliderVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageDevelopment, 0)

/** Version information for the camera slider. */
#define kCamSliderVersion	kCamSliderVersion1

/** Feature for testing if the slider is open. */
#define kCamSliderFlagsOpened			0x00000001

/** Camera Slider creator ID. */
#define kCamSliderCreator		'cslP'

/** Notification creator. */
#define kCamSliderNotifyEvent	kCamSliderCreator

/** Feature value should be kCamSliderVersion. */
#define kCamSliderFtrVersionNum	0

/** Feature value should be kCamSliderFlagsOpened or 0. */
#define kCamSliderFtrFlagsNum	1

#endif // __PalmCameraSlider_H__
