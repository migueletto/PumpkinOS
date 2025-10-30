/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: BtLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		This file describes the API for Bluetooth communication.  The API is 
 *		asynchronous meaning that, for operations dependent on a response from 
 *      the remote device or local radio module, instead of blocking until the 
 *      operation is complete the operation is set up to complete in the 
 *      background and the API returns immediately.  When the operation is 
 *      complete Bluetooth sends an event through callbacks.  
 *      Also, events initiated from remote devices (i.e. receiving data) are 
 *      handled through callback events.
 *		Events are divided into two categories:
 *		1. Management events for ACL links and global bluetooth settings.  
 *		2. Socket events for communication via RFCOMM, L2CAP, and SDP.    
 *
 *****************************************************************************/

#ifndef __BTLIB_H
#define __BTLIB_H

#include "BtLibTypes.h"

#ifdef BTLIB_INTERNAL
#define BTLIB_TRAP(trapNum)
#else
#define BTLIB_TRAP(trapNum) SYS_TRAP(trapNum)
#endif


#ifdef __cplusplus
extern "C" {
#endif

#pragma mark *---------Library Common------*
/********************************************************************
 * Library Common
 ********************************************************************/
 
//--------------------------------------------------------------------
// Initializes the Bluetooth library.  The open call also attempts to 
// bring up the Bluetooth stack and intialize ocmmunications with the radio.
//
// btLibRefNum -> the reference number for the bluetooth library
// allowStackToFail -> if allowStackToFail is false, and the Bluetooth Lib 
//  can not initialize the Bluetooth Stack for some reason
//  (for example, it can't find a radio), the Lib show an error dialog,return a 
//  btLibErrRadioFailed error and the stack will be left closed.  Setting allowStackToFail 
//  to true will supress the error dialog, and allow the Lib to come up without the stack.
//  In this case, the open call will return btLibErrRadioFailed if the stack
//  fails to initialize, but the Library will remain open!! This can be usefull  
//  if an application only wants to use a few of the Libraries utility functions
//  and doesn't need to talk to the radio.
//
//  Any aplication that actually needs to communicate
//  with the radio must set allowStackToFail to false.
//
//	Returns:   	
//				btLibErrNoError - callback registered successfully. 
//				btLibErrBusy - the Bluetooth Library is in use by 
//						the Bluetooth serial VDRV.
//				btLibErrAlreadyOpen - The Lib is already open. Note
//						that this is not really an error, and the app
//                should still call BtLibClose when is is finished.
//				btLibErrRadioInitFailed - The stack or radio could not be initialized.
//						The library may or may not be open, based upon the value of
//					   allowStackToFail.  See the comment above.
//
// 	Events:			 
//				None
//
Err BtLibOpen( UInt16 btLibRefNum, Boolean allowStackToFail)
	BTLIB_TRAP(sysLibTrapOpen);

// closes the lib
Err BtLibClose( UInt16 btLibRefNum)
	BTLIB_TRAP(sysLibTrapClose);

// Closes existing connections, 
// save the current accessible mode
// sets mode to connectable only
Err BtLibSleep( UInt16 btLibRefNum)
	BTLIB_TRAP(sysLibTrapSleep);

// restores the accessible mode
Err BtLibWake( UInt16 btLibRefNum)
	BTLIB_TRAP(sysLibTrapWake); 

// Handles the (BtKeyChar + libEvtHookKeyMask) callback
Err BtLibHandleEvent (UInt16 btLibRefNum, void *eventP)
	BTLIB_TRAP(btLibTrapHandleEvent);

//********************************************************************
// Used by device driver to inform of Attach, Detach, Attention,
// Sleep and Wake. 
//
void BtLibHandleTransportEvent(UInt16 btLibRefNum, void *transportEventP)
	BTLIB_TRAP(btLibTrapHandleTransportEvent);
	
#pragma mark *---------Management----------*
/********************************************************************
 * Management
 * 
 * The management APIs are used for discovery, ACL links, 
 * and global bluetooth settings.  
 ********************************************************************/

//--------------------------------------------------------------------
// Registers for ME callbacks.  In general, applications
// should unregister for the Management callback before terminating.
// BtLibRegisterManagementNotification does not prevent an application
// from regitering multiple notification callbacks.
//
// btLibRefNum -> the reference number for the bluetooth library
// callbackP -> a callback proc.
// refCon -> Caller-defined data to pass to the callback handler.
//
//	Returns:   	
//				btLibErrNoError - callback registered successfully. 
//				btLibErrAlreadyRegistered - callbackP is already registered
//				btLibErrParamErroror - Invalid parameter passed in. 
//
// 	Events:			 
//				None
//
Err BtLibRegisterManagementNotification( UInt16 btLibRefNum, BtLibManagementProcPtr callbackP, UInt32 refCon)
	BTLIB_TRAP(btLibTrapRegisterManagementNotification);

//--------------------------------------------------------------------
// Unregisters for callback
//
// btLibRefNum -> the reference number for the bluetooth library
// callbackP -> the callback proc to unregister
//
//	Returns:   	
//				btLibErrNoError - callback unregistered successfully. 
//
//				btLibErrParamErroror - Invalid parameter passed in. 
//
// 	Events:			 
//				None
//
Err BtLibUnregisterManagementNotification(UInt16 btLibRefNum, BtLibManagementProcPtr callbackP)
	BTLIB_TRAP(btLibTrapUnregisterManagementNotification);		
			
//--------------------------------------------------------------------
// Starts an inquiry, devices returned through callbacks
//
// btLibRefNum -> the reference number for the bluetooth library
// timeOut -> Maximum amount of time before the Inquiry is
// 		halted, in seconds. Max is 60 Seconds. Because of the
//      constants defined within the Bt Spec, time value will be rounded down
//      to nearest multiple of 1.28 seconds. Value greater than 60 
//      will be treated as 60.
//      Pass in NULL to use the default value specified by the
//      Generic Access Profile (~10 seconds).
//
// maxResp -> The maximum number of responses. Responses may not 
//		be unique.
//
//	Returns:   	
//				btLibErrPending - The results will be returned through callbacks.  
//
//				btLibErrInProgress - An inquiry is already in process.  
//
// 	Events:			 
//    		   	btLibManagementEventInquiryResult - occurs every time a device is discovered.  
//
//				btLibManagementEventInquiryComplete - occurs when the inquiry is complete.  
//
Err BtLibStartInquiry( UInt16 btLibRefNum,  UInt8 timeOut, UInt8 maxResp)
	BTLIB_TRAP(btLibTrapStartInquiry);

//--------------------------------------------------------------------
// Cancels an inquiry process in progress
//
// btLibRefNum -> the reference number for the bluetooth library
//
//  Returns: 	
//				btLibErrPending - A callback with the event btLibManagementEventInquiryCanceled 
//  			signals the completion of this operation. 
//
//	 			btLibErrNotInProgress - No inquiry is in progress to be canceled.  
//
//				btLibErrInProgress - The inquiry is in already being cancled.
//
//				btLibErrNoError - The inquiry process was canceled
//         immediately. It actually never was started. 
//
// 	Events:
//    		   	btLibManagementEventInquiryCanceled - occurs to confirm that an inquiry has been
//				canceled.  
//
Err BtLibCancelInquiry( UInt16 btLibRefNum )
	BTLIB_TRAP(btLibTrapCancelInquiry);

//--------------------------------------------------------------------
// This blocking call performs a full discovery for an application, including name and feature retreval
// and testing.  This function takes over the UI and presents a choice box to the user, allowing
// the user to select one device from the list of devices that were discovered and meet the criteria.
//
// btLibRefNum -> the reference number for the bluetooth library
// instructionTxt -> the text displayed at teh top of the selection box, NULL for default
// deviceFilterList -> array of BtLibClassOfDeviceTypes, function checks each element in list against the remote
//            device's BtLibClassOfDeviceType value.  Any match in the list is considered
//            a success. Passing NULL skips this test.
// deviceFilterListLen -> number of elements in deviceFilterList
// selectedDeviceP <- pointer to a allocated BtLibDeviceAddressType.  Selected device addres is returned here
// addressAsName -> a debug option to show the remote devices bluetooth addresses instead of friendly names
//	showLastList -> if true, causes all other paramaters to be ignored and displays the same list as the
//           previous call to BtLibDiscoverSingleDevice.  Calling BtLibStartInquiry in between calls
//           to BtLibDiscoverSingleDevice may cause only a partial list to be displayed.
//
//  Returns: 	
//				btLibErrNoError - success
//				btLibErrCanceled - user canceled
//
// 	Events:
//				None
//
Err BtLibDiscoverSingleDevice( UInt16 btLibRefNum,  Char* instructionTxt, BtLibClassOfDeviceType* deviceFilterList,
	UInt8 deviceFilterListLen , BtLibDeviceAddressType *selectedDeviceP, Boolean addressAsName, Boolean showLastList)
 BTLIB_TRAP(btLibTrapDiscoverSingleDevice);

//--------------------------------------------------------------------
// This blocking call performs a full discovery for an application, including name and feature retreval
// and testing.  This function takes over the UI and presents a choice box to the user, allowing
// the user to select multiple devices from the list of devices that were discovered and meet the criteria.
//
// btLibRefNum -> the reference number for the bluetooth library
// instructionTxt -> the text displayed at the top of the selection box, NULL for default
// buttonTxt -> test for the done button, NULL for default
// deviceFilterList -> array of BtLibClassOfDeviceTypes, function checks each element in list against the remote
//		device's BtLibClassOfDeviceType value.  Any match in the list is considered
//		a success. Passing NULL skips this test.
// deviceFilterListLen -> number of elements in deviceFilterList
// numDevicesSelected <- pointer to a allocated UInt8.  returns number of deices selected.  App should call
//		BtLibGetSelectedDevices go get the actuall device list  
// addressAsName -> a debug option to show the remote devices bluetooth addresses instead of friendly names
//	showLastList -> if true, causes all other paramaters to be ignored and displays the same list as the
//           previous call to BtLibDiscoverMultipleDevices.  Calling BtLibStartInquiry in between calls
//           to BtLibDiscoverMultipleDevices may cause only a partial list to be displayed.
//
//  Returns: 	
//				btLibErrNoError - success
//				btLibErrCanceled - user canceled
//
// 	Events:
//				None
//
Err BtLibDiscoverMultipleDevices( UInt16 btLibRefNum,  Char* instructionTxt, Char* buttonTxt, BtLibClassOfDeviceType* deviceFilterList, 
								  UInt8 deviceFilterListLen , UInt8 *numDevicesSelected, Boolean addressAsName, Boolean showLastList)
 BTLIB_TRAP(btLibTrapDiscoverMultipleDevices);

//--------------------------------------------------------------------	
// Gets the list of devices selected during the last BtLibDiscoverMultipleDevices
//
// btLibRefNum -> the reference number for the bluetooth library
// selectedDeviceArray <-> array to place the results in
// arraySize -> number of elements in selectedDeviceArray
// numDevicesReturned <- number of results placed in selectedDeviceArray
//  Returns: 	
//				btLibErrNoError - success
//
// 	Events:
//				None
//
Err BtLibGetSelectedDevices( UInt16 btLibRefNum, BtLibDeviceAddressType* selectedDeviceArray, UInt8 arraySize, UInt8 *numDevicesReturned)
 BTLIB_TRAP(btLibTrapGetSelectedDevices);

//--------------------------------------------------------------------
// Get the remote device's name.  The bluetooth library keeps a small cache of device names.
// If retrievalMethod is not btLibRemoteOnly, this routine firsts checks that cache for a name.  
// If the name is not in the cache and the retrival method is not btLibCachedOnly, the function
// queries the remote device for its name, forming a temporary ACL connection if one is not already in place.
// If the name is in the cache, the value is returned immediately, otherwise the function returns a 
// btLibErrPending error, and the name is returned through registered callbacks
//
// btLibRefNum -> the reference number for the bluetooth library
// remoteDevice -> the address of the device who's name is desired
// nameP -> pointer to name structure to be filled in by the call.  Must allocate enough memory
// 	for the name and the null terminator (249 bytes is the max possible)
// retrievalMethod -> get the name from the cache or the remote device
//
//  Returns: 	
//				btLibErrPending - The results will be returned through a callback. 
//
//				btLibErrBusy - There is already a name request pending
//
//				btLibErrNoError - Name structure was filled in from cache successfully. No callback
//						will be returned
//					 
//
// 	Events:
//				btLibManagementEventNameResult - This event with a status equal to btLibErrNoError
//				signals that the friendly name was successfully filled in.  
//				
//
Err BtLibGetRemoteDeviceName( UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP, 
	BtLibFriendlyNameType* nameP, BtLibGetNameEnum retrievalMethod)
 BTLIB_TRAP(btLibTrapGetRemoteDeviceName);

//--------------------------------------------------------------------
// Attempts to create an ACL link to a remote device.  In order to create links to
// more than one device. you must call BtLibPiconetCreate.   Outbound connects 
// allow the master slave switch when you have not called BtLibPiconetCreate.
//
// btLibRefNum -> the reference number for the bluetooth library
// remoteDeviceP -> a remote device address 
//
//  Returns: 	
//				btLibErrPending - The results will be returned through a callback.
//				btLibErrAlreadyConnected - The connection is already in place (no event returned)
//				btLibErrTooMany - Reached maximum number of ACL Links allowed. 
//
// 	Events:
//				btLibManagementEventACLConnectOutbound - This event with a status equal to 0 signals
//				that the ACL link is up.  
//
Err BtLibLinkConnect( UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP)
 BTLIB_TRAP(btLibTrapLinkConnect);

//--------------------------------------------------------------------
// Disconnect ACL Link. Result returned through callback event. 
//
// btLibRefNum -> the reference number for the bluetooth library
// remoteDeviceP -> the address of the remote device
//
//  Returns: 	
//				btLibErrPending - The results will be returned through a callback.
//          btLibErrNoError - A connection attempt was canceled before completing.
//          	No event will be generated.
//				btLibErrNoAclLink - No link exists to disconnect. (No event generated) 
//
// 	Events:
//				btLibManagementEventACLDisconnect - This event signals that the link has 
//				disconnected.  The error field of the event contains the reason the link
//				was disconnected.
//
Err BtLibLinkDisconnect( UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP)
 BTLIB_TRAP(btLibTrapLinkDisconnect);

//--------------------------------------------------------------------
// Set up to be the master of a piconet. Set up initial state for whether
// to allow inbound connections or not.      
//
// btLibRefNum -> the reference number for the bluetooth library
// unlockInbound -> If true the piconet will accept inbound connections. This
// 	initializes the piconet to the unlocked state. A value of false initializes
//		the piconet to the locked state which only allows outbound connections. 
// discoverable -> unlockInbound must be true or this parameter is ignored.
//		If true the radio will be set to respond to inquiries.  If false the 
//		radio is just connectable. 
//
//  Returns: 	
//				btLibErrNoError - Successfully configured to be master.
//				btLibErrPending - This happens when there is a pre-existing ACL link
//					because the link needs its link policy changed and may need a role
//					switch.
//				btLibErrFailed - A piconet already exists.   
//
// 	Events:
//				btLibManagementEventPiconetCreated - Piconet created. A non-zero status
//					gives the reason for failure.  
//
//				btLibManagementEventAccessibilityChange - If the radio is discoverable
//					and accessable then this event will let you know that the radio
//				 	is no longer discoverable or connectable. 
// 
Err BtLibPiconetCreate(UInt16 btLibRefNum, Boolean unlockInbound, Boolean discoverable)
 BTLIB_TRAP(btLibTrapPiconetCreate);

//--------------------------------------------------------------------
// Disconnect links to all devices.  Remove all retrictions on our master/slave
// role.   
//
// btLibRefNum -> the reference number for the bluetooth library
//
//  Returns: 	
//				btLibErrPending - Destroying the piconet.
//				btLibErrNoError - Piconet destroyed.  
//				btLibErrNoPiconet - Piconet does not exist.
//
// 	Events:
//				btLibManagementEventACLDisconnect - This event signals that the link has 
//				disconnected.
//
//				btLibManagementEventPiconetDestroyed - This event signals that the piconet has been
//				completely destroyed.  
//
Err BtLibPiconetDestroy(UInt16 btLibRefNum)
	BTLIB_TRAP(btLibTrapPiconetDestroy);

//--------------------------------------------------------------------
// Accept inbound ACL links into the piconet.  Allowing inbound connections
// lowers the bandwidth availble for data transmission within the current
// members of the piconet because the radio periodically has to scan for 
// incoming links.    
//
// btLibRefNum -> the reference number for the bluetooth library
// discoverable -> If true the radio will be set to respond to 
//		inquiries.  If false the radio is just connectable.  
//
//  Returns: 	
//				btLibErrNoError - Devices can be added to the piconet.  
//				btLibErrNoPiconet - BtLibPicoCreate has not been succesfully called. 
//
// 	Events:
//				btLibManagementEventAccessibilityChange - If allowInbound is true then
//					this event informs you that you are discoverable and connectable
//
Err BtLibPiconetUnlockInbound(UInt16 btLibRefNum, Boolean discoverable)
 BTLIB_TRAP(btLibTrapPiconetUnlock);

//--------------------------------------------------------------------
// Don't allow remote devices to connect into the piconet. (Outbound 
// connections are still allowed).  When a piconet is locked bandwith 
// is maximized for data transmission between the members of the piconet.  
//
// btLibRefNum -> the reference number for the bluetooth library
//
//  Returns: 	
//				btLibErrNoError - Devices cannnot be added to the piconet.
//				btLibErrNoPiconet - BtLibPicoCreate has not been succesfully called. 
//
// 	Events:
//				btLibManagementEventAccessibilityChange - If the radio is discoverable
//					and accessable then this event will let you know that the radio
//				 	is no longer discoverable or connectable.  
//
Err BtLibPiconetLockInbound(UInt16 btLibRefNum)
 BTLIB_TRAP(btLibTrapPiconetLock);

//--------------------------------------------------------------------
// Set the state of an ACL link
//
// btLibRefNum -> Bluetooth library reference number. 
// remoteDevice -> the address of the remote device (identifies ACL link).
// pref -> link preference to set. 
// linkState -> value corresponding to preference. 
// linkStateSize -> size in bytes of linkState.
// 
// 	pref:											linkState:
// 	btLibLinkPref_Authenticated			ignored, size is also ignored
// 	btLibLinkPref_Encrypted					Boolean  (Note: link must first be authenticated)
//  
//		Returns: 	
//				btLibErrPending - The results will be returned through a callback.
//				btLibErrFailed  - You must first authenticate a link before you can 
//					encrypt it.  
//
// 	Events:
//				btLibManagementEventAuthenticationComplete - Signals that link authentication request has completed. 
//
//				btLibManagementEventEncryptionChange - Signals encryption change.  
//
Err BtLibLinkSetState( UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP, BtLibLinkPrefsEnum pref, void* linkState, UInt16 linkStateSize)
 BTLIB_TRAP(btLibTrapLinkSetState);

//--------------------------------------------------------------------
// Get the state of an ACL link
//
// btLibRefNum -> Bluetooth library reference number. 
// remoteDevice -> the address of the remote device (identifies ACL link).
// pref -> link preference to get (see LINK PREFERENCES comment below).
// linkState <- memory to store linkState value (see LINK PREFERENCES comment below).
// linkStateSize <- size in bytes of linkState.
// 
// 	pref:											linkState:
// 	btLibLinkPref_Authenticated			Boolean
// 	btLibLinkPref_Encrypted					Boolean
// 	btLibLinkPref_LinkRole					BtLibConnectionRoleEnum 
//  
//  Returns: 	
//				btLibErrNoError - the linkState variable has been filled in.  
//
// 	Events:
//				None
//
Err BtLibLinkGetState( UInt16 btLibRefNum,  BtLibDeviceAddressTypePtr remoteDeviceP, BtLibLinkPrefsEnum pref, void* linkState, UInt16 linkStateSize)
 BTLIB_TRAP(btLibTrapLinkGetState);				 

//--------------------------------------------------------------------
// Set the general management preferences
//
// btLibRefNum -> Bluetooth library reference number. 
// pref -> management preference to set.
// prefValue -> value corresponding to preference.
// prefValueSize -> size in bytes of prefValue
// 
// pref:										prefValue:
// btLibPref_Name							BtLibFriendlyNameType		// In general, only the OS should set the device name
// btLibPref_UnconnectedAccessible	BtLibAccessiblityInfoType  // In general, only the OS should set the unconnected accessible mode
// btLibPref_LocalClassOfDevice		BtLibClassOfDeviceType 		// In general, only the OS should set btLibPref_LocalClassOfDevice
//
//  Returns: 	
//				btLibErrNoError - Preference set.  
//				
//				btLibErrPending - The results will be returned through a callback.
//
// 	Events:
//				btLibManagementEventAccessibilityChange - Signals that accessibility has been changed 
//
//				btLibManagementEventLocalNameChange - Signals that name has been changed.
//
Err BtLibSetGeneralPreference( UInt16 btLibRefNum, BtLibGeneralPrefEnum pref, void* prefValue, UInt16 prefValueSize)
 BTLIB_TRAP(btLibTrapSetGeneralPreference);


//--------------------------------------------------------------------
// Get the general management preferences
//
// btLibRefNum -> Bluetooth library reference number. 
// pref -> management preference to get. 
// prefValue -> memory to store prefValue. 
// prefValueSize -> size in bytes of prefValue.
// 
// pref:										prefValue:
// btLibPref_LocalName					BtLibFriendlyNameType
// btLibPref_UnconnectedAccessible	BtLibAccessibleModeEnum 
// btLibPref_CurrentAccessible		BtLibAccessibleModeEnum	
// btLibPref_LocalClassOfDevice		BtLibClassOfDeviceType	
// btLibPref_LocalDeviceAddress		BtLibDeviceAddressType		
//
//  Returns: 	
//				btLibErrNoError - Preference stored in prefValue.  
//
// 	Events:
//				None
//
Err BtLibGetGeneralPreference( UInt16 btLibRefNum, BtLibGeneralPrefEnum pref, void* prefValue, UInt16 prefValueSize)
 BTLIB_TRAP(btLibTrapGetGeneralPreference);

#pragma mark *---------Sockets--------------*
/********************************************************************
 * Sockets
 *
 * The sockets API is used to manage RFCOMM, L2CAP, and SDP 
 * communications.
 ********************************************************************/

//-------------------------------------------------------------------- 
// Create a socket.  In general, applications should detroy
// all sockets before terminating.
//	
// btLibRefNum -> the reference number for the bluetooth library
// socketP <- the returned socket value
// callbackP -> the socket callback proc.
// refCon -> Caller-defined data to pass to the callback handler.
// socketProtocol -> the protocol (L2CAP, RFComm, or SDP) to associate with this socket
//
//  Returns:	
//				btLibErrNoError - Indicates that the socket was created.
//
//				btLibErrOutOfMemory - Not enough memory to create socket.
//
//				btLibErrTooMany - Failed because reached maximum number of sockets 
//				allocated for system.
//
// 	Events: 
//				None
// 
Err BtLibSocketCreate( UInt16 btLibRefNum,  BtLibSocketRef* socketRefP, BtLibSocketProcPtr callbackP, 
							   UInt32 refCon,BtLibProtocolEnum socketProtocol)				
	BTLIB_TRAP(btLibTrapSocketCreate);

//--------------------------------------------------------------------
// Close a socket, free's associated resources and kills connections
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> the socket to close
//
//  Returns:	
//				btLibErrNoError - Socket closed successfully.
//				btLibErrSocket - Invalid socket passed in.  
//
// 	Events:
//				None
//		
Err BtLibSocketClose( UInt16 btLibRefNum,  BtLibSocketRef socket)
BTLIB_TRAP(btLibTrapSocketClose);

//--------------------------------------------------------------------
// Sets up an L2Cap or RfComm socket as a listener
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a listener socket
// listenInfo -> protocol specific listening information
//
//	Returns:	
//				btLibErrNoError - Socket listening for incoming connections.  
//				btLibErrSocketPsmUnavailable - the given PSM is in use (L2CAP only)
//				btLibErrTooMany - There are no resources to create a listener sock of this type
// 	Events:
//				btLibSocketEventConnectRequest - Once a socket is set to listen, when a remote 
//				device initiates a connection then this event is sent.  This event will set the
//				listener socket up to respond to the correct RfComm or L2cap channel.  You must 
//				respond to this callback with a call to BtLibSocketRespondToConnection on the 
//				listener socket to accept or reject the connection.  
//
Err BtLibSocketListen( UInt16 btLibRefNum,  BtLibSocketRef socket, BtLibSocketListenInfoType *listenInfo)
 BTLIB_TRAP(btLibTrapSocketListen);

//--------------------------------------------------------------------
// Create an outbound L2Cap or RfComm connection
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a socket
// listenInfo -> Bluetooth device address and protocol specific connection information
//
//	Returns:	
//				btLibErrPending - The results will be returned through a callback.
//
//				btLibErrNoAclLink - ACL link for remote device does not exist
//
// 	Events:
//				btLibSocketEventConnectedOutbound - This event with a status of zero signals success of the
//				connections.  A non-zero status gives a reason for failure.  
//	
//				btLibSocketEventDisconnected - If the connection fails or if connection establishment is 
//				 successful then this event can occur when the channel disconnects.
//
//				btLibSocketEventData - If connection establishment is successful then 
//				this event can occur if the remote device sends data.  
//		
Err BtLibSocketConnect( UInt16 btLibRefNum,  BtLibSocketRef socket, BtLibSocketConnectInfoType* connectInfo)
 BTLIB_TRAP(btLibTrapSocketConnect);

//--------------------------------------------------------------------
// Accept or reject an in-bound connection on a given listener socket.  Called in response 
// to btLibSocketEventConnectRequest event devlivered to a listener socket.  
// A New connection socket is returned in btLibSocketEventConnectedInbound callback 
// to the listener socket after BtLibSocketRespondToConnection is called.
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a listener socket.
// accept -> TRUE means accept connection.  FALSE means reject connection.
//
//  Returns: 					
//				btLibErrPending - The results will be returned through a callback.
//				
//				btLibErrSocketRole - the socket passed in is not Listening.  
//
//				btLibErrFailed - Listener socket does not have a pending connection.  If this
//					error occurs it means that the function has been called with the wrong listener,
//				 	or that this function was called even though no btLibSocketEventConnectRequest 
//					callback occured for the socket.  
//
// 	Events:
//				btLibSocketEventConnectedInbound - the connection was made.  contains 
//					the reference for the new connection socket (which will use the same callback
//					as the listener)
//				btLibSocketEventDisconnected - the connection failed, or the rejection is complete. 
//				
//	
Err BtLibSocketRespondToConnection( UInt16 btLibRefNum,  BtLibSocketRef socket, Boolean accept)
 BTLIB_TRAP(btLibTrapSocketRespondToConnection);

//--------------------------------------------------------------------	
// Send data over a connected L2Cap or RFCOMM socket.  dataLen must be less then the mtu
// for the socket.  Completion returned through callback.
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a connection socket
// data -> Buffer containing the data to send. If this function returns btLibErrPending, the
//         contents of the buffer must not be modified until the btLibSocketEventSendComplete occurs.
// dataLen -> Length of data to send.
//
//	Returns:	
//				btLibErrPending - The results will be returned through a callback.
//
//				btLibErrBusy - A send is already in process.  
//
//				btLibErrNoAclLink - ACL link for remote device does not exist
//
//				btLibErrSocketRole - Socket is not connected.  
//
// 	Events:
//				btLibSocketEventSendComplete - This event, with a status of 0, signals 
//				that the data has been successfully transmitted.  
//
Err BtLibSocketSend( UInt16 btLibRefNum, BtLibSocketRef socket, UInt8 *data, UInt32 dataLen)
 BTLIB_TRAP(btLibTrapSocketSend);
 
 //--------------------------------------------------------------------	
// RFCOMM uses a credit based flow control mechanism.  This function advances
// credit on a given RFCOMM connectiontion socket.  Multiple calls to this function
// have a cummulative effect. 
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> an RFCOMM connection socket
// credit -> Each credit value represents one RFCOMM packet.  Advancing n
//   credits allows the remote device to send n packets.  Once those
//   packets have been sent, the remote device can no longer send (flow
//   is off).  Subsequent calls to RF_AdvanceCredit() will allow the
//   remote device to send again (flow is on).  Credits are additive, 
//   so calling this function once with 3 credits and then with 2 credits 
//   will grant a total of 5 credits to the remote device, allowing the
//   remote device to send 5 RFCOMM packets.
//
//	Returns:	
//				btLibErrNoError - success
//				btLibErrFailed - to many credits advanced   
//				btLibErrSocket - bad socket value
//				btLibErrSocketProtocol - socket is not an RFCOMM socket
//				btLibErrSocketRole - socket is not a connection socket
//
Err BtLibSocketAdvanceCredit( UInt16 btLibRefNum, BtLibSocketRef socket, UInt8 credit)
BTLIB_TRAP(btLibTrapSocketAdvanceCredit);

//--------------------------------------------------------------------
// Get socket information
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a socket
// infoType -> type of info to get (see SOCKET INFO comment below).
// valueP <- memory to store result (see SOCKET INFO comment below).
// valueSize -> size in bytes of memory for ValueP
//
// btLibSocketInfo_Protocol					BtLibProtocolEnum*
// btLibSocketInfo_RemoteDeviceAddress		BtLibDeviceAddressType*
// 
// btLibSocketInfo_SendPending				Boolean*
// btLibSocketInfo_MaxTxSize					UInt32*
// btLibSocketInfo_MaxRxSize					UInt32*
//
// The MaxTxSize and MaxRxSize will be the same except in the case where RFCOMM is
// using credit based flow control.  In that case MaxRxSize will be equal to the negotiated
// frame size and the MaxTxSize will equal MaxRxSize - 1.  We transmit packets with credit 
// information which takes one byte away from the maximum frame size available for transmit.  
// However, the spec does not require that the credit information is always included so it is
// possible that a non-palm implementation of RFCOMM could send a packet where the byte we 
// we reserve for credit information is used for additional data.  
//
// btLibSocketInfo_L2CapPsm 					BtLibL2CapPsmType*
//
// For an outbound L2Cap socket the psm returned represents the psm used to connect to the
// remote device.  For an inbound L2Cap socket the psm returned represents the psm remote 
// devices would use to connect to us.
//
// btLibSocketInfo_L2CapChannel 				BtLibL2CapChannelIdType*
//
// btLibSocketInfo_RfCommServerId			BtLibRfCommServerIdType*
// btLibSocketInfo_RfCommOutstandingCredits UInt16*
//
// The RFCOMM Service ID is only valid for RFCOMM listener sockets.
//
// btLibSocketInfo_SdpServiceRecordHandle	BtLibSdpRemoteServiceRecordHandle*
//	Returns:	
//				btLibErrNoError - success, results are placed in valueP.
//				btLibErrSocket - the socket is not in use
//				btLibErrSocketRole - the socket has the wrong role for the request
//				btLibErrSocketProtocol - the socket has the wrong protocol for the request
//				btLibErrParamError - an invalid param was passed in. 
// 	Events:
//				None
//
Err BtLibSocketGetInfo( UInt16 btLibRefNum,  BtLibSocketRef socket, BtLibSocketInfoEnum infoType, 
	void * valueP, UInt32 valueSize)
 BTLIB_TRAP(btLibTrapSocketGetInfo);
	

 #pragma mark *---------SDP-----------------*
/********************************************************************
 * SDP
 *
 * The SDP API is used create and advertise service records to remote
 * devices and to discover services available on remote devices.  Only 
 * one outstanding query at a time is allowed per socket.  
 ********************************************************************/
 
//--------------------------------------------------------------------
// Get available RFCOMM server channel for a UUID service list
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> an SDP socket
// rDev ->  remote device to query
// serviceUUIDList -> List of UUIDs for service record.  All UUIDs in
// 	the serviceUUIDList must be present in the remote record for a match.  
// uuidListLen -> Length of serviceUUIDList.  Max of 12.  
//
//  Returns:	
//				btLibErrPending - The results will be returned through a callback.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrSocket - Invalid socket passed into function.
//				btLibErrSocketRole - Socket has incorrect role.
//				btLibErrSocketProtocol - Must pass in an SDP socket.
//				btLibErrOutOfMemory - Could not get memory to perform the query.
//				btLibErrParamError - Invalid parameter passed into function.
//
//   Events:
//				btLibSocketEventSdpGetServerChannelByUuid - This event with
// 			a status of btLibErrNoError signals that serverChannel has been filled in.  
//          If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not 
//				complete successfully.  
//
Err BtLibSdpGetServerChannelByUuid( UInt16 btLibRefNum, BtLibSocketRef socket, 
	 BtLibDeviceAddressType *rDev, BtLibSdpUuidType* serviceUUIDList, UInt8 uuidListLen)
	BTLIB_TRAP(btLibTrapSdpGetGetServerChannelByUuid);
	
//--------------------------------------------------------------------
// Get available L2Cap psm for a UUID service list
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> an SDP socket
// rDev ->  remote device to query
// serviceUUIDList -> List of UUIDs for service record.  All UUIDs in
// 	the serviceUUIDList must be present in the remote record for a match.
// uuidListLen -> Length of serviceUUIDList.  Max of 12.  
//
//  Returns:	
//				btLibErrPending - The results will be returned through a callback.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrSocket - Invalid socket passed into function.
//				btLibErrSocketRole - Socket has incorrect role.
//				btLibErrSocketProtocol - Must pass in an SDP socket.
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrParamError - Invalid parameter passed into function.
//
// 	Events:
//				btLibSocketEventSdpGetPsmByUuid - - This event with
// 			a status of btLibErrNoError signals that psm has been filled in.  
//          If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not 
//				complete successfully.  
//
Err BtLibSdpGetPsmByUuid( UInt16 btLibRefNum, BtLibSocketRef socket, 
	 BtLibDeviceAddressType *rDev, BtLibSdpUuidType* serviceUUIDList, UInt8 uuidListLen)
	BTLIB_TRAP(btLibTrapSdpGetPsmByUuid);	
 
//--------------------------------------------------------------------
// Create an SDP Record.
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH <- Handle to the newly created service record.  	
//
//  Returns:	
//				btLibErrNoError - Indicates that the SDP Record was created.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrOutOfMemory - Could not get memory to create the record.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordCreate( UInt16 btLibRefNum, BtLibSdpRecordHandle* recordH )
	BTLIB_TRAP(btLibTrapSdpServiceRecordCreate);

//--------------------------------------------------------------------
// Destroy an sdp record (free's memory)
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> Sdp record to be destroyed.  	
//
//  Returns:	
//				btLibErrNoError - Indicates that the SDP Record was destroyed.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordDestroy( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH)
	BTLIB_TRAP(btLibTrapSdpServiceRecordDestroy);

//--------------------------------------------------------------------
// Advertises a service record so that remote devices can query it.
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> SDP record not currently advertised. 
//
//  Returns:	
//				btLibErrNoError - Indicates that the SDP Record is being advertised.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrSdpFormat - Sdp record is improperly formatted.
//				btLibErrSdpRemoteRecord - Operation not valid on remote record.
//				btLibErrSdpAdvertised - Record is already advertised.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordStartAdvertising( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH)
	BTLIB_TRAP(btLibTrapSdpServiceRecordStartAdvertising);

//--------------------------------------------------------------------
// Stop advertising an Sdp record
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> SDP record that is currently advertised. 
//
//  Returns:	
//				btLibErrNoError - Indicates that the SDP Record is no longer advertised.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrSdpNotAdvertised - Record was not advertised.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordStopAdvertising( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH)
	BTLIB_TRAP(btLibTrapSdpServiceRecordStopAdvertising);
	
//--------------------------------------------------------------------
// Sets up a basic Sdp record for L2Cap and RFCOMM listener sockets
//
// btLibRefNum -> the reference number for the bluetooth library
// socket -> a RFCOMM or L2CAP socket in listening mode
// serviceUUIDList -> List of UUIDs for service record
// uuidListLen -> Length of serviceUUIDList.  Max of 12.  
// serviceName -> Name of the service (English only)
// serviceNameLen -> Length of serviceName.  
// recordH -> The service record to be set up   	
//
//  Returns:	
//				btLibErrNoError - Indicates that the SDP Record was set up.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrSdpRemoteRecord - Operation not valid on remote record.
//				btLibErrSdpAdvertised - Operation not valid on advertised record. 
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrSocket - Invalid socket passed into function.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordSetAttributesForSocket( UInt16 btLibRefNum,   BtLibSocketRef socket, 
	BtLibSdpUuidType* serviceUUIDList, UInt8 uuidListLen, const Char* serviceName, UInt16 serviceNameLen, 
	BtLibSdpRecordHandle recordH)
	BTLIB_TRAP(btLibTrapSdpServiceRecordSetAttributesForSocket);
	
//--------------------------------------------------------------------
// Get the service record handle(s) for service classes advertised on a remote device
// 
// btLibRefNum -> the reference number for the bluetooth library
// socket -> an SDP socket
// rDev ->  remote device to query
// uuidList -> list of uuids identifying service classes that the remote service should support.
//		All UUIDs in the serviceUUIDList must be present in the remote record for a match.
// uuidListLen -> number of elements in the uuidList. Max value is 12.	
// serviceRecordList <- array to store results of query
// numSrvRec <-> number of service records serviceRecordList can store. This value is sent to 
//              the SDP server so it can limit the number of responses.  This value is set
//					 upon receiving btLibSocketEventSdpServiceRecordHandle to the actual number of 
//					 record handles received.  
//
//  Returns:	
//				btLibErrPending - The results will be returned through a callback.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrSocket - Invalid socket passed into function.
//				btLibErrSocketProtocol - Must pass in an SDP socket.
//				btLibErrInProgress - Query already pending on this socket.  
//				btLibErrNoAclLink - No ACL link in place to remote device.
//				btLibErrOutOfMemory - Could not get memory to do the query. 
//				btLibErrBusy - Failed because connection is parked.  
//
// 	Events:
//				btLibSocketEventSdpServiceRecordHandle - This event with a status of btLibErrNoError 
//				signals that the serviceRecordList and numSrvRec has been filled in with the SDP 
//				response results.  If the status is btLibErrNoError then the associated event data
//				is valid, otherwise the eventData is not valid because the SDP operation did not
//				complete successfully.  
//
Err BtLibSdpServiceRecordsGetByServiceClass( UInt16 btLibRefNum, BtLibSocketRef socket, 
	BtLibDeviceAddressType *rDev, BtLibSdpUuidType* uuidList,  UInt16 uuidListLen, 
	BtLibSdpRemoteServiceRecordHandle *serviceRecordList, UInt16 *numSrvRec)
	BTLIB_TRAP(btLibTrapSdpServiceRecordsGetByServiceClass);

//--------------------------------------------------------------------
// Associate an SDPRecord with a socket and a remote device so that 
// BtLibSdpServiceRecordGetAttribute() can be called to get remote attributes. 
//
// btLibRefNum -> the reference number for the bluetooth library 
// socket -> an SDP socket
// rDev -> device to query
// remoteHandle -> remote service record handle
// recordH -> an empty SDP record.  	
//
//  Returns:	
//				btLibErrNoError - Indicates that the mapping was successful.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrParamError - Invalid parameter passed into function.
//				btLibErrSocket - Invalid socket passed into function.
//				btLibErrSocketProtocol - Must pass in an SDP socket.
//				btLibErrSdpMapped - socket or SDP record is already mapped.
//				btLibErrOutOfMemory - Could not get memory to do the mapping. 
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordMapRemote( UInt16 btLibRefNum, BtLibSocketRef socket, 
	BtLibDeviceAddressType* rDev, BtLibSdpRemoteServiceRecordHandle remoteHandle, 
	BtLibSdpRecordHandle recordH)
	BTLIB_TRAP(btLibTrapSdpServiceRecordMapRemote);

//--------------------------------------------------------------------	
// Set a specific attribute type's value for a given record.  This function only works
// on sdp records that are local and not currently advertised.  
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> SDP record
// attribute -> type of attribute to set
// attributeValues -> attributes that correspond to attribute type
// listNumber -> identifies which list to use (usually 0)  Ignored for non 
//		ProtocolDescriptorListEntry attributes.  	
// listEntry -> The item to get in the list.  Ignored for non list atttributes.  	 	
//
//  Returns:	
//				btLibErrNoError - Indicates that the attribute value was set successfully.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrSdpRemoteRecord - Operation not valid on remote record.
//				btLibErrSdpAdvertised - Operation not valid on advertised record.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordSetAttribute( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, BtLibSdpAttributeDataType *attributeValues,  UInt16 listNumber, 
	UInt16 listEntry)
	BTLIB_TRAP(btLibTrapSdpServiceRecordSetAttribute);

