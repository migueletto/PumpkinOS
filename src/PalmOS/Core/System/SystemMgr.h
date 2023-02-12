/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SystemMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Pilot system equates
 *
 *****************************************************************************/

#ifndef __SYSTEMMGR_H__
#define __SYSTEMMGR_H__

// Include elementary types
#include <PalmTypes.h>
#include <CoreTraps.h>				// Trap Numbers.

// Other types
#include <SystemResources.h>		// Resource definitions.


// System Headers 
#include <Rect.h>
#include <Font.h>
#include <Window.h>
#include <InsPoint.h>
#include <Event.h>
#include <DataMgr.h>					// for DmOpenRef
#include <LibTraps.h>


/************************************************************
 * System Constants
 *************************************************************/
// Define the number of ticks/second
// NOTE:  It is strongly recommended that developers avoid using these
// defines, and use the SysTicksPerSecond() API (below) instead....
#if EMULATION_LEVEL == EMULATION_MAC
	#define	sysTicksPerSecond				60					// 60/sec on Macintosh
#elif EMULATION_LEVEL == EMULATION_NONE
	#define	sysTicksPerSecond				100				// 100/sec on Pilot
#elif EMULATION_LEVEL == EMULATION_WINDOWS
	#define	sysTicksPerSecond				1000				// 1000/sec on Windows PC
#elif EMULATION_LEVEL == EMULATION_UNIX
	#define sysTicksPerSecond				1000
	// 1000/sec on Linux
#else
	#error Invalid EMULATION_LEVEL
#endif

	
	
/************************************************************
 * Rules for creating and using the Command Parameter Block
 * passed to SysUIAppSwitch 
 *************************************************************/

// A parameter block containing application-specific information may be passed
// to an application when launching it via SysUIAppSwitch.  To create the
// parameter block, you allocate a memory block using MemPtrNew and then you must
// call MemPtrSetOwner to set the block's owner ID to 0.  This assigns the block's
// ownership to the system so that it will not be automatically freed by the system
// when the calling app exits. The command block must be self contained. It must not
// have pointers to anything on the stack or in memory blocks owned by an application.
// The launching and launched applications do not need to worry about freeing the
// command block since the system will do this after the launched application exits.
// If no parameter block is being passed, this parameter must be NULL.


/************************************************************
 * Action Codes
 * 
 * IMPORTANT ACTION CODE CONSIDERATIONS:
 *
 * Many action codes are "sent" to apps via a direct function call into the app's
 * PilotMain() function without launching the app.  For these action codes, the
 * application's global and static variables are *not* available, unless the
 * application is already running. Some action codes are synchronized with the
 * currently running UI applcation via the event manager (alarm action codes,
 * for example), while others, such as HotSync action codes, are sent from a
 * background thread. To find out if your app is running (is the current UI
 * app) when an action code is received, test the sysAppLaunchFlagSubCall flag
 * (defined in SystemMgr.h) which is passed to your PilotMain in the
 * launchFlags parameter (the third PilotMain parameter). If it is non-zero,
 * you may assume that your app is currently running and the global variables
 * are accessible. This information is useful if your app maintains an open
 * data database (or another similar resource) when it is running. If the app
 * receives an action code and the sysAppLaunchFlagSubCall is set in
 * launchFlags, the handler may access global variables and use the open
 * database handle while handling the call. On the other hand, if the
 * sysAppLaunchFlagSubCall flag is not set (ie., zero), the handler will need
 * to open and close the database itself and is not allowed to access global
 * or static variables.
 *
 *************************************************************/
 
// NOTE: for defining custom action codes, see sysAppLaunchCmdCustomBase below.

// System SysAppLaunch Commands
#define	sysAppLaunchCmdNormalLaunch		0	// Normal Launch

#define	sysAppLaunchCmdFind					1	// Find string

#define	sysAppLaunchCmdGoTo					2	// Launch and go to a particular record

#define	sysAppLaunchCmdSyncNotify			3  // Sent to apps whose databases changed during
															// HotSync after the sync has been completed,
															// including when the app itself has been installed
															// by HotSync. The data database(s) must have the
															// same creator ID as the application for this
															// mechanism to function correctly. This is a
															// good opportunity to update/initialize/validate
															// the app's data, such as resorting records,
															// setting alarms, etc.
															//
															// Parameter block: None.
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a
															//		direct function call into the app's
															//		PilotMain function from the background
															//		thread of the HotSync application.


#define	sysAppLaunchCmdTimeChange			4  // Sent to all applications and preference
															// panels when the system time is changed.
															// This notification is the right place to
															// update alarms and other time-related
															// activities and resources.
															//
															// Parameter block: None.
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdSystemReset			5  // Sent to all applications and preference
															// panels when the system is either soft-reset
															// or hard-reset.  This notification is the
															// right place to initialize and/or validate
															// your application's preferences/features/
															// database(s) as well as to update alarms and
															// other time-related activities and resources.
															//
															// Parameter block: SysAppLaunchCmdSystemResetType
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdAlarmTriggered		6  // Sent to an application at the time its
															// alarm time expires (even when another app
															// is already displaying its alarm dialog box).
															// This call is intended to allow the app to
															// perform some very quick activity, such as
															// scheduling the next alarm or performing a
															// quick maintenance task.  The handler for
															// sysAppLaunchCmdAlarmTriggered must take as
															// little time as possible and is *not* allowed
															// to block (this would delay notification for
															// alarms set by other applications).  
															//
															// Parameter block: SysAlarmTriggeredParamType
															//		(defined in AlarmMgr.h)
															// Restrictions: No accessing of global or
															//		static variables unless sysAppLaunchFlagSubCall
															//		flag is set, as discussed above.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdDisplayAlarm		7  // Sent to an application when it is time
															// to display the alarm UI. The application
															// is responsible for making any alarm sounds
															// and for displaying the alarm UI.
															// sysAppLaunchCmdDisplayAlarm calls are ordered
															// chronoligically and are not overlapped.
															// This means that your app will receive
															// sysAppLaunchCmdDisplayAlarm only after
															// all earlier alarms have been displayed.
															//
															// Parameter block: SysDisplayAlarmParamType
															//		(defined in AlarmMgr.h)
															// Restrictions: No accessing of global or
															//		static variables unless sysAppLaunchFlagSubCall
															//		flag is set, as discussed above.  UI calls are
															//		allowed to display the app's alarm dialog.
															// Notes: This action code is sent via a direct
															//		function call into the app's PilotMain
															//		function without "launching" the app.

#define	sysAppLaunchCmdCountryChange		8  // The country has changed

#define	sysAppLaunchCmdSyncRequestLocal	9  // Sent to the HotSync application to request a
															// local HotSync.  ("HotSync" button was pressed.)

#define	sysAppLaunchCmdSyncRequest			sysAppLaunchCmdSyncRequestLocal	// for backward compatibility

#define	sysAppLaunchCmdSaveData			  	10 // Sent to running app before sysAppLaunchCmdFind
															// or other action codes that will cause data
															// searches or manipulation.

