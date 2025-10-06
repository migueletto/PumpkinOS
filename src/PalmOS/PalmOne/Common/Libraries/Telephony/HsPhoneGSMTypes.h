/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup Telephony
 *
 */
 
 
/**	
 *
 * @file 	HsPhoneGSMTypes.h
 *
 * @brief  Header File for Phone Library API ---- NETWORK CATEGORY
 *
 * NOTES:
 * 	All implementations of the Handspring Phone Library support a common API. 
 * 	This API is broken up into various categories for easier management.  This file
 * 	defines the GSM category.  These API calls are used to interact with the wireless network.					
 */



#ifndef _HS_PHONE_GSM_TYPES_H__
#define _HS_PHONE_GSM_TYPES_H__
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
 *  @name
 *
 */
/*@{*/
#define		GSMLockSelectorOperatorLock		('PN')  /**< 		*/
#define		GSMLockSelectorProviderLock		('PP')  /**< 		*/
/*@}*/


/**
 *  @name
 *
 */
/*@{*/
//<chg 09-16-2002 TRS> moved from gsmlibrary.h
#define phnNetPrefGPRSPDPContextID  1					/**< 		*/

// <chg 09-12-2002 TRS> DUO CLEANUP - moved from gsmlibrary.h (for now)
// This is a prebuilt database.  So if this name changes, need to make
// sure that the following references to the Ring Tone database name
// are updated as well:
// 1. In the Viewer/Prebuilt directory, update the actual name
//	  of the pdb file
// 2. In the GSMLibrary Makefile, update ringTonePdbName
// 3. In GSMLibraryCommon.rcp, update the DATA 'dflt' 1 resource
// 4. In PhoneLib.h
#define GSMLibRingsDbName			"System Ring Tones"  	/**< Name of GSM RingTone DB  */

// Return if the lineNumber is unknown.  Maybe returned from the PhnLibGetRadioState
// if the activeLineNumber as not been determined yet.
#define kLineNumberUnknown 0						/**< 		*/
/*@}*/

/**
 *  
 **/
typedef struct  {
	UInt16	firstEntry;		/**< 		*/
	UInt16	lastEntry;		/**< 		*/
	UInt16	maxNameLength;		/**< 		*/
	UInt16	maxNumberLength;	/**< 		*/
} PhnPhoneBookInfoType, *PhnPhoneBookInfoPtr;



/**
 *  SIM Application Toolkit's data types
 *  type of request/respond we are sending
 **/
typedef enum {
	kSATSelectMainMenuItem,		/**< 		*/
	kSATClearText,			/**< 		*/
	kSATSetInkey,			/**< 		*/
	kSATSetInput,			/**< 		*/
	kSATMakeCall,			/**< 		*/
	kSATSelectItem = 6,		/**< 		*/
    kSATReqLaunchBrowser,		/**< 		*/
    kSATReqPlayTone,			/**< 		*/
    kSATReqRefresh,			/**< 		*/
	kSATCancel	   = 95,	/**< 		*/
	kSATUnknownNotification,	/**< 		*/
	kSATApplicationBusy,		/**< 		*/
	kSATUserNotResponse,		/**< 		*/
	kSATEndSession			/**< 		*/
} SATRequestType;


/**
 *  decision we are sending to SIM
 **/
typedef enum {
	kSATSessionAborted,		/**< 		*/
	kSATItemSelected,		/**< 		*/
	kSATHelpRequested,		/**< 		*/
	kSATNavigateBack		/**< 		*/
} SATDecisionType;


/**
 *  type of notification sent by SAT
 **/
typedef enum {
	kSATMainMenuAvailable,		/**< 		*/
	kSATDisplayText,		/**< 		*/
	kSATGetInkey,			/**< 		*/
	kSATGetInput,			/**< 		*/
	kSATSetupCall,			/**< 		*/
	kSATPlayTone,			/**< 		*/
	kSATDisplaySubmenus,		/**< 		*/
	kSATRefresh,			/**< application do not get this notification */
	kSATSendSS,			/**< 		*/
	kSATSendSMS,			/**< 		*/
	kSATSendUSSD,			/**< 		*/
    kSATLaunchBrowser,			/**< 		*/
	kSATTimeout		= 98,	/**< 		*/
	kSATSessionEnd	= 99		/**< 		*/
} SATNotificationType;

#define isValidSATNotificationType(s) (((s >= kSATMainMenuAvailable) && (s <= kSATSendUSSD)) \
              || (s == kSATTimeout) || (s == kSATSessionEnd)) 	/**< 		*/

/**
 *  
 **/
typedef enum PhnGPRSPDPType
{
  phnGPRSPDPTypeIP = 1,			/**< 		*/
  phnGPRSPDPTypePPP = 2			/**< 		*/
} PhnGPRSPDPType;

#define isValidPhnGPRSPDPType(t) ((t >= phnGPRSPDPTypeIP) && (t <= phnGPRSPDPTypePPP)) 	/**< 		*/

/**
 *  
 **/
typedef enum PhnGPRSQOSType
{
  kGPRSQOSTypeRequired = 1,		/**< 		*/
  kGPRSQOSTypeMinimum = 2		/**< 		*/
} PhnGPRSQOSType;

#define isValidPhnGPRSQOSType(t) ((t >= kGPRSQOSTypeRequired) && (t <= kGPRSQOSTypeMinimum))	/**< 		*/

/**
 *  Used when we get a list of context that are currently
 *  stored on the radio.
 **/
