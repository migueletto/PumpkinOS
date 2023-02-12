/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SystemPkt.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Structure of System Packets for the Serial Link Manager. These
 *	packets are used by the Debugger, Console, and Remote UI modules
 *	for communication with the host computer.
 *
 *****************************************************************************/

#ifndef __SYSTEM_PKT_H__
#define __SYSTEM_PKT_H__

#include <SerialLinkMgr.h>


//*************************************************************************
//   Constants for System Packets
//
//*************************************************************************

// Max # of bytes we can read/write at a time with the ReadMem and WriteMem
//  commands;
#define	sysPktMaxMemChunk			256

// Maximum body size for System Packets. This is big enough to have
//  256 bytes of data for the Read and Write Mem command plus whatever other
//  parameters are in the body for these commands.
#define	sysPktMaxBodySize			(sysPktMaxMemChunk+16)


// Default nitial timeout value for packet receive routines in ticks
#define	sysPktInitialTimeout		(sysTicksPerSecond*10)



//*************************************************************************
// Packet Body Structure
//*************************************************************************

// Common fields at the start of every body
#define	_sysPktBodyCommon					\
	UInt8	command;								\
	UInt8	_filler
	

// Generic System Packet Body
typedef struct SysPktBodyType {
	_sysPktBodyCommon;
	UInt8				data[sysPktMaxBodySize-2];
	} SysPktBodyType;
typedef SysPktBodyType*	SysPktBodyPtr;



//*************************************************************************
// The max size of the array of SlkWriteDataTypes used by System Packet
//  assembly routines in order to minimize stack usage.
//*************************************************************************
#define	sysPktMaxBodyChunks	3



//*************************************************************************
// packet commands
//*************************************************************************

//--------------------------------------------------------------------
// get state command
//--------------------------------------------------------------------
#define	sysPktStateCmd					0x00
#define	sysPktStateRsp					0x80

//--------------------------------------------------------------------
// read memory command
//--------------------------------------------------------------------
#define	sysPktReadMemCmd				0x01
#define	sysPktReadMemRsp				0x81
				
typedef struct SysPktReadMemCmdType {
	_sysPktBodyCommon;								// Common Body header
	void*					address;						// Address to read
	UInt16					numBytes;					// # of bytes to read
	} SysPktReadMemCmdType;
typedef SysPktReadMemCmdType*	SysPktReadMemCmdPtr;

typedef struct SysPktReadMemRspType {
	_sysPktBodyCommon;								// Common Body header
	// UInt8				data[?];						// variable size
	} SysPktReadMemRspType;
typedef SysPktReadMemRspType*	SysPktReadMemRspPtr;


//--------------------------------------------------------------------
// write memory command
//--------------------------------------------------------------------
#define	sysPktWriteMemCmd				0x02
#define	sysPktWriteMemRsp				0x82

typedef struct SysPktWriteMemCmdType {
	_sysPktBodyCommon;								// Common Body header
	void*					address;						// Address to write
	UInt16				numBytes;					// # of bytes to write
	// UInt8				data[?];						// variable size data
	} SysPktWriteMemCmdType;
typedef SysPktWriteMemCmdType*	SysPktWriteMemCmdPtr;

typedef struct SysPktWriteMemRspType {
	_sysPktBodyCommon;								// Common Body header
	} SysPktWriteMemRspType;
typedef SysPktWriteMemRspType*	SysPktWriteMemRspPtr;


//--------------------------------------------------------------------
// single-step command
//--------------------------------------------------------------------
#define	sysPktSingleStepCmd			0x03
// no response

//--------------------------------------------------------------------
// get routine name command
//--------------------------------------------------------------------
#define	sysPktGetRtnNameCmd			0x04
#define	sysPktGetRtnNameRsp			0x84

//--------------------------------------------------------------------
// read registers command
//--------------------------------------------------------------------
#define	sysPktReadRegsCmd				0x05
#define	sysPktReadRegsRsp				0x85


//--------------------------------------------------------------------
// write registers command
//--------------------------------------------------------------------
#define	sysPktWriteRegsCmd			0x06
#define	sysPktWriteRegsRsp			0x86

//--------------------------------------------------------------------
// continue command
//--------------------------------------------------------------------
#define	sysPktContinueCmd				0x07
// no response


//--------------------------------------------------------------------
// Remote Procedure call
//--------------------------------------------------------------------
#define	sysPktRPCCmd					0x0A
#define	sysPktRPCRsp					0x8A

typedef struct SysPktRPCParamInfo {
	UInt8 byRef;							// true if param is by reference
	UInt8	size;								// # of Bytes of paramData	(must be even)			
	UInt16	data[1];						// variable length array of paramData
	} SysPktRPCParamType;
	