//--------------------------------------------------------------------
// Get a specific attribute type's value for a given record 
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> SDP record
// attribute -> type of attribute to get
// attributeValues <- space for attribute value to be received. 
// listNumber -> identifies which list to use (usually 0)  Ignored for non 
//		ProtocolDescriptorListEntry attributes.  	  
// listEntry -> The item to get in the list.  Ignored for non list atttributes.  	
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetAttribute - This event with a status of btLibErrNoError 
//				signals that attributeValues has been filled in with the SDP response results.  
//				If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not
//				complete successfully.  
//
Err BtLibSdpServiceRecordGetAttribute( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, BtLibSdpAttributeDataType* attributeValues, UInt16 listNumber, 
	UInt16 listEntry)
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetAttribute);

//--------------------------------------------------------------------	
// Get the size of a string or URL type of atttribute for a given record
//
// btLibRefNum -> the reference number for the bluetooth library
// recordH -> SDP record
// attribute -> type of attribute to get size for
// size <-  size of attribute.  
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the size of the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetStringLen - This event with a status of btLibErrNoError 
//				signals that size variable has been filled in with the SDP response results.  
//				If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not
//				complete successfully.  
//
Err BtLibSdpServiceRecordGetStringOrUrlLength( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, UInt16* size)
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetStringOrUrltLength);

