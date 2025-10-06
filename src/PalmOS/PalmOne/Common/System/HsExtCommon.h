/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *
 * @ingroup HSExt
 *
 */

/**
 * @file 	HsExtCommon.h
 * @brief   Common defines shared between HsExt.h (#include for 68K programs)
 * and HsExtArm.h (#include for ARM programs).
 *
 */



#ifndef	  __HS_EXT_COMMON_H__
#define	  __HS_EXT_COMMON_H__

#if 0
#pragma mark --------  Public Defines   ----------------
#endif


/******************************************************************************
 * Public Defines
 *****************************************************************************/

////////////////////////////////////////
#if 0
#pragma mark Version String
#endif
////////////////////////////////////////

#define hsVersionStringSize		  32


////////////////////////////////////////
#if 0
#pragma mark Attributes
#endif
////////////////////////////////////////

/**
 * Current system activity. Possible bits for hsAttrSysActivity 
 **/
/*@{*/
#define hsAttrSysActiveHotSync	  0x0001    /**< Hot sync in progress  */
#define hsAttrSysActiveProgress	  0x0002    /**< Progress dialog is up (e.g. beaming)  */
/*@{*/

/**
 * Type of phone. Possible values of hsAttrPhoneType
 **/
/*@{*/
#define hsAttrPhoneTypeCDMA       0x0001   /**< CDMA Phone  */
#define hsAttrPhoneTypeGSM        0x0002   /**< GSM Phone  */
/*@}*/

/** Possible values of hsAttrRingSwitch
 *  For now there are only two values, but it's left open to have a control
 *  that works more like a volume control and has a range of settings. The
 *  value of the "loud" setting was chosen so that it can be decided later
 *  to make this a bit field or a scalar value. */
#define hsAttrRingSwitchMute		  0    /**< Silent position*/
#define hsAttrRingSwitchLoud		  0x8000 /**< Sounds-on position */


/**
 * Max length (in bytes) of a word (corrected or not) in the word
 * correction dictionary.  With null, word can be 26 bytes.
 **/
#define hsWordCorrectMaxWordLen		25		/**<		*/



////////////////////////////////////////
#if 0
#pragma mark Errors
#endif
////////////////////////////////////////

/**
 * Error codes
 **/
/*@{*/
#define	hsOLDErrorClass					  (appErrorClass+0x0100)
#define	hsErrNotSupported				  (hsOLDErrorClass | 0x01)
#define	hsErrInvalidCardNum				  (hsOLDErrorClass | 0x02)
#define	hsErrReadOnly					  (hsOLDErrorClass | 0x03)
#define	hsErrInvalidParam				  (hsOLDErrorClass | 0x04)
#define	hsErrBufferTooSmall				  (hsOLDErrorClass | 0x05)
#define	hsErrInvalidCardHdr				  (hsOLDErrorClass | 0x06)
#define	hsErrCardPatchAlreadyInstalled	  (hsOLDErrorClass | 0x07)
#define	hsErrCardPatchNotInstalled		  (hsOLDErrorClass | 0x08)
#define	hsErrNotEnoughPower				  (hsOLDErrorClass | 0x09)
#define	hsErrCardNotInstalled			  (hsOLDErrorClass | 0x0A)
#define hsErrInvalidPeriod				  (hsOLDErrorClass | 0x0B)
#define hsErrPeriodicNotInstalled		  (hsOLDErrorClass | 0x0C)
#define hsErrFixedStorageRequired		  (hsOLDErrorClass | 0x0D)
#define hsErrPeriodicAlreadyInstalled	  (hsOLDErrorClass | 0x0E)
#define hsErrInUse						  (hsOLDErrorClass | 0x0F)
#define hsErrIndicatorInvalidSequence	  (hsOLDErrorClass | 0x10)
#define hsErrIndicatorInvalidCap		  (hsOLDErrorClass | 0x11)
#define hsErrIndicatorDisabled			  (hsOLDErrorClass | 0x12)
// ***DO NOT DEFINE ANY MORE ERRORS IN THIS RANGE*** USE hsExtErrorClass
// BASED DEFINITIONS BELOW FOR NEW ERROR CODES!

// IMPORTANT: THE OLD ERROR CLASS USED FOR HANDSPRING EXTENSIONS ERROR
// CODES WAS IN **APP** ERROR CODE SPACE. ALL NEW CODES MUST BE DEFINED
// IN TERMS OF THE UNIQUE ERROR CLASS hsExtErrorClass

/** Unexpected numerical underflow encountered */
#define hsErrUnderflow					  (hsExtErrorClass | 0x13)

/** The user replied "No" when prompted to turn on wireless mode */
#define hsErrUserNoRadio				  (hsExtErrorClass | 0x14)

/** Failed to register on carrier's network, or timed out while
 *  waiting to register. */
