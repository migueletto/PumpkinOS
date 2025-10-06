/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmLcdOverlay.h
 * @version 1.0
 * @date 	
 *
 * @brief Exported LCD Overlay functions
 * 
 *
 * <hr>
 */


#ifndef __PalmLcdOverlay_H__
#define __PalmLcdOverlay_H__

// Palm OS common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>


/**
 * @name To Define when building the library
 *
 */
/*@{*/
#if (CPU_TYPE != CPU_68K) || (defined BUILDING_OVLY_LIB)
	#define ARM_NATIVECODE			/**<		*/
	#define OVLY_LIB_TRAP(trapNum)		/**<		*/
#else
	#include <LibTraps.h>
	#define OVLY_LIB_TRAP(trapNum) SYS_TRAP(trapNum)	/**<		*/
#endif
/*@}*/


/**
 * @name Type and creator of the Library
 *
 */
/*@{*/
#define		kLcdOverlayLibType		'libr'		/**< database type */
#define		kLcdOverlayLibCreator	'Povl'			/**< database creator */
/*@}*/

/**
 * @name Internal library name which can be passed to SysLibFind()
 *
 */
/*@{*/
#define		kLcdOverlayLibName		"LcdOverlayManager"	/**<		*/
/*@}*/

/**
 * @name Library versionning
 *
 */
/*@{*/
#define		kLcdOverlayLibVersion1	sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)	/**<		*/
#define		kLcdOverlayLibVersion	kLcdOverlayLibVersion1					/**<		*/
/*@}*/		

/**
 * @name LCD Overlay result codes
 * 	  (appErrorClass is reserved for 3rd party apps/libraries.
 * 	  It is defined in SystemMgr.h)
 *
 */
/*@{*/
#define kLcdOverlayLibErrorClass	(oemErrorClass + 0x200)					/**<		*/
#define kLcdOverlayLibErrBadParam			(kLcdOverlayLibErrorClass | 0x01)	/**< invalid parameter */
#define	kLcdOverlayLibErrNoMemory			(kLcdOverlayLibErrorClass | 0x02)	/**< Memory error */
#define kLcdOverlayLibErrNotOpen			(kLcdOverlayLibErrorClass | 0x03)	/**< library is not open */	
#define kLcdOverlayLibErrStillOpen			(kLcdOverlayLibErrorClass | 0x04)	/**< returned from LcdOverlayLibClose() if the library is still open */
#define	kLcdOverlayLibErrInternal			(kLcdOverlayLibErrorClass | 0x05)	/**< Internal error */
#define	kLcdOverlayLibErrNotSupported		(kLcdOverlayLibErrorClass | 0x06)		/**< Unsupported function */
#define	kLcdOverlayLibErrNotCompatible		(kLcdOverlayLibErrorClass | 0x07)		/**< Bad Version */
/*@}*/

/***********************************************************************
 * Constants - control commands
 ***********************************************************************/
 
/**
 * @name Command definition
 *
 */
/*@{*/
#define kLcdOverlayLibCtrlFormatSet			0x01	/**< Parameter as LcdOverlayLibFormatType* */
#define kLcdOverlayLibCtrlFormatGet			0x02	/**< Parameter as LcdOverlayLibFormatType* */
#define kLcdOverlayLibCtrlFormatQuery			0x03	/**< Parameter as LcdOverlayLibFormatType* */

#define kLcdOverlayLibCtrlRectSet			0x04	/**< Parameter as RectangleType* (low res coords) */
#define kLcdOverlayLibCtrlRectGet			0x05	/**< Parameter as RectangleType* (low res coords) */

#define kLcdOverlayLibCtrlStart				0x06	/**< No parameter */
#define kLcdOverlayLibCtrlDraw				0x07	/**< Parameter as void* */
#define kLcdOverlayLibCtrlStop				0x08	/**< No parameter */
/*@}*/

typedef UInt16 LcdOverlayLibControlType;		/**<		*/

/***********************************************************************
 * Constants - Capabilities Settings
 ***********************************************************************/