//--------------------------------------------------------------------
// Get the number of entries in an atribute list
// valid for: ServiceClassIdListEntry, ProtocolDescriptorListEntry, BrowseGroupListEntry, 
// LanguageBaseAttributeIDListEntry and ProfileDescriptorListEntry
//
// btLibRefNum -> the reference number for the bluetooth library.
// recordH -> SDP record.
// attribute -> type of attribute to get number of list entries for.
// listNumber -> identifies which list to use (usually 0)  Ignored for non 
//		ProtocolDescriptorListEntry attributes.  	  
// numEntries <-  size of attribute.  
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the size of the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetNumListEntries - This event with a status of btLibErrNoError 
//				signals that numEntries variable has been filled in with the SDP response results.  
//				If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not
//				complete successfully.  
//
Err BtLibSdpServiceRecordGetNumListEntries( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, UInt16 listNumber, UInt16 *numEntries )
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetNumListEntries);

//--------------------------------------------------------------------
// Get the number of lists since there can be more than one protocol descriptor
// list in an SDP record.  
// valid for: ProtocolDescriptorListEntry
//
// btLibRefNum -> the reference number for the bluetooth library.
// recordH -> SDP record.
// attribute -> type of attribute to get number of lists for.
// numLists <-  number of lists.  
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the size of the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetNumLists - This event with a status of btLibErrNoError 
//				signals that numLists variable has been filled in with the SDP response results.  
//				If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not
//				complete successfully.  
//
Err BtLibSdpServiceRecordGetNumLists( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, UInt16 *numLists )
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetNumLists);

