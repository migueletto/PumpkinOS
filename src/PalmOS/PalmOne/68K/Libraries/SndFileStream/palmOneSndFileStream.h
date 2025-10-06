/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 * @file	palmOneSndFileStream.h
 *
 * @brief	Public 68K include file for sound files stream support for Treo 600
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


#ifndef __SNDFILESTREAM_H__
#define __SNDFILESTREAM_H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/

#include <PalmTypes.h>
#include <SystemMgr.h>
#include <SystemResources.h>

#if defined( BUILDING_SNDFILESTREAM ) || defined( BUILDING_SNDFILESTREAM_INTO_APP )
#define	SNDFILESTREAM_TRAP( x ) /**<Helper macro */
#else
#define SNDFILESTREAM_TRAP( x )	SYS_TRAP( x ) /**<Helper macro. */
#endif

#include <FileStream.h>
#include <SoundMgr.h>
#include <VFSMgr.h>

/**
 * @name Type and creator of the Sound File Stream library
 *
 * Used with FtrGet.
 */
/*@{*/
#define kSndFileStreamLibType		sysFileTLibrary 									/**<Type of the sound file stream library. */
#define kSndFileStreamLibCreator	'sfsL' 												/**<Creator ID of the sound file stream library. */
#define kSndFileStreamLibName		"SndFileStream-sfsL" 								/**<Name of the sound file stream library. */
#define kSndFileStreamLibVersion	sysMakeROMVersion( 1, 0, 0, sysROMStageRelease, 0 ) /**<Version of the sound file stream library. */
/*@}*/

/**
 * @name Utility macros
 *
 * These macros take care of opening and closing the sound file stream library
 * in various situations.
 */
/*@{*/
#if defined( BUILDING_SNDFILESTREAM_INTO_APP )
#define	SndFileStreamOpenLibrary( refNum, err )	    \
	{                                               \
		(refNum)	= 0;							\
		(err)		= errNone;						\
	}
#define SndFileStreamCloseLibrary( refNum, err )	\
	{												\
		(refNum)	= sysInvalidRefNum;				\
		(err)		= errNone;						\
	}
#else
#define SndFileStreamOpenLibrary( refNum, err )	    			\
	{															\
		Boolean	loaded;											\
																\
		loaded = false;											\
		(err) = SysLibFind( kSndFileStreamLibName, &(refNum) );	\
		if ( sysErrLibNotFound == (err) )						\
		{														\
			(err) = SysLibLoad( kSndFileStreamLibType,			\
				kSndFileStreamLibCreator, &(refNum) );			\
			loaded = true;										\
		}														\
																\
		if ( errNone == (err) )									\
		{														\
			(err) = SndFileStreamLibOpen( (refNum) );			\
			if ( errNone != (err) )								\
			{													\
				if ( loaded )									\
				{												\
					SysLibRemove( (refNum) );					\
				}												\
																\
				(refNum) = sysInvalidRefNum;					\
			}													\
		}														\
		else													\
		{														\
			(refNum) = sysInvalidRefNum;						\
		}														\
	}
#define SndFileStreamCloseLibrary( refNum, err )				\
	{															\
		if ( sysInvalidRefNum == (refNum) )						\
		{														\
			(err) = sysErrParamErr;								\
		}														\
		else													\
		{														\
			(err) = SndFileStreamLibClose( (refNum) );			\
			if ( errNone == (err) )								\
			{													\
				(err) = SysLibRemove( (refNum) );				\
				(refNum) = sysInvalidRefNum;					\
			}													\
			else if ( kSndFileStreamLibErrStillOpen == (err) )	\
			{													\
				(err) = errNone;								\
			}													\
		}														\
	}
#endif
/*@}*/

/**
 * @name Error codes
 *
 * These macros take care of opening and closing the sound file stream library
 * in various situations.
 */
/*@{*/
#define kSndFileStreamLibErrNone					errNone
#define kSndFileStreamLibErrAlreadyOpen				1
#define kSndFileStreamLibErrNotOpen					2
#define kSndFileStreamLibErrStillOpen				3
#define kSndFileStreamLibErrOutOfMemory				4
#define kSndFileStreamLibErrCantOpenFile			5
#define kSndFileStreamLibErrUnsupportedFile			6
#define kSndFileStreamLibErrCancel					7
#define kSndFileStreamLibErrOperationNotSupported	8
/*@}*/

/**
 * @name Play back sound file stream
 *
 * Use for playbackCount parameter of tagSndFileStreamData to play back a
 * stream indefinitely.
 */
/*@{*/
#define kSndFileStreamLibPlaybackForever	0xFFFF
/*@}*/

/**
 * Notification broadcast by SndFileStream when it has completed playing a stream.
 * This is an alternative to using callbacks in the case where SndFileStream is used
 * by an ARM app (eg. Blazer) or by a 68K app.
 */
