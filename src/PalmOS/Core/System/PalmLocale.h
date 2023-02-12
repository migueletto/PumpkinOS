/******************************************************************************
 *
 * Copyright (c) 2000-2003 PalmSource, Inc. All rights reserved.
 *
 * File: PalmLocale.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	Public header for simple constants that support locales (information
 *	specific to locales and regions).  This file is designed to support
 *	Rez in addition to C/C++.
 *
 *****************************************************************************/

#ifndef	__PALMLOCALE_H__
#define	__PALMLOCALE_H__


// Names of the known encodings.


#define	encodingNameAscii		"us-ascii"
#define	encodingNameISO8859_1	"ISO-8859-1"
#define	encodingNameCP1252		"Windows-1252"
#define	encodingNameShiftJIS	"Shift_JIS"
#define	encodingNameCP932		"Windows-31J"
#define	encodingNameUTF8		"UTF-8"
#define encodingNameUCS2		"ISO-10646-UCS-2"

#define encodingNamePalmGSM		"palmGSM"

#define encodingNameBig5		"Big5"
#define encodingNameBig5_HKSCS	"Big5-HKSCS"

#define encodingNameGB2312		"GB2312"
#define encodingNameHZ			"HZ-GB-2312"

// Maximum length of any encoding name.

#define	maxEncodingNameLength	40



/* Rez doesn't even support macros, so for Rez we must simplify all of the #defines.
Thus, whenever you modify any constants, please do so in the !rez section,
then execute the following MPW commands (from elsewhere with this file as the target)
so that the Rez case will be automatically updated:

Find ¥;Find /TAG SOURCE START/!1:/TAG SOURCE END/Á1
Copy ¤
Find ¥;Find /TAG DESTINATION START/!1:/TAG DESTINATION END/Á1
Echo
Paste ¤
Find ¥;Find /TAG DESTINATION START/
Replace -c ° /(#define[ ¶t]+l[a-z0-9_]+[ ¶t]+)¨1LANGUAGE_VALUE¶(([0-9]+)¨2¶)/ "¨1¨2"
Replace -c ° /(#define[ ¶t]+c[a-z0-9_]+[ ¶t]+)¨1COUNTRY_VALUE¶(([0-9]+)¨2¶)/ "¨1¨2"
Replace -c ° /(#define[ ¶t]+charEncoding[a-z0-9_]+[ ¶t]+)¨1CHAR_ENCODING_VALUE¶(([0-9]+)¨2¶)/ "¨1¨2"
*/

#ifndef rez
	#define rez 0
#endif

#if rez
	#define PALM_LOCALE_HAS_TYPES 0
#endif
#ifndef PALM_LOCALE_HAS_TYPES
	#define PALM_LOCALE_HAS_TYPES 1
#endif


#if PALM_LOCALE_HAS_TYPES // Normal (e.g., non-Rez) case

/***********************************************************************
 * Locale constants
 **********************************************************************/

/* Language codes (ISO 639).  The first 8 preserve the old values for the deprecated
LanguageType; the rest are sorted by the 2-character language code.

WARNING! Keep in sync with BOTH:
			1)	LanguageCode array in OverlayMgr.c
			2)	localeLanguage #define in UIResDefs.r
*/
#define LANGUAGE_VALUE(value) ((LanguageType)value)

// Leave the following line unchanged before 1st #define to be copied to rez section:
// TAG SOURCE START

#define lEnglish		LANGUAGE_VALUE(0)	// EN
#define lFrench			LANGUAGE_VALUE(1)	// FR
#define lGerman			LANGUAGE_VALUE(2)	// DE
#define lItalian		LANGUAGE_VALUE(3)	// IT
#define lSpanish		LANGUAGE_VALUE(4)	// ES
#define lUnused			LANGUAGE_VALUE(5)	// Reserved

// New in 3.1
#define lJapanese		LANGUAGE_VALUE(6)	// JA (Palm calls this jp)
#define lDutch			LANGUAGE_VALUE(7)	// NL

