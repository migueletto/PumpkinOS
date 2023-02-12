/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Keyboard.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This file defines the keyboard's  structures 
 *   and routines.
 *
 *****************************************************************************/

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <PalmTypes.h>
#include <CoreTraps.h>

#include <Event.h>		// for EventType
#include <Rect.h>			// for PointType


#define kbdReturnKey		linefeedChr
#define kbdTabKey			tabChr
#define kbdBackspaceKey	backspaceChr
#define kbdShiftKey		2
#define kbdCapsKey		1
#define kbdNoKey			0xff


typedef enum
	{
	kbdAlpha = 0,
	kbdNumbersAndPunc = 1,
	kbdAccent = 2,
	kbdDefault = 0xff		// based on graffiti mode (usually alphaKeyboard)
	} KeyboardType;


typedef struct KeyboardStatus KeyboardStatus;

// Shift state flags
#define KeyboardShiftFlag			0x0001
#define KeyboardCapslockFlag		0x0002


/************************************************************
 * Keyboard procedures
 *************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// At some point the Graffiti code will need access to the
// shift and caps lock info.  Either export the structures
// or provide calls to the info.

extern void SysKeyboardDialogV10 ()
							SYS_TRAP(sysTrapSysKeyboardDialogV10);

extern void SysKeyboardDialog (KeyboardType kbd)
							SYS_TRAP(sysTrapSysKeyboardDialog);


KeyboardStatus *KeyboardStatusNew(UInt16 keyboardID)
							SYS_TRAP(sysTrapKeyboardStatusNew);

void KeyboardStatusFree(KeyboardStatus *ks)
							SYS_TRAP(sysTrapKeyboardStatusFree);


void KbdSetLayout(KeyboardStatus *ks, UInt16 layout)
							SYS_TRAP(sysTrapKbdSetLayout);

UInt16 KbdGetLayout(const KeyboardStatus *ks)
							SYS_TRAP(sysTrapKbdGetLayout);


void KbdSetPosition(KeyboardStatus *ks, const PointType *p)
							SYS_TRAP(sysTrapKbdSetPosition);

void KbdGetPosition(const KeyboardStatus *ks, PointType *p)
							SYS_TRAP(sysTrapKbdGetPosition);


void KbdSetShiftState(KeyboardStatus *ks, UInt16 shiftState)
							SYS_TRAP(sysTrapKbdSetShiftState);

UInt16 KbdGetShiftState(const KeyboardStatus *ks)
							SYS_TRAP(sysTrapKbdGetShiftState);


void KbdDraw(KeyboardStatus *ks, Boolean keyTopsOnly, Boolean ignoreModifiers)
							SYS_TRAP(sysTrapKbdDraw);

void KbdErase(KeyboardStatus *ks)
							SYS_TRAP(sysTrapKbdErase);


Boolean KbdHandleEvent(KeyboardStatus *ks, EventType * pEvent)
							SYS_TRAP(sysTrapKbdHandleEvent);


#ifdef __cplusplus
}
#endif

#endif // __KEYBOARD_H__
