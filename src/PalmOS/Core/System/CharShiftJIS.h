/******************************************************************************
 *
 * Copyright (c) 1998-2003 PalmSource, Inc. All rights reserved.
 *
 * File: CharShiftJIS.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *         	Header file for Shift-JIS (code page 932) Japanese character
 *			encoding. These are based on Windows-J implementation of the
 *			Shift-JIS standard.
 *
 * Written by TransPac Software, Inc.
 *
 *****************************************************************************/

#ifndef __CHARSHIFTJIS_H__
#define __CHARSHIFTJIS_H__

/***********************************************************************
 * Public macros
 ***********************************************************************/

#define	TxtCharIsSJISHiragana(ch)	(	(TxtCharXAttr(ch) & charXClassSJISMask)	\
								==	charXClassSJISHiragana)
#define	TxtCharIsSJISKatakana(ch)	(	(TxtCharXAttr(ch) & charXClassSJISMask)	\
								==	charXClassSJISKatakana)

/***********************************************************************
 * Public constants
 ***********************************************************************/

// Transliteration operations that are not universal, but can be applied
// to Japanese text.

#define	translitOpSJISFullToHalfKatakana	(translitOpCustomBase+0)
#define	translitOpSJISHalfToFullKatakana	(translitOpCustomBase+1)
#define	translitOpSJISFullToHalfRomaji		(translitOpCustomBase+2)
#define	translitOpSJISHalfToFullRomaji		(translitOpCustomBase+3)
#define	translitOpSJISKatakanaToHiragana	(translitOpCustomBase+4)
#define	translitOpSJISHiraganaToKatakana	(translitOpCustomBase+5)
#define	translitOpSJISCombineSoundMark		(translitOpCustomBase+6)
#define	translitOpSJISDivideSoundMark		(translitOpCustomBase+7)
#define	translitOpSJISRomajiToHiragana		(translitOpCustomBase+8)
#define	translitOpSJISHiraganaToRomaji		(translitOpCustomBase+9)

// Extended character attributes for the Shift-JIS (CP932) code page.
// Note that these attributes have to be on an encoding basis, since
// they're shared across all languages which use this encoding. For
// Japanese there's only one language, so we're OK to encode wrapping
// info here, which is often language-dependent.

#define	charXAttrSJISMask			0x00ff
#define	charXAttrSJISFollowing		0x0001
#define	charXAttrSJISLeading		0x0002
#define	charXAttrSJISBreak			0x0004
#define	charXAttrSJISMicroSoft		0x0008

#define	charXClassSJISMask			0x0f00
#define	charXClassSJISRomaji		0x0100
#define	charXClassSJISHiragana		0x0200
#define	charXClassSJISKatakana		0x0300
#define	charXClassSJISKanaSound		0x0400
#define	charXClassSJISGreek			0x0500
#define	charXClassSJISCyrillic		0x0600
#define	charXClassSJISKanjiL1		0x0700
#define	charXClassSJISKanjiL2		0x0800
#define	charXClassSJISKanjiOther	0x0900
#define	charXClassSJISOther			0x0a00
#define	charXClassSJISUndefined		0x0b00

#define kSJISFirstHighByte			0x81
#define kSJISFirstLowByte			0x40

// Character codes that are specific to Shift JIS. These names
// are generated from the Unicode 2.0 data files.

#define	chrSJISYenSign							0x005c

#define	chrSJISFirstDoubleByte					0x8140

#define	chrSJISHalfwidthIdeographicFullStop 	0x00A1
#define	chrSJISHalfwidthLeftCornerBracket 		0x00A2
#define	chrSJISHalfwidthRightCornerBracket 		0x00A3
#define	chrSJISHalfwidthIdeographicComma 		0x00A4
#define	chrSJISHalfwidthKatakanaMiddleDot 		0x00A5
#define	chrSJISHalfwidthKatakana_WO				0x00A6
#define	chrSJISHalfwidthKatakanaSmall_A 		0x00A7
#define	chrSJISHalfwidthKatakanaSmall_I 		0x00A8
#define	chrSJISHalfwidthKatakanaSmall_U 		0x00A9
#define	chrSJISHalfwidthKatakanaSmall_E 		0x00AA
#define	chrSJISHalfwidthKatakanaSmall_O 		0x00AB
#define	chrSJISHalfwidthKatakanaSmall_YA 		0x00AC
#define	chrSJISHalfwidthKatakanaSmall_YU 		0x00AD
#define	chrSJISHalfwidthKatakanaSmall_YO 		0x00AE
#define	chrSJISHalfwidthKatakanaSmall_TU 		0x00AF
#define	chrSJISHalfwidthKatakanaHiraganaProlongedSoundMark	0x00B0
#define	chrSJISHalfwidthKatakana_A				0x00B1
#define	chrSJISHalfwidthKatakana_I				0x00B2
#define	chrSJISHalfwidthKatakana_U				0x00B3
#define	chrSJISHalfwidthKatakana_E				0x00B4
#define	chrSJISHalfwidthKatakana_O				0x00B5
#define	chrSJISHalfwidthKatakana_KA				0x00B6
#define	chrSJISHalfwidthKatakana_KI				0x00B7
#define	chrSJISHalfwidthKatakana_KU				0x00B8
#define	chrSJISHalfwidthKatakana_KE				0x00B9
#define	chrSJISHalfwidthKatakana_KO				0x00BA
#define	chrSJISHalfwidthKatakana_SA				0x00BB
#define	chrSJISHalfwidthKatakana_SI				0x00BC
#define	chrSJISHalfwidthKatakana_SU				0x00BD
#define	chrSJISHalfwidthKatakana_SE				0x00BE
#define	chrSJISHalfwidthKatakana_SO				0x00BF
#define	chrSJISHalfwidthKatakana_TA				0x00C0
#define	chrSJISHalfwidthKatakana_TI				0x00C1
#define	chrSJISHalfwidthKatakana_TU				0x00C2
#define	chrSJISHalfwidthKatakana_TE				0x00C3
#define	chrSJISHalfwidthKatakana_TO				0x00C4
#define	chrSJISHalfwidthKatakana_NA				0x00C5
#define	chrSJISHalfwidthKatakana_NI				0x00C6
#define	chrSJISHalfwidthKatakana_NU				0x00C7
#define	chrSJISHalfwidthKatakana_NE				0x00C8
#define	chrSJISHalfwidthKatakana_NO				0x00C9
#define	chrSJISHalfwidthKatakana_HA				0x00CA
#define	chrSJISHalfwidthKatakana_HI				0x00CB
#define	chrSJISHalfwidthKatakana_HU				0x00CC
#define	chrSJISHalfwidthKatakana_HE				0x00CD
#define	chrSJISHalfwidthKatakana_HO				0x00CE
#define	chrSJISHalfwidthKatakana_MA				0x00CF
#define	chrSJISHalfwidthKatakana_MI				0x00D0
#define	chrSJISHalfwidthKatakana_MU				0x00D1
#define	chrSJISHalfwidthKatakana_ME				0x00D2
#define	chrSJISHalfwidthKatakana_MO				0x00D3
#define	chrSJISHalfwidthKatakana_YA				0x00D4
#define	chrSJISHalfwidthKatakana_YU				0x00D5
#define	chrSJISHalfwidthKatakana_YO				0x00D6
#define	chrSJISHalfwidthKatakana_RA				0x00D7
#define	chrSJISHalfwidthKatakana_RI				0x00D8
#define	chrSJISHalfwidthKatakana_RU				0x00D9
#define	chrSJISHalfwidthKatakana_RE				0x00DA
#define	chrSJISHalfwidthKatakana_RO				0x00DB
#define	chrSJISHalfwidthKatakana_WA				0x00DC
#define	chrSJISHalfwidthKatakana_N				0x00DD
#define	chrSJISHalfwidthKatakanaVoicedSoundMark 	0x00DE
#define	chrSJISHalfwidthKatakanaSemiVoicedSoundMark 0x00DF