#define hsErrOutOfCoverage				  (hsExtErrorClass | 0x15)

/** The blocking call was interrupted by AppStop event */
#define hsErrAppStopEvent				  (hsExtErrorClass | 0x16)

/** Request failed because of insufficient memory */
#define hsErrOutOfMemory				  (hsExtErrorClass | 0x17)

/** The SIM is missing, invalid, wrong unlock code, or some other SIM
 *  problem detected that prevented network registration from completing. */
#define hsErrSIMMissing					  (hsExtErrorClass | 0x18)

/** Failed to attach GPRS, or timed out while waiting to register */
#define hsErrGPRSNotAttached			  (hsExtErrorClass | 0x19)

/** The requiested operation timed out, without completing its objective */
#define hsErrTimeOut					  (hsExtErrorClass | 0x1A)

/** The requiested operation was interrupted or cancelled by user,
 *  before completing its objective */
#define hsErrUserInterrupted			  (hsExtErrorClass | 0x1B)

/*@}*/


////////////////////////////////////////
#if 0
#pragma mark Libraries
#endif
////////////////////////////////////////

/**
 * Library alias names that SysLibFind maps to a real library name
 *  according to the appropriate hsPrefSerialLibXXX setting.
 * 
 * By Convention, library alias names start with a '*'. The exception
 *  is the "Serial Library" name which is mapped in order to 
 *  be compatible with pre-existing applications that already use it. 
 **/
/*@{*/
#define	hsLibAliasDefault			  "Serial Library" /**< hsPrefSerialLibDef */
#define	hsLibAliasHotSyncLocal		  "*HsLoc  SerLib" /**< hsPrefSerialLibHotSyncLocal */
#define	hsLibAliasHotSyncModem		  "*HsMdm  SerLib" /**< hsPrefSerialLibHotSyncModem */
#define	hsLibAliasIrda				  "*Irda   SerLib" /**< hsPrefSerialLibIrda */
#define	hsLibAliasConsole			  "*Cons   SerLib" /**< hsPrefSerialLibConsole */
/*@{*/

/**
 * Actual library name of the Dragonball's built-in serial library.
 * This is the default value of the hsPrefDefSerialLib pref setting which
 *  SysLibFind uses to map an incoming library name to an actual library
 *  name. 
 **/
#define	hsLibNameBuiltInSerial		  "BuiltIn SerLib"


////////////////////////////////////////
#if 0
#pragma mark Features
#endif
////////////////////////////////////////

/**
 * This is the Handspring feature id. Apps can tell if they're on
 * a handspring device if they get 0 err back from: 
 * err = FtrGet (hsFtrCreator, hsFtrIDVersion, &value)
 **/
#define	hsFtrCreator				 'hsEx'


/**
 * Version of Handspring extensions
 *  0xMMmfsHHh, where:
 *
 *   MM.mm.f are the major, minor, and fix revs of PalmOS which is modified 
 *			  by this version of HsExtensions
 *
 *   s is the release stage (3-release,2-beta,1-alpha,0-development)
 *
 *   HH.h are the major and minor rev of HsExtensions for the relevant
 *			  version of PalmOS
 **/
#define	hsFtrIDVersion				  0		 


#define	hsFtrIDModDate				  1		 

/** 
 * Feature number indicating that the Launcher Database Mgr library is loaded.
 *  The value of the feature is the refNum of the loaded library.
 *  Call FtrGet (hsFtrCreator, hsFtrIDLdbMgrLibRefNum, ...) to get this feature.
 *
 * THIS FEATURE IS DEPRICATED
 *
 **/
#define	hsFtrIDLdbMgrLibRefNum		  2

/** 
 * If this feature is present, then we won't use the optimization to grab the
 * memory semaphore before doing multiple data manager calls.
 **/
#define hsFtrIDNoMemSemaphore         3

/** This features contains boolean flags to let developers know that a bug
 * present in older devices has been fixed.
 */
#define hsFtrIDBugsFixed			  4

/** 
 * This feature tells what type of hardware keys a device has.
 */

/*@{*/
#define hsFtrIDTypeOfKeyboard		  5

/**
 *  The masks used to interpret the hardware keyboard feature
 **/  
#define	hsFtrValKeyboardBasic		0x0001  /**< Basic keys (pwr/scroll/app) */
#define hsFtrValKeyboardQwerty		0x0002  /**< Qwerty keys (A-Z, Period, Space, Backspace, Enter,
												  Menu, Option, Shift) */
#define hsFtrValKeyboardJog			0x0004	/**< Jog keys (jog-up, jog-down,jog-scan) */
#define hsFtrValKeyboardCommCentric	0x0008	/**< DEPRICATED - Communicator centric buttons */
#define hsFtrValKeyboardCustomApps	0x0010  /**< Non-standard app keys (Standard is: date/addr/todo/memo)
												  replaces hsFtrValKeyboardCommCentric */

