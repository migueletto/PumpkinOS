/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Preferences.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Header for the system preferences
 *
 *****************************************************************************/

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <DateTime.h>
#include <Localize.h>
#include <SystemMgr.h>
#include <LocaleMgr.h>	// CountryType, kMaxCountryNameLen, etc.
#include <AttentionMgr.h>		// AttnFlagsType

/***********************************************************************
 *	Constants
 ***********************************************************************/

#define noPreferenceFound			-1

// Preference version constants
#define preferenceDataVer2			2 // Palm OS 2.0
#define preferenceDataVer3			3 // Palm OS 3.0
#define preferenceDataVer4			4 // Palm OS 3.1
#define preferenceDataVer5			5 // Palm OS 3.2a
#define preferenceDataVer6			6 // Palm OS 3.2b/3.3
#define preferenceDataVer8			8 // Palm OS 3.5
#define preferenceDataVer9			9 // Palm OS 4.0
#define preferenceDataVer10		   10 // Palm OS 5.0 (ARM Port)
#define preferenceDataVer11		   11 // PalmOS 5.1 (Banzai)
#define preferenceDataVer12		   12 // PalmOS 5.3

// Be SURE to update "preferenceDataVerLatest" when adding a new prefs version...
#define preferenceDataVerLatest	preferenceDataVer12


#define defaultAutoOffDuration		2								//	minutes
#define defaultAutoOffDurationSecs	(2 * minutesInSeconds)	// seconds

#define peggedAutoOffDuration			0xFF							// minutes (UInt8)
#define peggedAutoOffDurationSecs	0xFFFF						// seconds (UInt16)

#define defaultAutoLockType			never 						//Never auto lock device
#define defaultAutoLockTime			0								
#define defaultAutoLockTimeFlag		0

// Obsolete after V20
#if EMULATION_LEVEL == EMULATION_NONE
	#define defaultSysSoundLevel		slOn
	#define defaultGameSoundLevel		slOn
	#define defaultAlarmSoundLevel	slOn
#else	// EMULATION_LEVEL != EMULATION_NONE
	#define defaultSysSoundLevel		slOff
	#define defaultGameSoundLevel		slOff
	#define defaultAlarmSoundLevel	slOn
#endif

	
#if EMULATION_LEVEL == EMULATION_NONE
	#define defaultSysSoundVolume		sndMaxAmp
	#define defaultGameSoundVolume	sndMaxAmp
	#define defaultAlarmSoundVolume	sndMaxAmp
#else	// EMULATION_LEVEL != EMULATION_NONE
	#define defaultSysSoundVolume		0
	#define defaultGameSoundVolume	0
	#define defaultAlarmSoundVolume	sndMaxAmp
#endif

typedef enum
	{
	unitsEnglish = 0,		// Feet, yards, miles, gallons, pounds, slugs, etc.
	unitsMetric				//	Meters, liters, grams, newtons, etc.
	} MeasurementSystemType;


//	These sound levels must corrospond to positions in the popup lists
//	used by the preferences app.  These are made obsolete after V20.  The
// loudness of the sound is now represented as a number from 0 to sndMaxAmp.
typedef enum {
	slOn = 0,
	slOff = 1
	} SoundLevelTypeV20;

//	Device Automatic Locking options.
typedef enum {
   never = 0,					//Auto-Lock disabled.
   uponPowerOff,				// Auto lock when the device powers off.
   atPresetTime,				//Auto lock at HH:MM every day.
   afterPresetDelay				//Auto lock after x minutes or hours.
   } SecurityAutoLockType;


// The number format (thousands separator and decimal point).  This defines
// how numbers are formatted and not neccessarily currency numbers (i.e. Switzerland).
typedef enum {
	alOff,									// Never show an animation
	alEventsOnly,							// Show an animation for an event
	alEventsAndRandom,					// Also show random animation
	alEventsAndMoreRandom				// Show random animations more frequently
	} AnimationLevelType;


// Handedness states
#define prefRightHanded		0
#define prefLeftHanded		1


