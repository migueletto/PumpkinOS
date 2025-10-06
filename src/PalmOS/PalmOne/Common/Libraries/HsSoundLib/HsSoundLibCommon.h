/******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup Sound Sound Library
 * @brief		This library provides support for the specific sound functionality
 *				of the Treo smartphones.
 *
 * Developers should be able to use Palm OS Sound Manager APIs for most of the work.
 * The calling application should always load this library with SysLibLoad() before
 * use, even if it is already open by another application (ie. SysLibFind() returns
 * a valid refNum). When the application is done with the library it should be
 * unloaded with SysLibRemove(). We do this because there is no good way to synchronize
 * loading and unloading of libraries among multiple applications. It also greatly
 * simplifies internal synchronization.
 *
 * @{
 * @}
 */
/**
 * @ingroup Sound
 */

/**
 * @file	HsSoundLibCommon.h
 *
 * @brief	Public 68K common header file for sound support for Treo 600 and
 * 			Treo 650 smartphones.
 *
 * This file contains the common constants and structures to be used with
 * the sound library.
 *
 */

#ifndef __HSSOUNDLIB_COMMON__H__
#define __HSSOUNDLIB_COMMON__H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/

#include <PalmTypes.h>
#include <Common/System/HsErrorClasses.h>

/**
 * @name HsSoundLib information
 *
 */
/*@{*/
#define hsSndLibVersion				0x01070000	/**<Version of the sound library. */
#define hsSoundLibCreatorID			hsFileSoundLib /**<Creator ID of the sound library. */
#define hsSoundLibTypeID			sysFileTLibrary /**<Type of the sound library. */
#define hsSoundLibName				"HsSoundLib" /**<Name of the sound library. */
/*@}*/

/**
 * @name Sound library event notifications
 *
 */
/*@{*/
#define hsSndNotify					hsFileSoundLib /**<HsSoundLib broadcaster's ID */
#define hsSndNotifyUnplugged		'hOff' /**<No accesory is currently attached to the headset jack. */
#define hsSndNotifyHeadset			'hSet' /**<Headset jack connected and configured for mono headset with microphone. */
#define hsSndNotifyHeadphones		'hPho' /**<Headset jack connected and configured for stereo headphones. */
#define hsSndNotifyHybrid           'hHyb' /**<A hybrid headset/headphone is currently attached to the headset jack. */
#define hsSndNotifyAnswerButton		'hAns' /**<While a headset or hybrid was connected, the answer-button event was detected. */
#define hsSndNotifyAnswerHold		'hHol' /**<After hsSndNotifyAnswerButton notification, this event is sent if answer-button is still held down for some time. */
#define hsSndNotifyAnswerRelease	'hRel' /**<After hsSndNotifyAnswerButton notification, this event is sent when answer-button released. */
#define hsSndNotifyRingerSwitchOn	'hROn' /**<Notifies that ringer switch is in the on (play sounds to back speaker) position. */
#define hsSndNotifyRingerSwitchOff	'hROf' /**<Notifies that ringer switch is in the silent (no sounds to back speaker) position. */
#define hsSndNotifyHighPriority		'hHPr' /**<A high priority sound is about to start playing. */
#define hsSndNotifyAlert			'hAlr' /**<An alert sound is about to start playing. */
#define hsSndNotifyCarkitActive		'hCar' /**<Wired carkit has been activated, audio goes to carkit path. */
#define hsSndNotifyCarkitInactive	'hCOf' /**<Wired carkit is no longer active, audio routes normally. */
/*@}*/

/**
 * @name Sound library error codes
 */
