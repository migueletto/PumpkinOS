/**
 * \file HsChars.h
 *
 * Definitions of Handspring's character codes.
 * (see Palm's Chars.h)
 *
 * \license
 *
 * Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsChars.h#5 $
 *
 ****************************************************************/

#ifndef __HSCHAR_H__
#define __HSCHAR_H__

// ------------------------------------------------------------------------------
//  Key codes and events
// ------------------------------------------------------------------------------

// Keycode range assigned to us from Palm
#define	hsChrRangeMin				  0x1600  // vchrSlinkyMin
#define	hsChrRangeMax				  0x16FF  // vchrSlinkyMax


// OBSOLETE - No more graffiti area so no more "dots"
// key codes for the "dot" soft icons
//#define hsChrMidLeftDot			  0x1600
//#define hsChrMidRightDot			  0x1601
//#define hsChrBotLeftDot			  0x1602
//#define hsChrBotRightDot			  0x1603

// OBSOLETE - we no longer detect the serial cradle
// The virtual cradle 2 character for notification of serial connection
//#define	hsChrCradle2OnChr		  0x1604	  // dock input level asserted
//#define	hsChrCradle2OffChr		  0x1605	  // dock input level de-asserted

// OBSOLETE - No more springboard
// card removed or inserted
//#define	hsChrCardStatusChg		  0x1606

// DOLATER: OBSOLETE? - PhonePrefs checks it, but nobody enqueues it.
// no-op key event that is used to interrupt apps that are
// interrupted by key events...
#define hsChrNoop					  0x1607

// OBSOLETE - Replaced by Palm's vchrJogPush
// jog-in key
//#define hsChrJogScan				  0x1608

// key that invokes ListType
#define hsChrSymbol					  0x1609
#define hsChrPostProcess			  hsChrSymbol

// answer incoming call
#define hsChrAnswer					  0x160A

// the lid was either opened or closed
#define hsChrLidStatusChg			  0x160B

// toggle the radio power
#define hsChrRadioPower				  0x160C

// shift, option, or control (menu) - update the graffiti state indicator
#define hsChrModifierKey			  0x160D	// was hsChrUpdateGrfIndicator

// shift + page-up
#define	hsChrShiftPageUp			  0x160E

// shift + page-down
#define hsChrShiftPageDown			  0x160F

// please shut down netlib
#define hsChrShutdownNetlib			  0x1610

// mute switch on or off
#define hsChrMuteOn					  0x1611
#define hsChrMuteOff				  0x1612


// 5-way rocker (see "Chars.h" section of HsNavCommon.h)
//#define vchrRockerUp					0x0132		// 5-way rocker up
//#define vchrRockerDown				0x0133		// 5-way rocker down
//#define vchrRockerLeft				0x0134		// 5-way rocker left
//#define vchrRockerRight				0x0135		// 5-way rocker right
//#define vchrRockerCenter				0x0136		// 5-way rocker center/press

// opt + app
#define hsChrOptHard1				  vchrHard5
#define hsChrOptHard2				  vchrHard6
#define hsChrOptHard3				  vchrHard7
#define hsChrOptHard4				  vchrHard8
#define hsChrOptHardPower			  vchrHard9
#define hsChrOptReserved			  vchrHard10	// for future use

// AVAILABLE
//#define hsChrAvailable13			  0x1613
//#define hsChrAvailable14			  0x1614
//#define hsChrAvailable15			  0x1615
//#define hsChrAvailable16			  0x1616
//#define hsChrAvailable17			  0x1617

// enqueued after the display is turned on
// (the display is usually still off for Palm's late wakeup)
#define hsChrWakeup					  0x1618

// causes the keyguard dialog to be displayed
// for internal use only... use hsAttrKeyboardLocked to programmatically enable/disable it
#define hsChrKeyboardLock			  0x1619

// delete the word at the cursor in a text edit field
#define hsChrDeleteWord				  0x161A

// we now define these audio volume up and down key events (original Treo used vchrJogUp and vchrJogDown)
#define hsChrVolumeUp				  0x161B
#define hsChrVolumeDown				  0x161C

// this character is enqueued by NetMaster library when it needs to execute
// code from the UI task's context while handling a background login request
#define hsChrNetMasterUIExecute		  0x161D

// This character is enqueued by our patch to EvtGetEvent when the device needs
//	to shutdown because the battery is too low.  If the battery countdown
//	dialog is able to handle the shutdown itself, then it will clear the
//	system's shutdown time and the system will not shutdown the device.  If the
//	battery countdown dialog is not able to handle the shutdown itself, the
//	system will handle the shutdown the next time EvtGetEvent is called.
#define hsChrLowBatteryShutdown		  0x161E

// side button (below volume keys on Treo650)
#define hsChrSide					  0x161F

// Opt + RightShift == "dark mode" on Treo650
#define hsChrDarkMode                                     0x1620

#endif  // __HSCHAR_H__


