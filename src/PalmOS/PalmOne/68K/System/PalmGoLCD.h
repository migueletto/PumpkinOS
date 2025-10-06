/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 * @ingroup GoLCD
 *
 */
 
/**
 * @file 	PalmGoLCD.h
 * @version 1.1
 *
 * @brief Public include header file for the GoLCD Library.
 *
 * 	  You can use the functions in this API to enable or disable fullscreen
 * 	  writing, enable and disable the Graffiti 2 shift indicator (GSI) as a control for
 * 	  full-screen writing, enable and disable Graffiti 2 inking, and even change the colors
 * 	  of Graffiti 2 inking and the GSI.
 *
 * 	  You can check whether the GoLCD Manager is installed by examining its feature set.
 * 	  Call FtrGet as follows:
 * @code err = FtrGet (goLcdCreator, goLcdFtrNumVersion, &value); @endcode
 * 
 *
 * <hr>
 */


#ifndef __PALMGOLCD_H__
#define __PALMGOLCD_H__

#include <PalmTypes.h>
#include <LibTraps.h>

#include <Common/System/PalmGoLCDCommon.h>


/********************************************************************
 * Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens the GoLCD library.
 *   	  This function should be called prior to calling the other GoLCD functions.
 * 
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @retval Err The error code returned from the library. If this is errNone, the
 *	   function was sucessful.
 *
 * @code
 * UInt16 gGoLcdLibRefNum = 0;
 * err = SysLibFind(goLcdLibName, &gGoLcdLibRefNum);
 * if( err == sysErrLibNotFound )
 * {
 *     err = SysLibLoad(goLcdLibType, goLcdLibCreator, &gGoLcdLibRefNum);
 *         if( !err ) {
 *             err = GoLcdLibOpen(gGoLcdLibRefNum);
 *         }
 * } @endcode
 *
 * @see GoLcdLibClose
 */
Err GoLcdLibOpen(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapOpen);

/**
 * @brief Closes the GoLCD library.
 *	  This function should be called after your application has finished with the GoLCD
 * 	  library.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @retval Err The error code returned from the library. If this is errNone, the
 *	   function was sucessful.
 * 
 * @code
 * err = GoLcdLibClose(gGoLcdLibRefNum);
 * if( err == errNone )
 *     err = SysLibRemove(gGoLcdLibRefNum); @endcode
 *
 * @see GoLcdLibOpen
 */
Err GoLcdLibClose(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapClose);

/**
 * @brief Returns whether the GoLCD functionality is enabled, disabled, or not available on
 * 	  this device.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @retval GoLcdStatusType Returns the current GoLCD status.
 *
 * @see GoLcdStatusType
 * 
 * @code
 * GoLcdStatusType gCurrentGoLcdStatus = goLcdNotAvailable;
 * gCurrentGoLcdStatus = GoLcdGetStatus(gGoLcdLibRefNum);
 * switch(gCurrentGoLcdStatus) {
 *     case goLcdDisabled:
 *         // GoLcd is disabled
 *         break;
 *     case goLcdEnabled:
 *         // GoLcd is Enabled
 *         break;
 *     case goLcdNotAvailable:
 *     default:
 *         // Not available
 *         break;
 * } @endcode
 *
 * @see GoLcdSetStatus
 */
GoLcdStatusType GoLcdGetStatus(UInt16 libRefNum)
				SYS_TRAP(kGoLcdLibTrapGetStatus);

/**
 * @brief Enables or disables the GoLCD functionality.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param status	IN:  Sets the new GoLCD status.
 * @retval GoLcdStatusType Returns the previous GoLCD status.
 * 
 * @code
 * GoLcdStatusType gPreviousGoLcdStatus = goLcdNotAvailable;
 * gPreviousGoLcdStatus = GoLcdSetStatus(gGoLcdLibRefNum, goLcdEnabled); @endcode
 *
 * @see GoLcdGetStatus
 */
GoLcdStatusType GoLcdSetStatus(UInt16 libRefNum, GoLcdStatusType status)
				SYS_TRAP(kGoLcdLibTrapSetStatus);

/**
 * @brief Returns the current state of GoLCD inking.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param stateP:	IN:  Points to the returned GoLCD ink state.
 * @param colorModeP:	IN:  Points to the returned GoLCD color mode for ink.
 * @param rgbP:		IN:  Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 			     value corresponding to the current color used for inking. (The RGBColorType is
 * 			     defined in the header file Bitmap.h.)
 *
 * @code
 * GoLcdInkStateType gCurrentGoLcdInkState = goLcdInkDisabled;
 * GoLcdColorModeType gCurrentGoLcdColorMode = goLcdColorDefault;
 * RGBColorType gCurrentColor;
 * GoLcdGetInkState(gGoLcdLibRefNum, &gCurrentGoLcdInkState, &gCurrentGoLcdColorMode, &gCurrentColor); @endcode
 *
 * @see GoLcdSetInkState
 */
