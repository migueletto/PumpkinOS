/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Tone
 *
 */

/**
 * @file   TonesLib.h
 * @brief  Public 68K include file that controls the MIDI tones on a device.
 *
 * Notes:
 *
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 * synchronization.
 */

#ifndef __TONESLIB__H__
#define __TONESLIB__H__

#ifdef __cplusplus
extern "C" {
#endif

/// Open and initialize the Tones library instance.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibOpen (UInt16 refNum)
                 SYS_TRAP (sysLibTrapOpen);

/// Close the Tones library instance.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibClose (UInt16 refNum)
                 SYS_TRAP (sysLibTrapClose);

/// Standard function to receive Palm OS wake up notification.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibWake (UInt16 refNum)
                 SYS_TRAP (sysLibTrapWake);

/// Standard function to receive Palm OS sleep notification.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibSleep (UInt16 refNum)
                 SYS_TRAP (sysLibTrapSleep);

/// Get the list of tone names.
///
/// @param refNum:		IN:	 Library reference number from SysLibLoad.
/// @param list:		IN:	 Pointer to the list of tone names to return.
/// @param listLength:	OUT: Count of how many tone names are in the list.
/// @return Library error code.
Err TonesLibGetToneList (UInt16 refNum, ToneItemPtr * list, UInt16 * listLength)
                 SYS_TRAP (tonesLibTrapGetToneList);

/// Get the list of tone IDs.
///
/// @param refNum:		IN:  Library reference number from SysLibLoad.
/// @param list:		IN:  Pointer to the list of tone IDs to return.
/// @param listLength:	OUT: Count of how many tone IDs are in the list.
/// @return Library error code.
Err TonesLibGetToneIDs ( UInt16 refNum, ToneIdentifier ** list, UInt16 * listLength)
                 SYS_TRAP (tonesLibTrapGetToneIDs);

/// Get the name of a certain tone given its ID.
///
/// @param refNum:		IN:  Library reference number from SysLibLoad.
/// @param toneId:		IN:  Tone ID of the tone name to get.
/// @param name:		OUT: Pointer to the returned tone name.
/// @param maxLength:	OUT: Character length of the tone name.
/// @return Library error code.
Err TonesLibGetToneName (UInt16 refNum, ToneIdentifier toneId, CharPtr name, UInt16
	maxLength)
                 SYS_TRAP (tonesLibTrapGetToneName);

/// Get the tone data size given its ID.
///
/// @param refNum:		IN:	 Library reference number from SysLibLoad.
/// @param toneId:		IN:  ID number of the tone.
/// @param sizeP:		OUT: Size of the specified tone.
/// @return Library error code.
Err TonesLibGetToneSize (UInt16 refNum, ToneIdentifier toneId, UInt32* sizeP)
                 SYS_TRAP (tonesLibTrapGetToneSize);

/// Play a tone given its ID.
///
/// @param refNum:		IN:	Library reference number from SysLibLoad.
/// @param toneId:		IN:	ID of the tone to play.
/// @param playCount:	IN: Number of times you want the tone repeated.
/// @param volume:		IN: Volume level at which you want the tone played.
/// @param vibrate:		IN: Vibrate mode to use when playing the tone.
/// @param blocking:	IN: Controls whether the tone play is a blocking call
///							or play in the background and return immediately.
/// @return Library error code.
Err TonesLibPlayTone (UInt16 refNum, ToneIdentifier toneId, UInt16 playCount,
	ToneVolume volume, ToneVibrateType vibrate, Boolean blocking)
                 SYS_TRAP (tonesLibTrapPlayTone);

/// Add a MIDI tone to the system tone database.
///
/// @param refNum:			IN:	Library reference number from SysLibLoad.
/// @param midiTone:		IN: The data buffer handle where the midi tone data is located.
/// @param toneName:		IN: Name of the tone.
/// @param protectedTone:	IN: Save tone as protected record in the tone database.
/// @return Library error code.
Err TonesLibAddMidiTone (UInt16 refNum, MemHandle midiTone, CharPtr toneName,
	Boolean protectedTone)
                 SYS_TRAP (tonesLibTrapAddMidiTone);

/// Create new tone and return the pointer to the start of the sound stream.
///
/// @param refNum:			IN:  Library reference number from SysLibLoad.
/// @param streamP:			OUT: Pointer to the sound stream of the new tone.
/// @param toneType:		IN:  Type of the tone. @see ToneType.
/// @param toneName:		IN:  Name of the tone.
/// @param protectedTone:	IN:  Save tone as protected record in the tone database.
/// @return Library error code.
Err TonesLibToneCreate (UInt16 refNum, UInt32* streamP, ToneType toneType,
						CharPtr toneName, Boolean protectedTone)
                 SYS_TRAP (tonesLibTrapToneCreate);

/// Write sound data to the stream returned by TonesLibToneCreate().
///
/// @param refNum:	 IN: Library reference number from SysLibLoad.
/// @param stream:	 IN: Reference to the steam returned by TonesLibToneCreate().
/// @param dataP:	 IN: Pointer to the Tone(Sound) data.
/// @param size:	 IN: Size of the data to be written.
/// @return Library error code.
Err TonesLibToneWrite (UInt16 refNum, UInt32 stream, const void* dataP, Int32 size)
                 SYS_TRAP (tonesLibTrapToneWrite);

/// Close the reference to the tone stream.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @param stream: IN:  Reference to the steam returned by TonesLibToneCreate().
/// @return Library error code.
Err TonesLibToneClose (UInt16 refNum, UInt32 stream)
                 SYS_TRAP (tonesLibTrapToneClose);

/// Display UI to manage tones. Users can delete, play, or send a tone from this UI.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibManageTones (UInt16 refNum)
                 SYS_TRAP (tonesLibTrapManageTones);

/// Delete a tone given its ID.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @param toneId: IN:	ID of the tone to be deleted.
/// @return Library error code.
Err TonesLibDeleteTone (UInt16 refNum, ToneIdentifier toneId)
                  SYS_TRAP (tonesLibTrapDeleteTone);

/// Create the default tone database if it does not exist, and verify that all the tones
/// in a database are resident on the device.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibInitTonesDB (UInt16 refNum)
                   SYS_TRAP (tonesLibTrapInitTonesDB);

/// Get the sound preference for a type of (application) sound.
///
/// @param refNum:		 IN:	Library reference number from SysLibLoad.
/// @param soundType:	 IN:	Type of (application: email, SMS, phone call, etc) sound.
/// @param soundPrefs:	 OUT:	Preference settings for a type of sound.
/// @return Library error code.
Err TonesLibGetSoundPrefs (UInt16 refNum, SoundPrefType soundType, SoundPreference * soundPrefs)
                  SYS_TRAP (tonesLibTrapGetSoundPrefs);

/// Set the sound preference for a type of (application) sound.
///
/// @param refNum:		 IN:	Library reference number from SysLibLoad.
/// @param soundType:	 IN:	Type of (application) sound.
/// @param soundPrefs:	 IN:	Preference settings for the type of sound.
/// @return Library error code.
Err TonesLibSetSoundPrefs (UInt16 refNum, SoundPrefType soundType, SoundPreference * soundPrefs)
                  SYS_TRAP (tonesLibTrapSetSoundPrefs);

/// Display the UI to select tones and return the tone type, tone name, and handle to
/// the tone data.
///
/// @param refNum:		IN:	 Library reference number from SysLibLoad.
/// @param toneType:	OUT: Type of tone.
/// @param toneH:		OUT: Handle to the tone selected. Must be freed by the caller.
/// @param name: 		OUT: Name of the tone.
/// @return Library error code.
Err TonesLibPickTone (UInt16 refNum, ToneType * toneType, MemHandle *toneH, Char
	* name)
                  SYS_TRAP (tonesLibTrapPickTone);

/// Stop the playback of the current tone.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibStopTone (UInt16 refNum)
                  SYS_TRAP (tonesLibTrapStopTone);

/// Get the sound-on volume of a sound type.
///
/// @param refNum:			IN:	 Library reference number from SysLibLoad.
/// @param soundType:		IN:	 Type of (application) sound.
/// @param soundOnVolumeP:	OUT: Volume of the sound.
/// @return Library error code.
Err	TonesLibGetSoundOnVolume (UInt16 refNum, SoundPrefType soundType,
							  ToneVolume* soundOnVolumeP)
				  SYS_TRAP (tonesLibTrapGetSoundOnVolume);

/// Set the sound-on volume of a sound type.
///
/// @param refNum:			IN:	Library reference number from SysLibLoad.
/// @param soundType:		IN:	Type of (application) sound.
/// @param soundOnVolume:	IN:	Volume of the tone.
/// @return Library error code.
Err	TonesLibSetSoundOnVolume (UInt16 refNum, SoundPrefType soundType,
							  ToneVolume soundOnVolume)
				  SYS_TRAP (tonesLibTrapSetSoundOnVolume);

/// Display the UI to create and/or manages tones.
///
/// @param refNum:	IN:	Library reference number from SysLibLoad.
/// @param flags:	IN: Set to kTonesDialogFlagNonModal to hide the Done button from the dialog.
/// @return Library error code.
Err TonesLibDoDialog (UInt16 refNum, UInt32 flags)
                 SYS_TRAP (tonesLibTrapDoDialog);

/// No definition.
///
/// @param refNum: IN:	Library reference number from SysLibLoad.
/// @return Library error code.
Err TonesLibRecordTone (UInt16 refNum)
				SYS_TRAP (tonesLibTrapToneRecord);

#ifdef __cplusplus
}
#endif

#endif // __TONESLIB__H__
