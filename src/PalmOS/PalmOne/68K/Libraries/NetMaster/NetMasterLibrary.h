/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @defgroup NETMASTER NetMaster Library
 *
 * @{
 * @}
 */

/**
 * @file 	NetMasterLibrary.h
 * @version 1.0
 * @date 	01/31/2002
 *
 * @brief This is the main header file for the NetMaster Library.
 *
 * Usage Model
 *
 * The NetMaster library is preloaded by the system software before
 * the system sends sysAppLaunchCmdSystemReset.
 *
 * Clients of NetMaster library need to call SysLibFind (netMasterLibName,...)
 * to get a library refNum of NetMaster.
 *
 * If SysLibFind returns a non-zero error, clients <b>MUST</b> assume that
 * NetMaster was not loaded for a <b>good</b> reason
 * (such as when the user performed a "safe" reset), or is
 * simply not present on the system and fail gracefully.
 *
 * Clients <b>MUST NOT</b> load NetMaster themselves (such as via SysLibLoad or
 * SysLibInstall) -- keep in mind that if the system didn't load it,
 * there was a good reason for it!
 *
 */

#ifndef _NET_MASTER_LIBRARY_H_
#define _NET_MASTER_LIBRARY_H_


#include <Common/Libraries/NetMaster/NetMasterLibTarget.h>
#include <Common/Libraries/NetMaster/NetMasterLibTraps.h>
#include <Common/Libraries/NetMaster/NetMasterLibErrors.h>
#include <68K/Libraries/NetMaster/NetMasterLibTypes.h>



#ifdef BUILDING_NET_MASTER_LIB_DISPATCH_TABLE	// defined in the dispatch table
									 			// module when building the NetMaster
									 			// library itself
  #define NETMASTER_LIB_TRAP(trapNum)

#else
  #define NETMASTER_LIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif



