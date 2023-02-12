/******************************************************************************
 *
 * Copyright (c) 2001-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PceNativeCall.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *      Headers for native code support
 *
 *****************************************************************************/

#ifndef __PCENATIVECALL_H__
#define __PCENATIVECALL_H__


#define kPceNativeWantA0			(0x10000000)
#define kPceNativeTrapNoMask		(0x00000FFF)

#define PceNativeTrapNo(sysTrapNo)	(sysTrapNo & kPceNativeTrapNoMask)


/************************************************************
 *
 * FUNCTION:		Call68KFuncType
 *
 * DESCRIPTION:		The prototype for callback entry point to
 *					emulated routines or OS traps, passed to
 *					NativeFuncType functions.
 *
 * PARAMETERS:		emulStateP -> private to emulator, must be passed
 *					trapOrFunction -> if <= kPceNativeTrapNoMask, treat
 *							this as a trap number.  Otherwise, treat
 *							this as a pointer to a 68K function to call.
 *					argsOnStackP -> pointer to memory to be copied to the
 *							68K stack prior to calling the function.
 *							Normally contains the arguments for the 68K
 *							code.
 *					argsSizeAndwantA0 -> the number of bytes from
 *							argsOnStackP to actually copy to the 68K
 *							emulator stack.  If the function or trap
 *							returns its result in 68K register A0
 *							(when result is a pointer type), OR the
 *							size with kPceNativeWantA0.
 *
 * RESULT:			returns value from 68K call, either A0 or D0 register.
 *
 *************************************************************/
typedef unsigned long Call68KFuncType(const void *emulStateP, unsigned long trapOrFunction, const void *argsOnStackP, unsigned long argsSizeAndwantA0);


/************************************************************
 *
 * FUNCTION:		NativeFuncType
 *
 * DESCRIPTION:		The prototype for native functions called
 *					from the 68K emulator through PceNativeCall
 *
 * PARAMETERS:		emulStateP -> private to emulator
 *					userData -> pointer passed through from PceNativeCall
 *					call68KFuncP -> function to call back into 68K emulated
 *									routines or OS traps.
 *
 * RESULT:			returns value will be passed back through PceNativeCall.
 *
 *************************************************************/
typedef unsigned long NativeFuncType(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP);


// only define the trap if we've already got the macros that allow defining traps
// usually result: skip this part when #including this in native source

#ifdef __CORETRAPS_H_

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************
 *
 * FUNCTION:		PceNativeCall
 *
 * DESCRIPTION:		In the ARM emulator, call the passed ARM instructions
 *					and send through the user data pointer and
 *					return the result to the 68K caller.
 *
 * PARAMETERS:		nativeFuncP -> pointer to native function to call
 *					userDataP -> pointer to memory to pass through to function
 *
 * RESULT:			returns the return value from the native function.
 *
 * NOTE:			This trap will be unimplemented on devices that
 *					are not running a 68K emulator, check the device
 *					type before calling this function.
 *
 *************************************************************/

UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
		SYS_TRAP(sysTrapPceNativeCall);

#ifdef __cplusplus 
}
#endif

#endif	// __CORETRAPS_H_

#endif	// __PCENATIVECALL_H__