// New in 4.0
#define lAfar			LANGUAGE_VALUE(8)	// AA
#define lAbkhazian		LANGUAGE_VALUE(9)	// AB
#define lAfrikaans		LANGUAGE_VALUE(10)	// AF
#define lAmharic		LANGUAGE_VALUE(11)	// AM
#define lArabic			LANGUAGE_VALUE(12)	// AR
#define lAssamese		LANGUAGE_VALUE(13)	// AS
#define lAymara			LANGUAGE_VALUE(14)	// AY
#define lAzerbaijani	LANGUAGE_VALUE(15)	// AZ
#define lBashkir		LANGUAGE_VALUE(16)	// BA
#define lByelorussian	LANGUAGE_VALUE(17)	// BE
#define lBulgarian		LANGUAGE_VALUE(18)	// BG
#define lBihari			LANGUAGE_VALUE(19)	// BH
#define lBislama		LANGUAGE_VALUE(20)	// BI
#define lBengali		LANGUAGE_VALUE(21)	// BN (Bangla)
#define lTibetan		LANGUAGE_VALUE(22)	// BO
#define lBreton			LANGUAGE_VALUE(23)	// BR
#define lCatalan		LANGUAGE_VALUE(24)	// CA
#define lCorsican		LANGUAGE_VALUE(25)	// CO
#define lCzech			LANGUAGE_VALUE(26)	// CS
#define lWelsh			LANGUAGE_VALUE(27)	// CY
#define lDanish			LANGUAGE_VALUE(28)	// DA
#define lBhutani		LANGUAGE_VALUE(29)	// DZ
#define lGreek			LANGUAGE_VALUE(30)	// EL
#define lEsperanto		LANGUAGE_VALUE(31)	// EO
#define lEstonian		LANGUAGE_VALUE(32)	// ET
#define lBasque			LANGUAGE_VALUE(33)	// EU
#define lPersian		LANGUAGE_VALUE(34)	// FA (Farsi)
#define lFinnish		LANGUAGE_VALUE(35)	// FI
#define lFiji			LANGUAGE_VALUE(36)	// FJ
#define lFaroese		LANGUAGE_VALUE(37)	// FO
#define lFrisian		LANGUAGE_VALUE(38)	// FY
#define lIrish			LANGUAGE_VALUE(39)	// GA
#define lScotsGaelic	LANGUAGE_VALUE(40)	// GD
#define lGalician		LANGUAGE_VALUE(41)	// GL
#define lGuarani		LANGUAGE_VALUE(42)	// GN
#define lGujarati		LANGUAGE_VALUE(43)	// GU
#define lHausa			LANGUAGE_VALUE(44)	// HA
#define lHindi			LANGUAGE_VALUE(45)	// HI
#define lCroatian		LANGUAGE_VALUE(46)	// HR
#define lHungarian		LANGUAGE_VALUE(47)	// HU
#define lArmenian		LANGUAGE_VALUE(48)	// HY
#define lInterlingua	LANGUAGE_VALUE(49)	// IA
#define lInterlingue	LANGUAGE_VALUE(50)	// IE
#define lInupiak		LANGUAGE_VALUE(51)	// IK
#define lIndonesian		LANGUAGE_VALUE(52)	// IN
#define lIcelandic		LANGUAGE_VALUE(53)	// IS
#define lHebrew			LANGUAGE_VALUE(54)	// IW
#define lYiddish		LANGUAGE_VALUE(55)	// JI
#define lJavanese		LANGUAGE_VALUE(56)	// JW
#define lGeorgian		LANGUAGE_VALUE(57)	// KA
#define lKazakh			LANGUAGE_VALUE(58)	// KK
#define lGreenlandic	LANGUAGE_VALUE(59)	// KL
#define lCambodian		LANGUAGE_VALUE(60)	// KM
#define lKannada		LANGUAGE_VALUE(61)	// KN
#define lKorean			LANGUAGE_VALUE(62)	// KO
#define lKashmiri		LANGUAGE_VALUE(63)	// KS
#define lKurdish		LANGUAGE_VALUE(64)	// KU
#define lKirghiz		LANGUAGE_VALUE(65)	// KY
#define lLatin			LANGUAGE_VALUE(66)	// LA
#define lLingala		LANGUAGE_VALUE(67)	// LN
#define lLaothian		LANGUAGE_VALUE(68)	// LO
#define lLithuanian		LANGUAGE_VALUE(69)	// LT
#define lLatvian		LANGUAGE_VALUE(70)	// LV (Lettish)
#define lMalagasy		LANGUAGE_VALUE(71)	// MG
#define lMaori			LANGUAGE_VALUE(72)	// MI
#define lMacedonian		LANGUAGE_VALUE(73)	// MK
#define lMalayalam		LANGUAGE_VALUE(74)	// ML
#define lMongolian		LANGUAGE_VALUE(75)	// MN
#define lMoldavian		LANGUAGE_VALUE(76)	// MO
#define lMarathi		LANGUAGE_VALUE(77)	// MR
#define lMalay			LANGUAGE_VALUE(78)	// MS
#define lMaltese		LANGUAGE_VALUE(79)	// MT
#define lBurmese		LANGUAGE_VALUE(80)	// MY
#define lNauru			LANGUAGE_VALUE(81)	// NA
#define lNepali			LANGUAGE_VALUE(82)	// NE
#define lNorwegian		LANGUAGE_VALUE(83)	// NO
#define lOccitan		LANGUAGE_VALUE(84)	// OC
#define lAfan			LANGUAGE_VALUE(85)	// OM (Oromo)
#define lOriya			LANGUAGE_VALUE(86)	// OR
#define lPunjabi		LANGUAGE_VALUE(87)	// PA
#define lPolish			LANGUAGE_VALUE(88)	// PL
#define lPashto			LANGUAGE_VALUE(89)	// PS (Pushto)
#define lPortuguese		LANGUAGE_VALUE(90)	// PT
#define lQuechua		LANGUAGE_VALUE(91)	// QU
#define lRhaetoRomance	LANGUAGE_VALUE(92)	// RM
#define lKurundi		LANGUAGE_VALUE(93)	// RN
#define lRomanian		LANGUAGE_VALUE(94)	// RO
#define lRussian		LANGUAGE_VALUE(95)	// RU
#define lKinyarwanda	LANGUAGE_VALUE(96)	// RW
#define lSanskrit		LANGUAGE_VALUE(97)	// SA
#define lSindhi			LANGUAGE_VALUE(98)	// SD
#define lSangho			LANGUAGE_VALUE(99)	// SG
#define lSerboCroatian	LANGUAGE_VALUE(100)	// SH
#define lSinghalese		LANGUAGE_VALUE(101)	// SI
#define lSlovak			LANGUAGE_VALUE(102)	// SK
#define lSlovenian		LANGUAGE_VALUE(103)	// SL
#define lSamoan			LANGUAGE_VALUE(104)	// SM
#define lShona			LANGUAGE_VALUE(105)	// SN
#define lSomali			LANGUAGE_VALUE(106)	// SO
#define lAlbanian		LANGUAGE_VALUE(107)	// SQ
#define lSerbian		LANGUAGE_VALUE(108)	// SR
#define lSiswati		LANGUAGE_VALUE(109)	// SS
#define lSesotho		LANGUAGE_VALUE(110)	// ST
#define lSudanese		LANGUAGE_VALUE(111)	// SU
#define lSwedish		LANGUAGE_VALUE(112)	// SV
#define lSwahili		LANGUAGE_VALUE(113)	// SW
#define lTamil			LANGUAGE_VALUE(114)	// TA
#define lTelugu			LANGUAGE_VALUE(115)	// TE
#define lTajik			LANGUAGE_VALUE(116)	// TG
#define lThai			LANGUAGE_VALUE(117)	// TH
#define lTigrinya		LANGUAGE_VALUE(118)	// TI
#define lTurkmen		LANGUAGE_VALUE(119)	// TK
#define lTagalog		LANGUAGE_VALUE(120)	// TL
#define lSetswana		LANGUAGE_VALUE(121)	// TN
#define lTonga			LANGUAGE_VALUE(122)	// TO
#define lTurkish		LANGUAGE_VALUE(123)	// TR
#define lTsonga			LANGUAGE_VALUE(124)	// TS
#define lTatar			LANGUAGE_VALUE(125)	// TT
#define lTwi			LANGUAGE_VALUE(126)	// TW
#define lUkrainian		LANGUAGE_VALUE(127)	// UK
#define lUrdu			LANGUAGE_VALUE(128)	// UR
#define lUzbek			LANGUAGE_VALUE(129)	// UZ
#define lVietnamese		LANGUAGE_VALUE(130)	// VI
#define lVolapuk		LANGUAGE_VALUE(131)	// VO
#define lWolof			LANGUAGE_VALUE(132)	// WO
#define lXhosa			LANGUAGE_VALUE(133)	// XH
#define lYoruba			LANGUAGE_VALUE(134)	// YO
#define lChinese		LANGUAGE_VALUE(135)	// ZH
#define lZulu			LANGUAGE_VALUE(136)	// ZU
//
#define	lLanguageNum	LANGUAGE_VALUE(137)	// Number of Languages

/* Country codes (ISO 3166).  The first 33 preserve the old values for the
deprecated CountryType; the rest are sorted by the 2-character country code.

WARNING!	Keep in sync with BOTH:
			1)	CountryCode array in OverlayMgr.c
			2)	localeCountry #define in UIResDefs.r
*/
#define COUNTRY_VALUE(value) ((CountryType)value)

#define cAustralia					COUNTRY_VALUE(0)		// AU
#define cAustria					COUNTRY_VALUE(1)		// AT
#define cBelgium					COUNTRY_VALUE(2)		// BE
#define cBrazil						COUNTRY_VALUE(3)		// BR
#define cCanada						COUNTRY_VALUE(4)		// CA
#define cDenmark					COUNTRY_VALUE(5)		// DK
#define cFinland					COUNTRY_VALUE(6)		// FI
#define cFrance						COUNTRY_VALUE(7)		// FR
#define cGermany					COUNTRY_VALUE(8)		// DE
#define cHongKong					COUNTRY_VALUE(9)		// HK
#define cIceland					COUNTRY_VALUE(10)		// IS
#define cIreland					COUNTRY_VALUE(11)		// IE
#define cItaly						COUNTRY_VALUE(12)		// IT
#define cJapan						COUNTRY_VALUE(13)		// JP
#define cLuxembourg					COUNTRY_VALUE(14)		// LU
#define cMexico						COUNTRY_VALUE(15)		// MX
#define cNetherlands				COUNTRY_VALUE(16)		// NL
#define cNewZealand					COUNTRY_VALUE(17)		// NZ
#define cNorway						COUNTRY_VALUE(18)		// NO
#define cSpain						COUNTRY_VALUE(19)		// ES
#define cSweden						COUNTRY_VALUE(20)		// SE
#define cSwitzerland				COUNTRY_VALUE(21)		// CH
#define cUnitedKingdom				COUNTRY_VALUE(22)		// GB (UK)
#define cUnitedStates				COUNTRY_VALUE(23)		// US
#define cIndia						COUNTRY_VALUE(24)		// IN
#define cIndonesia					COUNTRY_VALUE(25)		// ID
#define cRepublicOfKorea			COUNTRY_VALUE(26)		// KR
#define cMalaysia					COUNTRY_VALUE(27)		// MY
#define cChina						COUNTRY_VALUE(28)		// CN
#define cPhilippines				COUNTRY_VALUE(29)		// PH
#define cSingapore					COUNTRY_VALUE(30)		// SG
#define cThailand					COUNTRY_VALUE(31)		// TH
#define cTaiwan						COUNTRY_VALUE(32)		// TW

