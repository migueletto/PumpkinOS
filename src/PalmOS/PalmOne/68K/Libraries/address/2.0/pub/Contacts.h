/*
 * Contacts.h
 *
 * public header for shared library
 *
 * Copyright (c) 2005 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 */
 /******************************************************************************
 *
 * Copyright (c) 2005 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Contacts.h
 *
 * Release: TBD
 *
 * Description:
 *		This header file contains the public routines used to add/update
 *		contacts and manage the data contained in the database for
 *		the built-in Contacts application.
 *
 * History:
 *		December 23, 2004	Created by Scott Silverman
 *
 *			Name		Date			Description
 *			----		----			-----------
 *			SDS			5/31/05			Creation.
 *
 *****************************************************************************/

#ifndef CONTACTS_H_
#define CONTACTS_H_

/* Palm OS common definitions */
#include <SystemMgr.h>

#define CONTACTS_LIB_TRAP	SYS_TRAP

/*********************************************************************
 * Type and creator of Sample Library database
 *********************************************************************/
#define		ContactsLibCreatorID		'P1CT'
#define		ContactsLibTypeID			sysFileTLibrary

/*********************************************************************
 * Internal library name which can be passed to SysLibFind()
 *********************************************************************/
#define		ContactsLibName				"ContactsLib-P1CT"

/*********************************************************************
 * ContactsLog result codes
 *********************************************************************/
#define ContactsLibErrorClass					(appErrorClass | 0x7300)
#define	ContactsLibErrTooManyClients			(ContactsLibErrorClass | 1)	// No More client can open the lib
#define	ContactsLibErrInvalidHandle				(ContactsLibErrorClass | 2)
#define	ContactsLibErrInvalidParam				(ContactsLibErrorClass | 3)	
#define	ContactsLibErrUIdNotFound				(ContactsLibErrorClass | 4)	
#define	ContactsLibErrCategoryNotFound			(ContactsLibErrorClass | 5)	
#define	ContactsLibErrLibAlreadyInUse 			(ContactsLibErrorClass | 6)	
#define	ContactsLibErrVerNotSupported			(ContactsLibErrorClass | 7)	
#define	ContactsLibErrOutOfMemory				(ContactsLibErrorClass | 8)	
#define	ContactsLibErrSizeParamOverFlow			(ContactsLibErrorClass | 9)	
#define	ContactsLibErrEmptyContact				(ContactsLibErrorClass | 10)	
#define	ContactsLibErrInvalidReference			(ContactsLibErrorClass | 11)
#define	ContactsLibErrContactNotFound			(ContactsLibErrorClass | 12)
#define	ContactsLibErrAttachAlreadyExist		(ContactsLibErrorClass | 13)	// Attachment cannot be added it already exist in the record.
#define	ContactsLibErrAttachNotExist			(ContactsLibErrorClass | 14)		
#define	ContactsLibErrAttachOverFlow			(ContactsLibErrorClass | 15)		
#define	ContactsLibErrRecordNotFound			(ContactsLibErrorClass | 16)		
#define	ContactsLibErrAttachOverflow			(ContactsLibErrorClass | 17)		
#define	ContactsLibErrInvalidField				(ContactsLibErrorClass | 18)	
#define	ContactsLibErrTooManySession			(ContactsLibErrorClass | 19)	
#define	ContactsLibErrAlreadyInWriteMode		(ContactsLibErrorClass | 20)	// User have requested to create Session in ReadWriteMode but it is already open.
#define	ContactsLibErrInvalidSessionHandle		(ContactsLibErrorClass | 21)	
#define	ContactsLibErrFieldNotFound				(ContactsLibErrorClass | 23)
#define	ContactsLibErrStringNotFound			(ContactsLibErrorClass | 24)
#define	ContactsLibErrInvalidAppInfoBlock		(ContactsLibErrorClass | 25)
#define	ContactsLibErrAppInfoIsMising			(ContactsLibErrorClass | 26)
#define	ContactsLibErrReadOnlySession			(ContactsLibErrorClass | 27)
#define	ContactsLibErrUIdExist					(ContactsLibErrorClass | 28)
#define	ContactsLibErrMaxLimitReached			(ContactsLibErrorClass | 29)
#define	ContactsLibErrListNotExist				(ContactsLibErrorClass | 30)
#define	ContactsLibErrContactNotLocked			(ContactsLibErrorClass | 31)
#define	ContactsLibErrLibStillOpen				(ContactsLibErrorClass | 32)


#define	PALM_OS
#ifdef PALM_OS
	typedef MemHandle						ContactsContHandle;
	typedef	MemHandle						ContactsSession;

	#define	ContactsNumCategories			dmRecNumCategories
	#define	ContactsAllCategories			dmAllCategories
	#define	ContactsUnfiledCategory			dmUnfiledCategory
	#define ContactsSeekForward				dmSeekForward
	#define	ContactsSeekBackward			dmSeekBackward
	
	typedef DateType						ContactsDateType;
#endif

#define ContactsLabelLength				16
typedef char 							ContactsLabel[ContactsLabelLength];

/* Contact Field Labels */
typedef enum  
{
	contactsNoType,
	contactsWorkPhone,		// applicable to Phone1-7 fields
	contactsHomePhone,		// applicable to Phone1-7 fields
	contactsFaxPhone,		// applicable to Phone1-7 fields
	contactsOtherPhone,		// applicable to Phone1-7 fields
	contactsEmailPhone,		// applicable to Phone1-7 fields
	contactsMainPhone,		// applicable to Phone1-7 fields
	contactsPagerPhone,		// applicable to Phone1-7 fields
	contactsMobilePhone,	// applicable to Phone1-7 fields
	contactsOtherChat,		// applicable to Chat1 & Chat2 fields
	contactsAimChat,		// applicable to Chat1 & Chat2 fields
	contactsMsnChat,		// applicable to Chat1 & Chat2 fields
	contactsYahooChat,		// applicable to Chat1 & Chat2 fields
	contactsIcqChat,		// applicable to Chat1 & Chat2 fields
	contactsWorkAddress,	// applicable to Address1-3 fields
	contactsHomeAddress,	// applicable to Address1-3 fields
	contactsOtherAddress	// applicable to Address1-3 fields
}	ContactsLibFieldLabels;

