/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MemoTransfer.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *      Memo Book routines to transfer records.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <UIResources.h>
#include <Category.h>
#include <Form.h>
#include <ErrorMgr.h>
#include <TraceMgr.h>
#include <PalmLocale.h>
#include <PalmUtils.h>

#include "MemoDB.h"
#include "MemoMain.h"
#include "MemoRsc.h"


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define identifierLengthMax			40
#define mimeVersionString			"MIME-Version: 1.0\015\012"
#define mimeMultipartString 		"Content-type: multipart/mixed;"
#define mimeBoundaryString 			"boundary="
#define memoSuffix					("." memoExtension)
#define simpleBoundary				"simple boundary"
#define delimiter					"--" simpleBoundary
#define crlf						"\015\012"

#define importBufferMaxLength		 80

#define stringZLen 					-1 	// pass to WriteFunc to calculate strlen


// Stream interface to exgsockets to optimize performance
#define maxStreamBuf 				512  // MUST BE LARGER than importBufferMaxLength

typedef struct StreamType {
	ExgSocketPtr socket;
	UInt16 pos;
	UInt16 len;
	UInt16 bufSize;
	Char   buf[maxStreamBuf];
} StreamType;


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void			PrvMemoImportMimeCleanup(DmOpenRef dbP, UInt32 firstRecordID, void* inputStream, UInt16 numRecordsReceived, UInt16* numRecordsReceivedP);
static void			PrvStreamInit(StreamType *streamP, ExgSocketPtr exgSocketP);
static void			PrvStreamFlush(StreamType *streamP);
static UInt32		PrvStreamWrite(StreamType *streamP, const Char * stringP, Int32 length, Err *errP);
static UInt32		PrvStreamRead(StreamType * streamP, Char *bufP, UInt32 length, Err *errP);
static ExgSocketPtr PrvStreamSocket(StreamType *streamP);
static UInt32		PrvReadFunction(const void * stream, Char * bufferP, UInt32 length);
static UInt32		PrvWriteFunction(void * stream, const Char * const bufferP, Int32 length);
static void			PrvTransferCleanFileName(Char* ioFileName);
static void			PrvSetDescriptionAndFilename(Char * textP, Char **descriptionPP, MemHandle *descriptionHP, Char **filenamePP, MemHandle *filenameHP, const Char * const prefix);
static Err			PrvMemoSendRecordTryCatch (DmOpenRef dbP, Int16 recordNum, MemoDBRecordPtr recordP, ExgSocketPtr exgSocketP);
static Err			PrvMemoSendCategoryTryCatch (DmOpenRef dbP, UInt16 categoryNum, ExgSocketPtr exgSocketP, UInt16 index);
static void			PrvMemoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID);
static Err			PrvReadThroughCRLF(ReadFunctionF inputFunc, void * inputStreamP, Char * bufferP, UInt16 * bufferLengthP);
static void			PrvMemoImportFinishRecord(DmOpenRef dbP, UInt16 indexNew, MemHandle *newRecordHPtr, UInt16 *newRecordSizePtr, void * inputStream);

/************************************************************
 *
 * FUNCTION: MemoImportMime
 *
 * DESCRIPTION: Import a Mime record.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			obeyUniqueIDs - true to obey any unique ids if possible
 *			beginAlreadyRead - whether the begin statement has been read
 *			numRecordsRecievedP - number of records received
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			rsf		4/24/97			Initial Revision
 *			bob		01/26/98		re-wrote MIME parser part to get delimiters right
 *			grant	6/25/99			Return count of records received in numRecordsReceivedP.
 *			kwk		06/25/99		Moved return out of ErrTry block.
 *			FPa		11/22/00		Fixed ErrTry/Catch/Throw problems
 *
 *************************************************************/