#define	sysAppLaunchCmdInitDatabase	  	11	// Sent to an application when a database with
															// a matching Creator ID is created during
															// HotSync (in response to a "create db"
															// request). This allows the application to
															// initialize a newly-created database during
															// HotSync.  This might include creating some
															// default records, setting up the database's
															// application and sort info blocks, etc.
															//
															// Parameter block: SysAppLaunchCmdInitDatabaseType
															// Restrictions: No accessing of global or
															//		static variables; no User Interface calls.
															// Notes: This action code is sent via a
															//		direct function call into the app's
															//		PilotMain function from the background
															//		thread of the HotSync application.

#define	sysAppLaunchCmdSyncCallApplicationV10	12 // Used by DesktopLink Server command "call application";
																	// Pilot v1.0 only!!!
																		
//------------------------------------------------------------------------
// New launch codes defined for PalmOS 2.0
//------------------------------------------------------------------------

#define	sysAppLaunchCmdPanelCalledFromApp	13 // The panel should display a done
																// button instead of the pick list.
																// The Done button will return the user
																// to the last app.

#define	sysAppLaunchCmdReturnFromPanel		14	// A panel returned to this app

#define	sysAppLaunchCmdLookup				  	15	// Lookup info managed by an app

#define	sysAppLaunchCmdSystemLock			  	16	// Lock the system until a password is entered.

#define	sysAppLaunchCmdSyncRequestRemote		17	// Sent to the HotSync application to request
																// a remote HotSync.  ("Remote HotSync" button
																// was pressed.)

#define	sysAppLaunchCmdHandleSyncCallApp		18 // Pilot v2.0 and greater.  Sent by DesktopLink Server to an application to handle
																// the "call application" command; use DlkControl with
																// control code dlkCtlSendCallAppReply to send the reply(see DLServer.h).
																// This action code replaces the v1.0 code sysAppLaunchCmdSyncCallApplication.
																// vmk 11/26/96

#define sysAppLaunchCmdAddRecord					19	// Add a record to an applications's database. 


//------------------------------------------------------------------------
// Standard Service Panel launch codes (used by network panel, dialer panel, etc.)
//------------------------------------------------------------------------
#define	sysSvcLaunchCmdSetServiceID			20
#define	sysSvcLaunchCmdGetServiceID			21
#define	sysSvcLaunchCmdGetServiceList			22
#define	sysSvcLaunchCmdGetServiceInfo			23


#define sysAppLaunchCmdFailedAppNotify			24	// An app just switched to failed. 
#define sysAppLaunchCmdEventHook					25	// Application event hook callback 
#define sysAppLaunchCmdExgReceiveData			26	// Exg command for app to receive data. 
#define sysAppLaunchCmdExgAskUser 				27	// Exg command sent before asking user. 


//------------------------------------------------------------------------
// Standard Dialer Service launch codes (30 - 39 reserved)
//------------------------------------------------------------------------

// sysDialLaunchCmdDial: dials the modem(optionally displays dial progress UI), given service id
// and serial library reference number
#define sysDialLaunchCmdDial						30
// sysDialLaunchCmdHangUp: hangs up the modem(optionally displays disconnect progress UI), given service id
// and serial library reference number
#define sysDialLaunchCmdHangUp					31
#define sysDialLaunchCmdLast						39


//------------------------------------------------------------------------
// Additional standard Service Panel launch codes (used by network panel, dialer panel, etc)
// (40-49 reserved)
//------------------------------------------------------------------------

#define sysSvcLaunchCmdGetQuickEditLabel		40		// SvcQuickEditLabelInfoType 
#define sysSvcLaunchCmdLast						49


//------------------------------------------------------------------------
// New launch codes defined for PalmOS 3.x where x >= 1
//------------------------------------------------------------------------

#define sysAppLaunchCmdURLParams					50		// Sent from the Web Clipper application.
																	// This launch code gets used to satisfy 
																	// URLs like the following:
																	//    palm:memo.appl?param1=value1&param2=value2
																	// Everything in the URL past the '?' is passed 
																	// to the app as the cmdPBP parameter of PilotMain(). 

#define sysAppLaunchCmdNotify						51		// This is a NotifyMgr notification sent 
																	// via SysNotifyBroadcast.  The cmdPBP parameter
																	// points to a SysNotifyParamType structure
																	// containing more specific information
																	// about the notification (e.g., what it's for).
																	
#define sysAppLaunchCmdOpenDB						52		// Sent to switch to an application and have it
																	// "open" up the given data file. The cmdPBP 
																	// pointer is a pointer to a SysAppLaunchCmdOpenDBType 
																	// structure that has the cardNo and localID of the database
																	// to open. This action code is used by the Launcher
																	// to launch data files, like Eleven PQA files that
																	// have the dmHdrAttrLaunchableData bit set in their
																	// database attributes. 

#define sysAppLaunchCmdAntennaUp					53		// Sent to switch only to the launcher when
																	// the antenna is raised and the launcher
																	// is the application in the buttons preferences
																	// that is to be run when the antenna is raised is
																	// the launcher.

#define sysAppLaunchCmdGoToURL					54		// Sent to Clipper to have it launch and display
																	// a given URL.  cmdPBP points to the URL string.

// Begin Change - BGT 03/21/2000

//------------------------------------------------------------------------
// New launch codes defined for Network panel plug-in
//------------------------------------------------------------------------
		
#define sysAppLaunchNppiNoUI						55		// Sent to network panel plug-in ("nppi") to have it launch 
																	// without UI and load to netlib
															
#define sysAppLaunchNppiUI							56		// Sent to network panel plug-in ("nppi") to have it launch 
																	// with UI
// End Change - BGT 03/21/2000

//------------------------------------------------------------------------
// New launch codes defined for PalmOS 4.x where x >= 0
//------------------------------------------------------------------------
															
#define sysAppLaunchCmdExgPreview				57		// Sent to an application by the Exchange Manager when the
																	// application needs to produce a preview.


#define sysAppLaunchCmdCardLaunch				58		// Sent to an application by the Launcher when the
																	// application is being run from a card.

#define sysAppLaunchCmdExgGetData				59			// Exg command for app to send data requested by an ExgGet 

																	
																	
#define sysAppLaunchCmdAttention					60		// sent to an application by the attention manager
																	// when the application needs to take action on an entry
																	// that has been submitted to the attention manager queue.

#define sysAppLaunchPnpsPreLaunch				61		//pre-launch code for Pnps devices, 
														//cmdPBP points to SysAppLaunchCmdPnpsType


#define sysAppLaunchCmdMultimediaEvent			63		// launch code for multimedia session event notification


// PALM OS 6 defines here

// PALM OS 5.3 continue here
#define sysAppLaunchCmdFepPanelAddWord			87		// launch code for adding a word to the FEP's user dictionary
 														// event notification.
#define sysAppLaunchCmdLookupWord				88		// launch code for word lookup event notification

// ***ADD NEW SYSTEM ACTION CODES BEFORE THIS COMMENT***

//------------------------------------------------------------------------
// Custom action code base (custom action codes begin at this value)
//------------------------------------------------------------------------
#define	sysAppLaunchCmdCustomBase	0x8000

