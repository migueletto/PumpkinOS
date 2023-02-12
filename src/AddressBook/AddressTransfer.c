/******************************************************************************
 *
 * Copyright (c) 1997-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressTransfer.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *      Address Book routines to transfer records.
 *
 *****************************************************************************/

#include "sec.h"

#include "Address.h"
#include "AddressTransfer.h"
#include "AddressRsc.h"
#include "AddrDefines.h"

#include <TraceMgr.h>
#include <PdiLib.h>
#include <UDAMgr.h>
#include <ErrorMgr.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <Category.h>


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

#define identifierLengthMax			40
#define addrFilenameExtension		"vcf"
#define addrFilenameExtensionLength	3
#define addrMIMEType				"text/x-vCard"

// Aba: internal version of vCalendar. Must be updated
// the export side of the vCalendar code evoluate

#define kVObjectVersion						"4.0"

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static Err	PrvTransferPdiLibLoad(UInt16* refNum, Boolean *loadedP);
static void	PrvTransferPdiLibUnload(UInt16 refNum, Boolean loaded);
static Err	PrvTransferSendRecordTryCatch (DmOpenRef dbP, Int16 recordNum, AddrDBRecordPtr recordP, UDAWriterType* media);
static void	PrvTransferCleanFileName(Char* ioFileName);
static Err	PrvTransferSendCategoryTryCatch (DmOpenRef dbP, UInt16 categoryNum, UDAWriterType*  media, UInt16 index);
static void PrvTransferSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID);


/***********************************************************************
 *
 * FUNCTION:		TransferRegisterData
 *
 * DESCRIPTION:		Register with the exchange manager to receive data
 * with a certain name extension.
 *
 * PARAMETERS:		nothing
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf		12/2/97		Created
 *
 ***********************************************************************/
void TransferRegisterData (void)
{
	MemHandle resH = DmGetResource(strRsc, ExgDescriptionStr);
	void *desc = MemHandleLock(resH);

	ExgRegisterDatatype(AddressBookCreator, exgRegExtensionID, addrFilenameExtension, desc, 0);
	ExgRegisterDatatype(AddressBookCreator, exgRegTypeID, addrMIMEType, desc, 0);
	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}

/***********************************************************************
 *
 * FUNCTION:    TransferSendRecord
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				recordNum - the record to send
 *				prefix - the scheme with ":" suffix and optional "?" prefix
 *				noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *         dje     4/24/00  Don't specify target creator ID
 *         ABa     6/20/00  Integrate Pdi library
 *
 ***********************************************************************/
void TransferSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix, UInt16 noDataAlertID)
{
	AddrDBRecordType record;
	MemHandle recordH;
	MemHandle descriptionH;
	UInt16 descriptionSize = 0;
	Int16 descriptionWidth;
	Boolean descriptionFit;
	UInt16 newDescriptionSize;
	MemHandle nameH;
	MemHandle resourceH;
	Char *resourceP;
	Err error;
	ExgSocketType exgSocket;
	UDAWriterType* media;
	UInt8 schemeLength;

	TraceInit();
	TraceOutput(TL(appErrorClass, "Pdi Version"));

	// important to init structure to zeros...
	MemSet(&exgSocket, sizeof(exgSocket), 0);

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	error = AddrDBGetRecord (dbP, recordNum, &record, &recordH);
	ErrNonFatalDisplayIf(error, "Can't get record");

	if (AddrDBRecordContainsData(&record))
	{
		// Figure out whether a person's name or company should be displayed.
		descriptionH = NULL;
		exgSocket.description = NULL;
		if (!(SortByCompany && record.fields[ad_company]) &&
			(record.fields[ad_name] || record.fields[ad_firstName]))
		{
			if (record.fields[ad_name] && record.fields[ad_firstName])
				descriptionSize = sizeOf7BitChar(' ') + sizeOf7BitChar('\0');
			else
				descriptionSize = sizeOf7BitChar('\0');

			if (record.fields[ad_name])
				descriptionSize += StrLen(record.fields[ad_name]);

			if (record.fields[ad_firstName])
				descriptionSize += StrLen(record.fields[ad_firstName]);


			descriptionH = MemHandleNew(descriptionSize);
			if (descriptionH)
			{
				exgSocket.description = MemHandleLock(descriptionH);

				if (record.fields[ad_firstName])
				{
					StrCopy(exgSocket.description, record.fields[ad_firstName]);
					if (record.fields[ad_name])
						StrCat(exgSocket.description, " ");
				}
				else
					exgSocket.description[0] = '\0';

				if (record.fields[ad_name])
				{
					StrCat(exgSocket.description, record.fields[ad_name]);
				}
			}

		}
		else if (record.fields[ad_company])
		{
			descriptionSize = StrLen(record.fields[ad_company]) + sizeOf7BitChar('\0');

			descriptionH = MemHandleNew(descriptionSize);
			if (descriptionH)
			{
				exgSocket.description = MemHandleLock(descriptionH);
				StrCopy(exgSocket.description, record.fields[ad_company]);
			}
		}

		// Truncate the description if too long
		if (descriptionSize > 0)
		{
			// Make sure the description isn't too long.
			newDescriptionSize = descriptionSize;
			WinGetDisplayExtent(&descriptionWidth, NULL);
			FntCharsInWidth (exgSocket.description, &descriptionWidth, (Int16 *)&newDescriptionSize, &descriptionFit);

			if (newDescriptionSize > 0)
			{
				if (newDescriptionSize != descriptionSize)
				{
					exgSocket.description[newDescriptionSize] = nullChr;
					MemHandleUnlock(descriptionH);
					MemHandleResize(descriptionH, newDescriptionSize + sizeOf7BitChar('\0'));
					exgSocket.description = MemHandleLock(descriptionH);
				}
			}
			else
			{
				MemHandleFree(descriptionH);
			}
			descriptionSize = newDescriptionSize;
		}

		// Make a filename
		schemeLength = StrLen(prefix);
		if (descriptionSize > 0)
		{
			// Now make a filename from the description
			nameH = MemHandleNew(schemeLength + imcFilenameLength);
			exgSocket.name = MemHandleLock(nameH);
			StrCopy(exgSocket.name, prefix);
			StrNCat(exgSocket.name, exgSocket.description,
					schemeLength + imcFilenameLength - addrFilenameExtensionLength - sizeOf7BitChar('.'));
			StrCat(exgSocket.name, ".");
			StrCat(exgSocket.name, addrFilenameExtension);
		}
		else
		{
			// A description is needed.  Either there never was one or the first line wasn't usable.
			descriptionH = DmGetResource(strRsc, BeamDescriptionStr);
			exgSocket.description = MemHandleLock(descriptionH);

			resourceH = DmGetResource(strRsc, BeamFilenameStr);
			resourceP = MemHandleLock(resourceH);
			nameH = MemHandleNew(schemeLength + StrLen(resourceP) + 1);
			exgSocket.name = MemHandleLock(nameH);
			StrCopy(exgSocket.name, prefix);
			StrCat(exgSocket.name, resourceP);
			MemHandleUnlock(resourceH);
			DmReleaseResource(resourceH);
		}

		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);
		exgSocket.length = MemHandleSize(recordH) + 100;		// rough guess
		//exgSocket.target = AddressBookCreator;		// commented out 4/24/00 dje
		exgSocket.type = (Char *)addrMIMEType;
		error = ExgPut(&exgSocket);   // put data to destination


		TraceOutput(TL(appErrorClass, "TransferSendRecord: description = %s, name = %s", exgSocket.description, exgSocket.name));
		// ABa: Changes to use new streaming mechanism
		media = UDAExchangeWriterNew(&exgSocket,  512);
		if (!error)
		{
			if (media)
				error = PrvTransferSendRecordTryCatch(dbP, recordNum, &record, media);
			else
				error = exgMemError;
				
			ExgDisconnect(&exgSocket, error);
		}
		
		if (media)
			UDADelete(media);

		// Clean up
		if (descriptionH)
		{
			MemHandleUnlock (descriptionH);
			if (MemHandleDataStorage (descriptionH))
				DmReleaseResource(descriptionH);
			else
				MemHandleFree(descriptionH);
		}
		if (nameH)
		{
			MemHandleUnlock (nameH);
			if (MemHandleDataStorage (nameH))
				DmReleaseResource(nameH);
			else
				MemHandleFree(nameH);
		}
	}
	else
		FrmAlert(noDataAlertID);


	MemHandleUnlock(recordH);
	
	// No need to release the record because AddrDBGetRecord didn't mark it busy.
	//DmReleaseRecord(dbP, recordNum, false);
	
	TraceClose();
	return;
}




