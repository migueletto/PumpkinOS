/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file 	HsPhoneAudioTypes.h
 *
 * @brief  File contains Phone Audio data types.
 *
 * 					
 */


#ifndef __HS_PHONE_AUDIO_TYPES_H__

#define __HS_PHONE_AUDIO_TYPES_H__

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


/**
 *  @name Phone slider switch
 */
/*@{*/
#define kSliderLow			0 	/**<		*/
#define kSliderHigh			1	/**<		*/
#define kSliderPositions	2		/**<		*/
/*@}*/

/**
 *
 **/
typedef struct
  {
    UInt8            ringerID;		/**<		*/
    UInt16            volume;		/**<		*/
    Boolean         vibrate;		/**<		*/
    Boolean         msgAlert;		/**<		*/
    Boolean         svcAlert;		/**<		*/
  }
PhnRingingProfileType;

/**
 *
 **/
typedef enum
  {
	phnTTYSettingAll,                /**< Send/Receive Text  */
	phnTTYSettingVoiceCarryOver, 	 /**< Send Voice/ Receive Text  */
	phnTTYSettingHearCarryOver, 	 /**< Send Text / Receive Voice  */
	phnTTYSettingOff    		/**< Send/Receive Voice -- NORMAL mode  */
  }
_PhnTTYSetting;


typedef UInt8 PhnTTYSetting;		/**<		*/

/**
 * Item description here
 **/
typedef struct {
	PhnRingingProfileType profiles[kSliderPositions];	/**<		*/
} PhnRingingInfoType, * PhnRingingInfoPtr;			/**<		*/

/**
 *
 **/
typedef enum 
{
  phnAudioLocalMode,		/**<		*/
  phnAudioSilentMode,		/**<		*/
  phnAudioNetworkMode		/**<		*/
}_PhnAudioSendMode;


typedef UInt8 PhnAudioSendMode;		/**<		*/

#endif // __HS_PHONE_AUDIO_TYPES_H__
