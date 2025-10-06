/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Transparency Transparency Library
 * @brief		This library can be used to couple data between a Ccp channel and a serial
 * 				port.
 *
 * It is meant to be used for diagnostics and tethered mode. Since it is a
 * library, it can work without being in a certain application.
 *
 * @{
 * @}
 */
/**
 * @ingroup Transparency
 */

/**
 * @file TransparencyLibTypes.h
 * @brief	Public common header file for the Transparency Library on Treo devices.
 *
 * This file contains the common types of the Transparency Library.
 * <hr>
 */

#ifndef __TRANSPARENCY_LIB_TYPES_INT__H__

#define __TRANSPARENCY_LIB_TYPES_INT__H__

/**
 * Transparency library names
 **/
#define  transLibName                   "Transparency Library"

/**
 * Creator ID of Transparency Library - 'HsTL'
 **/
#define  transLibCreator                hsFileCTransparencyLib

/**
 * Type ID of transparency library
 **/
#define  transLibType                   sysFileTLibrary

/**
 * @name Function Traps
 */
/*@{*/
#define transLibTrapOpenConnection      sysLibTrapCustom
#define transLibTrapCloseConnection     sysLibTrapCustom+1
/*@}*/

/**
 * @name Error Codes
 */
/*@{*/

/**
 * Error class for the transparency library
 **/
#define   transLibErrorClass  0x5000

/**
 *  Unkown command requested. For example, an unknown channel type was passed as
 *  a port.
 **/
#define   transLibErrUnknown  (transLibErrorClass | 1)

/**
 *  No connection is open.
 **/
#define   transLibErrNotOpen  (transLibErrorClass | 2)

/**
 *  Transparency library already open
 **/
#define   transLibErrAlreadyOpen  (transLibErrorClass | 3)

/**
 *  Transparency library error when there is cradle not attached to the bottom port
 **/
#define   transLibErrDockNotAttached (transLibErrorClass | 4)

/*@}*/

#endif
