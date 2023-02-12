/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ErrorBase.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Error Management 
 *
 * -----------------------------------------------------------------------
 *	Exception Handling
 *
 *		This unit implements an exception handling mechanism that is similar
 *		to "real" C++ Exceptions. Our Exceptions are untyped, and there
 *		must be one and only one Catch block for each Try block.
 *
 *	Try/Catch Syntax:
 *
 *		ErrTry {
 *			// Do something which may fail.
 *			// Call ErrThrow() to signal failure and force jump
 *			// to the following Catch block.
 *		}
 *
 *		ErrCatch(inErr) {
 *			// Recover or cleanup after a failure in the above Try block.
 *			// "inErr" is an ExceptionCode identifying the reason
 *			// for the failure.
 *			
 *			// You may call Throw() if you want to jump out to
 *			// the next Catch block.
 *
 *			// The code in this Catch block does not execute if
 *			// the above Try block completes without a Throw.
 *
 *		} ErrEndCatch
 *
 *		You must structure your code exactly as above. You can't have a
 *		ErrTry { } without a ErrCatch { } ErrEndCatch, or vice versa.
 *
 *
 *	ErrThrow
 *
 *		To signal failure, call ErrThrow() from within a Try block. The
 *		Throw can occur anywhere in the Try block, even within functions
 *		called from the Try block. A ErrThrow() will jump execution to the
 *		start of the nearest Catch block, even across function calls.
 *		Destructors for stack-based objects which go out of scope as
 *		a result of the ErrThrow() are called.
 *
 *		You can call ErrThrow() from within a Catch block to "rethrow"
 *		the exception to the next nearest Catch block.
 *
 *
 *	Exception Codes
 *
 *		An ExceptionCode is a 32-bit number. You will normally use
 *		Pilot error codes, which are 16-bit numbers. This allows
 *		plently of room for defining codes for your own kinds of errors.
 *
 *
 *	Limitations
 *
 *		Try/Catch and Throw are based on setjmp/longjmp. At the
 *		beginning of a Try block, setjmp saves the machine registers.
 *		Throw calls longjmp, which restores the registers and jumps
 *		to the beginning of the Catch block. Therefore, any changes
 *		in the Try block to variables stored in registers will not
 *		be retained when entering the Catch block. 
 *
 *		The solution is to declare variables that you want to use
 *		in both the Try and Catch blocks as "volatile". For example:
 *
 *		volatile long	x = 1;		// Declare volatile local variable
 *		ErrTry {
 *			x = 100;						// Set local variable in Try
 *			ErrThrow(-1);
 *		}
 *
 *		ErrCatch(inErr) {
 *			if (x > 1) {				// Use local variable in Catch 	
 *				SysBeep(1);
 *			}
 *		} ErrEndCatch
 *
 *****************************************************************************/

#ifndef __ERRORBASE_H__
#define __ERRORBASE_H__

// Include elementary types
#include <PalmTypes.h>						// Basic types
#include <CoreTraps.h>						// Trap Numbers.


#ifdef PALMOS
#if EMULATION_LEVEL != EMULATION_NONE
#include <setjmp.h>
#endif
#else
#include <setjmp.h>
#endif

// Max message length supported by ErrCustomAlert
#define errMaxMsgLength	511


/************************************************************
 * Error Classes for each manager
 *************************************************************/
#define	errNone						0x0000	// No error