void GoLcdGetInkState(UInt16 libRefNum, GoLcdInkStateType *stateP, GoLcdColorModeType *colorModeP, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapGetInkState);

/**
 * @brief Sets the state of GoLCD inking.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param state:	IN:  Sets the new GoLCD ink state.
 * @param colorMode:	IN:  Sets the new GoLCD color mode for ink.
 * @param rgbP:		IN:  Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 			     value corresponding to the current color used for inking. (The RGBColorType is
 * 			     defined in the header file Bitmap.h.)
 *
 * @remarks	<em>Inking</em>, also known as <em>echoing</em>, refers to the drawing of Graffiti 2 strokes in the
 *			application area of the display.
 *
 * @code
 * RGBColorType gNewColor = { 0, 255, 0, 0}; // Red
 * GoLcdSetInkState(gGoLcdLibRefNum, goLcdInkEnabled, goLcdColorOverride, &gNewColor); @endcode
 *
 * @see GoLcdGetInkState
 */
void GoLcdSetInkState(UInt16 libRefNum, GoLcdInkStateType state, GoLcdColorModeType colorMode, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapSetInkState);

/**
 * @brief Returns the GSI state associated with GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param stateP:	IN:  Points to the returned GoLCD GSI state.
 * @param colorModeP:	IN:  Points to the returned GoLCD color mode for GSI.
 * @param rgbP:		IN:  Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 			     value corresponding to the current color used for inking. (The RGBColorType is
 * 			     defined in the header file Bitmap.h.)
 *
 * @remarks	In normal operation, the GSI is drawn in the lower-right portion of the screen when
 * 			the user enters the shift keystroke. The functionality of the GSI can be changed into
 * 			an enable and disable control for GoLCD. This function returns the current state of
 * 			the GSI.
 *
 * @code
 * GoLcdGsiStateType gCurrentGoLcdGsiState = goLcdGsiNormal;
 * GoLcdColorModeType gCurrentGoLcdColorMode = goLcdColorDefault;
 * RGBColorType gCurrentColor;
 * GoLcdGetGsiState(gGoLcdLibRefNum, &gCurrentGoLcdGsiState, &gCurrentGoLcdColorMode, &gCurrentColor); @endcode
 *
 * @see GoLcdSetGsiState
 */
void GoLcdGetGsiState(UInt16 libRefNum, GoLcdGsiStateType *stateP, GoLcdColorModeType *colorModeP, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapGetGsiState);

/**
 * @brief Sets the GSI state associated with GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param state:	IN:  Sets the GoLCD GSI state.
 * @param colorMode:	IN:  Sets the GoLCD color mode for GSI.
 * @param rgbP:		IN:  Valid only if colorMode is set to goLcdColorOverride. Points to an RGB color
 * 			     value corresponding to the current color used for inking. (The RGBColorType is
 * 			     defined in the header file Bitmap.h.)
 *
 * @remarks In normal operation, the GSI is drawn in the lower-right portion of the screen when
 * 			the user enters the shift keystroke. The functionality of the GSI can be changed into
 * 			an enable and disable control for GoLCD. This function determines whether the
 * 			GSI is converted into a GoLCD control. This setting will apply to all GSIs in any
 * 			active form.
 *
 * @code
 * RGBColorType gNewColor = { 0, 255, 0, 0}; // Red
 * GoLcdSetGsiState(gGoLcdLibRefNum, goLcdGsiOverride, goLcdColorOverride, &gNewColor); @endcode
 *
 * @see GoLcdGetGsiState
 */
void GoLcdSetGsiState(UInt16 libRefNum, GoLcdGsiStateType state, GoLcdColorModeType colorMode, RGBColorType *rgbP)
				SYS_TRAP(kGoLcdLibTrapSetGsiState);

