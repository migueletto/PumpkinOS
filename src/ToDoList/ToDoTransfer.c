/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoTransfer.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *      To Do routines to transfer records.
 *
 *****************************************************************************/

#include <PalmOS.h>

#include <PdiLib.h>
#include <UDAMgr.h>
#include <TraceMgr.h>

#include <PalmUtils.h>

#include "ToDo.h"
#include "ToDoRsc.h"

#define identifierLengthMax		40
#define tempStringLengthMax		20
#define defaultPriority				1
#define incompleteFlag				0x7F
#define todoSuffix					".vcs"
#define todoMIMEType				"text/x-vCalendar"

// Aba: internal version of vCalendar. Must be updated
// the export side of the vCalendar code evoluate

#define kVObjectVersion				"4.0"

extern Char * GetToDoNotePtr (ToDoDBRecordPtr recordP);	// needed to see if ToDo empty

/***********************************************************************
 *
 * FUNCTION:		PrvPdiLibLoad
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
static Err PrvPdiLibLoad(UInt16* refNum, Boolean *loadedP)
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
		TraceOutput(TL(appErrorClass, "Loading Pdi library."));
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
 * FUNCTION:		PrvPdiLibUnload
 *
 * DESCRIPTION:		Unload Pdi library
 * PARAMETERS:		The refnum of the pdi library 
 *						Whether the library was loaded (and therefore needs to be unloaded)
 *
 * RETURNED:		NONE
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			ABa		4/10/00		Created
 *
 ***********************************************************************/
static void PrvPdiLibUnload(UInt16 refNum, Boolean loaded)
{
	if (PdiLibClose(refNum) == 0)
	{
		TraceOutput(TL(appErrorClass, "Unloading Pdi library."));
		if (loaded)
			SysLibRemove(refNum);
	}
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

/************************************************************
 *
 * FUNCTION: MatchDateToken
 *
 * DESCRIPTION: Extract date from the given string
 *
 * PARAMETERS:
 *		tokenP	-	string ptr from which to extract
 *		dateP		-	ptr where to store date (optional)
 *
 * RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			djk		2/2/98	Copied code from Date Book
 *
 *************************************************************/
static void MatchDateToken (const char* tokenP,	DateType * dateP)
{
    char			identifier[identifierLengthMax];
    int			nv;

    // Use identifier[] as a temp buffer to copy parts of the vCal DateTime
    // so we can convert them to correct form.  This date portion
    // is 4 chars (date) + 2 chars (month) + 2 chars (day) = 8 chars long

    // Read the Year
    StrNCopy(identifier, tokenP, 4);
    identifier[4] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < firstYear || lastYear < nv)
        nv = firstYear;
    dateP->year = nv - firstYear;
    tokenP += StrLen(identifier) * sizeof(Char);

    // Read the Month
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || monthsInYear < nv)
        nv = 1;
    dateP->month = nv;
    tokenP += StrLen(identifier) * sizeof(Char);

    // Read the Day
    StrNCopy(identifier, tokenP, 2);
    identifier[2] = nullChr;
    nv = StrAToI(identifier);
    // Validate the number and use it.
    if (nv < 1 || 31 < nv)
        nv = 1;
    dateP->day = nv;
    tokenP += StrLen(identifier) * sizeof(Char);
}


