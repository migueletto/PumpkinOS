/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 *
 * @ingroup SystemDef
 *
 */

/**
 * @file 	HsExtUtilBigButton.h
 * @version 3.0
 * @date 	12/18/2001
 *
 * Description:
 *	  This file contains routines for managing 'big' buttons, 
 *	  which are thumbable buttons with a bitmap and some text
 *	  (common in applications designed to be used for telephony)
 *
 *
 * History:
 *	  2001-12-18  dia	Creaded by Doug Anderson
 *	  2002-01-22  dia	Supports \r or \n as line endings now.
 *				Note that '\r\n' is not supported (it's
 *				treated as two separate line endings)
 *
 */


#ifndef _HS_EXT_UTIL_BIG_BUTTON_H
#define _HS_EXT_UTIL_BIG_BUTTON_H


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


/***************************************************************
 *  Function:    HsUtilBbutInstallFromResource
 *
 *  Summary:
 *	  Installs big buttons on the given form based on a 'bBut'
 *	  resource with the same ID.  A 'bBut' resource is basically
 *	  just an array of HsBigButtonMappingType.
 *
 *	  Once installed, the gadget will get events from PalmOS and
 *	  will sent a 'gadgetMisc' event with the selector
 *	  bbutGadgetMiscSelectSelector when selected.
 *
 *	IMPORTANT:
 *	  + The form must not be visible when you install the buttons,
 *		since we don't draw them (it turns out to be impossible to
 *		check whether the gadget is visible without peeking
 *		into its data structure, which we can only do from the
 *		event handler).
 *
 *  Parameters:
 *	  frmP	  IN	The form to install the big buttons on.
 *
 *  Returns:
 *	  nothing
 *  
 *  Called By: 
 *	  user
 *
 *  History:
 *    2001-12-17  dia	Created based on PhoneApp's CustomButton
 *	  2001-12-19  dia	Renamed, adding 'FromResource' just in
 *						case we add more install functions (and
 *						because I changed the resource slightly)
 *
 ****************************************************************/


/**
 *  @brief 
 *
 *  @param frmP:	IN:  
 *  @retval Error code.
 **/
Err
HsUtilBbutInstallFromResource (FormType* frmP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUtilBbutInstallFromResource);

/**
 *  @brief 
 *
 *  @param frmP:	IN:  
 *  @param gadgetID:	IN:  
 *  @param textP:	IN:  
 *  @param font:	IN:  
 *  @param flags:	IN:  
 *  @retval Error code.
 **/
void
HsUtilBbutInstallNewButton (FormType* frmP, UInt16 gadgetID, const Char* textP, FontID font, UInt8 flags)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUtilBbutInstallNewButton);
/**
 *  @brief 
 *
 *  @param frmP:	IN:  
 *  @param gadgetID:	IN:  
 *  @param newLabel:	IN:  
 *  @retval Error code.
 **/
void
HsUtilBbutSetLabel (FormType* frmP, UInt16 gadgetID, const Char* newLabel)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUtilBbutSetLabel);


/**
 *  @brief 
 *
 *  @param frmP:	IN: 
 *  @param gadgetID:	IN:  
 *  @param bitmapP:	IN:   
 *  @retval Error code.
 **/
void
HsUtilBbutSetBitmap (FormType* frmP, UInt16 gadgetID, const BitmapPtr bitmapP)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUtilBbutSetBitmap);

/***************************************************************
 *  Function:    HsUtilBbutHitBigButton
 *
 *  Summary:
 *	  Like CtlHitControl, but for big buttons.
 *
 *	Notes: 
 *	  + Due to the fact that PalmOS lacks accessors for
 *		gadgets directly, this function (and any others that
 *		refer to a big button's gadget) takes a form pointer
 *		and an index instead of a FormGadgetType*.
 *
 *  Parameters:
 *	  frmP			  IN	The form containing the big button's
 *							gadget.
 *	  gadgetIndex	  IN	The index of the big button's gadget.
 *
 *  Returns:
 *	  nothing
 *  
 *  Called By: 
 *	  user
 *
 *  History:
 *    2001-12-19  dia	Created.
 *
 ****************************************************************/

/**
 *  @brief 
 *
 *  @param frmP:	IN: 
 *  @param gadgetIndex:	IN:   
 *  @retval Error code.
 **/
void
HsUtilBbutHitBigButton (FormType* frmP, UInt16 gadgetIndex)
				SYS_SEL_TRAP (sysTrapHsSelector, hsSelUtilBbutHitBigButton);




#endif // _HS_EXT_UTIL_BIG_BUTTON_H
