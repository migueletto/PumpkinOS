/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */


/**
 *
 * @file 	HsPhoneTypes.h
 *
 * @brief  Telephony data types that are common between ARM and 68K
 *
 */


#ifndef __HS_PHONE_TYPES_H__

#define __HS_PHONE_TYPES_H__

#include <PalmTypes.h>
#include <PalmCompatibility.h>  /** For VoidHand */

/******************************************************************************
 * Defines
 *****************************************************************************/
/**
 *  @name Type and creator of CDMA and GSM phone libraries.
 *
 **/
/*@{*/
#define phnLibDbType			'libr'       		/**< Phone Library type  */

#define phnLibCDMADbCreator       'PIL!'			/**< CDMA Library creator  */
#define phnLibCDMAName			  "Phone Library"       /**< CDMA Library name  */

#define phnLibOldCDMADbCreator	  hsFileCCDMAPhoneLib      	/**< Old CDMA Library creator  */
#define phnLibOldCDMAName		  "PhoneLib.prc"       	/**< Old CDMA Library name  */


#define phnLibGSMDbCreator		hsFileCGSMPhoneLib             /**< GSM Library creator  */
#define phnLibGSMDbName			"Phone Library"                /**< GSM Library database name  */
#define phnLibGSMName				"GSMLibrary.lib"        /**< GSM Library name  */


#define phnVoiceCall1Active	  0x0001 /**< There is a voice call active on line1  */
#define phnVoiceCall2Active	  0x0002 /**< There is a voice call active on line2 */
#define phnCSDCallActive	  0x0004 /**< There is a data call currently active */
										 // Note: Actually the virtual modem has
										 //	control but does not neccessarily
										 //	have an active data call
#define phnGPRSCallActive	  0x0008 /**< There is a GPRS (GSM) session active  */
#define phnPacketDataCallActive   0x0008 /**< There is an active 1X (CDMA) call  */
#define phnPacketDataCallDormant  0x0010 /**< There is a dormany 1X (CDMA) call  */

#define phnCall1StatusActive      0x0020 /**< The first  call's status is phnConnectionActive */
#define phnCall2StatusActive      0x0040 /**< The second call's status is phnConnectionActive */
#define phnCall1StatusDialing     0x0080 /**< The first  call's status is phnConnectionDialing */
#define phnCall2StatusDialing     0x0100 /**< The second call's status is phnConnectionDialing */
#define phnCall1StatusIncoming    0x0200 /**< The first  call's status is phnConnectionIncoming */
#define phnCall2StatusIncoming    0x0400 /**< The second call's status is phnConnectionIncoming */
#define phnCall1StatusAlerting    0x0800 /**< The first  call's status is phnConnectionIncoming */
#define phnCall2StatusAlerting    0x1000 /**< The second call's status is phnConnectionIncoming */
/*@}*/

/**
 *  @name CDMA Threshold level for signal bar (based on Generic Handset requirement)
 *
 **/
/*@{*/
#define maxDBm4signalLevel0     105	/**<  will display no bar, mid percent = 10 */
#define maxDBm4signalLevel1     95	/**<  will display 1 bar, mid percent = 30 */
#define maxDBm4signalLevel2     85	/**<  will display 2 bars, mid percent = 50 */
#define maxDBm4signalLevel3     75	/**<  will display 3 bars, mid percent = 70 */
/*@}*/


/**
 *  if below maxDBm4signalLevel3 then display 4 bars, mid percent = 90
 *  Threshold ecio level for signal bar (based on Generic Handset requirement)
 **/
/*@{*/
#define maxDB4ecioLevel0     15	/**<  will display no bar */
#define maxDB4ecioLevel1     13	/**<  will display 1 bar */
#define maxDB4ecioLevel2     11	/**<  will display 2 bars */
#define maxDB4ecioLevel3     9	/**<  will display 3 bars */
/*@}*/

/**
 *  @name Volume
 *
 **/
/*@{*/
#define phnVolumeMin		0       /**< Minimum volume level */
#define phnVolumeMax		7       /**< Maximum volume level */
/*@}*/


/**
 *  @name Enumerations and Typedefs
 *
 **/
/*@{*/
#define phnLibUnknownID		    0xff000000  /**< 		*/
#define phnLibNoLookupNeededID	0xff000001  	/**< 		*/
#define phnLibFtrNumResetFlag   10		/**< 		*/
#define phnLibFtrNumBottomBRate 11		/**< 		*/
#define phnLibFtrNumBlockATCmd  12		/**< 		*/
#define phnLibFtrValBlockATCmd  'noAT'		/**< 		*/
/*@}*/

typedef UInt16		PhnConnectionID;   	/**< Connection ID  */
typedef UInt32		PhnDatabaseID;    	 /**< Database ID  */
typedef VoidHand	PhnAddressHandle; 	 /**< 		*/
typedef VoidHand	PhnAddressList;   	 /**< 		*/
typedef UInt32		PhnOperatorID;    	 /**< 		*/