#define	chrSJISIdeographicSpace					0x8140
#define	chrSJISIdeographicComma					0x8141
#define	chrSJISIdeographicFullStop				0x8142
#define	chrSJISFullwidthComma						0x8143
#define	chrSJISFullwidthFullStop					0x8144
#define	chrSJISKatakanaMiddleDot					0x8145
#define	chrSJISFullwidthColon						0x8146
#define	chrSJISFullwidthSemicolon				0x8147
#define	chrSJISFullwidthQuestionMark 			0x8148
#define	chrSJISFullwidthExclamationMark 		0x8149
#define	chrSJISKatakanaHiraganaVoicedSoundMark	0x814A
#define	chrSJISKatakanaHiraganaSemiVoicedSoundMark	0x814B
#define	chrSJISAcuteAccent							0x814C
#define	chrSJISFullwidthGraveAccent				0x814D
#define	chrSJISDiaeresis							0x814E
#define	chrSJISFullwidthCircumflexAccent 		0x814F
#define	chrSJISFullwidthMacron					0x8150
#define	chrSJISFullwidthLowLine					0x8151
#define	chrSJISKatakanaIterationMark 			0x8152
#define	chrSJISKatakanaVoicedIterationMark 	0x8153
#define	chrSJISHiraganaIterationMark 			0x8154
#define	chrSJISHiraganaVoicedIterationMark 	0x8155
#define	chrSJISDittoMark							0x8156
#define	chrSJISIdeographicIterationMark 		0x8158
#define	chrSJISIdeographicClosingMark 			0x8159
#define	chrSJISIdeographicNumberZero 			0x815A
#define	chrSJISKatakanaHiraganaProlongedSoundMark 0x815B
#define	chrSJISHorizontalBar						0x815C
#define	chrSJISHyphen								0x815D
#define	chrSJISFullwidthSolidus					0x815E
#define	chrSJISFullwidthReverseSolidus 		0x815F
#define	chrSJISFullwidthTilde						0x8160
#define	chrSJISParallelTo							0x8161
#define	chrSJISFullwidthVerticalLine 			0x8162
#define	chrSJISFullwidthHorizontalEllipsis	0x8163
#define	chrSJISTwoDotLeader						0x8164
#define	chrSJISLeftSingleQuotationMark 		0x8165
#define	chrSJISRightSingleQuotationMark 		0x8166
#define	chrSJISLeftDoubleQuotationMark 		0x8167
#define	chrSJISRightDoubleQuotationMark 		0x8168
#define	chrSJISFullwidthLeftParenthesis 		0x8169
#define	chrSJISFullwidthRightParenthesis 		0x816A
#define	chrSJISLeftTortoiseShellBracket 		0x816B
#define	chrSJISRightTortoiseShellBracket 		0x816C
#define	chrSJISFullwidthLeftSquareBracket 	0x816D
#define	chrSJISFullwidthRightSquareBracket 	0x816E
#define	chrSJISFullwidthLeftCurlyBracket 		0x816F
#define	chrSJISFullwidthRightCurlyBracket 	0x8170
#define	chrSJISLeftAngleBracket					0x8171
#define	chrSJISRightAngleBracket					0x8172
#define	chrSJISLeftDoubleAngleBracket 			0x8173
#define	chrSJISRightDoubleAngleBracket 		0x8174
#define	chrSJISLeftCornerBracket					0x8175
#define	chrSJISRightCornerBracket				0x8176
#define	chrSJISLeftWhiteCornerBracket 			0x8177
#define	chrSJISRightWhiteCornerBracket 		0x8178
#define	chrSJISLeftBlackLenticularBracket 	0x8179
#define	chrSJISRightBlackLenticularBracket 	0x817A
#define	chrSJISFullwidthPlusSign					0x817B
#define	chrSJISFullwidthHyphenMinus				0x817C
#define	chrSJISPlusMinusSign						0x817D
#define	chrSJISMultiplicationSign				0x817E
#define	chrSJISDivisionSign						0x8180
#define	chrSJISFullwidthEqualsSign				0x8181
#define	chrSJISNotEqualTo							0x8182
#define	chrSJISFullwidthLessThanSign 			0x8183
#define	chrSJISFullwidthGreaterThanSign 		0x8184
#define	chrSJISLessThanOverEqualTo				0x8185
#define	chrSJISGreaterThanOverEqualTo 			0x8186
#define	chrSJISInfinity								0x8187
#define	chrSJISTherefore							0x8188
#define	chrSJISMaleSign								0x8189
#define	chrSJISFemaleSign							0x818A
#define	chrSJISDegreeSign							0x818B
#define	chrSJISPrime									0x818C
#define	chrSJISDoublePrime							0x818D
#define	chrSJISDegreeCelsius						0x818E
#define	chrSJISFullwidthYenSign					0x818F
#define	chrSJISFullwidthDollarSign				0x8190
#define	chrSJISFullwidthCentSign					0x8191
#define	chrSJISFullwidthPoundSign				0x8192
#define	chrSJISFullwidthPercentSign				0x8193
#define	chrSJISFullwidthNumberSign				0x8194
#define	chrSJISFullwidthAmpersand				0x8195
#define	chrSJISFullwidthAsterisk					0x8196
#define	chrSJISFullwidthCommercialAt 			0x8197
#define	chrSJISSectionSign							0x8198
#define	chrSJISWhiteStar							0x8199
#define	chrSJISBlackStar							0x819A
#define	chrSJISWhiteCircle							0x819B
#define	chrSJISBlackCircle							0x819C
#define	chrSJISBullseye								0x819D
#define	chrSJISWhiteDiamond						0x819E
#define	chrSJISBlackDiamond						0x819F
#define	chrSJISWhiteSquare							0x81A0
#define	chrSJISBlackSquare							0x81A1
#define	chrSJISWhiteUpPointingTriangle 		0x81A2
#define	chrSJISBlackUpPointingTriangle 		0x81A3
#define	chrSJISWhiteDownPointingTriangle 		0x81A4
#define	chrSJISBlackDownPointingTriangle 		0x81A5
#define	chrSJISReferenceMark						0x81A6
#define	chrSJISPostalMark							0x81A7
#define	chrSJISRightwardsArrow					0x81A8
#define	chrSJISLeftwardsArrow						0x81A9
#define	chrSJISUpwardsArrow						0x81AA
#define	chrSJISDownwardsArrow						0x81AB
#define	chrSJISGetaMark								0x81AC
#define	chrSJISElementOf							0x81B8
#define	chrSJISContainsAsMember					0x81B9
#define	chrSJISSubsetOfOrEqualTo					0x81BA
#define	chrSJISSupersetOfOrEqualTo				0x81BB
#define	chrSJISSubsetOf								0x81BC
#define	chrSJISSupersetOf							0x81BD
#define	chrSJISUnion									0x81BE
#define	chrSJISIntersection						0x81BF
#define	chrSJISLogicalAnd							0x81C8
#define	chrSJISLogicalOr							0x81C9
#define	chrSJISFullwidthNotSign					0x81CA
#define	chrSJISRightwardsDoubleArrow 			0x81CB
#define	chrSJISLeftRightDoubleArrow				0x81CC
#define	chrSJISForAll								0x81CD
#define	chrSJISThereExists							0x81CE
#define	chrSJISAngle									0x81DA
#define	chrSJISUpTack								0x81DB
#define	chrSJISArc									0x81DC
#define	chrSJISPartialDifferential				0x81DD
#define	chrSJISNabla									0x81DE
#define	chrSJISIdenticalTo							0x81DF
#define	chrSJISApproximatelyEqualToOrTheImageOf 0x81E0
#define	chrSJISMuchLessThan						0x81E1
#define	chrSJISMuchGreaterThan					0x81E2
#define	chrSJISSquareRoot							0x81E3
#define	chrSJISReversedTilde						0x81E4
#define	chrSJISProportionalTo						0x81E5
#define	chrSJISBecause								0x81E6
#define	chrSJISIntegral								0x81E7
#define	chrSJISDoubleIntegral						0x81E8
#define	chrSJISAngstromSign						0x81F0
#define	chrSJISPerMilleSign						0x81F1
#define	chrSJISMusicSharpSign						0x81F2
#define	chrSJISMusicFlatSign						0x81F3
#define	chrSJISEighthNote							0x81F4
#define	chrSJISDagger								0x81F5
#define	chrSJISDoubleDagger						0x81F6
#define	chrSJISPilcrowSign							0x81F7
#define	chrSJISLargeCircle							0x81FC

