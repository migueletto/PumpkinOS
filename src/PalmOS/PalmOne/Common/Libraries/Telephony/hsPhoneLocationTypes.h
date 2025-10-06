/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file hsPhoneLocationTypes.h
 *
 * @brief  File contains Phone Location data types.
 *				
 */


#ifndef __HS_PHONE_LOCATION_TYPES_H__

#define __HS_PHONE_LOCATION_TYPES_H__

#include <PalmTypes.h>

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

typedef UInt8 PhnPDSessionConfigParamType;


/**
 *
 **/
typedef enum
  {
    PhnPDConfigIPAddr = 1,	/**< Binary value of the Position Determination Entity (PDE) IP Address */
    PhnPDConfigPortID,		/**< Binary value of the PDE Port ID. */
    PhnPDConfigGPSLock,		/**< Level of GPS lock set in the modem. */
    PhnPDConfigPTLMMode		/**< The protocol transport layer mechanism to be used to talk to the PDE. */
  }_PhnPDSessionConfigParamType;



#endif // __HS_PHONE_LOCATION_TYPES_H__
