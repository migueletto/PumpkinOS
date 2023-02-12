/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SystemResources.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for both PalmRez and the C Compiler. This file contains
 *  equates used by both tools. When compiling using the C compiler
 *  the variable RESOURCE_COMPILER must be defined.
 *
 *****************************************************************************/

#ifndef 	__SYSTEMRESOURCES_H__
#define	__SYSTEMRESOURCES_H__

//-----------------------------------------------------------
// This section is common to both the C and Resource Compiler
//-----------------------------------------------------------

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
//	apps shall be all-lower case (they are defined below).  Other
//	file/database types must be mixed-case,
//	or all caps.  These other types are internal to the applications, and
//	therefore the system is unconcerned with their exact values.
//................................................................
#define	sysFileCSystem					'psys'	// Creator type for System files
#define	sysFileCOEMSystem				'poem'	// Creator type for OEM System files
#define	sysFileCPalmDevice				'pdvc'	// Creator type for Palm Devices, analogous to 'poem'

#define  sysFileCGraffiti				'graf'	// Creator type for Graffiti databases
#define	sysFileCSystemPatch				'ptch'	// Creator for System resource file patches

#define	sysFileCCalculator				'calc'	// Creator type for Calculator App
#define	sysFileCSecurity				'secr'	// Creator type for Security App
#define	sysFileCPreferences				'pref'	// Creator type for Preferences App
#define	sysFileCAddress					'addr'	// Creator type for Address App
#define	sysFileCToDo					'todo'	// Creator type for To Do App
#define	sysFileCDatebook				'date'	// Creator type for Datebook App
#define	sysFileCMemo					'memo'	// Creator type for MemoPad App
#define	sysFileCSync					'sync'	// Creator type for HotSync App
#define	sysFileCMemory					'memr'	// Creator type for Memory App
#define	sysFileCMail					'mail'	// Creator type for Mail App
#define	sysFileCExpense					'exps'	// Creator type for Expense App
#define	sysFileCLauncher				'lnch'	// Creator type for Launcher App
#define	sysFileCClipper					'clpr'	// Creator type for clipper app.
#define	sysFileCDial					'dial'	// Creator type for dial app.
#define	sysFileCSetup					'setp'	// Creator type for setup app.
#define  sysFileCActivate				'actv'	// Creator type for activation app.
#define  sysFileCGenenicActivate		'gafd'	// New Generic Activation application working for all Palm models
#define  sysFileCFlashInstaller			'fins'	// Creator type for FlashInstaller app.
#define	sysFileCRFDiag					'rfdg'	// Creator type for RF diagnostics app.
#define	sysFileCMessaging				'msgs'	// Creator type for Messaging App
#define	sysFileCModemFlashTool			'gsmf'	// Creator type for Palm V modem flash app.
#define	sysFileCJEDict					'dict'	// Creator type for JEDict app.
#define sysFileCWordLookup				'word'	// Creator type for Word Lookup app.
#define	sysFileHotSyncServer			'srvr'	// Creator type for HotSync(R) Server app.
#define	sysFileHotSyncServerUpdate		'hssu'	// Creator type for HotSync(R) Server update app.
#define	sysFileCCardInfo				'cinf'	// Creator type for the Card info app.
#define	sysFileCPhone					'fone'	// Creator type for integrated phone components.
#define	sysFileCSmsMessenger			'smsm'	// Creator type for SMS messenger app.
#define	sysFileCNetTrace				'nett'	// Creator type for Net Trace StdIO app.
#define	sysFileCPing					'ping'	// Creator type for Ping StdIO app.
#define	sysFileCLanguagePicker			'lpkr'	// Creator type for Language Picker app.

// The following apps are manufacturing, calibration and maintenance related
#define	sysFileCMfgExtension			'mfx1'	// Creator type for Manufacturing Extension.
#define	sysFileCMfgFunctional			'mfgf'	// Creator type for Manufacturing functional test autostart app.
#define sysFileCMfgCalibration			'mfgc'	// Creator type for Manufacturing radio calibration app.