/*@{*/

/** This feature describes the radio hardware available */
#define hsFtrIDRadioAttributes		  6

// The masks used to interpret the radio attributes feature
#define	hsFtrValRadioAvailable		0x0001  	/**< Radio is present and available for use */
#define hsFtrValRadioBuiltin		0x0002  	/**< Radio is part of the base hardware and not on a module */

/**
 *  This feature defines how the power putton behaves
 *  Note: In HsExtensions 1.0 for PalmOS 5.2.1 this feature is set, but ignored
 **/  
/*@{*/
#define hsFtrIDPowerKeyResponse		  7
#define hsFtrValPowerKeyDoubleClkBacklight	0x0001  /**< Double click for backlight */
#define hsFtrValPowerKeyTripleClkInvert		0x0002  /**< Triple click for invers (requires DblClkBacklight)*/
#define hsFtrValPowerKeyHoldRadioPower		0x0004  /**< Hold for radio power */
/*@{*/

/**
 *  This feature indicates the functionality of the status gadgets API
 **/
#define hsFtrIDStatusGadgetRev		  8

#define hsFtrValStatusGadgetRev1	  0x0001  	/**< Revision 1 of API */

/**
 *  If this feature is set, the graffiti area is disabled
 **/  
#define hsFtrIDGrfAreaDisabled		  9

/**
 *  This feature tells which keyboard layout is being used
 **/
#define	hsFtrIDKeyboardLayout		  10

/** 
 * The values used to interpret the keyboard layout feature
 **/
/*@{*/
#define	hsFtrValKeyboardLayout_enUS		0x0001  /**< English */
#define	hsFtrValKeyboardLayout_deDE		0x0002  /**< German */
#define	hsFtrValKeyboardLayout_frFR		0x0003  /**< French */
/*@{*/

/**
 * This features contains the DB Id to open and
 * play the resource with type 'smfr' with resource id 1
 * when the lid opens
 *
 * THIS FEATURE IS DEPRICATED
 **/
#define hsFtrIDToneOnLidOpen		  11

/** 
 * This feature and value (s) is used to see if PhoneApp should be relaunched
 * when waking up.
 *
 * THIS FEATURE IS DEPRICATED
 **/
#define hsFtrIDCurrentAppOnSleep      12
#define hsFtrValCurrentAppOnSleepPhoneAppSpeedDialView   0x0001  /**< PhoneApp current view is Speed dial */

/**
 * This feature indicates whether the Manufacturing test app is
 * present on the device.
 **/
#define hsFtrIDTestingBootMode		  13

/** 
 * If this feature exists, then our navigation support exists.  The
 *version of the navigation support is stored in the feature's value.
 **/
#define hsFtrIDNavigationSupported	  14

/** 
 * This feature can be used to determine the hardware "API" revision.
 * It changes when there's a revision of the hardware, but the device
 * is otherwise the same.  Very few apps should ever need this.  Drivers
 * should generally ask the HAL directly for this info (though HALAttr)
 * since that's more efficient.
 **/
#define hsFtrIDHardwareApiRevision	  15


/**
 * Display Attribute Features
 */

#define hsDispFtrCreator					  'DisA'

/** Attributes about the display / controller combination... */
#define hsDispFtrIDDisplayAttributes		  0

/**
 * @name Values for the display attributes, which is a bitfield.
 *
 */
/*@{*/
#define hsDispFtrValLCDControllerTypeMask	  0x000000FF
#define hsDispFtrValDragonballVZ			  0x00000000
#define hsDispFtrValSed1375					  0x00000001
#define hsDispFtrValSed1376					  0x00000002
#define hsDispFtrValSed13A3					  0x00000003
#define hsDispFtrValOmapX10				0x00000004	/**< TI OMAP 1510/310 */
#define hsDispFtrValBulverdePxa27X			  0x00000005	/**< Intel Bulverde PXA 270, 271, etc. */

#define hsDispFtrValLCDDisplayTypeMask		  0x0000FF00		/**<		*/
#define hsDispFtrValBwStn				 0x00000100	/**< The black and white STN used in black and white Visors and Treos. */
#define hsDispFtrValColorStn				  0x00000200	/**< Color STN */
#define hsDispFtrValColorTft				  0x00000300	/**< Color TFT (used in Prism) */
#define hsDispFtrValColorOled				  0x00000400	/**< Organic LED */
/*@}*/

/**
 * @name Available Shades
 * 	 This feature contains the number of shades of gray, red, green, and blue
 *	 available on this display.  Note that this will _never_ be bigger than
 *	 the number of shades available on the controller, since the display is
 *	 fundamentally limited by what you can set the controller to.  See masks
 *	 below for accessing...
 */
/*@{*/
#define hsDispFtrIDDisplayShades			  1

