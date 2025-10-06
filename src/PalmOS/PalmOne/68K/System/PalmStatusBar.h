/******************************************************************************
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/** 
 *@ingroup System
 *
 */
 
/**
 * @file 	PalmStatusBar.h
 * @version 1.0
 * @date 	03/03/2003
 *
 * @brief  Public API for the Status Bar Library
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
#define statLibName		"StatusBarMgrLib"	/** 		*/
#define statLibCreator		'sBar'			/** 		*/
#define statLibType		sysFileTLibrary		/** 		*/
#define statFtrNumVersion	(0)			/** 		*/
/*@}*/

/** 
 * @name Status Bar Errors
 *
 */
/*@{*/
#define statErrorClass		(appErrorClass  | 0x0900)	/** 		*/
#define statErrNoStatusBar	(statErrorClass | 1)		/** 		*/
#define statErrInvalidSelector	(statErrorClass | 2)		/** 		*/
#define statErrInputWindowOpen 	(statErrorClass | 3)		/** 		*/
#define statErrBadParam		(statErrorClass | 101)		/** 		*/
#define statErrInvalidState	(statErrorClass | 102)		/** 		*/
/*@}*/

/**
 * Status Bar Attributes
 **/
typedef enum StatAttrTypeTag
{
    statAttrExists     = 0,         /**< device supports the status bar */
    statAttrBarVisible,             /**< status bar is visible */
    statAttrDimension               /**< bounds of status bar window */
} StatAttrType;

/** 
 * @name Library Traps
 *
 */
/*@{*/
#define kStatusBarMgrLibTrapOpen	sysLibTrapOpen		/** 		*/
#define kStatusBarMgrLibTrapClose	sysLibTrapClose		/** 		*/
#define kStatusBarMgrLibGetAttribute	(sysLibTrapCustom)	/** 		*/
#define kStatusBarMgrLibHide		(sysLibTrapCustom + 1)	/** 		*/
#define kStatusBarMgrLibShow		(sysLibTrapCustom + 2)	/** 		*/
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
 * @param refnum: 	IN:  
 * @retval Err error code.
 **/
Err StatLibOpen(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibTrapOpen);

/**
 * @brief
 *
 * @param refnum: 	IN:  
 * @retval Err error code.
 **/				
Err StatLibClose(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibTrapClose);

/**
 * @brief
 *
 * @param refnum: 	IN:  
 * @param selector:	IN:  
 * @param *dataP: 	IN:  
 * @retval Err error code.
 **/
Err StatGetAttribute(UInt16 refnum, StatAttrType selector, UInt32 *dataP)
				SYS_TRAP(kStatusBarMgrLibGetAttribute);

/**
 * @brief
 *
 * @param refnum: 	IN:  
 * @retval Err error code.
 **/				
Err StatHide(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibHide);

/**
 * @brief
 *
 * @param refnum: 	IN:  
 * @retval Err error code.
 **/				
Err StatShow(UInt16 refnum)
				SYS_TRAP(kStatusBarMgrLibShow);

#ifdef __cplusplus
}
#endif

#endif  //__STATUSBARMGRLIB_H__
