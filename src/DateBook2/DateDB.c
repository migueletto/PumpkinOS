/******************************************************************************
 *
 * Copyright (c) 1904-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateDB.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *		Appointment Manager routines
 *
 * History:
 *			Name	Date		Description
 *			----	----		-----------
 *			rsf		1/25/95		Created
 *			rbb		4/22/99		Added support for snooze feature
 *								Moved code to link w/ 16-bit jumps
 *			peter	3/22/00		Optimized ApptAlarmMunger
 *			gap		07/28/00	Added preliminary attention manager support.
 *
 *****************************************************************************/

#include <PalmOS.h>

// Set this to get to private database defines
#define __APPTMGR_PRIVATE__
#include "Datebook.h"


/***********************************************************************
 *
 *	Internal Structutes
 *
 ***********************************************************************/

// The following structure doesn't really exist.  The first field
// varies depending on the data present.  However, it is convient
// (and less error prone) to use when accessing the other information.
typedef struct {
	ApptDateTimeType 	when;
	ApptDBRecordFlags	flags;	// A flag set for each  datum present
	char				firstField;
	UInt8				reserved;
} ApptPackedDBRecordType;

typedef ApptPackedDBRecordType * ApptPackedDBRecordPtr;


typedef struct {
	DmOpenRef	dbP;
	UInt16		cardNo;
	LocalID		dbID;
	DatebookPreferenceType	prefs;
} AlarmPostingData;

typedef AlarmPostingData * AlarmPostingDataPtr;


typedef Int16 comparF (const void *, const void *, UInt16 other);



/***********************************************************************
 *
 *	Internal Routines
 *
 ***********************************************************************/


void ECApptDBValidate (DmOpenRef dbP);

static Int16 TimeCompare (TimeType t1, TimeType t2);

static Int16 DateCompare (DateType d1, DateType d2);




/************************************************************
 *
 *  FUNCTION: ECApptDBValidate
 *
 *  DESCRIPTION: This routine validates the integrity of a
 *					  datebook datebase.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	11/15/95	Initial Revision
 *
 *************************************************************/
#if	EMULATION_LEVEL != EMULATION_NONE

#define maxDescLen	tableMaxTextItemSize
#define maxNoteLen	noteViewMaxLength

void ECApptDBValidate (DmOpenRef dbP)
{
	UInt16 i;
	UInt16 size;
	UInt16 len;
	UInt16 blockSize;
	UInt16 numRecord;
	MemHandle recH;
	DateType date;
	ApptDBRecordType rec;
	
	numRecord = DmNumRecords (dbP);
	for (i = 0 ; i < numRecord; i++)
		{
		recH = DmQueryRecord (dbP, i);
		if (! recH) continue;

		ApptGetRecord (dbP, i, &rec, &recH);
		
		// Is the event an untimed event?
		if (TimeToInt(rec.when->startTime) == apptNoTime)
			{
			// There should not be and end time if there is no start time.
			if (TimeToInt(rec.when->endTime) != apptNoTime)
				ErrDisplay ("DB integrity error");				
			}

		// Validate the event date.
		if ((rec.when->date.month == 0) ||
			 (rec.when->date.month > 12) ||
			 (rec.when->date.day == 0) ||
			 (rec.when->date.day > DaysInMonth (rec.when->date.month, 
			 	rec.when->date.year + firstYear)))
			ErrDisplay ("DB integrity error");				
			 

		// The start time may not be greater than the end time.
		else if (TimeCompare (rec.when->startTime, rec.when->endTime) > 0)
			{
			ErrDisplay ("DB integrity error");				
			}

		// Validate the alarm info.
		if (rec.alarm)
			{
			if (rec.alarm->advance > 99)
				ErrDisplay ("DB integrity error");				

			if (rec.alarm->advanceUnit > aauDays)
				ErrDisplay ("DB integrity error");				
			}


		// Validate the repeat info.
		if (rec.repeat)
			{
			// Validate the repeat type.
			if (rec.repeat->repeatType > repeatYearly)
				ErrDisplay ("DB integrity error");				

			// Validate the repeat end date.
			date = rec.repeat->repeatEndDate;
			if (DateToInt (date) != apptNoEndDate)
				{
				if (DateCompare (date, rec.when->date) < 0)
					ErrDisplay ("DB integrity error");				
				
				if ((date.month == 0) ||
					 (date.month > 12) ||
					 (date.day == 0) ||
					 (date.day > DaysInMonth (date.month, date.year + firstYear)))
					ErrDisplay ("DB integrity error");
				}				
			
			// Validate the repeat frequency.
			if (rec.repeat->repeatFrequency > 99)
				ErrDisplay ("DB integrity error");				

			// Validate the "repeatOn" info
			if (rec.repeat->repeatType == repeatWeekly)
				{
				if (rec.repeat->repeatOn == 0)
					ErrDisplay ("DB integrity error");				
				}
			else if (rec.repeat->repeatType == repeatMonthlyByDay)
				{
				if (rec.repeat->repeatOn > domLastSat)
					ErrDisplay ("DB integrity error");				
				}
			else
				{
				if (rec.repeat->repeatOn != 0)
					ErrDisplay ("DB integrity error");				
				}

			// Validate the "repeatStartOfWeek" info,
			if (rec.repeat->repeatType == repeatWeekly)
				{
				if (rec.repeat->repeatStartOfWeek > monday)
					ErrDisplay ("DB integrity error");
				}
			else if (rec.repeat->repeatStartOfWeek)
				ErrDisplay ("DB integrity error");

			}

		// Validate the record size.
		size = sizeof (ApptDateTimeType) + sizeof (ApptDBRecordFlags);
		if (rec.alarm)
			size += sizeof (AlarmInfoType);
		if (rec.repeat)
			size += sizeof (RepeatInfoType);
		if (rec.exceptions)
			size += sizeof (DateType) * rec.exceptions->numExceptions +
				sizeof (UInt16);
		if (rec.description)
			{
			len = StrLen (rec.description);
			ErrFatalDisplayIf (len > maxDescLen, "DB integrity error");
			size += len + 1;
			}
		if (rec.note)
			{
			len = StrLen (rec.note);
			ErrFatalDisplayIf (len > maxNoteLen, "DB integrity error");
			size += len + 1;
			}

		blockSize = MemHandleSize (recH);
//		ErrFatalDisplayIf ( (blockSize != size), "DB integrity error");

		MemHandleUnlock (recH);
		}
}
#endif

/***********************************************************************
 *
 * FUNCTION:    DateCompare
 *
 * DESCRIPTION: This routine compares two dates.
 *
 * PARAMETERS:  d1 - a date 
 *              d2 - a date 
 *
 * RETURNED:    if d1 > d2  returns a positive int
 *              if d1 < d2  returns a negative int
 *              if d1 = d2  returns zero
 *
 * NOTE: This routine treats the DateType structure like an unsigned int,
 *       it depends on the fact the the members of the structure are ordered
 *       year, month, day form high bit to low low bit.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95		Initial Revision
 *
 ***********************************************************************/
static Int16 DateCompare (DateType d1, DateType d2)
{
	UInt16 int1, int2;
	
	int1 = DateToInt(d1);
	int2 = DateToInt(d2);
	
	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    TimeCompare
 *
 * DESCRIPTION: This routine compares two times.  "No time" is represented
 *              by minus one, and is considered less than all times.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    if t1 > t2  returns a positive int
 *              if t1 < t2  returns a negative int
 *              if t1 = t2  returns zero
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95		Initial Revision
 *
 ***********************************************************************/
static Int16 TimeCompare (TimeType t1, TimeType t2)
{
	Int16 int1, int2;
	
	int1 = TimeToInt(t1);
	int2 = TimeToInt(t2);
	
	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);
	return 0;

}


/************************************************************
 *
 *  FUNCTION: ApptAppInfoInit
 *
 *  DESCRIPTION: Create and initialize the app info chunk if missing.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err	ApptAppInfoInit(DmOpenRef dbP)
{
	UInt16 				cardNo;
	MemHandle 		h;
	LocalID 			dbID;
	LocalID 			appInfoID;
	ApptAppInfoPtr	appInfoP;
	
	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;
		
	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;
	
	if (appInfoID == 0) 
		{
		h = DmNewHandle(dbP, sizeof(ApptAppInfoType));
		if (! h) return dmErrMemError;

		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL);
		}
		
	// Get pointer to app Info chunk
	appInfoP = MemLocalIDToLockedPtr(appInfoID, cardNo);
	
	// Init it
	DmSet(appInfoP, 0, sizeof(ApptAppInfoType), 0); 

	// Unlock it
	MemPtrUnlock(appInfoP);
	
	return 0;
}



/************************************************************
 *
 *  FUNCTION:    ApptComparePackedRecords
 *
 *  DESCRIPTION: Compare two packed records.
 *
 *  PARAMETERS:  r1    - database record 1
 *				     r2    - database record 2
 *               extra - extra data, not used in the function
 *
 *  RETURNS:    -1 if record one is less
 *		           1 if record two is less
 *
 *  CREATED: 1/14/95 
 *
 *  BY: Roger Flores
 *
 *	COMMENTS:	Compare the two records key by key until
 *	there is a difference.  Return -1 if r1 is less or 1 if r2
 *	is less.  A zero is never returned because if two records
 *	seem identical then their unique IDs are compared!
 *
 *************************************************************/ 
static Int16 ApptComparePackedRecords (ApptPackedDBRecordPtr r1, 
	ApptPackedDBRecordPtr r2, Int16 extra, SortRecordInfoPtr info1, 
	SortRecordInfoPtr info2, MemHandle appInfoH)
{
#pragma unused (extra, info1, info2, appInfoH)

	Int16 result;

	if ((r1->flags.repeat) || (r2->flags.repeat))
		{
		if ((r1->flags.repeat) && (r2->flags.repeat))
			result = 0;
		else if (r1->flags.repeat)
			result = -1;
		else
			result = 1;
		}

	else
		{
		result = DateCompare (r1->when.date, r2->when.date);
		if (result == 0)
			{
			result = TimeCompare (r1->when.startTime, r2->when.startTime);
			}
		}
	return result;
}