#define	chrSJISFullwidthDigitZero				0x824F
#define	chrSJISFullwidthDigitOne					0x8250
#define	chrSJISFullwidthDigitTwo					0x8251
#define	chrSJISFullwidthDigitThree				0x8252
#define	chrSJISFullwidthDigitFour				0x8253
#define	chrSJISFullwidthDigitFive				0x8254
#define	chrSJISFullwidthDigitSix					0x8255
#define	chrSJISFullwidthDigitSeven				0x8256
#define	chrSJISFullwidthDigitEight				0x8257
#define	chrSJISFullwidthDigitNine				0x8258
#define	chrSJISFullwidthCapital_A				0x8260
#define	chrSJISFullwidthCapital_B				0x8261
#define	chrSJISFullwidthCapital_C				0x8262
#define	chrSJISFullwidthCapital_D				0x8263
#define	chrSJISFullwidthCapital_E				0x8264
#define	chrSJISFullwidthCapital_F				0x8265
#define	chrSJISFullwidthCapital_G				0x8266
#define	chrSJISFullwidthCapital_H				0x8267
#define	chrSJISFullwidthCapital_I				0x8268
#define	chrSJISFullwidthCapital_J				0x8269
#define	chrSJISFullwidthCapital_K				0x826A
#define	chrSJISFullwidthCapital_L				0x826B
#define	chrSJISFullwidthCapital_M				0x826C
#define	chrSJISFullwidthCapital_N				0x826D
#define	chrSJISFullwidthCapital_O				0x826E
#define	chrSJISFullwidthCapital_P				0x826F
#define	chrSJISFullwidthCapital_Q				0x8270
#define	chrSJISFullwidthCapital_R				0x8271
#define	chrSJISFullwidthCapital_S				0x8272
#define	chrSJISFullwidthCapital_T				0x8273
#define	chrSJISFullwidthCapital_U				0x8274
#define	chrSJISFullwidthCapital_V				0x8275
#define	chrSJISFullwidthCapital_W				0x8276
#define	chrSJISFullwidthCapital_X				0x8277
#define	chrSJISFullwidthCapital_Y				0x8278
#define	chrSJISFullwidthCapital_Z				0x8279
#define	chrSJISFullwidthSmall_A					0x8281
#define	chrSJISFullwidthSmall_B					0x8282
#define	chrSJISFullwidthSmall_C					0x8283
#define	chrSJISFullwidthSmall_D					0x8284
#define	chrSJISFullwidthSmall_E					0x8285
#define	chrSJISFullwidthSmall_F					0x8286
#define	chrSJISFullwidthSmall_G					0x8287
#define	chrSJISFullwidthSmall_H					0x8288
#define	chrSJISFullwidthSmall_I					0x8289
#define	chrSJISFullwidthSmall_J					0x828A
#define	chrSJISFullwidthSmall_K					0x828B
#define	chrSJISFullwidthSmall_L					0x828C
#define	chrSJISFullwidthSmall_M					0x828D
#define	chrSJISFullwidthSmall_N					0x828E
#define	chrSJISFullwidthSmall_O					0x828F
#define	chrSJISFullwidthSmall_P					0x8290
#define	chrSJISFullwidthSmall_Q					0x8291
#define	chrSJISFullwidthSmall_R					0x8292
#define	chrSJISFullwidthSmall_S					0x8293
#define	chrSJISFullwidthSmall_T					0x8294
#define	chrSJISFullwidthSmall_U					0x8295
#define	chrSJISFullwidthSmall_V					0x8296
#define	chrSJISFullwidthSmall_W					0x8297
#define	chrSJISFullwidthSmall_X					0x8298
#define	chrSJISFullwidthSmall_Y					0x8299
#define	chrSJISFullwidthSmall_Z					0x829A