/* Contacts record fields */
typedef enum
{
	contactsLastName,
	contactsFirstName,
	contactsCompany,
	contactsTitle,
	
	contactsPhone1,
	contactsPhone2,
	contactsPhone3,
	contactsPhone4,
	contactsPhone5,
	contactsPhone6,
	contactsPhone7,
		
	contactsChat1,
	contactsChat2,
	
	contactsWebpage,
		
	contactsCustom1,
	contactsCustom2,
	contactsCustom3,
	contactsCustom4,
	contactsCustom5,
	contactsCustom6,
	contactsCustom7,
	contactsCustom8,
	contactsCustom9,
		
	contactsAddress1,
	contactsCity1,
	contactsState1,
	contactsZipCode1,
	contactsCountry1,
	contactsAddress2,
	contactsCity2,
	contactsState2,
	contactsZipCode2,
	contactsCountry2,
	contactsAddress3,
	contactsCity3,
	contactsState3,
	contactsZipCode3,
	contactsCountry3,	
	
	contactsNote,
	
	contactsBirthdate,
	contactsBirthdayMask,
	contactsBirthdayPreset,
	contactsPicture,
	contactsUniqueID		= contactsPicture + 2,
	contactsIndex,
	contactsCategoryID,
	contactsPrivate			= contactsCategoryID + 2,
	contactsBusy,
	contactsDirty,
	contactsRingtoneInfo	= contactsDirty + 4,
	contactsAnniversaryDate,
	contactsAnniversaryMask,
	contactsAnniversaryPreset,
	contactsNumFields
} ContactsLibAllFields;

typedef enum
{
	contactsStrLastName,
	contactsStrFirstName,
	contactsStrCompany,
	contactsStrTitle,
	
	contactsStrPhone1,			// phone fields, all can accept a label
	contactsStrPhone2,
	contactsStrPhone3,
	contactsStrPhone4,
	contactsStrPhone5,
	contactsStrPhone6,
	contactsStrPhone7,
		
	contactsStrChat1,			// instant message id and service, both can accept a label
	contactsStrChat2,
	
	contactsStrWebpage,
		
	contactsStrCustom1,			// custom fields
	contactsStrCustom2,
	contactsStrCustom3,
	contactsStrCustom4,
	contactsStrCustom5,
	contactsStrCustom6,
	contactsStrCustom7,
	contactsStrCustom8,
	contactsStrCustom9,
		
	contactsStrAddress1,		// first set of address fields, Address1 can accept a label
	contactsStrCity1,
	contactsStrState1,
	contactsStrZipCode1,
	contactsStrCountry1,	
	contactsStrAddress2,		// second set of address fields, Address2 can accept a label
	contactsStrCity2,
	contactsStrState2,
	contactsStrZipCode2,
	contactsStrCountry2,	
	contactsStrAddress3,		// third set of address fields, Address3 can accept a label
	contactsStrCity3,
	contactsStrState3,
	contactsStrZipCode3,
	contactsStrCountry3,	
	
	contactsStrNote				// This field is assumed to be < 32K
} ContactsLibStrFields;

typedef enum
{
	contactsCustomCustom1	= contactsCustom1,
	contactsCustomCustom2,
	contactsCustomCustom3,
	contactsCustomCustom4,
	contactsCustomCustom5,
	contactsCustomCustom6,
	contactsCustomCustom7,
	contactsCustomCustom8,
	contactsCustomCustom9
} ContactsLibCustomFields;

typedef enum
{
	contactsWithLabelPhone1			= contactsPhone1,
	contactsWithLabelPhone2,
	contactsWithLabelPhone3,
	contactsWithLabelPhone4,
	contactsWithLabelPhone5,
	contactsWithLabelPhone6,
	contactsWithLabelPhone7,
	contactsWithLabelChat1,
	contactsWithLabelChat2,
	contactsWithLabelAddress1		= contactsAddress1,
	contactsWithLabelAddress2		= contactsAddress2,
	contactsWithLabelAddress3		= contactsAddress3
} ContactsLibWithLabelFields;

typedef enum ContactsLabelTypes
{
	ContactsChatLabel, 
	ContactsAddressLabel, 
	ContactsPhoneLabel
} ContactsLabelType;

typedef enum
{
	contactsDateBirthdate				= contactsBirthdate,
	contactsDateAnniversary				= contactsAnniversaryDate
} ContactsLibDateFields;

typedef enum
{
	contactsUInt16Index					= contactsIndex,
	contactsUInt16Category				= contactsCategoryID
} ContactsLibUInt16Fields;

typedef enum
{
	contactsUInt32UniqueID				= contactsUniqueID
} ContactsLibUInt32Fields;

typedef enum
{
	contactsUInt8Private				= contactsPrivate,
	contactsUInt8Busy,
	contactsUInt8Dirty,
	contactsUInt8BirthdayMask			= contactsBirthdayMask,
	contactsUInt8BirthdayPreset,
	contactsUInt8AnniversaryMask		= contactsAnniversaryMask,
	contactsUInt8AnniversaryPreset
} ContactsLibUInt8Fields;

typedef enum
{
	contactsBinaryPictureData			= contactsPicture,
	contactsBinaryRingtoneIdentifier	= contactsRingtoneInfo
} ContactsLibBinaryFields;

typedef enum
{
	ContactsLibSessionModeReadOnly	= 0x0001,
	ContactsLibSessionModeReadWrite	= 0x0003
} SessionMode;


#define ContactsLibTrapGetVersion 					(sysLibTrapCustom)

#define ContactsLibTrapOpenSession					(sysLibTrapCustom + 23)				
#define ContactsLibTrapCloseSession					(sysLibTrapCustom + 24)					

