/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Serial manager
 *
 *****************************************************************************/

#ifndef __SERIALMGR_H
#define __SERIALMGR_H


// Pilot common definitions
#include <PalmTypes.h>
#include <ErrorBase.h>

// New Serial manager feature numbers
#define sysFtrNewSerialPresent     1
#define sysFtrNewSerialVersion     2

#define serMgrVersion              2

/********************************************************************
 * Serial Manager Errors
 * the constant serErrorClass is defined in SystemMgr.h
 ********************************************************************/

#define	serErrBadParam				(serErrorClass | 1)
#define	serErrBadPort				(serErrorClass | 2)
#define	serErrNoMem					(serErrorClass | 3)
#define	serErrBadConnID			(serErrorClass | 4)
#define	serErrTimeOut				(serErrorClass | 5)
#define	serErrLineErr				(serErrorClass | 6)
#define	serErrAlreadyOpen			(serErrorClass | 7)
#define  serErrStillOpen			(serErrorClass | 8)
#define	serErrNotOpen				(serErrorClass | 9)
#define	serErrNotSupported		(serErrorClass | 10)		// functionality not supported
#define	serErrNoDevicesAvail		(serErrorClass	| 11)		// No serial devices were loaded or are available.
// New error codes for USB support
#define	serErrConfigurationFailed	(serErrorClass | 12)

//
// mask values for the lineErrors  from SerGetStatus
//

#define	serLineErrorParity		0x0001			// parity error
#define	serLineErrorHWOverrun	0x0002			// HW overrun
#define	serLineErrorFraming		0x0004			// framing error
#define 	serLineErrorBreak			0x0008			// break signal asserted
#define 	serLineErrorHShake		0x0010			// line hand-shake error
#define	serLineErrorSWOverrun	0x0020			// HW overrun
#define	serLineErrorCarrierLost	0x0040			// CD dropped


/********************************************************************
 * Serial Port Definitions
 ********************************************************************/

#define serPortLocalHotSync	0x8000		// Use physical HotSync port

#define serPortCradlePort		0x8000		// Cradle Port (Auto detect cradle type)
#define serPortIrPort			0x8001		// Use available IR port.
#define serPortConsolePort		0x8002		// Console port
#define serPortCradleRS232Port	0x8003	// Cradle RS232 Port
#define serPortCradleUSBPort	0x8004		// Cradle USB Port

// This constant is used by the Serial Link Mgr only
#define serPortIDMask			0xC000


/********************************************************************
 * Serial Settings Descriptor
 ********************************************************************/
	
#define		srmSettingsFlagStopBitsM			0x00000001		// mask for stop bits field
#define		srmSettingsFlagStopBits1			0x00000000		//  1 stop bits	
#define		srmSettingsFlagStopBits2			0x00000001		//  2 stop bits	
#define		srmSettingsFlagParityOnM			0x00000002		// mask for parity on
#define		srmSettingsFlagParityEvenM			0x00000004		// mask for parity even
#define		srmSettingsFlagXonXoffM				0x00000008		// (NOT IMPLEMENTED) mask for Xon/Xoff flow control
#define		srmSettingsFlagRTSAutoM				0x00000010		// mask to prevent UART input overflow using RTS (NOTE: this flag 
																// alone does not prevent software overruns from the serial input buffer)
#define		srmSettingsFlagCTSAutoM				0x00000020		// mask for CTS xmit flow control (see srmSettingsFlagFlowControlIn below)
#define		srmSettingsFlagBitsPerCharM		0x000000C0		// mask for bits/char
#define		srmSettingsFlagBitsPerChar5		0x00000000		//  5 bits/char	
#define		srmSettingsFlagBitsPerChar6		0x00000040		//  6 bits/char	
#define		srmSettingsFlagBitsPerChar7		0x00000080		//  7 bits/char	
#define		srmSettingsFlagBitsPerChar8		0x000000C0		//  8 bits/char
#define		srmSettingsFlagFlowControlIn		0x00000100		// mask to prevent the serial input buffer overflow, using RTS. Use in
																// conjunction with srmSettingsFlagRTSAutoM for a fully flow controlled input.
#define		srmSettingsFlagRTSInactive		0x00000200		// if set and srmSettingsFlagRTSAutoM==0, RTS is held in the inactive (flow off) state forever.


// Default settings
#define		srmDefaultSettings		(srmSettingsFlagBitsPerChar8	|		\
												 srmSettingsFlagStopBits1		|		\
												 srmSettingsFlagRTSAutoM | srmSettingsFlagFlowControlIn)
												
#define		srmDefaultCTSTimeout		(5*sysTicksPerSecond)


// Status bitfield constants

#define srmStatusCtsOn				0x00000001
#define srmStatusRtsOn				0x00000002
#define srmStatusDsrOn				0x00000004
#define srmStatusBreakSigOn		0x00000008


