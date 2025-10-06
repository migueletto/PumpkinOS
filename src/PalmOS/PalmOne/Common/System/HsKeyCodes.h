/******************************************************************************
 * Copyright (c) 2004 PalmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
 
/**
 * @file 	HsKeyCodes.h
 * @version 1.0
 * @date 	
 *
 *        This file defines the key codes.  These match the vchrs for
 *        virtual key characters, but vchars are only used in the legacy
 *        event loop.  Everywhere else, they are considered deprecated
 *        in favor of key now.  These are NOT ascii, unicode or otherwise
 *        although, for standard QWERTY US keboards, the key codes for
 *        the letters are the same as the lowercase ASCII values, but
 *        only so it's easier to determine what you're looking at.
 *
 * Note:
 *		  This is a Palm OS 6 header that we got early in order to make 
 *		  the Handspring key driver as close as possible to what it
 *		  should be in the future. This header will be depricated in 
 *		  favor of Palm's official header once it's in their SDK.
 *
 */

#ifndef __KEY_CODES_H__
#define __KEY_CODES_H__


/*
 *	How are these different from the 5 way jog?
 *		chrLeftArrow
 *		chrRightArrow
 *		chrUpArrow
 *		chrDownArrow
 */

/*
 *	Keys with a "HS" comment are unique to Handspring
 */