/**
 * This feature contains the number of shades of gray, red, green, and blue
 * available on this controller.  This can be larger than the number of shades
 * that the display can show, since the controller can do hardware dithering
 * to deal with the fact that the display can't show all the shades.
 *
 * Software dithering is handled separately.  If there is software dithering, it
 * can be suppressed.  See hsAttrDitherSuppress
 **/
#define hsDispFtrIDControllerShades			  2

/**
 * You can use these bitmasks to get the number of shades
 * ...note that the actual number of shades is the (value+1).  This is needed
 * so in the future we can support displays/controllers that can have 256 shades.
 **/
#define hsDispFtrValGrayOffset				  24
#define hsDispFtrValRedOffset				  16
#define hsDispFtrValGreenOffset				   8
#define hsDispFtrValBlueOffset				   0
/*@}*/


// Sample usage:
//
// UInt32 displayShades;
// UInt16 redShades;
// Err err = 0;
//
// err = FtrGet (hsDispFtrCreator, hsDispFtrIDDisplayShades, &displayShades);
// if (!err)
//   {
//     // Get the number of red shades - 1...
//     redShades = (UInt16) ((UInt8) (displayShades >> hsDispFtrValRedOffset));
//     
//     // Account for +1 after doing the conversion to 8-bit...
//     redShades++;
//   }



////////////////////////////////////////
#if 0
#pragma mark Utilities
#endif
////////////////////////////////////////

/** This is the buffer size that should be sent to HsExtDoSaveAsDialog() */
#define hsMaxSaveAsNameSize 32



#if 0
#pragma mark --------  Public Enums   ----------------
#endif

/******************************************************************************
 * Public Enums
 *****************************************************************************/

////////////////////////////////////////
#if 0
#pragma mark Version String
#endif
////////////////////////////////////////

typedef enum
  {
	hsVerStrComplete,	  	/**< Composite, formatted for display in info dialog */
	hsVerStrSerialNo,	  	/**< Complete serial number (ROM token 'snum' is limited to 12 digits) */
	hsVerStrHardware,	  	/**< Revision of the hardware */
	hsVerStrProductName,  		/**< Like, "Treo 600" */
	hsVerStrCarrierID,	  	/**< Carrier ID code for carrier branding */
	hsVerStrProductRev,	  	/**< Complete product; covering hardware, radio firmware and software */
	hsVerStrROMBuild,	  	/**< ROM buld number (not the same as the build number in sysFtrNumROMVersion) */
	hsVerStrFirmwareVer,  		/**< Version of the radio firmware */
	hsVerStrPalmOSVer,	  	/**< See sysFtrNumROMVersion */
	hsVerStrLifeTime		/**< Radio life timer */
  } HsVerStrEnum;


////////////////////////////////////////
#if 0
#pragma mark Attributes
#endif
////////////////////////////////////////

/**
 * Parameters to HsAttrGet/Set calls
 **/