/*@{*/
#define sndFileStreamNotifyStreamComplete 	'StrC'
/*@}*/

/**
 * Sound file format
 *
 * The sound file format of the stream.
 */
typedef enum tagSndFileFormatEnum
{
	kSndFileFormatRaw = 0,	/**<Raw bytes. */
	kSndFileFormatWAV,		/**<RIFF-WAVE. */
	kSndFileFormatTreoQCP,	/**<RIFF-QLCM. */
	kSndFileFormatTreoAMR,	/**<Raw RTP packets. */
	kSndFileFormatMP3		/**<MP3 format */
}
SndFileFormatType;

/** Reference to sound file stream returned by SndFileStreamCreate()/Open() */
typedef void *SndFileStreamRef;

/**
 * @brief Holds data about a sound file stream.
 */
typedef struct tagSndFileStreamData
{
	UInt16		streamFlags;		/**<Flag for the sound file stream. */
	UInt16		streamMode;			/**<Use mode sndInput or sndOutput for sound file stream.*/
	UInt32		streamRefCon;		/**<Reserved for third party use. */

	UInt16		playbackCount;		/**<How many times to play the sound. Values of 0 and 1 both cause
										the sound to be played back one time.  Is decremented each time
										play back of the sound is restarted. */

	UInt16		streamComplete;		/**<Set to true when the stream has stopped because
								        the playbackCount has been decremented to zero, maximumBytes is reached, or
							            maximumMilliseconds is reached */

	UInt32		dataSize;			/**<Size of the audio data in bytes. */
	UInt32		dataDuration;		/**<Duration of the audio data in milliseconds. */
	UInt32		customDataSize;		/**<Size of custom data in bytes. */

	UInt32		fileFormat;			/**<Format of the sound file stream.  For example, kSndFileFormatWAV. */
	UInt32		sampleFormat;		/**<Format of the soud sample. Example: palmCodecAudioIMA_ADPCM */
	UInt32		sampleRate;			/**<Sample rate of hte sound file in frames/sec.  For example, 8000. */
	UInt32		sampleType;			/**<Type of the sound file.  For example, sndInt16. */
	UInt32		sampleWidth;		/**<Whether the sound is stereo, mono, and so forth.  For example, sndMono. */
	UInt32		sampleBlockAlign;	/**<Block alignment of the sound file.  For example, 256 for IMA ADPCM. */

	UInt32		maximumMilliseconds;/**<Maximum milliseconds of play back. */
	UInt32		maximumBytes;		/**<Maximum value for lastPlayedBytes. */

	UInt32		currentMillisecond;	/**<Current millisecond of audio. */
	UInt32		lastByte;			/**<Last byte played or recorded on channel, */

	MemPtr		customDataP;		/**<Pointer to custom data. */
}
SndFileStreamData, *SndFileStreamDataPtr;

/// Callback function passed when calling SndFileStreamCreate() and
/// SndFileStreamOpen(). It will be called during playback and recording.
///
/// @param stream:				IN:  Reference to stream returned by SndFileStreamCreate()/SndFileStreamOpen().
/// @param streamDataP:			IN:  @see SndFileStreamData
/// @param customDataModified:	OUT: If true, custom data has been modified.
/// @retval Err Error code.
typedef Err (*SndFileStreamCallbackPtr)( SndFileStreamRef stream,
	SndFileStreamDataPtr streamDataP, Boolean *customDataModified );

/**
 * @brief Descriptions of the stream file.
 *
 * Used with SndFileStreamCreate()/SndFileStreamOpen()
 */
typedef struct tagSndFileStreamFileSpec
{
	UInt16	flags;      /**< Set to kSndFileStreamFileSpecFlagVFS or kSndFileStreamFileSpecFlagFileStream */

	UInt16	volRefNum;  /**< The volume reference number returned from VFSVolumeEnumerate(). */

	Char	*fileNameP; /**< Name of the stream file */

	UInt32	fileCreator;/**< Creator ID of the stream file  */
	UInt32	fileType;   /**< See File Stream Database Types */
}
SndFileStreamFileSpec, *SndFileStreamFileSpecPtr;

/**
 * @name File Flags
 *
 */
/*@{*/
#define kSndFileStreamFileSpecFlagVFS			(0x0001) /**<VFS file */
#define kSndFileStreamFileSpecFlagFileStream	(0x0002) /**<File stream*/
/*@}*/

/**
 * @name File stream database types
 *
 * File steam databases opened using kSndFileStreamFileSpecFlagFileStream.
 *
 */
