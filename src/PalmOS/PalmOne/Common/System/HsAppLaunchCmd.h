/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *
 * @ingroup SystemDef
 *
 */
 
/**
 * @file 	HsAppLaunchCmd.h
 * @version 1.0
 * @date    04/20/2000	
 *
 * @brief Public header file akin to Palm's AppLaunchCmd.h 	
 *	
 */
 
/** 
 * @author	 dia
 *
 * History:
 *	12-oct-1999 dia - Created by Douglas Anderson
 *   20-apr-2000 dia - New APIs for dialing.
 *
 * <hr>
 */

#ifndef	  __HSAPPLAUNCHCMD_H__
#define	  __HSAPPLAUNCHCMD_H__

//#include <CoreCompatibility.h>
#include <AppLaunchCmd.h>
#include <Common/Libraries/Imaging/palmOnePhotoCommon.h>

#include <Common/Libraries/Telephony/HsPhoneTypes.h>
#include <Common/Libraries/Telephony/HsPhoneNetworkTypes.h>
#include <Common/Libraries/Telephony/HsPhoneGSMTypes.h>
#include <Common/Libraries/Telephony/HsPhoneSecurityTypes.h>
#include <Common/Libraries/Telephony/HsPhoneCDMATypes.h>
#include <Common/Libraries/Telephony/HsPhoneIOTATypes.h>
#include <Common/Libraries/Telephony/HsPhoneEvent.h>

//=============================================================================
// Useful macros...
//=============================================================================

// <chg 07-mar-00 dia>
// The LaunchWithCommand() macro was added in the OS 3.2 headers, so we
// only define this macro if we're compiling with old headers.

#ifndef LaunchWithCommand
  #define LaunchWithCommand(type, creator, command, commandParams) \
  { \
	UInt16				cardNo; \
	LocalID				dbID; \
	DmSearchStateType	searchState; \
	Err 					err; \
	DmGetNextDatabaseByTypeCreator(true, &searchState, type, \
		creator, true,	&cardNo, &dbID); \
	ErrNonFatalDisplayIf(!dbID, "Could not find db"); \
	if (dbID) \
      { \
		err = SysUIAppSwitch(cardNo, dbID, command, (Ptr) commandParams); \
		ErrNonFatalDisplayIf(err, "Could not launch db"); \
	  } \
  }
#endif // def(LaunchWithCommand)


// <chg 07-mar-00 dia>
// Unfortunately, CallWithCmmand() was never defined (I should have known 
// better).  Still, just in case Palm ever does define it, I'll couch
// it in ifdefs.  Hopefully, they follow the other macro and it will look
// just like this.

#ifndef CallWithCommand
  /** internal */
  #define CallWithCommand(type, creator, command, commandParams) \
  { \
	UInt16				cardNo; \
	LocalID				dbID; \
	DmSearchStateType	searchState; \
	UInt32 result; \
	Err err; \
	DmGetNextDatabaseByTypeCreator(true, &searchState, type, \
		creator, true,	&cardNo, &dbID); \
	ErrNonFatalDisplayIf(!dbID, "Could not find db"); \
	if (dbID) \
      { \
		err = SysAppLaunch(cardNo, dbID, 0, command, (Ptr) commandParams, &result); \
		ErrNonFatalDisplayIf(err, "Could not launch db"); \
	  } \
  }
#endif // def(CallWithCommand)


//=============================================================================
// Handspring-modified address book
//=============================================================================

//-------------------------------------------------------------------
// Extra launch codes
//-------------------------------------------------------------------

/** Launch codes start at customBase + 0x2300 so that hopefully there won't be any conflict with
  * any launch codes that PalmSource or others add to the address book.
  * IMPORTANT: See also AddrCustomNotificationCodes . */
typedef enum 
  {
	addrAppLaunchCmdHsLookup= sysAppLaunchCmdCustomBase + 0x2300,   /**< Launch Contacts for lookup */
    addrAppLaunchCmdCallerID                                        /**< Launch Contacts for callerID lookup */
  } 
AddrCustomActionCodes;