/******************************************************************************
 * Enumerations
 *****************************************************************************/
/**
 *  Classes of service for which applications can register
 **/
enum _PhoneServiceClassType
  {
    phnServiceVoice = 1,		/**< 0x0001 */
    phnServiceSMS = 2,			/**< 0x0002 */
    phnServiceActivation = 4,		/**< 0x0004 */
    phnServiceData = 8,			/**< 0x0008 */
    phnServiceIOTA = 16,		/**< 0x0010 */
	phnServiceSIMToolkit = 32, 	/**< 0x0020 */
    phnServiceAFLT = 64,		/**< 0x0040 */
    phnServiceMisc = 128,		/**< 0x0080 */
	phnServiceEssentials = 256,	/**< 0x0100 */
    phnServiceMMS = 512,       		/**< 0x200 */
    phnServiceWAP = 1024,      		/**< 0x400 */
    phnServiceAll = phnServiceVoice | phnServiceSMS | phnServiceAFLT |
                    phnServiceData | phnServiceActivation | phnServiceMisc | phnServiceIOTA |
					phnServiceEssentials | phnServiceSIMToolkit | phnServiceMMS | phnServiceWAP,
    phnServiceMax = 0xFFFF       	/**< Reserved. */
  };

typedef UInt16 PhoneServiceClassType;		/**<		*/

#define isValidPhoneServiceClassType(s) ((s & phnServiceAll) != 0) 	/**<		*/

/**
 *  Phone address fields
 **/
typedef enum {
	phnAddrFldPhone,		/**< 		*/
	phnAddrFldFirstName,		/**< 		*/
	phnAddrFldLastName		/**< 		*/
} _PhnAddressField;

#define isValidPhnAddressField(a) ((a >= phnAddrFldPhone) && (a <= phnAddrFldLastName))	/**<		*/

// make sure that size of enum is same across all compilers
typedef UInt8 PhnAddressField;		/**<		*/

/**
 *  GSM CLIR Mode
 **/
typedef enum  {
	gsmDialCLIRDefault, 			/**< 		*/
	gsmDialCLIRTemporaryInvocation, 	/**< 		*/
	gsmDialCLIRTemporarySuppression		/**< 		*/
} _GSMDialCLIRMode;

#define isValidGSMDialCLIRMode(d) ((d >= gsmDialCLIRDefault) && (d <= gsmDialCLIRTemporarySuppression))		/**<		*/

// make sure that size of enum is same across all compilers
typedef UInt8 GSMDialCLIRMode;		/**<		*/


/**
 *  Power state of the radio
 **/
enum _PhnPowerType
  {
    phnPowerOff,		/**< 		*/
    phnPowerOn,			/**< 		*/
	phnPowerStartCharging, 	/**< 		*/
	phnPowerStopCharging, 	/**< 		*/
	phnPowerLow,		/**< 		*/
    phnPowerSave                /**< Modem goes to deep sleep after failing to search for service */
  };

typedef UInt8 PhnPowerType;		/**< 		*/

#define isValidPhnPowerType(p) (p <= phnPowerSave)		/**<		*/
//#define isValidPhnPowerType(p) ((p >= phnPowerOff) && (p <= phnPowerSave))

/**
 *  Status of the SIM on GSM radio only
 **/
enum _GSMSIMStatus {
	simMissing, 		/**< 		*/
	simFailure, 		/**< 		*/
	simWrong, 		/**< 		*/
	simNotReady, 		/**< 		*/
	simReady,		/**< 		*/
	simUnknown, 		/**< 		*/
	simPresent		/**< 		*/
	// <chg 05-05-2002 TRS> bug # 13072
};

typedef UInt8 GSMSIMStatus;		/**<		*/


/**
 *  LifeTimer Enums
 **/
enum
{
  lifeTimeVoice,	/**< 		*/
  lifeTimeData		/**< 		*/
};

typedef UInt8 PhnLifeTimer;	/**< 		*/

/**
 *  Currently on the GSM side there is no way for the control channel to know
 *  that a data call has gone active. To resolove this issue there are two
 *  posibble solutions.
 *
 *  1. Have the radio send an unoslicted event accorss all channels to indicate a
 *     data call has gone active
 *  2. Have the NetLib post a notification to the GSM Library that a data call is
 *     going active.
 **/

/**
 *
 **/
typedef enum
  {
    autoLockMode,               /**< option Lock Phone on Power Down is chosen */
    immediateLockMode           /**< option Lock Phone immediately is chosen */
  }
PhnLockMode;

#define isValidPhnLockMode(m) ((m >= autoLockMode) && (m <= immediateLockMode))		/**<		*/

#endif // ifndef __HS_PHONE_TYPES_H__
