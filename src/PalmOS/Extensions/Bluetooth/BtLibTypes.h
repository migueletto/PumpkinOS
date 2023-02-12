/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: BtLibTypes.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for BtLib
 *
 *****************************************************************************/

#ifndef __BTLIBTYPES_H
#define __BTLIBTYPES_H

#include <PalmTypes.h>
#include <LibTraps.h>

  #pragma mark *---------General--------------*
/********************************************************************
 * General
 ********************************************************************/

//Library Name
#define btLibName					"Bluetooth Library"
 
// Traps
#define btLibTrapHandleEvent										(sysLibTrapCustom)
#define btLibTrapRegisterManagementNotification				(sysLibTrapCustom+1)
#define btLibTrapUnregisterManagementNotification			(sysLibTrapCustom+2)
#define btLibTrapStartInquiry										(sysLibTrapCustom+3)
#define btLibTrapCancelInquiry									(sysLibTrapCustom+4)
#define btLibTrapDiscoverSingleDevice							(sysLibTrapCustom+5)
#define btLibTrapDiscoverMultipleDevices						(sysLibTrapCustom+6)
#define btLibTrapGetSelectedDevices								(sysLibTrapCustom+7)
#define btLibTrapGetRemoteDeviceName							(sysLibTrapCustom+8)
#define btLibTrapLinkConnect										(sysLibTrapCustom+9)
#define btLibTrapLinkDisconnect									(sysLibTrapCustom+10)
#define btLibTrapLinkSetState										(sysLibTrapCustom+11)
#define btLibTrapLinkGetState										(sysLibTrapCustom+12)
#define btLibTrapSetGeneralPreference							(sysLibTrapCustom+13)
#define btLibTrapGetGeneralPreference							(sysLibTrapCustom+14)
#define btLibTrapSocketCreate										(sysLibTrapCustom+15)
#define btLibTrapSocketRespondToConnection					(sysLibTrapCustom+16)
#define btLibTrapSocketClose										(sysLibTrapCustom+17)
#define btLibTrapSocketListen										(sysLibTrapCustom+18)
#define btLibTrapSocketConnect									(sysLibTrapCustom+19)
#define btLibTrapSocketSend										(sysLibTrapCustom+20)
#define btLibTrapSocketAdvanceCredit							(sysLibTrapCustom+21)
#define btLibTrapSocketGetInfo									(sysLibTrapCustom+22)
#define btLibTrapSdpServiceRecordsGetByServiceClass		(sysLibTrapCustom+23)
#define btLibTrapSdpServiceRecordMapRemote					(sysLibTrapCustom+24)		
#define btLibTrapSdpServiceRecordCreate						(sysLibTrapCustom+25)
#define btLibTrapSdpServiceRecordDestroy						(sysLibTrapCustom+26)
#define btLibTrapSdpServiceRecordStartAdvertising			(sysLibTrapCustom+27)
#define btLibTrapSdpServiceRecordStopAdvertising			(sysLibTrapCustom+28)	
#define btLibTrapSdpServiceRecordSetAttributesForSocket	(sysLibTrapCustom+29)
#define btLibTrapSdpServiceRecordSetAttribute				(sysLibTrapCustom+30)
#define btLibTrapSdpServiceRecordGetAttribute				(sysLibTrapCustom+31)
#define btLibTrapSdpServiceRecordGetStringOrUrltLength	(sysLibTrapCustom+32)
#define btLibTrapSdpServiceRecordGetNumListEntries			(sysLibTrapCustom+33)
#define btLibTrapSdpServiceRecordGetNumLists					(sysLibTrapCustom+34)
#define btLibTrapSdpServiceRecordSetRawAttribute			(sysLibTrapCustom+35)
#define btLibTrapSdpServiceRecordGetRawAttribute			(sysLibTrapCustom+36)
#define btLibTrapSdpServiceRecordGetSizeOfRawAttribute	(sysLibTrapCustom+37)	
#define btLibTrapSdpCompareUuids									(sysLibTrapCustom+38)
#define btLibTrapSdpGetGetServerChannelByUuid				(sysLibTrapCustom+39)
#define btLibTrapSdpGetPsmByUuid									(sysLibTrapCustom+40)	
#define btLibTrapServiceOpen										(sysLibTrapCustom+41)
#define btLibTrapServiceClose										(sysLibTrapCustom+42)
#define btLibTrapServiceIndicateSessionStart					(sysLibTrapCustom+43)
#define btLibTrapHandleTransportEvent							(sysLibTrapCustom+44)
#define btLibTrapSecurityFindTrustedDeviceRecord			(sysLibTrapCustom+45)
#define btLibTrapSecurityRemoveTrustedDeviceRecord			(sysLibTrapCustom+46)
#define btLibTrapSecurityGetTrustedDeviceRecordInfo		(sysLibTrapCustom+47)
#define btLibTrapSecurityNumTrustedDeviceRecords			(sysLibTrapCustom+48)
#define btLibTrapAddrBtdToA										(sysLibTrapCustom+49)
#define btLibTrapAddrAToBtd										(sysLibTrapCustom+50)
#define btLibTrapSdpParseRawDataElement						(sysLibTrapCustom+51)
#define btLibTrapSdpVerifyRawDataElement						(sysLibTrapCustom+52)
#define btLibTrapPiconetCreate									(sysLibTrapCustom+55)
#define btLibTrapPiconetDestroy									(sysLibTrapCustom+56)
#define btLibTrapPiconetUnlock									(sysLibTrapCustom+57)
#define btLibTrapPiconetLock										(sysLibTrapCustom+58)
#define btLibTrapServicePlaySound								(sysLibTrapCustom+59)

// Features 
#define btLibFeatureCreator		sysFileCBtLib
#define btLibFeatureVersion		0 // feature number to get version from the 'blth' feature 


// Generic no-error and error
#define btLibErrNoError		    		(0) //the opperation completed succesfully
#define btLibErrError		    		(blthErrorClass | 0xFF)// operation could not complete, unknown error
// Bluetooth errors are		errorBase + 0x0001..0x00BF
// Stack errors are			errorBase + 0x00C0..0x00DF
// Transport errors are		errorBase + 0x00E0..0x00FE
#define btLibErrNotOpen					(blthErrorClass | 0x01)// the library is not open
#define btLibErrBluetoothOff			(blthErrorClass | 0x02)// the the user has bluetooth turned off in the prefs panel
#define btLibErrNoPrefs					(blthErrorClass | 0x03)// the preference are missing
#define btLibErrAlreadyOpen  			(blthErrorClass | 0x04)// the lib is alreasy open, not an error
#define btLibErrOutOfMemory			(blthErrorClass | 0x05)// library mem alloc failed
#define btLibErrFailed		   	 	(blthErrorClass | 0x06)// remote operation completed but failed 
#define btLibErrInProgress				(blthErrorClass | 0x07)// an operation is already in progress
#define btLibErrParamError	    		(blthErrorClass | 0x08)// a bad param is passed in
#define btLibErrTooMany					(blthErrorClass | 0x09)// the library has reached it capacity for this type of registery
#define btLibErrPending         		(blthErrorClass | 0x0A)// The operation will return results through a callback
#define btLibErrNotInProgress   		(blthErrorClass | 0x0B)// Operation not in progress
// Indicates that HCI failed to initialize which implies that the 
// Bluetooth radio is having problems or may not exist. Upon receiving
// this event the application can assume that all pending operations have
// failed. For some pending operations events will be generated with an
// appropriate error but not for all. All pending commands that used 
// memory supplied by the application will result in an event so the
// application should not modify that memory until the event has been
// received. The library must be closed and then re-opened to attempt
// reinitialization of the radio.
#define btLibErrRadioInitFailed				(blthErrorClass | 0x0C) 	
// Indicates that a fatal error has occurred with the HCI. Probably
// the radio or HCI transport has a fatal error. Upon receiving
// this event the application can assume that all pending operations have
// failed. For some pending operations events will be generated with an
// appropriate error but not for all. All pending commands that used 
// memory supplied by the application will result in an event so the
// application should not modify that memory until the event has been
// received. ME will reset the HCI. If the reset is successful then 
// btLibErrRadioInitialized event will be sent to all registered handlers. 
// The btLibErrRadioInitFailed will be sent if the reset fails.
#define btLibErrRadioFatal				(blthErrorClass | 0x0D)
// Not an Error, Indicates that the HCI initialization is successful. The ME will now
// accept commands again.
#define btLibErrRadioInitialized		(blthErrorClass | 0x0E)
// Radio was reset because the device went to sleep. The radio will be reinitialized.  Same conditions
// apply to this error code as a btLibErrRadioFatal.  
#define btLibErrRadioSleepWake			(blthErrorClass | 0x0F)
#define btLibErrNoConnection    		(blthErrorClass | 0x10)// No connection on socket
#define btLibErrAlreadyRegistered   (blthErrorClass | 0x11)// the callbcakis already registered
#define btLibErrNoAclLink       		(blthErrorClass | 0x12)// No Acl link to remote device
#define btLibErrSdpRemoteRecord		(blthErrorClass | 0x13)// Operation not valid on remote SDP record
#define btLibErrSdpAdvertised			(blthErrorClass | 0x14)// Operation not valid on advertised SDP record
#define btLibErrSdpFormat				(blthErrorClass | 0x15)// Sdp Record improperly formatted.
#define btLibErrSdpNotAdvertised		(blthErrorClass | 0x16)// Operation not valid on non advertised record
#define btLibErrSdpQueryVersion  	(blthErrorClass | 0x17)// Invalid/unsupported SDP version
#define btLibErrSdpQueryHandle		(blthErrorClass | 0x18)// Invalid service record handle
#define btLibErrSdpQuerySyntax	   (blthErrorClass | 0x19)// Invalid request syntax
#define btLibErrSdpQueryPduSize  	(blthErrorClass | 0x1A)// Invalid PDU size
#define btLibErrSdpQueryContinuation (blthErrorClass | 0x1B) // Invalid Continuation data
#define btLibErrSdpQueryResources 	(blthErrorClass | 0x1C) // Insufficient resources to satisfy request
#define btLibErrSdpQueryDisconnect 	(blthErrorClass | 0x1D) // SDP disconnected
#define btLibErrSdpInvalidResponse 	(blthErrorClass | 0x1E) // SDP response from had invalid data
#define btLibErrSdpAttributeNotSet 	(blthErrorClass | 0x1F) // Attribute is not set for record
#define btLibErrSdpMapped         	(blthErrorClass | 0x20) // Operation Invalid on mapped record or socket
#define btLibErrSocket					(blthErrorClass | 0x21)// the socket reference is invalid
#define btLibErrSocketProtocol 		(blthErrorClass | 0x22)// The socket is the wrong protocol for the operation
#define btLibErrSocketRole				(blthErrorClass | 0x23)// The socket has the wrong role (listener/connection) for the operation
#define btLibErrSocketPsmUnavailable (blthErrorClass | 0x24)// The PSM was is already in use
#define btLibErrSocketChannelUnavailable (blthErrorClass | 0x25)// The Channel was unavailable on the remtoe device
#define btLibErrSocketUserDisconnect (blthErrorClass | 0x26)
#define btLibErrCanceled 				(blthErrorClass | 0x27)// The operation was canceled
#define btLibErrBusy						(blthErrorClass | 0x28)// The resource in question is busy.