#define	ContactsLibTrapCreateHandle					(sysLibTrapCustom + 90)			
#define ContactsLibTrapReleaseHandle				(sysLibTrapCustom + 4)		

#define ContactsLibTrapAddContact		 			(sysLibTrapCustom + 8)
#define ContactsLibTrapUpdateContact		 		(sysLibTrapCustom + 7)
#define ContactsLibTrapDeleteContact		 		(sysLibTrapCustom + 9)

#define ContactsLibTrapGetContact		 			(sysLibTrapCustom + 12)		
#define	ContactsLibTrapGetNumOfRecord				(sysLibTrapCustom + 28)
#define	ContactsLibTrapSeekRecord					(sysLibTrapCustom + 35)		
#define ContactsLibTrapGetFirstContact	 			(sysLibTrapCustom + 10)	
#define ContactsLibTrapGetNextContact	 			(sysLibTrapCustom + 11)


#define	ContactsLibTrapDoesFieldExist				(sysLibTrapCustom + 36)			
#define	ContactsLibTrapIsEmptyContact				(sysLibTrapCustom + 30)		

#define	ContactsLibTrapSetUInt8Field				(sysLibTrapCustom + 51)			
#define	ContactsLibTrapSetUInt16Field				(sysLibTrapCustom + 53)			
#define	ContactsLibTrapSetDateField					(sysLibTrapCustom + 55)			
#define	ContactsLibTrapSetStringField				(sysLibTrapCustom + 57)			
#define	ContactsLibTrapSetBinaryField				(sysLibTrapCustom + 59)				


#define	ContactsLibTrapGetUInt8Field				(sysLibTrapCustom + 50)		
#define	ContactsLibTrapGetUInt16Field				(sysLibTrapCustom + 52)		
#define	ContactsLibTrapGetUInt32Field				(sysLibTrapCustom + 63)		
#define	ContactsLibTrapGetDateField					(sysLibTrapCustom + 54)		
#define	ContactsLibTrapGetStringField				(sysLibTrapCustom + 56)						
#define	ContactsLibTrapGetBinaryField				(sysLibTrapCustom + 58)						

#define	ContactsLibTrapGetFieldLabel				(sysLibTrapCustom + 31)		
#define	ContactsLibTrapSetFieldLabel				(sysLibTrapCustom + 32)		
#define	ContactsLibTrapGetLabelText					(sysLibTrapCustom + 82)

#define	ContactsLibTrapRenameCustomLabel			(sysLibTrapCustom + 33)

#define ContactsLibTrapGetCategoryList				(sysLibTrapCustom + 2)		
#define	ContactsLibTrapGetNextCategory				(sysLibTrapCustom + 37)				
#define ContactsLibTrapGetCategoryName				(sysLibTrapCustom + 27)
#define	ContactsLibTrapSetCategoryName				(sysLibTrapCustom + 38)				
#define	ContactsLibTrapCategoryFind					(sysLibTrapCustom + 39)						
#define	ContactsLibTrapPosInCategory				(sysLibTrapCustom + 48)	

#define	ContactsLibTrapGetRingtoneForCategory		(sysLibTrapCustom + 91)			

#define	ContactsLibTrapLookUpString					(sysLibTrapCustom + 40)							

#define	ContactsLibGetLastErr						(sysLibTrapCustom + 64)

/*
 * FUNCTION: ContactsLib_OpenLibrary
 *
 * DESCRIPTION:
 *
 * User-level call to open the library.  This inline function
 * handles the messy task of finding or loading the library
 * and calling its open function, including handling cleanup
 * if the library could not be opened.
 * 
 * PARAMETERS:
 *
 * refNumP
 *		Pointer to UInt16 variable that will hold the new
 *      library reference number for use in later calls
 *
 * CALLED BY: System
 *
 * RETURNS:
 *		errNone
 *		memErrNotEnoughSpace
 *      sysErrLibNotFound
 *      sysErrNoFreeRAM
 *      sysErrNoFreeLibSlots
 *
 */
__inline Err ContactsLib_OpenLibrary(UInt16 *refNumP)
{
	Err error;
	Boolean loaded = false;
	
	/* first try to find the library */
	error = SysLibFind(ContactsLibName, refNumP);
	
	/* If not found, load the library instead */
	if (error == sysErrLibNotFound)
	{
		error = SysLibLoad(ContactsLibTypeID, ContactsLibCreatorID, refNumP);
		loaded = true;
	}
	
	if (error == errNone)
	{
		error = ContactsLibOpen(*refNumP);
		if (error != errNone)
		{
			if (loaded)
			{
				SysLibRemove(*refNumP);
			}

			*refNumP = sysInvalidRefNum;
		}
	}
	
	return error;
}

/*
 * FUNCTION: ContactsLib_CloseLibrary
 *
 * DESCRIPTION:	
 *
 * User-level call to closes the shared library.  This handles removal
 * of the library from system if there are no users remaining.
 *
 * PARAMETERS:
 *
 * refNum
 *		Pointer Library reference number obtained from ContactsLib_OpenLibrary().
 *		Upon successful close, contents will be invalidated.
 *
 * CALLED BY: Whoever wants to close the library
 *
 * RETURNS:
 *		errNone
 *		sysErrParamErr
 */
