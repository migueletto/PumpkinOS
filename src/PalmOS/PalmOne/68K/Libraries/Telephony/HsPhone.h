/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup Telephony Telephony Library
 *
 * @{
 * @}
 *
 * @ingroup Telephony
 */
 
 
/**
 * @file 	HsPhone.h
 *
 * @brief  Header file for the phone library (CDMA or GSM)
 *
 * Notes:
 * 	This is the generic header file that should be used by any code that
 * 	uses the CDMA or GSM phone libraries. If you add a function to this file
 * 	please makes sure you update both the CDMA and GSM libraries with this
 * 	function and update the ARM wrapper library if appropriate.							
 */



#ifndef HS_PHONE_H
#define HS_PHONE_H
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
/**< including the other type files */
#include <Common/Libraries/Telephony/HsPhoneSMSTypes.h>
#include <Common/Libraries/Telephony/HsPhoneMiscTypes.h>
#include <Common/Libraries/Telephony/HsPhoneNetworkTypes.h>
#include <Common/Libraries/Telephony/HsPhoneGSMTypes.h>
#include <Common/Libraries/Telephony/HsPhoneCDMATypes.h>
#include <Common/Libraries/Telephony/HsPhoneIOTATypes.h>
#include <Common/Libraries/Telephony/HsPhoneSecurityTypes.h>
#include <Common/Libraries/Telephony/HsPhoneCallControlTypes.h>
#include <Common/Libraries/Telephony/HsPhoneAudioTypes.h>
#include <Common/Libraries/Telephony/HsPhoneLocationTypes.h>

/**
 * Defines
 */

#include <68K/Libraries/Telephony/HsPhoneLibrary.h>         /**< CATEGORY:  LIBRARY */
#include <68K/Libraries/Telephony/HsPhoneMisc.h>            /**< CATEGORY:  MISC */
#include <68K/Libraries/Telephony/HsPhoneSMS.h>             /**< CATEGORY:  SMS */
#include <68K/Libraries/Telephony/HsPhoneNetwork.h>         /**< CATEGORY:  NETWORK */
#include <68K/Libraries/Telephony/HsPhoneGSM.h>             /**< CATEGORY:  GSM */
#include <68K/Libraries/Telephony/HsPhoneSecurity.h>        /**< CATEGORY:  SECURITY */
#include <68K/Libraries/Telephony/HsPhoneCDMA.h>            /**< CATEGORY:  CDMA */
#include <68K/Libraries/Telephony/HsPhoneIOTA.h>            /**< CATEGORY:  IOTA */
#include <68K/Libraries/Telephony/HsPhonePower.h>           /**< CATEGORY:  POWER */

#include <Common/Libraries/Telephony/HsPhoneEvent.h>           /**< CATEGORY:  NOTIFICATIONS & EVENTS */


#endif
