/*
 * $Id: pi-error.h,v 1.11 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-error.h:  definitions for errors returned by the SOCKET, DLP and
 *              FILE layers
 *
 * Copyright (c) 2004-2005, Florent Pillet.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
 */
 
#ifndef _PILOT_ERROR_H_
#define _PILOT_ERROR_H_

/** @file pi-error.h
 *  @brief Error definitions for the errors returned by libpisock's dlp_XXX functions.
 *
 * Most dlp_XXX functions return a value that is >= if the function
 * succeeded, or < 0 if there was an error. The error code can be directly
 * tested, and can also be retrieved using pi_error(). If the error code is
 * #PI_ERR_DLP_PALMOS, you should read the error code returned by the device
 * using pi_palmos_error().
 *
 * @note These error codes are tailored to not conflict with dlpErr* codes
 * defined in dlp.h, and which can be checked using pi_palmos_error()
 *
 */

/** @brief Type definition for error returned by various function.
 *
 * The reason we have a typedef is mostly for swig-generated bindings to
 * properly handle result codes
 */

/**< Type for result codes returned by various library functions (mainly for swig-generated bindings) */
typedef int PI_ERR;

/** @brief Definition of errors returned by various libpisock functions */
enum dlpErrorDefinitions {
	/* PROTOCOL level errors */
	PI_ERR_PROT_ABORTED		= -100,	/**< aborted by other end */
	PI_ERR_PROT_INCOMPATIBLE	= -101,	/**< can't talk with other end */
	PI_ERR_PROT_BADPACKET		= -102,	/**< bad packet (used with serial protocols) */

	/* SOCKET level errors */
	PI_ERR_SOCK_DISCONNECTED	= -200,	/**< connection has been broken */
	PI_ERR_SOCK_INVALID		= -201,	/**< invalid protocol stack */
	PI_ERR_SOCK_TIMEOUT		= -202,	/**< communications timeout (but link not known as broken) */
	PI_ERR_SOCK_CANCELED		= -203,	/**< last data transfer was canceled */
	PI_ERR_SOCK_IO			= -204,	/**< generic I/O error */
	PI_ERR_SOCK_LISTENER		= -205,	/**< socket can't listen/accept */

	/* DLP level errors */
	PI_ERR_DLP_BUFSIZE		= -300,	/**< provided buffer is not big enough to store data */
	PI_ERR_DLP_PALMOS		= -301,	/**< a non-zero error was returned by the device */
	PI_ERR_DLP_UNSUPPORTED		= -302,	/**< this DLP call is not supported by the connected handheld */
	PI_ERR_DLP_SOCKET		= -303,	/**< invalid socket */
	PI_ERR_DLP_DATASIZE		= -304,	/**< requested transfer with data block too large (>64k) */
	PI_ERR_DLP_COMMAND		= -305,	/**< command error (the device returned an invalid response) */

	/* FILE level error */
	PI_ERR_FILE_INVALID		= -400,	/**< invalid prc/pdb/pqa/pi_file file */
	PI_ERR_FILE_ERROR		= -401,	/**< generic error when reading/writing file */
	PI_ERR_FILE_ABORTED		= -402,	/**< file transfer was aborted by progress callback, see pi_file_retrieve(), pi_file_install(), pi_file_merge() */
	PI_ERR_FILE_NOT_FOUND		= -403,	/**< record or resource not found */
	PI_ERR_FILE_ALREADY_EXISTS	= -404,	/**< a record with same UID or resource with same type/ID already exists */

	/* GENERIC errors */
	PI_ERR_GENERIC_MEMORY		= -500,	/**< not enough memory */
	PI_ERR_GENERIC_ARGUMENT		= -501,	/**< invalid argument(s) */
	PI_ERR_GENERIC_SYSTEM		= -502	/**< generic system error */
};

/** @name libpisock error management macros */
/*@{*/
	#define IS_PROT_ERR(error)	((error)<=-100 && (error)>-200)	/**< Check whether the error code is at protocol level	*/
	#define IS_SOCK_ERR(error)	((error)<=-200 && (error)>-300) /**< Check whether the error code is at socket level	*/
	#define IS_DLP_ERR(error)	((error)<=-300 && (error)>-400) /**< Check whether the error code is at DLP level 	*/
	#define IS_FILE_ERR(error)	((error)<=-400 && (error)>-500) /**< Check whether the error code os a file error 	*/
	#define IS_GENERIC_ERR(error)	((error)<=-500 && (error)>-600) /**< Check whether the error code is a generic error 	*/
/*@}*/

#endif