typedef struct SysPktRPCType {
	_sysPktBodyCommon;					// Common Body header
	UInt16	trapWord;					// which trap to execute
	UInt32	resultD0;					// result from D0 placed here
	UInt32	resultA0;					// result from A0 placed here
	UInt16	numParams;					// how many parameters follow
	// Following is a variable length array ofSlkRPCParamInfo's
	SysPktRPCParamType	param[1];
	} SysPktRPCType;


//--------------------------------------------------------------------
// Set/Get breakpoints
//--------------------------------------------------------------------
#define	sysPktGetBreakpointsCmd		0x0B
#define	sysPktGetBreakpointsRsp		0x8B
#define	sysPktSetBreakpointsCmd		0x0C
#define	sysPktSetBreakpointsRsp		0x8C


//--------------------------------------------------------------------
// Remote UI Support - These packets are used by the screen driver
//  and event manager to support remote viewing and control of a Pilot
//  over the serial port.
//--------------------------------------------------------------------
#define	sysPktRemoteUIUpdCmd			0x0C

typedef struct SysPktRemoteUIUpdCmdType {
	_sysPktBodyCommon;								// Common Body header

  // These parameters are sent from traget to host after drawing operations
  	UInt16	rowBytes;						// rowbytes of update area
	UInt16	fromY;							// top of update rect
	UInt16	fromX;							// left of update rect
	UInt16	toY;								// top of screen rect
	UInt16	toX;								// left of screen rect
	UInt16	height;							// bottom of update rect
	UInt16	width;							// right of update rect
	
  // The actual pixels of the update area follow
  	UInt16	pixels;							// variable length...
	
	} SysPktRemoteUIUpdCmdType;


#define	sysPktRemoteEvtCmd			0x0D

typedef struct SysPktRemoteEvtCmdType {
	_sysPktBodyCommon;						// Common Body header

  // These parameters are sent from host to target to feed pen and keyboard
  //  events. They do not require a response.
   Boolean 	penDown;							// true if pen down
	UInt8 	padding1;
  	Int16		penX;								// X location of pen  
  	Int16		penY;								// Y location of pen  
  	
  	Boolean	keyPress;						// true if key event follows
	UInt8 	padding2;
  	UInt16	keyModifiers;					// keyboard modifiers
  	WChar		keyAscii;						// key ascii code
  	UInt16	keyCode;							// key virtual code
  	
	} SysPktRemoteEvtCmdType;


//--------------------------------------------------------------------
// Enable/Disable DbgBreak's command
//--------------------------------------------------------------------
#define	sysPktDbgBreakToggleCmd			0x0D
#define	sysPktDbgBreakToggleRsp			0x8D


//--------------------------------------------------------------------
// Program Flash command - programs one sector of the FLASH ram
// If numBytes is 0, this routine returns info on the flash in:
//   manuf - manufacturer code
//   device - device code
//--------------------------------------------------------------------
#define	sysPktFlashCmd						0x0E		// OBSOLETE AS OF 3.0! SEE BELOW!
#define	sysPktFlashRsp						0x8E		// OSBOLETE AS OF 3.0! SEE BELOW!


//--------------------------------------------------------------------
// Get/Set communication parameters
//--------------------------------------------------------------------
#define	sysPktCommCmd						0x0F
#define	sysPktCommRsp						0x8F

typedef struct SysPktCommCmdType {
	_sysPktBodyCommon;								// Common Body header
	Boolean						set;					// true to change parameters
	UInt8 						padding;
	UInt32						baudRate;			// new baud rate
	UInt32						flags;				// new flags
	} SysPktCommCmdType;
typedef SysPktCommCmdType *SysPktCommCmdPtr;

typedef struct SysPktCommRspType {
	_sysPktBodyCommon;								// Common Body header
	UInt32						baudRate;			// current baud rate
	UInt32						flags;				// current flags
	} SysPktCommRspType;
typedef SysPktCommRspType *SysPktCommRspPtr;


//--------------------------------------------------------------------
// Get/Set Trap Breaks 
//--------------------------------------------------------------------
#define	sysPktGetTrapBreaksCmd			0x10
#define	sysPktGetTrapBreaksRsp			0x90
#define	sysPktSetTrapBreaksCmd			0x11
#define	sysPktSetTrapBreaksRsp			0x91


//--------------------------------------------------------------------
// Gremlins Support - These packets are used by the screen driver
//  and event manager to support remote viewing and control of a Pilot
//  over the serial port.
//--------------------------------------------------------------------
#define	sysPktGremlinsCmd				0x12

