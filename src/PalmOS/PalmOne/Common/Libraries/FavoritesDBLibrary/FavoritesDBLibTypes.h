/*******************************************************************************
 * Copyright (c) 2004-2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 ******************************************************************************/
/**
 * @defgroup	Favorites Favorites Database Library
 * @brief		This library is used to provide support for applications to
 *				insert new entries to the Phone Favorites view on Treo 650
 *				devices.
 *
 * An application which wants to insert a new entry to the Phone application
 * Favorites view programmatically can call the appropriate FavDBNew*() API
 * depending on the favority type. Currently, this library supports adding
 * new application, phone number, sms destination, web url, email address,
 * and phone view.
 *
 * @{
 * @}
 */
/**
 * @ingroup Favorites
 *
 */

/**
 * @file FavoritesDBLibTypes.h
 * @brief	Public 68K common header file for FavoritesDB library API.
 *
 * This file contains the FavoritesDB library constants and error codes.
 */

#ifndef FAVORITESDB_LIB_TYPES__H__
#define FAVORITESDB_LIB_TYPES__H__

#define      favoritesDBLibName				"FavoritesDBLibrary" /**< Favorites DB library name */
#define      favoritesDBLibCreator			'HsFD'               /**< Favorites DB library creator ID */
#define      favoritesDBLibType				sysFileTLibrary      /**< Standard library type */

/**
 * @name Error Codes
 */
/*@{*/
#define      favoritesDBLibErrClass			0x9000
#define      favoritesDBLibErrCreateDBFailed  (favoritesDBLibErrClass | 1)
#define      favoritesDBLibErrNoMemory        (favoritesDBLibErrClass | 2)
#define      favoritesDBLibErrDBNotFound      (favoritesDBLibErrClass | 4)
#define      favoritesDBLibErrDBWriteFailed   (favoritesDBLibErrClass | 5)
#define      favoritesDBLibErrUserCancel      (favoritesDBLibErrClass | 6)
#define      favoritesDBLibErrBadParams       (favoritesDBLibErrClass | 7)
/*@}*/

/**
 * @name Function Traps
 */
/*@{*/
#define       favoritesDBLibTrapNewAppFavorite		  sysLibTrapCustom
#define       favoritesDBLibTrapNewPhoneFavorite	  sysLibTrapCustom+1
#define       favoritesDBLibTrapNewMsgFavorite		  sysLibTrapCustom+2
#define       favoritesDBLibTrapNewWebFavorite		  sysLibTrapCustom+3
#define       favoritesDBLibTrapNewEmailFavorite	  sysLibTrapCustom+4
#define       favoritesDBLibTrapNewPhoneViewFavorite  sysLibTrapCustom+5
#define       favoritesDBLibTrapResolveAddress		  sysLibTrapCustom+6
#define       favoritesDBLibTrapSeekSpeedDialRecord	  sysLibTrapCustom+7
#define       favoritesDBLibTrapGetRecordLabel		  sysLibTrapCustom+8
#define       favoritesDBLibTrapGetRecordId			  sysLibTrapCustom+9
#define       favoritesDBLibIsFavoriteWithId		  sysLibTrapCustom+10
#define       favoritesDBLibTrapGetSpeedDialInfo	  sysLibTrapCustom+11
#define       favoritesDBLibTrapLookupInSpeedDialDB	  sysLibTrapCustom+12
/*@}*/

#endif  // FAVORITESDB_LIB_TYPES__H__