/************************************************************
 *
 *  FUNCTION: ApptPackedSize
 *
 *  DESCRIPTION: Return the packed size of an ApptDBRecordType 
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ApptPackedSize (ApptDBRecordPtr r)
{
	UInt16 size;

	
	size = sizeof (ApptDateTimeType) + sizeof (ApptDBRecordFlags);
	
	if (r->alarm != NULL)
		size += sizeof (AlarmInfoType);
	
	if (r->repeat != NULL)
		size += sizeof (RepeatInfoType);
	
	if (r->exceptions != NULL)
		size += sizeof (UInt16) + 
			(r->exceptions->numExceptions * sizeof (DateType));
	
	if (r->description != NULL)
		size += StrLen(r->description) + 1;
	
	if (r->note != NULL)
		size += StrLen(r->note) + 1;
	

	return size;
}


/************************************************************
 *
 *  FUNCTION: ApptPack
 *
 *  DESCRIPTION: Pack an ApptDBRecordType
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the ApptPackedDBRecord is packed
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static void ApptPack(ApptDBRecordPtr s, ApptPackedDBRecordPtr d)
{
	ApptDBRecordFlags	flags;
	UInt16 	size;
	UInt32	offset = 0;
	
	
	*(UInt8 *)&flags = 0;			// clear the flags
	
	
	// copy the ApptDateTimeType
	//c = (char *) d;
	offset = 0;
	DmWrite(d, offset, s->when, sizeof(ApptDateTimeType));
	offset += sizeof (ApptDateTimeType) + sizeof (ApptDBRecordFlags);
	

	if (s->alarm != NULL)
		{
		DmWrite(d, offset, s->alarm, sizeof(AlarmInfoType));
		offset += sizeof (AlarmInfoType);
		flags.alarm = 1;
		}
	

	if (s->repeat != NULL)
		{
		DmWrite(d, offset, s->repeat, sizeof(RepeatInfoType));
		offset += sizeof (RepeatInfoType);
		flags.repeat = 1;
		}
	

	if (s->exceptions != NULL)
		{
		size = sizeof (UInt16) + 
			(s->exceptions->numExceptions * sizeof (DateType));
		DmWrite(d, offset, s->exceptions, size);
		offset += size;
		flags.exceptions = 1;
		}
	
	
	if (s->description != NULL)
		{
		size = StrLen(s->description) + 1;
		DmWrite(d, offset, s->description, size);
		offset += size;
		flags.description = 1;
		}
	

	
	if (s->note != NULL)
		{
		size = StrLen(s->note) + 1;
		DmWrite(d, offset, s->note, size);
		offset += size;
		flags.note = 1;
		}
	
	DmWrite(d, sizeof(ApptDateTimeType), &flags, sizeof(flags));
}


/************************************************************
 *
 *  FUNCTION: ApptUnpack
 *
 *  DESCRIPTION: Fills in the ApptDBRecord structure
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the record unpacked
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static void ApptUnpack(ApptPackedDBRecordPtr src, ApptDBRecordPtr dest)
{
	ApptDBRecordFlags	flags;
	char *p;

	
	flags = src->flags;
	p = &src->firstField;

	dest->when = (ApptDateTimeType *) src;
	
	if (flags.alarm) 
		{
		dest->alarm = (AlarmInfoType *) p;
		p += sizeof (AlarmInfoType);
		}
	else
		dest->alarm = NULL;

	
	if (flags.repeat)
		{
		dest->repeat = (RepeatInfoType *) p;
		p += sizeof (RepeatInfoType);
		}
	else
		dest->repeat = NULL;
	
	
	if (flags.exceptions)
		{
		dest->exceptions = (ExceptionsListType *) p;
		p += sizeof (UInt16) + 
			(((ExceptionsListType *) p)->numExceptions * sizeof (DateType));
		}
	else
		dest->exceptions = NULL;
		
	
	if (flags.description)
		{
		dest->description = p;
		p += StrLen(p) + 1;
		}
	else
		dest->description = NULL;
		
	
	if (flags.note)
		{
		dest->note = p;
		}
	else
		dest->note = NULL;
	
}

/************************************************************
 *
 *  FUNCTION: ApptFindSortPosition
 *
 *  DESCRIPTION: Return where a record is or should be
 *		Useful to find or find where to insert a record.
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: position where a record should be
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ApptFindSortPosition(DmOpenRef dbP, ApptPackedDBRecordPtr newRecord)
{
	return (DmFindSortPosition (dbP, newRecord, NULL, (DmComparF *)ApptComparePackedRecords, 0));
}


/************************************************************
 *
 *  FUNCTION: ApptSort
 *
 *  DESCRIPTION: Sort the appointment database.
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: nothing
 *
 *  CREATED: 9/5/95 
 *
 *  BY: Art Lamb
 *
 *************************************************************/
void ApptSort (DmOpenRef dbP)
{
	DmQuickSort(dbP, (DmComparF *)ApptComparePackedRecords, 0);
}


/***********************************************************************
 *
 * FUNCTION:    ApptFindFirst
 *
 * DESCRIPTION: This routine finds the first appointment on the specified
 *              day.
 *
 * PARAMETERS:  dbP    - pointer to the database
 *              date   - date to search for
 *              indexP - pointer to the index of the first record on the 
 *                       specified day (returned value)
 *
 * RETURNED:    true if a record has found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/15/95		Initial Revision
 *
 ***********************************************************************/
Boolean ApptFindFirst (DmOpenRef dbP, DateType date, UInt16* indexP)
{
	Err err;
	Int16 numOfRecords;
	Int16 kmin, probe, i;		// all positions in the database.
	Int16 result = 0;			// result of comparing two records
	UInt16 index;
	MemHandle recordH;
	Boolean found = false;
	ApptPackedDBRecordPtr r;


	kmin = probe = 0;
	numOfRecords = DmNumRecords(dbP);
	
	
	while (numOfRecords > 0)
		{
		i = numOfRecords >> 1;
		probe = kmin + i;
		
		index = probe;
		recordH = DmQueryNextInCategory (dbP, &index, dmAllCategories);
		if (recordH)
			{
			r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
			if (r->flags.repeat)
				result = 1;
			else
				result = DateCompare (date, r->when.date);
			MemHandleUnlock (recordH);
			}

		// If no handle, assume the record is deleted, deleted records
		// are greater.
		else
			result = -1;
			

		// If the date passed is less than the probe's date, keep searching.
		if (result < 0)
			numOfRecords = i;

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
			{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
			}

		// If the records are equal find the first record on the day.
		else
			{
			found = true;
			*indexP = index;
			while (true)
				{
				err = DmSeekRecordInCategory (dbP, &index, 1, dmSeekBackward, 
					dmAllCategories);
				if (err == dmErrSeekFailed) break;
				
				recordH = DmQueryRecord(dbP, index);
				r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
				if (r->flags.repeat)
					result = 1;
				else
					result = DateCompare (date, r->when.date);
				MemHandleUnlock (recordH);
				if (result != 0) break;
				*indexP = index;
				}

			break;
			}
		}

	
	// If that were no appointments on the specified day, return the 
	// index of the next appointment (on a future day).
	if (! found)
		{
		if (result < 0) 
			*indexP = probe;
		else if (DmNumRecords(dbP) == 0)
			*indexP = 0;
		else
			*indexP = probe + 1;
		}

	return (found);
}



/***********************************************************************
 *
 * FUNCTION:    ApptAddException
 *
 * DESCRIPTION: This routine adds an entry to the exceptions list of the 
 *              specified record.
 *
 *  PARAMETERS: database pointer
 *					 database index
 *					 exception date
 *
 * RETURNED:	 error code 
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/26/95	Initial Revision
 *
 ***********************************************************************/
Err ApptAddException (DmOpenRef dbP, UInt16 *index, DateType date)
{
	Err error;
	UInt16 size;
	MemHandle recordH;
	ApptDBRecordType r;
	ExceptionsListPtr exceptions;
	ApptDBRecordFlags changedFields;
	
	error = ApptGetRecord(dbP, *index, &r, &recordH);
	if (error) return error;
	
	// If the record already has an expections list, add an entry to 
	// the list.
	if (r.exceptions)
		{
		size = sizeof (ExceptionsListType) + 
			(sizeof (DateType) * r.exceptions->numExceptions);
		exceptions = MemPtrNew (size);
		ErrFatalDisplayIf ((!exceptions), "Out of memory");
		
		MemMove (exceptions, r.exceptions, size - sizeof (DateType));
		exceptions->numExceptions++;
		*(&exceptions->exception + r.exceptions->numExceptions) = date;
		}

	// Create an expections list.
	else
		{
		size = sizeof (ExceptionsListType);
		exceptions = MemPtrNew (size);
		ErrFatalDisplayIf ((!exceptions), "Out of memory");

		exceptions->numExceptions = 1;
		exceptions->exception = date;
		}
	
	MemHandleUnlock (recordH);
	
	// Update the record
	r.exceptions = exceptions;
	MemSet (&changedFields, sizeof (changedFields), 0);
	changedFields.exceptions = true;
	error = ApptChangeRecord (dbP, index, &r, changedFields);
	
	MemPtrFree (exceptions);

	return (error);
}


/***********************************************************************
 *
 * FUNCTION:    ApptRepeatsOnDate
 *
 * DESCRIPTION: This routine returns true if a repeating appointment
 *              occurrs on the specified date.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - date to check              
 *
 * RETURNED:    true if the appointment occurs on the date specified
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95		Initial Revision
 *
 ***********************************************************************/
Boolean ApptRepeatsOnDate (ApptDBRecordPtr apptRec, DateType date)
{
	Int16  i;
	UInt16 freq;
	UInt16 weeksDiff;
	UInt16 dayInMonth;
	UInt16 dayOfWeek;
	UInt16 dayOfMonth;
	UInt16 firstDayOfWeek;
	Int32 dateInDays;
	Int32 startInDays;
	Boolean onDate;
	DatePtr exceptions;
	DateType startDate;

	// Is the date passed before the start date of the appointment?
	if (DateCompare (date, apptRec->when->date) < 0)
		return (false);

	// Is the date passed after the end date of the appointment?
	if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
		return (false);
	

	// Get the frequency of occurrecne (ex: every 2nd day, every 3rd month, etc.).  
	freq = apptRec->repeat->repeatFrequency;
	
	// Get the date of the first occurrecne of the appointment.
	startDate = apptRec->when->date;

	switch (apptRec->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);
			onDate = ((dateInDays - startInDays) % freq) == 0;
			break;
			

		// Weekly repeating appointment (ex: every Monday and Friday). 
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			// Are we on a day of the week that the appointment repeats on.
			dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
			onDate = ((1 << dayOfWeek) & apptRec->repeat->repeatOn);
			if (! onDate) break;

			// Are we in a week in which the appointment occurrs, if not 
			// move to that start of the next week in which the appointment
			// does occur.
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) - 
				apptRec->repeat->repeatStartOfWeek + daysInWeek) % daysInWeek;

			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - 
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			onDate = (weeksDiff == 0);
			break;