extern Boolean
MemoImportMime(DmOpenRef dbR, void * inputStream, ReadFunctionF inputFunc,
			   Boolean UNUSED_PARAM(obeyUniqueIDs), Boolean UNUSED_PARAM(beginAlreadyRead), UInt16 *numRecordsReceivedP, Char* descriptionP, UInt16 descriptionSize)
{
	char *c;
	char boundaryString[69+2] = "";
	MemHandle newRecordH = NULL;
	Char * newRecordP;
	MemHandle newHandle;						// Used to follow resized records which move
	UInt16 indexNew = dmMaxRecordIndex;
	DmOpenRef dbP = dbR;
	Err err = 0;
	Char buffer[importBufferMaxLength + 1];
	UInt16 bufferLength = 0;
	UInt16 charsRead;
	UInt16 charsToWrite;
	UInt16 newRecordSize = 0;
	Char * nextCrChr;
	int addCr;
	Char * boundaryP;
	Char * boundaryEndP;
	UInt16 numRecordsReceived = 0;
	UInt32 firstRecordID = 0;

	// Keep the buffer always null terminated so we can use string functions on it.
	buffer[importBufferMaxLength] = nullChr;

	// Read chars into the buffer
	charsRead = inputFunc( inputStream, buffer, importBufferMaxLength - bufferLength);
	bufferLength += charsRead;
	buffer[bufferLength] = nullChr;

	if (charsRead == 0)
	{
		*numRecordsReceivedP = 0;
		return false;
	}

	// An error happens usually due to no memory.  It's easier just to
	// catch the error.  If an error happens, we remove the last record.
	// Then we throw a second time so the caller receives it and displays a message.
	ErrTry
	{
		// MIME start, find MIME ID and version
		if (StrNCompare(buffer, mimeVersionString, StrLen(mimeVersionString)) == 0)
		{
			// Remove the MIME header
			MemMove(buffer, &buffer[StrLen(mimeVersionString)], bufferLength - StrLen(mimeVersionString));
			bufferLength -= StrLen(mimeVersionString);

			// Read chars into the buffer
			charsRead = inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - bufferLength);
			bufferLength += charsRead;
			buffer[bufferLength] = nullChr;

			// scan header for a multi-part identifier
			// skip anything else until we get an entirely blank line
			do {
				if (StrNCompare(buffer, mimeMultipartString, StrLen(mimeMultipartString)) == 0)
				{
					// found a multi-part header, parse out the boundary string

					// PREVIEW Aba: Here we know that the memo is multipart => several memos
					if (descriptionP)
					{
						MemHandle headerStringH;
						Char* 	  headerStringP;

						headerStringH = DmGetResource(strRsc, FindMemoHeaderStr);
						headerStringP = MemHandleLock(headerStringH);
						StrCopy(descriptionP, headerStringP);
						MemHandleUnlock(headerStringH);
						DmReleaseResource(headerStringH);

						return true;
					}

					boundaryP = StrStr(buffer, mimeBoundaryString);
					boundaryP += StrLen(mimeBoundaryString);

					// Remove the boundary stuff so we can read in more into the buffer
					MemMove(buffer, boundaryP, &buffer[bufferLength] - boundaryP);
					bufferLength = (&buffer[bufferLength] - boundaryP);

					// Read chars into the buffer
					charsRead = inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - bufferLength);
					bufferLength += charsRead;
					buffer[bufferLength] = nullChr;

					boundaryP = buffer;
					if (*boundaryP == '"')
					{
						boundaryP++;
						boundaryEndP = StrChr(boundaryP, '"');
					}
					else
					{
						boundaryEndP = StrChr(boundaryP, crChr);
					}
					if (boundaryEndP == NULL)
					{
						ErrThrow(exgErrBadData);
					}
					boundaryString[0] = '-';
					boundaryString[1] = '-';
					MemMove(&boundaryString[2], boundaryP, boundaryEndP - boundaryP);
					boundaryString[boundaryEndP - boundaryP + 2] = nullChr;

					c = StrChr(boundaryEndP, crChr);
					if (c == NULL)
					{
						ErrThrow(exgErrBadData);
					}
					c += sizeOf7BitChar(crChr) + sizeOf7BitChar(linefeedChr);

					// Remove the boundary stuff so we can read in more into the buffer
					MemMove(buffer, c, &buffer[bufferLength] - c);
					bufferLength = (&buffer[bufferLength] - c);
				}
				else
				{
					// just an ordinary header line, skip it
					err = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
					if (err)
						ErrThrow(err);
				}

				// Read chars into the buffer
				charsRead = inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - bufferLength);
				bufferLength += charsRead;
				buffer[bufferLength] = nullChr;

				// stop at blank line by itself or EOF
			} while (buffer[0] != crChr && buffer[0] != nullChr);
			
			// We've now parsed the MIME header.  Preamble, segments, and postamble below.
		} // end of MIME parser

		do {
			// find the boundary and remove it, along with any header info in the body part
			if (*boundaryString != nullChr)
			{
				// Keep reading until we find a boundary
				while (buffer[0] != nullChr && StrNCompare(buffer, boundaryString, StrLen(boundaryString)) != 0)
				{
					err = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
					if (err)
						ErrThrow(err);
				}

				// Remove the boundary by removing all text until the end of the line.
				err = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
				if (err)
					ErrThrow(err);

				while (buffer[0] != nullChr && buffer[0] != crChr)
				{
					err = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
					if (err)
						ErrThrow(err);
				}
				err = PrvReadThroughCRLF(inputFunc, inputStream, buffer, &bufferLength);
				if (err)
					ErrThrow(err);
			}

			// could be that everything was header, and we're out of data.
			// weird error, but MemHandle it.
			if (bufferLength == 0)
			{
				ErrThrow(exgErrBadData);
			}


			addCr = 0;
			while (bufferLength > 0 &&
				   (*boundaryString == nullChr || StrNCompare(buffer, boundaryString, StrLen(boundaryString)) != 0))
			{
				// find CR or end of buffer
				nextCrChr = StrChr(buffer, crChr);
				if (nextCrChr != NULL)
					charsToWrite = nextCrChr - buffer;
				else
					charsToWrite = bufferLength;

				// PREVIEW Aba: Here we have the first line and we can exit
				if (descriptionP)
				{
					if (charsToWrite >= descriptionSize)
						charsToWrite = descriptionSize - 1;

					StrNCopy(descriptionP, buffer, charsToWrite);
					descriptionP[charsToWrite] = '\0';
					return true;
				}

				// if we're going to overflow record, close it out (leave room for terminating null)
				if (newRecordSize + charsToWrite + addCr > memoMaxLength)
				{
					// since we try to stop parsing at each CR, and most records from other sources (MIME)
					// should have a CR at least every 76 characters, we probably don't have to worry about
					// word wrap.  Still, beaming a lot of just plain text could break records on random
					// boundaries...
					PrvMemoImportFinishRecord(dbP, indexNew, &newRecordH, &newRecordSize, inputStream);
					addCr = 0;
					numRecordsReceived++;
				}

				// Make a record if we need one
				if (newRecordH == NULL)
				{
					indexNew = dmMaxRecordIndex;
					newRecordH = DmNewRecord(dbP, (UInt16 *)&indexNew, bufferLength);
					if (newRecordH == 0)
						ErrThrow(exgMemError);
					newRecordSize = 0;
				}

				// Write the buffer out to the record
				newHandle = DmResizeRecord(dbP, indexNew, newRecordSize + charsToWrite + addCr);
				if (newHandle)
					newRecordH = newHandle;
				else
					ErrThrow(exgMemError);

				newRecordP = MemHandleLock(newRecordH);
				if (addCr != 0)
					DmWrite(newRecordP, newRecordSize++, "\n", 1);
				DmWrite(newRecordP, newRecordSize, buffer, charsToWrite);
				newRecordSize += charsToWrite;
				MemHandleUnlock(newRecordH);

				// Remove the chars written so we can read more into the buffer
				if (nextCrChr != NULL)
				{
					if (charsToWrite < importBufferMaxLength-1)
					{
						MemMove(buffer, nextCrChr+2, bufferLength-(charsToWrite+2));	// delete LF
						bufferLength -= charsToWrite+2;
					}
					else
						// CR/LF was split by end of buffer, so DON'T delete the CR, catch it next time 'round
					{
						MemMove(buffer, nextCrChr, bufferLength-(charsToWrite));		// don't delete CR or LF
						bufferLength -= charsToWrite;
						nextCrChr = NULL;
					}
				}
				else
					buffer[bufferLength = 0] = nullChr;

				// Now read more
				charsRead = inputFunc( inputStream, &buffer[bufferLength], importBufferMaxLength - bufferLength);
				bufferLength += charsRead;
				buffer[bufferLength] = nullChr;

				if (nextCrChr != NULL)
					addCr = 1;
				else
					addCr = 0;
			}	// end of segment parser

			// Set the category for the record
			if (PrvStreamSocket(inputStream)->appData) {
				UInt16	attr;
				//Err		err;
				UInt16	categoryID = PrvStreamSocket(inputStream)->appData;

				// Get the attributes
				/*err = */DmRecordInfo(dbP, indexNew, &attr, NULL, NULL);

				// Set them to include the category, and mark the record dirty
				if ((attr & dmRecAttrCategoryMask) != categoryID) {
					attr &= ~dmRecAttrCategoryMask;
					attr |= categoryID | dmRecAttrDirty;
					err = DmSetRecordInfo(dbP, indexNew, &attr, NULL);
				}
			}

			PrvMemoImportFinishRecord(dbP, indexNew, &newRecordH, &newRecordSize, inputStream);
			numRecordsReceived++;

			// save the uniqueID of the first record we loaded
			// we will goto this record when we are done (after sorting)
			if (!firstRecordID)
			{
				// Store the information necessary to navigate to the record inserted.
				DmRecordInfo(dbP, indexNew, NULL, &firstRecordID, NULL);
			}

			// Now that the record is imported check if we need to import any more

			// Stop if there isn't any more input
			if (bufferLength == 0)
				break;

			// Stop if the boundary is followed by "--"
			if ((*boundaryString != nullChr)
				&& bufferLength >= StrLen(boundaryString) + 2
				&& StrNCompare(&buffer[StrLen(boundaryString)], "--", 2) == 0)
				break;

		} while (true);	// end of segment parser
	}	// end of Try

	ErrCatch(inErr)
	{
		// PREVIEW Aba
		if (descriptionP)
			return false;

		// Remove any incomplete record
		if (inErr && indexNew != dmMaxRecordIndex)
			DmRemoveRecord(dbP, indexNew);

		// if we got at least one record, sort and set goto parameters...
		if (firstRecordID)
		{
			MemoSort(dbP);
			PrvMemoSetGoToParams (dbP, PrvStreamSocket(inputStream), firstRecordID);
		}
	
		// return number of records received
		*numRecordsReceivedP = numRecordsReceived;
	
		if ( inErr != exgMemError )
			PrvMemoImportMimeCleanup(dbP, firstRecordID, inputStream, numRecordsReceived, numRecordsReceivedP);

		ErrThrow(inErr);
	} ErrEndCatch

	PrvMemoImportMimeCleanup(dbP, firstRecordID, inputStream, numRecordsReceived, numRecordsReceivedP);

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    MemoSendRecord
 *
 * DESCRIPTION: Beam or send a record.
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 * 					recordNum - the record to send
 *					prefix - the scheme with ":" suffix and optional "?" prefix
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *		   dje     4/21/00	Add Send support
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
extern void MemoSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix)
{
	MemoDBRecordPtr recordP;
	MemHandle recordH;
	MemHandle descriptionH;
	Err error;
	ExgSocketType exgSocket;
	MemHandle nameH;


	// important to init structure to zeros...
	MemSet(&exgSocket, sizeof(exgSocket), 0);

	// Form a description of what's being sent.  This will be displayed
	// by the system send dialog on the sending and receiving devices.
	recordH = DmQueryRecord (dbP, recordNum);
	recordP = (MemoDBRecordType *) MemHandleLock(recordH);

	// Set the description to be the beginning of the memo
	descriptionH = NULL;
	exgSocket.description = NULL;

	// Set the exg description to the record's description.
	PrvSetDescriptionAndFilename(&recordP->note, &exgSocket.description,
							  &descriptionH, &exgSocket.name, &nameH, prefix);

	// ABa: Clean superfluous '.' characters
	PrvTransferCleanFileName(exgSocket.name);

	exgSocket.length = MemHandleSize(recordH);		// rough guess
	//exgSocket.target = sysFileCMemo;		// commented out 4/24/00 dje
	exgSocket.type = (Char *)memoMIMEType;
	error = ExgPut(&exgSocket);   // put data to destination
	if (!error)
	{
		error = PrvMemoSendRecordTryCatch(dbP, recordNum, recordP, &exgSocket);

		ExgDisconnect(&exgSocket, error);
	}


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
	MemHandleUnlock(recordH);


	return;
}


