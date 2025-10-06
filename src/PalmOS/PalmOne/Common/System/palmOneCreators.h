/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	palmOneCreators.h
 * @version 1.0
 * @date 	02/29/2002
 *
 * @brief Contains Palm-specific creator codes.
 * 
 *
 * <hr>
 */


#ifndef __PALMONECREATORS_H__
#define __PALMONECREATORS_H__

#include <SystemResources.h>	// for classic creator code definitions

/**
 * @name 
 *
 */
/*@{*/
#define sysFileCTelMgrLibOem		'toem'	/**< Creator ID for the Telephony Manager Extension shared library. */
#define sysFileCSimATDriverOem		'sims'	/**< Creator ID for the SIM AT Driver. */
#define sysFileCSimOfficer		'sats'	/**< Creator ID for the SIM Application Toolkit client application. */
#define sysFileCSimWatcherLib		'simw'	/**< Creator ID for the SIM Watcher library. */
#define sysFileCProvisioning		'prov'	/**< Creator ID for the provisioning application. */
#define sysFileCMobileServices		'msvc'	/**< Creator ID for the Mobile Service Library. */
#define sysFileCMyFavorite		'myfv'	/**< Creator ID for the MyFavorite App. */
#define sysFileCNotePad			'npad'	/**< Creator ID for the Notepad app. */
#define sysFileCMobileApp		'mobi'  /**< Creator ID for the Mobile application */
#define kPalmCreatorIDNetSlipHack	'PNHk'	/**< Creator ID for the bluetooth slip app panel hack */
/*@}*/

/**
 * @name 
 *
 */
/*@{*/
#define kPalmOneCreatorIDContacts	'PAdd'	/**< 		*/
#define kPalmOneCreatorIDCalendar	'PDat'	/**< 		*/
#define kPalmOneCreatorIDMemos		'PMem'	/**< 		*/
#define kPalmOneCreatorIDNotePad	'npad'	/**< 		*/
#define kPalmOneCreatorIDTasks		'PTod'	/**< 		*/
#define kPalmOneCreatorIDVoiceMemo	'Vpad'	/**< 		*/
#define kPalmOneCreatorIDFavorites	'Fave'	/**< 		*/
#define kPalmOneCreatorIDFileBrowser 	'PFil'	/**< 		*/
/*@}*/

/**
 * Media types
 *
 * Media types for different media used on palmOne devices.
 *
 */
#define expMediaType_HD             'HDSK'  /**< Media type for the Hard Disk */


#endif	// __PALMONECREATORS_H__