// The following errors (along with btLibErrNoError) are returned in the status block
// of the BtLibManagementEventType.  They are normally used to give a reason for
// the even.
#define btLibMeStatusUnknownHciCommand		(blthErrorClass | 0x29) // Unknown HCI Command 
#define btLibMeStatusNoConnection  			(blthErrorClass | 0x2A) // No connection 
#define btLibMeStatusHardwareFailure     	(blthErrorClass | 0x2B) // Hardware Failure 
#define btLibMeStatusPageTimeout         	(blthErrorClass | 0x2C) // Page timeout 
#define btLibMeStatusAuthenticateFailure 	(blthErrorClass | 0x2D) // Authentication failure 
#define btLibMeStatusMissingKey          	(blthErrorClass | 0x2E) // Missing key 
#define btLibMeStatusMemoryFull          	(blthErrorClass | 0x2F) // Memory full 
#define btLibMeStatusConnnectionTimeout  	(blthErrorClass | 0x30) // Connection timeout 
#define btLibMeStatusMaxConnections      	(blthErrorClass | 0x31) // Max number of connections 
#define btLibMeStatusMaxScoConnections  	(blthErrorClass | 0x32) // Max number of SCO connections to a device 
#define btLibMeStatusMaxAclConnections  	(blthErrorClass | 0x33) // Max number of ACL connections to a device
#define btLibMeStatusCommandDisallowed   	(blthErrorClass | 0x34) // Command disallowed 
#define btLibMeStatusLimitedResources     (blthErrorClass | 0x35) // Host rejected due to limited resources 
#define btLibMeStatusSecurityError       	(blthErrorClass | 0x36) // Host rejected due to security reasons 
#define btLibMeStatusPersonalDevice      	(blthErrorClass | 0x37) // Host rejected (remote is personal device) 
#define btLibMeStatusHostTimeout         	(blthErrorClass | 0x38) // Host timeout 
#define btLibMeStatusUnsupportedFeature  	(blthErrorClass | 0x39) // Unsupported feature or parameter value 
#define btLibMeStatusInvalidHciParam     	(blthErrorClass | 0x3A) // Invalid HCI command parameters 
#define btLibMeStatusUserTerminated      	(blthErrorClass | 0x3B) // Other end terminated (user) 
#define btLibMeStatusLowResources        	(blthErrorClass | 0x3C) // Other end terminated (low resources) 
#define btLibMeStatusPowerOff            	(blthErrorClass | 0x3D) // Other end terminated (about to power off) 
#define btLibMeStatusLocalTerminated     	(blthErrorClass | 0x3E) // Terminated by local host 
#define btLibMeStatusRepeatedAttempts    	(blthErrorClass | 0x3F) // Repeated attempts 
#define btLibMeStatusPairingNotAllowed  	(blthErrorClass | 0x40) // Pairing not allowed 
#define btLibMeStatusUnknownLmpPDU      	(blthErrorClass | 0x41) // Unknown LMP PDU 
#define btLibMeStatusUnsupportedRemote   	(blthErrorClass | 0x42) // Unsupported Remote Feature 
#define btLibMeStatusScoOffsetRejected    (blthErrorClass | 0x43) // SCO Offset Rejected 
#define btLibMeStatusScoIntervalRejected  (blthErrorClass | 0x44) // SCO Interval Rejected 
#define btLibMeStatusScoAirModeRejected  	(blthErrorClass | 0x45) // SCO Air Mode Rejected 
#define btLibMeStatusInvalidLmpParam     	(blthErrorClass | 0x46) // Invalid LMP Parameters 
#define btLibMeStatusUnspecifiedError     (blthErrorClass | 0x47) // Unspecified Error 
#define btLibMeStatusUnsupportedLmpParam 	(blthErrorClass | 0x48) // Unsupported LMP Parameter Value 
#define btLibMeStatusRoleChangeNotAllowed (blthErrorClass | 0x49)
#define btLibMeStatusLmpResponseTimeout 	(blthErrorClass | 0x4A)
#define btLibMeStatusLmpTransdCollision  	(blthErrorClass | 0x4B)
#define btLibMeStatusLmpPduNotAllowed  	(blthErrorClass | 0x4C)

// The following are errors that come back in the status field of an btLibSocketEventDisconnected
// or an btLibSocketEventDisconnected (if the connection fails) 
// to explain why the disconnect (or connection failure) occured
#define btLibL2DiscReasonUnknown     	 	(blthErrorClass | 0x4D)// Disconnection occurred for an unknown reason. 
#define btLibL2DiscUserRequest       		(blthErrorClass | 0x4E)// Disconnection was requested by either the local or remote device. 
#define btLibL2DiscRequestTimeout   	 	(blthErrorClass | 0x4F)// An L2CAP request timed out. 
#define btLibL2DiscLinkDisc         		(blthErrorClass | 0x50)// The underlying ACL Link was disconnected. 
#define btLibL2DiscQosViolation       		(blthErrorClass | 0x51)// The connection was terminated due to a QOS violation. 
#define btLibL2DiscSecurityBlock      		(blthErrorClass | 0x52)// The local security manager refused the connection attempt. 
#define btLibL2DiscConnPsmUnsupported		(blthErrorClass | 0x53)// The remote device does not support the requested protocol service (PSM). 
#define btLibL2DiscConnSecurityBlock  		(blthErrorClass | 0x54)// The remote device's security architecture denied the connection. 
#define btLibL2DiscConnNoResources   		(blthErrorClass | 0x55)// The remote device is out of resources. 
#define btLibL2DiscConfigUnacceptable		(blthErrorClass | 0x56)// Configuration failed due to unacceptable parameters. 
#define btLibL2DiscConfigReject       		(blthErrorClass | 0x57)// Configuration was rejected (unknown reason). 
#define btLibL2DiscConfigOptions      		(blthErrorClass | 0x58)// Configuration failed due to an unrecognized configuration option.

// The following are errors that may come back with the service shutdown notification
#define btLibServiceShutdownAppUse			(blthErrorClass | 0x59)// The app opened the stack, overriding the service
#define btLibServiceShutdownPowerCycled	(blthErrorClass | 0x5A)// The power was cycled, causing the ACL connection to drop
#define btLibServiceShutdownAclDrop			(blthErrorClass | 0x5B)// The ACL connection was dropped
#define btLibServiceShutdownTimeout			(blthErrorClass | 0x5C)// The ACL connection was dropped
#define btLibServiceShutdownDetached		(blthErrorClass | 0x5D)// The ACL connection was dropped

#define btLibErrInUseByService				(blthErrorClass | 0x5E)// The stack can not be opened by an app because it is in use by a service	

#define btLibErrNoPiconet						(blthErrorClass | 0x5F)// A piconet is required for this operation
#define btLibErrRoleChange    				(blthErrorClass | 0x60)// Could not perform M/S switch
#define btLibErrSdpNotMapped					(blthErrorClass | 0x61)// Operation not valid on unmapped socket or record
#define btLibErrAlreadyConnected				(blthErrorClass | 0x62)// Connection already in place.

#define btLibErrStackNotOpen					(blthErrorClass | 0x63)// The BtStack was not opened with the library
#define btLibErrBatteryTooLow					(blthErrorClass | 0x64)// The Battery is to low to perform the operation
#define btLibErrNotFound						(blthErrorClass | 0x65)// The requested value was not found