/***********************************************************************
 *
 * FUNCTION:    TransferSendCategory
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				categoryNum - the category of records to send
 *				prefix - the scheme with ":" suffix and optional "?" prefix
 *				noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if any records are found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
void TransferSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID)
{
	Err error;
	Char description[dmCategoryLength];
	UInt16 index;
	Boolean foundAtLeastOneRecord;
	ExgSocketType exgSocket;
	UInt16 mode;
	LocalID dbID;
	UInt16 cardNo;
	Boolean databaseReopened;
	UDAWriterType* media;

	TraceInit();
	// If the database was opened to show secret records, reopen it to not see
	// secret records.  The idea is that secret records are not sent when a
	// category is sent.  They must be explicitly sent one by one.
	DmOpenDatabaseInfo(dbP, &dbID, NULL, &mode, &cardNo, NULL);
	if (mode & dmModeShowSecret)
	{
		dbP = DmOpenDatabase(cardNo, dbID, dmModeReadOnly);
		databaseReopened = true;
	}
	else
		databaseReopened = false;


	// important to init structure to zeros...
	MemSet(&exgSocket, sizeof(exgSocket), 0);


	// Make sure there is at least one record in the category.
	index = 0;
	foundAtLeastOneRecord = false;
	while (true)
	{
		if (DmSeekRecordInCategory(dbP, &index, 0, dmSeekForward, categoryNum) != 0)
			break;

		foundAtLeastOneRecord = DmQueryRecord(dbP, index) != 0;
		if (foundAtLeastOneRecord)
			break;


		index++;
	}


	// We should send the category because there's at least one record to send.
	if (foundAtLeastOneRecord)
	{
		// Form a description of what's being sent.  This will be displayed
		// by the system send dialog on the sending and receiving devices.
		CategoryGetName (dbP, categoryNum, description);
		exgSocket.description = description;

		// Now form a file name
		exgSocket.name = MemPtrNew(StrLen(prefix) + StrLen(description) + sizeOf7BitChar('.') + StrLen(addrFilenameExtension) + sizeOf7BitChar('\0'));
		if (exgSocket.name)
		{
			StrCopy(exgSocket.name, prefix);
			StrCat(exgSocket.name, description);
			StrCat(exgSocket.name, ".");
			StrCat(exgSocket.name, addrFilenameExtension);
		}
		// ABa: remove superfluous '.' chars
		PrvTransferCleanFileName(exgSocket.name);
		exgSocket.length = 0;		// rough guess
		//exgSocket.target = AddressBookCreator;		// commented out 4/24/00 dje
		exgSocket.type = (Char *)addrMIMEType;
		error = ExgPut(&exgSocket);   // put data to destination
		media = UDAExchangeWriterNew(&exgSocket, 512);
		if (!error)
		{
			if (media)
				error = PrvTransferSendCategoryTryCatch (dbP, categoryNum, media, index);
			else
				error = exgMemError;
				
			ExgDisconnect(&exgSocket, error);
		}

		// Release file name
		if (exgSocket.name)
			MemPtrFree(exgSocket.name);

		if (media)
			UDADelete(media);
	}
	else
		FrmAlert(noDataAlertID);

	if (databaseReopened)
		DmCloseDatabase(dbP);

	TraceClose();
	return;
}


/***********************************************************************
 *
 * FUNCTION:		ReceiveData
 *
 * DESCRIPTION:		Receives data into the output field using the Exg API
 *
 * PARAMETERS:		exgSocketP, socket from the app code
 *						 sysAppLaunchCmdExgReceiveData
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         ABa     6/20/00  Integrate Pdi library
 *
 ***********************************************************************/
