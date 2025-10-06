/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup LcdOverlay
 *
 */

/**
 *
 * @file	palmOneLcdOverlayCommon.h
 * @version 1.0
 * @brief   LCDOverlay API can be used to display YCbCr images directly on thee
 *          screen.
 */

#ifndef _PALMONELCDOVERLAYCOMMON_H_
#define _PALMONELCDOVERLAYCOMMON_H_

#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>

/***********************************************************************
 * Library trap
 ***********************************************************************/
/**
 * @name Function Trap Numbers
 */
/**@{*/
#define kLcdOverlayLibTrapGetVersion		(sysLibTrapCustom + 0)
#define kLcdOverlayLibTrapControl			(sysLibTrapCustom + 1)
/**@}*/
/***********************************************************************
 * Type and creator of the Library
 ***********************************************************************/

#define		kLcdOverlayLibName		"LcdOverlayManager" /**< LcdOverlay library name. */
#define		kLcdOverlayLibType		'libr'			    /**< LcdOverlay Type. */
#define		kLcdOverlayLibCreator	'Povl'			    /**< LcdOverlay Creator ID. */


/***********************************************************************
 * Library versioning
 ***********************************************************************/

#define		kLcdOverlayLibVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0) /**< LcdOverlay lib version */
#define		kLcdOverlayLibVersion	kLcdOverlayLibVersion1 /**< LcdOverlay lib version */

/***********************************************************************
 * LCD Overlay result codes
 ***********************************************************************/

/** Lcd Overlay error base number */
#define kLcdOverlayLibErrorClass			(oemErrorClass + 0x200)
/** invalid parameter */
#define kLcdOverlayLibErrBadParam			(kLcdOverlayLibErrorClass | 0x01)
/** Memory error */
#define	kLcdOverlayLibErrNoMemory			(kLcdOverlayLibErrorClass | 0x02)
/** library is not open */
#define kLcdOverlayLibErrNotOpen			(kLcdOverlayLibErrorClass | 0x03)
/** returned from LcdOverlayLibClose() if the library is still open */
#define kLcdOverlayLibErrStillOpen			(kLcdOverlayLibErrorClass | 0x04)
/** Internal error */
#define	kLcdOverlayLibErrInternal			(kLcdOverlayLibErrorClass | 0x05)
/** Unsupported function */
#define	kLcdOverlayLibErrNotSupported		(kLcdOverlayLibErrorClass | 0x06)
/** Bad Version */
#define	kLcdOverlayLibErrNotCompatible		(kLcdOverlayLibErrorClass | 0x07)

/***********************************************************************
 * Constants - control commands
 ***********************************************************************/

/** Parameter is LcdOverlayLibFormatType*. Set the format type of the overlay. */
#define kLcdOverlayLibCtrlFormatSet			0x01
/** Parameter is LcdOverlayLibFormatType*. Get  the format type of the overlay.*/
#define kLcdOverlayLibCtrlFormatGet			0x02
/** Parameter is LcdOverlayLibFormatType*. Query the type of format types supported. */
#define kLcdOverlayLibCtrlFormatQuery		0x03
/** Parameter is RectangleType* (low res coords). Set the overlay draw rectangle size*/
#define kLcdOverlayLibCtrlRectSet			0x04
/** Parameter is RectangleType* (low res coords). Get the overlay draw rectangle size*/
#define kLcdOverlayLibCtrlRectGet			0x05
/** Turn Overlay on before you do a draw. Takes no parameters */
#define kLcdOverlayLibCtrlStart				0x06
/** Draw picture using Overlay lib. void* pointer to image data */
#define kLcdOverlayLibCtrlDraw				0x07
/** Turn Overlay off usually after drawing is done. Takes no parameters */
#define kLcdOverlayLibCtrlStop				0x08

typedef UInt16 LcdOverlayLibControlType; /**< Used with the kLcdOverlayLibCtrl defines */

/***********************************************************************
 * Constants - Capabilities Settings
 ***********************************************************************/

/**
 * @brief Structure used in LcdOverlayLibControl() for setting parameters
 		  Currently only type is used to specify the format type (seelist
 		  below) when used with kLcdOverlayLibCtrlFormatSet/Get/Query.
 */

typedef struct LcdOverlayLibSettingTag
{
	UInt32 type;		 /**< Overlay (Image ) format type */
	Int32 value;		 /**< Not used currently */
	Int32 minValue;		 /**< Not used currently */
	Int32 maxValue;		 /**< Not used currently */
} LcdOverlayLibSettingType;

/** @brief Struct used in LcdOverlayLibControl() calls */
typedef struct LcdOverlayLibSettingTag		LcdOverlayLibFormatType;

/**
 * @name Data Formats
 * @brief Format types for overlay, Not all may be supported, do a query to find supported formats
 */
/**@{*/
#define kLcdOverlayLibDataFormatYCbCr444		0x02
#define kLcdOverlayLibDataFormatYCbCr422		0x04
#define kLcdOverlayLibDataFormatYCbCr420		0x08
#define kLcdOverlayLibDataFormatYCbCr444Planar	0x10
#define kLcdOverlayLibDataFormatYCbCr422Planar	0x20
#define kLcdOverlayLibDataFormatYCbCr420Planar	0x40
#define kLcdOverlayLibDataFormatRGB565			0x80 /**< This is actually RGB555 with 1 bit for transperancy, may change in future */
/**@}*/

#endif  // _PALMONELCDOVERLAYCOMMON_H_
