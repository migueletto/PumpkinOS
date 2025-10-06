/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**
 * @file 	HsPhoneCDMA.h
 *
 * @brief  Header File for Phone Library API ---- CDMA CATEGORY
 *
 * Notes:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the public portion of the CDMA category.  These API calls are used for features specific to
 * 	CDMA networks.								
 */


#ifndef HS_PHONECDMA_H
#define HS_PHONECDMA_H

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
 *  @brief This function returns the field’s value for a given address in a newly allocated
 * 	   block. This function returns 0 if there was an error while retrieving the data.
 *
 *  @param refNum: 	IN:
 *  @param address: 	IN:
 *  @param field: 	IN:
 *  @retval CharPtr
 **/

extern CharPtr  PhnLibAPGetField (UInt16 refNum, PhnAddressHandle address, PhnAddressField field)
  PHN_LIB_TRAP (PhnLibTrapAPGetField);

#endif  //HS_PHONECDMA_H
