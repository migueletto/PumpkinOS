/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** @ingroup STE
 *
 */

/**
 * @file 	SmartTextEngineDef.h
 *
 * @brief Public include file for Smart Text Engine shared library
 *
 * The calling application should always load this library with
 * SysLibLoad() before use, even if it is already open by another
 * application(ie, SysLibFind() returns a valid refnum). When
 * the application is done with the library, it should be
 * unloaded with SysLibRemove(). We do this because there is
 * no good way to synchronize loading and unloading of libraries
 * among multiple applications. It also greatly simplifies internal
 *
 */


#ifndef _SMARTTEXTENGINEDEF_H_
#define _SMARTTEXTENGINEDEF_H_

#include <PalmOS.h>

#include "Common/Libraries/SmartTextEngine/SmartTextEngineErrors.h"
#include "Common/Libraries/SmartTextEngine/SmartTextEngineRsc.h"


#ifdef __cplusplus
extern "C" {
#endif


#define steLibName			"SmartTextEngine.lib"	   /**< Internal library name which can(but shouldn't) be passed to SysLibFind() */
#define steLibDBName		"Smart Text Engine" 	   /**< Name used for DmFindDatabase */


/**
 * @name Parsed types
 *
 */
/*@{*/
#define kParsedPhoneNumber		  1
#define kParsedURL				  2
#define kParsedEmail			  3
/*@}*/

#define kSmartTextBit			 0x4000 /**<No definition */

/**
 * @name Font types
 *
 */
/*@{*/
#define kSTEStdFont 			  (kSmartTextBit | 0x00)
#define kSTEBoldFont			  (kSmartTextBit | 0x01)
/*@}*/

/**
 * @name Font colors
 *
 */
/*@{*/
#define kSTECurrentFont 		  (kSmartTextBit | 0x10)	/**<Current font color*/
#define kSTEBlackFont			  (kSmartTextBit | 0x11)	/**<Black font color*/
#define kSTEBlueFont			  (kSmartTextBit | 0x12)	/**<Blue font color*/
#define kSTERedFont 			  (kSmartTextBit | 0x13)	/**<Red font color*/
#define kSTEGreenFont			  (kSmartTextBit | 0x14)	/**<Green font color*/
#define kSTEYellowFont			  (kSmartTextBit | 0x15)	/**<Yellow font color*/
#define kSTEPurpleFont			  (kSmartTextBit | 0x16)	/**<Purple font color*/
#define kSTEOrangeFont			  (kSmartTextBit | 0x17)	/**<Orange font color*/
#define kSTEGrayFont			  (kSmartTextBit | 0x18)	/**<Gray font color*/
/*@}*/

/**
 * @name Formating
 *
 */
/*@{*/
#define kSTELineBreak			  (kSmartTextBit | 0x50)	/**<Line Break**/
#define kSTELeftIndent			  (kSmartTextBit | 0x51)	/**<Left Indentation*/
#define kSTERightAlign			  (kSmartTextBit | 0x52)	/**<Right Alignment*/
#define kSTECenterAlign 		  (kSmartTextBit | 0x53)	/**<Center Alignment*/

#define kSTEHorizontalLine		  (kSmartTextBit | 0x60)	/**<Horizontal Line*/

#define kSTEBitmap				  (kSmartTextBit | 0x70)	/**<Bitmap*/
#define kSTESmileyBitmap		  (kSmartTextBit | 0x71)	/**<Smiley Bitmap*/
#define kSTECharacterBitmap 	  (kSmartTextBit | 0x72)	/**<Character Bitmap*/

#define kSTEHyperlink			  (kSmartTextBit | 0x80)	/**<Hyperlink*/
/*@}*/

/**
 * @name Emoticons
 *
 */
/*@{*/
#define kSmileSmiley			  (((UInt32)LargeSmiley00BitMap << 16) | Smiley00BitMap)	/**<Smiling Smily*/
#define kWinkSmiley 			  (((UInt32)LargeSmiley01BitMap << 16) | Smiley01BitMap)	/**<Winking Smily*/
#define kFrownSmiley			  (((UInt32)LargeSmiley02BitMap << 16) | Smiley02BitMap)	/**<Frowning Smily*/
#define kBigTeethSmiley 		  (((UInt32)LargeSmiley03BitMap << 16) | Smiley03BitMap)	/**<Big Teeth Smily*/
#define kTongueSmiley			  (((UInt32)LargeSmiley04BitMap << 16) | Smiley04BitMap)	/**<Tongue Smily*/
#define kDevilSmiley			  (((UInt32)LargeSmiley05BitMap << 16) | Smiley05BitMap)	/**<Devil Smily*/
#define kOMouthSmiley			  (((UInt32)LargeSmiley06BitMap << 16) | Smiley06BitMap)	/**<O-mouth Smily*/
#define kBigTongueSmiley		  (((UInt32)LargeSmiley07BitMap << 16) | Smiley07BitMap)	/**<Big Tongue Smily*/
#define kHeartSmiley			  (((UInt32)LargeSmiley08BitMap << 16) | Smiley08BitMap)	/**<Heart Smily*/
#define kBigFrownSmiley 		  (((UInt32)LargeSmiley09BitMap << 16) | Smiley09BitMap)	/**<Big Frown Smily*/
#define kQuestionSmiley 		  (((UInt32)LargeSmiley10BitMap << 16) | Smiley10BitMap)	/**<Question Smily*/
#define kPukeSmiley 			  (((UInt32)LargeSmiley11BitMap << 16) | Smiley11BitMap)	/**<Puke Smily*/
#define kBigSmileSmiley 		  (((UInt32)LargeSmiley12BitMap << 16) | Smiley12BitMap)	/**<Big Smile Smily*/
#define kBlushSmiley			  (((UInt32)LargeSmiley13BitMap << 16) | Smiley13BitMap)	/**<Blush Smily*/
#define kSmirkSmiley			  (((UInt32)LargeSmiley14BitMap << 16) | Smiley14BitMap)	/**<Smirk Smily*/
#define kClownSmiley			  (((UInt32)LargeSmiley15BitMap << 16) | Smiley15BitMap)	/**<Clown Smily*/
#define kStraightMouthSmiley	  (((UInt32)LargeSmiley16BitMap << 16) | Smiley16BitMap)	/**<Straight Mouth Smily*/
#define kYinYangSmiley			  (((UInt32)LargeSmiley17BitMap << 16) | Smiley17BitMap)	/**<Yin Yang Smily*/
#define kAngelSmiley			  (((UInt32)LargeSmiley18BitMap << 16) | Smiley18BitMap)	/**<Angel Smily*/
#define kCigarSmiley			  (((UInt32)LargeSmiley19BitMap << 16) | Smiley19BitMap)	/**<Cigar Smily*/
#define kFlipSmiley 			  (((UInt32)LargeSmiley20BitMap << 16) | Smiley20BitMap)	/**<Flip Smily*/
#define kUFlipSmiley			  (((UInt32)LargeSmiley21BitMap << 16) | Smiley21BitMap)	/**<Up Flip Smily*/
#define kLFlipSmiley			  (((UInt32)LargeSmiley22BitMap << 16) | Smiley22BitMap)	/**<Left Flip Smily*/
#define kRFlipSmiley			  (((UInt32)LargeSmiley23BitMap << 16) | Smiley23BitMap)	/**<Right Flip Smily*/


#define kDeltaCharacter 		  (((UInt32)Delta14BitMap << 16)  | Delta11BitMap)	/**<Delta Character*/
#define kPhiCharacter			  (((UInt32)Phi14BitMap << 16)	  | Phi11BitMap)	/**<Phi Character*/
#define kGammaCharacter 		  (((UInt32)Gamma14BitMap << 16)  | Gamma11BitMap)	/**<Gamma Character*/
#define kLambdaCharacter		  (((UInt32)Lambda14BitMap << 16) | Lambda11BitMap)	/**<Lambda Character*/
#define kOmegaCharacter 		  (((UInt32)Omega14BitMap << 16)  | Omega11BitMap)	/**<Omega Character*/
#define kPiCharacter			  (((UInt32)Pi14BitMap << 16)	  | Pi11BitMap)		/**<Pi Character*/
#define kPsiCharacter			  (((UInt32)Psi14BitMap << 16)	  | Psi11BitMap)	/**<Psi Character*/
#define kSigmaCharacter 		  (((UInt32)Sigma14BitMap << 16)  | Sigma11BitMap)	/**<Sigma Character*/
#define kThetaCharacter 		  (((UInt32)Theta14BitMap << 16)  | Theta11BitMap)	/**<Theta Character*/
#define kXiCharacter			  (((UInt32)Xi14BitMap << 16)	  | Xi11BitMap)		/**<Xi Character*/
/*@}*/

/**
 * @name Link colors (from the CLUT)
 *
 */
/*@{*/
#define kSTECurrent 		254 // taking a chance no one will use this index
#define kSTEBlack			255	/**<Black*/
#define kSTEBlue			95	/**<Blue*/
#define kSTERed 			125 /**<Red*/
#define kSTEGreen			210	/**<Green*/
#define kSTEYellow			120	/**<Yellow*/
#define kSTEPurple			23	/**<Purple*/
#define kSTEOrange			116	/**<Orange*/
#define kSTEGray			220	/**<Gray*/
/*@}*/

/**
 * @name Inverse Color
 *
 */
/*@{*/
#define kSTEBlackInverse	0	/**<Black Inverse*/
#define kSTEBlueInverse 	114	/**<Blue Inverse*/
#define kSTERedInverse		8	/**<Red Inverse*/
#define kSTEGreenInverse	18	/**<Green Inverse*/
#define kSTEYellowInverse	6	/**<Yellow Inverse*/
#define kSTEPurpleInverse	1	/**<Purple Inverse*/
#define kSTEOrangeInverse	30	/**<Orange Inverse*/
#define kSTEGrayInverse 	25	/**<Gray Inverse*/
/*@}*/


// Smart Text delimiters

/**
 * @name Use standard or bold font
 *
 */
/*@{*/
#define steStdFont			"//STESTDFONT//"	/**<Standard Font*/
#define steBoldFont 		"//STEBOLDFONT//"	/**<Bold Font*/
/*@}*/

/**
 * @name Font colors
 *
 */
/*@{*/
#define steCurrentFont		"//STECURRENTFONT//" /**<Current font color*/
#define steBlackFont		"//STEBLACKFONT//"	/**<Black font color*/
#define steBlueFont 		"//STEBLUEFONT//"	/**<Blue font color*/
#define steRedFont			"//STEREDFONT//"	/**<Red font color*/
#define steGreenFont		"//STEGREENFONT//"	/**<Green font color*/
#define steYellowFont		"//STEYELLOWFONT//"	/**<Yellow font color*/
#define stePurpleFont		"//STEPURPLEFONT//"	/**<Purple font color*/
#define steOrangeFont		"//STEORANGEFONT//"	/**<Orange font color*/
#define steGrayFont 		"//STEGRAYFONT//"	/**<Gray font color*/
/*@}*/

/**
 * @name Used for formatting
 *
 */
/*@{*/
#define steLeftIndent		"//STELEFTINDENT="	/**<Left Indentation*/
#define steRightAlign		"//STERIGHTALIGN="	/**<Right Indentation*/
#define steCenterAlign		"//STECENTERALIGN//"	/**<Center Alignment*/
/*@}*/

#define steBitmap			"//STEBITMAP=" /**< Graphics support */

/**
 * @name Miscellaneous
 *
 */
/*@{*/
#define steHorizontalLine	"//STEHORIZONTALLINE//"	/**<Horizontal Line*/
#define steLineBreak		"//STELINEBREAK//"	/**<Line Break*/
/*@}*/

// Custom link that calls a callback procedure when selected
#define steHyperlink		"//STEHYPERLINK="	/**<Hyperlink*/

// use this parameter to scroll to the end of the list
#define kScrollToEnd				0xFFFF	/**< Used to scroll to end*/

/**
 * @name Text Selection
 *
 */
/*@{*/
#define kSelectUntilEndOfRecord		0xFFFFFFFE	/**<Select till end of record*/
#define kSelectUntilEnd 			0xFFFFFFFF	/**<Select until End*/
/*@}*/

/**
 * @name Initializing the Smart Text Engine
 *
 */
/*@{*/
#define steParsePhoneNumbers		0x00000001	/**<Parse Phone Numbers*/
#define steParseURLs				0x00000002	/**<Parse URLs*/
#define steParseEmail				0x00000004	/**<parse Email*/
#define steParseEmoticons			0x00000008	/**<parse Emotions*/
#define steLargeFont			    0x00000010	/**<Large Font*/
#define steCannotSelectText 		0x00000020	/**<Cannot Select Text*/
#define steParseMoreEmoticons	    0x00000040	/**<Parse More Emoticons*/
#define steScrollWalksLinks 		0x00000080	/**<Scroll Walks links*/
#define steInvertHighlighting		0x00000100	/**<Invert highlighting*/
#define steNoLinksBeforeColon		0x00000200	/**<No links before colon*/
#define steAcceleratedScrolling		0x00000400	/**<No Accelerated Scrolling*/
#define steAllowShortPhoneNumbers	0x00000800	/**<Allow Short Phone Numbers*/
#define steParseGSMGreekChars		0x00001000	/**<Parse GSM Greek Characters*/
#define steDoNotCenterVertically	0x00002000	/**<Do not center vertically*/
#define steScrollWalksMessages 		0x00004000	/**<Scroll walks messages*/
#define steTextMustBeVisible 		0x00008000	/**<If true, then any routine that returns selected text must verify the text is visible first*/
#define steAllowPhoneNumberWords	0x00010000	/**<If true, then the phone number parser will allow numbers like 1-800-TESTING*/
/*@}*/

/** Used for custom hyperlinks */
typedef void (*STECallbackType) (UInt32 refNum);


/**
 * @brief Holds parsed item information.
 *
 * Given a message, or list of messages,
 * this structure gives the locations of all parsed items.
 */
typedef struct
{
  UInt16  parsedType;		  /**< what kind of data did we find?*/
  UInt16  startLocation;	  /**< the starting location of the parsed data we found*/
  UInt16  itemLength;		  /**< the character length of the parsed data we found*/
  UInt16  msgNumber;		  /**< when parsing a list of message, this is the message number, otherwise 0*/
  UInt16  color;			  /**< used to store the font color (for custom hyperlinks)*/
  UInt16  nameLength;		  /**< used to store the length of the link name (for custom hyperlinks)*/
  UInt32  miscData; 		  /**< used now to store left indentation and other info.  Adapt as needed.*/
} ParsedInfo;

/**
 * @brief Holds the list of parsed item information.
 */
typedef struct
{
  UInt16		size;		  /**< how many items did we find in the text?*/
  UInt16		maxSize;	  /**< maximum number of items that can be added without resizing*/
  ParsedInfo	info[1];	  /**< parsing info follows*/
} ParsedInfoList;


#ifdef __cplusplus
}
#endif


#endif // _SMARTTEXTENGINEDEF_H_