typedef enum 
  {	/*
	 *	These attributes with various sizes are defined 
	 *	only for compatability with existing 68K code, and
	 *	are explicitly marshalled by the 68K/ARM shim for
	 *	the HsAttrGet() and HsAttrSet() functions.
	 *
	 *	Note: The definitions for hsAttrE911Mode in the 
	 *	OS 3.5 headers, and for hsAttrKeyboardLocked in
	 *	the OS 4.0 headers both had an enum value of 8.
	 *	Apps using hsAttrE911Mode will probably have to be
	 *	recompiled anyway so backward compatability has
	 *	been maintained hsAttrKeyboardLocked.
	 */
	hsAttr68KLidOpen,				/**< UInt16 @see hsAttrLidOpen */
	hsAttr68KSysSleepReason,		/**< UInt16 @see hsAttrSysSleepReason */
	hsAttr68KDisplayOn,				/**< UInt16  @see hsAttrDisplayOn */
	hsAttr68KRingSwitch,			/**< UInt16 @see hsAttrRingSwitch */
	hsAttr68KPostProcessRsc,		/**< UInt32 @see hsAttrPostProcessRsc */
	hsAttr68KSysActive,				/**< UInt16 @see hsAttrSysActive */
	hsAttr68KActiveCalls,			/**< UInt16 @see hsAttrActiveCalls */
	hsAttr68KPhoneType,				/**< UInt32 @see hsAttrPhoneType */
	hsAttr68KKeyboardLocked,		/**< Boolean @see hsAttrKeyboardLocked */
	hsAttr68KKeyLockNextWakeTicks,	/**< UInt32 @see hsAttrKeyLockNextWakeTicks */
  	
	/*
	 * All new 68K and ARM code shall use only the following 
	 * attributes, all of which take a 32-bit value parameter.
	 */
	hsAttrLidOpen = 0x80,		/**< false=lid closed, true=lid open 
									(returns hsErrNotSupported if no lid) */
	hsAttrSysSleepReason,		/**< Same as the SleepEventParamType.reason
						    		   from the last sysNotifySleepRequestEvent
						    		   unless the real reason is different
						    		   (e.g. hsSysSleepLid) */
	hsAttrDisplayOn,			/**< true = display is on, false = dispay is off */
	hsAttrRingSwitch,		/**< State of ring switch, can be either hsAttrRingSwitchMute or hsAttrRingSwitchLoud */
	hsAttrPostProcessRsc,		/**< Pointer to resource containing the post-processing lists */
	hsAttrSysActive,			/**< Bits representing various system activity. Current
									 implementation sets only the hsAttrSysActiveHotSync
									 bit if the HotSync dialog is active*/
	hsAttrActiveCalls,			/**< Number of active voice calls */
	hsAttrPhoneType,			/**< returns hsAttrPhoneTypeCDMA if a CDMA phone 
									 or hsAttrPhoneTypeGSM if a GSM phone */
	hsAttrKeyboardLocked,		/**< true = keyguard active */
	hsAttrKeyLockNextWakeTicks,	/**< deprecated */
	hsAttrKeyboardDB,			/**< DmOpenRef of database containing keyboard layout resources */
	hsAttrDigitizerLock,		/**< true = digitizer is locked */
	hsAttrE911Mode,				/**< true = phone is in 911 mode */	
	hsAttrDitherSuppress,		/**< count of requests to supress dithering
										Set to non-zero to increment, set to zero to decrement */
	hsAttrHotSyncSuppress,		/**< count of requests to suppress HotSync
										Set to non-zero to increment, set to zero to decrement */

	hsAttrPhoneRadioPowerState,	/**< notifies HsExt that cell phone radio was just powered on
										or off; non-zero if cell phone radio was powered on, zero if
										powered down.  Power on does *not* imply completion of SIM
										detection or registration */

    hsAttrAllKeysWakeFromSleep, /**< (Refcount) sets the device in a mode where _all_ keys wake
                                        the device from sleep and are processed.  This is intended in 
                                        special cases where we've powered off the screen to save power 
                                        (or something similar), but the user still thinks the device of 
                                        the device as on.  As a side effect, this will stop 'keyboard lock' 
                                        from starting when the device turns off.
                                        Set to non-zero to increment, set to zero to decrement */

    hsForcePrefAutoOffDurationSecs, //  Forces the Keyguard dialog to have a timeout period of prefAutoOffDurationSecs
    hsAttrKeyguardDialogOn,         //  Denotes if the Keyguard dialog is On or not.

    // This section is reserved so that if we need to add adds more attributes in "SDK 2.0"
	// products that they won't conflict with attributes added in "SDK 2.1"
	hsAttrReserved_SDK20_1,
	hsAttrReserved_SDK20_2,
	hsAttrReserved_SDK20_3,
	hsAttrReserved_SDK20_4,
	hsAttrReserved_SDK20_5,
	hsAttrReserved_SDK20_6,
	hsAttrReserved_SDK20_7

	/*
	 * Attributes greater than 0x9000 are defined for internal use
	 * in Prv/HsExtAttr.h
	 */
  } HsAttrEnum;


////////////////////////////////////////
#if 0
#pragma mark Preferences
#endif
////////////////////////////////////////

/**
 * Equates for the HsPrefGet/Set calls
 **/