// New in 4.0
#define cAndorra					COUNTRY_VALUE(33)		// AD
#define cUnitedArabEmirates			COUNTRY_VALUE(34)		// AE
#define cAfghanistan				COUNTRY_VALUE(35)		// AF
#define cAntiguaAndBarbuda			COUNTRY_VALUE(36)		// AG
#define cAnguilla					COUNTRY_VALUE(37)		// AI
#define cAlbania					COUNTRY_VALUE(38)		// AL
#define cArmenia					COUNTRY_VALUE(39)		// AM
#define cNetherlandsAntilles		COUNTRY_VALUE(40)		// AN
#define cAngola						COUNTRY_VALUE(41)		// AO
#define cAntarctica					COUNTRY_VALUE(42)		// AQ
#define cArgentina					COUNTRY_VALUE(43)		// AR
#define cAmericanSamoa				COUNTRY_VALUE(44)		// AS
#define cAruba						COUNTRY_VALUE(45)		// AW
#define cAzerbaijan					COUNTRY_VALUE(46)		// AZ
#define cBosniaAndHerzegovina		COUNTRY_VALUE(47)		// BA
#define cBarbados					COUNTRY_VALUE(48)		// BB
#define cBangladesh					COUNTRY_VALUE(49)		// BD
#define cBurkinaFaso				COUNTRY_VALUE(50)		// BF
#define cBulgaria					COUNTRY_VALUE(51)		// BG
#define cBahrain					COUNTRY_VALUE(52)		// BH
#define cBurundi					COUNTRY_VALUE(53)		// BI
#define cBenin						COUNTRY_VALUE(54)		// BJ
#define cBermuda					COUNTRY_VALUE(55)		// BM
#define cBruneiDarussalam			COUNTRY_VALUE(56)		// BN
#define cBolivia					COUNTRY_VALUE(57)		// BO
#define cBahamas					COUNTRY_VALUE(58)		// BS
#define cBhutan						COUNTRY_VALUE(59)		// BT
#define cBouvetIsland				COUNTRY_VALUE(60)		// BV
#define cBotswana					COUNTRY_VALUE(61)		// BW
#define cBelarus					COUNTRY_VALUE(62)		// BY
#define cBelize						COUNTRY_VALUE(63)		// BZ
#define cCocosIslands				COUNTRY_VALUE(64)		// CC
#define cDemocraticRepublicOfTheCongo	COUNTRY_VALUE(65)		// CD
#define cCentralAfricanRepublic		COUNTRY_VALUE(66)		// CF
#define cCongo						COUNTRY_VALUE(67)		// CG
#define cIvoryCoast					COUNTRY_VALUE(68)		// CI
#define cCookIslands				COUNTRY_VALUE(69)		// CK
#define cChile						COUNTRY_VALUE(70)		// CL
#define cCameroon					COUNTRY_VALUE(71)		// CM
#define cColumbia					COUNTRY_VALUE(72)		// CO
#define cCostaRica					COUNTRY_VALUE(73)		// CR
#define cCuba						COUNTRY_VALUE(74)		// CU
#define cCapeVerde					COUNTRY_VALUE(75)		// CV
#define cChristmasIsland			COUNTRY_VALUE(76)		// CX
#define cCyprus						COUNTRY_VALUE(77)		// CY
#define cCzechRepublic				COUNTRY_VALUE(78)		// CZ
#define cDjibouti					COUNTRY_VALUE(79)		// DJ
#define cDominica					COUNTRY_VALUE(80)		// DM
#define cDominicanRepublic			COUNTRY_VALUE(81)		// DO
#define cAlgeria					COUNTRY_VALUE(82)		// DZ
#define cEcuador					COUNTRY_VALUE(83)		// EC
#define cEstonia					COUNTRY_VALUE(84)		// EE
#define cEgypt						COUNTRY_VALUE(85)		// EG
#define cWesternSahara				COUNTRY_VALUE(86)		// EH
#define cEritrea					COUNTRY_VALUE(87)		// ER
#define cEthiopia					COUNTRY_VALUE(88)		// ET
#define cFiji						COUNTRY_VALUE(89)		// FJ
#define cFalklandIslands			COUNTRY_VALUE(90)		// FK
#define cMicronesia					COUNTRY_VALUE(91)		// FM
#define cFaeroeIslands				COUNTRY_VALUE(92)		// FO
#define cMetropolitanFrance			COUNTRY_VALUE(93)		// FX
#define cGabon						COUNTRY_VALUE(94)		// GA
#define cGrenada					COUNTRY_VALUE(95)		// GD
#define cGeorgia					COUNTRY_VALUE(96)		// GE
#define cFrenchGuiana				COUNTRY_VALUE(97)		// GF
#define cGhana						COUNTRY_VALUE(98)		// GH
#define cGibraltar					COUNTRY_VALUE(99)		// GI
#define cGreenland					COUNTRY_VALUE(100)	// GL
#define cGambia						COUNTRY_VALUE(101)	// GM
#define cGuinea						COUNTRY_VALUE(102)	// GN
#define cGuadeloupe					COUNTRY_VALUE(103)	// GP
#define cEquatorialGuinea			COUNTRY_VALUE(104)	// GQ
#define cGreece						COUNTRY_VALUE(105)	// GR
#define cSouthGeorgiaAndTheSouthSandwichIslands	COUNTRY_VALUE(106)	// GS
#define cGuatemala					COUNTRY_VALUE(107)	// GT
#define cGuam						COUNTRY_VALUE(108)	// GU
#define cGuineaBisseu				COUNTRY_VALUE(109)	// GW
#define cGuyana						COUNTRY_VALUE(110)	// GY
#define cHeardAndMcDonaldIslands	COUNTRY_VALUE(111)	// HM
#define cHonduras					COUNTRY_VALUE(112)	// HN
#define cCroatia					COUNTRY_VALUE(113)	// HR
#define cHaiti						COUNTRY_VALUE(114)	// HT
#define cHungary					COUNTRY_VALUE(115)	// HU
#define cIsrael						COUNTRY_VALUE(116)	// IL
#define cBritishIndianOceanTerritory	COUNTRY_VALUE(117)	// IO
#define cIraq						COUNTRY_VALUE(118)	// IQ
#define cIran						COUNTRY_VALUE(119)	// IR
#define cJamaica					COUNTRY_VALUE(120)	// JM
#define cJordan						COUNTRY_VALUE(121)	// JO
#define cKenya						COUNTRY_VALUE(122)	// KE
#define cKyrgyzstan					COUNTRY_VALUE(123)	// KG (Kirgistan)
#define cCambodia					COUNTRY_VALUE(124)	// KH
#define cKiribati					COUNTRY_VALUE(125)	// KI
#define cComoros					COUNTRY_VALUE(126)	// KM
#define cStKittsAndNevis			COUNTRY_VALUE(127)	// KN
#define cDemocraticPeoplesRepublicOfKorea	COUNTRY_VALUE(128)	// KP
#define cKuwait						COUNTRY_VALUE(129)	// KW
#define cCaymanIslands				COUNTRY_VALUE(130)	// KY
#define cKazakhstan					COUNTRY_VALUE(131)	// KK
#define cLaos						COUNTRY_VALUE(132)	// LA
#define cLebanon					COUNTRY_VALUE(133)	// LB
#define cStLucia					COUNTRY_VALUE(134)	// LC
#define cLiechtenstein				COUNTRY_VALUE(135)	// LI
#define cSriLanka					COUNTRY_VALUE(136)	// LK
#define cLiberia					COUNTRY_VALUE(137)	// LR
#define cLesotho					COUNTRY_VALUE(138)	// LS
#define cLithuania					COUNTRY_VALUE(139)	// LT
#define cLatvia						COUNTRY_VALUE(140)	// LV
#define cLibya						COUNTRY_VALUE(141)	// LY
#define cMorrocco					COUNTRY_VALUE(142)	// MA
#define cMonaco						COUNTRY_VALUE(143)	// MC
#define cMoldova					COUNTRY_VALUE(144)	// MD
#define cMadagascar					COUNTRY_VALUE(145)	// MG
#define cMarshallIslands			COUNTRY_VALUE(146)	// MH
#define cMacedonia					COUNTRY_VALUE(147)	// MK
#define cMali						COUNTRY_VALUE(148)	// ML
#define cMyanmar					COUNTRY_VALUE(149)	// MM
#define cMongolia					COUNTRY_VALUE(150)	// MN
#define cMacau						COUNTRY_VALUE(151)	// MO
#define cNorthernMarianaIslands		COUNTRY_VALUE(152)	// MP
#define cMartinique					COUNTRY_VALUE(153)	// MQ
#define cMauritania					COUNTRY_VALUE(154)	// MR
#define cMontserrat					COUNTRY_VALUE(155)	// MS
#define cMalta						COUNTRY_VALUE(156)	// MT
#define cMauritius					COUNTRY_VALUE(157)	// MU
#define cMaldives					COUNTRY_VALUE(158)	// MV
#define cMalawi						COUNTRY_VALUE(159)	// MW
#define cMozambique					COUNTRY_VALUE(160)	// MZ
#define cNamibia					COUNTRY_VALUE(161)	// NA
#define cNewCaledonia				COUNTRY_VALUE(162)	// NC
#define cNiger						COUNTRY_VALUE(163)	// NE
#define cNorfolkIsland				COUNTRY_VALUE(164)	// NF
#define cNigeria					COUNTRY_VALUE(165)	// NG
#define cNicaragua					COUNTRY_VALUE(166)	// NI
#define cNepal						COUNTRY_VALUE(167)	// NP
#define cNauru						COUNTRY_VALUE(168)	// NR
#define cNiue						COUNTRY_VALUE(169)	// NU
#define cOman						COUNTRY_VALUE(170)	// OM
#define cPanama						COUNTRY_VALUE(171)	// PA
#define cPeru						COUNTRY_VALUE(172)	// PE
#define cFrenchPolynesia			COUNTRY_VALUE(173)	// PF
#define cPapuaNewGuinea				COUNTRY_VALUE(174)	// PG
#define cPakistan					COUNTRY_VALUE(175)	// PK
#define cPoland						COUNTRY_VALUE(176)	// PL
#define cStPierreAndMiquelon		COUNTRY_VALUE(177)	// PM
#define cPitcairn					COUNTRY_VALUE(178)	// PN
#define cPuertoRico					COUNTRY_VALUE(179)	// PR
#define cPortugal					COUNTRY_VALUE(180)	// PT
#define cPalau						COUNTRY_VALUE(181)	// PW
#define cParaguay					COUNTRY_VALUE(182)	// PY
#define cQatar						COUNTRY_VALUE(183)	// QA
#define cReunion					COUNTRY_VALUE(184)	// RE
#define cRomania					COUNTRY_VALUE(185)	// RO
#define cRussianFederation			COUNTRY_VALUE(186)	// RU
#define cRwanda						COUNTRY_VALUE(187)	// RW
#define cSaudiArabia				COUNTRY_VALUE(188)	// SA
#define cSolomonIslands				COUNTRY_VALUE(189)	// SB
#define cSeychelles					COUNTRY_VALUE(190)	// SC
#define cSudan						COUNTRY_VALUE(191)	// SD
#define cStHelena					COUNTRY_VALUE(192)	// SH
#define cSlovenia					COUNTRY_VALUE(193)	// SI
#define cSvalbardAndJanMayenIslands	COUNTRY_VALUE(194)	// SJ
#define cSlovakia					COUNTRY_VALUE(195)	// SK
#define cSierraLeone				COUNTRY_VALUE(196)	// SL
#define cSanMarino					COUNTRY_VALUE(197)	// SM
#define cSenegal					COUNTRY_VALUE(198)	// SN
#define cSomalia					COUNTRY_VALUE(199)	// SO
#define cSuriname					COUNTRY_VALUE(200)	// SR
#define cSaoTomeAndPrincipe			COUNTRY_VALUE(201)	// ST
#define cElSalvador					COUNTRY_VALUE(202)	// SV
#define cSyranArabRepublic			COUNTRY_VALUE(203)	// SY
#define cSwaziland					COUNTRY_VALUE(204)	// SZ
#define cTurksAndCaicosIslands		COUNTRY_VALUE(205)	// TC
#define cChad						COUNTRY_VALUE(206)	// TD
#define cFrenchSouthernTerritories	COUNTRY_VALUE(207)	// TF
#define cTogo						COUNTRY_VALUE(208)	// TG
#define cTajikistan					COUNTRY_VALUE(209)	// TJ
#define cTokelau					COUNTRY_VALUE(210)	// TK
#define cTurkmenistan				COUNTRY_VALUE(211)	// TM
#define cTunisia					COUNTRY_VALUE(212)	// TN
#define cTonga						COUNTRY_VALUE(213)	// TO
#define cEastTimor					COUNTRY_VALUE(214)	// TP
#define cTurkey						COUNTRY_VALUE(215)	// TR
#define cTrinidadAndTobago			COUNTRY_VALUE(216)	// TT
#define cTuvalu						COUNTRY_VALUE(217)	// TV
#define cTanzania					COUNTRY_VALUE(218)	// TZ
#define cUkraine					COUNTRY_VALUE(219)	// UA
#define cUganda						COUNTRY_VALUE(220)	// UG
#define cUnitedStatesMinorOutlyingIslands	COUNTRY_VALUE(221)	// UM
#define cUruguay					COUNTRY_VALUE(222)	// UY
#define cUzbekistan					COUNTRY_VALUE(223)	// UZ
#define cHolySee					COUNTRY_VALUE(224)	// VA
#define cStVincentAndTheGrenadines	COUNTRY_VALUE(225)	// VC
#define cVenezuela					COUNTRY_VALUE(226)	// VE
#define cBritishVirginIslands		COUNTRY_VALUE(227)	// VG
#define cUSVirginIslands			COUNTRY_VALUE(228)	// VI
#define cVietNam					COUNTRY_VALUE(229)	// VN
#define cVanuatu					COUNTRY_VALUE(230)	// VU
#define cWallisAndFutunaIslands		COUNTRY_VALUE(231)	// WF
#define cSamoa						COUNTRY_VALUE(232)	// WS
#define cYemen						COUNTRY_VALUE(233)	// YE
#define cMayotte					COUNTRY_VALUE(234)	// YT
#define cYugoslavia					COUNTRY_VALUE(235)	// YU
#define cSouthAfrica				COUNTRY_VALUE(236)	// ZA
#define cZambia						COUNTRY_VALUE(237)	// ZM
#define cZimbabwe					COUNTRY_VALUE(238)	// ZW
//
#define	cCountryNum					COUNTRY_VALUE(239)	// Number of Countries