//
// Info fields describing serial HW capabilities.
//

#define serDevCradlePort			0x00000001		// Serial HW controls RS-232 serial from cradle connector of handheld.
#define serDevRS232Serial			0x00000002		// Serial HW has RS-232 line drivers
#define serDevIRDACapable			0x00000004		// Serial Device has IR line drivers and generates IRDA mode serial.	
#define serDevModemPort				0x00000008		// Serial deivce drives modem connection.
#define serDevCncMgrVisible		0x00000010		// Serial device port name string to be displayed in Connection Mgr panel.
#define serDevConsolePort			0x00000020		// Serial device is the default console port.
#define serDevUSBCapable   		0x00000040		// USB driver for USB hardware connected to the cradle connector of the handheld.


typedef struct DeviceInfoType {
	UInt32 serDevCreator;								// Four Character creator type for serial driver ('sdrv')
	UInt32 serDevFtrInfo;								// Flags defining features of this serial hardware.
	UInt32 serDevMaxBaudRate;							// Maximum baud rate for this device.
	UInt32 serDevHandshakeBaud;						// HW Handshaking is reccomended for baud rates over this
	Char *serDevPortInfoStr;							// Description of serial HW device or virtual device.			
	UInt8 reserved[8];									// Reserved.
} DeviceInfoType;

typedef DeviceInfoType *DeviceInfoPtr;

//
// Function IDs
//
// Standard set of function ids for the SrmOpen.  Out of convenience, function ids
// use the same namespace as creator ids.  Custom functions can be defined by
// using your app's creator id.  The driver must have knowledge of that creator
// id for it to be of any use.  A driver should handle an unknown function id
// gracefully, either use default functionality or return a serErrBadParam error.
// 
#define serFncUndefined 	0L							// Undefined function
#define serFncPPPSession	netIFCreatorPPP		// NetLib PPP Interface
#define serFncSLIPSession  netIFCreatorSLIP		// NetLib SLIP Interface
#define serFncDebugger		sysFileCSystem			// PalmOS Debugger
#define serFncHotSync		sysFileCSync			// HotSync function
#define serFncConsole		sysFileCSystem			// PalmOS Console
#define serFncTelephony    sysFileCTelMgrLib		// Telephony Library


//
// Open Configuration Structure
//
typedef struct SrmOpenConfigType {
	UInt32 baud;				// Baud rate that the connection is to be opened at.
									// Applications that use drivers that do not require
									// baud rates can set this to zero or any other value.
									// Drivers that do not require a baud rate should 
									// ignore this field
	UInt32 function;			//	Designates the function of the connection. A value
									// of zero indictates default behavior for the protocol.
									// Drivers that do not support multiple functions should
									// ignore this field.	
	MemPtr drvrDataP;			// Pointer to driver specific data.
	UInt16 drvrDataSize;		// Size of the driver specific data block.	
	UInt32 sysReserved1;    // System Reserved
	UInt32 sysReserved2;    // System Reserved 
} SrmOpenConfigType;

typedef SrmOpenConfigType* SrmOpenConfigPtr;

/********************************************************************
 * Transfer modes for USB
 ********************************************************************/
typedef enum SrmTransferModeType {
	srmTransferFirstReserved = 0,		// RESERVE 0
	srmUSBInterruptMode,
	srmUSBBulkMode,
	srmUSBIsochronous
} SrmTransferModeType;



/********************************************************************
 * Type of a wakeup handler procedure which can be installed through the
 *   SerSetWakeupHandler() call.
 ********************************************************************/
typedef void (*WakeupHandlerProcPtr)(UInt32 refCon);

/********************************************************************
 * Type of an emulator-mode only blocking hook routine installed via
 * SerControl function serCtlEmuSetBlockingHook.  This is supported only
 * under emulation mode.  The argument to the function is the value
 * specified in the SerCallbackEntryType structure.  The intention of the
 * return value is to return false if serial manager should abort the
 * current blocking action, such as when an app quit event has been received;
 * otherwise, it should return true.  However, in the current implementation,
 * this return value is ignored.  The callback can additionally process
 * events to enable user interaction with the UI, such as interacting with the
 * debugger. 
 ********************************************************************/
typedef Boolean (*BlockingHookProcPtr)  (UInt32 userRef);


/********************************************************************
 * Serial Library Control Enumerations (Pilot 2.0)
 ********************************************************************/

/********************************************************************
 * Structure for specifying callback routines.
 ********************************************************************/
typedef struct SrmCallbackEntryType {
	BlockingHookProcPtr	funcP;					// function pointer
	UInt32					userRef;					// ref value to pass to callback
} SrmCallbackEntryType;
typedef SrmCallbackEntryType*	SrmCallbackEntryPtr;


