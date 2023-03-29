/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: MemoDB.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *		Memo Manager routines
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <PalmUtils.h>

#include "MemoDB.h"
#include "MemoMain.h"


/************************************************************
 *
 *  FUNCTION: MemoLocalizeAppInfo
 *
 *  DESCRIPTION: Look for localize app info strings and copy
 *  them into the app info block.
 *
 *  PARAMETERS: application info ptr
 *
 *  RETURNS: nothing
 *
 *  CREATED: 12/13/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static void MemoLocalizeAppInfo (MemoAppInfoPtr appInfoP)
{
	Int16 		i;
	Char **		stringsP;
	MemHandle 	stringsH;
	MemHandle 	localizedAppInfoH;
	Char *		localizedAppInfoP;
	//MemoAppInfoPtr	nilP = 0;


	localizedAppInfoH = DmGetResource(appInfoStringsRsc, LocalizedAppInfoStr);
	if (localizedAppInfoH)
		{
		localizedAppInfoP = MemHandleLock(localizedAppInfoH);
		stringsH = SysFormPointerArrayToStrings(localizedAppInfoP, 
			dmRecNumCategories);
		stringsP = MemHandleLock(stringsH);
		
		// Copy each category
		for (i = 0; i < dmRecNumCategories; i++)
			{
			if (stringsP[i][0] != '\0')
				//DmStrCopy(appInfoP, (UInt32) nilP->categoryLabels[i], stringsP[i]);
				DmStrCopy(appInfoP, OffsetOf(MemoAppInfoType, categoryLabels) + i*dmCategoryLength, stringsP[i]);
			}
		
		MemPtrFree(stringsP);
		MemPtrUnlock(localizedAppInfoP);
		}
}


/************************************************************
 *
 *  FUNCTION: MemoAppInfoInit
 *
 *  DESCRIPTION: Create an app info chunk if missing.  Set
 *		the strings to a default.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/3/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err	MemoAppInfoInit(DmOpenRef dbP)
{
	UInt16 				cardNo;
	MemHandle			h;
	LocalID 				dbID;
	LocalID 				appInfoID;
	//MemoAppInfoPtr 	nilP = 0;
	MemoAppInfoPtr		appInfoP;
	
	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;
	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, 
			NULL, NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;
	
	if (appInfoID == 0)
		{
		h = DmNewHandle (dbP, sizeof (MemoAppInfoType));
		if (! h) return dmErrMemError;
		
		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL);
		}
	
	appInfoP = MemLocalIDToLockedPtr(appInfoID, cardNo);

	// Clear the app info block.
	DmSet (appInfoP, 0, sizeof(MemoAppInfoType), 0);

	// Initialize the categories.
	MemoLocalizeAppInfo (appInfoP);
	CategoryInitialize ((AppInfoPtr) appInfoP, LocalizedAppInfoStr);

	// Initialize the sort order.
	//DmSet (appInfoP, (UIntPtr)&nilP->sortOrder, sizeof(appInfoP->sortOrder), soAlphabetic);
	DmSet (appInfoP, OffsetOf(MemoAppInfoType, sortOrder), sizeof(appInfoP->sortOrder), soAlphabetic);

	MemPtrUnlock(appInfoP);

	// The conduit ignores dmHdrAttrAppInfoDirty
	return 0;
}


/************************************************************
 *
 *  FUNCTION:    MemoGetAppInfo
 *
 *  DESCRIPTION: Get the app info chunk 
 *
 *  PARAMETERS:  database pointer
 *
 *  RETURNS:     MemHandle to the to do application info block 
 *               (MemoAppInfoType)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *
 *************************************************************/
static MemHandle MemoGetAppInfo (DmOpenRef dbP)
{
	Err error;
	UInt16 cardNo;
	LocalID dbID;
	LocalID appInfoID;
	
	error = DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL);
	ErrFatalDisplayIf (error,  "Get getting app info block");
	
	error = DmDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL, 
			NULL, NULL, &appInfoID, NULL, NULL, NULL);
	ErrFatalDisplayIf (error,  "Get getting app info block");

	return ((MemHandle) MemLocalIDToGlobal (appInfoID, cardNo));
}


/************************************************************
 *
 *  FUNCTION: 	  MemoCompareRecords
 *
 *  DESCRIPTION: Compare two records.
 *
 *  PARAMETERS:  database record 1
 *					  database record 2
 *
 *  RETURNS:    -n if record one is less (n != 0)
 *			        n if record two is less
 *
 *	COMMENTS:	 Compare the two records key by key until
 *	there is a difference.  Return -n if r1 is less or n if r2
 *	is less.  A zero is never returned because if two records
 *	seem identical then their unique IDs are compared!
 *
 * This function accepts record data chunk pointers to avoid
 * requiring that the record be within the database.  This is
 * important when adding records to a database.  This prevents
 * determining if a record is a deleted record (which are kept
 * at the end of the database and should be considered "greater").
 * The caller should test for deleted records before calling this
 * function!
 *
 *	HISTORY:
 *		07/18/96	art	Created by Art Lamb.
 *		11/30/00	kwk	Use TxtCompare vs. StrCompare to avoid one
 *							extra trap call.
 *
 *************************************************************/
