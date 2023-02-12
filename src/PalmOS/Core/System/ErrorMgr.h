/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: ErrorMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Include file for Error Management that depend on ERROR_CHECK_LEVEL
 *    All the rest of the old ErrorMgr.h is in ErrorBase.h
 *
 *****************************************************************************/

#ifndef __ERRORMGR_H__
#define __ERRORMGR_H__

#include <ErrorBase.h>						// Invariant error management stuff

#include <PalmOptErrorCheckLevel.h>	// #define ERROR_CHECK_LEVEL

//------------------------------------------------------------
// Full Error Checking
//------------------------------------------------------------
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL

#define ErrFatalDisplay(msg) \
	ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg)

#define ErrFatalDisplayIf(condition, msg) \
	do {if (condition) ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg);} while (0)
		
#define ErrNonFatalDisplayIf(condition, msg) \
	do {if (condition) ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg);} while (0)

#define ErrNonFatalDisplay(msg) \
	ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg)
	
#define ErrDisplay(msg) \
	ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg)




//------------------------------------------------------------
// Fatal  Error Checking Only
//------------------------------------------------------------
#elif ERROR_CHECK_LEVEL == ERROR_CHECK_PARTIAL

#define ErrFatalDisplay(msg) \
	ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg)

#define ErrFatalDisplayIf(condition, msg) \
	do {if (condition) ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg);} while (0)

#define ErrNonFatalDisplayIf(condition, msg) 

#define ErrNonFatalDisplay(msg) 
	
#define ErrDisplay(msg) \
	ErrDisplayFileLineMsg(__FILE__, (UInt16) __LINE__, msg)



//------------------------------------------------------------
// No  Error Checking  
//------------------------------------------------------------
#elif ERROR_CHECK_LEVEL == ERROR_CHECK_NONE

#define ErrFatalDisplay(msg) 

#define ErrFatalDisplayIf(condition, msg) 

#define ErrNonFatalDisplayIf(condition, msg) 

#define ErrNonFatalDisplay(msg) 

#define ErrDisplay(msg)  


//------------------------------------------------------------
// Not Defined...
//------------------------------------------------------------
#else
#error	ERROR: the compiler define 'ERROR_CHECK_LEVEL' must be defined!

#endif // ERROR_CHECK_LEVEL


#endif // __ERRORMGR_H__