/*@{*/
#define hsSndErrBase				hsSoundLibErrorClass
#define HS_SND_ERR(x)				((Err)((x) + hsSndErrBase))
#define hsSndErr_Failed				(HS_SND_ERR(1))  /**<The operation or request failed. */
#define hsSndErr_NotSupported		(HS_SND_ERR(2))  /**<Attempt to use functionality that is not supported in this version. */
#define hsSndErr_InvalidPort		(HS_SND_ERR(3))  /**<An invalid port was passed to a function. Operation failed. */
#define hsSndErr_PortCannotMute		(HS_SND_ERR(4))  /**<Unable to change mute state on a port that does not have mute capabilities. */
#define hsSndErr_PortCannotVolume	(HS_SND_ERR(5))  /**<Unable to change volume gains on a port that does not have volume capabilities. */
#define hsSndErr_PortCannotBoost	(HS_SND_ERR(6))  /**<Unable to change boost state on a port that does not have boost capabilities. */
#define hsSndErr_PortCannotPan		(HS_SND_ERR(7))  /**<Unable to change pan settings on a port that does not have panning capabilities. */
#define hsSndErr_InvalidSwitch		(HS_SND_ERR(8))  /**<An invalid switch was passed to a function. Operation failed. */
#define hsSndErr_UnknownSndParam	(HS_SND_ERR(9))  /**<An unknown HsSndParam selector was passed to a function call. */
#define hsSndErr_UnknownPortClass	(HS_SND_ERR(10)) /**<An unknown HsSndPortClass value was passed to a function call. */
#define hsSndErr_PortIDNotInClass	(HS_SND_ERR(11)) /**<The portID/portClass combination is not supported. */
#define hsSndErr_PortMuteLocked		(HS_SND_ERR(12)) /**<Unable to change mute state when hsSndParamMode_Normal is the default. */
#define hsSndErr_PortVolumeLocked	(HS_SND_ERR(13)) /**<Unable to change volume parameter when hsSndParamMode_Normal is the default. */
#define hsSndErr_PortBoostLocked	(HS_SND_ERR(14)) /**<Unable to change boost state when hsSndParamMode_Normal is the default. */
#define hsSndErr_PortPanLocked		(HS_SND_ERR(15)) /**<Unable to change pan value when hsSndParamMode_Normal is the default. */
#define hsSndErr_DoMidiProgressMsg	(HS_SND_ERR(16)) /**<Special message returned by Midi progress callback to request a dynamic volume change. */
#define hsSndErr_LastPublicError	(HS_SND_ERR(63)) /**<just a marker for last public library error code, private sound error codes start at 64 */
/*@}*/

/**
 * Beeps
 *
 * Provides additional system sounds for PalmOS SndPlaySystemSound.
 */
enum hsSndSysBeepTag
{
	hsSndCardInsert = 64, /**<No definition. */
	hsSndCardRemove, /**<No definition. */
	hsSndSyncStart, /**<No definition. */
	hsSndSyncStop, /**<No definition. */
	hsSndChargerAttach, /**<No definition. */
	hsSndChargerDetach, /**<No definition. */
	hsSndRadioOn, /**<No definition. */
	hsSndRadioOff, /**<No definition. */
	hsSndBatFull, /**<No definition. */
	hsSndBatLow, /**<No definition. */
	hsSndBatShutdown, /**<No definition. */
	hsSndDeepWake, /**<No definition. */
	hsSndDeepSleep, /**<No definition. */
	hsSndRadioInServiceArea, /**<No definition. */
	hsSndRadioOutServiceArea /**<No definition. */

};

/** Holds beep tone type. @see hsSndSysBeepTag */
typedef UInt8 hsSndSysBeepType;

/**
 * @name Sound mode volume controls
 *
 * Use with HsSndGetUserVolume and HsSndSetUserVolume.
 */
/*@{*/
#define hsSndMode_Current				((HsSndMode) 0x0000) /**<Pass to spefify the currently active sound mode. */
#define hsSndMode_Receiver				((HsSndMode) 0x0001) /**<Mode when Radio audio goes to receiver speaker. */
#define hsSndMode_Speakerphone			((HsSndMode) 0x0002) /**<Mode when Radio audio goes to loud speaker (speaker phone) */
#define hsSndMode_SpeakerHost			((HsSndMode) 0x0003) /**<Mode when radio is off and speaker used exclusively by system and third party audio. */
#define hsSndMode_Headset				((HsSndMode) 0x0004) /**<Mode when any audio goes to a mono headphones. */
#define hsSndMode_Headphones			((HsSndMode) 0x0005) /**<Mode when any audio goes to stereo headphones. */
#define hsSndMode_HeadphonesHost		((HsSndMode) 0x0006) /**<Mode when radio is off and headphones used exclusively by system and third party audio. */
#define hsSndMode_Carkit				((HsSndMode) 0x0007) /**<Mode when wired carkit is attached and active to the device.*/
#define hsSndMode_Reserved				((HsSndMode) 0x0008) /**<Reserved for future definition. Do not use. */
/*@}*/

/** Holds audio mode type */
typedef Int16 HsSndMode;

/**
 * @name Tone type parameter
 *
 */
/*@{*/
#define hsSndToneType_Square			0  /**<Used when calling HsSndTonePlay. */
/*@}*/

/**
 * Sound command values
 *
 * The HsSndCmdType is used as the first parameter to the
 * function HsSndFormatPlay.  Use these command values.
 */
enum HsSndCmdEnumTag
{
	hsSndCmdPlay = 1,	/**<Use to initiate a playback session. */
	hsSndCmdStop		/**<Use to terminate the current playback session. */

};

/** Holds sound command type */
typedef UInt16 HsSndCmdType;