typedef enum 
	{
	prefVersion,
	prefCountry,
	prefDateFormat,
	prefLongDateFormat,
	prefWeekStartDay,
	prefTimeFormat,
	prefNumberFormat,
	prefAutoOffDuration,					// prefAutoOffDurationSecs is now preferred (prefAutoOffDuration is in minutes)
	prefSysSoundLevelV20,				// slOn or slOff - error beeps and other non-alarm/game sounds
	prefGameSoundLevelV20,				// slOn or slOff - game sound effects 
	prefAlarmSoundLevelV20,				// slOn or slOff - alarm sound effects 
	prefHidePrivateRecordsV33,
	prefDeviceLocked,
	prefLocalSyncRequiresPassword,
	prefRemoteSyncRequiresPassword,
	prefSysBatteryKind,
	prefAllowEasterEggs,
	prefMinutesWestOfGMT,				// deprecated old unsigned minutes EAST of GMT
	prefDaylightSavings,					// deprecated old daylight saving time rule
	prefRonamaticChar,
	prefHard1CharAppCreator,			// App creator for hard key #1
	prefHard2CharAppCreator,			// App creator for hard key #2
	prefHard3CharAppCreator,			// App creator for hard key #3
	prefHard4CharAppCreator,			// App creator for hard key #4
	prefCalcCharAppCreator,				// App creator for calculator soft key
	prefHardCradleCharAppCreator,		// App creator for hard cradle key
	prefLauncherAppCreator,				// App creator for launcher soft key
	prefSysPrefFlags,		
	prefHardCradle2CharAppCreator,	// App creator for 2nd hard cradle key
	prefAnimationLevel,

	// Additions for PalmOS 3.0:
	prefSysSoundVolume,					// actual amplitude - error beeps and other non-alarm/game sounds
	prefGameSoundVolume,					// actual amplitude - game sound effects
	prefAlarmSoundVolume,				// actual amplitude - alarm sound effects
	prefBeamReceive,						// not used - use ExgLibControl with ir(Get/Set)ScanningMode instead
	prefCalibrateDigitizerAtReset,	// True makes the user calibrate at soft reset time
	prefSystemKeyboardID,				// ID of the preferred keyboard resource
	prefDefSerialPlugIn,					// creator ID of the default serial plug-in

	// Additions for PalmOS 3.1:
	prefStayOnWhenPluggedIn,			// don't sleep after timeout when using line current
	prefStayLitWhenPluggedIn,			// keep backlight on when not sleeping on line current

	// Additions for PalmOS 3.2:
	prefAntennaCharAppCreator,			// App creator for antenna key

	// Additions for PalmOS 3.3:
	prefMeasurementSystem,				// English, Metric, etc.
	
	// Additions for PalmOS 3.5:
	prefShowPrivateRecords,				// returns privateRecordViewEnum
	prefAutoOffDurationSecs,			// auto-off duration in seconds
	
	// Additions for PalmOS 4.0:
	prefTimeZone,							// GMT offset in minutes, + for east of GMT, - for west
	prefDaylightSavingAdjustment,		// current DST adjustment in minutes (typically 0 or 60)

	prefAutoLockType,						// Never, on poweroff, after preset delay or at preset time.
	prefAutoLockTime,						// Auto lock preset time or delay.
	prefAutoLockTimeFlag,    			// For Minutes or Hours.

	prefLanguage,							// Language spoken in country selected via Setup app/Formats panel
	prefLocale,								// Locale for country selected via Setup app/Formats panel 
	
	prefTimeZoneCountry,					// Country used to specify time zone.
	
	prefAttentionFlags,					// User prefs for getting user's attention

	prefDefaultAppCreator,				// Default application launched on reset.
	
	// Additions for PalmOS 5.0:
	prefDefFepPlugInCreator,			// creator ID of the default Fep plug-in

	// Additions for PalmOS 5.1:
	prefColorThemeID,						// Resource ID of the color theme.

	// Additions for PalmOS 5.3
	prefHandednessChoice,				// reserved for future use
	prefHWRCreator							// Creator for the hand writing recognition library
	} SystemPreferencesChoice;
	
	
typedef struct {
	UInt16 version;						// Version of preference info
	
	// International preferences
	CountryType country;					// Country the device is in
	DateFormatType dateFormat;			// Format to display date in
	DateFormatType longDateFormat;	// Format to display date in
	UInt8 weekStartDay;					// Sunday or Monday
	TimeFormatType timeFormat;			// Format to display time in
	NumberFormatType numberFormat;	// Format to display numbers in
	
	// system preferences
	UInt8 autoOffDuration;				// Time period before shutting off (in minutes)
	SoundLevelTypeV20 sysSoundLevel;		//	slOn or slOff - error beeps and other non-alarm sounds
	SoundLevelTypeV20 alarmSoundLevel;	//	slOn or slOff - alarm only
	Boolean hideSecretRecords;			// True to not display records with
												// their secret bit attribute set
	Boolean deviceLocked;				// Device locked until the system
												// password is entered
	UInt8 reserved1;
	UInt16 sysPrefFlags;					// Miscellaneous system pref flags
												//  copied into the global GSysPrefFlags
												//  at boot time.
	SysBatteryKind	sysBatteryKind;	// The type of batteries installed. This
												// is copied into the globals GSysbatteryKind
												//  at boot time.
	UInt8 reserved2;
	
	} SystemPreferencesTypeV10;