// Your custom launch codes can be defined like this:
//
//	typedef enum {
//		myAppCmdDoSomething = sysAppLaunchCmdCustomBase, 
//		myAppCmdDoSomethingElse,
//		myAppCmdEtcetera
//
//		} MyAppCustomActionCodes;



//------------------------------------------------------------------------
// SysAppLaunch flags (passed to PilotMain)
//------------------------------------------------------------------------

#define	sysAppLaunchFlagNewThread		0x01	// create a new thread for application
															//  - implies sysAppLaunchFlagNewStack
#define	sysAppLaunchFlagNewStack		0x02	// create separate stack for application
#define	sysAppLaunchFlagNewGlobals		0x04	// create new globals world for application
															//  - implies new owner ID for Memory chunks
#define	sysAppLaunchFlagUIApp			0x08	// notifies launch routine that this is a UI app being
															//  launched.
#define	sysAppLaunchFlagSubCall			0x10	// notifies launch routine that the app is calling it's
															//  entry point as a subroutine call. This tells the launch
															//  code that it's OK to keep the A5 (globals) pointer valid 
															//  through the call.
															// IMPORTANT: This flag is for internal use by
															//  SysAppLaunch only!!! It should NEVER be set
															//  by the caller. 
#define sysAppLaunchFlagDataRelocated	0x80	// global data (static ptrs) have been "relocated"
															//  by either SysAppStartup or StartupCode.c
															// IMPORTANT: This flag is for internal use by
															//  SysAppLaunch only!!! It should NEVER be set
															//  by the caller. 

// The set of private, internal flags that should never be set by the caller
#define sysAppLaunchFlagPrivateSet		(sysAppLaunchFlagSubCall | sysAppLaunchFlagDataRelocated)



//-------------------------------------------------------------------
// Parameter blocks for action codes
// NOTE: The parameter block for the  sysAppLaunchCmdFind  and sysAppLaunchCmdGoTo
//  action codes are defined in "Find.h";
//---------------------------------------------------------------------------

// For sysAppLaunchCmdSaveData
typedef struct {
	Boolean		uiComing;							// true if system dialog will be put up
															// before coming action code arrives.
	UInt8			reserved1;
	} SysAppLaunchCmdSaveDataType;
	
// For sysAppLaunchCmdSystemReset
typedef struct {
	Boolean		hardReset;							// true if system was hardReset, false if soft-reset.
	Boolean		createDefaultDB;					// true if app should create default database.
	} SysAppLaunchCmdSystemResetType;
	

// For sysAppLaunchCmdInitDatabase
typedef struct SysAppLaunchCmdInitDatabaseType {
	DmOpenRef	dbP;									// Handle of the newly-created database,
															//		already open for read/write access.
															//		IMPORTANT: The handler *MUST* leave
															//		this database handle open on return.
	UInt32		creator;								//	Creator ID of the newly-created database
	UInt32		type;									// Type ID of the newly-created database
	UInt16		version;								// Version number of the newly-created database
	} SysAppLaunchCmdInitDatabaseType;


// For sysAppLaunchCmdSyncCallApplicationV10
// This structure used on Pilot v1.0 only.  See sysAppLaunchCmdHandleSyncCallApp
// for later platforms.
typedef struct SysAppLaunchCmdSyncCallApplicationTypeV10 {
	UInt16		action;					// call action id (app-specific)
	UInt16		paramSize;				// parameter size
	void *		paramP;					// ptr to parameter
	UInt8			remoteSocket;			// remote socket id
	UInt8			tid;						// command transaction id
	Boolean		handled;					// if handled, MUST be set true by the app
	UInt8			reserved1;
	} SysAppLaunchCmdSyncCallApplicationTypeV10;


// For sysAppLaunchCmdHandleSyncCallApp (Pilot v2.0 and greater).
// This structure replaces SysAppLaunchCmdSyncCallApplicationType
// which was used in Pilot v1.0
typedef struct SysAppLaunchCmdHandleSyncCallAppType {
	UInt16		pbSize;					// this parameter block size (set to sizeof SysAppLaunchCmdHandleSyncCallAppType)
	UInt16		action;					// call action id (app-specific)
	void *		paramP;					// ptr to parameter
	UInt32		dwParamSize;			// parameter size
	void *		dlRefP;					// DesktopLink reference pointer for passing
												// to DlkControl()'s dlkCtlSendCallAppReply code
												
	Boolean		handled;					// initialized to FALSE by DLServer; if
												// handled, MUST be set TRUE by the app(the
												// handler MUST call DlkControl with
												// control code dlkCtlSendCallAppReply);
												// if the handler is not going to send a reply,
												// it should leave this field set to FALSE, in which
												// case DesktopLink Server will send the default
												// "unknown request" reply.
	
	UInt8			reserved1;
											
	Err			replyErr;				// error from dlkCtlSendCallAppReply
	
	// RESERVED FOR FUTURE EXTENSIONS				
	UInt32		dwReserved1;			// RESERVED -- set to null!!!
	UInt32		dwReserved2;			// RESERVED -- set to null!!!

	// Target executable creator and type for testing the mechanism
	// in EMULATION MODE ONLY!!!
	#if EMULATION_LEVEL != EMULATION_NONE
		UInt32		creator;
		UInt32		type;
	#endif

	} SysAppLaunchCmdHandleSyncCallAppType;

// For sysAppLaunchCmdFailedAppNotify
typedef struct 
	{
	UInt32		creator;
	UInt32		type;
	Err			result;
	} SysAppLaunchCmdFailedAppNotifyType;
	
	
// For sysAppLaunchCmdOpenDB
typedef struct 
	{
	UInt16		cardNo;
	LocalID		dbID;
	} SysAppLaunchCmdOpenDBType;
	

// For sysAppLaunchCmdCardLaunch
typedef struct 
	{
	Err			err;
	UInt16		volRefNum;
	const Char	*path;
	UInt16		startFlags;			// See vfsStartFlagXXX constants below
	} SysAppLaunchCmdCardType;

#define sysAppLaunchStartFlagAutoStart		0x0001	// this bit in the 'startFlags' field is set for an app which is run automatically on card insertion
#define sysAppLaunchStartFlagNoUISwitch		0x0002	// set this bit in the 'startFlags' field to prevent a UI switch to the start.prc app
#define sysAppLaunchStartFlagNoAutoDelete	0x0004	// set this bit in the 'startFlags' field to prevent VFSMgr from deleting start.prc app on volume unmount


//for launch code sysAppLaunchPnpsPreLaunch
typedef struct {
	Err		error;			//an error code from the pre-launch application, set to errNone to prevent normal launching
	UInt16	volRefNum;		//Non-zero if an optional file system was mounted
	UInt16	slotLibRefNum;	//always valid for a slot driver call
 	UInt16	slotRefNum;		//always valid for a slot driver call 
 }SysAppLaunchCmdPnpsType;


/************************************************************
 * Structure of Application info for an application. Applications
 *  do not necessarily have to be on their own thread - there
 *  can be more than 1 app on the same AMX task. Each application
 *  has an assocated SysAppInfoType structure which holds the
 *  application specific information like the database MemHandle of the
 *  app, the code MemHandle, the stack chunk pointer, the owner ID, etc.
 *
 * As of PalmOS 3.X, one of these structures is created for each 
 *  app running as an action code.
 *
 ****
 ****IMPORTANT: ADD NEW FIELDS AT THE END OF THE STRUCTURE FOR
 ****           BACKWARD COMPATIBILITY 
 ****
 *************************************************************/
