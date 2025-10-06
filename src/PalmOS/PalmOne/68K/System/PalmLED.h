/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmLED.h
 * @version 1.0
 * @date 	10/01/2003
 *
 * @brief PalmOne Generic LED dirver
 *
 * 	  Pledlib.prc, version 1.0
 * 	  This file contains the Generic LED driver defines. This driver is a standard
 * 	  libr driver that supports the BT notificaiton led. the interface follows the
 * 	  standard palm unser notification interface.
 * 
 * 	  Future drivers will support multiple Led's.
 * 
 * NOTE: Make sure to check the version number for this driver implementation.
 * 
 *
 * <hr>
 */


#ifndef __PalmLED_H__
#define __PalmLED_H__


// Palm OS common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>
#include <SystemMgr.h>


// To Define when building the library
// TODO: DO I need this???
//#if (CPU_TYPE != CPU_68K) || (defined BUILDING_CODECMGR_LIB)
//	#define CODECMGR_LIB_TRAP(trapNum)
//#else
//	#include <LibTraps.h>
//	#define CODECMGR_LIB_TRAP(trapNum) 	SYS_TRAP(trapNum)
//#endif

/**
 * @name Type and creator of the Library
 *
 */
/*@{*/
#define kPalmLedName			"PalmLed"		/**<		*/
#define kPalmLedType			sysFileTLibrary		/**<		*/
#define kPalmLedCreator			'Pled'			/**<		*/
#define kPalmLedModule			195			/**<		*/
/*@}*/

/**
 * @name Library version
 *
 */
/*@{*/
#define	kPalmLedVersion1		sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)	/**<		*/
#define	kPalmLedVersion			kPalmLedVersion1					/**<		*/
/*@}*/

/**
 * @name Standard library functions
 *
 */
/*@{*/
#define kPalmLedTrapGetAttributes	(sysLibTrapCustom+0)	/**<		*/
#define kPalmLedTrapSetAttributes	(sysLibTrapCustom+1)	/**<		*/
/*@}*/


/**
 * @name Get/Set Attribute
 *
 */
/*@{*/
#define kHALLEDPropertyUnderSleep	10
	/**
 	 * dataP points to a UInt32 that indicates how flexible the LED during Sleep is, as follows:
 	 **/
	#define kHALLEDPropertySleepOn			1		/**< bit 0 - ON during sleep */
	#define kHALLEDPropertySleepPattern		2		/**< bit 1 - pattern on */
	#define kHALLEDPropertySleepReserve		4		/**< bit 2 - reserve */
	#define kHALLEDPropertySleepColorRed		8		/**< bit 3 - red */
	#define kHALLEDPropertySleepColorGreen		0x10		/**< bit 4 - green */
	#define kHALLEDPropertySleepColorBlue		0x20		/**< bit 5 - blue */
	#define kHALLEDPropertySleepColorAmberWhite	0x40		/**< bit 6 - amber/white */
									/**< bit 7-31 reserved */

#define kHALLEDCapcityUnderSleep	11	
	/**
 	 * dataP points to a UInt32 that indicates how flexible the LED during Sleep is, as follows:
 	 **/
	#define kHALLEDCapcitySleepOn			1	/**< bit 0	ON during Sleep available */
	#define kHALLEDCapcitySleepPattern		2	/**< bit 1	pattern when sleep available. */
	#define kHALLEDCapcitySleepPatternSetting	4	/**< bit 2	pattern setting available */
	#define kHALLEDCapcitySleepMultiColor		8	/**< bit 3	multi-color available */
								/**< bit 4-31	reserved */
/*@}*/

/***********************************************************************
 * Error codes
 ***********************************************************************/

// TBD

/******************************************************************
 * Constants and Types
 ******************************************************************/

 // Iterator used by CodecSupportedFormatEnumerate


/***********************************************************************
 * Library trap
 ***********************************************************************/



/***********************************************************************
 * Generic Led driver defines
 ***********************************************************************/


/***********************************************************************
 * API Prototypes
 ***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

//#if (CPU_TYPE != CPU_68K) || (defined BUILDING_PALMLED_LIB)
#if 0
#include <Emul68K.h>



/**
 * @brief 68K Glue Code
 *
 * @param emulStateP:	IN:  
 * @param *libEntryP:	IN:  
 * @param trapValue:	IN:  
 * @retval Err error code.
 **/
Err LibDispatch68K(EmulStateRef emulStateP, LibTblEntry68KType *libEntryP, UInt32 trapValue);


#else


/**
 * @brief Standard library function
 *
 * @param refnum:	IN:   
 * @retval Err error code.
 **/
Err PalmLedOpen(UInt16 refnum)
		SYS_TRAP(sysLibTrapOpen);

/**
 * @brief Standard library function
 *
 * @param refnum:	IN:   
 * @retval Err error code.
 **/
Err PalmLedClose(UInt16 refnum)
		SYS_TRAP(sysLibTrapClose);

/**
 * @brief Standard library function
 *
 * @param refnum:	IN:   
 * @retval Err error code.
 **/
Err PalmLedSleep(UInt16 refnum)
		SYS_TRAP(sysLibTrapSleep);

/**
 * @brief Standard library function
 *
 * @param refnum:	IN:   
 * @retval Err error code.
 **/
Err PalmLedWake(UInt16 refnum)
		SYS_TRAP(sysLibTrapWake);

/**
 * @brief Standard library function
 *
 * @param refnum:	IN:   
 * @param attr:		IN:   
 * @param dataP:	IN:   
 * @retval Err error code.
 **/
Err	HALAttnGetPalmLedAttributes(UInt16 refnum, UInt32 attr, void* dataP)
		SYS_TRAP(kPalmLedTrapGetAttributes);

/**
 * @brief Standard library function
 *
 * @param refnum:	IN:  
 * @param attr:		IN:   
 * @param dataP:	IN:    
 * @retval Err error code.
 **/
Err	HALAttnSetPalmLedAttributes(UInt16 refnum, UInt32 attr, void* dataP)
		SYS_TRAP(kPalmLedTrapSetAttributes);


#endif // IF(CPU...
#endif // __PalmLED_H__