Err TransferReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP)
{
	volatile Err err;
	UInt16 pdiRefNum = sysInvalidRefNum;
	PdiReaderType* reader = NULL;
	UDAReaderType* stream = NULL;
	Boolean loaded;

	// accept will open a progress dialog and wait for your receive commands
	if ((err = ExgAccept(exgSocketP)) != 0)
		return err;

	TraceInit();

	if ((err = PrvTransferPdiLibLoad(&pdiRefNum, &loaded)))
	{
		pdiRefNum = sysInvalidRefNum;
		goto errorDisconnect;
	}	
		
	if ((stream = UDAExchangeReaderNew(exgSocketP)) == NULL)
	{
		err = exgMemError;
		goto errorDisconnect;
	}
		
	if ((reader = PdiReaderNew(pdiRefNum, stream, kPdiOpenParser)) == NULL)
	{
		err = exgMemError;
		goto errorDisconnect;
	}
	
	reader->appData = exgSocketP;
	
	ErrTry
	{
		// Keep importing records until it can't
		while(TransferImportVCard(dbP, pdiRefNum, reader, false, false)){};
	}
	ErrCatch(inErr)
	{
		err = inErr;
	} ErrEndCatch

	// Aba: A record has been added in the Database iff the GoTo
	// uniqueID parameter != 0.
	// In the case no record is added, return an error
	if (err == errNone && exgSocketP->goToParams.uniqueID == 0)
		err = exgErrBadData;
	
errorDisconnect:
	if (reader)
		PdiReaderDelete(pdiRefNum, &reader);

	if (stream)
		UDADelete(stream);
	
	if (pdiRefNum != sysInvalidRefNum)
		PrvTransferPdiLibUnload(pdiRefNum, loaded);
		
	ExgDisconnect(exgSocketP, err); // closes transfer dialog
	err = errNone;	// error was reported, so don't return it

	TraceClose();
	
	return err;
}


/************************************************************
 *
 * FUNCTION: TransferImportVCard
 *
 * DESCRIPTION: Import a VCard record.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			obeyUniqueIDs - true to obey any unique ids if possible
 *			beginAlreadyRead - whether the begin statement has been read
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf		4/24/97		Initial Revision
 *			bhall	8/12/99		moved category beaming code from gromit codeline
 *          ABa     6/20/00  	Integrate Pdi library
 *
 *************************************************************/


Boolean TransferImportVCard(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, Boolean obeyUniqueIDs, Boolean beginAlreadyRead)
{
	volatile AddrDBRecordType 	newRecord;
	AddressPhoneLabels 			phoneLabel;
	UInt16						phoneField;
	UInt16 						indexNew;
	UInt16 						indexOld;
	UInt32 						uid;
	Err 						err;
	UInt32 						uniqueID;
	volatile Err 				error = 0;
	UInt16						property;
	//Char* 						addressBufferP = NULL;
	//Char* 						nameBufferP = NULL;
	UInt16						i;
	UInt16						categoryID;
	char 						categoryName[dmCategoryLength];


	// if we have ExgManager socket
	if (reader->appData != NULL)
	{
		categoryID = ((ExgSocketPtr)(reader->appData))->appData;
	}

	// Initialize a new record
	for (i=0; i < addrNumFields; i++)
		newRecord.fields[i] = NULL;	// clear the record

	newRecord.options.phones.phone1 = 0;	// Work
	newRecord.options.phones.phone2 = 1;	// Home
	newRecord.options.phones.phone3 = 2;	// Fax
	newRecord.options.phones.phone4 = 7;	// Other
	newRecord.options.phones.phone5 = 3;	// Email
	newRecord.options.phones.displayPhoneForList = ad_phone1 - firstPhoneField;

	uid = 0;

	ErrTry
	{
		TraceOutput(TL(appErrorClass, __FILE__ ":TransferImportVCard:%d", __LINE__));

		phoneField = firstPhoneField;

		if (!beginAlreadyRead)
		{
			PdiReadProperty(pdiRefNum, reader);
			beginAlreadyRead = reader->property == kPdiPRN_BEGIN_VCARD;
		}
		
		// if not "BEGIN:VCARD"
		if (!beginAlreadyRead)
			ErrThrow(exgErrBadData);
			
		PdiEnterObject(pdiRefNum, reader);
		PdiDefineResizing(pdiRefNum, reader, 16, tableMaxTextItemSize);

		TraceOutput(TL(appErrorClass, __FILE__ ":TransferImportVCard:%d", __LINE__));
		while (PdiReadProperty(pdiRefNum, reader) == 0 && (property = reader->property) != kPdiPRN_END_VCARD)
		{
			TraceOutput(TL(appErrorClass, "TransferImportVCard (PdiReadProperty): property = %s", reader->propertyName));
			switch(property)
			{
			case kPdiPRN_N:
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_name], kPdiResizableBuffer, kPdiDefaultFields);
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_firstName], kPdiResizableBuffer, kPdiDefaultFields);
				break;

			case kPdiPRN_ADR:
				{
					UInt8 n = 0;
					UInt16 totalSize;
					Char* addressPostOfficeP = NULL;
					Char* addressExtendedP = NULL;
					Char* strings[3];

					PdiReadPropertyField(pdiRefNum, reader, &addressPostOfficeP, kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, &addressExtendedP, kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_address], kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_city], kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_state], kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_zipCode], kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_country], kPdiResizableBuffer, kPdiDefaultFields);

					if (newRecord.fields[ad_address] != NULL)
					{
						strings[n++] = newRecord.fields[ad_address];
						totalSize += StrLen(newRecord.fields[ad_address]);
					}
					if (addressPostOfficeP != NULL)
					{
						strings[n++] = addressPostOfficeP;
						totalSize += StrLen(addressPostOfficeP);
					}
					if (addressExtendedP != NULL)
					{
						strings[n++] = addressExtendedP;
						totalSize += StrLen(addressExtendedP);
					}

					if (addressPostOfficeP != NULL || addressExtendedP != NULL)
					{
						Char* result = NULL;

						totalSize += (n - 1) * (sizeOf7BitChar(linefeedChr) + sizeOf7BitChar(spaceChr)) + sizeOf7BitChar(nullChr);
						if (totalSize > tableMaxTextItemSize)
						{
							totalSize = tableMaxTextItemSize;
						}
						result = MemHandleLock(MemHandleNew(totalSize));
						if (result != NULL)
						{
							*result = 0;
							while (n > 0)
							{
								n--;
								StrNCat(result, strings[n], totalSize);
								if (n != 0)
								{
									StrNCat(result, "\n ", totalSize);
								}
							}
						}
						if (addressPostOfficeP != NULL)
						{
							MemPtrFree(addressPostOfficeP);
						}
						if (addressExtendedP != NULL)
						{
							MemPtrFree(addressExtendedP);
						}
						if (newRecord.fields[ad_address] != NULL && result != NULL)
						{
							MemPtrFree(newRecord.fields[ad_address]);
						}
						if (result != NULL)
						{
							newRecord.fields[ad_address] = result;
						}
					}
				}
				break;

			case kPdiPRN_FN:
				// Take care of FN iff no name nor first name.
				if (newRecord.fields[ad_name] == NULL && newRecord.fields[ad_firstName] == NULL)
				{
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_name], kPdiResizableBuffer, kPdiDefaultFields);
				}
				break;

			case kPdiPRN_NICKNAME:
				// Take care of nickname iff no first name.
				if (newRecord.fields[ad_firstName] == NULL)
				{
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_firstName], kPdiResizableBuffer, kPdiDefaultFields);
				}
				break;

			case kPdiPRN_ORG:
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_company], kPdiResizableBuffer, kPdiConvertSemicolon);
				break;

			case kPdiPRN_TITLE:
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_title], kPdiResizableBuffer, kPdiConvertSemicolon);
				break;

			case kPdiPRN_NOTE:
				PdiDefineResizing(pdiRefNum, reader, 16, noteViewMaxLength);
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_note], kPdiResizableBuffer, kPdiNoFields);
				PdiDefineResizing(pdiRefNum, reader, 16, tableMaxTextItemSize);
				break;

			case kPdiPRN_X_PALM_CATEGORY:
				{
					Char* categoryStringP = NULL;
					PdiReadPropertyField(pdiRefNum, reader, &categoryStringP, kPdiResizableBuffer, kPdiNoFields);
					if (categoryStringP != NULL)
					{
						// Make a copy
						StrNCopy(categoryName, categoryStringP, dmCategoryLength);
						// Free the string (Imc routines allocate the space)
						MemPtrFree(categoryStringP);
						// If we ever decide to use vCard 3.0 CATEGORIES, we would need to skip additional ones here
					}
				}
				break;

			case kPdiPRN_TEL:
			case kPdiPRN_EMAIL:
				if (phoneField <= lastPhoneField)
				{
					if (reader->property == kPdiPRN_EMAIL)
					{
						phoneLabel = emailLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_FAX))
					{
						phoneLabel = faxLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_PAGER))
					{
						phoneLabel = pagerLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_CAR) || PdiParameterPairTest(reader, kPdiPAV_TYPE_CELL))
					{
						phoneLabel = mobileLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_X_X_PALM_MAIN))
					{
						phoneLabel = mainLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_HOME))
					{
						phoneLabel = homeLabel;
					}
					else if (PdiParameterPairTest(reader, kPdiPAV_TYPE_WORK))
					{
						phoneLabel = workLabel;
					}
					else
					{
						phoneLabel = otherLabel;
					}
					SetPhoneLabel(&newRecord, phoneField, phoneLabel);
					if (PdiParameterPairTest(reader, kPdiPAV_TYPE_PREF))
					{
						newRecord.options.phones.displayPhoneForList = phoneField - firstPhoneField;
					}
					PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[phoneField], kPdiResizableBuffer, kPdiNoFields);
					if (newRecord.fields[phoneField] != NULL)
					{
						phoneField++;
					}
				}
				break;

			case kPdiPRN_X_PALM_CUSTOM:
				ErrNonFatalDisplayIf(reader->customFieldNumber >= (lastRenameableLabel - firstRenameableLabel + 1),
									 "Invalid Custom Field");
				PdiReadPropertyField(pdiRefNum, reader, (Char **) &newRecord.fields[ad_custom1 + reader->customFieldNumber],
									 kPdiResizableBuffer, kPdiNoFields);
				break;

			case kPdiPRN_BEGIN_VCARD:
				TransferImportVCard(dbP, pdiRefNum, reader, obeyUniqueIDs, true);
				break;

			}
		} // end while

		// We don't have to search for the pref phone, because we are sure it is not null
		// (or it is but there was no TEL or EMAIL property in the vcard and so the
		// displayList is firstPhoneField)

		// if the company and name fields are identical, assume company only
		if (newRecord.fields[ad_name] != NULL
			&& newRecord.fields[ad_company] != NULL
			&& newRecord.fields[ad_firstName] == NULL
			&& StrCompare(newRecord.fields[ad_name], newRecord.fields[ad_company]) == 0)
		{
			MemPtrFree(newRecord.fields[ad_name]);
			newRecord.fields[ad_name] = NULL;
		}

		// Before adding the record verify that one field at least in non NULL
		for (i = 0; i < addrNumFields; i++)
		{
			if (newRecord.fields[i] != NULL)
				goto addRecord;
		}
		
		// All fields are NULL: not really an error but we must not add the record
		ErrThrow(errNone);