__inline Err ContactsLib_CloseLibrary(UInt16 *refNumP)
{
	Err error;
	
	if (*refNumP == sysInvalidRefNum)
	{
		return sysErrParamErr;
	}

	error = ContactsLibClose(*refNumP);

	if (error == errNone)
	{
		/* no users left, so unload library */
		SysLibRemove(*refNumP);
		
		*refNumP = sysInvalidRefNum;	// invalidate so caller cannot use any longer.
	} 
	else if (error == ContactsLibErrLibStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}


#endif /* CONTACTSLIB_H_ */


/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/*
 * FUNCTION: ContactsLibGetVersion
 *
 * DESCRIPTION:	
 *
 * Returns the version number of the Contacts Library
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * RETURNS:
 *		Version number in the form where only the two low bytes are used.
 *		Of those two bytes, the high byte is the major version and
 *		the low byte is the minor version
 *		(i.e. the value 0x00000201 represents version 2.01).
 */
extern	UInt32		ContactsGetVersion(UInt16 libRefnum)	CONTACTS_LIB_TRAP(ContactsLibTrapGetVersion);

/*
 * FUNCTION: ContactsOpenSession
 *
 * DESCRIPTION:	
 *
 * Returns the session handle to be passed to other Contacts Lib routines
 * One can either specify a new session to be read only or read/write.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * mode
 *		This is the mode in which you want to open a session.  Must be one of the
 *		valid SessionMode types.
 *
 * RETURNS:
 *		a ContactsSession
 *		NULL (unsuccessful)
 *
 * USAGE:
 *		Typical usage is to always first open a session and retain the session handle returned.
 *		Then subsequent routines to add a new contact for instance will use the internal data
 *		stored in the session handle to access the underlying contacts database.
 * NOTES:
 *		It is important to always close the session with a call to CloseSession
 *		after you are finished with the session.  Doing so will free system memory used internally
 *		by the library and also free a spot for other users to open sessions.
 *		With this version of the library, there is a limit of 4 open sessions.
 */
extern	ContactsSession		ContactsOpenSession(UInt16 libRefnum, SessionMode mode)	CONTACTS_LIB_TRAP(ContactsLibTrapOpenSession);

/*
 * FUNCTION: ContactsCloseSession
 *
 * DESCRIPTION:	
 *
 * Nullifies the open session handle and frees any internal memory used by the library for this session.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid session handle that was obtain from a call to OpenSession.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrInvalidSessionHandle
 *
 * USAGE:
 *		This must be called after a session is no longer needed.  It will prevent memory leaks
 *		of any memory allocated by the OpenSession call.  It also will make available the
 *		session for another caller.
 * NOTES:
 *		It is important to always close the session with a call to ContactsCloseSession
 *		after you are finished with the session.  Doing so will free system memory used internally
 *		by the library and also free a spot for other users to open sessions.
 *		With this version of the library, there is a limit to a total of 4 open sessions.
 */
extern	Err		ContactsCloseSession(UInt16 libRefnum, ContactsSession hCurSession)	CONTACTS_LIB_TRAP(ContactsLibTrapCloseSession);

/*
 * FUNCTION: ContactsCreateHandle
 *
 * DESCRIPTION:	
 *
 * Returns a brand new ContactsContHandle..
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid read/write session handle that was obtain from a call to OpenSession.
 *
 * RETURNS:
 *		a valid ContHandle
 *		NULL (only possible way this could fail is if out of memory)
 *
 * USAGE:
 *		This is called to create a new handle which is then used to set a series of fields.
 *		One would create the handle, do a series of 'sets' and then add the new handle by calling
 *		AddContact.
 * NOTES:
 *		It is critical that after the handle is finished being used, a call to
 *		ReleaseHandle is called.
 */
extern	ContactsContHandle		ContactsCreateHandle(UInt16 libRefnum, ContactsSession hSession)	CONTACTS_LIB_TRAP(ContactsLibTrapCreateHandle);

/*
 * FUNCTION: ContactsReleaseHandle
 *
 * DESCRIPTION:	
 *
 * Used to release the memory used by ANY valid ContactsContHandle.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * RETURNS:
 *		errNone
 *		P1ctErrInvalidHandle
 *
 * USAGE:
 *		Once a handle has been obtained, either from a call to CreateHandle or GetContact,
 *		It is critical that the ReleaseHandle routine is called.
 */
extern	Err		ContactsReleaseHandle(UInt16 libRefnum, ContactsContHandle hContact)	CONTACTS_LIB_TRAP(ContactsLibTrapReleaseHandle);

/*
 * FUNCTION: ContactsAddContact
 *
 * DESCRIPTION:	
 *
 * Used to add the contact described by a ContHandle to the database.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hSession
 *		This is any valid read/write session handle that was obtain from a call to OpenSession.
 *
 * hContact
 *		The contact handle returned by CreateContact.
 * 
 * RETURNS:
 *		errNone
 *		ContactsLibErrInvalidSessionHandle
 *		P1ctErrReadOnlySession
 *
 * USAGE:
 *		Once a handle has been obtained from CreateHandle,
 *		the user will follow with a series of calls to "Set" routines that will set the fields values.
 *		Once all of the calls to the "set" routines have been made, it is necessary to call
 *		the AddContact routine to write the data to the database.
 *		The final step is to call ReleaseHandle on the ContHandle.
 */
extern	Err		ContactsAddContact(UInt16 libRefnum, ContactsSession hSession, ContactsContHandle hContact)	CONTACTS_LIB_TRAP(ContactsLibTrapAddContact);																																

/*
 * FUNCTION: ContactsUpdateContact
 *
 * DESCRIPTION:	
 *
 * Writes the entry to the database.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid read/write session handle that was obtain from a call to ContactsOpenSession.
 *
 * hContact
 *		This is any valid ContHandle that was returned by one of the routines that returns a ContHandle. 
 *
 * RETURNS:
 *		errNone
 *		P1ctErrInvalidHandle
 *
 * USAGE:
 *		Once a ContHandle has been obtained, the user can modify the entry by doing one or more "Set" calls.
 *		Once all of the set routines have been made, a call to UpdateContact must be made to save the changes.
 *
 * NOTES:
 *		There are several routines that will return a valid ContHandle (ie GetContact, GetFirstContact, GetNextContact).
 *		Although a call to UpdateContact is not required, if any changes have been made by calling any of the set
 *		routines, UpdateContact must be called to save those changes to the database.
 *		Remember, it IS required to call ReleaseHandle for any ContHandle obtained.
 */
extern	Err		ContactsUpdateContact(UInt16 libRefnum, ContactsSession hSession, ContactsContHandle hContact)	CONTACTS_LIB_TRAP(ContactsLibTrapUpdateContact);

/*
 * FUNCTION: ContactsDeleteContact
 *
 * DESCRIPTION:	
 *
 * Immeditely deletes the contact from the Contacts database.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid read/write session handle that was obtain from a call to OpenSession.
 *
 * hContact
 *		This is the valid ContHandle that was returned by one of the routines that returns a ContHandle.
 *
 * archive
 *		Pass true if you want the entry to be archived on PC.
 *
 * RETURNS:
 *		errNone
 *		P1ctErrReadOnlySession
 *
 * USAGE:
 *		Once a ContHandle has been obtained from any of the routines that return a ContHandle (except CreateHandle),
 *		calling this routine will delete this entry.
 *
 * NOTES:
 *		Remember, it STILL is required to call ReleaseHandle for the ContHandle after calling DeleteContact.
 */
extern	Err		ContactsDeleteContact(UInt16 libRefnum, ContactsSession hSession, ContactsContHandle hContact, Boolean archive)	CONTACTS_LIB_TRAP(ContactsLibTrapDeleteContact);

/*
 * FUNCTION: ContactsGetContact
 *
 * DESCRIPTION:	
 *
 * Gets a ContHandle by either index (0 based) or by uniqueID.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid session handle that was obtain from a call to OpenSession.
 *
 * indexP
 *		pass either a UInt16* containing a 0 based index or NULL.
 *
 * uniqueIDP
 *		Pass either a UInt32* containing a valid uniqueID or NULL.
 *
 * RETURNS:
 *		a valid ContHandle
 *		NULL
 *
 * USAGE:
 *		The routine first checks to see if uniqueIDP is not NULL, if not, a search of the database will be commenced
 *		using the uniqueID stored in that pointer.
 *		IF uniqueIDP is NULL, it will expect a non-NULL indexP which its contents will be used to get the
 *		ContHandle at that index location.
 *
 * NOTES:
 *		Remember to call ReleaseHandle for the ContHandle after finish using it.
 *		IF NULL is returned, the error causing the failure can be obtained call GetLastErr.
 */
extern	ContactsContHandle		ContactsGetContact(UInt16 libRefnum, ContactsSession hSession, UInt16* indexP, UInt32 *uniqueIDP, Boolean ignored) CONTACTS_LIB_TRAP(ContactsLibTrapGetContact);

/*
 * FUNCTION: ContactsGetNumRecords
 *
 * DESCRIPTION:	
 *
 * returns the number of records in any given category.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 *
 * category
 *		either the category index of the category you wish use OR ContactsAllCategories to specify all categories.
 *		To specify "Unfiled," use ContactsUnfiledCategory for the category.
 *
 * RETURNS:
 *		the number of contacts in the specified category.
 *
 * NOTE:
 *		To get the category for an existing record call ContactsGetUInt16Field for that record.
 */
extern	UInt16		ContactsGetNumRecords(UInt16 libRefnum, ContactsSession hSession, UInt16 category)	CONTACTS_LIB_TRAP(ContactsLibTrapGetNumOfRecord);

/* FUNCTION: ContactsPosInCategory
 *
 * DESCRIPTION:	
 *
 * Returns the position of a given record in a given category.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hSession
 *		This any valid session handle that was obtain from a call to OpenSession.
 *
 * hContact
 *		ContHandle of a given contact record.
 *
 * pIndex
 *		on return, contains the index of the record.
 *
 * category
 *		category in which the entry is contained.
 *
 * RETURNS:
 *		position of the record within a given category (0 based).
 *
 * NOTE:
 *		this routine is useful when only a given category is being displayed and for scrolling purposes,
 *		one needs to determine a record's position in that category.
 *		It is important that GetLastError is called and that noError is returned.
 *		This function can return 0 in a valid case but also will return 0 in an error case so
 *		GetLastError should be checked for noError.
 */
extern	UInt16		ContactsPosInCategory(UInt16 libRefnum, ContactsSession hSession, ContactsContHandle hContact, UInt16 *pIndex, UInt16 category)	CONTACTS_LIB_TRAP(ContactsLibTrapPosInCategory);

/*
 * FUNCTION: ContactsSeekRecord
 *
 * DESCRIPTION:	
 *
 * Returns a ContHandle specified by the seek criteria passed.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This is any valid session handle that was obtain from a call to ContactsOpenSession.
 *
 * indexP
 *		This is a pointer that contains the initial index in which to seek from.
 *
 * offset
 *		The number of entries to seek to past the intial position.
 *
 * direction
 *		Specifies to either seek forward or backward from the initial index ( ContactsSeekForward or ContactsSeekBackward)
 *
 * category
 *		Specifies the category in which to seek in.
 *
 * RETURNS:
 *		true	- a valid index has been found.
 *		false	- no index matching the seek criteria has been found.
 *
 *		indexP	- contains the valid index if the routine returned true.
 */
extern	Boolean		ContactsSeekRecord(UInt16 libRefnum, ContactsSession hSession, UInt16 *indexP, UInt16 offset, Int16 direction, UInt16 category)	CONTACTS_LIB_TRAP(ContactsLibTrapSeekRecord);

/* FUNCTION: ContactsGetFirstContact
 *
 * DESCRIPTION:	
 *
 * Gets the very first ContHandle in a given category
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hSession
 *		This any valid session handle that was obtain from a call to OpenSession.
 *
 * category
 *		either specify a category or pass ContactsAllCategories to indicate the very first contact in all the categories.
 *
 * RETURNS:
 *		a valid ContHandle
 *		NULL
 *
 * NOTES:
 *		Remember to call ReleaseHandle for the ContHandle after finish using it.
 *		If a NULL is returned, the exact error causing the failure can be obtained from GetLastErr.
 */
extern	ContactsContHandle		ContactsGetFirstContact(UInt16 libRefnum, ContactsSession hSession, UInt16 category)	CONTACTS_LIB_TRAP(ContactsLibTrapGetFirstContact);

/* FUNCTION: ContactsGetNextContact
 *
 * DESCRIPTION:	
 *
 * Gets the next ContHandle after a specified handle in a category
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * Category
 *		This is the category in which you wish to search.
 *
 * hSourceContact
 *		This is the contact to be used as the reference for the search.
 *
 * RETURNS:
 *		a valid ContHandle
 *		NULL
 *
 * NOTES:
 *		The specified hSourceContact does not have to be part of the specified category for the routine
 *		to succeed but it is logical that it would be.
 *		If a ContHandle is returned, remember to call ReleaseHandle for afer done using it.
 */
extern	ContactsContHandle		ContactsGetNextContact(UInt16 libRefnum, ContactsSession hSession, ContactsContHandle hSourceContact, UInt16 Category)	CONTACTS_LIB_TRAP(ContactsLibTrapGetNextContact);

/*
 * FUNCTION: ContactsDoesFieldExist
 *
 * DESCRIPTION:	
 *
 * Returns true if the field has been populated in the sepcified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * 
 * hContact
 *		This is the valid ContHandle that was returned by one of the routines that returns a ContHandle.
 *
 * NameOfField
 *		any valid fieldName (any one of the ContactsLibAllFields fields).
 *
 * RETURNS:
 *		true - if the field contains data.
 *		false - if the specified field is empty in the specified record.
 *
 * USAGE:
 *		This routine is most handy for the picture field to see if a picture exists prior to getting
 *		the data.  However, this routine can be used for any of the fields.
 */
extern	Boolean		ContactsDoesFieldExist(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibAllFields NameOfField)	CONTACTS_LIB_TRAP(ContactsLibTrapDoesFieldExist);

/* FUNCTION: ContactsIsEmptyContact
 *
 * DESCRIPTION:	
 *
 * Will return true if ALL of the fields of a given record are empty.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 *
 * hContact
 *		The contact you which to know is empty or not.
 *
 * RETURNS:
 *		true	- record is completely empty.
 *		false	- there is at least one field that contains data in the specified record.
 */
extern	Boolean		ContactsIsEmptyContact (UInt16 libRefnum,  ContactsSession hSession, ContactsContHandle hCont, UInt16 *Index, UInt32 *UniqueID)	CONTACTS_LIB_TRAP(ContactsLibTrapIsEmptyContact);


/* FUNCTION: ContactsSetUInt8Field
 *
 * DESCRIPTION:	
 *
 * Set the value of the specified UInt8 field.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hContact
 *		This any valid read/write session handle that was obtain from a call to OpenSession.
 *
 * nameOfField
 *		This is the field of size UInt8 in which you want to set (ContactsLibUInt8Fields).
 *
 * fieldValue
 *		value you wish to assign to that field.
 *
 * RETURNS:
 *		noErr.
 *		P1ctErrInvalidParam
 *		ContactsLibErrInvalidHandle.
 */
extern	Err		ContactsSetUInt8Field(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibUInt8Fields nameOfField, UInt8 fieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapSetUInt8Field);															

/* FUNCTION: ContactsSetUInt16Field
 *
 * DESCRIPTION:	
 *
 * Set the value of the specified UInt16 field.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hContact
 *		This any valid session handle that was obtain from a call to OpenSession.
 *
 * NameOfField
 *		This is the field of size UInt16 in which you want to set (ContactsLibUInt16Fields).
 *
 * FieldValue
 *		value you wish to set field with.
 *
 * RETURNS:
 *		noErr.
 *		P1ctErrInvalidParam
 *		ContactsLibErrInvalidHandle.
 */
extern	Err		ContactsSetUInt16Field(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibUInt16Fields NameOfField, UInt16 FieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapSetUInt16Field);															

/* FUNCTION: ContactsSetDateField
 *
 * DESCRIPTION:	
 *
 * Set the value of the specified Date field.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hContact
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 *
 * nameOfField
 *		This is the field of type ContactsDateType in which you want to set (ContactsLibDateFields).
 *
 * fieldValue
 *		value you wish to set field with.
 *
 * RETURNS:
 *		noErr.
 *		P1ctErrInvalidParam
 *		ContactsLibErrInvalidHandle.
 */
extern	Err		ContactsSetDateField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibDateFields nameOfField, ContactsDateType fieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapSetDateField);															