#define	chrSJISHiraganaSmall_A					0x829F
#define	chrSJISHiragana_A							0x82A0
#define	chrSJISHiraganaSmall_I					0x82A1
#define	chrSJISHiragana_I							0x82A2
#define	chrSJISHiraganaSmall_U					0x82A3
#define	chrSJISHiragana_U							0x82A4
#define	chrSJISHiraganaSmall_E					0x82A5
#define	chrSJISHiragana_E							0x82A6
#define	chrSJISHiraganaSmall_O					0x82A7
#define	chrSJISHiragana_O							0x82A8
#define	chrSJISHiragana_KA							0x82A9
#define	chrSJISHiragana_GA							0x82AA
#define	chrSJISHiragana_KI							0x82AB
#define	chrSJISHiragana_GI							0x82AC
#define	chrSJISHiragana_KU							0x82AD
#define	chrSJISHiragana_GU							0x82AE
#define	chrSJISHiragana_KE							0x82AF
#define	chrSJISHiragana_GE							0x82B0
#define	chrSJISHiragana_KO							0x82B1
#define	chrSJISHiragana_GO							0x82B2
#define	chrSJISHiragana_SA							0x82B3
#define	chrSJISHiragana_ZA							0x82B4
#define	chrSJISHiragana_SI							0x82B5
#define	chrSJISHiragana_ZI							0x82B6
#define	chrSJISHiragana_SU							0x82B7
#define	chrSJISHiragana_ZU							0x82B8
#define	chrSJISHiragana_SE							0x82B9
#define	chrSJISHiragana_ZE							0x82BA
#define	chrSJISHiragana_SO							0x82BB
#define	chrSJISHiragana_ZO							0x82BC
#define	chrSJISHiragana_TA							0x82BD
#define	chrSJISHiragana_DA							0x82BE
#define	chrSJISHiragana_TI							0x82BF
#define	chrSJISHiragana_DI							0x82C0
#define	chrSJISHiraganaSmall_TU					0x82C1
#define	chrSJISHiragana_TU							0x82C2
#define	chrSJISHiragana_DU							0x82C3
#define	chrSJISHiragana_TE							0x82C4
#define	chrSJISHiragana_DE							0x82C5
#define	chrSJISHiragana_TO							0x82C6
#define	chrSJISHiragana_DO							0x82C7
#define	chrSJISHiragana_NA							0x82C8
#define	chrSJISHiragana_NI							0x82C9
#define	chrSJISHiragana_NU							0x82CA
#define	chrSJISHiragana_NE							0x82CB
#define	chrSJISHiragana_NO							0x82CC
#define	chrSJISHiragana_HA							0x82CD
#define	chrSJISHiragana_BA							0x82CE
#define	chrSJISHiragana_PA							0x82CF
#define	chrSJISHiragana_HI							0x82D0
#define	chrSJISHiragana_BI							0x82D1
#define	chrSJISHiragana_PI							0x82D2
#define	chrSJISHiragana_HU							0x82D3
#define	chrSJISHiragana_BU							0x82D4
#define	chrSJISHiragana_PU							0x82D5
#define	chrSJISHiragana_HE							0x82D6
#define	chrSJISHiragana_BE							0x82D7
#define	chrSJISHiragana_PE							0x82D8
#define	chrSJISHiragana_HO							0x82D9
#define	chrSJISHiragana_BO							0x82DA
#define	chrSJISHiragana_PO							0x82DB
#define	chrSJISHiragana_MA							0x82DC
#define	chrSJISHiragana_MI							0x82DD
#define	chrSJISHiragana_MU							0x82DE
#define	chrSJISHiragana_ME							0x82DF
#define	chrSJISHiragana_MO							0x82E0
#define	chrSJISHiraganaSmall_YA					0x82E1
#define	chrSJISHiragana_YA							0x82E2
#define	chrSJISHiraganaSmall_YU					0x82E3
#define	chrSJISHiragana_YU							0x82E4
#define	chrSJISHiraganaSmall_YO					0x82E5
#define	chrSJISHiragana_YO							0x82E6
#define	chrSJISHiragana_RA							0x82E7
#define	chrSJISHiragana_RI							0x82E8
#define	chrSJISHiragana_RU							0x82E9
#define	chrSJISHiragana_RE							0x82EA
#define	chrSJISHiragana_RO							0x82EB
#define	chrSJISHiraganaSmall_WA					0x82EC
#define	chrSJISHiragana_WA							0x82ED
#define	chrSJISHiragana_WI							0x82EE
#define	chrSJISHiragana_WE							0x82EF
#define	chrSJISHiragana_WO							0x82F0
#define	chrSJISHiragana_N							0x82F1

