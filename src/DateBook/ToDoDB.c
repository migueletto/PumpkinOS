/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: ToDoDB.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *		To Do Manager routines
 *
 *****************************************************************************/

// Set this to get to private database defines
#define __TODOMGR_PRIVATE__

#include <PalmOS.h>

#ifdef PALMOS
#define UIntPtr UInt32
#endif

#include "ToDo.h"
#include "ToDoDB.h"

// Export error checking routines
void ECToDoDBValidate(DmOpenRef dbP);


/************************************************************
 * Private routines used only in this module
 *************************************************************/


/************************************************************
 *
 *  FUNCTION: ECToDoDBValidate
 *
 *  DESCRIPTION: This routine validates the integrity of the to do
 *		datebase.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: nothing
 *
 *  CREATED: 6/22/95 
 *
 *  BY: Art Lamb
 *
 *************************************************************/
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL

#define maxDescLen	256
#define maxNoteLen	4096

void ECToDoDBValidate (DmOpenRef dbP)
{
	UInt16 i;
	UInt16 size;
	UInt16 cLen;
	UInt16 recSize;
	UInt16 descLen;
	UInt16 noteLen;
	UInt16 numRecord;
	UInt16 priority;
	Char * c;
	Char * note;
	MemHandle recH;
	ToDoDBRecordPtr rec;
	
	numRecord = DmNumRecords (dbP);
	for (i = 0 ; i < numRecord; i++)
		{
		recH = DmQueryRecord (dbP, i);
		if (! recH) continue;

		rec = MemHandleLock (recH);
		
		priority = rec->priority & priorityOnly;
		ErrFatalDisplayIf (priority > toDoMaxPriority,  "DB integrity error");
		
		descLen = StrLen (&rec->description);
		ErrFatalDisplayIf (descLen > maxDescLen, "DB integrity error");

		note = &rec->description + descLen + 1;
		noteLen = StrLen (note);
		ErrFatalDisplayIf (noteLen > maxNoteLen, "DB integrity error");
		
		// Validate the record size.
		size = sizeof (DateType) + 1;
		c = &rec->description;
		cLen = StrLen(c) + 1;
		size += cLen;
		c += cLen;
		size += StrLen(c) + 1;
		recSize = MemPtrSize (rec);
		ErrFatalDisplayIf ( (recSize != size), "DB integrity error");

		MemPtrUnlock (rec);
		}
}
#endif

/************************************************************
 *
 *  FUNCTION: DateTypeCmp
 *
 *  DESCRIPTION: Compare two dates
 *
 *  PARAMETERS: 
 *
 *  RETURNS: 
 *
 *  CREATED: 1/20/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static Int16 DateTypeCmp(DateType d1, DateType d2)
{
	Int16 result;
	
	result = d1.year - d2.year;
	if (result != 0)
		{
		if (DateToInt(d1) == 0xffff)
			return 1;
		if (DateToInt(d2) == 0xffff)
			return -1;
		return result;
		}
	
	result = d1.month - d2.month;
	if (result != 0)
		return result;
	
	result = d1.day - d2.day;

	return result;

}


/************************************************************
 *
 *  FUNCTION: CategoryCompare
 *
 *  DESCRIPTION: Compare two categories
 *
 *  PARAMETERS:  attr1, attr2 - record attributes, which contain 
 *						  the category. 
 *					  appInfoH - MemHandle of the applications category info
 *
 *  RETURNS: 0 if they match, non-zero if not
 *				 + if s1 > s2
 *				 - if s1 < s2
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/5/96	Initial Revision
 *
 *************************************************************/
static Int16 CategoryCompare (UInt8 attr1, UInt8 attr2, MemHandle appInfoH)
{
	Int16 result;
	UInt8 category1;
	UInt8 category2;
	ToDoAppInfoPtr appInfoP;
	
	
	category1 = attr1 & dmRecAttrCategoryMask;
	category2 = attr2 & dmRecAttrCategoryMask;

	result = category1 - category2;
	if (result != 0)
		{
		if (category1 == dmUnfiledCategory)
			return (1);
		else if (category2 == dmUnfiledCategory)
			return (-1);
		
		appInfoP = MemHandleLock (appInfoH);
		result = StrCompare (appInfoP->categoryLabels[category1],
									appInfoP->categoryLabels[category2]);

		MemPtrUnlock (appInfoP);
		return result;
		}
	
	return result;
}