// Demo Apps
#define	sysFileCGraffitiDemo			'gdem'	// Creator type for Graffiti Demo
#define	sysFileCMailDemo				'mdem'	// Creator type for Mail Demo

#define	sysFileCFirstApp				sysFileCPreferences	// Creator type for First App after reset
#define	sysFileCAltFirstApp				sysFileCSetup		// Creator type for First alternate App after reset (with hard key pressed)
#define	sysFileCDefaultApp				sysFileCPreferences	// Creator type for Default app
#define	sysFileCDefaultButton1App		sysFileCDatebook	// Creator type for dflt hard button 1 app
#define	sysFileCDefaultButton2App		sysFileCAddress		// Creator type for dflt hard button 2 app
#define	sysFileCDefaultButton3App		sysFileCToDo		// Creator type for dflt hard button 3 app
#define	sysFileCDefaultButton4App		sysFileCMemo		// Creator type for dflt hard button 4 app
#define	sysFileCDefaultCalcButtonApp	sysFileCCalculator	// Creator type for dflt calc button app
#define	sysFileCDefaultCradleApp		sysFileCSync		// Creator type for dflt hot sync button app
#define	sysFileCDefaultModemApp			sysFileCSync		// Creator type for dflt modem button app
#define	sysFileCDefaultAntennaButtonApp	sysFileCLauncher	// Creator type for dflt antenna up button app
#define	sysFileCNullApp					'0000'				// Creator type for non-existing app 
#define	sysFileCSimulator				'\?\?\?\?'	// Creator type for Simulator files (app.tres, sys.tres)
													//	'????' does not compile with VC++ (Elaine Server)

#define	sysFileCDigitizer				'digi'	// Creator type for Digitizer Panel
#define	sysFileCDateTime				'dttm'	// Creator type for Date & Time Panel
#define	sysFileCGeneral					'gnrl'	// Creator type for General Panel
#define	sysFileCFormats					'frmt'	// Creator type for Formats Panel
#define	sysFileCShortCuts				'shct'	// Creator type for ShortCuts Panel
#define	sysFileCButtons					'bttn'	// Creator type for Buttons Panel
#define	sysFileCOwner					'ownr'	// Creator type for Owner Panel
#define	sysFileCModemPanel				'modm'	// Creator type for Modem Panel
#define	sysFileCDialPanel				'dial'	// Creator type for Dial Panel
#define	sysFileCNetworkPanel			'netw'	// Creator type for Network Panel
#define  sysFileCWirelessPanel			'wclp'	// Creator type for the Web Clipping  Panel.
#define	sysFileCUserDict       			'udic'	// Creator type for the Japanese FEP's UserDict panel.
#define	sysFileCPADHtal					'hpad'	// Creator type for PAD HTAL library
#define	sysFileCTCPHtal					'htcp'	// Creator type for TCP HTAL library
#define	sysFileCRELHtal					'hrel'	// Creator type for REL HTAL library
#define	sysFileCMineHunt				'mine'	// Creator type for MineHunt App
#define	sysFileCPuzzle15				'puzl'	// Creator type for Puzzle "15" App
#define	sysFileCOpenLibInfo				'olbi'	// Creator type for Feature Manager features
																// used for saving open library info under PalmOS v1.x
#define	sysFileCHwrFlashMgr				'flsh'	// Creator type for HwrFlashMgr features
#define	sysFileCPhonePanel				'phop'	// Creator type for Phone Panel

// Added by BGT, 08/01/2000
#define sysFileDRAMFixOriginal			'mmfx'	// Creator type for 1.0 DRAM Fix
#define sysFileDRAMFix					'dmfx'	// Creator type for 1.0.3 DRAM Fix and later

