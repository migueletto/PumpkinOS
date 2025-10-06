/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup LcdOverlay LCD Overlay Library
 *
 * @{
 * @}
 */

/**
 *
 * @file	palmOneLcdOverlay.h
 * @version 1.0
 *
 * @brief   LCDOverlay API can be used to display YCbCr images directly on the
 *          screen.
 */

#ifndef _PALMONELCDOVERLAY_H_
#define _PALMONELCDOVERLAY_H_

#include <PalmTypes.h>
#include <LibTraps.h>
#include <Common\Libraries\LcdOverlayLib\palmOneLcdOverlayCommon.h> // For LCD Overlay common data


/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opens LcdOverlay library.
 *
 * This function should be called prior to calling the other LcdOverlay functions.
 *
 * @param  	refNum:	IN: Reference number of the LcdOverlay library.
 * @retval  Err	The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 */
extern Err LcdOverlayLibOpen(UInt16 refNum)
	SYS_TRAP(sysLibTrapOpen);

/**
 * Closes LcdOverlay library.
 *
 *
 * @param  	refNum:	IN: Reference number of the LcdOverlay library.
 * @return	Err The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 */
extern Err LcdOverlayLibClose(UInt16 refNum)
	SYS_TRAP(sysLibTrapClose);


/**
 * Returns version of the LcdOverlay Library.
 *
 * @param  	refNum:			IN:  Reference number of the LcdOverlay library.
 * @param   sdkVersion:		IN:  Expected version number.
 * @param   libVersionP:	OUT: Pointer to version number returned.
 *
 * @retval	Err The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 */
extern Err LcdOverlayLibGetVersion(UInt16 refNum, UInt32 sdkVersion, UInt32* libVersionP)
	SYS_TRAP(kLcdOverlayLibTrapGetVersion);


/**
 * Control the setting of the LcdOverlay Library.
 *
 *
 * @param  	refNum:	  	IN:  Reference number of the LcdOverlay library.
 * @param   cmdId:     	IN:  The commandId. One of the above stated control commands.
 * @param   parameterP:	OUT: Parameter associated with a command.
 *
 * @retval	Err The error code returned from the library. If this is errNone, the
 *			function was sucessful.
 *
 */
extern Err LcdOverlayLibControl(UInt16 refNum, LcdOverlayLibControlType cmdId, void* parameterP)
	SYS_TRAP(kLcdOverlayLibTrapControl);


#ifdef __cplusplus
}
#endif


#endif  // _PALMONELCDOVERLAY_H_
