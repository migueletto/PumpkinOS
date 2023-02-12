/******************************************************************************
 *
 * Copyright (c) 1997-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ExgLib.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file the Exchange Library interface. The Exchange Library is a
 *		generic interface to any number of librarys. Any Exchange Library
 *		MUST have entrypoint traps in exactly the order listed here.
 *		The System Exchange manager functions call these functions when 
 *		applications make calls to the Exchange manager. Applications will
 *		usually not make direct calls to this API.
 *
 *****************************************************************************/

#ifndef __EXGLIB_H__
#define __EXGLIB_H__

#include <PalmTypes.h>
#include <LibTraps.h>
#include <ExgMgr.h>

// special exchange mgr event key
#define exgIntDataChr 0x01ff

//-----------------------------------------------------------------------------
// 	Obx library call ID's. Each library call gets the trap number:
//   exgTrapXXXX which serves as an index into the library's dispatch table.
//   The constant sysLibTrapCustom is the first available trap number after
//   the system predefined library traps Open,Close,Sleep & Wake.
//
// WARNING!!! This order of these traps MUST match the order of the dispatch
//  table in and Exchange library!!!
//-----------------------------------------------------------------------------

#define exgLibTrapHandleEvent		(sysLibTrapCustom)
#define exgLibTrapConnect			(sysLibTrapCustom+1)
#define exgLibTrapAccept			(sysLibTrapCustom+2)
#define exgLibTrapDisconnect		(sysLibTrapCustom+3)
#define exgLibTrapPut				(sysLibTrapCustom+4)
#define exgLibTrapGet				(sysLibTrapCustom+5)
#define exgLibTrapSend				(sysLibTrapCustom+6)
#define exgLibTrapReceive			(sysLibTrapCustom+7)
#define exgLibTrapControl			(sysLibTrapCustom+8)
#define exgLibTrapRequest			(sysLibTrapCustom+9)
#define exgLibTrapReserved1			(sysLibTrapCustom+10)
#define exgLibTrapReserved2			(sysLibTrapCustom+11)
#define exgLibTrapReserved3			(sysLibTrapCustom+12)
#define exgLibTrapReserved4			(sysLibTrapCustom+13)
#define exgLibTrapReserved5			(sysLibTrapCustom+14)
#define exgLibTrapReserved6			(sysLibTrapCustom+15)
#define exgLibTrapReserved7			(sysLibTrapCustom+16)
#define exgLibTrapReserved8			(sysLibTrapCustom+17)
#define exgLibTrapReserved9			(sysLibTrapCustom+18)
#define exgLibTrapReserved10		(sysLibTrapCustom+19)
#define exgLibTrapLast				(sysLibTrapCustom+20)


/************************************************************
 * Net Library procedures.
 *************************************************************/ 
#pragma mark Functions
#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------
// Library initialization, shutdown, sleep and wake
//--------------------------------------------------
// Open the library - enable server for receiving data.
Err		ExgLibOpen (UInt16 libRefnum)
			SYS_TRAP(sysLibTrapOpen);
					
Err		ExgLibClose (UInt16 libRefnum)
			SYS_TRAP(sysLibTrapClose);
					
Err		ExgLibSleep (UInt16 libRefnum)
			SYS_TRAP(sysLibTrapSleep);
					
Err		ExgLibWake (UInt16 libRefnum)
			SYS_TRAP(sysLibTrapWake);
					
//	Handle events that this library needs. This will be called by
//	sysHandle event when certain low level events are triggered.					
Boolean		ExgLibHandleEvent(UInt16 libRefnum, void *eventP)
			SYS_TRAP(exgLibTrapHandleEvent);
						
//  Establish a new connection 						
Err	 	ExgLibConnect(UInt16 libRefNum, ExgSocketType *exgSocketP)
			SYS_TRAP(exgLibTrapConnect);

// Accept a connection request from remote end
Err		ExgLibAccept(UInt16 libRefnum, ExgSocketType *exgSocketP)
			SYS_TRAP(exgLibTrapAccept);

// Disconnect
Err		ExgLibDisconnect(UInt16 libRefnum, ExgSocketType *exgSocketP, Err error)
			SYS_TRAP(exgLibTrapDisconnect);

// Initiate a Put command. This passes the name and other information about
// an object to be sent
Err		ExgLibPut(UInt16 libRefnum, ExgSocketType *exgSocketP)
			SYS_TRAP(exgLibTrapPut);

// Initiate a Get command. This requests an object from the remote end.
Err		ExgLibGet(UInt16 libRefNum, ExgSocketType *exgSocketP)
			SYS_TRAP(exgLibTrapGet);

// Send data to remote end - called after a Put command
UInt32 	ExgLibSend(UInt16 libRefNum, ExgSocketType *exgSocketP, const void *bufP, UInt32 bufLen, Err *errP)
			SYS_TRAP(exgLibTrapSend);

// Receive data from remote end -- called after Accept
UInt32 	ExgLibReceive(UInt16 libRefNum, ExgSocketType *exgSocketP, void *bufP, UInt32 bufSize, Err *errP)
			SYS_TRAP(exgLibTrapReceive);

// Send various option commands to the Exg library
Err 	ExgLibControl(UInt16 libRefNum, UInt16 op, void *valueP, UInt16 *valueLenP)
			SYS_TRAP(exgLibTrapControl);

// Tell the Exg library to check for incoming data
Err 	ExgLibRequest(UInt16 libRefNum, ExgSocketType *socketP)
			SYS_TRAP(exgLibTrapRequest);

						
#ifdef __cplusplus
}
#endif

#endif  // __EXGLIB_H__