//			// Compute the first occurrence of the appointment that occurs
//			// on the same day of the week as the date passed.
//			startDayOfWeek = DayOfWeek (startDate.month, startDate.day, 
//				startDate.year+firstYear);
//			startInDays = DateToDays (startDate);
//			if (startDayOfWeek < dayOfWeek)
//				startInDays += dayOfWeek - startDayOfWeek;
//			else if (startDayOfWeek > dayOfWeek)
//				startInDays += dayOfWeek+ (daysInWeek *freq) - startDayOfWeek;
//			
//			// Are we in a week in which the appointment repeats.
//			dateInDays = DateToDays (date);
//			onDate = (((dateInDays - startInDays) / daysInWeek) % freq) == 0;
//			break;


		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Are we in a month in which the appointment repeats.
			onDate = ((((date.year - startDate.year) * monthsInYear) + 
						   (date.month - startDate.month)) % freq) == 0;
			if (! onDate) break;

			// Do the days of the month match (ex: 3rd Friday)
			dayOfMonth = DayOfMonth (date.month, date.day, date.year+firstYear);
			onDate = (dayOfMonth == apptRec->repeat->repeatOn);
			if (onDate) break;
			
			// If the appointment repeats on one of the last days of the month,
			// check if the date passed is also one of the last days of the 
			// month.  By last days of the month we mean: last sunday, 
			// last monday, etc.
			if ((apptRec->repeat->repeatOn >= domLastSun) &&
				 (dayOfMonth >= dom4thSun))
				{
				dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
				dayInMonth = DaysInMonth (date.month, date.year+firstYear);
				onDate = (((date.day + daysInWeek) > dayInMonth) &&
							 (dayOfWeek == (apptRec->repeat->repeatOn % daysInWeek)));
				}
			break;						


		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Are we in a month in which the appointment repeats.
			onDate = ((((date.year - startDate.year) * monthsInYear) + 
						   (date.month - startDate.month)) % freq) == 0;
			if (! onDate) break;
			
			// Are we on the same day of the month as the start date.
			onDate = (date.day == startDate.day);
			if (onDate) break;

			// If the staring day of the appointment is greater then the 
			// number of day in the month passed, and the day passed is the 
			// last day of the month, then the appointment repeats on the day.
			dayInMonth = DaysInMonth (date.month, date.year+firstYear);
			onDate = ((startDate.day > dayInMonth) && (date.day == dayInMonth));
			break;


		// Yearly repeating appointment.
		case repeatYearly:
			// Are we in a year in which the appointment repeats.
			onDate = ((date.year - startDate.year) % freq) == 0;
			if (! onDate) break;
			
			// Are we on the month and day that the appointment repeats.
			onDate = (date.month == startDate.month) &&
				      (date.day == startDate.day);
			if (onDate) break;
			
			// Specal leap day processing.
			if ( (startDate.month == february) && 
				  (startDate.day == 29) &&
				  (date.month == february) && 
				  (date.day == DaysInMonth (date.month, date.year+firstYear)))
				{
				onDate = true;
				}				      
			break;
		default:
			break;
		}

	// Check for an exception.
	if ((onDate) && (apptRec->exceptions))
		{
		exceptions = &apptRec->exceptions->exception;
		for (i = 0; i < apptRec->exceptions->numExceptions; i++)
			{
			if (DateCompare (date, exceptions[i]) == 0)
				{
				onDate = false;
				break;
				}
			}
		}


	return (onDate);
}

/***********************************************************************
 *
 * FUNCTION:    FindNextRepeat
 *
 * DESCRIPTION: This routine computes the date of the next 
 *              occurrence of a repeating appointment.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - passed:   date to start from
 *                        returned: date of next occurrence             
 *              searchForward - search for the next occurrence before or after the 
 *						  specified date
 *
 * RETURNED:    true if an occurrence was found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95	Initial Revision
 *			gap	09/25/00	Add capability to search for the most recent previous occurrence
 *								(needed for attention manager support)
 *
 ***********************************************************************/
static Boolean FindNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	Int16  i;
	Int32  adjust;
	Int32  daysTilNext;
	Int32  monthsTilNext;
	UInt16 day;
	UInt16 freq;
	UInt16 year;
	UInt16 weeksDiff;
	UInt16 monthsDiff;
	UInt16 daysInMonth;
	UInt16 dayOfWeek;
	UInt16 apptWeekDay;
	UInt16 firstDayOfWeek;
	UInt32 dateInDays;
	UInt32 startInDays;
	DateType start;
	DateType date;
	DateType next;
	

	date = *dateP;
	
	if (searchForward)
		{
		// Is the date passed after the end date of the appointment?
		if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
			return (false);
		
		// Is the date passed before the start date of the appointment?
		if (DateCompare (date, apptRec->when->date) < 0)
			date = apptRec->when->date;
		}
	else
		{
		// Is the date passed is before the start date of the appointment? 
		// return false now
		if (DateCompare (date, apptRec->when->date) < 0)
			return (false);

		// Is the date passed after the end date of the appointment?
		// search backwards from repeat end date for first valid occurrence.
		if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
			date = apptRec->repeat->repeatEndDate;
		}

	// apptRec->repeat->repeatEndDate can be passed into this routine
	// or be set in the else case above.  Since apptNoEndDate is not a 
	// valid date (month is 15) set it must be set to the last date 
	// support by the current OS  12/31/31
	if ( DateToInt(date) == apptNoEndDate)
		date.month = 12;

	// Get the frequency on occurrecne (ex: every 2nd day, every 3rd month, etc).  
	freq = apptRec->repeat->repeatFrequency;
	
	// Get the date of the first occurrecne of the appointment.
	start = apptRec->when->date;	

	switch (apptRec->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);
			if (searchForward)
				daysTilNext = (dateInDays - startInDays + freq - 1) / freq * freq;
			else
				daysTilNext = (dateInDays - startInDays) / freq * freq;
			if (startInDays + daysTilNext > (UInt32) maxDays)
				return (false);
			DateDaysToDate (startInDays + daysTilNext, &next);
			break;
			


		// Weekly repeating appointment (ex: every Monday and Friday). 
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) - 
				apptRec->repeat->repeatStartOfWeek + daysInWeek) % daysInWeek;

			dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
			apptWeekDay = (dayOfWeek - apptRec->repeat->repeatStartOfWeek +
				daysInWeek) % daysInWeek;

			// Are we in a week in which the appointment occurrs, if not 
			// move to that start of the next week in which the appointment
			// does occur.
			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - 
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			if (weeksDiff)
				{
				if (searchForward)
					{
					adjust = ((freq - weeksDiff) * daysInWeek) - apptWeekDay;
					apptWeekDay = 0;
					dayOfWeek = (dayOfWeek + adjust) % daysInWeek;
					}
				else
					{
					adjust = (weeksDiff * daysInWeek) + (daysInWeek-1 - apptWeekDay);
					apptWeekDay = 6;
					dayOfWeek = (dayOfWeek - adjust) % daysInWeek;
					}
				}
			else
				adjust = 0;
			
			// Find the next day on which the appointment repeats.
			if (searchForward)
				{
				for (i = 0; i < daysInWeek; i++)
					{
					if (apptRec->repeat->repeatOn & (1 << dayOfWeek)) break;
					adjust++;
					if (++dayOfWeek == daysInWeek)
						dayOfWeek = 0;
					if (++apptWeekDay == daysInWeek)
						adjust += (freq - 1) * daysInWeek;
					}

				if (dateInDays + adjust > (UInt32) maxDays)
					return (false);
				DateDaysToDate (dateInDays + adjust, &next);
				}
			else
				{
				for (i = 0; i < daysInWeek; i++)
					{
					if (apptRec->repeat->repeatOn & (1 << dayOfWeek)) break;
					adjust++;
					if (--dayOfWeek == 0)
						dayOfWeek = daysInWeek-1;
					if (--apptWeekDay == 0)
						adjust += (freq - 1) * daysInWeek;
					}		
				
				// determine if date goes past first day (unsigned int wraps around)
				if (dateInDays - adjust > dateInDays) 
					return (false);
					
				DateDaysToDate (dateInDays - adjust, &next);
				}

			break;



		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Compute the number of month until the appointment repeats again.
			if (searchForward)
				monthsTilNext = ((((date.year - start.year) * monthsInYear) + (date.month - start.month)) + freq - 1) /freq * freq;
			else
				monthsTilNext = (((date.year - start.year) * monthsInYear) + (date.month - start.month)) /freq * freq;

			while (true)
				{
				year = start.year + (start.month - 1 + monthsTilNext) / monthsInYear;
				if (year >= numberOfYears)
					return (false);

				next.year = year;
				next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
	
				dayOfWeek = DayOfWeek (next.month, 1, next.year+firstYear);
				if ((apptRec->repeat->repeatOn % daysInWeek) >= dayOfWeek)
					day = apptRec->repeat->repeatOn - dayOfWeek + 1;
				else
					day = apptRec->repeat->repeatOn + daysInWeek - dayOfWeek + 1;
	
				// If repeat-on day is between the last sunday and the last
				// saturday, make sure we're not passed the end of the month.
				if ( (apptRec->repeat->repeatOn >= domLastSun) &&
					  (day > DaysInMonth (next.month, next.year+firstYear)))
					{
					day -= daysInWeek;
					}
				next.day = day;

				// Its posible that "next date" calculated above is 
				// before the date passed.  If so, move forward
				// by the length of the repeat freguency and perform
				// the calculation again.
				if (searchForward)
					{
					if ( DateToInt(date) > DateToInt (next))
						monthsTilNext += freq;
					else
						break;
					}
				else
					{
					if ( DateToInt(date) < DateToInt (next))
						monthsTilNext -= freq;
					else
						break;
					}
				}
			break;						



		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Compute the number of month until the appointment repeats again.
			monthsDiff = ((date.year - start.year) * monthsInYear) + (date.month - start.month);
			if (searchForward)
				{
				monthsTilNext = (monthsDiff + freq - 1) / freq * freq;
				if ((date.day > start.day) && (!(monthsDiff % freq)))
					monthsTilNext += freq;
				}
			else
				{
				monthsTilNext = monthsDiff / freq * freq;
				if ((date.day < start.day) && (!(monthsDiff % freq)))
					monthsTilNext -= freq;
				}
				
			year = start.year + (start.month - 1 + monthsTilNext) / monthsInYear;
			if (year >= numberOfYears)
				return (false);

			next.year = year;
			next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
			next.day = start.day;

			// Make sure we're not passed the last day of the month.
			daysInMonth = DaysInMonth (next.month, next.year+firstYear);
			if (next.day > daysInMonth)
				next.day = daysInMonth;
			break;



		// Yearly repeating appointment.
		case repeatYearly:
			next.day = start.day;
			next.month = start.month;

			if (searchForward)
				{
				year = start.year + ((date.year - start.year + freq - 1) / freq * freq);
				if (	(date.month > start.month) ||
				  		((date.month == start.month) && (date.day > start.day)) )
					year += freq;
				}
			else
				{
				year = start.year + ((date.year - start.year) / freq * freq);
				if (	(date.month < start.month) ||
				  		((date.month == start.month) && (date.day < start.day)) ) 
					year -= freq;
				}
			

			// Specal leap day processing.
			if ( (next.month == february) && (next.day == 29) &&
				  (next.day > DaysInMonth (next.month, year+firstYear)))
				{
				next.day = DaysInMonth (next.month, year+firstYear);
				}				      
			if (year >= numberOfYears)
				return (false);

			next.year = year;	
			break;
		default:
			break;
		}
		
	if (searchForward)
		{
		// Is the next occurrence after the end date of the appointment?
		if (DateCompare (next, apptRec->repeat->repeatEndDate) > 0)
			return (false);

		ErrFatalDisplayIf ((DateToInt (next) < DateToInt (*dateP)),
			"Calculation error");
		}
	else
		{
		// Is the next occurrence before the start date of the appointment?
		if (DateCompare (next, apptRec->when->date) < 0)
			return (false);
		}

	*dateP = next;
	return (true);
}



