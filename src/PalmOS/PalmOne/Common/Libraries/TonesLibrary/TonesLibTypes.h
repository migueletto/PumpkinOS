/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Tone Ring Tone Library
 * @brief		This library provides support for application that wants to
 *				manage or play ring tones from within the application itself.
 *
 * Applications that wish to play, manage or create ring tones from within the
 * application themselves can use the APIs provided by this library. This library
 * can also be used to set different ring tones preferences that are used by
 * the system when there is, for example, an incoming call, email or sms alert, etc.
 *
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 *
 * @{
 * @}
 */
/**
 * @ingroup Tone
 */

/**
 * @file 	TonesLibTypes.h
 * @brief   Public 68K header file for the Ring Tone library on Treo devices.
 *
 * This file contains the common constants for the tones and also the structure
 * that contains the ring-tone related sound preferences.
 * <hr>
 */

#ifndef TONES_LIB_TYPES__H__
#define TONES_LIB_TYPES__H__

#define      tonesLibName           "Tones Library" /**<Name of the Tone Types library. */
#define      tonesLibCreator        hsFileCTonesLib /**<Creator ID of the Tone Types library. */
#define      tonesLibType           sysFileTLibrary /**<Type of the Tone Types library. */

#define      kMaxTonesNameSize       32 /**<Maximum size of a tone name. */
#define		  tonesLibPlayForever	  0xFFFF /**<Used to play a tone over and over indefinitely. */

/**
 * @name Error Codes
 *
 */
/*@{*/
#define      tonesLibErrClass        0x9000 /**<No definition. */
#define      tonesLibErrCreateDBFailed  tonesLibErrClass | 1/**<No definition. */
#define      tonesLibErrNoMemory        tonesLibErrClass | 2/**<No definition. */
#define      tonesLibErrNoTone          tonesLibErrClass | 3/**<No definition. */
#define      tonesLibErrDBNotFound      tonesLibErrClass | 4/**<No definition. */
#define      tonesLibErrDBWriteFailed   tonesLibErrClass | 5/**<No definition. */
#define      tonesLibErrUserCancel      tonesLibErrClass | 6/**<No definition. */
#define      tonesLibErrBadParams       tonesLibErrClass  | 7/**<No definition. */
#define      tonesLibErrUnknown         tonesLibErrClass | 8/**<No definition. */
#define      tonesLibErrNotAllowed       tonesLibErrClass | 9/**<No definition. */
#define      tonesLibErrAlreadyPlaying       tonesLibErrClass | 10/**<No definition. */
/*@}*/

/**
 * @name Utility Macros
 *
 */
/*@{*/
#define      CompareToneIdentifiers(tone1, tone2) \
             (tone1.id == tone2.id && tone1.toneType == tone2.toneType)/**<No definition. */
#define      CopyToneIdentifier(destP, src) \
			 destP->id = src.id; \
             destP->toneType = src.toneType/**<No definition. */
#define      CopyMIDIToneIdentifier(destP, src) \
			 destP->id = src; \
             destP->toneType = toneTypeMIDI/**<No definition. */
/*@}*/

/**
 * Tone types
 */
enum _ToneType {
	toneTypeMIDI,		  /**<Record in MIDI Ring Tones DB. */
	toneTypeWAV,		  /**<WAV in a SndFileStream. */
	toneTypeAMR,		  /**<AMR in a SndFIleStream. */
	toneTypeSystemSound,  /**<Unused. */
	toneTypeQCELP,		  /**<QCELP in a SndFileStream. */
	toneTypeMP3			  /**<MP3 in a SndFileStream */
};

/** Holds tone type value */
typedef UInt16 ToneType;

/**
 * Sound preferences type
 */
enum _SoundPrefType
{
  soundPrefTypePhone,		/**<Tone pref for phone. */
  soundPrefTypeEmail,		/**<Tone pref for email. */
  soundPrefTypeCalendar,	/**<Tone pref for Calendar. */
  soundPrefTypeIM,			/**<Tone pref for IM. */
  soundPrefTypeMMS,			/**<Tone pref for MMS. */
  soundPrefTypeSMS,			/**<Tone pref for SMS. */
  soundPrefTypeLast			/**<Last item in enum. Please keep it as last item. */
};

/** Holds sound preference type value */
typedef UInt16  SoundPrefType;

/**
 * Tone volume
 */
enum _ToneVolume
{
  toneVolumeOff,	/**<Volume off */
  toneVolume1,		/**<Volume level 1 */
  toneVolume2,		/**<Volume level 2 */
  toneVolume3,		/**<Volume level 3 */
  toneVolume4,		/**<Volume level 4 */
  toneVolume5,		/**<Volume level 5 */
  toneVolume6,		/**<Volume level 6 */
  toneVolume7		/**<Volume level 7 */
};

