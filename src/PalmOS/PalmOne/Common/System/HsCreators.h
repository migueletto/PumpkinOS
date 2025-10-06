/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup SystemDef
 *
 */

/**
 * @file 	HsCreators.h
 * @version 1.0
 * @date    01/08/2000
 *
 * @brief Public header file for the creator IDs used by Handspring
 */

/*
 * @author	 dia
 *
 *  History:
 *	4-May-1999 VMK - Created by Vitaly Kruglikov
 *	8-Jan-2000 DOC - Added resource type and id for character lookup table
 *					  used by Manhattan's keyboard driver
 */

#ifndef	  __HS_CREATORS_H__
#define	  __HS_CREATORS_H__



//-----------------------------------------------------------
// Note from Palm Computing's SystemMgr.rh:
//................................................................
// File types and creators
//
//	Each database shall have a creator ID and a type.
//
//	The creator ID shall establish which application, patch, or extension
//	a particular database is associated with.  The creator ID should identify
//	the application/patch/extension, NOT who created it.
//
//	The type will determine which part of an application,
//	patch, or extension a particular database is.
//
//	There can be only one database per application, patch, or extension
//	which has type 'application', 'patch', or 'extension'.
//
//	Creators:
//
//	ROM-based applications created by Palm Computing have all-lower case
//	creator ID's.  Third-party applications have creator ID's which
//	are either all caps, or mixed case.  The same requirements go for
//	system patches and extensions.
//
//	All applications, system patches and extensions shall have unique creator
//	ID's.
//
//	Types:
//
//	'Application', 'Extension', and 'Patch' file/database types for ROM-based
//	apps shall be all-lower case.  Other file/database types must be mixed-case,
//	or all caps.  These other types are internal to the applications, and
//	therefore the system is unconcerned with their exact values.
//................................................................

//  All Creators with "Yes" in the Reg column have been registered into
//  Palm's Creator database using the web page at
//  http://dev.palmos.com/creatorid/


// ------------------------------------------------------------------
// Apps											  // Reg : Comment
// ------------------------------------------------------------------
/**
 * @name Misc. App Creator IDs
 */
/*@{*/
#define hsFileCAdvCalculator			'HsPr'	  // Yes : Advanced Calculator application
#define hsFileCCardSetup				'HsCd'	  // Yes : ALL card setup utilities have
												  //	  this creator

#define	hsFileCSampleCardWelcome		'CWel'	  // Yes : Sample card welcome app
#define	hsFileCHandTerminal				'HTrm'	  // Yes : HandTerminal application sample code

#define hsFileCFileMover				'FlMv'	  // Yes : FileMover application (for flash cards)

#define hsFileCFileInstaller			'FlIn'	  // Yes : FileInstaller application (for flash cards)

#define hsFileCDateBook3H				'HsDB'	  // Yes : Handspring's version of Datebk3.

#define hsFileCBackup					'HsBk'	  // Yes : Serial Flash Backup application
#define hsFileCBackupWelcome			'HsBw'	  // Yes : Not used
/*@}*/

/**
 * @name Phone App Creator IDs
 */
/*@{*/
#define hsFileCPhone					'HsPh'	  // Yes : Phone application
#define hsFileCPhoneTips				'PhTp'	  // Yes : Phone tips
#define hsFileCPhoneSecurity	        'HsSE'    // Yes : Phone Security application
#define	hsFileCPhoneWizard				'HsPw'	  // Yes : Phone wizard application
#define hsFileCPhoneRingerPanel			'HsRN'    // Yes : Ringer Panel
#define hsFileCDisplayPanel				'HsDs'	  // Yes:  Display Panel
#define hsFileCKeyguardPanel			'HsKg'	  // Yes:  Keyguard prefs Panel
//#define hsFileCSMS						'SMSa'	  // No  : Phone SMS app (Short Message Service)
#define hsFileCSMS						'SMSh'	  // Yes : Phone SMS app (Short Message Service)
#define hsFileCSimPhoneBook				'HsPB'	  // Yes : Phone - Sim PhoneBook app
#define hsFileCSimServices				'HsSS'	  // Yes : Phone - Sim Services app
#define hsFileCIM						'HsIM'	  // Yes : Phone - IM (Instant Messaging) app
//#define hsFileCSmartTextEngine			'HsRT'    // Yes : Smart Text Engine shared library (68K)
#define hsFileCSmartTextEngine			'HsST'    // Yes : Smart Text Engine shared library (ARM)
#define hsFileCTexter					'HsCh'	  // Yes : Texter, aka SMS