typedef enum SrmCtlEnum {
	srmCtlFirstReserved = 0,		// RESERVE 0
	
	srmCtlSetBaudRate,				// Sets the current baud rate for the HW.
											// valueP = pointer to Int32, valueLenP = pointer to sizeof(Int32)
											
	srmCtlGetBaudRate,				// Gets the current baud rate for the HW.
											
	srmCtlSetFlags,					// Sets the current flag settings for the serial HW.
	
	srmCtlGetFlags,					// Gets the current flag settings the serial HW.
	
	srmCtlSetCtsTimeout,				// Sets the current Cts timeout value.
	
	srmCtlGetCtsTimeout,				// Gets the current Cts timeout value.
	
	srmCtlStartBreak,					// turn RS232 break signal on:
											// users are responsible for ensuring that the break is set
											// long enough to genearate a valid BREAK!
											// valueP = 0, valueLenP = 0
											
	srmCtlStopBreak,					// turn RS232 break signal off:
											// valueP = 0, valueLenP = 0

	srmCtlStartLocalLoopback,		// Start local loopback test
											// valueP = 0, valueLenP = 0
											
	srmCtlStopLocalLoopback,		// Stop local loopback test
											// valueP = 0, valueLenP = 0


	srmCtlIrDAEnable,					// Enable  IrDA connection on this serial port
											// valueP = 0, valueLenP = 0

	srmCtlIrDADisable,				// Disable  IrDA connection on this serial port
											// valueP = 0, valueLenP = 0

	srmCtlRxEnable,					// enable receiver  ( for IrDA )
	
	srmCtlRxDisable,					// disable receiver ( for IrDA )

	srmCtlEmuSetBlockingHook,		// Set a blocking hook routine FOR EMULATION
											// MODE ONLY - NOT SUPPORTED ON THE PILOT
											//PASS:
											// valueP = pointer to SerCallbackEntryType
											// *valueLenP = sizeof(SerCallbackEntryType)
											//RETURNS:
											// the old settings in the first argument										

	srmCtlUserDef,						// Specifying this opCode passes through a user-defined
											//  function to the DrvControl function. This is for use
											//  specifically by serial driver developers who need info
											//  from the serial driver that may not be available through the
											//  standard SrmMgr interface.
											
	srmCtlGetOptimalTransmitSize,	// This function will ask the port for the most efficient buffer size
											// for transmitting data packets.  This opCode returns serErrNotSupported
											// if the physical or virtual device does not support this feature.
											// The device can return a transmit size of 0, if send buffering is
											// requested, but the actual size is up to the caller to choose.
											// valueP = pointer to UInt32 --> return optimal buf size
											// ValueLenP = sizeof(UInt32)
	
	srmCtlSetDTRAsserted,			// Enable or disable DTR.
	
	srmCtlGetDTRAsserted,			// Determine if DTR is enabled or disabled.
	
	srmCtlSetYieldPortCallback,   // Set the yield port callback
	
	srmCtlSetYieldPortRefCon,     // Set the yield port refNum
	
											// ***** ADD NEW ENTRIES BEFORE THIS ONE
	
	srmCtlSystemReserved = 0x7000, // Reserve control op code space for system use.
	
	srmCtlCustom = 0x8000,     	// Reserve control op code space for licensee use.
	
	srmCtlLAST						
	
} SrmCtlEnum;

#define srmCtlSystemStart		0x7000	// Start poitn for system op codes.
#define srmCtlCustomStart		0x8000	// Start point for custom op codes.


/********************************************************************
 * Serial Hardware Library Routines
 ********************************************************************/

#ifdef BUILDING_NEW_SERIAL_MGR
	#define SERIAL_TRAP(serialSelectorNum)
#else
	#define SERIAL_TRAP(serialSelectorNum) \
		_SYSTEM_API(_CALL_WITH_SELECTOR)(_SYSTEM_TABLE, sysTrapSerialDispatch, \
													serialSelectorNum)
#endif


// *****************************************************************
// * New Serial Manager trap selectors
// *****************************************************************

// The numbering of these #defines *MUST* match the order in SerialMgr.c
#define sysSerialInstall				0
#define sysSerialOpen					1
#define sysSerialOpenBkgnd				2
#define sysSerialClose					3
#define sysSerialSleep					4
#define sysSerialWake					5
#define sysSerialGetDeviceCount		6
#define sysSerialGetDeviceInfo		7
#define sysSerialGetStatus				8
#define sysSerialClearErr				9
#define sysSerialControl				10
#define sysSerialSend					11
#define sysSerialSendWait				12
#define sysSerialSendCheck				13
#define sysSerialSendFlush				14
#define sysSerialReceive				15
#define sysSerialReceiveWait			16
#define sysSerialReceiveCheck			17
#define sysSerialReceiveFlush			18
#define sysSerialSetRcvBuffer			19
#define sysSerialRcvWindowOpen		20
#define sysSerialRcvWindowClose		21
#define sysSerialSetWakeupHandler	22
#define sysSerialPrimeWakeupHandler	23
#define sysSerialOpenV4					24
#define sysSerialOpenBkgndV4			25
#define sysSerialCustomControl		26

