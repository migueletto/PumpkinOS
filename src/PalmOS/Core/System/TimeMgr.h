/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: TimeMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Time manager functions
 *
 *****************************************************************************/

#ifndef __TIMEMGR_H__
#define __TIMEMGR_H__


// Include elementary types
#include <PalmTypes.h>
#include <CoreTraps.h>				// Trap Numbers.



/************************************************************
 * Time Manager result codes
 * (timErrorClass is defined in SystemMgr.h)
 *************************************************************/
#define timErrMemory			(timErrorClass | 1)



/************************************************************
 * Function Prototypes
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


//-------------------------------------------------------------------
// Initialization
//-------------------------------------------------------------------
Err			TimInit(void)	SYS_TRAP(sysTrapTimInit);


//-------------------------------------------------------------------
// API
//-------------------------------------------------------------------

// seconds since 1/1/1904
UInt32		TimGetSeconds(void)		
						SYS_TRAP(sysTrapTimGetSeconds);
					
// seconds since 1/1/1904
void  		TimSetSeconds(UInt32 seconds) 	
						SYS_TRAP(sysTrapTimSetSeconds);
					
// ticks since power on
UInt32 		TimGetTicks(void)					
						SYS_TRAP(sysTrapTimGetTicks);


#ifdef __cplusplus 
}
#endif



#endif // __TIMEMGR_H__
