/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialSdrv.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Constants and data structures for serial drvr ('sdrv') code.
 *
 *****************************************************************************/

#ifndef __SERIALSDRV_H__
#define __SERIALSDRV_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <HAL.h>

#include <SerialDrvr.h>

	
// ¥¥¥¥¥¥¥¥¥¥ Constants

#define kSdrvResType			'sdrv'


// ¥¥¥¥¥¥¥¥¥¥ Typdefs

typedef enum SdrvCtlOpCodeEnum {				// Control function opCodes
	sdrvOpCodeNoOp = 0, 
	sdrvOpCodeSetBaudRate = 0x1000,			// Set baud rate
	sdrvOpCodeSetSettingsFlags,				// Set port send/rcv settings.
	sdrvOpCodeClearErr,							// Clear any HW errors.
	sdrvOpCodeEnableUART,						// Enable the UART.
	sdrvOpCodeDisableUART,						// Disable the UART.
	sdrvOpCodeEnableUARTInterrupts,			// Enable the UART interrupts.
	sdrvOpCodeDisableUARTInterrupts,			// Disable the UART interrupts.
	sdrvOpCodeSetSleepMode,						// Put the HW in sleep mode.
	sdrvOpCodeSetWakeupMode,					// Wake the HW from sleep mode.
	sdrvOpCodeRxEnable,							// Enable the RX lines.
	sdrvOpCodeRxDisable,							// Disbale the RX lines.
	sdrvOpCodeLineEnable,						// Enable the RS-232 lines.
	sdrvOpCodeFIFOCount,							// Return bytes in HW FIFO.
	sdrvOpCodeEnableIRDA,						// Enable the IR mode for the UART.
	sdrvOpCodeDisableIRDA,						// Disable the IR mode for the UART.
	sdrvOpCodeStartBreak,						// Start a break signal.
	sdrvOpCodeStopBreak,							// Stop a break signal.
	sdrvOpCodeStartLoopback,					// Start loopback mode.
	sdrvOpCodeStopLoopback,						// Stop loopback mode.
	sdrvOpCodeFlushTxFIFO,						// Flush HW TX FIFO.
	sdrvOpCodeFlushRxFIFO,						// Flsuh HW RX FIFO.
	sdrvOpCodeGetOptTransmitSize,				// Get HW optimal buffer size.
	sdrvOpCodeEnableRTS,							// De-assert the RTS line to allow data to be received.
	sdrvOpCodeDisableRTS,						// Assert the RTS line to prevent rcv buffer overflows.
	sdrvOpCodeSetDTRAsserted,					// Assert or deassert DTR signal
	sdrvOpCodeGetDTRAsserted,					// Yields 'true' if DTR is asserted, 'false' otherwise.
	sdrvOpCodeUserDef = 0x2000,
	
	// --- Insert new control code above this line
	sdrvOpCodeSystem = 0x7000,
	sdrvOpCodeCustom = 0x8000
} SdrvCtlOpCodeEnum;

#define sdrvOpCodeSystemStart		0x7000	// Start poitn for system op codes.
#define sdrvOpCodeCustomStart		0x8000	// Start point for custom op codes.

typedef void *SdrvDataPtr;

#if EMULATION_LEVEL == EMULATION_NONE && !defined(__GNUC__)

typedef void (*SerialMgrISPProcPtr)(void *portP:__A0);

typedef Err (*SdrvOpenProcPtr)(SdrvDataPtr *drvrDataP, 
										 UInt32 baudRate, 
										 void *portP, 
										 SerialMgrISPProcPtr saveDataProc);
typedef Err (*SdrvCloseProcPtr)(SdrvDataPtr drvrDataP);
typedef Err (*SdrvControlProcPtr)(SdrvDataPtr drvrDataP,
											SdrvCtlOpCodeEnum controlCode, 
								 			void *controlDataP, 
								 			UInt16 *controlDataLenP);
typedef UInt16 (*SdrvStatusProcPtr)(SdrvDataPtr drvrDataP);
typedef UInt16 (*SdrvReadCharProcPtr)(SdrvDataPtr drvrDataP:__A0):__D0;
typedef Err (*SdrvWriteCharProcPtr)(SdrvDataPtr drvrDataP, UInt8 aChar);

#else
typedef void (*SerialMgrISPProcPtr)(void *portP);
typedef Err (*SdrvOpenProcPtr)(SdrvDataPtr *drvrDataP, 
										 UInt32 baudRate, 
										 void *portP, 
										 void *saveDataProc);
typedef Err (*SdrvCloseProcPtr)(SdrvDataPtr drvrDataP);
typedef Err (*SdrvControlProcPtr)(SdrvDataPtr drvrDataP,
											SdrvCtlOpCodeEnum controlCode, 
								 			void *controlDataP, 
								 			UInt16 *controlDataLenP);
typedef UInt16 (*SdrvStatusProcPtr)(SdrvDataPtr drvrDataP);
typedef UInt16 (*SdrvReadCharProcPtr)(SdrvDataPtr drvrDataP);
typedef Err (*SdrvWriteCharProcPtr)(SdrvDataPtr drvrDataP, UInt8 aChar);
#endif


typedef struct {
	SdrvOpenProcPtr 		drvOpen;
	SdrvCloseProcPtr 		drvClose;
	SdrvControlProcPtr 	drvControl;
	SdrvStatusProcPtr 	drvStatus;
	SdrvReadCharProcPtr 	drvReadChar;
	SdrvWriteCharProcPtr drvWriteChar;
} SdrvAPIType;

typedef SdrvAPIType *SdrvAPIPtr;


// Normally, serial drvr functions are accessed (by the NewSerialMgr)
// through the above SdrvAPIType structure of ProcPtrs.

// However, SerialMgrDbg.c (the Serial Mgr linked to the boot/debugger code)
// needs to call the HAL's debug serial code through the HAL_CALL macro.


Err DrvOpen(SdrvDataPtr *drvrData, UInt32 baudRate, void *portP, 
				SerialMgrISPProcPtr saveDataProc)
		HAL_CALL(sysTrapDbgSerDrvOpen);

Err DrvClose(SdrvDataPtr drvrData)
		HAL_CALL(sysTrapDbgSerDrvClose);

Err DrvControl(SdrvDataPtr drvrData, SdrvCtlOpCodeEnum controlCode, 
					void *controlData, UInt16 *controlDataLen)
		HAL_CALL(sysTrapDbgSerDrvControl);

UInt16 DrvStatus(SdrvDataPtr drvrData)
		HAL_CALL(sysTrapDbgSerDrvStatus);

Err DrvWriteChar(SdrvDataPtr drvrData, UInt8 aChar)
		HAL_CALL(sysTrapDbgSerDrvWriteChar);

#if EMULATION_LEVEL == EMULATION_NONE && !defined(__GNUC__)
#pragma parameter __D0 DrvReadChar(__A0)
#endif
UInt16 DrvReadChar(SdrvDataPtr drvrData)
		HAL_CALL(sysTrapDbgSerDrvReadChar);

#endif		// __SERIALSDRV_H__
