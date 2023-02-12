/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Chars.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *        This file defines the characters in fonts.
 *
 *****************************************************************************/

#ifndef __CHARS_H__
#define __CHARS_H__

// Standard Unicode 2.0 names for the ascii characters. These exist in
// all of the text fonts, no matter what character encoding is being
// used by PalmOS.

#define	chrNull							0x0000
#define	chrStartOfHeading				0x0001
#define	chrStartOfText					0x0002
#define	chrEndOfText					0x0003
#define	chrEndOfTransmission			0x0004
#define	chrEnquiry						0x0005
#define	chrAcknowledge					0x0006
#define	chrBell							0x0007
#define	chrBackspace					0x0008
#define	chrHorizontalTabulation			0x0009
#define	chrLineFeed						0x000A
#define	chrVerticalTabulation			0x000B
#define	chrFormFeed						0x000C
#define	chrCarriageReturn				0x000D
#define	chrShiftOut						0x000E
#define	chrShiftIn						0x000F
#define	chrDataLinkEscape				0x0010
#define	chrDeviceControlOne				0x0011
#define	chrDeviceControlTwo				0x0012
#define	chrDeviceControlThree			0x0013
#define	chrDeviceControlFour			0x0014
#define	chrNegativeAcknowledge			0x0015
#define	chrSynchronousIdle				0x0016
#define	chrEndOfTransmissionBlock		0x0017
#define	chrCancel						0x0018
#define	chrEndOfMedium					0x0019
#define	chrSubstitute					0x001A
#define	chrEscape						0x001B
#define	chrFileSeparator				0x001C
#define	chrGroupSeparator				0x001D
#define	chrRecordSeparator				0x001E
#define	chrUnitSeparator				0x001F
#define	chrSpace						0x0020
#define	chrExclamationMark				0x0021
#define	chrQuotationMark				0x0022
#define	chrNumberSign					0x0023
#define	chrDollarSign					0x0024
#define	chrPercentSign					0x0025
#define	chrAmpersand					0x0026
#define	chrApostrophe					0x0027
#define	chrLeftParenthesis				0x0028
#define	chrRightParenthesis				0x0029
#define	chrAsterisk						0x002A
#define	chrPlusSign						0x002B
#define	chrComma						0x002C
#define	chrHyphenMinus					0x002D
#define	chrFullStop						0x002E
#define	chrSolidus						0x002F
#define	chrDigitZero					0x0030
#define	chrDigitOne						0x0031
#define	chrDigitTwo						0x0032
#define	chrDigitThree					0x0033
#define	chrDigitFour					0x0034
#define	chrDigitFive					0x0035
#define	chrDigitSix						0x0036
#define	chrDigitSeven					0x0037
#define	chrDigitEight					0x0038
#define	chrDigitNine					0x0039
#define	chrColon						0x003A
#define	chrSemicolon					0x003B
#define	chrLessThanSign					0x003C
#define	chrEqualsSign					0x003D
#define	chrGreaterThanSign				0x003E
#define	chrQuestionMark					0x003F
#define	chrCommercialAt					0x0040
#define	chrCapital_A					0x0041
#define	chrCapital_B					0x0042
#define	chrCapital_C					0x0043
#define	chrCapital_D					0x0044
#define	chrCapital_E					0x0045
#define	chrCapital_F					0x0046
#define	chrCapital_G					0x0047
#define	chrCapital_H					0x0048
#define	chrCapital_I					0x0049
#define	chrCapital_J					0x004A
#define	chrCapital_K					0x004B
#define	chrCapital_L					0x004C
#define	chrCapital_M					0x004D
#define	chrCapital_N					0x004E
#define	chrCapital_O					0x004F
#define	chrCapital_P					0x0050
#define	chrCapital_Q					0x0051
#define	chrCapital_R					0x0052
#define	chrCapital_S					0x0053
#define	chrCapital_T					0x0054
#define	chrCapital_U					0x0055
#define	chrCapital_V					0x0056
#define	chrCapital_W					0x0057
#define	chrCapital_X					0x0058
#define	chrCapital_Y					0x0059
#define	chrCapital_Z					0x005A
#define	chrLeftSquareBracket			0x005B
// #define	chrReverseSolidus			0x005C (not in Japanese fonts)
#define	chrRightSquareBracket			0x005D
#define	chrCircumflexAccent				0x005E
#define	chrLowLine						0x005F
#define	chrGraveAccent					0x0060
#define	chrSmall_A						0x0061
#define	chrSmall_B						0x0062
#define	chrSmall_C						0x0063
#define	chrSmall_D						0x0064
#define	chrSmall_E						0x0065
#define	chrSmall_F						0x0066
#define	chrSmall_G						0x0067
#define	chrSmall_H						0x0068
#define	chrSmall_I						0x0069
#define	chrSmall_J						0x006A
#define	chrSmall_K						0x006B
#define	chrSmall_L						0x006C
#define	chrSmall_M						0x006D
#define	chrSmall_N						0x006E
#define	chrSmall_O						0x006F
#define	chrSmall_P						0x0070
#define	chrSmall_Q						0x0071
#define	chrSmall_R						0x0072
#define	chrSmall_S						0x0073
#define	chrSmall_T						0x0074
#define	chrSmall_U						0x0075
#define	chrSmall_V						0x0076
#define	chrSmall_W						0x0077
#define	chrSmall_X						0x0078
#define	chrSmall_Y						0x0079
#define	chrSmall_Z						0x007A
#define	chrLeftCurlyBracket				0x007B
#define	chrVerticalLine					0x007C
#define	chrRightCurlyBracket			0x007D
#define	chrTilde						0x007E
#define	chrDelete						0x007F