#define hsFileCMessageStore				'HsMS'	  // Yes : Message Store (used by NeXTer)
#define hsFileSMSTreoTransport			'Trn0'	  // Yes
#define hsFileMMSTreoTransport			'Trn1'	  // Yes
#define hsFileSMSPalmTransport			'Trn2'	  // Yes
#define hsFileMMSPalmTransport			'Trn3'	  // Yes
/*@}*/

/**
 * @name Misc. App Creator IDs
 */
/*@{*/
#define hsFileCImagine					'HsIm'    // Yes : Imagine (imaging app)

#define hsFileCAddrBookExtensions       'HsAX'    // Yes : Handspring address book extensions
#define hsFileCDialingPanel             'HsDi'    // Yes : Dialing Panel...
#define hsFileCAddrBookOneFingerMode	'HsAl'	  // Yes : Address book modifications - one finger mode

#define	hsFileCPinballGame				'HsPg'	  // Yes : Pinball color game
#define hsFileCSetISP					'HsJS'	  // Yes : SetISP application -Sitti 2001-05-17 for Hydra project.
/*@}*/

/**
 * @name Web Browser Creator IDs
 */
/*@{*/
#define hsFileCBlazer					'BLZ1'	  // Yes : Bluelark Blazer browser
#define hsFileCBlazer3					'BLZ5'	  // Yes : Blazer 3 browser (OS 5)
#define hsFileCBlazer3TutorialLauncher	'HsTM'	  // Yes : Blazer 3 Tutorial mode launcher & device tutorial content
#define hsFileCBlazer3TipsSharedContent 'HsSC'	  // Yes : Blazer 3 Shared tutorial/tips content
/*@}*/

/**
 * @name Misc. App Creator IDs
 */
/*@{*/
#define hsFileCCityTime					'CiAa'	  // No  : CityTime app - creator belongs
												  //		to original author, Darren Beck

#define hsFileCKeyboardDemo				'kDem'	  // Yes : Keyboard Demo

#define hsFileCFirmwareUpdater			'HsFU'	  // Yes : Wismo Firmware updater
#define hsFileCGPRSUpdater				'HsGU'	  // Yes : GPRS Updater for the Liberty project
#define hsFileCRomUpdater				'hROM'	  // Yes : ROM updater
#define hsFileCCDMAUpdater              'HsCU'    // Yes : CDMA Firmware Updater

#define hsFileCCDMAActivation           'HsAc'    // Yes : ## Application

#define hsFileCGSMActivation			'HsGA'    // Yes : ## Application

#define hsFileCIOTA                     'Iota'    // Yes : IOTA Application (Sprint)
#define hsTetheredTreo					'HsTt'	  // Yes : Tethering application for Treos

#define hsFileCDebugPrefs				'hsCn'	  // Yes : Debug Prefs application

#define hsFileCCamera					'Bcam'	  // yes : Camera app
#define hsFileCCameraTips				'BcTp'	  // yes : Camera Tips
#define hsFileCDuoHack					'DuHk'	  // yes : Duo Hack
#define hsFileCTutorialContent			'TMTp'    // yes : Duo tutorial content
#define hsFileCEmailTipsContent         'HsTC'    // yes : Email tips content
#define hsFileCNavSample				'HsNS'	  // yes : Sample navigation app
#define hsFileCSave2Card				'HsSD'    // yes : Save Exchange manager file to VFS card
#define hsFileCBrightness                               'brtP'    // yes : brightness popup app for Treo 650
#define hsFileCGettingStartedApp                        'GSGa'    // yes : app for launching Getting Starged Guide (GSG)
/*@}*/


// Testing apps...
#define hsFileCLangSpoofer				'HsLS'    // Yes : Language spoofer
#define hsFileCAddrTester				'HsAT'    // Yes : Address Tester
#define hsFileCMailWrapper              'HsMW'    // Yes : Mail Wrapper...
#define hsFileCUsbTester                'HsU2'    // No  : Usb tester...
#define hsFileCPhoneWrapper				'HsPW'	  // Yes : Phone Wrapper
#define hsFileCSetMem					'Hssm'	  // Yes : Handspring Set Mem App
#define hsFileCSerialCoupler			'Hssc'	  // Yes : Handspring serial coupler app
#define hsFileCUartCoupler				'Hsuc'	  // Yes : Handspring uart coupler app
#define hsFileCSpringWriter				'Hss1'	  // Yes : Not used (feel free to take)
#define hsFileCUsbResetTest				'Hsur'	  // Yes : USB reset tester.
#define hsFileCGasTest					'Hsgt'	  // Yes : Gas Guage testing application
#define hsFileCIsolateMemoryChip		'Isom'	  // Yes : Isolate a failing memory chip
#define hsFileCCameraTest				'HsZZ'	  // Yes : Camera test application

