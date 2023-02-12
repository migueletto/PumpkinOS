/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: UIResDefs.r
 *
 * Release: 
 *
 * Description:
 *		Type Declarations for Rez and DeRez
 *
 *	Note: only *all lowercase* entries are reserved.  We're OK with
 * 		the existing ones, but newly added resource types should be
 *		all downcased.
 *
 * Conditionally supported types from MPW's SysTypes.r:
 *		NFNT, FONT
 *		(If you need SysTypes.r file, include it BEFORE UIResDefs.r)
 *
 * Conditionally supported types from MPW's Types.r:
 *		ICON, MBAR, MENU, PICT
 *		(If you need Types.r file, include it BEFORE UIResDefs.r)
 *
 * Usage:
 *		 Resource files can be DeRez'ed with the following command:
 *			derez MyRsrcFile.rsrc UIResDefs.r > MyRezSource.r
 *
 *		 Source files can be Rez'ed with the following command:
 *			rez UIResDefs.r MyRezSource.r -t rsrc -c RSED -o MyRsrcFile.rsrc
 *
 * Padding:
 *		 For some reason, some cstring types are defined in the ResEdit
 *		 templates as ECST and others are CSTR.  It would appear that
 *		 Rez aligns relative to the start of the resource, which doesn't
 *		 quite jive with ResEdit's even padded cstring type.  It's not clear
 *		 that PalmOS (RezConvert.cp for example) requires even padding.
 *		 All of the affected strings occur at the end of the resource type.
 *		 If the ECST's are important, these rez templates should be tweaked.
 *
 * Currently supported types:
 *
 *		taif	// App Icon Family
 *		tbmf	// Bitmap Family
 *
 *		tAIN	// App Icon Name
 *		tAIS	// App Info Strings
 *		tver	// App Version string
 *		taic	// App Default Category
 *
 *		pref	// App Preference
 *		xprf 	// App Extended Preference
 *
 *		Talt	// Alert
 *
 *		tFRM	// Form
 *
 *		tTTL	// Form title
 *		tBTN	// Form object: button
 *		tCBX	// Form object: checkbox
 *		tFBM	// Form object: form bitmap
 *		tFLD	// Form object: field
 *		tLBL	// Form object: label
 *		tLST	// Form object: list
 *		tPBN	// Form object: push button
 *		tPUL	// Form object: popup list
 *		tPUT	// Form object: popup trigger
 *		tSLT	// Form object: selector trigger
 *		tREP	// Form object: repeating button
 *		tSCL	// Form object: scrollbar
 *		tTBL	// Form object: table
 *		tGDT	// Form object: gadget
 *		tGSI	// Form object: grafitti shift indicator
 *		tsld	// Form object: slider
 *		tslf	// Form object: feeback slider
 *		tgbn	// Form object: graphic button
 *		tgpb	// Form object: graphic push button
 *		tgrb	// Form object: graphic repeating button
 *
 *		tSTL	// String List
 *		tSTR	// String
 *		tint 	// Soft Constant
 *		wrdl 	// Word List
 *
 *		tfnf	// Font Family
 *
 *		Wave	// Wave Sound
 *
 *		tclt	// Color Table
 *
 *		tAIB	// Palm OS binary resource type for App Icon Bitmap resource
 *		Tbmp	// Palm OS binary resource type for Bitmap resource 
 *		wave	// Palm OS binary resource type for WAV sound resource
 *
 *		tRAW	// Custom resource specified as binary data
 *
 * Currently supported types for system resources:
 *
 *		feat 	// Features data
 *		silk 	// Silk Screen
 *		fnti 	// Font Index		// defined in SystemResDefsPrv.r
 *		fntm 	// Font Map			// defined in SystemResDefsPrv.r
 *
 * Other known but unsupported types:
 *		 tkbd (in keyboard.rsrc, created by MakeKbd.c.)
 *
 * Note that private resource types are defined in SystemResDefsPrv.r
 *
 *****************************************************************************/

#ifndef __PALMTYPES_R__
#define __PALMTYPES_R__


//#######################################################################################################
//#######################################################################################################


//#ifndef __SYSTYPES_R__	/* If MPW's SysTypes.r was NOT included, */
#ifndef __FONTS_R__
						/* then define NFNT and FONT */

	/*----------------------------FONT = Font Description-----------------------------------*/
	/* PROBLEMS: the offset to the offset/width table has been changed to a longint, with the
				 high word stored in the neg descent field (if its not -1).  Rez can't handle
				 this. */
	type 'FONT' {
			/* Font Type Flags */
	FontType:
			integer = 0x9000;										/* Standard Palm font */
	
		FirstChar:
			integer;												/* first char			*/
		LastChar:
			integer;												/* last char			*/
			integer;												/* width max			*/
			integer;												/* kern max				*/
			integer;												/* neg descent			*/
			integer;												/* font rect width		*/
		Height:
			integer;												/* font rect height		*/
		Offset:
			unsigned integer = ((WidthTable-Offset)/16);			/* offset to off/wid tab*/
			integer;												/* ascent				*/
			integer;												/* descent				*/
			integer;												/* leading				*/
		RowWords:
			integer;												/* row width (in words)	*/
	
			/* Tables */
			/* Bit image */
			hex string [($$Word(RowWords) <<($$BitField(FontType, 12, 2)
				& 3) + 1) * $$Word(Height)];
	
			/* Location Table */
			array [$$Word(LastChar) - $$Word(FirstChar) + 3] {
				integer;
			};
	
			/* Offset/Width Table */
		WidthTable:
			array [$$Word(LastChar) - $$Word(FirstChar) + 3] {
				integer;
			};
	
			/* Optional Character Width Table */
			Array [($$Word(FontType) & 0x0002) != 0] {
				array [$$Word(LastChar) - $$Word(FirstChar) + 3] {
					integer;
				};
			};
	
			/* Optional Image Height Table */
			Array [($$Word(FontType) & 0x0001) != 0] {
				array [$$Word(LastChar) - $$Word(FirstChar) + 3] {
					integer;
				};
			};
	};


	/*----------------------------NFNT = Font Description-----------------------------------*/
	type 'NFNT' as 'FONT';


