/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file 	HsPhoneMiscTypes.h
 *
 * @brief  Header File for Phone Library API ---- NETWORK CATEGORY
 *
 * NOTES:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the SMS category.  These API calls are used to interact with the wireless network.					
 */



#ifndef _HS_PHONE_MISC_TYPES_H__
#define _HS_PHONE_MISC_TYPES_H__
#include <PalmOS.h>
#include <PalmTypes.h>
#ifndef __CORE_COMPATIBILITY_H__
#include <PalmCompatibility.h>
	/** workaround for differing header files in sdk-3.5 and sdk-internal */
	#ifndef __CORE_COMPATIBILITY_H__
	#define __CORE_COMPATIBILITY_H__
	#endif 
#endif 


/**
 *  Vibrate mode while ringing.
 **/
typedef enum
  {
    phnVibrateOff,		/**<		*/
    phnVibrateOn		/**<		*/
  }
PhnVibrateType;

#define isValidPhnVibrateType(t) ((t >= phnVibrateOff) && (t <= phnVibrateOn)) 	/**<		*/


/**
 *  Phone Connection Type
 **/
typedef enum {
  voiceConnection	= 1,	/**<		*/
  csdConnection		= 2,	/**<		*/
  gprsConnection    = 3,	/**<		*/
  oneXConnection    = 4		/**<		*/
} PhnConnectionEnum;
	
#define isValidPhnConnectionEnum(e) ((e >= voiceConnection) && (e <= oneXConnection))	/**<		*/

/**
 * 
 **/
typedef enum
{
  phnHandsetMode, 		/**< sets default mode to handset mode */
  phnHeadsetMode, 		/**< sets default mode to headdset mode */
  phnSpeakerPhoneMode, 		/**< sets default mode to speaker phone mode */
  phnCarKitMode, 		/**< set carkit mode */
  phnHandsetLidCloseMode,	/**<		*/
  phnAutoMode,			/**< automatic mode where modem internaly decides the mode */
  phnBluetoothHeadsetMode,	/**<		*/
  phnBluetoothHandsfreeMode,	/**<		*/
  phnEquipmentModeLast		/**<		*/

}PhnEquipmentMode;

#define isValidPhnEquipmentMode(m) (m <= phnEquipmentModeLast)	/**<		*/


/**
 *  Radio state
 **/
typedef struct
  {
    UInt16 version;		/**<		*/
    UInt32 size;		/**<		*/
    UInt16 maxConnections;	/**<		*/
    Boolean alsSupported; 	/**< GSM only feature. if False activeLineNumber should always be 1 */
    Int16 activeLineNumber;	/**<		*/
  }
PhnRadioStateType,* PhnRadioStatePtr;

#define kRadioStateVersion 1		/**<		*/

/** 
 *  Line State
 **/
typedef struct
  {
    UInt16 version;		/**<		*/
    UInt16 size;    		/**<		*/
    Boolean activeLine;		/**<		*/
    Boolean divertIndicator;	/**<		*/
    Boolean voiceMailIndicator;	/**<		*/
  }
PhnLineStateType, * PhnLineStatePtr;
#define kLineStateVersion 1		/**<		*/


/** 
 *  Phone Library Attributes
 **/
typedef enum
  {
    phnGsmAttrFirst = 0,			/**<		*/
    phnGsmNoFWVersionCompatibilityCheck,	/**<		*/
    phnGsmSimBookSimMaxEntries,			/**<		*/
    phnGsmSimBookSimUsedEntries,		/**<		*/
    phnGsmRadioAudioSet,			/**<		*/

    phnGsmAttrLast
  }
PhnLibAttrType, * PhnLibAttrPtr;

typedef enum 
{
    phnOprtModeNone =-1,   /**< FOR INTERNAL USE OF CM ONLY! */
    phnOprtModePwroff,    /**< phone is powering off */
    phnOprtModeOffline,   /**< phone is offline Digital*/
    phnOprtModeOfflineA, /**< phone is offline analog */
    phnOprtModeOnline,    /**< phone is online */
    phnOprtModeLPM,       /**< phone is in LPM - Low Power Mode */
    phnOprtModeReset,     /**< phone is resetting - i.e. power-cycling */

    phnOprtModeMax        

} _PhnOprtModeType;

typedef UInt8 PhnOprtModeType;

#endif // _HS_PHONE_MISC_TYPES_H__
