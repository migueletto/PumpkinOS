/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup Carrier Carrier Customization Library
 *
 * @{
 * @}
 */

/**
 * @file CarrierCustomLib.h
 * @version 1.0
 * @date 12/01/2002
 *
 * @brief This is the main header file for the Carrier Customization
 * Library.
 *
 * This includes all the other required headers, so there is no need
 * to include Traps, Errors, and Types if this is included.
 *
 * Usage Model:
 *
 * This library is automatically loaded at boot time by HsExtensions.
 * Apps that call this library should only have to call
 * SysLibFind.  They should never have to load the library.
 * LibOpen and LibClose should not be called either.
 *
 */

/*
 * @author       dla
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/68k/libraries/carriercustomlib/CarrierCustomLib.h#4 $
 *
 */

#ifndef __CARRIER_CUSTOMIZATION_LIBRARY_H__
#define __CARRIER_CUSTOMIZATION_LIBRARY_H__


#include <68K/Libraries/CarrierCustomLib/CarrierCustomLibTraps.h>
#include <68K/Libraries/CarrierCustomLib/CarrierCustomLibErr.h>
#include <68K/Libraries/CarrierCustomLib/CarrierCustomLibTypes.h>

#define ENCRYPTED_DATABASE true /**< internal */

#ifdef BUILDING_CARRIER_CUSTOMIZATION_PREF_LIB_DISPATCH_TABLE
  #define CCUSTOM_LIB_TRAP(trapNum)
#else
  #define CCUSTOM_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif

#if 0
#pragma mark -------- Constants --------
#endif

#if 0
#pragma mark -------- Structs --------
#endif


#if 0
#pragma mark -------- Exported Globals --------
#endif


#if 0
#pragma mark -------- Exported Functions --------
#endif


#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open the library, initialize library globals
 * Initializes the library globals, based on the last known carrier.
 * This should never be called by an application, as it is automatically
 * loaded by the system and opened.
 *
 * @param refNum:  IN:  CCustomLib reference number (from SysLibFind
 *                      or SysLibLoad)
 *
 * @retval Err Error Code.
 */
extern Err
CCustomLibOpen(UInt16 refNum)
	CCUSTOM_LIB_TRAP(sysLibTrapOpen);


/**
 * Closes library and frees up allocated resources
 * Close the library and decrements the reference count.
 * This should never be called by an application.
 *
 * @param refNum:  IN:  CCustomLib reference number (from SysLibFind
 *                      or SysLibLoad)
 *
 * @retval Err Error Code.
 */
extern Err
CCustomLibClose(UInt16 refNum)
	CCUSTOM_LIB_TRAP(sysLibTrapClose);


/**
 * Standard library sleep routine
 *
 * @param refNum:  IN:  CCustomLib reference number (from SysLibFind
 *                      or SysLibLoad)
 *
 * @retval Err Error code.
 */
extern Err
CCustomLibSleep(UInt16 refNum)
	CCUSTOM_LIB_TRAP(sysLibTrapSleep);


/**
 * Standard library wake routine
 *
 * @param refNum:  IN:  CCustomLib reference number (from SysLibFind
 *                      or SysLibLoad)
 *
 * @retval Err Error code.
 */
extern Err
CCustomLibWake(UInt16 refNum)
	CCUSTOM_LIB_TRAP(sysLibTrapWake);


/** Get Library API Version number.
 *
 *  @param refNum:          IN:  Library reference number
 *  @param majorVerP:       OUT: Major version number
 *  @param minorVerP:       OUT: Minor version number
 *  @param bugFixVerP:      OUT: Bug fix version number
 *  @retval Err             Customization Library error code
 */
extern Err
CCustomLibVersionGet(UInt16 refNum, UInt32* majorVerP, UInt32* minorVerP, UInt32* bugFixVerP)
  CCUSTOM_LIB_TRAP(CCustomLibTrapVersionGet);


/**
 * Returns the specified setting.
 * Returns the carrier settings for the given field.
 * Any memory allocation must be freed by the calling application.
 * Currently, only string types are allocated.
 *
 * @param refNum:        IN:     CCustomLib reference number (from SysLibFind
 *                               or SysLibLoad)
 * @param settingsType:  IN:     object type specified by settingsTypeEnum
 * @param dataP:         IN,OUT: allocated object containing data, undefined on err
 *
 * @retval Err Error code.
 */
extern Err
CCustomLibGetCarrierSetting(UInt16 refNum, CCSMSettingsType settingsType, void *dataP)
  CCUSTOM_LIB_TRAP(CCustomLibTrapGetCarrierSetting);


/**
 * Returns the carrier ID from the SIM.
 * Makes a copy of MMC/MNC code and returns it.
 *
 * @param refNum:           IN:      CCustomLib reference number (from SysLibFind
 *                                   or SysLibLoad)
 * @param length:           IN,OUT:  length of buffer passed in, length of buffer
 *                                   used/needed (not including terminator)
 * @param dataP:            OUT:     pointer to store the value
 * @retval Err Error code.
 */
extern Err
CCustomLibGetCarrierCode(UInt16 refNum, UInt8 *length, char **dataP)
  CCUSTOM_LIB_TRAP(CCustomLibTrapGetCarrierCode);


/**
 * Returns the actual length of the carrier ID.
 * If no carrier code is passed it, returns the length of the current carrier ID.
 * This length is usually 6 for North American countries and 5 for others.
 *
 * @param refNum:            IN:       CCustomLib reference number (from SysLibFind
 *                                     or SysLibLoad)
 * @param lengthP:           IN,OUT:   length of carrier code.
 *                                     Uses the current carrier if no carrier code is passed in
 * @param carrierCodeP:      IN:       optional carrier code
 * @retval Err Error code.
 */
extern Err
CCustomLibGetCarrierCodeLength(UInt16 refNum, UInt8 *lengthP, char *carrierCodeP)
  CCUSTOM_LIB_TRAP(CCustomLibTrapGetCarrierCodeLength);


#ifdef __cplusplus
}
#endif


#endif // __CARRIER_CUSTOMIZATION_LIBRARY_H__
