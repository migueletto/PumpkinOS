/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: BuildDefaults.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Build variable defaults for Palm OS.
 *
 *    This file is included by <PalmTypes.h>.
 *    This file supercedes the old <BuildRules.h> file.
 *
 *****************************************************************************/

#ifndef 	__BUILDDEFAULTS_H__
#define	__BUILDDEFAULTS_H__

#include <BuildDefines.h>

// To override build options in a local component, include <BuildDefines.h>
// first, then define switches as need, and then include <PalmTypes.h>.

// Some projects used to have a local copy of a file called "AppBuildRules.h"
// or "AppBuildRulesMSC.h", which was automatically included by <BuildRules.h>
// to override certain system default compile-time switches.  These local
// "prefix" files can still be used.  The project source files should be changed
// to include <BuildDefines.h>, then "AppBuildRules.MSC.h", then <PalmTypes.h>
// instead of the previous <Pilot.h>


/************************************************************
 * Include the following when running under the CodeWarrior
 *  IDE so that default build options can be overriden. The default
 *  version of this file is in the Incs: directory. If an app wishes
 *  to override the default options, it should create a copy of this
 *  include file in its own local directory. This include file
 *  is never used when building from MPW since MPW can specify compiler
 *  defines from the command line.
 *
 * Other environments can override all of these settings by simply
 *  pre-defining CMD_LINE_BUILD and any other desired settings.
 *************************************************************/

#ifdef PALMOS
#ifndef CMD_LINE_BUILD				// typically pre-defined only from MPW

	#if !defined(__MWERKS__) && !defined(__PALMOS_TRAPS__) && !defined(_MSC_VER)

		// assume other environments generally build only PalmOS executables
		#define __PALMOS_TRAPS__ 	1

	#endif


	#if	__PALMOS_TRAPS__			// defined by CodeWarrior IDE or above

		// Settings to build a PalmOS executable
		#ifndef EMULATION_LEVEL
			#define EMULATION_LEVEL		EMULATION_NONE		// building Palm OS executable
		#endif

		#ifndef USE_TRAPS
			#define USE_TRAPS				1						// use Palm OS traps
		#endif

	#endif


#endif
#endif



/************************************************************
 * Settings that can be overriden in the makefile (for MPW)
 *	OR (for CodeWarrior) in "AppBuildRules.h".  If there is no
 * local copy of "AppBuildRules.h" within the project directory,
 * the one in the Incs directory will be used instead.
 *************************************************************/
// This default option is probably ok for now
#ifdef PALMOS
#ifndef	EMULATION_LEVEL
	#define	EMULATION_LEVEL		EMULATION_MAC
#endif

// This default option is probably ok for now
#if EMULATION_LEVEL == EMULATION_NONE
	#define	MEMORY_TYPE				MEMORY_LOCAL
#endif

// This default option is probably ok for now
#ifndef MEMORY_TYPE
	#define	MEMORY_TYPE 			MEMORY_LOCAL
#endif
#else
	#define	MEMORY_TYPE 			MEMORY_LOCAL
#endif

// This default option is probably ok for now
#ifndef ENVIRONMENT
	#define	ENVIRONMENT				ENVIRONMENT_CW
#endif

// This default option is probably ok to leave as a default
#ifndef	PLATFORM_TYPE
	#define	PLATFORM_TYPE			PLATFORM_VIEWER
#endif

#ifndef	ERROR_CHECK_LEVEL
	#define	ERROR_CHECK_LEVEL		#error "ERROR_CHECK_LEVEL not defined; Try including ''PalmOptErrorCheckLevel.h''!"
	// The following allows <PalmOptErrorCheckLevel.h> to redefine ERROR_CHECK_LEVEL
	// without letting it change ERROR_CHECK_LEVEL if it was already defined,
	// such as would be the case when using a project prefix file...
	#define	ERROR_CHECK_LEVEL_OK_TO_REDEFINE
#endif

// This default option is probably ok for now
#ifdef PALMOS
#ifndef CPU_TYPE
   #if defined(__INTEL__) || defined(__i386__)
      #define  CPU_TYPE          CPU_x86
   #elif defined(__POWERPC__) || defined(__powerpc__)
      #define  CPU_TYPE          CPU_PPC
   #elif defined(__arm__)
      #define  CPU_TYPE          CPU_ARM
   #else
	   #define	CPU_TYPE				CPU_68K
   #endif
#endif
#else
  #define  CPU_TYPE          CPU_x86
#endif
// add additional checking to the build


#ifndef MODEL
	#define	MODEL						#error "MODEL not defined; Try including ''PalmOptModel.h''!"
#endif

// This default option is probably ok to leave as a default
#ifndef	MEMORY_FORCE_LOCK
	#define	MEMORY_FORCE_LOCK		MEMORY_FORCE_LOCK_ON
#endif

// Must be defined (-d or prefix file) before using.  See comment in <BuildDefines.h>.
#ifndef	DEBUG_LEVEL
	#define	DEBUG_LEVEL				#error "DEBUG_LEVEL must be defined before using!"
#endif

#ifndef 	DEFAULT_LIB_ENTRIES
	#define		DEFAULT_LIB_ENTRIES	12			// space for 12 libraries in library table
#endif

#ifndef	USER_MODE
	#define	USER_MODE				#error "USER_MODE not defined; Try including ''PalmOptUserMode.h''!"
#endif

#ifndef	INTERNAL_COMMANDS
	#define	INTERNAL_COMMANDS		#error "INTERNAL_COMMANDS must be defined before using!"
#endif

#ifndef	INCLUDE_DES
	#define	INCLUDE_DES				INCLUDE_DES_ON
#endif