addRecord:
		err = AddrDBNewRecord(dbP, (AddrDBRecordType*)&newRecord, &indexNew);

		// Memory error ? 
		if (err)
			ErrThrow(exgMemError);

#ifdef VCARD_CATEGORIES
		// If a category was included, try to use it
		if (categoryName[0])
		{
			UInt16	categoryID;
			UInt16	attr;
			Err		err;

			// Get the category ID
			categoryID = CategoryFind(dbP, categoryName);

			// If it doesn't exist, and we have room, create it
			if (categoryID == dmAllCategories)
			{
				// Find the first unused category
				categoryID = CategoryFind(dbP, "");

				// If there is a slot, fill it with the name we were given
				if (categoryID != dmAllCategories)
				{
					CategorySetName(dbP, categoryID, categoryName);
				}
			}

			// Set the category for the record
			if (categoryID != dmAllCategories)
			{
				// Get the attributes
				err = DmRecordInfo(dbP, indexNew, &attr, NULL, NULL);

				// Set them to include the category, and mark the record dirty
				if ((attr & dmRecAttrCategoryMask) != categoryID)
				{
					attr &= ~dmRecAttrCategoryMask;
					attr |= categoryID | dmRecAttrDirty;
					err = DmSetRecordInfo(dbP, indexNew, &attr, NULL);
				}
			}
		}
#endif
		// Set the category for the record
		if (categoryID)
		{
			UInt16	attr;
			//Err		err;

			// Get the attributes
			/*err =*/ DmRecordInfo(dbP, indexNew, &attr, NULL, NULL);

			// Set them to include the category, and mark the record dirty
			if ((attr & dmRecAttrCategoryMask) != categoryID)
			{
				attr &= ~dmRecAttrCategoryMask;
				attr |= categoryID | dmRecAttrDirty;
				/*err =*/ DmSetRecordInfo(dbP, indexNew, &attr, NULL);
			}
		}

		// If uid was set then a unique id was passed to be used.
		if (uid != 0 && obeyUniqueIDs)
		{
			// We can't simply remove any old record using the unique id and
			// then add the new record because removing the old record could
			// move the new one.  So, we find any old record, change the new
			// record, and then remove the old one.
			indexOld = indexNew;

			// Find any record with this uid.  indexOld changes only if
			// such a record is found.
			DmFindRecordByID (dbP, uid, &indexOld);

			// Change this record to this uid.  The dirty bit is set from
			// newly making this record.
			DmSetRecordInfo(dbP, indexNew, NULL, &uid);

			// Now remove any old record.
			if (indexOld != indexNew)
			{
				DmRemoveRecord(dbP, indexOld);
			}
		}


		// Store the information necessary to navigate to the record inserted.
		DmRecordInfo(dbP, indexNew, NULL, &uniqueID, NULL);
		// DOLATER ABa: About the goto code if think I will remove this hack
		// and add a thing like;
		// if (I'm sure data is comming from exchange manager) then
		// 		PrvTransferSetGoToParams.
		// Why not a ExgSocketPtr in reader ?
		//   != NULL <=> I'm sure data is comming from exchange manager
		// Or removing this call (and last one) from Import and storing indexNew
		// in the reader structure (userdata). This will save time when importing a whole category