#endif // __FONTS_R__
//#endif // __SYSTYPES_R__


//#######################################################################################################
//#######################################################################################################


/*======================================================================================*/
#ifndef __TYPES_R__		/* If MPW's Types.r was NOT included, */
						/* then define ICON, MENU, MBAR, and PICT */

#ifndef __ICONS_R__


/*----------------------------ICON = Icon-----------------------------------------------*/
type 'ICON' {
		hex string[128];										/* Icon data			*/
};


#endif


#ifndef __MENUS_R__


/*----------------------------MENU = Menu-----------------------------------------------*/
type 'MENU' {
		integer;												/* Menu ID				*/
		fill word[2];
		integer 		textMenuProc = 0;						/* ID of menu def proc	*/
		fill word;
		unsigned hex bitstring[31]
						allEnabled = 0x7FFFFFFF;				/* Enable flags 		*/
		boolean 		disabled, enabled;						/* Menu enable			*/
		pstring 		apple = "\0x14";						/* Menu Title			*/
		wide array {
				pstring;										/* Item title			*/
				byte			noIcon; 						/* Icon number			*/
				char			noKey = "\0x00",				/* Key equivalent or	*/
								hierarchicalMenu = "\0x1B";		/* hierarchical menu	*/
				char			noMark = "\0x00",				/* Marking char or id	*/
								check = "\0x12";				/* of hierarchical menu	*/
				fill bit;
				unsigned bitstring[7]
								plain;							/* Style				*/
		};
		byte = 0;
};


/*----------------------------MBAR = Menu Bar-------------------------------------------*/
type 'MBAR' {
		integer = $$CountOf(MenuArray); 						/* Number of menus		*/
		wide array MenuArray{
				integer;										/* Menu resource ID 	*/
		};
};


#endif // __MENUS_R__


#endif // __TYPES_R__


//#######################################################################################################
//#######################################################################################################


#ifndef __PICT_R__
#ifndef __PICTUTILS_R__


/*----------------------------PICT = Quickdraw Picture----------------------------------*/
type 'PICT' {
		unsigned integer;								/* Length	*/
		rect;											/* Frame	*/
		hex string; 									/* Data 	*/
};


#endif // __PICTUTILS_R__
#endif // __PICT_R__


//#######################################################################################################
//#######################################################################################################



#define alertType		informationAlert=0, confirmationAlert=1,		\
						warningAlert=2, errorAlert=3

#define palmFont		stdFont=0, boldFont=1, largeFont=2,				\
						symbolFont=3, checkboxFont=4, symbol11Font=4,	\
						symbol7Font=5, ledFont=6, largeBoldFont=7


/* Language codes (ISO 639).  The first 8 preserve the old values for the deprecated
LanguageType; the rest are sorted by the 2-character language code.

WARNING! Keep in sync with BOTH:
			1)	LanguageCode array in OverlayMgr.c
			2)	LanguageType #defines in PalmLocale.h
*/
#define localeLanguage																					\
		lEnglish=0,																						\
		lFrench,																						\
		lGerman,																						\
		lItalian,																						\
		lSpanish,																						\
																										\
		lUnused,																						\
																										\
		lJapanese,																						\
		lDutch,																							\
																										\
		lAfar,																							\
		lAbkhazian,																						\
		lAfrikaans,																						\
		lAmharic,																						\
		lArabic,																						\
		lAssamese,																						\
		lAymara,																						\
		lAzerbaijani,																					\
		lBashkir,																						\
		lByelorussian,																					\
		lBulgarian,																						\
		lBihari,																						\
		lBislama,																						\
		lBengali,																						\
		lTibetan,																						\
		lBreton,																						\
		lCatalan,																						\
		lCorsican,																						\
		lCzech,																							\
		lWelsh,																							\
		lDanish,																						\
		lBhutani,																						\
		lGreek,																							\
		lEsperanto,																						\
		lEstonian,																						\
		lBasque,																						\
		lPersian,																						\
		lFinnish,																						\
		lFiji,																							\
		lFaroese,																						\
		lFrisian,																						\
		lIrish,																							\
		lScotsGaelic,																					\
		lGalician,																						\
		lGuarani,																						\
		lGujarati,																						\
		lHausa,																							\
		lHindi,																							\
		lCroatian,																						\
		lHungarian,																						\
		lArmenian,																						\
		lInterlingua,																					\
		lInterlingue,																					\
		lInupiak,																						\
		lIndonesian,																					\
		lIcelandic,																						\
		lHebrew,																						\
		lYiddish,																						\
		lJavanese,																						\
		lGeorgian,																						\
		lKazakh,																						\
		lGreenlandic,																					\
		lCambodian,																						\
		lKannada,																						\
		lKorean,																						\
		lKashmiri,																						\
		lKurdish,																						\
		lKirghiz,																						\
		lLatin,																							\
		lLingala,																						\
		lLaothian,																						\
		lLithuanian,																					\
		lLatvian,																						\
		lMalagasy,																						\
		lMaori,																							\
		lMacedonian,																					\
		lMalayalam,																						\
		lMongolian,																						\
		lMoldavian,																						\
		lMarathi,																						\
		lMalay,																							\
		lMaltese,																						\
		lBurmese,																						\
		lNauru,																							\
		lNepali,																						\
		lNorwegian,																						\
		lOccitan,																						\
		lAfan,																							\
		lOriya,																							\
		lPunjabi,																						\
		lPolish,																						\
		lPashto,																						\
		lPortuguese,																					\
		lQuechua,																						\
		lRhaetoRomance,																					\
		lKurundi,																						\
		lRomanian,																						\
		lRussian,																						\
		lKinyarwanda,																					\
		lSanskrit,																						\
		lSindhi,																						\
		lSangho,																						\
		lSerboCroatian,																					\
		lSinghalese,																					\
		lSlovak,																						\
		lSlovenian,																						\
		lSamoan,																						\
		lShona,																							\
		lSomali,																						\
		lAlbanian,																						\
		lSerbian,																						\
		lSiswati,																						\
		lSesotho,																						\
		lSudanese,																						\
		lSwedish,																						\
		lSwahili,																						\
		lTamil,																							\
		lTelugu,																						\
		lTajik,																							\
		lThai,																							\
		lTigrinya,																						\
		lTurkmen,																						\
		lTagalog,																						\
		lSetswana,																						\
		lTonga,																							\
		lTurkish,																						\
		lTsonga,																						\
		lTatar,																							\
		lTwi,																							\
		lUkrainian,																						\
		lUrdu,																							\
		lUzbek,																							\
		lVietnamese,																					\
		lVolapuk,																						\
		lWolof,																							\
		lXhosa,																							\
		lYoruba,																						\
		lChinese,																						\
		lZulu
	