typedef struct SysPktGremlinsCmdType {
	_sysPktBodyCommon;								// Common Body header

  // These parameters are sent from target to host to send Gremlins stuff
  UInt16	action;
  UInt8	data[32];
	
	} SysPktGremlinsCmdType;
	
// Gremlins action codes
#define	sysPktGremlinsIdle			1
#define	sysPktGremlinsEvent			2


//--------------------------------------------------------------------
// Find data - searches a range of addresses for data
//--------------------------------------------------------------------
#define	sysPktFindCmd				0x13
#define	sysPktFindRsp				(sysPktFindCmd | 0x80)

typedef struct SysPktFindCmdType {
	_sysPktBodyCommon;								// Common Body header

	UInt32	firstAddr;								// first address to search
	UInt32	lastAddr;								// last address to begin searching
	UInt16	numBytes;								// number of data bytes to match
	Boolean	caseInsensitive;						// if true, perform a case-insensitive search
	UInt8 	padding;
	} SysPktFindCmdType;
typedef SysPktFindCmdType*			SysPktFindCmdPtr;

typedef struct SysPktFindRspType {
	_sysPktBodyCommon;								// Common Body header

  UInt32		addr;										// address where data was found
  Boolean	found;									// true if data was found
  UInt8 		padding;
	
	} SysPktFindRspType;
typedef SysPktFindRspType*			SysPktFindRspPtr;


//--------------------------------------------------------------------
// Get/Set Trap Conditionals. These are used to tell the debugger
//  to conditionally break on a trap depending on the value of the
//  first word on the stack. They are used when setting a-traps on
//  library calls. This is a 3.0 feature. 
//--------------------------------------------------------------------
#define	sysPktGetTrapConditionsCmd			0x14
#define	sysPktGetTrapConditionsRsp			0x94
#define	sysPktSetTrapConditionsCmd			0x15
#define	sysPktSetTrapConditionsRsp			0x95


//--------------------------------------------------------------------
// Checksum data - checksums a range of memory.
// This is a (late) 3.0 feature.
//--------------------------------------------------------------------
#define	sysPktChecksumCmd				0x16
#define	sysPktChecksumRsp				(sysPktChecksumCmd | 0x80)

typedef struct SysPktChecksumType {
	_sysPktBodyCommon;								// Common Body header

	UInt32	firstAddr;								// -> first address to checksum
	UInt16	numBytes;								// -> number of bytes to checksum
	UInt16	seed;										// -> initial checksum value
	UInt16	checksum;								// <- checksum result
	} SysPktChecksumType;
typedef SysPktChecksumType*	SysPktChecksumPtr;


//--------------------------------------------------------------------
// NEW Program Flash command - programs one sector of the FLASH ram
// If numBytes is 0, this routine returns address to store flash code.
// Supercedes Obsolete 1.0 and 2.0 sysPktFlashCmd call above in the 3.0 ROM
//--------------------------------------------------------------------
#define	sysPktExecFlashCmd				0x17
#define	sysPktExecFlashRsp				(sysPktExecFlashCmd | 0x80)


//--------------------------------------------------------------------
// message from remote unit
//--------------------------------------------------------------------
#define	sysPktRemoteMsgCmd				0x7f


//--------------------------------------------------------------------
// sysPktRemoteMsg
// Send a text message
//--------------------------------------------------------------------
typedef struct SysPktRemoteMsgCmdType {
	_sysPktBodyCommon;								// Common Body header
	//UInt8		text;									// variable length text goes here
	} SysPktRemoteMsgCmdType;
typedef SysPktRemoteMsgCmdType *SysPktRemoteMsgCmdPtr;





/*******************************************************************
 * Prototypes
 *******************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//================================================================
//
// Host Only Routines.
//
//================================================================
#if (EMULATION_LEVEL != EMULATION_NONE)

	
	//-------------------------------------------------------------------
	// RPC
	//------------------------------------------------------------------
	// Init preparate on an RPC packet header and body.
	Err 		SlkRPCInitPacket(SlkPktHeaderType *headerP, UInt16 dstSocket,
						SysPktRPCType *bodyP, UInt16 trapWord);
	
	
	// Stuff a parameter into an RPC packet body
	void *	SlkRPCStuffParam(SysPktRPCType *bodyP, const void *dataP,
							Int16 dataSize, Boolean byRef);
	
	// Send RPC packet and wait for response.
	Err		SlkRPCExecute(SlkPktHeaderPtr headerP, SysPktRPCType *bodyP, 
							Boolean async);
	
#endif	// (EMULATION_LEVEL != EMULATION_NONE)


#ifdef __cplusplus
}
#endif
	



#endif //__SYSTEM_PKT_H__