#define hsFileCUseSerial				'HSUS'	  // Yes : App to signal FlashPrep for serial use
#define hsFileCTraceViewer				'HsTV'	  // Yes : Trace Viewer App
#define hsFileCPmUIUtilLibTest			'PmUT'	  // Yes : PmUIUtilLibTest App

// ------- Types
#define	hsFileTCardSetup				'HsSU'	  // N/A : ALL card setup utilities have this
												  //		type
#define	hsFileTCardSetupShell			'HApp'	  // N/A : Card Setup Shell application type

#define hsFileTSndFile					'SndF'	  // Yes : Unused... was for FileStream databases holding
												  // sampled sound that can be played with SndFileStream


// Creators for things other than apps...
#define hsFileCAutomaticHelper          'Auto'    // Yes : Apps that handle the "automatic"
                                                  //       choice for dialing.

// ------------------------------------------------------------------
// Panels											  // Reg : Comment
// ------------------------------------------------------------------

#define hsFileCPhoneServicesPanel		'hsPS'	  // No: Phone preference panel

#define hsFileCLCDAdjustPanel			'HsLA'	  // Yes: LCD Adjustment preference panel

#define hsFileCMultiChannelRingToneDB   'MCnl'    // Yes: Database that has multi-channel ring tones

#define hsFileCDefaultAppsPanel			'HsDH'	  // Yes: Default Helper Apps panel

#define hsFileCButtonPrefsPanel			'HsBt'	  // Yes: Buttons Prefs Panel
// ------------------------------------------------------------------
// Patches
// ------------------------------------------------------------------
//#define hsFileC....

// ------- Others
#define hsFileCHandspringPalmOSUpdate	'Hspt'	  // Yes : Creator of PalmOS 3.5 patches
												  //	   sanctioned by Handspring.  Type
												  //	   of these files should be 'ptch'.
#define hsFileCHandspringSystemResource	'HsSr'	  // Yes: Creator of database that overrides
												  //	  palm system resources.


// ------- Types
#define	hsFileTHandspringHal			'HwAl'	  // N/A : File type of HAL.prc
#define	hsFileTHandspringPatch			'HsPt'	  // N/A  : File type of HsExtensions.prc

#define	hsFileTHandspringUIObj			'HsUi'	  // N/A  : File type of HsExtUIRsc.prc



// ------------------------------------------------------------------
// Extensions
// ------------------------------------------------------------------
#define hsFileCHandspringExt          'HsEx'	  // Yes : Handspring system extensions
#define hsFileCNavExt                 'HsEp'    // Yes : Handspring NAV extensions
#define hsFileCModemMgrExt            'HsMM'    // Yes : Handspring ModemMgr extensions
#define hsFileCDataMgrExt             'HsDM'    // Yes : Handspring DataMgr extensions
// 'HsPM' has been registed with PalmSource on March 24, 2003 at 3:10pm PST
#define hsFileCProgressMgrExt		  'HsPM'	// Yes : Handspring Progress Manager patch extensions
#define hsFileCUIExt				  'HsUI'	// Yes: Handspring UI extensions

// ------------------------------------------------------------------
// Libraries
// ------------------------------------------------------------------

/**
 * @name Library Creator IDs
 */
/*@{*/
#define hsFileCSampleSerLib				'HsCs'	  // Yes : Sample Serial 16650 library

#define hsFileCTaskLib                  'HsTS'    // Yes: Task library

#define hsFileCFfsCSegLib				'FfMg'	  // Yes : Flash File System code segment
												  //		library

#define hsFileCLdbMgr					'HsLm'	  // Yes : Launcher Database Manager library


#define hsFileCModemSerLib				'HsMs'	  // Yes : Modem Module Serial library

#define hsFileCFlMgr					'FLMG'	  // Yes : Flash Manager

#define hsFileCFluMgr					'FlMg'	  // Yes : Arm based Flash Utility Library