#define	memErrorClass				0x0100	// Memory Manager
#define	dmErrorClass				0x0200	// Data Manager
#define	serErrorClass				0x0300	// Serial Manager
#define	slkErrorClass				0x0400	// Serial Link Manager
#define	sysErrorClass				0x0500	// System Manager
#define	fplErrorClass				0x0600	// Floating Point Library
#define	flpErrorClass				0x0680	// New Floating Point Library
#define	evtErrorClass				0x0700  	// System Event Manager
#define	sndErrorClass				0x0800  	// Sound Manager
#define	almErrorClass				0x0900  	// Alarm Manager
#define	timErrorClass				0x0A00  	// Time Manager
#define	penErrorClass				0x0B00  	// Pen Manager
#define	ftrErrorClass				0x0C00  	// Feature Manager
#define	cmpErrorClass				0x0D00  	// Connection Manager (HotSync)
#define	dlkErrorClass				0x0E00	// Desktop Link Manager
#define	padErrorClass				0x0F00	// PAD Manager
#define	grfErrorClass				0x1000	// Graffiti Manager
#define	mdmErrorClass				0x1100	// Modem Manager
#define	netErrorClass				0x1200	// Net Library
#define	htalErrorClass				0x1300	// HTAL Library
#define	inetErrorClass				0x1400	// INet Library
#define	exgErrorClass				0x1500	// Exg Manager
#define	fileErrorClass				0x1600	// File Stream Manager
#define	rfutErrorClass				0x1700	// RFUT Library
#define	txtErrorClass				0x1800	// Text Manager
#define	tsmErrorClass				0x1900	// Text Services Library
#define	webErrorClass				0x1A00	// Web Library
#define	secErrorClass				0x1B00	// Security Library
#define	emuErrorClass				0x1C00	// Emulator Control Manager
#define	flshErrorClass				0x1D00	// Flash Manager
#define	pwrErrorClass				0x1E00	// Power Manager
#define	cncErrorClass				0x1F00	// Connection Manager (Serial Communication)
#define	actvErrorClass				0x2000	// Activation application
#define	radioErrorClass				0x2100	// Radio Manager (Library)
#define	dispErrorClass				0x2200	// Display Driver Errors.
#define	bltErrorClass				0x2300	// Blitter Driver Errors.
#define	winErrorClass				0x2400	// Window manager.
#define	omErrorClass				0x2500	// Overlay Manager
#define	menuErrorClass				0x2600	// Menu Manager
#define	lz77ErrorClass				0x2700	// Lz77 Library
#define	smsErrorClass				0x2800	// Sms Library
#define	expErrorClass				0x2900	// Expansion Manager and Slot Driver Library
#define	vfsErrorClass				0x2A00	// Virtual Filesystem Manager and Filesystem library
#define	lmErrorClass				0x2B00	// Locale Manager
#define	intlErrorClass				0x2C00	// International Manager
#define pdiErrorClass				0x2D00	// PDI Library
#define	attnErrorClass				0x2E00	// Attention Manager
#define	telErrorClass				0x2F00	// Telephony Manager
#define hwrErrorClass				0x3000	// Hardware Manager (HAL)
#define	blthErrorClass				0x3100	// Bluetooth Library Error Class
#define	udaErrorClass				0x3200	// UDA Manager Error Class

#define cpmErrorClass               0x3800  // Crypto Manager
#define sslErrorClass               0x3900  // SSL (from RSA)
#define uilibErrorClass				0x3A00	// UI Library (Forms, Controls, etc)

#define pinsErrorClass				0x5000	// Pen Input Manager
#define statErrorClass				0x5100	// Status Bar Manager

#define	oemErrorClass				0x7000	// OEM/Licensee errors (0x7000-0x7EFF shared among ALL partners)
#define errInfoClass				0x7F00	// special class shows information w/o error code
#define	appErrorClass				0x8000	// Application-defined errors



/********************************************************************
 * Try / Catch / Throw support
 *
 * ---------------------------------------------------------------------
 * Exception Handler structure
 *  
 *	An ErrExceptionType object is created for each ErrTry & ErrCatch block.
 *	At any point in the program, there is a linked list of
 *	ErrExceptionType objects. GErrFirstException points to the
 *	most recently entered block. A ErrExceptionType blocks stores
 *	information about the state of the machine (register values)
 *	at the start of the Try block
 ********************************************************************/
	
