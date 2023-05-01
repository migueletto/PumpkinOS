/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SoundMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Sound Manager
 *
 *****************************************************************************/

#ifndef __SOUNDMGR_H__
#define __SOUNDMGR_H__


// Include elementary types
#include <PalmTypes.h>
#include <CoreTraps.h>				// Trap Numbers.

#include <Preferences.h>


/************************************************************
 * Sound Manager constants
 *
 *************************************************************/

// Sound Manager max and default volume levels
#define sndMaxAmp				64
//#define sndVolumeMask		0x0ff
#define sndDefaultAmp		sndMaxAmp

#define sndMidiNameLength	32			// MIDI track name length *including* NULL terminator


/************************************************************
 * Sound Manager data structures
 *
 *************************************************************/

//
// Command numbers for SndCommandType's cmd field
//
enum SndCmdIDTag {
									
	sndCmdFreqDurationAmp = 1,		// play a sound, blocking for the entire duration (except for zero amplitude)
											// param1 = frequency in Hz
											// param2 = duration in milliseconds
											// param3 = amplitude (0 - sndMaxAmp); if 0, will return immediately
											
	// Commands added in PilotOS v3.0
	// ***IMPORTANT***
	// Please note that SndDoCmd() in PilotOS before v3.0 will Fatal Error on unknown
	// commands (anything other than sndCmdFreqDurationAmp).  For this reason,
	// applications wishing to take advantage of these new commands while staying
	// compatible with the earlier version of the OS, _must_ avoid using these commands
	// when running on OS versions less thatn v3.0 (see sysFtrNumROMVersion in SystemMgr.h).
	// Beginning with v3.0, SndDoCmd has been fixed to return sndErrBadParam when an
	// unknown command is passed.
	//
	sndCmdNoteOn,						// start a sound given its MIDI key index, max duration and velocity;
											// the call will not wait for the sound to complete, returning imeediately;
											// any other sound play request made before this one completes will interrupt it.
											// param1 = MIDI key index (0-127)
											// param2 = maximum duration in milliseconds
											// param3 = velocity (0 - 127) (will be interpolated as amplitude)
	
	sndCmdFrqOn,						// start a sound given its frequency in Hz, max duration and amplitude;
											// the call will not wait for the sound to complete, returning imeediately;
											// any other sound play request made before this one completes will interrupt it.
											// param1 = frequency in Hz
											// param2 = maximum duration in milliseconds 
											// param3 = amplitude (0 - sndMaxAmp)
	
	sndCmdQuiet							// stop current sound
											// param1 = 0
											// param2 = 0
											// param3 = 0

	};

typedef UInt8 SndCmdIDType;

	

//
// SndCommandType: used by SndDoCmd()
//

typedef struct SndCommandType {
	SndCmdIDType	cmd;					// command id
	UInt8 			reserved;
	Int32			param1;					// first parameter
	UInt16			param2;					// second parameter
	UInt16			param3;					// third parameter
} SndCommandType;

typedef SndCommandType*		SndCommandPtr;


//
// Beep numbers used by SndSysBeep()
//

enum SndSysBeepTag {
	sndInfo = 1,
	sndWarning,
	sndError,
	sndStartUp,
	sndAlarm,
	sndConfirmation,
	sndClick
	};

typedef UInt8	SndSysBeepType;

/************************************************************
 * Standard MIDI File (SMF) support structures
 *************************************************************/


// Structure of records in the MIDI sound database:
//
// Each MIDI record consists of a record header followed immediately by the
// Standard MIDI File (SMF) data stream.  Only SMF format #0 is presently supported.
// The first byte of the record header is the byte offset from the beginning of the record
// to the SMF data stream.  The name of the record follows the byte offset
// field.  sndMidiNameLength is the limit on name size (including NULL).
#define sndMidiRecSignature	'PMrc'
typedef struct SndMidiRecHdrType {
	UInt32		signature;				// set to sndMidiRecSignature
	UInt8			bDataOffset;			// offset from the beginning of the record
												// to the Standard Midi File data stream
	UInt8			reserved;				// set to zero
	} SndMidiRecHdrType;

typedef struct SndMidiRecType {
	SndMidiRecHdrType		hdr;			// offset from the beginning of the record
												// to the Standard Midi File data stream
	Char			name[2];					// Track name: 1 or more chars including NULL terminator.
												// If a track has no name, the NULL character must still
												// be provided.
												// Set to 2 to pad the structure out to a word boundary.
	} SndMidiRecType;