// Unless otherwise specified, assume CML_ENCODER is off
#ifndef	CML_ENCODER
	#define 	CML_ENCODER				CML_ENCODER_OFF
#endif

// Derive the path for resource files.
#ifndef RESOURCE_FILE_PREFIX
	#define	RESOURCE_FILE_PREFIX	""
#endif

// LOCALE_SUFFIX is used for the name of merged resource files,
// and also sys.tres.
#ifndef LOCALE_SUFFIX
	#define	LOCALE_SUFFIX			""
#endif



// Set these according to which Shell commands you want to
// link with.  These are often overridden by other components
// in a local "AppBuildRules.h" which is currently obsolete.
#ifndef SHELL_COMMAND_DB
	#define	SHELL_COMMAND_DB			0	// Not Available
#endif

#ifndef SHELL_COMMAND_UI
	#define	SHELL_COMMAND_UI			1	// Available
#endif

#ifndef SHELL_COMMAND_APP
	#define	SHELL_COMMAND_APP			1	// Available
#endif

#ifndef SHELL_COMMAND_EMULATOR
	#define	SHELL_COMMAND_EMULATOR	1	// Available
#endif


#ifndef TRACE_OUTPUT
	#define TRACE_OUTPUT_OK_TO_REDEFINE
	#define	TRACE_OUTPUT	TRACE_OUTPUT_OFF
#endif

#ifndef SCREEN_DENSITY
	#define SCREEN_DENSITY_OK_TO_REDEFINE
	#define SCREEN_DENSITY	SCREEN_DENSITY_STANDARD
#endif


// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	DYN_MEM_SIZE_MAX
	#define	DYN_MEM_SIZE_MAX		#error "DYN_MEM_SIZE_MAX is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	SMALL_ROM_SIZE
	#define	SMALL_ROM_SIZE			#error "SMALL_ROM_SIZE is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	CONSOLE_SERIAL_LIB
	#define	CONSOLE_SERIAL_LIB	#error "CONSOLE_SERIAL_LIB is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	PILOT_SERIAL_MGR
	// Palm OS 3.5 code now assumes PILOT_SERIAL_MGR == PILOT_SERIAL_MGR_NEW
	#define	PILOT_SERIAL_MGR		#error "PILOT_SERIAL_MGR is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	MEMORY_VERSION
	// Palm OS 3.5 code now assumes MEMORY_VERSION == MEMORY_VERSION_2
	#define	MEMORY_VERSION			#error "MEMORY_VERSION is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	GRAPHICS_VERSION
	// Palm OS 3.5 code now assumes GRAPHICS_VERSION == GRAPHICS_VERSION_2
	#define	GRAPHICS_VERSION		#error "GRAPHICS_VERSION is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	HW_TARGET
	// Palm OS 3.5 is now hardware independent; HAL should handle this
	#define	HW_TARGET				#error "HW_TARGET is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef 	HW_REV
	// Palm OS 3.5 is now hardware independent; HAL should handle this
	#define	HW_REV					#error "HW_REV is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef	RMP_LIB_INCLUDE
	#define	RMP_LIB_INCLUDE		#error "RMP_LIB_INCLUDE is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
#ifndef	OEM_PRODUCT
	#define	OEM_PRODUCT				#error "OEM_PRODUCT is an obsolete build flag!"
#endif

// Obsolete option; this is left here to catch any stragglers (test code, etc.)...
// LANGUAGE/COUNTRY are no longer supported. If absolutely necessary, use LOCALE instead, but
// build-time dependencies on the target LOCALE are a _really_ bad idea.
#ifdef 	LANGUAGE
	#undef LANGUAGE
#endif
#define	LANGUAGE					#error "LANGUAGE is now obsolete; use LOCALE and include PalmOptLocale.h"

#ifdef	COUNTRY
	#undef COUNTRY
#endif
#define	COUNTRY					#error "COUNTRY is now obsolete; use LOCALE and include PalmOptLocale.h"

// Moved to <PalmTypes.h>:
//#if defined(__GNUC__) && defined(__UNIX__)
//	// Ensure that structure elements are 16-bit aligned
//	#pragma pack(2)
//#endif

// Default to allow access to internal data structures exposed in system/ui header files.
// If you want to verify that your app does not access data structure internals then define
// DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS before including this file.


// Starting with the Palm OS SDK 4.0 update 1, the default behavior disallow
// direct access to internal structures.

//#define DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS

#ifndef DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS

#define ALLOW_ACCESS_TO_INTERNALS_OF_CLIPBOARDS
#define ALLOW_ACCESS_TO_INTERNALS_OF_CONTROLS
#define ALLOW_ACCESS_TO_INTERNALS_OF_FIELDS
#define ALLOW_ACCESS_TO_INTERNALS_OF_FINDPARAMS
#define ALLOW_ACCESS_TO_INTERNALS_OF_FORMS
#define ALLOW_ACCESS_TO_INTERNALS_OF_LISTS
#define ALLOW_ACCESS_TO_INTERNALS_OF_MENUS
#define ALLOW_ACCESS_TO_INTERNALS_OF_PROGRESS
#define ALLOW_ACCESS_TO_INTERNALS_OF_SCROLLBARS
#define ALLOW_ACCESS_TO_INTERNALS_OF_TABLES

//#define ALLOW_ACCESS_TO_INTERNALS_OF_BITMAPS
#define ALLOW_ACCESS_TO_INTERNALS_OF_FONTS
#define ALLOW_ACCESS_TO_INTERNALS_OF_WINDOWS

#endif

// This is defined to allow support of deprecated API function names.  The API header files
// map old function names to new function names if ALLOW_OLD_API_NAMES is defined.
#define ALLOW_OLD_API_NAMES		1

#endif