#ifdef PALMOS
#if EMULATION_LEVEL != EMULATION_NONE
	#define	ErrJumpBuf	jmp_buf
#else
	typedef long* ErrJumpBuf[12];			// D3-D7,PC,A2-A7
#endif
#else
	#define	ErrJumpBuf	jmp_buf
#endif
	
// Structure used to store Try state.
typedef struct ErrExceptionType {
	struct ErrExceptionType*	nextP;	// next exception type
	ErrJumpBuf						state;	// setjmp/longjmp storage
	Int32								err;		// Error code
	} ErrExceptionType;
typedef ErrExceptionType *ErrExceptionPtr;


// Try & Catch macros
#define	ErrTry																\
	{																				\
		ErrExceptionType	_TryObject;										\
		_TryObject.err = 0;													\
		_TryObject.nextP = (ErrExceptionPtr)*ErrExceptionList();	\
		*ErrExceptionList() = (MemPtr)&_TryObject;					\
		if (ErrSetJump(_TryObject.state) == 0) {

		
// NOTE: All variables referenced in and after the ErrCatch must 
// be declared volatile.  Here's how for variables and pointers:
// volatile UInt16					oldMode;
//	ShlDBHdrTablePtr volatile hdrTabP = nil;
// If you have many local variables after the ErrCatch you may
// opt to put the ErrTry and ErrCatch in a separate enclosing function.
#define	ErrCatch(theErr)												\
			*ErrExceptionList() = (MemPtr)_TryObject.nextP;		\
			} 																	\
		else {																\
			Int32	theErr = _TryObject.err;							\
			*ErrExceptionList() = (MemPtr)_TryObject.nextP;
			
			
#define	ErrEndCatch														\
			}																	\
	}



/********************************************************************
 * Error Manager Routines
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifdef PALMOS
#if EMULATION_LEVEL != EMULATION_NONE
	#define	ErrSetJump(buf)			setjmp(buf)
	#define	ErrLongJump(buf,res) 	longjmp(buf,res)

#else
	Int16	ErrSetJump(ErrJumpBuf buf)
								SYS_TRAP(sysTrapErrSetJump);
								
	void	ErrLongJump(ErrJumpBuf buf, Int16 result)
								SYS_TRAP(sysTrapErrLongJump);
#endif
#else
	#define	ErrSetJump(buf)			setjmp(buf)
	#define	ErrLongJump(buf,res) 	longjmp(buf,res)
#endif
						
MemPtr*	ErrExceptionList(void)
							SYS_TRAP(sysTrapErrExceptionList);	
							
void  ErrThrow(Int32 err) 
							SYS_TRAP(sysTrapErrThrow);

void	ErrDisplayFileLineMsg(const Char * const filename, UInt16 lineNo, 
			const Char * const msg)
							SYS_TRAP(sysTrapErrDisplayFileLineMsg);

							
//---------------------------------------------------------------------
// 2/25/98 - New routine for PalmOS >3.0 to display a UI alert for 
// run-time errors. This is most likely to be used by network applications
// that are likely to encounter run-time errors like can't find the server,
//  network down, etc. etc. 
//
// This routine will lookup the text associated with 'errCode' and display
//  it in an alert. If errMsgP is not NULL, then that text will be used
//  instead of the associated 'errCode' text. If 'preMsgP' or 'postMsgP' 
//  is not null, then that text will be pre-pended or post-pended 
//  respectively.
//
// Apps that don't use the extra parameters may want to just use the
//  macro below 'ErrAlert'
//---------------------------------------------------------------------
UInt16	ErrAlertCustom(Err errCode, Char *errMsgP, Char *preMsgP, 
			Char *	postMsgP)
							SYS_TRAP(sysTrapErrAlertCustom);
							
#define	ErrAlert(err) ErrAlertCustom(err, 0, 0, 0)



#ifdef __cplusplus 
}
#endif




#endif // __ERRORBASE_H__