/************************************************************
 *
 *  FUNCTION: ToDoAppInfoInit
 *
 *  DESCRIPTION: Create an app info chunk if missing.  Set
 *		the strings to a default.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/20/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err	ToDoAppInfoInit(DmOpenRef dbP)
{
	UInt16 cardNo;
	UInt16 wordValue;
	MemHandle h;
	LocalID dbID;
	LocalID appInfoID;
	ToDoAppInfoPtr	nilP = 0;
	ToDoAppInfoPtr appInfoP;
	
	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;

	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
		 NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;
	
	if (appInfoID == 0)
		{
		h = DmNewHandle(dbP, sizeof (ToDoAppInfoType));
		if (! h) return dmErrMemError;

		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
			NULL, &appInfoID, NULL, NULL, NULL);
		}
	
	appInfoP = MemLocalIDToLockedPtr(appInfoID, cardNo);

	// Clear the app info block.
	DmSet (appInfoP, 0, sizeof(ToDoAppInfoType), 0);

	// Initialize the categories.
	CategoryInitialize ((AppInfoPtr) appInfoP, LocalizedAppInfoStr);

	// I don't know what this field is used for.
	wordValue = 0xFFFF;
	DmWrite (appInfoP, (UIntPtr)&nilP->dirtyAppInfo, &wordValue,
		sizeof(appInfoP->dirtyAppInfo));

	// Initialize the sort order.
	DmSet (appInfoP, (UIntPtr)&nilP->sortOrder, sizeof(appInfoP->sortOrder), 
		soPriorityDueDate);

	MemPtrUnlock (appInfoP);

	return 0;
}


/************************************************************
 *
 *  FUNCTION: ToDoGetAppInfo
 *
 *  DESCRIPTION: Get the app info chunk 
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: MemHandle to the to do application info block (ToDoAppInfoType)
 *
 *  CREATED: 5/12/95 
 *
 *  BY: Art Lamb
 *
 *************************************************************/
MemHandle ToDoGetAppInfo (DmOpenRef dbP)
{
	Err error;
	UInt16 cardNo;
	LocalID dbID;
	LocalID appInfoID;
	
	error = DmOpenDatabaseInfo (dbP, &dbID, NULL, NULL, &cardNo, NULL);
	ErrFatalDisplayIf (error,  "Get getting to do app info block");

	error = DmDatabaseInfo (cardNo, dbID, NULL, NULL, NULL, NULL, NULL, 
			NULL, NULL, &appInfoID, NULL, NULL, NULL);
	ErrFatalDisplayIf (error,  "Get getting to do app info block");

	return ((MemHandle) MemLocalIDToGlobal (appInfoID, cardNo));
}


/************************************************************
 *
 *  FUNCTION: ToDoGetSortOrder
 *
 *  DESCRIPTION: This routine gets the sort order value from the 
 *               'to do' application info block.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS:    true - if the 'to do' record are sorted by priority, 
 *              false - if the records are sorted by due date.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/12/95	Initial Revision
 *       art	3/22/96	Rename routine and added more sort orders
 *
 *************************************************************/
UInt8 ToDoGetSortOrder (DmOpenRef dbP)
{
	UInt8 sortOrder;
	ToDoAppInfoPtr appInfoP;
			
	appInfoP = MemHandleLock (ToDoGetAppInfo (dbP));
	sortOrder = appInfoP->sortOrder;
	MemPtrUnlock (appInfoP);	

	return (sortOrder);
}


/************************************************************
 *
 *  FUNCTION: ToDoCompareRecords
 *
 *  DESCRIPTION: Compare two records.
 *
 *  PARAMETERS: database record 1
 *				database record 2
 *
 *  RETURNS: -n if record one is less (n != 0)
 *			  n if record two is less
 *
 *  CREATED: 1/23/95 
 *
 *  BY: Roger Flores
 *
 *	COMMENTS:	Compare the two records key by key until
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
 *************************************************************/
 