/* FUNCTION: ContactsSetStringField
 *
 * DESCRIPTION:	
 *
 * Set the value of the specified String field.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hContact
 *		This any valid read/write session handle that was obtain from a call to OpenSession.
 *
 * nameOfField
 *		This is the field of type Char * in which you want to set (ContactsLibStrFields).
 *
 * FieldValue
 *		value you wish to set field with.
 *
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err		ContactsSetStringField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibStrFields nameOfField, char *FieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapSetStringField);

/* FUNCTION: ContactsSetBinaryField
 *
 * DESCRIPTION:	
 *
 * Set the value of the specified Binary field.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 *
 * hContact
 *		This any valid session handle that was obtain from a call to OpenSession.
 *
 * NameOfField
 *		This is the field of type Binary in which you want to set (ContactsLibBinaryFields).
 *
 * FieldValue
 *		value you wish to set field with.
 *
 * DataSize
 *		Length in bytes of the data you wish to set.
 *
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err		ContactsSetBinaryField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibBinaryFields NameOfField, void *pFieldData, UInt16 DataSize)	CONTACTS_LIB_TRAP(ContactsLibTrapSetBinaryField);

/* FUNCTION: ContactsGetUInt8Field
 *
 * DESCRIPTION:	
 *
 * Fetches the UInt8 value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type UInt8 in which you want to set (ContactsLibUInt8Fields).
 * pFieldValue
 *		pointer to the UInt8 in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err						ContactsGetUInt8Field(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibUInt8Fields NameOfField, UInt8 *pFieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapGetUInt8Field);

/* FUNCTION: ContactsGetUInt16Field
 *
 * DESCRIPTION:	
 *
 * Fetches the UInt16 value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type UInt16 in which you want to set (ContactsLibUInt16Fields).
 * pFieldValue
 *		pointer to the UInt16 in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err						ContactsGetUInt16Field(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibUInt16Fields NameOfField, UInt16 *pFieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapGetUInt16Field);

/* FUNCTION: ContactsGetUInt32Field
 *
 * DESCRIPTION:	
 *
 * Fetches the UInt32 value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type UInt32 in which you want to set (ContactsLibUInt32Fields).
 * pFieldValue
 *		pointer to the UInt32 in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err						ContactsGetUInt32Field(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibUInt32Fields NameOfField, UInt32 *pFieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapGetUInt32Field);

/* FUNCTION: ContactsGetDateField
 *
 * DESCRIPTION:	
 *
 * Fetches the Date value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid ContHandle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type DateType in which you want to set (ContactsLibDateFields).
 * pFieldValue
 *		pointer to the Date in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err						ContactsGetDateField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibDateFields NameOfField, DateType *pFieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapGetDateField);

/* FUNCTION: ContactsGetStringField
 *
 * DESCRIPTION:	
 *
 * Fetches the String value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid ContHandle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type String in which you want to set (ContactsLibStrFields).
 * pFieldValue
 *		pointer to the String in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 */
