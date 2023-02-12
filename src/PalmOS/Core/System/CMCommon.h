/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CMCommon.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Connection Management Protocol common definitions.
 *
 *****************************************************************************/

#ifndef __CM_COMMON_H__
#define __CM_COMMON_H__

#if EMULATION_LEVEL == 1		// EMULATION_WINDOWS
	
	// Pilot common definitions
	#include "PalmTypes.h"
	
#else	// EMULATION_LEVEL != EMULATION_WINDOWS
	
	// Pilot common definitions
	#include <PalmTypes.h>
	
#endif	// EMULATION_LEVEL == EMULATION_WINDOWS

#if EMULATION_WINDOWS != 1
	LOGIC ERROR
#endif



/********************************************************************
 * CMP 1.0 Command Format
 ********************************************************************/

// IT IS ANTICIPATED THAT the 1.0 PACKET FORMAT WILL BE USED TO INITIATE
// HOT SYNC "WAKE-UP" MESSAGE INDEFINITELY.  BASED ON THE COMMUNICATIONS
// VERSION NUMBER PROVIDED IN THIS BASIC COMMAND STRUCTURE, FUTURE VERSIONS
// OF DESKTOP AND PILOT SOFTWARE MAY SUPPORT ADDITIONAL CONNECTION MANAGEMENT
// COMMANDS/FEATURES AS THE NEED ARISES.  FOR EXAMPLE, TCP-ENABLED HOTSYNCS
// MAY PROVIDE ADDITIONAL INFORMATION, SUCH AS THE INTERNET ADDRESS OF THE
// PREFERRED HOTSYNC PC.  THIS DESIGN PROVIDES FOR BACKWARD COMPATIBILITY
// WITH OLDER VERSIONS OF THE SOFTWARE.  FUTURE COMMANDS WILL BE ADDED
// FOLLOWING THIS BASIC COMMAND SET.

// The CMP packets are used for transmitting data for the Pilot CMP protocol layer.
// The structure of the CMP packet is built into the data part of the SLP packet and is,
// therefore, constrained by the SLP packet data size limitations.


// Packet type values used in the CMP packet body "type" field:
//
typedef enum CmpType {
	cmpWakeup	= 1,			// CMP Wakeup packet sent from server to client
	cmpInit,						// CMP Init packet sent from client to server
	cmpAbort,					// CMP Abort packet sent from client to server
	cmpExtended					// For future command extensions
	} CmpType;


// cmpInit and cmpWakeup packet body structure:
//
typedef struct CmpBodyType {
	UInt8	type;								// CMP packet type

	UInt8	flags;							// in cmpInit packets: indicates which values were adjusted by
												// the client(defined below); in cmpWakeup pakcets: null;
												// in cmpAbort packets: reason for abort.

	UInt8	verMajor;						// Major PAD version number
	UInt8	verMinor;						// Minor PAD version number
	UInt16 wReserved;						// RESERVED -- set to NULL!!!
	//UInt32	commVersion;				// in cmpWakeup: server comm software version number;
												// in cmpInit: client comm version

	UInt32 baudRate;						// in cmpWakeup: max server baud rate;
												// in cmpInit packets: the prescribed baud rate
	} CmpBodyType;
	
typedef CmpBodyType*		CmpBodyPtr;

// Flags used in the packet body "flags" field of cmpInit
// packets.  These values are additive in the sense that they can be
// added or or'ed to form a composite value:
#define cmpInitFlagChangeBaudRate	0x80		// signals that the baud rate was adjusted by the client

// CM v1.1 extensions
#define cmpInitFlagRcvTOut1Min		0x40		// instructs the Pilot to set its receive timeout to 1 minute
#define cmpInitFlagRcvTOut2Min		0x20		// instructs the Pilot to set its receive timeout to 1 minute

// CM v1.2 extensions
#define cmpInitFlagLongPacketEnable	0x10		// Instructs the device to enable long packet support.

// Flags used in the packet body "flags" field of cmpAbort packets.
// These values are additive in the sense that they can be
// added or or'ed to form a composite value:
#define cmpAbortFlagVersionError		0x80		// signals incompatible comm versions

// Flags used in the packert body "flags" field of cmpWakeup packets.
// These values are additive in the sense that they can be
// added or or'ed to form a composite value:

// CM v1.2 extensions
#define cmpWakeupFlagLongPacketEnable	0x10	// Instructs the desktop that the device supports long
															// packets. 

/********************************************************************
 * CMP Protocol Parameters
 ********************************************************************/

// This transaction id is used by cmpWakeup packets.  cmpInit and cmpAbort
// packets MUST use a different transaction id.
//
#define cmpWakeupTransactionID		0xFF

