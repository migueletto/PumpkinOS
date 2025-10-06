/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/** 
 * @defgroup	GoLCD Full Screen Writing API
 * @brief		This library provides support for applications to enable/disable
 *				full screen graffiti programmatically.
 *
 * Applications can use the functions in this library to enable or disable fullscreen
 * 	  writing, enable and disable the Graffiti 2 shift indicator (GSI) as a control for
 * fullscreen writing, enable and disable Graffiti 2 inking, and even change the colors
 * 	  of Graffiti 2 inking and the GSI.
 *
 * Developers can check whether the GoLCD Manager is installed by examining its feature set.
 * 	  Call FtrGet as follows:
 * 	  @code err = FtrGet (goLcdCreator, goLcdFtrNumVersion, &value); @endcode
 *
 * @{
 * @}
 */
/**
 * @ingroup GoLCD
 */

/**
 * @file 	PalmGoLCDCommon.h
 * @version 1.0
 * @brief Public common header file for the GoLCD Library.
 *
 * This file contains the library constants and structures that hold different states of
 * the fullscreen feature, graffiti indicators, inking and the colors.
 * <hr>
 */

#ifndef __PALMGOLCD_COMMON_H__
#define __PALMGOLCD_COMMON_H__

#include <PalmTypes.h>


/**
 * @name Library Constants
 *
 */
/*@{*/
#define goLcdLibName          "GoLcdLib"	/**< GoLCD library name. */
#define goLcdLibType          'libr'		/**< GoLCD library type. */
#define goLcdLibCreator       'GAny'		/**< GoLCD creator ID. */
#define goLcdLibFtrNum         0		/**< GoLCd feature number. */
/*@}*/

#define goLcdCreator		'GAny'		/**< Feature creator ID */
#define goLcdFtrNumVersion 	0			/**< Current version of the supported features */

/**
 * Notification code that applications can register for when the status
 * of Full Screen Writing (GoLCD) is changed.
 */
#define goLcdNotifyStatusEvent	'GAny'


/**
 * GoLCD Status.
 * One of the following: goLcdNotAvailable, goLcdDisabled, goLcdEnabled
 */
typedef Int32 GoLcdStatusType;

/**
 * @name Feature Status
 */
/*@{*/ 
#define	goLcdNotAvailable	   -1		/**< GoLCD not available on this device. */
#define goLcdDisabled			0	/**< GoLCD functionality is disabled. */
#define goLcdEnabled			1	/**< GoLCD functionality is enabled. */
/*@}*/

/**
 * GoLCD Ink State.
 * One of the following: goLcdInkDisabled, goLcdInkEnabled
 */
typedef Int32 GoLcdInkStateType;

/**
 * @name Ink States
 */
/*@{*/ 
#define goLcdInkDisabled		0	/**< GoLCD inking is disabled. */
#define goLcdInkEnabled			1	/**< GoLCD inking is enabled. */
/*@}*/

/**
 * GoLCD Timeout Mode.
 * One of the following: goLcdGraffitiMode, goLcdPenTapMode
 */
typedef UInt32 GoLcdModeType;

/**
 * When it enters goLcdGraffitiMode, GoLCD continuously
 * interprets all pen events as Graffiti 2 strokes. There is a timeout
 * value associated with goLcdGraffitiMode, however. If it
 * does not receive a new pen event within the allotted time, it
 * returns to goLcdPenTapMode.
 * <br>
 * The default time-out value for goLcdGraffitiMode is
 * 150 system ticks, or 1500 milliseconds.
 */
#define goLcdGraffitiMode		0

/**
 * GoLCD starts in goLcdPenTapMode. When a penDownEvent is
 * received, a timer is started. If a penUpEvent is received before
 * the timer reaches the time-out value for this mode, the pen
 * events are passed on to the event-handler for the application
 * control. Otherwise, if the pen events exceed the time-out
 * value and the x, y coordinates change significantly, GoLCD
 * enters goLcdGraffitiMode and treats the pen events as a
 * Graffiti 2 stroke.
 * <br>
 * The default time-out value for goLcdPenTapMode is 15 system
 * ticks, or 150 milliseconds.
 */
#define goLcdPenTapMode			1

/** Two modes are currently supported */
#define goLcdModeCount			2


/**
 * GoLCD GSI State.
 * One of the following: goLcdGsiNormal, goLcdGsiOverride
 */
typedef UInt32 GoLcdGsiStateType;

#define goLcdGsiNormal			0 	/**< GSI is handled as normal. */
#define goLcdGsiOverride		1 	/**< GoLCD overrides the GSI with its own control. */


/**
 * GoLCD Color Mode.
 * One of the following: goLcdColorDefault, goLcdColorOverride, goLcdColorInverted
 */	
typedef UInt32 GoLcdColorModeType;

#define goLcdColorDefault		0	/**< Use the default color scheme. */
#define goLcdColorOverride		1	/**< Use the specified color (passed in as a separate argument). */
#define goLcdColorInverted		2  	/**< Use only with ink color. Inverts the color of the display area beneath the Graffiti 2 stroke. */

/**
 * @name Function Traps
 *
 */
/*@{*/
#define kGoLcdLibTrapOpen				sysLibTrapOpen
#define kGoLcdLibTrapClose				sysLibTrapClose

#define kGoLcdLibTrapGetStatus			(sysLibTrapCustom)
#define kGoLcdLibTrapSetStatus			(sysLibTrapCustom+1)
#define kGoLcdLibTrapGetInkState		(sysLibTrapCustom+2)
#define kGoLcdLibTrapSetInkState		(sysLibTrapCustom+3)
#define kGoLcdLibTrapGetTimeout			(sysLibTrapCustom+4)
#define kGoLcdLibTrapSetTimeout			(sysLibTrapCustom+5)
#define kGoLcdLibTrapGetBounds			(sysLibTrapCustom+6)
#define kGoLcdLibTrapSetBounds			(sysLibTrapCustom+7)
#define kGoLcdLibTrapGetGsiState		(sysLibTrapCustom+8)
#define kGoLcdLibTrapSetGsiState		(sysLibTrapCustom+9)
/*@}*/

/** Current library version */
#define GoLcdLibAPIVersion			(sysMakeROMVersion(3, 6, 0, sysROMStageBeta, 0))


#endif 	//__PALMGOLCD_COMMON_H__