#define btLibNotYetSupported		(blthErrorClass | 0xEF)// dgb only, unsupported feature <do later-djk>remove


 #pragma mark *---------Management----------*
/********************************************************************
 * Management
 ********************************************************************/
#define btLibManagementEventNotificationType	'btme'
#define btLibDeviceAddressSize	6

// Represents a 48-bit Bluetooth device address.
typedef struct BtLibDeviceAddressType {
    UInt8    address[btLibDeviceAddressSize];
} BtLibDeviceAddressType, * BtLibDeviceAddressTypePtr;
 
typedef enum {
	btLibLinkPref_Authenticated,
	btLibLinkPref_Encrypted,
	btLibLinkPref_LinkRole
} BtLibLinkPrefsEnum;
 
 
typedef enum {
	btLibMasterRole,
	btLibSlaveRole
} BtLibConnectionRoleEnum;

typedef enum {
	btLibCachedThenRemote,  // Look for a name in the cache, if none is found ask the remote device
	btLibCachedOnly,			// Look for a name in the cache, if none is found then fail
	btLibRemoteOnly			// Ignore the cache, ask the remote device for it's name directly
}	BtLibGetNameEnum;

// The BtLibGeneralPrefEnum is used to get and set general bluetooth preferences 
typedef enum {
	btLibPref_Name,
	btLibPref_UnconnectedAccessible, 
	btLibPref_CurrentAccessible,     // BtLibGetGeneralPreference only 
	btLibPref_LocalClassOfDevice, 
	btLibPref_LocalDeviceAddress	   // BtLibGetGeneralPreference only 		
} BtLibGeneralPrefEnum;

// The BtLibPinType is used to get/set the deice PIN value
typedef struct BtLibPinType{
	UInt8* pinP;		// ptr to an array containg the pin value
	UInt8 pinLength;	// number of bytes in the pin
} BtLibPinType;

// Max device name length, including null terminator
#define btLibMaxDeviceNameLength 249

// The BtLibFriendlyNameType is used to get/set the device friendly name
typedef struct BtLibFriendlyNameType{
	UInt8  *name;			// an array containing the device's freindly name (null terminated)
 	UInt8  nameLength;	// Size of the friendly name, including the null terminator, max is btLibMaxDeviceNameLength
 } BtLibFriendlyNameType, *BtLibFriendlyNameTypePtr;
 
 // The BtLibAccessibleModeEnum 
 // is used to get/set accessibility information
typedef enum {
 	btLibNotAccessible = 0x00,
 	btLibConnectableOnly = 0x02,
 	btLibDiscoverableAndConnectable = 0x03 
} BtLibAccessibleModeEnum;


// Mode of the acl link with a remote device.  
// ONLY HOLD AND ACTIVE MODE CURRENTLY SUPPORTED
typedef enum {
	btLibSniffMode, 
	btLibHoldMode,   
	btLibParkMode,
	btLibActiveMode
} BtLibLinkModeEnum;
 
/*---------------------------------------------------------------------------
 * BtLibClassOfDeviceType
 *
 *     Bit pattern representing the class of device along with the 
 *     supported services. There can be more than one supported service.
 *     Service classes can be ORed together. The Device Class is composed
 *     of a major device class plus a minor device class. The class of
 *     device value is created by ORing together the service classes plus
 *     one major device class plus on minor device class. The minor device
 *     class is interpreted in the context of the major device class.
 */

typedef UInt32 BtLibClassOfDeviceType;
/* Group: Major Service Classes.(Can be ORed together) */
#define btLibCOD_LimitedDiscoverableMode   0x00002000
#define btLibCOD_Networking                0x00020000
#define btLibCOD_Rendering                 0x00040000
#define btLibCOD_Capturing                 0x00080000
#define btLibCOD_ObjectTransfer            0x00100000
#define btLibCOD_Audio                     0x00200000
#define btLibCOD_Telephony                 0x00400000
#define btLibCOD_Information               0x00800000
#define btLibCOD_ServiceAny	             0x00ffE000

/* Group: Major Device Classes (Select one) */
#define btLibCOD_Major_Misc                0x00000000
#define btLibCOD_Major_Computer            0x00000100
#define btLibCOD_Major_Phone               0x00000200
#define btLibCOD_Major_Lan_Access_Point    0x00000300
#define btLibCOD_Major_Audio               0x00000400
#define btLibCOD_Major_Peripheral          0x00000500
#define btLibCOD_Major_Unclassified        0x00001F00
#define btLibCOD_Major_Any						 0x00001F00

#define btLibCOD_Minor_Any	         		 0x000000FC
/* Group: Minor Device Class - Computer Major class (Select one)*/
#define btLibCOD_Minor_Comp_Unclassified   0x00000000
#define btLibCOD_Minor_Comp_Desktop        0x00000004
#define btLibCOD_Minor_Comp_Server         0x00000008
#define btLibCOD_Minor_Comp_Laptop         0x0000000C
#define btLibCOD_Minor_Comp_Handheld       0x00000010
#define btLibCOD_Minor_Comp_Palm           0x00000014
#define btLibCOD_Minor_Comp_Any	          btLibCOD_Minor_Any

/* Group: Minor Device Class - Phone Major class (Select one)*/
#define btLibCOD_Minor_Phone_Unclassified  0x00000000
#define btLibCOD_Minor_Phone_Cellular      0x00000004
#define btLibCOD_Minor_Phone_Cordless      0x00000008
#define btLibCOD_Minor_Phone_Smart         0x0000000C
#define btLibCOD_Minor_Phone_Modem         0x00000010
#define btLibCOD_Minor_Phone_Any	          btLibCOD_Minor_Any

/* Group: Minor Device Class - LAN Access Point Major class (Select one)*/
#define btLibCOD_Minor_Lan_0               0x00000000     /* fully available */
#define btLibCOD_Minor_Lan_17              0x00000020     /* 1-17% utilized */
#define btLibCOD_Minor_Lan_33              0x00000040     /* 17-33% utilized */
#define btLibCOD_Minor_Lan_50              0x00000060     /* 33-50% utilized */
#define btLibCOD_Minor_Lan_67              0x00000080     /* 50-67% utilized */
#define btLibCOD_Minor_Lan_83              0x000000A0     /* 67-83% utilized */
#define btLibCOD_Minor_Lan_99              0x000000C0     /* 83-99% utilized */
#define btLibCOD_Minor_Lan_NoService       0x000000E0     /* 100% utilized */
#define btLibCOD_Minor_Lan_Any	          btLibCOD_Minor_Any

/* Group: Minor Device Class - Audio Major class (Select one)*/
#define btLibCOD_Minor_Audio_Unclassified  0x00000000
#define btLibCOD_Minor_Audio_Headset       0x00000004
#define btLibCOD_Minor_Audio_Any	          btLibCOD_Minor_Any

/* Group: Masks used to isolate the class of device components */
#define btLibCOD_Service_Mask              0x00ffE000
#define btLibCOD_Major_Mask                0x00001F00
#define btLibCOD_Minor_Mask                0x000000FC
 
//
// Event codes for Management notifications/callbacks
//
typedef enum{
	// The Radio is initialized
	btLibManagementEventRadioState = 1,

	// A Remote Device has responded to an Inquiry.  
	btLibManagementEventInquiryResult,
	
	// The inquiry process has completed.  
	btLibManagementEventInquiryComplete,
	
	// The inquiry process has been cancelled.
	btLibManagementEventInquiryCanceled,
	
	// An ACL link has disconnected.
	btLibManagementEventACLDisconnect,
	
	// A remote device has established an ACL link to us
	btLibManagementEventACLConnectInbound,
	
	// An ACL link has been established.  
	btLibManagementEventACLConnectOutbound,
	
	// The piconet has been set up
	btLibManagementEventPiconetCreated,
	
	// The piconet has been destroyed.  
	btLibManagementEventPiconetDestroyed,
	
	// Mode Change (Hold, Park, Sniff, or Active)
	btLibManagementEventModeChange,
	
	// Accesibility Change.  Determines whether we accept connections and 
	// respond to Inquiries
	btLibManagementEventAccessibilityChange,
	
	// A link has enabled or disabled encryption
	btLibManagementEventEncryptionChange,
	
	// A link has switched its master/slave role
	btLibManagementEventRoleChange,
	
	// A remote name request has completed.
	btLibManagementEventNameResult,

	// A local name change has occurred
	btLibManagementEventLocalNameChange,
	
	// A remote device authentication is complete.
	btLibManagementEventAuthenticationComplete,
	
	// A passkey request is received.  The event
	// is for informational purposes only, the OS
	// will handle the request.  Because this can 
	// happen during  or after link establishment 
	// you may want to consider disabling any 
	// failure timers while the passkey dialog is 
	// up.  The requestComplete event signals the 
	// completion of the passkey entry.     
	btLibManagementEventPasskeyRequest,
		
	// A passkey request was processed by the OS.  
	// The status code for this event will tell 
	// you whether the passkey was entered or 
	// or cancelled.  This event does NOT tell you
	// that the authentication completed.
	btLibManagementEventPasskeyRequestComplete,
	
	// Pairing process is complete and the link is authenticated.
	btLibManagementEventPairingComplete
	
} BtLibManagementEventEnum; 

