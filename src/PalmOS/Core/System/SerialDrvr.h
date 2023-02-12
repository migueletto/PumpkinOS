/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SerialDrvr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Constants and data structures for serial drvr ('sdrv') code.
 *
 *****************************************************************************/

#ifndef __SERIALDRVR_H__
#define __SERIALDRVR_H__

#include <PalmTypes.h>

// ********** Constants

#define kDrvrCreator				0
#define kDrvrResID				0
#define kDrvrCODEType			'code'


// kDrvrVersion is included by all sdrv's and vdrv's and is returned by the drvr to the
// serial manager to show the driver version is consistent with the required version of
// drvr the new serial manager needs in order to operate properly. 
//
// Note: Version 4 driver were introduced in PalmOS 4.0.  

#define kDrvrVersion3			3
#define kDrvrVersion4			4

#define kDrvrVersion				kDrvrVersion4

#define kMaxPortDescStrLen		64
#define kPortDescStrID			1000

// Flags denoting capabilities and features of this port.

#define portPhysicalPort		0x00000001		// Should be unset for virtual port.

#define portBkgndModeSupported 0x00000002    // Denotes that this port can be used for background mode
                                             // This flag is only applicable to virtual drivers.  Background
                                             // mode support is implied on physical drivers.

#define portRS232Capable		0x00000004		// Denotes this serialHW has a RS-232 port.
#define portIRDACapable			0x00000008		// Denotes this serialHW has a IR port and support IRDA mode.

#define portCradlePort 			0x00000010		// Denotes this SerialHW controls the cradle port.
#define portExternalPort		0x00000020		// Denotes this SerialHW's port is external or on a memory card.
#define portModemPort			0x00000040		// Denotes this SerialHW communicates with a modem.
#define portConsolePort			0x00000100		// Denotes this SerialHW's port is the console port.

#define portCncMgrVisible		0x00000080		// Denotes this serial port's name is to be displayted in the Connection panel.
#define portPrivateUse			0x00001000		// Set if this drvr is for special software and NOT general apps in system.	
#define portUSBCapable  		0x00000200		// Denotes that the driver supports USB hardware.



// ********** Structs

typedef enum DrvrIRQEnum {
	drvrIRQNone = 0x00,
	drvrIRQ1 = 0x01,
	drvrIRQ2 = 0x02,
	drvrIRQ3 = 0x04,
	drvrIRQ4 = 0x08,
	drvrIRQ5 = 0x10,
	drvrIRQ6 = 0x20,
	drvrIRQOther = 0x40
} DrvrIRQEnum;
	

// ***** Info about this particular port

typedef struct {								
	UInt32 drvrID; 					// e.g. creator type, such as 'u328'
	UInt32 drvrVersion;				// version of code that works for this HW.
	UInt32 maxBaudRate;				// Maximum baud rate for this uart.
	UInt32 handshakeThreshold;		// Baud rate at which hardware handshaking should be used.
	UInt32 portFlags;					// flags denoting features of this uart.
	const Char *portDesc;					// Pointer to null-terminated string describing this HW.
	DrvrIRQEnum irqType;				// IRQ line for this uart serial HW.
	// Changed PalmOS 4.0 was reserved now multiple entries.
	UInt8 multipleEnumerations;
	// Need to know what the database id is if we are going to have one driver have
	// multiple ids.
	UInt32 dbCreator;
	
} DrvrInfoType;

typedef DrvrInfoType *DrvrInfoPtr;


typedef enum DrvrEntryOpCodeEnum {			// OpCodes for the entry function.
	drvrEntryGetUartFeatures,
	drvrEntryGetDrvrFuncts,
	drvrEntryGetUartFtrsNEntries 				// Opcode for drivers that have mulitiple
														// entries in the driver table.
} DrvrEntryOpCodeEnum;


typedef enum DrvrStatusEnum {
	drvrStatusCtsOn			= 0x0001,
	drvrStatusRtsOn			= 0x0002,
	drvrStatusDsrOn 			= 0x0004,
	drvrStatusTxFifoFull  	= 0x0008,
	drvrStatusTxFifoEmpty	= 0x0010,
	drvrStatusBreakAsserted	= 0x0020,
	drvrStatusDataReady		= 0x0040,		// For polling mode debugger only at this time.
	drvrStatusLineErr			= 0x0080			// For polling mode debugger only at this time.
} DrvrStatusEnum;	


// ********** Entry Point Function type

typedef Err (*DrvEntryPointProcPtr)(DrvrEntryOpCodeEnum opCode, void *uartData);


// ********** ADT and functions for Rcv Queue.

// Note that qGetSpace or GetSpaceProcPtr will return back at most one less then the 
// total size of the queue buffer.  The total usable space in the receive queue is the
// size of the queue minus one.

typedef Err (*WriteByteProcPtr)(void *theQ, UInt8 theByte, UInt16 lineErrs);
typedef Err (*WriteBlockProcPtr)(void *theQ, UInt8 *bufP, UInt16 size, UInt16 lineErrs);
typedef UInt32 (*GetSizeProcPtr)(void *theQ);
typedef UInt32 (*GetSpaceProcPtr)(void *theQ);
typedef void (*SignalCheckPtr)(void *theQ, UInt16 lineErrs);

typedef struct DrvrRcvQType {
	void *				rcvQ;
	WriteByteProcPtr	qWriteByte;
	WriteBlockProcPtr qWriteBlock;
	GetSizeProcPtr		qGetSize;
	GetSpaceProcPtr	qGetSpace;
	SignalCheckPtr		qSignalCheck;
	
} DrvrRcvQType;

typedef DrvrRcvQType *DrvrHWRcvQPtr;

// ***** Our receive queue.

typedef struct SrmRcvQType {
	UInt32 				qStart;					// start offset of queue
	UInt32 				qEnd;						// end offset of queue
	UInt32 				qSize;					// size of queue
	UInt8 				*qData;					// pointer to queue data					
	void*					qPort;					// Open port pointer for this Q.
} SrmRcvQType;

typedef SrmRcvQType *SrmRcvQPtr;

#endif		// __SERIALDRVR_H__