/***********************************************************************
 *
 * FUNCTION:    NextRepeat
 *
 * DESCRIPTION: This routine computes the date of the next 
 *              occurrence of a repeating appointment.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - passed:   date to start from
 *                        returned: date of next occurrence             
 *
 * RETURNED:    true if the appointment occurs again
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95	Initial Revision
 *
 ***********************************************************************/
 /*
static Boolean NextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP)
{
	Int16  i;
	UInt16 day;
	UInt16 freq;
	UInt16 year;
	UInt16 adjust;
	UInt16 weeksDiff;
	UInt16 monthsDiff;
	UInt16 daysInMonth;
	UInt16 dayOfWeek;
	UInt16 apptWeekDay;
	UInt16 firstDayOfWeek;
	UInt16 daysTilNext;
	UInt16 monthsTilNext;
	UInt32 dateInDays;
	UInt32 startInDays;
	DateType start;
	DateType date;
	DateType next;

	date = *dateP;

	// Is the date passed after the end date of the appointment?
	if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
		return (false);
	
	// Is the date passed before the start date of the appointment?
	if (DateCompare (date, apptRec->when->date) < 0)
		date = apptRec->when->date;

	// Get the frequency on occurrecne (ex: every 2nd day, every 3rd month, etc).  
	freq = apptRec->repeat->repeatFrequency;
	
	// Get the date of the first occurrecne of the appointment.
	start = apptRec->when->date;
	

	switch (apptRec->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);
			daysTilNext = (dateInDays - startInDays + freq - 1) / freq * freq;
			if (startInDays + daysTilNext > (UInt32) maxDays)
				return (false);
			DateDaysToDate (startInDays + daysTilNext, &next);
			break;
			


		// Weekly repeating appointment (ex: every Monday and Friday). 
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) - 
				apptRec->repeat->repeatStartOfWeek + daysInWeek) % daysInWeek;

			dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
			apptWeekDay = (dayOfWeek - apptRec->repeat->repeatStartOfWeek +
				daysInWeek) % daysInWeek;

			// Are we in a week in which the appointment occurrs, if not 
			// move to that start of the next week in which the appointment
			// does occur.
			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - 
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			if (weeksDiff)
				{
				adjust = ((freq - weeksDiff) * daysInWeek)- apptWeekDay;
				apptWeekDay = 0;
				dayOfWeek = (dayOfWeek + adjust) % daysInWeek;
				}
			else
				adjust = 0;
			
			// Find the next day on which the appointment repeats.
			for (i = 0; i < daysInWeek; i++)
				{
				if (apptRec->repeat->repeatOn & (1 << dayOfWeek)) break;
				adjust++;
				if (++dayOfWeek == daysInWeek)
					dayOfWeek = 0;
				if (++apptWeekDay == daysInWeek)
					adjust += (freq - 1) * daysInWeek;
				}

			if (dateInDays + adjust > (UInt32) maxDays)
				return (false);
			DateDaysToDate (dateInDays + adjust, &next);
//			next = date;
//			DateAdjust (&next, adjust);
			break;



		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Compute the number of month until the appointment repeats again.
			monthsTilNext = (date.month - start.month);

			monthsTilNext = ((((date.year - start.year) * monthsInYear) + 
						          (date.month - start.month)) + freq - 1) /
						       freq * freq;

			while (true)
				{
				year = start.year + 
								 (start.month - 1 + monthsTilNext) / monthsInYear;
				if (year >= numberOfYears)
					return (false);

				next.year = year;
				next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
	
				dayOfWeek = DayOfWeek (next.month, 1, next.year+firstYear);
				if ((apptRec->repeat->repeatOn % daysInWeek) >= dayOfWeek)
					day = apptRec->repeat->repeatOn - dayOfWeek + 1;
				else
					day = apptRec->repeat->repeatOn + daysInWeek - dayOfWeek + 1;
	
				// If repeat-on day is between the last sunday and the last
				// saturday, make sure we're not passed the end of the month.
				if ( (apptRec->repeat->repeatOn >= domLastSun) &&
					  (day > DaysInMonth (next.month, next.year+firstYear)))
					{
					day -= daysInWeek;
					}
				next.day = day;

				// Its posible that "next date" calculated above is 
				// before the date passed.  If so, move forward
				// by the length of the repeat freguency and preform
				// the calculation again.
				if ( DateToInt(date) > DateToInt (next))
					monthsTilNext += freq;
				else
					break;
				}
			break;						



		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Compute the number of month until the appointment repeats again.
			monthsDiff = ((date.year - start.year) * monthsInYear) + 
				(date.month - start.month);
			monthsTilNext = (monthsDiff + freq - 1) / freq * freq;

			if ((date.day > start.day) && (!(monthsDiff % freq)))
				monthsTilNext += freq;
				
			year = start.year + 
							 (start.month - 1 + monthsTilNext) / monthsInYear;
			if (year >= numberOfYears)
				return (false);

			next.year = year;
			next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
			next.day = start.day;

			// Make sure we're not passed the last day of the month.
			daysInMonth = DaysInMonth (next.month, next.year+firstYear);
			if (next.day > daysInMonth)
				next.day = daysInMonth;
			break;



		// Yearly repeating appointment.
		case repeatYearly:
			next.day = start.day;
			next.month = start.month;

			year = start.year + 
				((date.year - start.year + freq - 1) / freq * freq);
			
			if ((date.month > start.month) ||
				((date.month == start.month) && (date.day > start.day)))
				 year += freq;

			// Specal leap day processing.
			if ( (next.month == february) && (next.day == 29) &&
				  (next.day > DaysInMonth (next.month, year+firstYear)))
				{
				next.day = DaysInMonth (next.month, year+firstYear);
				}				      
			if (year >= numberOfYears)
				return (false);

			next.year = year;	
			break;
		}

	// Is the next occurrence after the end date of the appointment?
	if (DateCompare (next, apptRec->repeat->repeatEndDate) > 0)
		return (false);

	ErrFatalDisplayIf ((DateToInt (next) < DateToInt (*dateP)),
		"Calculation error");

	*dateP = next;
	return (true);
}
*/

/***********************************************************************
 *
 * FUNCTION:    IsException
 *
 * DESCRIPTION: This routine returns true the date passed is in a 
 *              repeating appointment's exception list.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - date to check              
 *
 * RETURNED:    true if the date is an exception date.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95		Initial Revision
 *
 ***********************************************************************/
static Boolean IsException (ApptDBRecordPtr apptRec, DateType date)
{
	int i;
	DatePtr exceptions;

	if (apptRec->exceptions)
		{
		exceptions = &apptRec->exceptions->exception;
		for (i = 0; i < apptRec->exceptions->numExceptions; i++)
			{
			if (DateCompare (date, exceptions[i]) == 0)
			return (true);
			}
		}
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    	ApptNextRepeat
 *
 * DESCRIPTION:	This routine computes the next occurrence of a 
 *              	repeating appointment.
 *
 * PARAMETERS:		apptRec - a pointer to an appointment record
 *              	dateP   - passed:   date to start from
 *                       	 returned: date of next occurrence   
 *						searchForward	- true if searching for next occurrence
 *										  	- false if searching for most recent       
 *
 * RETURNED:		true if there is an occurrence of the appointment
 *						between the date passed and the appointment's end date
 *						(if searching forward) or start date (if searching
 *						backwards)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/20/95	Initial Revision
 *			gap	9/25/00	Add capability to search backwards for the most
 *								recent occurrence of the event (needed for attention
 *								manager support)
 *
 ***********************************************************************/
Boolean ApptNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	DateType date;
	
	date = *dateP;
	
	while (true)
		{
		// Compute the next time the appointment repeats.
		if (! FindNextRepeat (apptRec, &date, searchForward))
			return (false);

		// Check if the date computed is in the exceptions list.
		if (! IsException (apptRec, date))
			{
			*dateP = date;
			return (true);
			}
			
		DateAdjust (&date, (searchForward) ? 1 : -1);

		}		
}