// Special meanings given to characters by the PalmOS
#define	chrTab						chrHorizontalTabulation			// 0x0009
#define	vchrPageUp					chrVerticalTabulation			// 0x000B
#define	vchrPageDown				chrFormFeed						// 0x000C
#define	chrOtaSecure				chrDeviceControlFour			// 0x0014
#define	chrOta						chrNegativeAcknowledge			// 0x0015
#define	chrCommandStroke			chrSynchronousIdle				// 0x0016
#define	chrShortcutStroke			chrEndOfTransmissionBlock		// 0x0017
#define	chrEllipsis					chrCancel						// 0x0018
#define	chrNumericSpace				chrEndOfMedium					// 0x0019
#define	chrCardIcon					chrSubstitute					// 0x001A	Card Icon glyph, added in PalmOS 4.0
#define	chrLeftArrow				chrFileSeparator				// 0x001C
#define	chrRightArrow				chrGroupSeparator				// 0x001D
#define	chrUpArrow					chrRecordSeparator				// 0x001E
#define	chrDownArrow				chrUnitSeparator				// 0x001F


//  The following are key codes used for virtual events, like
//   low battery warnings, etc. These keyboard events MUST
//   have the commandKeyMask bit set in the modifiers in order
//   to be recognized.
#define	vchrLowBattery				0x0101		// Display low battery dialog
#define	vchrEnterDebugger			0x0102		// Enter Debugger
#define	vchrNextField				0x0103		// Go to next field in form
#define	vchrStartConsole			0x0104		// Startup console task
#define	vchrMenu					0x0105		// Ctl-A
#define	vchrCommand					0x0106		// Ctl-C
#define	vchrConfirm					0x0107		// Ctl-D
#define	vchrLaunch					0x0108		// Ctl-E
#define	vchrKeyboard				0x0109		// Ctl-F popup the keyboard in appropriate mode
#define	vchrFind					0x010A
#define	vchrCalc					0x010B
#define	vchrPrevField				0x010C
#define	vchrAlarm					0x010D		// sent before displaying an alarm
#define	vchrRonamatic				0x010E		// stroke from graffiti area to top half of screen
#define	vchrGraffitiReference		0x010F		// popup the Graffiti reference
#define	vchrKeyboardAlpha			0x0110		// popup the keyboard in alpha mode
#define	vchrKeyboardNumeric			0x0111		// popup the keyboard in number mode
#define	vchrLock					0x0112		// switch to the Security app and lock the device
#define	vchrBacklight				0x0113		// toggle state of backlight
#define	vchrAutoOff					0x0114		// power off due to inactivity timer
// Added for PalmOS 3.0
#define	vchrExgTest					0x0115		// put exchange Manager into test mode (&.t)
#define	vchrSendData				0x0116		// Send data if possible
#define	vchrIrReceive				0x0117		// Initiate an Ir receive manually (&.i)
// Added for PalmOS 3.1
#define	vchrTsm1					0x0118		// Text Services silk-screen button
#define	vchrTsm2					0x0119		// Text Services silk-screen button
#define	vchrTsm3					0x011A		// Text Services silk-screen button
#define	vchrTsm4					0x011B		// Text Services silk-screen button	
// Added for PalmOS 3.2
#define	vchrRadioCoverageOK			0x011C		// Radio coverage check successful
#define	vchrRadioCoverageFail		0x011D		// Radio coverage check failure
#define	vchrPowerOff				0x011E		// Posted after autoOffChr or hardPowerChr
												// to put system to sleep with SysSleep.
