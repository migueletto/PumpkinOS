/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneSecurity.h
 *
 * @brief  Header File for Phone Library API ---- SECURITY CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the public portion of the Security category.  These API calls are used 
 * 	to protect access to device and phone features.
 *			
 */


#ifndef HS_PHONESECURITY_H
#define HS_PHONESECURITY_H
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 
#include <Common/Libraries/Telephony/HsPhoneTraps.h>     /**< trap table definition for phone library calls */
#include <Common/Libraries/Telephony/HsPhoneErrors.h>    /**< error codes returned by phone library functions */
#include <Common/Libraries/Telephony/HsPhoneTypes.h>

/**
 *  @brief 
 * 
 *  @param refNum: 	IN:
 *  @param lock:	IN:
 *  @param autoLockCheckBoxState:	IN:
 *  @retval Err Error code.
 **/
extern Err PhnLibGetPhoneLock (UInt16 refNum, Boolean * lock, Boolean * autoLockCheckBoxState)
  PHN_LIB_TRAP(PhnLibTrapGetPhoneLock);


#endif