// Used by SerialMgrDispatch.c
#define maxSerialSelector				sysSerialCustomControl


#ifdef __cplusplus
extern "C" {
#endif

Err SerialMgrInstall(void)
	SERIAL_TRAP(sysSerialInstall);

Err SrmOpen(UInt32 port, UInt32 baud, UInt16 *newPortIdP)
	SERIAL_TRAP(sysSerialOpen);
	
Err SrmExtOpen(UInt32 port, SrmOpenConfigType* configP, UInt16 configSize, UInt16 *newPortIdP)
	SERIAL_TRAP(sysSerialOpenV4);

Err SrmExtOpenBackground(UInt32 port, SrmOpenConfigType* configP, UInt16 configSize, UInt16 *newPortIdP)
	SERIAL_TRAP(sysSerialOpenBkgndV4);
	
Err SrmOpenBackground(UInt32 port, UInt32 baud, UInt16 *newPortIdP)
	SERIAL_TRAP(sysSerialOpenBkgnd);

Err SrmClose(UInt16 portId)
	SERIAL_TRAP(sysSerialClose);
	
Err SrmSleep()
	SERIAL_TRAP(sysSerialSleep);
	
Err SrmWake()
	SERIAL_TRAP(sysSerialWake);

Err SrmGetDeviceCount(UInt16 *numOfDevicesP)
	SERIAL_TRAP(sysSerialGetDeviceCount);

Err SrmGetDeviceInfo(UInt32 deviceID, DeviceInfoType *deviceInfoP)
	SERIAL_TRAP(sysSerialGetDeviceInfo);

Err SrmGetStatus(UInt16 portId, UInt32 *statusFieldP, UInt16 *lineErrsP)
	SERIAL_TRAP(sysSerialGetStatus);

Err SrmClearErr (UInt16 portId)
	SERIAL_TRAP(sysSerialClearErr);

Err SrmControl(UInt16 portId, UInt16 op, void *valueP, UInt16 *valueLenP)
	SERIAL_TRAP(sysSerialControl);
	
Err  SrmCustomControl(UInt16 portId, UInt16 opCode, UInt32 creator, 
							void* valueP, UInt16* valueLenP)
	SERIAL_TRAP(sysSerialCustomControl);
	
UInt32 SrmSend (UInt16 portId, const void *bufP, UInt32 count, Err *errP)
	SERIAL_TRAP(sysSerialSend);

Err SrmSendWait(UInt16 portId)
	SERIAL_TRAP(sysSerialSendWait);

Err SrmSendCheck(UInt16 portId, UInt32 *numBytesP)
	SERIAL_TRAP(sysSerialSendCheck);

Err SrmSendFlush(UInt16 portId)
	SERIAL_TRAP(sysSerialSendFlush);

UInt32 SrmReceive(UInt16 portId, void *rcvBufP, UInt32 count, Int32 timeout, Err *errP)
	SERIAL_TRAP(sysSerialReceive);

Err SrmReceiveWait(UInt16 portId, UInt32 bytes, Int32 timeout)
	SERIAL_TRAP(sysSerialReceiveWait);

Err SrmReceiveCheck(UInt16 portId,  UInt32 *numBytesP)
	SERIAL_TRAP(sysSerialReceiveCheck);

Err SrmReceiveFlush(UInt16 portId, Int32 timeout)
	SERIAL_TRAP(sysSerialReceiveFlush);

Err SrmSetReceiveBuffer(UInt16 portId, void *bufP, UInt16 bufSize)
	SERIAL_TRAP(sysSerialSetRcvBuffer);

Err SrmReceiveWindowOpen(UInt16 portId, UInt8 **bufPP, UInt32 *sizeP)
	SERIAL_TRAP(sysSerialRcvWindowOpen);

Err SrmReceiveWindowClose(UInt16 portId, UInt32 bytesPulled)
	SERIAL_TRAP(sysSerialRcvWindowClose);

Err SrmSetWakeupHandler(UInt16 portId, WakeupHandlerProcPtr procP, UInt32 refCon)
	SERIAL_TRAP(sysSerialSetWakeupHandler);

Err SrmPrimeWakeupHandler(UInt16 portId, UInt16 minBytes)
	SERIAL_TRAP(sysSerialPrimeWakeupHandler);

void SrmSelectorErrPrv (UInt16 serialSelector);		// used only by SerialMgrDispatch.c

#ifdef __cplusplus
}
#endif

#endif		// __SERIALMGR_H