/** Holds sound port ID information. */
typedef UInt32 HsSndPortID;

/** Holds sound switch ID information. */
typedef UInt32 HsSndSwitchID;

/** Used as a parameter to HsSndPortSetParam function. */
typedef UInt16 HsSndParam;

/** Used as a parameter to HsSndSwitchGetParam function. */
typedef UInt16 HsSndPortClass;

/** Holds sound format type */
typedef UInt16 HsSndFormatType;

#define hsSndBayID_AC97					0x01000000 /**<An AC97 compatible chip is supported as the analog sound patch bay. */

/**
 * @name Utility Macros - I
 *
 */
/*@{*/
#define HS_AC97(n)						((n) | hsSndBayID_AC97) /**<Associates the AC97 Bay ID with other element IDs */
#define HS_RDIO(n)						((n) | hsSndBayID_Radio) /**<Associates the Radio Bay ID (hsSndBayID_Radio) with other element IDs */
#define HsSndParamValue_SetWithLock(v)	(((v) & ~hsSndParamMode_Mask) | hsSndParamMode_LockParam) /**<Adds LockParam to parameter value. */
#define HsSndParamValue_SetWithUnlock(v)(((v) & ~hsSndParamMode_Mask) | hsSndParamMode_UnlockParam) /**<Adds UnLockParam to parameter value. */
#define HsSndParamValue_RemoveMode(v)	(((v) & 0x8000)? (v) | hsSndParamMode_Mask : (v) & ~hsSndParamMode_Mask) /**<Removes LockParam or UnLockParam from parameter value. */
/*@}*/

/**
 * @name Sound switch configuration types
 *
 * Used with HsSndSwitchID.
 */
/*@{*/
#define hsSndSwitchID_InRadioSpkr		HS_AC97(0x00010000) /**<Controls the destination port for data coming from the Radio. */
#define hsSndSwitchID_OutRadioMic		HS_AC97(0x00020000) /**<Controls the source microphone to connect to the Radio. */
#define hsSndSwitchID_InMic				HS_AC97(0x00030000) /**<Test control for Mfg to directly connect Mic input into any destination speaker. */
#define hsSndSwitchID_InHostPlay		HS_AC97(0x00040000) /**<Controls which speaker to direct PalmOS audio to.*/
#define hsSndSwitchID_OutHostRec		HS_AC97(0x00050000) /**<Controls from which port to digitize and send to PalmOS for recording. */
/*@}*/

/**
 * @name Sound port configuration types
 *
 * Used with HsSndPortID.
 */
/*@{*/
#define hsSndPortID_Off					0x00000000 /**<Special meta port ID used to turn off a sound switch. */
#define hsSndPortID_Current				HS_AC97(0x00000000) /**<Special meta port ID used to specify currently active user port. */
#define hsSndPortID_OutReceiver			HS_AC97(0x00000001) /**<Receiver speaker. */
#define hsSndPortID_OutSpeaker			HS_AC97(0x00000002) /**<Back speaker for system audio and speakerphone functions, */
#define hsSndPortID_OutHeadset			HS_AC97(0x00000003) /**<Audio output to mono headset (right channel output).  Left can be used as input. */
#define hsSndPortID_OutHeadphones		HS_AC97(0x00000004) /**<Configures the headset port for full left and right stereo output, no inputs. */
#define hsSndPortID_OutSpeakerAuto		HS_AC97(0x00000005) /**<Automatic port, according to headset insertion. Use with hsSndSwitchID_InRadioSpkr. */
#define hsSndPortID_InLineRec			HS_AC97(0x00000006) /**<Configures headset jack as input, precluding headset use (special port for test purposes). */
#define hsSndPortID_InBaseMic			HS_AC97(0x00000007) /**<Microphone at the base of the smartphone body. */
#define hsSndPortID_InHeadsetMic		HS_AC97(0x00000008) /**<Microphone on a mono headset. Ccomes from left channel as input. */
#define hsSndPortID_InMicAuto			HS_AC97(0x00000009) /**<Atomatic port, according to headset insertion. Use with hsSndSwitchID_OutRadioMic. */
#define hsSndPortID_OutRadioMic			HS_AC97(0x0000000A) /**<Output connected to the Radio microphone input. */
#define hsSndPortID_InRadioSpkr			HS_AC97(0x0000000B) /**<Input from the Radio speaker output. */
#define hsSndPortID_OutHostRec			HS_AC97(0x0000000C) /**<Output to the Host digitized recording input. */
#define hsSndPortID_InHostPlay			HS_AC97(0x0000000D) /**<Input from the Host converted to analog for output to speakers. */
#define hsSndPortID_OutBtHeadset		HS_AC97(0x0000000E) /**<Output port used by InRadioSpkr switch to put Call in bluetooth headset mode. */
#define hsSndPortID_OutBtHandsfree		HS_AC97(0x0000000F) /**<Output port used by InRadioSpkr switch to put Call in bluetooth handsfree mode. */
#define hsSndPortID_InHostBaseMic		HS_AC97(0x00000010) /**<Input port that controls source base port for host recording. */
#define hsSndPortID_InHostHeadsetMic	HS_AC97(0x00000011) /**<Input port controlling source headset port for host recording. */
#define hsSndPortID_InHostPhone			HS_AC97(0x00000012) /**<Input port used by host if phone or TTY is active. */
#define hsSndPortID_InCarkit			HS_AC97(0x00000013) /**<Input port used when wired carkit is activated. */
#define hsSndPortID_OutCarkit			HS_AC97(0x00000014) /**<Output port used when wired carkit is activated. */
#define hsSndPortID_LastPort			HS_AC97(0x00000014) /**<Update at end.  Ssame ID as last defined port in sequence. */
/*@}*/

