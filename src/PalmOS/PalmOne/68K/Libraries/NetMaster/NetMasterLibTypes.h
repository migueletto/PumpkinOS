/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup NETMASTER
 */

/**
 * @file 	NetMasterLibTypes.h
 * @version 1.0
 * @date 	01/31/2002
 *
 * @brief This file contains public data types exported by the NetMaster Library
 *
 * For usage model: @see NetMasterLibrary.h
 */


#ifndef _NET_MASTER_LIB_TYPES_H_
#define _NET_MASTER_LIB_TYPES_H_



/** Opaque structures */
struct NetMasterContextTypeTag;


/**
 * @name NetMaster API Version Number
 * @brief Current library API version number returned by NetMasterLibVersionGet()
 */
/*@{*/
#define netMasterLibVersionMajor	  0
#define	netMasterLibVersionMinor	  4
#define netMasterLibVersionBugFix	  0
/*@}*/

// Change details:
// 0.1.0 -- original
// 0.2.0: implemented original VPN override notification
// 0.3.0: implemented NetMasterNetLibOpenWithOptions ().
// 0.4.0: implemented initial Palm OS 5.x support, including
//		   initial Data Session Control API.


/** TCP/IP Network Auto-login definitions used by NetMasterAutoLoginSettingGet()
 *  and NetMasterAutoLoginSettingSet()
 */
typedef UInt32	NetMasterAutoLoginType;
enum
  {
	/**
	 *  For CDMA 1xRTT: Automatically log in to the TCP/IP network when the
	 *  wireless mode is turned on and network is acquired;
	 *  For GSM GPRS: Attempt to attach when the radio is powered on (ideally
	 *  a combined attach)
	 */
	// NOTE: Used by Handspring's Network Panel
	netMasterAutoLoginWhenWirelessOn	= 0x00000001UL,

	/**
	 * Prompt the user before connecting to the Internet;
	 * Also known as the "NetGuard" feature.
	 */
	// NOTE: Used by Handspring's Network Panel
	netMasterLoginNetGuardEnabled		= 0x00000002UL,

	netMasterAutoLoginClearAll			= 0xFFFFFFFFUL
  };


/** NetLib interface shutdown options for NetMasterNetInterfacesShutDown */
typedef UInt32	NetMasterNetIFShutDownOptionsType;
enum
  {
	/** If the Progress dialog is up, dismiss it even if it is
	 *  in the error display state
	 */
	netMasterNetIFShutDownOptDismissPrgDialog	= 0x00000001UL,

	/** If set, and the session being connected at the moment has
	 *  a fallback, this will allow the fallback to be processed (for example,
	 *  when aborting GPRS logon, but allowing the fallback, if any, to
	 *  take place)
     */
	netMasterNetIFShutDownOptCurrentOnly		= 0x00000002UL,


	/** If set, will cause the cached NetGuard setting to be
	 *  cleared: apps that Disconnect the network connection as the
	 *  result of user request (such as Disconnect button or menu item),
	 *  MUST set this flag -- this causes the system to put up the
	 *  NetGuard prompt during a subsequent data network login,
	 *  if dictated by the netMasterLoginNetGuardEnabled setting.
	 *
	 *  If *not* set, will cause the cached NetGuard setting to
	 *  be preserved.
	 */
	netMasterNetIFShutDownOptResetNetGuard		= 0x00000004UL


  };


// ------------------------------------------------------------------
// Client Notification Events: (netMasterClientEventNotificationTypeID)
//
// Parameter block and supporting data structures sent to the currently
// registered NetMaster library client(s) via Palm OS Notification Manager.
// ------------------------------------------------------------------

/** The application that wishes to receive these notifications registers
 *  via Palm OS Notification Manager's SysNotifyRegister() for the event type
 *  netMasterClientEventNotificationTypeID to receive the events listed in
 *  NetMasterClientEventEnum. See Palm OS Notification Manager
 *  documentation for registration and notification handling details.
 */
// (Handspring registered 'HICe' with Palm Source on 08-Oct-2002 at 8:23pm PST)
#define netMasterClientEventNotificationTypeID	  'HnMe'


/** NetMaster library client event selector */
typedef UInt16	  NetMasterClientEventEnum;
enum
  {
	netMasterClientEventUnknown		= 0,	// reserve 0

	/** Queries the VPN vendor to install its own connection profile name,
	 *  if any; NetMaster library broadcasts this event to all registered
	 *  clients from the context of the UI task when a client application
	 *  calls NetLibOpen() or NetLibOpenConfig().
	 *  Parameter block:
	 *  NetMasterClientEventInfoType:NetMasterClientEventVPNConnInstallType
	 */
	netMasterClientEventVPNConnInstall,




	netMasterClientEventLAST
  };



/**
 * @brief Data block for netMasterClientEventVPNConnInstall
 */