//
// The notifydetailsP element of the SysNotifyParamType points to the following for the 
// Management callback
//
typedef struct _BtLibManagementEventType{
	//---------------------------------------------------------------------------
	// event
	//
 	//     Description: Event code for callback.
 	//
 	//	   Events: All
    //
	BtLibManagementEventEnum event;
	
	//---------------------------------------------------------------------------
	// status
	//
 	//     Description: status of the event
 	//
 	//	   Events: All
    //	
	Err status;
	
	union{
		//---------------------------------------------------------------------------
		// bdAddr
	 	//
 	 	//     Description: Contains Bluetooth device address of remote device
 	 	//
 	 	//	   Events: btLibManagementEventACLConnectInbound
 	 	//	           btLibManagementEventACLConnectOutbound
 	 	//			   btLibManagementEventACLDisconnect
		//			   btLibManagementEventAuthenticationComplete
		//	
		BtLibDeviceAddressType    bdAddr;       
		
		//---------------------------------------------------------------------------
		// accessible
	 	//
 	 	//     Description: Contains Radio Accessibility
 	 	//
 	 	//	   Events: btLibManagementEventAccessibilityChange
     	//	
		BtLibAccessibleModeEnum	 accessible;    
		
		//---------------------------------------------------------------------------
		// name
	 	//
 	 	//     Description: Contains Bluetooth friendly name
 	 	//
 	 	//	   Events: btLibManagementEventNameResult
 	 	//			   btLibManagementEventLocalNameChange
     	//	
		struct{
			BtLibDeviceAddressType    bdAddr;
			BtLibFriendlyNameType     name;
		} nameResult;
		
		//---------------------------------------------------------------------------
		// inquiryResult
	 	//
 	 	//     Description: Represents information about a single device found during an
 	 	//     Inquiry process.
 	 	//
 	 	//	   Events: btLibManagementEventInquiryResult
     	//
		struct 
		{
    		BtLibDeviceAddressType    bdAddr;           /* Device Address */
    		BtLibClassOfDeviceType    classOfDevice;
		} inquiryResult;

		//---------------------------------------------------------------------------
		// modeChange
	 	//
 	 	//     Description: Bluetooth device address of acl link whose mode changed
 	 	//	   the current mode and the length of time to be in that mode if applicable
 	 	//
 	 	//	   Events: 	btLibManagementEventModeChange
     	//
		struct
		{
			BtLibDeviceAddressType    bdAddr;          
			BtLibLinkModeEnum         curMode;
			UInt16                    interval;
		} modeChange;
		
		//---------------------------------------------------------------------------
		// encryptionChange
	 	//
 	 	//     Description: Bluetooth device address of acl link where encryption
 	 	//	   was enabled or disabled
 	 	//
 	 	//	   Events: 	btLibManagementEventEncryptionChange
     	//
		struct 
		{
			BtLibDeviceAddressType   bdAddr;
			Boolean                  enabled;
		} encryptionChange;

		//---------------------------------------------------------------------------
		// roleChange
	 	//
 	 	//     Description: Bluetooth device address of acl link that changed roles
 	 	//     along with its new role.  
 	 	//
 	 	//	   Events: 	btLibManagementEventRoleChange
     	//
		struct 
		{
			BtLibDeviceAddressType    bdAddr;           
    		BtLibConnectionRoleEnum   newRole;       /* New role */
		} roleChange;

	} eventData;

} BtLibManagementEventType; 
 
typedef void (*BtLibManagementProcPtr) (BtLibManagementEventType *mEvent, UInt32 refCon);
 
  #pragma mark *---------L2CAP---------------*
/********************************************************************
 * L2CAP
 ********************************************************************/
 typedef UInt16 BtLibL2CapPsmType;
 typedef UInt16 BtLibL2CapChannelIdType;
 
 #define BT_L2CAP_RANDOM_PSM 0xFFFF
 #define BT_L2CAP_MTU 672
 
 
 #pragma mark *---------RFCOMM-------------*
/********************************************************************
 * RFCOMM
 ********************************************************************/
 typedef UInt8 BtLibRfCommServerIdType;
 #define BT_RF_MAX_FRAMESIZE (BT_L2CAP_MTU - 5)
 #define BT_RF_MIN_FRAMESIZE 23
 #define BT_RF_DEFAULT_FRAMESIZE 127

 #pragma mark *---------SDP-----------------*
/********************************************************************
 * SDP
 ********************************************************************/

typedef MemHandle BtLibSdpRecordHandle;

typedef UInt32 BtLibSdpRemoteServiceRecordHandle;

// In reality, all UUID's are 128 bit's.  As an optimization, the Bluetooth
// special interest group secured a range of UUID's, allowing UUID's listed within
// the bluetooth spec to be represented by a smaller 16 or 32 bit value.  Since these
// shorter UUID's are kept unqiue by registration (in the form of inclusion in an offial spec
// document) _only_ the spec may create 16 and 32 bit UUID's. All other developers should use
// normal 128-bit UUID's for their services.
typedef enum{
	btLibUuidSize16 = 2,
	btLibUuidSize32 = 4,
	btLibUuidSize128 = 16
} BtLibSdpUuidSizeEnum;

typedef struct BtLibSdpUuidType{
	BtLibSdpUuidSizeEnum	size;
	UInt8	UUID[16];	
}BtLibSdpUuidType;

// Represents the UUID associated with a specific service and
// profile. 
//
// Note: All UUID constants are defined as strings so that they 
// can easily be copied into the BtLibSdpUuidType structure
// that is used by the API. The macro BtLibSdpUuidInitialize is provided
// as a convenient method of initializing a BtLibSdpUuidType.  

// Macro for initializing UUIDs
// BtLibSdpUuidInitialize(BtLibSdpUuidType uuid, UInt8* define, BtLibSdpUuidSizeEnum defineSize)
// 
// Example:
//	BtLibSdpUuidType uuid;
//
// BtLibSdpUuidInitialize(uuid, btLibSdpUUID_SC_LAN_ACCESS_PPP, btLibUuidSize16);

#define BtLibSdpUuidInitialize(uuid, define, defineSize)  do { (uuid).size = (defineSize); \
	MemMove((uuid).UUID, (define), (uuid).size) } while (0); 
  
#define btLibSdpUUID_SC_SERVICE_DISCOVERY_SERVER   "\x10\x00" 
#define btLibSdpUUID_SC_BROWSE_GROUP_DESC         	"\x10\x01" 
#define btLibSdpUUID_SC_PUBLIC_BROWSE_GROUP       	"\x10\x02" 
#define btLibSdpUUID_SC_SERIAL_PORT              	"\x11\x01" 
#define btLibSdpUUID_SC_LAN_ACCESS_PPP            	"\x11\x02" 
#define btLibSdpUUID_SC_DIALUP_NETWORKING         	"\x11\x03" 
#define btLibSdpUUID_SC_IRMC_SYNC                 	"\x11\x04" 
#define btLibSdpUUID_SC_OBEX_OBJECT_PUSH          	"\x11\x05" 
#define btLibSdpUUID_SC_OBEX_FILE_TRANSFER       	"\x11\x06" 
#define btLibSdpUUID_SC_IRMC_SYNC_COMMAND         	"\x11\x07" 
#define btLibSdpUUID_SC_HEADSET                   	"\x11\x08" 
#define btLibSdpUUID_SC_CORDLESS_TELEPHONY        	"\x11\x09"
#define btLibSdpUUID_SC_INTERCOM                  	"\x11\x10"
#define btLibSdpUUID_SC_FAX                       	"\x11\x11"
#define btLibSdpUUID_SC_HEADSET_AUDIO_GATEWAY     	"\x11\x12"
#define btLibSdpUUID_SC_WAP                       	"\x11\x13"
#define btLibSdpUUID_SC_WAP_CLIENT                	"\x11\x14"
#define btLibSdpUUID_SC_PNP_INFO                  	"\x11\x15"
#define btLibSdpUUID_SC_GENERIC_NETWORKING        	"\x12\x01"
#define btLibSdpUUID_SC_GENERIC_FILE_TRANSFER     	"\x12\x02"
#define btLibSdpUUID_SC_GENERIC_AUDIO             	"\x12\x03"
#define btLibSdpUUID_SC_GENERIC_TELEPHONY         	"\x12\x04"

// Represents the UUID associated with a protocol.
#define btLibSdpUUID_PROT_SDP                     	"\x00\x01"
#define btLibSdpUUID_PROT_RFCOMM                  	"\x00\x03"
#define btLibSdpUUID_PROT_TCS_BIN                 	"\x00\x05"
#define btLibSdpUUID_PROT_L2CAP                   	"\x01\x00"
#define btLibSdpUUID_PROT_IP                      	"\x00\x09"
#define btLibSdpUUID_PROT_UDP                     	"\x00\x02"
#define btLibSdpUUID_PROT_TCP                     	"\x00\x04"
#define btLibSdpUUID_PROT_TCS_AT                  	"\x00\x06"
#define btLibSdpUUID_PROT_OBEX                    	"\x00\x08"
#define btLibSdpUUID_PROT_FTP                     	"\x00\x0A"
#define btLibSdpUUID_PROT_HTTP                    	"\x00\x0C"
#define btLibSdpUUID_PROT_WSP                     	"\x00\x0E"