// Added for PalmOS 3.5
#define	vchrResumeSleep				0x011F		// Posted by NotifyMgr clients after they
												// have deferred a sleep request in order 
												// to resume it.
#define	vchrLateWakeup				0x0120		// Posted by the system after waking up
												// to broadcast a late wakeup notification.
												// FOR SYSTEM USE ONLY
#define	vchrTsmMode					0x0121		// Posted by TSM to trigger mode change.
#define	vchrBrightness				0x0122		// Activates brightness adjust dialog
#define	vchrContrast				0x0123		// Activates contrast adjust dialog

#define	vchrExpCardInserted			0x0124		// ExpansionMgr card inserted & removed.  
#define	vchrExpCardRemoved			0x0125		// NOTE: these keys will never show up in an 
												// app's event loop (they are caught inside 
												// EvtGetEvent()), and will probably be 
												// deprecated soon (see comments in ExpansionMgr.c).
#define  vchrExgIntData				0x01FF		// Exchange Manager wakeup event

// Added for PalmOS 4.0				NOTE: 0x1FF is used above - not in numeric order!
#define	vchrAttnStateChanged		0x0126		// Posted by AttentionMgr API to open or update dialog
#define	vchrAttnUnsnooze			0x0127		// Posted when AttentionMgr snooze timer expires
#define	vchrAttnIndicatorTapped		0x0128		// Posted when AttentionIndicator is tapped
#define	vchrAttnAllowClose			0x0129		// Posted when AttnAllowClose is called
#define	vchrAttnReopen				0x012A		// Posted when AttnReopen is called
#define vchrCardCloseMenu			0x012B		// Posted when a card is inserted
#define vchrIrGotData				0x012C		// Posted when IR Receive initiated
												// and copying of an app is imminent

// Added for PalmOS 5.0
#define	vchrResetAutoOff			0x012D		// Resets autoOff timer in EvtMgr

// Added for PalmOS 5.0 R2
// the following vchrJog/Rocker values exist to allow all hardware that has these
// (optional) control clusters to emit the same key codes.
#define vchrThumbWheelUp			0x012E		// optional thumb-wheel up
#define vchrThumbWheelDown			0x012F		// optional thumb-wheel down
#define vchrThumbWheelPush			0x0130		// optional thumb-wheel press/center
#define vchrThumbWheelBack			0x0131		// optional thumb-wheel cluster back

#define vchrRockerUp				0x0132		// 5-way rocker up
#define vchrRockerDown				0x0133		// 5-way rocker down
#define vchrRockerLeft				0x0134		// 5-way rocker left
#define vchrRockerRight				0x0135		// 5-way rocker right
#define vchrRockerCenter			0x0136		// 5-way rocker center/press

#define vchrInputAreaControl		0x0137		// Toggle for opening and closing input area


// The application launching buttons generate the following
// key codes and will also set the commandKeyMask bit in the 
// modifiers field
#define	vchrHardKeyMin				0x0200
#define	vchrHardKeyMax				0x02FF		// 256 hard keys

#define	vchrHard1					0x0204
#define	vchrHard2					0x0205
#define	vchrHard3					0x0206
#define	vchrHard4					0x0207

#define	vchrHardPower				0x0208
#define	vchrHardCradle				0x0209		// Button on cradle pressed
#define	vchrHardCradle2				0x020A		// Button on cradle pressed and hwrDockInGeneric1
												// input on dock asserted (low).
#define	vchrHardContrast			0x020B		// Sumo's Contrast button
#define	vchrHardAntenna				0x020C		// Eleven's Antenna switch
#define	vchrHardBrightness			0x020D		// Hypothetical Brightness button

// Added the following vchrHardXX values for Licensees & Silicon Partners to use:
// (note that values in this range are necessary due to the way TxtCharIsHardKey is defined)
#define	vchrHard5					0x0214		// Alternative vchrHard1 action
#define	vchrHard6					0x0215		// Alternative vchrHard2 action
#define	vchrHard7					0x0216		// Alternative vchrHard3 action
#define	vchrHard8					0x0217		// Alternative vchrHard4 action
#define	vchrHard9					0x0218		// Alternative vchrHardPower or other action
#define	vchrHard10					0x0219		// Alternative vchrHardCradle or other action

