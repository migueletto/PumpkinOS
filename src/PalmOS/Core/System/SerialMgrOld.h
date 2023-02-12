/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialMgrOld.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Serial manager
 *
 *****************************************************************************/

#ifndef __SERIALMGROLD_H_
#define __SERIALMGROLD_H_

// Pilot common definitions
#include <PalmTypes.h>
#include <SystemMgr.h>


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


/********************************************************************
 * Serial Port Definitions
 ********************************************************************/

#define	serPortDefault				0x0000	// Use prefDefSerialPlugIn
#define	serPortLocalHotSync		0x8000	// Use physical HotSync port
#define	serPortMaskLocal			0x7FFF	// Mask off HotSync "hint" (for SerialMgr)


/********************************************************************
 * Serial Settings Descriptor
 ********************************************************************/

typedef struct SerSettingsType {
	UInt32		baudRate;			// baud rate
	UInt32		flags;				// miscellaneous settings
	Int32			ctsTimeout;			// max # of ticks to wait for CTS to become asserted
											// before transmitting; used only when
											// configured with serSettingsFlagCTSAutoM.
	} SerSettingsType;
typedef SerSettingsType*	SerSettingsPtr;
	
#define		serSettingsFlagStopBitsM			0x00000001		// mask for stop bits field
#define		serSettingsFlagStopBits1			0x00000000		//  1 stop bits	
#define		serSettingsFlagStopBits2			0x00000001		//  2 stop bits	
#define		serSettingsFlagParityOnM			0x00000002		// mask for parity on
#define		serSettingsFlagParityEvenM			0x00000004		// mask for parity even
#define		serSettingsFlagXonXoffM				0x00000008		// (NOT IMPLEMENTED) mask for Xon/Xoff flow control
#define		serSettingsFlagRTSAutoM				0x00000010		// mask for RTS rcv flow control
#define		serSettingsFlagCTSAutoM				0x00000020		// mask for CTS xmit flow control
#define		serSettingsFlagBitsPerCharM		0x000000C0		// mask for bits/char
#define		serSettingsFlagBitsPerChar5		0x00000000		//  5 bits/char	
#define		serSettingsFlagBitsPerChar6		0x00000040		//  6 bits/char	
#define		serSettingsFlagBitsPerChar7		0x00000080		//  7 bits/char	
#define		serSettingsFlagBitsPerChar8		0x000000C0		//  8 bits/char	


// Default settings
#define		serDefaultSettings		(serSettingsFlagBitsPerChar8	|		\
												serSettingsFlagStopBits1		|		\
												serSettingsFlagRTSAutoM)
												
#define		serDefaultCTSTimeout		(5*sysTicksPerSecond)

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
 * Type of a wakeup handler procedure which can be installed through the
 *   SerSetWakeupHandler() call.
 ********************************************************************/
typedef void (*SerWakeupHandler)  (UInt32 refCon);

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
typedef Boolean (*SerBlockingHookHandler)  (UInt32 userRef);




/********************************************************************
 * Serial Library Control Enumerations (Pilot 2.0)
 ********************************************************************/

/********************************************************************
 * Structure for specifying callback routines.
 ********************************************************************/
typedef struct SerCallbackEntryType {
	MemPtr		funcP;					// function pointer
	UInt32		userRef;					// ref value to pass to callback
	} SerCallbackEntryType;
typedef SerCallbackEntryType*	SerCallbackEntryPtr;

// v2.0 extension
typedef enum SerCtlEnum {
	serCtlFirstReserved = 0,		// RESERVE 0
	
	serCtlStartBreak,					// turn RS232 break signal on:
											// users are responsible for ensuring that the break is set
											// long enough to genearate a valie BREAK!
											// valueP = 0, valueLenP = 0
											
	serCtlStopBreak,					// turn RS232 break signal off:
											// valueP = 0, valueLenP = 0

	serCtlBreakStatus,				// Get RS232 break signal status(on or off):
											// valueP = pointer to UInt16 for returning status(0 = off, !0 = on)
											// *valueLenP = sizeof(UInt16)
											
	serCtlStartLocalLoopback,		// Start local loopback test
											// valueP = 0, valueLenP = 0
											
	serCtlStopLocalLoopback,		// Stop local loopback test
											// valueP = 0, valueLenP = 0

	serCtlMaxBaud,						// Get maximum supported baud rate:
											// valueP = pointer to UInt32 for returned baud
											// *valueLenP = sizeof(UInt32)
	
	serCtlHandshakeThreshold,		// retrieve HW handshake threshold; this is the maximum baud rate
											// which does not require hardware handshaking
											// valueP = pointer to UInt32 for returned baud
											// *valueLenP = sizeof(UInt32)
	
	serCtlEmuSetBlockingHook,		// Set a blocking hook routine FOR EMULATION
											// MODE ONLY - NOT SUPPORTED ON THE PILOT
											//PASS:
											// valueP = pointer to SerCallbackEntryType
											// *valueLenP = sizeof(SerCallbackEntryType)
											//RETURNS:
											// the old settings in the first argument
											

	serCtlIrDAEnable,					// Enable  IrDA connection on this serial port
											// valueP = 0, valueLenP = 0

	serCtlIrDADisable,				// Disable  IrDA connection on this serial port
											// valueP = 0, valueLenP = 0

	serCtlIrScanningOn,				// Start Ir Scanning mode	
	
	serCtlIrScanningOff,				// Stop Ir Scanning mode
	
	serCtlRxEnable,					// enable receiver  ( for IrDA )
	
	serCtlRxDisable,					// disable receiver ( for IrDA )

	serCtlLAST							// ADD NEW ENTRIES BEFORE THIS ONE
	} SerCtlEnum;