// This structure is defined by The Bluetooth Core Specification 1.1
// It is used in defining the BtSdpAtributeValuesType stucture.
typedef struct BtLibLanguageBaseTripletType{
	UInt16	naturalLanguageIdentifier; // see btLibLang defines
	UInt16	characterEncoding;
	UInt16	baseAttributeID;
} BtLibLanguageBaseTripletType;

//
// Natural Language Indentifiers.  The language base triplet contains the language 
// identifiers defined in the ISO 639:1988 specification.  The values are used
// to determine the language of a string.  
//
#define btLibLangAfar  			'aa'
#define btLibLangAbkihazian 	'ab'
#define btLibLangAfrikaans		'af'
#define btLibLangAmharic		'am'
#define btLibLangArabic			'ar'
#define btLibLangAssamese		'as'
#define btLibLangAymara			'ay'
#define btLibLangAzerbaijani	'az'

#define btLibLangBashkir		'ba'
#define btLibLangByelorussian	'be'
#define btLibLangBulgarian		'bg'
#define btLibLangBihari			'bh'
#define btLibLangBislama		'bi'
#define btLibLangBengali		'bn'
#define btLibLangTibetan		'bo'
#define btLibLangBreton			'br'

#define btLibLangCatalan		'ca'
#define btLibLangCorsican		'co'
#define btLibLangCzech			'cs'
#define btLibLangWelsh			'cy'

#define btLibLangDanish			'da'
#define btLibLangGerman			'de'
#define btLibLangBhutani		'dz'

#define btLibLangGreek			'el'
#define btLibLangEnglish		'en'
#define btLibLangEsperanto		'eo'
#define btLibLangSpanish		'es'
#define btLibLangEstonian		'et'
#define btLibLangBasque			'eu'

#define btLibLangPersian		'fa'
#define btLibLangFinnish		'fi'
#define btLibLangFijian			'fj'
#define btLibLangFaroese		'fo'
#define btLibLangFrench			'fr'
#define btLibLangFrisian		'fy'

#define btLibLangIrish			'ga'
#define btLibLangScotsGaelic	'gd'
#define btLibLangGalician		'gl'
#define btLibLangGuarani		'gn'
#define btLibLangGujarati		'gu'

#define btLibLangHausa			'ha'
#define btLibLangHindi			'hi'
#define btLibLangCroation		'hr'
#define btLibLangHungarian		'hu'
#define btLibLangArmenian		'hy'

#define btLibLangInterlingua	'ia'
#define btLibLangInterlingue	'ie'
#define btLibLangInupiak		'ik'
#define btLibLangIndonesian	'in'
#define btLibLangIcelandic		'is'
#define btLibLangItalian		'it'
#define btLibLangHebrew			'iw'

#define btLibLangJapanese		'ja'
#define btLibLangYiddish		'ji'
#define btLibLangJavanese		'jw'

#define btLibLangGeorgian		'ka'
#define btLibLangKazakh			'kk'
#define btLibLangGreenlandic	'kl'
#define btLibLangCambodian		'km'
#define btLibLangKannada		'kn'
#define btLibLangKorean			'ko'
#define btLibLangKashmiri		'ks'
#define btLibLangKurdish		'ku'
#define btLibLangKirghiz		'ky'

#define btLibLangLatin			'la'
#define btLibLangLingala		'ln'
#define btLibLangLaothian		'lo'
#define btLibLangLithuanian	'lt'
#define btLibLangLatvian		'lv'

#define btLibLangMalagasy		'mg'
#define btLibLangMaori			'mi'
#define btLibLangMacedonian	'mk'
#define btLibLangMalayalam		'ml'
#define btLibLangMongolian		'mn'
#define btLibLangMoldavian		'mo'
#define btLibLangMarathi		'mr'
#define btLibLangMalay			'ms'
#define btLibLangMaltese		'mt'
#define btLibLangBurmese		'my'

#define btLibLangNaura			'na'
#define btLibLangNepali			'ne'
#define btLibLangDutch			'nl'
#define btLibLangNorwegian		'no'

#define btLibLangOccitan		'oc'
#define btLibLangOromo			'om'
#define btLibLangOriya			'or'

#define btLibLangPunjabi		'pa'
#define btLibLangPolish			'pl'
#define btLibLangPashto			'ps'
#define btLibLangPortuguese	'pt'

#define btLibLangQuechua		'qu'

#define btLibLangRhaeto_Romance 'rm'
#define btLibLangKirundi		'rn'
#define btLibLangRomanian		'ro'
#define btLibLangRussian		'ru'
#define btLibLangKinyarwanda	'rw'

#define btLibLangSanskrit		'sa'
#define btLibLangSindhi			'sd'
#define btLibLangSangho			'sg'
#define btLibLangSerbo_Croation 'sh'
#define btLibLangSinghalese	'si'
#define btLibLangSlovak			'sk'
#define btLibLangSlovenian		'sl'
#define btLibLangSamoan			'sm'
#define btLibLangShona			'sn'
#define btLibLangSomali			'so'
#define btLibLangAlbanian		'sq'
#define btLibLangSerbian		'sr'
#define btLibLangSiswati		'ss'
#define btLibLangSesotho		'st'
#define btLibLangSundanese		'su'
#define btLibLangSwedish		'sv'
#define btLibLangSwahili		'sw'

#define btLibLangTamil			'ta'
#define btLibLangTelugu			'te'
#define btLibLangTajik			'tg'
#define btLibLangThai 			'th'
#define btLibLangTigrinya		'ti'
#define btLibLangTurkmen		'tk'
#define btLibLangTagalog		'tl'
#define btLibLangSetswanna		'tn'
#define btLibLangTonga			'to'
#define btLibLangTurkish		'tr'
#define btLibLangTsonga			'ts'
#define btLibLangTatar			'tt'
#define btLibLangTwi				'tw'

#define btLibLangUkranian		'uk'
#define btLibLangUrdu			'ur'
#define btLibLangUzbek			'uz'

#define btLibLangVietnamese	'vi'
#define btLibLangVolapuk		'vo'

#define btLibLangWolof			'wo'

#define btLibLangXhosa			'xh'

#define btLibLangYoruba			'yo'

#define btLibLangChinese		'zh'
#define btLibLangZulu			'zu'