/*@{*/
#define kSndFileTypeWAV	'WAVf'	/**<WAV file   -> kSndFileFormatWAV .    */
#define kSndFileTypeQCP	'QCPf'	/**<QCELP file -> kSndFileFormatTreoQCP. */
#define kSndFileTypeAMR	'AMRf'	/**<AMR file   -> kSndFileFormatTreoAMR. */
#define kSndFileTypeMP3 'MP3f'	/**<MP3 file   -> kSndFileFormatMP3      */
/*@}*/

/**
 * @name Library traps
 *
 */
/*@{*/
#define	kSndFileStreamLibTrapVersion				(sysLibTrapCustom + 0)
#define	kSndFileStreamLibTrapCreate					(sysLibTrapCustom + 1)
#define	kSndFileStreamLibTrapOpen					(sysLibTrapCustom + 2)
#define	kSndFileStreamLibTrapClose					(sysLibTrapCustom + 3)
#define	kSndFileStreamLibTrapStart					(sysLibTrapCustom + 4)
#define	kSndFileStreamLibTrapPause					(sysLibTrapCustom + 5)
#define	kSndFileStreamLibTrapStop					(sysLibTrapCustom + 6)
#define	kSndFileStreamLibTrapSeek					(sysLibTrapCustom + 7)
#define	kSndFileStreamLibTrapGetCurrentStreamData	(sysLibTrapCustom + 8)
#define	kSndFileStreamLibTrapDeviceControl			(sysLibTrapCustom + 9)
#define	kSndFileStreamLibTrapSetVolume				(sysLibTrapCustom + 10)
#define	kSndFileStreamLibTrapGetVolume				(sysLibTrapCustom + 11)
#define	kSndFileStreamLibTrapSetPan					(sysLibTrapCustom + 12)
#define	kSndFileStreamLibTrapGetPan					(sysLibTrapCustom + 13)
#define	kSndFileStreamLibTrapUIPlay					(sysLibTrapCustom + 14)
#define	kSndFileStreamLibTrapUIRecord				(sysLibTrapCustom + 15)
/*@}*/

#ifdef __cplusplus
extern "C" {
#endif

/// Opens the Sound File Stream library.
///
/// @param libRefNum: IN:  Library reference number
/// @retval Err Error code.
extern Err SndFileStreamLibOpen( UInt16 libRefNum )
	SNDFILESTREAM_TRAP( sysLibTrapOpen );

/// Closes the Sound File Stream library.
///
/// @param libRefNum: IN:  Library reference number
/// @retval Err Error code.
extern Err SndFileStreamLibClose( UInt16 libRefNum )
	SNDFILESTREAM_TRAP( sysLibTrapClose );

/// Standard library sleep routine.
///
/// @param libRefNum: IN:  Library reference number
/// @retval Err Error code.
extern Err SndFileStreamLibSleep( UInt16 libRefNum )
	SNDFILESTREAM_TRAP( sysLibTrapSleep );

/// Standard library wake routine.
///
/// @param libRefNum: IN:  Library reference number
/// @retval Err Error code.
extern Err SndFileStreamLibWake( UInt16 libRefNum )
	SNDFILESTREAM_TRAP( sysLibTrapWake );

/// Retrieve the library version number.
///
/// @param libRefNum:    IN:  Library reference number
/// @param *versionP:    OUT: Library version number.
/// @retval Err Error code.
extern Err SndFileStreamLibVersion( UInt16 libRefNum, UInt32 *versionP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapVersion );

/// Create a new sound file stream.
///
/// @param libRefNum:    IN:  Library reference number
/// @param streamP:      OUT: Reference to the created sound stream
/// @param streamDataP:  IN:  @see SndFileStreamData
/// @param fileSpecP:    IN:  Description of the file to be created
/// @param callbackP:    IN:  User defined callback function, to be called on playback/recording.
/// @retval Err Error code.
extern Err SndFileStreamCreate( UInt16 libRefNum, SndFileStreamRef *streamP,
		SndFileStreamDataPtr streamDataP, SndFileStreamFileSpecPtr fileSpecP,
		SndFileStreamCallbackPtr callbackP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapCreate );

/// Open an already existing sound file stream.
///
/// @param libRefNum:    IN:  Library reference number
/// @param streamP:      OUT: Reference to the opened sound stream.
/// @param streamDataP:  IN:  @see SndFileStreamData
/// @param fileSpecP:    IN:  Description of the file to be opened.
/// @param callbackP:    IN:  User defined callback function, to be called on playback/recording.
/// @retval Err Error code.
extern Err SndFileStreamOpen( UInt16 libRefNum, SndFileStreamRef *streamP,
		SndFileStreamDataPtr streamDataP, SndFileStreamFileSpecPtr fileSpecP,
		SndFileStreamCallbackPtr callbackP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapOpen );

/// Close the reference to the file stream.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening file stream.
/// @retval Err Error code.
extern Err SndFileStreamClose( UInt16 libRefNum, SndFileStreamRef stream )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapClose );

