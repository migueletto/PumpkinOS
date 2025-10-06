/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Helper
 */

/**
 * @file  DefaultHelperLib.h
 * @brief Default Helper library owns control over the Default Helpers database.
 */

#ifndef __DEFAULTHELPERLIB__H__
#define __DEFAULTHELPERLIB__H__

#ifdef __cplusplus
extern "C" {
#endif

/// Standard library open function
///
/// @param refNum: IN: Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err DefaultHelperLibOpen (UInt16 refNum)
                 SYS_TRAP (sysLibTrapOpen);

/// Standard library close function
///
/// @param refNum: IN: Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err DefaultHelperLibClose (UInt16 refNum)
                 SYS_TRAP (sysLibTrapClose);

/// Standard library wake function
///
/// @param refNum: IN: Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err DefaultHelperLibWake (UInt16 refNum)
                 SYS_TRAP (sysLibTrapWake);

/// Standard library sleep function
///
/// @param refNum: IN: Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err DefaultHelperLibSleep (UInt16 refNum)
                 SYS_TRAP (sysLibTrapSleep);

/// Given a specified serviceID, return the creator type of the default helper
/// application.
///
/// @param refNum:				IN:  Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @param serviceID:			IN:  Type of service to get info from.
/// @param defaultAppCreatorP	OUT: Upon return of the function, a pointer to the creator type of the default
///									 application for the specified serviceID.
/// @retval Err Error code.
Err DefaultHelperLibGetDefaultHelper (UInt16 refNum,
									  UInt32 serviceID,
									  UInt32* defaultAppCreatorP)
                 SYS_TRAP (defaultHelperLibTrapGetDefaultHelper);

/// Given a specified serviceID, set the creator type of the default helper application.
///
/// @param refNum:				 IN: Default helper library reference number (from SysLibFind()/SysLibLoad())
/// @param serviceID:			 IN: Type of service to get info from.
/// @param defaultAppCreator:	 IN: Creator type of the application to be set as the default application for the
///									 specified serviceID. If 0, then remove the helper service from the Default Apps
///									 database.
/// @retval Err Error code.
Err DefaultHelperLibSetDefaultHelper (UInt16 refNum,
									  UInt32 serviceID,
									  UInt32 defaultAppCreator)
                 SYS_TRAP (defaultHelperLibTrapSetDefaultHelper);

#ifdef __cplusplus
}
#endif

#endif // __DEFAULTHELPERLIB__H__