/** Holds tone volume value */
typedef UInt16  ToneVolume;

/**
 * Number of times to play tone
 */
enum _TonePlayNumberTimes
{
  tonePlayOnce,		/**<Play once. */
  tonePlayTwice,	/**<Play twice. */
  tonePlay3Times,	/**<Play three times. */
  tonePlay5Times,	/**<Play five times. */
  tonePlay10Times,	/**<Play ten times. */
  tonePlay100Times	/**<Play one hundred times. */
};

/** Holds number of times to play a tone value */
typedef UInt8 TonePlayNumberTimes;

/**
 * Tone repeat interval
 */
enum _ToneRepeatInterval
{
  toneRepeatEveryMinute,	/**<Repeat every minute. */
  toneRepeatEvery5Minutes,	/**<Repeat every 5 minutes. */
  toneRepeatEvery10Minutes,	/**<Repeat every 10 minutes. */
  toneRepeatEvery30Minutes	/**<Repeat every 30 minutes. */
};

/** Holds the tone repeat interval value */
typedef UInt8 ToneRepeatInterval;

/**
 * Tone time unit
 */
enum _ToneTimeUnit
{
  toneTimeUnitMinutes,	 /**< Minutes */
  toneTimeUnitHours,	 /**< Hours */
  toneTimeUnitDays		 /**< Days  */

};

/** Holds the tone time unit value */
typedef UInt8 ToneTimeUnit;

/**
 * Tone vibrate type
 */
enum _ToneVibrateType
{
  toneVibrateNone,		/**< None */
  toneVibrateRing,		/**< Phone ring event */
  toneVibrateAlert,		/**< Alert event */
  toneVibrateMail,		/**< New mail */
  toneVibrateCalender,	/**< Calendar event */
  toneVibrateSMS,		/**< New SMS */
  toneVibrateVoicemail	/**< New Voicemail */

};

/** Holds the tone vibrate type value */
typedef UInt8 ToneVibrateType;

/**
 * @brief No definition.
 */
typedef struct {
	UInt32 id;			/**<Tone ID */
	ToneType toneType;	/**<Tony Type (AMR, MIDI, etc). @see ToneType */
} ToneIdentifier;

/**
 * @brief Tone characteristics
 */
typedef struct {

	Char   name [kMaxTonesNameSize];  /**<Name of the ring tone.  */
	ToneIdentifier identifier;  /**<Unique ID for the ring tone.  */
	Boolean protectedTone;   /**<Whether Tone is protected and cannot be beamed or sent.  */

} ToneItemType, * ToneItemPtr;

/**
 * @brief  Structure for IM Sound Pref.
 */
typedef struct {

	ToneIdentifier chatTone;	/**<Tone used for chatting */
	ToneIdentifier messageTone;	/**<Tone used for messaging */

} SoundPreferenceIM;

/**
 * @brief  Structure for Phone Sound Pref.
 */
typedef struct {
	ToneIdentifier unknownCallerTone;	/**<Tone used when caller is unknown */
	ToneIdentifier knownCallerTone;		/**<Tone used when caller is known */
	ToneIdentifier roamingTone;			/**<Tone used when roaming */
	ToneIdentifier  voicemailTone;		/**<Tone used for incoming voicemail */
	Boolean serviceTone;				/**<No definition. */
} SoundPreferencePhone;

/**
 * @brief  Structure for Email Sound Pref.
 */
typedef struct {
    ToneIdentifier alertTone;	/**<Tone used for new email alert */
	Boolean newMailAlert;		/**<Play tone if true */
} SoundPreferenceEmail;

/**
 * @brief  Structure for Calendar Sound Pref.
 */
typedef struct {
	ToneIdentifier alarmTone;				/**<Tone used for alarm */
	ToneIdentifier reminderTone;			/**<Tone used for event reminder */
	TonePlayNumberTimes playSoundTimes;		/**<Number of times tone should be played */
	TonePlayNumberTimes repeatAlarmTimes;	/**<Number of time alarm should be repeated */
	ToneRepeatInterval repeatInterval;		/**<Alarm repeat interval */
	UInt16 presetTime;						/**<Alarm preset time */
	ToneTimeUnit presetTimeUnit;			/**<Alarm preset time unit (minute, hour, day) */
	Boolean alarmPreset;					/**<Preset alarm if true */
} SoundPreferenceCalendar;

/**
 * @brief  Structure for SMS Sound Pref.
 */