//
// The btLibCharSet define is used by the characterEncoding portion of the SDP
// language base triplet to specify the format of strings in the SDP attributes.  
// The values for these defines are derived from the file located at the web site
// referenced in the SDP portion of the 1.1 bluetooth specification.
//
// http://www.isi.edu/in-notes/iana/assignments/character-sets
#define btLibCharSet_US_ASCII 3
#define btLibCharSet_ISO_10646_UTF_1 27
#define btLibCharSet_ISO_646_basic_1983 28
#define btLibCharSet_INVARIANT 29
#define btLibCharSet_ISO_646_irv_1983 30
#define btLibCharSet_BS_4730 20
#define btLibCharSet_NATS_SEFI 31
#define btLibCharSet_NATS_SEFI_ADD 32
#define btLibCharSet_NATS_DANO 33
#define btLibCharSet_NATS_DANO_ADD 34
#define btLibCharSet_SEN_850200_B 35
#define btLibCharSet_SEN_850200_C 21
#define btLibCharSet_KS_C_5601_1987 36
#define btLibCharSet_ISO_2022_KR 37
#define btLibCharSet_EUC_KR 38
#define btLibCharSet_ISO_2022_JP 39
#define btLibCharSet_ISO_2022_JP_2 40
#define btLibCharSet_ISO_2022_CN 104
#define btLibCharSet_ISO_2022_CN_EXT 105
#define btLibCharSet_JIS_C6220_1969_jp 41
#define btLibCharSet_JIS_C6220_1969_ro 42
#define btLibCharSet_IT 22
#define btLibCharSet_PT 43
#define btLibCharSet_ES 23
#define btLibCharSet_greek7_old 44
#define btLibCharSet_latin_greek 45
#define btLibCharSet_DIN_66003 24
#define btLibCharSet_NF_Z_62_010__1973_ 46
#define btLibCharSet_Latin_greek_1 47
#define btLibCharSet_ISO_5427 48
#define btLibCharSet_JIS_C6226_1978 49
#define btLibCharSet_BS_viewdata 50
#define btLibCharSet_INIS 51
#define btLibCharSet_INIS_8 52
#define btLibCharSet_INIS_cyrillic 53
#define btLibCharSet_ISO_5427_1981 54
#define btLibCharSet_ISO_5428_1980 55
#define btLibCharSet_GB_1988_80 56
#define btLibCharSet_GB_2312_80 57
#define btLibCharSet_NS_4551_1 25
#define btLibCharSet_NS_4551_2 58
#define btLibCharSet_NF_Z_62_010 26
#define btLibCharSet_videotex_suppl 59
#define btLibCharSet_PT2 60
#define btLibCharSet_ES2 61
#define btLibCharSet_MSZ_7795_3 62
#define btLibCharSet_JIS_C6226_1983 63
#define btLibCharSet_greek7 64
#define btLibCharSet_ASMO_449 65
#define btLibCharSet_iso_ir_90 66
#define btLibCharSet_JIS_C6229_1984_a 67
#define btLibCharSet_JIS_C6229_1984_b 68
#define btLibCharSet_JIS_C6229_1984_b_add 69
#define btLibCharSet_JIS_C6229_1984_hand 70
#define btLibCharSet_JIS_C6229_1984_hand_add 71
#define btLibCharSet_JIS_C6229_1984_kana 72
#define btLibCharSet_ISO_2033_1983 73
#define btLibCharSet_ANSI_X3_110_1983 74
#define btLibCharSet_ISO_8859_1 4
#define btLibCharSet_ISO_8859_2 5
#define btLibCharSet_T_61_7bit 75
#define btLibCharSet_T_61_8bit 76
#define btLibCharSet_ISO_8859_3 6
#define btLibCharSet_ISO_8859_4 7
#define btLibCharSet_ECMA_cyrillic 77
#define btLibCharSet_CSA_Z243_4_1985_1 78
#define btLibCharSet_CSA_Z243_4_1985_2 79
#define btLibCharSet_CSA_Z243_4_1985_gr 80
#define btLibCharSet_ISO_8859_6 9
#define btLibCharSet_ISO_8859_6_E 81
#define btLibCharSet_ISO_8859_6_I 82
#define btLibCharSet_ISO_8859_7 10
#define btLibCharSet_T_101_G2 83
#define btLibCharSet_ISO_8859_8 11
#define btLibCharSet_ISO_8859_8_E 84
#define btLibCharSet_ISO_8859_8_I 85
#define btLibCharSet_CSN_369103 86
#define btLibCharSet_JUS_I_B1_002 87
#define btLibCharSet_ISO_6937_2_add 14
#define btLibCharSet_IEC_P27_1 88
#define btLibCharSet_ISO_8859_5 8
#define btLibCharSet_JUS_I_B1_003_serb 89
#define btLibCharSet_JUS_I_B1_003_mac 90
#define btLibCharSet_ISO_8859_9 12
#define btLibCharSet_greek_ccitt 91
#define btLibCharSet_NC_NC00_10_81 92
#define btLibCharSet_ISO_6937_2_25 93
#define btLibCharSet_GOST_19768_74 94
#define btLibCharSet_ISO_8859_supp 95
#define btLibCharSet_ISO_10367_box 96
#define btLibCharSet_ISO_8859_10 13
#define btLibCharSet_latin_lap 97
#define btLibCharSet_JIS_X0212_1990 98
#define btLibCharSet_DS_2089 99
#define btLibCharSet_us_dk 100
#define btLibCharSet_dk_us 101
#define btLibCharSet_JIS_X0201 15
#define btLibCharSet_KSC5636 102
#define btLibCharSet_ISO_10646_UCS_2 1000
#define btLibCharSet_ISO_10646_UCS_4 1001
#define btLibCharSet_DEC_MCS 2008
#define btLibCharSet_hp_roman8 2004
#define btLibCharSet_macintosh 2027
#define btLibCharSet_IBM037 2028
#define btLibCharSet_IBM038 2029
#define btLibCharSet_IBM273 2030
#define btLibCharSet_IBM274 2031
#define btLibCharSet_IBM275 2032
#define btLibCharSet_IBM277 2033
#define btLibCharSet_IBM278 2034
#define btLibCharSet_IBM280 2035
#define btLibCharSet_IBM281 2036
#define btLibCharSet_IBM284 2037
#define btLibCharSet_IBM285 2038
#define btLibCharSet_IBM290 2039
#define btLibCharSet_IBM297 2040
#define btLibCharSet_IBM420 2041
#define btLibCharSet_IBM423 2042
#define btLibCharSet_IBM424 2043
#define btLibCharSet_IBM437 2011
#define btLibCharSet_IBM500 2044
#define btLibCharSet_IBM775 2087
#define btLibCharSet_IBM850 2009
#define btLibCharSet_IBM851 2045
#define btLibCharSet_IBM852 2010
#define btLibCharSet_IBM855 2046
#define btLibCharSet_IBM857 2047
#define btLibCharSet_IBM860 2048
#define btLibCharSet_IBM861 2049
#define btLibCharSet_IBM862 2013
#define btLibCharSet_IBM863 2050
#define btLibCharSet_IBM864 2051
#define btLibCharSet_IBM865 2052
#define btLibCharSet_IBM866 2086
#define btLibCharSet_IBM868 2053
#define btLibCharSet_IBM869 2054
#define btLibCharSet_IBM870 2055
#define btLibCharSet_IBM871 2056
#define btLibCharSet_IBM880 2057
#define btLibCharSet_IBM891 2058
#define btLibCharSet_IBM903 2059
#define btLibCharSet_IBM904 2060
#define btLibCharSet_IBM905 2061
#define btLibCharSet_IBM918 2062
#define btLibCharSet_IBM1026 2063
#define btLibCharSet_EBCDIC_AT_DE 2064
#define btLibCharSet_EBCDIC_AT_DE_A 2065
#define btLibCharSet_EBCDIC_CA_FR 2066
#define btLibCharSet_EBCDIC_DK_NO 2067
#define btLibCharSet_EBCDIC_DK_NO_A 2068
#define btLibCharSet_EBCDIC_FI_SE 2069
#define btLibCharSet_EBCDIC_FI_SE_A 2070
#define btLibCharSet_EBCDIC_FR 2071
#define btLibCharSet_EBCDIC_IT 2072
#define btLibCharSet_EBCDIC_PT 2073
#define btLibCharSet_EBCDIC_ES 2074
#define btLibCharSet_EBCDIC_ES_A 2075
#define btLibCharSet_EBCDIC_ES_S 2076
#define btLibCharSet_EBCDIC_UK 2077
#define btLibCharSet_EBCDIC_US 2078
#define btLibCharSet_UNKNOWN_8BIT 2079
#define btLibCharSet_MNEMONIC 2080
#define btLibCharSet_MNEM 2081
#define btLibCharSet_VISCII 2082
#define btLibCharSet_VIQR 2083
#define btLibCharSet_KOI8_R 2084
#define btLibCharSet_KOI8_U 2088
#define btLibCharSet_IBM00858 2089
#define btLibCharSet_IBM00924 2090
#define btLibCharSet_IBM01140 2091
#define btLibCharSet_IBM01141 2092
#define btLibCharSet_IBM01142 2093
#define btLibCharSet_IBM01143 2094
#define btLibCharSet_IBM01144 2095
#define btLibCharSet_IBM01145 2096
#define btLibCharSet_IBM01146 2097
#define btLibCharSet_IBM01147 2098
#define btLibCharSet_IBM01148 2099
#define btLibCharSet_IBM01149 2100
#define btLibCharSet_Big5_HKSCS 2101
#define btLibCharSet_UNICODE_1_1 1010
#define btLibCharSet_SCSU 1011
#define btLibCharSet_UTF_7 1012
#define btLibCharSet_UTF_16BE 1013
#define btLibCharSet_UTF_16LE 1014
#define btLibCharSet_UTF_16 1015
#define btLibCharSet_UNICODE_1_1_UTF_7 103
#define btLibCharSet_UTF_8 106
#define btLibCharSet_iso_8859_13 109
#define btLibCharSet_iso_8859_14 110
#define btLibCharSet_ISO_8859_15 111
#define btLibCharSet_JIS_Encoding 16
#define btLibCharSet_Shift_JIS 17
#define btLibCharSet_EUC_JP 18
#define btLibCharSet_Extended_UNIX_Code_Fixed_Width_for_Japanese 19
#define btLibCharSet_ISO_10646_UCS_Basic 1002
#define btLibCharSet_ISO_10646_Unicode_Latin1 1003
#define btLibCharSet_ISO_Unicode_IBM_1261 1005
#define btLibCharSet_ISO_Unicode_IBM_1268 1006
#define btLibCharSet_ISO_Unicode_IBM_1276 1007
#define btLibCharSet_ISO_Unicode_IBM_1264 1008
#define btLibCharSet_ISO_Unicode_IBM_1265 1009
#define btLibCharSet_ISO_8859_1_Windows_3_0_Latin_1 2000
#define btLibCharSet_ISO_8859_1_Windows_3_1_Latin_1 2001
#define btLibCharSet_ISO_8859_2_Windows_Latin_2 2002
#define btLibCharSet_ISO_8859_9_Windows_Latin_5 2003
#define btLibCharSet_Adobe_Standard_Encoding 2005
#define btLibCharSet_Ventura_US 2006
#define btLibCharSet_Ventura_International 2007
#define btLibCharSet_PC8_Danish_Norwegian 2012
#define btLibCharSet_PC8_Turkish 2014
#define btLibCharSet_IBM_Symbols 2015
#define btLibCharSet_IBM_Thai 2016
#define btLibCharSet_HP_Legal 2017
#define btLibCharSet_HP_Pi_font 2018
#define btLibCharSet_HP_Math8 2019
#define btLibCharSet_Adobe_Symbol_Encoding 2020
#define btLibCharSet_HP_DeskTop 2021
#define btLibCharSet_Ventura_Math 2022
#define btLibCharSet_Microsoft_Publishing 2023
#define btLibCharSet_Windows_31J 2024
#define btLibCharSet_GB2312 2025
#define btLibCharSet_Big5 2026
#define btLibCharSet_windows_1250 2250
#define btLibCharSet_windows_1251 2251
#define btLibCharSet_windows_1252 2252
#define btLibCharSet_windows_1253 2253
#define btLibCharSet_windows_1254 2254
#define btLibCharSet_windows_1255 2255
#define btLibCharSet_windows_1256 2256
#define btLibCharSet_windows_1257 2257
#define btLibCharSet_windows_1258 2258
#define btLibCharSet_TIS_620 2259
#define btLibCharSet_HZ_GB_2312 2085