#if EMULATION_LEVEL != EMULATION_NONE
		// Don't call PrvTransferSetGoToParams for shell commands.  Do this by seeing which
		// input function is passed - the one for shell commands or the local one for exchange.
		if (reader->appData != NULL)
#endif
			PrvTransferSetGoToParams (dbP, (ExgSocketPtr)(reader->appData), uniqueID);
		;
		TraceOutput(TL(appErrorClass, __FILE__ ":TransferImportVCard:%d", __LINE__));
	} //end of ErrTry


	ErrCatch(inErr)
	{
		// Throw the error after the memory is cleaned up.
		error = inErr;
	} ErrEndCatch

	// Free any temporary buffers used to store the incoming data.
	for (i=0; i < addrNumFields; i++)
	{
		if (newRecord.fields[i] != NULL)
		{
			MemPtrFree(newRecord.fields[i]);
			newRecord.fields[i] = NULL;	// clear the record
		}
	}

	// misformed vCard (no BEGIN:xxx): we must stop the import
	if (error == exgErrBadData)
		return false;

	// in other case (typically a memory error) we must throw up the error
	if (error != errNone)
		ErrThrow(error);

	// if no error we must inform caller to continue iff not EOF
	return ((reader->events & kPdiEOFEventMask) == 0);


}

/************************************************************
 *
 * FUNCTION: TransferExportVCard
 *
 * DESCRIPTION: Export a record as a Imc VCard record
 *
 * PARAMETERS:
 *			dbP - pointer to the database to export the records from
 *			index - the record number to export
 *			recordP - whether the begin statement has been read
 *			outputStream - pointer to where to export the record to
 *			outputFunc - function to send output to the stream
 *			writeUniqueIDs - true to write the record's unique id
 *
 * RETURNS: nothing
 *
 *	HISTORY:
 *		08/06/97	rsf	Created by Roger Flores
 *		06/09/99	grant	Ensure that phone numbers labeled "other" aren't
 *							tagged as ";WORK" or ";HOME".
 *		08/12/99	bhall	moved category beaming code from gromit codeline
 *		10/30/99	kwk	Use TxtGetChar before calling TxtCharIsDigit.
 *         ABa     6/20/00  Integrate Pdi library
 *
 *************************************************************/