/* Various character encodings supported by the PalmOS. Actually these
are a mixture of character sets (repetoires or coded character sets
in Internet lingo) and character encodings (CES - character encoding
standard). Many, however, are some of both (e.g. CP932 is the Shift-JIS
encoding of the JIS character set + Microsoft's extensions).

The following character set values are used by:
	a) Palm devices
	b) Palm wireless servers
	
WARNING!	Be aware that a device supporting a new character set
			will require some character set definition and maybe
			some development on the wireless server side.
*/
#define CHAR_ENCODING_VALUE(value) ((CharEncodingType)value)

// Unknown to this version of PalmOS.
#define	charEncodingUnknown		CHAR_ENCODING_VALUE(0)

// Maximum character encoding _currently_ defined
#define	charEncodingMax			CHAR_ENCODING_VALUE(90)

// Latin Palm OS character encoding, and subsets.
// PalmOS variant of CP1252, with 10 extra Greek characters
#define charEncodingPalmGSM		CHAR_ENCODING_VALUE(78)
// PalmOS version of CP1252
#define	charEncodingPalmLatin	CHAR_ENCODING_VALUE(3)
// Windows variant of 8859-1
#define	charEncodingCP1252		CHAR_ENCODING_VALUE(7)
// ISO 8859 Part 1
#define	charEncodingISO8859_1	CHAR_ENCODING_VALUE(2)
// ISO 646-1991
#define	charEncodingAscii		CHAR_ENCODING_VALUE(1)

