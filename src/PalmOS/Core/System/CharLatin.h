/******************************************************************************
 *
 * Copyright (c) 1994-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CharLatin.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *			This file defines the characters found in the Palm OS Latin
 *			character encoding, which is based on the Microsoft code page
 *			1252 character encoding (Microsoft extension to ISO 8859-1
 *			character encoding).
 *
 *****************************************************************************/

#ifndef __CHARLATIN_H__
#define __CHARLATIN_H__

/***********************************************************************
 * Public constants
 ***********************************************************************/

// Characters found in Chars.h are guaranteed to exist in the regular
// (stdFont, boldFont, largeFont, largeBoldFont) fonts on the device,
// even if the character encoding supported by the device is not Latin.

// The characters listed below are those from the Palm OS Latin character
// encoding which are not part of every possible character encoding that
// will be supported by the Palm OS, and thus should ONLY be used when
// you have first verified that the device's character encoding is
// 
// Characters that are part of code page 1252, but not guaranteed
// to exist in every possible PalmOS encoding. These names are based on
// the Unicode 2.0 standard.

#define	chrReverseSolidus						0x005C	// Is yen char in Japanese fonts.

#define chrEuroSign								0x0080	// Was numeric space (valid thru 3.2)
// Undefined									0x0081
#define	chrSingleLow9QuotationMark				0x0082
#define	chrSmall_F_Hook							0x0083
#define	chrDoubleLow9QuotationMark				0x0084
#define	chrHorizontalEllipsis					0x0085	// Also at 0x18 in 3.1 and later roms.
#define	chrDagger								0x0086
#define	chrDoubleDagger							0x0087
#define	chrModifierCircumflexAccent				0x0088
#define	chrPerMilleSign							0x0089
#define	chrCapital_S_Caron						0x008A
#define	chrSingleLeftPointingAngleQuotationMark	0x008B
#define	chrCapital_OE							0x008C
// Undefined									0x008D	// Was diamondChr (valid thru 3.0)
// Undefined									0x008E	// Was clubChr (valid thru 3.0)
														// Will become chrCapital_Z_Caron
// Undefined									0x008F	// Was heartChr (valid thru 3.0)
// Undefined									0x0090	// Was spadeChr (valid thru 3.0)
#define	chrLeftSingleQuotationMark				0x0091
#define	chrRightSingleQuotationMark				0x0092
#define	chrLeftDoubleQuotationMark				0x0093
#define	chrRightDoubleQuotationMark				0x0094
#define	chrBullet								0x0095
#define	chrEnDash								0x0096
#define	chrEmDash								0x0097
#define	chrSmallTilde							0x0098
#define	chrTradeMarkSign						0x0099
#define	chrSmall_S_Caron						0x009A
#define	chrSingleRightPointingAngleQuotationMark 0x009B
#define	chrSmall_OE								0x009C
// Undefined									0x009D	// Was command stroke (valid thru 3.0)
// Undefined									0x009E	// Was shortcut stroke (valid thru 3.0)
														// Will become chrSmall_Z_Caron