/** Switch to using notifications rather than launch codes to avoid potential
  * conflicts if PalmSource modifies the address book app. */
typedef enum
  {
		// NOTE: 
		// Shouldn't be using these values!  Notification types are
		// supposed to be 4-character creator IDs that are registered in Palm's
		// Creator database!  Guess it's a little late now, since these codes are
		// out in the field.
		addrAppNotificationHsLookup = sysAppLaunchCmdCustomBase + 0x2300,
		addrAppNotificationCmdCallerID,
		addrAppNotificationPhonebookApp,

		// DON'T ADD ANY MORE based on 2300:  See above NOTE

		addrAppNotificationListLookupTypeGet = 'HsLL',  	// Registered 2001-09-12 w/ Creator DB
		addrAppNotificationCreateNewRecord = 'HsNR', 		// Registered 2002-07-30 w/ Creator DB
		addrAppNotificationDialPrefs = 'HsDP',				// Registered 2003-01-29 w/ Creator DB
		addrAppNotificationBeamBusinessCard = 'HsBB',		// Registered 2003-02-14 w/ Creator DB
		addrAppNotificationPhotoCallerID = 'HsPC'
  }
AddrCustomNotificationCodes;

//-------------------------------------------------------------------
// Support for addrAppLaunchCmdHsLookup
//-------------------------------------------------------------------

/** Parameter block is the same as OS 3.1's block for sysAppLaunchCmdLookup */
typedef AddrLookupParamsType AddrHsLookupParamsType;

/** Parameter block is the same as OS 3.1's block for sysAppLaunchCmdLookup */
typedef AddrHsLookupParamsType * AddrHsLookupParamsPtr;

// We support all of the lookup fields that defined in the AddressLookupFields
// enum for the OS 3.1 headers, plus these two.  The string versions of these
// (for the formatstring paramter) are "voicephone" and "anyphone".
// ...start at 0x70 so hopefully we get no conflicts.

/** Expansion to the Palm OS AddressLookupFields. */
typedef enum {
	addrLookupVoicePhones=0x70,		/**< All phone fields except fax and email. */
	addrLookupAnyPhones ,			/**< All phone fields (including email) */
	addrLookupSMSPhones ,			/**< Mobile, other and email fields */
	addrLookupSMSPhonesNoEmail		/**< Mobile and other fields */
} AddressHsLookupFields;


//-------------------------------------------------------------------
// Support for addrAppLaunchCmdCallerID
//-------------------------------------------------------------------

/** Flag definitions for AddrCallerIDParamsType flags field
  * If this flag is set, we match numbers that are
  * shorter than the usual minimum required length.
  * We match the *entire* string, not substrings. */
#define addrLookupNoMinLength		1


/** Parameter block for addrAppLaunchCmdCallerID. */
typedef struct 
  {
    Char* lookupString;   /**< Must be only digits and include country code.
                               Example: 14085551212 */
    Char* formatString;   /**< Format of the output string like AddrLookupParamsPtr->formatString
                               Example ^name ( ^phonetypelabel ) might result
                               in "David ( work )" */

	Char* resultString; /**<  Returned heap allocated string resultStringH; */
	Boolean (*callback)(void*); /**< Call back; Returns true to stop execution */
    void* ref;  /**< Reference that gets passed to callbackFunc */
	UInt32 uniqueID; /**< Unique ID of the matching record 0 if no match */
	UInt16 flags; /**< Flags associated with the called ID */
	UInt16 recordNum; /**<record number of the matching record. */

  }
AddrCallerIDParamsType, * AddrCallerIDParamsPtr;


//-------------------------------------------------------------------
// Support for addrAppNotificationListLookupTypeGet
//-------------------------------------------------------------------

/** Various address book lookup methods. */
enum
  {
	addrListLookupTypeLastName,			  /**< phone lookup will be by last name... */
	addrListLookupTypeFirstName,		  /**< phone lookup will be by first name... */
	addrListLookupTypeCompany,			  /**< phone lookup will be by company (currently unused)... */
	addrListLookupTypeIntelligent		  /**< phone lookup will be intelligent (either name or
										       some other matching) (currently unused)...  */
  };