static Int16 ToDoCompareRecords(ToDoDBRecordPtr r1, 
	ToDoDBRecordPtr r2, Int16 sortOrder, 
	SortRecordInfoPtr info1, SortRecordInfoPtr info2,
	MemHandle appInfoH)
{
	Int16 result = 0;

	// Sort by priority, due date, and category.
	if (sortOrder == soPriorityDueDate)
		{
		result = (r1->priority & priorityOnly) - (r2->priority & priorityOnly);
		if (result == 0)
			{
			result = DateTypeCmp (r1->dueDate, r2->dueDate);
			if (result == 0)
				{
				result = CategoryCompare (info1->attributes, info2->attributes, appInfoH);
				}
			}
		}

	// Sort by due date, priority, and category.
	else if (sortOrder == soDueDatePriority)
		{
		result = DateTypeCmp(r1->dueDate, r2->dueDate);
		if (result == 0)
			{
			result = (r1->priority & priorityOnly) - (r2->priority & priorityOnly);
			if (result == 0)
				{
				result = CategoryCompare (info1->attributes, info2->attributes, appInfoH);
				}
			}
		}
	
	// Sort by category, priority, due date
	else if (sortOrder == soCategoryPriority)
		{
		result = CategoryCompare (info1->attributes, info2->attributes, appInfoH);
		if (result == 0)
			{
			result = (r1->priority & priorityOnly) - (r2->priority & priorityOnly);
			if (result == 0)
				{
				result = DateTypeCmp (r1->dueDate, r2->dueDate);
				}
			}
		}

	// Sort by category, due date, priority
	else if (sortOrder == soCategoryDueDate)
		{
		result = CategoryCompare (info1->attributes, info2->attributes, appInfoH);
		if (result == 0)
			{
			result = DateTypeCmp (r1->dueDate, r2->dueDate);
			if (result == 0)
				{
				result = (r1->priority & priorityOnly) - (r2->priority & priorityOnly);
				}
			}
		}
	
	return result;
}