typedef struct BtLibProtocolDescriptorListEntryType{
	BtLibSdpUuidType protoUUID;				
	union { 
		BtLibL2CapPsmType psm;				// valid only if protoUUID is L2Cap
		BtLibRfCommServerIdType channel; // valid only if protoUUID is RFCOMM
	} param;
} BtLibProtocolDescriptorListEntryType;

typedef struct BtLibProfileDescriptorListEntryType{
	BtLibSdpUuidType profUUID;				
	UInt16 version;
} BtLibProfileDescriptorListEntryType;

typedef struct BtLibStringType{
	Char* str;
	UInt16 strLen;
} BtLibStringType;

typedef struct BtLibUrlType{
	Char* url;
	UInt16 urlLen;
}BtLibUrlType;

// The BtSdpAttributeIdType creates constants for the service attribute definitions
// outlined in the Service Discovery Protocol section of the Core document
// of the Bluetooth 1.1 specification.  For more information on each of the attribute 
// types, see the Bluetooth specification  
typedef UInt16 BtLibSdpAttributeIdType;

#define 	btLibServiceClassIdList 					0x0001	 
#define 	btLibServiceRecordState	 					0x0002    		
#define 	btLibServiceId 								0x0003    						
#define 	btLibProtocolDescriptorList 				0x0004 
#define 	btLibBrowseGroupList 						0x0005   		
#define 	btLibLanguageBaseAttributeIdList			0x0006
#define 	btLibTimeToLive								0x0007     				
#define 	btLibAvailability								0x0008   				
#define 	btLibProfileDescriptorList					0x0009  
#define 	btLibDocumentationUrl						0x000A  			
#define 	btLibClientExecutableUrl					0x000B   		
#define 	btLibIconUrl									0x000C


// These offsets must be added to the baseAttributeID field of the BtLibLanguageBaseTripletType
// attribute in order to get the attribute ID of the service name, service description, or 
// provider name if they are set in a service record.  
#define  btLibServiceNameOffset                 0x0000
#define  btLibServiceDescriptionOffset          0x0001
#define  btLibProviderNameOffset						0x0002


// The BtLibSdpAtributeDataType structure is used to get and set values of SDP attributes defined
// by the BtLibSdpAttributeIdType.  
//
// When getting a string, call BtLibSdpServiceRecordGetStringOrUrlLength() first to get the 
// string size.  Then set the pointer value of the BtLibStringType to point to a memory block
// large enough to get the string with BtLibSdpServiceRecordGetAttribute().  
typedef union BtLibSdpAttributeDataType {

		BtLibSdpUuidType serviceClassUuid;

		UInt32 serviceRecordState;

		BtLibSdpUuidType serviceIdUuid;
		
 		BtLibProtocolDescriptorListEntryType protocolDescriptorListEntry;
		
		BtLibSdpUuidType browseGroupUuid;

		BtLibLanguageBaseTripletType languageBaseTripletListEntry;

		UInt32 timeToLive;

		UInt8 availability; 

		BtLibProfileDescriptorListEntryType profileDescriptorListEntry; 

		BtLibUrlType documentationUrl;

		BtLibUrlType clientExecutableUrl;
		
		BtLibUrlType iconUrl;
		
		BtLibStringType serviceName;
		
		BtLibStringType serviceDescription;
		
		BtLibStringType providerName;

} BtLibSdpAttributeDataType;

/********************************************************************
 * SDP raw data parsing types and defines
 ********************************************************************/
/*---------------------------------------------------------------------------
 *     Specifies the type of a Data Element.
 *
 *     Data Elements begin with a single byte that contains both type and
 *     size information. To read the type from this byte, use the
 *     BtLibSdpGeRawtElementType macro.
 *
 *     To create the first byte of a Data Element, bitwise OR the
 *     Data Element type and Data Element size values into a single byte.
 */

#define btLibDETD_NIL  0x00 /* Specifies nil, the null type.
                        	  * Requires a size of btLibDESD_1BYTE, which for this type
                        	  * means an actual size of 0.
                        	  */
#define btLibDETD_UINT 0x08 /* Specifies an unsigned integer. Must use size
                        	  * btLibDESD_1BYTE, btLibDESD_2BYTES, btLibDESD_4BYTES, 
                        	  * btLibDESD_8BYTES, or btLibDESD_16BYTES.
                        	  */
#define btLibDETD_SINT 0x10 /* Specifies a signed integer. May use size
                        	  * btLibDESD_1BYTE, btLibDESD_2BYTES, btLibDESD_4BYTES, 
                        	  * btLibDESD_8BYTES, or btLibDESD_16BYTES.
                        	  */
#define btLibDETD_UUID 0x18 /* Specifies a Universally Unique Identifier (UUID).
                        	  * Must use size btLibDESD_2BYTES, btLibDESD_4BYTES, or
                        	  * btLibDESD_16BYTES.
                        	  */
#define btLibDETD_TEXT 0x20 /* Specifies a text string. Must use sizes
                        	  * btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS, or 
                        	  * btLibDESD_ADD32_BITS.
                        	  */
#define btLibDETD_BOOL 0x28 /* Specifies a boolean value. Must use size
                        	  * btLibDESD_1BYTE.
                             */
#define btLibDETD_SEQ  0x30 /* Specifies a data element sequence. The data contains
                        	  * a sequence of Data Elements. Must use sizes
                        	  * btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS, or btLibDESD_ADD_32BITS.
                        	  */
#define btLibDETD_ALT  0x38 /* Specifies a data element alternative. The data contains
                        	  * a sequence of Data Elements. This type is sometimes
                        	  * used to distinguish between two possible sequences.
                        	  * Must use size btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS,
                        	  * or btLibDESD_ADD_32BITS.
                        	  */
#define btLibDETD_URL  0x40 /* Specifies a Uniform Resource Locator (URL).
                        	  * Must use size btLibDESD_ADD_8BITS, btLibDESD_ADD_16BITS,
                        	  * or btLibDESD_ADD_32BITS.
                        	  */

#define btLibDETD_MASK 0xf8 /* AND this value with the first byte of a Data
                        	  * Element to return the element's type.
                        	  */
                        
/*---------------------------------------------------------------------------
 *     Specifies the size of a Data Element.
 *
 *     Data Elements begin with a single byte that contains both type and
 *     size information. To read the size from this byte, use the
 *     BtLibSdpGetRawDataElementSize macro.
 *
 *     To create the first byte of a Data Element, bitwise OR the
 *     Data Element type and Data Element size values into a single byte.
 *     For example, a standard 16 bit unsigned integer with a value of 0x57
 *     could be encoded as follows:
 * 
 *     U8 val[3] = { btLibDETD_UINT | btLibDESD_2BYTES, 0x00, 0x57 };
 *
 *     The text string "hello" could be encoded as follows:
 *
 *     U8 str[7] = { btLibDETD_TEXT | btLibDESD_ADD_8BITS, 0x05, 'h','e','l','l','o' };
 */
 
#define btLibDESD_1BYTE      0x00 /* Specifies a 1-byte element. However, if
                              	  * type is btLibDETD_NIL then the size is 0.
                              	  */
#define btLibDESD_2BYTES     0x01 /* Specifies a 2-byte element. */
#define btLibDESD_4BYTES     0x02 /* Specifies a 4-byte element. */
#define btLibDESD_8BYTES     0x03 /* Specifies an 8-byte element. */
#define btLibDESD_16BYTES    0x04 /* Specifies a 16-byte element. */
#define btLibDESD_ADD_8BITS  0x05 /* The element's actual data size, in bytes,
                              	  * is contained in the next 8 bits.
                              	  */
#define btLibDESD_ADD_16BITS 0x06 /* The element's actual data size, in bytes,
                              	  * is contained in the next 16 bits.
                              	  */
#define btLibDESD_ADD_32BITS 0x07 /* The element's actual data size, in bytes,
                              	  * is contained in the next 32 bits.
                              	  */

#define btLibDESD_MASK       0x07 /* AND this value with the first byte of a Data
                              	  * Element to return the element's size.
                              	  */
                          
#pragma mark *---------Sockets--------------*
/********************************************************************
 * Sockets
 ********************************************************************/
 typedef Int16 BtLibSocketRef;
 
 //
 // Event codes for Socket notifications/callbacks
 //
 typedef enum{
	// A remote device has requested a connection, must respond to this event with call to 
	// BtLibSocketRespondToConnection
	btLibSocketEventConnectRequest = 1,
	
	// A connection has been attempt is has completed, check status for success (status == btLibErrNoError)
	btLibSocketEventConnectedOutbound,

	// A remote connection has been accepted.  
	btLibSocketEventConnectedInbound,	
	
	// The connection has been lost, the socket is now invalid
	btLibSocketEventDisconnected,	
	
	// Data to receive on a socket
	btLibSocketEventData,
	
	// The data passed into BtLibSocketSend has been sent
	btLibSocketEventSendComplete,
	
	// The get service record handle by Service Class call has completed
	btLibSocketEventSdpServiceRecordHandle,
	
	// An attribute request has completed
	btLibSocketEventSdpGetAttribute,
	
	// A string or URL length request has completed
	btLibSocketEventSdpGetStringLen,
	
	// A number of list entries request has completed
	btLibSocketEventSdpGetNumListEntries,
	
	// A number of lists request has completed
	btLibSocketEventSdpGetNumLists,
	
	// A get raw attribute request has completed
	btLibSocketEventSdpGetRawAttribute,
	
	// A get raw attribute size request has completed
	btLibSocketEventSdpGetRawAttributeSize,
	
	// A get server channel request has completed
	btLibSocketEventSdpGetServerChannelByUuid,
	
	// A get psm request has completed
	btLibSocketEventSdpGetPsmByUuid		
	
} BtLibSocketEventEnum;

