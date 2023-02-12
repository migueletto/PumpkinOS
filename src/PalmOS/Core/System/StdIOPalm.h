/******************************************************************************
 *
 * Copyright (c) 1998-2002 PalmSource, Inc. All rights reserved.
 *
 * File: StdIOPalm.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This header file must be included by Palm standard IO apps so that they
 *	can use primitive standard IO functions like printf, getchar(), etc. 
 *
 *****************************************************************************/

/*
 *	  A PalmOS standard IO app is built like a normal PalmOS app but has
 *	a database type of 'sdio' instead of 'appl'. In addition, it must
 *	be named "Cmd-<cmdname>" where <cmdname> is the name of the command that
 *	users will enter to execute the app. For example, the 'ping' command
 *  would be placed in a database named "Cmd-ping". 
 *
 *	  In order to run a PalmOS standard IO app, a regular PalmOS app
 *	must be running first that provides a window for the output. This type
 *	of app is called a StdIO Provider. The StdIO Provider launches the
 *	stdio app when the user types in a command line and "hits" enter. 
 *  The provider app passes a structure pointer that contains the callbacks
 *	necessary for performing IO to the stdio app through the cmdPBP parameter
 *	of PilotMain. 
 *
 *		In addition to including this header, standard IO apps must link with
 *  the module :Libraries:Palm OS Glue:PalmOSGlue.lib. This module provides a
 *	PilotMain that extracts the command line arguments from the cmd and
 *	cmdPBP parameters and the glue code necessary for jumping through the 
 *  appropriate callbacks provided by the StdIO provider.
 *
 *
 *	  A minimal PalmOS stdio app's source file might look like this:
 *-------------------------------------------------------------------------

	#include	<StdIOPalm.h>

	// When compiling for the device, the entry point must be called SioMain(). 
	#if EMULATION_LEVEL == EMULATION_NONE
		#define	MyHello	SioMain
	#endif

	Int16	MyHello(UInt16 argc, char* argv[])
	{
		printf("\nHello World");
	}

 *-------------------------------------------------------------------------
 *
 *  Notice that the bulk of the code is in a routine named MyHello() this is
 *	so the routine can easily be called and tested using the Simulator.
 *
 *	When compiling for the viewer though, MyHello is renamed to SioMain(). 
 *  This is the assumed entry point used by the StdIOPalm.c glue code when it
 *	is compiled for the viewer. 
 *    
 **********************************************************************/
#ifndef		__STDIOPALM_H__
#define		__STDIOPALM_H__

// Define this so we don't get the standard <stdio.h> stuff
#define		_STDIO_H

// Get PalmOS includes
#include <PalmTypes.h>
#include <StringMgr.h>	// for _Palm_va_list


// All PalmOS Standard IO apps have the following database type
#define		sioDBType	sysFileTStdIO


// Stub out the "FILE" type until we support real file IO
typedef	void	FILE;



/*******************************************************************
 * C Standard IO macros and functions
 ********************************************************************/


// We don't support re-direction yet, stdin is always the keyboard and
//  stdout is always the stdio window. 
#define	stdin		((FILE*)0)
#define	stdout		((FILE*)1)
#define	stderr		((FILE*)2)

#ifndef	EOF
#define EOF			-1
#endif


//------------------------------------------------------------------------
// Functions provided in the StdIOPalm.c module which must be linked in with
//  the application source. 
//------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Your "main" entry point must have these calling conventions and
//  must be named "SioMain". 
Int16		SioMain(UInt16 argc, const Char * argv[]);
typedef 	Int16 (*SioMainProcPtr)(UInt16 argc, const Char * argv[]);


// This routine can be used to add your command to a simulator app
//  that is a StdIO provider. It registers the command name and it's
//  procedure pointer as a built-in command. THis is necessary in order
//  to test under the Simulator because it doesn't support executing
//  other databases.  
void 		SioAddCommand(const Char * cmdStr, SioMainProcPtr cmdProcP);