#if 0
/************************************************************
 *
 *  FUNCTION: ToDoSize
 *
 *  DESCRIPTION: Return the size of a ToDoDBRecordType
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/10/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ToDoSize(ToDoDBRecordPtr r)
{
	UInt16 size;
	char *c;
	int cLen;
	
	c = &r->description;
	cLen = StrLen(c) + 1;
	size = sizeof (DateType) + 1 + cLen;
	c += cLen;
	
	size += StrLen(c) + 1;

	return size;
}
#endif


/************************************************************
 *
 *  FUNCTION: ToDoFindSortPosition
 *
 *  DESCRIPTION: Return where a record is or should be.
 *		Useful to find a record or find where to insert a record.
 *
 *  PARAMETERS: database record (not deleted!)
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/11/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ToDoFindSortPosition(DmOpenRef dbP, ToDoDBRecord *newRecord, 
		SortRecordInfoPtr newRecordInfo)
{
	int sortOrder;
	UInt16 r;
	
	sortOrder = ToDoGetSortOrder (dbP);
		
	r = (DmFindSortPosition (dbP, newRecord, newRecordInfo, 
		(DmComparF *)ToDoCompareRecords, sortOrder));
		
	return r;
}


/************************************************************
 *
 *  FUNCTION: ToDoNewRecord
 *
 *  DESCRIPTION: Create a new record in sorted position
 *
 *  PARAMETERS: database pointer
 *				database record
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/10/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err ToDoNewRecord(DmOpenRef dbP, ToDoItemPtr item, UInt16 category, UInt16 *index)
{
	Err 						err;
	UInt16					size;
	Char						zero=0;
	UInt16					attr;
	UInt16 					newIndex;
	UInt32					offset;
	ToDoDBRecordPtr		nilP=0;
	ToDoDBRecordPtr		recordP;
	MemHandle 				recordH;
	SortRecordInfoType	sortInfo;

	// Compute the size of the new to do record.
	size = sizeDBRecord;
	if (item->description)
		size += StrLen (item->description);
	if (item->note)
		size += StrLen (item->note);
		

	//  Allocate a chunk in the database for the new record.
	recordH = (MemHandle)DmNewHandle(dbP, size);
	if (recordH == NULL)
		return dmErrMemError;

	// Pack the the data into the new record.
	recordP = MemHandleLock (recordH);
	DmWrite(recordP, (UIntPtr)&nilP->dueDate, &item->dueDate, sizeof(nilP->dueDate));
	DmWrite(recordP, (UIntPtr)&nilP->priority, &item->priority, sizeof(nilP->priority));

	//offset = (UInt32)&nilP->description;
	offset = OffsetOf(ToDoDBRecordType, description);
	if (item->description)
		{
		DmStrCopy(recordP, offset, item->description);
		offset += StrLen (item->description) + 1;
		}
	else
		{
		DmWrite(recordP, offset, &zero, 1);
		offset++;
		}
	
	if (item->note) 
		DmStrCopy(recordP, offset, item->note);
	else
		DmWrite(recordP, offset, &zero, 1);
	
	
	sortInfo.attributes = category;
	sortInfo.uniqueID[0] = 0;
	sortInfo.uniqueID[1] = 0;
	sortInfo.uniqueID[2] = 0;
	
	// Determine the sort position of the new record.
	newIndex = ToDoFindSortPosition (dbP, recordP, &sortInfo);

	MemPtrUnlock (recordP);

	// Insert the record.
	err = DmAttachRecord(dbP, &newIndex, recordH, 0);
	if (err) 
		MemHandleFree(recordH);
	else
		{
		*index = newIndex;

		// Set the category.
		DmRecordInfo (dbP, newIndex, &attr, NULL, NULL);
		attr &= ~dmRecAttrCategoryMask;
		attr |= category;
		DmSetRecordInfo (dbP, newIndex, &attr, NULL);
		}
		
	return err;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoInsertNewRecord
 *
 * DESCRIPTION: This routine creates a new record and inserts it after
 *              the specified position.  The new record is assigned the 
 *              same priority and due date as the record it is 
 *              inserted after.
 *
 * PARAMETERS:  database pointer
 *              database record
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
Err ToDoInsertNewRecord (DmOpenRef dbP, UInt16 * index)
{
	UInt8					priority;
	ToDoDBRecordPtr 	rec;
	ToDoDBRecordPtr 	newRec;
	MemHandle 		 	recH;
	MemHandle			 	newRecH;
	Err				 	err;
	UInt16 				size;
	ToDoDBRecordPtr	nilP=0;
	UInt16				zero=0;
	UInt16 				newIndex;
	UInt16				category;
	UInt16				attr;


	// Make a new chunk
	size = sizeof (ToDoDBRecord) + 1;
	newRecH = DmNewHandle (dbP, size);
	if (! newRecH) return dmErrMemError;


	// Get the record that the new record will be inserted after.
	recH = DmQueryRecord (dbP, *index);
	ErrFatalDisplayIf (!recH, "Error inserting new to do record");
	rec = MemHandleLock (recH);

	// Get the category of the current record.
	DmRecordInfo (dbP, *index, &attr, NULL, NULL);
	category = (attr & dmRecAttrCategoryMask);
		
	// Set the priority and the due date of the new record to the same
	// values as the record we're inserting after.  This will insure
	// that the records are in the proper sort order.
	newRec = (ToDoDBRecord *) MemHandleLock (newRecH);
	DmWrite(newRec, (UIntPtr)&nilP->dueDate, &rec->dueDate, sizeof(rec->dueDate));

	priority = rec->priority & priorityOnly;
	DmWrite(newRec, (UIntPtr)&nilP->priority, &priority, sizeof(rec->priority));
	
	// Description is 1 byte, plus add extra byte for note field
	DmWrite(newRec, (UIntPtr)&nilP->description, &zero, 2);

	MemPtrUnlock (rec);

	// Attach in place
	newIndex = *index + 1;
	err = DmAttachRecord(dbP, &newIndex, newRecH, 0);
	if (err) 
		{
		MemHandleFree(newRecH);
		}
	else
		{
		MemPtrUnlock (newRec);

		// Set the category of the new record.
		DmRecordInfo (dbP, newIndex, &attr, NULL, NULL);
		attr |= category;
		DmSetRecordInfo (dbP, newIndex, &attr, NULL);

		*index = newIndex;
		}

#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
	ECToDoDBValidate (dbP);
#endif
	
	return err;
}


/************************************************************
 *
 *  FUNCTION: ToDoChangeSortOrder
 *
 *  DESCRIPTION: Change the ToDo Database's sort order
 *
 *  PARAMETERS: database pointer
 *				TRUE if sort by company
 *
 *  RETURNS: nothing
 *
 *  CREATED: 1/17/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err ToDoChangeSortOrder(DmOpenRef dbP, UInt8 sortOrder)
{
	ToDoAppInfoPtr appInfoP;
	ToDoAppInfoPtr	nilP = 0;
	UInt16			dirtyAppInfo;


	appInfoP = MemHandleLock (ToDoGetAppInfo (dbP));

	if (appInfoP->sortOrder != sortOrder)
		{
		dirtyAppInfo = appInfoP->dirtyAppInfo | toDoSortByPriorityDirty;
		DmWrite(appInfoP, (UIntPtr)&nilP->dirtyAppInfo, &dirtyAppInfo, sizeof(appInfoP->dirtyAppInfo)); 
		DmWrite(appInfoP, (UIntPtr)&nilP->sortOrder, &sortOrder, sizeof(appInfoP->sortOrder));
		
		DmInsertionSort(dbP, (DmComparF *) &ToDoCompareRecords, (Int16) sortOrder);
		}
		
	MemPtrUnlock (appInfoP);	

	return 0;
}


/************************************************************
 *
 *  FUNCTION: ToDoSort
 *
 *  DESCRIPTION: Sort the appointment database.
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: nothing
 *
 *  CREATED: 10/17/95 
 *
 *  BY: Art Lamb
 *
 *************************************************************/
