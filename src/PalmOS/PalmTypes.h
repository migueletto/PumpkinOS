#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmTypes.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Common header file for all Palm OS components.
 *		Contains elementary data types
 *
 *****************************************************************************/

#ifndef __PALMTYPES_H__
#define __PALMTYPES_H__
#endif


/************************************************************
 * Environment configuration
 *************************************************************/
// <BuildDefaults.h> must be included here, rather than in <PalmOS.h>
// because they must be included for ALL builds.
// Not every build includes <PalmOS.h>.

// To override build options in a local component, include <BuildDefines.h>
// first, then define switches as need, and THEN include <PalmTypes.h>.
// This new mechanism supercedes the old "AppBuildRules.h" approach.
// More details available in <BuildDefaults.h>.
#include <BuildDefaults.h>


/************************************************************
 * Useful Macros 
 *************************************************************/
#if defined(__GNUC__) && defined(__UNIX__)		// used to be in <BuildRules.h>
	// Ensure that structure elements are 16-bit aligned
	// Other [host] development platforms may need this as well...
	//#pragma pack(2)
#endif


/********************************************************************
 * Elementary data types
 ********************************************************************/
// Determine if we need to define our basic types or not
#ifdef PALMOS
#ifndef  __TYPES__         // (Already defined in CW11)
#ifndef  __MACTYPES__        // (Already defined in CWPro3)
#define  __DEFINE_TYPES_ 1
#endif
#endif
#endif

// Fixed size data types
#ifdef PALMOS
typedef signed char    Int8;
typedef signed short   Int16;
typedef signed long    Int32;

#if __DEFINE_TYPES_
typedef unsigned char  UInt8;
typedef unsigned short  UInt16;
typedef unsigned long   UInt32;
#endif
#else
typedef int8_t		Int8;
typedef int16_t		Int16;	
typedef int32_t		Int32;

typedef uint8_t  	UInt8;
typedef uint16_t  UInt16;
typedef uint32_t  UInt32;
#endif


// Logical data types
typedef uint8_t	  Boolean;

typedef char		  Char;
typedef UInt16		WChar;		// 'wide' int'l character type.

typedef UInt16		Err;

//typedef UInt32		LocalID;		// local (card relative) chunk ID
//typedef void *		LocalID;		// local (card relative) chunk ID
typedef local_id_t		LocalID;		// local (card relative) chunk ID

typedef Int16 		Coord;		// screen/window coordinate


typedef void *		MemPtr;		// global pointer
//typedef void *    	MemHandle;	// global handle
//typedef UInt32    	MemHandle;	// global handle
typedef mem_handle_t    MemHandle;	// global handle


typedef Int32 (*ProcPtr)();


/************************************************************
 * Useful Macros 
 *************************************************************/

// The min() and max() macros which used to be defined here have been removed
// because they conflicted with facilities in C++.  If you need them, you
// should define them yourself, or see PalmUtils.h -- but please read the
// comments in that file before using it in your own projects.


//#define OffsetOf(type, member)	((UInt32) &(((type *) 0)->member))
#define OffsetOf(type, member)	offsetof(type, member)




/************************************************************
 * Common constants
 *************************************************************/
#ifndef NULL
#define NULL	0
#endif	// NULL

#ifndef bitsInByte
#define bitsInByte	8
#endif	// bitsInByte


// Include the following typedefs if types.h wasn't read.
#ifndef true
#define true			1
#endif
#ifndef false
#define false			0
#endif


/************************************************************
 * Misc
 *************************************************************/

// Standardized infinite loop notation
// Use in place of while(1), while(true), while(!0), ...
#define loop_forever	for (;;)


/************************************************************
 * Define whether or not we are direct linking, or going through
 *  traps.
 *
 * When eumulating we use directy linking.
 * When running under native mode, we use traps EXCEPT for the
 *   modules that actually install the routines into the trap table. 
 *   These modules will set the DIRECT_LINK define to 1
 *************************************************************/

#define	USE_TRAPS 0						// direct link (Simulator)


/********************************************************************
 * Palm OS System and Library trap macro definitions:
 ********************************************************************/

#define _DIRECT_CALL(table, vector)
#define _DIRECT_CALL_WITH_SELECTOR(table, vector, selector)
#define _DIRECT_CALL_WITH_16BIT_SELECTOR(table, vector, selector)

#ifndef _STRUCTURE_PICTURES

#define _SYSTEM_TABLE	15
#define _HAL_TABLE		15

#define _OS_CALL(table, vector)  __attribute__ ((systrap (vector)))
#define _OS_CALL_WITH_SELECTOR(table, vector, selector)
#define _OS_CALL_WITH_16BIT_SELECTOR(table, vector, selector)

#define _HAL_API(kind)		_OS##kind
#define _SYSTEM_API(kind)	_DIRECT##kind


/************************************************************
 * Palm specific TRAP instruction numbers
 *************************************************************/
#define sysDbgBreakpointTrapNum		0		// For soft breakpoints		
#define sysDbgTrapNum					8		// For compiled breakpoints			
#define sysDispatchTrapNum				15		// Trap dispatcher


#define SYS_TRAP(trapNum)
	
#define ASM_SYS_TRAP(trapNum)


#endif //__PALMTYPES_H__
