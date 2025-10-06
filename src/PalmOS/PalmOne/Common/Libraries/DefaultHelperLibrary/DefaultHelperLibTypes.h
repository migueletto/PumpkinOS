/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Helper Default Helper Library
 * @brief		This library is used to provide support for applications to be
 *				the default handler for certain services on the device.
 *
 * This library is used in conjunction with Helper API from the Palm OS SDK.
 * Typically, an application which wants to be a default app to handle certain
 * services (SMS, Email, etc) will register itself in the following way:
 * - Check if the application is already the default handler for a particular
 *   service
 * - Call DefaultHelperLibSetDefaultHelper() if it's not already the default app.
 *   Please refer to Palm OS Helper API Service Class ID Constants for service ID.
 *
 * @{
 * @}
 */
/**
 * @ingroup Helper
 */

/**
 * @file 	DefaultHelperLibTypes.h
 * @version 1.0
 * @brief	Public 68K common header file for Default Helper Library API.
 *
 * This file contains the type information and constants for Default Helper Library.
 * <hr>
 */

#ifndef DEFAULT_HELPER_LIB_TYPES__H__
#define DEFAULT_HELPER_LIB_TYPES__H__

#define      defaultHelperLibName				"Default Helper Library" /**< Default Helper Library name */
#define      defaultHelperLibCreator			'HsDh'                   /**< Default Helper Library creator ID */
#define      defaultHelperLibType				sysFileTLibrary          /**< Standard library type */

/**
 * @name Error Codes
 */
/*@{*/
#define      defaultHelperLibErrClass			0x9000
#define      defaultHelperLibErrCreateDBFailed  (defaultHelperLibErrClass | 1)
#define      defaultHelperLibErrNoMemory        (defaultHelperLibErrClass | 2)
#define      defaultHelperLibErrNoHelperApp     (defaultHelperLibErrClass | 3)
#define      defaultHelperLibErrDBNotFound      (defaultHelperLibErrClass | 4)
#define      defaultHelperLibErrDBWriteFailed   (defaultHelperLibErrClass | 5)
#define      defaultHelperLibErrUserCancel      (defaultHelperLibErrClass | 6)
#define      defaultHelperLibErrBadParams       (defaultHelperLibErrClass | 7)
/*@}*/

/**
 * @name Function Traps
 */
/*@{*/
#define       defaultHelperLibTrapGetDefaultHelper		sysLibTrapCustom
#define       defaultHelperLibTrapSetDefaultHelper		sysLibTrapCustom+1
/*@}*/

#endif  // DEFAULT_HELPER_LIB_TYPES__H__