/***********************************************************************
 *
 * FUNCTION:    SetDescriptionAndFilename
 *
 * DESCRIPTION: Derive and allocate a decription and filename from some text.
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
static void SetDescriptionAndFilename(Char * textP, Char **descriptionPP,
                                      MemHandle *descriptionHP, Char **filenamePP, MemHandle *filenameHP,
                                      const Char * const prefix)
{
    Char * descriptionP = NULL;
    Int16 descriptionSize;
    Int16 descriptionWidth;
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
        DmReleaseResource(*descriptionHP);
        *descriptionHP = NULL;			// so the resource isn't freed
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
        *filenameHP = MemHandleNew(schemeLength + filenameLength + StrLen(todoSuffix) + sizeOf7BitChar('\0'));
        filenameP = MemHandleLock(*filenameHP);
        if (filenameP)
        {
			StrCopy(filenameP, prefix);
            MemMove(&filenameP[schemeLength], descriptionP, filenameLength);
            MemMove(&filenameP[schemeLength + filenameLength], todoSuffix,
                    StrLen(todoSuffix) + sizeOf7BitChar('\0'));
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
 * FUNCTION:    ToDoSendRecordTryCatch
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
static Err ToDoSendRecordTryCatch (DmOpenRef dbP, Int16 recordNum,
      ToDoDBRecordPtr recordP, UDAWriterType* media)
{
    volatile Err error = 0;


    UInt16 pdiRefNum;

    PdiWriterType* writer;
    Boolean loaded;

    if ((error = PrvPdiLibLoad(&pdiRefNum, &loaded)))
        return error;

    writer = PdiWriterNew(pdiRefNum, media, kPdiPalmCompatibility);
    if (writer)
    {

        // An error can happen anywhere during the send process.  It's easier just to
        // catch the error.  If an error happens, we must pass it into ExgDisconnect.
        // It will then cancel the send and display appropriate ui.
        ErrTry
        {
		    PdiWriteBeginObject(pdiRefNum, writer, kPdiPRN_BEGIN_VCALENDAR);
		    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_VERSION);
		    PdiWritePropertyValue(pdiRefNum, writer, (Char*)"1.0", kPdiWriteData);

			PdiWritePropertyStr(pdiRefNum, writer, "X-PALM", kPdiNoFields, 1);
			PdiWritePropertyValue(pdiRefNum, writer, (Char*) kVObjectVersion, kPdiWriteData);

            ToDoExportVCal(dbP, recordNum, recordP, pdiRefNum, writer, true);
		    PdiWriteEndObject(pdiRefNum, writer, kPdiPRN_END_VCALENDAR);
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
	PrvPdiLibUnload(pdiRefNum, loaded);
    return error;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendRecord
 *
 * DESCRIPTION: Send a record.
 *
 * PARAMETERS:	 dbP - pointer to the database
 * 				 recordNum - the record to send
 *				 prefix - the scheme with ":" suffix and optional "?" prefix
 *				 noDataAlertID - alert to put up if there is nothing to send
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  5/9/97    Initial Revision
 *
 ***********************************************************************/