// Japanese Palm OS character encoding, and subsets.
// PalmOS version of CP932
#define	charEncodingPalmSJIS	CHAR_ENCODING_VALUE(5)
// Windows variant of ShiftJIS
#define	charEncodingCP932		CHAR_ENCODING_VALUE(8)
// Encoding for JIS 0208-1990 + 1-byte katakana
#define	charEncodingShiftJIS	CHAR_ENCODING_VALUE(4)

// Unicode character encodings
#define charEncodingUCS2		CHAR_ENCODING_VALUE(9)
#define charEncodingUTF8		CHAR_ENCODING_VALUE(6)
#define charEncodingUTF7		CHAR_ENCODING_VALUE(24)
#define charEncodingUTF16		CHAR_ENCODING_VALUE(75)
#define charEncodingUTF16BE		CHAR_ENCODING_VALUE(76)
#define charEncodingUTF16LE		CHAR_ENCODING_VALUE(77)
#define charEncodingUTF32		CHAR_ENCODING_VALUE(84)
#define charEncodingUTF32BE		CHAR_ENCODING_VALUE(85)
#define charEncodingUTF32LE		CHAR_ENCODING_VALUE(86)
#define charEncodingUTF7_IMAP	CHAR_ENCODING_VALUE(87)
#define	charEncodingUCS4		CHAR_ENCODING_VALUE(88)

// Latin character encodings
#define charEncodingCP850		CHAR_ENCODING_VALUE(12)
#define charEncodingCP437		CHAR_ENCODING_VALUE(13)
#define charEncodingCP865		CHAR_ENCODING_VALUE(14)
#define charEncodingCP860		CHAR_ENCODING_VALUE(15)
#define charEncodingCP861		CHAR_ENCODING_VALUE(16)
#define charEncodingCP863		CHAR_ENCODING_VALUE(17)
#define charEncodingCP775		CHAR_ENCODING_VALUE(18)
#define charEncodingMacIslande	CHAR_ENCODING_VALUE(19)
#define charEncodingMacintosh	CHAR_ENCODING_VALUE(20)
#define charEncodingCP1257		CHAR_ENCODING_VALUE(21)
#define charEncodingISO8859_3	CHAR_ENCODING_VALUE(22)
#define charEncodingISO8859_4	CHAR_ENCODING_VALUE(23)

// Extended Latin character encodings
#define charEncodingISO8859_2	CHAR_ENCODING_VALUE(26)
#define charEncodingCP1250		CHAR_ENCODING_VALUE(27)
#define charEncodingCP852		CHAR_ENCODING_VALUE(28)
#define charEncodingXKamenicky	CHAR_ENCODING_VALUE(29)
#define charEncodingMacXCroate	CHAR_ENCODING_VALUE(30)
#define charEncodingMacXLat2	CHAR_ENCODING_VALUE(31)
#define charEncodingMacXRomania	CHAR_ENCODING_VALUE(32)
#define charEncodingGSM			CHAR_ENCODING_VALUE(90)

// Japanese character encodings
#define charEncodingEucJp		CHAR_ENCODING_VALUE(25)
#define charEncodingISO2022Jp	CHAR_ENCODING_VALUE(10)
#define charEncodingXAutoJp		CHAR_ENCODING_VALUE(11)

// Greek character encodings
#define charEncodingISO8859_7	CHAR_ENCODING_VALUE(33)
#define charEncodingCP1253		CHAR_ENCODING_VALUE(34)
#define charEncodingCP869		CHAR_ENCODING_VALUE(35)
#define charEncodingCP737		CHAR_ENCODING_VALUE(36)
#define charEncodingMacXGr		CHAR_ENCODING_VALUE(37)

// Cyrillic character encodings
#define charEncodingCP1251		CHAR_ENCODING_VALUE(38)
#define charEncodingISO8859_5	CHAR_ENCODING_VALUE(39)
#define charEncodingKoi8R		CHAR_ENCODING_VALUE(40)
#define charEncodingKoi8		CHAR_ENCODING_VALUE(41)
#define charEncodingCP855		CHAR_ENCODING_VALUE(42)
#define charEncodingCP866		CHAR_ENCODING_VALUE(43)
#define charEncodingMacCyr		CHAR_ENCODING_VALUE(44)
#define charEncodingMacUkraine	CHAR_ENCODING_VALUE(45)

// Turkish character encodings
#define charEncodingCP1254		CHAR_ENCODING_VALUE(46)
#define charEncodingISO8859_9	CHAR_ENCODING_VALUE(47)
#define charEncodingCP857		CHAR_ENCODING_VALUE(48)
#define charEncodingMacTurc		CHAR_ENCODING_VALUE(49)
#define charEncodingCP853		CHAR_ENCODING_VALUE(50)

// Arabic character encodings
#define charEncodingISO8859_6	CHAR_ENCODING_VALUE(51)
#define charEncodingAsmo708		CHAR_ENCODING_VALUE(52)
#define charEncodingCP1256		CHAR_ENCODING_VALUE(53)
#define charEncodingCP864		CHAR_ENCODING_VALUE(54)
#define charEncodingAsmo708Plus	CHAR_ENCODING_VALUE(55)
#define charEncodingAsmo708Fr	CHAR_ENCODING_VALUE(56)
#define charEncodingMacAra		CHAR_ENCODING_VALUE(57)