/**
 * @name RadioBayID
 */
/*@{*/
#define hsSndBayID_Radio				0x02000000 /**<A defined patch bay for Telephony use. */
/*@}*/

/**
 * @name Sound switch configuration type for Radio patch bay
 *
 * Used with HsSndSwitchID.
 */
/*@{*/
#define hsSndSwitchID_RadioCall			HS_RDIO(0x00010000) /**<Keeps track of the audio connection of a Radio call. */
/*@}*/

/**
 * @name Sound port configuration type for hsSndSwitchID_RadioCall
 *
 *  hsSndPortID_Off is already defined for all patch bays.  Use it to turn off the switch indicating no radio call connected.
 */
/*@{*/
#define hsSndPortID_ConnectCall			HS_RDIO(0x00000020) /**<Virtual port to indicate a radio call is now connected. */
/*@}*/

/**
 * @name Sound port classes
 *
 * Used when passing a HsSndPortClass parameter in HsSndSwitchGetPort and HsSndSwitchSetPort functions.
 */
/*@{*/
#define hsSndPortClass_Virtual			((UInt16) 0) /**<Symbolic port definition that applies to any port. Use for portID save and restore operations. */
#define hsSndPortClass_Real				((UInt16) 1) /**<Current physical port. Use to change port parameters. Applies only to actual ports, not "auto" ports. */
#define hsSndPortClass_BaseDefault		((UInt16) 2) /**<Default port to use when restoring a Built-In port (For example, user unplugged headset while "auto" port was enabled.) */
#define hsSndPortClass_Anchor			((UInt16) 3) /**<Use to obtain the fixed port in a switch that doesn't change--the port that is patched to other ports. Valid only with HsSndSwitchGetPort.  */
/*@}*/

/**
 * @name Sound parameters
 *
 * Used when passing a parameter of type HsSndParam.
 */
/*@{*/
#define hsSndParam_Volume				((UInt16) 1) /**<Changes the volume of a port */
#define hsSndParam_Mute					((UInt16) 2) /**<Change the mute state of a port. */
#define hsSndParam_Boost				((UInt16) 3) /**<Changes the boost state of a port. */
#define hsSndParam_Pan					((UInt16) 4) /**<Change the panning value of a port. */
/*@}*/

/**
 * @name Sound parameter expansion macros
 *
 * Used to add sound parameter values for additional sound parameter control.  For example, muteState = HsSndParamValue_SetWithLock(hsSndMute_Enable);.
 */
/*@{*/
#define hsSndParamMode_Mask				0x7000	/**<The 3 upper bits after the sign bit in parameter values are reserved for the parameter mode. */
#define hsSndParamMode_Normal			0x0000	/**<Default parameter mode allowing parameter to be changed with any other hsSndParamMode. */
#define hsSndParamMode_LockParam		0x4000	/**<Limits parameter so it can only be changed with hsSndParamMode_LockParam or hsSndParamMode_UnlockParam. */
#define hsSndParamMode_UnlockParam		0x2000	/**<Removes hsSndParamMode_LockParam so it can be changed with any other hsSndParamMode. */
/*@}*/


/**
 * @name Mute parameters
 * Used to enable or disable a hsSndParam_Mute parameter.
 */
/*@{*/
#define hsSndMute_Enable				1	/**<Activates mute on a port. */
#define hsSndMute_Disable				0	/**<Disables mute on a port. */
/*@}*/

/**
 * @name Boost parameters
 *
 * Used to enable or disable a hsSndParam_Boost parameter.
 */
