/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: NotifyMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Notification Manager
 *
 *****************************************************************************/

#ifndef	__NOTIFY_H__
#define	__NOTIFY_H__

#include <PalmTypes.h>
#include <CoreTraps.h>
#include <LocaleMgr.h>
#include <DataMgr.h>
#include <SysEvent.h>
#include <Day.h>

#include <PalmOptErrorCheckLevel.h>

typedef struct SysNotifyParamType
	{
	UInt32		notifyType;		// What type of event occurred?
	UInt32		broadcaster;	// normally creator code of broadcasting app
	void *		notifyDetailsP;	// ptr to notification-specific data, if any
	void *		userDataP;		// user specified ptr passed back with notification
	Boolean		handled;		// true if event is handled yet
	UInt8		reserved2;
	} SysNotifyParamType;


typedef Err (*SysNotifyProcPtr)(SysNotifyParamType *notifyParamsP);

#define sysNotifyNormalPriority		0	// clients should use this priority

#define sysNotifyBroadcasterCode	sysFileCSystem // broadcaster code for events broadcast by the system

#define sysNotifyNoDatabaseID		0xFFFFFFFF		// for system use only!!!

#define sysNotifyVersionNum			1	// version of the NotifyMgr, obtained from the feature

// The queue is much smaller in debug builds so that we aren't tempted to overflow it 
// (and larger for release builds so that when we do, it will work).
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
#define sysNotifyDefaultQueueSize	10
#else
#define sysNotifyDefaultQueueSize	30
#endif

/*
	Notify Manager Events:
	
	Note that in general, for notifications regarding the creation of information
	(e.g. sysNotifyDBAddedEvent) the broadcast goes out AFTER the fact (e.g.,
	after the database has been created) and that notifications regarding
	the deletion of information are broadcast BEFORE the fact.  A notable and
	unfortunate exception to this rule is the database deleted notification, which
	is sent out after the fact.

	The sleep & wake notifications require a little special attention.  These
	notifications are _not_ guaranteed to always be broadcast, and hence are unsuitable
	for applications where external hardware must be shut off to conserve power when
	we go to sleep.  For example, the sleep notification will not be broadcast when 
	we do an emergency  shutdown due to low battery conditions.  Also note that any 
	sort of prolonged activity (especially displaying UI sich as a dialog) in response 
	to these noticfications can complicate things immensely.  UI is very tricky because 
	these notifications are broadcast from SysHandleEvent.  The result is that you may 
	receive more than one sleep notification if one of the notification clients puts up 
	a dialog or something, and it remains there long enough for another AutoOff event 
	to come through.  There are also possible problems with reentrancy and stack space 
	usage since the notification is broadcast from SysHandleEvent inside an application's 
	event loop.
*/

#define sysNotifySyncStartEvent		'hots'	// Sent at the start of a hotsync.
											// Always sent from UI thread.
											// param: none
											
#define sysNotifySyncFinishEvent	'sync'	// Sent at the end of a hotsync.
											// Always sent from UI thread.
											// param: none

#define sysNotifyAntennaRaisedEvent	'tena'	// Sent when the antenna is raised on a
											// Palm VII series device.
											// Always sent from UI thread.
											// param: none

#define sysNotifyResetFinishedEvent	'rstf'	// Broadcast after all app launchcodes
											// are sent after a reset.
											// Always sent from UI thread.
											// param: none

#define sysNotifyForgotPasswordEvent 'bozo'	// Broadcast when the user presses the 
											// 'Forgotten Password' button in the
											// security app, just before every database's
											// private records are deleted.

#define sysNotifySecuritySettingEvent 'ssch' // Broadcast when the security setting
                      // is changed either by the user in the Security application or 
                      // programmatically using SecSvcsSetDeviceSetting.
											
#define sysNotifyTimeChangeEvent	'time'	// Broadcast when the time is changed by the user.
											// param: Int32*, ptr to time change delta in 
											// seconds (e.g., a value of 100 means that the 
											// user set the clock ahead 100 seconds).