//--------------------------------------------------------------------	
// Compare two UUIDs
//
// btLibRefNum -> the reference number for the bluetooth library.
// uuid1 -> a uuid
// uuid2 -> another uuid
//
//  Returns:	
//				btLibErrNoError - UUIDs match
//				btLibErrError - UUIDs do not match
//				btLibErrParamError - Invalid parameter passed into function. 
//
// 	Events:
//				None
//
Err BtLibSdpCompareUuids(UInt16 btLibRefNum, BtLibSdpUuidType *uuid1, BtLibSdpUuidType *uuid2)
	BTLIB_TRAP(btLibTrapSdpCompareUuids);

#pragma mark *--------Raw SDP----------*
// Raw SDP APIs - If you have a profile or application specific SDP attribute then you
// will need to use the Raw SDP APIs to form and parse SDP protocol data yourself. (Refer
// to the bluetooth 1.1 specification to see how SDP data is formatted.) 
//

//--------------------------------------------------------------------	
// Set the value for any attribute type for a given record.  This function only works
// on SDP records that are local and not currently advertised.  
//
// btLibRefNum -> the reference number for the bluetooth library.
// recordH -> SDP record.
// attributeID -> type of attribute to set.
// value -> raw SDP attribute data (as would appear in raw SDP protocol data).
// valSize -> size in bytes of value.   	
//
//  Returns:	
//				btLibErrNoError - Indicates that the attribute value was set successfully.
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrSdpRemoteRecord - Operation not valid on remote record.
//				btLibErrSdpAdvertised - Operation not valid on advertised record.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//
// 	Events:
//				None
//
Err BtLibSdpServiceRecordSetRawAttribute( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, const UInt8* value, UInt16 valSize)
	BTLIB_TRAP(btLibTrapSdpServiceRecordSetRawAttribute);

