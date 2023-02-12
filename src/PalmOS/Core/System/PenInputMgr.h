/******************************************************************************
 *
 * Copyright (c) 2002-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PenInputMgr.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *  This header file describes the Pen Input Manager,
 *  part of Pen Input Services.
 *
 *****************************************************************************/

#ifndef _PENINPUTMGR_H_
#define _PENINPUTMGR_H_


#include <PalmTypes.h>
#include <ErrorBase.h>
#include <Window.h>


// Pen Input Manager Features
#define pinCreator					'pins'
#define pinFtrAPIVersion			1

// preliminary 1.0 release from Garmin
#define pinAPIVersion1_0			0x01000000
// PINS API version number (1.1 for OS 5.3)
#define pinAPIVersion1_1			0x01103000
// PINS API version number (2.0 for OS 6.0)
#define pinAPIVersion2_0			0x02003000


// Pen Input Manager errors
#define pinErrNoSoftInputArea		(pinsErrorClass | 0x00)
#define pinErrInvalidParam			(pinsErrorClass | 0x01)

// control bar errors
#define statErrInvalidLocation		(statErrorClass | 0x01)
#define statErrInvalidName			(statErrorClass | 0x02)
#define statErrInputWindowOpen		(statErrorClass | 0x03)


// Input area states
typedef enum
{
	pinInputAreaOpen,
	pinInputAreaClosed,
	pinInputAreaNone,				// do not use
	reserved1,						// do not use
	reserved2,						// do not use
	pinInputAreaUser,				// restore the last user selection of input area state

	pinInputAreaReset = 0xFFFF		// for internal use only
}
PINInputAreaStateType;


// Input trigger states
typedef enum 
{
	pinInputTriggerEnabled,
	pinInputTriggerDisabled,
	pinInputTriggerNone				// do not use
}
PINInputTriggerStateType;


// selectors for StatGetAttribute
#define	statAttrBarVisible	0
#define statAttrDimension	1


// Selectors for the PIN trap dispatcher and PINS_TRAP area defined in Window.h
	

#ifdef __cplusplus
extern "C" {
#endif

// Input area API
UInt16	PINGetInputAreaState(void)
				PINS_TRAP(pinPINGetInputAreaState);
				
Err		PINSetInputAreaState(UInt16 state)
				PINS_TRAP(pinPINSetInputAreaState);
				
UInt16	PINGetInputTriggerState(void)
				PINS_TRAP(pinPINGetInputTriggerState);
								
Err		PINSetInputTriggerState(UInt16 state)
				PINS_TRAP(pinPINSetInputTriggerState);


// control bar API
Err		StatGetAttribute(UInt16 selector, UInt32* dataP)
				PINS_TRAP(pinStatGetAttribute);
				
Err		StatHide(void)
				PINS_TRAP( pinStatHide);
				
Err		StatShow(void)
				PINS_TRAP(pinStatShow);
				
#ifdef __cplusplus
}
#endif


#endif // _PENINPUTMGR_H_