/* Country codes (ISO 3166).  The first 33 preserve the old values for the
deprecated CountryType; the rest are sorted by the 2-character country code.

WARNING! Keep in sync with BOTH:
			1)	CountryCode array in OverlayMgr.c
			2)	CountryType #defines in PalmLocale.h
*/
#define  localeCountry																					\
		cAustralia=0,																					\
		cAustria,																						\
		cBelgium,																						\
		cBrazil,																						\
		cCanada,																						\
		cDenmark,																						\
		cFinland,																						\
		cFrance,																						\
		cGermany,																						\
		cHongKong,																						\
		cIceland,																						\
		cIreland,																						\
		cItaly,																							\
		cJapan,																							\
		cLuxembourg,																					\
		cMexico,																						\
		cNetherlands,																					\
		cNewZealand,																					\
		cNorway,																						\
		cSpain,																							\
		cSweden,																						\
		cSwitzerland,																					\
		cUnitedKingdom,																					\
		cUnitedStates,																					\
		cIndia,																							\
		cIndonesia,																						\
		cRepublicOfKorea,																				\
		cMalaysia,																						\
		cChina,																							\
		cPhilippines,																					\
		cSingapore,																						\
		cThailand,																						\
		cTaiwan,																						\
																										\
		cAndorra,																						\
		cUnitedArabEmirates,																			\
		cAfghanistan,																					\
		cAntiguaAndBarbuda,																				\
		cAnguilla,																						\
		cAlbania,																						\
		cArmenia,																						\
		cNetherlandsAntilles,																			\
		cAngola,																						\
		cAntarctica,																					\
		cArgentina,																						\
		cAmericanSamoa,																					\
		cAruba,																							\
		cAzerbaijan,																					\
		cBosniaAndHerzegovina,																			\
		cBarbados,																						\
		cBangladesh,																					\
		cBurkinaFaso,																					\
		cBulgaria,																						\
		cBahrain,																						\
		cBurundi,																						\
		cBenin,																							\
		cBermuda,																						\
		cBruneiDarussalam,																				\
		cBolivia,																						\
		cBahamas,																						\
		cBhutan,																						\
		cBouvetIsland,																					\
		cBotswana,																						\
		cBelarus,																						\
		cBelize,																						\
		cCocosIslands,																					\
		cDemocraticRepublicOfTheCongo,																	\
		cCentralAfricanRepublic,																		\
		cCongo,																							\
		cIvoryCoast,																					\
		cCookIslands,																					\
		cChile,																							\
		cCameroon,																						\
		cColumbia,																						\
		cCostaRica,																						\
		cCuba,																							\
		cCapeVerde,																						\
		cChristmasIsland,																				\
		cCyprus,																						\
		cCzechRepublic,																					\
		cDjibouti,																						\
		cDominica,																						\
		cDominicanRepublic,																				\
		cAlgeria,																						\
		cEcuador,																						\
		cEstonia,																						\
		cEgypt,																							\
		cWesternSahara,																					\
		cEritrea,																						\
		cEthiopia,																						\
		cFiji,																							\
		cFalklandIslands,																				\
		cMicronesia,																					\
		cFaeroeIslands,																					\
		cMetropolitanFrance,																			\
		cGabon,																							\
		cGrenada,																						\
		cGeorgia,																						\
		cFrenchGuiana,																					\
		cGhana,																							\
		cGibraltar,																						\
		cGreenland,																						\
		cGambia,																						\
		cGuinea,																						\
		cGuadeloupe,																					\
		cEquatorialGuinea,																				\
		cGreece,																						\
		cSouthGeorgiaAndTheSouthSandwichIslands,														\
		cGuatemala,																						\
		cGuam,																							\
		cGuineaBisseu,																					\
		cGuyana,																						\
		cHeardAndMcDonaldIslands,																		\
		cHonduras,																						\
		cCroatia,																						\
		cHaiti,																							\
		cHungary,																						\
		cIsrael,																						\
		cBritishIndianOceanTerritory,																	\
		cIraq,																							\
		cIran,																							\
		cJamaica,																						\
		cJordan,																						\
		cKenya,																							\
		cKyrgyzstan,																					\
		cCambodia,																						\
		cKiribati,																						\
		cComoros,																						\
		cStKittsAndNevis,																				\
		cDemocraticPeoplesRepublicOfKorea,																\
		cKuwait,																						\
		cCaymanIslands,																					\
		cKazakhstan,																					\
		cLaos,																							\
		cLebanon,																						\
		cStLucia,																						\
		cLiechtenstein,																					\
		cSriLanka,																						\
		cLiberia,																						\
		cLesotho,																						\
		cLithuania,																						\
		cLatvia,																						\
		cLibya,																							\
		cMorrocco,																						\
		cMonaco,																						\
		cMoldova,																						\
		cMadagascar,																					\
		cMarshallIslands,																				\
		cMacedonia,																						\
		cMali,																							\
		cMyanmar,																						\
		cMongolia,																						\
		cMacau,																							\
		cNorthernMarianaIslands,																		\
		cMartinique,																					\
		cMauritania,																					\
		cMontserrat,																					\
		cMalta,																							\
		cMauritius,																						\
		cMaldives,																						\
		cMalawi,																						\
		cMozambique,																					\
		cNamibia,																						\
		cNewCalidonia,																					\
		cNiger,																							\
		cNorfolkIsland,																					\
		cNigeria,																						\
		cNicaragua,																						\
		cNepal,																							\
		cNauru,																							\
		cNiue,																							\
		cOman,																							\
		cPanama,																						\
		cPeru,																							\
		cFrenchPolynesia,																				\
		cPapuaNewGuinea,																				\
		cPakistan,																						\
		cPoland,																						\
		cStPierreAndMiquelon,																			\
		cPitcairn,																						\
		cPuertoRico,																					\
		cPortugal,																						\
		cPalau,																							\
		cParaguay,																						\
		cQatar,																							\
		cReunion,																						\
		cRomania,																						\
		cRussianFederation,																				\
		cRwanda,																						\
		cSaudiArabia,																					\
		cSolomonIslands,																				\
		cSeychelles,																					\
		cSudan,																							\
		cStHelena,																						\
		cSlovenia,																						\
		cSvalbardAndJanMayenIslands,																	\
		cSlovakia,																						\
		cSierraLeone,																					\
		cSanMarino,																						\
		cSenegal,																						\
		cSomalia,																						\
		cSuriname,																						\
		cSaoTomeAndPrincipe,																			\
		cElSalvador,																					\
		cSyranArabRepublic,																				\
		cSwaziland,																						\
		cTurksAndCaicosIslands,																			\
		cChad,																							\
		cFrenchSouthernTerritories,																		\
		cTogo,																							\
		cTajikistan,																					\
		cTokelau,																						\
		cTurkmenistan,																					\
		cTunisia,																						\
		cTonga,																							\
		cEastTimor,																						\
		cTurkey,																						\
		cTrinidadAndTobago,																				\
		cTuvalu,																						\
		cTanzania,																						\
		cUkraine,																						\
		cUganda,																						\
		cUnitedStatesMinorOutlyingIslands,																\
		cUruguay,																						\
		cUzbekistan,																					\
		cHolySee,																						\
		cStVincentAndTheGrenadines,																		\
		cVenezuela,																						\
		cBritishVirginIslands,																			\
		cUSVirginIslands,																				\
		cVietNam,																						\
		cVanuatu,																						\
		cWallisAndFutunaIslands,																		\
		cSamoa,																							\
		cYemen,																							\
		cMayotte,																						\
		cYugoslavia,																					\
		cSouthAfrica,																					\
		cZambia,																						\
		cZimbabwe
		