typedef enum
  {			
  
	// The following are used by SysLibFind() to resolve a virtual library
	//  name to an actual one.  They are also used directly by SrmOpen()
	//	to figure out whether to override a new-style driver w/ an old-style
	//	driver.
	hsPrefSerialLibDef,		/**< Char[] : Name of serial library to substitute for hsLibAliasDefault */
	hsPrefSerialLibHotSyncLocal,  	/**< Char[] : Name of serial library to substitute for hsLibAliasHotSyncLocal */
	hsPrefSerialLibHotSyncModem,  	/**< Char[] : Name of serial library to substitute for hsLibAliasHotSyncModem */
	hsPrefSerialLibIrda,		/**< Char[] : Name of serial library to substitute for hsLibAliasIrda */
	hsPrefSerialLibConsole,		/**< Char[] : Name of serial library to substitute for hsLibAliasConsole */

	/**** End of prefs recognized by Visor, Visor Deluxe, Platnium, Prism, Edge ****/

	// The following are used in SysHandleEvent() to launch apps in response
	//  to associated virtual characters.
	hsPrefJogScanCharAppCreator,	  /**< UInt32 : Creator type of app to launch in response to hsChrJogScan */
	hsPrefLidStatusChgCharAppCreator, /**< UInt32 : Creator type of app to launch in response to hsChrLidStatusChg */
    hsPrefOptHard1CharAppCreator,	  /**< UInt32 : Creator type of app to launch n response to hsChrOptHard1 */
    hsPrefOptHard2CharAppCreator,	  /**< UInt32 : Creator type of app to launch in response to hsChrOptHard2 */
    hsPrefOptHard3CharAppCreator,	  /**< UInt32 : Creator type of app to launch in response to hsChrOptHard3 */
    hsPrefOptHard4CharAppCreator,	  /**< UInt32 : Creator type of app to launch in response to hsChrOptHard4 */

	// The following are used to set Key Manager preferences
	hsPrefTempShiftTimeOut,			  /**< UInt32 : Number of ticks until a temporary
						shift state (upper shift or option shift) times out. */

	hsPrefLidClosedAutoOffSeconds,	  /**< UInt16 : Number of seconds for auto-off when the lid is closed */

	hsPrefLCDRefreshRate,		  /**< UInt16 : Type of refresh rate for lighting see HsLCDRefreshRateEnum */

	/**** End of prefs recognized by Treo 90, 180, 270, 300 ****/

	hsPrefNetworkTimeSelected,	  /**< UInt16: used by CDMA devices - should it read the time from the network*/

	hsPrefKeyguardAutoEnableTime,	  /**< UInt16: How many seconds to wait before enabling keyguard*/


	hsPrefKeyguardAutoEnableAutoOffExtraTime, /**< UInt16 : How many seconds to wait before enabling
													  keyguard when hsPrefKeyguardAutoEnableTime is
													  set to hsKeyguardAutoPowerOff and the device
													  powers off due to the auto-off timer */

	hsPrefKeyguardFlags,			  /**< UInt16 : Flags controlling keyguard's behaviour */

	hsPrefTouchscreenDisableFlags,	  /**< UInt16: Used by the phone app to determine when to disable touchscreen */

	// The following are used in SysHandleEvent() to launch apps in response
	//  to associated virtual characters.
    hsPrefOptHardPowerCharAppCreator, /**< UInt32 : Creator type of app to launch in response to hsChrOptHardPower */

    hsPrefOptReservedCharAppCreator,  /**< UInt32 : Creator type of app to launch in response to hsChrOptReserved (for future use) */

	hsPrefPowerOnAppCreator,			  /**< UInt32 : Creator type of app to launch
											  instead of the normally mapped app when
											  the app key turned the power on */

	/**** End of prefs recognized by Treo 600 ****/
  
	hsPrefSideHardCharAppCreator,		/**< UInt32 : Creator type of app to launch
											  instead of the normally mapped app when
											  the side key is pressed */

	hsPrefBlinkGreenLEDInCoverage,		/**< UInt16 : Set to true if we want the green
											  LED to blink when the device is in coverage */

	// Leave this one at end!!!
	hsPrefLast

  } HsPrefEnum;


/**
 * @name Special values for hsPrefKeyguardAutoEnableTime
 *
 */
/*@{*/
#define hsKeyguardAutoDisabled					0	/**< Never */
#define hsKeyguardAutoPowerOff					0xFFFF	/**< When device powers off */
/*@}*/

/**
 * @name Values for hsPrefKeyguardFlags
 *
 */
/*@{*/
#define hsKeyguardFlagPowerOnDialog				0x0001	/**< Show keyguard dialog at power-on */
/*@}*/

/**
 * @name Bit definitions for hsPrefTouchscreenDisableFlags
 *
 */
/*@{*/
#define hsTouchscreenDisableFlagIncomingCall	0x0001			/**< 		*/
#define hsTouchscreenDisableFlagDuringCall		0x0002		/**< 		*/
/*@}*/

////////////////////////////////////////
#if 0
#pragma mark Status Gadgets
#endif
////////////////////////////////////////

/**
 * Equates for supporting system handled status gadgets
 **/
typedef enum
  {
	hsStatusGadgetBattery = 1,			/**< Status gadget is a battery meter */
	hsStatusGadgetSignal = 2			/**< Status gadget is a signal strength indicator */
  } HsStatusGadgetTypeEnum;				



////////////////////////////////////////
#if 0
#pragma mark Light Manager
#endif
////////////////////////////////////////

/**
 * Light manager equates
 **/

/**
 * These are different modes that can be used to affect the light manager,
 * which is in charge of temporary changes in the lighting situation.
 **/

typedef enum
  {
	// Preset modes
	// ------------
	// These modes have saved values associated with them...
	hsLightModeNormal = 0,		/**< Light mode is at normal presets */
	hsLightModeNight  = 1,		/**< Light mode is at night-mode presets */

	hsLightModeNumPresets,	/**< Num presets.  SUBJECT TO CHANGE. */

	/** Reserved mode to force compiler to use 16-bit for enum... */
	hsLightModeReserved = 0x7FFF	
  }
HsLightModeEnum;


/**
 *  These circumstances modify the mode and allow us to handle 
 *  different logical circumstances on devices with different hardware.
 *
 *  - These are applied in the following order:
 *   1. Alert woke device
 *   2. keylight off
 *	 3. Quick typing
 *	 4. User not looking
 *   5. User not looking for a long time
 **/