#define sysNotifySleepRequestEvent	'slpq'	// A "Sleep Request Event" is broadcast when the device
											// is about to go to sleep, and is a chance for clients
											// to do perform an action or even delay going to sleep
											// for a little while.  This may be broadcast multiple 
											// times if one or more clients defer the sleep. Parameter 
											// is a pointer to a SleepEventParamType.  See below 
											// details on this structure. Note that this event is 
											// NOT guaranteed to be broadcast.  See the big comment 
											// at the top of this list for important detials & warnings.
											// Always sent from UI thread.

#define sysNotifySleepNotifyEvent	'slp!'	// A "Sleep Notify Event" is broadcast when the device is 
											// definitely going to sleep as soon as the broadcast 
											// is finished.  Parameter is unused.
											// Note that this event is NOT guaranteed to be broadcast.  
											// See the big comment at the top of this list for 
											// important detials & warnings.
											// Always sent from UI thread.


#define sysNotifyEarlyWakeupEvent	'worm'	// ...the early bird gets the worm...
											// Broadcast just after the device wakes up
											// at the early stage where the screen may 
											// still be turned off and we may quickly go 
											// back to sleep after handling an alarm or 
											// charger event.  
											// Always sent from UI thread.


#define sysNotifyLateWakeupEvent	'lazy'	// Broadcast after the device wakes up
											// at the later stage of the wakeup 
											// process after we turn the screen on, 
											// broadcast from EvtResetAutoOffEvent 
											// the first time it is called implying
											// that the device will remain awake 
											// for at least a little while.
											// Always sent from UI thread.


#define sysNotifyDisplayChangeEvent	'scrd'	// Sent when the display depth is
											// changed, notifyDetailsP has old/new depth
											// see SysNotifyDisplayChangeDetailsType
											

#define sysNotifyMenuCmdBarOpenEvent	'cbar'	// Sent by FormHandleEvent when a menuCmdBarOpenEvent
												// passes through. The system will not open the toolbar 
												// if the 'handled' field is set in the parameter block, 
												// so most clients should not set it.  The notification
												// is provided so that Hack-like entities can catch
												// it and add their own buttons to the bar. These
												// buttons will show up to the left of cut/copy/paste/undo and
												// to the right of everything else.

#define cncNotifyProfileEvent	 'cncp'	// sent by the Connection Panel to inform its clients
													// that the current configuration has change
													//	void * notifyDetailsP : has to be casted as UInt32 
													// to get the Broadcasted Profile ID user specified ptr
													// passed back with notification
													//	void * userDataP : has to be casted as UInt16 
													// to get the device kind of the Broadcasted Profile ID 

#define	sysExternalConnectorAttachEvent	'ecna'		// Broadcast when anything is attached to 
													// the external connector.

#define	sysExternalConnectorDetachEvent	'ecnd'		// Broadcast when anything is detached from 
													// the external connector.

#define sysNotifyCardInsertedEvent	'crdi'	// Broadcast when an ExpansionMgr card is 
											// inserted into a slot, and the slot driver 
											// calls ExpCardInserted.  Always broadcast
											// from UI task.
											// ExpansionMgr will play a sound & attempt to
											// mount a volume unless the corresponding 
											// bits in the 'handled' field are set by a 
											// notification handler (see ExpansionMgr.h).
											// PARAMETER: slot number cast as void*
															
#define sysNotifyCardRemovedEvent	'crdo'	// Broadcast when an ExpansionMgr card is 
											// removed from a slot, and the slot driver 
											// calls ExpCardRemoved.  Always broadcast
											// from UI task.
											// ExpansionMgr will play a sound & attempt to
											// unmount a volume unless the corresponding 
											// bits in the 'handled' field are set by a 
											// notification handler (see ExpansionMgr.h).
											// PARAMETER: slot number cast as void*