// Start of a custom op code range for licensees that wrote old serial 
// manager replacements.  Note that the serial compatiblity library
// does not pass these op codes to new serial manager plugins.
#define serCtlFirstCustomEntry 0xA800

/********************************************************************
 * Serial Library Routines
 * These are define as external calls only under emulation mode or
 *  under native mode from the module that actually installs the trap
 *  vectors
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Used by mac applications to map the pilot serial port to a particular
// macintosh port.
UInt16 SerSetMapPort( UInt16 pilotPort, UInt16 macPort );

// Acquires and opens a serial port with given baud and default settings.
Err SerOpen(UInt16 refNum, UInt16 port, UInt32 baud)
				SYS_TRAP(sysLibTrapOpen);
				
// Used by debugger to re-initialize serial port if necessary
Err SerDbgAssureOpen(UInt16 refNum, UInt16 port, UInt32 baud);

// Closes the serial connection previously opened with SerOpen.
Err SerClose(UInt16 refNum)
				SYS_TRAP(sysLibTrapClose);

// Puts serial library to sleep
Err SerSleep(UInt16 refNum)
				SYS_TRAP(sysLibTrapSleep);

// Wake Serial library
Err SerWake(UInt16 refNum)
				SYS_TRAP(sysLibTrapWake);

// Get attributes of the serial connection
Err SerGetSettings(UInt16 refNum, SerSettingsPtr settingsP)
				SYS_TRAP(sysLibTrapCustom);
				
// Set attributes of the serial connection
Err SerSetSettings(UInt16 refNum, SerSettingsPtr settingsP)
				SYS_TRAP(sysLibTrapCustom+1);

// Return status of serial connection
UInt16 SerGetStatus(UInt16 refNum, Boolean * ctsOnP, 
				Boolean * dsrOnP)
				SYS_TRAP(sysLibTrapCustom+2);
				
// Reset error condition of serial connection
Err SerClearErr(UInt16 refNum)
				SYS_TRAP(sysLibTrapCustom+3);
				
				


// Sends a buffer of data (may queue it up and return).
Err SerSend10(UInt16 refNum, const void * bufP, UInt32 size)
				SYS_TRAP(sysLibTrapCustom+4);

// Waits until the serial transmit buffer empties.
// The timeout arg is ignored; CTS timeout is used
Err SerSendWait(UInt16 refNum, Int32 timeout)
				SYS_TRAP(sysLibTrapCustom+5);

// Returns how many characters are left in the send queue waiting 
//  for transmission
Err SerSendCheck(UInt16 refNum, UInt32 * numBytesP)
				SYS_TRAP(sysLibTrapCustom+6);

// Flushes the data out of the transmit buffer
Err SerSendFlush(UInt16 refNum)
				SYS_TRAP(sysLibTrapCustom+7);




// Receives a buffer of data of the given size.
Err SerReceive10(UInt16 refNum, void * bufP, UInt32 bytes, Int32 timeout)
				SYS_TRAP(sysLibTrapCustom+8);

// Waits for at least 'bytes' bytes of data to arrive at the serial input.
//  but does not read them in
Err SerReceiveWait(UInt16 refNum, UInt32 bytes, Int32 timeout)
				SYS_TRAP(sysLibTrapCustom+9);

// Returns how many characters are in the receive queue
Err SerReceiveCheck(UInt16 refNum, UInt32 * numBytesP)
				SYS_TRAP(sysLibTrapCustom+10);

// Flushes any data coming into the serial port, discarding the data.
void SerReceiveFlush(UInt16 refNum, Int32 timeout)
				SYS_TRAP(sysLibTrapCustom+11);


// Specify a new input buffer.  To restore the original buffer, pass
// bufSize = 0.
Err SerSetReceiveBuffer(UInt16 refNum, void * bufP, UInt16 bufSize)
				SYS_TRAP(sysLibTrapCustom+12);


// The receive character interrupt service routine, called by kernel when
//  a UART interrupt is detected. 
Boolean SerReceiveISP(void)		
				SYS_TRAP(sysTrapSerReceiveISP);



// "Back Door" into the serial receive queue. Used by applications (like TCP Media layers)
//  that need faster access to received characters
Err	SerReceiveWindowOpen(UInt16 refNum, UInt8 ** bufPP, UInt32 * sizeP)
				SYS_TRAP(sysLibTrapCustom+13);
				
Err	SerReceiveWindowClose(UInt16 refNum, UInt32 bytesPulled)
				SYS_TRAP(sysLibTrapCustom+14);
				
// Can be called by applications that need an alternate wakeup mechanism
//  when characters get enqueued by the interrupt routine.  
Err	SerSetWakeupHandler(UInt16 refNum, SerWakeupHandler procP, 
					UInt32 refCon)
				SYS_TRAP(sysLibTrapCustom+15);
	
// Called to prime wakeup handler			
Err	SerPrimeWakeupHandler(UInt16 refNum, UInt16 minBytes)
				SYS_TRAP(sysLibTrapCustom+16);
	
// Called to perform a serial manager control operation			
// (v2.0 extension)
Err	SerControl(UInt16 refNum, UInt16 op, void * valueP, UInt16 * valueLenP)
				SYS_TRAP(sysLibTrapCustom+17);


// Sends a buffer of data (may queue it up and return).
UInt32 SerSend(UInt16 refNum, const void * bufP, UInt32 count, Err* errP)
				SYS_TRAP(sysLibTrapCustom+18);

// Receives a buffer of data of the given size.
UInt32 SerReceive(UInt16 refNum, void * bufP, UInt32 count, Int32 timeout, Err* errP)
				SYS_TRAP(sysLibTrapCustom+19);


#ifdef __cplusplus
}
#endif

	
#endif	//__SERIALMGROLD_H_