typedef enum
  {
	hsLightCircumstanceFirst		= 0,	/**< 		*/
	hsLightCircumstanceAlertWokeDevice	= 0,	/**< An important alert woke the  device up from sleep.*/
	hsLightCircumstanceQuickTyping		= 1,	/**< We want the user to be able  to access the keyboard easily */
	hsLightCircumstanceUserNotLooking	= 2,	/**< We suspect the user isn't looking at the device if they're
							     not actively using the device, so we'll dim the screen (but we don't
												  actually want to turn it off).*/
	hsLightCircumstanceUserNotLookingLongTime = 3,  /**< similar to above, but if the inactivity persists then 
							   we want to turn the screen off **/

	hsLightCircumstanceKeylightOff		= 4,	/* turn keylight off */

	hsLightCircumstanceNumCircumstances,		/**< Num circumstances.   SUBJECT TO CHANGE.*/

	/**
	 *  Reserved circumstance to force compiler to use 16-bit for enum...
	 **/
	hsLightCircumstanceReserved = 0x7FFF
  }
HsLightCircumstanceEnum;


/**
 * @name Bit Flag for Light Circumstances
 *
 * application may call HsCurrentLightCircumstance() to get current
 * enabled light circumstances
 *
 * test bit to see if light circumstance is enabled or disabled
 *
 */
/*@{*/
#define hsLightCircumstanceAlertWokeDeviceFlagBit		0x0001 		/**< 		*/
#define hsLightCircumstanceQuickTypingFlagBit			0x0002		/**< 		*/
#define hsLightCircumstanceUserNotLookingFlagBit		0x0004		/**< 		*/
#define hsLightCircumstanceUserNotLookingLongTimeFlagBit	0x0008		/**< 		*/
#define hsLightCircumstanceKeylightOffFlagBit			0x0010		/**< 		*/
/*@}*/

////////////////////////////////////////
#if 0
#pragma mark Indicators
#endif
////////////////////////////////////////

/**
 *
 **/
typedef enum 
  {
	kIndicatorTypeLed,
	kIndicatorTypeVibrator,

	// This must be last
	kIndicatorTypeCount
  }
HsIndicatorTypeEnum;

/**
 *  Pass a numeric count or one of these special values to HsIndicatorState()
 **/
typedef enum
  {
	kIndicatorCountGetState	= 0,        /**< retrieve the current state */
	kIndicatorCountForever	= 0xFFFF    /**< run forever */
  }
HsIndicatorCountEnum;


/** 
 * There are several different sources of turning on the indicator,
 * (Charge, Signal, Ring, App) with each source being in exactly one
 * state at any moment. The high byte of these enum values indicates
 * the source, and the low byte indicates which state that source is
 * in. Each source has a hard-coded priority, so only the indication
 * of the highest priority source not in its "None" state will appear
 * on the indicator. The value of the high byte does not represent 
 * the priority.

 * A virtual state that is used to get the state that is currently
 * active on the indicator (the highest priority active source). This
 * value is also the result when there is no active source.
 * (States with * aren't used at present)
 **/
typedef enum
  {

	kIndicatorStateNull	  = 0x0000,

	kIndicatorForceNone	  = 0x0100,		/**< Don't force anything */
	kIndicatorForceOn	  = 0x0101,		/**< Force the indicator on */
	kIndicatorForceRed	  = 0x0101,		/**< Force the red led on */
	kIndicatorForceGreen,				/**< Force the green led on */
	kIndicatorForceRedGreen,			/**< Force both leds on (orange) */
	kIndicatorForceOff	  = 0x0180,		/**< Force both leds off */

	kIndicatorRingNone	  = 0x0200,		/**< Phone not ringing */
	kIndicatorRingActive,				/**< Phone ringing */
	kIndicatorRingContact,				/**< *Phone ringing, CID recognized */
	kIndicatorRingRoam,					/**< *Phone ringing, roaming */

	kIndicatorAlertNone	  = 0x0300,		/**< Alert not active */
	kIndicatorAlertAlert,				/**< Generic alert */
	kIndicatorAlertMail,				/**< Incoming mail */
	kIndicatorAlertCalendar,			/**< Datebook alarm */
	kIndicatorAlertSMS,					/**< Incoming SMS */
	kIndicatorAlertVoicemail,			/**< Voicemail waiting */

	kIndicatorBattNone	  = 0x0400,		/**< Not connected to a charger */
	kIndicatorBattCharge,				/**< Battery is charging */
	kIndicatorBattTrickle,				/**< Battery is topping off */
	kIndicatorBattFull,					/**< Battery is fully charged */
	kIndicatorBattLow,					/**< *Battery is low (and not on a charger) */

	kIndicatorUsageNone	  = 0x0600,		/**< Not in use */
	kIndicatorUsageVoiceCall,			/**< Phone on an active voice call */
	kIndicatorUsageDataCall,			/**< Phone on an active data call */

	kIndicatorSignalNone  = 0x0500,			/**< Radio turned off */
	kIndicatorSignalBad,				/**< Out of coverage */
	kIndicatorSignalGood,				/**< In coverage */
	kIndicatorSignalRoam				/**< *In coverage, roaming */
  }