/// Start streaming sound into file.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening stream.
/// @retval Err Error code.
extern Err SndFileStreamStart( UInt16 libRefNum, SndFileStreamRef stream )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapStart );

/// Pause sound streaming.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening stream.
/// @param pause:        IN: If true, pause. Otherwise, resume.
/// @retval Err Error code.
extern Err SndFileStreamPause( UInt16 libRefNum, SndFileStreamRef stream,
		Boolean pause )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapPause );

/// Stop sound streaming.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening stream.
/// @retval Err Error code.
extern Err SndFileStreamStop( UInt16 libRefNum, SndFileStreamRef stream )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapStop );

/// Move the pointer to the stream to a user-defined offset.
///
/// @param libRefNum:       IN: Library reference number
/// @param stream:          IN: Reference to the sound stream, returned when creating/opening stream.
/// @param seekMilliseconds IN: Offset in milliseconds.
/// @retval Err Error code.
extern Err SndFileStreamSeek( UInt16 libRefNum, SndFileStreamRef stream,
		UInt32 seekMilliseconds )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapSeek );

/// Get streaming data.
///
/// @param libRefNum:    IN:  Library reference number
/// @param stream:       IN:  Reference to the sound stream, returned when creating/opening stream.
/// @param streamDataP:  OUT: @see SndFileStreamData
/// @retval Err Error code.
extern Err SndFileStreamGetCurrentStreamData( UInt16 libRefNum,
		SndFileStreamRef stream, SndFileStreamDataPtr streamDataP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapGetCurrentStreamData );

/// Control device when streaming.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening stream.
/// @param cmd:          IN: No definition.
/// @param param:        IN: No definition.
/// @param size:         IN: No definition.
/// @retval Err Error code.
extern Err SndFileStreamDeviceControl( UInt16 libRefNum,
		SndFileStreamRef stream, Int32 cmd, void *param, Int32 size )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapDeviceControl );

/// See SndStreamSetVolume() documented under Palm OS Sound Manager API.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream, returned when creating/opening stream.
/// @param volume:       IN: No definition.
/// @retval Err Error code.
extern Err SndFileStreamSetVolume( UInt16 libRefNum, SndFileStreamRef stream,
		Int32 volume )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapSetVolume );

/// See SndStreamGetVolume() documented under Palm OS Sound Manager API.
///
/// @param libRefNum:   IN:  Library reference number
/// @param stream:      IN:  Reference to the sound stream, returned when creating/opening stream.
/// @param volumeP:     OUT: Range 0 - 32k. See Palm OS SoundMgr API documentation for more details.
/// @retval Err Error code.
extern Err SndFileStreamGetVolume( UInt16 libRefNum, SndFileStreamRef stream,
		Int32 *volumeP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapGetVolume );

/// Set stereo balance.
/// See SndStreamSetPan() documented under Palm OS Sound Manager API.
///
/// @param libRefNum:    IN: Library reference number
/// @param stream:       IN: Reference to the sound stream.
/// @param panPosition:  IN: Range [-1024, 1024]
/// @retval Err Error code.
extern Err SndFileStreamSetPan( UInt16 libRefNum, SndFileStreamRef stream,
		Int32 panPosition )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapSetPan );

/// Get stereo balance.
/// See SndStreamGetPan() documented under Palm OS Sound Manager API.
///
/// @param libRefNum:        IN:  Library reference number
/// @param stream:           IN:  Reference to the sound stream.
/// @param panPositionP:     OUT: Range [-1024, 1024]
/// @retval Err Error code.
extern Err SndFileStreamGetPan( UInt16 libRefNum, SndFileStreamRef stream,
		Int32 *panPositionP )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapGetPan );

/// Play sound stream with UI
///
/// @param libRefNum:        IN: Library reference number
/// @param fileSpecP:        IN: Pointer to the sound file to be played.
/// @param allowRerecord:    IN: No definition.
/// @retval Err Error code.
extern Err SndFileStreamUIPlay( UInt16 libRefNum,
		SndFileStreamFileSpecPtr fileSpecP, Boolean allowRerecord )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapUIPlay );

/// Record sound stream with UI
///
/// @param libRefNum:        IN: Library reference number
/// @param streamDataP:      IN: Stream data to be used for recording.
/// @param fileSpecP:        IN: Pointer to the sound file to be recorded.
/// @param allowRerecord:    IN: No definition.
/// @retval Err Error code.
extern Err SndFileStreamUIRecord( UInt16 libRefNum,
		SndFileStreamDataPtr streamDataP, SndFileStreamFileSpecPtr fileSpecP,
		Boolean allowRerecord )
	SNDFILESTREAM_TRAP( kSndFileStreamLibTrapUIRecord );

#ifdef __cplusplus
}
#endif

#endif //__SNDFILESTREAM_H__