// Midi records found by SndCreateMidiList.
typedef struct SndMidiListItemType
	{
	Char			name[sndMidiNameLength];			// including NULL terminator
	UInt32		uniqueRecID;
	LocalID		dbID;
	UInt16 		cardNo;
	} SndMidiListItemType;


// Commands for SndPlaySmf
enum SndSmfCmdEnumTag {
	sndSmfCmdPlay = 1,					// play the selection
	sndSmfCmdDuration						// get the duration in milliseconds of the entire track
	};

typedef UInt8 SndSmfCmdEnum;

typedef void SndComplFuncType(void *chanP, UInt32 dwUserData);
typedef SndComplFuncType *SndComplFuncPtr;


// Return true to continue, false to abort
typedef Boolean SndBlockingFuncType(void *chanP, UInt32 dwUserData, Int32 sysTicksAvailable);
typedef SndBlockingFuncType *SndBlockingFuncPtr;

typedef struct SndCallbackInfoType {
	MemPtr		funcP;			// pointer to the callback function (NULL = no function)
	UInt32	dwUserData;		// value to be passed in the dwUserData parameter of the callback function
	} SndCallbackInfoType;


typedef struct SndSmfCallbacksType {
	SndCallbackInfoType	completion;		// completion callback function (see SndComplFuncType)
	SndCallbackInfoType	blocking;		// blocking hook callback function (see SndBlockingFuncType)
	SndCallbackInfoType	reserved;		// RESERVED -- SET ALL FIELDS TO ZERO BEFORE PASSING
	} SndSmfCallbacksType;


#define sndSmfPlayAllMilliSec		0xFFFFFFFFUL

typedef struct SndSmfOptionsType {
	// dwStartMilliSec and dwEndMilliSec are used as inputs to the function for sndSmfCmdPlay and as
	// outputs for sndSmfCmdDuration
	UInt32	dwStartMilliSec;				// 0 = "start from the beginning"
	UInt32	dwEndMilliSec;					// sndSmfPlayAllMilliSec = "play the entire track";
													// the default is "play entire track" if this structure
													// is not passed in
	
	// The amplitude and interruptible fields are used only for sndSmfCmdPlay
	UInt16	amplitude;						// relative volume: 0 - sndMaxAmp, inclusively;  the default is
													// sndMaxAmp if this structure is not passed in; if 0, the play will
													// be skipped and the call will return immediately
	
	Boolean	interruptible;					// if true, sound play will be interrupted if
													// user interacts with the controls (digitizer, buttons, etc.);
													// if false, the paly will not be interrupted; the default behavior
													// is "interruptible" if this structure is not passed in
													
	UInt8		reserved1;
	UInt32	reserved;						// RESERVED! -- MUST SET TO ZERO BEFORE PASSING
	} SndSmfOptionsType;


typedef struct SndSmfChanRangeType {
	UInt8	bFirstChan;							// first MIDI channel (0-15 decimal)
	UInt8	bLastChan;							// last MIDI channel (0-15 decimal)
	} SndSmfChanRangeType;




/************************************************************
 * Sound Manager result codes
 * (sndErrorClass is defined in SystemMgr.h)
 *************************************************************/
#define	sndErrBadParam			(sndErrorClass | 1)
#define	sndErrBadChannel		(sndErrorClass | 2)
#define	sndErrMemory			(sndErrorClass | 3)
#define	sndErrOpen				(sndErrorClass | 4)
#define	sndErrQFull				(sndErrorClass | 5)
#define	sndErrQEmpty			(sndErrorClass | 6)		// internal
#define	sndErrFormat			(sndErrorClass | 7)		// unsupported data format
#define	sndErrBadStream			(sndErrorClass | 8)		// invalid data stream
#define	sndErrInterrupted		(sndErrorClass | 9)		// play was interrupted
#define sndErrNotImpl			(sndErrorClass |10)		// function not implemented
#define sndErrInvalidStream		(sndErrorClass |11)		// invalid stream-identifier