//--------------------------------------------------------------------
// Get the value for any attribute type for a given record
//
// btLibRefNum -> the reference number for the bluetooth library.
// recordH -> SDP record.
// attributeID -> type of attribute to get.
// value <- raw SDP attribute data (as would appear in raw SDP protocol data).
// valSize <-> pointer to size of value buffer (in) pointer to number of bytes in attribute (out).   	
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetRawAttribute - This event with a status of btLibErrNoError 
//				signals that value and valSize variables has been filled in with the SDP response
//				results.  If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not complete 
//				successfully.  
//
Err BtLibSdpServiceRecordGetRawAttribute( UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, UInt8* value, UInt16* valSize)
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetRawAttribute);

//--------------------------------------------------------------------	
// Get the size of any attribute type for a given record
//
// btLibRefNum -> the reference number for the bluetooth library.
// recordP -> SDP record.
// attributeID -> type of attribute to get the size of.
// size <- size of attribute corresponding to attributeID.	
//
//  Returns:	
//				btLibErrNoError - Indicates that getting the attribute value was successful.
//				btLibErrSdpAttributeNotSet - Attribute is not in record.  
//				btLibErrNotOpen - BtLib needs to be open to make this call.
//				btLibErrPending - For remote records the data may need to be retrieved from the 
//					remote device.  The results will be returned through a callback.
//				btLibErrNoAclLink - ACL link for remote device does not exist.
//				btLibErrParamError - Invalid parameter passed into function. 
//				btLibErrOutOfMemory - Could not get memory to perform the operation.
//				btLibErrInProgress - Query already pending on this socket.  (remote only)
//				btLibErrBusy - Failed because connection is parked.  (remote only)
//
// 	Events:
//				btLibSocketEventSdpGetRawAttributeSize - This event with a status of btLibErrNoError 
//				signals that the size variable has been filled in with the SDP response
//				results.  If the status is btLibErrNoError then the associated event data is valid, 
//				otherwise the eventData is not valid because the SDP operation did not complete 
//				successfully.  
//
Err BtLibSdpServiceRecordGetSizeOfRawAttribute(UInt16 btLibRefNum, BtLibSdpRecordHandle recordH, 
	BtLibSdpAttributeIdType attributeID, UInt16* size)
	BTLIB_TRAP(btLibTrapSdpServiceRecordGetSizeOfRawAttribute);
	