void ToDoSort (DmOpenRef dbP)
{
	int sortOrder;
	
	sortOrder = ToDoGetSortOrder (dbP);
	DmInsertionSort(dbP, (DmComparF *) &ToDoCompareRecords, (Int16) sortOrder);
}


/************************************************************
 *
 *  FUNCTION: ToDoChangeRecord
 *
 *  DESCRIPTION: Change a record in the ToDo Database
 *
 *  PARAMETERS: database pointer
 *					 database index
 *					 database record
 *					 changed fields
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/14/95 
 *
 *  BY: Roger Flores
 *
 *	COMMENTS:	Records are not stored with extra padding - they
 *	are always resized to their exact storage space.  This avoids
 *	a database compression issue.  The code works as follows:
 *	
 *
 *************************************************************/
Err ToDoChangeRecord(DmOpenRef dbP, UInt16 *index, 
	ToDoRecordFieldType changedField, void * data)
{
	Err 						err;
	Int16 					cLen;
	UInt16					attr;
	UInt16 					curSize;
	UInt16 					newSize;
	UInt16 					newIndex = 0;
	UInt8						priority;
	UInt32					offset;
	Char * 					c;
	MemHandle 				recordH=0;
	ToDoDBRecord			temp;
	ToDoDBRecordPtr 		src;
	ToDoDBRecordPtr 		nilP = 0;
	SortRecordInfoType	sortInfo;
	
	// Get the record which we are going to change
	recordH = DmQueryRecord(dbP, *index);
	src = MemHandleLock (recordH);
	

	// If the  record is being change such that its sort position will 
	// change,  move the record to its new sort position.
	if ( (changedField == toDoPriority) || 
		  (changedField == toDoDueDate)  ||
		  (changedField == toDoCategory) )
		{
		MemSet (&sortInfo, sizeof (sortInfo), 0);
		DmRecordInfo (dbP, *index, &attr, NULL, NULL);
		sortInfo.attributes = attr;

		if (changedField == toDoPriority)
			{
			temp.priority = *(UInt16 *) data;
			temp.dueDate = src->dueDate;
			sortInfo.attributes = attr;
			}
		else if (changedField == toDoDueDate) 
			{
			temp.priority = src->priority;
			temp.dueDate = *((DatePtr)data);
			sortInfo.attributes = attr;
			}
		else
			{
			temp.priority = src->priority;
			temp.dueDate = src->dueDate;
			sortInfo.attributes = *(UInt16 *) data;			
			}
			
		newIndex = ToDoFindSortPosition (dbP, &temp, &sortInfo);
		DmMoveRecord (dbP, *index, newIndex);
		if (newIndex > *index) newIndex--;
		*index = newIndex;
		}


	if (changedField == toDoPriority)
		{
		priority = *(UInt16 *) data | (src->priority & completeFlag);
		DmWrite (src, (UIntPtr)&nilP->priority, &priority, sizeof(src->priority));
		goto exit;
		}
			
	if (changedField == toDoComplete) 
		{
		priority = (*(UInt16 *) data << 7) |  (src->priority & priorityOnly);
		DmWrite (src, (UIntPtr)&nilP->priority, &priority, sizeof(src->priority));
		goto exit;
		}

	if (changedField == toDoDueDate)
		{ 
		DmWrite (src, (UIntPtr)&nilP->dueDate, data, sizeof(src->dueDate));
		goto exit;
		}
		
	if (changedField == toDoCategory)
		{
		attr = (attr & ~dmRecAttrCategoryMask) | *(UInt16 *) data;
		DmSetRecordInfo (dbP, newIndex, &attr, NULL);
		goto exit;
		}


	// Calculate the size of the changed record. First,
	// find the size of the data used from the old record.
	newSize = sizeof (DateType) + 1;
	newSize += StrLen((Char *) data) + 1;
	c = &src->description;
	cLen = StrLen(c) + 1;
	if (changedField != toDoDescription)
		newSize += cLen;
	if (changedField != toDoNote)
		{
		c += cLen;
		newSize += StrLen(c) + 1;
		}
		

	// Change the description field.
	if (changedField == toDoDescription)
		{
		// If the new description is longer, expand the record.
		curSize = MemPtrSize (src);
		if (newSize > curSize)
			{
			MemPtrUnlock (src);
			err = MemHandleResize (recordH, newSize);
			if (err) return (err);
			src = MemHandleLock (recordH);
			}
		
		// Move the note field.
		offset = sizeof(DateType) + 1 + StrLen (data) + 1;
		c = &src->description;
		c += StrLen(c) + 1;
		DmWrite (src, offset, c, StrLen(c) + 1);
		
		
		// Write the new description field.
		offset = sizeof(DateType) + 1;
		DmStrCopy (src, offset, data);

		// If the new description is shorter, shrink the record.
		if (newSize < curSize)
			MemHandleResize (recordH, newSize);
		goto exit;
		}


	// Change the note field
	if (changedField == toDoNote)
		{
		offset = sizeof(DateType) + 1 + StrLen((Char *)&src->description) + 1;
		MemPtrUnlock (src);
		err = MemHandleResize (recordH, newSize);
		if (err) return (err);

		src = MemHandleLock (recordH);
		DmStrCopy (src, offset, data);
		goto exit;
		}

exit:
	MemPtrUnlock (src);

#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
	ECToDoDBValidate (dbP);
#endif

	return 0;
}


