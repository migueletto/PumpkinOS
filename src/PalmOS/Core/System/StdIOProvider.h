/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: StdIOProvider.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This header file must be included by apps that want to provide
 *	a standard IO window and "execute" standard IO apps in it. See the
 *	comments in the file "StdIOProvier.c" for more info
 *
 *****************************************************************************/

#ifndef		__STDIOPROVIDER_H__
#define		__STDIOPROVIDER_H__

#define		_STDIO_PALM_C_

#include <PalmTypes.h>
#include <StdIOPalm.h>
#include <SysEvent.h>


/****************************************************************
 * Provider SioGlobalsType includes the client visible fields
 * in the beginning
 ****************************************************************/
typedef struct {
	SioGlobalsType	client;
	
	UInt32			provA5;			// saved A5 register

	MemHandle 		textH;  		// holds latest text
	UInt16		 	formID;			// Form ID that contains text field
	UInt16			fieldID;		// Field ID 
	UInt16		 	scrollerID;
	Boolean			echo;

	UInt8			reserved;
	} SioProvGlobalsType, *SioProvGlobalsPtr;



/*******************************************************************
 * Function Prototypes
 ********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Err			SioInit(UInt16 formID, UInt16 fieldID, UInt16 scrollerID);
Err			SioFree(void);
Boolean 	SioHandleEvent (SysEventType * eventP);

// This routine will execute a command line. It is faster than
//  using the "system()" call but can only be used by the
//  StdIO provider app itself. 
Int16 		SioExecCommand(const Char * cmd);

void SioClearScreen(void);

#ifdef __cplusplus 
}
#endif

#endif //_STDIOPROVIDER_H_