typedef struct SysAppInfoType {
	Int16				cmd;							// command code for app
	MemPtr			cmdPBP;						// cmd ParamBlock  
	UInt16			launchFlags;				// launch flags  
	
	UInt32			taskID;						// AMX task ID of task that app runs in
	MemHandle		codeH;						// handle of the main code segment
	DmOpenRef		dbP;							// Application database access pointer of App
	UInt8				*stackP;						// stack chunk for the App
	UInt8				*globalsChunkP;			// globals chunk for the App

	UInt16			memOwnerID;					// owner ID for Memory Manager chunks 
	MemPtr			dmAccessP;					// pointer to linked list of opened DB's
	Err				dmLastErr;					// Last error from Data Manager
	MemPtr			errExceptionP;				// ErrTry,Catch exception list

	// PalmOS v3.0 fields begin here	
	UInt8				*a5Ptr;						// A5 pointer for this app
	UInt8				*stackEndP;					// stack chunk end for the App (last byte)
	UInt8				*globalEndP;				// global chunk end for the App (last byte)
	struct SysAppInfoType *rootP;				// Points to the SysAppInfoType first
														// allocated for this thread.
	MemPtr			extraP;						// unused pointer for the App.
	} SysAppInfoType;
typedef SysAppInfoType *SysAppInfoPtr;


/************************************************************
 * Function prototype for libraries
 *************************************************************/

// ***IMPORTANT***
// ***IMPORTANT***
// ***IMPORTANT***
//
// The assembly level TrapDispatcher() function uses a hard-coded value for
// the size of the structure SysLibTblEntryType to obtain a pointer to a
// library entry in the library table.  Therefore, any changes to this structure,
// require corresponding changes in TrapDispatcher() in ROMBoot.c.  Furthermore,
// it is advantageous to keep the size of the structure a power of 2 as this
// improves performance by allowing the entry offset to be calculated by shifting
// left instead of using the multiply instruction.  vmk 8/27/96 (yes, I fell into
// this trap myself)
typedef struct SysLibTblEntryType {
	MemPtr		*dispatchTblP;					// pointer to library dispatch table
	void			*globalsP;						// Library globals
	
	// New INTERNAL fields for v2.0 (vmk 8/27/96):
	LocalID		dbID;								// database id of the library
	void			*codeRscH;						// library code resource handle for RAM-based libraries
	} SysLibTblEntryType;
typedef SysLibTblEntryType*	SysLibTblEntryPtr;

// Emulated versions of libraries have a slightly different dispatch table
// Enough for the offset to the library name and the name itself.
#if EMULATION_LEVEL != EMULATION_NONE
typedef struct SimDispatchTableType {
	UInt32	numEntries;							// number of library entries
	void		*entries[1];						// dispatch routine entries
														// followed by pointer to name
   } SimDispatchTableType;
typedef SimDispatchTableType*	SimDispatchTablePtr;
#endif


// Library entry point procedure
typedef Err (*SysLibEntryProcPtr)(UInt16 refNum, SysLibTblEntryPtr entryP);

// This library refNum is reserved for the Debugger comm library
#define	sysDbgCommLibraryRefNum		0

// This portID is reserved for identifying the debugger's port
#define  sysDbgCommPortID				0xC0FF

// This refNum signals an invalid refNum
#define	sysInvalidRefNum				0xFFFF


/************************************************************
 * Function prototype for Kernel
 *************************************************************/
// Task termination procedure prototype for use with SysTaskSetTermProc
typedef void (*SysTermProcPtr)(UInt32 taskID, Int32 reason);

// Timer procedure for use with SysTimerCreate
typedef void (*SysTimerProcPtr)(Int32 timerID, Int32 param);


/************************************************************
 * Structure of the pref=0 resource in applications. Note, this
 *  structure must mirror the structure of the sysResTAppPrefs
 *	 resource as defined in SystemResources.h.
 *************************************************************/
typedef struct SysAppPrefs {
	UInt16	priority;					// task priority
	UInt32	stackSize;					// required stack space
	UInt32	minHeapSpace;				// minimum heap space required
	} SysAppPrefsType;
typedef SysAppPrefsType *SysAppPrefsPtr;


/************************************************************
 * Structure of the xprf=0 resource in resource DBs. Note, this
 * structure must mirror the structure of the sysResTExtPrefs
 * resource as defined in SystemResources.h. Also, fields can only
 *	be added (at the end), never removed or changed.
 *************************************************************/

#define	sysExtPrefsVers				1

// Flags defined for SysExtPrefsType.flags
#define	sysExtPrefsNoOverlayFlag	0x00000001

typedef struct SysExtPrefsType {
	UInt16	version;						// version of structure.
	UInt32	flags;						// 32 boolean flags.
	} SysExtPrefsType;



/************************************************************
 * System Errors
 *************************************************************/
#define	sysErrTimeout							(sysErrorClass | 1)
#define	sysErrParamErr							(sysErrorClass | 2)
#define	sysErrNoFreeResource					(sysErrorClass | 3)
#define	sysErrNoFreeRAM						(sysErrorClass | 4)
#define	sysErrNotAllowed						(sysErrorClass | 5)
#define	sysErrSemInUse							(sysErrorClass | 6)
#define	sysErrInvalidID						(sysErrorClass | 7)
#define	sysErrOutOfOwnerIDs					(sysErrorClass | 8)
#define	sysErrNoFreeLibSlots					(sysErrorClass | 9)
#define	sysErrLibNotFound						(sysErrorClass | 10)
#define	sysErrDelayWakened					(sysErrorClass | 11)	// SysTaskDelay wakened by SysTaskWake before delay completed.
#define	sysErrRomIncompatible				(sysErrorClass | 12)
#define	sysErrBufTooSmall						(sysErrorClass | 13)
#define	sysErrPrefNotFound					(sysErrorClass | 14)

// NotifyMgr error codes:
#define	sysNotifyErrEntryNotFound			(sysErrorClass | 16) // could not find registration entry in the list
#define	sysNotifyErrDuplicateEntry			(sysErrorClass | 17) // identical entry already exists
#define	sysNotifyErrBroadcastBusy			(sysErrorClass | 19) // a broadcast is already in progress - try again later.
#define	sysNotifyErrBroadcastCancelled	(sysErrorClass | 20) // a handler cancelled the broadcast

// AMX error codes continued - jb 10/20/98
#define	sysErrMbId								(sysErrorClass | 21)
#define	sysErrMbNone							(sysErrorClass | 22)	
#define	sysErrMbBusy							(sysErrorClass | 23)
#define	sysErrMbFull							(sysErrorClass | 24)
#define	sysErrMbDepth							(sysErrorClass | 25)
#define	sysErrMbEnv								(sysErrorClass | 26)