#define sysNotifyVolumeMountedEvent	'volm'	// Broadcast when a VFSMgr volume is 
											// mounted, Always broadcast from UI task.
											// VFSMgr will run start.prc (if present),
											// and SysUIAppSwitch  to it or the Launcher
											// unless the appropriate bits in the 'handled'
											// field are set by a notification handler.
											// PARAMETER: VFSAnyMountParamPtr cast as void*

#define sysNotifyVolumeUnmountedEvent	'volu'	// Broadcast AFTER a VFSMgr volume is 
												// unmounted, Always broadcast from UI task.
												// VFSMgr will delete start.prc (if it was loaded).
												// The volume ref number for the unmounted volume is
												// NO LONGER VALID, so don't bother trying to use it.
												// PARAMETER: volume refNum cast as void*

#define sysNotifyHelperEvent		'hlpr'	// Sent by Address Book (or any
											// 3rd party application) to
											// communicate with Phone Apps


#define sysNotifyPOSEMountEvent     'pose'  // Sent by HostFS to communicate with itself,
											// saving on stack depth.


#define sysNotifyLocaleChangedEvent	'locc'	// Notify apps/panel that the system locale  
												// has changed.
												// This is broadcast by the language picker after it
												// has changed the locale.
												// Always sent from UI thread.
												// param: ptr to SysNotifyLocaleChangedType




#define sysNotifyRetryEnqueueKey	'retk'		// An EvtEnqueueKey failed, so retry.


#define sysNotifyGotUsersAttention	'attn'	// Got users attention.

											
#define sysNotifyDBDeletedEvent	'dbs-'	// broadcast AFTER a database is removed from the device.
														// Note that the database ID in the parameter block is
														// NO LONGER VALID, and you WILL CRASH the device if you
														// try to pass it to any DataMgr routines.
														// notifyDetailsP: ptr to SysNotifyDBDeletedType.

// Notification CreatorID for notifying Launcher and third-party Launchers of changes
#define sysNotifyDBCreatedEvent 'dbcr'	// broadcast (deferred) after a database is created on the device.

#define sysNotifyDBChangedEvent 'dbmn'  // broadcast (deferred) after database info is set on a database, such
												// as by DmSetDatabaseInfo

#define sysNotifyDBDirtyEvent   'dbdr'  // broadcast (deferred) when an overlay is opened, a database is opened for
												// write, or other events which can "dirty" the database info --
												// the Launcher normally checks over such databases on reset and
												// updates the LaunchDB.

#define sysNotifyDeviceUnlocked	'unlk'	// Broadcasted by the Security app After the device is 
										//unlocked.
										//notifyDetailsP: none 

#define sysNotifyPhoneEvent			'fone'		// Sent by third-party applications to communicate
												// with built-in or external phones.  
												// NOTE THAT THIS EVENT IS FOR FUTURE USE
																				
#define sysNotifyNetLibIFMediaEvent	'neti'		// Broadcasted by NetLib to communicate its interfaces media Up and Down requests
												// to registered Apps.
												// notifyDetailsP: ptr to SysNotifyNetLibIFMediaType.

#define sysNotifyDeleteProtectedEvent			'-pdb'		// Broadcast BEFORE a database is removed from the device when a user
																			// or installer application wants to remove a protected database.  This
																			// will ONLY be issued from the UI task and should be issued before each
																			// user driven deletion that targets a protected database that the app
																			// running does not own (i.e. needs to request be unprotected).
																			// This gives the unlucky client a chance to do any required 
																			// cleanup work.  Note that if an application has multiple protected 
																			// databases, this notification may be sent out more than once.
																			// notifyDetailsP: ptr to SysNotifyDBInfoType.

#define sysNotifyIrDASniffEvent			'irda'		// Sent by IrDA sniffing code when an incomming IrDA frame is detected										