// Libraries.  If the resource used by these are expected to be treated as part of
// the system's usage then the Memory app must be changed.
#define	sysFileTLibrary					'libr'	// File type of Shared Libraries
#define	sysFileTLibraryExtension		'libx'	// File type of library extensions

#define	sysFileCNet						'netl'	// Creator type for Net (TCP/IP) Library
#define	sysFileCRmpLib					'netp'	// Creator type for RMP Library (NetLib plug-in)
#define	sysFileCINetLib 				'inet'	// Creator type for INet Library
#define	sysFileCSecLib 					'secl'	// Creator type for Ir Library
#define	sysFileCWebLib 					'webl'	// Creator type for Web Library
#define	sysFileCIrLib 					'irda'	// Creator type for Ir Library
#define	sysFileCBtLib					'blth'	//	Creator type for Bt Library
#define	sysFileCBtTransLib				'bttx'	// Creator for the Bt HCI Transport library
#define	sysFileCLocalLib				'locl'	// Creator type for Local exchange library
#define	sysFileCLz77Lib					'lz77'	// Creator type for LZ77 Library (Registered)
#define	sysFileCSmsLib					'smsl'	// Creator type for SMS Library
#define	sysFileCBtExgLib				'btex'	// Creator type for Bluetooth Exchange Library
#define	sysFileCPdiLib  				'pdil'	// Creator type for PDI Library
#define	sysFileCTelMgrLib  				'tmgr'	// Creator type for Telephony Manager Library
#define sysFileCTelTaskSerial 			'spht'  // Creator type for Serial Telephony Task
#define sysFileTTelTaskSerial 			'ttsk'  // File type for Serial Telephony Task
#define sysFileCBaseATDriver			'patd'  // Creator type for the Base AT Driver
#define sysFileTBaseATDriver			'patd'  // File type for the Base AT Driver (same as Creator)
#define sysFileCStandardGsm				'stgd'  // Creator type for the Standard GSM Driver
#define	sysFileTPhoneDriver				'pdrv'	// File type for Phone Drivers of Telephony Task

#define	sysFileCSerialMgr				'smgr'	// Creator for SerialMgrNew used for features.
#define	sysFileCSerialWrapper			'swrp'	// Creator type for Serial Wrapper Library.
#define	sysFileCIrSerialWrapper			'iwrp'	// Creator type for Ir Serial Wrapper Library.
#define	sysFileCTextServices			'tsml'	// Creator type for Japanese Kana-Kanji FEP (Text Services Library).
#define sysFileCPinyinFep				'piny'	// Creator type for Simplified Chinese Pinyin FEP (Text Services Library) & panel.

#define	sysFileTUartPlugIn				'sdrv'	// File type for SerialMgrNew physical port plug-in.
#define	sysFileTVirtPlugin				'vdrv'	// Flir type for SerialMgrNew virtual port plug-in.
#define	sysFileCUart328 				'u328'	// Creator type for '328 UART plug-in
#define	sysFileCUart328EZ				'u8EZ'	// Creator type for '328EZ UART plug-in
#define	sysFileCUart650 				'u650'	// Creator type for '650 UART plug-in
#define	sysFileCVirtIrComm				'ircm'	// Creator type for IrComm virtual port plug-in.
#define	sysFileCVirtRfComm				'rfcm'	// Creator type for RfComm (Bluetooth) virtual port plug-in.
#define	sysFileCBtConnectPanelHelper	'btcp'	// Creator type for the Bt Connection Panel helper app.

#define	sysFileCPDIUSBD12				'pusb'	// Creator type for USB database
#define	sysPortUSBDesktop				'usbd'	// Creator type for USB Desktop 
#define	sysPortUSBConsole				'usbc'	// Creator type for USB Console
#define	sysPortUSBPeripheral			'usbp'	// Creator type for USB Peripheral

#define	sysFileCExternalConnector		'econ'	// Creator type for the external connector