/***********************************************************************
 *
 * FUNCTION:    MemoSendCategory
 *
 * DESCRIPTION: Beam or send all visible records in a category.
 *
 * PARAMETERS:		 dbP - pointer to the database to add the record to
 * 					 categoryNum - the category of records to send
 *					 prefix - the scheme with ":" suffix and optional "?" prefix
 *					 noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if any records are found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   5/9/97   Initial Revision
 *		   dje     4/21/00	Add Send support
 *         dje     4/24/00  Don't specify target creator ID
 *
 ***********************************************************************/
extern void MemoSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID)
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
		exgSocket.name = MemPtrNew(StrLen(prefix) + StrLen(description) + StrLen(memoSuffix) + sizeOf7BitChar('\0'));
		if (exgSocket.name)
		{
			StrCopy(exgSocket.name, prefix);
			StrCat(exgSocket.name, description);
			StrCat(exgSocket.name, memoSuffix);
		}

		// ABa: Clean superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);

		exgSocket.length = 0;		// rough guess
		//exgSocket.target = sysFileCMemo;		// commented out 4/24/00 dje
		exgSocket.type = (Char *)memoMIMEType;
		error = ExgPut(&exgSocket);   // put data to destination

		if (!error)
		{
			error = PrvMemoSendCategoryTryCatch (dbP, categoryNum, &exgSocket, index);

			ExgDisconnect(&exgSocket, error);
		}

		// Clean up
		if (exgSocket.name)
			MemPtrFree(exgSocket.name);
	}
	else
		FrmAlert(noDataAlertID);

	if (databaseReopened)
		DmCloseDatabase(dbP);

	return;
}