typedef struct PhnGPRSPDPContextType
{
  UInt16 contextId;			/**< 		*/
  PhnGPRSPDPType pdpType;		/**< 		*/
  char*	 apn;				/**< 		*/
  char*	 pdpAddr;			/**< 		*/
  UInt16 pdpDataCompression; 		/**< 0 means OFF, 1 means ON  */
  UInt16 pdpHdrCompression;		/**< 		*/
} PhnGPRSPDPContextType;

/**
 *  This is a list node that is returened when we
 *  retreive the current list of PDP contexts stored
 *  on the device
 **/
typedef struct PhnGPRSPDPContextListNodeType
{
  struct PhnGPRSPDPContextType type;		/**< 		*/
  struct PhnGPRSPDPContextListNodeType* nextP;	/**< 		*/
} PhnGPRSPDPContextListNodeType;

/**
 *  
 **/
typedef enum  {
	gsmMessagesConfirmMove, gsmMessagesCantReceive	/**< 		*/
} GSMSIMMessagesDialogKind;

#define isValidGSMSIMMessagesDialogKind(d) ((d >= gsmMessagesConfirmMove) && (d <= gsmMessagesCantReceive))	/**< 		*/

/**
 *  
 **/
enum GSMSATEventMenuFlag {
	phnSATEventMenuFlagHelp		= 1,	/**< help is available */
	phnSATEventMenuFlagNextAct	= 2	/**< nextAction is valid  */
};


/**
 *  Possible values of format for PhnSATEventGetInkey and PhnSATEventGetInput
 **/
enum GSMSATEventInputFormatEnum {
	phnSATEventInputFormatDigit		= 0,  	/**< Get Inkey or Get Input */
							/**< 0-9, #, *, or + */
	phnSATEventInputFormatSMSChar	= 1,  		/**< Get Inkey or Get Input */
							/**< SMS character set */
	phnSATEventInputFormatUCS2		= 2, 	/**< Get Inkey or Get Input */
	phnSATEventInputFormatUnpacked	= 3,  		/**< Get Input only */
	phnSATEventInputFormatPacked	= 4,  		/**< Get Input only */

	phnSATEventInputFormatYesNo		= 5,  	/**< Get Inkey or Get Input */


	
	phnSATEventInputFormatUnknown	= 255		/**< KEEP THIS ONE AT THE END */
};

/**
 *  
 **/
enum GSMATEventInputEchoModeEnum {
	phnSATEventInputEchoModeShowInput = 0,		/**< Show input as entered */
	phnSATEventInputEchoModeHideInput = 1		/**< Display bullets or boxes instead of entered characters */
};


/**
 *  @name Special response values for PhnSATEventGetInkey (besides a single character)
 *
 */
/*@{*/
#define phnSATInkeyResponseValueYes		((UInt32)'YES_')	/**< 		*/
#define phnSATInkeyResponseValueNo		((UInt32)'NO__')	/**< 		*/
/*@}*/


/**
 *  Possible values of timeUnit for PhnSATEventPlayTone
 **/
enum GSMSATEventToneTimeUnitEnum {
	phnSATEventToneTimeUnitMinutes		= 0,  	/**< Duration specified in minutes */
	phnSATEventToneTimeUnitSeconds		= 1,  	/**< Duration specified in seconds */
	phnSATEventToneTimeUnitDeciSeconds	= 2   	/**< Duration specified in 1/10 seconds */
};


/**
 *  Possible values of toneType for PhnSATEventPlayTone
 **/
enum GSMSATEventToneTypeEnum {
	phnSATEventToneTypeNoneSpecified = 0xFFFF,	/**< Means just do a normal beep... */
	phnSATEventToneTypeDial = 0,			/**< 		*/
	phnSATEventToneTypeBusy = 1,			/**< 		*/
	phnSATEventToneTypeCongestion = 2,		/**< 		*/
	phnSATEventToneTypeRadioAck = 3,		/**< 		*/
	phnSATEventToneTypeDropped = 4,			/**< 		*/
	phnSATEventToneTypeError = 5,			/**< 		*/
	phnSATEventToneTypeCallWaiting = 6,		/**< 		*/
	phnSATEventToneTypeRinging = 7,			/**< 		*/
	phnSATEventToneTypeGeneralBeep = 8,		/**< 		*/
	phnSATEventToneTypePositiveBeep = 9,		/**< 		*/
	phnSATEventToneTypeNegativeBeep = 10		/**< 		*/
};

#define isValidGSMSATEventToneTypeEnum(x) (((x >= phnSATEventToneTypeDial) && (x <= phnSATEventToneTypeNegativeBeep)) || (x == phnSATEventToneTypeNoneSpecified))	/**< 		*/


/**
 * Possible values of callType for PhnSATEventSetupCall
 **/
enum GSMSATEventSetupCallCallTypeEnum {
	phnSATEventSetupCallCallTypeVoice = 0,  /**<  SIM wants to set up a voice call  */
	phnSATEventSetupCallCallTypeData  = 1,  /**<  SIM wants to set up a data call */
	phnSATEventSetupCallCallTypeFax	  = 2   /**<  SIM wants to set up a fax  */
};

#define isValidGSMSATEventSetupCallCallTypeEnum(x) ((x >= phnSATEventSetupCallCallTypeVoice) && (x <= phnSATEventSetupCallCallTypeFax))	/**< 		*/


#endif // _HS_PHONE_GSM_TYPES_H__