// The following key character RANGES are reserved for use by licensees.
// All have the commandKeyMask bit set in the event's modifiers field.
// Note that ranges include the Min and Max values themselves (i.e. key
// codes >= min and <= max are assigned to the following licensees).
//
// Virtual key events have the key CHARACTER field of the event record
// set to the vchrXXX value. As a general rule the key CODE field
// of the event is set to zero, however, this field may be used to
// pass along additional information about the virtual key event; for
// example, the source of the event.
// 
// Programming example:
//	
// 	theErr = EvtEnqueueKey(vchrXXX, 0, commandKeyMask);
//

//		Kyocera (formerly Qualcomm)
#define	vchrThumperMin				0x0300
#define	vchrThumperMax				0x03FF			// 256 command keys

//		Palm
#define	vchrPalmMin					0x0500			// 256 command keys
#define	vchrPalmMax					0x05FF


//		TRG
#define	vchrCFlashMin				0x1500
#define	vchrCFlashMax				0x150F			//  16 command keys

//		Samsung
#define	vchrPhoenixMin				0x1550
#define	vchrPhoenixMax				0x156F			//  32 command keys

//		Symbol
#define	vchrSPTMin					0x15A0
#define	vchrSPTMax					0x15AF			//  16 command keys

//		Handspring
#define	vchrSlinkyMin				0x1600
#define	vchrSlinkyMax				0x16FF			// 256 command keys

//		Sony
#define	vchrSonyMin					0x1700			// 256 command keys
#define	vchrSonyMax					0x17FF			// (range increased from previous 16)

//		Acer
#define	vchrAcerMin					0x1800
#define	vchrAcerMax					0x18FF			// 256 command keys


#define	vchrLegendMin				0x1C00			//  16 command keys
#define	vchrLegendMax				0x1C0F

//		AlphaSmart
#define	vchrAlphaSmartMin			0x2000			// 256 command keys
#define	vchrAlphaSmartMax			0x20FF




// Old names for some of the characters.
#define	nullChr						chrNull							// 0x0000
#define	backspaceChr				chrBackspace					// 0x0008
#define	tabChr						chrHorizontalTabulation			// 0x0009
#define	linefeedChr					chrLineFeed						// 0x000A
#define	pageUpChr					vchrPageUp						// 0x000B
#define	chrPageUp					vchrPageUp						// 0x000B
#define	pageDownChr					vchrPageDown					// 0x000C
#define	chrPageDown					vchrPageDown					// 0x000C
#define	crChr						chrCarriageReturn				// 0x000D
#define	returnChr					chrCarriageReturn				// 0x000D
#define	otaSecureChr				chrOtaSecure					// 0x0014
#define	otaChr						chrOta							// 0x0015