HsIndicatorStateEnum;
	

////////////////////////////////////////
#if 0
#pragma mark Company / HAL / Device IDs
#endif
////////////////////////////////////////

// These constants can be used to tell one device from another...
//
// See the following, in Palm's SystemMgr.h:

// #define	sysFtrNumOEMCompanyID			20	// GHwrOEMCompanyID value       (PalmOS 3.5)
// #define	sysFtrNumOEMDeviceID			21	// GHwrOEMDeviceID value        (PalmOS 3.5)
// #define	sysFtrNumOEMHALID				22	// GHwrOEMHALID value           (PalmOS 3.5)


// Company ID
// ----------

// This is defined in HwrMiscFlags.h already...
// #define hwrOEMCompanyIDHandspring	'hspr'	// Devices made by Handspring


/**
 * @name HAL IDs
 *	 Handspring generally doesn't differentiate different HALs and uses the same
 *	 ID in multiple places...  Use the device ID to figure out which devide you're
 *	 on.  If you're looking to see which version of HsExtensions is available, see
 *	 the HsExtensions feature (hsFtrIDVersion)...
 */
/*@{*/
#define hsHALIDHandspringOs35_40	  'Hs01'	/**< Used by all Handspring OS 3.5 HALs and 4.0 HALs */

#define hsHALIDHandspringOs5Rev1	  'H5_1'	/**< Used by Handspring's 5.0 HALs */
#define hsHALIDHandspringOs5Rev1Sim	  'H5s1'	/**< Used by Handspring's 5.0 Simulator HALs */

#define hsHALIDHandspringOs5Rev2	  'H5_2'	/**< Used by Handspring's 5.2 HALs */
#define hsHALIDHandspringOs5Rev2Sim	  'H5s2'	/**< Used by Handspring's 5.2 Simulator HALs */
/*@}*/

/**
 * @name Device IDs
 *	 Older devices don't use 4-character constants; newer devices do.  Sometimes
 *	 devices will have the same ID if they are extremely similar.  You may need
 *	 to use alternate means to differentiate between such products.
 */
/*@{*/
#define hsDeviceIDVisor			  0x00		/**< Visor, Visor Deluxe */
#define hsDeviceIDVisorPlatinum	  0x08			/**< Visor Platinum, some Visor Neos */
#define hsDeviceIDVisorEdge		  0x09		/**< Visor Edge */
#define hsDeviceIDVisorPrism	  0x0A			/**< Visor Prism */
#define hsDeviceIDTreo180		  0x0B		/**< Treo 180 */
#define hsDeviceIDTreo270		  0x0D		/**< Treo 270 */
#define hsDeviceIDTreo300		  0x0E		/**< Treo 300 */
#define hsDeviceIDVisorNeo		  0x8A		/**< Visor Neo (some Neos may have 0x08) */
#define hsDeviceIDVisorPro		  0x8B		/**< Visor Pro */
#define hsDeviceIDTreo90		  0x8C		/**< Treo 90 */
/*@}*/

// kPalmOneDeviceID constants defined in palmOneResources.h.  From now on device IDs should
//	be defined there.

#define hsDeviceIDTreo600		  kPalmOneDeviceIDTreo600	/**< Treo 600 */
#define hsDeviceIDTreo600Sim	  kPalmOneDeviceIDTreo600Sim  		/**< Treo 600 Simulator */

// For backward compatibility
#define hsDeviceIDOs5Device1	  kPalmOneDeviceIDTreo600
#define hsDeviceIDOs5Device1Sim	  kPalmOneDeviceIDTreo600Sim


// Sample code
// -----------

// To use this to detect Handspring devices, do something like:
//  {
//    UInt32 company = 0, deviceID = 0;
//
//    FtrGet (sysFtrCreator, sysFtrNumOEMCompanyID, &company);
//    FtrGet (sysFtrCreator, sysFtrNumOEMDeviceID, &deviceID);
//
//    if ((company == hwrOEMCompanyIDHandspring) && (deviceID == hsDeviceIDOs5Device1))
//	    {
//		  ...
//		}
//  }
//
// To check for Handspring devices in general and to get the versions
// of HsExtensions, use something like:
//  {
//	  UInt32 version = 0;
//
//	  FtrGet (hsFtrCreator, hsFtrIDVersion, &version);
//    if (version >= 0x05000000)
//		{
//		  ...
//		}
//	}

#endif	  // __HS_EXT_COMMON_H__