/***********************************************************************
 *
 * FUNCTION:     ToDoSetDBBackupBit
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database and set
 *					  the backup bit on it.
 *
 * PARAMETERS:   dbP -	the database to set backup bit,
 *								can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	4/1/99	Initial Revision
 *
 ***********************************************************************/
void ToDoSetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef localDBP;
	LocalID dbID;
	UInt16 cardNo;
	UInt16 attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
		{
		localDBP = DmOpenDatabaseByTypeCreator (toDoDBType, sysFileCToDo, dmModeReadWrite);
		if (localDBP == NULL)  return;
		}
	else
		{
		localDBP = dbP;
		}
	
	// now set the backup bit on localDBP
	DmOpenDatabaseInfo(localDBP, &dbID, NULL, NULL, &cardNo, NULL);
	DmDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	attributes |= dmHdrAttrBackup;
	DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
	// close database if necessary
   if (dbP == NULL) 
   	{
   	DmCloseDatabase(localDBP);
      }
}


/***********************************************************************
 *
 * FUNCTION:     ToDoGetDatabase
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
 *			jmp		10/04/99	Initial Revision
 *
 ***********************************************************************/
Err ToDoGetDatabase (DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
	UInt16 cardNo;
	LocalID dbID;

	*dbPP = NULL;
  
  // Find the application's data file.  If it doesn't exist create it.
	dbP = DmOpenDatabaseByTypeCreator (toDoDBType, sysFileCToDo, mode);
	if (!dbP)
		{
		error = DmCreateDatabase (0, toDoDBName, sysFileCToDo, toDoDBType, false);
		if (error)
			return error;
		
		dbP = DmOpenDatabaseByTypeCreator(toDoDBType, sysFileCToDo, mode);
		if (!dbP)
			return (1);

		// Set the backup bit.  This is to aid syncs with non Palm software.
		ToDoSetDBBackupBit(dbP);
		
		error = ToDoAppInfoInit (dbP);
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