/***********************************************************************
 *
 * FUNCTION:    UnDayOfMonth
 *
 * DESCRIPTION: Inverse of DayOfMonth routine.  Takes a month and year
 *		and a dayOfMonth value (e.g., dom1stSun, domLastFri, etc.) and computes
 *		what date that day is for that month.
 *
 * PARAMETERS:	month
 *					year - a year (1904, etc.)
 *					dayOfMonth - a dayOfMonth, like those returned from DayOfMonth
 *
 * RETURNED:    date in month
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	3/1/99	Initial Revision
 *
 ***********************************************************************/
static UInt16 UnDayOfMonth(UInt16 month, UInt16 year, UInt16 dayOfMonth)
{
	Int16 dayOfWeek;
	Int16 firstDayOfWeek;
	Int16 week;
	Int16 day;

	dayOfWeek = dayOfMonth % daysInWeek;
	week = dayOfMonth / daysInWeek;
	
	firstDayOfWeek = DayOfWeek(month, 1, year);
	day = (dayOfWeek - firstDayOfWeek + daysInWeek) % daysInWeek + 1 + week * daysInWeek;
	
	// adjust for last-fooday in months with only 4 foodays
	while (day > DaysInMonth(month, year))
		day -= daysInWeek;
	
	return day;
}


/***********************************************************************
 *
 * FUNCTION:    CountTotalRepeats
 *
 * DESCRIPTION: Counts the total number of times a repeating event occurs.
 * 	Returns apptNoEndDate (-1) if the event has no end date (repeats forever).
 *		The default value returned is 1, since an appointment that repeats on 0
 * 	days is not allowed.
 *
 * PARAMETERS:	apptRecP - pointer to an appointment record
 *
 * RETURNED:	number of times the appointment repeats
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	3/1/99	Initial Revision
 *
 ***********************************************************************/
static Int32 CountTotalRepeats(ApptDBRecordPtr apptRecP)
{
	DateType start;
	DateType end;
	UInt16 daysTotal;
	UInt16 freq;
	UInt16 weeks;
	UInt16 months;
	UInt16 years;
	UInt16 dayOfWeek;
	UInt16 startDOW;
	UInt16 endDOW;
	UInt16 daycount;
	UInt16 repeatOnDay;
	UInt32 startInDays;
	UInt32 endInDays;
	UInt32 firstSunday;
	UInt32 lastSaturday;

	ErrFatalDisplayIf(apptRecP == NULL, "no appointment");
	ErrFatalDisplayIf(apptRecP->repeat == NULL, "appointment does not repeat");
	ErrFatalDisplayIf(apptRecP->when == NULL, "appointment has no date");
	ErrFatalDisplayIf(apptRecP->repeat->repeatFrequency == 0, "zero repeat frequency");

	// Get the frequency of occurrence (ex: every 2nd day, every 3rd month, etc).  
	freq = apptRecP->repeat->repeatFrequency;
	
	// Get the date of the first occurrence of the appointment.
	start = apptRecP->when->date;
	
	// Get the date of the last occurrence of the appointment.
	end = apptRecP->repeat->repeatEndDate;

	// Does the appointment repeat forever?
	if (DateToInt(end) == apptNoEndDate)
		return (apptNoEndDate);
	
	// if the end date is somehow before the start date, just return 1
	if (DateCompare(end, start) < 0)
		return 1;
	
	daysTotal = 0;
	switch (apptRecP->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			startInDays = DateToDays(start);
			endInDays = DateToDays(end);
			daysTotal = ((endInDays - startInDays) / freq) + 1;
			break;
			
		// Weekly repeating appointment (ex: every Monday and Friday).
		// The strategy is to break the time period into 3 fragments that
		// are easily dealt with - days before the first sunday, whole weeks
		// from the first sunday to the last saturday, and days after the
		// last saturday.
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			startInDays = DateToDays(start);
			endInDays = DateToDays(end);
			startDOW = DayOfWeek(start.month, start.day, start.year + firstYear);
			endDOW = DayOfWeek(end.month, end.day, end.year + firstYear);
			
			// find firstSunday and lastSaturday
			if (startDOW != sunday)
				{
				firstSunday = startInDays - startDOW + daysInWeek;
				}
			else
				{
				firstSunday = startInDays;
				}
			if (endDOW != saturday)
				{
				lastSaturday = endInDays - endDOW - 1;
				}
			else
				{
				lastSaturday = endInDays;
				}
			
			// compute number of full sunday-to-saturday weeks
			weeks = (lastSaturday - firstSunday + 1) / daysInWeek;
			
			// count number of times appt repeats in a full week
			daycount = 0;
			for (dayOfWeek = sunday; dayOfWeek < daysInWeek; dayOfWeek++)
				{
				//if repeat on dayOfWeek, daycount++
				if (RepeatOnDOW(apptRecP->repeat, dayOfWeek))
					daycount++;
				}
			
			// Now we are ready to total the repetitions.
			daysTotal = 0;
			// fragment 1 - before firstSunday
			if (startDOW != sunday)
				{
				for (dayOfWeek = startDOW; dayOfWeek < daysInWeek; dayOfWeek++)
					{
					// if repeat on dayOfWeek, daysTotal++
					if (RepeatOnDOW(apptRecP->repeat, dayOfWeek))
						daysTotal++;
					}
				}
			
			// fragment 2 - full weeks from firstSunday to lastSaturday
			daysTotal += (daycount * (weeks / freq));
			
			// fragment 3 - after lastSaturday
			if (endDOW != saturday)
				{
				for (dayOfWeek = sunday; dayOfWeek <= endDOW; dayOfWeek++)
					{
					// if repeat of dayOfWeek, daysTotal++
					if (RepeatOnDOW(apptRecP->repeat, dayOfWeek))
						daysTotal++;
					}
				}
			break;


		// Monthly-by-day repeating appointment
		case repeatMonthlyByDay:
			// Compute the number of months
			months = ((end.year - start.year) * monthsInYear) + (end.month - start.month);
			
			// if the end day is too early in the last month, don't include that month
			repeatOnDay = UnDayOfMonth(end.month, end.year + firstYear,
												apptRecP->repeat->repeatOn);
			if (end.day < repeatOnDay)
				months--;
			
			daysTotal = months / freq + 1;	// repeats once every freq months
			break;
		
		
		// Monthly-by-date repeating appointment
		case repeatMonthlyByDate:
			// Compute the number of months
			months = ((end.year - start.year) * monthsInYear) + (end.month - start.month);
			
			// if the end day is too early in the last month, don't include that month
			if (end.day < start.day)
				months--;
			
			daysTotal = months / freq + 1;	// repeats once every freq months
			break;
		
		
		// Yearly repeating appointment.
		case repeatYearly:
			years = end.year - start.year;
			
			// if the end day is too early in the last year, don't include that year
			if (end.month < start.month
					|| (end.month == start.month && end.day < start.day))
				years--;
			
			daysTotal = years / freq + 1;		// repeats once every freq years
			break;
		
		default:
			daysTotal = 1;
			break;
		}
	
	ErrNonFatalDisplayIf(daysTotal == 0, "event repeats on 0 days");
	ErrNonFatalDisplayIf(daysTotal < 0, "event repeats on negative days");
	if (daysTotal <= 0)  daysTotal = 1;
	
	return (daysTotal);
}


/***********************************************************************
 *
 * FUNCTION:    ApptHasMultipleOccurences
 *
 * DESCRIPTION: Does the given appointment occur more than once?
 *
 *		This function compares the repeat info and the exception list for
 *		an appointment to determine if it has more than one visible (non-excepted)
 *		occurence.
 *		The decision is based solely on the number of times the appointment
 *		repeats versus the number of exceptions.
 *
 * PARAMETERS:  ApptDBRecordPtr apptRecP - the appointment to examine
 *
 * RETURNED:    true if the appointment occurs more than once
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	3/2/99	Initial Revision
 *
 ***********************************************************************/
