/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: SysUtils.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  These are miscellaneous routines.
 *
 *****************************************************************************/

#ifndef __SYSUTILS_H__
#define __SYSUTILS_H__

// Include elementary types
#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>				// Trap Numbers.
#include <HostControl.h>



//typedef Int16 _comparF (const void *, const void *, Int16 other);
typedef Int16 _comparF (void *, void *, Int32 other);
typedef _comparF * CmpFuncPtr;

typedef Int16 _searchF (void const *searchData, void const *arrayData, Int32 other);
typedef _searchF * SearchFuncPtr;


// For backwards compatibility
#define GremlinIsOn hostSelectorGremlinIsRunning

/************************************************************
 * Constants
 *************************************************************/
#define	sysRandomMax		0x7FFF			// Max value returned from SysRandom()


/************************************************************
 * Macros
 *************************************************************/
#define Abs(a) (((a) >= 0) ? (a) : -(a))

/************************************************************
 * procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

Boolean	SysBinarySearch (void const *baseP, UInt16 numOfElements, Int16 width, 
				SearchFuncPtr searchF, void const *searchData, 
				Int32 other, Int32 *position, Boolean findFirst)
						SYS_TRAP(sysTrapSysBinarySearch);

void 		SysInsertionSort (void *baseP, UInt16 numOfElements, Int16 width, 
				CmpFuncPtr comparF, Int32 other)
						SYS_TRAP(sysTrapSysInsertionSort);

void 		SysQSort (void *baseP, UInt16 numOfElements, Int16 width, 
				CmpFuncPtr comparF, Int32 other)
						SYS_TRAP(sysTrapSysQSort);

void		SysCopyStringResource (Char *string, Int16 theID)
						SYS_TRAP(sysTrapSysCopyStringResource);

MemHandle SysFormPointerArrayToStrings(Char *c, Int16 stringCount)
						SYS_TRAP(sysTrapSysFormPointerArrayToStrings);
						
						
// Return a random number ranging from 0 to sysRandomMax.
// Normally, 0 is passed unless you want to start with a new seed.
Int16		SysRandom(Int32 newSeed)
						SYS_TRAP(sysTrapSysRandom);


Char *	SysStringByIndex(UInt16 resID, UInt16 index, Char *strP, UInt16 maxLen)
						SYS_TRAP(sysTrapSysStringByIndex);

Char *	SysErrString(Err err, Char *strP, UInt16 maxLen)
						SYS_TRAP(sysTrapSysErrString);

// This function is not to be called directly.  Instead, use the various Emu* calls
// in EmuTraps.h because they work for Poser, the device, and the simulator, and 
// they are safer because of the type checking.
UInt32		HostControl(HostControlTrapNumber selector, ...)
						SYS_TRAP(sysTrapHostControl);

#ifdef PALMOS
void PumpkinDebug(UInt16 level, Char *sys, Char *buf)
						SYS_TRAP(sysTrapPumpkinDebug);

void PumpkinDebugBytes(UInt16 level, Char *sys, void *buf, UInt32 len)
						SYS_TRAP(sysTrapPumpkinDebugBytes);
#endif


// For backwards compatibility
#define SysGremlins HostControl

#ifdef __cplusplus
}
#endif

#endif // __SYSUTILS_H__