// Simplified Chinese character encodings
#define charEncodingGB2312		CHAR_ENCODING_VALUE(58)
#define charEncodingHZ			CHAR_ENCODING_VALUE(59)
#define charEncodingGBK			CHAR_ENCODING_VALUE(82)
#define charEncodingPalmGB		CHAR_ENCODING_VALUE(83)

// Traditional Chinese character encodings
#define charEncodingBig5		CHAR_ENCODING_VALUE(60)
#define charEncodingBig5_HKSCS	CHAR_ENCODING_VALUE(79)
#define charEncodingBig5Plus	CHAR_ENCODING_VALUE(80)
#define charEncodingPalmBig5	CHAR_ENCODING_VALUE(81)
#define	charEncodingISO2022CN	CHAR_ENCODING_VALUE(89)

// Vietnamese character encodings
#define charEncodingViscii		CHAR_ENCODING_VALUE(61)
#define charEncodingViqr		CHAR_ENCODING_VALUE(62)
#define charEncodingVncii		CHAR_ENCODING_VALUE(63)
#define charEncodingVietnet		CHAR_ENCODING_VALUE(65)
#define charEncodingCP1258		CHAR_ENCODING_VALUE(66)

// Korean character encodings
#define charEncodingEucKr		CHAR_ENCODING_VALUE(67)		// Was charEncodingKsc5601
#define charEncodingCP949		CHAR_ENCODING_VALUE(68)
#define charEncodingISO2022Kr	CHAR_ENCODING_VALUE(69)

// Hebrew character encodings
#define charEncodingISO8859_8I	CHAR_ENCODING_VALUE(70)
#define charEncodingISO8859_8	CHAR_ENCODING_VALUE(71)
#define charEncodingCP1255		CHAR_ENCODING_VALUE(72)
#define charEncodingCP1255V		CHAR_ENCODING_VALUE(73)

// Thai character encodings
#define charEncodingTis620		CHAR_ENCODING_VALUE(74)
#define charEncodingCP874		CHAR_ENCODING_VALUE(64)


// Leave the following line unchanged at end of section:
// TAG SOURCE END

/* Rez doesn't even support macros, so for Rez we simplify all of the #defines.

WARNING!	This section is auto-generated (see comment near top of file).
*/
#else // !PALM_LOCALE_HAS_TYPES

// Leave the following line unchanged at beginning of rez section
// TAG DESTINATION START

#define lEnglish		0	// EN
#define lFrench			1	// FR
#define lGerman			2	// DE
#define lItalian		3	// IT
#define lSpanish		4	// ES
#define lUnused			5	// Reserved

// New in 3.1
#define lJapanese		6	// JA (Palm calls this jp)
#define lDutch			7	// NL

// New in 4.0
#define lAfar			8	// AA
#define lAbkhazian		9	// AB
#define lAfrikaans		10	// AF
#define lAmharic		11	// AM
#define lArabic			12	// AR
#define lAssamese		13	// AS
#define lAymara			14	// AY
#define lAzerbaijani	15	// AZ
#define lBashkir		16	// BA
#define lByelorussian	17	// BE
#define lBulgarian		18	// BG
#define lBihari			19	// BH
#define lBislama		20	// BI
#define lBengali		21	// BN (Bangla)
#define lTibetan		22	// BO
#define lBreton			23	// BR
#define lCatalan		24	// CA
#define lCorsican		25	// CO
#define lCzech			26	// CS
#define lWelsh			27	// CY
#define lDanish			28	// DA
#define lBhutani		29	// DZ
#define lGreek			30	// EL
#define lEsperanto		31	// EO
#define lEstonian		32	// ET
#define lBasque			33	// EU
#define lPersian		34	// FA (Farsi)
#define lFinnish		35	// FI
#define lFiji			36	// FJ
#define lFaroese		37	// FO
#define lFrisian		38	// FY
#define lIrish			39	// GA
#define lScotsGaelic	40	// GD
#define lGalician		41	// GL
#define lGuarani		42	// GN
#define lGujarati		43	// GU
#define lHausa			44	// HA
#define lHindi			45	// HI
#define lCroatian		46	// HR
#define lHungarian		47	// HU
#define lArmenian		48	// HY
#define lInterlingua	49	// IA
#define lInterlingue	50	// IE
#define lInupiak		51	// IK
#define lIndonesian		52	// IN
#define lIcelandic		53	// IS
#define lHebrew			54	// IW
#define lYiddish		55	// JI
#define lJavanese		56	// JW
#define lGeorgian		57	// KA
#define lKazakh			58	// KK
#define lGreenlandic	59	// KL
#define lCambodian		60	// KM
#define lKannada		61	// KN
#define lKorean			62	// KO
#define lKashmiri		63	// KS
#define lKurdish		64	// KU
#define lKirghiz		65	// KY
#define lLatin			66	// LA
#define lLingala		67	// LN
#define lLaothian		68	// LO
#define lLithuanian		69	// LT
#define lLatvian		70	// LV (Lettish)
#define lMalagasy		71	// MG
#define lMaori			72	// MI
#define lMacedonian		73	// MK
#define lMalayalam		74	// ML
#define lMongolian		75	// MN
#define lMoldavian		76	// MO
#define lMarathi		77	// MR
#define lMalay			78	// MS
#define lMaltese		79	// MT
#define lBurmese		80	// MY
#define lNauru			81	// NA
#define lNepali			82	// NE
#define lNorwegian		83	// NO
#define lOccitan		84	// OC
#define lAfan			85	// OM (Oromo)
#define lOriya			86	// OR
#define lPunjabi		87	// PA
#define lPolish			88	// PL
#define lPashto			89	// PS (Pushto)
#define lPortuguese		90	// PT
#define lQuechua		91	// QU
#define lRhaetoRomance	92	// RM
#define lKurundi		93	// RN
#define lRomanian		94	// RO
#define lRussian		95	// RU
#define lKinyarwanda	96	// RW
#define lSanskrit		97	// SA
#define lSindhi			98	// SD
#define lSangho			99	// SG
#define lSerboCroatian	100	// SH
#define lSinghalese		101	// SI
#define lSlovak			102	// SK
#define lSlovenian		103	// SL
#define lSamoan			104	// SM
#define lShona			105	// SN
#define lSomali			106	// SO
#define lAlbanian		107	// SQ
#define lSerbian		108	// SR
#define lSiswati		109	// SS
#define lSesotho		110	// ST
#define lSudanese		111	// SU
#define lSwedish		112	// SV
#define lSwahili		113	// SW
#define lTamil			114	// TA
#define lTelugu			115	// TE
#define lTajik			116	// TG
#define lThai			117	// TH
#define lTigrinya		118	// TI
#define lTurkmen		119	// TK
#define lTagalog		120	// TL
#define lSetswana		121	// TN
#define lTonga			122	// TO
#define lTurkish		123	// TR
#define lTsonga			124	// TS
#define lTatar			125	// TT
#define lTwi			126	// TW
#define lUkrainian		127	// UK
#define lUrdu			128	// UR
#define lUzbek			129	// UZ
#define lVietnamese		130	// VI
#define lVolapuk		131	// VO
#define lWolof			132	// WO
#define lXhosa			133	// XH
#define lYoruba			134	// YO
#define lChinese		135	// ZH
#define lZulu			136	// ZU
//
#define	lLanguageNum	137	// Number of Languages

/* Country codes (ISO 3166).  The first 33 preserve the old values for the
deprecated CountryType; the rest are sorted by the 2-character country code.

WARNING!	Keep in sync with BOTH:
			1)	CountryCode array in OverlayMgr.c
			2)	localeCountry #define in UIResDefs.r
*/
#define COUNTRY_VALUE(value) ((CountryType)value)