// Any entries added to this structure must be initialized in 
// Prefereces.c:GetPreferenceResource
//


typedef struct 
	{
	UInt16 version;						// Version of preference info
	
	// International preferences
	CountryType country;					// Country the device is in (see PalmLocale.h)
	DateFormatType dateFormat;			// Format to display date in
	DateFormatType longDateFormat;	// Format to display date in
	Int8 weekStartDay;					// Sunday or Monday
	TimeFormatType timeFormat;			// Format to display time in
	NumberFormatType numberFormat;	// Format to display numbers in
	
	// system preferences
	UInt8 autoOffDuration;				// Time period in minutes before shutting off (use autoOffDurationSecs instead).
	SoundLevelTypeV20 sysSoundLevelV20;		//	slOn or slOff - error beeps and other non-alarm/game sounds
	SoundLevelTypeV20 gameSoundLevelV20;	//	slOn or slOff - game sound effects 
	SoundLevelTypeV20 alarmSoundLevelV20;	//	slOn or slOff - alarm sound effects
	Boolean hideSecretRecords;			// True to not display records with
												// their secret bit attribute set
	Boolean deviceLocked;				// Device locked until the system
												// password is entered
	Boolean localSyncRequiresPassword;	// User must enter password on Pilot
	Boolean remoteSyncRequiresPassword;	// User must enter password on Pilot
	UInt16 sysPrefFlags;					// Miscellaneous system pref flags
												//  copied into the global GSysPrefFlags
												//  at boot time. Constants are
												//  sysPrefFlagXXX defined in SystemPrv.h
	SysBatteryKind	sysBatteryKind;	// The type of batteries installed. This
												// is copied into the globals GSysbatteryKind
												//  at boot time.
	UInt8 reserved1;
	UInt32 minutesWestOfGMT;			// minutes west of Greenwich
	DaylightSavingsTypes	daylightSavings;	// Type of daylight savings correction
	UInt8 reserved2;
	UInt16 ronamaticChar;				// character to generate from ronamatic stroke.
												//  Typically it popups the onscreen keyboard.
	UInt32	hard1CharAppCreator;		// creator of application to launch in response
												//  to the hard button #1. Used by SysHandleEvent.
	UInt32	hard2CharAppCreator;		// creator of application to launch in response
												//  to the hard button #2. Used by SysHandleEvent.
	UInt32	hard3CharAppCreator;		// creator of application to launch in response
												//  to the hard button #3. Used by SysHandleEvent.
	UInt32	hard4CharAppCreator;		// creator of application to launch in response
												//  to the hard button #4. Used by SysHandleEvent.
	UInt32	calcCharAppCreator;		// creator of application to launch in response
												//  to the Calculator icon. Used by SysHandleEvent.
	UInt32	hardCradleCharAppCreator;	// creator of application to launch in response
												//  to the Cradle button. Used by SysHandleEvent.
	UInt32	launcherCharAppCreator;		// creator of application to launch in response
												//  to the launcher button. Used by SysHandleEvent.
	UInt32	hardCradle2CharAppCreator;	// creator of application to launch in response
												//  to the 2nd Cradle button. Used by SysHandleEvent.
	AnimationLevelType animationLevel;	// amount of animation to display
	
	Boolean	maskPrivateRecords;			// Only meaningful if hideSecretRecords is true.
													//true to show a grey placeholder box for secret records.
													//was reserved3 - added for 3.5
													
	
	// Additions for PalmOS 3.0:
	UInt16 sysSoundVolume;					//	system amplitude (0 - sndMaxAmp) - taps, beeps
	UInt16 gameSoundVolume;					//	game amplitude (0 - sndMaxAmp) - explosions
	UInt16 alarmSoundVolume;				//	alarm amplitude (0 - sndMaxAmp)
	Boolean beamReceive;						// False turns off IR sniffing, sends still work.
	Boolean calibrateDigitizerAtReset;	// True makes the user calibrate at soft reset time
	UInt16 systemKeyboardID;				// ID of the preferred keyboard resource
	UInt32 defSerialPlugIn;					// creator ID of the default serial plug-in

	// Additions for PalmOS 3.1:
	Boolean stayOnWhenPluggedIn;			// don't sleep after timeout when using line current
	Boolean stayLitWhenPluggedIn;			// keep backlight on when not sleeping on line current

	// Additions for PalmOS 3.2:
	UInt32	antennaCharAppCreator;		// creator of application to launch in response
													//  to the antenna key. Used by SysHandleEvent.

	// Additions for PalmOS 3.5:
	MeasurementSystemType measurementSystem;	// metric, english, etc.
	UInt8						reserved3;				
	UInt16 autoOffDurationSecs;					// Time period in seconds before shutting off.
	
	// Additions for PalmOS 4.0:
	Int16 timeZone;							// minutes east of Greenwich
	Int16	daylightSavingAdjustment;		// current daylight saving correction in minutes
	CountryType timeZoneCountry;			// country used to specify time zone.
	SecurityAutoLockType  autoLockType;		// Never, on power off, after preset delay or at preset time
	UInt32				autoLockTime;		// Auto lock preset time or delay.
	Boolean				autoLockTimeFlag;	// For Minutes or Hours.
	LanguageType		language;			// Language spoken in country selected via Setup app/Formats panel

	AttnFlagsType		attentionFlags;		// User prefs for getting user's attention
	
	UInt32				defaultAppCreator;	// Creator of the default "safe" app that is launched
											// on a reset.
	// Additions for PalmOS 5.0:
	UInt32	defFepPlugInCreator;			// Creator of the default Fep plug-in
											// on a reset.

	DmResID				colorThemeID;		// Resource ID of the color theme.
	
	// Additions for PalmOS 5.3
	UInt16				handedness;
	UInt32				HWRCreator;			// creator of the HWR library
	} SystemPreferencesType;


	
