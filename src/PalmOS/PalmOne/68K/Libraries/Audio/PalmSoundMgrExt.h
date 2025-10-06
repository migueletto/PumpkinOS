/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 * @file	PalmSoundMgrExt.h
 *
 * @brief	Public 68K include file for the Extended Sound Manager.
 *
 * Notes:
 * You should check to see if you are using a version of Palm OS SDK that
 * implements the Extended Sound API.  For lack of a better check,
 * look for the earlier of the system traps and hope that the rest is
 * likewise defined.
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


#ifndef __SOUNDMGREXT_H__
#define __SOUNDMGREXT_H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/
#include <SoundMgr.h>

#ifndef sysTrapSndStreamCreateExtended

/**
 * @name Extended Sound library traps
 *
 */
/*@{*/
#ifndef sysTrapSndStreamCreateExtended
#define sysTrapSndStreamCreateExtended				0xA466
#endif

#ifndef sysTrapSndStreamDeviceControl
#define sysTrapSndStreamDeviceControl				0xA467
#endif
/*@}*/

/**
 * Sound format type
 */
enum {
	sndFormatPCM		= 0,			/**<Raw PCM (Pulse Code Modulation), (attributes defined by type parameter). */
	sndFormatIMA_ADPCM	= 'APCM',		/**<IMA ADPCM (Interactive Multimedia Association Adaptive Differential PCM). */
	sndFormatDVI_ADPCM	= 'DPCM',		/**<Intel/MS/DVI ADPCM. */
	sndFormatMP3		= 'MPG3',		/**<Moving Picture Experts Group, Audio Layer III (MPEG-1 and MPEG-2 depending on bit-rate). */
	sndFormatAAC		= 'DAAC',		/**<Dolby Advanced Audio Coding. */
	sndFormatOGG		= 'OGGV'		/**<Ogg Vorbis. */
};

/** Holds sound format type. */
typedef UInt32 SndFormatType;


/// Extended Sound Manager callback.
///
/// @param userDataP:	IN:	Pointer to the user data passed in SndStreamCreateExtended.
/// @param streamRef:	IN:	Opaque reference to the stream returned by SndStreamCreateExtended.
/// @param bufferP:		IN:	Data buffer.
/// @param bufferSizeP:	IN: Pointer to the buffer size in bytes. This MUST be set before returning to the number of bytes read or written.
/// @retval	Err Error code.
typedef Err (*SndStreamVariableBufferCallback) (
	void 			*userDataP,
	SndStreamRef 	streamRef,
	void 			*bufferP,
	UInt32 			*bufferSizeP
);


#ifdef __cplusplus
extern "C" {
#endif

/// Creates a sound stream using the Extended Sound API.
///
/// @param streamP:		IN: Receives the opaque reference to the stream.
/// @param mode:		IN: Direction of the sream, either sndInput or sndOutput.
/// @param format:		IN: Sound format. (sndFormatPCM...)
/// @param sampleRate:	IN:	Sampling rate, in frames-per-second. (8000, 44100,...)
/// @param sampleType:	IN:	Sample size and endianness of the data. (sndInt16...)
/// @param width:		IN: The number of channels: sndMono for one channel, sndStrereo for two.
/// @param sndCallback:	IN:	A callback function that gets called when another buffer for sound is needed.
/// @param userDataP:	IN:	Caller-defined data that gets passed to SndStreamVariableBufferCallback.
/// @param bufferSize:	IN: Preferred size in bytes for the buffers that are passed to sndCallback.
/// @param armNative:	IN: Pass true if the callback function is written in ARM-native code. If it’s 68K, pass false.
/// @retval Err Error code.
extern Err SndStreamCreateExtended(
		SndStreamRef 					*streamP,
		SndStreamMode 					mode,
		SndFormatType 					format,
		UInt32 							sampleRate,
		SndSampleType 					sampleType,
		SndStreamWidth 					width,
		SndStreamVariableBufferCallback	sndCallback,
		void 							*userDataP,
		UInt32 							bufferSize,
		Boolean 						armNative)
			SYS_TRAP(sysTrapSndStreamCreateExtended);

/// An extended function to handle new or device specific custom sound controls.
///
/// @param stream:	IN:	Opaque reference to the stream returned by SndStreamCreateExtended.
/// @param cmd:		IN:	One of the defined custom control enums, such as sndControlCodecCustomControl.
/// @param paramP:	IN: Pointer to data specific to the specific custom control defined by cmd.
/// @param size:	IN:	Size in bytes of the structure passed in paramP.
/// @retval Err Error code.
extern Err SndStreamDeviceControl(SndStreamRef stream, Int32 cmd, void *paramP, Int32 size)
							SYS_TRAP(sysTrapSndStreamDeviceControl);

#ifdef __cplusplus
}
#endif

#endif /* sysTrapSndStreamCreateExtended - 20 aug 2003 */

#endif /* __SOUNDMGREXT_H__ */