typedef enum {
	keyNull							= 0x0000,
	
	keyBackspace					= 0x0008,
	keyTab							= 0x0009,
	keyReturn						= 0x000a,
	keyPageUp						= 0x000b,
	keyPageDown						= 0x000c,
	keyNumericEnter					= 0x000d,
	
	keyEscape						= 0x001b,
	
	keyLeftArrow					= 0x001c,
	keyRightArrow					= 0x001d,
	keyUpArrow						= 0x001e,
	keyDownArrow					= 0x001f,

	keySpace						= 0x0020,
	
	keySingleQuote					= 0x0027,
	keyComma						= 0x002c,
	keyDash							= 0x002d,
	keyPeriod						= 0x002e,
	keySlash						= 0x002f,
	
	keyZero							= 0x0030,
	keyOne							= 0x0031,
	keyTwo							= 0x0032,
	keyThree						= 0x0033,
	keyFour							= 0x0034,
	keyFive							= 0x0035,
	keySix							= 0x0036,
	keySeven						= 0x0037,
	keyEight						= 0x0038,
	keyNine							= 0x0039,
	
	keySemiColon					= 0x003b,
	keyLessThan						= 0x003c, // many European keyboards have this key (next to the left shift)
	keyEquals						= 0x003d,
	keyAt							= 0x0040,

	keyOpenBracket					= 0x005b,
	keyBackslash					= 0x005c,
	keyCloseBracket					= 0x005d,
	keyBacktick						= 0x0060,
	
	keyA							= 0x0061,
	keyB							= 0x0062,
	keyC							= 0x0063,
	keyD							= 0x0064,
	keyE							= 0x0065,
	keyF							= 0x0066,
	keyG							= 0x0067,
	keyH							= 0x0068,
	keyI							= 0x0069,
	keyJ							= 0x006a,
	keyK							= 0x006b,
	keyL							= 0x006c,
	keyM							= 0x006d,
	keyN							= 0x006e,
	keyO							= 0x006f,
	keyP							= 0x0070,
	keyQ							= 0x0071,
	keyR							= 0x0072,
	keyS							= 0x0073,
	keyT							= 0x0074,
	keyU							= 0x0075,
	keyV							= 0x0076,
	keyW							= 0x0077,
	keyX							= 0x0078,
	keyY							= 0x0079,
	keyZ							= 0x007a,

	// Some old virtual character codes that can also serve as
	// physical key codes for actions.
	keyNextField					= 0x0103,
	keyMenu							= 0x0105,
	keyLaunch						= 0x0108,	// HS
	keyFind							= 0x010A,
	keyPrevField					= 0x010C,
	keyLockDevice					= 0x0112,
	
	keyThumbWheelUp					= 0x012E,	// (JogUp)
	keyThumbWheelDown				= 0x012F,	// (JogDown)
	keyThumbWheelPush				= 0x0130,	// (JogPress)
	keyThumbWheelBack				= 0x0131,	// (JogBack)

	keyRockerUp						= 0x0132,
	keyRockerDown					= 0x0133,
	keyRockerLeft					= 0x0134,
	keyRockerRight					= 0x0135,
	keyRockerCenter					= 0x0136,

	keyHard1						= 0x0204,
	keyHard2						= 0x0205,
	keyHard3						= 0x0206,
	keyHard4						= 0x0207,

	keyHardPower					= 0x0208,
	keyHardCradle					= 0x0209,
	keyHardCradle2					= 0x020A,
	keyHardContrast					= 0x020B,
	keyHardAntenna					= 0x020C,
	keyHardBrightness				= 0x020D,

	// keyHard 5-10 are for Licensees & Silicon Partners to use.
	// That means that this key code might be something totally different on
	// a different device.  Be warned.
	// (note that values in this range are necessary due to the way
	// TxtCharIsHardKey is defined)
	keyHard5						= 0x0214,
	keyHard6						= 0x0215,
	keyHard7						= 0x0216,
	keyHard8						= 0x0217,
	keyHard9						= 0x0218,
	keyHard10						= 0x0219,

	// Handspring specific keys
	hsKeyMfgTest					= 0x1600,	// HS
	hsKeySymbol						= 0x1609,	// HS (Keycap says "Alt")
	hsKeyRadioPower					= 0x160C,	// HS

	// These are keys that are found on standard computer keyboards
	// that aren't covered above
	
	keyLeftShift					= 0xe000,	// (Handspring shift key)
	keyRightShift					= 0xe001,
	keyLeftControl					= 0xe002,
	keyRightControl					= 0xe003,
	keyLeftCommand					= 0xe004,
	keyRightCommand					= 0xe005,
	keyLeftAlt						= 0xe006,	// (Handspring option key)
	keyRightAlt						= 0xe007,
	
	keySide							= 0xe008,	// HS (side button)

	keyHelp							= 0xe009,
	
	keyNumLock						= 0xe00a,
	keyScrollLock					= 0xe00b,
	keyCapsLock						= 0xe00c,

	keyInsert						= 0xe00d,
	keyDelete						= 0xe00e,
	keyHome							= 0xe00f,
	keyEnd							= 0xe010,
	
	keyPrintScreen					= 0xe011,
	keyPause						= 0xe012,
	
	keyNumericPeriod				= 0xe013,
	keyNumericPlus					= 0xe014,
	keyNumericDash					= 0xe015,
	keyNumericAsterisk				= 0xe016,
	keyNumericSlash					= 0xe017,
	keyNumericEquals				= 0xe018,
	
	keyNumericZero					= 0xe020,
	keyNumericOne					= 0xe021,
	keyNumericTwo					= 0xe022,
	keyNumericThree					= 0xe023,
	keyNumericFour					= 0xe024,
	keyNumericFive					= 0xe025,
	keyNumericSix					= 0xe026,
	keyNumericSeven					= 0xe027,
	keyNumericEight					= 0xe028,
	keyNumericNine					= 0xe029,
	
	keyF1							= 0xe031,
	keyF2							= 0xe032,
	keyF3							= 0xe033,
	keyF4							= 0xe034,
	keyF5							= 0xe035,
	keyF6							= 0xe036,
	keyF7							= 0xe037,
	keyF8							= 0xe038,
	keyF9							= 0xe039,
	keyF10							= 0xe03a,
	keyF11							= 0xe03b,
	keyF12							= 0xe03c,
	keyF13							= 0xe03d,
	keyF14							= 0xe03e,
	keyF15							= 0xe03f,

	
	// These are multimedia, internet or otherwise interesting buttons
	// that are sometimes found on keyboards or devices.
	
	keyBack							= 0xe050,
	keyForward						= 0xe051,
	keyStopInternet					= 0xe052,
	keyRefresh						= 0xe053,
	keySearch						= 0xe054,
	keyFavorites					= 0xe055,
	keyHomePage						= 0xe056,

	keyMail							= 0xe057,

	keyMute							= 0xe058,
	keyVolumeUp						= 0xe059,
	keyVolumeDown					= 0xe05a,
	keyPlay							= 0xe05b,
	keyPauseAudio					= 0x305c,
	keyPlayPause					= 0xe05d,
	keyStopAudio					= 0xe05e,
	keyPrevTrack					= 0xe05f,
	keyNextTrack					= 0xe060,
	keyFastForward					= 0xe061,
	keyRewind						= 0xe062,
	keyEject						= 0xe063,
	
	keyMedia						= 0xe064,

	keyMyComputer					= 0xe065,
	keyMyDocuments					= 0xe066,
	keyMyMusic						= 0xe067,
	keyMyPictures					= 0xe068,
	keyCalculator					= 0xe069,

	keyLogOff						= 0xe06a

	
} KeyCodeType;

#endif 	// __KEY_CODES_H__