// NotifyMgr Phase #2 Error Codes:
#define	sysNotifyErrQueueFull				(sysErrorClass | 27) // deferred queue is full.
#define	sysNotifyErrQueueEmpty				(sysErrorClass | 28) // deferred queue is empty.
#define	sysNotifyErrNoStackSpace			(sysErrorClass | 29) // not enough stack space for a broadcast
#define	sysErrNotInitialized					(sysErrorClass | 30) // manager is not initialized

// AMX error/warning codes continued - jed 9/10/99
#define	sysErrNotAsleep						(sysErrorClass | 31)	// Task woken by SysTaskWake was not asleep, 1 wake pending
#define	sysErrNotAsleepN						(sysErrorClass | 32)	// Task woken by SysTaskWake was not asleep, >1 wake pending


// Power Manager error codes - soe, srj 9/19/00
#define	pwrErrNone								(pwrErrorClass | 0)
#define	pwrErrBacklight						(pwrErrorClass | 1)
#define	pwrErrRadio								(pwrErrorClass | 2)
#define	pwrErrBeam								(pwrErrorClass | 3)
#define  pwrErrGeneric							(pwrErrorClass | 4)


/************************************************************
 * System Features
 *************************************************************/
#define	sysFtrCreator			sysFileCSystem		// Feature Creator

#define	sysFtrNumROMVersion	1						// ROM Version
			// 0xMMmfsbbb, where MM is major version, m is minor version
			// f is bug fix, s is stage: 3-release,2-beta,1-alpha,0-development,
			// bbb is build number for non-releases 
			// V1.12b3   would be: 0x01122003
			// V2.00a2   would be: 0x02001002
			// V1.01     would be: 0x01013000

#define	sysFtrNumProcessorID	2						// Product id
			// 0xMMMMRRRR, where MMMM is the processor model and RRRR is the revision.
#define	sysFtrNumProcessorMask		0xFFFF0000		// Mask to obtain processor model
#define	sysFtrNumProcessor328		0x00010000		// Motorola 68328		(Dragonball)
#define	sysFtrNumProcessorEZ		0x00020000		// Motorola 68EZ328	(Dragonball EZ)
#define	sysFtrNumProcessorVZ		0x00030000		// Motorola 68VZ328	(Dragonball VZ)
#define	sysFtrNumProcessorSuperVZ	0x00040000		// Motorola 68SZ328	(Dragonball SuperVZ)
#define	sysFtrNumProcessorARM720T	0x00100000		// ARM 720T
#define sysFtrNumProcessorARM7TDMI	0x00110000		// ARM7TDMI
#define sysFtrNumProcessorARM920T	0x00120000		// ARM920T
#define sysFtrNumProcessorARM922T	0x00130000		// ARM922T
#define sysFtrNumProcessorARM925	0x00140000		// ARM925
#define sysFtrNumProcessorStrongARM	0x00150000		// StrongARM
#define sysFtrNumProcessorXscale	0x00160000		// Xscale
#define sysFtrNumProcessorARM710A	0x00170000		// ARM710A
#define sysFtrNumProcessorARM925T	0x00180000		// ARM925T
#define	sysFtrNumProcessorx86		0x01000000		// Intel CPU		(Palm Simulator)

// The following sysFtrNumProcessorIs68K(x) and sysFtrNumProcessorIsARM(x)
// macros are intended to be used to test the value returned from a call to
//    FtrGet(sysFtrCreator, sysFtrNumProcessorID, &value);
// in order to determine if the code being executed is running on a 68K or ARM processor.
 
#define sysFtrNumProcessor68KIfZero    0xFFF00000   // 68K if zero; not 68K if non-zero
#define sysFtrNumProcessorIs68K(x)     (((x&sysFtrNumProcessor68KIfZero)==0)? true : false)

#define sysFtrNumProcessorARMIfNotZero 0x00F00000   // ARM if non-zero
#define sysFtrNumProcessorIsARM(x)     (((x&sysFtrNumProcessorARMIfNotZero)!=0)? true : false)

#define	sysFtrNumProductID	sysFtrNumProcessorID	// old (obsolete) define

#define	sysFtrNumBacklight	3						// Backlight
			// bit 0:	1 if present. 0 if Feature does not exist or backlight is not present

#define	sysFtrNumEncryption	4						// Which encryption schemes are present
#define	sysFtrNumEncryptionMaskDES		0x00000001 // bit 0:	1 if DES is present

#define	sysFtrNumCountry		5						// International ROM identifier
			// Result is of type CountryType as defined in Preferences.h.
			// Result is essentially the "default" country for this ROM.
			// Assume cUnitedStates if sysFtrNumROMVersion >= 02000000
			// and feature does not exist. Result is in low sixteen bits.
			
#define	sysFtrNumLanguage		6						// Language identifier
			// Result is of untyped; values are defined in Incs:BuildRules.h
			// Result is essentially the "default" language for this ROM.
			// This is new for the WorkPad (v2.0.2) and did NOT exist for any of the
			// following: GermanPersonal, GermanPro, FrenchPersonal, FrenchPro
			// Thus we can't really assume anything if the feature doesn't exist,
			// though the actual language MAY be determined from sysFtrNumCountry,
			// above. Result is in low sixteen bits.

#define	sysFtrNumDisplayDepth	7					// Display depth
			// Result is the "default" display depth for the screen.					(PalmOS 3.0)
			// This value is used by ScrDisplayMode when setting the default display depth.
			
#define	sysFtrNumHwrMiscFlags		8				// GHwrMiscFlags value			(PalmOS 3.1)
#define	sysFtrNumHwrMiscFlagsExt	9				// GHwrMiscFlagsExt value		(PalmOS 3.1)
			
#define	sysFtrNumIntlMgr				10
			// Result is a set of flags that define functionality supported
			// by the Int'l Manager.															(PalmOS 3.1)

#define	sysFtrNumEncoding				11
			// Result is the character encoding (defined in PalmLocale.h) supported
			// by this ROM. If this feature doesn't exist then the assumed encoding
			// is PalmLatin (superset of Windows code page 1252)						(PalmOS 3.1)
			
#define	sysFtrDefaultFont				12
			// Default font ID used for displaying text.									(PalmOS 3.1)

#define	sysFtrDefaultBoldFont		13
			// Default font ID used for displaying bold text.							(PalmOS 3.1)

#define	sysFtrNumGremlinsSupportGlobals	14		// Globals for supporting gremlins.
			// This value is a pointer to a memory location that stores global variables needed
			// for intelligently supporting gremlins.  Currently, it is only used in Progress.c.
			// It is only initialized on first use (gremlins and progress bar in combination)
			// when ERROR_CHECK_LEVEL == ERROR_CHECK_FULL.								(PalmOS 3.2)

#define	sysFtrNumVendor				15
			// Result is the vendor id, in the low sixteen bits.						(PalmOS 3.3)

#define	sysFtrNumCharEncodingFlags	16
			// Flags for a given character encoding, specified in TextMgr.h		(PalmOS 3.5)
			
#define	sysFtrNumNotifyMgrVersion	17 // version of the NotifyMgr, if any		(PalmOS 3.5)
			
#define	sysFtrNumOEMROMVersion		18	// Supplemental ROM version, provided by OEM
			// This value may be present in OEM devices, and is in the same format
			// as sysFtrNumROMVersion.															(PalmOS 3.5)