/**
 * @brief Returns the length, in system ticks, of the time-out value of GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param mode:		IN:  Specifies the GoLCD mode.
 * @retval UInt32 Returns the length (in system ticks) of the time-out value.
 *
 * @remarks GoLCD starts in goLcdPenTapMode. When a penDownEvent is received, a timer is
 * 			started. If a penUpEvent is received before the timer reaches the time-out value for
 * 			this mode, the pen events are passed on to the event handler for the application
 * 			control. Otherwise, if the pen events exceed the time-out value and the x, y
 * 			coordinates change significantly, GoLCD enters goLcdGraffitiMode and treats the
 * 			pen events as a Graffiti 2 stroke.
 * 			<br> The default time-out value for goLcdPenTapMode is 15 system ticks, or
 * 			150 milliseconds.
 * 			<br> When it enters goLcdGraffitiMode, GoLCD continuously interprets all pen events
 * 			as Graffiti 2 strokes. There is a time-out value associated with goLcdGraffitiMode,
 * 			however. If it does not receive a new pen event within the allotted time, it returns
 * 			to goLcdPenTapMode.
 * 			<br> The default time-out value for goLcdGraffitiMode is 150 system ticks, or
 * 			1500 milliseconds.
 * 			
 * @code
 * UInt32 gCurrentTimeout = GoLcdGetTimeout(gGoLcdLibRefNum, goLcdPenTapMode); @endcode
 *
 * @see GoLcdSetTimeout
 */
UInt32 GoLcdGetTimeout(UInt16 libRefNum, GoLcdModeType mode)
				SYS_TRAP(kGoLcdLibTrapGetTimeout);

/**
 * @brief Sets the length, in system ticks, of the time-out value of GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param mode:		IN:  Specifies the GoLCD mode.
 * @param ticks:	IN:  The new time-out length in system ticks.
 *
 * @retval UInt32 Returns the length (in system ticks) of the previous time-out value.
 *
 * @remarks GoLCD starts in goLcdPenTapMode. When a penDownEvent is received, a timer is
 * 			started. If a penUpEvent is received before the timer reaches the time-out value for
 * 			this mode, the pen events are passed on to the event handler for the application
 * 			control. Otherwise, if the pen events exceed the time-out value and the x, y
 * 			coordinates change significantly, GoLCD enters goLcdGraffitiMode and treats the
 * 			pen events as a Graffiti 2 stroke.
 * 			<br> The default time-out value for goLcdPenTapMode is 15 system ticks, or
 * 			150 milliseconds.
 * 			<br> When it enters goLcdGraffitiMode, GoLCD continuously interprets all pen events
 * 			as Graffiti 2 strokes. There is a time-out value associated with goLcdGraffitiMode,
 * 			however. If it does not receive a new pen event within the allotted time, it returns
 * 			to goLcdPenTapMode.
 * 			<br> The default time-out value for goLcdGraffitiMode is 150 system ticks, or
 * 			1500 milliseconds.
 *
 * @code
 * UInt32 gPreviousTimeout = GoLcdSetTimeout(gGoLcdLibRefNum, goLcdPenTapMode, 100); @endcode
 *
 * @see GoLcdGetTimeout
 */
UInt32 GoLcdSetTimeout(UInt16 libRefNum, GoLcdModeType mode, UInt32 ticks)
				SYS_TRAP(kGoLcdLibTrapSetTimeout);

/**
 * @brief Returns the current bounds defined for use with GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param rectP:	IN:  Points to the returned rectangle.
 *
 * @remarks GoLCD ignores any succession of pen events (a series of penDownEvents followed
 * 			by a penUpEvent) if the initial penDownEvent occurs outside its bounds. By default,
 * 			its bounds are the entire display and users can start drawing full-screen writing
 * 			characters anywhere except for the right-most scroll bar and the menu bar area.
 *
 * @code
 * RectangleType gCurrentBounds;
 * GoLcdGetBounds(gGoLcdLibRefNum, &gCurrentBounds); @endcode
 *
 * @see GoLcdSetBounds
 */
void GoLcdGetBounds(UInt16 libRefNum, RectangleType *rectP)
				SYS_TRAP(kGoLcdLibTrapGetBounds);

/**
 * @brief Sets the bounds defined for use with GoLCD.
 *
 * @param libRefNum:	IN:  Reference number of the GoLCD library.
 * @param rectP:	IN:  Points to the rectangle value to set.
 *
 *
 * @remarks GoLCD ignores any succession of pen events (a series of penDownEvents followed
 * 			by a penUpEvent) if the initial penDownEvent occurs outside its bounds. By default,
 * 			its bounds are the entire display and users can start drawing full-screen writing
 * 			characters anywhere except for the right-most scroll bar and the menu bar area.
 *
 * @code
 * RectangleType gNewBounds = { { 0, 0 } , { 50, 50 } };
 * GoLcdGetBounds(gGoLcdLibRefNum, &gNewBounds); @endcode
 *
 * @see GoLcdGetBounds
 */
void GoLcdSetBounds(UInt16 libRefNum, RectangleType *rectP)
				SYS_TRAP(kGoLcdLibTrapSetBounds);

#ifdef __cplusplus 
}
#endif

#endif 	//__PALMGOLCD_H__