typedef struct
  {
	// --------------------------------------------------------------
	// Inputs
	// --------------------------------------------------------------

	/** Name of the connection that NetLib will use unless the VPN
	 *  client overrides it by setting vpnConnProfileOverrideName
	 *  to a non-empty connection name string.  Watch out for the case
	 *  where the user inadvertently selected the VPN connection name
	 *  itself as the connection name for the network profile. VPN vendors
	 *  that choose to override this connection name, will likely wish
	 *  to save this connection name so that their destination connection.
	 */
	Char					defaultConnProfileName[cncProfileNameSize];

	UInt32					inReserved1;  /**< set to zero by this lib */
	UInt32					inReserved2;  /**< set to zero by this lib */


	// --------------------------------------------------------------
	// Outputs -- modify only if you wish to override the default
	//            connection name!
	// --------------------------------------------------------------

	/** The VPN vendor may override the default connection name that NetLib
	 *  will use by setting this field to a non-empty string -- this string
	 *  MUST be a valid connection name that presently exists in the connection
	 *  panel!
	 *
	 *  If the VPN vendor presently does *not* wish to override the default
	 *  connection (based on user settings in the VPN vendor's configuration
	 *  UI), the VPN vendor should leave this field alone (don't modify
	 *  it in any way in this case to avoid conflicts with other VPN vendor
	 *  that may be currently installed on the same device).
	 */
	Char					vpnConnProfileOverrideName[cncProfileNameSize];
  }
NetMasterClientEventVPNConnInstallType;


/** Increment this version # when adding fields to NetMasterClientEventInfoType,
 *  except when just adding a new event data pointer to eventData
 */
#define netMasterClientCurrentEventVersion		1

/**
 * @brief This structure is passed as the 'detailsP' of the notification.
 * @see Palm OS Notification Manager
 */
typedef struct
  {
	// --------------------------------------------------------------
	// Inputs
	// --------------------------------------------------------------

	/** Version of this structure -- all future versions MUST define
	  * backward compatible Event structures.
	  * NetMaster Library sets this to netMasterClientCurrentEventVersion
	  * at the time the library was built. */
	UInt16						version;

	/** Event ID */
	NetMasterClientEventEnum	eventID;

	/** Data corresponding to the event ID */
	union
	  {
		/** genericEventDataP is for the convenience of the internal
		  * code only. */
		void*									genericEventDataP;

		/** For netMasterClientEventVPNConnInstall */
		NetMasterClientEventVPNConnInstallType*	vpnConnInstallP;

	  }
	eventData;

	UInt32							inReserved1; 	/**< Reserved values; Set to 0 by NetMaster library */
	UInt32							inReserved2;	/**< Reserved values; Set to 0 by NetMaster library */
	UInt32							inReserved3;	/**< Reserved values; Set to 0 by NetMaster library */
	UInt32							inReserved4;	/**< Reserved values; Set to 0 by NetMaster library */


	// --------------------------------------------------------------
	// Outputs - none for now
	// --------------------------------------------------------------
  }
NetMasterClientEventInfoType;


/** Special NetLibOpenConfig options passed to
 *  NetMasterNetLibOpenWithOptions()
 */
typedef UInt32	  NetMasterNetLibOpenOptionsType;
enum
  {
	/** netMasterNetLibOpenOptAutoDismiss:
	 *
	 *  If set, auto-dismiss the progress dialog if error occurs
	 */
	netMasterNetLibOpenOptAutoDismiss			= 0x00000001UL,


	/** netMasterNetLibOpenOptBypassNetGuard:
	 *
	 *  If set, bypass the NetGuard feature -- for use by IOTA
	 *  service only!!!
	 *
	 *  IMPORTANT: *not* available on systems before Palm OS 5.0
	 */
	//
	// DOLATER \todo -- should this be in "protected" incs?
	netMasterNetLibOpenOptBypassNetGuard		= 0x00000002UL,

	/** If this flag is set by the client, then the connection will
	 *  be attempted only with the requested NetPref record, and the
	 *  fallback network profile, if any, will be ignored.
	 *
	 *  IMPORTANT: *not* available on systems before Palm OS 5.0
	 */
	netMasterNetLibOpenOptPrimaryProfileOnly	= 0x00000004UL

  };


/** Login priority values that may be passed to NetMasterSCNetAttach */
typedef UInt32	  NetMasterLoginPriorityEnum;
enum
  {
	/** The vast majority of background and foreground applications SHOULD
	 *  ALWAYS use the Default login priority.  This allows Data Session
	 *  Control system software to optimize end-user experience
	 *  on the device by following specific data session
	 *  management rules requested by network operators.
	 */
	netMasterLoginPriorityDefault			= 0,

	/** The Low login priority is the *least* disruptive to existing data
	 *  sessions that were established with a different network service
	 *  profile than the one that you're requesting.
	 */
	netMasterLoginPriorityLow				= 1,

	/** The High login priority is the *most* disruptive to existing data
	 *  sessions that were established with a different network service
	 *  profile than the one that you're requesting.  This will typically
	 *  cause even active foreground data sessions to be torn down. This
	 *  priority is reserved for *internal use only*, and should be avoided
	 *  by other developers as its usage will typically break the data
	 *  session management model of the device, resulting in poor end-user
	 *  experience.
	 */
	netMasterLoginPriorityHigh				= 2

  };



/** Default value to pass as the anchorTimeoutSec parameter to NetMasterSCNetAttach */
#define netMasterAnchorTimeoutDefault	  (-1)


// ----------------------------------------------------------------
/** Data Context type used by NetMasterSCNetAttach,
 *  NetMasterSCNetDetach and friends.
 */
// ----------------------------------------------------------------
typedef UInt32	  NetMasterDataContextIDType;

// ----------------------------------------------------------------
/** Data Session type used by NetMasterSCSessionIDGetFromContext
 *  and friends.
 */
// ----------------------------------------------------------------
typedef UInt32	  NetMasterDataSessionIDType;


#endif // _NET_MASTER_LIB_TYPES_H_
