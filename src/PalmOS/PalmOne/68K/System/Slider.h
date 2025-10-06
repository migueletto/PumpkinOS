/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/

/**
 * @file 	Slider.h
 * @version 1.0
 * @date 	04/03/2002
 *
 * Header file for all Palm Devices
 * Contains constants & structs necessary to access slider saved pref.
 *
 * @author    George Puckett
 * <hr>
 */

#ifndef __SLIDER_H__
#define __SLIDER_H__


#include <PalmTypes.h> 


// Creator type for Power Panel
#define sysFileCPower						'Powr'

// notification sent to inform HAL that the preferences need to be updated
#define SLIDER_PREF_CHANGED_NOTIFY_TYPE		'spcn' 

// App preferences
#define appPrefsVersion			0
#define appPrefsID				1


/***********************************************************************
 *	Types
 ***********************************************************************/

// App Preferences
typedef struct
{
	UInt8		autoOnWhenOpened;		//	non-zero if the device should auto-on when opened
	UInt8		autoOffWhenClosed;	//	non-zero if the device should auto-off when closed
} PowerPanelAppPrefs;

#endif /* __SLIDER_H__ */