/********************************************************************
 * Sound Manager Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------
// Initialization
//-------------------------------------------------------------------

// Initializes the Sound Manager.  Should only be called by
// Pilot initialization code.
Err			SndInit(void)	SYS_TRAP(sysTrapSndInit);

// Frees the Sound Manager.
//void			SndFree(void)	SYS_TRAP(sysTrapSndFree);


//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

// Sets default sound volume levels
//
// Any parameter may be passed as NULL
extern void			SndSetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *defAmpP)
							SYS_TRAP(sysTrapSndSetDefaultVolume);

// Gets default sound volume levels
//
// Any parameter may be passed as NULL
extern void			SndGetDefaultVolume(UInt16 *alarmAmpP, UInt16 *sysAmpP, UInt16 *masterAmpP)
							SYS_TRAP(sysTrapSndGetDefaultVolume);

// Executes a sound command on the given sound channel (pass
// channelP = 0 to use the shared channel).
extern Err			SndDoCmd(void * /*SndChanPtr*/ channelP, SndCommandPtr cmdP, Boolean noWait)
							SYS_TRAP(sysTrapSndDoCmd);

// Plays one of several defined system beeps/sounds (see sndSysBeep...
// constants).
extern void			SndPlaySystemSound(SndSysBeepType beepID)
							SYS_TRAP(sysTrapSndPlaySystemSound);


// NEW FOR v3.0
// Performs an operation on a Standard MIDI File (SMF) Format #0
extern Err		SndPlaySmf(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP,
						SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP,
						Boolean bNoWait)
							SYS_TRAP(sysTrapSndPlaySmf);

// NEW FOR v3.0
// Creates a list of all midi records.  Useful for displaying in lists.
// For creator wildcard, pass creator=0;
extern Boolean		SndCreateMidiList(UInt32 creator, Boolean multipleDBs, UInt16 *wCountP, MemHandle *entHP)
							SYS_TRAP(sysTrapSndCreateMidiList);
							