#define	sysFtrNumErrorCheckLevel	19 // ROM build setting of ERROR_CHECK_LEVEL
			// May be set to ERROR_CHECK_NONE, ERROR_CHECK_PARTIAL, or ERROR_CHECK_FULL
			// as defined in <BuildDefines.h>.												(PalmOS 3.5)

#define	sysFtrNumOEMCompanyID			20			// GHwrOEMCompanyID value		(PalmOS 3.5)
#define	sysFtrNumOEMDeviceID			21			// GHwrOEMDeviceID value		(PalmOS 3.5)
#define	sysFtrNumOEMHALID				22			// GHwrOEMHALID value			(PalmOS 3.5)
#define	sysFtrNumDefaultCompression		23			// Default Clipper's compression (Palmos 3.5)

#define 	sysFtrNumWinVersion			24			// Window version					(PalmOS 4.0)

#define	sysFtrNumAccessorTrapPresent	25			// If accessor trap exists		(PalmOS 4.0)

#define	sysFtrNumInputAreaFlags		26			// Active silkscreen flags (PalmOS 4.2SC)
			// Flags for active silkscreen, specified in Graffiti.h



/************************************************************
 * ROM token information (for SysGetROMToken, below)
 *************************************************************/
// Additional tokens and token information is located in <Hardware.h>
#define	sysROMTokenSnum			'snum'	// Memory Card Flash ID (serial number)


/************************************************************
 * Macros for extracting and combining ROM/OS version components
 *************************************************************/

// ROM/OS stage numbers
#define sysROMStageDevelopment	(0)
#define sysROMStageAlpha			(1)
#define sysROMStageBeta				(2)
#define sysROMStageRelease			(3)


// MACRO: sysMakeROMVersion
//
// Builds a ROM version value from the major, minor, fix, stage, and build numbers
//
#define sysMakeROMVersion(major, minor, fix, stage, buildNum)			\
		(																					\
		(((UInt32)((UInt8)(major) & 0x0FF)) << 24) |							\
		(((UInt32)((UInt8)(minor) & 0x00F)) << 20) |							\
		(((UInt32)((UInt8)(fix)   & 0x00F)) << 16) |							\
		(((UInt32)((UInt8)(stage) & 0x00F)) << 12) |							\
		(((UInt32)((UInt16)(buildNum) & 0x0FFF)))								\
		)


// Macros for parsing the ROM version number
// (the system OS version is obtained by calling
// FtrGet(sysFtrCreator, sysFtrNumROMVersion, dwOSVerP), where dwOSVerP is
// a pointer to to a UInt32 variable that is to receive the OS version number)
#define sysGetROMVerMajor(dwROMVer)		(((UInt16)((dwROMVer) >> 24)) & 0x00FF)
#define sysGetROMVerMinor(dwROMVer)		(((UInt16)((dwROMVer) >> 20)) & 0x000F)
#define sysGetROMVerFix(dwROMVer)		(((UInt16)((dwROMVer) >> 16)) & 0x000F)
#define sysGetROMVerStage(dwROMVer)		(((UInt16)((dwROMVer) >> 12)) & 0x000F)
#define sysGetROMVerBuild(dwROMVer)		(((UInt16)(dwROMVer))         & 0x0FFF)




/************************************************************
 * System Types
 *************************************************************/
 
// Types of batteries installed.
typedef enum {
	sysBatteryKindAlkaline=0,
	sysBatteryKindNiCad,
	sysBatteryKindLiIon,
	sysBatteryKindRechAlk,
	sysBatteryKindNiMH,
	sysBatteryKindLiIon1400,
	sysBatteryKindLast=0xFF   // insert new battery types BEFORE this one
	} SysBatteryKind;
	
// Different battery states (output of hwrBattery)
typedef enum {
	sysBatteryStateNormal=0,
	sysBatteryStateLowBattery,
	sysBatteryStateCritBattery,
	sysBatteryStateShutdown
	} SysBatteryState;


// SysCreateDataBaseList can generate a list of database.
typedef struct 
	{
	Char			name[dmDBNameLength];
	UInt32		creator;
	UInt32		type;
	UInt16		version;
	LocalID		dbID;
	UInt16 		cardNo;
	BitmapPtr	iconP;
	} SysDBListItemType;
	

// Structure of a generic message that can be send to a mailbox
// through the SysMailboxSend call. Note, this structure MUST 
// be  CJ_MAXMSZ bytes large, where CJ_MAXMSZ is defined in
// the AMX includes.
typedef struct {
	UInt32		data[3];
	} SysMailboxMsgType;
	

// Constants used by the SysEvGroupSignal call
#define	sysEvGroupSignalConstant			0
#define	sysEvGroupSignalPulse				1

// Constants used by the SysEvGroupWait call
#define	sysEvGroupWaitOR						0
#define	sysEvGroupWaitAND						1



/************************************************************
 * System Pre-defined "file descriptors"
 * These are used by applications that use the  Net Library's 
 *   NetLibSelect() call 
 *************************************************************/
#define	sysFileDescStdIn						0


//============================================================================
// jhl 7/26/00 Integrate HSIMgr functionality
//============================================================================
#define sysNotifyHSISerialPortInUseEvent				'hsiu'	// Sent when serial port is in use
#define sysNotifyHSIPeripheralRespondedEvent			'hspr'	// Sent with peripheral response
#define sysNotifyHSIPeripheralNotRespondingEvent		'hspn'	// Sent when peripheral does not respond
#define sysNotifyHSINoConnectionEvent					'ncon'	// Sent on VID of no connection
#define sysNotifyHSIUSBCradleEvent						sysPortUSBDesktop	// Sent on VID of USB Cradle
#define sysNotifyHSIRS232CradleEvent					'rs2c'	// Sent on VID of RS232 Cradle
#define sysNotifyHSIUSBPeripheralEvent					sysPortUSBPeripheral	// Sent on VID of USB Peripheral
#define sysNotifyHSIRS232PeripheralEvent				'rs2p'	// Sent on VID of RS232 Peripheral
#define sysNotifyHSIDebugEvent							'dbug'	// Sent on VID of Debug

#define sysMaxHSIResponseSize					64
#define sysHSISerialInquiryBaud				9600
#define sysHSISerialInquiryString			"ATI3\015\012"
#define sysHSISerialInquiryStringLen		6
#define sysHSISerialInterChrTimeout			3				// ticks (20-30 ms)
#define sysHSISerialInquiryTimeout			11				// ticks (100-110 ms)

typedef struct SysHSIResponseType {
	// "Voltage ID" from modem pin converted to 4 character VID
//	UInt32		VID;
	// Actual voltage detected on modem VID pin
//	UInt16		mVolts;
	// Character string received in response to inquiry string
	// (will be NUL terminated)
	Char			responseBuffer[sysMaxHSIResponseSize];
	// Length of string in responseBuffer
	UInt16		responseLength;
} SysHSIResponseType;


// Constants defined for future use

// Orientation states
#define sysOrientationUser             0
#define sysOrientationPortrait         1
#define sysOrientationLandscape        2
#define sysOrientationReversePortrait  3
#define sysOrientationReverseLandscape 4

