/** 
 * \file HsKeyTypes.h
 *
 * Handspring's key constant definitions shared by DAL and Palm OS
 * (see Palm's CmnKeyTypes.h)
 *
 * \license
 * 
 * Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsKeyTypes.h#4 $
 *
 ****************************************************************/

#ifndef __HSKEYTYPES_H__
#define __HSKEYTYPES_H__

//#include "HsChars.h"
#include <Common/System/HsChars.h>

/********************************************************************
 * Definition of the Bit numbers corresponding to Palm's key bits
 * (see CmnKeyTypes.h)
 ********************************************************************/
#define	keyBitNumPower					 0	// Power key
#define	keyBitNumPageUp					 1	// Page-up
#define	keyBitNumPageDown				 2	// Page-down
#define	keyBitNumHard1					 3	// App #1
#define	keyBitNumHard2					 4	// App #2
#define	keyBitNumHard3					 5	// App #3
#define	keyBitNumHard4					 6	// App #4
#define	keyBitNumCradle					 7	// Button on cradle
#define	keyBitNumAntenna				 8	// Antenna "key" <chg 3-31-98 RM>
#define	keyBitNumContrast				 9	// Contrast key


#define keyBitNumJogUp					12	// jog wheel up
#define keyBitNumJogDown				13	// jog wheel down
#define keyBitNumJogPress				14	// press/center on jog wheel
#define keyBitNumJogBack				15	// jog wheel back button
#define keyBitNumRockerUp				16	// 5-way rocker up
#define keyBitNumRockerDown				17	// 5-way rocker down
#define keyBitNumRockerLeft				18	// 5-way rocker left
#define keyBitNumRockerRight			19	// 5-way rocker right
#define keyBitNumRockerCenter			20	// 5-way rocker center/press


// Definition of the 1st extension bit field returned in the bit
//  field array passed into KeyCurrentStateEx.  The following bit
//	names are based on the American keyboard.
#define	keyBitExt1Q				0x00000001	// Q key
#define keyBitNumExt1Q				 	 0
#define	keyBitExt1W				0x00000002	// W key
#define keyBitNumExt1W					 1
#define	keyBitExt1E				0x00000004	// E key
#define keyBitNumExt1E					 2
#define	keyBitExt1R				0x00000008	// R key
#define keyBitNumExt1R					 3
#define	keyBitExt1T				0x00000010	// T key
#define keyBitNumExt1T					 4
#define	keyBitExt1Y				0x00000020	// Y key
#define keyBitNumExt1Y					 5
#define	keyBitExt1U				0x00000040	// U key
#define keyBitNumExt1U					 6
#define	keyBitExt1I				0x00000080	// I key
#define keyBitNumExt1I					 7
#define	keyBitExt1O				0x00000100	// O key
#define keyBitNumExt1O					 8
#define	keyBitExt1P				0x00000200	// P key
#define keyBitNumExt1P					 9
#define	keyBitExt1A				0x00000400	// A key
#define keyBitNumExt1A					10
#define	keyBitExt1S				0x00000800	// S key
#define keyBitNumExt1S					11
#define	keyBitExt1D				0x00001000	// D key
#define keyBitNumExt1D					12
#define	keyBitExt1F				0x00002000	// F key
#define keyBitNumExt1F					13
#define	keyBitExt1G				0x00004000	// G key
#define keyBitNumExt1G					14
#define	keyBitExt1H				0x00008000	// H key
#define keyBitNumExt1H					15
#define	keyBitExt1J				0x00010000	// J key
#define keyBitNumExt1J					16
#define	keyBitExt1K				0x00020000	// K key
#define keyBitNumExt1K					17
#define	keyBitExt1L				0x00040000	// L key
#define keyBitNumExt1L					18
#define keyBitExt1Bksp			0x00080000	// Backspace key
#define keyBitNumExt1Bksp				19
#define	keyBitExt1Opt			0x00100000	// Option key
#define keyBitNumExt1Opt				20
#define	keyBitExt1Z				0x00200000	// Z key
#define keyBitNumExt1Z					21
#define	keyBitExt1X				0x00400000	// X key
#define keyBitNumExt1X					22
#define	keyBitExt1C				0x00800000	// C key
#define keyBitNumExt1C					23
#define	keyBitExt1V				0x01000000	// V key
#define keyBitNumExt1V					24
#define	keyBitExt1B				0x02000000	// B key
#define keyBitNumExt1B					25
#define	keyBitExt1N				0x04000000	// N key
#define keyBitNumExt1N					26
#define	keyBitExt1M				0x08000000	// M key
#define keyBitNumExt1M					27
#define	keyBitExt1Period		0x10000000	// Period key
#define keyBitNumExt1Period				28
#define keyBitExt1Enter			0x20000000	// Enter key
#define keyBitNumExt1Enter				29
#define	keyBitExt1Shift			0x40000000	// Shift key
#define keyBitNumExt1Shift				30
#define keyBitExt1Space			0x80000000	// Space key
#define keyBitNumExt1Space				31