void TransferExportVCard(DmOpenRef dbP, Int16 index, AddrDBRecordType *recordP, UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs)
{
	int			i;
	UInt32			uid;
	AddrAppInfoPtr appInfoP= NULL;
	Char uidString[12];
	Boolean personOnlyAtHome = false;
	Boolean personOnlyAtWork = false;
	AddressPhoneLabels phoneLabel;
	MemHandle unnamedRecordStrH;
	Char * unnamedRecordStr;
	Char* fields[7];

	PdiWriteBeginObject(pdiRefNum, writer, kPdiPRN_BEGIN_VCARD);
	PdiWriteProperty(pdiRefNum, writer, kPdiPRN_VERSION);
	PdiWritePropertyValue(pdiRefNum, writer, (Char*)"2.1", kPdiWriteData);

	PdiWritePropertyStr(pdiRefNum, writer, "X-PALM", kPdiNoFields, 1);
	PdiWritePropertyValue(pdiRefNum, writer, (Char*) kVObjectVersion, kPdiWriteData);

	PdiWriteProperty(pdiRefNum, writer, kPdiPRN_N);

	if (recordP->fields[ad_name] != NULL ||
		recordP->fields[ad_firstName] != NULL)
	{
		fields[0] = recordP->fields[ad_name];
		fields[1] = recordP->fields[ad_firstName];
		PdiWritePropertyFields(pdiRefNum, writer, fields, 2, kPdiWriteText);
		TraceOutput(TL(appErrorClass | 5, "name = %s", recordP->fields[ad_name]));
	}
	else if (recordP->fields[ad_company] != NULL)
		// no name field, so try emitting company in N: field
	{
		PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[ad_company], kPdiWriteText);
		TraceOutput(TL(appErrorClass | 5, "name = %s", recordP->fields[ad_company]));
	}
	else
		// no company name either, so emit unnamed identifier
	{
		unnamedRecordStrH = DmGetResource(strRsc, UnnamedRecordStr);
		unnamedRecordStr = MemHandleLock(unnamedRecordStrH);

		PdiWritePropertyValue(pdiRefNum, writer, unnamedRecordStr, kPdiWriteText);
		TraceOutput(TL(appErrorClass | 5, "name = %s", unnamedRecordStr));

		MemHandleUnlock(unnamedRecordStrH);
		DmReleaseResource(unnamedRecordStrH);
	}

	if (recordP->fields[ad_address] != NULL ||
		recordP->fields[ad_city] != NULL ||
		recordP->fields[ad_state] != NULL ||
		recordP->fields[ad_zipCode] != NULL ||
		recordP->fields[ad_country] != NULL)
	{
		PdiWriteProperty(pdiRefNum, writer, kPdiPRN_ADR);
		if (!recordP->fields[ad_country])
		{
			PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_DOM, false);
		}
		// ABa: Add a tag HOME or WORK
		if (recordP->fields[ad_company])
		{
			// if there's a company name, assume work address
			PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_WORK, false);
		}
		else
		{
			// If no company name, assume home address
			PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_HOME, false);
		}
		fields[0] = fields[1] = NULL;
		fields[2] = recordP->fields[ad_address];
		fields[3] = recordP->fields[ad_city];
		fields[4] = recordP->fields[ad_state];
		fields[5] = recordP->fields[ad_zipCode];
		fields[6] = recordP->fields[ad_country];
		PdiWritePropertyFields(pdiRefNum, writer, fields, 7, kPdiWriteText);
	}

	if (recordP->fields[ad_company] != NULL)
	{
		PdiWriteProperty(pdiRefNum, writer, kPdiPRN_ORG);
		PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[ad_company], kPdiWriteText);
	}
	// Emit a title
	if (recordP->fields[ad_title] != NULL)
	{
		// We want to encode ';' with quoted-printable because we convert
		// non encoded ';' into '\n'. This change fixes the bug on ';' in TITLE
		PdiWritePropertyStr(pdiRefNum, writer, "TITLE", kPdiSemicolonFields, 1);
		PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[ad_title], kPdiWriteText);
	}
	// Emit a note
	if (recordP->fields[ad_note] != NULL)
	{
		PdiWriteProperty(pdiRefNum, writer, kPdiPRN_NOTE);
		PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[ad_note], kPdiWriteMultiline);
	}

	for (i = firstPhoneField; i <= lastPhoneField; i++)
	{
		if (recordP->fields[i] != NULL)
		{
			phoneLabel = (AddressPhoneLabels) GetPhoneLabel(recordP, i);
			if (phoneLabel == homeLabel)
			{
				if (personOnlyAtWork)
				{
					personOnlyAtWork = false;
					break;
				}
				else
				{
					personOnlyAtHome = true;
				}
			}
			else if (phoneLabel == workLabel)
			{
				if (personOnlyAtHome)
				{
					personOnlyAtHome = false;
					break;
				}
				else
				{
					personOnlyAtWork = true;
				}
			}

		}
	}

	// Now emit the phone fields
	for (i = firstPhoneField; i <= lastPhoneField; i++)
	{
		if (recordP->fields[i] != NULL)
		{
			phoneLabel = (AddressPhoneLabels) GetPhoneLabel(recordP, i);
			if (phoneLabel != emailLabel)
			{
				// The item
				PdiWriteProperty(pdiRefNum, writer, kPdiPRN_TEL);

				// Is this prefered?  Assume so if listed in the list view.
				if (recordP->options.phones.displayPhoneForList == i - firstPhoneField)
				{
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_PREF, false);
				}

				// Add a home or work tag, unless this field is labeled "other".
				// We don't want "other" phone numbers to be tagged as ";WORK" or
				// ";HOME", because then they are interpreted as "work" or "home" numbers
				// on the receiving end.
				if (phoneLabel != otherLabel)
				{
					if (personOnlyAtHome || phoneLabel == homeLabel)
						PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_HOME, false);
					else if (personOnlyAtWork || phoneLabel == workLabel)
						PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_WORK, false);
				}

				switch (phoneLabel)
				{
				case faxLabel:
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_FAX, false);
					break;

				case pagerLabel:
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_PAGER, false);
					break;

				case mobileLabel:
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_CELL, false);
					break;

				case mainLabel:
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_X_X_PALM_MAIN, false);
					
				case workLabel:
				case homeLabel:
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_VOICE, false);
					break;

				case otherLabel:
					break;
				default:
					break;
				}

				PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[i], kPdiWriteText);
			}
			else
			{
				// The item
				PdiWriteProperty(pdiRefNum, writer, kPdiPRN_EMAIL);

				// Is this prefered?  Assume so if listed in the list view.
				if (recordP->options.phones.displayPhoneForList == i - firstPhoneField)
				{
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_PREF, false);
				}

				if (personOnlyAtHome || phoneLabel == homeLabel)
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_HOME, false);
				else if (personOnlyAtWork || phoneLabel == workLabel)
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_WORK, false);

				// Now try to identify the email type by it's syntax.
				// A '@' indicates a probable internet address.
				if (StrChr(recordP->fields[i], '@') == NULL)
				{
					if (TxtCharIsDigit(TxtGetChar(recordP->fields[i], 0)))
					{
						if (StrChr(recordP->fields[i], ',') != NULL)
							PdiWriteParameterStr(pdiRefNum, writer, "", "CIS");
						// We know that a hyphen is never part of a multi-byte char.
						else if (recordP->fields[i][3] == '-')
						{
							PdiWriteParameterStr(pdiRefNum, writer, "", "MCIMail");
						}
					}
				}
				else
				{
					PdiWriteParameter(pdiRefNum, writer, kPdiPAV_TYPE_INTERNET, false);
				}
				PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[i], kPdiWriteText);
			}
		}
	}



	for (i = firstRenameableLabel; i <= lastRenameableLabel; i++)
	{
		if (recordP->fields[i] != NULL)
		{
			PdiWriteProperty(pdiRefNum, writer, kPdiPRN_X_PALM_CUSTOM);

			// Emit the custom field number
			StrIToA(uidString, i - firstRenameableLabel + 1);
			PdiWriteParameterStr(pdiRefNum, writer, "", uidString);

			if (appInfoP == NULL)
				appInfoP = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);

			// Emit the custom label name if used.  This will enable smart matching.
			if (appInfoP->fieldLabels[i][0] != nullChr)
			{
				//DOLATER ABa the Pdi library must escape or omit ":" and ";" chars
				// when beaming to an old palm
				PdiWriteParameterStr(pdiRefNum, writer, "", appInfoP->fieldLabels[i]);
			}
			PdiWritePropertyValue(pdiRefNum, writer, recordP->fields[i], kPdiWriteText);
		}
	}

	if (appInfoP != NULL)
		MemPtrUnlock(appInfoP);


	// Emit an unique id
	if (writeUniqueIDs)
	{
		// Get the record's unique id and append to the string.
		DmRecordInfo(dbP, index, NULL, &uid, NULL);
		StrIToA(uidString, uid);
		PdiWriteProperty(pdiRefNum, writer, kPdiPRN_UID);
		PdiWritePropertyValue(pdiRefNum, writer, uidString, kPdiWriteData);
	}

#ifdef VCARD_CATEGORIES
	// Emit category
	{
		Char description[dmCategoryLength];
		UInt16 attr;
		UInt16 category;

		// Get category name
		DmRecordInfo (dbP, index, &attr, NULL, NULL);
		category = attr & dmRecAttrCategoryMask;
		CategoryGetName(dbP, category, description);

		PdiWriteProperty(pdiRefNum, writer, kPdiPRN_X_PALM_CATEGORY);
		// DOLATER Pdi library will take care of this special value
		// and must omit ";" and ":" when beaming to an old Palm
		PdiWritePropertyValue(pdiRefNum, writer, description, kPdiWriteText);
	}

#endif

	PdiWriteEndObject(pdiRefNum, writer, kPdiPRN_END_VCARD);
	
	if (writer->error)
		ErrThrow(writer->error);
}