// This is the baud rate at which CMP connections are initiated
//
#define cmpInitialBaudRate				9600L

// This is the time limit for trying to initiate a connection
//
#define cmpMaxInitiateSec				20


/********************************************************************
 * CMP v2.0 Extensible Command Format
 ********************************************************************/
 
// THE v2.0 COMMANDS USE A MORE FLEXIBLE STRUCTURE TO ANTICIPATE DYNAMIC
// ADDITION OF NEW ARGUMENTS.  THE COMMAND HEADER AND ARGUMENT IDENTIFIERS
// AND THEIR STRUCTURES WILL BE SPECIFIED IN THIS INCLUDE FILE.  THE PACKAGING
// OF THE ARGUMENTS WILL BE SPECIFIED IN THE TRANSPORT IMPLEMENTATION
// WHICH PROVIDES THE UTILITY ROUTINES TO PACKAGE DYNAMIC ARGUMENTS.

typedef struct CmpCommandHeaderType {
	UInt8	hdrType;										// set to cmpExtended
	UInt8	cmd;											// extended command/response id
	UInt16 errorCode;									// used in replies; set to null in commands!
	UInt8	argCount;									// number of arguments
	UInt8	reserved1;									// RESERVED - SET TO NULL!
	UInt32 reserved2;									// RESERVED - SET TO NULL!
	} CmpCommandHeaderType;


// Generic command type for 
typedef union CmpGenericCommandType {
	UInt8						hdrType;			// command header type(CmpType)
	CmpBodyType				body10;			// v1.0 command/response body
	CmpCommandHeaderType	exHdr;			// v2.0 extended command/response header
	} CmpGenericCommandType;
	
typedef CmpGenericCommandType*		CmpGenericCommandPtr;


// cmpRespBit is used to form a response ID from a
// command ID by or'ing it with the function ID.
#define cmpRespBit				0x80

// cmpCmdIDMask is used to mask out the command/response ID value
#define cmpCmdIDMask				0x7f

// Command/Response argument id's begin at this value
#define cmpFirstArgID			0x20


//-------------------------------------------------------------------------
// Connection Management error codes returned in the response errorCode
// field.
//-------------------------------------------------------------------------
typedef enum CmpRespErrEnum {
	cmpRespErrNone = 0,					// reserve 0 for no error
	cmpRespErrSystem,						// general Pilot system error
	cmpRespErrUnknownCmd,				// unknown command ID
	cmpRespErrMemory,						// insufficient memory
	cmpRespErrParam,						// invalid parameter
	
	cmpRespErrLast							// *KEEP THIS ENTRY LAST*
	} CmpRespErrEnum;


//-------------------------------------------------------------------------
// Connection Management Extended Command ID's
//-------------------------------------------------------------------------
typedef enum CmpCmdEnum {

	cmpCmdReserved = 0x0F,			// range reserved for internal use
	
	//----------------------------
	// v2.0 COMMANDS
	//----------------------------
	cmpCmdXCommPrefs,					// exchange communications protocol preferences
	cmpCmdHShakeComplete,			// hand-shake completion command
	
	cmpCmdLast							// *KEEP THIS ENTRY LAST*
	} CmpCmdEnum;



//-------------------------------------------------------------------------
// v2.0 Connection Management Command/Response Structures
//-------------------------------------------------------------------------

// CmpCommPrefsType: common data structure shared by cmpCmdXCommPrefs and
// cmpCmdHShakeComplete.  In cmpCmdXCommPrefs, the data structure is used
// to exchange capabilities.  In cmpCmdHShakeComplete, the structure is
// used to instruct the Pilot on which settings/features to use.  Some
// fields apply to direct PAD-based connections only and should be ignored
// by other transports such as TCP.  
//
typedef struct CmpCommPrefsType {
												// OFFSET
	UInt32				maxPktDataSize;//	0;		maximum supported packet data size
	UInt32				maxDataBlkSize;//	4;		maximum supported data block size
	UInt32				maxBaudRate;	//	8;		maximum supported baud rate/baud rate to use
	UInt32				hwHShakeAbove;	// 12;	requires hardware handshake above this baud rate
	UInt32				flags;			//	16;	flags
	UInt32				version;			//	20;	transport version (comm version)
	UInt32				reserved2;		//	24;	RESERVED - SET TO NULL!
	UInt32				reserved3;		//	28;	RESERVED - SET TO NULL!
												// TOTAL: 32 bytes;
	} CmpCommPrefsType;
	

//--------------------------------------
// Packet-level CRC preferences
//--------------------------------------

