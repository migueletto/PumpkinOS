/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneIOTA.h
 *
 * @brief  Header File for Phone Library API ---- IOTA CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 *	This API is broken up into various categories for easier management.  This file
 * 	defines the IOTA category.  These API calls are used for features specific to
 * 	IOTA operation on CDMA networks. 							
 */


#ifndef HS_PHONEIOTA_H
#define HS_PHONEIOTA_H
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
  
#define isValidPhnIOTAStatus(i) ((i >= phnIOTAUnknown) && (i <= phnIOTANAMSelect)) 	/**< No definition. */

#endif
