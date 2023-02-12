/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: BuildDefines.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Build variable defines for Palm OS.
 *    
 *    This file is included by <BuildDefaults.h>.
 *		It should be included by any local component that wishes
 *		to override any system default compile-time switches.
 *		For more details, refer to <BuildDefaults.h>.
 *
 *    This file supercedes the old <BuildRules.h> file.
 *
 *****************************************************************************/

#ifndef __BUILDDEFINES_H__
#define __BUILDDEFINES_H__


#define PALMOS_SDK_VERSION		0x0510


/************************************************************
 * Compilation Control Options
 *************************************************************/

// The makefile should set the define EMULATION_LEVEL to one of the following
//  constants:
#define	EMULATION_NONE			0		// native environment
#define	EMULATION_WINDOWS		1		// emulate on Windows
#define	EMULATION_DOS			2		// emulate on DOS
#define	EMULATION_MAC			3		// emulate on Macintosh
#define	EMULATION_UNIX			4		// emulate on Linux


// If emulation is not EMULATION_NONE, the following define controls
// whether we are talking to a touchdown device over the serial port,
// or if we are emulating a memory card in local RAM.
//
// The makefile should set the define MEMORY_TYPE to one of the following
//  constants
#define	MEMORY_LOCAL				0		// Emulate using local memory card image
#define	MEMORY_REMOTE				1		// Pass calls through serial port to the device


// The makefile should set the define ENVIRONMENT to one of the following:
#define	ENVIRONMENT_CW				0		// CodeWarrior compiler/linker (IDE or MPW)
#define	ENVIRONMENT_MPW				1		// Apple's MPW compiler/linker


// The makefile should set the define PLATFORM_TYPE to one of the following
//  constants:
#define	PLATFORM_VIEWER				0		// PalmPilot Viewer
#define	PLATFORM_SERVER				1		// Server code


// The makefile should set the define ERROR_CHECK_LEVEL to one of the 
//  following constants:
#define	ERROR_CHECK_NONE			0		// compile no error checking code
#define	ERROR_CHECK_PARTIAL			1		// display for fatal errors only
#define	ERROR_CHECK_FULL			2		// display fatal or non-fatal errors


// The makefile should set the define CPU_TYPE to one of the 
//  following constants:
#define	CPU_68K  					0		// Motorola 68K type
#define	CPU_x86  					1		// Intel x86 type
#define	CPU_PPC  					2		// Motorola/IBM PowerPC type
#define	CPU_ARM  					3		// ARM type





// The makefile should set the define MODEL to one of the
// following constants. This equate is currently only use by
//  special purpose applications like Setup that need to install different
//  files for each type of product. Normally, model dependent behavior
//  should be run-time based off of Features using FtrGet(). 
#define	MODEL_GENERIC				0		// Not one of the specific models that follow
#define	MODEL_ELEVEN				1		// Eleven (Palm Seven)
#define	MODEL_SUMO					2		// EZ product 


// The makefile should set the define MEMORY_FORCE_LOCK to one of the
// following.
#define	MEMORY_FORCE_LOCK_OFF		0	// Don't force all handles to be locked
#define	MEMORY_FORCE_LOCK_ON		1	// Force all handles to be locked before usage


// The makefile should set the define DEBUG_LEVEL to one of the
// following. THIS DEFINE IS ONLY USED BY A COUPLE MODULES SO WE
//  DON'T GIVE IT A DEFAULT VALUE BELOW...
// ANY MODULE THAT USES THIS DEFINE SHOULD VERIFY THAT IT IS DEFINED!!
#define	DEBUG_LEVEL_NONE			1	// None: Does not auto-launch Console or Debugger
#define	DEBUG_LEVEL_PARTIAL			2	// Partial: Auto-Launches Console but skips debugger
#define	DEBUG_LEVEL_FULL			3	// Full: Waits in debugger on reset


// The makefile should set the define DEFAULT_DATA to one of the following:
// Setting this define to USE_DEFAULT_DATA will cause the core apps to include default
// data in the build.
#define DO_NOT_USE_DEFAULT_DATA		0
#define USE_DEFAULT_DATA			1


// The makefile should set the define USER_MODE to one of the 
// following constants:
#define	USER_MODE_NORMAL			0	// normal operation
#define	USER_MODE_DEMO				1	// demo mode - Graffiti and pop-up keyboard disabled


// The makefile should set the define INTERNAL_COMMANDS to one of the 
// following constants:
#define	INTERNAL_COMMANDS_EXCLUDE	0
#define	INTERNAL_COMMANDS_INCLUDE	1	// Include internal shell commands


// The makefile should set the define INCLUDE_DES to one of the 
// following constants:
#define	INCLUDE_DES_OFF				0
#define	INCLUDE_DES_ON				1	// include it


// Used by Net Library to link in the CML encoder
#define	CML_ENCODER_OFF				0
#define	CML_ENCODER_ON				1

// The makefile should set the define TRACE_OUTPUT to one of the following
//  constants:
#define	TRACE_OUTPUT_OFF			0
#define	TRACE_OUTPUT_ON				1

// SCREEN_DENSITY constants
#define	SCREEN_DENSITY_STANDARD		0
#define 	SCREEN_DENSITY_QVGA		1
#define	SCREEN_DENSITY_DOUBLE		2




#endif