Boolean ApptHasMultipleOccurences(ApptDBRecordPtr apptRecP)
{
	Int32 totalRepeats;
	Int32 numExceptions;
	
	ErrFatalDisplayIf(apptRecP == NULL, "no appointment");

	// if the appointment does not repeat, then it can't occur more than once
	if (!apptRecP->repeat)  return false;
	
	totalRepeats = CountTotalRepeats(apptRecP);
	if (apptRecP->exceptions)
		numExceptions = apptRecP->exceptions->numExceptions;
	else
		numExceptions = 0;
	
	if (totalRepeats == apptNoEndDate)  return true;
	if ((totalRepeats - numExceptions) > 1)  return true;
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    ApptListCompare
 *
 * DESCRIPTION: This routine compares two entries in the appointment list, 
 *              it's called by ApptGetAppointments via the quick sort 
 *              routine.
 *
 * PARAMETERS:  a     - a pointer to an entry in the appointment list
 *					 b     - a pointer to an entry in the appointment list
 *              extra - extra data passed to quick sort - not used
 *
 * RETURNED:    if a1 > a2  returns a positive int
 *              if a1 < a2  returns a negative int
 *              if a1 = a2  returns zero
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/15/95		Initial Revision
 *
 ***********************************************************************/
static Int16 ApptListCompare (ApptInfoPtr a1, ApptInfoPtr  a2, Int32 extra)
{
#pragma unused (extra)

	Int16 result;
	
	result = TimeCompare (a1->startTime, a2->startTime);
	if (result == 0)
		{
		result = TimeCompare (a1->endTime, a2->endTime);
		}
	return result;
}


/***********************************************************************
 *
 * FUNCTION:    ApptGetAppointments
 *
 * DESCRIPTION: This routine returns a list of appointments that are on 
 *              the date specified
 *
 * PARAMETERS:  dbP    - pointer to the database
 *              date   - date to search for
 *              countP - number of appointments on the specified 
 *                       day (returned value)
 *
 * RETURNED:    handle of the appointment list (ApptInfoType)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/15/95		Initial Revision
 *
 ***********************************************************************/
#if 0
VoidHand ApptGetAppointments (DmOpenRef dbP, DateType date, UInt16* countP)
{
	Err	error;
	Int16	result;
	Int16	count = 0;
	UInt16	recordNum;
	Boolean repeats;
	MemHandle recordH;
	MemHandle apptListH;
	ApptInfoPtr apptList;
	ApptDBRecordType apptRec;
	ApptPackedDBRecordPtr r;

	// Allocated a block to hold the appointment list.
	apptListH = MemHandleNew (sizeof (ApptInfoType) * apptMaxPerDay);
	ErrFatalDisplayIf(!apptListH, "Out of memory");
	if (! apptListH) return (0);

	apptList = MemHandleLock (apptListH);
	

	// Find the first non-repeating appointment of the day.
	if (ApptFindFirst (dbP, date, &recordNum))
		{
		while (count < apptMaxPerDay)
			{
			// Check if the appointment is on the date passed, if it is 
			// add it to the appointment list.		
			recordH = DmQueryRecord (dbP, recordNum);
			r = MemHandleLock (recordH);
			result = DateCompare (r->when.date, date);

			if (result == 0)
				{
				// Add the record to the appoitment list.
				apptList[count].startTime = r->when.startTime;				
				apptList[count].endTime = r->when.endTime;				
				apptList[count].recordNum = recordNum;	
				count++;
				}
			MemHandleUnlock (recordH);
			if (result != 0) break;

			// Get the next record.
			error = DmSeekRecordInCategory (dbP, &recordNum, 1, dmSeekForward, dmAllCategories);
			if (error == dmErrSeekFailed) break;
			}
		}


	// Add the repeating appointments to the list.  Repeating appointments
	// are stored at the beginning of the database.
	recordNum = 0;
	while (count < apptMaxPerDay)
		{
		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);
		if (! recordH) break;
		
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		
		repeats = (r->flags.repeat != 0);
		if (repeats)
			{
			ApptUnpack (r, &apptRec);
			if (ApptRepeatsOnDate (&apptRec, date))
				{
				// Add the record to the appoitment list.
				apptList[count].startTime = r->when.startTime;				
				apptList[count].endTime = r->when.endTime;				
				apptList[count].recordNum = recordNum;	
				count++;
				}
			}
		MemHandleUnlock (recordH);

		// If the record has no repeating info we've reached the end of the 
		// repeating appointments.
		if (! repeats) break;
		
		 recordNum++;
		}

	
	// Sort the list by start time.
	SysInsertionSort (apptList, count, sizeof (ApptInfoType), ApptListCompare, 0L);
	

	// If there are no appointments on the specified day, free the appointment
	// list.
	if (count == 0)
		{
		MemPtrFree (apptList);
		apptListH = 0;
		}

	// Resize the appointment list block to release any unused space.
	else
		{
		MemHandleUnlock (apptListH);
		MemHandleResize (apptListH, count * sizeof (ApptInfoType));
		}

	*countP = count;
	return (apptListH);
}
#endif


/***********************************************************************
 *
 * FUNCTION:    ApptGetAlarmTime
 *
 * DESCRIPTION: This routine determines the date and time of an alarm for
 *				the event passed.  Depending on the search direction specified,
 *				it will return either the time of the next occurrence of the alarm 
 *				to fire, or the time of the most recently triggered alarm.
 *
 * PARAMETERS:  apptRec     - pointer to an appointment record
 *              currentTime - current date and time in seconds
 *					 searchForward - designates whether to find the next (true) or 
 *										most recent (false) occurrence of an event.
 *
 * RETURNED:    date and time of the alarm, in seconds, or zero if there
 *              is no alarm
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/20/95	Initial Revision
 *			gap	9/25/00	Add capability to search backwards for the most
 *								recent occurrence of the event (needed for attention
 *								manager support)
 *			gap	10/17/00	small optimization - removed advance calculation
 *								out of while loop as we only need to do this once.
 *								also, add advance to current time in order to
 *								correctly position the start of a backward search in
 *								order to take into account the advance time.
 *
 ***********************************************************************/