/*@{*/
#define hsSndBoost_Enable				1	/**<Activates boost on a port. */
#define hsSndBoost_Disable				0	/**<Disables boost on a port. */
/*@}*/

/**
 * @name Pan parameters
 *
 * Used as special hsSndParam_Pan parameters otherwise use actual Pan value.
 */
/*@{*/
#define hsSndPan_Middle					((Int16) 0x0000) /**<Pan at midpoint between left and right. */
#define hsSndPan_Right					((Int16) 0x7FFF) /**<Pan all the way to the right.  Cannot use with hsSndParamMode (defaults to Normal mode). */
#define hsSndPan_Left					((Int16) 0x8000) /**<Pan all the way to the left.  Cannot use with hsSndParamMode (defaults to Normal mode). */
#define hsSndPan_RightNormal			((Int16) 0x07FF) /**<Pan all the way to the right. Can use with any hsSndParamMode. */
#define hsSndPan_LeftNormal				((Int16) 0x0800) /**<Pan all the way to the left.  Can use with any hsSndParamMode. */
/*@}*/

/**
 * @name Sound port flag settings
 *
 * Used with the "flags" variable in HsSndPortInfo structure.
 */
/*@{*/
#define hsSndPortFlags_Enabled			0x00000001	/**<Port is enabled and in use. */
#define hsSndPortFlags_Disabled			0x00000000	/**<Port is disabled. */
#define hsSndPortFlags_EnableMask		0x00000001  /**<No definition. */

#define hsSndPortFlags_MuteOn			0x00000002	/**<Volume is muted. */
#define hsSndPortFlags_MuteOff			0x00000000	/**<Volume is as specified. (Not muted). */
#define hsSndPortFlags_MuteMask			0x00000002  /**<No definition. */

#define hsSndPortFlags_TypeDigital		0x00000004	/**<Sound data through port is digital. */
#define hsSndPortFlags_TypeAnalog		0x00000000	/**<Sound data through port is analog. */
#define hsSndPortFlags_TypeMask			0x00000004  /**<No definition. */

#define hsSndPortFlags_BoostOn			0x00000008	/**<Boost control on port is active. */
#define hsSndPortFlags_BoostOff			0x00000000	/**<Boost control on port is inactive. */
#define hsSndPortFlags_BoostMask		0x00000008  /**<No definition. */

#define hsSndPortFlags_CanInputOnly		0x00000000	/**<Port is for sound input exclusively. */
#define hsSndPortFlags_CanOutputOnly	0x00000010	/**<Port is for sound output exclusively. */
#define hsSndPortFlags_CanIOSelect		0x00000020	/**<Port can switch between being for sound input or output. */
#define hsSndPortFlags_CanIOBothways	0x00000030	/**<Port can be for sound input and output simultaneously. */
#define hsSndPortFlags_CanIOMask		0x00000030  /**<No definition. */

#define hsSndPortFlags_InputOn			0x00000040	/**<Port as sound input is enabled. */
#define hsSndPortFlags_InputOff			0x00000000	/**<Port as sound input is disabled. */
#define hsSndPortFlags_InputMask		0x00000040  /**<No definition. */

#define hsSndPortFlags_OutputOn			0x00000080	/**<Port as sound output is enabled. */
#define hsSndPortFlags_OutputOff		0x00000000	/**<Port as sound output is disabled. */
#define hsSndPortFlags_OutputMask		0x00000080  /**<No definition. */

#define hsSndPortFlags_CanMute_Yes		0x00000100	/**<Port has mute control available. */
#define hsSndPortFlags_CanMute_No		0x00000000	/**<Port has no mute control. */
#define hsSndPortFlags_CanMuteMask		0x00000100  /**<No definition. */

#define hsSndPortFlags_CanBoost_Yes		0x00000200	/**<Port has boost control available. */
#define hsSndPortFlags_CanBoost_No		0x00000000	/**<Port has no boost control. */
#define hsSndPortFlags_CanBoostMask		0x00000200  /**<No definition. */

#define hsSndPortFlags_CanPan_Yes		0x00000400	/**<Port has pan control available. */
#define hsSndPortFlags_CanPan_No		0x00000000	/**<Port has No pan control. */
#define hsSndPortFlags_CanPanMask		0x00000400  /**<No definition. */

#define hsSndPortFlags_CanVol_Yes		0x00000800	/**<Port has volume control available. */
#define hsSndPortFlags_CanVol_No		0x00000000	/**<Port has No volume control. */
#define hsSndPortFlags_CanVolMask		0x00000800  /**<No definition. */