#define hsFileCHwmCSegLib				'Hhwm'	  // Yes : Phone HAL

#define hsFileCCDMAPhoneLib             'HsCL'    // Yes : CDMA Phone Library

#define hsFileCCDMAMsgLib               'HsMg'    // Yes : CDMA Message Library

#define hsFileCGSMPhoneLib              'GSM!'    // Yes : GSM Phone Library

#define GSMLibRingsDBCreatorID          'GSMr'    // Yes : moved from gsmlibrary.h <chg 09-12-2002 TRS> DUO CLEANUP

#define hsFileCTransparencyLib          'HsTL'    // No : Transparency Library creator

#define hsFileCComChannelProvider       'HsCP'    // No : Com Channel Provider creator

#define hsFileSMSExchangeLib            'HsSX'    // No : SMS Exchange Library

#define hsFileCFakeNetLib				'netl'	  // Same as the real NetLib used to be, so our
												  //  "fake" NetLib library will be called by
												  //  clients instead of the "true" NetLib.
#define hsFileCPatchedNetLib			'netb'	  // We change the creator id of the "true" NetLib in 5.x
												  //  to this value to make sure that our "fake" NetLib
												  //  wrapper will be called instead.

#define hsFileCameraLib					'HsCa'	  // No : Camera Library

#define hsFileCARMTelephonyWrapper      'PIL!'    // Yes : ARM Telephony Wrapper
#define hsFileCTelephonyInterfaceLayer  'TIL!'    // Yes : Telephony Interface Layer
#define hsFileCCodecWrapper				'CdWr'	  // Yes : CPM Codec Wrapper

#define hsFileBrowserCoreLib			'AcBC'	  // Yes : Blazer browser core library

#define hsFileSoundLib					'HsAu'	  // Yes : HsSoundLib
#define hsAmrLib						'AMRL'	  // No: Amr library used by AmrEncLib (below)
#define hsAmrEncLib						'AENL'	  // No: AmrEncLib audio speech encoder
#define hsAmrDecLib						'ADEL'	  // No: AmrDecLib audio speech decoder

#define hsFileCMCLLib                   'MCLL'    // No : MCL Library

#define pmFileCKeyLib					'PmKe'	  // Yes : Key Library

#define pmFileCUIUtilLib				'PmUU'	  // Yes : UI Utility Library

#define pmFileCSystemLib                'PmSy'    // Yes : System Library

#define pmFileCSysGadgetLib				'PmSg'	  // Yes : System Gadget Library

// Driver database creator and driver creator for the built-in Serial driver
// IMPORTANT: There is a "feature" in the Connection Panel that makes it
//  always pick the first serial driver as the default method when creating
//  a new connection and assuming the first method supports modems.
// So... we make sure the creator of the built-in serial comes first in
//  the directory listing by giving it a creator ID that sorts before
//  all the other serial driver creator IDs (hsFileCUsbLib, hsFileCSerDrvrAuto,
//	sysFileCVirtIrComm).
#define	hsFileCSerLib					'A8xZ'	  // Yes  : SerialHWMgr driver ID

#define	hsFileCModuleSerLib				'HsSs'	  // Yes  : Serial lib for UART2 (goes to springboard)
												  //        ...only present on some devices like Belle...

#define hsFileCVirtualModemSerLib		'FakM'	  // Yes  : Virtual Modem

#define hsFileCVirtualGSMSerLib			'VGSM'	  // Yes  : Virtual GSM

#define hsFileCDirectRadioSerLib		'DirR'	  // Yes  : Direct Radio access SdrvDirectRadio

#define hsFileCHsSerWrapperLib			'Hssw'	  // Yes  : Old-style library used to wrap new drivers.

#define hsFileSPI1SerLib				'HsSp'	  // Yes  : SPI1-Serial library

#define hsFileCHsSSLLib109a				'hSSL'	  // Yes  : Handspring version of SSLPlus 1.0.9a.
												  //		...also defined as PalmHsSSLPLUS109aLibCreatorID