#define	chrCapital_Y_Diaeresis					0x009F
#define	chrNoBreakSpace							0x00A0
#define	chrInvertedExclamationMark				0x00A1
#define	chrCentSign								0x00A2
#define	chrPoundSign							0x00A3
#define	chrCurrencySign							0x00A4
#define	chrYenSign								0x00A5
#define	chrBrokenBar							0x00A6
#define	chrSectionSign							0x00A7
#define	chrDiaeresis							0x00A8
#define	chrCopyrightSign						0x00A9
#define	chrFeminineOrdinalIndicator				0x00AA
#define	chrLeftPointingDoubleAngleQuotationMark	0x00AB
#define	chrNotSign								0x00AC
#define	chrSoftHyphen							0x00AD
#define	chrRegisteredSign						0x00AE
#define	chrMacron								0x00AF
#define	chrDegreeSign							0x00B0
#define	chrPlusMinusSign						0x00B1
#define	chrSuperscriptTwo						0x00B2
#define	chrSuperscriptThree						0x00B3
#define	chrAcuteAccent							0x00B4
#define	chrMicroSign							0x00B5
#define	chrPilcrowSign							0x00B6
#define	chrMiddleDot							0x00B7
#define	chrCedilla								0x00B8
#define	chrSuperscriptOne						0x00B9
#define	chrMasculineOrdinalIndicator			0x00BA
#define	chrRightPointingDoubleAngleQuotationMark 0x00BB
#define	chrVulgarFractionOneQuarter				0x00BC
#define	chrVulgarFractionOneHalf				0x00BD
#define	chrVulgarFractionThreeQuarters			0x00BE
#define	chrInvertedQuestionMark					0x00BF
#define	chrCapital_A_Grave						0x00C0
#define	chrCapital_A_Acute						0x00C1
#define	chrCapital_A_Circumflex					0x00C2
#define	chrCapital_A_Tilde						0x00C3
#define	chrCapital_A_Diaeresis					0x00C4
#define	chrCapital_A_RingAbove					0x00C5
#define	chrCapital_AE							0x00C6
#define	chrCapital_C_Cedilla					0x00C7
#define	chrCapital_E_Grave						0x00C8
#define	chrCapital_E_Acute						0x00C9
#define	chrCapital_E_Circumflex					0x00CA
#define	chrCapital_E_Diaeresis					0x00CB
#define	chrCapital_I_Grave						0x00CC
#define	chrCapital_I_Acute						0x00CD
#define	chrCapital_I_Circumflex					0x00CE
#define	chrCapital_I_Diaeresis					0x00CF
#define	chrCapital_Eth							0x00D0
#define	chrCapital_N_Tilde						0x00D1
#define	chrCapital_O_Grave						0x00D2
#define	chrCapital_O_Acute						0x00D3
#define	chrCapital_O_Circumflex					0x00D4
#define	chrCapital_O_Tilde						0x00D5
#define	chrCapital_O_Diaeresis					0x00D6
#define	chrMultiplicationSign					0x00D7
#define	chrCapital_O_Stroke						0x00D8
#define	chrCapital_U_Grave						0x00D9
#define	chrCapital_U_Acute						0x00DA
#define	chrCapital_U_Circumflex					0x00DB
#define	chrCapital_U_Diaeresis					0x00DC
#define	chrCapital_Y_Acute						0x00DD
#define	chrCapital_Thorn						0x00DE
#define	chrSmall_SharpS							0x00DF
#define	chrSmall_A_Grave						0x00E0
#define	chrSmall_A_Acute						0x00E1
#define	chrSmall_A_Circumflex					0x00E2
#define	chrSmall_A_Tilde						0x00E3
#define	chrSmall_A_Diaeresis					0x00E4
#define	chrSmall_A_RingAbove					0x00E5
#define	chrSmall_AE								0x00E6
#define	chrSmall_C_Cedilla						0x00E7
#define	chrSmall_E_Grave						0x00E8
#define	chrSmall_E_Acute						0x00E9
#define	chrSmall_E_Circumflex					0x00EA
#define	chrSmall_E_Diaeresis					0x00EB
#define	chrSmall_I_Grave						0x00EC
#define	chrSmall_I_Acute						0x00ED
#define	chrSmall_I_Circumflex					0x00EE
#define	chrSmall_I_Diaeresis					0x00EF
#define	chrSmall_Eth							0x00F0
#define	chrSmall_N_Tilde						0x00F1
#define	chrSmall_O_Grave						0x00F2
#define	chrSmall_O_Acute						0x00F3
#define	chrSmall_O_Circumflex					0x00F4
#define	chrSmall_O_Tilde						0x00F5
#define	chrSmall_O_Diaeresis					0x00F6
#define	chrDivisionSign							0x00F7
#define	chrSmall_O_Stroke						0x00F8
#define	chrSmall_U_Grave						0x00F9
#define	chrSmall_U_Acute						0x00FA
#define	chrSmall_U_Circumflex					0x00FB
#define	chrSmall_U_Diaeresis					0x00FC
#define	chrSmall_Y_Acute						0x00FD
#define	chrSmall_Thorn							0x00FE
#define	chrSmall_Y_Diaeresis					0x00FF