typedef UInt16 AddrListLookupTypeEnum;    /**< AddrListLookupTypeEnum -> UInt16 */

/** Contains data used when launching address book for contact lookup. */
typedef struct
  {
	UInt16					version;	  /**< IN/OUT: Should always be 0... */
	AddrListLookupTypeEnum	type;		  /**< OUT: The type of phone lookup that
										     		will occur if you do a lookup by
										  		    listName. */
  }
AddrListLookupTypeGetParamsType;


//-------------------------------------------------------------------
// Support for addrAppLaunchCmdHsPhonebook
//-------------------------------------------------------------------
/** Parameter block for addrAppLaunchCmdHsPhonebook. */
typedef struct
  {
	Char*	  lookupStrP; /**< The lookup string that should be
						       preloaded in the lookup field */
	Boolean	  activeCall; /**< Whether the active call icon should be
						       usable or not... could change in a sublaunch
						       when a call is dropped */
	Boolean   lookup;     /**< True if this notification is for a lookup
						       false if it is for a new contact */
	Char *    newPhoneNum;/**< Phone number of the new contact. */
  }
AddrPhonebookAppParamsType, * AddrPhonebookAppParamsPtr;


// <chg 2002-08-05 knm>
// This is used to send a new contact data to AddressApp. Currently
// SMSApp's "Save to Contacts" feature uses this.
//-------------------------------------------------------------------
// Support for addrAppNotificationCreateNewRecord
//-------------------------------------------------------------------

/** Contains data when launching phone book with create new record view. */
typedef struct
  {
	Char*	data;   /**< Data to save (Null terminated string) */
	UInt8	field;  /**< Phone number, URL, or email address field number.
                          *  They are defined in ViewForm.c as follows:
                          *  #define kAddrDBRecMobileFld 5   // phone3
                          *  #define kAddrDBRecEMailFld  7   // phone5
                          *  #define kAddrDBRecURLFld    14  // custom1
                          *  The values are based on the field definition in AddressDB.h. */
  }
AddrCreateNewRecordParamsType, * AddrCreateNewRecordParamsPtr;

//-------------------------------------------------------------------
// Support for addrAppNotificationHsLookup
//-------------------------------------------------------------------
#define	AddrHsLookupUserDataVersion		0 /**< Version number of the addrAppNotificationHsLookup command and structures */

/** Structure that contains the data used for looking up user info in address book app */
typedef struct
  {
	UInt16					version;		/**< IN: Version of the userData structure */
	Boolean					matchFound;		/**< OUT: TRUE if ANY match was found.
													  (i.e. 1 or more matches were
													  found)
													  FALSE if NO match was found. */
	Boolean					useMultiFilter;	/**< IN: TRUE if lookup should use multifilter.
													 FALSE if lookup should NOT use multifilter. */
  }
AddrHsLookupUserDataType, *AddrHsLookupUserDataPtr;

//-------------------------------------------------------------------
// Support for addrAppNotificationPhotoCallerID
//-------------------------------------------------------------------
/** Support for addrAppNotificationPhotoCallerID */
typedef struct
{
	UInt16		recordNum;	/**< Record for which we want to get the picture id */
	Coord		xExtent;	/**< Dimension of the photo required. Pass 0 to get max size. */
	Coord		yExtent;	/**< Dimension of the photo required. Pass 0 to get max size. */
	void		*imageBits; /**< Bits that represent the caller id image. image size returned in xExtent & yExtent */
}
AddrPhotoCallerIDParamsType, *AddrPhotoCallerIDParamsPtr;
	
//-------------------------------------------------------------------
// Support for dialPnlLaunchCmd... commands
//-------------------------------------------------------------------

// Boolean IsDialingAvailable (UInt32 serviceFlags)
// ------------------------------------------------

