/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressAutoFill.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This module contains routines that support the auto-fill feature
 *   of the address application. (Started from ExpLookup.c from Expense)
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrTools.h"
#include "AddrDefines.h"
#include "AddressAutoFill.h"

#include <ErrorMgr.h>
#include <StringMgr.h>
#include <TimeMgr.h>
#include <UIResources.h>
#include <PalmUtils.h>

/***********************************************************************
 *
 *	Defines
 *
 ***********************************************************************/

#define maxLookupEntries  100		// max number of entries per lookup database


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

//static Boolean	PrvAutoFillLookupStringInList (ListPtr lst, Char *key, UInt16 *indexP, Boolean *uniqueP);
static Int16	PrvAutoFillPartialCaselessCompare (Char * s1, Char * s2);
static Int16 	PrvAutoFillLookupCompareRecords (Char * str, LookupRecordPtr r2, Int16 /* dummy */, SortRecordInfoPtr /* info1 */, SortRecordInfoPtr /* info2 */, MemHandle /* appInfoH */);
static Int16 	PrvAutoFillSortCompareRecords (LookupRecordPtr r1, LookupRecordPtr r2, Int16 /* dummy */, SortRecordInfoPtr /* info1 */, SortRecordInfoPtr /* info2 */, MemHandle /* appInfoH */);


/***********************************************************************
 *
 * FUNCTION:     AutoFillInitDB
 *
 * DESCRIPTION:  This routine initializes a lookup database from a
 *               resource.  Each string in the resource becomes a record
 *               in the database.  The strings in the resouce are delimited
 *               by the '|' character.
 *
 * PARAMETERS:   type      - database type
 *               creator   - database creator type
 *               name      - name given to the database
 *               initRscID - reosource used to initialize the database
 *
 * RETURNED:     error code, 0 if successful
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		1/12/96		Initial Revision
 *			grant	4/7/99		Set backup bit on new databases.
 *			bhall	11/1/99		now handles case where init string not present
 * 								also sorts the db in case initial values out of order
 *			bhall	11/8/99		Fixed prob where records not properly sorted
 *								Fixed prob where last record of init string skipped
 ***********************************************************************/
Err AutoFillInitDB (UInt32 type, UInt32 creator, const Char * name,
					UInt16 initRscID)
{
	Char zero=0;
	UInt16 error = 0;
	UInt16 index;
	UInt32 strLen;
	UInt32 time;
	Char * endP;
	Char * resP;
	MemHandle resH;
	MemHandle rH;
	DmOpenRef dbP = 0;
	LookupRecordPtr rP;
	//LookupRecordPtr nilP = 0;
	Boolean done;

	dbP = DmOpenDatabaseByTypeCreator (type, creator, dmModeReadWrite);
	if (dbP) goto exit;

	// If the description datebase does not exist, create it.
	error = DmCreateDatabase (0, name, creator, type, false);
	if (error) return (error);

	// Then open it
	dbP = DmOpenDatabaseByTypeCreator (type, creator, dmModeReadWrite);
	if (! dbP) return DmGetLastErr();

	// Set the backup bit.  This is to aid syncs with non-Palm software.
	// Also set hidden bit so launcher info doesn't give confusing record counts.
	ToolsSetDBAttrBits(dbP, dmHdrAttrBackup | dmHdrAttrHidden);

	// We will need the time for default records we may create
	time = TimGetSeconds();

	// Attempt to initialize the description datebase from a resource that contains strings
	// delimited by "|" characters.
	resH = DmGetResource(strRsc, initRscID);
	if (resH) {
		// Lock down the init string
		resP = MemHandleLock(resH);

		// Parse it and add records as we go
		index = 0;
		done = false;
		while (!done) {
			// Find the next one to add
			endP = StrChr(resP, '|');
			if (endP) {
				strLen = (UInt16) (endP - resP);
			} else {
				strLen = StrLen(resP);
				done = true;
			}

			// If we don't find one, stop
			if (strLen == 0) break;

			// Create a new record
			rH = DmNewRecord(dbP, &index, sizeof (LookupRecordType) + strLen);
			rP = MemHandleLock(rH);

			// Write out the data
			DmWrite (rP, OffsetOf(LookupRecordType, time), &time, sizeof (UInt32));
			DmWrite (rP, OffsetOf(LookupRecordType, text), resP, strLen);
			DmWrite (rP, OffsetOf(LookupRecordType, text) + strLen, &zero, 1); // null-terminate
			MemPtrUnlock(rP);

			// Release it, dirty
			DmReleaseRecord(dbP, index, true);

			// Move to the next
			resP += strLen + 1;
			index++;
		}

		// Done with the init string - release it
		MemHandleUnlock(resH);
		DmReleaseResource(resH);

		// Now sort the database - required for proper operation
		error = DmQuickSort(dbP, (DmComparF *)PrvAutoFillSortCompareRecords, 0);
	}

exit:
	// If we had the database open, close it
	if (dbP) DmCloseDatabase(dbP);

	return (0);
}