extern void ToDoSendRecord (DmOpenRef dbP, Int16 recordNum, const Char * const prefix, UInt16 noDataAlertID)
{
    ToDoDBRecordPtr recordP;
    MemHandle recordH;
    MemHandle descriptionH = NULL;
    Err error;
    ExgSocketType exgSocket;
    MemHandle nameH = NULL;
    Boolean empty;
    UDAWriterType* media;
    
    // important to init structure to zeros...
    MemSet(&exgSocket, sizeof(exgSocket), 0);

    // Form a description of what's being sent.  This will be displayed
    // by the system send dialog on the sending and receiving devices.
    recordH = (MemHandle) DmGetRecord(dbP, recordNum);
    recordP = (ToDoDBRecordPtr) MemHandleLock(recordH);

    // If the description field is empty and the note field is empty,
    // consider the record empty.
    empty = (! recordP->description) && (! *GetToDoNotePtr(recordP));

    if (!empty)
    {
        // Set the exg description to the record's description.
        SetDescriptionAndFilename(&recordP->description, &exgSocket.description,
                                  &descriptionH, &exgSocket.name, &nameH, prefix);

		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);
		TraceOutput(TL(appErrorClass, "ToDoSendRecord: description = %s, name = %s", exgSocket.description, exgSocket.name));

        exgSocket.length = MemHandleSize(recordH) + 100;		// rough guess
        
        
        // NOTE: ABa To test that the DateBook sublaunches the ToDo app
        // when parsing VTODO, just comment out the next line
        // And do the same in ToDoSendCategory
        exgSocket.target = sysFileCToDo;	// include target since todo and date book share an extension
        exgSocket.type = (Char *)todoMIMEType;
        
        error = ExgPut(&exgSocket);   // put data to destination
		// ABa: Changes to use new streaming mechanism 
        media = UDAExchangeWriterNew(&exgSocket,  512);
        if (!error)
        {
        	if (media)
	            error = ToDoSendRecordTryCatch(dbP, recordNum, recordP, media);
	        else
	        	error = exgMemError;
	        	
            // Release the record before the database is sorted in loopback mode.
            if (recordH)
            {
                MemHandleUnlock(recordH);
                DmReleaseRecord(dbP, recordNum, false);
                recordH = NULL;
            }

            ExgDisconnect(&exgSocket, error);
        }
        
        if (media)
	        UDADelete(media);
 
	    // Clean up
	    if (descriptionH)
	        MemHandleFree(descriptionH);
	    if (nameH)
	        MemHandleFree(nameH);
   }
    else
        FrmAlert(noDataAlertID);

    if (recordH)
    {
        MemHandleUnlock(recordH);
        DmReleaseRecord(dbP, recordNum, false);
    }

    return;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendCategoryTryCatch
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
static Err ToDoSendCategoryTryCatch (DmOpenRef dbP, UInt16 categoryNum,
    UDAWriterType*  media, UInt16 recordNum)
{
    volatile Err error = 0;
    volatile MemHandle outRecordH = 0;
    ToDoDBRecordPtr outRecordP;

    UInt16 pdiRefNum;
    PdiWriterType* writer;
    Boolean loaded;

    if ((error = PrvPdiLibLoad(&pdiRefNum, &loaded)))
        return error;

    writer = PdiWriterNew(pdiRefNum, media, kPdiPalmCompatibility);
    if (writer)
    {
        // An error can happen anywhere during the send process.  It's easier just to
        // catch the error.  If an error happens, we must pass it into ExgDisconnect.
        // It will then cancel the send and display appropriate ui.
        ErrTry
        {
		    PdiWriteBeginObject(pdiRefNum, writer, kPdiPRN_BEGIN_VCALENDAR);
		    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_VERSION);
		    PdiWritePropertyValue(pdiRefNum, writer, (Char*)"1.0", kPdiWriteData);
            // Loop through all records in the category.
	        while (DmSeekRecordInCategory(dbP, &recordNum, 0, dmSeekForward, categoryNum) == 0)
	        {
	            // Emit the record.  If the record is private do not emit it.
	            outRecordH = (MemHandle) DmQueryRecord(dbP, recordNum);
	
	            if (outRecordH != 0)
	            {
	                outRecordP = (ToDoDBRecordPtr) MemHandleLock(outRecordH);
	
	                ToDoExportVCal(dbP, recordNum, outRecordP, pdiRefNum, writer, true);
	
	                MemHandleUnlock(outRecordH);
	            }
	
	            recordNum++;
	        }
		    PdiWriteEndObject(pdiRefNum, writer, kPdiPRN_END_VCALENDAR);
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
	PrvPdiLibUnload(pdiRefNum, loaded);

    return error;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoSendCategory
 *
 * DESCRIPTION: Send all visible records in a category.
 *
 * PARAMETERS:  dbP - pointer to the database
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
 *
 ***********************************************************************/
extern void ToDoSendCategory (DmOpenRef dbP, UInt16 categoryNum, const Char * const prefix, UInt16 noDataAlertID)
{
    Err error;
    Char description[dmCategoryLength];
    UInt16 recordNum;
    Boolean foundAtLeastOneRecord;
    ExgSocketType exgSocket;
    UInt16 mode;
    LocalID dbID;
    UInt16 cardNo;
    Boolean databaseReopened;
    UDAWriterType* media;

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
    recordNum = 0;
    foundAtLeastOneRecord = false;
    while (true)
    {
        if (DmSeekRecordInCategory(dbP, &recordNum, 0, dmSeekForward, categoryNum) != 0)
            break;

        foundAtLeastOneRecord = DmQueryRecord(dbP, recordNum) != 0;
        if (foundAtLeastOneRecord)
            break;


        recordNum++;
    }
    // We should send the category because there's at least one record to send.
    if (foundAtLeastOneRecord)
    {
        // Form a description of what's being sent.  This will be displayed
        // by the system send dialog on the sending and receiving devices.
        CategoryGetName (dbP, categoryNum, description);
        exgSocket.description = description;

        // Now form a file name
        exgSocket.name = MemPtrNew(StrLen(prefix) + StrLen(description) + StrLen(todoSuffix) + sizeOf7BitChar('\0'));
        if (exgSocket.name)
        {
			StrCopy(exgSocket.name, prefix);
            StrCat(exgSocket.name, description);
            StrCat(exgSocket.name, todoSuffix);
        }


		//ABa: remove superfluous '.' characters
		PrvTransferCleanFileName(exgSocket.name);

        exgSocket.length = 0;		// rough guess
        
        
        exgSocket.target = sysFileCToDo;	// include target since todo and date book share an extension
        exgSocket.type = (Char *)todoMIMEType;
        error = ExgPut(&exgSocket);   // put data to destination

        media = UDAExchangeWriterNew(&exgSocket, 512);
        if (!error)
        {
        	if (media)
	            error = ToDoSendCategoryTryCatch (dbP, categoryNum, media, recordNum);
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

    return;
}


/************************************************************
 *
 * FUNCTION: ToDoImportVEvent
 *
 * DESCRIPTION: Import a VCal record of type vEvent.
 *
 * This function doesn't exist on the device because
 * the Datebook imports vEvent records.  On the emulator
 * this function exists for linking completeness.
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
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	9/12/97		Created
 *
 *************************************************************/

static Boolean ToDoImportVEvent(DmOpenRef UNUSED_PARAM(dbP), UInt16 UNUSED_PARAM(pdiRefNum), PdiReaderType* UNUSED_PARAM(reader), 
	Boolean UNUSED_PARAM(obeyUniqueIDs), Boolean UNUSED_PARAM(beginAlreadyRead), UInt32 * UNUSED_PARAM(uniqueIDP))
{
	return false;
}

/***********************************************************************
 *
 * FUNCTION:		ToDoSetGoToParams
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
 *			ABa	08/18/00 	make it public for use in ToDo.c
 *
 ***********************************************************************/
extern void ToDoSetGoToParams (DmOpenRef dbP, ExgSocketPtr exgSocketP, UInt32 uniqueID)
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

        exgSocketP->goToCreator = sysFileCToDo;
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


/***********************************************************************
 *
 * FUNCTION:		ReceiveData
 *
 * DESCRIPTION:	Receives data into the output field using the Exg API
 *
 * PARAMETERS:		exgSocketP, socket from the app code
 *						 sysAppLaunchCmdExgReceiveData
 *
 * RETURNED:		error code or zero for no error.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			???	???		Created
 *			art	10/17/97	Added "go to" record logic
 *			bhall	09/27/99	Now always read until no more data
 *								(previously datebook called us once per event)
 *
 ***********************************************************************/
extern Err ToDoReceiveData(DmOpenRef dbP, ExgSocketPtr exgSocketP)
{
    volatile Err err = 0;
    UInt16 pdiRefNum = sysInvalidRefNum;
    PdiReaderType* reader = NULL;
    UDAReaderType* stream = NULL;
    Boolean loaded;

    // accept will open a progress dialog and wait for your receive commands
    if ((err = ExgAccept(exgSocketP)) != 0)
    	return err;

    if ((err = PrvPdiLibLoad(&pdiRefNum, &loaded)))
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
    
    // Catch errors receiving records.  The import routine will clean up the
    // incomplete record.  This routine passes the error to ExgDisconnect
    // which displays appropriate ui.

    ErrTry
    {
    
        // Keep importing records until it can't
        while(ToDoImportVCal(dbP, pdiRefNum, reader, false, false, ToDoImportVEvent)){};
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
		PrvPdiLibUnload(pdiRefNum, loaded);
		
	ExgDisconnect(exgSocketP, err); // closes transfer dialog
	err = errNone;	// error was reported, so don't return it

    return err;
}


/************************************************************
 *
 * FUNCTION: ToDoImportVToDo
 *
 * DESCRIPTION: Import a VCal record of type vToDo
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			obeyUniqueIDs - true to obey any unique ids if possible
 *			beginAlreadyRead - whether the begin statement has been read
 *			uniqueIDP - (returned) id of record inserted.
 *
 * RETURNS: true if the input was read
 *
 *	REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97	Created
 *			art	10/17/97	Added parameter to return unique id
 *
 *************************************************************/

extern Boolean ToDoImportVToDo(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
   Boolean obeyUniqueIDs, Boolean beginAlreadyRead, UInt32 * uniqueIDP)
{
    UInt16 indexNew;
    UInt16 indexOld;
    UInt32 uid;
    volatile ToDoItemType newToDo;
    int   nv;
    char * fieldP;
    //Boolean firstLoop = true;
    volatile Err error = 0;
    char categoryName[dmCategoryLength];
    UInt16	categoryID = dmUnfiledCategory;
    Boolean oneProperty = false;	// Aba: true iff one property was found in the note

    *uniqueIDP = 0;
    categoryName[0] = 0;

	TraceOutput(TL(appErrorClass, "ToDoImportVToDo: BEGIN"));
    // Read in the vEvent entry
    if (!beginAlreadyRead)
    {
        PdiReadProperty(pdiRefNum, reader);
        beginAlreadyRead = reader->property == kPdiPRN_BEGIN_VTODO;
    }
    if (!beginAlreadyRead)
        return false;

    // Ennter the object
    PdiEnterObject(pdiRefNum, reader);
    
    // Initialize the record to default values
    *((UInt16 *) &newToDo.dueDate) = toDoNoDueDate;
    newToDo.priority = defaultPriority;
    newToDo.priority &= incompleteFlag;
    newToDo.description = NULL;
    newToDo.note = NULL;
    fieldP = NULL;
    uid = 0;

    // An error happens usually due to no memory.  It's easier just to
    // catch the error.  If an error happens, we remove the last record.
    // Then we throw a second time so the caller receives it and displays a message.
    ErrTry
    {
        while (PdiReadProperty(pdiRefNum, reader) == 0 && reader->property != kPdiPRN_END_VTODO)
        {
			TraceOutput(TL(appErrorClass, "ToDoImportVToDo: property = %s", reader->propertyName));
			switch(reader->property)
			{
            // Handle Priority tag
			case kPdiPRN_PRIORITY:
                PdiReadPropertyField(pdiRefNum, reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
                if (fieldP != NULL)
                {
                    nv = StrAToI(fieldP);
                    nv = min(nv, toDoMaxPriority);
                    newToDo.priority = nv | (newToDo.priority & completeFlag);
                }
                oneProperty = true;
				break;
		
            // Handle the due date.
            case kPdiPRN_DUE:
                PdiReadPropertyField(pdiRefNum, reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
                if (fieldP != NULL)
                {
                    // Extract due date
                    MatchDateToken(fieldP, (DateType*)&newToDo.dueDate);
                }
                oneProperty = true;
                break;


            // Handle the two cases that indicate completed
            //the Status:completed property
            case kPdiPRN_STATUS:
                PdiReadPropertyField(pdiRefNum, reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
                if (fieldP != NULL)
                {
                    if (StrCaselessCompare(fieldP, "COMPLETED") == 0)
                    {
                        newToDo.priority |=  completeFlag;
                    }
                }
                oneProperty = true;
                break;

            // and the date/time completed property
            case kPdiPRN_COMPLETED:
                newToDo.priority |=  completeFlag;
                oneProperty = true;
                break;

            // Handle the description
            case kPdiPRN_DESCRIPTION:
                PdiReadPropertyField(pdiRefNum, reader, (Char**) &newToDo.description, kPdiResizableBuffer, kPdiDefaultFields);
                oneProperty = true;
            	break;

            // Treat attachments as notes
            case kPdiPRN_ATTACH:
                // Note: vCal permits attachments of types other than text, specifically
                // URLs and Content ID's.  At the moment, wee will just treat both of these
                // as text strings
                PdiReadPropertyField(pdiRefNum, reader, (Char**) &newToDo.note, kPdiResizableBuffer, kPdiDefaultFields);
                oneProperty = true;
                break;

            // read in the category
            case kPdiPRN_CATEGORIES:
                PdiReadPropertyField(pdiRefNum, reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
                if (fieldP != NULL)
                {
                    char *spot = fieldP;

                    // If the category was not a predefined vCal catval,
                    // we need to skip the leading special category name mark ("X-")
                	if (spot[0] == 'X' && spot[1] == '-')
                	{
                		spot += 2;
                	}

                    // Make a copy, leaving room for the terminator, cropping if necessary.
                    StrNCopy(categoryName, spot, dmCategoryLength - 1);

                    // Make sure it is null terminated.
                    categoryName[dmCategoryLength - 1] = 0;
                }
                oneProperty = true;
	            break;

            // read in the unique identifier
            case kPdiPRN_UID:
                PdiReadPropertyField(pdiRefNum, reader, &fieldP, kPdiResizableBuffer, kPdiDefaultFields);
                if (fieldP != NULL)
                {
                	uid = StrAToI(fieldP);
                    // Check the uid for reasonableness.
                    if (uid < (dmRecordIDReservedRange << 12))
                        uid = 0;
	            }
                oneProperty = true;
	            break;
	        }
        }
        
        if (fieldP != NULL)
	        MemPtrFree(fieldP);
	        
	    // Non syntax error but empty object: don't create it
	    if (oneProperty == false)
	    	ErrThrow(errNone);


        // Make sure that there are values for description and note
        if (newToDo.description == NULL)
        {

            newToDo.description = (Char *) MemPtrNew(sizeOf7BitChar('\0'));
            *newToDo.description = '\0' ;
        }

        if (newToDo.note == NULL)
        {
            // Need to add bound checking here so we don't read in
            // more chars than the max note size
            newToDo.note = (Char *) MemPtrNew(sizeOf7BitChar('\0'));
            *newToDo.note = '\0' ;
        }


        // Set the category for the record (we saved category ID in appData field)
        if (((ExgSocketPtr)(reader->appData))->appData)
        {
            categoryID = ((ExgSocketPtr)(reader->appData))->appData & dmRecAttrCategoryMask;
        }
        // we really need to recognize the vCal category specified, our category picker needs to somehow
        // know the default category. Without that support, we need to always put things into unfiled by
        // default because that is what we show the user. This logic really needs to run before the ask dialog
        // comes up, but we have not yet parsed the data then... Hmmmm
#ifdef OLD_CATEGORY_TRANSPORT
        // If a category was included, try to use it
        else if (categoryName[0]) {

            // Get the category ID
            categoryID = CategoryFind(dbP, categoryName);

            // If it doesn't exist, and we have room, create it
            if (categoryID == dmAllCategories) {
                // Find the first unused category
                categoryID = CategoryFind(dbP, "");

                // If there is a slot, fill it with the name we were given
                if (categoryID != dmAllCategories) {
                    CategorySetName(dbP, categoryID, categoryName);
                }
                else
                    cateoryID = dmUnfiledCategory; // reset to default
            }
        }
#endif	// OLD_CATEGORY_TRANSPORT

        // Write the actual record
        TraceOutput(TL(appErrorClass, "Add the record"));
        if (ToDoNewRecord(dbP, (ToDoItemType*)&newToDo, categoryID, &indexNew))
            ErrThrow(exgMemError);

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

        // Return the unique id of the record inserted.
        DmRecordInfo(dbP, indexNew, NULL, uniqueIDP, NULL);
    }

    ErrCatch(inErr)
    {
        // Throw the error after the memory is cleaned winUp.
        error = inErr;
    } ErrEndCatch


    // Free any temporary buffers used to store the incoming data.
    if (newToDo.note) MemPtrFree(newToDo.note);
    if (newToDo.description) MemPtrFree(newToDo.description);

    if (error)
        ErrThrow(error);

    return ((reader->events & kPdiEOFEventMask) == 0);
}


/************************************************************
 *
 * FUNCTION: ToDoImportVCal
 *
 * DESCRIPTION: Import a VCal record of type vEvent and vToDo
 *
 * The Datebook handles vCalendar records.  Any vToDo records
 * are sent to the ToDo app for importing.
 *
 * This routine doesn't exist for the device since the Datebook
 * is sent vCal data.  The Datebook will To Do any vToDo records
 * via an action code.  This routine is only for the simulator
 * because the Datebook can't run at the same time.
 *
 * PARAMETERS:
 *			dbP - pointer to the database to add the record to
 *			inputStream	- pointer to where to import the record from
 *			inputFunc - function to get input from the stream
 *			obeyUniqueIDs - true to obey any unique ids if possible
 *			beginAlreadyRead - whether the begin statement has been read
 *			vToDoFunc - function to import vToDo records
 *						on the device this is a function to call ToDo to read
 *						for the shell command this is an empty function
 *
 * RETURNS: true if the input was read
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97		Created
 *			roger	9/12/97		Modified to work on the device
 *			ABa	06/21/00		Integrate Pdi library
 *
 *************************************************************/

extern Boolean ToDoImportVCal(DmOpenRef dbP, UInt16 pdiRefNum, PdiReaderType* reader, 
	Boolean obeyUniqueIDs, Boolean beginAlreadyRead, ImportVEventF vEventFunc)
{
    //UInt32 uid;
    UInt32 uniqueID = 0;
    Err error = 0;

    //uid = 0;


	TraceOutput(TL(appErrorClass, "ToDoImportVCal: BEGIN"));
    // Read in the vEvent entry
    if (!beginAlreadyRead)
    {
        PdiReadProperty(pdiRefNum, reader);
        beginAlreadyRead = reader->property == kPdiPRN_BEGIN_VCALENDAR;
    }
    if (!beginAlreadyRead)
        return false;

    // Read in the vcard entry
    if (beginAlreadyRead)
    {
    	PdiEnterObject(pdiRefNum, reader);
        while (PdiReadProperty(pdiRefNum, reader) == 0 && reader->property != kPdiPRN_END_VCALENDAR)
        {
            // Hand it off to the correct sub-routine
            // Note: here VCalEventRead is a dummy routine that just runs until it finds an end
            if (reader->property == kPdiPRN_BEGIN_VTODO)
            {
                ToDoImportVToDo(dbP, pdiRefNum, reader, obeyUniqueIDs, true, &uniqueID);
                ToDoSetGoToParams (dbP, reader->appData, uniqueID);
            }
            else if (reader->property == kPdiPRN_BEGIN_VEVENT)
            {
                error = vEventFunc(dbP, pdiRefNum, reader, obeyUniqueIDs, true, &uniqueID);
                if (error)
                    ErrThrow(error);
            }
        }

    }

    return ((reader->events & kPdiEOFEventMask) == 0);
}


/************************************************************
 *
 * FUNCTION: ToDoExportVCal
 *
 * DESCRIPTION: Export a VCALENDAR record.
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
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			djk	8/9/97	Created
 *
 *************************************************************/

extern void ToDoExportVCal(DmOpenRef dbP, Int16 index, ToDoDBRecordPtr recordP, 
	UInt16 pdiRefNum, PdiWriterType* writer, Boolean writeUniqueIDs)
{

    Char * 		note;   			// b/c the note doesnt have its own pointer in the record
    UInt32		uid;
    Char 		tempString[tempStringLengthMax];
    UInt16 		attr;

    ErrNonFatalDisplayIf (dmCategoryLength > tempStringLengthMax,
                          "ToDoExportVCal: tempString too Int16");

    PdiWriteBeginObject(pdiRefNum, writer, kPdiPRN_BEGIN_VTODO);

    // Emit the Category
    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_CATEGORIES);
    DmRecordInfo (dbP, index, &attr, NULL, NULL);
    CategoryGetName(dbP, (attr & dmRecAttrCategoryMask), tempString + 2);

    // Check to see if the category is a predefined vCal catval
    if((StrCaselessCompare(tempString, "Personal") != 0) &&
       (StrCaselessCompare(tempString, "Business") != 0))
    {
    	tempString[0] = 'X';
    	tempString[1] = '-';
    	PdiWritePropertyValue(pdiRefNum, writer, tempString, kPdiWriteText);
    }
	else
	{
    	PdiWritePropertyValue(pdiRefNum, writer, tempString + 2, kPdiWriteText);
	}

    // Emit the Due date
    if (DateToInt(recordP->dueDate) != toDoNoDueDate)
    {
	    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_DUE);
        // NOTE: since we don't keep a time for ToDo due dates,
        // we will truncate the ISO 8601 date/time to an ISO 8601 date
        // as allowed by the standard
        StrPrintF(tempString, "%d%02d%02d", firstYear + recordP->dueDate.year,
                  recordP->dueDate.month, recordP->dueDate.day);
    	PdiWritePropertyValue(pdiRefNum, writer, tempString, kPdiWriteData);
    }

    // Emit the completed flag
    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_STATUS);
    if (recordP->priority & completeFlag)
	{   
    	PdiWritePropertyValue(pdiRefNum, writer, (Char*) "COMPLETED", kPdiWriteData);
    }
    else
    {
    	PdiWritePropertyValue(pdiRefNum, writer, (Char*) "NEEDS ACTION", kPdiWriteData);
    }

    // Emit the Priority Level
    if ((recordP->priority & priorityOnly) != 0)
    {
	    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_PRIORITY);
        StrPrintF(tempString, "%d", recordP->priority & priorityOnly);
    	PdiWritePropertyValue(pdiRefNum, writer, tempString, kPdiWriteData);
    }

    // Emit the Decsription Text
    if(recordP->description != '\0')
    {
	    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_DESCRIPTION);
    	PdiWritePropertyValue(pdiRefNum, writer, &recordP->description, kPdiWriteText);
    }

    // Get the pointer to the note
    note = (&recordP->description) + StrLen(&recordP->description) + 1;

    // Emit the note
    if(*note != '\0')
    {
	    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_ATTACH);
    	PdiWritePropertyValue(pdiRefNum, writer, note, kPdiWriteText);
    }

    // Emit an unique id
    if (writeUniqueIDs)
    {
	    PdiWriteProperty(pdiRefNum, writer, kPdiPRN_UID);
        // Get the record's unique id and append to the string.
        DmRecordInfo(dbP, index, NULL, &uid, NULL);
        StrIToA(tempString, uid);
    	PdiWritePropertyValue(pdiRefNum, writer, tempString, kPdiWriteData);
    }

    PdiWriteEndObject(pdiRefNum, writer, kPdiPRN_END_VTODO);

	if (writer->error)
		ErrThrow(writer->error);
}