//--------------------------------------------------------------------	
// Determines the offset and number of bytes in a Data Element. Note that
// offset plus length is the total size of the Data Element.
//
// btLibRefNum -> the reference number for the bluetooth library.
// value -> raw SDP attribute data.
// offsetP <- stores the offset from value to the start of the data 
// 	elements data field.  
// lengthP <- stores length of data element.	
//
//  Returns:	
//				btLibErrNoError - successfully parsed the attribute.
//				btLibErrParamError - Invalid parameter passed into function. 
//
// 	Events:
//				none
//
Err BtLibSdpParseRawDataElement(UInt16 btLibRefNum, const UInt8* value, UInt16 *offset, 
	UInt32 *length)
	BTLIB_TRAP(btLibTrapSdpParseRawDataElement);

//--------------------------------------------------------------------	
// Verifies that a raw Sdp Data Element is properly formed. All size descriptors
// in the element are checked to make sure that the complete Data Element
// fits into the indicated length.
//
// In the case of Data Element Sequences or Alternates, this
// function is called recursively. The parameter "maxLevel" is used
// to stop the recursion in the case of bad data. "maxLevel" must
// be large enough to handle the complete Data Element. For example,
// a simple data element such as btLibDETD_UINT would need a "maxLevel" of 1,
// but a Data Element Sequence of UUIDs would need a "maxLevel" of 2.
//
// btLibRefNum -> the reference number for the bluetooth library.
// value -> raw SDP attribute data.
// valSize -> size of value in bytes. The size of the Data Element must
// 	be less than or equal to this parameter or this function will fail. 
// maxLevel -> The maximum level of recursion. Must be at least 1.	
//
//  Returns:	
//				btLibErrNoError - Sdp Data is properly formatted.  		
//				btLibErrError - Sdp Data is not properly formatted.
//				btLibErrParamError - Invalid parameter passed into function. 	
//
// 	Events:
//				none
//
Err BtLibSdpVerifyRawDataElement(UInt16 btLibRefNum, const UInt8 *value, UInt16 valSize, 
	UInt8 maxLevel)
	BTLIB_TRAP(btLibTrapSdpVerifyRawDataElement);

