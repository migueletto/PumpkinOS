/**
 * \file HsExtUtilBigButtonCommon.h
 *
 * <Common code for Big Buttons that can be used by ARM and 68K code.>
 *
 * <This file contains all defines and enums that are used by Big Buttons.
 * There are some structs here too that might need to be moved to 68K/ARM
 * specific code. Moved code from HsExtUtilBigButton.h >
 *
 * \license
 *
 *    Copyright (c) 2002 Handspring Inc., All Rights Reserved
 *
 * \author  Arun Mathias
 *
 * $Id: //device/handheld/dev/sdk/2.1/incs/common/system/HsExtUtilBigButtonCommon.h#4 $
 *
 *****************************************************************************/

#ifndef __HS_EXT_UTIL_BIG_BUTTON_COMMON_H__

#define __HS_EXT_UTIL_BIG_BUTTON_COMMON_H__


/**
 * This selector gets sent with a frmGadgetMiscEvent to let you know that the
 * gadget has been selected.  The numbers are picked semi-randomly so that generic
 * code (like HsUtilFrmDoDialogWithCallback()) can tell that this is actually 
 * a button press without doing the wrong thing for other people's gadget events.
 *
 * Note that in most code, you shouldn't need to worry about checking the data,
 * since you know exactly what gadgets are on your form and what kind of events
 * they send.
 **/
#define bbutGadgetMiscSelectSelector  7014					/**< Randomish number so people can
															   easily detect big button presses */
#define bbutGadgetMiscSelectData	  ((void*)0xBADC0DE1)	/**< Magic # to make conflicts even less
															   likely for  detecting big  button presses.*/

								 
/**
 * This is the type of big button resources.  The ID should match the ID of
 * the form...
 **/
#define hsBigButtonRscType					'bBut'		   /**< Registered: yes */


/**
 * This is the structure of a single big button mapping...
 **/
typedef struct
  {
	UInt8 flags;
	UInt8 font;

	UInt16 gadgetID;

	UInt16 bitmapID;
	UInt16 stringID;
  }
HsBigButtonMappingType;

/**
 * This is the structure of a big button resource...
 **/
typedef struct
  {
	UInt16 version;
	HsBigButtonMappingType mapArr[0];
  }
HsBigButtonReourceType;

#endif // ifndef __HS_EXT_UTIL_BIG_BUTTON_COMMON_H__
