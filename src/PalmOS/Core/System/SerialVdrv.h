/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialVdrv.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Constants and data structures for virtual driver ('vdrv') code.
 *
 *****************************************************************************/

#ifndef __SERIALVDRV_H__
#define __SERIALVDRV_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <HAL.h>

#include <SerialDrvr.h>
#include <SerialMgr.h>


// ********** Constants

#define kVdrvResType			'vdrv'


// ********** Typedefs

typedef enum VdrvCtlOpCodeEnum {				// Control function opCodes
	vdrvOpCodeNoOp = 0, 
	vdrvOpCodeSetBaudRate = 0x1000,			// Set the port's baud rate.
	vdrvOpCodeSetSettingsFlags,				// Set the ports send/rvc settings
	vdrvOpCodeSetCtsTimeout,					// The HW handshake timeout.
	vdrvOpCodeClearErr,							// Clear any HW errors.
	vdrvOpCodeSetSleepMode,						// Put in sleep mode.
	vdrvOpCodeSetWakeupMode,					// Wake from sleep mode.
	vdrvOpCodeFIFOCount,							// Return bytes in FIFO
	vdrvOpCodeStartBreak,						// Start a break signal.
	vdrvOpCodeStopBreak,							// Stop a break signal
	vdrvOpCodeStartLoopback,					// Start loopback mode.
	vdrvOpCodeStopLoopback,						// Stop loopback mode.
	vdrvOpCodeFlushTxFIFO,						// Flush the TX FIFO.
	vdrvOpCodeFlushRxFIFO,						// Flush the RX FIFO.
	vdrvOpCodeSendBufferedData,				// Send any buffered data in e vdrv.
	vdrvOpCodeRcvCheckIdle,						// Check idle state.
	vdrvOpCodeEmuSetBlockingHook,				// Special opCode for the simulator.
	vdrvOpCodeGetOptTransmitSize,				// Get the optimal TX buffer size for this port.
	vdrvOpCodeGetMaxRcvBlockSize,				// Get the optimal RX buffer size for this port.
	vdrvOpCodeNotifyBytesReadFromQ,			// Notify the vdrv bytes have been removed from Q.
	vdrvOpCodeSetDTRAsserted,					// Assert or deassert DTR signal
	vdrvOpCodeGetDTRAsserted,					// Yields 'true' if DTR is asserted, 'false' otherwise.
	vdrvOpCodeWaitForConfiguration,			// Some protocols like USB have an enumeration or 
														// discovery phase.  This control code is called
														// from Send and Receive to give the driver time to
														// wait until configuration completes\.
	vdrvOpCodeGetUSBDeviceDescriptor,		// Query driver for device descriptor for USB  	
	vdrvOpCodeGetUSBConfigDescriptor,		// Query driver for configuration descriptor for USB
	vdrvOpCodeEnableIRDA,						// Enable irda, if supported
	vdrvOpCodeDisableIRDA,						// Disable irda, if supported
	vdrvOpCodeEnableUART,						// Enable the UART
	vdrvOpCodeDisableUART,						// Disable the UART
	vdrvOpCodeRxEnable,							// Enable receive lines
	vdrvOpCodeRxDisable,							// Disable receive lines
	vdrvOpCodeLineEnable,						// Enable the RS-232 lines.
	vdrvOpCodeEnableUARTInterrupts,			// Enable the UART interrupts.
	vdrvOpCodeDisableUARTInterrupts,			// Disable the UART interrupts.
	vdrvOpCodeSetReceiveQueue,					// Set the receive queue
	
	vdrvOpCodeSaveState,                   // Save the state of the driver and port
	                                       // Used for port yielding
	                                       // A drive does not need to support this
	                                       // if it can not save state.
	vdrvOpCodeRestoreState,                // Restore the state of the driver and port
	                                       // Used for port yielding
	                                       // A drive does not need to support this
	                                       // if it can not save state.
	vdrvOpCodeSetYieldPortCallback,        // Set the yield port callback
	                                       // Port yielding is handled by the serial manager
	                                       // but if a virtual driver has a port beneath it
	                                       // that needs to be yieldable then it can pass the
	                                       // callback down to the port beneath it.
	vdrvOpCodeSetYieldPortRefCon,          // Set the yield port refCon
	                                       // Port yielding is handled by the serial manager
	                                       // but if a virtual driver has a port beneath it
	                                       // that needs to be yieldable then it can pass the
	                                       // refCon down to the port beneath it.
	
	vdrvOpCodeUserDef = 0x2000,							// User defined
	
	// --- Insert new control code above this line
	vdrvOpCodeSystem = 0x7000,					// All op codes in this range are reserved for the system
	vdrvOpCodeCustom = 0x8000					// All op codes in this range are reserved for licensees
} VdrvCtlOpCodeEnum;