#define	chrSJISKatakanaSmall_A					0x8340
#define	chrSJISKatakana_A							0x8341
#define	chrSJISKatakanaSmall_I					0x8342
#define	chrSJISKatakana_I							0x8343
#define	chrSJISKatakanaSmall_U					0x8344
#define	chrSJISKatakana_U							0x8345
#define	chrSJISKatakanaSmall_E					0x8346
#define	chrSJISKatakana_E							0x8347
#define	chrSJISKatakanaSmall_O					0x8348
#define	chrSJISKatakana_O							0x8349
#define	chrSJISKatakana_KA							0x834A
#define	chrSJISKatakana_GA							0x834B
#define	chrSJISKatakana_KI							0x834C
#define	chrSJISKatakana_GI							0x834D
#define	chrSJISKatakana_KU							0x834E
#define	chrSJISKatakana_GU							0x834F
#define	chrSJISKatakana_KE							0x8350
#define	chrSJISKatakana_GE							0x8351
#define	chrSJISKatakana_KO							0x8352
#define	chrSJISKatakana_GO							0x8353
#define	chrSJISKatakana_SA							0x8354
#define	chrSJISKatakana_ZA							0x8355
#define	chrSJISKatakana_SI							0x8356
#define	chrSJISKatakana_ZI							0x8357
#define	chrSJISKatakana_SU							0x8358
#define	chrSJISKatakana_ZU							0x8359
#define	chrSJISKatakana_SE							0x835A
#define	chrSJISKatakana_ZE							0x835B
#define	chrSJISKatakana_SO							0x835C
#define	chrSJISKatakana_ZO							0x835D
#define	chrSJISKatakana_TA							0x835E
#define	chrSJISKatakana_DA							0x835F
#define	chrSJISKatakana_TI							0x8360
#define	chrSJISKatakana_DI							0x8361
#define	chrSJISKatakanaSmall_TU					0x8362
#define	chrSJISKatakana_TU							0x8363
#define	chrSJISKatakana_DU							0x8364
#define	chrSJISKatakana_TE							0x8365
#define	chrSJISKatakana_DE							0x8366
#define	chrSJISKatakana_TO							0x8367
#define	chrSJISKatakana_DO							0x8368
#define	chrSJISKatakana_NA							0x8369
#define	chrSJISKatakana_NI							0x836A
#define	chrSJISKatakana_NU							0x836B
#define	chrSJISKatakana_NE							0x836C
#define	chrSJISKatakana_NO							0x836D
#define	chrSJISKatakana_HA							0x836E
#define	chrSJISKatakana_BA							0x836F
#define	chrSJISKatakana_PA							0x8370
#define	chrSJISKatakana_HI							0x8371
#define	chrSJISKatakana_BI							0x8372
#define	chrSJISKatakana_PI							0x8373
#define	chrSJISKatakana_HU							0x8374
#define	chrSJISKatakana_BU							0x8375
#define	chrSJISKatakana_PU							0x8376
#define	chrSJISKatakana_HE							0x8377
#define	chrSJISKatakana_BE							0x8378
#define	chrSJISKatakana_PE							0x8379
#define	chrSJISKatakana_HO							0x837A
#define	chrSJISKatakana_BO							0x837B
#define	chrSJISKatakana_PO							0x837C
#define	chrSJISKatakana_MA							0x837D
#define	chrSJISKatakana_MI							0x837E
#define	chrSJISKatakana_MU							0x8380
#define	chrSJISKatakana_ME							0x8381
#define	chrSJISKatakana_MO							0x8382
#define	chrSJISKatakanaSmall_YA					0x8383
#define	chrSJISKatakana_YA							0x8384
#define	chrSJISKatakanaSmall_YU					0x8385
#define	chrSJISKatakana_YU							0x8386
#define	chrSJISKatakanaSmall_YO					0x8387
#define	chrSJISKatakana_YO							0x8388
#define	chrSJISKatakana_RA							0x8389
#define	chrSJISKatakana_RI							0x838A
#define	chrSJISKatakana_RU							0x838B
#define	chrSJISKatakana_RE							0x838C
#define	chrSJISKatakana_RO							0x838D
#define	chrSJISKatakanaSmall_WA					0x838E
#define	chrSJISKatakana_WA							0x838F
#define	chrSJISKatakana_WI							0x8390
#define	chrSJISKatakana_WE							0x8391
#define	chrSJISKatakana_WO							0x8392
#define	chrSJISKatakana_N							0x8393
#define	chrSJISKatakana_VU							0x8394
#define	chrSJISKatakanaSmall_KA					0x8395
#define	chrSJISKatakanaSmall_KE					0x8396

