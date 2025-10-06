/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup PmSysGadgetLib
 */

/**
 * @file 	PmSysGadgetLib.h
 * @version 1.0
 * @brief   System Library that exports status-related APIs.
 *
 * This library is based on status gadgets APIs from HsExtensions and
 * was broken apart from HsExtensions so the APIs could be ported to other platforms.
 *
 */

/*
 * $Id: //device/handheld/dev/sdk/2.1/incs/68k/libraries/pmsysgadgetlib/PmSysGadgetLib.h#5 $
 *
 */

#ifndef __PM_STATUS_LIB_H__
#define __PM_STATUS_LIB_H__


/********************************************************************
 * Prototypes
 ********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Standard library open routine
 *
 * @param	refNum: IN: Library reference number
 * @retval	Err		Error Code
 */
Err
PmSysGadgetLibOpen (UInt16 refNum)
			SYS_TRAP (kPmSysGadgetLibTrapOpen);
/**
 * @brief Standard library close routine
 *
 * @param	refNum:	IN:	Library reference number
 * @retval	Err		Error Code
 */
Err
PmSysGadgetLibClose (UInt16 refNum)
			SYS_TRAP (kPmSysGadgetLibTrapClose);

/**
 * @brief Tell the system that a gadget on the form is a certain type of status gadget so that
 *        the system can properly draw and update it.
 *
 * Tell the system that a gadget on the form is a certain type of status gadget so that
 * the system can properly draw and update it.
 * A form can only have one gadget for each status type. If more than one gadget is
 * set to a certain type, only the last one set will be recorded. This function can also be
 * used to erase and release a gadget whose type has already been set. To do this, type
 * should be set to 0.
 *
 * @param	refNum:		IN: Library reference number
 * @param	frmP:		IN: Form to draw gadget on
 * @param	gadgetID:	IN: Gadget resource ID
 * @param	type:		IN: Gadget type (battery, signal, bluetooth, etc)
 * @retval	Err			Error code
 */
Err
PmSysGadgetStatusGadgetTypeSet (UInt16 refNum, void* frmP, UInt16 gadgetID,
								UInt16 /*PmStatusGadgetTypeEnum*/ type)
			SYS_TRAP (kPmSysGadgetLibTrapStatusGadgetTypeSet);

/**
 * @brief Explicitly update all status gadgets on the current form.
 *
 * @param	refNum:	IN: Library reference number
 * @retval	Nothing
 */
void
PmSysGadgetStatusGadgetsUpdate (UInt16 refNum)
			SYS_TRAP (kPmSysGadgetLibTrapStatusGadgetsUpdate);

/// Installs big buttons on the given form based on a 'bBut'
/// resource with the same ID.  A 'bBut' resource is basically
/// just an array of HsBigButtonMappingType.
///
/// Once installed, the gadget will get events from PalmOS and
/// will sent a 'gadgetMisc' event with the selector
/// bbutGadgetMiscSelectSelector when selected.
///
///	IMPORTANT:
/// The form must not be visible when you install the buttons,
/// since we don't draw them (it turns out to be impossible to
/// check whether the gadget is visible without peeking
/// into its data structure, which we can only do from the
/// event handler).
///
/// @param refNum:	IN: Library reference number
/// @param frmP:	IN: The form to install the big buttons on.
/// @retval Err Error code.
Err
PmSysGadgetBbutInstallFromResource (UInt16 refNum, FormType* frmP)
				SYS_TRAP (kPmSysGadgetLibTrapBbutInstallFromResource);

/// Set big button on a form.
///
/// @param refNum:		IN: Library reference number
/// @param frmP:		IN: The form to set the big button's gadget on.
/// @param gadgetID:	IN: The ID of the big button's gadget.
/// @param textP:		IN: Pointer to string to set on the gadget.
/// @param font:		IN: Font ID to be used for the text.
/// @param flags:		IN: Flags associated to the gadget.
/// @retval None.
void
PmSysGadgetBbutInstallNewButton (UInt16 refNum, FormType* frmP, UInt16 gadgetID,
								 const Char* textP, FontID font, UInt8 flags)
				SYS_TRAP (kPmSysGadgetLibTrapBbutInstallNewButton);

/// Set label on a big button gadget.
///
/// @param refNum:		IN: Library reference number
/// @param frmP:		IN: The form containing the big button's gadget.
/// @param gadgetID:	IN: The ID of the big button's gadget.
/// @param newLabel:	IN: Pointer to label string to set on the gadget.
/// @retval None.
void
PmSysGadgetBbutSetLabel (UInt16 refNum, FormType* frmP, UInt16 gadgetID, const Char* newLabel)
				SYS_TRAP (kPmSysGadgetLibTrapBbutSetLabel);

/// Set bitmap image on a big button gadget.
///
/// @param refNum:		IN: Library reference number
/// @param frmP:		IN: The form containing the big button's gadget.
/// @param gadgetID:	IN: The ID of the big button's gadget.
/// @param bitmapP:		IN: Pointer to the bitmap to set on the gadget.
/// @retval None.
void
PmSysGadgetBbutSetBitmap (UInt16 refNum, FormType* frmP, UInt16 gadgetID, const BitmapPtr bitmapP)
				SYS_TRAP (kPmSysGadgetLibTrapBbutSetBitmap);

/// Like CtlHitControl, but for big buttons.
///
/// Notes:
/// Due to the fact that PalmOS lacks accessors for
/// gadgets directly, this function (and any others that
/// refer to a big button's gadget) takes a form pointer
/// and an index instead of a FormGadgetType*.
///
/// @param frmP:		IN: The form containing the big button's gadget.
/// @param gadgetIndex:	IN: The index of the big button's gadget.
/// @retval None.
void
PmSysGadgetBbutHitBigButton (UInt16 refNum, FormType* frmP, UInt16 gadgetIndex)
				SYS_TRAP (kPmSysGadgetLibTrapBbutHitBigButton);

#ifdef __cplusplus
}
#endif

#endif  // __PM_STATUS_LIB_H__