#ifdef __cplusplus
extern "C" {
#endif


/// Get Library API Version number.
///
///  @param		refNum:		IN:  Library reference number
///  @param		majorVerP:	OUT: Major version number
///  @param		minorVerP:	OUT: Minor version number
///  @param 	bugFixVerP:	OUT: Bug fix version number
///  @retval	Err			NetMaster Library error code

extern Err
NetMasterLibVersionGet (UInt16 refNum, UInt32* majorVerP, UInt32* minorVerP,
						UInt32* bugFixVerP)
	NETMASTER_LIB_TRAP (netMasterLibTrapLibVersionGet);


/// An extension, of sorts, of the NetLibOpen function -- allows the
/// caller to specify special options, such as "auto-dismiss on error".
/// See the definition of NetMasterNetLibOpenOptionsType for more details.
///
/// The refNum parameter must be the refnum of NetMaster library
/// (not of NetLib).
///
/// IMPORTANT: This call is semantically equivalent to calling NetLibOpen;
/// if this call succeeds (returns 0 error code), the caller is
/// responsible for calling NetLibClose
/// when done with NetLib; if this call fails (non-zero error code
/// returned), the caller must *NOT* make the matching call to NetLibClose.
///
/// @param	refNum:				IN:  NetMasterLib reference number (from SysLibFind()).
/// @param	netLibOpenOptions:	IN:  See NetMasterNetLibOpenOptionsType.
/// @param	netIFErrsP			OUT: Error returned by the net interface if open failed.
/// @retval Err					Error code.
extern Err
NetMasterNetLibOpenWithOptions (UInt16 refNum,
							NetMasterNetLibOpenOptionsType netLibOpenOptions,
							UInt16* netIFErrsP)
	NETMASTER_LIB_TRAP (netMasterLibTrapNetLibOpenWithOptions);


/// Get the "auto-login" settings
///
/// @param	refNum:		IN:  NetMasterLib reference number (from SysLibFind).
/// @param	settingsP:	OUT: Any of the NetMasterAutoLoginType constants bitwise-or’ed together.
/// @retval	Err			Error code.
extern Err
NetMasterAutoLoginSettingGet (UInt16 refNum,
							  NetMasterAutoLoginType* settingsP)
	NETMASTER_LIB_TRAP (netMasterLibTrapAutoLoginSettingGet);


/// NetMasterAutoLoginSettingSet:
///
/// set the "auto-login" settings
///
/// @param	refNum:			IN:  NetMasterLib reference number.
/// @param	flagsToClear:	IN:  Flags to clear -- any of the NetMasterAutoLoginType
///					  	         constants bitwise-or'ed together or
///					  	         netMasterAutoLoginClearAll to clear all flags; the
///					  	         flag clearing operation is performed before the
///					  	         flag setting operation. Pass 0 to skip the clearing
///					  	         operation.
/// @param	flagsToSet:		IN:  Flags to set -- any of the NetMasterAutoLoginType
///					  	         constants bitwise-or'ed together. Pass 0 to skip the
///					  	         setting operation.
/// @retval	Err				Error code.
extern Err
NetMasterAutoLoginSettingSet (UInt16 refNum,
							  NetMasterAutoLoginType flagsToClear,
							  NetMasterAutoLoginType flagsToSet)
	NETMASTER_LIB_TRAP (netMasterLibTrapAutoLoginSettingSet);


/// Function:	NetMasterNetLibOpenIfFullyUp
///
/// Summary:	Open NetLib only if all of its current interfaces are already
///				fully up.  If this call succeeds (returns true), it is
///				semantically equivalent to a successful NetLibOpen call, which
///				implies that the caller is responsible for calling NetLibClose
///				to match this call when done with NetLib.  If
///				NetMasterNetLibOpenIfFullyUp fails (returns false),
///				the caller *MUST NOT* make a matching call to NetLibClose.
///
/// @param	refNum:			IN:  NetMasterLib reference number (from SysLibFind
///						         or SysLibLoad)
/// @param	netLibRefNumP:	OUT: NetLib refnum on success, undefined on failure
/// @retval	Boolean			True if opened, false if not (failure)
extern Boolean
NetMasterNetLibOpenIfFullyUp (UInt16 refNum, UInt16* netLibRefNumP)
	NETMASTER_LIB_TRAP (netMasterLibTrapNetLibOpenIfFullyUp);


/// Function:	NetMasterNetLibIsFullyUp
///
/// Summary:	Checks if NetLib is fully up -- i.e. all of its interfaces
///				are up.
///
/// @param	refNum:	IN:  NetMasterLib reference number (from SysLibFind
/// @retval	Boolean	True if NetLib is fully up, false if at least one interface is down.
extern Boolean
NetMasterNetLibIsFullyUp (UInt16 refNum)
	NETMASTER_LIB_TRAP (netMasterLibTrapNetLibIsFullyUp);


/// Shut down NetLib's interfaces, if they are presently up; does nothing
/// if they are presently down; blocking;  If NetLib is in the process
/// of logging on to the IP network (Connection progress dialog is up),
/// will simulate the press of the Cancel key and return without waiting
/// for interfaces to shut down.
///
/// WARNING: will dead-lock if called from the context of a Notification
/// Manager notification.  This is because Notification Manager erroneously
/// grabs the Memory Manager semaphore before sending a notification, and
/// this causes a dead-lock when the TCP/IP stack's background task calls
/// into Memory Manager or Data Manager API as part of the shut-down.
///
/// Shut down NetLib’s interfaces, if they are presently up; does nothing if they are
/// presently down.
///
/// @param	refNum:				IN: NetMasterLib reference number (from SysLibFind)
/// @param	reasonErrCode:		IN: Error code that is the reason for this call to
///                         	    NetMasterNetInterfacesShutDown; 0 if not an error, or reason is unknown.
///                         	    Future version of this API may use the error code to display an alert or log a debug trace.
/// @param	shutDownOptions:	IN: Shutdown options. See NetMasterNetIFShutDownOptionsType.
/// @retval	Err Error code.
extern Err
NetMasterNetInterfacesShutDown (UInt16 refNum, Err reasonErrCode,
							NetMasterNetIFShutDownOptionsType shutDownOptions)
	NETMASTER_LIB_TRAP (netMasterLibTrapNetInterfacesShutDown);


/// Test if NetLib is in the Logging On state.
/// Returns true if some client is presently inside NetLibOpen or
/// NetLibOpenConfig.
///
/// @param	refNum:					IN:  NetMaster library reference number.
/// @param	isLogonActiveP:			OUT: True if NetLib is in the middle of
///  				   					 logging in and an error has not yet been encountered.
///  				   					 This parameter is OPTIONAL -- pass NULL to ignore.
/// @param	isPrgDialogUpP:			OUT: If NetLib's Service Connection Progress
///  				   					 dialog is up, NetMasterIsLoggingIn will set
///  				   					 *isPrgDialogUpP to true; false otherwise.
///  				   					 This parameter is OPTIONAL -- pass NULL to ignore.
/// @param	waitingInErrorStateP:	OUT: If NetLib's Service Connection Prorgress
///  								     dialog is in the error acknowledegement state
///  				   					 (displaying an error message to the user,
///  				   					 waiting for OK), NetMasterIsLoggingIn will set
///  				   					 *waitingInErrorStateP to true; false otherwise.
///  				   					 This parameter is OPTIONAL -- pass NULL to ignore.
/// @retval Boolean					True if some client is presently inside NetLibOpen or NetLibOpenConfig.
extern Boolean
NetMasterNetLibIsLoggingIn (UInt16 refNum, Boolean* isLogonActiveP,
							Boolean* isPrgDialogUpP,
							Boolean* waitingInErrorStateP)
	NETMASTER_LIB_TRAP (netMasterLibTrapNetLibIsLoggingIn);



//-------------------------------------------------------------------------
// NetMaster Data Session Control API
//-------------------------------------------------------------------------

#if 0
#pragma mark -- Data Session Control API --
#endif


/*

		Usage Model for Data Session Control API



Background:

Palm OS 5.2 does *not* provide a robust model for arbitrating the
activation (loogging in) of different network service profiles
between competing tasks.  For example, the MMS application,
running in the background, may receive a trigger and need to log
in with the MMS APN (network profile), while the Browser app
has NetLib open with the Internet or browser-specific APN.
NetLib does not provide a way to arbitrate between the needs
of both apps, and this version of NetLib and/or the initial
hardware that runs it do not support multiple concurrent data
sessions over the same Network Interface (PPP, for example). This
API attempts to bridge the gap somewhat by keeping track of the
current NetLib usage by various tasks, and trying to make
intelligent decisions about when a request to activate a
particular network profile may be allowed to shut down another
previously active profile.  This API also attempts to simplify
and make more robust the process of logging in with a network
profile that is not the "default" network profile. Before this
API, applications had to get the current default network profile
ID, set a new default, call NetLibOpen, do their networking, then
restore the default ID. This worked somewhat in a model where
only one app runs and performs data networking at a time.
However, multiple concurrent clients, possibly running on
different tasks, open lots of room for the multiple writer
problem as well as the inability to restore the default profile
ID should a crash or soft-reset occur during the application's
execution.


Disclaimer:

This API is *not* for everyone.  It is intended to be a
temporary solution to a (hopefully) temprorary limitation in
Palm OS's Network library.  Future versions of Palm OS or NetLib
may make it impossible or unnecessary for this API to be
supported, and it would then be deprecated.  Most apps that
conntect using the "default" network profile (as selected in
the Network Panel) should continue to use the standard NetLib
API (NetLibOpen and NetLibClose) in order to remain compatible
with future releases of Palm OS.


Usage Model:

1. The NetMaster Data Session Control API *MUST NOT* be
intermixed with NetLib's "session control" API, which includes
NetLibOpen, NetLibOpenConfig, NetLibOpenIfCloseWait, NetLibClose,
NetLibConnectionRefresh, NetLibIFUp, NetLibIFDown, NetLibIFAttach,
and NetLibIFDetach.  You MUST either use one set of session
control API or the other, exclusively.

2. The NetMaster Data Session Control API works closely with the
Handspring NetPref library.  Applications first look up the
desired network profile record using the NetPref library API,
then provide this record's ID to NetMasterSCNetAttach via the
netPrefRecID parameter.  This API only works with network
profiles that are managed by NetPref library.  Other profiles
are NOT supported.

3. The NetMaster Data Session Control API is centered around
the concepts of data sessions and data contexts.  A given data
session represents an established (logged-in) data session via
a specific network profile ID.  An "active" data session is one
that is believed to be logged in; otherwise it is "inactive"
A given data context represents a client (an executable such as
an application or library) that successfully attached to a data
session by calling NetMasterSCNetAttach.  Multiple data contexts
may be "attached" to a single data session. The Data Session
Control API provides functions that operation on both Data
Contexts and Data Sessions.  The functions that most clients
utilizing this API will use are NetMasterSCNetAttach,
NetMasterSCNetDetach, NetMasterSCSessionIDGetFromContext, and
NetMasterSCSessionIsActive. The remaining functions are mainly
for troubleshooting purposes.

4. A client begins networking over a particular network profile
by calling NetMasterSCNetAttach() with the desired network
profile ID and other specialized parameters. The client uses
NetMasterSCNetAttach instead of NetLib's NetLibOpen or
NetLibOpenConfig. If there is already an active
data session where the requested network profile ID was
either the primary or fallback profile, NetMasterSCNetAttach
"attaches" the caller to that data session. *See NetPref API
documenation for definition of primary and fallback profiles. If,
on the other hand, the active data session was established from
a different network profile, NetMasterSCNetAttach employs an
internal algorithm to see if it can preempt (shut down) the
active data session, and do so if it can. If it cannot make room
for a new data session by tearing down an existing data session,
the function fails.  If there was no active data session or if
one was successfully shut down to make room for the new data
session, NetMasterSCNetAttach will attempt to establish a new
data session with the requested network profile, and attach the
client to that data session.  On success, NetMasterSCNetAttach
returns an error code of 0 and a non-zero Context ID of a newly
created data context that is attached to the data session. The
returned Context ID may be passed to other functions of this API
family that require a data context ID.  This data context ID is
valid only until you destroy it by calling NetMasterSCNetDetach.
Once the Context ID is destroyed, it *MUST NOT* be passed to any
functions.  If NetMasterSCNetAttach fails, you may need to retry
at a later time.  The error code netErrAlreadyOpenWithOtherConfig
indicates that NetMasterSCNetAttach could not preempt another
active data session.  Other non-zero error codes are typically
either NetLib or NetMaster library error codes.

When the client is finished with using the network session that
was acquired with NetMasterSCNetAttach, such as when exiting the
networking application or terminating its task, etc., the client
MUST destroy the data context by calling NetMasterSCNetDetach.
User's of the NetMaster Data Session Control API call
NetMasterSCNetDetach instead of NetLibClose.

5. Once attached to a data session, the client may periodically
call NetMasterSCSessionIsActive to check if that session is
still logged in.  NetMasterSCSessionIsActive takes a data session
ID as a parameter.  Call NetMasterSCSessionIDGetFromContext
to get the data session ID from a valid data context ID.  If it
reports that the data session is *not* active, this means that
the data session was shut down for some reason, such as loss of
network coverage for an extended time, wireless mode being turned
off, preemted by another client requesting a different network
profile, etc.  When this occurs, you should destroy the data
context by calling NetMasterSCNetDetach, since this API has
*no* concept of data session re-activation. If you need to resume
your data session, you need to get a new data context via
NetMasterSCNetAttach, discussed above.

6. If the application is in a state where
the anchor timeout that it initially requested when
calling NetMasterSCNetAttach is now bigger
than necessary, the application MUST reduce its anchor
timeout to the minimum acceptable value by calling
NetMasterSCContextAnchorTimeoutSet so that other services
may be activated more quickly when necessary.


7. The remaining NetMaster Data Session Control API's are
intended for troubleshooting purposes and not described in detail
yet.

*/



// IMPORTANT: NetMasterSCNetAttach/NetMasterSCNetDetach are implemented under
// Palm OS 5.x only and not available before Palm OS 5.0.  These functions
// may be used from UI and background tasks. These functions semantically
// replace NetLibOpen/NetLibClose.
//
// Attach to the network using the specified service; this will call
//  NetLibOpen and execute any required fallback services, as necessary
//  on the caller's behalf.
//
// loginPriority		IN	Login priority -- most applications should use
//							 the netMasterLoginPriorityDefault priority.
//
// anchorTimeoutSec		IN	Initial data session anchor timeout request.
// 							 The system may limit this request if it is bigger
// 							 than maximum allowed for the given device
// 							 configuration.  This value is used by the system
// 							 to decide when a data session may be considered
// 							 idle, and may be shut down to make room for
// 							 a different login request. This value is treated
// 							 as a hint only, and the system makes no
// 							 guarantees about the longevity of the data
// 							 session. The duration is in seconds, which may
// 							 be 0.  For system default, pass
// 							 netMasterAnchorTimeoutDefault (RECOMMENDED!)
//
// isNetIFErrorP		OUT	[OPTIONAL] when non zero error code is returned,
//							 this boolean indicates whether the error
//							 was from the network interface. For
//							 foreground-initiated logins, the
//							 implication is that if it *is* an
//							 error from the network interface,
//							 an error message was most likely
//							 already displayed to the user via the
//							 standard progress dialog UI. If
//							 the error is *not* from the network
//							 interface, no error has been displayed
//							 to the user. Pass NULL to ignore this
//							 parameter..
//
// IMPORTANT: if this call succeeds (returns 0
//  error code), the caller is responsible for calling NetMasterSCNetDetach
//  when done with this data context; if this call fails (non-zero error code
//  returned), the caller must *NOT* make the matching call to
//  NetMasterSCNetDetach.


/// Attach to the network using the specified service; this will call NetLibOpen and
/// execute any required fallback services, as necessary on the caller’s behalf.
///
/// @param refNum:		 		IN:  NetMasterLib reference number (from SysLibFind).
/// @param netPrefRecID:		IN:  NetPref Record ID of the network profile to attach.
/// @param netLibOpenOptions:	IN:  Special options, such as “auto-dismiss on error”; 0 = no options.
/// @param loginPriority:		IN:  Login priority. Most applications should use the netMasterLoginPriorityDefault priority.
/// @param anchorTimeoutSec:	IN:  It is bigger than maximum allowed for the given device configuration. This
///                        		     value is used by the system to decide when a data session may be considered
///                        		     idle, and may be shut down to make room for a different login request. This
///                        		     value is treated as a hint only, and the system makes no guarantees about the
///                        		     longevity of the data session. The duration is in seconds, which may be 0. For
///                        		     system default, pass netMasterAnchorTimeoutDefault (RECOMMENDED!)
/// @param dataCxtIDP:			OUT: Data context on success, undefined on failure.
/// @param isNetIFErrorP:		OUT: When non zero error code is returned, this boolean indicates
///                           		 whether the error was from the network interface. For foreground-initiated
///                           		 logins, the implication is that if it *is* an error from the network interface, an
///                           		 error message was most likely already displayed to the user via the standard
///                           		 progress dialog UI. If the error is *not* from the network interface, no error has
///                           		 been displayed to the user. Pass NULL to ignore this parameter.
/// @retval Err					Error code.
extern Err
NetMasterSCNetAttach (UInt16 refNum, UInt32 netPrefRecID,
					NetMasterNetLibOpenOptionsType netLibOpenOptions,
					NetMasterLoginPriorityEnum loginPriority,
					Int32 anchorTimeoutSec,
					NetMasterDataContextIDType* dataCxtIDP,
					Boolean* isNetIFErrorP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCNetAttach);

/// Detach from the data session that was successfully established by
/// NetMasterSCNetAttach.
///
/// @param	refNum:		 IN: NetMasterLib reference number (from SysLibFind).
/// @param	dataCxtID:	 IN: Data context created by NetMasterSCNetAttach.
/// @retval Err			 Error code.
extern Err
NetMasterSCNetDetach (UInt16 refNum,
					NetMasterDataContextIDType dataCxtID)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCNetDetach);