/***********************************************************************
 *
 * FUNCTION:		MemoReceiveData
 *
 * DESCRIPTION:		Receives data into the output field using the Exg API
 *
 * PARAMETERS:		dbP - database to put received memos in
 *						exgSocketP - socket from the app code sysAppLaunchCmdExgReceiveData
 *						numRecordsReceivedP - number of records received is returned here
 *
 * RETURNED:		error code or zero for no error.
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	6/25/99	Keep count of received records and return in numRecordsReceivedP
 *
 ***********************************************************************/
extern Err MemoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt16 *numRecordsReceivedP)
{
	volatile Err err;
	UInt16 numRecordsReceived;
	StreamType stream;

	// initialize new record count
	ErrNonFatalDisplayIf(numRecordsReceivedP == NULL, "NULL numRecordsReceivedP");
	*numRecordsReceivedP = 0;

	PrvStreamInit( &stream, exgSocketP);

	// accept will open a progress dialog and wait for your receive commands
	err = ExgAccept(exgSocketP);

	if (!err)
	{
		// Catch errors receiving records.  The import routine will clean up the
		// incomplete record.  This routine displays an error message.
		ErrTry
		{
			// Keep importing records until it can't
			while (MemoImportMime(dbP, &stream, PrvReadFunction, false, false, &numRecordsReceived, NULL, 0))
			{
				*numRecordsReceivedP += numRecordsReceived;
			};

// catch the records from the final MemoImportMime
			*numRecordsReceivedP += numRecordsReceived;
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
	
		ExgDisconnect(exgSocketP, err); // closes transfer dialog
		err = errNone;	// error was reported, so don't return it
	}

	return err;
}


/************************************************************
 *
 * FUNCTION: MemoExportMime
 *
 * DESCRIPTION: Export a record as a Imc Mime record
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
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf		8/6/97		Initial Revision
 *			vsm		12/3/02		If the note is encoded in anything other than
 *								usAscii, then send the correct charset rather
 *								than ISO-8859-1.
 *
 *************************************************************/

void MemoExportMime(DmOpenRef UNUSED_PARAM(dbP), Int16 UNUSED_PARAM(index), MemoDBRecordType * recordP,
					void * outputStream, WriteFunctionF outputFunc, Boolean UNUSED_PARAM(writeUniqueIDs),
					Boolean outputMimeInfo)
{
	Char * c;
	Char * eolP;
	//UInt32 len;
	CharEncodingType	curEncoding;

	// Write out all of the memo.  All linefeeds must be replaced with CRLF combos.
	c = &recordP->note;

	if (outputMimeInfo)
	{

		if ((curEncoding = TxtStrEncoding((Char const *)c)) != charEncodingAscii)
		{
			outputFunc(outputStream, "Content-Type: Text/plain; charset=" , stringZLen);
			outputFunc(outputStream, TxtEncodingName(curEncoding), stringZLen);
			outputFunc(outputStream, crlf, stringZLen);
		}
		outputFunc(outputStream, crlf, stringZLen);
	}

	while (*c != '\0')
	{
		eolP = StrChr(c, linefeedChr);
		if (eolP)
		{
			/*len = */outputFunc( outputStream, c, eolP - c);

			outputFunc(outputStream, crlf, stringZLen);
			c = eolP + sizeOf7BitChar(linefeedChr);
		}
		else if (*c != '\0')
		{
			eolP = StrChr(c, '\0');
			/*len = */outputFunc( outputStream, c, eolP - c);

			c = eolP;
		}
	}
	outputFunc(outputStream, crlf, stringZLen);	// always end with an extra crlf
}

/***********************************************************************
 *
 * FUNCTION:		MemoTransferPreview
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
 *         ABa    11/10/00   Created
 *
 ***********************************************************************/
void MemoTransferPreview(ExgPreviewInfoType *infoP)
{
	volatile Err 	err;
	UInt16 			numRecordsReceived;
	StreamType 		stream;

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

	PrvStreamInit(&stream, infoP->socketP);

	err = ExgAccept(infoP->socketP);

	if (!err)
	{
		ErrTry
		{
			MemoImportMime((DmOpenRef) NULL, &stream, PrvReadFunction, false, false, &numRecordsReceived, infoP->string, infoP->size);
		}

		ErrCatch(inErr)
		{
			err = inErr;
		} ErrEndCatch

		ExgDisconnect(infoP->socketP, err); // closes transfer dialog
	}

	infoP->error = err;
}


//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvStreamInit
 *
 * DESCRIPTION: Function to put Initialize a stream socket.
 *
 * PARAMETERS:  streamP - the output stream
 *				exgSocketP - pointer to an intitialized exgSocket
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static void PrvStreamInit(StreamType *streamP, ExgSocketPtr exgSocketP)
{
	streamP->socket = exgSocketP;
	streamP->bufSize = maxStreamBuf;
	streamP->pos = 0;
	streamP->len = 0;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamFlush
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static void PrvStreamFlush(StreamType *streamP)
{
	Err err = 0;
	while (streamP->len && !err)
		streamP->len -= ExgSend(streamP->socket,streamP->buf,streamP->len,&err);

}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamWrite
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  streamP - the output stream
 *				stringP - the string to put
 *
 * RETURNED:    nothing
 *				If the all the string isn't sent an error is thrown using ErrThrow.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static UInt32 PrvStreamWrite(StreamType *streamP, const Char * stringP, Int32 length, Err *errP)
{
	UInt32 count = 0;
	*errP = 0;

	while (count < length && !*errP)
	{
		if (streamP->len < streamP->bufSize)
		{
			streamP->buf[streamP->len++] = *stringP++;
			count++;
		}
		else
			streamP->len -= ExgSend(streamP->socket, streamP->buf, streamP->len, errP);
	}
	return count;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamRead
 *
 * DESCRIPTION: Function to get a character from the input stream.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    a character of EOF if no more data
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static UInt32 PrvStreamRead(StreamType * streamP, Char *bufP, UInt32 length, Err *errP)
{
	UInt32 count = 0;

	*errP = 0;
	while (count < length)
	{
		if (streamP->pos < streamP->len)
			bufP[count++] = streamP->buf[streamP->pos++];
		else
		{	streamP->pos = 0;
			streamP->len = ExgReceive(streamP->socket, streamP->buf, streamP->bufSize, errP);
		if (!streamP->len || *errP)
			break;
		}
	}
	return count;
}

/***********************************************************************
 *
 * FUNCTION:    PrvStreamSocket
 *
 * DESCRIPTION: returns the socket from a stream.
 *
 * PARAMETERS:  streamP - the output stream
 *
 * RETURNED:    The socket associated with the stream
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *		   gavin   10/5/99   Initial revision
 *
 ***********************************************************************/
static ExgSocketPtr PrvStreamSocket(StreamType *streamP)
{
	return streamP->socket;
}

/***********************************************************************
 *
 * FUNCTION:    GetChar
 *
 * DESCRIPTION: Function to get a character from the exg transport.
 *
 * PARAMETERS:  exgSocketP - the exg connection
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *
 ***********************************************************************/
static UInt32 PrvReadFunction(const void * stream, Char * bufferP, UInt32 length)
{
	Err err;
	UInt32 bytesRead;
	bytesRead = PrvStreamRead((StreamType *)stream, bufferP, length, &err);
	if (err)
		ErrThrow(err);
	return bytesRead;
}


/***********************************************************************
 *
 * FUNCTION:    PutString
 *
 * DESCRIPTION: Function to put a string to the exg transport.
 *
 * PARAMETERS:  exgSocketP - the exg connection
 *					 stringP - the string to put
 *
 * RETURNED:    nothing
 *					 If the all the string isn't sent an error is thrown using ErrThrow.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/15/97   Initial Revision
 *
 ***********************************************************************/
static UInt32 PrvWriteFunction(void * stream, const Char * const bufferP, Int32 length)
{
	UInt32 len;
	Err err;

	// passing -1 length will assume a null terminated string
	if (length == -1)  length = StrLen(bufferP);

	len = PrvStreamWrite( stream, bufferP, length, &err);

	// If the bytes were not sent throw an error.
	if ((len == 0 && length > 0) || err)
		ErrThrow(err);

	return len;
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
 * FUNCTION:    PrvSetDescriptionAndFilename
 *
 * DESCRIPTION: Derive and allocate a decription and filename from some text.
 *				The filename will be a URL which includes the specified scheme.
 *
 * PARAMETERS:  textP - the text string to derive the names from
 *					 descriptionPP - pointer to set to the allocated description
 *					 descriptionHP - MemHandle to set to the allocated description
 *					 filenamePP - pointer to set to the allocated filename
 *					 filenameHP - MemHandle to set to the allocated description
 *					 prefix - the scheme with ":" suffix and optional "?" prefix
 *
 * RETURNED:    a description and filename are allocated and the pointers are set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   11/4/97   Initial Revision
 *
 ***********************************************************************/
static void PrvSetDescriptionAndFilename(Char * textP, Char **descriptionPP,
									  MemHandle *descriptionHP, Char **filenamePP, MemHandle *filenameHP, const Char * const prefix)
{
	Char * descriptionP;
	Int16 descriptionSize;
	Coord descriptionWidth;
	Boolean descriptionFit;
	Char * spaceP;
	Char * filenameP;
	MemHandle resourceH;
	Char * resourceP;
	UInt8 filenameLength;
	UInt8 schemeLength;
	Coord unused;


	descriptionSize = StrLen(textP);
	WinGetDisplayExtent(&descriptionWidth, &unused);
	FntCharsInWidth (textP, &descriptionWidth, &descriptionSize, &descriptionFit);

	if (descriptionSize > 0)
	{
		*descriptionHP = MemHandleNew(descriptionSize+sizeOf7BitChar('\0'));
		if (*descriptionHP)
		{
			descriptionP = MemHandleLock(*descriptionHP);
			MemMove(descriptionP, textP, descriptionSize);
			descriptionP[descriptionSize] = nullChr;
		}
	}
	else
	{
		*descriptionHP = DmGetResource(strRsc, BeamDescriptionStr);
		descriptionP = MemHandleLock(*descriptionHP);
	}


	if (descriptionSize > 0)
	{
		// Now form a file name.  Use only the first word or two.
		spaceP = StrChr(descriptionP, spaceChr);
		if (spaceP)
			// Check for a second space
			spaceP = StrChr(spaceP + sizeOf7BitChar(spaceChr), spaceChr);

		// If at least two spaces were found then use only that much of the description.
		// If less than two spaces were found then use all of the description.
		if (spaceP)
			filenameLength = spaceP - descriptionP;
		else
			filenameLength = StrLen(descriptionP);


		// Allocate space and form the filename
		schemeLength = StrLen(prefix);
		*filenameHP = MemHandleNew(schemeLength + filenameLength + StrLen(memoSuffix) + sizeOf7BitChar('\0'));
		filenameP = MemHandleLock(*filenameHP);
		if (filenameP)
		{
			StrCopy(filenameP, prefix);
			MemMove(&filenameP[schemeLength], descriptionP, filenameLength);
			MemMove(&filenameP[schemeLength + filenameLength], memoSuffix,
					StrLen(memoSuffix) + sizeOf7BitChar('\0'));
		}
	}
	else
	{
		resourceH = DmGetResource(strRsc, BeamFilenameStr);
		resourceP = MemHandleLock(resourceH);

		// Allocate space and form the filename
		filenameLength = StrLen(resourceP);
		schemeLength = StrLen(prefix);
		*filenameHP = MemHandleNew(schemeLength + filenameLength + sizeOf7BitChar('\0'));
		filenameP = MemHandleLock(*filenameHP);
		if (filenameP)
		{
			StrCopy(filenameP, prefix);
			StrCat(filenameP, resourceP);
		}

		MemHandleUnlock(resourceH);
		DmReleaseResource(resourceH);
	}


	*descriptionPP = descriptionP;
	*filenamePP = filenameP;
}


/***********************************************************************
 *
 * FUNCTION:    PrvMemoSendRecordTryCatch
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	 dbP - pointer to the database to add the record to
 * 				 recordNum - the record number to send
 * 				 recordP - pointer to the record to send
 * 				 exgSocketP - the exchange socket used to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *
 ***********************************************************************/
static Err PrvMemoSendRecordTryCatch (DmOpenRef dbP, Int16 recordNum,
								   MemoDBRecordPtr recordP, ExgSocketPtr exgSocketP)
{
	volatile Err error = 0;
	StreamType stream;

	PrvStreamInit(&stream, exgSocketP);

	// An error can happen anywhere during the send process.  It's easier just to
	// catch the error.  If an error happens, we must pass it into ExgDisconnect.
	// It will then cancel the send and display appropriate ui.
	ErrTry
	{
		MemoExportMime(dbP, recordNum, recordP, &stream, PrvWriteFunction, true, false);
	}

	ErrCatch(inErr)
	{
		error = inErr;
	} ErrEndCatch

	PrvStreamFlush( &stream);
	return error;
}


/***********************************************************************
 *
 * FUNCTION:    PrvMemoSendCategoryTryCatch
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:		dbP - pointer to the database to add the record to
 * 					categoryNum - the category of records to send
 * 					exgSocketP - the exchange socket used to send
 * 					index - the record number of the first record in the category to send
 *
 * RETURNED:    0 if there's no error
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/11/97  Initial Revision
 *
 ***********************************************************************/
static Err PrvMemoSendCategoryTryCatch (DmOpenRef dbP, UInt16 categoryNum,
									 ExgSocketPtr exgSocketP, UInt16 index)
{
	volatile Err error = 0;
	volatile MemHandle outRecordH = 0;
	MemoDBRecordType *outRecordP;
	StreamType stream;

	// An error can happen anywhere during the send process.  It's easier just to
	// catch the error.  If an error happens, we must pass it into ExgDisconnect.
	// It will then cancel the send and display appropriate ui.
	ErrTry
	{

		PrvStreamInit(&stream, exgSocketP);

		// Write out the beginning of a multipart mime message
		PrvWriteFunction(&stream,
					  mimeVersionString
					  "Content-type: multipart/mixed; boundary=\"" simpleBoundary "\"" crlf crlf, stringZLen);

		// Loop through all records in the category.
		while (DmSeekRecordInCategory(dbP, &index, 0, dmSeekForward, categoryNum) == 0)
		{
			// Emit the record.  If the record is private do not emit it.
			outRecordH = DmQueryRecord (dbP, index);

			if (outRecordH != 0)
			{
				outRecordP = (MemoDBRecordType *) MemHandleLock(outRecordH);

				// Emit a mime boundary
				PrvWriteFunction(&stream, delimiter crlf, stringZLen);

				MemoExportMime(dbP, index, outRecordP, &stream, PrvWriteFunction, true, true);

				MemHandleUnlock(outRecordH);
			}

			index++;
		}
		outRecordH = 0;
		dbP = 0;

		// All done.  Write out an epilogue.
		PrvWriteFunction(&stream, delimiter "--" crlf crlf, stringZLen);
	}

	ErrCatch(inErr)
	{
		error = inErr;

		if (outRecordH)
			MemHandleUnlock(outRecordH);
	} ErrEndCatch

	PrvStreamFlush(&stream);

	return error;
}

/***********************************************************************
 *
 * FUNCTION:		PrvMemoSetGoToParams
 *
 * DESCRIPTION:	Store the information necessary to navigate to the
 *                record inserted into the launch code's parameter block.
 *
 * PARAMETERS:		 dbP        - pointer to the database to add the record to
 *						 exgSocketP - parameter block passed with the launch code
 *						 uniqueID   - unique id of the record inserted
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/17/97	Created
 *
 ***********************************************************************/
static void PrvMemoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID)
{
	UInt16	recordNum;
	UInt16	cardNo;
	LocalID 	dbID;


	if (! uniqueID) return;

	DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL);

	// The this the the first record inserted, save the information
	// necessary to navigate to the record.
	if (! exgSocketP->goToParams.uniqueID)
	{
		DmFindRecordByID (dbP, uniqueID, &recordNum);

		exgSocketP->goToCreator = sysFileCMemo;
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


/************************************************************
 *
 * FUNCTION: PrvReadThroughCRLF
 *
 * DESCRIPTION: Consume data up to and including the next CRLF.
 *
 * PARAMETERS:
 *			inputStreamP	- pointer to where to import from
 *			bufferP - where the input stream is stored
 *			bufferLengthP - the length of bufferP used
 *
 * RETURNED: error code or zero for no error.
 *
 * ASSUMPTIONS:
 *			Buffer is full when called
 *  		Buffer is big enough to hold a full line (including LF)
 *			  ...so CR/LF will never split
 *	END CONDITION:
 *			Buffer is full when routine exits.
 *
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bob	1/26/98  initial revision
 *
 *************************************************************/
static Err PrvReadThroughCRLF(ReadFunctionF inputFunc, void * inputStreamP, Char * bufferP, UInt16 * bufferLengthP)
{
	char *c;
	UInt16 charsRead;
	Err err = 0;
	Boolean gotOne = false;

	while (*bufferLengthP > 0 && !gotOne)
	{
		c = StrChr(bufferP, crChr);
		if (c == NULL)
			c = &bufferP[*bufferLengthP];		// end of the buffer
		else if (c < bufferP + *bufferLengthP - 1)		// guard against buffer splitting cr/lf
		{
			c += sizeOf7BitChar(crChr) + sizeOf7BitChar(linefeedChr);
			gotOne = true;
		}

		// Consume everything up to the CR/NULL
		MemMove(bufferP, c, &bufferP[*bufferLengthP] - c);
		*bufferLengthP = &bufferP[*bufferLengthP] - c;

		// Read in more chars
		charsRead = inputFunc(inputStreamP, bufferP + *bufferLengthP, importBufferMaxLength - *bufferLengthP);
		*bufferLengthP += charsRead;
		bufferP[*bufferLengthP] = nullChr;
	}

	return err;
}


/************************************************************
 *
 * FUNCTION: PrvMemoImportFinishRecord
 *
 * DESCRIPTION: Make sure record is null terminated, and close it.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			indexNew	- index of new record
 *			newRecordH - MemHandle to new record
 *			newRecordSize - bytes currently in new record
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			bob	1998-02-05	Moved out of MemoImportMime
 *
 *************************************************************/
static void PrvMemoImportFinishRecord(DmOpenRef dbP, UInt16 indexNew, MemHandle *newRecordHPtr, UInt16 *newRecordSizePtr, void * inputStream)
{
	Char * newRecordP;
	MemHandle newHandle;						// Used to follow resized records which move


	ErrNonFatalDisplayIf(*newRecordHPtr == NULL, "Null record MemHandle.");

	// Make sure the record is nullChr terminated
	newRecordP = MemHandleLock(*newRecordHPtr);
	if (newRecordP[*newRecordSizePtr - sizeof(char)] != nullChr)
	{
		MemHandleUnlock(*newRecordHPtr);
		newHandle = DmResizeRecord(dbP, indexNew, *newRecordSizePtr + sizeOf7BitChar(nullChr));
		if (newHandle)
			*newRecordHPtr = newHandle;
		else
			ErrThrow(exgMemError);

		newRecordP = MemHandleLock(*newRecordHPtr);
		DmWrite(newRecordP, *newRecordSizePtr, "", sizeOf7BitChar(nullChr));
	}

	// let the record go
	MemHandleUnlock(*newRecordHPtr);
	DmReleaseRecord(dbP, indexNew, true);


	*newRecordHPtr = NULL;
	*newRecordSizePtr = 0;
}


/************************************************************
 *
 * FUNCTION: PrvMemoImportMimeCleanup
 *
 * DESCRIPTION: Cleanup function for MemoImportMime
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			firstRecordID - uniqueID of the first record we loaded
 *			inputStream	- pointer to where to import the record from
 *			numRecordsReceived - number of records received
 *			numRecordsReceivedP - pointer to the number of records received
 *
 * RETURNS: None
 *
 *	REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			FPa		11/22/00		Initial Revision
 *
 *************************************************************/
void PrvMemoImportMimeCleanup(DmOpenRef dbP, UInt32 firstRecordID, void* inputStream, UInt16 numRecordsReceived, UInt16* numRecordsReceivedP)
{
	// if we got at least one record, sort and set goto parameters...
	if (firstRecordID)
	{
		MemoSort(dbP);
		PrvMemoSetGoToParams (dbP, PrvStreamSocket(inputStream), firstRecordID);
	}

	// return number of records received
	*numRecordsReceivedP = numRecordsReceived;
}