// Alternative names for some characters.

#define	chrBackslash							chrReverseSolidus;
#define	chrNonBreakingSpace						chrNoBreakSpace;

// Old character names.

#define lowSingleCommaQuoteChr	chrSingleLow9QuotationMark	// 0x0082
#define scriptFChr				chrSmall_F_Hook				// 0x0083
#define lowDblCommaQuoteChr		chrDoubleLow9QuotationMark	// 0x0084
#define daggerChr				chrDagger					// 0x0086
#define dblDaggerChr			chrDoubleDagger				// 0x0087
#define circumflexChr			chrModifierCircumflexAccent	// 0x0088
#define perMilleChr				chrPerMilleSign				// 0x0089
#define upSHacekChr				chrCapital_S_Caron			// 0x008A
#define leftSingleGuillemetChr	chrSingleLeftPointingAngleQuotationMark	// 0x008B
#define upOEChr					chrCapital_OE				// 0x008C
#define singleOpenCommaQuoteChr	chrLeftSingleQuotationMark	// 0x0091
#define singleCloseCommaQuoteChr chrRightSingleQuotationMark // 0x0092
#define dblOpenCommaQuoteChr	chrLeftDoubleQuotationMark	// 0x0093
#define dblCloseCommaQuoteChr	chrRightDoubleQuotationMark	// 0x0094
#define bulletChr				chrBullet					// 0x0095
#define enDashChr				chrEnDash					// 0x0096
#define emDashChr				chrEmDash					// 0x0097
#define spacingTildeChr			chrSmallTilde				// 0x0098
#define trademarkChr			chrTradeMarkSign			// 0x0099
#define lowSHacekChr			chrSmall_S_Caron			// 0x009A
#define rightSingleGuillemetChr	chrSingleRightPointingAngleQuotationMark // 0x009B
#define lowOEChr				chrSmall_OE					// 0x009C
#define upYDiaeresisChr			chrCapital_Y_Diaeresis		// 0x009F
#define nonBreakSpaceChr		chrNoBreakSpace				// 0x00A0
#define invertedExclamationChr	chrInvertedExclamationMark	// 0x00A1
#define centChr					chrCentSign					// 0x00A2
#define poundChr				chrPoundSign				// 0x00A3
#define currencyChr				chrCurrencySign				// 0x00A4
#define yenChr					chrYenSign					// 0x00A5
#define brokenVertBarChr		chrBrokenBar				// 0x00A6
#define sectionChr				chrSectionSign				// 0x00A7
#define spacingDiaeresisChr		chrDiaeresis				// 0x00A8
#define copyrightChr			chrCopyrightSign			// 0x00A9
#define feminineOrdinalChr		chrFeminineOrdinalIndicator	// 0x00AA
#define leftGuillemetChr		chrLeftPointingDoubleAngleQuotationMark // 0x00AB
#define notChr					chrNotSign					// 0x00AC
#define softHyphenChr			chrSoftHyphen				// 0x00AD
#define registeredChr			chrRegisteredSign			// 0x00AE
#define spacingMacronChr		chrMacron					// 0x00AF
#define degreeChr				chrDegreeSign				// 0x00B0
#define plusMinusChr			chrPlusMinusSign			// 0x00B1
#define superscript2Chr			chrSuperscriptTwo			// 0x00B2
#define superscript3Chr			chrSuperscriptThree			// 0x00B3
#define spacingAcuteChr			chrAcuteAccent				// 0x00B4
#define microChr				chrMicroSign				// 0x00B5
#define paragraphChr			chrPilcrowSign				// 0x00B6
#define middleDotChr			chrMiddleDot				// 0x00B7
#define spacingCedillaChr		chrCedilla					// 0x00B8
#define superscript1Chr			chrSuperscriptOne			// 0x00B9
#define masculineOrdinalChr		chrMasculineOrdinalIndicator // 0x00BA
#define rightGuillemetChr		chrRightPointingDoubleAngleQuotationMark // 0x00BB
#define fractOneQuarterChr		chrVulgarFractionOneQuarter	// 0x00BC
#define fractOneHalfChr			chrVulgarFractionOneHalf	// 0x00BD
#define fractThreeQuartersChr	chrVulgarFractionThreeQuarters // 0x00BE
#define invertedQuestionChr		chrInvertedQuestionMark		// 0x00BF
#define upAGraveChr				chrCapital_A_Grave			// 0x00C0
#define upAAcuteChr				chrCapital_A_Acute			// 0x00C1
#define upACircumflexChr		chrCapital_A_Circumflex		// 0x00C2
#define upATildeChr				chrCapital_A_Tilde			// 0x00C3
#define upADiaeresisChr			chrCapital_A_Diaeresis		// 0x00C4
#define upARingChr				chrCapital_A_RingAbove		// 0x00C5
#define upAEChr					chrCapital_AE				// 0x00C6
#define upCCedillaChr			chrCapital_C_Cedilla		// 0x00C7
#define upEGraveChr				chrCapital_E_Grave			// 0x00C8
#define upEAcuteChr				chrCapital_E_Acute			// 0x00C9
#define upECircumflexChr		chrCapital_E_Circumflex		// 0x00CA
#define upEDiaeresisChr			chrCapital_E_Diaeresis		// 0x00CB
#define upIGraveChr				chrCapital_I_Grave			// 0x00CC
#define upIAcuteChr				chrCapital_I_Acute			// 0x00CD
#define upICircumflexChr		chrCapital_I_Circumflex		// 0x00CE
#define upIDiaeresisChr			chrCapital_I_Diaeresis		// 0x00CF
#define upEthChr				chrCapital_Eth				// 0x00D0
#define upNTildeChr				chrCapital_N_Tilde			// 0x00D1
#define upOGraveChr				chrCapital_O_Grave			// 0x00D2
#define upOAcuteChr				chrCapital_O_Acute			// 0x00D3
#define upOCircumflexChr		chrCapital_O_Circumflex		// 0x00D4
#define upOTildeChr				chrCapital_O_Tilde			// 0x00D5
#define upODiaeresisChr			chrCapital_O_Diaeresis		// 0x00D6
#define multiplyChr				chrMultiplicationSign		// 0x00D7
#define upOSlashChr				chrCapital_O_Stroke			// 0x00D8
#define upUGraveChr				chrCapital_U_Grave			// 0x00D9
#define upUAcuteChr				chrCapital_U_Acute			// 0x00DA
#define upUCircumflexChr		chrCapital_U_Circumflex		// 0x00DB
#define upUDiaeresisChr			chrCapital_U_Diaeresis		// 0x00DC
#define upYAcuteChr				chrCapital_Y_Acute			// 0x00DD
#define upThorn					chrCapital_Thorn			// 0x00DE
#define lowSharpSChr			chrSmall_SharpS				// 0x00DF							
#define lowAGraveChr			chrSmall_A_Grave			// 0x00E0
#define lowAAcuteChr			chrSmall_A_Acute			// 0x00E1
#define lowACircumflexChr		chrSmall_A_Circumflex		// 0x00E2
#define lowATildeChr			chrSmall_A_Tilde			// 0x00E3
#define lowADiaeresisChr		chrSmall_A_Diaeresis		// 0x00E4
#define lowARingChr				chrSmall_A_RingAbove		// 0x00E5
#define lowAEChr				chrSmall_AE					// 0x00E6
#define lowCCedillaChr			chrSmall_C_Cedilla			// 0x00E7
#define lowEGraveChr			chrSmall_E_Grave			// 0x00E8
#define lowEAcuteChr			chrSmall_E_Acute			// 0x00E9
#define lowECircumflexChr		chrSmall_E_Circumflex		// 0x00EA
#define lowEDiaeresisChr		chrSmall_E_Diaeresis		// 0x00EB
#define lowIGraveChr			chrSmall_I_Grave			// 0x00EC
#define lowIAcuteChr			chrSmall_I_Acute			// 0x00ED
#define lowICircumflexChr		chrSmall_I_Circumflex		// 0x00EE
#define lowIDiaeresisChr		chrSmall_I_Diaeresis		// 0x00EF
#define lowEthChr				chrSmall_Eth				// 0x00F0
#define lowNTildeChr			chrSmall_N_Tilde			// 0x00F1
#define lowOGraveChr			chrSmall_O_Grave			// 0x00F2
#define lowOAcuteChr			chrSmall_O_Acute			// 0x00F3
#define lowOCircumflexChr		chrSmall_O_Circumflex		// 0x00F4
#define lowOTildeChr			chrSmall_O_Tilde			// 0x00F5
#define lowODiaeresisChr		chrSmall_O_Diaeresis		// 0x00F6
#define divideChr				chrDivisionSign				// 0x00F7
#define lowOSlashChr			chrSmall_O_Stroke			// 0x00F8
#define lowUGraveChr			chrSmall_U_Grave			// 0x00F9
#define lowUAcuteChr			chrSmall_U_Acute			// 0x00FA
#define lowUCircumflexChr		chrSmall_U_Circumflex		// 0x00FB
#define lowUDiaeresisChr		chrSmall_U_Diaeresis		// 0x00FC
#define lowYAcuteChr			chrSmall_Y_Acute			// 0x00FD
#define lowThorn				chrSmall_Thorn				// 0x00FE
#define lowYDiaeresisChr		chrSmall_Y_Diaeresis		// 0x00FF