/**
 * @brief Generic settings
 *  	  For settings without manual values, only type is used
 **/
typedef struct LcdOverlayLibSettingTag
{
	UInt32 type;		/**< For Manual, use constant below */
	Int32 value;		/**< For manual only */
	Int32 minValue;		/**< This value is used for Get/Query Only and is valid for Manual value */
	Int32 maxValue;		/**< This value is used for Get/Query Only and is valid for Manual value */
} LcdOverlayLibSettingType;

/**
 * @name Overlay data format
 *
 */
/*@{*/
#define kLcdOverlayLibDataFormatYCbCr444		0x02	/**<		*/
#define kLcdOverlayLibDataFormatYCbCr422		0x04	/**<		*/
#define kLcdOverlayLibDataFormatYCbCr420		0x08	/**<		*/

#define kLcdOverlayLibDataFormatYCbCr444Planar	0x10		/**<		*/
#define kLcdOverlayLibDataFormatYCbCr422Planar	0x20		/**<		*/
#define kLcdOverlayLibDataFormatYCbCr420Planar	0x40		/**<		*/

#define kLcdOverlayLibDataFormatRGB565			0x80	/**<		*/
/*@}*/

typedef struct LcdOverlayLibSettingTag		LcdOverlayLibFormatType;	/**<		*/

/**
 * @name Library trap
 *
 */
/*@{*/
#define kLcdOverlayLibTrapGetVersion		(sysLibTrapCustom + 0)		/**<		*/
#define kLcdOverlayLibTrapControl		(sysLibTrapCustom + 1)		/**<		*/
/*@}*/

/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARM_NATIVECODE

/**
 * @brief Standard library open function
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibOpen(UInt16 refNum)
	OVLY_LIB_TRAP(sysLibTrapOpen);

/**
 * @brief Standard library close function
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibClose(UInt16 refNum)
	OVLY_LIB_TRAP(sysLibTrapClose);

/**
 * @brief
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibSleep(UInt16 refNum)
	OVLY_LIB_TRAP(sysLibTrapSleep);

/**
 * @brief
 *
 * @param refNum:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibWake(UInt16 refNum)
	OVLY_LIB_TRAP(sysLibTrapWake);

/**
 * @brief Custom library API functions
 *
 * @param refNum:	IN:  
 * @param sdkVersion:	IN:  
 * @param libVersionP:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibGetVersion(UInt16 refNum, UInt32 sdkVersion, UInt32* libVersionP)
	OVLY_LIB_TRAP(kLcdOverlayLibTrapGetVersion);
	
/**
 * @brief
 *
 * @param refNum:	IN:  
 * @param cmdId:	IN:  
 * @param parameterP:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibControl(UInt16 refNum, LcdOverlayLibControlType cmdId, void* parameterP)
	OVLY_LIB_TRAP(kLcdOverlayLibTrapControl);

#else

/**
 * Prototype used when building ARM Native Library.
 **/
/**
 * @brief
 *
 * @retval Err error code.
 **/
extern Err LcdOverlayLibOpen(void);

/**
 * @brief
 *
 * @retval Err error code.
 **/
extern Err LcdOverlayLibClose(void);

/**
 * @brief
 *
 * @retval Err error code.
 **/
extern Err LcdOverlayLibSleep(void);

/**
 * @brief
 * 
 * @retval Err error code.
 **/
extern Err LcdOverlayLibWake(void);

/**
 * @brief Custom library API functions
 *
 * @param sdkVersion:	IN:  
 * @param libVersionP:	IN:  
 * @retval Err error code.
 **/
extern Err LcdOverlayLibGetVersion(UInt32 sdkVersion, UInt32* libVersionP);

/**
 * @brief
 *
 * @param cmdId:	IN:  
 * @param parameterP:	IN:  
 * @retval Err error code.
 **/	
extern Err LcdOverlayLibControl(LcdOverlayLibControlType cmdId, void* parameterP);

#endif


#ifdef __cplusplus
}
#endif

#endif // __PalmLcdOverlay_H__
