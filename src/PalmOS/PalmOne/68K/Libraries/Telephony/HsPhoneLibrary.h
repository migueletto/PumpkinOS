/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneLibrary.h
 *
 * @brief  Header File for Phone Library API ---- LIBRARY CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the LIBRARY category.  These API calls are used for basic Palm Library
 * 	operation.					
 */


#ifndef HS_PHONELIBRARY_H
#define HS_PHONELIBRARY_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 
#include <Common/Libraries/Telephony/HsPhoneTraps.h>     /** trap table definition for phone library calls */
#include <Common/Libraries/Telephony/HsPhoneErrors.h>    /** error codes returned by phone library functions */
#include <Common/Libraries/Telephony/HsPhoneTypes.h>


/**
 *  @brief Open the phone library. Global space has been created on SysLibLoad already. 
 *        This function just manages the openCount.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @retval Err Error code.
 **/
  extern Err      PhnLibOpen (UInt16 refNum)
                  SYS_TRAP (sysLibTrapOpen);

/**
 *  @brief Closes up a phone library. This function does not free global space. 
 *        It just manages the openCount.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @retval Err Error code.
 **/
  extern Err      PhnLibClose (UInt16 refNum)
                  SYS_TRAP (sysLibTrapClose);

/**
 *  @brief (For System Use Only)
 *
 *  @param refNum:	IN:
 *  @retval Err Error code.
 **/
  extern Err      PhnLibSleep (UInt16 refNum)
                  SYS_TRAP (sysLibTrapSleep);

/**
 *  @brief (For System Use Only)
 *
 *  @param refNum:	IN:
 *  @retval Err Error code.
 **/
  extern Err      PhnLibWake (UInt16 refNum)
                  SYS_TRAP (sysLibTrapWake);

/**
 *  @brief This function is used to indicate the version number of the current Phone Library.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @param dwVerP:	IN:  Version number
 *  @retval Err Error code.
 **/
  extern Err      PhnLibGetLibAPIVersion (UInt16 refNum, UInt32* dwVerP)
                  PHN_LIB_TRAP (PhnLibTrapGetLibAPIVersion);

/**
 *  @brief This is an unsupported function starting with Treo 600.
 *
 *  @param refNum:	IN:  Library reference number returned by HsGetPhoneLibrary().
 *  @retval Nothing
 **/
  extern void     PhnLibUninstall (UInt16 refNum)
                  PHN_LIB_TRAP (PhnLibTrapUninstall);

/**
 *  @brief Register or unregister (when services == 0) an application so that notifications
 *        from the Phone Library (Chapter 13) can be broadcasted to registered applications.
 *
 *  @param refNum:	IN:  Library reference number obtained from SysLibLoad.
 *  @param creator:	IN:  Creator ID of application registering for notifications
 *  @param services:	IN:  Bitfield of services for which notifications are desired. See PhnServiceClassType
 *                           for bit definitions.
 *                           Register with a value of 0 to unregister for notifications.
 *  @retval 0 for success; otherwise failed
 **/
  extern Err      PhnLibRegister (UInt16 refNum, DWord creator, UInt16 services)
                  PHN_LIB_TRAP (PhnLibTrapRegister);

#endif