/***********************************************************************
 *
 * FUNCTION:     AutoFillLookupStringInDatabase
 *
 * DESCRIPTION:  This routine seatches a database for a the string passed.
 *
 * PARAMETERS:   dpb       - description database
 *					  key       - string to lookup record with
 *					  indexP    - to contain the record found
 *
 * RETURNED:     true if a unique match was found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/4/96	Initial Revision
 *
 ***********************************************************************/
Boolean AutoFillLookupStringInDatabase (DmOpenRef dbP, Char * key, UInt16 * indexP)
{
	Int16 					result;
	Int16 					numOfRecords;
	UInt16					kmin, probe, i;
	MemHandle 			rH;
	Boolean				match = false;
	LookupRecordPtr 	rP;
	Boolean 				unique;

	unique = false;

	if ((! key) || (! *key) ) return false;

	numOfRecords = DmNumRecords(dbP);
	if (numOfRecords == 0) return false;

	result = 0;
	kmin = probe = 0;


	while (numOfRecords > 0)
	{
		i = numOfRecords / 2;
		probe = kmin + i;


		// Compare the two records.  Treat deleted records as greater.
		// If the records are equal look at the following position.
		rH = DmQueryRecord (dbP, probe);
		if (rH == 0)
		{
			result = -1;		// Delete record is greater
		}
		else
		{
			rP = MemHandleLock(rH);
			result = PrvAutoFillPartialCaselessCompare (key, &rP->text);
			MemHandleUnlock(rH);
		}


		// If the date passed is less than the probe's date , keep searching.
		if (result < 0)
			numOfRecords = i;

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
		{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
		}

		// If equal stop here!  Make sure the match is unique by checking
		// the records before and after the probe,  if either matches then
		// we don't have a unique match.
		else
		{
			// Start by assuming we have a unique match.
			match = true;
			unique = true;
			*indexP = probe;

			// If we not have a unique match,  we want to return the
			// index one the first item that matches the key.
			while (probe)
			{
				rH = DmQueryRecord (dbP, probe-1);
				rP = MemHandleLock(rH);
				result = PrvAutoFillPartialCaselessCompare (key, &rP->text);
				MemHandleUnlock(rH);
				if (result != 0) break;

				unique = false;
				*indexP = probe-1;
				probe--;
			}

			if (! unique) break;


			if (probe + 1 < DmNumRecords(dbP))
			{
				rH = DmQueryRecord (dbP, probe+1);
				if (rH)
				{
					rP = MemHandleLock(rH);
					result = PrvAutoFillPartialCaselessCompare (key, &rP->text);
					MemHandleUnlock(rH);
					if (result == 0)
						unique = false;
				}
			}
			break;
		}
	}
	return (match);
}