#define sysNotifyProcessPenStrokeEvent      'hpps'      // Sent to enable custom recognition of strokes on the system area of the digitizer (buttons, etc.)
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyVirtualCharHandlingEvent   'hvch'      // Sent to enable custom handling handling of vchars
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyEventDequeuedEvent         'hede'      // Sent for each event being returned by EvtGetEvent. Be very careful
                                                        // about registering for this; you can easily impact performance (in a bad way!)
                                                        // Also note: for speed, notifyDetailsP points to a event structure with 
                                                        // system-native endianness, meaning you might need to byte-swap what you
                                                        // read depending on what endianness your code uses.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyIdleTimeEvent              'hidl'      // Sent when the system is idle and is about to doze.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyAppLaunchingEvent          'hapl'		// Sent before an app is launched (normalLaunch only)
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyAppQuittingEvent           'hapq'      // Sent right after an app has quit (normalLaunch only)
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyInsPtEnableEvent           'hipe'      // Sent at the start of InsPtEnable(). notifyDetailsP points to a Boolean: the enableIt param.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyKeyboardDialogEvent        'hkbd'      // Sent to enable replacement of SysKeyboardDialog()'s UI.
                                                        // The notifyDetailsP points to the KeyboardType.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyDisplayResizedEvent		'scrs'		// Broadcast by PINSetInputAreaState after the input area 
														// opens or closes.  NotifyDetailsP is a pointer to a
														// SysNotifyDisplayResizedDetailsType structure.

#define sysNotifyInputAreaDrawingEvent		'iaad'		// Broadcast by HALRedrawInputArea right before the input area bitmap
														// is drawn. If the notification is handled, the bitmap is not drawn.
														// NotifyDetailsP is a pointer to a SysNotifyInputAreaDrawingDetailsType structure.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyInputAreaPendownEvent		'iapd'		// Broadcast by SysHandleEvent when a pen-down has occurred in the input area.
														// If the notification is handled, further pen-down processing is skipped.
														// NotifyDetailsP is a pointer to a SysNotifyInputAreaPendownDetailType structure.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifySelectDay					'seld'		// Broadcast by SelectDay before displaying any UI.
														// If the notification is handled, further UI is skipped.
														// NotifyDetailsP is a pointer to a SysNotifySelectDayDetailsType structure.
                                                        // Note: future OS releases might not send this notification.

#define sysNotifyGsiDrawIndicator			'gsid'		// Broadcast by GsiDrawIndicator before drawing the GSI.
														// If the notification is handled, further drawing is skipped.
                                                        // Note: future OS releases might not send this notification.

// for sysNotifyAppLaunchingEvent and sysNotifyAppQuittingEvent
typedef struct SysNotifyAppLaunchOrQuitTag {
	UInt32    version;   // this is version 0
	UInt32    dbID;      // the application being started or quitting (note: type unsafeness is intentional)
	UInt16    cardNo;    // the application being started or quitting
} SysNotifyAppLaunchOrQuitType;


// for sysNotifyVirtualCharHandlingEvent
typedef struct SysNotifyVirtualCharHandlingTag {
	UInt32                     version;   // this is version 0
	struct _KeyDownEventType   keyDown;
} SysNotifyVirtualCharHandlingType;


// for sysNotifyProcessPenStrokeEvent
typedef struct SysNotifyPenStrokeTag {
	UInt32     version;   // this is version 0
	PointType  startPt;
	PointType  endPt;
} SysNotifyPenStrokeType;


// for sysNotifyDisplayChangeEvent
typedef struct SysNotifyDisplayChangeDetailsTag {
	UInt32 oldDepth;
	UInt32 newDepth;
} SysNotifyDisplayChangeDetailsType;


// For sysNotifyLocaleChangedEvent
typedef struct SysNotifyLocaleChangedTag {
	LmLocaleType 	oldLocale;
	LmLocaleType 	newLocale;
} SysNotifyLocaleChangedType;


// Possible values for the sleep-reason for the sysNotifySleepEvent
#define sysSleepPowerButton		0
#define sysSleepAutoOff			1
#define sysSleepResumed			2
#define sysSleepUnknown			3