#define hsSndPortFlags_Mute_Locked		0x00001000	/**<If locked, can only be changed with lock/unlock mute parameter changes. */
#define hsSndPortFlags_Mute_Unlocked	0x00000000	/**<Mute can be changed with any type of mute parameters. */
#define hsSndPortFlags_Mute_LockMask	0x00001000  /**<No definition. */

#define hsSndPortFlags_Boost_Locked		0x00002000	/**<If locked, can only be changed with lock/unlock boost parameter changes/ */
#define hsSndPortFlags_Boost_Unlocked	0x00000000	/**<Boost can be changed with any type of boost parameters. */
#define hsSndPortFlags_Boost_LockMask	0x00002000  /**<No definition. */

#define hsSndPortFlags_Pan_Locked		0x00004000	/**<If locked, can only be changed with lock/unlock pan parameter changes. */
#define hsSndPortFlags_Pan_Unlocked		0x00000000	/**<Pan can be changed with any type of pan parameters. */
#define hsSndPortFlags_Pan_LockMask		0x00004000  /**<No definition. */

#define hsSndPortFlags_Vol_Locked		0x00008000	/**<If locked, can only be changed with lock/unlock volume parameter changes. */
#define hsSndPortFlags_Vol_Unlocked		0x00000000	/**<Volume can be changed with any type of volume parameters. */
#define hsSndPortFlags_Vol_LockMask		0x00008000  /**<No definition. */
/*@}*/

/**
 * @brief Holds sound port information.
 */
typedef struct
{
	UInt32		signature;	 /**< Always set to hsSndPortInfo_Signature. */
	UInt32		flags;		 /**< Control and information about port. */
	HsSndPortID	portID;		 /**< Identifies the portID. */
	Int16		minVol;		 /**< Bottom value of the volume range.*/
	Int16		maxVol;		 /**< Top value of the volume range.*/
	Int16		unityVol;	 /**< Volume value when there is no gain or dampening.*/
	Int16		stepVol;     /**< Step size for volume changes within vol range.*/
	Int16		currentVol;  /**< Tracks current volume setting.*/
	Int16		currentPan;	 /**< Pan control (0 is midpoint pan between L and R) max pan = maxVol - minVol. */
	UInt16		stepDB;		 /**< How many dBs per step = (stepDB >> 8) / (stepDB & 0x0F). */
	UInt16		boostDB;     /**< How many dBs with boost = (boostDB >> 8) / (boostDB & 0x0F). */
	UInt16		reserved1;	 /**< Reserved field, set to zero */
	UInt16		reserved2;	 /**< Reserved field, set to zero */
} HsSndPortInfo, *HsSndPortInfoPtr;


/**
 * @name Command flag settings
 *
 * Used with the "Commandflags" parameter in HsSndFormatPlay function.
 */
/*@{*/
#define hsSndCmdFlags_noWait		0x00000001  /**< Use wait period for the play command. */
#define hsSndCmdFlags_yesWait		0x00000000  /**< No wait period for the play command. */
#define hsSndCmdFlags_waitMask		0x00000001 /**<No definition. */
/*@}*/


/**
 * Sound formats
 */
enum HsSndFormatEnumTag
{
	hsSndFormatMidi = 2, /**<Midi format. */
	hsSndFormatAmr, /**<Obsolete.  Use Codec Plug-in Manager or streaming component. */
    pmSndFormatSetAttackEnvID /**<For internal use only (escape code used by PmSndStreamSetAttackEnvID macro function). */

};


/**
 * @name Sound buffer flag settings
 *
 * Used with the "flags" parameter in HsSndBufferData.
 */
 /*@{*/
#define hsSndBufFlags_canInterrupt		0x0001	/**< Playback can be interrupted by user events. */
#define hsSndBufFlags_cannotInterrupt	0x0000	/**< (Default) Playback cannot be interrupted by user events. */
#define hsSndBufFlags_interruptMask		0x0001 /**<No definition. */

#define hsSndBufFlags_yesRepeat			0x0002	/**< Repeat the buffer using the "repeat" count in HsSndBufferData.controls. */
#define hsSndBufFlags_noRepeat			0x0000	/**< (Default) Buffer used only once. */
#define hsSndBufFlags_repeatMask		0x0002 /**<No definition. */

#define hsSndBufFlags_yesEndDelay		0x0004	/**< Delay completion of playback using the "delay" count in HsSndBufferData.controls. */
#define hsSndBufFlags_noEndDelay		0x0000	/**< (Default) No delay after last batch of audio data processed. */
#define hsSndBufFlags_endDelayMask		0x0004 /**<No definition. */