// NEW FOR v3.2
// Plays a MIDI sound which is read out of an open resource database
extern Err 			SndPlaySmfResource(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
							SYS_TRAP(sysTrapSndPlaySmfResource);


// NEW FOR v4.0
// Plays MIDI sounds regardless of the how the interruptible flag is set
extern Err		SndPlaySmfIrregardless(void *chanP, SndSmfCmdEnum cmd, UInt8 *smfP, SndSmfOptionsType *selP,
						SndSmfChanRangeType *chanRangeP, SndSmfCallbacksType *callbacksP,
						Boolean bNoWait)
							SYS_TRAP(sysTrapSndPlaySmfIrregardless);

extern Err 		SndPlaySmfResourceIrregardless(UInt32 resType, Int16 resID, SystemPreferencesChoice volumeSelector)
							SYS_TRAP(sysTrapSndPlaySmfResourceIrregardless);
							

extern Err 		SndInterruptSmfIrregardless(void)
							SYS_TRAP(sysTrapSndInterruptSmfIrregardless);


//
// Sound streams API. New for v5
//

#define sndFtrIDVersion		0		// ID of feature containing (new) sound manager version
									// Check for existence of this feature to determine if 
									//  the Sound streams APIs are available
#define sndMgrVersionNum	(100)   // The current version (1.00) 

typedef UInt32 SndStreamRef;

enum SndStreamModeTag {
	sndInput,
	sndOutput
};

typedef Int8 SndStreamMode;

enum SndStreamWidthTag {
	sndMono,
	sndStereo
};
typedef Int8 SndStreamWidth;

/*
 * special 'cookie' values that can be used instead of a real volume setting
 * */
enum
{
	sndSystemVolume = -1,
	sndGameVolume = -2,
	sndAlarmVolume = -3
};

/*
 * the lower 4 bits of the sampletype indicate the size in bytes of a single
 * sample.
 */
enum SndSampleTag {
	sndInt8 		= 0x01,
	sndUInt8		= 0x11,

	sndInt16Big		= 0x02,
	sndInt16Little	= 0x12,
	sndInt32Big		= 0x04,
	sndInt32Little	= 0x14,
	sndFloatBig		= 0x24,
	sndFloatLittle	= 0x34,
#if SYS_ENDIAN == BIG_ENDIAN
	sndInt16		= sndInt16Big,			// native-endian
	sndInt16Opposite= sndInt16Little,		// opposite of native-endian 
	sndInt32		= sndInt32Big,		
	sndInt32Opposite= sndInt32Little,		
	sndFloat		= sndFloatBig,
	sndFloatOpposite= sndFloatLittle
#else
	sndInt16		= sndInt16Little,
	sndInt16Opposite= sndInt16Big,
	sndInt32		= sndInt32Little,		
	sndInt32Opposite= sndInt32Big,
	sndFloat		= sndFloatLittle,
	sndFloatOpposite= sndFloatBig
#endif
};
typedef Int16 SndSampleType;


// Flags to be used with SndPlayResource
#define sndFlagSync		0x00000000
#define sndFlagAsync	0x00000001
#define sndFlagNormal	sndFlagSync

// Special Panning values
#define sndPanCenter		(0)
#define sndPanFullLeft		(-1024)
#define sndPanFullRight		(1024)

typedef void *SndPtr;

typedef Err (*SndStreamBufferCallback)(void *userdata,
		SndStreamRef channel, void *buffer, UInt32 numberofframes);

extern Err SndStreamCreate(SndStreamRef *channel,	/* channel-ID is stored here */
						SndStreamMode mode,			/* input/output, enum */
						UInt32 samplerate,				/* in frames/second, e.g. 44100, 48000 */
						SndSampleType type,				/* enum, e.g. sndInt16 */
						SndStreamWidth width,			/* enum, e.g. sndMono */
						SndStreamBufferCallback func,	/* function that gets called to fill a buffer */
						void *userdata,					/* gets passed in to the above function */
						UInt32 buffsize,				/* preferred buffersize in frames, not guaranteed, use 0 for default */
						Boolean armNative)				/* true if callback is arm native */
							SYS_TRAP(sysTrapSndStreamCreate);

extern Err SndStreamDelete(SndStreamRef channel)
							SYS_TRAP(sysTrapSndStreamDelete);

extern Err SndStreamStart(SndStreamRef channel)
							SYS_TRAP(sysTrapSndStreamStart);

extern Err SndStreamPause(SndStreamRef channel, Boolean pause)
							SYS_TRAP(sysTrapSndStreamPause);

extern Err SndStreamStop(SndStreamRef channel)
							SYS_TRAP(sysTrapSndStreamStop);

extern Err SndStreamSetVolume(SndStreamRef channel, Int32 volume)
							SYS_TRAP(sysTrapSndStreamSetVolume);

extern Err SndStreamGetVolume(SndStreamRef channel, Int32 *volume)
							SYS_TRAP(sysTrapSndStreamGetVolume);

extern Err SndPlayResource(SndPtr sndP, Int32 volume, UInt32 flags)
							SYS_TRAP(sysTrapSndPlayResource);

extern Err SndStreamSetPan(SndStreamRef channel, Int32 panposition)
							SYS_TRAP(sysTrapSndStreamSetPan);

extern Err SndStreamGetPan(SndStreamRef channel, Int32 *panposition)
							SYS_TRAP(sysTrapSndStreamGetPan);

	
typedef Err (*SndStreamVariableBufferCallback)(void *userdata,
		SndStreamRef channel, void *buffer, UInt32 *bufferSizeP);

enum SndFormatTag {
	sndFormatPCM		= 0,			// Raw PCM (Pulse Code Modulation), (attributes defined by type parameter)
	sndFormatIMA_ADPCM	= 'APCM',		// IMA ADPCM (Interactive Multimedia Association Adaptive Differential PCM)
	sndFormatDVI_ADPCM	= 'DPCM',		// Intel/MS/DVI ADPCM
	sndFormatMP3		= 'MPG3',		// Moving Picture Experts Group, Audio Layer III (MPEG-1 and MPEG-2 depending on bit-rate)
	sndFormatAAC		= 'DAAC',		// Dolby Advanced Audio Coding
	sndFormatOGG		= 'OGGV'		// Ogg Vorbis
};

typedef UInt32 SndFormatType;

extern Err SndStreamCreateExtended(
	SndStreamRef *channel,	/* channel-ID is stored here */
	SndStreamMode mode,		/* input/output, enum */
	SndFormatType format,	/* enum, e.g., sndFormatMP3 */
	UInt32 samplerate,		/* in frames/second, e.g. 44100, 48000 */
	SndSampleType type,		/* enum, e.g. sndInt16, if applicable, or 0 otherwise */
	SndStreamWidth width,	/* enum, e.g. sndMono */
	SndStreamVariableBufferCallback func,	/* function that gets called to fill a buffer */
	void *userdata,			/* gets passed in to the above function */
	UInt32 buffsize,			/* preferred buffersize, use 0 for default */
	Boolean armNative)				/* true if callback is arm native */
			SYS_TRAP(sysTrapSndStreamCreateExtended);

extern Err SndStreamDeviceControl( SndStreamRef channel, Int32 cmd, void* param, Int32 size)
			SYS_TRAP(sysTrapSndStreamDeviceControl);
						
#ifdef __cplusplus 
}
#endif


#endif  // __SOUND_MGR_H__