#define vdrvOpCodeSystemStart		0x7000	// Start point for system op codes.
#define vdrvOpCodeCustomStart		0x8000	// Start point for custom op codes.

typedef struct VdrvConfigType {
	UInt32 baud;				// Baud rate to connect at
	UInt32 drvrId;				// Creator of the port that was opened
	UInt32 function;			// Function id of the connection
	MemPtr drvrDataP;			// Pointer to driver specific data.
	UInt16 drvrDataSize;		// Size of the driver specific data block.
	UInt32 sysReserved1;    // System Reserved
	UInt32 sysReserved2;    // System Reserved
} VdrvConfigType;

typedef VdrvConfigType *VdrvConfigPtr;


typedef void *VdrvDataPtr;

typedef Err (*VdrvOpenProcPtr)(VdrvDataPtr *drvrDataP, UInt32 baudRate, DrvrHWRcvQPtr rcvQP);
typedef Err (*VdrvOpenProcV4Ptr)(VdrvDataPtr *drvrDataP, VdrvConfigPtr configP, DrvrHWRcvQPtr rcvQP);

typedef Err (*VdrvCloseProcPtr)(VdrvDataPtr drvrDataP);

typedef UInt16 (*VdrvStatusProcPtr)(VdrvDataPtr drvrDataP);
typedef Err (*VdrvControlProcPtr)(VdrvDataPtr drvrDataP,
											VdrvCtlOpCodeEnum controlCode, 
								 			void *controlDataP, 
								 			UInt16 *controlDataLenP);

typedef Err (*VdrvReadProcPtr)(VdrvDataPtr drvrDataP, void **bufP, UInt32 *sizeP);
typedef UInt32 (*VdrvWriteProcPtr)(VdrvDataPtr drvrDataP, const void *bufP, UInt32 size, Err *errP);

typedef Err (*VdrvControlCustomProcPtr)(VdrvDataPtr drvrDataP, UInt16 opCode, UInt32 creator, void* controlDataP, 
					UInt16* controlDataLenP);


typedef struct {
	VdrvOpenProcPtr 		drvOpen;	
	VdrvCloseProcPtr 		drvClose;
	VdrvControlProcPtr 	drvControl;
	VdrvStatusProcPtr 	drvStatus;
	VdrvReadProcPtr 		drvRead;
	VdrvWriteProcPtr 		drvWrite;
	VdrvOpenProcV4Ptr		drvOpenV4;
	VdrvControlCustomProcPtr	drvControlCustom;
} VdrvAPIType;

typedef VdrvAPIType *VdrvAPIPtr;


// Normally, virtual drvr functions are accessed (by the NewSerialMgr)
// through the above SdrvAPIType structure of ProcPtrs.

// However, SerialMgrDbg.c (the Serial Mgr linked to the boot/debugger code)
// needs to call the HAL's debug serial code through the HAL_CALL macro.

// Note that this version of DrvOpen conforms with version four of the virtual driver 
// model.
Err VDrvOpen(VdrvDataPtr *drvrData, VdrvConfigPtr configP, DrvrHWRcvQPtr rcvQP)
		HAL_CALL(sysTrapDbgSerDrvOpen);

Err VDrvClose(VdrvDataPtr drvrData)
		HAL_CALL(sysTrapDbgSerDrvClose);

Err VDrvControl(VdrvDataPtr drvrData, VdrvCtlOpCodeEnum controlCode, 
					void *controlData, UInt16 *controlDataLen)
		HAL_CALL(sysTrapDbgSerDrvControl);
		

UInt16 VDrvStatus(VdrvDataPtr drvrData)
		HAL_CALL(sysTrapDbgSerDrvStatus);

UInt32 VDrvDbgWrite(VdrvDataPtr drvrData, void * bufP, UInt32 count, 
					Err* errP)
		HAL_CALL(sysTrapDbgSerDrvWriteChar);

UInt32 VDrvDbgRead(VdrvDataPtr drvrData, void * bufP, UInt32 count, 
					Int32 timeout, Err* errP)
		HAL_CALL(sysTrapDbgSerDrvReadChar);
		
// Note the debugger does not use Custom Control calls so there is no corresponding HAL
// trap.
Err VDrvCustomControl(VdrvDataPtr drvrData, UInt16 opCode, UInt32 creator, void* controlDataP, 
					UInt16* controlDataLenP);	

#endif		// __SERIALVDRV_H__