/** Support for dialPnlLaunchCmd... commands */
typedef struct 
  {
    Boolean result;             /**< OUT: Space for result... */
    UInt8 _reserved0;           /**< IN:  Reserved byte--MUST BE 0. */
    
    const UInt32* serviceIDsP;  /**< IN:  Array of services to check... */
    UInt16 numServices;         /**< IN:  Num elements in service IDs array. */
  }
DialPnlIsDialingAvailableParamsType;


// void DispatchDial (UInt16 service, void* paramP)
// ------------------------------------------------

/** Use with the function DispatchDial (UInt16 service, void* paramP) */
typedef struct
  {
    UInt32	serviceID;		/**< IN:  The service type to dial... */
    void*	dataP;          /**< IN:  Basic Data to send to the helper... */
    Char*	descriptionStr; /**< IN:  Description of data to dial... */
	UInt8	handled;		/**< OUT: Non-zero if handled */
    Err		err;			/**< OUT: Err code returned... */
  }
DialPnlDispatchDialParamsType;


//=============================================================================
// Handspring phone application
//=============================================================================

// DOLATER: Some of these may be obsolete now...

//------------------------------------------------------------------------
// Phone application's custom launch codes
//------------------------------------------------------------------------

/** Launch the phone application and dial the passed number */
#define phoneAppLaunchCmdDial			(sysAppLaunchCmdCustomBase)

/** Launch the phone application and dial the last number dialed. */
#define phoneAppLaunchCmdRedial			(sysAppLaunchCmdCustomBase+1)

/** Launch the phone application and display the active calls view. */
#define phoneAppLaunchCmdViewCalls		(sysAppLaunchCmdCustomBase+2)

/** Launch the phone application and prompt to power on the phone */
#define phoneAppLaunchCmdPowerOn		(sysAppLaunchCmdCustomBase+3)

/** Launch the phone application and dial a number */
#define phoneAppLaunchCmdDialVoiceMail	(sysAppLaunchCmdCustomBase+4)

/** Launch the phone application and display the call history view. */
#define phoneAppLaunchCmdViewHistory	(sysAppLaunchCmdCustomBase+5)

/** Launch the phone application and display the speed dial view. */
#define phoneAppLaunchCmdViewSpeed		(sysAppLaunchCmdCustomBase+6)

/** Launch the phone application and display the keypad view. */
#define phoneAppLaunchCmdViewKeypad		(sysAppLaunchCmdCustomBase+7)

/** Launch the phone application and display the Phone Home view */
#define phoneAppLaunchCmdViewPhoneHome	(sysAppLaunchCmdCustomBase+8)

/** Launch the ## Application */
#define phoneAppLaunchCmdActivation     (sysAppLaunchCmdCustomBase+10)

/** Set the Wallpaper */
#define phoneAppLaunchCmdSetWallpaper	(sysAppLaunchCmdCustomBase+11)

/** Start the Network-initiated USSD */
#define phoneAppLaunchCmdStartNetInitUSSD	(sysAppLaunchCmdCustomBase+12)

/** Launch the phone application and dial the passed number for SATK SETUP CALL */
#define phoneAppLaunchCmdDialSTKSETUPCALL	(sysAppLaunchCmdCustomBase+13)

/** Launch the phone application to display the UI to ask the user if they
  * would like to search again. */

#define phoneAppLaunchCmdNetworkSearch	(sysAppLaunchCmdCustomBase+14)

/** Launch the phone application and send DTMF tone to the network */
#define phoneAppLaunchCmdSendDTMFTone	(sysAppLaunchCmdCustomBase+15)

/** Launch the phone application and display missed calls in the call history view. */
#define phoneAppLaunchCmdViewMissedCalls  (sysAppLaunchCmdCustomBase+16)

/** Display Dial Prefs */
#define phoneAppLaunchCmdDisplayDialPrefs (sysAppLaunchCmdCustomBase+17)

/** Query for the carrier flag to determine if Dial Prefs menu option should be displyed */
#define phoneAppLaunchCmdGetDialPrefFlag  (sysAppLaunchCmdCustomBase+18)

/** Query for the carrier flag to determine if Dial Prefs menu option should be displyed */
#define phoneAppLaunchCmdGetUSDialingRules  (sysAppLaunchCmdCustomBase+19)

