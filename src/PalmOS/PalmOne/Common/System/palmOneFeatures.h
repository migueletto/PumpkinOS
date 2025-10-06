/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *@ingroup System
 *
 */

/**
 * @file 	palmOneFeatures.h
 * @version 1.0
 * @date 	02/29/2002
 *
 * @brief Contains Palm-specific feature definitions.
 *
 * <hr>
 */

#ifndef __PALMONEFEATURES_H__
#define	__PALMONEFEATURES_H__

/**
 * The following feature should be defined on all devices that support hi-res
 * screens & include NotePad 2.0 or higher.  As the OS does not yet provide a
 * method for the desktop SW/conduits to query the handheld in order to
 * determine the density capabilities of the device, this feature has been
 * added.  By testing the presence/content of this feature, the device can
 * properly convert any old lo-density RLE data to be hi-density PNG data on
 * the first hotsync to the device rather than have to do a note-by-note
 * conversion at run time on the handheld the first time each old note has been
 * opened.
 */
#define densityFtrCreator       'dnsT'		/**< 		*/
#define densityFtrVersion       0		/**< 		*/
/**
 * one of the DensityType values defined in the Bitmap.h file should be stored
 * in this feature designating the density supported by the device.
 */

/**
 * @name
 *
 */
/*@{*/
#define screenFtrCreator		'scnP'		/**< Creator for screen feature */
#define dpiFtrValue				1	/**< Screen DPI feature */
/*@}*/

/**
 * @name Headset feature
 *
 */
/*@{*/
#define headsetFtrCreator		'hseT'		/**< 		*/
#define headsetFtrVersion		0		/**< 		*/
/*@}*/

/**
 * @name
 *
 */
/*@{*/
#define	sysExternalHeadsetInsertEvent		'hsiN'	/**< Broadcast when headset is inserted */
#define	sysExternalHeadsetRemoveEvent		'hsrM'	/**< Broadcast when headset is removed */
/*@}*/

/**
 * Hard Disk Spinup notification
 *
 * Notification sent by the hard disk slot driver that the disk has spun up
 *
 */
#define sysNotifyHardDiskSpinup   'HSPU'


/**
 * @name USB feature
 *
 * This feature provides information on the USB capabilities of the device.
 *
 */
/*@{*/
#define pmUsbFtrCreator		'pUSB'		/**< Creator for USB feature */
#define pmUsbFtrVersion			0		/**< USB feature number */
/*@}*/




#endif // __PALMONEFEATURES_H__
