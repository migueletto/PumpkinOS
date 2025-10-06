/******************************************************************************
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 *****************************************************************************/
/**
 * @ingroup Favorites
 */

/**
 * @file  FavoritesDBLib.h
 * @brief Contains the API to control the Favorites DB.
 *
 */

#ifndef FAVORITESDB_LIB__H__
#define FAVORITESDB_LIB__H__

/** List of different views in the phone applications. */
typedef enum
{
  favDBLibPhoneViewCallLog = 0, /**< call log view */
  favDBLibPhoneViewDialPad,     /**< dial pad view */
  favDBLibPhoneViewContacts     /**< contacts view */
} FavDBLibPhoneViewEnum;

#ifdef __cplusplus
extern "C" {
#endif

/// Standard library open function
///
/// @param refNum: IN: Favorites DB library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err FavDBLibOpen (UInt16 refNum)
                 SYS_TRAP (sysLibTrapOpen);

/// Standard library close function
///
/// @param refNum: IN: Favorites DB library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err FavDBLibClose (UInt16 refNum)
                 SYS_TRAP (sysLibTrapClose);

/// Standard library wake function
///
/// @param refNum: IN: Favorites DB library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err FavDBLibWake (UInt16 refNum)
                 SYS_TRAP (sysLibTrapWake);

/// Standard library sleep function
///
/// @param refNum: IN: Favorites DB library reference number (from SysLibFind()/SysLibLoad())
/// @retval Err Error code.
Err FavDBLibSleep (UInt16 refNum)
                 SYS_TRAP (sysLibTrapSleep);

/// Create a new favorite application entry.
///
/// @param refNum:		IN:  library reference number.
/// @param position:	IN:  position of the new favorite app in the entries.
/// @param labelP:		IN:  pointer to the label of the newly created entry.
/// @param quickKey:	IN:  shortcut used to start favorite app.
/// @param flags		IN:  no definition.
/// @param appCreator	IN:  Creator ID of the new favorite app.
/// @param indexP		OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err FavDBLibNewAppFavorite (UInt16 refNum,
							Word position,
							CharPtr labelP,
							WChar quickKey,
							Word flags,
							UInt32 appCreator,
							WordPtr indexP)
                 SYS_TRAP (favoritesDBLibTrapNewAppFavorite);

/// Create a new favorite phone number.
///
/// @param refNum:				IN:  library reference number.
/// @param position:			IN:  position of the new favorite phone number in the entries.
/// @param labelP:				IN:  pointer to the label of the newly created entry.
/// @param quickKey:			IN:  shortcut used to call the number.
/// @param flags:				IN:  no definition.
/// @param numberP:				IN:  New favorite phone number in string.
/// @param extraDigitsP:		IN:  Extra digits.
/// @param autoDialExtraDigits:	IN:  Always include extra digit when dialing if true.
/// @param ringID:				IN:  Ring tone ID for this number.
/// @param ringToneType:		IN:  Ring tone type for this number.
/// @param indexP:				OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err FavDBLibNewPhoneFavorite (UInt16 refNum,
							  Word position,
							  CharPtr labelP,
							  WChar quickKey,
							  Word flags,
							  CharPtr numberP,
							  CharPtr extraDigitsP,
							  Boolean autoDialExtraDigits,
							  UInt32 ringID,
							  UInt16 ringToneType,
							  WordPtr indexP)
                 SYS_TRAP (favoritesDBLibTrapNewPhoneFavorite);

/// Create a new favorite messaging destination.
///
/// @param refNum:		IN:  library reference number.
/// @param position:	IN:  position of the new favorite contact in the entries.
/// @param labelP:		IN:  pointer to the label of the newly created entry.
/// @param quickKey:	IN:  shortcut used to start composing message to the contact.
/// @param flags:		IN:  No definition.
/// @param contactIdP	IN:  Mobile number/ Email address of the new favorites messaging contact.
/// @param indexP		OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err FavDBLibNewMsgFavorite (UInt16 refNum,
							Word position,
							CharPtr labelP,
							WChar quickKey,
							Word flags,
							CharPtr contactIdP,
							WordPtr indexP)
				 SYS_TRAP (favoritesDBLibTrapNewMsgFavorite);

/// Create a new favorite URL.
///
/// @param refNum:		IN:  library reference number.
/// @param position:	IN:  position of the new favorite URL.
/// @param labelP:		IN:  pointer to the label of the newly created entry.
/// @param quickKey:	IN:  shortcut used to launch the URL.
/// @param flags:		IN:  no definition.
/// @param urlP:		IN:  the URL in string.
/// @param indexP:		OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err FavDBLibNewWebFavorite (UInt16 refNum,
							Word position,
							CharPtr labelP,
							WChar quickKey,
							Word flags,
							CharPtr urlP,
							WordPtr indexP)
				 SYS_TRAP (favoritesDBLibTrapNewWebFavorite);