/***********************************************************************
 *
 * FUNCTION:     AutoFillLookupSave
 *
 * DESCRIPTION:  This routine saves the string passed to the specified
 *               lookup database.
 *
 * PARAMETERS:   type      - database type
 *               creator   - database creator type
 *               str       - string to save to lookup record
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/16/96	Initial Revision
 *
 ***********************************************************************/
void AutoFillLookupSave (UInt32 type, UInt32 creator, Char * str)
{
	UInt16 i;
	UInt16 index;
	UInt16 numRecords;
	UInt32 strLen;
	UInt32 time;
	Boolean insert = true;
	MemHandle  h;
	DmOpenRef dbP;
	LookupRecordPtr r;
	//LookupRecordPtr nilP = 0;


	dbP = DmOpenDatabaseByTypeCreator (type, creator, dmModeReadWrite);
	if (! dbP) return;

	strLen = StrLen (str);

	// There is a limit on the number of entries in a lookup database,
	// if we've reached that limit find the oldest entry and delete it.
	numRecords = DmNumRecords (dbP);
	if (numRecords >= maxLookupEntries)
	{
		time = ~0;
		for (i = 0; i < numRecords; i++)
		{
			r = MemHandleLock (DmQueryRecord (dbP, i));
			if (r->time < time)
			{
				index = i;
				time = r->time;
			}
			MemPtrUnlock (r);
		}
		DmRemoveRecord(dbP, index);
	}

	// Check if the string passed already exist in the database,  if it
	// doesn't insert it.  If it does,  write the passed string to
	// the record, the case of one or more of the character may
	// changed.
	index = DmFindSortPosition (dbP, str, NULL, (DmComparF *)PrvAutoFillLookupCompareRecords, 0);

	if (index)
	{
		h = DmQueryRecord (dbP, index-1);

		r = MemHandleLock (h);
		if (StrCaselessCompare (str, &r->text) == 0)
		{
			insert = false;
			index--;
		}
		MemPtrUnlock (r);
	}

	if (insert)
	{
		h = DmNewRecord (dbP, &index, sizeof (LookupRecordType) + strLen);
		if (! h) goto exit;

		DmReleaseRecord (dbP, index, true);
	}
	else
	{
		h = DmResizeRecord (dbP, index, sizeof (LookupRecordType) + strLen);
		if (! h) goto exit;
	}


	// Copy the string passed to the record and time stamp the entry with
	// the current time.
	time = TimGetSeconds ();
	h = DmGetRecord (dbP, index);
	r = MemHandleLock (h);
	DmWrite (r, OffsetOf(LookupRecordType, time), &time, sizeof (UInt32));
	DmWrite (r, OffsetOf(LookupRecordType, text), str, strLen + 1);
	MemPtrUnlock (r);

	DmReleaseRecord (dbP, index, true);


exit:
	DmCloseDatabase (dbP);
}

//#pragma mark -

#if 0
/***********************************************************************
 *
 * FUNCTION:     PrvAutoFillLookupStringInList
 *
 * DESCRIPTION:  This routine seatches a list for a the string passed.
 *
 * PARAMETERS:   lst       - pointer to a list object
 *					  key       - string to lookup record with
 *					  indexP    - to contain the record found
 *
 *
 * RETURNED:     true if a match was found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/10/96	Initial Revision
 *
 ***********************************************************************/