// Driver database creator for the USB drivers
#define	hsFileCUsbLib					'HsUs'	  // Yes : USB Library database creator
  // These driver ID's are installed by the USB database
  #define	hsFileCSerDrvrUSBConsole	'HsUc'	  // Yes : SerialHWMgr driver ID
  #define	hsFileCSerDrvrUSBDesktop	'HsUd'	  // Yes : SerialHWMgr driver ID
  #define	hsFileCSerDrvrUSBRemFS		'HsUr'	  // Yes : SerialHWMgr driver ID
  #define	hsFileCSerDrvrUSBGeneric	'HsUg'	  // Yes : SerialHWMgr driver ID
  #define	hsFileCSerDrvrUSBGeneric2	'HsUh'	  // Yes : SerialHWMgr driver ID


// Driver database creator and driver creator for the auto-select virtual driver
// ...this one is able to be set by springboard modules; by default it maps to
// the cradle...
#define	hsFileCSerDrvrAuto				'HsSA'	  // Yes  : SerialHWMgr driver ID

#define	hsFileCSerDrvrCradle			'HsCS'	  // Yes  : SerialHWMgr driver ID

#define hsFileCSDLib					'Hssd'	  // Yes  : SD Library ID

#define hsFileCCCLib              'CCLb'  // Yes : Carrier Customization Library

#define hsFileCOldGrfLib				'HsGf'	  // Yes  : Creator ID given to PalmOS's Graffiti Lib

#define hsFileCTonesLib					'HsTo'	  // Yes  : Ringtones library
/*@}*/

// ------- Types
#define hsFileTCodeSeg					'CSeg'	  // N/A  : Code segment library


// ------------------------------------------------------------------
// ROM tokens
// ------------------------------------------------------------------
#define hsFileCFFSInfoTok1				'Ffs1'	  // Yes : Flash file system info
												  //	    ROM Token string; see
												  //	    FfsMgrPrv.h for format
												  //	    details (FOR HANDSPRING, INC.
												  //	    USE ONLY!)

#define hsFileCFfsCrType				'Ffs2'	  // Yes : ascii string of colon-separated
				// creator and type of the module's Flash File System Manager. This value
				// MUST be unique for each Flash File System Manager implementation. This
				// token provides a level of indirction that allows applications to locate
				// the provider of FfsMgr services on any module that supports this
				// architecture and is fully compatible with the Handspring, Inc.'s FfsMgr
				// API. The format is "CCCC:TTTT", where CCCC is the unique 4-character
				// creator ID of your FfsMgr registred with Palm Computing, and TTTT is the
				// 4-character type id of your FfsMgr (must be mixed case ascii). (for example,
				// the reserved value used by Handspring, Inc. implementation is "FfMg:CSeg")


#define	hsFileCCardAccessTime			'HsAT'	  // Yes : ascii decimal string of
												  //	    access time in nano-seconds

#define	hsFileCCardWelcomeDuringReset	'HsWR'	  // Yes : actual value ignored. Presence
												  //		means launch welcome app during reset

#define	hsFileCCardWelcomeAppTypeCreator 'HsWT'	  // Yes : type & creator of card welcome app
												  // ONLY AVAILABLE IN VERSION 3.5 AND LATER
												  //       Ex: 	-tokStr "HsWT" "applMYAP"

#define hsFileCSmartSmallromToken		'HsSR'	  // Yes: defines that the given smallrom is
												  //      smart about running from RAM--it will
												  //      auto-drop into the appropriate debugger.

#define hsFileCBuiltinHsExtToken		'HsHX'	  // Yes: defines what version of HSExtension is
												  //	  builtin to the ROM.
												  // ONLY AVAILABLE IN HSEXT 3.5 VERSION 1.2 AND LATER

#define hsBluetoothIdToken              'BTid'    // Yes: this is the 6 byte Bluetooth ID stored in the ROM token area
                                                  //   first available in the Treo 650 project.

#define htcManufacturingInfoToken       'HTCM'    // yes - HTC's manufacturing info token ID as specified
                                                  // 6/4/04 by Kim Horng, HTC MFG Test Engineering II Dep.,
                                                  // Tel : (03)3753252  ext : 5455, Mail : Kim_Horng@htc.com.tw
#define pmLimitLocaleToken				'LLid'	  // yes : Set this token to a certain locale to keep the
												  //       language picker from extracting other locales...
#define pmDebugLockoutToken				'Dblk'	  // ??? : This token is used by the DAL to tell if debugging
												  //	   is locked out (in other words, a password is set).

//................................................................
// Miscellaneous database types
//................................................................

#define hsFileTModemSetupPatch			'HsMp'	  // N/A  : The Patch for the Modem Module's
												  //	     CardSetup app has this type and
												  //	     the same creator as the Modem
												  //	     Module's serial library (hsFileCModemSerLib)