/// Create a new favorite email entry.
///
/// @param refNum:		IN:  library reference number.
/// @param position:	IN:  position of the new favorite email in the entries.
/// @param labelP:		IN:  pointer to the label of the newly created entry.
/// @param quickKey:	IN:  shortcut used to start sending email to the new favorite .
/// @param flags:		IN:  no definition.
/// @param contactIdP:	IN:  Email address in string.
/// @param indexP:		OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err FavDBLibNewEmailFavorite (UInt16 refNum,
							  Word position,
							  CharPtr labelP,
							  WChar quickKey,
							  Word flags,
							  CharPtr contactIdP,
							  WordPtr indexP)
				 SYS_TRAP (favoritesDBLibTrapNewEmailFavorite);

/// Create a new favorite phone app view.
///
/// @param refNum:			IN:  library reference number.
/// @param position:		IN:  position of the new favorite in the entries.
/// @param labelP:			IN:  pointer to the label of the newly created entry.
/// @param quickKey:		IN:  shortcut used to launch phone app in the favorite view.
/// @param flags:			IN:  no definition.
/// @param phoneView:		IN:  See FavDBLibPhoneViewEnum for values.
/// @param indexP:			OUT: index of the new entry in the DB.
/// @retval Err Error code.
Err
FavDBLibNewPhoneViewFavorite (UInt16 refNum,
							  Word position,
							  CharPtr labelP,
							  WChar quickKey,
							  Word flags,
							  UInt16 phoneView,
							  WordPtr indexP)
				  SYS_TRAP (favoritesDBLibTrapNewPhoneViewFavorite);

/// No definition.
///
/// @param refNum:		IN: library reference number.
/// @param addressP:	IN: pointer to the address string to be resolved.
/// @param kind:		IN: no definition.
/// @retval CharPtr no definition.
CharPtr
FavDBLibResolveAddress (UInt16 refNum,
						CharPtr addressP,
						Word kind)
				  SYS_TRAP (favoritesDBLibTrapResolveAddress);

/// No definition.
///
/// @param refNum:		IN: library reference number.
/// @param indexP:		IN,OUT: no definition.
/// @param offset:		IN: offset in the DB record indeces to start search from.
/// @param direction:	IN: Forward/backward search from offset.
/// @retval Boolean		True if found. Otherwise, false.
Boolean
FavDBLibSeekSpeedDialRecord(UInt16 refNum,
							UInt16 * indexP,
							Int16 offset,
							Int16 direction)
				  SYS_TRAP (favoritesDBLibTrapSeekSpeedDialRecord);

/// Get the label from the favorite item given the record index.
///
/// @param refNum:		IN:  library reference number.
/// @param index:		IN:  index of the record to get label from.
/// @param labelPP:		OUT: pointer to the label string.
/// @retval Boolean True if found. Otherwise, false.
Boolean
FavDBLibGetRecordLabel (UInt16 refNum,
						   UInt16 index,
						   CharPtr *labelPP)
				  SYS_TRAP (favoritesDBLibTrapGetRecordLabel);

/// Get the unique ID of an entry given the record index in the Favorites DB.
///
/// @param refNum:		IN:  library reference number.
/// @param index:		IN:  index of the record to get unique id from.
/// @param uniqueIdP:	OUT: unique ID of the record.
/// @retval Err Error code.
Err
FavDBLibGetRecordId (UInt16 refNum,
					 UInt16 index,
					 UInt32 *uniqueIdP)
				  SYS_TRAP (favoritesDBLibTrapGetRecordId);

/// Determine whether item with keyId is in the favorites DB.
///
/// @param refNum:		IN:  library reference number.
/// @param keyId:		IN:  unique id of the item whose info is to be obtained.
/// @param existsP:		OUT: true if item is found among favorites.
/// @retval Err Error code.
Err
FavDBLibIsFavoriteWithId(UInt16 refNum,
						 UInt32 keyId,
						 BooleanPtr existsP)
				  SYS_TRAP (favoritesDBLibIsFavoriteWithId);

/// Get full info about the favorite phone number given the record index.
///
/// @param refNum:				IN:  library reference number.
/// @param index:				IN:  index of the item in the DB.
/// @param numberPP:			OUT: pointer to the speed dial phone number.
/// @param extraDigitsPP:		OUT: pointer to the extra digits in the phone number.
/// @param autoExtraDigitsP:	OUT: if true, extra digits are auto-dialed.
/// @param isVoicemailP:		OUT: if true, phone number is a voicemail number.
/// @retval Boolean True if found.
Boolean
FavDBLibGetSpeedDialInfo (UInt16 refNum,
						   UInt16  index,
						   CharPtr *numberPP,
						   CharPtr *extraDigitsPP,
						   BooleanPtr autoExtraDigitsP,
						   BooleanPtr isVoicemailP)
				  SYS_TRAP (favoritesDBLibTrapGetSpeedDialInfo);

/// no definition.
///
/// @param refNum:	IN:  library reference number.
/// @param keyP:	IN:  no definition.
/// @param nameP:	OUT: no definition.
/// @retval Boolean True if found.
Boolean
FavDBLibLookupInSpeedDialDB (UInt16 refNum,
							 CharPtr keyP,
							 CharPtr *nameP)
				  SYS_TRAP (favoritesDBLibTrapLookupInSpeedDialDB);

#ifdef __cplusplus
}
#endif

#endif  // FAVORITESDB_LIB__H__
