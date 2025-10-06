/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
 /** @ingroup Systemdef
 *
 */

/**
 * @file 	HardwareUtils68K.h
 * @version 1.0
 * @date 	 
 * 
 * @brief     public API of HardwareUtils library used by 68K applications
 */

#ifndef __HWUTILS68K_H__
#define	__HWUTILS68K_H__

#include <PalmTypes.h>
#include <LibTraps.h>

/**
 * @name 
 *
 */
/*@{*/
#define kHWUtilsType					sysFileTLibrary		/**<		*/
#define kHWUtilsCreator					'HWut'			/**<		*/
#define kHWUtilsName					"HardwareUtils"		/**<		*/
/*@}*/

/**
 * @name 
 *
 */
/*@{*/
#define kHWUtilsTrapOpen				sysLibTrapOpen		/**<		*/
#define kHWUtilsTrapClose				sysLibTrapClose		/**<		*/
/*@}*/

/**
 * @name 
 *
 */
/*@{*/
#define kHWUtilsTrapBlinkLED			(sysLibTrapCustom + 0)		/**<		*/
#define kHWUtilsTrapSetBlinkRate		(sysLibTrapCustom + 1)		/**<		*/
#define kHWUtilsTrapEnableDisplay		(sysLibTrapCustom + 2)		/**<		*/
#define	kHWUtilsTrapGetDisplayState		(sysLibTrapCustom + 3)		/**<		*/
/*@}*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief 
 *
 *  @param refnum:	IN:  
 *  @retval err error code.
 **/
Err HWUOpen(UInt16 refnum)
			SYS_TRAP(kHWUtilsTrapOpen);

/**
 *  @brief 
 *
 *  @param refnum:	IN:  
 *  @retval err error code.
 **/
Err HWUClose(UInt16 refnum)
			SYS_TRAP(kHWUtilsTrapClose);

/**
 *  @brief 
 * 
 *  @param refnum:	IN:  
 *  @param b:		IN:  
 *  @retval err error code.
 **/
Err HWUBlinkLED(UInt16 refnum, Boolean b)
			SYS_TRAP(kHWUtilsTrapBlinkLED);

/**
 *  @brief 
 *
 *  @param refnum:	IN:  
 *  @param rate:	IN:  
 *  @retval err error code.
 **/
Err HWUSetBlinkRate(UInt16 refnum, UInt16 rate)
			SYS_TRAP(kHWUtilsTrapSetBlinkRate);

/**
 *  @brief 
 *
 *  @param refnum:	IN:  
 *  @param on:		IN:  
 *  @retval err error code.
 **/
Err	HWUEnableDisplay(UInt16 refnum, Boolean on)
			SYS_TRAP(kHWUtilsTrapEnableDisplay);

/**
 *  @brief 
 *
 *  @param refnum:	IN:  
 *  @retval err error code.
 **/
Boolean HWUGetDisplayState(UInt16 refnum)
			SYS_TRAP(kHWUtilsTrapGetDisplayState);

#ifdef __cplusplus 
}
#endif

#endif 	//__HWUTILS68K_H__

