//**************************************************************
//
// Project:
//    Handspring PalmOS System Extensions
//
// Copyright info:
//    Copyright 2001 Handspring, Inc. All Rights Reserved.   
//
// FileName:
//    HsExtUtilBigButtonRsc.h
// 
// Description:
//	  This file contains resource #defines for 
//	  HsExtUtilBigButton.h and is suitable for including in
//	  '.rcp' files.
//
// ToDo:
//
// History:
//	  2001-12-18  dia	Creaded by Doug Anderson
//	  2002-01-03  dia	We now have a reserved range to keep
//						real bitmaps and strings in the resource
//						chain, so we don't need the flags to tell 
//						us whether the user wanted ours; moved
//						resource IDs to HsResourceIDs.h.
//
//***************************************************************/


#ifndef __HS_EXT_UTIL_BIG_BUTTON_RSC__H__

#define __HS_EXT_UTIL_BIG_BUTTON_RSC__H__ 1

// Here's a sample usage:
// HEX "bBut" rscBigYesNoDialogFormID
//	  hsBigButtonMappingVersion0.w				// version
//
//	  hsBigButtonFlagNone						// flags
// 	  hsBigButtonFontStd						// font
// 	  rscBigYesNoDialogYesButtonID.w			// gadgetID
//	  hsBigButtonGreenCheckBitmapID.w			// bitmapID
//	  hsBigButtonYesStringID.w					// stringID
//
//	  hsBigButtonFlagNone						// flags
//	  hsBigButtonFontStd						// font
//	  rscBigYesNoDialogNoButtonID.w				// gadgetID
//	  hsBigButtonRedXBitmapID.w					// bitmapID
//	  hsBigButtonNoStringID.w					// stringID



// Use this version
// ----------------
#define hsBigButtonMappingVersion0			0


// Use these flags
// ---------------
#define hsBigButtonFlagNone					0x00
#define hsBigButtonFlagNoBorder				0x01  // Don't draw the border...
#define hsBigButtonFlagNoBackground			0x02  // Don't draw the background of the button
#define hsBigButtonFlagNoBorderOrBackground	0x03  // Combination for Palm-rc...

// Use these fonts
// ---------------
#define hsBigButtonFontStd					0
#define hsBigButtonFontBold					1
#define hsBigButtonFontLarge				2
#define hsBigButtonFontSymbol				3
#define hsBigButtonFontSymbol11				4
#define hsBigButtonFontSymbol7				5
#define hsBigButtonFontLed					6
#define hsBigButtonFontLargeBold			7



#endif // ifndef __HS_EXT_UTIL_BIG_BUTTON_RSC_H__