// for sysNotifySleepRequestEvent
typedef struct SleepEventParamTag {
	UInt16 reason;		// contains the reason we are going to sleep. See above list for possible values.
	UInt16 deferSleep;	// Only used for the sysNotifySleepRequestEvent, this should be incremented by 
						// clients that want to defer the sleep.  They are then responsible for enqueing
						// a resumeSleepChr in the event queue.
} SleepEventParamType;


// for sysNotifyDeleteProtectedEvent, and possibly others...
typedef struct SysNotifyDBInfoTag {
	LocalID	dbID;				// database ID of dabatase
	UInt16	cardNo;			// card number that dabatase resides on
	UInt16	attributes;		// database attributes
	Char	dbName[dmDBNameLength];	// name of database
	UInt32	creator;			// creator code of dabatase
	UInt32	type;				// type of dabatase
} SysNotifyDBInfoType;


// for sysNotifyDBDeletedEvent
typedef struct SysNotifyDBDeletedTag {
	LocalID	oldDBID;			// old database ID of dabatase, no longer valid
	UInt16	cardNo;			// card number that dabatase resides on
	UInt16	attributes;		// database attributes
	Char	dbName[dmDBNameLength];
	UInt32	creator;			// creator code of dabatase
	UInt32	type;				// type code of dabatase
} SysNotifyDBDeletedType;


// for sysNotifyDBCreatedEvent
typedef struct SysNotifyDBCreatedTag {
	Char	dbName[dmDBNameLength];	// Database name
	UInt32	creator;				// creator code of dabatase
	UInt32	type;					// type code of dabatase
	LocalID newDBID;				// LocalID of newly-created database
	UInt16	cardNo;					// card number that dabatase resides on
	Boolean resDB;					// Is it a resource DB?
	UInt8	padding;				// Explicitly pad struct to multiple of 4
} SysNotifyDBCreatedType;

// for sysNotifyDBChangedEvent
typedef struct SysNotifyDBChangedTag {
	Char     dbName[dmDBNameLength];	// new database name of DB
	LocalID	 dbID;						// database ID of DB
	UInt32   creator;					// creator code of DB
	UInt32   type;						// new type code of DB
	UInt32   crDate;					// new creation date of DB
	UInt32   modDate;					// new modification date of DB
	UInt32   bckUpDate;					// new backup date of DB
	UInt32   modNum;					// new modification number of DB
	LocalID  appInfoID;					// new appinfo block of DB
	LocalID  sortInfoID;				// new sortinfo block of DB
	UInt16	 attributes;				// new DB attributes
	UInt16   cardNo;					// card number that dabatase resides on
	UInt16   version;					// new DB version
	UInt16   fields;					// what fields above are set?

	/* Old field values, prior to the DmSetDatabaseInfo */
	Char     oldName[dmDBNameLength];
	UInt32   oldCreator;
	UInt32   oldType;
	UInt16   oldAttributes;
	UInt16   padding;					// Explicitly pad struct to multiple of 4
} SysNotifyDBChangedType;

#define DBChangedFieldSetName             0x1
#define DBChangedFieldSetCreator          0x2
#define DBChangedFieldSetType             0x4
#define DBChangedFieldSetCrDate           0x8
#define DBChangedFieldSetModDate         0x10
#define DBChangedFieldSetBckUpDate       0x20
#define DBChangedFieldSetModNum          0x40
#define DBChangedFieldSetAppInfo         0x80
#define DBChangedFieldSetSortInfo       0x100
#define DBChangedFieldSetAttributes     0x200
#define DBChangedFieldSetVersion        0x400

// for sysNotifyDBDirtyEvent
typedef struct SysNotifyDBDirtyTag {
	Char	dbName[dmDBNameLength];	// Database name
	UInt32	creator;				// creator code of dabatase
	UInt32	type;					// type code of dabatase
} SysNotifyDBDirtyType;