#define cAustralia					0		// AU
#define cAustria					1		// AT
#define cBelgium					2		// BE
#define cBrazil						3		// BR
#define cCanada						4		// CA
#define cDenmark					5		// DK
#define cFinland					6		// FI
#define cFrance						7		// FR
#define cGermany					8		// DE
#define cHongKong					9		// HK
#define cIceland					10		// IS
#define cIreland					11		// IE
#define cItaly						12		// IT
#define cJapan						13		// JP
#define cLuxembourg					14		// LU
#define cMexico						15		// MX
#define cNetherlands				16		// NL
#define cNewZealand					17		// NZ
#define cNorway						18		// NO
#define cSpain						19		// ES
#define cSweden						20		// SE
#define cSwitzerland				21		// CH
#define cUnitedKingdom				22		// GB (UK)
#define cUnitedStates				23		// US
#define cIndia						24		// IN
#define cIndonesia					25		// ID
#define cRepublicOfKorea			26		// KR
#define cMalaysia					27		// MY
#define cChina						28		// CN
#define cPhilippines				29		// PH
#define cSingapore					30		// SG
#define cThailand					31		// TH
#define cTaiwan						32		// TW

// New in 4.0
#define cAndorra					33		// AD
#define cUnitedArabEmirates			34		// AE
#define cAfghanistan				35		// AF
#define cAntiguaAndBarbuda			36		// AG
#define cAnguilla					37		// AI
#define cAlbania					38		// AL
#define cArmenia					39		// AM
#define cNetherlandsAntilles		40		// AN
#define cAngola						41		// AO
#define cAntarctica					42		// AQ
#define cArgentina					43		// AR
#define cAmericanSamoa				44		// AS
#define cAruba						45		// AW
#define cAzerbaijan					46		// AZ
#define cBosniaAndHerzegovina		47		// BA
#define cBarbados					48		// BB
#define cBangladesh					49		// BD
#define cBurkinaFaso				50		// BF
#define cBulgaria					51		// BG
#define cBahrain					52		// BH
#define cBurundi					53		// BI
#define cBenin						54		// BJ
#define cBermuda					55		// BM
#define cBruneiDarussalam			56		// BN
#define cBolivia					57		// BO
#define cBahamas					58		// BS
#define cBhutan						59		// BT
#define cBouvetIsland				60		// BV
#define cBotswana					61		// BW
#define cBelarus					62		// BY
#define cBelize						63		// BZ
#define cCocosIslands				64		// CC
#define cDemocraticRepublicOfTheCongo	65		// CD
#define cCentralAfricanRepublic		66		// CF
#define cCongo						67		// CG
#define cIvoryCoast					68		// CI
#define cCookIslands				69		// CK
#define cChile						70		// CL
#define cCameroon					71		// CM
#define cColumbia					72		// CO
#define cCostaRica					73		// CR
#define cCuba						74		// CU
#define cCapeVerde					75		// CV
#define cChristmasIsland			76		// CX
#define cCyprus						77		// CY
#define cCzechRepublic				78		// CZ
#define cDjibouti					79		// DJ
#define cDominica					80		// DM
#define cDominicanRepublic			81		// DO
#define cAlgeria					82		// DZ
#define cEcuador					83		// EC
#define cEstonia					84		// EE
#define cEgypt						85		// EG
#define cWesternSahara				86		// EH
#define cEritrea					87		// ER
#define cEthiopia					88		// ET
#define cFiji						89		// FJ
#define cFalklandIslands			90		// FK
#define cMicronesia					91		// FM
#define cFaeroeIslands				92		// FO
#define cMetropolitanFrance			93		// FX
#define cGabon						94		// GA
#define cGrenada					95		// GD
#define cGeorgia					96		// GE
#define cFrenchGuiana				97		// GF
#define cGhana						98		// GH
#define cGibraltar					99		// GI
#define cGreenland					100	// GL
#define cGambia						101	// GM
#define cGuinea						102	// GN
#define cGuadeloupe					103	// GP
#define cEquatorialGuinea			104	// GQ
#define cGreece						105	// GR
#define cSouthGeorgiaAndTheSouthSandwichIslands	106	// GS
#define cGuatemala					107	// GT
#define cGuam						108	// GU
#define cGuineaBisseu				109	// GW
#define cGuyana						110	// GY
#define cHeardAndMcDonaldIslands	111	// HM
#define cHonduras					112	// HN
#define cCroatia					113	// HR
#define cHaiti						114	// HT
#define cHungary					115	// HU
#define cIsrael						116	// IL
#define cBritishIndianOceanTerritory	117	// IO
#define cIraq						118	// IQ
#define cIran						119	// IR
#define cJamaica					120	// JM
#define cJordan						121	// JO
#define cKenya						122	// KE
#define cKyrgyzstan					123	// KG (Kirgistan)
#define cCambodia					124	// KH
#define cKiribati					125	// KI
#define cComoros					126	// KM
#define cStKittsAndNevis			127	// KN
#define cDemocraticPeoplesRepublicOfKorea	128	// KP
#define cKuwait						129	// KW
#define cCaymanIslands				130	// KY
#define cKazakhstan					131	// KK
#define cLaos						132	// LA
#define cLebanon					133	// LB
#define cStLucia					134	// LC
#define cLiechtenstein				135	// LI
#define cSriLanka					136	// LK
#define cLiberia					137	// LR
#define cLesotho					138	// LS
#define cLithuania					139	// LT
#define cLatvia						140	// LV
#define cLibya						141	// LY
#define cMorrocco					142	// MA
#define cMonaco						143	// MC
#define cMoldova					144	// MD
#define cMadagascar					145	// MG
#define cMarshallIslands			146	// MH
#define cMacedonia					147	// MK
#define cMali						148	// ML
#define cMyanmar					149	// MM
#define cMongolia					150	// MN
#define cMacau						151	// MO
#define cNorthernMarianaIslands		152	// MP
#define cMartinique					153	// MQ
#define cMauritania					154	// MR
#define cMontserrat					155	// MS
#define cMalta						156	// MT
#define cMauritius					157	// MU
#define cMaldives					158	// MV
#define cMalawi						159	// MW
#define cMozambique					160	// MZ
#define cNamibia					161	// NA
#define cNewCaledonia				162	// NC
#define cNiger						163	// NE
#define cNorfolkIsland				164	// NF
#define cNigeria					165	// NG
#define cNicaragua					166	// NI
#define cNepal						167	// NP
#define cNauru						168	// NR
#define cNiue						169	// NU
#define cOman						170	// OM
#define cPanama						171	// PA
#define cPeru						172	// PE
#define cFrenchPolynesia			173	// PF
#define cPapuaNewGuinea				174	// PG
#define cPakistan					175	// PK
#define cPoland						176	// PL
#define cStPierreAndMiquelon		177	// PM
#define cPitcairn					178	// PN
#define cPuertoRico					179	// PR
#define cPortugal					180	// PT
#define cPalau						181	// PW
#define cParaguay					182	// PY
#define cQatar						183	// QA
#define cReunion					184	// RE
#define cRomania					185	// RO
#define cRussianFederation			186	// RU
#define cRwanda						187	// RW
#define cSaudiArabia				188	// SA
#define cSolomonIslands				189	// SB
#define cSeychelles					190	// SC
#define cSudan						191	// SD
#define cStHelena					192	// SH
#define cSlovenia					193	// SI
#define cSvalbardAndJanMayenIslands	194	// SJ
#define cSlovakia					195	// SK
#define cSierraLeone				196	// SL
#define cSanMarino					197	// SM
#define cSenegal					198	// SN
#define cSomalia					199	// SO
#define cSuriname					200	// SR
#define cSaoTomeAndPrincipe			201	// ST
#define cElSalvador					202	// SV
#define cSyranArabRepublic			203	// SY
#define cSwaziland					204	// SZ
#define cTurksAndCaicosIslands		205	// TC
#define cChad						206	// TD
#define cFrenchSouthernTerritories	207	// TF
#define cTogo						208	// TG
#define cTajikistan					209	// TJ
#define cTokelau					210	// TK
#define cTurkmenistan				211	// TM
#define cTunisia					212	// TN
#define cTonga						213	// TO
#define cEastTimor					214	// TP
#define cTurkey						215	// TR
#define cTrinidadAndTobago			216	// TT
#define cTuvalu						217	// TV
#define cTanzania					218	// TZ
#define cUkraine					219	// UA
#define cUganda						220	// UG
#define cUnitedStatesMinorOutlyingIslands	221	// UM
#define cUruguay					222	// UY
#define cUzbekistan					223	// UZ
#define cHolySee					224	// VA
#define cStVincentAndTheGrenadines	225	// VC
#define cVenezuela					226	// VE
#define cBritishVirginIslands		227	// VG
#define cUSVirginIslands			228	// VI
#define cVietNam					229	// VN
#define cVanuatu					230	// VU
#define cWallisAndFutunaIslands		231	// WF
#define cSamoa						232	// WS
#define cYemen						233	// YE
#define cMayotte					234	// YT
#define cYugoslavia					235	// YU
#define cSouthAfrica				236	// ZA
#define cZambia						237	// ZM
#define cZimbabwe					238	// ZW
//
#define	cCountryNum					239	// Number of Countries