static Int16 MemoCompareRecords (MemoDBRecordPtr r1, 
	MemoDBRecordPtr r2, Int16 sortOrder, SortRecordInfoPtr UNUSED_PARAM(info1), 
	SortRecordInfoPtr UNUSED_PARAM(info2), MemHandle UNUSED_PARAM(appInfoH))
{
	Int16 result;

	// Alphabetize;
	if (sortOrder == soAlphabetic)
		{
		result = TxtCompare(	&r1->note,	// const Char *s1
									0xFFFF,		// UInt16 s1Len,
									NULL,			// UInt16 *s1MatchLen,
									&r2->note,	// const Char *s2,
									0xFFFF,		// UInt16 s2Len,
									NULL);		// UInt16 *s2MatchLen
		}
	else
		result = 0;
	
	return result;
}


/************************************************************
 *
 *  FUNCTION:    MemoGetSortOrder
 *
 *  DESCRIPTION: This routine get the sort order value from the 
 *               to do application info block.
 *
 *  PARAMETERS:  database pointer
 *
 *  RETURNS:     true - if the to do record are sorted by priority, 
 *               false - if the records are sorted by due date.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *
 *************************************************************/
UInt8 MemoGetSortOrder (DmOpenRef dbP)
{
	UInt8 sortOrder;
	MemoAppInfoPtr appInfoP;
			
	appInfoP = MemHandleLock (MemoGetAppInfo (dbP));
	sortOrder = appInfoP->sortOrder;
	MemPtrUnlock (appInfoP);	

	return (sortOrder);
}


/************************************************************
 *
 *  FUNCTION: MemoChangeSortOrder
 *
 *  DESCRIPTION: Change the Memo Database's sort order
 *
 *  PARAMETERS: database pointer
 *				TRUE if sort by company
 *
 *  RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *
 *************************************************************/
Err MemoChangeSortOrder(DmOpenRef dbP, Boolean sortOrder)
{
	MemoAppInfoPtr appInfoP;
	//MemoAppInfoPtr	nilP = 0;


	appInfoP = MemHandleLock (MemoGetAppInfo (dbP));

	if (appInfoP->sortOrder != sortOrder)
		{
		//DmWrite (appInfoP, (UIntPtr)&nilP->sortOrder, &sortOrder, sizeof(appInfoP->sortOrder));
		DmWrite (appInfoP, OffsetOf(MemoAppInfoType, sortOrder), &sortOrder, sizeof(appInfoP->sortOrder));
		
		if (sortOrder == soAlphabetic)
			DmInsertionSort (dbP, (DmComparF *) &MemoCompareRecords, (Int16) sortOrder);
		}
		
	MemPtrUnlock (appInfoP);	

	return 0;
}


/************************************************************
 *
 *  FUNCTION: MemoSort
 *
 *  DESCRIPTION: Sort the appointment database.
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *			gap	6/29/00	When in manual sort mode this
 *								routine was exiting immediately which caused
 *								deleted and non-deleted records to be mixed in 
 *								the database as opposed to having deleted records
 *								all appended at the end of the database.  Now check
 *								to move records above all deleted records (which can 
 *								occur in the case of receiving multiple beamed records).
 *
 *************************************************************/
void MemoSort (DmOpenRef dbP)
{
	Int16 sortOrder;
	
	sortOrder = MemoGetSortOrder (dbP);
	DmInsertionSort (dbP, (DmComparF *) &MemoCompareRecords, (Int16) sortOrder);
}


/************************************************************
 *
 *  FUNCTION: MemoNewRecord
 *
 *  DESCRIPTION: Create a new record
 *
 *  PARAMETERS: database pointer
 *				database record
 *
 *  RETURNS: zero if successful, errorcode if not
 *
 *  REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			monty	9/13/95	Initial Revision
 *
 *************************************************************/
Err MemoNewRecord (DmOpenRef dbP, MemoItemPtr item, UInt16 *index)
{
	Err 					result;
	int					size = 0;
	UInt32				offset;
	MemHandle			recordH;	
	MemoDBRecordPtr	recordP;

	// Compute the size of the new memo record.
	size = StrLen (item->note);

	//  Allocate a chunk in the database for the new record.
	recordH = (MemHandle)DmNewHandle(dbP, (UInt32) size);
	if (recordH == NULL)
		return dmErrMemError;

	// Pack the the data into the new record.
	recordP = MemHandleLock (recordH);
	//offset = (UIntPtr)&nilP->note;
  offset = OffsetOf(MemoDBRecordType, note);
	DmStrCopy(recordP, offset, item->note);
	
	MemPtrUnlock (recordP);

	// Insert the record.
	result = DmAttachRecord(dbP, index, recordH, 0);
	if (result) 
		MemHandleFree(recordH);
	
	return result;
}