#define	sysFileCExpansionMgr			'expn'	// Creator of Expansion Manager extension database
#define	sysFileCVFSMgr					'vfsm'	// Creator code for VFSMgr...
#define	sysFileCFATFS					'fatf'	// Creator type for FAT filesystem library

#define	sysFileCSdSpiCard				'sdsd'	// Creator type for Slot Driver: SD bus, SPI mode, memory cards

#define sysFileCSlotDriverPnps			'pnps'	//Creator ID for Pnps Serial Peripheral Slot Driver 

#define sysFileCSoundMgr				'sndm'  // Creator type for Sound Manager features

#define sysFileCMultimedia				'mdia'	// Creator type for C Multimedia APIs (MMxxx)

#define	sysFileTSystem					'rsrc'	// File type for Main System File
#define	sysFileTSystemPatch				'ptch'	// File type for System resource file patches
#define	sysFileTProductUpdate			'pupd'	//	File type for Product Update patches
#define	sysFileTKernel					'krnl'	// File type for System Kernel (AMX)
#define	sysFileTBoot					'boot'	// File type for SmallROM System File
#define	sysFileTSmallHal				'shal'	// File type for SmallROM HAL File
#define	sysFileTBigHal					'bhal'	// File type for Main ROM HAL File
#define	sysFileTSplash					'spls'	// File type for Main ROM Splash File
#define	sysFileTUIAppShell				'uish'	// File type for UI Application Shell
#define	sysFileTOverlay					'ovly'	// File type for UI overlay database
#define	sysFileTExtension				'extn'	// File type for System Extensions
#define	sysFileTApplication				'appl'	// File type for applications
#define	sysFileTPanel					'panl'	// File type for preference panels
#define	sysFileTSavedPreferences		'sprf'	// File type for saved preferences
#define	sysFileTPreferences				'pref'	// File type for preferences
#define	sysFileTMidi					'smfr'	// File type for Standard MIDI File record databases
#define	sysFileTpqa						'pqa '	// File type for the PQA files.
#define	sysFileTLocaleModule			'locm'	// File type for locale modules.
#define	sysFileTActivationPlugin		'actp'	// File type for activation plug-ins.
#define	sysFileTUserDictionary			'dict'	// File type for input method user dictionary.
#define	sysFileTLearningData			'lean'	// File type for input method learning data.

#define	sysFileTGraffitiMacros			'macr'	//  Graffiti Macros database

#define	sysFileTHtalLib					'htal'	//  HTAL library

#define  sysFileTExgLib					'exgl'	// Type of Exchange libraries

#define	sysFileTSlotDriver				'libs'	// File type for slot driver libraries
#define	sysFileTFileSystem				'libf'	// File type for file system libraries

#define	sysFileTFileStream				'strm'	//  Default File Stream database type

#define	sysFileTTemp					'temp'	//  Temporary database type; as of Palm OS 4.0, the
																//  system WILL automatically delete any db's of
																//  this type at reset time (however, apps are still
																//  responsible for deleting the ones they create
																//  before exiting to protect valuable storage space)

// Begin Change - BGT 03/21/2000

#define	sysFileTNetworkPanelPlugin		'nppi'	// File type for network preference panel plug-ins

// End Change - BGT 03/21/2000

#define	sysFileTScriptPlugin			'scpt'	// File type for plugin to the Network Panel to 
																//extend scripting capabilities.

#define	sysFileTStdIO					'sdio'	// File type for standard IO apps 

#define  sysFileTSimulator             	'\?\?\?\?'  // File type for Simulator files (app.tres, sys.tres)
                                    				// '????' does not compile with VC++ (Elaine Server)