// The  horizEllipsisChr (0x85) still exists in the font, but (in 3.1 and later roms)
// has been duplicated at location 0x18, so that it will be available with all future
// character encodings. If you are running on pre-3.1 roms, then you should use the
// chrHorizontalEllipsis character constant name (0x85), otherwise use chrEllipsis (0x18).
// The ChrHorizEllipsis macro in Chars.h can be used to determine the correct character code.

#define	horizEllipsisChr	_Obsolete__use_ChrHorizEllipsis_macro

// The following characters were moved in the four standard fonts with the
// 3.1 release of PalmOS; they still exist in their old positions in the
// font, but eventually will be removed:
//
// Old character name	Old position	New character name	New position
//
// numericSpaceChr		0x80			chrNumericSpace		0x19
// commandStrokeChr		0x9D			chrCommandStroke	0x16
// shortcutStrokeChr	0x9E			chrShortcutStroke	0x17

#define numericSpaceChrV30			0x80	// 	** COPIED TO 0x19; will be removed **
#define	commandStrokeChrV30			0x9D	//	** COPIED TO 0x16; will be removed **
#define	shortcutStrokeChrV30		0x9E	//	** COPIED TO 0x17; will be removed **

#define	numericSpaceChr		_Obsolete__use_ChrNumericSpace_macro
#define	commandStrokeChr	_Obsolete__use_commandStrokeChrV30_or_chrCommandStroke
#define	shortcutStrokeChr	_Obsolete__use_shortcutStrokeChrV30_or_chrShortcutStroke

// The following characters were removed from the four standard fonts and
// placed in the 9pt symbol font (see Chars.h).
//
// diamondChr			0x8D
// clubChr				0x8E
// heartChr				0x8F
// spadeChr				0x90

#define diamondChrV30				0x8D	// As of PalmOS v3.2, these characters are
#define clubChrV30					0x8E	// now available in the Symbol-9 font. They
#define heartChrV30					0x8F	// still appear in the regular fonts for now,
#define spadeChrV30					0x90	// but they WILL be removed in a future release.

#endif // __CHARLATIN_H__