/** Launch the phone application and display the Options menu */
#define phoneAppLaunchOptionsMenu		  (sysAppLaunchCmdCustomBase+20)

/** Launch the phone application and display the Options menu */
#define phoneAppLaunchCmdGetALSLineNumber	  (sysAppLaunchCmdCustomBase+21)

// Launch the phone application if dialed from bt handsfree 
#define phoneAppLaunchCmdBluetoothDial (sysAppLaunchCmdCustomBase+22)

// Launch the phone application if dialed from bt handsfree 
#define phoneAppLaunchCmdBluetoothDialVoiceMail (sysAppLaunchCmdCustomBase+23)


//------------------------------------------------------------------------
// Data send this request to dial a number (phoneAppLaunchCmdDial).
//------------------------------------------------------------------------

/** Phone application dial methods. */
typedef enum
  {
	PhoneAppDialMethodNormal = 0,	  /**< Normal PhnLibDial() */
	PhoneAppDialMethodSATSetupCall	  /**< Use SIM Toolkit and respond "affirmative"
									       to the SIM's request to set up a call. */
  }
PhoneAppDialMethodEnum;

/** Contains the data used by Phone app launch command dial. */
typedef struct 
	{
	UInt8					version;			/**< version number, set to one */
	Boolean					confirm;			/**< confirm before dialing. */
	Char*					name;				/**< optional */
	Char*					number;				/**< optional */
	Char*					extraDigits;		/**< optional */
	// Version 1
	UInt32					failLaunchCreator;	/**< app to relaunch if dialling fails */
	UInt16					failLaunchCode;		/**< launch code to use when relaunching */
	void*					failLaunchParams;	/**< params to use when relaunching */
	UInt16					dialMethod;			/**< Method to use to dial the phone number
													 See PhoneAppDialMethodEnum. */
	}
PhoneAppLaunchCmdDialType, * PhoneAppLaunchCmdDialPtr;	

/** Contains data used by PhoneAppLaunchCmdSetWallpaper. */
typedef struct
  {
	UInt32					wallpaperUID;		/**< OBSOLETE */
	// Version 2
	PalmPhotoFileLocation	wallpaperLocation;	/**< path to image */
  } 
PhoneAppLaunchCmdSetWallpaperType, *PhoneAppLaunchCmdSetWallpaperPtr;

/** Contains data used by PhoneAppLaunchCmdSendDTMFTone. */
typedef struct
  {
	char				tone;		/**< the key that was pressed to trigger the launch cmd */
	Boolean				repeated;	/**< true if the character was on autoRepeat */
  }
PhoneAppLaunchCmdSendDTMFToneType, *PhoneAppLaunchCmdSendDTMFTonePtr;

/** Contains data used by PhoneAppLaunchCmdGetDialPref */
typedef struct
{
  Boolean				displayDialPrefs; /**< If true Dial Preferences option is made available */
}
PhoneAppLaunchCmdGetDialPrefFlagType, *PhoneAppLaunchCmdGetDialPrefFlagPtr;

/** Data structure that goes with phoneAppLaunchCmdGetALSLineNumber launch cmd */
typedef struct
{
  UInt8		lineNumber; /**<ALS Line number */
}
PhoneAppLaunchCmdGetALSLineNumberType, *PhoneAppLaunchCmdGetALSLineNumberTypePtr;

/** Data structure that goes with phoneAppLaunchCmdGetUSDialingRules launch cmd */
enum
  {
	hsDialPrefsAddAreaCodeUnknown = 0,	  /**< we don't know yet, so prompt when appropriate */
	hsDialPrefsAddAreaCodeNo,			  /**< no auto-area code */
	hsDialPrefsAddAreaCodeYes,			  /**< add area code automatically before dialing */

	hsDialPrefsAdd4DigitPrefixUnknown, //For 6 digit dialing
	hsDialPrefsAdd4DigitPrefixNo,
	hsDialPrefsAdd4DigitPrefixYes,
	
	hsDialPrefsAdd5DigitPrefixUnknown,
	hsDialPrefsAdd5DigitPrefixNo,
	hsDialPrefsAdd5DigitPrefixYes,
	
	hsDialPrefsAdd6DigitPrefixUnknown,
	hsDialPrefsAdd6DigitPrefixNo,
	hsDialPrefsAdd6DigitPrefixYes
  };