UInt32 ApptGetAlarmTime (ApptDBRecordPtr apptRec, UInt32 currentTime, Boolean searchForward)
{
	UInt32				advance;
	UInt32				alarmTime;
	DateType				repeatDate;
	DateTimeType		curDateTime;
	DateTimeType		apptDateTime;

	if (!apptRec->alarm)
		return apptNoTime;

	// Non-repeating appointment?
	if (! apptRec->repeat)
		{
		// An alarm on an untimed event triggers at midnight.
		if (TimeToInt (apptRec->when->startTime) == apptNoTime)
			{
			apptDateTime.minute = 0;
			apptDateTime.hour = 0;
			}
		else
			{
			apptDateTime.minute = apptRec->when->startTime.minutes;
			apptDateTime.hour = apptRec->when->startTime.hours;
			}
		apptDateTime.second = 0;
		apptDateTime.day = apptRec->when->date.day;
		apptDateTime.month = apptRec->when->date.month;
		apptDateTime.year = apptRec->when->date.year + firstYear;



		// Compute the time of the alarm by adjusting the date and time 
		// of the appointment by the length of the advance notice.
		advance = apptRec->alarm->advance;
		switch (apptRec->alarm->advanceUnit)
			{
			case aauMinutes:
				advance *= minutesInSeconds;
				break;
			case aauHours:
				advance *= hoursInSeconds;
				break;
			case aauDays:
				advance *= daysInSeconds;
				break;
			}

		alarmTime = TimDateTimeToSeconds (&apptDateTime) - advance;
		
		if (searchForward)
			{
			if (alarmTime >= currentTime)
				return (alarmTime);
			else
				return (0);
			}
		else
			{
			if (alarmTime <= currentTime)
				return (alarmTime);
			else
				return (0);
			}
		}


	// Repeating appointment.

	// calculate the appointment alarm advance time.
	switch (apptRec->alarm->advanceUnit)
		{
		case aauMinutes:
			advance = (UInt32) apptRec->alarm->advance * minutesInSeconds;
			break;
		case aauHours:
			advance = (UInt32) apptRec->alarm->advance * hoursInSeconds;
			break;
		case aauDays:
			advance = (UInt32) apptRec->alarm->advance * daysInSeconds;
			break;
		}
	
	// if searchin backwards, adjust the start point of 
	// the search to account for the alarm advance time.
	if (!searchForward)
		TimSecondsToDateTime (currentTime+advance, &curDateTime);
	else
		TimSecondsToDateTime (currentTime, &curDateTime);
	
	repeatDate.year = curDateTime.year - firstYear;
	repeatDate.month = curDateTime.month;
	repeatDate.day = curDateTime.day;
	
	while (ApptNextRepeat (apptRec, &repeatDate, searchForward))
		{
		// An alarm on an untimed event triggers at midnight.
		if (TimeToInt (apptRec->when->startTime) == apptNoTime)
			{
			apptDateTime.minute = 0;
			apptDateTime.hour = 0;
			}
		else
			{
			apptDateTime.minute = apptRec->when->startTime.minutes;
			apptDateTime.hour = apptRec->when->startTime.hours;
			}
		apptDateTime.second = 0;
		apptDateTime.day = repeatDate.day;
		apptDateTime.month = repeatDate.month;
		apptDateTime.year = repeatDate.year + firstYear;

		// Compute the time of the alarm by adjusting the date and time 
		// of the appointment by the length of the advance notice.
		alarmTime = TimDateTimeToSeconds (&apptDateTime) - advance;
		
		if (searchForward)
			{
			if (alarmTime >= currentTime)
				return (alarmTime);

			DateAdjust (&repeatDate, 1);
			} 
		else
			{
			if (alarmTime <= currentTime)
				return (alarmTime);

			DateAdjust (&repeatDate, -1);
			} 
		}
		
	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    ApptAlarmMunge
 *
 * DESCRIPTION: Helper routine for ApptAlarmMunger. Process one appointment.
 *
 * PARAMETERS:  inDbR			- reference to open database
 *					 inPackedRecordP - pointer to packed record in storage heap
 *					 inAlarmStart	- first valid alarm time
 *					 inAlarmStop	- last valid alarm time
 *					 inOutEarliestAlarmP - ???
 *					 outAudibleP	- true if alarm sound should play, nil is ok
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			peter	3/22/00	Initial Revision. Based on existing code from
 *								ApptAlarmMunger.
 *			gap	06/06/00	Only increment the alarm counter variable (numAlarms)
 *								when a new alarm is added to the alarms as opposed to
 *								every time an alarm is processed.  Can result in a 
 *								Fatal Alert "Error Querying Record" if count does not
 *								match the number of alarms in list and just the right
 *								value happens to reside in the memory following the
 *								actual list contents.
 *
 ***********************************************************************/
static void ApptAlarmMunge (
	ApptPackedDBRecordPtr	inPackedRecordP,
	UInt16						inRecordNum,
	UInt32						inAlarmStart,
	UInt32						inAlarmStop,
	AlarmPostingDataPtr		inPostingAlarmData,
	UInt32 *						inOutEarliestAlarmP)
{
	ApptDBRecordType			apptRec;
	UInt32						alarmTime;
	UInt32 						uniqueID;
	AttnLevelType 				attnLevel;
	AttnFlagsType 				attnFlags;

	if ( inPackedRecordP->flags.alarm )
		{
		ApptUnpack (inPackedRecordP, &apptRec);
		
		// Get the first alarm on or after inAlarmStart
		alarmTime = ApptGetAlarmTime (&apptRec, inAlarmStart, true);
		
		// If in range, add the alarm to the output
		if ( alarmTime && (alarmTime >= inAlarmStart) && (alarmTime <= inAlarmStop) )
			{
			
			// Post the alarm to the attention manager if requested.
			if (inPostingAlarmData)
				{
				// get the unique ID as that is how we will reference the note in the attn mgr
				DmRecordInfo (inPostingAlarmData->dbP, inRecordNum, NULL, &uniqueID, NULL);

				// remove it from the attention manager list in the event if it is already posted
				AttnForgetIt(inPostingAlarmData->cardNo, inPostingAlarmData->dbID, uniqueID);
			
				// determine if it is an untimed event. untimed events will be subtle 
				if (TimeToInt (apptRec.when->startTime) == apptNoTime)
					{
					attnLevel = kAttnLevelSubtle;
					attnFlags = kAttnFlagsNothing;
					}
				else
					{
					attnLevel = kAttnLevelInsistent;
					attnFlags = kAttnFlagsUseUserSettings;	// Use all effects (sound, LED & vibrate) user has enabled via general prefs
					}
								
				// post the note's alarm to the attention manager queue
				AttnGetAttention(inPostingAlarmData->cardNo, inPostingAlarmData->dbID, uniqueID,
					NULL, attnLevel, attnFlags,
					inPostingAlarmData->prefs.alarmSoundRepeatInterval,
					inPostingAlarmData->prefs.alarmSoundRepeatCount);
				}
			
			// Remember the earliest in-range alarm for our return value
			if ( (alarmTime < *inOutEarliestAlarmP) || (*inOutEarliestAlarmP == 0) )
				*inOutEarliestAlarmP = alarmTime;
			}	
		}
}


/***********************************************************************
 *
 * FUNCTION:    ApptAlarmMunger
 *
 * DESCRIPTION: Finds all appointments with alarms within a given range
 *					 of time.
 *
 * PARAMETERS:  dbR			- reference to open database
 *					 alarmStart	- first valid alarm time
 *					 alarmStop	- last valid alarm time
 *					 countP		- in: # of alarms to fit into alarmListH
 *									  out: # of alarms found
 *									  nil is ok
 *					 alarmListP	- list of alarms found, nil is ok
 *					 audibleP	- true if alarm sound should play, nil is ok
 *
 * RETURNED:    time of the first alarm within range
 *
 * COMMENTS:	 To find all alarms at a specific time, set alarmStart
 *					 and alarmStop to the same time.
 *
 *					 To find the just the time of the next alarm, set
 *					 alarmStart to now+1, alarmStop to max, and the three
 *					 output parameters to nil.
 *              
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/20/99	Initial Revision. Based on existing code from
 *								ApptGetTimeOfNextAlarm and ApptGetAlarmsList
 *			peter	3/22/00	Pulled out guts into ApptAlarmMunge and replaced
 *								simple loop with optimized pair of loops
 *
 ***********************************************************************/

static UInt32 ApptAlarmMunger (
	DmOpenRef					inDbR,
	UInt32						inAlarmStart,
	UInt32						inAlarmStop,
	Boolean						postAlarmsInRange )
{
	UInt16						numRecords;
	UInt16						recordNum;
	UInt16						firstNonRepeatingEventNum;
	UInt16						firstEventOnOrAfterAlarmStart;
	UInt32						earliestAlarm = 0;
	MemHandle					recordH;
	UInt32						uniqueID;
	UInt32						startSeconds;
	ApptPackedDBRecordPtr	r;
	DateTimeType				startDateTime;
	AlarmPostingData			postingData;
	AlarmPostingDataPtr		postingDataPtr;
	
	
	// If alarms found in range are to be posted to the attention manager cache all
	// the information that is the same for all alarms to be posted.
	if (postAlarmsInRange)
		{
		DmSearchStateType	searchInfo;
		
		postingData.dbP = inDbR;
		
		// Load Date Book's prefs so we can get the user-specified alarm sound and nag information
		DatebookLoadPrefs (&postingData.prefs);

		// get the card number & dataBase ID for the app
		DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &postingData.cardNo, &postingData.dbID);

		postingDataPtr = &postingData;
		}
	else
		postingDataPtr = NULL;
		
	
	// First process all the repeating events, since they're sorted before all other events:
	numRecords = DmNumRecords (inDbR);
	for (recordNum = 0; recordNum < numRecords; recordNum++)
		{
		recordH = DmQueryRecord (inDbR, recordNum);
		DmRecordInfo (inDbR, recordNum, NULL, &uniqueID, NULL);
		if ( !recordH )
			break;
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		if ( ! r->flags.repeat )
			{
			MemHandleUnlock (recordH);
			break;
			}
		
		ApptAlarmMunge (r, recordNum, inAlarmStart, inAlarmStop, postingDataPtr, &earliestAlarm);
		
		MemHandleUnlock (recordH);
		}
	firstNonRepeatingEventNum = recordNum;
	
	// Then search from the end for the first event on or after the specified alarm start,
	// since the typical search is for alarms around now, but if no purging is done,
	// there can be far more records in the past than in the future.
	for (recordNum = numRecords - 1;
		recordNum >= firstNonRepeatingEventNum && recordNum < numRecords;
			// in case firstNonRepeatingEventNum is zero, otherwise loop will never exit
		recordNum--)
		{
		recordH = DmQueryRecord (inDbR, recordNum);
		DmRecordInfo (inDbR, recordNum, NULL, &uniqueID, NULL);
		if ( !recordH )
			continue;		// Skip over deleted records at the end
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		startDateTime.year = r->when.date.year + firstYear;
		startDateTime.month = r->when.date.month;
		startDateTime.day = r->when.date.day;
		startDateTime.hour = r->when.startTime.hours;
		startDateTime.minute = r->when.startTime.minutes;
		startDateTime.second = 0;
		startSeconds = TimDateTimeToSeconds(&startDateTime);
		MemHandleUnlock (recordH);
		if ( startSeconds < inAlarmStart )
			break;
		}
	firstEventOnOrAfterAlarmStart = recordNum + 1;
	
	// On release ROMs, we skip non-repeating events before the specified alarm time,
	// but on debug ROMs, we take the time to verify that none of these skipped events
	// would have any effect on the output of this routine. This effectively asserts
	// that our assumption about the order of the events in the database is correct.
	// That is: first all repeating events, then all non-repeating events, in order from
	// earliest to latest start date/time, then archived events and slots where deleted
	// events were.
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
	for (recordNum = firstNonRepeatingEventNum; recordNum < firstEventOnOrAfterAlarmStart; recordNum++)
		{
		UInt32							oldEarliestAlarm;
		
		recordH = DmQueryRecord (inDbR, recordNum);
		DmRecordInfo (inDbR, recordNum, NULL, &uniqueID, NULL);
		if ( !recordH )
			break;
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		
		oldEarliestAlarm = earliestAlarm;
		ApptAlarmMunge (r, recordNum, inAlarmStart, inAlarmStop, NULL, &earliestAlarm);
		ErrNonFatalDisplayIf(oldEarliestAlarm != earliestAlarm,
			"An earlier alarm was found in skipped records");
					
		MemHandleUnlock (recordH);
		}
#endif
	
	// Then process non-repeating events on or after the specified alarm start. We can ignore
	// events before this because alarms always occur on or before the start of the event.
	// We process the events in the order they appear in the database, so that they get added
	// to the list in the proper order.
	for (recordNum = firstEventOnOrAfterAlarmStart; recordNum < numRecords; recordNum++)
		{
		recordH = DmQueryRecord (inDbR, recordNum);
		DmRecordInfo (inDbR, recordNum, NULL, &uniqueID, NULL);
		if ( !recordH )
			break;
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		
		ApptAlarmMunge (r, recordNum, inAlarmStart, inAlarmStop, postingDataPtr, &earliestAlarm);
		
		MemHandleUnlock (recordH);
		}

	return earliestAlarm;
}


/***********************************************************************
 *
 * FUNCTION:    ApptGetTimeOfNextAlarm
 *
 * DESCRIPTION: This routine determines the time of the next scheduled
 *              alarm.
 *
 * PARAMETERS:  dbP           - pointer to the database
 *              timeInSeconds - time to search forward from
 *
 * RETURNED:    time of the next alarm, in seconds from 1/1/1904, or
 *              zero if there are no alarms scheduled.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/20/95	Initial Revision
 *			rbb	4/20/99	Rewritten to use new ApptAlarmMunger
 *			gap	9/28/00	Rewritten to remove snooze & alarm internals
 *								replaced by attention manager
 *
 ***********************************************************************/
UInt32 ApptGetTimeOfNextAlarm (DmOpenRef inDbR, UInt32 inAlarmStart )	
{
	UInt32	roundedStart;
	UInt32	nextAlarm;
		
	roundedStart = inAlarmStart - (inAlarmStart % minutesInSeconds);
	if (roundedStart != inAlarmStart)
		roundedStart += minutesInSeconds;
		
	nextAlarm = ApptAlarmMunger (inDbR, roundedStart, apptEndOfTime, false);
		
	return nextAlarm;
}


/***********************************************************************
 *
 * FUNCTION:    ApptPostTriggeredAlarms
 *
 * DESCRIPTION: This routine posts all events that have an alarm matching
 *              the specified time to the attention manager.
 *
 * PARAMETERS:  dbP           - pointer to the database
 *              timeInSeconds - time of alarm that triggered
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	9/28/00	Initial Revision
 *
 ***********************************************************************/
void ApptPostTriggeredAlarms (DmOpenRef inDbR, UInt32 inAlarmTime)	
{
	ApptAlarmMunger (inDbR, inAlarmTime, inAlarmTime, true);
}


/***********************************************************************
 *
 * FUNCTION:    AddAppointmentToList
 *
 * DESCRIPTION: This routine adds an appointment to a list of appointments. 
 *
 * PARAMETERS:  apptListH - pointer to list to add appointment to
 *					 count     - number of appointments on the specified day
 *					 startTime - start time of appointment
 *					 endTime   - end time of appointment
 *					 recordNum - index of associated record
 *
 * RETURNED:    true if successful, false if error
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/17/96	Initial Revision
 *			rbb	4/27/99	Patched error detection when allocating apptListH
 *
 ***********************************************************************/