/************************************************************
 *
 * FUNCTION:    MemoSortRecord
 *
 * DESCRIPTION: Move the passed record to its correct sort
 *              position.
 *
 * PARAMETERS:  database pointer
 *				    record index
 *
 * RETURNS:     zero if successful, errorcode if not
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/96	Initial Revision
 *			gap	6/29/00	When user is in manual sort mode, the sort
 *								routine was exiting immediately which caused
 *								deleted and non-deleted records to be mixed in 
 *								the database as opposed to having deleted records
 *								all appended at the end of the database.  Now check
 *								to move record above all deleted records (in the case
 *								of new records) before exiting the routine when
 *								manual sort is selected.
 *								
 *
 *************************************************************/
Err MemoSortRecord (DmOpenRef dbP, UInt16 * indexP)
{
	Err err;
	Int16 sortOrder;
	UInt16 index;
	UInt16 attributes;
	UInt32 uniqueID;
	MemHandle recordH;
	MemHandle h;
	Boolean dontMove;
	MemoDBRecordPtr cmp;
	MemoDBRecordPtr recordP;
	
	sortOrder = MemoGetSortOrder (dbP);

	// Check if the record is already in the correct position.
	recordP = MemHandleLock (DmQueryRecord (dbP, *indexP));		
	if (*indexP > 0)
		{
		// This record wasn't deleted and deleted records are at the end of the
		// database so the prior record may not be deleted!
		h = DmQueryRecord (dbP, *indexP-1);
		if (! h)
			dontMove = false;
		else
			{
			if (sortOrder == soUnsorted)
				dontMove = true;
			else
				{
				cmp = MemHandleLock (h);		
				dontMove = (MemoCompareRecords (cmp, recordP, sortOrder, NULL, NULL, 0) < 1);
				MemPtrUnlock (cmp);
				}
			}
		}
	else 
		dontMove = true;

	if (dontMove && (*indexP+1 < DmNumRecords (dbP)))
		{
		DmRecordInfo(dbP, *indexP+1, &attributes, NULL, NULL);
		if ( ! (attributes & dmRecAttrDelete) )
			{
			cmp = MemHandleLock (DmQueryRecord (dbP, *indexP+1));
			dontMove &= (MemoCompareRecords (recordP, cmp, sortOrder, NULL, NULL, 0) < 1);
			MemPtrUnlock (cmp);
			}
		}
	MemPtrUnlock (recordP);

	if (dontMove) return (0);


	// Since the routine that determines the records sort position uses a 
	// binary search algorythm we need to remove the record from the database 
	// before we can determine its new position.  We will also save and restore the 
	// record's attributes and unique ID.
	DmRecordInfo (dbP, *indexP, &attributes, &uniqueID, NULL);

	err = DmDetachRecord (dbP, *indexP, &recordH);
	if (err) return (err);
	
	recordP = MemHandleLock (recordH);		
	index = DmFindSortPosition (dbP, recordP, NULL, (DmComparF *)MemoCompareRecords, sortOrder);
	MemPtrUnlock (recordP);
	
	err = DmAttachRecord (dbP, &index, recordH, 0);
	if (err) return (err);

	DmSetRecordInfo (dbP, index, &attributes, &uniqueID);	

	*indexP = index;
	
	return (err);
}

/***********************************************************************
 *
 * FUNCTION:     MemoGetDatabase
 *
 * DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     Err - zero if no error, else the error
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			jmp		10/02/99	Initial Revision
 *
 ***********************************************************************/
Err MemoGetDatabase (DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
	UInt16 cardNo;
	LocalID dbID;

	*dbPP = NULL;
  
  // Find the application's data file.  If it doesn't exist create it.
	dbP = DmOpenDatabaseByTypeCreator (memoDBType, sysFileCMemo, mode);
	if (!dbP)
		{
		error = DmCreateDatabase (0, memoDBName, sysFileCMemo, memoDBType, false);
		if (error)
			return error;
		
		dbP = DmOpenDatabaseByTypeCreator(memoDBType, sysFileCMemo, mode);
		if (!dbP)
			return (1);

		// Set the backup bit.  This is to aid syncs with non Palm software.
		SetDBBackupBit(dbP);
		
		error = MemoAppInfoInit (dbP);
      if (error) 
      	{
			DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL);
      	DmCloseDatabase(dbP);
      	DmDeleteDatabase(cardNo, dbID);
         return error;
         }
		}
	
	*dbPP = dbP;
	return 0;
}