/*----------------------------tfnf = Font Family Description----------------------------*/

type 'tfnf'
{
	integer = 1;	// struct version
	integer = $$CountOf(DensityArray);
	wide array DensityArray 
	{
		integer;	// density
		integer;	// nfntResID
	};
};



/*######################################################################################*/


/*-------------- constants used by 'taif' and 'tbmf' ------------*/

/* previous version of definition had a misspelling, keep it for compatibility */
#define noTransprency	noTransparency


/*----------------------------tbmf = Bitmap Family ---------------------------------*/

type 'tbmf' 
{
	integer;												/* App Icon ID			*/
	integer;												/* width 				*/
	integer;												/* height 				*/
	integer = $$CountOf(PICTArray); 						/* Number of images		*/
	wide array PICTArray 
	{
		integer;											/* depth 				*/
		byte		uncompressed=0, compressed=1;			/* compression flag		*/
		fill byte;
		byte		noTransparency=0, hasTransparency=1;	/* transparency flag	*/
		fill byte;
		integer;											/* PICT id for data 	*/
		integer		compression_None		= -1,	
					compression_ScanLine	= 0,	
					compression_RLE			= 1,	
					compression_PackBits	= 2,
					compression_Best		= 100
					;										/* compression type 	*/
		hex integer;										/* transparent index 	*/

		switch
		{
			case versionZero:
				key integer = 0;
	
			case versionOne:
				key integer = 1;

				integer = 0;		/* reserved 	*/
				hex byte;			/* transparentColor.index */
				hex byte;			/* transparentColor.red */
				hex byte;			/* transparentColor.green */
				hex byte;			/* transparentColor.blue */
	
			case versionTwo:
				key integer = 2;

				integer = 0;		/* reserved 	*/
				hex byte;			/* transparentColor.index */
				hex byte;			/* transparentColor.red */
				hex byte;			/* transparentColor.green */
				hex byte;			/* transparentColor.blue */
	
				integer	kDensityNormal = 72, kDensityDouble = 144;		/* density */
		};
	};
};


/*----------------------------taif = App Icon Family ----------------------------------*/

type 'taif' as 'tbmf';


/*######################################################################################*/


/*----------------------------tclt = Color Table ----------------------------------*/

type 'tclt' {
	unsigned integer;								/* numEntries		*/
	wide array {
		unsigned byte;								/* index/reserved	*/
		unsigned hex byte;							/* red				*/
		unsigned hex byte;							/* green			*/
		unsigned hex byte;							/* blue				*/
	};
};