#define	escapeChr					chrEscape						// 0x001B
#define	leftArrowChr				chrLeftArrow					// 0x001C
#define	rightArrowChr				chrRightArrow					// 0x001D
#define	upArrowChr					chrUpArrow						// 0x001E
#define	downArrowChr				chrDownArrow					// 0x001F
#define	spaceChr					chrSpace						// 0x0020
#define	quoteChr					chrQuotationMark				// 0x0022 '"'
#define	commaChr					chrComma						// 0x002C ','
#define	periodChr					chrFullStop						// 0x002E '.'
#define	colonChr					chrColon						// 0x003A ':'
#define	lowBatteryChr				vchrLowBattery					// 0x0101
#define	enterDebuggerChr			vchrEnterDebugger				// 0x0102
#define	nextFieldChr				vchrNextField					// 0x0103
#define	startConsoleChr				vchrStartConsole				// 0x0104
#define	menuChr						vchrMenu						// 0x0105
#define	commandChr					vchrCommand						// 0x0106
#define	confirmChr					vchrConfirm						// 0x0107
#define	launchChr					vchrLaunch						// 0x0108
#define	keyboardChr					vchrKeyboard					// 0x0109
#define	findChr						vchrFind						// 0x010A
#define	calcChr						vchrCalc						// 0x010B
#define	prevFieldChr				vchrPrevField					// 0x010C
#define	alarmChr					vchrAlarm						// 0x010D
#define	ronamaticChr				vchrRonamatic					// 0x010E
#define	graffitiReferenceChr		vchrGraffitiReference			// 0x010F
#define	keyboardAlphaChr			vchrKeyboardAlpha				// 0x0110
#define	keyboardNumericChr			vchrKeyboardNumeric				// 0x0111
#define	lockChr						vchrLock						// 0x0112
#define	backlightChr				vchrBacklight					// 0x0113
#define	autoOffChr					vchrAutoOff						// 0x0114
#define	exgTestChr					vchrExgTest						// 0x0115
#define	sendDataChr					vchrSendData					// 0x0116
#define	irReceiveChr				vchrIrReceive					// 0x0117
#define	radioCoverageOKChr			vchrRadioCoverageOK				// 0x011C
#define	radioCoverageFailChr		vchrRadioCoverageFail			// 0x011D
#define	powerOffChr					vchrPowerOff					// 0x011E
#define	resumeSleepChr				vchrResumeSleep					// 0x011F
#define	lateWakeupChr				vchrLateWakeup					// 0x0120
#define	brightnessChr				vchrBrightness					// 0x0121
#define	contrastChr					vchrContrast					// 0x0122
#define	hardKeyMin					vchrHardKeyMin					// 0x0200
#define	hardKeyMax					vchrHardKeyMax					// 0x02FF
#define	hard1Chr					vchrHard1						// 0x0204
#define	hard2Chr					vchrHard2						// 0x0205
#define	hard3Chr					vchrHard3						// 0x0206
#define	hard4Chr					vchrHard4						// 0x0207
#define	hardPowerChr				vchrHardPower					// 0x0208
#define	hardCradleChr				vchrHardCradle					// 0x0209
#define	hardCradle2Chr				vchrHardCradle2					// 0x020A
#define	hardContrastChr				vchrHardContrast				// 0x020B
#define	hardAntennaChr				vchrHardAntenna					// 0x020C
#define	hardBrightnessChr			vchrHardBrightness				// 0x020D


// Macros to determine correct character code to use for drawing numeric space
// and horizontal ellipsis.

#define	ChrNumericSpace(chP)														\
	do {																			\
		UInt32 attribute;															\
		if ((FtrGet(sysFtrCreator, sysFtrNumROMVersion, &attribute) == 0)			\
		&& (attribute >= sysMakeROMVersion(3, 1, 0, 0, 0))) {						\
			*(chP) = chrNumericSpace;												\
		} else {																	\
			*(chP) = 0x80;															\
		}																			\
	} while (0)

#define	ChrHorizEllipsis(chP)														\
	do {																			\
		UInt32 attribute;															\
		if ((FtrGet(sysFtrCreator, sysFtrNumROMVersion, &attribute) == 0)			\
		&& (attribute >= sysMakeROMVersion(3, 1, 0, 0, 0))) {						\
			*(chP) = chrEllipsis;													\
		} else {																	\
			*(chP) = 0x85;															\
		}																			\
	} while (0)

// Characters in the 9 point symbol font.  Resource ID 9003
enum symbolChars {
	symbolLeftArrow = 3,
	symbolRightArrow,
	symbolUpArrow,
	symbolDownArrow,
	symbolSmallDownArrow,
	symbolSmallUpArrow,
	symbolMemo = 9,
	symbolHelp,
	symbolNote,
	symbolNoteSelected,
	symbolCapsLock,
	symbolNumLock,
	symbolShiftUpper,
	symbolShiftPunc,
	symbolShiftExt,
	symbolShiftNone,
	symbolNoTime,
	symbolAlarm,
	symbolRepeat,
	symbolCheckMark,
	// These next four characters were moved from the 0x8D..0x90
	// range in the main fonts to the 9pt Symbol font in PalmOS 3.1
	symbolDiamondChr,
	symbolClubChr,
	symbolHeartChr,
	symbolSpadeChr
	};

// Character in the 7 point symbol font.  Resource ID 9005
enum symbol7Chars {
	symbol7ScrollUp = 1,
	symbol7ScrollDown,
	symbol7ScrollUpDisabled,
	symbol7ScrollDownDisabled
	};

//	Characters in the 11 point symbol font.  Resource ID 9004
enum symbol11Chars {
	symbolCheckboxOff = 0,
	symbolCheckboxOn,
	symbol11LeftArrow,
	symbol11RightArrow,
	symbol11Help,
	symbol11LeftArrowDisabled,		// New for Palm OS v3.2
	symbol11RightArrowDisabled		// New for Palm OS v3.2
	};


#endif // __CHARS_H__