//
// The notifydetailsP element of the SysNotifyParamType points to the following for the 
// Socket callback
//
typedef struct _BtLibSocketEventType{

	//---------------------------------------------------------------------------
	// event
	//
 	//     Description: Event code for callback.
 	//
 	//	   Events: All
    //
	BtLibSocketEventEnum event;

	//---------------------------------------------------------------------------
	// socket
	//
 	//     Description: Socket that the callback is for.
 	//
 	//	   Events: All
    //	
	BtLibSocketRef socket;
	
	//---------------------------------------------------------------------------
	// status
	//
 	//     Description: status of the event
 	//
 	//	   Events: All
    //		
	Err    status;
	
	union
	{
		//---------------------------------------------------------------------------
		//	Connect Request
		//
	
		//---------------------------------------------------------------------------
		// data
		//
 		//    Description:  The data ptr is valid only during the callback
 		//			for inbound data.
 		//
 		//	   Events: 	btLibSocketEventData
 		//					btLibSocketEventSendComplete
    	//			
		struct
		{
			UInt16 dataLen;
			UInt8  *data;
		} data;
		
		//---------------------------------------------------------------------------
		// newSocket
		//
 		//    Description: New Socket from accepting incoming connections
 		//
 		//	   Events: 	btLibSocketEventConnectedInbound
    	//
		BtLibSocketRef newSocket;
		
		//---------------------------------------------------------------------------
		// requestingDevice
		//
 		//    Description: the Address of the remote device requesting the connection
 		//
 		//	   Events: 	btLibSocketEventConnectRequest
 		BtLibDeviceAddressType requestingDevice;

		//---------------------------------------------------------------------------
		// sdpByUuid
		//
 		//    Description: a L2cap psm or RfComm channel to connect to from SDP query
 		//
 		//	   Events: 	btLibSocketEventSdpGetServerChannelByUuid
 		//             btLibSocketEventSdpGetPsmByUuid
 		struct {
 			// The remote handle of SDP record used for query
 			BtLibSdpRemoteServiceRecordHandle remoteHandle; 
 		
			union { 
				BtLibL2CapPsmType psm;				
				BtLibRfCommServerIdType channel; 
			} param;
		} sdpByUuid;
				
		//---------------------------------------------------------------------------
		// serviceRecordHandles
		//
 		//    Description: Remote service record handles list.
 		//
 		//	   Events: 	btLibSocketEventSdpServiceRecordHandle
		struct
		{
			UInt16 numSrvRec;
			BtLibSdpRemoteServiceRecordHandle *serviceRecordList;
		} sdpServiceRecordHandles;
		
		struct {	
			
			// Valid for all Sdp events except: btLibSocketEventSdpServiceRecordHandle, 
			// btLibSocketEventSdpGetServerChannelByUuid, and btLibSocketEventSdpGetPsmByUuid
			BtLibSdpAttributeIdType attributeID; //attribute of query
			BtLibSdpRecordHandle recordH;	//local record handle for query
			
			union {
				//---------------------------------------------------------------------------
				// data
				//
 				//    Description: Attribute data requested from remote device
 				//
 				//	   Events: 	btLibSocketEventSdpGetAttribute
				struct
				{
					BtLibSdpAttributeDataType *attributeValues;
					UInt16 listNumber;
					UInt16 listEntry;
				} data;

				//---------------------------------------------------------------------------
				// rawData
				//
 				//    Description: Raw attribute data requested from remote device
		 		//
		 		//	   Events: 	btLibSocketEventSdpGetRawAttribute
				struct
				{
					UInt16 valSize;
					UInt8 *value;
				} rawData;	
		
				//---------------------------------------------------------------------------
				// valSize
				//
 				//    Description: size of buffer needed to get a raw attirbute
 				//
 				//	   Events: 		btLibSocketEventSdpGetRawAttributeSize	
				UInt16 valSize;

		
				//---------------------------------------------------------------------------
				// strLength
				//
 				//    Description: size of buffer needed to get a string or URL attribute
 				//
 				//	   Events: 		btLibSocketEventSdpGetStringLen		
				UInt16 strLength;

		
				//---------------------------------------------------------------------------
				// numItems
				//
 				//    Description: Count of lists or list entries for a particular attribute
 				//
 				//	   Events: 		btLibSocketEventSdpGetNumListEntries
				//						btLibSocketEventSdpGetNumLists
				UInt16 numItems;
			
			} info;
			
		} sdpAttribute;
				
	} eventData;
	
} BtLibSocketEventType, *BtLibSocketEventTypePtr; 

typedef void (*BtLibSocketProcPtr)(BtLibSocketEventType *sEvent, UInt32 refCon);

 typedef enum{
 	btLibL2CapProtocol,
 	btLibRfCommProtocol,
 	btLibSdpProtocol
} BtLibProtocolEnum;
 
 // the BtLibSocketInfoEnum is used to retrive information about 
 // at given socket in conjunction with the BtLibSocketGetInfo call
typedef enum {
 	// General Info
 	btLibSocketInfo_Protocol =0,
 	btLibSocketInfo_RemoteDeviceAddress,
 	
 	// L2Cap and RfComm Shared
 	btLibSocketInfo_SendPending = 100,
 	btLibSocketInfo_MaxTxSize,
 	btLibSocketInfo_MaxRxSize,
 	
 	// L2Cap Info
 	btLibSocketInfo_L2CapPsm = 200,
 	btLibSocketInfo_L2CapChannel,
 	
 	// RFComm specific
 	btLibSocketInfo_RfCommServerId = 300,
 	btLibSocketInfo_RfCommOutstandingCredits,
 	
 	// SDP specific
 	btLibSocketInfo_SdpServiceRecordHandle = 400
 	
} BtLibSocketInfoEnum;
  
 // the socket listen info is used by the BtLibSocketListen
 // call to setup a listening socket
typedef struct BtLibSocketListenInfoType { 	 	 	
   
	union {
		struct {
		 	/* Pre-assigned assigned PSM values are permitted; however, they
 		  	* must be odd, within the range of 0x1001 to 0xFFFF, and have
 		  	* the 9th bit (0x0100) set to zero. Passing in BT_L2CAP_RANDOM_PSM
 		  	* will automatically create a usable PSM for the channel. In this case
 		  	* the actual PSM value will be filled in by the call. */
			BtLibL2CapPsmType	localPsm;
			UInt16 localMtu;
			UInt16 minRemoteMtu;
		} L2Cap;
		
		struct {
			// The Service Id is assigned by RFCOMM
			// and returned in the serviceID field
			BtLibRfCommServerIdType serviceID;
			
			// BT_RF_MIN_FRAMESIZE <= maxFrameSize <= BT_RF_MAX_FRAMESIZE
			// Use BT_RF_DEFAULT_FRAMESIZE if you don't care
			UInt16	maxFrameSize;
			
			// Setting advance credit to a value other then 0 causes the socket
			// to automatically advance the remote device the set amount of credit
			// upon a successful connection.  Addition credit can be advanced one a
			// connection is in place with the BtLibSocketAdvanceCredit call.
			UInt8	advancedCredit; 
		} RfComm;
			
	} data;
	
} BtLibSocketListenInfoType;

typedef struct BtLibSocketConnectInfoType {

	BtLibDeviceAddressTypePtr remoteDeviceP;
	
	union {
		struct {
			BtLibL2CapPsmType remotePsm;
			UInt16 minRemoteMtu; 
			UInt16 localMtu;
		} L2Cap;
		
		struct {
			// specifies the remote rfcomm service to conenct to. Obtianed throug SDP
			BtLibRfCommServerIdType remoteService; 
			// BT_RF_MIN_FRAMESIZE <= maxFrameSize <= BT_RF_MAX_FRAMESIZE
			// Use BT_RF_DEFAULT_FRAMESIZE if you don't care
			UInt16	maxFrameSize;
			// Setting advance credit to a value other then 0 causes the socket
			// to automatically advance the remote device the set amount of credit
			// upon a successful connection.  Addition credit can be advanced one a
			// connection is in place with the BtLibSocketAdvanceCredit call.
			UInt8 advancedCredit;
		}	RfComm;
	} data;

 
 } BtLibSocketConnectInfoType;

#pragma mark *---------Services-------------*

#define BtLibServiceNotifyType		'btsv'

typedef enum {
	btLibNotifyServiceStartup,
	btLibNotifyServiceAllShutdown, // see err for reason
	btLibnotifyServiceNotInSessionShutdown
} BtLibServiceNotifyEventType;


typedef struct _BtLibServiceNofityDetailType {
	BtLibServiceNotifyEventType	event;
	Err err;
} BtLibServiceNofityDetailType;


#endif	// __BTLIBTYPES_H