/*----------------------------tAIN = App Info Name ------------------------------------*/

type 'tAIN' {
	cstring;										/* App Icon Name */
	align word;										/* App Icon Name is defined in */
													/* TMPL as an even-padded cString. */
};


/*----------------------------tver = App Version --------------------------------------*/

type 'tver' {
	cstring;										/* Version String */
	align word;										/* Version String is defined in */
													/* TMPL as an even-padded cString. */
};


/*----------------------------taic = App Default Category ------------------------------*/

type 'taic' {
	cstring;										/* App Default Category Name String */
};


/*----------------------------tAIS = App Info Strings ------------------------------*/

type 'tAIS' {
													/* dmRecNumCategories = 16 categories */
	cstring;										/* Localize app info string[0] */
	cstring;										/* Localize app info string[1] */
	cstring;										/* Localize app info string[2] */
	cstring;										/* Localize app info string[3] */
	cstring;										/* Localize app info string[4] */
	cstring;										/* Localize app info string[5] */
	cstring;										/* Localize app info string[6] */
	cstring;										/* Localize app info string[7] */
	cstring;										/* Localize app info string[8] */
	cstring;										/* Localize app info string[9] */
	cstring;										/* Localize app info string[10] */
	cstring;										/* Localize app info string[11] */
	cstring;										/* Localize app info string[12] */
	cstring;										/* Localize app info string[13] */
	cstring;										/* Localize app info string[14] */
	cstring;										/* Localize app info string[15] */
};


/*----------------------------pref = App Preferences -----------------------------------*/

type 'pref' {
	integer std_priority		= 0x001E;			/* priority */
	longint std_stackSize		= 4096;				/* stackSize */
	longint std_minHeapSpace	= 4096;				/* minHeapSpace */
};


/*----------------------------xprf = App Extended Preferences --------------------------*/

type 'xprf' {
	integer = 0x0001;								/* Version number */
	fill bit[31];									/* Reserved flags */
	boolean	allowOverlays, disableOverlays;			/* T->disable overlays */
};


/*----------------------------Talt = Alert ---------------------------------------------*/

type 'Talt' {
	integer			alertType;						/* Alert Type */
	integer;										/* Help Rsc ID */
	integer;										/* # Buttons */
	integer;										/* Default Button ID */
	cstring;										/* Title */
	cstring;										/* Message */

	array ButtonArray {
		cstring;									/* Button Text */
	};
};


/*----------------------------tFRM = Form ----------------------------------------------*/

type 'tFRM' {
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			notModal=0, modal=1;			/* Modal */
	fill byte;
	byte			noSaveBehind=0, saveBehind=1;	/* Save behind */
	fill byte;
	fill word;
	fill word;
	integer;										/* Form ID */
	integer;										/* Help Rsc ID */
	integer;										/* Menu Rsc ID */
	integer;										/* Default Button ID */
	fill word;
	fill word;

	integer = $$Countof(ObjectArray);				/* Item count */
	wide array ObjectArray {
		integer;									/* Object ID */
		string[4];									/* Object Type */
	};
};


/*######################################################################################*/


/*----------------------------tBTN = Form object: button -------------------------------*/

type 'tBTN' {
	integer;										/* Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			noFrame=0, frame=1;				/* Frame */
	fill byte;
	byte			boldFrame=0, nonBoldFrame=1;	/* Non-bold frame */
	fill byte;
	byte			palmFont;						/* Font ID */
	cstring;										/* Button Label */
};


/*----------------------------tCBX = Form object: checkbox -----------------------------*/

type 'tCBX' {
	integer;										/* Check Box ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			notSelected=0, selected=1;		/* Selected */
	fill byte;
	byte;											/* Group ID */
	byte			palmFont;						/* Font ID */
	cstring;										/* Check Box Label */
	align word;										/* Check Box Label is defined in */
													/* TMPL as an even-padded cString. */
};


/*----------------------------tTTL = Form title ----------------------------------------*/

type 'tTTL' {
	cstring;										/* Title */
	align word;										/* Title string is defined in */
													/* TMPL as an even-padded cString. */
};


/*----------------------------tFBM = Form object: form bitmap --------------------------*/

type 'tFBM' {
	integer;										/* X position */
	integer;										/* Y position */
	integer;										/* Bitmap Rsc ID */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
};


/*----------------------------tFLD = Form object: field --------------------------------*/

type 'tFLD' {
	integer;											/* Field ID */
	integer;											/* Left */
	integer;											/* Top */
	integer;											/* Width */
	integer;											/* Height */
	byte			notUsable=0, usable=1;				/* Usable */
	fill byte;
	byte			notEditable=0, editable=1;			/* Editable */
	fill byte;
	byte			notUnderlined=0, underlined=1;		/* Underlined */
	fill byte;
	fill word;											/* Solid Underline (???) */
	byte			notSingleLine=0, singleLine=1;		/* Single Line */
	fill byte;
	byte			notDynamicSize=0, dynamicSize=1;	/* Dynamic Size */
	fill byte;
	byte			notLeftJustified=0, leftJustified=1;/* Left Justified */
	fill byte;
	integer;											/* Max chars */
	byte			palmFont;							/* Font ID */
	fill byte;
	byte			notAutoShift=0, autoShift=1;		/* Autoshift */
	fill byte;
	byte			notHasScrollbar=0, hasScrollbar=1;	/* Has Scrollbar */
	fill byte;
	byte			notNumeric=0, numeric=1;			/* Numeric */
	fill byte;
};


/*----------------------------tTBL = Form object: table --------------------------------*/

type 'tTBL' {
	integer;										/* Table ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notEditable=0, editable=1;		/* Editable */
	fill byte;
	fill word;										/* reserved 1 */
	fill word;										/* reserved 2 */
	fill word;										/* reserved 3 */
	integer;										/* Rows */
	integer = $$Countof(ColumnArray);				/* Columns */
	array ColumnArray {
		integer;									/* Column Width */
	};
};