/** MAX Length of the dialing prefixes for 7,6,5 and 4 digit numbers
 * respectively, including zero-terminator. */
#define hsDialPrefsAutoAreaCodeLength	  11
#define hsDialPrefs6DigitPrefixLength   11
#define hsDialPrefs5DigitPrefixLength   11
#define hsDialPrefs4DigitPrefixLength   11

#define hsDialPrefsMaxDialingPrefixLength 11

/** HsDialPrefsUSDialingRulesType */
typedef struct
  {
	UInt8  rulesEnabled;		/**< [OUT] if non-zero, then these should be
								           be applied; if zero -- don't apply. */
	UInt8  _reserved0;			/**< caller must set to 0 (zero) -- for byte alignment, and reserved for
								     future use by the DialPrefs API */

	UInt8  areaCodeCtl;			/**< [OUT] one of hsDialPrefsAddAreaCodeUnknown,
								           hsDialPrefsAddAreaCodeNo, or
								           hsDialPrefsAddAreaCodeYes;
								           default is hsDialPrefsAddAreaCodeUnknown */

	UInt8  add1BeforeAreaCode;	/**< [OUT] if non-zero, automatically add "1" in front
								           of the area code of US/Canada numbers;
								           if zero -- don't.  Default is to add the "1"
								           in front of US/Canada area codes. */

	char  autoAreaCode[hsDialPrefsAutoAreaCodeLength];
								/**< [OUT] the "auto" area code digits to add to 7-digit
								           numbers if areaCodeCtl is hsDialPrefsAddAreaCodeYes; ignore otherwise; */

	UInt8  prefix6DigitCtl; //Dialing prefix for 6-digit dialed numbers
													// one of hsDialPrefsAdd6DigitPrefixUnknown,
													//hsDialPrefsAdd6DigitPrefixNo,
													//hsDialPrefsAdd6DigitPrefixYes
	UInt8  prefix5DigitCtl; //Dialing prefix for 5-digit dialed numbers
													// one of hsDialPrefsAdd5DigitPrefixUnknown,
													//hsDialPrefsAdd5DigitPrefixNo,
													//hsDialPrefsAdd5DigitPrefixYes
	UInt8  prefix4DigitCtl; //Dialing prefix for 4-digit dialed numbers
													// one of hsDialPrefsAdd4DigitPrefixUnknown,
													//hsDialPrefsAdd4DigitPrefixNo,
													//hsDialPrefsAdd4DigitPrefixYes

	
	char auto6DigitPrefix[hsDialPrefs6DigitPrefixLength];
	char auto5DigitPrefix[hsDialPrefs5DigitPrefixLength];
	char auto4DigitPrefix[hsDialPrefs4DigitPrefixLength];
  }
HsDialPrefsUSDialingRulesType;

//------------------------------------------------------------------------
// Phone application's custom notification codes
//------------------------------------------------------------------------

/** This notification is broadcast by the Phone application */
#define pmPhoneAppNotifyHomeAppEvent		'pEvt'	  // Registered with PalmSource Creator ID DB
 
/** The parameter for pmPhoneAppNotifyHomeAppEvent notification is 
  * of type PhnEventPtr(as defined in HsPhoneEvent.h */
#define pmPhoneAppNotifySetWallpaper		'pWlp'	  // Registered with PalmSource Creator ID DB

// Parameter structure for pmPhoneAppNotifySetWallpaper notification 
// See PhoneAppLaunchCmdSetWallpaperType

// -----------------------------------------------------------------------------
// Sound Prefs custom launch codes.
// -----------------------------------------------------------------------------

/** Launch sounds Prefs in Phone Prefs view */
#define soundPrefsLaunchCmdPhone			(sysAppLaunchCmdCustomBase)

