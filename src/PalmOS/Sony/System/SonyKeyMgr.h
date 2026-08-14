/******************************************************************************
 *                                                                            *
 *            (C) Copyright 2000-2002, Sony Corporation                       *
 *                                                                            *
 *----------------------------------------------------------------------------*
 *                                                                            *
 *    <IDENTIFICATION>                                                        *
 *       file name    : $Workfile: SonyKeyMgr.h $
 *                                                                            *
 *    <PROFILE>                                                               *
 *       KeyBit definitions and KeyManger for Sony System                     *
 *                                                                            *
 *    <HISTORY>                                                               *
 *       Started on   : 99/04/28                                              *
 *       Last Modified: $Date: 03/06/17 17:21 $
 *                                                                            *
 ******************************************************************************/
/* this file is best viewed by setting TAB-stop as 3 */

#ifndef __SONYKEYMGR_H__
#define __SONYKEYMGR_H__

/******************************************************************************
 *    Definition of bit field returned from KeyCurrentState                   *
 ******************************************************************************/
// Those are actual keys
#define keyBitJogSelect		(0x0400)		// JogDial Push key
#define keyBitJogBack		(0x0800)		// JogDial Back key
#define keyBitCapture		(0x1000)		// Capture Button
#define keyBitVoiceRec		(0x2000)		// VoiceRec Button
#define keyBitCaptureHalf	(0x4000)		// Capture-Half Button
#define keyBitAllKeys		(0x7FFF)		// Which bits in keyState are actually
													//   used for keys
// Those are keys for Accessaries
#define keyBitGameExt0		(0x00010000L)	// GameController Extension bit0
#define keyBitGameExt1		(0x00020000L)	// GameController Extension bit1
// Those are virtual keys
#define keyBitJogUp			(0x10000000L)	// JogDial Up
#define keyBitJogDown		(0x20000000L)	// JogDial Down
#define keyBitRmcKey			(0x40000000L)	// Audio Remote Controller
#define keyBitHWKeyboard	(0x80000000L)	// Hardware Keyboard

#define keyBitHome			(0x01000000L)	// Home Button
#define keyBitGraffiti		(0x02000000L)	// Graffiti Button

/******************************************************************************
 *    Key manager Routines                                                    *
 ******************************************************************************/
/* We can add some initialization for Sony Original Keys

#ifdef __cplusplus
extern "C" {
#endif

// Set/Get the auto-key repeat rate
Err 		KeyRates(Boolean set, UInt16 *initDelayP, UInt16 *periodP, 
						UInt16 *doubleTapDelayP, Boolean *queueAheadP)
							SYS_TRAP(sysTrapKeyRates);
							
// Get the current state of the hardware keys
// This is now updated every tick, even when more than 1 key is held down.
UInt32	KeyCurrentState(void)
							SYS_TRAP(sysTrapKeyCurrentState);
							
// Set the state of the hardware key mask which controls if the key
// generates a keyDownEvent
UInt32	KeySetMask(UInt32 keyMask)
							SYS_TRAP(sysTrapKeySetMask);
							
#ifdef __cplusplus
}
#endif

*/	
#endif	//__SONYKEYMGR_H__