/// Changes the data context's session anchor timeout value.
/// The requested anchor timeout value is in seconds. The system
/// will apply internal limits to the value if necessary. The
/// actual value is returned via the optional output arugment
/// oActualAnchorTimeoutSecP (pass NULL to ignore).
///
/// IMPORTANT:
/// If the application is in a state where
/// the anchor timeout that it initially requested when
/// calling NetMasterSCNetAttach is now bigger
/// than necessary, the application MUST reduce its anchor
/// timeout to the minimum acceptable value so that other services
/// may be activated more quickly when necessary.
///
/// @param refNum:						IN:  NetMasterLib reference number (from SysLibFind).
/// @param dataCxtID:					IN:  Data context created by NetMasterSCNetAttach.
/// @param requestedAnchorTimeoutSec:	IN:  Requested anchor timeout in seconds.
/// @param oActualAnchorTimeoutSecP:	OUT: Actual anchor timeout that was applied (pass NULL to ignore).
///                                      	 Undefined on error.
/// @retval Err							Error code.
extern Err
NetMasterSCContextAnchorTimeoutSet (UInt16 refNum,
									NetMasterDataContextIDType dataCxtID,
									Int32 requestedAnchorTimeoutSec,
									Int32* oActualAnchorTimeoutSecP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCContextAnchorTimeoutSet);



/// Given a data context, get the associated data session ID.  0 or more
/// data contexts may be associated with a data session.  A data session
/// represents a single network login instance.  i.e., if you had two
/// concurrent network login instances, such as GPRS and 802.11 for example,
/// there would be two network data sessions.
///
/// @param refNum:		IN:	 NetMasterLib reference number (from SysLibFind).
/// @param dataCxtID:	IN:	 Data context created by NetMasterSCNetAttach.
/// @param sessionP:	OUT: Data session ID on success, undefined on failure.
/// @retval Err			Error code.
extern Err
NetMasterSCSessionIDGetFromContext (UInt16 refNum,
								   NetMasterDataContextIDType dataCxtID,
								   NetMasterDataSessionIDType* sessionP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionIDGetFromContext);


/// Get the current count of network data sessions that are active (believed
/// to be logged in).
///
/// @param refNum:				IN:  NetMasterLib reference number (from SysLibFind).
/// @param activeSessionCountP:	OUT: Count of active data sessions.
/// @retval Err					Error code.
extern Err
NetMasterSCSessionCountGet (UInt16 refNum, Int16* activeSessionCountP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionCountGet);


/// Fill an array with session ID's of the currently active data sessions.
/// Initially, only one active data session may be supported, but this may
/// increase with the hardware and system software capabilities.
///
/// @param refNum:			IN:  NetMasterLib reference number (from SysLibFind).
/// @param sessionIDs:		OUT: Array for returning session IDs on success, contents undefined on failure.
/// @param maxEntries:		IN:  Max # of session ID’s that may be stored in the caller’s array sessionIDs.
/// @param entriesStoredP:	OUT: On success, number of entries that were actually returned in the sessionIDs
///                              array, which may be less than maxEntries (possibly 0); undefined on failure.
/// @param Err				Error code.
extern Err
NetMasterSCSessionsEnumerate (UInt16 refNum,
							  NetMasterDataSessionIDType sessionIDs[],
							  Int16 maxEntries, Int16* entriesStoredP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionsEnumerate);