/* Various character encodings supported by the PalmOS. Actually these
are a mixture of character sets (repetoires or coded character sets
in Internet lingo) and character encodings (CES - character encoding
standard). Many, however, are some of both (e.g. CP932 is the Shift-JIS
encoding of the JIS character set + Microsoft's extensions).

The following character set values are used by:
	a) Palm devices
	b) Palm wireless servers
	
WARNING!	Be aware that a device supporting a new character set
			will require some character set definition and maybe
			some development on the wireless server side.
*/


///////////////////////////////////////////////////////////////////////////
// Warning: The following files are interdependent and MUST to be modified together:
//
//   -> Viewer\Incs\Core\System\PalmLocale.h
//      (defines charset IDs)
//
//   -> Server\apps\Elaine\Src\Core\CharsetInfo.cpp
//      (defines charset attributes like Asian, cp1252 compatible, etc.)
//
//   -> Server\CstPalmOSCharsets.lst
//      (list the iso names of
//
///////////////////////////////////////////////////////////////////////////


#define CHAR_ENCODING_VALUE(value) ((CharEncodingType)value)

// Unknown to this version of PalmOS.
#define	charEncodingUnknown		0

// Maximum character encoding _currently_ defined

#define	charEncodingMax			90

// Latin Palm OS character encoding, and subsets.
// PalmOS variant of CP1252, with 10 extra Greek characters
#define charEncodingPalmGSM		78
// PalmOS version of CP1252
#define	charEncodingPalmLatin	3
// Windows variant of 8859-1
#define	charEncodingCP1252		7
// ISO 8859 Part 1
#define	charEncodingISO8859_1	2
// ISO 646-1991
#define	charEncodingAscii		1

// Japanese Palm OS character encoding, and subsets.
// PalmOS version of CP932
#define	charEncodingPalmSJIS	5
// Windows variant of ShiftJIS
#define	charEncodingCP932		8
// Encoding for JIS 0208-1990 + 1-byte katakana
#define	charEncodingShiftJIS	4

// Unicode character encodings
#define charEncodingUCS2		9
#define charEncodingUTF8		6
#define charEncodingUTF7		24
#define charEncodingUTF16		75
#define charEncodingUTF16BE		76
#define charEncodingUTF16LE		77
#define charEncodingUTF32		84
#define charEncodingUTF32BE		85
#define charEncodingUTF32LE		86
#define charEncodingUTF7_IMAP	87
#define charEncodingUCS4		88

// Latin character encodings
#define charEncodingCP850		12
#define charEncodingCP437		13
#define charEncodingCP865		14
#define charEncodingCP860		15
#define charEncodingCP861		16
#define charEncodingCP863		17
#define charEncodingCP775		18
#define charEncodingMacIslande	19
#define charEncodingMacintosh	20
#define charEncodingCP1257		21
#define charEncodingISO8859_3	22
#define charEncodingISO8859_4	23
#define charEncodingGSM			90

// Extended Latin character encodings
#define charEncodingISO8859_2	26
#define charEncodingCP1250		27
#define charEncodingCP852		28
#define charEncodingXKamenicky	29
#define charEncodingMacXCroate	30
#define charEncodingMacXLat2	31
#define charEncodingMacXRomania	32

// Japanese character encodings
#define charEncodingEucJp		25
#define charEncodingISO2022Jp	10
#define charEncodingXAutoJp		11

// Greek character encodings
#define charEncodingISO8859_7	33
#define charEncodingCP1253		34
#define charEncodingCP869		35
#define charEncodingCP737		36
#define charEncodingMacXGr		37

// Cyrillic character encodings
#define charEncodingCP1251		38
#define charEncodingISO8859_5	39
#define charEncodingKoi8R		40
#define charEncodingKoi8		41
#define charEncodingCP855		42
#define charEncodingCP866		43
#define charEncodingMacCyr		44
#define charEncodingMacUkraine	45

// Turkish character encodings
#define charEncodingCP1254		46
#define charEncodingISO8859_9	47
#define charEncodingCP857		48
#define charEncodingMacTurc		49
#define charEncodingCP853		50

// Arabic character encodings
#define charEncodingISO8859_6	51
#define charEncodingAsmo708		52
#define charEncodingCP1256		53
#define charEncodingCP864		54
#define charEncodingAsmo708Plus	55
#define charEncodingAsmo708Fr	56
#define charEncodingMacAra		57

// Simplified Chinese character encodings
#define charEncodingGB2312		58
#define charEncodingHZ			59
#define charEncodingGBK			82
#define charEncodingPalmGB		83

// Traditional Chinese character encodings
#define charEncodingBig5		60
#define charEncodingBig5_HKSCS	79
#define charEncodingBig5Plus	80
#define charEncodingPalmBig5	81
#define charEncodingISO2022CN	89

// Vietnamese character encodings
#define charEncodingViscii		61
#define charEncodingViqr		62
#define charEncodingVncii		63
#define charEncodingVietnet		65
#define charEncodingCP1258		66

// Korean character encodings
#define charEncodingKsc5601		67
#define charEncodingCP949		68
#define charEncodingISO2022Kr	69

// Hebrew character encodings
#define charEncodingISO8859_8I	70
#define charEncodingISO8859_8	71
#define charEncodingCP1255		72
#define charEncodingCP1255V		73

// Thai character encodings
#define charEncodingTis620		74
#define charEncodingCP874		64


// Leave the following line unchanged at end of section
// TAG DESTINATION END

#endif // !PALM_LOCALE_HAS_TYPES

#endif // __PALMLOCALE_H__
