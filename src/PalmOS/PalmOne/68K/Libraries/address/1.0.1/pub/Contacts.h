/*
 * Contacts.h
 *
 * public header for shared library
 *
 * Copyright (c) 2004 palmOne, Inc. or its subsidiaries.
 * All rights reserved.
 */
 /******************************************************************************
 *
 * Copyright (c) 2004 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Contacts.c
 *
 * Release: TBD
 *
 * Description:
 *		This header file contains the public routines used to add/update
 *		contacts in the built-in Contacts application.
 *
 * History:
 *		December 23, 2004	Created by Scott Silverman
 *
 *			Name		Date			Description
 *			----		----			-----------
 *			SDS			12/23/04		Creation.
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
 * (appErrorClass is reserved for 3rd party apps/libraries.
 * It is defined in SystemMgr.h)
 *********************************************************************/
#define ContactsLibErrParam					(appErrorClass | 1)		
#define ContactsLibErrNotOpen				(appErrorClass | 2)		
#define ContactsLibErrStillOpen				(appErrorClass | 3)		
#define	ContactsLibErrNotEnoughMemory		(appErrorClass | 4)		
#define	ContactsLibErrUnknown				(appErrorClass | 5)		
#define	ContactsLibErrRecordNotFound		(appErrorClass | 6)		


typedef UInt32	ContactsLibHdl;

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
	
	contactsPhone1,			// phone fields, all can accept a label
	contactsPhone2,
	contactsPhone3,
	contactsPhone4,
	contactsPhone5,
	contactsPhone6,
	contactsPhone7,
		
	contactsChat1,			// instant message id and service, both can accept a label
	contactsChat2,
	
	contactsWebpage,
		
	contactsCustom1,		// custom fields
	contactsCustom2,
	contactsCustom3,
	contactsCustom4,
	contactsCustom5,
	contactsCustom6,
	contactsCustom7,
	contactsCustom8,
	contactsCustom9,
		
	contactsAddress1,		// first set of address fields, Address1 can accept a label
	contactsCity1,
	contactsState1,
	contactsZipCode1,
	contactsCountry1,	
	contactsAddress2,		// second set of address fields, Address2 can accept a label
	contactsCity2,
	contactsState2,
	contactsZipCode2,
	contactsCountry2,	
	contactsAddress3,		// third set of address fields, Address3 can accept a label
	contactsCity3,
	contactsState3,
	contactsZipCode3,
	contactsCountry3,	
	
	contactsNote,			// This field is assumed to be < 4K
	
	contactsBirthdate,		// birthday info ( DateType )
	contactsReserved1,
	contactsReserved2,

	contactsReserved3,
		
	contactsReserved4,
	contactsUniqueID,		// UInt32
	contactsReserved5,
	contactsCategoryID		// UInt16
	//contactsReserved6,
	//contactsReserved7,
	//contactsReserved8,
	//contactsReserved9,
	//contactsReserved10,
	//contactsReserved11,
	//contactsReserved12
} ContactsLibFields;


/*********************************************************************
 * API Prototypes
 *********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#define	contactsLibTrapNewContact			(sysLibTrapCustom + 90)			
#define	contactsLibTrapGetContact			(sysLibTrapCustom + 91)			
#define	contactsLibTrapSetField				(sysLibTrapCustom + 92)			
#define	contactsLibTrapGetField				(sysLibTrapCustom + 93)			
#define	contactsLibTrapDeleteContact		(sysLibTrapCustom + 94)			
#define	contactsLibTrapReleaseContact		(sysLibTrapCustom + 95)			

// DO NOT USE DIRECTLY, for ease, instead use ContactsLib_OpenLibrary
extern Err ContactsLibOpen(UInt16 refNum)
	CONTACTS_LIB_TRAP(sysLibTrapOpen);
				
// DO NOT USE DIRECTLY, for ease, instead use ContactsLib_CloseLibrary
extern Err ContactsLibClose(UInt16 refNum)
	CONTACTS_LIB_TRAP(sysLibTrapClose);

/*
 * FUNCTION: ContactsLibNewContact
 *
 * DESCRIPTION:
 *
 * Used to create a new contact entry.
 * Returns a ContactsLibHdl which is to be used in subsequent
 * routines to refer to this contact.
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * newContactHdl
 *		This is a pointer to a ContactsLibHdl.  This is filled in
 *		with a non-zero value on the successful completion of this
 *		routine.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *
 * NOTES:
 *		This routine should be followed by a series of calls
 *		to ContactsLibSetField.
 *		Upon the conclusion of all of the calls to set the
 *		new record, a call to ContactsLibReleaseContact MUST
 *		be called.
 *
 */
extern Err ContactsLibNewContact(UInt16 refNum, ContactsLibHdl *newContactHdl)
	CONTACTS_LIB_TRAP(contactsLibTrapNewContact);