Boolean PrvAutoFillLookupStringInList (ListPtr lst, Char * key, UInt16 * indexP,
							Boolean * uniqueP)
{
	Int16 		result;
	Int16 		numItems;
	UInt16		kmin, probe, i;
	Char		*itemP;
	Boolean		match = false;

	*uniqueP = false;

	if ((! key) || (! *key) ) return false;

	numItems = LstGetNumberOfItems (lst);
	if (numItems == 0) return false;

	result = 0;
	kmin = probe = 0;


	while (numItems > 0)
	{
		i = numItems / 2;
		probe = kmin + i;


		// Compare the a list item to the key.
		itemP = LstGetSelectionText (lst, probe);
		result = PrvAutoFillPartialCaselessCompare (key, itemP);


		// If the date passed is less than the probe's date , keep searching.
		if (result < 0)
			numItems = i;

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
		{
			kmin = probe + 1;
			numItems = numItems - i - 1;
		}

		// If equal stop here!  Make sure the match is unique by checking
		// the item before and after the probe,  if either matches then
		// we don't have a unique match.
		else
		{
			// Start by assuming we have a unique match.
			match = true;
			*uniqueP = true;
			*indexP = probe;

			// If we not have a unique match,  we want to return the
			// index one the first item that matches the key.
			while (probe)
			{
				itemP = LstGetSelectionText (lst, probe-1);
				if (PrvAutoFillPartialCaselessCompare (key, itemP) != 0)
					break;
				*uniqueP = false;
				*indexP = probe-1;
				probe--;
			}

			if (!*uniqueP) break;

			if (probe + 1 < LstGetNumberOfItems (lst))
			{
				itemP = LstGetSelectionText (lst, probe+1);
				if (PrvAutoFillPartialCaselessCompare (key, itemP) == 0)
					*uniqueP = false;
			}
			break;
		}
	}
	return (match);
}
#endif


/************************************************************
 *
 *  FUNCTION: PrvAutoFillPartialCaselessCompare
 *
 *  DESCRIPTION: Compares two strings with case and accent insensitivity.
 *               If all of s1 matches all or the start of s2 then
 *               there is a match
 *
 *  PARAMETERS: 2 string pointers
 *
 *  RETURNS: 0 if they match, non-zero if not
 *
 *  REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/4/96	Initial Revision
 *			kwk	05/16/99	Use StrNCaselessCompare.
 *
 *************************************************************/
Int16 PrvAutoFillPartialCaselessCompare (Char * s1, Char * s2)
{

	// Check for err
	ErrFatalDisplayIf(s1 == NULL || s2 == NULL, "NULL string passed");

	return (StrNCaselessCompare (s1, s2, StrLen (s1)));
}


/***********************************************************************
 *
 * FUNCTION:     PrvAutoFillLookupCompareRecords
 *
 * DESCRIPTION:  Compare two lookup records.  This is a callback
 *               called by DmFindSortPosition.
 *
 * PARAMETERS:   database record 1
 *               database record 2
 *
 *  RETURNS:     -n if record one is less (n != 0)
 *			         n if record two is less
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/16/96	Initial Revision
 *
 ***********************************************************************/
Int16 PrvAutoFillLookupCompareRecords (Char * str, LookupRecordPtr r2, Int16 UNUSED_PARAM(dummy), SortRecordInfoPtr UNUSED_PARAM(info1), SortRecordInfoPtr UNUSED_PARAM(info2), MemHandle UNUSED_PARAM(appInfoH))
{
	return (StrCaselessCompare (str, &r2->text));
}

/***********************************************************************
 *
 * FUNCTION:     PrvAutoFillSortCompareRecords
 *
 * DESCRIPTION:  Compare two sort records.
 *               This is a callback called by DmQuickSort.
 *
 * PARAMETERS:   database record 1
 *               database record 2
 *
 *  RETURNS:     -n if record one is less (n != 0)
 *			      n if record two is less
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bhall	11/8/99		Initial Revision
 *
 ***********************************************************************/
Int16 PrvAutoFillSortCompareRecords (LookupRecordPtr r1, LookupRecordPtr r2, Int16 UNUSED_PARAM(dummy), SortRecordInfoPtr UNUSED_PARAM(info1), SortRecordInfoPtr UNUSED_PARAM(info2), MemHandle UNUSED_PARAM(appInfoH))
{
	return (StrCaselessCompare (&r1->text, &r2->text));
}