// cmpCommPrefsFlagSupportPktCRC16: if set, the transport layer is capable of
// CRC-16 protection of every data packet(v2.0 extension). (cmpCmdXCommPrefs)
#define cmpCommPrefsFlagSupportPktCRC16		0x80000000L

// cmpCommPrefsFlagUsePktCRC16: if set, instructs the transport layer to use
// the CRC-16 protection of every data packet(v2.0 extension). (cmpCmdHShakeComplete)
#define cmpCommPrefsFlagUsePktCRC16				0x00008000L

//--------------------------------------
// Data offset size preferences
//--------------------------------------

// NOTE:
// If specifying Int16 offsets, Int16 argument wrappers will be used.
// If specifying long offsets, long argument wrappers will be used.

// cmpCommPrefsFlagSupportShortOffsets: if set, indicates that the transport layer
// supports Int16(single-word) offsets/sizes in its data packets.  The smaller
// field does not permit bigger blocks to be transmitted(v1.0-compatible). (cmpCmdXCommPrefs)
#define cmpCommPrefsFlagSupportShortOffsets	0x40000000L

// cmpCommPrefsFlagUseShortOffsets: if set, instructs the transport layer to use
// Int16(single-word) offsets/sizes in its data packets(v1.0-compatible). (cmpCmdHShakeComplete)
#define cmpCommPrefsFlagUseShortOffsets		0x00004000L

// cmpCommPrefsFlagSupportLongOffsets: if set, indicates that the transport layer
// supports long(double-word) offsets/sizes in its data packets.  The larger
// field permits bigger blocks to be transmitted(v2.0 extension). (cmpCmdXCommPrefs)
#define cmpCommPrefsFlagSupportLongOffsets	0x20000000L

// cmpCommPrefsFlagUseLongOffsets: if set, instructs the transport layer to use
// long(double-word) offsets/sizes in its data packets(v2.0 extension). (cmpCmdHShakeComplete)
#define cmpCommPrefsFlagUseLongOffsets			0x00002000L



//////////////////////////////////////////////////////////////////////////
//	cmpCmdXCommPrefs
//////////////////////////////////////////////////////////////////////////
//
//		Description:
//			Exchange communications preferences.
//
//		Possible error codes
//			cmpRespErrSystem,
//			cmpRespErrMemory,
//			cmpRespErrParam,

//---------------------
// Command arguments:
//---------------------
enum {
	cmpXCommPrefsReqArgID = cmpFirstArgID
	};


// cmpXCommPrefsReqArgID argument structure:
//
typedef struct CmpXCommPrefsReqType {
												// OFFSET
	CmpCommPrefsType	prefs;			//	0;		CMP comm preferences
												// TOTAL: 32 bytes;
	} CmpXCommPrefsReqType;


//---------------------
// Response arguments:
//---------------------
enum {
	cmpXCommPrefsPrefsRespArgID = cmpFirstArgID,		// preferences argument
	cmpXCommPrefsIPAddrRespArgID							// preferred PC IP Address/host name
	};


// cmpXCommPrefsPrefsRespArgID argument structure:
//
typedef struct CmpXCommPrefsPrefsRespType {
												// OFFSET
	CmpCommPrefsType	prefs;			//	0;		CMP comm preferences
												// TOTAL: 32 bytes;
	} CmpXCommPrefsPrefsRespType;


// cmpXCommPrefsIPAddrRespArgID argument structure:
//
typedef struct CmpXCommPrefsIPAddrRespType {
												// OFFSET
	UInt8					ipAddr[2];		//	0;		IP address/host name string, zero-terminated
												// TOTAL: variable size;
	} CmpXCommPrefsIPAddrRespType;



//////////////////////////////////////////////////////////////////////////
//	cmpCmdHShakeComplete
//////////////////////////////////////////////////////////////////////////
//
//		Description:
//			Completes the Extended Connection Management hand-shake.  Instructs
//			the Pilot on which settings/features to use.
//
//		Possible error codes
//			cmpRespErrSystem,
//			cmpRespErrMemory,
//			cmpRespErrParam

//---------------------
// Command arguments:
//---------------------
enum {
	cmpHShakeCompleteReqArgID = cmpFirstArgID
	};


// cmpHShakeCompleteReqArgID argument structure:
//
typedef struct CmpHShakeCompleteReqType {
												// OFFSET
	CmpCommPrefsType	final;			//	0;		final settings/features to use
												// TOTAL: 32 bytes;
	} CmpHShakeCompleteReqType;


//---------------------
// Response arguments:
//---------------------

// No response arguments;  the response consists of the command header only



#endif	// __CM_COMMON_H__