extern	Err						ContactsGetStringField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibStrFields NameOfField, char **pFieldValue)	CONTACTS_LIB_TRAP(ContactsLibTrapGetStringField);

/* FUNCTION: ContactsGetBinaryField
 *
 * DESCRIPTION:	
 *
 * Fetches the binary value of the specified field from the specified record.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid ContHandle that was obtain from a call to ContactsOpenSession.
 * NameOfField
 *		This is the field of type binary in which you want to set (ContactsLibBinaryFields).
 * pFieldValue
 *		pointer to the binary in which the fetched value will be placed.
 * RETURNS:
 *		noErr.
 *		ContactsLibErrInvalidHandle.
 * NOTES:
 *		There are two possible fields that contain binary data, pictureField & ringtoneIdentifier.
 *		pictureField will return a pointer to a string of data of the picture (JPEG).
 *		ringtoneIdentifier returns a pointer to a 6 byte structure of type ToneIdentifier (described in ToneLibTypes.h)
 */
extern	void*					ContactsGetBinaryField(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibBinaryFields NameOfField, UInt16 *pDataSize)	CONTACTS_LIB_TRAP(ContactsLibTrapGetBinaryField);

/* FUNCTION: ContactsGetRingtoneForCategory
 *
 * DESCRIPTION:	
 *
 * Gets the ToneIdentifier for a given category.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * categoryID
 *		The index of the category.
 * tone
 *		This is a pointer to a struct of type ToneIdentifier which contains the toneType (UInt16) and toneID (UInt32).
 * RETURNS:
 *		noErr.
 */