#define	chrSJISGreekCapitalAlpha					0x839F
#define	chrSJISGreekCapitalBeta					0x83A0
#define	chrSJISGreekCapitalGamma					0x83A1
#define	chrSJISGreekCapitalDelta					0x83A2
#define	chrSJISGreekCapitalEpsilon				0x83A3
#define	chrSJISGreekCapitalZeta					0x83A4
#define	chrSJISGreekCapitalEta					0x83A5
#define	chrSJISGreekCapitalTheta					0x83A6
#define	chrSJISGreekCapitalIota					0x83A7
#define	chrSJISGreekCapitalKappa					0x83A8
#define	chrSJISGreekCapitalLamda					0x83A9
#define	chrSJISGreekCapitalMu						0x83AA
#define	chrSJISGreekCapitalNu						0x83AB
#define	chrSJISGreekCapitalXi						0x83AC
#define	chrSJISGreekCapitalOmicron				0x83AD
#define	chrSJISGreekCapitalPi						0x83AE
#define	chrSJISGreekCapitalRho					0x83AF
#define	chrSJISGreekCapitalSigma					0x83B0
#define	chrSJISGreekCapitalTau					0x83B1
#define	chrSJISGreekCapitalUpsilon				0x83B2
#define	chrSJISGreekCapitalPhi					0x83B3
#define	chrSJISGreekCapitalChi					0x83B4
#define	chrSJISGreekCapitalPsi					0x83B5
#define	chrSJISGreekCapitalOmega					0x83B6
#define	chrSJISGreekSmallAlpha					0x83BF
#define	chrSJISGreekSmallBeta						0x83C0
#define	chrSJISGreekSmallGamma					0x83C1
#define	chrSJISGreekSmallDelta					0x83C2
#define	chrSJISGreekSmallEpsilon					0x83C3
#define	chrSJISGreekSmallZeta						0x83C4
#define	chrSJISGreekSmallEta						0x83C5
#define	chrSJISGreekSmallTheta					0x83C6
#define	chrSJISGreekSmallIota						0x83C7
#define	chrSJISGreekSmallKappa					0x83C8
#define	chrSJISGreekSmallLamda					0x83C9
#define	chrSJISGreekSmallMu						0x83CA
#define	chrSJISGreekSmallNu						0x83CB
#define	chrSJISGreekSmallXi						0x83CC
#define	chrSJISGreekSmallOmicron					0x83CD
#define	chrSJISGreekSmallPi						0x83CE
#define	chrSJISGreekSmallRho						0x83CF
#define	chrSJISGreekSmallSigma					0x83D0
#define	chrSJISGreekSmallTau						0x83D1
#define	chrSJISGreekSmallUpsilon					0x83D2
#define	chrSJISGreekSmallPhi						0x83D3
#define	chrSJISGreekSmallChi						0x83D4
#define	chrSJISGreekSmallPsi						0x83D5
#define	chrSJISGreekSmallOmega					0x83D6

#define	chrSJISCyrillicCapital_A					0x8440
#define	chrSJISCyrillicCapital_BE				0x8441
#define	chrSJISCyrillicCapital_VE				0x8442
#define	chrSJISCyrillicCapital_GHE				0x8443
#define	chrSJISCyrillicCapital_DE				0x8444
#define	chrSJISCyrillicCapital_IE				0x8445
#define	chrSJISCyrillicCapital_IO				0x8446
#define	chrSJISCyrillicCapital_ZHE				0x8447
#define	chrSJISCyrillicCapital_ZE				0x8448
#define	chrSJISCyrillicCapital_I					0x8449
#define	chrSJISCyrillicCapitalShort_I 			0x844A
#define	chrSJISCyrillicCapital_KA				0x844B
#define	chrSJISCyrillicCapital_EL				0x844C
#define	chrSJISCyrillicCapital_EM				0x844D
#define	chrSJISCyrillicCapital_EN				0x844E
#define	chrSJISCyrillicCapital_O					0x844F
#define	chrSJISCyrillicCapital_PE				0x8450
#define	chrSJISCyrillicCapital_ER				0x8451
#define	chrSJISCyrillicCapital_ES				0x8452
#define	chrSJISCyrillicCapital_TE				0x8453
#define	chrSJISCyrillicCapital_U					0x8454
#define	chrSJISCyrillicCapital_EF				0x8455
#define	chrSJISCyrillicCapital_HA				0x8456
#define	chrSJISCyrillicCapital_TSE				0x8457
#define	chrSJISCyrillicCapital_CHE				0x8458
#define	chrSJISCyrillicCapital_SHA				0x8459
#define	chrSJISCyrillicCapital_SHCHA 			0x845A
#define	chrSJISCyrillicCapitalHardSign 		0x845B
#define	chrSJISCyrillicCapital_YERU				0x845C
#define	chrSJISCyrillicCapitalSoftSign 		0x845D
#define	chrSJISCyrillicCapital_E					0x845E
#define	chrSJISCyrillicCapital_YU				0x845F
#define	chrSJISCyrillicCapital_YA				0x8460
#define	chrSJISCyrillicSmall_A					0x8470
#define	chrSJISCyrillicSmall_BE					0x8471
#define	chrSJISCyrillicSmall_VE					0x8472
#define	chrSJISCyrillicSmall_GHE					0x8473
#define	chrSJISCyrillicSmall_DE					0x8474
#define	chrSJISCyrillicSmall_IE					0x8475
#define	chrSJISCyrillicSmall_IO					0x8476
#define	chrSJISCyrillicSmall_ZHE					0x8477
#define	chrSJISCyrillicSmall_ZE					0x8478
#define	chrSJISCyrillicSmall_I					0x8479
#define	chrSJISCyrillicSmallShort_I				0x847A
#define	chrSJISCyrillicSmall_KA					0x847B
#define	chrSJISCyrillicSmall_EL					0x847C
#define	chrSJISCyrillicSmall_EM					0x847D
#define	chrSJISCyrillicSmall_EN					0x847E
#define	chrSJISCyrillicSmall_O					0x8480
#define	chrSJISCyrillicSmall_PE					0x8481
#define	chrSJISCyrillicSmall_ER					0x8482
#define	chrSJISCyrillicSmall_ES					0x8483
#define	chrSJISCyrillicSmall_TE					0x8484
#define	chrSJISCyrillicSmall_U					0x8485
#define	chrSJISCyrillicSmall_EF					0x8486
#define	chrSJISCyrillicSmall_HA					0x8487
#define	chrSJISCyrillicSmall_TSE					0x8488
#define	chrSJISCyrillicSmall_CHE					0x8489
#define	chrSJISCyrillicSmall_SHA					0x848A
#define	chrSJISCyrillicSmall_SHCHA				0x848B
#define	chrSJISCyrillicSmallHardSign 			0x848C
#define	chrSJISCyrillicSmall_YERU				0x848D
#define	chrSJISCyrillicSmallSoftSign 			0x848E
#define	chrSJISCyrillicSmall_E					0x848F
#define	chrSJISCyrillicSmall_YU					0x8490
#define	chrSJISCyrillicSmall_YA					0x8491

