/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	palmOneStatusBar.h
 * @version 1.0
 * @date 	06/17/2004
 *
 * @brief  Public API for the palmOne Status Bar Library
 *
 *	   This file created by CS2 to get PalmStatusBar.h compiling in the presence
 *	   of Platform 5.4, which includes PalmSource's status bar manager.
 *	   All I did was add "palmOne" in some form to the front of every constant,
 *	   type, and API in the file.  As such, this file is probably not suitable
 *	   for use building the palmOne status bar library, but it is useful for
 *	   clients of that library on 5.4, as long as the actual values defined in
 *	   this file stay in synch with its neighbor, PalmStatusBar.h which is
 *	   presumably still used to build that library.
 *	   The palmOne status bar is currently an entirely separate entity from the
 *	   PalmSource status bar, so entirely disparate headers are desirable.
 * 
 * 
 * <hr>
 */


#ifndef __STATUSBARMGRLIB_H__
#define __STATUSBARMGRLIB_H__

#include <PalmTypes.h>
#include <LibTraps.h>


/** 
 * @name Library type and creator
 *
 */
/*@{*/
#define palmOneStatusBarLibName			"StatusBarMgrLib"	/**<		*/
#define palmOneStatusBarLibCreator		'sBar'			/**<		*/
#define palmOneStatusBarLibType			sysFileTLibrary		/**<		*/
#define palmOneStatusBarFtrNumVersion	(0)				/**<		*/
/*@}*/

/** 
 * @name Status Bar Errors
 *
 */
/*@{*/
#define palmOneStatusBarErrorClass		(appErrorClass  | 0x0900)		/**<		*/
#define palmOneStatusBarErrNoStatusBar		(palmOneStatusBarErrorClass | 1)	/**<		*/
#define palmOneStatusBarErrInvalidSelector	(palmOneStatusBarErrorClass | 2)	/**<		*/
#define palmOneStatusBarErrInputWindowOpen 	(palmOneStatusBarErrorClass | 3)	/**<		*/
#define palmOneStatusBarErrBadParam		(palmOneStatusBarErrorClass | 101)	/**<		*/
#define palmOneStatusBarErrInvalidState		(palmOneStatusBarErrorClass | 102)	/**<		*/
/*@}*/

/**
 * Status Bar Attributes
 **/
typedef enum PalmOneStatAttrTypeTag
{
    palmOneStatusBarAttrExists     = 0,         /**< device supports the status bar */
    palmOneStatusBarAttrBarVisible,             /**< status bar is visible */
    palmOneStatusBarAttrDimension               /**< bounds of status bar window */
} PalmOneStatAttrType;


/** 
 * @name Library Traps
 *
 */
/*@{*/
#define kPalmOneStatusBarMgrLibTrapOpen		sysLibTrapOpen			/**<		*/
#define kPalmOneStatusBarMgrLibTrapClose	sysLibTrapClose			/**<		*/
#define kPalmOneStatusBarMgrLibGetAttribute	(sysLibTrapCustom)		/**<		*/
#define kPalmOneStatusBarMgrLibHide		(sysLibTrapCustom + 1)		/**<		*/
#define kPalmOneStatusBarMgrLibShow		(sysLibTrapCustom + 2)		/**<		*/
/*@}*/

/**
 * Prototypes
 **/
#ifdef __cplusplus
extern "C" {
#endif
   
/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/
Err PalmOneStatusBarLibOpen(UInt16 refnum)
				SYS_TRAP(kPalmOneStatusBarMgrLibTrapOpen);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/				
Err PalmOneStatusBarLibClose(UInt16 refnum)
				SYS_TRAP(kPalmOneStatusBarMgrLibTrapClose);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @param selector:	IN:  
 * @param *dataP:	IN:  
 * @retval Err error code.
 **/
Err PalmOneStatusBarGetAttribute(UInt16 refnum, PalmOneStatAttrType selector, UInt32 *dataP)
				SYS_TRAP(kPalmOneStatusBarMgrLibGetAttribute);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/				
Err PalmOneStatusBarHide(UInt16 refnum)
				SYS_TRAP(kPalmOneStatusBarMgrLibHide);

/**
 * @brief
 *
 * @param refnum:	IN:  
 * @retval Err error code.
 **/				
Err PalmOneStatusBarShow(UInt16 refnum)
				SYS_TRAP(kPalmOneStatusBarMgrLibShow);

#ifdef __cplusplus
}
#endif

#endif  //__STATUSBARMGRLIB_H__