//................................................................
// Resource types and IDs
//................................................................
#define	sysResTBootCode					'boot'	// Resource type of boot resources
#define	sysResIDBootReset				10000		// Reset code 
#define	sysResIDBootInitCode			10001		// Init code 
#define	sysResIDBootSysCodeStart		10100		// System code resources start here
#define	sysResIDBootSysCodeMin			10102		// IDs 'Start' to this must exist!!
#define	sysResIDBootUICodeStart			10200		// UI code resources start here
#define	sysResIDBootUICodeMin			10203		// IDs 'Start' to this must exist!!
#define	sysResIDProdUpdCodeStart		10300		//	Product Update code resources start here

#define	sysResIDBootHAL					19000		// HAL initial code resource (from HAL.prc)
#define	sysResIDBootHALCodeStart		19100		// start of additional high-level HAL code resources 

#define	sysResIDBitmapSplash			19000		// ID of (boot) splash screen bitmap
#define	sysResIDBitmapConfirm			19001		// ID of hard reset confirmation bitmap

#define	sysResTAppPrefs					'pref'	// Resource type of App preferences resources
#define	sysResIDAppPrefs					0			// Application preference

#define	sysResTExtPrefs					'xprf'	// Resource type of extended preferences
#define	sysResIDExtPrefs					0			// Extended preferences

#define	sysResTAppCode					'code'	// Resource type of App code resources
#define	sysResTProductUpdateCode		'pupd'	// Resource type of Product Update code resources
#define	sysResTAppGData					'data'	// Resource type of App global data resources

#define	sysResTExtensionCode			'extn'	// Resource type of Extensions code
#define	sysResTExtensionOEMCode			'exte'	// Resource type of OEM Extensions code

#define	sysResTFeatures					'feat'	// Resource type of System features table
#define	sysResIDFeatures				10000		// Resource ID of System features table
#define	sysResIDOverlayFeatures			10001		// Resource ID of system overlay feature table.

#define	sysResTLibrary					'libr'	// Resource type of System Libraries
//#define	sysResIDLibrarySerMgr328		10000		// Dragonball (68328) UART
//#define	sysResIDLibrarySerMgr681		10001		// 68681 UART
//#define	sysResIDLibraryRMPPlugIn		10002		// Reliable Message Protocol NetLib Plug-in

#define	sysResTSilkscreen				'silk'	// Resource type of silkscreen info.

#define	sysResTGrfTemplate				'tmpl'	// Graffiti templates "file"
#define	sysResIDGrfTemplate				10000		// Graffiti templates "file" ID
#define	sysResTGrfDictionary			'dict'	// Graffiti dictionary "file"
#define	sysResIDGrfDictionary			10000		// Graffiti dictionary "file" ID
#define	sysResIDGrfDefaultMacros		10000		// sysResTDefaultDB resource with Graffiti Macros database

#define	sysResTDefaultDB				'dflt'	// Default database resource type
#define	sysResIDDefaultDB				1			// resource ID of sysResTDefaultDB in each app

#define	sysResTCompressedDB				'cpdb'	// Compressed database resource type
#define	sysResIDCompressedDB			10000		// resource ID of first sysResTCompressedDB

#define	sysResTErrStrings				'tSTL'	// list of error strings
#define	sysResIDErrStrings				10000		// resource ID is (errno>>8)+sysResIDErrStrings

#define	sysResIDOEMDBVersion			20001		// resource ID of "tver" and "tint" versions in OEM stamped databases

#define	sysResTButtonDefaults			'hsbd'	// Hard/soft button default apps
#define	sysResIDButtonDefaults			10000		// resource ID of system button defaults resource

// System Sounds

#define sysResTSound					'wave'	// Resource type for sound resources.

// Resource IDs for various system sounds.
// These resources are contained in the "device resources" database.
#define sysResIDSndInfo					10000
#define sysResIDSndWarning				10001
#define sysResIDSndError				10002
#define sysResIDSndStartUp				10003
#define sysResIDSndAlarm				10004
#define sysResIDSndConfirmation			10005
#define sysResIDSndClick				10006

#define sysResIDSndCardInsert			10010
#define sysResIDSndCardRemove			10011

#define sysResIDSndSyncStart			10020
#define sysResIDSndSyncStop				10021

