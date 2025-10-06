/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 * @file	HsSoundLib.h
 *
 * @brief	Public 68K include file for main sound support for Treo 600
 * 			and Treo 650 smartphones.
 *
 * This header file and associated header files support the specific sound
 * functionality of the Treo smartphones. You should use the Palm OS Sound
 * Manager APIs for most of your work.
 *
 * Notes:
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 */

#ifndef __HSSOUNDLIB__H__
#define __HSSOUNDLIB__H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/
#include <PalmOS.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Preference Macros
 */
/*@{*/
#define PmSndSetPreference( libRef, prefID, prefValue ) \
        HsSndSwitchSetPort( libRef, pmSndSwitchID_Preferences, (HsSndPortID) (prefValue), (HsSndPortClass) (prefID) );

#define PmSndGetPreference( libRef, prefID, prefValuePtr ) \
        HsSndSwitchGetPort( libRef, pmSndSwitchID_Preferences, (HsSndPortID *) (prefValuePtr), (HsSndPortClass) (prefID) );
/*@}*/


/**
 * @name Special Function Macros
 */
/*@{*/
#define PmSndStreamSetAttackEnvID(libRef,stream,attackEnvID) HsSndFormatPlay( libRef, (HsSndFormatType) pmSndFormatSetAttackEnvID, (HsSndCmdType) attackEnvID, (void*)0, (UInt32) stream )


/// Open and initialize the Sound library instance.
///
/// @param libRef:	IN: Library reference number (from SysLibLoad)
/// @return Library error code.
Err	  HsSndOpen (UInt16 libRef)
		  SYS_TRAP (kHsSoundLibTrapOpen);

/// Close the Sound library instance.
///
/// @param libRef:	IN: Library reference number (from SysLibLoad)
/// @return Library error code.
Err	  HsSndClose (UInt16 libRef)
		  SYS_TRAP (kHsSoundLibTrapClose);

/// Returns the library version number.
///
/// @param libRef:	IN: Library reference number (from SysLibLoad)
/// @return Library error code.
UInt32  HsSndGetVersion (UInt16 libRef)
		  SYS_TRAP (kHsSoundLibTrapGetVersion);

/// Returns information about the sound port.
///
/// @param libRef:		IN: Library reference number
/// @param portID:		IN: Sound port ID.
/// @param *portInfoP:	IN: Pointer to an HsSndPortInfo structure provided by the caller.
/// @return Library error code.
Err   HsSndPortGetInfo (UInt16 libRef, HsSndPortID portID, HsSndPortInfo *portInfoP)
		  SYS_TRAP (kHsSoundLibTrapPortGetInfo);

/// Set a sound port's audio parameter.
///
/// @param libRef:		IN: Library reference number (from SysLibLoad)
/// @param portID:		IN: Sound port ID.
/// @param paramSel:	IN: Parameter selector.
/// @param value:		IN: Value to set.
/// @param returnValue:	OUT: Pointer to the updated value.
/// @return Library error code.
Err   HsSndPortSetParam (UInt16 libRef, HsSndPortID portID, HsSndParam paramSel, Int16 value, Int16 * returnValue)
		  SYS_TRAP (kHsSoundLibTrapPortSetParam);

/// Get the current sound port for a particular switch.
///
/// @param libRef:		IN: Library reference number (from SysLibLoad)
/// @param switchID:	IN: Switch ID.
/// @param portIDP:		OUT: Port ID returned to the caller.
/// @param portClass:	IN: Port class selector.
/// @return Library error code.
Err   HsSndSwitchGetPort (UInt16 libRef, HsSndSwitchID switchID, HsSndPortID * portIDP, HsSndPortClass portClass)
		  SYS_TRAP (kHsSoundLibTrapSwitchGetPort);

/// Set the current sound port for a particular switch.
///
/// @param libRef:		IN: Library reference number (from SysLibLoad)
/// @param switchID:	IN: Switch ID.
/// @param portID:		IN: The new port ID for the audio switch.
/// @param portClass:	IN: Port class selector.
/// @return Library error code.
Err   HsSndSwitchSetPort (UInt16 libRef, HsSndSwitchID switchID, HsSndPortID portID, HsSndPortClass portClass)
		  SYS_TRAP (kHsSoundLibTrapSwitchSetPort);

/// Play a sound.
///
/// @param libRef:		IN: Library reference number (from SysLibLoad).
/// @param freq:		IN: Frequency of the tone.
/// @param amp:			IN: Amplitude of the tone.
/// @param duration:	IN: Duration of the tone.
/// @param toneType:	IN: Type of tone to play.
/// @return Library error code.
Err   HsSndTonePlay (UInt16 libRef, UInt32 freq, UInt16 amp, UInt32 duration, UInt32 toneType)
		  SYS_TRAP (kHsSoundLibTrapTonePlay);

/// Stop playing a sound that was started with HsSndTonePlay().
///
/// @param libRef:	IN: Library reference number (from SysLibLoad).
/// @return Library error code.
Err   HsSndToneStop (UInt16 libRef)
		  SYS_TRAP (kHsSoundLibTrapToneStop);

/// Play formatted audio data, such as MIDI, AMR, and so forth.
///
/// @param libRef:			IN: Library reference number (from SysLibLoad).
/// @param format:			IN: Audio format to be played.
/// @param command:			IN: Command to execute.  Should be set to hsSndCmdPlay.
/// @param commandData:		IN: Command to the sound buffer structure HsSndBufferData
/// @param commandFlags:	IN: Command flags for the play command.
/// @return Library error code.
Err   HsSndFormatPlay (UInt16 libRef, HsSndFormatType format, HsSndCmdType command, void * commandData, UInt32 commandFlags)
		  SYS_TRAP (kHsSoundLibTrapFormatPlay);

/// Records formatted audio data, such as MIDI, AMR, and so forth.
/// (Currently a stub and not implemented yet).
///
/// @param libRef:			IN: Library reference number (from SysLibLoad).
/// @param format:			IN: Audio format to be recorded.
/// @param command:			IN: Command to execute. Should be set to hsSndCmdPlay.
/// @param commandData:		IN: Pointer to the sound buffer structure HsSndBufferData.
/// @param commandFlags:	IN: The command flags for the record command.
/// @return Library error code.
Err   HsSndFormatRecord (UInt16 libRef, HsSndFormatType format, HsSndCmdType command, void * commandData, UInt32 commandFlags)
		  SYS_TRAP (kHsSoundLibTrapFormatRecord);

/// Get the user volume setting for the specified sound mode.
///
/// @param libRef:		IN: Library reference number (from SysLibLoad).
/// @param mode:		IN: The audio configuration mode.
/// @param volumeP:		IN: The sound level.
/// @return Library error code.
Err   HsSndGetUserVolume (UInt16 libRef, HsSndMode mode, Int16 * volumeP)
		  SYS_TRAP (kHsSoundLibTrapGetUserVolume);

/// Set the user volume for the specified sound mode.
///
/// @param libRef:	IN: Library reference number (from SysLibLoad).
/// @param mode:	IN: The audio configuration mode.
/// @param volume:	IN: The sound level.
/// @return Library error code.
Err   HsSndSetUserVolume (UInt16 libRef, HsSndMode mode, Int16 volume)
		  SYS_TRAP (kHsSoundLibTrapSetUserVolume);


#ifdef __cplusplus
}
#endif

#endif // __HSSOUNDLIB__H__