#define	chrSJISBoxDrawingsLightHorizontal 					0x849F
#define	chrSJISBoxDrawingsLightVertical 						0x84A0
#define	chrSJISBoxDrawingsLightDownAndRight 					0x84A1
#define	chrSJISBoxDrawingsLightDownAndLeft 					0x84A2
#define	chrSJISBoxDrawingsLightUpAndLeft 						0x84A3
#define	chrSJISBoxDrawingsLightUpAndRight 					0x84A4
#define	chrSJISBoxDrawingsLightVerticalAndRight 			0x84A5
#define	chrSJISBoxDrawingsLightDownAndHorizontal 			0x84A6
#define	chrSJISBoxDrawingsLightVerticalAndLeft 				0x84A7
#define	chrSJISBoxDrawingsLightUpAndHorizontal 				0x84A8
#define	chrSJISBoxDrawingsLightVerticalAndHorizontal 		0x84A9
#define	chrSJISBoxDrawingsHeavyHorizontal 					0x84AA
#define	chrSJISBoxDrawingsHeavyVertical 						0x84AB
#define	chrSJISBoxDrawingsHeavyDownAndRight 					0x84AC
#define	chrSJISBoxDrawingsHeavyDownAndLeft 					0x84AD
#define	chrSJISBoxDrawingsHeavyUpAndLeft 						0x84AE
#define	chrSJISBoxDrawingsHeavyUpAndRight 					0x84AF
#define	chrSJISBoxDrawingsHeavyVerticalAndRight 			0x84B0
#define	chrSJISBoxDrawingsHeavyDownAndHorizontal 			0x84B1
#define	chrSJISBoxDrawingsHeavyVerticalAndLeft 				0x84B2
#define	chrSJISBoxDrawingsHeavyUpAndHorizontal 				0x84B3
#define	chrSJISBoxDrawingsHeavyVerticalAndHorizontal 		0x84B4
#define	chrSJISBoxDrawingsVerticalHeavyAndRightLight 		0x84B5
#define	chrSJISBoxDrawingsDownLightAndHorizontalHeavy 	0x84B6
#define	chrSJISBoxDrawingsVerticalHeavyAndLeftLight 		0x84B7
#define	chrSJISBoxDrawingsUpLightAndHorizontalHeavy 		0x84B8
#define	chrSJISBoxDrawingsVerticalLightAndHorizontalHeavy 	0x84B9
#define	chrSJISBoxDrawingsVerticalLightAndRightHeavy 		0x84BA
#define	chrSJISBoxDrawingsDownHeavyAndHorizontalLight 	0x84BB
#define	chrSJISBoxDrawingsVerticalLightAndLeftHeavy 		0x84BC
#define	chrSJISBoxDrawingsUpHeavyAndHorizontalLight 		0x84BD
#define	chrSJISBoxDrawingsVerticalHeavyAndHorizontalLight 	0x84BE

#define	chrSJISCircledDigitOne					0x8740
#define	chrSJISCircledDigitTwo					0x8741
#define	chrSJISCircledDigitThree					0x8742
#define	chrSJISCircledDigitFour					0x8743
#define	chrSJISCircledDigitFive					0x8744
#define	chrSJISCircledDigitSix					0x8745
#define	chrSJISCircledDigitSeven					0x8746
#define	chrSJISCircledDigitEight					0x8747
#define	chrSJISCircledDigitNine					0x8748
#define	chrSJISCircledNumberTen					0x8749
#define	chrSJISCircledNumberEleven				0x874A
#define	chrSJISCircledNumberTwelve				0x874B
#define	chrSJISCircledNumberThirteen 			0x874C
#define	chrSJISCircledNumberFourteen 			0x874D
#define	chrSJISCircledNumberFifteen				0x874E
#define	chrSJISCircledNumberSixteen				0x874F
#define	chrSJISCircledNumberSeventeen 			0x8750
#define	chrSJISCircledNumberEighteen 			0x8751
#define	chrSJISCircledNumberNineteen 			0x8752
#define	chrSJISCircledNumberTwenty				0x8753
#define	chrSJISRomanNumeralOne					0x8754
#define	chrSJISRomanNumeralTwo					0x8755
#define	chrSJISRomanNumeralThree					0x8756
#define	chrSJISRomanNumeralFour					0x8757
#define	chrSJISRomanNumeralFive					0x8758
#define	chrSJISRomanNumeralSix					0x8759
#define	chrSJISRomanNumeralSeven					0x875A
#define	chrSJISRomanNumeralEight					0x875B
#define	chrSJISRomanNumeralNine					0x875C
#define	chrSJISRomanNumeralTen					0x875D
#define	chrSJISSquareMiri							0x875F
#define	chrSJISSquareKiro							0x8760
#define	chrSJISSquareSenti							0x8761
#define	chrSJISSquareMeetoru						0x8762
#define	chrSJISSquareGuramu						0x8763
#define	chrSJISSquareTon							0x8764
#define	chrSJISSquareAaru							0x8765
#define	chrSJISSquareHekutaaru					0x8766
#define	chrSJISSquareRittoru						0x8767
#define	chrSJISSquareWatto							0x8768
#define	chrSJISSquareKarorii						0x8769
#define	chrSJISSquareDoru							0x876A
#define	chrSJISSquareSento							0x876B
#define	chrSJISSquarePaasento						0x876C
#define	chrSJISSquareMiribaaru					0x876D
#define	chrSJISSquarePeezi							0x876E
#define	chrSJISSquareMm								0x876F
#define	chrSJISSquareCm								0x8770
#define	chrSJISSquareKm								0x8771
#define	chrSJISSquareMg								0x8772
#define	chrSJISSquareKg								0x8773
#define	chrSJISSquareCc								0x8774
#define	chrSJISSquareMSquared						0x8775
#define	chrSJISSquareEraNameHeisei				0x877E
#define	chrSJISReversedDoublePrimeQuotationMark 0x8780
#define	chrSJISLowDoublePrimeQuotationMark 		0x8781
#define	chrSJISNumeroSign						0x8782
#define	chrSJISSquareKk							0x8783
#define	chrSJISTelephoneSign					0x8784
#define	chrSJISCircledIdeographHigh				0x8785
#define	chrSJISCircledIdeographCentre 			0x8786
#define	chrSJISCircledIdeographLow				0x8787
#define	chrSJISCircledIdeographLeft				0x8788
#define	chrSJISCircledIdeographRight 			0x8789
#define	chrSJISParenthesizedIdeographStock 		0x878A
#define	chrSJISParenthesizedIdeographHave 		0x878B
#define	chrSJISParenthesizedIdeographRepresent 	0x878C
#define	chrSJISSquareEraNameMeizi				0x878D
#define	chrSJISSquareEraNameTaisyou				0x878E
#define	chrSJISSquareEraNameSyouwa				0x878F
#define	chrSJISApproximatelyEqualToOrTheImageOfDup	0x8790		// Same as 0x81E0
#define	chrSJISIdenticalToDup					0x8791				// Same as 0x81DF
#define	chrSJISIntegralDup						0x8792				// Same as 0x81E7
#define	chrSJISContourIntegral					0x8793
#define	chrSJISNArySummation					0x8794
#define	chrSJISSquareRootDup					0x8795				// Same as 0x81E3
#define	chrSJISUpTackDup						0x8796				// Same as 0x81DB
#define	chrSJISAngleDup							0x8797				// Same as 0x81DA
#define	chrSJISRightAngle						0x8798
#define	chrSJISRightTriangle					0x8799
#define	chrSJISBecauseDup						0x879A				// Same as 0x81E6
#define	chrSJISIntersectionDup					0x879B				// Same as 0x81BF
#define	chrSJISUnionDup							0x879C				// Same as 0x81BE