typedef SystemPreferencesType *SystemPreferencesPtr;


// structure of the resource that holds hard/soft button defaults
typedef struct {
	UInt16 keyCode;						// virtual key code of the hard/soft button
	UInt32 creator;						// app creator code
} ButtonDefaultAppType;

typedef struct {
	UInt16 numButtons;					// number of default button assignments
	ButtonDefaultAppType button[1];	// array of button assignments
} ButtonDefaultListType;


//-------------------------------------------------------------------
// Preferences routines
//-------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif


extern DmOpenRef PrefOpenPreferenceDBV10 (void)
		SYS_TRAP(sysTrapPrefOpenPreferenceDBV10);

extern DmOpenRef PrefOpenPreferenceDB (Boolean saved)
		SYS_TRAP(sysTrapPrefOpenPreferenceDB);

extern void PrefGetPreferences(SystemPreferencesPtr p)
		SYS_TRAP(sysTrapPrefGetPreferences);

extern void PrefSetPreferences(SystemPreferencesPtr p)
		SYS_TRAP(sysTrapPrefSetPreferences);

extern UInt32 PrefGetPreference(SystemPreferencesChoice choice)
		SYS_TRAP(sysTrapPrefGetPreference);

extern void PrefSetPreference(SystemPreferencesChoice choice, UInt32 value)
		SYS_TRAP(sysTrapPrefSetPreference);

extern Int16 PrefGetAppPreferences (UInt32 creator, UInt16 id, void *prefs, 
	UInt16 *prefsSize, Boolean saved)
		SYS_TRAP(sysTrapPrefGetAppPreferences);

extern Boolean PrefGetAppPreferencesV10 (UInt32 type, Int16 version, void *prefs,
	UInt16 prefsSize)
		SYS_TRAP(sysTrapPrefGetAppPreferencesV10);

extern void PrefSetAppPreferences (UInt32 creator, UInt16 id, Int16 version, 
	const void *prefs, UInt16 prefsSize, Boolean saved)
		SYS_TRAP(sysTrapPrefSetAppPreferences);

extern void PrefSetAppPreferencesV10 (UInt32 creator, Int16 version, void *prefs,
	UInt16 prefsSize)
		SYS_TRAP(sysTrapPrefSetAppPreferencesV10);


#ifdef __cplusplus 
}
#endif


#endif	// __PREFERENCES_H__