#define hsCompressedDbCreatorId			'Lz7C'	  // Yes: Creator and type id's of the compressed
#define hsCompressedDbTypeId			'Lz7C'	  //	  databases

#define hsKbdLayoutDbTypeCreator		'KbLy'	  // Yes: Type and creator of database containing
												  //	  keyboard layout resources

#define hsFileCTextEncodingConverterLib 'TecL'    // Yes: TextEncodingConverter database creator
#define hsFileCTextEncodingConverterTable 'TecD'  // Yes: TextEncodingConverter conversion table database type
#define hsRscTextEncodingConverterTable 'TecT'    // Yes: TextEncodingConverter conversion table resource type

#define hsFileCPowerKey					'000P'	  // Yes: Virtual app that can be mapped to an
												  //	  app key to make it act like the power key

#define hsFileCKeyguard					'kLoc'	  // No:  Virtual app that can be mapped to an
												  //	  app key to make it lock the keyboard

#define pmSlotDriverActivityIndicator   'PmAI'    // Yes: Use this ID for the TFFS slot driver Custom Control
                                                  //      API to turn the activity indicator on or off.

#define pmROMTokenWrite                 'TOKw'    // Yes: Use this ID for the TFFS slot driver Custom Control
                                                  //      API to write ROM Tokens

#define pmROMTokenRead                  'TOKr'    // Yes: Use this ID for the TFFS slot driver Custom Control
                                                  //      API to read ROM Tokens

//................................................................
// Notification types
//................................................................
#define hsNotifyRemSleepRequestEvent	  'REMq'  // Yes: Notification sent after sysNotifySleepRequestEvent
												  //	  to determine if anyone needs to continue processing
												  //	  with the screen turned off. The details parameter
												  //	  is SleepEventParamType, in which deferSleep should
												  //	  be incremented if REM sleep is required.

#define hsNotifyRemSleepEvent			  'REMs'  // Yes: Notification sent to indicate that REM sleep is starting
												  //	  (because hsNotifyRemSleepRequestEvent was deferred).
												  //	  The screen is already turned off and the autooff timer has
												  //	  been primed to expire soon. That will cause the cycle to
												  //	  restart with sysNotifySleepRequestEvent and
												  //	  hsNotifyRemSleepRequestEvent both being sent again.

#define hsNotifySetInitialFldStateEvent	  'HsFg'  // Yes: Notification sent so app can set the initial shift state
												  //	  of a field that just received the focus.

#define	pmNotifyGetNewMsgCountFromHelper  'gMgC'  // Yes: Notification sent by an application to prompt messaging
												  //	  apps registered for this notification to broadcast their
												  //	  new/unread message counts.  The details parameter is
												  //	  PmGetMsgCountNotifyParamType (defined in DefaultHelperLibTypes.h).

#define pmNotifyBroadcastNewMsgCount	  'bMgC'  // Yes: Notification sent by an application in response to
												  //	  a pmNotifyGetNewMsgCountFromHelper notification.
												  //	  The details parameter is PmBroadcastNewMsgCountNotifyParamType
												  //	   (defined in DefaultHelperLibTypes.h).

#define pmNotifyBrightModeKeyEvent		  'PmBM'  // Yes: Notification sent by HsExtensions when the key event it receives
												  //	  will kick the device out of dark mode and into bright mode.
												  //	  HsExtensions will eat-up this key to prevent further processing
												  //	  unless this notification is set as handled.

#define pmNotifyTraceSettingsChangedEvent (hsFileCTraceViewer)
												  // Yes: Notification sent by Trace Viewer app (or another client that
												  //	  changes trace settings) to let interested parties know
												  //	  that trace settings *may* have changed.  No parameter
												  //	  is presently defined, so notification detailsP will
												  //	  be NULL for now, but may be non-NULL in the future.

//................................................................
// System Resource types and IDs
//................................................................
#define	hsRscTKbdChrLookupData			'HsKb'	  // Yes: type of a HEX resource that is a table
												  //      used by the keyboard driver for looking up the
												  //	  character and modifier of a key.

// Note: These resource ID's have changed for OS5 based products
// since there is now a 'shift' table in addition to the normal
// and 'option' tables, and there are minor differences in the
// definition of the data.

//#define hsRscKbdChrDataID				1000
//#define hsRscKbdChrOptDataID			1001