// Orientation trigger states
#define sysOrientationTriggerDisabled  0
#define sysOrientationTriggerEnabled   1



/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// Prototype for Pilot applications entry point
UInt32	PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags);


// SystemMgr routines
void		SysUnimplemented(void)
							SYS_TRAP(sysTrapSysUnimplemented);

void		SysColdBoot(void *card0P, UInt32 card0Size, 
							void *card1P, UInt32 card1Size,
							UInt32	sysCardHeaderOffset)
							SYS_TRAP(sysTrapSysColdBoot);
							
void 		SysInit(void)
							SYS_TRAP(sysTrapSysInit);
							
void 		SysReset(void)
							SYS_TRAP(sysTrapSysReset);
							
void 		SysPowerOn(void *card0P, UInt32 card0Size, 
							void *card1P, UInt32 card1Size,
							UInt32 sysCardHeaderOffset, Boolean reFormat);
							
							
void 		SysDoze(Boolean onlyNMI)
							SYS_TRAP(sysTrapSysDoze);
							
Err 		SysSetPerformance(UInt32 *sysClockP, UInt16 *cpuDutyP)
							SYS_TRAP(sysTrapSysSetPerformance);
							
void 		SysSleep(Boolean untilReset, Boolean emergency)
							SYS_TRAP(sysTrapSysSleep);
							
UInt16 	SysSetAutoOffTime(UInt16	seconds)
							SYS_TRAP(sysTrapSysSetAutoOffTime);

UInt16 	SysTicksPerSecond(void)
							SYS_TRAP(sysTrapSysTicksPerSecond);

Err		SysLaunchConsole(void)
							SYS_TRAP(sysTrapSysLaunchConsole);

Boolean	SysHandleEvent(EventPtr eventP)
							SYS_TRAP(sysTrapSysHandleEvent);
							
void 		SysUILaunch(void)
							SYS_TRAP(sysTrapSysUILaunch);

Err 		SysUIAppSwitch(UInt16 cardNo, LocalID dbID, UInt16 cmd, MemPtr cmdPBP)
							SYS_TRAP(sysTrapSysUIAppSwitch);
							
Err 		SysCurAppDatabase(UInt16 *cardNoP, LocalID *dbIDP)
							SYS_TRAP(sysTrapSysCurAppDatabase);
							
Err		SysBroadcastActionCode(UInt16 cmd, MemPtr cmdPBP)
							SYS_TRAP(sysTrapSysBroadcastActionCode);
							
Err 		SysAppLaunch(UInt16 cardNo, LocalID dbID, UInt16 launchFlags,
							UInt16 cmd, MemPtr cmdPBP, UInt32 *resultP)
							SYS_TRAP(sysTrapSysAppLaunch);
							
UInt16	SysNewOwnerID(void)
							SYS_TRAP(sysTrapSysNewOwnerID);
							
UInt32	SysSetA5(UInt32 newValue)
							SYS_TRAP(sysTrapSysSetA5);

// Routines used by startup code
Err		SysAppStartup(SysAppInfoPtr *appInfoPP, MemPtr *prevGlobalsP, 
							MemPtr *globalsPtrP)
							SYS_TRAP(sysTrapSysAppStartup);
							
Err		SysAppExit(SysAppInfoPtr appInfoP, MemPtr prevGlobalsP, MemPtr globalsP)
							SYS_TRAP(sysTrapSysAppExit);
							

#if EMULATION_LEVEL != EMULATION_NONE
// Simulator-specific routines
MemPtr	SysCardImageInfo(UInt16 cardNo, UInt32 *sizeP);
							
void		SysCardImageDeleted(UInt16 cardNo);
#endif  // EMULATION_LEVEL != EMULATION_NONE

UInt16 	SysUIBusy(Boolean set, Boolean value)
							SYS_TRAP(sysTrapSysUIBusy);

UInt8		SysLCDContrast(Boolean set, UInt8 newContrastLevel)
							SYS_TRAP(sysTrapSysLCDContrast);

UInt8		SysLCDBrightness(Boolean set, UInt8 newBrightnessLevel)
							SYS_TRAP(sysTrapSysLCDBrightness);


// System Dialogs
void		SysBatteryDialog(void)
							SYS_TRAP(sysTrapSysBatteryDialog);

// Utilities
Err		SysSetTrapAddress(UInt16 trapNum, void *procP)
							SYS_TRAP(sysTrapSysSetTrapAddress);
							
void *	SysGetTrapAddress(UInt16 trapNum)
							SYS_TRAP(sysTrapSysGetTrapAddress);
							
UInt16	SysDisableInts(void)
							SYS_TRAP(sysTrapSysDisableInts);
							
void		SysRestoreStatus(UInt16 status)
							SYS_TRAP(sysTrapSysRestoreStatus);
							
extern Char * SysGetOSVersionString()
							SYS_TRAP(sysTrapSysGetOSVersionString);

// The following trap is a public definition of HwrGetROMToken from <Hardware.h>
// See token definitions (like sysROMTokenSerial) above...
Err		SysGetROMToken(UInt16 cardNo, UInt32 token, UInt8 **dataP, UInt16 *sizeP )
							SYS_TRAP(sysTrapHwrGetROMToken);


// Library Management
Err		SysLibInstall(SysLibEntryProcPtr libraryP, UInt16 *refNumP)
							SYS_TRAP(sysTrapSysLibInstall);

Err		SysLibLoad(UInt32 libType, UInt32 libCreator, UInt16 *refNumP)
							SYS_TRAP(sysTrapSysLibLoad);

							
Err		SysLibRemove(UInt16 refNum)
							SYS_TRAP(sysTrapSysLibRemove);
							
Err		SysLibFind(const Char *nameP, UInt16 *refNumP)
							SYS_TRAP(sysTrapSysLibFind);
							
SysLibTblEntryPtr	SysLibTblEntry(UInt16 refNum)
							SYS_TRAP(sysTrapSysLibTblEntry);
							
// Generic Library calls
Err		SysLibOpen(UInt16 refNum)
							SYS_TRAP(sysLibTrapOpen);
Err		SysLibClose(UInt16 refNum)
							SYS_TRAP(sysLibTrapClose);
Err		SysLibSleep(UInt16 refNum)
							SYS_TRAP(sysLibTrapSleep);
Err		SysLibWake(UInt16 refNum)
							SYS_TRAP(sysLibTrapWake);
							
							
//-----------------------------------------------------
// Kernel Prototypes
//-----------------------------------------------------
// Task Creation and deleation
Err		SysTranslateKernelErr(Err err)
							SYS_TRAP(sysTrapSysTranslateKernelErr);
							
Err		SysTaskCreate(UInt32 *taskIDP, UInt32 *creator, ProcPtr codeP,
							MemPtr stackP, UInt32 stackSize, UInt32 attr, UInt32 priority,
							UInt32 tSlice)
							SYS_TRAP(sysTrapSysTaskCreate);
							
Err		SysTaskDelete(UInt32 taskID, UInt32 priority)
							SYS_TRAP(sysTrapSysTaskDelete);
							
Err		SysTaskTrigger(UInt32 taskID)
							SYS_TRAP(sysTrapSysTaskTrigger);

