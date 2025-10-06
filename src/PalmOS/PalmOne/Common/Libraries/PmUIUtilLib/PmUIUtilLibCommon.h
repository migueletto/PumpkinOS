/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	PmUIUtilLib UI Utility Library
 * @brief		This library provides support for the misc. UI-related functionalities.
 *
 * Applications can use this library API to retrieve the different layers of colors of
 * the focus ring and use the colors to manually draw custom-shaped focus rings. Please
 * also refer to the HsNav.h header file for more of the library constants and definitions.
 *
 * @{
 * @}
 */
/**
 * @ingroup PmUIUtilLib
 */

/**
 * @file  PmUIUtilLibCommon.h
 * @brief Public 68k common header file for system Library that exports UI utility API functions.
 *
 * This file contains the constants and definitions for the system UI-Util library.
 * <hr>
 */

#ifndef __PM_UI_UTIL_LIB_COMMON_H__
#define __PM_UI_UTIL_LIB_COMMON_H__


/********************************************************************
 * Constants and Enums
 ********************************************************************/

#define kPmUIUtilLibType		sysFileTLibrary		/**< Library type */
#define kPmUIUtilLibCreator		pmFileCUIUtilLib	/**< Library creator ID */
#define kPmUIUtilLibName		"PmUIUtilLib-PmUU"	/**< Library name */


enum PmUIUtilAttrEnumTag
{
  pmUIUtilAttrAutoSnoozeAttns		/**< whether we want to auto snooze the current instance of the attn dialog */

  /*
   * Attributes greater than 0x9000 are defined for internal use in Prv/PmPrvKeyLibCommon.h
   */
};

/** @see PmUIUtilAttrEnumTag */
typedef UInt16 PmUIUtilAttrEnum;


/********************************************************************
 * Traps
 ********************************************************************/

/**
 * @name Function Traps
 */
/*@{*/
#define kPmUIUtilLibTrapOpen					  sysLibTrapOpen
#define kPmUIUtilLibTrapClose					  sysLibTrapClose
#define kPmUIUtilLibTrapGetObjectUsable			  (sysLibTrapCustom)
#define kPmUIUtilLibTrapPutObjectAfterTitle		  (sysLibTrapCustom+1)
#define kPmUIUtilLibTrapFrmDoDialogWithCallback	  (sysLibTrapCustom+2)
#define kPmUIUtilLibTrapFrmDoDialog				  (sysLibTrapCustom+3)
#define kPmUIUtilLibTrapFrmDoTimedDialogEx		  (sysLibTrapCustom+4)
#define kPmUIUtilLibTrapFrmSetHelpID			  (sysLibTrapCustom+5)
#define kPmUIUtilLibTrapFrmSetLabel				  (sysLibTrapCustom+6)
#define kPmUIUtilLibTrapFrmSetLabelFont			  (sysLibTrapCustom+7)
#define kPmUIUtilLibTrapFrmGetDefaultButtonID	  (sysLibTrapCustom+8)
#define kPmUIUtilLibTrapFrmGetEventHandler		  (sysLibTrapCustom+9)
#define kPmUIUtilLibTrapCtlGetStyle				  (sysLibTrapCustom+10)
#define kPmUIUtilLibTrapCtlGetFont				  (sysLibTrapCustom+11)
#define kPmUIUtilLibTrapLstGetItemsText			  (sysLibTrapCustom+12)
#define kPmUIUtilLibTrapLstGetTopItem			  (sysLibTrapCustom+13)
#define kPmUIUtilLibTrapLstPopupListExt			  (sysLibTrapCustom+14)
#define kPmUIUtilLibTrapSclUpdateScrollBar		  (sysLibTrapCustom+15)
#define kPmUIUtilLibTrapSclScroll				  (sysLibTrapCustom+16)
#define kPmUIUtilLibTrapSclScrollPage			  (sysLibTrapCustom+17)
#define kPmUIUtilLibTrapSliHandleKeyDown		  (sysLibTrapCustom+18)
#define kPmUIUtilLibTrapWinGetMaxDepth			  (sysLibTrapCustom+19)
#define kPmUIUtilLibTrapAttnNagInfo				  (sysLibTrapCustom+20)
#define kPmUIUtilLibTrapAttnEnable				  (sysLibTrapCustom+21)
#define kPmUIUtilLibTrapAttnIndicatorAllow		  (sysLibTrapCustom+22)
#define kPmUIUtilLibTrapAttnGetAttentionExt		  (sysLibTrapCustom+23)
#define kPmUIUtilLibTrapAttnUpdateExt			  (sysLibTrapCustom+24)
#define kPmUIUtilLibTrapAttrGet					  (sysLibTrapCustom+25)
#define kPmUIUtilLibTrapAttrSet					  (sysLibTrapCustom+26)
#define kPmUIUtilLibTrapLstSetTempSelection		  (sysLibTrapCustom+27)
#define kPmUIUtilLibTrapLstGetTempSelection		  (sysLibTrapCustom+28)
#define kPmUIUtilLibTrapNavGetFocusColor		  (sysLibTrapCustom+29)
#define kPmUIUtilLibTrapNavSetFocusColor		  (sysLibTrapCustom+30)
/*@}*/

#endif // __PM_UI_UTIL_LIB_COMMON_H__