/** Launch Sound Prefs in SMS Prefs view */
#define soundPrefsLaunchCmdSMS  			(sysAppLaunchCmdCustomBase+1)

/** Launch Sound Prefs in IM Prefs view */
#define soundPrefsLaunchCmdIM   			(sysAppLaunchCmdCustomBase+2)

/** Launch Sound Prefs in Email Prefs view */
#define soundPrefsLaunchCmdEmail  			(sysAppLaunchCmdCustomBase+3)

/** Launch Sound Prefs in Calendar Prefs view */
#define soundPrefsLaunchCmdCalendar  		(sysAppLaunchCmdCustomBase+4)

/** Launch Sound Prefs in the MMS Prefs view */
#define soundPrefsLaunchCmdMMS              (sysAppLaunchCmdCustomBase+5)

/** Launch Sound Prefs in the General Prefs view */
#define soundPrefsLaunchCmdGeneral              (sysAppLaunchCmdCustomBase+6)

/** Different sound pref dialog views. */
typedef enum
{
  soundPrefsViewTones,
  soundPrefsViewVolume
} SoundPrefsView;

/** Contains data used by SoundPrefsLaunchCmd. */
typedef struct
{
  SoundPrefsView launchView;            /**< Launch to the tones/volume view. */
}

SoundPrefsLaunchCmdParamType, * SoundPrefsLaunchCmdParamPtr;

// -----------------------------------------------------------------------------
// Handspring Web browser custom launch codes
// -----------------------------------------------------------------------------

/** Launch the browser in the bookmark dialog */
#define		sysAppLaunchWebBrowserLaunchBookmarkDialog		(sysAppLaunchCmdCustomBase)

/** Retrieve the browser's home page */
#define		sysAppLaunchWebBrowserGetHomePage				      (sysAppLaunchCmdCustomBase + 1)

/** Set the browser's home page */
#define		sysAppLaunchWebBrowserSetHomePage				      (sysAppLaunchCmdCustomBase + 2)

/** Launch the browser in autocrawl mode */
#define   sysAppLaunchWebBrowserAutoCrawl               (sysAppLaunchCmdCustomBase + 3)

/** Launch the browser in tips/tutorial mode */
#define   sysAppLaunchWebBrowserTutorialMode            (sysAppLaunchCmdCustomBase + 4)

/** Launch the browser in minimal mode */
#define   sysAppLaunchWebBrowserMinimalMode             (sysAppLaunchCmdCustomBase + 5)

/** Contains data used when launching web browser in minimal mode. */
typedef struct {
  Char*                    launchUrl;			/**< URL to browse when launching */
  Char*                    doneButtonLabel;		/**< Label on the Done button */
} webBrowserMinimalModeLaunchInfo;

/* Launch the browser with a specific network service ID */
#define   sysAppLaunchWebBrowserURLWithSvcID             (sysAppLaunchCmdCustomBase + 6)

/** Contains data used when launching web browser with a specific SvcID. */
typedef struct {
  UInt32                   svcID;				/**< Network SvcID from NetPref */
  Char*                    launchUrl;			/**< URL to browse when launching */
} webBrowserURLWithSvcIDLaunchInfo;


//=============================================================================
// Handspring Camera application
//=============================================================================

/** Stop any active Camera preview */
#define	  cameraAppLaunchCmdStopPreview					  (sysAppLaunchCmdCustomBase)

/**Notification to request stop/pause video preview/playback.
  *In the event of high priority UI action like an incoming phone call,
  *if the responsible application does not take action the sender of this 
  *notification may take appropriate action in order to handle the high priority task */
#define pmNotifyHighPriorityUITask		'pmUT'					

/**Notification to request stop/pause audio preview/playback.
  *In the event of high priority UI action like an incoming phone call,
  *if the responsible application does not take action the sender of this 
  *notification may take appropriate action in order to handle the high priority task */
#define pmNotifyHighPriorityAudioTask 	'pmAT'					

#endif // __HSAPPLAUNCHCMD_H__





