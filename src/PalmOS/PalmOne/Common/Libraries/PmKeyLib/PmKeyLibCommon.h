/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	PmKeyLib Key Library
 * @brief		This library provides support for the key-related functionalities.
 *
 * Application can use this library to detect the state of particular keys, whether
 * they are pressed or released. This library can also be used to enable/disable
 * a certain key programmatically, translate keycodes to characters, and also manage
 * the different settings of the keypad.
 *
 * @{
 * @}
 */
/**
 * @ingroup PmKeyLib
 */

/**
 * @file  PmKeyLibCommon.h
 * @brief Public 68k common header file for system library that exports key-related APIs.
 *
 * This library is based on key APIs from HsExtensions and was broken apart from
 * HsExtensions so the APIs could be ported to other platforms.
 */

#ifndef __PMKEYLIBCOMMON_H__
#define __PMKEYLIBCOMMON_H__

#include <palmOneSystemCommon.h>	// for sysMakeLibAPIVersion

/********************************************************************
 * Constants and Enums
 ********************************************************************/

#define kPmKeyLibType				sysFileTLibrary /**< standard library type */
#define kPmKeyLibCreator			pmFileCKeyLib   /**< PmKeyLib creator ID */
#define kPmKeyLibName				"PmKeyLib-PmKe"	/**< PmKeyLib database name */
#define kPmKeyLibAPIMajorVersion	2	/**< Major version incremented when API
											 changes */
#define kPmKeyLibAPIMinorVersion	0	/**< Minor version incremented with
											 each release of library that
											 doesn't change the API */ 
#define kPmKeyLibAPIVersion			sysMakeLibAPIVersion(kPmKeyLibAPIMajorVersion, kPmKeyLibAPIMinorVersion)


/**
 * @name Feature values
 */
/*@{*/
#define	kPmKeyLibFtrValAPIVersion	0	/**< Feature value used to get/set
											 library API version
											 (kPmKeyLibAPIVersion).  Note
											 that the first release of the
											 library did not set this feature.
											 Therefore, if the library does
											 exist but the feature does not,
											 you can assume the major version
											 of the library is 1.  Note also
											 that the feature is not set
											 until library is loaded so you
											 must load PmKeyLib before getting
											 the feature */
/*@}*/



// Add in once we need public attributes
enum PmKeyAttrEnumTag
{ 
  /*
   * End of enums in API version 1 (no enums defined in version 1)
   */


  pmKeyAttrPagingMode			/**< Whether currently pressed key is in page mode */
  /*
   * End of enums in API version 2
   */


  /*
   * Attributes greater than 0x9000 are defined for internal use
   * in Prv/PmPrvKeyLibCommon.h
   */
};
typedef UInt16 PmKeyAttrEnum;


/********************************************************************
 * Traps
 ********************************************************************/
/**
 * @name Function Traps
 */
/*@{*/
#define kPmKeyLibTrapOpen					  sysLibTrapOpen
#define kPmKeyLibTrapClose					  sysLibTrapClose
#define kPmKeyLibTrapKeysPressed			  (sysLibTrapCustom)
#define kPmKeyLibTrapStop					  (sysLibTrapCustom+1)
#define kPmKeyLibTrapEnableKey				  (sysLibTrapCustom+2)
#define kPmKeyLibTrapChrCodeToKeyCode		  (sysLibTrapCustom+3)
#define kPmKeyLibTrapKeyCodeToChrCode		  (sysLibTrapCustom+4)
#define kPmKeyLibTrapEventIsFromKeyboard	  (sysLibTrapCustom+5)
#define kPmKeyLibTrapAttrGet				  (sysLibTrapCustom+6)
#define kPmKeyLibTrapAttrSet				  (sysLibTrapCustom+7)
/*
 * End of traps in API version 1
 */

/*@}*/

#endif // __PMKEYLIBCOMMON_H__