/*----------------------------tGDT = Form object: gadget -------------------------------*/

type 'tGDT' {
	integer;										/* Gadget ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
};


/*----------------------------tGSI = Form object: grafitti shift indicator -------------*/

type 'tGSI' {
	integer;										/* X position */
	integer;										/* Y position */
};


/*----------------------------tLBL = Form object: label --------------------------------*/

type 'tLBL' {
	integer;										/* Label ID */
	integer;										/* Left */
	integer;										/* Top */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			palmFont;						/* Font ID */
	cstring;										/* Label */
};


/*----------------------------tLST = Form object: list ---------------------------------*/

type 'tLST' {
	integer;										/* Label ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			palmFont;						/* Font ID */
	fill byte;
	integer;										/* Visible Items */
	integer = $$Countof(StringArray);				/* Item count */
	array StringArray {
		cstring;									/* Label */
	};
};


/*----------------------------tPBN = Form object: push button --------------------------*/

type 'tPBN' {
	integer;										/* Push Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte;											/* Group ID */
	byte			palmFont;						/* Font ID */
	cstring;										/* Button Label */
};


/*----------------------------tPUL = Form object: popup list ---------------------------*/

type 'tPUL' {
	integer;										/* Control ID */
	integer;										/* List ID */
};


/*----------------------------tPUT = Form object: push trigger -------------------------*/

type 'tPUT' {
	integer;										/* Popup Trigger */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			palmFont;						/* Font ID */
	cstring;										/* Popup Label */
	
	//align word;									/* Popup Label is defined in */
	//												/* TMPL as an even-padded cString. */
};


/*----------------------------tSLT = Form object: selector trigger ---------------------*/

#define tSLT_DEFINED

type 'tSLT' {
	integer;										/* ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			palmFont;						/* Font ID */
	cstring;										/* Label */
	
	//align word;									/* Label is defined in */
	//												/* TMPL as an even-padded cString. */
};


/*----------------------------tREP = Form object: repeating button ---------------------*/

type 'tREP' {
	integer;										/* Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			noFrame=0, frame=1;				/* Frame */
	fill byte;
	byte			boldFrame=0, nonBoldFrame=1;	/* Non-bold frame */
	fill byte;
	byte			palmFont;						/* Font ID */
	cstring;										/* Button Label */
};


/*----------------------------tSCL = Form object: scrollbar ----------------------------*/

type 'tSCL' {
	integer;										/* Scroll Bar ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	integer;										/* Value */
	integer;										/* Minimum Value */
	integer;										/* Maximum Value */
	integer;										/* Page Size */
};


/*----------------------------tsld = Form object: slider ------------------------------*/

type 'tsld' {
	integer;										/* Slider ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	integer;										/* Value */
	integer;										/* Minimum Value */
	integer;										/* Maximum Value */
	integer;										/* Page Size */
	byte			horizontal=0, vertical=1;		/* Horizontal? */
	fill byte;
	integer;										/* Thumb (Tbmp) ID, 0 for default */
	integer;										/* Background (Tbmp) ID, 0 for default */
};


/*----------------------------tslf = Form object: feedback slider ---------------------*/

type 'tslf' {
	integer;										/* Feedback Slider ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	integer;										/* Value */
	integer;										/* Minimum Value */
	integer;										/* Maximum Value */
	integer;										/* Page Size */
	byte			horizontal=0, vertical=1;		/* Horizontal? */
	fill byte;
	integer;										/* Thumb (Tbmp) ID, 0 for default */
	integer;										/* Background (Tbmp) ID, 0 for default */
};


/*----------------------------tgrb = Form object: graphic repeating button -------------*/

type 'tgrb' {
	integer;										/* Graphic Repeating Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			noFrame=0, frame=1;				/* Frame */
	fill byte;
	byte			boldFrame=0, nonBoldFrame=1;	/* Non-bold frame */
	fill byte;
	integer;										/* Bitmap (Tbmp) ID */
	integer;										/* Selected Bitmap ID (0 for none) */
};


/*----------------------------tgbn = Form object: graphic button ----------------------*/

type 'tgbn' {
	integer;										/* Graphic Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte			rightAnchor=0, leftAnchor=1;	/* Left Anchor */
	fill byte;
	byte			noFrame=0, frame=1;				/* Frame */
	fill byte;
	byte			boldFrame=0, nonBoldFrame=1;	/* Non-bold frame */
	fill byte;
	integer;										/* Bitmap (Tbmp) ID */
	integer;										/* Selected Bitmap ID (0 for none) */
};


/*----------------------------tgpb = Form object: graphic push button -----------------*/

type 'tgpb' {
	integer;										/* Graphic Push Button ID */
	integer;										/* Left */
	integer;										/* Top */
	integer;										/* Width */
	integer;										/* Height */
	byte			notUsable=0, usable=1;			/* Usable */
	fill byte;
	byte;											/* Group ID */
	fill byte;
	integer;										/* Bitmap (Tbmp) ID */
	integer;										/* Selected Bitmap ID (0 for none) */
};


/*######################################################################################*/


/*----------------------------tSTR = String---------------------------------------------*/

type 'tSTR' {
	cstring;										/* The String */
													/* TMPL allows additional hex data */
};


/*----------------------------tSTL = String List----------------------------------------*/

type 'tSTL' {		// Beware - Item count is one-based, but the system routine
					// must be passed a zero-based value to retrieve strings...
	cstring;										/* Prefix */

	integer = $$Countof(StringArray);				/* Item count */
	array StringArray {
		cstring;									/* Text */
	};
};


/*----------------------------tint = Soft Constant--------------------------------------*/

type 'tint' {
	unsigned longint;								/* The Constant */
};


/*----------------------------wrdl = Word List------------------------------------------*/

type 'wrdl' {
	integer = $$Countof(IntegerArray);				/* Item count */
	array IntegerArray {
		integer;
	};
};


/*######################################################################################*/


/*----------------------------Wave = Wave Sound Description----------------------------*/

type 'Wave'
{
	literal longint = 'Wave';	// resource format ID
	integer = 1;				// resource format version

	integer = 0;				// data format = path

	pstring;					// name of .WAV file
};


/*######################################################################################*/


/*----------------------------tRAW = Custom Resource from Raw Data Description----------*/

type 'tRAW'
{
	literal longint = 'tRAW';		// resource format ID
	integer = 1;					// resource format version

	literal longint;				// Custom resource type
	integer;						// Custom resource ID
	
	integer rawDataFromFile = 0,
			rawDataInline	= 1;	// data format

	//-------------------------------------------

	// raw data from file section:
	
	pstring kNoRawDataFile = "";	// name of data file if rawDataFromFile, else empty
	align word;

	//-------------------------------------------

	// raw data inline section:
	//
	//	longint = length of data if rawDataInline, else 0
	//	hex string[ inlineDataLen ]
	
	#if 1

		// display binary data as a "hex string"
		
		#if derez

			RawDataLen:
				fill long; 	
				hex string[ $$Long(RawDataLen) ] kNoRawDataInline = $"";

		#else
		
			RawDataLen:
				longint = (RawDataInlineEnd - RawDataInlineStart) >> 3; 	
			RawDataInlineStart:
				hex string kNoRawDataInline = $"";
			RawDataInlineEnd:
	
		#endif

	#else
	
		// display binary data as an array of hex bytes

		longint = $$CountOf(RawDataInline); 	// length of data if rawDataInline, else empty
		array RawDataInline {
			unsigned hex byte;					// raw data if rawDataInline, else empty
		};
	
	#endif
};


/*######################################################################################*/


/*-----------------------------silk = system resource-----------------------------------*/

#define	alphaGraffitiSilkscreenArea			0
#define	numericGraffitiSilkscreenArea		1

type 'silk' {
	integer = 0x0001;				// version
	
	literal longint;				// vendor creator ('psys' for 3Com)
	
	integer localeLanguage;			// locale language
	
	integer localeCountry;			// locale country
	
	integer = $$Countof(RectArray);	// rectangle count
	wide array RectArray {
		point;						// topLeft of bounds.
		point;						// extent of bounds.
		
		literal longint				// area type
			screenArea = 'scrn',
			graffitiArea = 'graf';
		
		integer;					// area index
	};
	
	integer = $$Countof(ButtonArray);
	wide array ButtonArray {
		point;						// topLeft of bounds.
		point;						// extent of bounds.
		
		hex integer					// keyDown.chr
			vchrMenu = 0x0105,
			vchrLaunch = 0x0108,
			vchrKeyboard = 0x0109,
			vchrFind = 0x010a,
			vchrCalc = 0x010b,
			vchrKeyboardAlpha = 0x0110,
			vchrKeyboardNumeric = 0x0111,
			vchrTsm1 = 0x0118,
			vchrTsm2 = 0x0119,
			vchrTsm3 = 0x011a,
			vchrTsm4 = 0x011b;
		hex integer;				// keyDown.keyCode
		hex integer					// keyDown.modifiers
			commandKeyMask = 0x0008;
	};
};


/*----------------------------feat = Features ------------------------------------------*/

type 'feat' {
	integer = $$Countof(CreatorArray);
	wide array CreatorArray {
		literal longint;			// Creator, e.g. 'psys'
		integer = $$Countof(FeatureArray);
		wide array FeatureArray {
			integer;				// Feature number
			longint;				// Feature value.
		};
	};
};


//#######################################################################################################
//#######################################################################################################


/*------------------ tAIB = Palm OS App Icon Bitmap binary resource data -----------------*/

/* Warning: this template has not been updated for current Palm OS features and is not	*/
/* recommended for use.  Instead, use 'taif' App Icon Family + PICT resources to		*/
/* define your App Icon Bitmaps.														*/

type 'tAIB' {
	array Images {
				switch {
					case Uncompressed:
						integer;  						/* width;						*/
	Height:			integer; 							/* height;						*/
	RowBytes:		integer; 							/* rowBytes;					*/
						key integer = 0; 				/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffset:			integer;						/* nextDepthOffset - # of DWords to next BitmapType */
						fill byte;						/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						hex string[$$Word(RowBytes[$$ArrayIndex(Images)]) * $$Word(Height[$$ArrayIndex(Images)])];
						align LONG;
					
					case Compressed:
						integer;  						/* width;						*/
	HeightComp:		integer; 							/* height;						*/
	RowBytesComp:	integer; 							/* rowBytes;					*/
						key integer	= -32768; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffsetComp:		integer;						/* nextDepthOffset - # of DWords to next BitmapType */
						fill byte;						/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						hex string[(($$Word(NextOffsetComp[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasTransparency:
						integer;  						/* width;						*/
	HeightT:			integer; 						/* height;						*/
	RowBytesT:		integer; 							/* rowBytes;					*/
						key integer = 8192; 			/* flags;						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetT:	integer;							/* nextDepthOffset - # of DWords to next BitmapType */
						unsigned byte;					/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						hex string[$$Word(RowBytesT[$$ArrayIndex(Images)]) * $$Word(HeightT[$$ArrayIndex(Images)])];
						align LONG;
					
					case Compressed_HasTransparency:
						integer;  						/* width;						*/
	HeightCompT:		integer; 						/* height;						*/
	RowBytesCompT:	integer; 							/* rowBytes;					*/
						key integer	= -24576; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffsetCompT:integer;							/* nextDepthOffset - # of DWords to next BitmapType */
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						hex string[(($$Word(NextOffsetCompT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasColorTable:
						integer;  						/* width;						*/
	HeightCT:		integer; 							/* height;						*/
	RowBytesCT:		integer; 							/* rowBytes;					*/
						key integer = 16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCT:	integer;							/* nextDepthOffset (dword)		*/
						fill byte;						/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[$$Word(RowBytesCT[$$ArrayIndex(Images)]) * $$Word(HeightCT[$$ArrayIndex(Images)])];
						align LONG;

					case Compressed_HasColorTable:
						integer;  						/* width;						*/
	HeightCompCT:	integer; 							/* height;						*/
	RowBytesCompCT:integer; 							/* rowBytes;					*/
						key integer = -16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCompCT:	integer;						/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[(($$Word(NextOffsetCompCT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasColorTable_HasTransparency:
						integer;  						/* width;						*/
	HeightCTT:		integer; 							/* height;						*/
	RowBytesCTT:		integer; 						/* rowBytes;					*/
						key integer = 16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCTT:	integer;							/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[$$Word(RowBytesCTT[$$ArrayIndex(Images)]) * $$Word(HeightCTT[$$ArrayIndex(Images)])];
						align LONG;

					case Compressed_HasColorTable_HasTransparency:
						integer;  						/* width;						*/
	HeightCompCTT:	integer; 							/* height;						*/
	RowBytesCompCTT:integer; 							/* rowBytes;					*/
						key integer = -16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCompCTT:	integer;						/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[(($$Word(NextOffsetCompCTT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;
				};
	};
};


/*------------------ Tbmp = Palm OS Bitmap binary resource data -----------------*/

/* Warning: this template has not been updated for current Palm OS features and is not	*/
/* recommended for use.  Instead, use 'tbmf' Bitmap Family + PICT resources to			*/
/* define your Bitmaps.																	*/

type 'Tbmp' {
	array Images {
				switch {
					case Uncompressed:
						integer;  						/* width;						*/
	Height:			integer; 							/* height;						*/
	RowBytes:		integer; 							/* rowBytes;					*/
						key integer = 0; 				/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffset:			integer;						/* nextDepthOffset - # of DWords to next BitmapType */
						fill byte;						/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						hex string[$$Word(RowBytes[$$ArrayIndex(Images)]) * $$Word(Height[$$ArrayIndex(Images)])];
						align LONG;
					
					case Compressed:
						integer;  						/* width;						*/
	HeightComp:		integer; 							/* height;						*/
	RowBytesComp:	integer; 							/* rowBytes;					*/
						key integer	= -32768; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffsetComp:		integer;						/* nextDepthOffset - # of DWords to next BitmapType */
						fill byte;						/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						hex string[(($$Word(NextOffsetComp[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasTransparency:
						integer;  						/* width;						*/
	HeightT:			integer; 						/* height;						*/
	RowBytesT:		integer; 							/* rowBytes;					*/
						key integer = 8192; 			/* flags;						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetT:	integer;							/* nextDepthOffset - # of DWords to next BitmapType */
						unsigned byte;					/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						hex string[$$Word(RowBytesT[$$ArrayIndex(Images)]) * $$Word(HeightT[$$ArrayIndex(Images)])];
						align LONG;
					
					case Compressed_HasTransparency:
						integer;  						/* width;						*/
	HeightCompT:		integer; 						/* height;						*/
	RowBytesCompT:	integer; 							/* rowBytes;					*/
						key integer	= -24576; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel 		*/
						byte;							/* version;						*/
	NextOffsetCompT:integer;							/* nextDepthOffset - # of DWords to next BitmapType */
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						hex string[(($$Word(NextOffsetCompT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasColorTable:
						integer;  						/* width;						*/
	HeightCT:		integer; 							/* height;						*/
	RowBytesCT:		integer; 							/* rowBytes;					*/
						key integer = 16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCT:	integer;							/* nextDepthOffset (dword)		*/
						fill byte;						/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[$$Word(RowBytesCT[$$ArrayIndex(Images)]) * $$Word(HeightCT[$$ArrayIndex(Images)])];
						align LONG;

					case Compressed_HasColorTable:
						integer;  						/* width;						*/
	HeightCompCT:	integer; 							/* height;						*/
	RowBytesCompCT:integer; 							/* rowBytes;					*/
						key integer = -16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCompCT:	integer;						/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[(($$Word(NextOffsetCompCT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;

					case Uncompressed_HasColorTable_HasTransparency:
						integer;  						/* width;						*/
	HeightCTT:		integer; 							/* height;						*/
	RowBytesCTT:		integer; 						/* rowBytes;					*/
						key integer = 16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCTT:	integer;							/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						fill byte;						/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[$$Word(RowBytesCTT[$$ArrayIndex(Images)]) * $$Word(HeightCTT[$$ArrayIndex(Images)])];
						align LONG;

					case Compressed_HasColorTable_HasTransparency:
						integer;  						/* width;						*/
	HeightCompCTT:	integer; 							/* height;						*/
	RowBytesCompCTT:integer; 							/* rowBytes;					*/
						key integer = -16384; 			/* flags; 						*/
						byte;							/* pixelSize; bits/pixel		*/
						byte;							/* version;						*/
	NextOffsetCompCTT:	integer;						/* nextDepthOffset (dword)		*/
						unsigned byte;					/* transparent index 			*/
						byte;							/* compression type				*/
						fill word;						/* reserved						*/
						integer noColorTable = 0;		/* color table					*/
						hex string[(($$Word(NextOffsetCompCTT[$$ArrayIndex(Images)]) - 4) << 2)];
						align LONG;
				};
	};
};


//#######################################################################################################
//#######################################################################################################


#endif // __PALMTYPES_R__