/*
 * FUNCTION: ContactsLibGetContact
 *
 * DESCRIPTION:
 *
 * Used to fetch access to a contact to either.
 * read a field(s) OR to change a field(s).
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * index
 *		Index of the record desired. Pass NULL to use uniqueID instead.
 * uniqueID
 *		Unique ID of the record desired. Pass NULL to use index instead.
 * theContactHdl
 *		This is a pointer to a ContactsLibHdl.  This is filled in
 *		with a non-zero value on the successful completion of this
 *		routine.
 * readOnly
 *		This should be set to true if you will only be reading
 *		values from an existing entry.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *		ContactsLibErrRecordNotFound
 *
 * NOTES:
 *		Upon the conclusion of all of the calls to set or get
 *		the fields of an existing record, a call to
 *		ContactsLibReleaseContact MUST be called.
 *
 */
extern Err ContactsLibGetContact(UInt16 refNum, UInt16 index, UInt32 uniqueID, ContactsLibHdl *theContactHdl, Boolean readOnly)
	CONTACTS_LIB_TRAP(contactsLibTrapGetContact);

/*
 * FUNCTION: ContactsLibSetField
 *
 * DESCRIPTION:
 *
 * Used to set individual fields for a new or
 * existing record.
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * theContactHdl
 *		This is a pointer to a ContactsLibHdl returned from either
 *		NewContact or GetContact.
 * fieldType
 *		This is the field wishing to be set.
 * data
 *		This is a pointer to the data to be used to set the field.
 *		Most fields for a contact would be a string pointer.
 *		The birthday for example would take a pointer to a DateType.
 *
 *		IMPORTANT: String data passed to this routine MUST
 *		remain in memory UNTIL Release is called.
 * optData
 *		This is used for setting auxilary data for a particular field.
 *		This is currently only applicable for setting a field that also
 *		has a label designation (i.e. phone fields).
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *		ContactsLibErrParam
 *
 * NOTES:
 *		Upon the conclusion of all of the calls to set or get
 *		the fields of an existing record, a call to
 *		ContactsLibReleaseContact MUST be called.
 *
 */
extern Err ContactsLibSetField(UInt16 refNum, ContactsLibHdl theContactHdl, ContactsLibFields fieldType, void *data, UInt32 optData)
	CONTACTS_LIB_TRAP(contactsLibTrapSetField);

/*
 * FUNCTION: ContactsLibGetField
 *
 * DESCRIPTION:
 *
 * Used to obtain the value of individual fields for
 * an existing record.
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * theContactHdl
 *		This is a pointer to a ContactsLibHdl returned from
 *		GetContact.
 * fieldType
 *		This is the field containing the data to get.
 * data
 *		This is a pointer to the MemHandle returned by the function.
 *		It contains an unlocked memory handle to the data.
 *		This handle is opened by the caller and should be freed
 *		upon completion of use.
 * optData
 *		This contains the auxilary field information.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *		ContactsLibErrParam
 *
 * NOTES:
 *		Upon the conclusion of all of the calls to set or get
 *		the fields of an existing record, a call to
 *		ContactsLibReleaseContact must be called.
 *
 *		MemHandleFree must be called on all handles returned by GetField
 *		or memory leaks could result.
 *
 */
extern Err ContactsLibGetField(UInt16 refNum, ContactsLibHdl theContactHdl, ContactsLibFields fieldType, MemHandle *data, UInt32 *optData)
	CONTACTS_LIB_TRAP(contactsLibTrapGetField);

/*
 * FUNCTION: ContactsLibDeleteContact
 *
 * DESCRIPTION:
 *
 * Used to delete an existing contact.
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * theContactHdl
 *		This is a pointer to a ContactsLibHdl returned from
 *		GetContact.
 * archive
 *		Pass true if you want the contact in question to be archived on the device.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *		ContactsLibErrParam
 *
 * NOTES:
 *		ContactsLibReleaseContact must be called after deleting a contact.
 *
 */
extern Err ContactsLibDeleteContact(UInt16 refNum, ContactsLibHdl theContactHdl, Boolean archive)
	CONTACTS_LIB_TRAP(contactsLibTrapDeleteContact);

/*
 * FUNCTION: ContactsReleaseContact
 *
 * DESCRIPTION:
 *
 * Used to free all the associated memory of a contact (new or existing).
 * 
 * PARAMETERS:
 *
 * refNum
 *		This is the reference number returned by ContactsLib_OpenLibrary
 * theContactHdl
 *		This is a pointer to a ContactsLibHdl returned from
 *		GetContact or NewContact.
 * saveChanges
 *		Pass true if you want any changes made to the entry to be saved.
 *		One can always abort any previous calls by simply passing false.
 *
 * RETURNS:
 *		errNone
 *		ContactsLibErrNotEnoughMemory
 *		ContactsLibErrParam
 *
 */
extern Err ContactsLibReleaseContact(UInt16 refNum, ContactsLibHdl *theContactHdl, Boolean saveChanges)
	CONTACTS_LIB_TRAP(contactsLibTrapReleaseContact);

#ifdef __cplusplus
}
#endif


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
	else if (error == ContactsLibErrStillOpen)
	{
		/* don't unload library, but mask "still open" from caller  */
		error = errNone;
	}
	
	return error;
}


#endif /* CONTACTSLIB_H_ */