// File IO routines
Int16		Siofgetc (FILE*	fs);
Char *		Siofgets (Char * strP, UInt16 maxChars, FILE* fs);
Int16 		Siofputc (Int16 c, FILE* fs);
Int16 		Siofputs (const Char * strP, FILE* fs);
Int16		Siofprintf(FILE* fs, const Char * formatP, ...);
Int16		Siovfprintf(FILE* fs, const Char * formatP, _Palm_va_list args);


// stdin/stdout IO routines
Int16		Sioprintf(const Char * formatP, ...);
Int16		Sioputs(const Char * strP);
Char *		Siogets(Char * strP);


// Execute a command line of text
Int16		Siosystem(const Char * cmdStrP);

#ifdef __cplusplus 
}
#endif



//------------------------------------------------------------------------
// Macros.
// If you don't want to use the C stdio names, define STDIO_PALM_NATIVE_NAMES_ONLY
//  before including this header. You might want to do this for apps that 
//  run in the simulator that want to use the Desktops stdio library as well
//  as the Palm stdio library from the same module. 
//------------------------------------------------------------------------
#ifndef		STDIO_PALM_NATIVE_NAMES_ONLY
#define		fgetc(fs)					Siofgetc(fs)
#define		fgets(strP,maxChars,fs)		Siofgets(strP,maxChars,fs)
#define		fputc(c,fs)					Siofputc(c,fs)
#define		fputs(strP,fs)				Siofputs(strP,fs)
#define		fprintf						Siofprintf
#define		vfprintf(fs,formatP,args)	Siovfprintf(fs,formatP,args)

#define		printf						Sioprintf
#define		puts(strP)					Sioputs(strP)
#define		gets(strP)					Siogets(strP)

#define		getchar()					Siofgetc(stdin)
#define		putc(c,fs)					Siofputc(c,fs)
#define		putchar(c)					Siofputc(c, stdout)

#define		sprintf						StrPrintF
#define		vsprintf(x,y,z)				StrVPrintF(x,(const Char *)y,z)

#define		system(strP)				Siosystem(strP)

#endif		//STDIO_PALM_NATIVE_NAMES_ONLY

#endif	//__STDIOPALM_H__


/* This is outside the "#ifndef __STDIOPALM_H__" so that StdIOProvider.h
   can pick it up even if StdIOPalm.h has already been included.  */
#if defined(_STDIO_PALM_C_) && !defined(__STDIOPALM_H__SIOGLOBALS__)
#define __STDIOPALM_H__SIOGLOBALS__
/**********************************************************
 * Structure of stdio parameters that are pointed to by the
 * application global GAppSioGlobalsP. In stdio apps, this application
 * global gets initialized from the cmdPBP parameter to PilotMain. 
 *
 * The stdio provider that launches the stdio app must create this
 * structure and initialize it.  
 *
 * NOTE: This structure should be considered for PRIVATE use
 *  by the StdIOPalm.c module only. Your source code should not
 *  reference it
 ********************************************************************/

typedef struct SioGlobalsType {
	// size of this structure, used for version matching/checking
	// and flags.
	UInt16			size;
	UInt32			flags;
	
	// argc, argv parameters to this command
	UInt16			argc;
	const Char **			argv;
	
	
	// Basic standard IO procedure pointers which are filled in
	//  by the standard IO provider application. These are the 
	//  callbacks available in the first rev. 
	Int16			(*fgetcProcP) (void* sioGP, FILE* fs);
	Char *			(*fgetsProcP) (void* sioGP, Char * strP, UInt16 n, FILE* fs);
	
	Int16			(*fputcProcP) (void* sioGP, Int16 c, FILE* fs);
	Int16			(*fputsProcP) (void* sioGP, const Char * strP, FILE* fs);
	
	Int16			(*vfprintfProcP) (void* sioGP, FILE* fs, 
							const Char * formatP, _Palm_va_list args);
	
	Int16			(*systemProcP) (void* sioGP, const Char * strP);


	// Standard IO provider private data follows this structure
	// UInt8				private[];
	} SioGlobalsType, *SioGlobalsPtr;
	

// This global is declared in StdIOPalm.c which must be linked in with any
//  stdio app. 
extern	SioGlobalsPtr	GAppSioGlobalsP;
#endif