#define	chrSJISSmallRomanNumeralOne				0xEEEF
#define	chrSJISSmallRomanNumeralTwo				0xEEF0
#define	chrSJISSmallRomanNumeralThree 			0xEEF1
#define	chrSJISSmallRomanNumeralFour 			0xEEF2
#define	chrSJISSmallRomanNumeralFive 			0xEEF3
#define	chrSJISSmallRomanNumeralSix				0xEEF4
#define	chrSJISSmallRomanNumeralSeven 			0xEEF5
#define	chrSJISSmallRomanNumeralEight 			0xEEF6
#define	chrSJISSmallRomanNumeralNine 			0xEEF7
#define	chrSJISSmallRomanNumeralTen				0xEEF8
#define	chrSJISFullwidthNotSignDup				0xEEF9				// Same as 0x81CA
#define	chrSJISFullwidthBrokenBar				0xEEFA
#define	chrSJISFullwidthApostrophe				0xEEFB
#define	chrSJISFullwidthQuotationMark 			0xEEFC

#define	chrSJISSmallRomanNumeralOneDup 			0xFA40				// Same as 0xEEEF
#define	chrSJISSmallRomanNumeralTwoDup 			0xFA41				// Same as 0xEEF0
#define	chrSJISSmallRomanNumeralThreeDup 		0xFA42				// Same as 0xEEF1
#define	chrSJISSmallRomanNumeralFourDup 		0xFA43				// Same as 0xEEF2
#define	chrSJISSmallRomanNumeralFiveDup 		0xFA44				// Same as 0xEEF3
#define	chrSJISSmallRomanNumeralSixDup 			0xFA45				// Same as 0xEEF4
#define	chrSJISSmallRomanNumeralSevenDup 		0xFA46				// Same as 0xEEF5
#define	chrSJISSmallRomanNumeralEightDup 		0xFA47				// Same as 0xEEF6
#define	chrSJISSmallRomanNumeralNineDup 		0xFA48				// Same as 0xEEF7
#define	chrSJISSmallRomanNumeralTenDup 			0xFA49				// Same as 0xEEF8
#define	chrSJISRomanNumeralOneDup				0xFA4A				// Same as 0x8754
#define	chrSJISRomanNumeralTwoDup				0xFA4B				// Same as 0x8755
#define	chrSJISRomanNumeralThreeDup				0xFA4C				// Same as 0x8756
#define	chrSJISRomanNumeralFourDup				0xFA4D				// Same as 0x8757
#define	chrSJISRomanNumeralFiveDup				0xFA4E				// Same as 0x8758
#define	chrSJISRomanNumeralSixDup				0xFA4F				// Same as 0x8759
#define	chrSJISRomanNumeralSevenDup				0xFA50				// Same as 0x875A
#define	chrSJISRomanNumeralEightDup				0xFA51				// Same as 0x875B
#define	chrSJISRomanNumeralNineDup				0xFA52				// Same as 0x875C
#define	chrSJISRomanNumeralTenDup				0xFA53				// Same as 0x875D
#define	chrSJISFullwidthNotSignDup2				0xFA54				// Same as 0xEEF9 & 0x81CA
#define	chrSJISFullwidthBrokenBarDup 			0xFA55				// Same as 0xEEFA
#define	chrSJISFullwidthApostropheDup 			0xFA56				// Same as 0xEEFB
#define	chrSJISFullwidthQuotationMarkDup 		0xFA57				// Same as 0xEEFC
#define	chrSJISParenthesizedIdeographStockDup 	0xFA58			// Same as 0x878A
#define	chrSJISNumeroSignDup					0xFA59				// Same as 0x8782
#define	chrSJISTelephoneSignDup					0xFA5A				// Same as 0x8784
#define	chrSJISBecauseDup2						0xFA5B				// Same as 0x81E6 & 0x879A

#define chrSJISLastDoubleByte					0xFCFC

// Alternative character names.

#define	chrSJISChouon	chrKatakanaHiraganaProlongedSoundMark

#endif // __CHARSHIFTJIS_H__