UInt32	SysTaskID()
							SYS_TRAP(sysTrapSysTaskID);
							
Err		SysTaskDelay(Int32 delay)
							SYS_TRAP(sysTrapSysTaskDelay);
							
Err		SysTaskSetTermProc(UInt32 taskID, SysTermProcPtr termProcP)
							SYS_TRAP(sysTrapSysTaskSetTermProc);
							
Err		SysTaskSwitching(Boolean enable)
							SYS_TRAP(sysTrapSysTaskSwitching);
							
Err		SysTaskWait(Int32 timeout)
							SYS_TRAP(sysTrapSysTaskWait);
							
Err		SysTaskWake(UInt32 taskID)
							SYS_TRAP(sysTrapSysTaskWake);		
							
void		SysTaskWaitClr(void)
							SYS_TRAP(sysTrapSysTaskWaitClr);	
						
Err		SysTaskSuspend(UInt32 taskID)
							SYS_TRAP(sysTrapSysTaskSuspend);
					
Err		SysTaskResume(UInt32 taskID)
							SYS_TRAP(sysTrapSysTaskResume);				
							
							
// Counting Semaphores
Err		SysSemaphoreCreate(UInt32 *smIDP, UInt32 *tagP, Int32 initValue)
							SYS_TRAP(sysTrapSysSemaphoreCreate);
							
Err		SysSemaphoreDelete(UInt32 smID)
							SYS_TRAP(sysTrapSysSemaphoreDelete);
							
Err		SysSemaphoreWait(UInt32 smID, UInt32 priority, Int32 timeout)
							SYS_TRAP(sysTrapSysSemaphoreWait);
							
Err		SysSemaphoreSignal(UInt32 smID)
							SYS_TRAP(sysTrapSysSemaphoreSignal);
							
Err		SysSemaphoreSet(UInt32 smID)
							SYS_TRAP(sysTrapSysSemaphoreSet);
							
							
// Resource Semaphores				
Err		SysResSemaphoreCreate(UInt32 *smIDP, UInt32 *tagP)
							SYS_TRAP(sysTrapSysResSemaphoreCreate);
							
Err		SysResSemaphoreDelete(UInt32 smID)
							SYS_TRAP(sysTrapSysResSemaphoreDelete);
							
Err		SysResSemaphoreReserve(UInt32 smID, UInt32 priority, Int32 timeout)
							SYS_TRAP(sysTrapSysResSemaphoreReserve);
							
Err		SysResSemaphoreRelease(UInt32 smID)
							SYS_TRAP(sysTrapSysResSemaphoreRelease);
							
							
							
// Timers							
Err		SysTimerCreate(UInt32 *timerIDP, UInt32 *tagP, 
							SysTimerProcPtr timerProc, UInt32 periodicDelay,
							UInt32	param)
							SYS_TRAP(sysTrapSysTimerCreate);
							
Err		SysTimerDelete(UInt32 timerID)
							SYS_TRAP(sysTrapSysTimerDelete);
							
Err		SysTimerWrite(UInt32 timerID, UInt32 value)
							SYS_TRAP(sysTrapSysTimerWrite);
							 
Err		SysTimerRead(UInt32 timerID, UInt32 *valueP)
							SYS_TRAP(sysTrapSysTimerRead);
							 

// Information
Err		SysKernelInfo(void *paramP)
							SYS_TRAP(sysTrapSysKernelInfo);

Boolean SysCreateDataBaseList(UInt32 type, UInt32 creator, UInt16 *dbCount, 
						MemHandle *dbIDs, Boolean lookupName)
							SYS_TRAP(sysTrapSysCreateDataBaseList);

Boolean	SysCreatePanelList(UInt16 *panelCount, MemHandle *panelIDs)
							SYS_TRAP(sysTrapSysCreatePanelList);
					
UInt16	SysBatteryInfo(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP,
						Int16 *maxTicksP, SysBatteryKind* kindP, Boolean *pluggedIn, UInt8 *percentP)
							SYS_TRAP(sysTrapSysBatteryInfo);
							
UInt16	SysBatteryInfoV20(Boolean set, UInt16 *warnThresholdP, UInt16 *criticalThresholdP,
						Int16 *maxTicksP, SysBatteryKind* kindP, Boolean *pluggedIn)
							SYS_TRAP(sysTrapSysBatteryInfoV20);
							
Boolean	SysGetStackInfo(MemPtr *startPP, MemPtr *endPP)
							SYS_TRAP(sysTrapSysGetStackInfo);



// Mailboxes
Err		SysMailboxCreate(UInt32 *mbIDP, UInt32 *tagP, UInt32 depth)
							SYS_TRAP(sysTrapSysMailboxCreate);

Err		SysMailboxDelete(UInt32 mbID)
							SYS_TRAP(sysTrapSysMailboxDelete);
							
Err		SysMailboxFlush(UInt32 mbID)
							SYS_TRAP(sysTrapSysMailboxFlush);

Err		SysMailboxSend(UInt32 mbID, void *msgP, UInt32 wAck)
							SYS_TRAP(sysTrapSysMailboxSend);

Err		SysMailboxWait(UInt32 mbID, void *msgP, UInt32 priority,
								Int32 timeout)
							SYS_TRAP(sysTrapSysMailboxWait);		
							
// Event Groups
Err		SysEvGroupCreate(UInt32 *evIDP, UInt32 *tagP, UInt32 init)
							SYS_TRAP(sysTrapSysEvGroupCreate);
							
//Err		SysEvGroupDelete(UInt32 evID)									// save trap table space - don't need
							//SYS_TRAP(sysTrapSysEvGroupDelete);
							
Err		SysEvGroupSignal(UInt32 evID, UInt32 mask, UInt32 value, Int32 type)
							SYS_TRAP(sysTrapSysEvGroupSignal);
							
Err		SysEvGroupRead(UInt32 evID, UInt32 *valueP)
							SYS_TRAP(sysTrapSysEvGroupRead);
							
Err		SysEvGroupWait(UInt32 evID, UInt32 mask, UInt32 value, Int32 matchType,
									Int32 timeout)
							SYS_TRAP(sysTrapSysEvGroupWait);
							


UInt16 SysGetOrientation(void)
						PINS_TRAP(pinSysGetOrientation);

Err SysSetOrientation(UInt16 orientation)
						PINS_TRAP(pinSysSetOrientation);


UInt16 SysGetOrientationTriggerState(void )
							PINS_TRAP(pinSysGetOrientationTriggerState);


Err SysSetOrientationTriggerState(UInt16 triggerState)
							PINS_TRAP(pinSysSetOrientationTriggerState);

#ifdef __cplusplus 
}
#endif



/************************************************************
 * Assembly Function Prototypes
 *************************************************************/
#define	_SysSemaphoreSignal		\
				ASM_SYS_TRAP(sysTrapSysSemaphoreSignal)

#define	_SysSemaphoreSet		\
				ASM_SYS_TRAP(sysTrapSysSemaphoreSet)

#define	_SysDoze		\
				ASM_SYS_TRAP(sysTrapSysDoze)


#endif  //__SYSTEMMGR_H__