#define hsSndBufFlags_attackEnvNone     0x0000 /**< (Default) No attack envelope to be used. */
#define hsSndBufFlags_attackEnvRings    0x0010 /**< Use special attack envelope style suitable for call ringtones. */
#define hsSndBufFlags_attackEnvMask     0x00F0 /**< Mask reserves up to 16 different envelope IDs. */
/*@}*/

/**
 * @name Buffer amplitude settings
 *
 * Used with HsSndBufferData.amplitude field.
 */
/*@{*/
#define hsSndAmplitude_silent			((UInt32) 0) /**< Boost control on port is active. */
#define hsSndAmplitude_unity			((UInt32) 1024) /**< Boost control on port is inactive. */
/*@}*/

/******************************************************************************/
/* HsSndBufferProc */
/**
   @brief Callback function used under various conditions with sound.

   @param userdata		IN  Userdata passed when setting the callback routine
   @param flags			IN  Copy of the HsSndBufferData flags field
   @param buffer        IN	Pointer originally passed in HsSndBufferData
   @param bytesUsed		OUT How many bytes have been consumed from the buffer
   @param time			OUT	Number of milliseconds available for callback processing

   @return				Error code.

*******************************************************************************/
typedef Err (*HsSndBufferProc)  (void * userdata, UInt32 flags,
								 void * buffer, UInt32 bytesUsed, UInt32 time);

/**
 * @brief Holds sound buffer information.
 *
 * When using the hsSndCmdPlay command in function HsSndFormatPlay,
 * you should pass a pointer to a properly filled HsSndBufferData
 * structure. This structure provides information about the buffer
 * of coded audio data that is going to be processed and played.
 */
typedef struct HsSndBufferData
{
	UInt32			signature;	/**<Always set to hsSndBufferData_Signature or error returned. */
	UInt32			flags;		/**<Flags and status bits for the buffer. Use the hsSndBufferFlags_* symbols appropriately.  Set any undefined or unused flags to zero.*/
	UInt32			amplitude;	/**<Volume level for playback. (Valid range from 0 to hsSndAmplitude_unity.)*/
	void *			buffer;		/**<Pointer to beginning of coded audio data to process and/or play. */
	UInt32			bufferSize;	/**<Size in bytes of the buffer of coded audio data. */
	HsSndBufferProc	progress_callback; /**<When Not NULL, is invoked at regular intervals. If progress_callback returns an error, then the play session terminates.*/
	void *			progress_userdata; /**<Passed to progress_callback when invoked. */
	HsSndBufferProc	completion_callback; /**<When Not NULL, it will be invoked when buffer processing completed. */
	void *			completion_userdata; /**<Passed to completion_callback when invoked. */

   /**
    * Compound field with repeat count and end delay controls.
    * Use HsSndBufControl_Set* macros with this field. Repeat Field: valid range from 1 to 254,
    * or hsSndBufControl_repeatForever (255). The processing of the buffer will be looped
    * according to this number. When repeatForever is used, then termination should be
    * done by returning an error code from the progress callback. Alternatively,
    * for non-blocking playback, the caller may issue another "Play" call with the "Stop"
    * command. This field is valid and non-zero when hsSndBufFlags_yesRepeat is set
    * in the buffer flags, otherwise this field must be set to zero. A repeat value of 1
    * means play one time. Values of 2 and greater will perform the requested number of
    * playbacks by looping on the same buffer data. Use HsSndBufControl_SetRepeat macro to set
    * values in this field. endDelay fld:  Valid range from 1 to 255, each unit is a
    * 1/50th second delay. Use HsSndBufControl_SetEndDelay macro to set values in this field.
    * This field is valid and non-zero when hsSndBufFlags_yesEndDelay is set in the buffer
    * flags, otherwise this field must be set to zero.
    */
	UInt32			controls;
	UInt32			reserved1;	/**<reserved, must set to zero*/
	UInt32			reserved2;	/**<reserved, must set to zero*/

} HsSndBufferData;


#define hsSndMidiMsg_SetVolume	1


#if CPU_ENDIAN == CPU_ENDIAN_BIG
	#define hsSndMidiProgressMsg_Signature	'MidP'
#else
	#define hsSndMidiProgressMsg_Signature	'PdiM' /* 'MidP' char array as UInt32 little endian */
#endif
typedef struct HsSndMidiProgressMsg
{
	UInt32		signature;
	UInt32		msgType;
	UInt32		param1;
	UInt32		param2;

} HsSndMidiProgressMsg;


#if CPU_ENDIAN == CPU_ENDIAN_BIG
	#define hsSndPortInfo_Signature	'SPRT'
#else
	#define hsSndPortInfo_Signature	'TRPS' /* "SPRT" char array as UInt32 little endian */
#endif

