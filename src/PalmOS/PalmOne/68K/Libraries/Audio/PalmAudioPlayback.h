/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Sound
 *
 */

/**
 *
 * @file	PalmAudioPlayback.h
 *
 * @brief	Public 68K include file that supports the stopping of audio
 *			playback on Treo 600 and Treo 650 smartphones.
 *
 * This header file and associated header files support the specific sound
 * functionality of the Treo smartphones. You should use the Palm OS Sound
 * Manager APIs for most of your work.
 *
 * Notes:
 *   This header contains a notification to call for stopping any existing
 *   background audio playback.
 *
 *   Any application ( usually video capture ) that needs to stop and eventually resume
 *   audio playback should use this header. Stopping or resuming playback is done by
 *   throwing a notification.
 *
 *   Audio playback applications should register for this notification and stop playback
 *   when they get the stop action code. They can optionally resume action when they get
 *   a notification with a resume action code. However resume is not
 *   a mandatory feature neither for the calling application nor for the audio application.
 *   In other words, the audio application may never get a resume after a stop action code.
 *
 *   Usage for any application requesting stop of audio playback:
 *
 *   PalmAudioPlaybackRequestStop('cccc'); // to stop background playback
 *   PalmAudioPlaybackRequestResume('cccc'); // to resume background playback
 *
 *   The calling application should always load this library with
 *   SysLibLoad() before use, even if it is already open by another
 *   application(ie, SysLibFind() returns a valid refnum). When
 *   the application is done with the library, it should be
 *   unloaded with SysLibRemove(). We do this because there is
 *   no good way to synchronize loading and unloading of libraries
 *   among multiple applications. It also greatly simplifies internal
 *   synchronization.
 */


#ifndef __PalmAudioPlayback_H__
#define __PalmAudioPlayback_H__

/***********************************************************************
 * Palm OS common definitions
 ***********************************************************************/
#include <PalmTypes.h>
#include <SystemMgr.h>
#include <NotifyMgr.h>
#include <MemoryMgr.h>

/**
 * @name PalmAudioPlayback information and notification
 *
 */

/*@{*/
#define	kPalmAudioPlaybackNotify1 sysMakeROMVersion(1, 0, 0, sysROMStageRelease, 0)

/** Version of the PalmAudioPlayback library. */
#define	kPalmAudioPlaybackNotifyVersion kPalmAudioPlaybackNotify1

/** PalmAudioPlayback creator ID. */
#define kPalmAudioPlaybackCreator 'apbP'

/** Playback notification event ID.
 *
 *  Applications playing audio should register for this notification
 *  and stop or optionally resume playback depending on the action code the application receives.
 */
#define kPalmAudioPlaybackNotifyEvent kPalmAudioPlaybackCreator
/*@}*/

/** Holds action codes for the kPalmAudioPlaybackNotifyEvent broadcast. */
typedef UInt16 PalmAudioPlaybackActionCodeType;

/**
 * @name Audio playback action codes
 *
 */

/*@{*/
/** Playback stop action code. */
#define kPalmAudioPlaybackActionCodeRequestStop  	((PalmAudioPlaybackActionCodeType)1)

/** Playback resume action code. */
#define kPalmAudioPlaybackActionCodeRequestResume    ((PalmAudioPlaybackActionCodeType)2)
/*@}*/

/** Audio playback notification structure.
 *
 *  Structure that is sent as part of notifyDetailsP when a kPalmAudioPlaybackNotifyEvent
 *  is thrown.
 */
typedef struct PalmAudioPlaybackNotifyEventTypeTag
{
	UInt32 version;							   	/**< Version. */
	PalmAudioPlaybackActionCodeType actionCode;	/**< Action code. */
	void* dataP;								/**< Data for action code, currently NULL. */
} PalmAudioPlaybackNotifyEventType;

/**
 * @name Audio playback helper macros
 *
 */
/*@{*/
#define PalmAudioPlaybackRequestStop(creator) \
	{ \
		PalmAudioPlaybackNotifyEventType request = \
		{ \
			kPalmAudioPlaybackNotifyVersion, \
		 	kPalmAudioPlaybackActionCodeRequestStop, \
		 	0 \
		}; \
		PalmAudioPlaybackSendRequest(&request, creator); \
	}
	/**<Request to stop playback. Any application that needs to request a stop of
	    audio playback should use this macro. "creator" is the creator id of the calling
	    function. */

#define PalmAudioPlaybackRequestResume(creator) \
	{ \
		PalmAudioPlaybackNotifyEventType request = \
		{ \
			kPalmAudioPlaybackNotifyVersion, \
		 	kPalmAudioPlaybackActionCodeRequestResume, \
		 	0 \
		}; \
		PalmAudioPlaybackSendRequest(&request, creator); \
	}
	/**<Request to resume playback. Any application that needs to request that
	    audio playback be resumed should use this macro. "creator" is the creator id of the calling
	    function. */

#define PalmAudioPlaybackSendRequest(requestP, creator) \
	{ \
		SysNotifyParamType		notify; \
		MemSet(&notify, sizeof(SysNotifyParamType), 0); \
		notify.notifyType 		= kPalmAudioPlaybackNotifyEvent; \
		notify.broadcaster 		= creator; \
		notify.notifyDetailsP 	= (void*)requestP; \
		SysNotifyBroadcast(&notify); \
	}
	/**<Actual broadcast of kPalmAudioPlaybackNotifyEvent notification. Used by Stop and
	    Resume macros. */
/*@}*/

#endif // __PalmAudioPlayback_H__