extern	Err						ContactsGetRingtoneForCategory(UInt16 libRefnum, UInt16 categoryID, void *tone)	CONTACTS_LIB_TRAP(ContactsLibTrapGetRingtoneForCategory);													

/* FUNCTION: ContactsGetFieldLabel
 *
 * DESCRIPTION:	
 *
 * Gets the field label index of a field containing a label.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid ContHandle.
 * FieldNum
 *		This is the field value of type (ContactsLibWithLabelFields).
 * RETURNS:
 *		Index of the label assigned to that field.
 * NOTES:
 *		In addition to a field have a field value, it can also have a field label.
 *		For example, the Address field can also have a label describing if it is a Home, Work, etc.
 *		This routine returns the index of the label which can then be used to get the actual label text
 *		using GetLabelText or GetLabelList.
 */
extern	Int16					ContactsGetFieldLabel(UInt16 libRefnum, ContactsContHandle hContact, ContactsLibWithLabelFields FieldNum)	CONTACTS_LIB_TRAP(ContactsLibTrapGetFieldLabel);

/* FUNCTION: ContactsSetFieldLabel
 *
 * DESCRIPTION:	
 *
 * Sets the field label index of a field containing a label.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hContact
 *		This any valid ContHandle.
 * FieldNum
 *		This is the field value of type (ContactsLibWithLabelFields).
 * LabelIndex
 *		This is the 0 based index of the label to which the field should be assigned.
 * RETURNS:
 *		Nothing.
 */
extern	void 					ContactsSetFieldLabel(UInt16 libRefnum, ContactsContHandle hToCont, ContactsLibWithLabelFields FieldNum, Int16 LabelIndex)	CONTACTS_LIB_TRAP(ContactsLibTrapSetFieldLabel);

/* FUNCTION: ContactsGetLabelText
 *
 * DESCRIPTION:	
 *
 * Gets a particular label for a field based on its index.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * type
 *		This is type of the field label you wish to get (ContactsLabelType).
 * labelNum
 *		This is index within the label list.
 * RETURNS:
 *		String containing the label name.
 *		pSizeOfBuffer
 *			contains the size of the allocated buffer returned.
 * NOTES:
 *		This routine allocates memory and the string pointer returned must be freed by the caller.
 */
extern	char*					ContactsGetLabelText(UInt16 libRefnum, ContactsSession hSession, ContactsLabelType type, UInt16 labelNum, UInt16 *pSizeOfBuffer)	CONTACTS_LIB_TRAP(ContactsLibTrapGetLabelText);

/* FUNCTION: ContactsRenameCustomLabel
 *
 * DESCRIPTION:	
 *
 * Sets one of the Custom Labels to a specified string.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * fieldNum
 *		This is field number of any one of the custom fields (ContactsLibCustomFields).
 * fieldLabel
 *		This is the string pointer in which you want the custom field's name to be assigned.
 *		(the maximum length of a field label is ContactsLabelLength (includes null byte))
 * RETURNS:
 *		noErr.
 * NOTES:
 *		This routine allocates memory and the string pointer returned must be freed by the caller.
 */