//---------------------------------------------------------------------------
// Returns a Data Element's type.
//
// header -> The first byte of a Data Element
//
//  Returns:
//     		The type of the Data Element.
//
#define  BtLibSdpGetRawElementType(header) ((header) & btLibDETD_MASK)

//---------------------------------------------------------------------------
// Returns a Data Element's size descriptor.
//
// header -> The first byte of a Data Element
//
//  Returns:
//    		The size characteristic of the Data Element.
//
#define BtLibSdpGetRawDataElementSize(header) ((header) & btLibDESD_MASK)   

#pragma mark *--------Security----------*

//---------------------------------------------------------------------------
// Search the device database for the device with the specified
// Bluetooth address. Return the index of the corresponding device
// record in the database.
// 
// btLibRefNum -> Reference number for the Bluetooth library.
// addrP -> Bluetooth address of remote device.
// index <- Index of the record.
//
//  Returns:
// 	btLibErrNoError - successful.
//    btLibErrNotFound - a record with the specified remote device
//                       address could not be found.
//
Err BtLibSecurityFindTrustedDeviceRecord(UInt16 btLibRefNum, BtLibDeviceAddressTypePtr addrP, UInt16 *index)
	BTLIB_TRAP(btLibTrapSecurityFindTrustedDeviceRecord);
	
//---------------------------------------------------------------------------
// Remove a device record from the device database.
// 
// btLibRefNum -> Reference number for the Bluetooth library.
// index -> Index of the record.
//
//  Returns:
//    btLibErrNoError - if successful.
//    btLibErrNotFound - if a record with the specified index
//     could not be found.
//
Err BtLibSecurityRemoveTrustedDeviceRecord(UInt16 btLibRefNum, UInt16 index)
	BTLIB_TRAP(btLibTrapSecurityRemoveTrustedDeviceRecord);

