/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup PmUIUtilLib
 */

/**
 * @file  PmUIUtilLib.h
 * @brief This is the public header for PmUIUtil library.
 *
 * This library contains general utility functions that supplement
 * OS functionality.  That is, it contains functions that
 * are general enough to be useful to many applications but
 * weren't included in the OS utility functions for whatever
 * reason.
 *
 */

#ifndef __PM_UI_UTIL_LIB_H__
#define __PM_UI_UTIL_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#pragma mark --------   Basic Lib API  ----------------
#endif

/**
 * @brief Standard library open routine
 *
 * @param	refNum:	IN: Library reference number
 * @retval	Err		Error code
 */
Err
PmUIUtilLibOpen (UInt16 refNum)
			SYS_TRAP (kPmUIUtilLibTrapOpen);

/**
 * @brief Standard library close routine
 *
 * @param	refNum:	IN: Library reference number
 * @retval	Err		Error code
 */
Err
PmUIUtilLibClose (UInt16 refNum)
			SYS_TRAP (kPmUIUtilLibTrapClose);



#if 0
#pragma mark -------- List API --------
#endif

void
PmUIUtilLstSetTempSelection (UInt16 refNum, const FormType* formP, UInt16 listID, Int16 itemNum)
			SYS_TRAP (kPmUIUtilLibTrapLstSetTempSelection);

Int16
PmUIUtilLstGetTempSelection (UInt16 refNum, const FormType* formP, UInt16 listID)
			SYS_TRAP (kPmUIUtilLibTrapLstGetTempSelection);


#if 0
#pragma mark -------- Attr API --------
#endif

Err
PmUIUtilAttrGet (UInt16 refNum, UInt16 /*PmUIUtilAttrEnum*/ attr, UInt32 flags, UInt32* valueP)
			SYS_TRAP (kPmUIUtilLibTrapAttrGet);

Err
PmUIUtilAttrSet (UInt16 refNum, UInt16 /*PmUIUtilAttrEnum*/ attr, UInt32 flags, UInt32 value)
			SYS_TRAP (kPmUIUtilLibTrapAttrSet);


#if 0
#pragma mark -------- PalmOne-Specific Nav API --------
#endif

void
PmUIUtilNavGetFocusColor (UInt16 refNum, HsNavFocusColorEnum color, RGBColorType* rgbColorP)
			SYS_TRAP (kPmUIUtilLibTrapNavGetFocusColor);

void
PmUIUtilNavSetFocusColor (UInt16 refNum, HsNavFocusColorEnum color, RGBColorType* rgbColorP,
						  RGBColorType* oldRgbColorP)
			SYS_TRAP (kPmUIUtilLibTrapNavSetFocusColor);

#ifdef __cplusplus
}
#endif

#endif // __PM_UI_UTIL_LIB_H__