extern	Err						ContactsRenameCustomLabel(UInt16 libRefnum, ContactsSession hSession, ContactsLibCustomFields fieldNum, Char* fieldLabel)	CONTACTS_LIB_TRAP(ContactsLibTrapRenameCustomLabel);

/* FUNCTION: ContactsGetCategoryList
 *
 * DESCRIPTION:	
 *
 * Returns the current list of categories for the Contacts application.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * pCategoryList
 *		A pointer to an array of string pointers.
 * RETURNS:
 *		Each of the locations in the pCategoryList will cotain a string pointer.  If the category
 *		does not exist, the string will be of length of zero.
 * NOTES:
 *		This routine allocates a string pointer for EVERY location in pCategoryList (even if
 *		no category exists at that location).
 */
extern	void					ContactsGetCategoryList(UInt16 libRefnum, ContactsSession hSession, char *pCategoryList[ContactsNumCategories])	CONTACTS_LIB_TRAP(ContactsLibTrapGetCategoryList);	

/* FUNCTION: ContactsGetNextCategory
 *
 * DESCRIPTION:	
 *
 * Returns the current list of categories for the Contacts application.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * currentCategory
 *		The index of a category to be used as a reference to seek to the next category.
 *		Passing ContactsAllCategories as the current Category is legal and can be used
 *		to initiat a search of all the available categories.
 * RETURNS:
 *		The index of the next category.
 */
extern	UInt16					ContactsGetNextCategory(UInt16	libRefnum, ContactsSession hSession, UInt16	currentCategory)	CONTACTS_LIB_TRAP(ContactsLibTrapGetNextCategory);

/* FUNCTION: ContactsGetCategoryName
 *
 * DESCRIPTION:	
 *
 * Returns the name of a category for a specified category index.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * categoryIndex
 *		The index of a category to be used as a reference to seek to the next category.
 *		Passing ContactsAllCategories as the current Category is legal and can be used
 *		to get the name of the "All" category label.
 * RETURNS:
 *		A string pointer to the category name.
 * NOTE:
 *		This routine returns a string pointer which must be freed.
 */
extern	Char*					ContactsGetCategoryName(UInt16 libRefnum, ContactsSession hSession, UInt16 categoryIndex)	CONTACTS_LIB_TRAP(ContactsLibTrapGetCategoryName);

/* FUNCTION: ContactsSetCategoryName
 *
 * DESCRIPTION:	
 *
 * Returns the name of a category for a specified category index.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * categoryIndex
 *		The index of a category in which to assign the new label.
 * pCategoryName
 *		A string pointer in which to assign the category label.
 * RETURNS:
 *		none.
 * NOTE:
 *		To clear out a particular category, simply pass NULL for the pCategoryName.
 */
extern	void					ContactsSetCategoryName(UInt16 libRefnum, ContactsSession hSession, UInt16 categoryIndex, const Char *pCatName)	CONTACTS_LIB_TRAP(ContactsLibTrapGetCategoryName);

/* FUNCTION: ContactsCategoryFind
 *
 * DESCRIPTION:	
 *
 * Returns the category index given a specified category name.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * pCatName
 *		Name of the label in which to locate its index..
 * RETURNS:
 *		index of the category name specified.
 */
extern	UInt16					ContactsCategoryFind(UInt16 libRefnum, ContactsSession hSession, const Char *pCatName)	CONTACTS_LIB_TRAP(ContactsLibTrapCategoryFind);

/* FUNCTION: ContactsGetRingtoneForCategory
 *
 * DESCRIPTION:	
 *
 * Returns ringtone for a given category.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * categoryID
 *		This is the index for any given category.
 * pTone
 *		a pointer to a ToneIdentifier structure.
 * RETURNS:
 *		Populates the pTone structure with a valid ToneIdentifier.
 */
extern	Err						ContactsGetRingtoneForCategory(UInt16 libRefnum, UInt16 categoryID, void *pTone)	CONTACTS_LIB_TRAP(ContactsLibTrapGetRingtoneForCategory);													

/* FUNCTION: ContactsLookUpString
 *
 * DESCRIPTION:	
 *
 * Searchs the contact database for the first match based on the specified key.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * hSession
 *		This any valid session handle that was obtain from a call to ContactsOpenSession.
 * key
 *		This is the string pointer to the key to be used for the search.
 * sortByCompany
 *		specifies how the db is sorted for this search.
 * category
 *		category to search within.
 * completeMatch
 *		contains true upon return if the key was completely matched.
 * pNumMatchingChars
 *		returns the number of matching characters upon return.  Pass NULL if dont care.
 * masked
 *		pass true if to include masked records in the search.
 * RETURNS:
 *		If a match is found, returns the ContHandle of the entry.
 * NOTE:
 *		Remember, a call to ReleaseHandle must be made if this routine returns a non-NULL value.
 */
extern	ContactsContHandle		ContactsLookUpString(UInt16 libRefnum, ContactsSession hSession, Char* key, Boolean sortByCompany, UInt16 category, 
														Boolean* completeMatch, UInt16* pNumMatchingChars, Boolean masked)	CONTACTS_LIB_TRAP(ContactsLibTrapLookUpString);

/* FUNCTION: ContactsGetLastError
 *
 * DESCRIPTION:	
 *
 * Returns ringtone for a given category.
 *
 * PARAMETERS:
 *
 * libRefnum
 *		Library reference number obtained from ContactsLib_OpenLibrary().
 * RETURNS:
 *		the last error code stored by the library.
 * NOTE:
 *		Some routines do not return error codes but store them internally.  The last error code store dinternally
 *		can be obtained by calling this routine.
 */
extern	Err 					ContactsGetLastError(UInt16 libRefnum)	CONTACTS_LIB_TRAP(ContactsLibGetLastErr);

#ifdef __cplusplus
}
#endif