/// Retrieves the unique ID's of the actual and primary NetPref records
/// that were used to establish the data session. The two may be different if
/// the primary record had a fallback, the primary failed, and the fallback
/// record succeeded.  In other words, *actualNetPrefIDP indicates the NetPref
/// record ID that succeeded in establishing the connection.
///
/// @param refNum:				IN:  NetMasterLib reference number (from SysLibFind).
/// @param sessionID:			IN:  Data session ID.
/// @param primaryNetPrefIDP:	OUT: NetPref record ID of the primary record used in connection establishment.
/// @param actualNetPrefIDP:	OUT: NetPref record ID of the actual record with which connection establishment
///                              	 succeeded. This may be the same as the primary record or a fallback of the
///                              	 primary record.
/// @retval Err					Error code.
extern Err
NetMasterSCSessionSvcRecIDGet (UInt16 refNum,
							 NetMasterDataSessionIDType sessionID,
							 UInt32* primaryNetPrefIDP,
							 UInt32* actualNetPrefIDP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionSvcRecIDGet);


/// Checks if the given session is still on the active list (believed
/// to be logged in).
///
/// @param refNum:		IN:  NetMasterLib reference number (from SysLibFind).
/// @param sessionID:	IN:  Data session ID.
/// @param isActiveP:	OUT: True if the given session is believed to be logged in, false if it’s not logged in.
/// @retval Err			Error code.
extern Err
NetMasterSCSessionIsActive (UInt16 refNum,
							NetMasterDataSessionIDType sessionID,
							Boolean* isActiveP)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionIsActive);


/// Request shut down of the given network session.  This
/// function may block for a significant period of time (typically
/// multiple seconds) while the network interfaces are being brought
/// down.
///
/// @param refNum:			IN: NetMasterLib reference number (from SysLibFind).
/// @param sessionID:		IN: data session ID.
/// @param shutDownOptions:	IN: shutdown options. See NetMasterNetIFShutDownOptionsType.
/// @retval Err				Error code.
extern Err
NetMasterSCSessionShutDown (UInt16 refNum,
							NetMasterDataSessionIDType sessionID,
							NetMasterNetIFShutDownOptionsType shutDownOptions)
	NETMASTER_LIB_TRAP (netMasterLibTrapSCSessionShutDown);



#ifdef __cplusplus
}
#endif


#endif // _NET_MASTER_LIBRARY_H_