//---------------------------------------------------------------------------
// Get information from a device record in the device database.
//		
// btLibRefNum -> Reference number for the Bluetooth library.
// index -> Index of the record to Remove.
// addrP <- Bluetooth address of remote device.  NULL if not desired.
// nameBuffer <- Pointer to buffer to store user-friendly name of
//     remote device.  NULL if not desired.
// nameBufferSize -> Size of the nameBuffer buffer.  If less than 
//     btLibMaxDeviceNameLength, name may be truncated. Ignored if
//     nameBuffer is NULL.
// cod <-  Pointer to a  BtLibClassOfDeviceType representing
//    the class of the device.  NULL if not desired.
// lastConnected <- The date since the device last connected. This
//     date is measured in seconds since midnight
//     January 1, 1904.   NULL if not desired.
// persistent <- If true, the device is bonded and can connect
//     to the local device without authentication. If
//     false, the device is paired but not bondedÑit
//     will need to reauthenticate if it connects again. NULL if not desired.
//
//  Returns:
//    btLibErrNoError - if successful.
//    btLibErrNotFound - if a record with the specified index
//     could not be found.
//
Err BtLibSecurityGetTrustedDeviceRecordInfo(UInt16 btLibRefNum,  UInt16 index, BtLibDeviceAddressTypePtr addrP, 
													Char* nameBuffer, UInt8 NameBufferSize, BtLibClassOfDeviceType *cod, 
													UInt32 *lastConnected, Boolean *persistentP)
	BTLIB_TRAP(btLibTrapSecurityGetTrustedDeviceRecordInfo);

//---------------------------------------------------------------------------
// Return the number of bonded devices in the device database or
// return the total number of devices in the device database.
// 
// btLibRefNum -> Reference number for the Bluetooth library.
// persistentOnly -> true to obtain the total number of bonded
//                   devices in the database. These are the same
//                   devices that appear in the trusted devices list.
//                   false to obtain the total number of devices in
//                   the device database. This includes the devices
//                   that are bonded and the devices that are paired
//                   but not bonded.
//
//  Returns:
//    Returns the requested number of device records.
//
UInt16 BtLibSecurityNumTrustedDeviceRecords( UInt16 btLibRefNum, Boolean persistentOnly)
	BTLIB_TRAP(btLibTrapSecurityNumTrustedDeviceRecords);

/********************************************************************
 * Bluetooth byte ordering routines
 ********************************************************************/
#define _BtLibNetSwap16(x) \
	((((x) >> 8) & 0xFF) | \
	 (((x) & 0xFF) << 8))

#define _BtLibNetSwap32(x) \
	((((x) >> 24) & 0x00FF) | \
	 (((x) >>  8) & 0xFF00) | \
	 (((x) & 0xFF00) <<  8) | \
	 (((x) & 0x00FF) << 24))

#if  SYS_ENDIAN == 2
// convert SDP byte orders (SDP is Big Endian)
// convert network long to host long
#define 		BtLibSdpNToHL(x)  (x)

// convert network Int16 to host Int16
#define		BtLibSdpNToHS(x)  (x)

// convert host long to network long
#define 		BtLibSdpHToNL(x)  (x)

// convert host Int16 to network Int16
#define		BtLibSdpHToNS(x)  (x)

// convert RFCOMM byte orders (RFCOMM is Big Endian)
#define 		BtLibRfCommNToHL(x)  (x)

#define		BtLibRfCommNToHS(x)  (x)

#define 		BtLibRfCommHToNL(x)  (x)

#define		BtLibRfCommHToNS(x)  (x)		

// convert L2CAP byte orders (L2CAP is Little Endian)
#define     BtLibL2CapNToHL(x) _BtLibNetSwap32(x)

#define		BtLibL2CapNToHS(x) _BtLibNetSwap16(x)

#define		BtLibL2CapHToNL(x) _BtLibNetSwap32(x)

#define		BtLibL2CapHToNS(x) _BtLibNetSwap16(x)

#elif  SYS_ENDIAN == 1
// convert SDP byte orders (SDP is Big Endian)
// convert network long to host long
#define 		BtLibSdpNToHL(x)  _BtLibNetSwap32(x)

// convert network Int16 to host Int16
#define		BtLibSdpNToHS(x)  _BtLibNetSwap16(x)

// convert host long to network long
#define 		BtLibSdpHToNL(x)  _BtLibNetSwap32(x)

// convert host Int16 to network Int16
#define		BtLibSdpHToNS(x)  _BtLibNetSwap16(x)

// convert RFCOMM byte orders (RFCOMM is Big Endian)
#define 		BtLibRfCommNToHL(x)  _BtLibNetSwap32(x)

#define		BtLibRfCommNToHS(x)  _BtLibNetSwap16(x)

#define 		BtLibRfCommHToNL(x)  _BtLibNetSwap32(x)

#define		BtLibRfCommHToNS(x)  _BtLibNetSwap16(x)

// convert L2CAP byte orders (L2CAP is Little Endian)
#define     BtLibL2CapNToHL(x) (x)

#define		BtLibL2CapNToHS(x) (x)

#define		BtLibL2CapHToNL(x) (x)

#define		BtLibL2CapHToNS(x) (x)
#else
#error "No CPU_ENDIAN defined"
#endif
	
/********************************************************************
 * Bluetooth device address translation and conversion routines
 ********************************************************************/

//--------------------------------------------------------------------	
// Convert 48-bit BtLibAddressType to ascii colon seperated form. 
//
// btLibRefNum -> the reference number for the bluetooth library
// btDevP -> address of a Bluetooth device
// spaceP <- memory to hold null terminated ascii formatted bluetooth device address
// spaceSize <-> size of spaceP in bytes (in), Length of spaceP excluding NULL (out)
//
//  Returns:	
//				btLibErrNoError - Indicates successful conversion
//
// 	Events:
//				None
//
Err	BtLibAddrBtdToA(UInt16 libRefNum, BtLibDeviceAddressType *btDevP, Char *spaceP,
	UInt16 spaceSize)
	BTLIB_TRAP(btLibTrapAddrBtdToA);
	
//--------------------------------------------------------------------						
// Convert a colon separated ascii string format of a Bluetooth device 
// address into a 48-bit BtLibDeviceAddressType.
//
// btLibRefNum -> the reference number for the bluetooth library
// btDevP <- memory to receive address of a Bluetooth device
// a <- string with ascii colon seperated bluetooth device address 
//
//  Returns:	
//				btLibErrNoError - Indicates successful conversion
//
// 	Events:
//				None
// 
Err BtLibAddrAToBtd(UInt16 libRefNum, const Char *a, BtLibDeviceAddressType *btDevP)
	BTLIB_TRAP(btLibTrapAddrAToBtd);
	
#pragma mark *--------Service----------*
// ** The Service's API is not yet documented **//

//--------------------------------------------------------------------	
// Used by services to open the Lib instead of BtLibOpen
//
Err BtLibServiceOpen(UInt16 btLibRefNum)
	BTLIB_TRAP(btLibTrapServiceOpen);

//--------------------------------------------------------------------	
// Used by services to close the Lib instead of BtLibClose
//
Err BtLibServiceClose(UInt16 btLibRefNum)	
	BTLIB_TRAP(btLibTrapServiceClose);
	
//--------------------------------------------------------------------	
// Used by services to note that they have begun a session
//
Err BtLibServiceIndicateSessionStart(UInt16 btLibRefNum)
	BTLIB_TRAP(btLibTrapServiceIndicateSessionStart);	

//--------------------------------------------------------------------	
// Used by services to indicate a session start to the user
//
Err BtLibServicePlaySound(UInt16 btLibRefNum)
	BTLIB_TRAP(btLibTrapServicePlaySound);	
	
	

#ifdef __cplusplus 
}
#endif

#endif