/***********************************************************************
 *
 * FUNCTION:		TransferPreview
 *
 * DESCRIPTION:	Create a short string preview of the data coming in.
 *
 * PARAMETERS:		infoP - the preview info from the command parameter block
 *						        of the sysAppLaunchCmdExgPreview launch
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         dje    8/31/00   Created
 *
 ***********************************************************************/
void TransferPreview(ExgPreviewInfoType *infoP)
{
	Err err;
	UInt16 pdiRefNum;
	PdiReaderType* reader;
	UDAReaderType* stream;
	Boolean loaded;
	Char *lastNameP = NULL, *firstNameP = NULL, *companyP = NULL;
	Boolean multiple = false;

	if (infoP->op == exgPreviewQuery)
	{
		infoP->types = exgPreviewShortString;
		return;
	}
	if (infoP->op != exgPreviewShortString)
	{
		infoP->error = exgErrNotSupported;
		return;
	}

	// if we have a description we don't have to parse the vObject
	if (infoP->socketP->description && *infoP->socketP->description)
	{
		StrNCopy(infoP->string, infoP->socketP->description, infoP->size - 1);
		infoP->string[infoP->size - 1] = 0;
		infoP->error = errNone;
		return;
	}

	err = ExgAccept(infoP->socketP);
	if (!err)
	{
		err = PrvTransferPdiLibLoad(&pdiRefNum, &loaded);
	}
	if (!err)
	{
		stream = UDAExchangeReaderNew(infoP->socketP);
		reader = PdiReaderNew(pdiRefNum, stream, kPdiOpenParser);
		reader->appData = infoP->socketP;
		if (reader)
		{
			PdiReadProperty(pdiRefNum, reader);
			if  (reader->property != kPdiPRN_BEGIN_VCARD)
				goto ParseError;
			PdiEnterObject(pdiRefNum, reader);
			PdiDefineResizing(pdiRefNum, reader, 16, tableMaxTextItemSize);

			while (PdiReadProperty(pdiRefNum, reader) == 0)
			{
				if (reader->property == kPdiPRN_BEGIN_VCARD)
				{
					multiple = true;
					break;
				}
				switch (reader->property)
				{
				case kPdiPRN_N:
					PdiReadPropertyField(pdiRefNum, reader, &lastNameP, kPdiResizableBuffer, kPdiDefaultFields);
					PdiReadPropertyField(pdiRefNum, reader, &firstNameP, kPdiResizableBuffer, kPdiDefaultFields);
					break;
				case kPdiPRN_FN:
					// Take care of FN iff no name nor first name.
					if (lastNameP == NULL && firstNameP == NULL)
						PdiReadPropertyField(pdiRefNum, reader, &lastNameP, kPdiResizableBuffer, kPdiDefaultFields);
					break;
				case kPdiPRN_NICKNAME:
					// Take care of nickname iff no first name.
					if (firstNameP == NULL)
						PdiReadPropertyField(pdiRefNum, reader, &firstNameP, kPdiResizableBuffer, kPdiDefaultFields);
					break;
				case kPdiPRN_ORG:
					PdiReadPropertyField(pdiRefNum, reader, &companyP, kPdiResizableBuffer, kPdiConvertSemicolon);
					break;
					// ignore other properties
				}
			}

			if (multiple)
			{
				MemHandle resH = DmGetResource(strRsc, ExgMultipleDescriptionStr);
				void *desc = MemHandleLock(resH);

				StrNCopy(infoP->string, desc, infoP->size);
				infoP->string[infoP->size - 1] = chrNull;
				MemHandleUnlock(resH);
				DmReleaseResource(resH);
			}
			else
			{
				// if the company and last name fields are identical, assume company only
				if (lastNameP != NULL
					&& companyP != NULL
					&& firstNameP == NULL
					&& StrCompare(lastNameP, companyP) == 0)
				{
					MemPtrFree(lastNameP);
					lastNameP = NULL;
				}

				// write the short string preview
				// DOLATER dje - Use the SortByCompany preference (getting it out of the database because we
				// 					might not have access to globals).
				if (firstNameP || lastNameP)
				{
					infoP->string[0] = chrNull;
					if (firstNameP)
						StrNCat(infoP->string, firstNameP, infoP->size);
					if (firstNameP && lastNameP)
						StrNCat(infoP->string, " ", infoP->size);
					if (lastNameP)
						StrNCat(infoP->string, lastNameP, infoP->size);
				}
				else if (companyP)
				{
					StrNCopy(infoP->string, companyP, infoP->size);
					infoP->string[infoP->size - 1] = chrNull;
				}
				else
				{
				ParseError:
					err = exgErrBadData;
				}
			}

			// clean up
			if (lastNameP)
				MemPtrFree(lastNameP);
			if (firstNameP)
				MemPtrFree(firstNameP);
			if (companyP)
				MemPtrFree(companyP);

			ExgDisconnect(infoP->socketP, err);
		}
		PdiReaderDelete(pdiRefNum, &reader);
		UDADelete(stream);
		PrvTransferPdiLibUnload(pdiRefNum, loaded);
	}
	infoP->error = err;
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:		PrvTransferPdiLibLoad
 *
 * DESCRIPTION:		Load Pdi library
 * PARAMETERS:		a pointer to an integer: the refNum of the libary
 *						return whether the library had to be loaded (and therefore
 *								needs to be unloaded)
 *
 * RETURNED:		An error if library is not found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		4/10/00		Created
 *
 ***********************************************************************/
Err PrvTransferPdiLibLoad(UInt16* refNum, Boolean *loadedP)
{
	Err	error;

	// Load the Pdi library

	// Check if the library was pre-loaded (this is useful if we can
	// be called from another app via an action code and want to use an existing
	// instance of the library in case our caller has already loaded it)
	*loadedP = false;
	error = SysLibFind(kPdiLibName, refNum);
	if (error != 0)
	{
		error = SysLibLoad(sysResTLibrary, sysFileCPdiLib, refNum);
		if (! error)
			*loadedP = true;
	}
	if (error)
	{
		// We're here because the Pdi library failed to load.
		// Inform the user or do something else defensive here.
		ErrNonFatalDisplay(kPdiLibName " not found");
		return error;
	}
	error = PdiLibOpen(*refNum);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:		PrvTransferPdiLibUnload
 *
 * DESCRIPTION:		Unload Pdi library
 * PARAMETERS:		The refnum of the pdi library
 *						Whether the library was loaded (and therefore needs to be unloaded)
 *
 * RETURNED:		An error if library is not found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		4/10/00		Created
 *
 ***********************************************************************/
void PrvTransferPdiLibUnload(UInt16 refNum, Boolean loaded)
{
	if (PdiLibClose(refNum) == 0)
	{
		TraceOutput(TL(appErrorClass, "Removing library..."));
		if (loaded)
			SysLibRemove(refNum);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvTransferSendRecordTryCatch
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				recordNum - the record number to send
 * 				recordP - pointer to the record to send
 * 				outputStream - place to send the data
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *         ABa    4/10/00   Load Pdi library
 *
 ***********************************************************************/
Err PrvTransferSendRecordTryCatch (DmOpenRef dbP, Int16 recordNum, AddrDBRecordPtr recordP, UDAWriterType* media)
{
	Err error = 0;
	UInt16 pdiRefNum;
	Boolean loaded;

	PdiWriterType* writer;

	if ((error = PrvTransferPdiLibLoad(&pdiRefNum, &loaded)))
	{
		ErrNonFatalDisplay("Can't load Pdi library");
		return error;
	}

	writer = PdiWriterNew(pdiRefNum, media, kPdiPalmCompatibility);
	if (writer)
	{

		// An error can happen anywhere during the send process.  It's easier just to
		// catch the error.  If an error happens, we must pass it into ExgDisconnect.
		// It will then cancel the send and display appropriate ui.
		ErrTry
		{
			TransferExportVCard(dbP, recordNum, recordP, pdiRefNum, writer, true);
			error = UDAWriterFlush(media);
		}


		ErrCatch(inErr)
		{
			error = inErr;
		} ErrEndCatch

		PdiWriterDelete(pdiRefNum, &writer);
	}
	else
	{
		error = exgMemError;
	}
	PrvTransferPdiLibUnload(pdiRefNum, loaded);

	return error;
}


/***********************************************************************
 *
 * FUNCTION:		PrvTransferCleanFileName
 *
 * DESCRIPTION:		Remove dot characters in file name but not the least
 * PARAMETERS:		a pointer to a string
 *
 * RETURNED:		String parameter doesn't contains superfluous dot characters
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		7/28/00		Created
 *
 ***********************************************************************/
static void PrvTransferCleanFileName(Char* ioFileName)
{
	Char* 	mayBeLastDotP;
	Char*  	lastDotP;
	UInt32	chrFullStopSize = TxtCharSize(chrFullStop);	
    
	// prevent NULL & empty string
	if (ioFileName == NULL || *ioFileName == 0)
		return;

	// remove dot but not the last one
	mayBeLastDotP = StrChr(ioFileName, 	chrFullStop);
	while ((lastDotP = StrChr(mayBeLastDotP + chrFullStopSize, chrFullStop)))
	{
		// remove the dot
		StrCopy(mayBeLastDotP, mayBeLastDotP + chrFullStopSize);
		mayBeLastDotP = lastDotP - chrFullStopSize;
	}
}

/***********************************************************************
 *
 * FUNCTION:    PrvTransferSendCategoryTryCatch
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:	dbP - pointer to the database to add the record to
 * 				categoryNum - the category of records to send
 * 				exgSocketP - the exchange socket used to send
 * 				index - the record number of the first record in the category to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *         ABa     6/20/00  Integrate Pdi library
 *
 ***********************************************************************/
Err PrvTransferSendCategoryTryCatch (DmOpenRef dbP, UInt16 categoryNum, UDAWriterType*  media, UInt16 index)
{
	volatile Err error = 0;
	volatile MemHandle outRecordH = 0;
	AddrDBRecordType outRecord;

	UInt16 pdiRefNum;
	PdiWriterType* writer;
	Boolean loaded;

	if ((error = PrvTransferPdiLibLoad(&pdiRefNum, &loaded)))
		return error;

	writer = PdiWriterNew(pdiRefNum, media, kPdiPalmCompatibility);
	if (writer)
	{
		// An error can happen anywhere during the send process.  It's easier just to
		// catch the error.  If an error happens, we must pass it into ExgDisconnect.
		// It will then cancel the send and display appropriate ui.
		ErrTry
		{
			// Loop through all records in the category.
			while (DmSeekRecordInCategory(dbP, &index, 0, dmSeekForward, categoryNum) == 0)
			{
				// Emit the record.  If the record is private do not emit it.
				if (AddrDBGetRecord(dbP, index, &outRecord, (MemHandle*)&outRecordH) == 0)
				{
					TransferExportVCard(dbP, index, &outRecord, pdiRefNum, writer, true);
					MemHandleUnlock(outRecordH);
				}
				index++;
			}
			error = UDAWriterFlush(media);
		}

		ErrCatch(inErr)
		{
			error = inErr;
			if (outRecordH)
				MemHandleUnlock(outRecordH);
		} ErrEndCatch

		PdiWriterDelete(pdiRefNum, &writer);
	}
	else
	{
		error = exgMemError;
	}
	PrvTransferPdiLibUnload(pdiRefNum, loaded);

	return error;
}


/***********************************************************************
 *
 * FUNCTION:		PrvTransferSetGoToParams
 *
 * DESCRIPTION:	Store the information necessary to navigate to the
 *                record inserted into the launch code's parameter block.
 *
 * PARAMETERS:		dbP        - pointer to the database to add the record to
 *					exgSocketP - parameter block passed with the launch code
 *					uniqueID   - unique id of the record inserted
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		10/17/97	Created
 *
 ***********************************************************************/
void PrvTransferSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID)
{
	UInt16		recordNum;
	UInt16		cardNo;
	LocalID 	dbID;


	if (! uniqueID) return;

	DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL);

	// The this the the first record inserted, save the information
	// necessary to navigate to the record.
	if (! exgSocketP->goToParams.uniqueID)
	{
		DmFindRecordByID (dbP, uniqueID, &recordNum);

		exgSocketP->goToCreator = AddressBookCreator;
		exgSocketP->goToParams.uniqueID = uniqueID;
		exgSocketP->goToParams.dbID = dbID;
		exgSocketP->goToParams.dbCardNo = cardNo;
		exgSocketP->goToParams.recordNum = recordNum;
	}

	// If we already have a record then make sure the record index
	// is still correct.  Don't update the index if the record is not
	// in your the app's database.
	else if (dbID == exgSocketP->goToParams.dbID &&
			 cardNo == exgSocketP->goToParams.dbCardNo)
	{
		DmFindRecordByID (dbP, exgSocketP->goToParams.uniqueID, &recordNum);

		exgSocketP->goToParams.recordNum = recordNum;
	}
}