typedef struct {
	ToneIdentifier messageTone;	/**<Tone used for SMS */
	Boolean privacy;			/**<No definition. */
	Boolean msgAlert;			/**<If true, alert when new SMS arrives */
	Boolean rcptAlert;			/**<If true, alert when recipient receives SMS */
} SoundPreferenceSMS;

/**
 * @brief  Structure for MMS Sound Pref.
 */
typedef struct {
  ToneIdentifier messageTone;	/**<Tone used when MMS arrives*/
} SoundPreferenceMMS;

/**
 * @brief  Structure containing all the Sound Preference data.
 */
typedef struct {
	/** union of various sound preferences */
	union _tonePrefs
	  {
		SoundPreferenceIM imSoundPrefs;				/**<No definition. */
		SoundPreferencePhone phoneSoundPrefs;		/**<No definition. */
		SoundPreferenceEmail emailSoundPrefs;		/**<No definition. */
		SoundPreferenceCalendar calendarSoundPrefs;	/**<No definition. */
		SoundPreferenceSMS  smsSoundPrefs;			/**<No definition. */
		SoundPreferenceMMS  mmsSoundPrefs;			/**<No definition. */
	  } tonePrefs;									/**<Holds data for various Sound Prefs */
	ToneVolume soundOnVolume;						/**<Volume of tone */
	Boolean soundOnVibrate;							/**<No definition. */
	Boolean soundOffVibrate;						/**<No definition. */
} SoundPreference;

#define kTonesDialogFlagNonModal	  0x00000001	/**<Hides the Done button from the play tone dialog. */

/**
 * @name Function Traps
 *
 */
/*@{*/
#define       tonesLibTrapGetToneList  sysLibTrapCustom /**<Trap ID for the function that gets the tone list. */
#define       tonesLibTrapGetToneIDs    sysLibTrapCustom+1/**<Trap ID for the function that gets the list of tone ids. */
#define       tonesLibTrapGetToneName   sysLibTrapCustom+2/**<Trap ID for the function that gets the name of a tone. */
#define       tonesLibTrapPlayTone      sysLibTrapCustom+3/**<Trap ID for the function that plays a tone. */
#define       tonesLibTrapAddMidiTone       sysLibTrapCustom+4/**<Trap ID for the function to add a MIDI tone.
  Note: This used to be tonesLibTrapAddTone. Leaving this
		  trap number maintains binary compatability with existing
 		  applications. Recompiling older source code will fail since
 		  the new version of TonesLibAddTone takes toneType as
 		  an additional parameter. */
#define       tonesLibTrapManageTones       sysLibTrapCustom+5/**<Trap ID for function to manage tones. */
#define       tonesLibTrapDeleteTone       sysLibTrapCustom+6/**<Trap ID for function to delete a tone. */
#define       tonesLibTrapGetSoundPrefs     sysLibTrapCustom+7/**<Trap ID for function to get a sound preference. */
#define       tonesLibTrapSetSoundPrefs     sysLibTrapCustom+8/**<Trap ID for function to set a sound preference. */
#define       tonesLibTrapInitTonesDB       sysLibTrapCustom+9/**<Trap ID for function to initialize the Tones database. */
#define       tonesLibTrapPickTone          sysLibTrapCustom+10/**<Trap ID for function to pick a tone to attach. */
#define       tonesLibTrapStopTone          sysLibTrapCustom+11/**<Trap ID for function to stop the playback of the current tone. */
#define		  tonesLibTrapGetSoundOnVolume	sysLibTrapCustom+12/**<Trap ID for function to get the on volume of a sound type. */
#define		  tonesLibTrapSetSoundOnVolume	sysLibTrapCustom+13/**<Trap ID for function to set the on volume of a sound type. */
#define       tonesLibTrapDoDialog          sysLibTrapCustom+14/**<Trap ID for function to display the tones list. */
#define		  tonesLibTrapGetToneSize		sysLibTrapCustom+15/**<Trap ID for function to get the size of a tone. */
#define		  tonesLibTrapToneCreate		sysLibTrapCustom+16/**<Trap ID for function to begin adding a new tone. */
#define		  tonesLibTrapToneWrite			sysLibTrapCustom+17/**<Trap ID for function to write data to a new tone. */
#define		  tonesLibTrapToneClose			sysLibTrapCustom+18/**<Trap ID for function to finish adding a new tone. */
#define		  tonesLibTrapToneRecord		sysLibTrapCustom+19/**<Trap ID for function to record a new tone. */
/*@}*/

#endif  // TONES_LIB_TYPES__H__