// for sysNotifyDisplayResizedEvent
typedef struct SysNotifyDisplayResizedDetailsTag {
	RectangleType	newBounds;
} SysNotifyDisplayResizedDetailsType;

// for sysNotifyInputAreaDrawingEvent. All structure data is in
// ARM native format!!!
typedef struct SysNotifyInputAreaDrawingDetailsTag {
	RectangleType	updateBounds;		// Area to redraw, in native coords, relative to input area window.
	const BitmapType* bitmapP;			// Ptr to bitmap that would be drawn by the OS.
	Boolean			selected;			// True if area is being drawn in response to user tap (button selected)
	Boolean			fullRedraw;			// True if <updateBounds> should be ignored.
	UInt16			padding;
} SysNotifyInputAreaDrawingDetailsType;

// for sysNotifyInputAreaPendownEvent. All structure data is
// in ARM native format!!!
typedef struct SysNotifyInputAreaPendownDetailTag {
	PointType		where;				// Pendown location, in native coords, relative to input area window.
} SysNotifyInputAreaPendownDetailType;

// for sysNotifySelectDay. All structure data is in ARM native format!!!
typedef struct SysNotifySelectDayDetailsTag {
	DaySelectorType*	daySelectorP;	// Describes selected date
	const
	Char*				titleP;			// Suggested form title
	Boolean				dayChanged;		// Return true if date changed
	UInt8				padding[3];
} SysNotifySelectDayDetailsType;

/*
Possible event types to be added in the future:
NOTE that these are NOT implemented yet!!!!

#define sysNotifyDBAddedEvent		'dbs+'	// Broadcast after a database is added to the device
											// param: ptr to SysNotifyDBInfoType.
											
#define sysNotifyInitializeEvent	'helo'	// broadcast to an app after its installed
											// so it can do any necessary initalization
											// This event is always broadcast.
											// param: ptr to SysNotifyDBInfoType.

#define sysNotifyCleanupEvent		'gbye'	// broadcast to an app just before its deleted
											// so it can do any necessary cleanup work.
											// This event is always broadcast.
											// The database's type will have been set to 
											// sysFileTTemp so that if case the handler 
											// crashes, the DB will be deleted on reset.
											// param: ptr to SysNotifyDBInfoType.

#define sysNotifyCardChangedEvent	'card'	// broadcast when the owner's business card
											// has changed, allowing apps to adjust for that.
											// param: ptr to new owner data.

*/




#ifdef __cplusplus
extern "C" {
#endif

extern Err SysNotifyRegister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, 
						SysNotifyProcPtr callbackP, Int8 priority, void *userDataP)
				SYS_TRAP(sysTrapSysNotifyRegister);		

extern Err SysNotifyUnregister(UInt16 cardNo, LocalID dbID, UInt32 notifyType, Int8 priority)
				SYS_TRAP(sysTrapSysNotifyUnregister);
		
/*
 *	WARNING: Never call SysNotifyBroadcast from a background task 
 *			(or from any code that might be called from a BG task)
 *			with the memory semaphore reserved.  Deadlock will 
 *			result when the broadcast is deferred and the UI task 
 *			tries to acquire the mem semaphore in order to send it out.
 */
extern Err SysNotifyBroadcast(SysNotifyParamType *notify)
				SYS_TRAP(sysTrapSysNotifyBroadcast);

extern Err SysNotifyBroadcastDeferred(SysNotifyParamType *notify, Int16 paramSize)
				SYS_TRAP(sysTrapSysNotifyBroadcastDeferred);


// Kick off a notification from an interrupt handler.
// Note: non-interrupt code should use SysNotifyBroadcast
extern Err SysNotifyBroadcastFromInterrupt(UInt32 notifyType, UInt32 broadcaster,
						void *notifyDetailsP)
				SYS_TRAP(sysTrapSysNotifyBroadcastFromInterrupt);


#ifdef __cplusplus 
}
#endif



#endif	// __NOTIFY_H__