// System Preferences
#define	sysResTSysPref					sysFileCSystem
#define	sysResIDSysPrefMain				0			// Main preferences
#define	sysResIDSysPrefPassword			1			// Password
#define	sysResIDSysPrefFindStr			2			// Find string
#define	sysResIDSysPrefCalibration		3			// Digitizer calibration.
#define	sysResIDDlkUserInfo				4			// Desktop Link user information.
#define	sysResIDDlkLocalPC				5			// Desktop Link local PC host name
#define	sysResIDDlkCondFilterTab		6			// Desktop Link conduit filter table
#define	sysResIDModemMgrPref			7			// Modem Manager preferences
#define	sysResIDDlkLocalPCAddr			8			// Desktop Link local PC host address
#define	sysResIDDlkLocalPCMask			9			// Desktop Link local PC host subnet mask

// These prefs store parameters to pass to an app when launched with a button
#define	sysResIDButton1Param			10			// Parameter for hard button 1 app
#define	sysResIDButton2Param			11			// Parameter for hard button 2 app
#define	sysResIDButton3Param			12			// Parameter for hard button 3 app
#define	sysResIDButton4Param			13			// Parameter for hard button 4 app
#define	sysResIDCalcButtonParam			14			// Parameter for calc button app
#define	sysResIDCradleParam				15			// Parameter for hot sync button app
#define	sysResIDModemParam				16			// Parameter for modem button app
#define	sysResIDAntennaButtonParam		17			// Parameter for antenna up button app

// New for Color, user's color preferences
#define	sysResIDPrefUIColorTableBase	17			// base + depth = ID of actual pref
#define	sysResIDPrefUIColorTable1		18			// User's UI colors for 1bpp displays
#define	sysResIDPrefUIColorTable2		19			// User's UI colors for 2bpp displays
#define	sysResIDPrefUIColorTable4		21			// User's UI colors for 4bpp displays
#define	sysResIDPrefUIColorTable8		25			// User's UI colors for 8bpp displays

#define	sysResIDSysPrefPasswordHint		26			// Password hint
#define sysResIDSysPrefPasswordHash 	27			// Password hash (MD5)


// FlashMgr Resources - old
#define	sysResTFlashMgr					'flsh'
#define	sysResIDFlashMgrWorkspace		1			// RAM workspace during flash activity

// FlashMgr Resources - new
#define	sysResTHwrFlashIdent			'flid'	// Flash identification code resource
#define	sysResIDHwrFlashIdent			10000		// Flash identification code resource

#define	sysResTHwrFlashCode				'flcd'	// Flash programming code resource
																// (resource ID determined by device type)

// FontMgr Resources
#define	sysResTFontMap					'fntm'	// Font map resource

// OEM Feature type and id.
#define	sysFtrTOEMSys					sysFileCOEMSystem
#define	sysFtrIDOEMSysHideBatteryGauge	1

// Onscreen keyboard features
#define	sysFtrTKeyboard					'keyb'
#define	sysFtrIDKeyboardActive			1			// Boolean value, true => keyboard is active.
																// Currently only used for Japanese.

// Activation status values.
#define	sysActivateStatusFeatureIndex	1
#define	sysActivateNeedGeorgeQuery		0
#define	sysActivateNeedMortyQuery		1
#define	sysActivateFullyActivated		2

#define sysMaxUserDomainNameLength		64

// Current clipper feature indeces
#define	sysClipperPQACardNoIndex		1
#define	sysClipperPQADbIDIndex			2

//-----------------------------------------------------------
// This section is only valid when running the resource compiler
//
// Actually, this section is obsolete.  Instear, .r files should
// inlude SysResTypes.rh to get these definitions.
//
//-----------------------------------------------------------

#ifdef RESOURCE_COMPILER

#include <SysResTypes.rh>

#endif


#endif // __SYSTEMRESOURCES_H__
