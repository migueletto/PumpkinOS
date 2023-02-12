/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DebugMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Debugging functions
 *
 *****************************************************************************/

#ifndef __DEBUGMGR_H__
#define __DEBUGMGR_H__

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>				// Trap Numbers.


//
// Constants and structures used in conjunction with DbgControl
//
#define dbgCtlNotHandled		false			// debug control operation was not handled
#define dbgCtlHandled			true			// debug control operation was handled

#define dbgCtlAllHandlersID	0				// indiacates that the operation is to be handled
														// by all handlers

#define dbgCtlHandlerNameLen	31				// maximum handler name length, not including null
#define dbgCtlHandlerVerLen	15				// maximum handler version string length, not including null

#define dbgCtlFirstCustomOp	0x8000		// debug handlers begin numbering their op[erations]
														// with this number; the system reserves all operation
														// number values below this one
// Typed of the DbgControl function
typedef Boolean DbgControlFuncType(UInt32 handlerID, UInt16 op, void *paramP, UInt32 *dwParamP);

typedef struct DbgCtlHandlerInfoType {
	DbgControlFuncType*	handlerFuncP;		// pointer to handler's DbgControl function
	UInt32	version;								// numeric version number (hander-defined)
	Boolean	enabled;								// true if handler is enabled; false if not
	Char		name[dbgCtlHandlerNameLen+1];	// null-terminated handler name
	Char		ver[dbgCtlHandlerVerLen+1];	// null-terminated handler version string
	UInt8		reserved1;
	UInt32	dwReserved;							// RESERVED -- CALLER MUST INITIALIZE TO ZERO!
	} DbgCtlHandlerInfoType;

typedef void DbgCtlEnumCallbackFunc(void *callbackDataP, UInt32 handlerID, DbgControlFuncType*	handlerFuncP);

typedef struct DbgCtlEnumInfoType {
	DbgCtlEnumCallbackFunc*	enumFuncP;
	void *					callbackDataP;
	} DbgCtlEnumInfoType;

// System-defined debug control operations
enum {
	
	dbgCtlOpEnumHandlers = 1,			// handlerID = dbgCtlAllHandlersID (applies to all handlers)
												// paramP = pointer to DbgCtlEnumInfoType
												// returns dbgCtlHandled if handled
												
	dbgCtlOpGetHandlerInfo,				// handlerID = desired handler creator
												// paramP = pointer to DbgCtlHandlerInfoType
												// returns dbgCtlHandled if handled

	dbgCtlOpEnableHandler,				// handlerID = desired handler creator or dbgCtlAllHandlersID
												// returns dbgCtlHandled if handled

	dbgCtlOpDisableHandler,				// handlerID = desired handler creator or dbgCtlAllHandlersID
												// returns dbgCtlHandled if handled

	dbgCtlOpGetEnabledStatus,			// handlerID = desired handler creator
												// dwParamP = pointer to UInt32 type variable to be filled in with
												// enabled status: non-zero = enabled, zero = disabled
												// returns dbgCtlHandled if handled

	dbgCtlOpGetVersion,					// handlerID = desired handler creator
												// dwParamP = pointer to UInt32 type variable to be filled in with
												// handler-specific version number
												// returns dbgCtlHandled if handled
												
												
	dbgCtlOpLAST
	};


#ifdef PALMOS
#ifdef __GNUC__
 #if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
	#define _DEBUGGER_TRAP  __attribute__ ((__callseq__ ("trap #8")))
 #else
	#define _DEBUGGER_TRAP
 #endif
#elif defined (__MWERKS__)	/* The equivalent in CodeWarrior syntax */
	#define _DEBUGGER_TRAP  = 0x4E40 + 8
#endif
#else
	#define _DEBUGGER_TRAP
#endif


/************************************************************
 * Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Int32 		DbgInit(MemPtr spaceP, MemPtr dispatchTableP[], Boolean openComm);

void 			DbgSrcBreak(void)
#if USE_TRAPS
	/* Use a trap instruction unless we're a simulator or we've explicitly
	   set USE_TRAPS to 0.  */
	_DEBUGGER_TRAP
#endif
	;
 
void 			DbgSrcMessage(const Char *debugStr)
							SYS_TRAP(sysTrapDbgSrcMessage);


// Debug control function for implementing debug tracing, etc. via debug handlers.
// The default implementation does nothing, leaving the real work up to "debug handlers".
// Debug handlers will be implemented as system extensions.  As they are loaded, extensions
// will override this function call (DbgControl) and chain to those handlers loaded before
// them.  When a debug control call is made by the client, a handler id of the handler that
// implements the functionality will be passed in as the first parameter.  The handler id is
// the unique creator id of the handler.  When a handler is called, it will first examine
// the handler ID -- if it matches its own, the handler will execute the command and return;
// if the handler id does not match, the handler must pass the call down the chain and return
// the value from that call.  The operation to be performed is indicated by the parameter "op".
// op is specific to each handler this means that the same op values may be used by different
// handlers, since handler id's are unique. op values defined by handlers must begin at
// dbgCtlFirstCustomOp.  Handler ID of dbgCtlAllHandlersID applies to all handlers.  When the
// handler id of zero is passed, each handler is responsible for executing the requested action
// and passing the call down the chain.  The last two parameters are defined by each handler
// for its own operations.
//typedef Boolean DbgControlFuncType(UInt32 handlerID, UInt16 op, void *paramP, UInt32 *dwParamP);
extern DbgControlFuncType DbgControl
							SYS_TRAP(sysTrapDbgControl);

void			DbgBreak(void)
#if EMULATION_LEVEL == EMULATION_NONE
	/* Use a trap instruction unless we're a simulator (i.e., even if we've
	   explicitly set USE_TRAPS to 0).  In particular the SmallROM compiles
	   with USE_TRAPS set to 0 and needs the DbgBreak to resolve as a trap
	   instruction.  */
	_DEBUGGER_TRAP
#endif
	;

void 			DbgMessage(const Char *aStr)
							SYS_TRAP(sysTrapDbgMessage);
	
Char *		DbgGetMessage(UInt8 *bufferP, Int32 timeout)
							SYS_TRAP(sysTrapDbgGetMessage);
							
Err			DbgCommSettings(UInt32 *baudP, UInt32 *flagsP)
							SYS_TRAP(sysTrapDbgCommSettings);
	
#ifdef __cplusplus 
}
#endif


/************************************************************
 * Assembly Function Prototypes
 *************************************************************/

// This is an optimization for assembly code on the device.
#if EMULATION_LEVEL == EMULATION_NONE
#define	_DbgBreak		\
				DC.W	m68kTrapInstr+sysDbgTrapNum		//lint !e773
#endif				


#endif // __DEBUGMGR_H__