#if CPU_ENDIAN == CPU_ENDIAN_BIG
	#define hsSndBufferData_Signature	'SBDA'
#else
	#define hsSndBufferData_Signature	'ADBS' /* 'SBDA' as little endian UInt32 */
#endif

/**
 * @name Buffer control macros
 *
 * Use these macros to set or get the proper control in
 * HsSndBufferData.controls field.
 */
/*@{*/
#define HsSndBufControl_SetRepeat(controls, value)	 (controls = ((controls & 0xFFFFFF00) |  (((UInt32) value) & 0xFF)      ))
#define HsSndBufControl_GetRepeat(controls)			 (controls & 0x000000FF)
#define HsSndBufControl_SetEndDelay(controls, value) (controls = ((controls & 0xFFFF00FF) | ((((UInt32) value) & 0xFF) << 8)))
#define HsSndBufControl_GetEndDelay(controls)		 ((controls & 0x0000FF00) >> 8)
#define hsSndBufControl_repeatForever				 (0xFF)
/*@}*/


// ===============================================================================================
// Gain Envelope Feature Support
// ===============================================================================================
/**
 * @name Gain Envelope Definitions
 *
 * Used as attackEnvID parameter to function PmSndStreamSetAttackEnvID.
 */
 /*@{*/
#define pmSndAttackEnvIDRings       (hsSndBufFlags_attackEnvRings) /**< The ringtone attack envelope ramp-up to low volume initially, and to full volume after about 4 seconds */


// ===============================================================================================
// Sound Preference Definitions (compatible with SndBayID and SndPortID)
// ===============================================================================================
#define pmSndBayID_AUDP					0x0F000000 /**< we are reserving SndBayID 15 for audio preference purposes */

#define PM_AUDP_CLASS_MASK				0x4000
// Utility macro to associate special port class marker to this IDs (so we can use it as an HsSndPortClass parameter)
#define PM_AUDP_CLASS(n)				((n) | PM_AUDP_CLASS_MASK) /**< utility to associate preferences class marker to prefIDs */

#define pmSndPrefID_AppsGroupHoldType	PM_AUDP_CLASS(0x0001) /**< specify how to hold application audio (mute/pause) */
#define pmSndPrefID_JackOutIfMicUnused	PM_AUDP_CLASS(0x0002) /**< shall we force jack out to stereo if Mic not in use? */
#define pmSndPrefID_PhoneRxGainLimit	PM_AUDP_CLASS(0x0003) /**< set if gain-limit phone Rx to receiver speaker */
#define pmSndPrefID_WakeOnAnswerBtn		PM_AUDP_CLASS(0x0004) /**< set to be able to wake device when headset answer button is pressed */
#define pmSndPrefID_AudioTable			PM_AUDP_CLASS(0x0005) /**< pass pointer to PmSndGain8Table to load for use, pass zero to return to default */
#define pmSndPrefID_GsmMccArray			PM_AUDP_CLASS(0x0006) /**< pass pointer to PmSndGsmMccArray to load for use, pass zero to return to default */
#define pmSndPrefID_AthenaActive		PM_AUDP_CLASS(0x0007) /**< set to activate audio paths to multi-connector, clear to return to normal */
#define pmSndPrefID_SleepMode			PM_AUDP_CLASS(0x0008) /**< set to zero for normal sleep, other values for partial audio sleep modes (for internal use only) */
#define pmSndPrefID_AGCEvent			PM_AUDP_CLASS(0x0009) /**< set to temporarily activate AGC if required (only useful when AGC has been disabled before hand) */

typedef UInt16	PmSndPrefID;

// Parameters to use with pmSndPrefID_WakeOnAnswerBtn
#define pmSndPref_WakeOnAnswerBtn_Disable	(0) /**< disable wake up on answer button feature */
#define pmSndPref_WakeOnAnswerBtn_Enable	(1) /**< enable  wake up on answer button feature */

// Parameters to use with pmSndPrefID_AthenaActive
#define pmSndPref_AthenaActive_No		(0) /**< disable routing to Athena path for automatic switches */
#define pmSndPref_AthenaActive_Yes		(1) /**< enable routing to Athena path for automatic switches */

// Utility macro to associate the hsSndBayID_Radio to other elements
#define PM_AUDP(n)						((n) | pmSndBayID_AUDP) /**< utility to associate the Preference Bay ID with other element ID */

// Use these for HsSndSwitchID in the Preferences patch bay
#define pmSndSwitchID_Preferences		PM_AUDP(0x00400000) /**< used with the HsSoundLib Switch calls to set and get sound preference data */



#endif // __HSSOUNDLIB_COMMON__H__