#define	keyBitsAllExt1			0xFFFFFFFF	// All keys


// Definition of the 2nd extension bit field returned in the bit
//  field array passed into KeyCurrentStateEx.  The following bit
//	names are based on the American keyboard.
#define	keyBitExt2Symbol		0x00000001	// Symbol (list type)
#define keyBitNumExt2Symbol				 0
#define	keyBitExt2CmdMenu		0x00000002	// Cmd/menu
#define keyBitNumExt2CmdMenu			 1
//#define keyBitExt2JogUp		0x00000004	// OBSOLETE: Treo Jog up
//#define keyBitNumExt2JogUp			 2
//#define keyBitExt2JogDown		0x00000008	// OBSOLETE: Treo Jog down
//#define keyBitNumExt2JogDown			 3
//#define keyBitExt2JogScan		0x00000010	// OBSOLETE: Treo Jog scan
//#define keyBitNumExt2JogScan			 4
#define keyBitExt2Launcher		0x00000020	// Launcher
#define keyBitNumExt2Launcher			 5
#define keyBitExt2Unused6		0x00000040	// UNUSED: Available for use (was going to be RadioPower)
#define keyBitNumExt2Unused6			 6
#define keyBitExt2MfgTest		0x00000080	// Manufacturing Test
#define keyBitNumExt2MfgTest			 7
#define keyBitExt2VolumeUp		0x00000100	// Volume Up
#define keyBitNumExt2VolumeUp			 8
#define keyBitExt2VolumeDown	0x00000200	// Volume Down
#define keyBitNumExt2VolumeDown			 9
#define keyBitExt2Side			0x00000400	// Side button
#define keyBitNumExt2Side				10
#define keyBitExt2Zero			0x00000800	// Zero key
#define keyBitNumExt2Zero			    11
#define keyBitExt2RShift		0x00001000	// Right Shift key
#define keyBitNumExt2RShift				12

#define	keyBitsAllExt2			0xFFFFFFFF	// All keys


// This keyBit is guaranteed not to map to any character
#define keyBitNumUnused			0xFFFF


/********************************************************************
 *Special (impossible) combinations of modifiers that we use
 * to send special events through the key queue
 ********************************************************************/
#define specialHoldKeyMask    (autoRepeatKeyMask | doubleTapKeyMask)
#define specialReleaseKeyMask (autoRepeatKeyMask | poweredOnKeyMask)
#define specialAvail3KeyMask  (doubleTapKeyMask  | poweredOnKeyMask)
#define specialAvail4KeyMask  (autoRepeatKeyMask | doubleTapKeyMask | poweredOnKeyMask)

#define specialKeyMask		  (autoRepeatKeyMask | doubleTapKeyMask | poweredOnKeyMask)


/********************************************************************
 *Extended shift state values for supporting an option and option lock state
 ********************************************************************/
// Temp shift states (see Graffiti.h)
#define hsGrfTempShiftOpt		(grfTempShiftLower+1)

// Local names for shift lock states (see GraffitiShift.h)
#define hsGlfOptLock			(glfNumLock << 1)

// Treo names (for compatability)
//#define keyGrfTempShiftOption	hsGrfTempShiftOpt
//#define keyGlfOptLock			hsGlfOptLock

// Internal shift states (extends GsiShiftStateTag)
#define hsGsiOptLock			0xFE
#define hsGsiShiftOpt			0xFF

#endif  // __HSKEYTYPES_H__