static Boolean AddAppointmentToList (MemHandle * apptListH, UInt16 count,
	TimeType startTime, TimeType endTime, UInt16 recordNum)
{
	Err err;
	UInt16 newSize;
	ApptInfoPtr apptList;

	if (count == 0)
		{
		// Allocated a block to hold the appointment list.
		*apptListH = MemHandleNew (sizeof (ApptInfoType) * (apptMaxPerDay / 10));
		ErrFatalDisplayIf(!*apptListH, "Out of memory");
		if (! *apptListH) return (false);
		apptList = MemHandleLock (*apptListH);
		}
		
	// Resize the list to hold more more appointments.
	else if ((count % (apptMaxPerDay / 10)) == 0)
		{
		if (count + (apptMaxPerDay / 10) > apptMaxPerDay)
			return (false);
		
		newSize = sizeof (ApptInfoType) * (count + (apptMaxPerDay / 10));
		err = MemHandleResize (*apptListH, newSize);
		apptList = MemHandleLock (*apptListH);

		ErrFatalDisplayIf(err, "Out of memory");
		if (err) return (false);
		}

	else
		apptList = MemHandleLock(*apptListH);

	apptList[count].startTime = startTime;				
	apptList[count].endTime = endTime;						
	apptList[count].recordNum = recordNum;	

	MemHandleUnlock(*apptListH);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    ApptGetAppointments
 *
 * DESCRIPTION: This routine returns a list of appointments that are in 
 *              the range of dates specified
 *
 * PARAMETERS:  dbP       - pointer to the database
 *              date   	  - start date to search from
 *              days   	  - number a days in search range
 *              apptLists - returned: array of handle of the 
 *									 appointment list (ApptInfoType)
 *              counts    - returned: returned: array of counts of the 
 *                          number of appointments in each list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/7/96	Initial Revision
 *
 ***********************************************************************/
void ApptGetAppointments (DmOpenRef dbP, DateType date, UInt16 days,
	MemHandle apptLists [], UInt16 counts [])
{
	UInt16	startDate;
	UInt16	endDate;
	UInt16	index;
	UInt16	recordNum;
	UInt32	dateInDays;
	DateType	apptDate;
	TimeType startTime;
	TimeType endTime;
	DateType tempDate;
	DateType repeatDate;
	Boolean repeats;
	MemHandle recordH;
	ApptInfoPtr apptList;
	ApptDBRecordType apptRec;
	ApptPackedDBRecordPtr r;

	
	MemSet (apptLists, days * sizeof (MemHandle), 0);
	MemSet (counts, days * sizeof (UInt16), 0);


	startDate = DateToInt (date);
	tempDate = date;
	DateAdjust (&tempDate, days-1);
	endDate = DateToInt(tempDate);

	// Find the first non-repeating appointment of the day.
	ApptFindFirst (dbP, date, &recordNum);
	while (true)
		{
		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);
		if (! recordH) break;

		// Check if the appointment is on the date passed, if it is 
		// add it to the appointment list.		
		r = MemHandleLock (recordH);
		startTime = r->when.startTime;
		endTime = r->when.endTime;
		apptDate = r->when.date;
		MemHandleUnlock (recordH);
		
		if ((DateToInt (apptDate) < startDate) || 
			 (DateToInt (apptDate) > endDate))
			break;
		
		// Add the record to the appoitment list.
		index = DateToDays (apptDate) - DateToDays (date);
		
		if (AddAppointmentToList (&apptLists[index], counts[index], 
				startTime, endTime, recordNum))
			counts[index]++;
		else
			break;
			
		recordNum++;
		}


	// Add the repeating appointments to the list.  Repeating appointments
	// are stored at the beginning of the database.
	recordNum = 0;
	dateInDays = DateToDays (date);
	while (true)
		{
		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);
		if (! recordH) break;
		
		r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
		repeats = (r->flags.repeat != 0);
		
		if (repeats)
			{
			ApptUnpack (r, &apptRec);

			if (days == 1)
				{
				if (ApptRepeatsOnDate (&apptRec, date))
					{
					if (AddAppointmentToList (apptLists, *counts, 
							r->when.startTime, r->when.endTime, recordNum))
						(*counts)++;
					}
				}
				
			else
				{
				repeatDate = date;
			 	while (ApptNextRepeat (&apptRec, &repeatDate, true))
					{
					if (DateToInt (repeatDate) > endDate)
						break;
					
					// Add the record to the appoitment list.
					index = DateToDays (repeatDate) - dateInDays;
	
					if (AddAppointmentToList (&apptLists[index], counts[index], 
							r->when.startTime, r->when.endTime, recordNum))
						counts[index]++;
					else
						break;
	
					if (DateToInt (repeatDate) == endDate)
						break;

					DateAdjust (&repeatDate, 1);
					}
				}
			}
		MemHandleUnlock (recordH);

		// If the record has no repeating info we've reached the end of the 
		// repeating appointments.
		if (! repeats) break;
		
		 recordNum++;
		}

	
	// Sort the list by start time.
	for (index = 0; index < days; index ++)
		{
		if (apptLists[index])
			{
			apptList = MemHandleLock(apptLists[index]);
			SysInsertionSort (apptList, counts[index], sizeof (ApptInfoType), 
				(_comparF *)ApptListCompare, 0L);
	
			MemHandleUnlock (apptLists[index]);
			MemHandleResize (apptLists[index], counts[index] *
				sizeof (ApptInfoType));
			}
		}
}


/************************************************************
 *
 *  FUNCTION: ApptGetRecord
 *
 *  DESCRIPTION: Get a read-only record from the Appointment Database
 *
 *  PARAMETERS: database pointer
 *				    database index
 *				    database record
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err ApptGetRecord (DmOpenRef dbP, UInt16 index, ApptDBRecordPtr r, 
	MemHandle * handleP)
{
	MemHandle handle;
	ApptPackedDBRecordPtr src;
	

	handle = DmQueryRecord(dbP, index);
	ErrFatalDisplayIf(DmGetLastErr(), "Error Querying record");
	
	src = (ApptPackedDBRecordPtr) MemHandleLock (handle);

	if (DmGetLastErr())
		{
		*handleP = 0;
		return DmGetLastErr();
		}
	
	ApptUnpack(src, r);
	
	*handleP = handle;
	return 0;
}


/************************************************************
 *
 *  FUNCTION: ApptChangeRecord
 *
 *  DESCRIPTION: Change a record in the Appointment Database
 *
 *  PARAMETERS: database pointer
 *					 database index
 *					 database record
 *					 changed fields
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *	COMMENTS:	Records are not stored with extra padding - they
 *	are always resized to their exact storage space.  This avoids
 *	a database compression issue.  The code works as follows:
 *	
 *	1)	get the size of the new record
 *	2)	make the new record
 *	3)	pack the packed record plus the changes into the new record
 *	4)	if the sort position is changes move to the new position
 *	5)	attach in position
 *
 *************************************************************/
Err ApptChangeRecord(DmOpenRef dbP, UInt16 *index, ApptDBRecordPtr r, 
	ApptDBRecordFlags changedFields)
{
	Err result;
	Int16 newIndex;
	UInt16 attributes;
	Boolean dontMove;
	MemHandle oldH;
	MemHandle srcH;
	MemHandle dstH;
	ApptDBRecordType src;
	ApptPackedDBRecordPtr dst;
	ApptPackedDBRecordPtr cmp;
	
	// We do not assume that r is completely valid so we get a valid
	// ApptDBRecordPtr...
	if ((result = ApptGetRecord(dbP, *index, &src, &srcH)) != 0)
		return result;
	
	// and we apply the changes to it.
	if (changedFields.when) 
		src.when = r->when;
	
	if (changedFields.alarm) 
		src.alarm = r->alarm;
	
	if (changedFields.repeat)
		src.repeat = r->repeat;
	
	if (changedFields.exceptions)
		src.exceptions = r->exceptions;
		
	if (changedFields.description)
		src.description = r->description;
		
	if (changedFields.note)
		src.note = r->note;


	// Allocate a new chunk with the correct size and pack the data from 
	// the unpacked record into it.
	dstH = DmNewHandle(dbP, (UInt32) ApptPackedSize(&src));
	if (dstH)
		{
		dst = MemHandleLock (dstH);
		ApptPack (&src, dst);
		}

	MemHandleUnlock (srcH);
	if (dstH == NULL)
		return dmErrMemError;


	// If the sort position is changed move to the new position.
	// Check if any of the key fields have changed. 
	if ((!changedFields.when) && (! changedFields.repeat))
		goto attachRecord;				// repeating events aren't in sorted order
	
		
	// Make sure *index-1 < *index < *index+1, if so it's in sorted 
	// order.  Leave it there.	
	if (*index > 0)
		{
		// This record wasn't deleted and deleted records are at the end of the
		// database so the prior record may not be deleted!
		cmp = MemHandleLock (DmQueryRecord(dbP, *index-1));		
		dontMove = (ApptComparePackedRecords (cmp, dst, 0, NULL, NULL, 0) <= 0);
		MemPtrUnlock (cmp);
		}
	else 
		dontMove = true;


	if (dontMove && (*index+1 < DmNumRecords (dbP)))
		{
		DmRecordInfo(dbP, *index+1, &attributes, NULL, NULL);
		if ( ! (attributes & dmRecAttrDelete) )
			{
			cmp = MemHandleLock (DmQueryRecord(dbP, *index+1));
			dontMove = dontMove && ApptComparePackedRecords (dst, cmp, 0, NULL, NULL, 0) <= 0;
			MemPtrUnlock (cmp);
			}
		}
		
	if (dontMove)
		goto attachRecord;
	
	
	// The record isn't in the right position.  Move it.
	newIndex = ApptFindSortPosition (dbP, dst);
	DmMoveRecord (dbP, *index, newIndex);
	if (newIndex > *index) newIndex--;
	*index = newIndex;						// return new position


attachRecord:
	// Attach the new record to the old index,  the preserves the 
	// category and record id.
	result = DmAttachRecord (dbP, index, dstH, &oldH);
	
	MemPtrUnlock(dst);

	if (result) return result;

	MemHandleFree(oldH);

	#if	EMULATION_LEVEL != EMULATION_NONE
		ECApptDBValidate (dbP);
	#endif

	return 0;
}


/************************************************************
 *
 *  FUNCTION: ApptNewRecord
 *
 *  DESCRIPTION: Create a new packed record in sorted position
 *
 *  PARAMETERS: database pointer
 *				database record
 *
 *  RETURNS: ##0 if successful, error code if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err ApptNewRecord(DmOpenRef dbP, ApptDBRecordPtr r, UInt16 *index)
{
	MemHandle recordH;
	ApptPackedDBRecordPtr recordP;
	UInt16 					newIndex;
	Err err;

	
	// Make a new chunk with the correct size.
	recordH = DmNewHandle (dbP, (UInt32) ApptPackedSize(r));
	if (recordH == NULL)
		return dmErrMemError;

	recordP = MemHandleLock (recordH);
	
	// Copy the data from the unpacked record to the packed one.
	ApptPack (r, recordP);

	newIndex = ApptFindSortPosition(dbP, recordP);

	MemPtrUnlock (recordP);

	// 4) attach in place
	err = DmAttachRecord(dbP, &newIndex, recordH, 0);
	if (err) 
		MemHandleFree(recordH);
	else
		*index = newIndex;
	
	return err;
}