#define hsRscKbdChrDataID				2000	  // ID of resource containing regular character lookup
												  //  table

#define hsRscKbdChrShiftDataID			2001	  // ID of resource containing shift character lookup
												  //  table

#define hsRscKbdChrOptDataID			2002	  // ID of resource containing option character lookup
												  //  table


#define	hsRscTKbdKeyCodeLookupData		'HsKc'	  // Yes: type of a HEX resource that is a table
												  //      used by the keyboard driver for looking up the
												  //	  keyCode of a key.
#define hsRscKbdKeyCodeDataID			2000	  // ID of resource containing keyCodes

// OptLock resources are depricated... now there's logic to exclude
// command keys from being generated from opt-lock
//#define	hsRscTOptLockKeys				'HsLk'	  // Yes: type of a HEX resource that specifies which
												  //  keys are affected by option lock state

//#define hsRscOptLockKeysID				1000	  // ID of resource that specifies which keys respect
												  //  option lock

//#define hsRscOptLockKeysExt1ID			1001	  // ID of resource that specifies which keys of 1st
												  //  extension group respect option lock

//#define hsRscOptLockKeysExt2ID			1002	  // ID of resource that specifies which keys of 2nd
												  //  extension group respect option lock

#define hsRscTPostProcessLists			'HsPp'	  // Yes: type of a HEX resource that defines lists
												  //	  used by a post-processing mechanism to
												  //	  convert an entered character into a related
												  //	  character or string.

//#define hsRscPostProcessList			19000	  // Using a Palm reserved ID (an ID > 10000) to prevent
												  //  resource conflicts.  Since we've registered
												  //  the HsPp id with Palm, we should not have resource
												  //  conflicts with Palm.
												  //  OBSOLETE: Now that we have a reserved handspring
												  //  range, using number in this range (defined in
												  //  HsResourceIDs.h)

//#define hsRscTWordCorrectionData		'HsWc'	  // Yes: type of a HEX resource that defines the words
												  //	  used by the word correction feature.
												  //	  OBSOLETE: using string list instead

//#define hsRscWordCorrectionDataID		19000	  // Using a Palm reserved ID (an ID > 10000) to prevent
												  //  resource conflicts.  Since we've registered
												  //  the HsWc id with Palm, we should not have resource
												  //  conflicts with Palm.
												  //  OBSOLETE: Now that we have a reserved handspring
												  //  range, using number in this range (defined in
												  //  HsResourceIDs.h)

//#define hsRscTButtonMappingData		'HsBm'	  // Yes: type of a HEX resource that defines the strings
												  //  used to map keyboard buttons to UI buttons.  This
												  //  OBSOLETE: using string list instead

#define hsRscTNavFocusColor				'HsFc'	  // Yes: type of a HEX resource that defines the focus
												  //  border colors.

#define hsRscTutorial             'HsTM'

#if 0 // DOLATER... THESE ARE LEGACY DEFINITIONS THAT NO LONGER MAKE SENSE IN PALM OS 5.X. DELETE ONCE
	  // BUILD IS VERIFIED.

// Note: The following must match the types/IDs in the HAL makefile

#define hsRscTHalDriver					'HwDv'	  // N/A: type of code resource containing a Hal driver

#define hsRscFirstHalDriverID			19001	  // The ID at which HAL starts loading sequentially
												  //  numbered drivers

#define hsRscHalKbdDriverID				19001	  // Using a Palm reserved ID (an ID > 10000) which
												  //  sequentially follows the main HAL boot resource.

#define hsRscTHsExPatchModule			'HsPt'	  // Yes: type of code resource containing HsExtensions
												  //  patch. Deliberately the same as hsFileTHandspringPatch

#define hsRscFirstHsExModuleID			19000	  // Using a Palm reserved ID (an ID > 10000) which
												  //  sequentially follows the main HAL boot resource.

#endif

//................................................................
// 3rd party applications
//................................................................
// These are external applications that are included in the ROM.  They are not written or
// compiled internally, but are included here so they can be referenced by internal
// applications/libraries.

#define	exFileCArcSoftCamera			'CmLo'	  // The ArcSoft Camera application

#define exFileCArcSoftMedia				'Foto'	  // The ArcSoft Media viewer application


//................................................................
// ATTENTION:
// ARM Module IDs are now located in Incs/Handspring/ARM/System/Prv
//................................................................

#endif	  // __HS_CREATORS_H__ -- include once




