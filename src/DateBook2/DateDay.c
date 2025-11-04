/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateDay.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This is the Datebook application's main module.  This module
 *   starts the application, dispatches events, and stops
 *   the application.
 *
 * History:
 *		June 12, 1995	Created by Art Lamb
 *			Name		Date		Description
 *			----		----		-----------
 *			???		????		Initial Revision
 *			frigino	970909	Added alarmSoundUniqueRecID to DatebookPreferenceType
 *									to remember the alarm sound to play. Moved
 *									DatebookPreferenceType out of this file.
 *			grant		3/5/99	Removed dependece on MemDeref and MemoryPrv.h.
 *									DetailsH was a handle that was always left locked;
 *									replaced by a pointer DetailsP.
 *			rbb		4/9/99	Removed time bar and end-time for zero-duration appts
 *			rbb		4/22/99	Added snooze
 *			rbb		6/10/99	Removed obsoleted code that worked around
 *									single-segment linker limitation
 *			grant		6/28/99	New global - RepeatDetailsP.  When editing an event's details,
 *									there is one details info block that is pointed to by either
 *									DetailsP or RepeatDetailsP but not both.  When the "Details"
 *									form is active, then DetailsP is valid and RepeatDetailsP
 *									should be NULL.  And vice versa for the "Repeat" form.
 *			gap		8/27/99	Replaced call of ExceptionAlert with call to RangeDialog
 *									in DetailsApply().
 *			gap		9/8/99	Added current, current & future, or all occurrences support
 *									for addition/removal of notes.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <Graffiti.h>
#include <SysEvtMgr.h>
#include <TxtGlue.h>		// For TxtGlueUpperChar

#include <PalmUtils.h>

#include "DateAlarm.h"
#include "Datebook.h"

#if WRISTPDA

// Note View

#ifdef NewNoteView
#undef NewNoteView
#endif

#define NewNoteView						10950

#ifdef NoteField
#undef NoteField
#endif

#define NoteField						10951

#ifdef NoteDoneButton
#undef NoteDoneButton
#endif

#define NoteDoneButton 					10952

#ifdef NoteDeleteButton
#undef NoteDeleteButton
#endif

#define NoteDeleteButton 				10953

#ifdef NoteScrollBar
#undef NoteScrollBar
#endif

#define NotePageUp						10954
#define NotePageDown					10955

// Scroll button labels.

static char UpArrowEnabled[]    = {   1, 0 };
static char DownArrowEnabled[]  = {   2, 0 };
static char UpArrowDisabled[]   = {   3, 0 };
static char DownArrowDisabled[] = {   4, 0 };
static char UpArrowHidden[]     = { ' ', 0 };
static char DownArrowHidden[]   = { ' ', 0 };

#endif

extern void ECApptDBValidate (DmOpenRef dbP);


/***********************************************************************
 *
 *	Global variables, declarded in DateGlobals.c.  Because of a bug in
 *  the Metrowerks compiler, we must compile the globals separately with
 *	 PC-relative strings turned off.
 *
 ***********************************************************************/
extern	MemHandle				ApptsH;
extern	UInt16					NumAppts;
extern	MemHandle				ApptsOnlyH;
extern	UInt16					NumApptsOnly;

extern	UInt16					TopVisibleAppt;
extern privateRecordViewEnum	CurrentRecordVisualStatus;		// applies to current record
extern privateRecordViewEnum	PrivateRecordVisualStatus;		// applies to all other records
extern	UInt16					PendingUpdate;						// code of pending day view update


// The following global variables are used to keep track of the edit
// state of the application.
extern 	UInt16					CurrentRecord;						// record being edited
extern 	Boolean					ItemSelected;						// true if a day view item is selected
extern 	UInt16					DayEditPosition;					// position of the insertion point in the desc field
extern	UInt16					DayEditSelectionLength;			// length of the current selection.
extern	Boolean					RecordDirty;						// true if a record has been modified


// The following global variables are only valid while editng the detail
// of an appointment.
extern	void*						DetailsP;
extern	DateType					RepeatEndDate;
extern	RepeatType				RepeatingEventType;
extern	UInt16					RepeatStartOfWeek;				// status of Repeat Dialog.

// This global variable is only valid while editing the repeat info of an appointment.
extern	void *				RepeatDetailsP;

extern	UInt16				TimeBarColumns;					// Number of columns of time bars.

// The following structure is used by the details dialog to hold
// changes made to an appointment record.
typedef struct {
	Boolean					secret;
	UInt8						reserved;
	ApptDateTimeType		when;
	AlarmInfoType 			alarm;
	RepeatInfoType 		repeat;
} DetailsType;

typedef DetailsType * DetailsPtr;


typedef enum {
	dateRangeNone,
	dateRangeCurrent,
	dateRangeCurrentAndFuture,
	dateRangeAll
} DateRangeType;


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void DayViewLoadTable (void);

static void DayViewLayoutDay (Boolean retieve);

static void DayViewDrawTimeBars (void);

static void DayViewInitRow (TablePtr table, UInt16 row, UInt16 apptIndex, 
	Int16 rowHeight, UInt32 uniqueID, UInt16 iconsWidth, FontID fontID);

static void DayViewUpdateDisplay (UInt16 updateCode);

static Err SplitRepeatingEvent (UInt16* indexP);

static Boolean RepeatDescRectHandler(FormGadgetTypeInCallback *gadgetP, UInt16 cmd, void *paramP);


#if WRISTPDA

static void ListViewUpdateScrollButtons( void );

static void NoteViewUpdateScrollButtons( void );

#endif


/***********************************************************************
 *
 * FUNCTION:    DoSecurity
 *
 * DESCRIPTION: Bring up security dialog and then reopen database if
 *						necessary.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			jaq		6/12/99		Initial Revision
 *
 ***********************************************************************/
void DoSecurity (void)
{
	Boolean		wasHiding;
	UInt16 mode;

	wasHiding = (PrivateRecordVisualStatus == hidePrivateRecords);
	 
	PrivateRecordVisualStatus = CurrentRecordVisualStatus = SecSelectViewStatus();
	
	if (wasHiding ^ (PrivateRecordVisualStatus == hidePrivateRecords)) //xor on two logical values - mode to open DB has changed
		{
		// Close the application's data file.
		DmCloseDatabase (ApptDB);	
		
		mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
			dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
		
		ApptDB = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
		ErrFatalDisplayIf(!ApptDB,"Can't reopen DB");
		}
		
	//For safety, simply reset the currentRecord
	//CurrentRecord = noRecordSelected;
}


/***********************************************************************
 *
 * FUNCTION:    SetDateToNextOccurrence
 *
 * DESCRIPTION: This routine set the "current date" global variable to
 *              the date that the specified record occurs on.  If the
 *              record is a repeating appointmnet, we set the date to 
 *              the next occurrence of the appointment.  If we are beyond
 *              the end date of the repeating appointment, we set the 
 *              date to the last occurrence of the event.
 *
 * PARAMETERS:  recordNum - index of appointment record 
 *
 * RETURNED:    true if successful, false if not.  It's posible that 
 *              a repeating event may have no displayable occurrences.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/24/95	Initial Revision
 *
 ***********************************************************************/
static Boolean SetDateToNextOccurrence (UInt16 recordNum)
{
	Boolean dateSet = true;
	MemHandle recordH;
	DateType today;
	DateTimeType dateTime;
	ApptDBRecordType apptRec;

	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);

	if (! apptRec.repeat)
		Date = apptRec.when->date;

	// If the appointment is a repeating event,  go to the date of the 
	// next occurrence of the appointment.
	else
		{
		// Get today's date.
		TimSecondsToDateTime (TimGetSeconds (), &dateTime);
		today.year = dateTime.year - firstYear;
		today.month = dateTime.month;
		today.day = dateTime.day;

		Date = today;
		if ( ! ApptNextRepeat (&apptRec, &Date, true))
			{
			// If we are beyond the end date of the repeating event, display
			// the last occurrence of the event.
			Date = apptRec.repeat->repeatEndDate;
							
			if ( ! ApptNextRepeat (&apptRec, &Date, false))
				{
				// It posible that there are no occurences that are displayable
				// (ex: an expections is created for each occurrences),  if so
				// just go to today.
				ErrDisplay ("No displayable occurrences of event");
				Date = today;
				dateSet = false;
				}
			}
		}
	MemHandleUnlock (recordH);
	
	return (dateSet);
}


/***********************************************************************
 *
 * FUNCTION:    GetTime
 *
 * DESCRIPTION: This routine selects the start and end time of an event.
 *
 * PARAMETERS:  startP  - passed:   current start time
 *                        returned: selected start time
 *              endtP   - passed:   current end time
 *                        returned: selected end time
 *              titleID - resource id of the title to display in the 
 *                        time picker dialog.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/24/95	Initial Revision
 *
 ***********************************************************************/
static Boolean GetTime (TimePtr startP, TimePtr endP, UInt16 titleStrID)
{
	Char* title;
	TimeType start, end;
	Boolean selected;
	Boolean untimed;
	DateTimeType dateTime;
	Int16	firstDisplayHour;
	
	
	// Get today's date, are we displaying today?
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	if (	Date.year == dateTime.year - firstYear &&
			Date.month == dateTime.month &&
			Date.day == dateTime.day)
		{
		firstDisplayHour = dateTime.hour+1;
		}
	else //not today
		{
		firstDisplayHour = DayStartHour;
		}

	// If the event is untimed, pass the default start time
	// and duration.
	if (TimeToInt (*startP) == apptNoTime)
		{
		untimed = true;
		start.hours = min (firstDisplayHour, hoursPerDay-1);
		start.minutes = 0;
		end.hours = min (firstDisplayHour + 1, hoursPerDay);
		end.minutes = 0;
		}
	else
		{
		untimed = false;
		start = *startP;
		end = *endP;
		}
		
	
	title = MemHandleLock(DmGetResource (strRsc, titleStrID));

	selected = SelectTime (&start, &end, untimed, title, DayStartHour, DayEndHour,start.hours);

	MemPtrUnlock (title);
	
	if (selected)
		{
		*startP = start;
		*endP = end;
		}
	
	return (selected);
}


/***********************************************************************
 *
 * FUNCTION:    ShowObject
 *
 * DESCRIPTION: This routine set an object usable and draws the object if
 *              the form it is in is visible.
 *
 * PARAMETERS:  frm      - pointer to a form
 *              objectID - id of the object to set usable
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void ShowObject (FormPtr frm, UInt16 objectID)
{
	FrmShowObject (frm, FrmGetObjectIndex (frm, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    HideObject
 *
 * DESCRIPTION: This routine set an object not-usable and erases it
 *              if the form it is in is visible.
 *
 * PARAMETERS:  frm      - pointer to a form
 *              objectID - id of the object to set not usable
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void HideObject (FormPtr frm, UInt16 objectID)
{
	FrmHideObject (frm, FrmGetObjectIndex (frm, objectID));
}


/***********************************************************************
 *
 * FUNCTION:    DeleteRecord
 *
 * DESCRIPTION: This routine deletes the specified appointment record.
 *
 * PARAMETERS:  record index
 *
 * RETURNED:    true if the record was deleted, false if the delete 
 *              operation was canceled.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/1/95	Initial Revision
 *			kcr	10/24/95	fixed bug with selection index of 'SaveBackup'
 *								object
 *			rbb	5/20/99	Added support for deleting "Current & Future"
 *			gap	8/01/00	add attention manager support
 *
 ***********************************************************************/
static Boolean DeleteRecord (UInt16 recordNum)
{
	Err err = errNone;
	UInt16 ctlIndex;
	UInt16 alertButton = RangeAllButton;
	FormPtr alert;
	MemHandle recordH;
	Boolean archive;
	//Boolean exception = false;
	ApptDBRecordType apptRec;
	ApptDBRecordFlags apptFlags = {};
	RepeatInfoType repeatInfo;
	DateRangeType dateRange = dateRangeAll;
	Boolean hasAlarm;

	
	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if an exception is being created.
	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
	
	if (apptRec.repeat && ApptHasMultipleOccurences(&apptRec))
		{
		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		// If the alert was canceled don't delete the record.
		if (alertButton == RangeCancelButton)
			{
			dateRange = dateRangeNone;
			}

		else if (alertButton == RangeCurrentButton)
			{
			dateRange = dateRangeCurrent;
			//exception = true;
			}
		
		else if (alertButton == RangeFutureButton)
			{
			// When editing the first instance, deleting "this and future"
			// will cause the event to disappear. Modify the dialog result
			// to trigger the confirmation dialog.
			if (DateToDays(apptRec.when->date) != DateToDays(Date))
				{
				dateRange = dateRangeCurrentAndFuture;
				repeatInfo = *(apptRec.repeat);
				}
			else
				{
				dateRange = dateRangeAll;
				}
			}
		}

	hasAlarm = (apptRec.alarm != NULL);	
	
	// Release the appointment record (apptRec is no longer valid)
	MemHandleUnlock (recordH);


	switch (dateRange)
		{
		// ignore dateRangeNone

		case dateRangeCurrent:
			// if the selected event had an alarm, be sure to remove it
			// from the posted alarm queue before the event is changed.
			if (hasAlarm)
				DeleteAlarmIfPosted(recordNum);

			// Add an exception to the current record.
			err = ApptAddException (ApptDB, &recordNum, Date);
			break;
		
		case dateRangeCurrentAndFuture:
			// if the selected event had an alarm, be sure to remove it
			// from the posted alarm queue before the event is changed.
			if (hasAlarm)
				DeleteAlarmIfPosted(recordNum);

			// Clip the meeting's end date to just before the displayed date
			repeatInfo.repeatEndDate = Date;
			DateAdjust (&repeatInfo.repeatEndDate, -1);

			// Fill part of apptRec with the new repeat info and mark the field
			// as modified. Only marked fields will be used by ApptChangeRecord.
			apptRec.repeat = &repeatInfo;
			apptFlags.repeat = true;
			
			err = ApptChangeRecord (ApptDB, &recordNum, &apptRec, apptFlags);
			break;
		
		case dateRangeAll:
			// Display an alert to comfirm the delete operation.
			alert = FrmInitForm (DeleteApptDialog);
		
			ctlIndex = FrmGetObjectIndex (alert, DeleteApptSaveBackup);
			FrmSetControlValue (alert, ctlIndex, SaveBackup);

			alertButton = FrmDoDialog (alert);
			archive = FrmGetControlValue (alert, ctlIndex);
			FrmDeleteForm (alert);

			if (alertButton == DeleteApptCancel)
				{
				dateRange = dateRangeNone;
				}
			else
				{
				SaveBackup = archive;
				
				// if the event to be deleted had an alarm, be sure to remove it
				// from the posted alarm queue before the event is deleted.
				if (hasAlarm)
					DeleteAlarmIfPosted(recordNum);

				// Delete or archive the record
				if (archive)
					DmArchiveRecord (ApptDB, recordNum);
				else
					DmDeleteRecord (ApptDB, recordNum);
					
				// move it to the end of the DB
				DmMoveRecord (ApptDB, recordNum, DmNumRecords (ApptDB));
				}
			break;
		default:
			break;
		}

	if (dateRange != dateRangeNone)
		{
		if (err)
			{
			FrmAlert (DeviceFullAlert);
			return (false);
			}

		// If the event to be delete had an alarm, be sure to remove it
		// from the posted alarm queue before the event is deleted and
		// reschedule the next alarm.
		if (hasAlarm)
			RescheduleAlarms (ApptDB);
		}

	return (dateRange != dateRangeNone);
}


/***********************************************************************
 *
 * FUNCTION:    CreateException
 *
 * DESCRIPTION: This routine creates an exception record.  It does not
 *              add the exception to the exceptions list of the original 
 *              record.
 *
 * PARAMETERS:  newRec - new record
 *              indexP - passed:   index of record being excepted
 *                       returned: index of exception record created
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	11/22/95	Initial Revision
 *
 ***********************************************************************/
static Err CreateException (ApptDBRecordPtr newRec, UInt16* indexP)
{
	Err err;
	UInt16 attr;
	
	// Get the secert setting of the record.
	DmRecordInfo (ApptDB, *indexP, &attr, NULL, NULL);	

	err = ApptNewRecord (ApptDB, newRec, indexP);				

	if (! err)
		{
		if (attr & dmRecAttrSecret)
			{
			DmRecordInfo (ApptDB, *indexP, &attr, NULL, NULL);	
			attr |= dmRecAttrSecret;
			DmSetRecordInfo (ApptDB, *indexP, &attr, NULL);
			}
		}
	return (err);
}


/***********************************************************************
 *
 * FUNCTION:    CreateNote
 *
 * DESCRIPTION: This routine adds an empty note to the current record.
 *
 * PARAMETERS:  prompt - if true, ask if an exception should be created.
 *
 * RETURNED:    true if the note was added, false if it was not.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/4/95	Initial Revision
 *       ryw   2/18/00  Added casts to satisfy const cstring checking, should be safe
 *
 ***********************************************************************/
static Boolean CreateNote (Boolean prompt)
{
	Err err;
	FormPtr alert;
	UInt16 alertButton;
	Boolean exception = false;
	Boolean splitEvent = false;
	MemHandle recordH;
	ApptDateTimeType when;
	ApptDBRecordType apptRec;
	ApptDBRecordType newRec;
	ApptDBRecordFlags changedFields;
	
	
	if (CurrentRecord == noRecordSelected) return (false);

	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
	MemHandleUnlock (recordH);

	// If the record already has a note, exit
	if (apptRec.note) return (true);

	// If we're changing a repeating appointment, check if all occurrences
	// are being changed, current & future occurrences are being changed,
	// or if and exception is being created.
	if (apptRec.repeat && prompt)
		{
		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);
		
		if (alertButton == RangeCancelButton)
			return (false);
			
		else if (alertButton == RangeCurrentButton)
			exception = true;
			
		else if (alertButton == RangeFutureButton)
			splitEvent = true;
		}

	if (exception)
		{
		// Add an exception to the current record.
		err = ApptAddException (ApptDB, &CurrentRecord, Date);
		if (err) goto Exit;

		// Create a new record on the current day that contains
		// the same description, time, and alarm setting as the 
		// repeating event.
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);

		when.startTime = apptRec.when->startTime;
		when.endTime = apptRec.when->endTime;
		when.date = Date;

		MemSet (&newRec, sizeof (newRec), 0);
		newRec.when = &when;
		newRec.description = apptRec.description;
		newRec.alarm = apptRec.alarm;
		newRec.note = (char *)"";
		
		err = CreateException (&newRec, &CurrentRecord);				
		MemHandleUnlock (recordH);
		if (err) goto Exit;
		}
	else
		{
		// Clear all changed fields flags.
		MemSet (&changedFields, sizeof (changedFields), 0);
		
		if (splitEvent)
			{
			// Split off the previous occurrences of the event
			err = SplitRepeatingEvent (&CurrentRecord);
			if (err) goto Exit;

			// Set the new start date for the event
			when.date = Date;
			when.startTime = apptRec.when->startTime;
			when.endTime = apptRec.when->endTime;
			newRec.when = &when;
			changedFields.when = true;
			}
		
		// Add the note to the record.
		newRec.note = (char *)"";
		changedFields.note = true;
		err = ApptChangeRecord (ApptDB, &CurrentRecord, &newRec, changedFields);
		if (err) goto Exit;
		}
	
	return (true);
	
Exit:
	FrmAlert (DeviceFullAlert);
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DeleteNote
 *
 * DESCRIPTION: This routine deletes the note field from an 
 *              appointment record.
 *
 * PARAMETERS:  exception  - true if a new record, an exception, should be
 *                           created.
 *					 splitEvent - true if only deleting note from current and 
 *									  future occurrences of the specified event.
 *
 * RETURNED:    true if the note was deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/13/95	Initial Revision
 *
 ***********************************************************************/
static Boolean DeleteNote (Boolean exception, Boolean splitEvent)
{
	Err err;
	MemHandle recordH;
	ApptDBRecordType newRec;
	ApptDBRecordType apptRec;
	ApptDateTimeType when;
	ApptDBRecordFlags changedFields;
	
	
	if (exception)
		{
		// Add an exception to the current record.
		err = ApptAddException (ApptDB, &CurrentRecord, Date);
		if (err) goto Exit;		

		// Create a new record on the current day that contains
		// the same description, time, and alarm settings as the 
		// repeating event, but not the note.
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);

		when.startTime = apptRec.when->startTime;
		when.endTime = apptRec.when->endTime;
		when.date = Date;

		MemSet (&newRec, sizeof (newRec), 0);
		newRec.when = &when;
		newRec.description = apptRec.description;
		newRec.alarm = apptRec.alarm;
		
		err = CreateException (&newRec, &CurrentRecord);
		MemHandleUnlock (recordH);
		if (err) goto Exit;
		}

	else
		{
		// Clear all changed fields flags.
		MemSet (&changedFields, sizeof (changedFields), 0);
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
		
		if (splitEvent)
			{
			// Split off the previous occurrences of the event
			err = SplitRepeatingEvent (&CurrentRecord);
			if (err) goto Exit;

			// Set the new start date for the event
			when.date = Date;
			when.startTime = apptRec.when->startTime;
			when.endTime = apptRec.when->endTime;
			newRec.when = &when;
			changedFields.when = true;
			}

		// Remove the note from the record.
		newRec.note = NULL;
		changedFields.note = true;

		err = ApptChangeRecord (ApptDB, &CurrentRecord, &newRec, changedFields);
		if (err) goto Exit;
		}
	
	return (true);

Exit:
	FrmAlert (DeviceFullAlert);
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    SplitRepeatingEvent
 *
 * DESCRIPTION: This routine splits a repeating appointment into two
 *              repeating appointment.  The orginal record record is 
 *              copied and the end date of the new record is set to 
 *              yesterday or the day before the current date, which ever
 *              earlier.
 *
 * PARAMETERS:  indexP  - passed:   index of records split.
 *                        returned: index of records orginal record.
 *
 * RETURNED:    error code
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/25/96	Initial Revision
 *
 ***********************************************************************/
static Err SplitRepeatingEvent (UInt16* indexP)
	{
	Err					err = 0;
	UInt16 					index;
	UInt32 				uniqueID;
	MemHandle				recordH;
	DateType				endDate;
	DateType				repeatDate;
//	DateTimeType		today;
 	RepeatInfoType		repeat;
	ApptDBRecordType	apptRec;


	// Get yesterday's date or the date of the day before the current date,
	// which ever earlier.
// 	TimSecondsToDateTime (TimGetSeconds(), &today);
//	endDate.year = today.year - firstYear;
//	endDate.month = today.month;
//	endDate.day = today.day;
//	if (DateToInt (endDate) > DateToInt (Date))
	endDate = Date;
	DateAdjust (&endDate, -1);


	ApptGetRecord (ApptDB, *indexP, &apptRec, &recordH);

	// Check for past occurrecnes of the event.
	repeatDate = apptRec.when->date;
	ApptNextRepeat (&apptRec, &repeatDate, true);
	if (DateToDays (repeatDate) <= DateToDays (endDate))
		{
		// Creating the new record may move a current record, so get its 
		// unique id so we can find it after the record is created.
		DmRecordInfo (ApptDB, *indexP, NULL, &uniqueID, NULL); 

		repeat = *apptRec.repeat;
		repeat.repeatEndDate = endDate;
		apptRec.repeat = &repeat;
		
		err = ApptNewRecord (ApptDB, &apptRec, &index);				
		DmFindRecordByID (ApptDB, uniqueID, indexP);
		}

	MemHandleUnlock (recordH);
	
	return (err);
	}


#if WRISTPDA
/***********************************************************************
 *
 * FUNCTION:    MonthlyRepeatHandleEvent
 *
 * DESCRIPTION: This routine is the partial event handler for the Monthly
 *              Repeat form of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         dmc    2/8/03    Initial Revision
 *
 ***********************************************************************/
Boolean MonthlyRepeatHandleEvent(EventType * event);
Boolean MonthlyRepeatHandleEvent(EventType * event)
{
	Boolean handled = false;
	FormPtr frm;
	UInt16  Obj;

	if (event->eType == keyDownEvent) {
		EventType newEvent;
		MemSet( & newEvent, sizeof( EventType ), 0 );
		newEvent.eType = ctlSelectEvent;
		frm = FrmGetActiveForm();
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to a Done button event.
			newEvent.data.ctlSelect.controlID = MonthlyRepeatOk;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm, 
			                                     FrmGetObjectIndex( frm,
			                                       MonthlyRepeatOk ) );
			EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to a Cancel button event.
			newEvent.data.ctlSelect.controlID = MonthlyRepeatCancel;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
			                                     FrmGetObjectIndex( frm,
			                                       MonthlyRepeatCancel ) );
			EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
			return true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelUp ) ||
					( event->data.keyDown.chr == vchrPageUp ) ) {
			// Select first list item on RockerUp/PageUp.
			Obj =  FrmGetObjectIndex( frm, MonthlyRepeatFourthButton );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), true );
			Obj =  FrmGetObjectIndex( frm, MonthlyRepeatLastButton );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), false );
			return true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelDown ) ||
					( event->data.keyDown.chr == vchrPageDown ) ) {
			// Select second list item on RockerDown/PageDown.
			Obj =  FrmGetObjectIndex( frm, MonthlyRepeatFourthButton );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), false );
			Obj =  FrmGetObjectIndex( frm, MonthlyRepeatLastButton );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), true );
			return true;
		}
	}

	return (handled);

}
#endif


/***********************************************************************
 *
 * FUNCTION:    MoveEvent
 *
 * DESCRIPTION:  This routine changes the date and / or time of the 
 *               specified event
 *
 * PARAMETERS:  recordNumP - 
 *              startTime  -
 *              startTime  - 
 *					 date       -
 *					 splitEvent -
 *					 moved      - (returned) true if moved
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	4/01/96		Initial Revision
 *			gap	9/08/99		Add support for moving only current, current & future,
 *									or all occurrences of the event.
 *			gap	9/15/99		Clean up Current/Future/All handling for weekview.
 *
 ***********************************************************************/
 Err MoveEvent (UInt16* recordNumP, TimeType startTime, TimeType endTime,
 		DateType startDate, Boolean timeChangeOnly, Boolean* moved)
	{

	Err					err;
	UInt16				id;
	UInt16				dayOfWeek;
	UInt16				alertButton;
	Int32					adjust;
	FormPtr				alert;
	Boolean				exception 	= false;
	Boolean				splitEvent 	= false;
	Boolean				applyToAll 	= false;
	MemHandle			recordH;
	RepeatInfoType		repeat;
	ApptDBRecordType	newRec;
	ApptDBRecordType	apptRec;
	ApptDateTimeType	when;
	ApptDBRecordFlags	changedFields;
	DateType 			date = startDate;
	Boolean				hasAlarm, isRepeating;

	ApptGetRecord (ApptDB, *recordNumP, &apptRec, &recordH);
	
	hasAlarm = (apptRec.alarm != NULL);
	isRepeating = (apptRec.repeat != NULL);

	*moved = false;
	
	// If we're changing a repeating appointmemt, check if the changes will
	// be applied to:
	//  - all occurrences of the event
	//  - the current and all future occurrences of the event
	//  - only the current event  (create an expection)
	if (isRepeating)
		{
		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);

		// If the alert was canceled don't apply any of the changes.
		if (alertButton == RangeCancelButton)
			{
			MemHandleUnlock(recordH);
			return (0);
			}

		// If the change is to be applied to the selected occurrence only
		// an exception will be created and the specified date and time   
		// will be applied.
		else if (alertButton == RangeCurrentButton)
			exception = true;
			
		// If the change is to be applied to the current and future
		// events, the current record will need to be split, and the
		// specified date and time will be applied.
		else if (alertButton == RangeFutureButton)
			splitEvent = true;

		// If the change is to be applied to the all occurrences of the event
		// the code below will do one of two things.  If the user only changed 
		// the time of the event, the original start date will be maintained and only
		// the time change will be applied.  If the user changes both the time and date,
		// the specified date and time will be applied.
		else if (alertButton == RangeAllButton)
			{
			applyToAll = true;
			
			if (timeChangeOnly)
				date = apptRec.when->date;
			}
		}

	// if there is an alarm associated with the item, remove it from
	// the attention manager queue before changing the event
	if (apptRec.alarm)
		{
		hasAlarm = true;
		DeleteAlarmIfPosted(*recordNumP);
		}
	
	MemHandleUnlock (recordH);


	// Add an exception to the current record,  and create a new record
	// at the new time.
	if (exception)
		{
		err = ApptAddException (ApptDB, recordNumP, Date);
		if (err) goto Exit;
		
		ApptGetRecord (ApptDB, *recordNumP, &apptRec, &recordH);

		when.startTime = startTime;
		when.endTime = endTime;
		when.date = date;

		MemSet (&newRec, sizeof (newRec), 0);
		newRec.description = apptRec.description;
		newRec.note = apptRec.note;
		newRec.when = &when;
		newRec.alarm = apptRec.alarm;

		err = CreateException (&newRec, recordNumP);
		MemHandleUnlock (recordH);
		if (err) goto Exit;
		}

	// Change the time of the current record.
	else	
		{
		MemSet (&changedFields, sizeof (changedFields), 0);
		MemSet (&newRec, sizeof (newRec), 0);

		if (isRepeating)
			{
			if (splitEvent)
				{
				err = SplitRepeatingEvent (recordNumP);
				if (err) goto Exit;
				}

			ApptGetRecord (ApptDB, *recordNumP, &apptRec, &recordH);

			// When changing the entire range of event, reset the start date by calculating
			// the number of days the user moved the event and apply the delta to the event's
			// start date, maintain the duration and clear the exceptions list.
			repeat = *apptRec.repeat;
			newRec.repeat = &repeat;
		
			// Maintain the duration of the event.
			if ((DateToInt (apptRec.repeat->repeatEndDate) != apptNoEndDate) && !(splitEvent))
				{
				adjust = (Int32) DateToDays (date) -
							(Int32) DateToDays (apptRec.when->date);

				DateAdjust (&repeat.repeatEndDate, adjust);

				changedFields.repeat = true;
				}

			// If the repeat type is weekly and the start date of the event
			// has been changed,  update the 'repeat on' field, which contains
			// the days of the week the event repeats on, such that the 
			// event occurs on the start date.
			if (apptRec.repeat->repeatType == repeatWeekly)
				{
				dayOfWeek = DayOfWeek (apptRec.when->date.month, 
					apptRec.when->date.day,
					apptRec.when->date.year+firstYear);
				repeat.repeatOn &= ~(1 << dayOfWeek);
				
				dayOfWeek = DayOfWeek (date.month, 
					date.day,
					date.year+firstYear);
				repeat.repeatOn |= (1 << dayOfWeek);

				changedFields.repeat = true;
				}

			// If the repeat type is monthly by day, get the day of the month (ex:
			// first Friday) of the start date of the event.
			else if (apptRec.repeat->repeatType == repeatMonthlyByDay)
				{
				repeat.repeatOn = DayOfMonth (date.month, date.day, date.year + firstYear);
				changedFields.repeat = true;

				// If we're in the fourth week, and the fourth week is also the last
				// week,  ask the user which week the event repeats in (fourth or last).
				if ( ((repeat.repeatOn / daysInWeek) == 3) &&
						(date.day + daysInWeek > DaysInMonth (date.month, date.year + firstYear)))
					{
					alert = FrmInitForm (MonthlyRepeatDialog);
					
					// Set the 4th / last push button.
					if (apptRec.repeat->repeatOn > dom4thSat)
						id = MonthlyRepeatLastButton;
					else
						id = MonthlyRepeatFourthButton;
					FrmSetControlGroupSelection (alert, MonthlyRepeatWeekGroup,id);

					#if WRISTPDA
					FrmSetEventHandler( alert, MonthlyRepeatHandleEvent );
					#endif

					alertButton = FrmDoDialog (alert);

					if (FrmGetObjectIndex (alert, MonthlyRepeatLastButton) ==
						 FrmGetControlGroupSelection (alert, MonthlyRepeatWeekGroup))
						repeat.repeatOn += daysInWeek;

					FrmDeleteForm (alert);

					if (alertButton == MonthlyRepeatCancel)
						{
						MemHandleUnlock (recordH);
						return (0);
						}
					}
				}

			// If the record is a repeating appointment, and the start date has changed, 
			// then clear the exceptions list.
			if ( !(applyToAll && timeChangeOnly) )
				{
				newRec.exceptions = NULL;
				changedFields.exceptions = true;
				}
				
			MemHandleUnlock (recordH);
			}

		when.date = date;
		when.startTime = startTime;
		when.endTime = endTime;
		newRec.when = &when;
		changedFields.when = true;

		err = ApptChangeRecord (ApptDB, recordNumP, &newRec, changedFields);
		if (err) goto Exit;
		}

	// If the appointment has an alarm, reschedule the next alarm.
	if (hasAlarm)
		RescheduleAlarms (ApptDB);

	*moved = true;
	return (0);
	

Exit:
	FrmAlert (DeviceFullAlert);
	return (err);
	}


/***********************************************************************
 *
 * FUNCTION:    PurgeRecords
 *
 * DESCRIPTION: This routine deletes appointments that are before the 
 *              user specified date.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the current view may need to be redrawn.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/1/95	Initial Revision
 *			gap	8/1/00	add attention manager support
 *
 ***********************************************************************/
static Boolean PurgeRecords (void)
{
	Int32 adjust;
	UInt16 index;
	UInt16 ctlIndex;
	UInt16 numRecord;
	UInt16 buttonHit;
	UInt16 rangeItem;
	ListPtr lst;
	FormPtr alert;
	MemHandle recordH;
	Boolean purge;
	Boolean archive = false;
	DateType purgeDate;
	DateTimeType dateTime;
	ApptDBRecordType apptRec;
	Boolean  hasAlarm;

	// Display an alert to comfirm the operation.
	alert = FrmInitForm (PurgeDialog);

	ctlIndex = FrmGetObjectIndex (alert, PurgeSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);

	buttonHit = FrmDoDialog (alert);

	archive = FrmGetControlValue (alert, ctlIndex);

	lst = FrmGetObjectPtr (alert, FrmGetObjectIndex (alert, PurgeRangeList));
	rangeItem = LstGetSelection (lst);

	FrmDeleteForm (alert);

	if (buttonHit == PurgeCancel)
		return (false);

	SaveBackup = archive;


	// Compute the purge date.
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	
	// One, two, or three weeks
	if (rangeItem < 3)
		adjust = ((rangeItem + 1) * daysInWeek);

	// One month
	else if (dateTime.month > january)
		adjust = DaysInMonth (dateTime.month-1, dateTime.year);
	else
		adjust = DaysInMonth (december, dateTime.year-1);
		
	purgeDate.year = dateTime.year - firstYear;
	purgeDate.month = dateTime.month;
	purgeDate.day = dateTime.day;
	DateAdjust (&purgeDate, -adjust);
	

	// Delete records.
	numRecord = DmNumRecords (ApptDB);
	if (! numRecord) return (false);
	
	for (index = numRecord-1; (Int16) index >= 0; index--)
		{
		recordH = DmGetRecord (ApptDB, index);
		if (recordH == 0) continue;
		
		ApptGetRecord (ApptDB, index, &apptRec, &recordH);
		if (apptRec.repeat)
			purge = (DateToInt(apptRec.repeat->repeatEndDate) <= DateToInt(purgeDate));
		else
			purge = (DateToInt(apptRec.when->date) <= DateToInt(purgeDate));

		hasAlarm = (apptRec.alarm != NULL);	
						
		MemHandleUnlock (recordH);

		DmReleaseRecord (ApptDB, index, false);
		
		if (purge)
			{			
			// if the event to be delete had an alarm, be sure to remove it
			// from the posted alarm queue before the event is deleted.
			if (hasAlarm)
				DeleteAlarmIfPosted(index);
				
			if (archive)
				DmArchiveRecord (ApptDB, index);
			else
				DmDeleteRecord (ApptDB, index);
				
			// Move deleted record to the end of the index so that the 
			// quick sort routine will work.
			DmMoveRecord (ApptDB, index, numRecord);
			}
		}
		
	// Schedule the next alarm.
	RescheduleAlarms (ApptDB);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    DatebookDayOfWeek
 *
 * DESCRIPTION: This routine returns the day-of-the-week, adjusted by the 
 *              preference setting that specifies the first day-of-
 *              the-week.  If the date passed is a Tuesday and the 
 *              start day of week is Monday, this routine will return
 *              a value of one.
 *
 * PARAMETERS:	 month - month (1-12)
 *              day   - day (1-31)
 *              year  - year (1904-2031)
 *
 * RETURNED:	 day of the week (0-first, 1-second)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/27/95	Initial Revision
 *
 ***********************************************************************/
static UInt16 DatebookDayOfWeek (UInt16 month, UInt16 day, UInt16 year)
{
	return ((DayOfWeek (month, day, year) - StartDayOfWeek + daysInWeek)
	 % daysInWeek);
}


/***********************************************************************
 *
 * FUNCTION:    SubstituteStr
 *
 * DESCRIPTION: This routine substitutes the occurrence a token, within
 *              a string, with another string.
 *
 * PARAMETERS:  str    - string containing token string
 *              token  - the string to be replaced
 *              sub    - the string to substitute for the token
 *              subLen - length of the substitute string.
 *
 * RETURNED:    pointer to the string
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/6/95	Initial Revision
 *
 ***********************************************************************/
static Char* SubstituteStr (Char* str, const Char* token, Char* sub, UInt16 subLen)
{
	int charsToMove;
	UInt16 tokenLen;
	UInt16 strLen;
	UInt16 blockSize;
	Char* ptr;
	MemHandle strH;

	// Find the start of the token string, if it doesn't exist, exit.
	ptr = StrStr(str, token);
	if (ptr == NULL) return (str);
	
	tokenLen = StrLen (token);
	charsToMove = subLen - tokenLen;
	
	
	// Resize the string if necessary.
	strH = MemPtrRecoverHandle (str);
	strLen = StrLen (str);
	blockSize = MemHandleSize (strH);
	if (strLen + charsToMove + 1 >= blockSize)
		{
		MemHandleUnlock (strH);
		MemHandleResize (strH, strLen + charsToMove + 1);
		str = MemHandleLock (strH);
		ptr = StrStr (str, token);
		ErrNonFatalDisplayIf(ptr == NULL, "Msg missing token");
		}
	
	// Make room for the substitute string.
	if (charsToMove)
		MemMove (ptr + subLen, ptr + tokenLen, StrLen (ptr + tokenLen)+1);
		
	// Replace the token with the substitute string.
	MemMove (ptr, sub, subLen);
	
	return (str);
}


/***********************************************************************
 *
 * FUNCTION:    SelectFont
 *
 * DESCRIPTION: This routine handles selection of a font in the List 
 *              View. 
 *
 * PARAMETERS:  currFontID - id of current font
 *
 * RETURNED:    id of new font
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/10/97	Initial Revision
 *
 ***********************************************************************/
static FontID SelectFont (FontID currFontID)
{
	UInt16 formID;
	FontID fontID;
	
	formID = (FrmGetFormId (FrmGetActiveForm ()));

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	if (fontID != currFontID)
		FrmUpdateForm (formID, updateFontChanged);

	return (fontID);
}


#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    RepeatSetDateTrigger
 *
 * DESCRIPTION: This routine sets the label of the trigger that displays 
 *              the end date of a repeating appointment. 
 *
 * PARAMETERS:  endDate	- date or -1 if no end date
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of 
 *      the due date trigger is large enough to hold the lagest posible
 *      label.  This label's memory is reserved by initializing the label
 *      in the resource file.     
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatSetDateTrigger (DateType endDate)
{
	Char* label;
	ListPtr lst;
	ControlPtr ctl;

	lst = GetObjectPtr (RepeatEndOnList);
	ctl = GetObjectPtr (RepeatEndOnTrigger);
	label = (Char *)CtlGetLabel (ctl);		// OK to cast; we call CtlSetLabel
	if (DateToInt (endDate) == apptNoEndDate)
		{
		StrCopy (label, LstGetSelectionText (lst, repeatNoEndDateItem));
		LstSetSelection (lst, noEndDateItem);
		}
	else
		{
		// Format the end date into a string.
		DateToDOWDMFormat (endDate.month, 
						 		 endDate.day, 
						 		 endDate.year + firstYear, 
						 		 ShortDateFormat, label);

//		DateToAscii (endDate.month, 
//						 endDate.day, 
//						 endDate.year + firstYear, 
//						 LongDateFormat, label);
		LstSetSelection (lst, repeatChooseDateItem);
		}
	CtlSetLabel (ctl, label);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatGetUIValues
 *
 * DESCRIPTION: This routine gets the current repeat settings of the 
 *              ui gadgets in the repeat dialog box
 *
 *
 * PARAMETERS:  frm     - pointer to the repeat dialog
 *              repeatP - RepeatInfoType structure (fill-in by this routine) 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/10/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatGetUIValues (FormPtr frm, RepeatInfoPtr repeatP)
{
	UInt16 freq;
	UInt16 i;
	UInt16 id;
	UInt16 index;
	Char* str;
	DetailsPtr details;

	// Get the block that contains the details of the current record.
	details = RepeatDetailsP;


	// Get the repeat type.
	index = FrmGetControlGroupSelection (frm, RepeatTypeGroup);
	id = FrmGetObjectId (frm, index);
	if (id == RepeatYearly)
		repeatP->repeatType = repeatYearly;
	else if (id <= RepeatWeekly)
		{
		repeatP->repeatType = (RepeatType) (id - RepeatNone);
		}
	else
		{
		index = FrmGetControlGroupSelection (frm, RepeatByGroup);
		id = FrmGetObjectId (frm, index);
		if (id == RepeatByDayPushButon)
			repeatP->repeatType = repeatMonthlyByDay;
		else
			repeatP->repeatType = repeatMonthlyByDate;
		}
	

	// Get the repeat end date.
	repeatP->repeatEndDate = RepeatEndDate;


	// Get the repeat frequency.
	str = FldGetTextPtr (GetObjectPtr (RepeatFrequenceField));
	if (str) freq = StrAToI (str);
	else freq = 0;
	
	if (freq)
		repeatP->repeatFrequency = freq;
	else
		repeatP->repeatFrequency = 1;


	// Get the start day of week.  If the original repeat type, that is the 
	// repeat type when the dialog was first displayed, is weekly then
	// use the start date in the repeat info (the unedit data), otherwise
	// use the current start of week.
	if (repeatP->repeatType == repeatWeekly)
		{
		if (details->repeat.repeatType == repeatWeekly)
			repeatP->repeatStartOfWeek = details->repeat.repeatStartOfWeek;
		else		
			repeatP->repeatStartOfWeek = StartDayOfWeek;
		}

	// For all other repeat types, the repeatStartOfWeek field is meaningless.
	else
		repeatP->repeatStartOfWeek = 0;


	// If the repeat type is weekly, get the day of the week the event
	// repeats on. 
	if (repeatP->repeatType == repeatWeekly)
		{
		repeatP->repeatOn = 0;
		index = FrmGetObjectIndex (frm, RepeatDayOfWeek1PushButton);
		for (i = 0; i < daysInWeek ; i++)
			{
			if (FrmGetControlValue (frm, index +
								((i - RepeatStartOfWeek + daysInWeek) % daysInWeek)))
				repeatP->repeatOn |= (1 << i);
			}
		}

	// If the repeat type is monthly by day, get the day of the month (ex:
	// fisrt Friday) of the start date of the event.
	else if (repeatP->repeatType == repeatMonthlyByDay)
		{
		if (details->repeat.repeatType == repeatMonthlyByDay)
			repeatP->repeatOn = details->repeat.repeatOn;
		else
			repeatP->repeatOn = DayOfMonth (details->when.date.month,
				details->when.date.day,
				details->when.date.year + firstYear);
		}

	// For all other repeat types, the repeatOn field is meaningless.
	else
		{
		repeatP->repeatOn = 0;
		}
}


/***********************************************************************
 *
 * FUNCTION:    RepeatSetUIValues
 *
 * DESCRIPTION: This routine sets the current repeat settings of the 
 *              ui gadgets in the repeat dialog box
 *
 * PARAMETERS:  frm     - pointer to the Repeat Dialog
 *              repeatP - pointer to a RepeatInfoType structure.
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		07/10/95	art	Created by Art Lamb
 *		08/03/99	kwk	Copy label ptrs when shifting day-of-week pushbutton titles,
 *							not just the first byte of each label.
 *
 ***********************************************************************/
static void RepeatSetUIValues (FormPtr frm, RepeatInfoPtr repeatP)
{
	UInt16			i;
	UInt16			id;
	UInt16			oldFreq;
	MemHandle		freqH;
	Char*				freqP;
	Boolean			on;
	FieldPtr			fld;

	// Set the selection of the "repeat type" push button group.
	id = repeatP->repeatType + RepeatNone;
	if (repeatP->repeatType > repeatMonthlyByDay)
		id--;
	FrmSetControlGroupSelection (frm, RepeatTypeGroup, id);
	

	// Set the frequency field
	if (repeatP->repeatType != repeatNone)	
		{
		fld = GetObjectPtr (RepeatFrequenceField);
		freqH = FldGetTextHandle (fld);
		if (freqH) 
			{
			freqP = MemHandleLock (freqH);
			oldFreq = StrAToI (freqP);
			}
		else
			{
			freqH = MemHandleNew (maxFrequenceFieldLen);
			freqP = MemHandleLock (freqH);
			oldFreq = 0;
			}

		if (oldFreq != repeatP->repeatFrequency)
			{
			StrIToA (freqP, repeatP->repeatFrequency);
			FldSetTextHandle (fld, freqH);
			if (FrmVisible (FrmGetActiveForm ()))
				{
				FldEraseField (fld);
				FldDrawField (fld);
				}
			}
		MemHandleUnlock (freqH);
		}	


	// Set the selection of the "repeat on" push button groups.
	if (repeatP->repeatType == repeatWeekly)
		{
		// If the appointment has a different start-day-of-week than
		// the dialog-box's current start-day-of-week, rearrange the
		//	labels on the days-of-week push buttons.
		//	Note that this will only handle button-label shifts of one day.
		if (StartDayOfWeek != RepeatStartOfWeek)
			{
			const Char* sundayLabel = CtlGetLabel (GetObjectPtr (RepeatDayOfWeek1PushButton));
			for (id = RepeatDayOfWeek1PushButton; id < RepeatDayOfWeek7PushButton; id++)
				{
				CtlSetLabel(GetObjectPtr(id), CtlGetLabel(GetObjectPtr(id + 1)));
				}
			CtlSetLabel(GetObjectPtr(RepeatDayOfWeek7PushButton), sundayLabel);
			RepeatStartOfWeek = StartDayOfWeek;
			}
		
		// Turn on the push buttons for the days the appointment repeats on.
		for (i = 0; i < daysInWeek; i++)
			{
			on = ((repeatP->repeatOn & (1 << i) ) != 0);
			id = RepeatDayOfWeek1PushButton + 
				((i - RepeatStartOfWeek + daysInWeek) % daysInWeek);
			CtlSetValue (GetObjectPtr (id), on);
			}
		}


	// Set the selection of the "repeat by" push button groups.
	if (repeatP->repeatType == repeatMonthlyByDate)	
		FrmSetControlGroupSelection (frm, RepeatByGroup, RepeatByDatePushButon);
	else
		FrmSetControlGroupSelection (frm, RepeatByGroup, RepeatByDayPushButon);
	

	// Set the "end on" trigger label and popup list selection.
	if (repeatP->repeatType != repeatNone)	
		{
		RepeatSetDateTrigger (repeatP->repeatEndDate);
		}
}


/***********************************************************************
 *
 * FUNCTION:    RepeatDrawDescription
 *
 * DESCRIPTION: This routine draws the text description of the current
 *              repeat type and frequency.
 *
 *              The description is created from a template string.  The 
 *              repeat type and frequency determines which template is 
 *              used.  The template may contain one or more of the 
 *              following token:
 *                   ^d - day name (ex: Monday)
 *							^f - frequency
 *                   ^x - day of the month ordinal number (ex: 1st - 31th)
 *                   ^m - month name (ex: July)
 *                   ^w - week ordinal number (1st, 2nd, 3rd, 4th, or last)
 *
 * PARAMETERS:  frm - pointer to the repeat dialog box
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		07/06/95	art	Created by Art Lam
 *		03/02/99	grant	Only do week ordinal substitution in the repeatMonthlyByDay case
 *		08/04/99	kwk	Use explicit string resources for days/months/years versus
 *							borrowing from system, so we can localize properly.
 *		10/21/99	gap	Remove rectangle drawing from proc.  Now a gadget in form.
 *		11/05/99	gap	Always use full day of week name when repeat.repeatType is repeatMonthlyByDay.
 *
 ***********************************************************************/
static void RepeatDrawDescription (FormPtr frm)
{
	UInt8 repeatOn;
	UInt16 i;
	UInt16 len;
	UInt16 freq;
	UInt16 dayOfWeek;
	UInt16 templateId;
	UInt16 repeatOnCount = 0;
	Char* descP;
	Char* resP;
	Char* saveResP;
	MemHandle descH;
	MemHandle resH;
	FieldPtr fld;
	DetailsPtr details;
	RepeatInfoType repeat;
	
	// Get the block that contains the details of the current record.
	details = RepeatDetailsP;

	fld = GetObjectPtr (RepeatDescField);
	FldEraseField (fld);
		
	// Get the current setting of the repeat ui gadgets.
	RepeatGetUIValues (frm, &repeat);

	// Determine which template string to use.  The template string is 
	// used to format the description string. Note that we could add
	// a soft constant which tells us whether we need to use different
	// strings for freq == 1 case (depends on language), thus saving space.
	freq = repeat.repeatFrequency;
	switch (repeat.repeatType)
		{
		case repeatNone:
			templateId = repeatNoneStrID;
			break;

		case repeatDaily:
			if (freq == 1)
				// "Every day"
				templateId = everyDayRepeatStrID;
			else
				// "Every [other | 2nd | 3rd...] day"
				templateId = dailyRepeatStrID;
			break;

		case repeatWeekly:
			if (freq == 1)
				// "Every week on [days of week]"
				templateId = everyWeekRepeat1DayStrID;
			else
				templateId = weeklyRepeat1DayStrID;
			
			// Generate offset to appropriate string id,
			// based on # of days that we need to append.
			for (i = 0; i < daysInWeek; i++)
				{
				if (repeat.repeatOn & (1 << i) ) repeatOnCount++;
				}
			templateId += (repeatOnCount - 1);
			break;
			
		case repeatMonthlyByDate: 
			if (freq == 1)
				// "The ^w ^d of every month"
				templateId = everyMonthByDateRepeatStrID;
			else
				templateId = monthtlyByDateRepeatStrID;
			break;

		case repeatMonthlyByDay:
			if (freq == 1)
				templateId = everyMonthByDayRepeatStrID;
			else
				templateId = monthtlyByDayRepeatStrID;
			break;

		case repeatYearly:
			if (freq == 1)
				templateId = everyYearRepeatStrID;
			else
				templateId = yearlyRepeatStrID;
			break;

		default:
			ErrNonFatalDisplay("Unknown repeat type");
			break;
		}

	// Allocate a block to hold the description and copy the template 
	// string into it.
	resH = DmGetResource (strRsc, templateId);
	resP = MemHandleLock (resH);
	descH = MemHandleNew (MemPtrSize(resP));
	descP = MemHandleLock (descH);
	StrCopy (descP, resP);
	MemHandleUnlock (resH);
	

	// Substitute the month name string for the month name token.
	resH = DmGetResource (strRsc, repeatMonthNamesStrID);
	resP = MemHandleLock (resH);
	for (i = 1; i < details->when.date.month; i++)
		resP = StrChr (resP, spaceChr) + 1;
	len = (UInt16) (StrChr (resP, spaceChr) - resP);
	descP = SubstituteStr (descP, monthNameToken, resP, len);
	MemHandleUnlock (resH);


	// Substitute the day name string for the day name token.
	if ( (repeatOnCount == 1)  || (repeat.repeatType == repeatMonthlyByDay) )
		templateId = repeatFullDOWNamesStrID;
	else
		templateId = repeatShortDOWNamesStrID;
	
	resH = DmGetResource (strRsc, templateId);
	resP = MemHandleLock (resH);
	if (repeat.repeatType == repeatWeekly)
		{
		dayOfWeek = repeat.repeatStartOfWeek;
		repeatOn = repeat.repeatOn;
		saveResP = resP;
		while (StrStr (descP, dayNameToken))
			{
			for (i = 0; i < daysInWeek; i++)
				{
				if (repeatOn & (1 << dayOfWeek) )
					{
					repeatOn &= ~(1 << dayOfWeek);
					break;
					}
				dayOfWeek = (dayOfWeek + 1 + daysInWeek) % daysInWeek;
				}
			resP = saveResP;
			for (i = 0; i < dayOfWeek; i++)
				resP = StrChr (resP, spaceChr) + 1;
			
			len = (UInt16) (StrChr (resP, spaceChr) - resP);
			descP = SubstituteStr (descP, dayNameToken, resP, len);
			}
		}
	else
		{
		dayOfWeek = DayOfWeek (details->when.date.month, details->when.date.day, 
			details->when.date.year + firstYear);
		for (i = 0; i < dayOfWeek; i++)
			resP = StrChr (resP, spaceChr) + 1;
		len = (UInt16) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, dayNameToken, resP, len);
		}
	MemHandleUnlock (resH);


	// Substitute the repeat frequency string for the frequency token. Note that
	// we do something special for 2nd (other), since the gender of 'other' changes
	// for some languages, depending on whether the next word is day, month, week,
	// or year.
	if (freq == 2)
		{
		Char otherFreqName[16];
		UInt16 index = (UInt16)repeat.repeatType - (UInt16)repeatNone;
		SysStringByIndex(freqOrdinal2ndStrlID, index, otherFreqName, sizeof(otherFreqName));
		descP = SubstituteStr (descP, frequenceToken, otherFreqName, StrLen(otherFreqName));
		}
	else
		{
		resH = DmGetResource (strRsc, freqOrdinalsStrID);
		resP = MemHandleLock (resH);
		for (i = 1; i < freq; i++)
			resP = StrChr (resP, spaceChr) + 1;
		len = (UInt16) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, frequenceToken, resP, len);
		MemHandleUnlock (resH);
		}


	// Substitute the repeat week string (1st, 2nd, 3rd, 4th, or last)
	// for the week ordinal token.
	if (repeat.repeatType == repeatMonthlyByDay)
		{
		resH = DmGetResource (strRsc, weekOrdinalsStrID);
		resP = MemHandleLock (resH);
		for (i = 0; i < repeat.repeatOn / daysInWeek; i++)
			resP = StrChr (resP, spaceChr) + 1;
		len = (UInt16) (StrChr (resP, spaceChr) - resP);
		descP = SubstituteStr (descP, weekOrdinalToken, resP, len);
		MemHandleUnlock (resH);
		}
	else
		{
		// make sure the week ordinal token really doesn't appear
		ErrNonFatalDisplayIf(StrStr(descP, weekOrdinalToken) != NULL, "week ordinal not substituted");
		}


	// Substitute the repeat date string (1st, 2nd, ..., 31th) for the
	// day ordinal token.
	resH = DmGetResource (strRsc, dayOrdinalsStrID);
	resP = MemHandleLock (resH);
	for (i = 1; i < details->when.date.day; i++)
		resP = StrChr (resP, spaceChr) + 1;
	len = (UInt16) (StrChr (resP, spaceChr) - resP);
	descP = SubstituteStr (descP, dayOrdinalToken, resP, len);
	MemHandleUnlock (resH);
	
	// Draw the description.
	MemHandleUnlock (descH);
	FldFreeMemory (fld);
	FldSetTextHandle (fld, descH);
	FldDrawField (fld);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatSelectEndDate
 *
 * DESCRIPTION: This routine selects the end date of a repeating event.
 *
 * PARAMETERS:  event - pointer to a popup select event
 *
 * RETURNED:    nothing
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatSelectEndDate (EventType* event)
{
	Int16 month, day, year;
	Char* titleP;
	MemHandle titleH;
	DetailsPtr details;

	// "No due date" items selected?
	if (event->data.popSelect.selection == repeatNoEndDateItem)
		{
		//DateToInt (RepeatEndDate) = apptNoEndDate;
		RepeatEndDate.year = 0x7f;
		RepeatEndDate.month = 0x0f;
		RepeatEndDate.day = 0x1f;
		}

	// "Select date" item selected?
	else if (event->data.popSelect.selection == repeatChooseDateItem)
		{
		details = RepeatDetailsP;

		if ( DateToInt (RepeatEndDate) == apptNoEndDate)
			{
			year = details->when.date.year + firstYear;
			month = details->when.date.month;
			day = details->when.date.day;
			}
		else
			{
			year = RepeatEndDate.year + firstYear;
			month = RepeatEndDate.month;
			day = RepeatEndDate.day;
			}

		titleH = DmGetResource (strRsc, endDateTitleStrID);
		titleP = (Char*) MemHandleLock (titleH);

		if (SelectDay (selectDayByDay, &month, &day, &year, titleP))
			{
			RepeatEndDate.day = day;
			RepeatEndDate.month = month;
			RepeatEndDate.year = year - firstYear;
			
			// Make sure the end date is not before the start date.
			if (DateToInt(RepeatEndDate) < DateToInt (details->when.date))
				{
				SndPlaySystemSound (sndError);
				//DateToInt (RepeatEndDate) = apptNoEndDate;
				RepeatEndDate.year = 0x7f;
				RepeatEndDate.month = 0x0f;
				RepeatEndDate.day = 0x1f;
				}
			}

		MemHandleUnlock (titleH);
		}

	RepeatSetDateTrigger (RepeatEndDate);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatChangeRepeatOn
 *
 * DESCRIPTION: This routine is called when one of the weekly "repeat on" 
 *              push button is pushed.  This routine checks 
 *              if all the buttons has been turned off,  if so the day
 *              of the week of the appointment's start date is turn on.
 *
 * PARAMETERS:  event - pointer to and event
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/7/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatChangeRepeatOn (EventType* event)
{
#pragma unused (event)

	UInt16 i;
	UInt16 id;
	UInt16 index;
	UInt16 dayOfWeek;
	FormPtr frm;
	Boolean on;
	DetailsPtr details;

	// Get the block that contains the details of the current record.
	details = RepeatDetailsP;

	frm = FrmGetActiveForm ();

	// Check if any of the buttons are on.
	index = FrmGetObjectIndex (frm, RepeatDayOfWeek1PushButton);
	on = false;
	for (i = 0; i < daysInWeek; i++)
		{
		if (FrmGetControlValue (frm, index + i) != 0)
			{
			on = true;
			break;
			}
		}

	// If all the buttons are off, turn on the start date's button.
	if (! on)
		{
		dayOfWeek = DayOfWeek (details->when.date.month,
			details->when.date.day,
			details->when.date.year+firstYear);
		dayOfWeek = (( dayOfWeek  - 
			details->repeat.repeatStartOfWeek + daysInWeek) % daysInWeek);

		id = RepeatDayOfWeek1PushButton + dayOfWeek;
		CtlSetValue (GetObjectPtr (id), true);
		}

	// Update the display of the repeat descrition.
	RepeatDrawDescription (frm);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatChangeType
 *
 * DESCRIPTION: This routine changes the ui gadgets in the repeat dialog
 *              such that they match the newly selected repeat type.  The 
 *              routine is called when one of the repeat type push buttons
 *              are pushed.
 *
 * PARAMETERS:  event - pointer to and event
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/07/95	Initial Revision
 *			gap	10/28/99	End on value maintained last selected date
 *								after switching repeat type instead of returning to 
 *								"no end date" also reset RepeatEndDate global such that
 * 							if end date is "no end date" and "choose" is selected from
 *								popup - calendar always defaults to current day.
 *
 ***********************************************************************/
static void RepeatChangeType (EventType* event)
{
	UInt16 id;
	UInt16 dayOfWeek;
	FormPtr frm;
	DetailsPtr details;
	RepeatType oldType;
	RepeatType newType;
	RepeatInfoType repeat;
	
	// If the type if monthly default to monthly-by-date.
	newType = (RepeatType) (event->data.ctlSelect.controlID - RepeatNone);
	if (newType > repeatWeekly) newType++;

	oldType = RepeatingEventType;
	if (oldType == newType) return;
	
	frm = FrmGetActiveForm ();
	
	// Get the block that contains the details of the current record.
	details = RepeatDetailsP;

	// Initialize the ui gadgets.
	if (newType == details->repeat.repeatType)
		{
		RepeatSetUIValues (frm, &details->repeat);
		
		// if reselecting current repeat type, reset RepeatEndDate global
		// to current date so if user attemps to "choose" a new day, the
		// default matches date displayed as opposed to last date picked 
		// last in choose form
		RepeatEndDate = details->repeat.repeatEndDate;
		}
	else
		{
		repeat.repeatType = newType;
		
		// when switching to a repeat type different from the current
		// setting, always start user with default end date and frequency
		// settings.
		//DateToInt(repeat.repeatEndDate) = defaultRepeatEndDate;
		repeat.repeatEndDate.year = 0x7f;
		repeat.repeatEndDate.month = 0x0f;
		repeat.repeatEndDate.day = 0x1f;
		//DateToInt(RepeatEndDate) = defaultRepeatEndDate;
		RepeatEndDate.year = 0x7f;
		RepeatEndDate.month = 0x0f;
		RepeatEndDate.day = 0x1f;
		repeat.repeatFrequency = defaultRepeatFrequency;
		
		repeat.repeatStartOfWeek = StartDayOfWeek;
		
		if (newType == repeatWeekly)
			{
			dayOfWeek = DayOfWeek (details->when.date.month, 
				details->when.date.day,
				details->when.date.year+firstYear);
			repeat.repeatOn = (1 << dayOfWeek);
			}
		else if (newType == repeatMonthlyByDay)
			{
			repeat.repeatOn = DayOfMonth (details->when.date.month, 
				details->when.date.day,
				details->when.date.year + firstYear);
			}
		else
			{
			repeat.repeatOn = 0;
			}
		RepeatSetUIValues (frm, &repeat);
		}
	

	// Hide the ui gadgets that are unique to the repeat type we are 
	// no longer editing.
	switch (oldType)
		{
		case repeatNone:
			HideObject (frm, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			HideObject (frm, RepeatDaysLabel);
			break;
		
		case repeatWeekly:
			HideObject (frm, RepeatWeeksLabel);
			for (id = RepeatRepeatOnLabel; id <= RepeatDayOfWeek7PushButton; id++)
				HideObject (frm, id);
			break;

		case repeatMonthlyByDay:
		case repeatMonthlyByDate:
			HideObject (frm, RepeatMonthsLabel);
			for (id = RepeatByLabel; id <= RepeatByDatePushButon; id++)
				HideObject (frm, id);
			break;

		case repeatYearly:
			HideObject (frm, RepeatYearsLabel);
			break;
		}


	// Handle switching to or from "no" repeat.
	if (oldType == repeatNone)
		{
		ShowObject (frm, RepeatEveryLabel);
		ShowObject (frm, RepeatFrequenceField);
		ShowObject (frm, RepeatEndOnLabel);
		ShowObject (frm, RepeatEndOnTrigger);
		}
	else if (newType == repeatNone)
		{
		HideObject (frm, RepeatEveryLabel);
		HideObject (frm, RepeatFrequenceField);
		HideObject (frm, RepeatEndOnLabel);
		HideObject (frm, RepeatEndOnTrigger);
		}



	// Show the ui object that are appropriate for the new repeat
	// type.
	switch (newType)
		{
		case repeatNone:
			ShowObject (frm, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			ShowObject (frm, RepeatDaysLabel);
			break;

		case repeatWeekly:
			ShowObject (frm, RepeatWeeksLabel);
			ShowObject (frm, RepeatRepeatOnLabel);
			for (id = RepeatRepeatOnLabel; id <= RepeatDayOfWeek7PushButton; id++)
				ShowObject (frm, id);
			break;
			
		case repeatMonthlyByDay: 
		case repeatMonthlyByDate:
			ShowObject (frm, RepeatMonthsLabel);
			ShowObject (frm, RepeatByLabel);
			ShowObject (frm, RepeatByDayPushButon);
			ShowObject (frm, RepeatByDatePushButon);
			break;

		case repeatYearly:
			ShowObject (frm, RepeatYearsLabel);
			break;
		}	

	RepeatingEventType = newType;
	
	// Update the display of the repeat descrition.
	RepeatDrawDescription (frm);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatApply
 *
 * DESCRIPTION: This routine applies the changes made in the Repeat Dialog.
 *              The changes or copied to a block intiialize by the 
 *              Details Dialog,  they a not written to the database until
 *              the Details Dialog is applied.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/24/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatApply (void)
{
	DetailsPtr		details;
	RepeatInfoType repeat;


	// Get the block that contains the details of the current record,
	// this block was initialized by the details dialog initializtion
	// routine.
	details = RepeatDetailsP;
	
	RepeatGetUIValues (FrmGetActiveForm (), &repeat);
	details->repeat = repeat;
}


/***********************************************************************
 *
 * FUNCTION:    RepeatDescRectHandler
 *
 * DESCRIPTION: This routine is the event handler for rectangle gadget
 *					 surrounding the repeat description in the "Repeat
 *              Dialog Box".
 *
 *					 Instead of drawing a static rectangle the size of the 
 *					 gadget bounds, I have sized the gadget to be the full area
 *					 of the repeat dialog the description field will still fit
 *					 should the field have to be slightly resized for any reason.
 *					 The bounding rect drawn is calculated from the field's 
 *					 bounding rect.
 *
 * PARAMETERS:  gadgetP	- pointer to the gadget
 *					 cmd		- the event type to be handled
 *					 paramp	- any additional data that is passed to the gadget
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/21/99	Initial Revision
 *
 ***********************************************************************/

static Boolean RepeatDescRectHandler(FormGadgetTypeInCallback *gadgetP, UInt16 cmd, void *paramP)
{	
	FieldPtr 		fld;
   RectangleType	r;
	
	switch(cmd)
	{
		case formGadgetEraseCmd:
		case formGadgetDrawCmd:
		
			// get the repeat description field and calculate a bounding box
			fld = GetObjectPtr (RepeatDescField);
			FldGetBounds (fld, &r);
			RctInsetRectangle (&r, -4);
			
			if (cmd == formGadgetDrawCmd)
				WinDrawRectangleFrame (simpleFrame, &r);
			else
				WinEraseRectangle (&r, 0);
			break;
		
		case formGadgetHandleEventCmd:
			return false;
			break;
			
		default:
			break;
	}
	
	return true;
}


/***********************************************************************
 *
 * FUNCTION:    RepeatInit
 *
 * DESCRIPTION: This routine initializes tthe "Repeat Dialog Box".  All
 *              the object in the dialog are initialize, even if they
 *              are not used given the current repeat type.
 *
 * PARAMETERS:  frm - pointer to the repeat dialog box
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/4/95	Initial Revision
 *
 ***********************************************************************/
static void RepeatInit (FormPtr frm)
{
	UInt16				id;
	DetailsPtr		details;


	// Get the block that contains the details of the current record,
	// this block was initialized by the details dialog initializtion
	// routine.
	details = RepeatDetailsP;
	

	// Set usable the ui object that are appropriate for the given repeat
	// type.
	switch (details->repeat.repeatType)
		{
		case repeatNone:
			ShowObject (frm, RepeatNoRepeatLabel);
			break;

		case repeatDaily:
			ShowObject (frm, RepeatEveryLabel);
			ShowObject (frm, RepeatFrequenceField);
			ShowObject (frm, RepeatDaysLabel);
			ShowObject (frm, RepeatEndOnLabel);
			ShowObject (frm, RepeatEndOnTrigger);
			break;

		case repeatWeekly:
			ShowObject (frm, RepeatEveryLabel);
			ShowObject (frm, RepeatFrequenceField);
			ShowObject (frm, RepeatWeeksLabel);
			ShowObject (frm, RepeatRepeatOnLabel);
			ShowObject (frm, RepeatEndOnLabel);
			ShowObject (frm, RepeatEndOnTrigger);
			for (id = RepeatDayOfWeek1PushButton; id <= RepeatDayOfWeek7PushButton; id++)
				ShowObject (frm, id);
			break;
			
		case repeatMonthlyByDay: 
		case repeatMonthlyByDate:
			ShowObject (frm, RepeatEveryLabel);
			ShowObject (frm, RepeatFrequenceField);
			ShowObject (frm, RepeatMonthsLabel);
			ShowObject (frm, RepeatByLabel);
			ShowObject (frm, RepeatByDayPushButon);
			ShowObject (frm, RepeatByDatePushButon);
			ShowObject (frm, RepeatEndOnLabel);
			ShowObject (frm, RepeatEndOnTrigger);
			break;

		case repeatYearly:
			ShowObject (frm, RepeatEveryLabel);
			ShowObject (frm, RepeatFrequenceField);
			ShowObject (frm, RepeatYearsLabel);
			ShowObject (frm, RepeatEndOnLabel);
			ShowObject (frm, RepeatEndOnTrigger);
			break;
		}

	FrmSetGadgetHandler(frm, FrmGetObjectIndex(frm, RepeatDescRectGadget), RepeatDescRectHandler);

	RepeatStartOfWeek = sunday;	
	RepeatEndDate = details->repeat.repeatEndDate;
	RepeatingEventType = details->repeat.repeatType;
	
	RepeatSetUIValues (frm, &details->repeat);
}


/***********************************************************************
 *
 * FUNCTION:    RepeatHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Repeat
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			gap	10/15/99	Added added handling of frmUpdateEvent
 *			gap	11/01/00	Update the description if the user changes the
 *								repeat every field via the silkscreen keyboard.
 *			gap	01/09/01 Remove double update caused by edit of a selection
 *								of text in the "Repeat Every" field.
 *
 ***********************************************************************/
Boolean RepeatHandleEvent (EventType* event)
{
	FormPtr frm;
	Boolean handled = false;

	#if WRISTPDA
	EventType newEvent;
	RepeatInfoType repeat;
	UInt16 controlId;
	#endif

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case RepeatOkButton:
				RepeatApply ();
				
				// Give control of the details info to the Details form
				DetailsP = RepeatDetailsP;
				RepeatDetailsP = 0;
				
				FrmGotoForm (DetailsDialog);
				handled = true;
				break;

			case RepeatCancelButton:
				// Give control of the details info to the Details form
				DetailsP = RepeatDetailsP;
				RepeatDetailsP = 0;
				
				FrmGotoForm (DetailsDialog);
				handled = true;
				break;				

			case RepeatNone:
			case RepeatDaily:
			case RepeatWeekly:
			case RepeatMonthly:
			case RepeatYearly:
				RepeatChangeType (event);
				handled = true;
				break;				

			case RepeatDayOfWeek1PushButton:
			case RepeatDayOfWeek2PushButton:
			case RepeatDayOfWeek3PushButton:
			case RepeatDayOfWeek4PushButton:
			case RepeatDayOfWeek5PushButton:
			case RepeatDayOfWeek6PushButton:
			case RepeatDayOfWeek7PushButton:
				RepeatChangeRepeatOn (event);
				handled = true;
				break;				

			case RepeatByDayPushButon:
			case RepeatByDatePushButon:
				RepeatDrawDescription (FrmGetActiveForm());
				handled = true;
				break;

			#if WRISTPDA
			case RepeatIncreaseType:
				frm = FrmGetActiveForm ();
				RepeatGetUIValues(frm, & repeat );
				controlId = RepeatNone + repeat.repeatType;
				if ( ( repeat.repeatType == repeatMonthlyByDay  ) || 
					 ( repeat.repeatType == repeatMonthlyByDate ) )
					controlId = RepeatMonthly;
				else if ( repeat.repeatType == repeatYearly )
					controlId = RepeatYearly;
				controlId++;
				if ( controlId <= RepeatYearly ) {
					MemSet( & newEvent, sizeof( EventType ), 0 );
					newEvent.eType = ctlSelectEvent;
					newEvent.data.ctlSelect.controlID = controlId;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
				}
				handled = true;
				break;
			case RepeatDecreaseType:
				frm = FrmGetActiveForm ();
				RepeatGetUIValues(frm, & repeat );
				controlId = RepeatNone + repeat.repeatType;
				if ( ( repeat.repeatType == repeatMonthlyByDay  ) || 
					 ( repeat.repeatType == repeatMonthlyByDate ) )
					controlId = RepeatMonthly;
				else if ( repeat.repeatType == repeatYearly )
					controlId = RepeatYearly;
				controlId--;
				if ( controlId >= RepeatNone ) {
					MemSet( & newEvent, sizeof( EventType ), 0 );
					newEvent.eType = ctlSelectEvent;
					newEvent.data.ctlSelect.controlID = controlId;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
				}
				handled = true;
				break;
			#endif
			}
		}

	#if WRISTPDA
	else if (event->eType == keyDownEvent) {
		frm = FrmGetActiveForm ();
		MemSet( & newEvent, sizeof( EventType ), 0 );
		newEvent.eType = ctlSelectEvent;
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to a Done button event.
			newEvent.data.ctlSelect.controlID = RepeatOkButton;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
			handled = true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to a Cancel button event.
			newEvent.data.ctlSelect.controlID = RepeatCancelButton;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
			handled = true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelUp ) ||
					( event->data.keyDown.chr == vchrPageUp ) ) {
			// Decrease the repeat type (i.e. move left), if possible.
			newEvent.data.ctlSelect.controlID = RepeatDecreaseType;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000005, true );
			handled = true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelDown ) ||
					( event->data.keyDown.chr == vchrPageDown ) ) {
			// Increase the repeat type (i.e. move right), if possible.
			newEvent.data.ctlSelect.controlID = RepeatIncreaseType;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000006, true );
			handled = true;
		}
	}
	#endif
	else if	(	(event->eType == keyDownEvent)
				&&	(!TxtCharIsHardKey(	event->data.keyDown.modifiers,
												event->data.keyDown.chr))
				&& (!EvtKeydownIsVirtual(event)))
		{
		WChar chr = event->data.keyDown.chr;
		if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
			{
			Boolean noSelection;
			UInt16 startPos, endPos;
			
			FieldPtr fld = GetObjectPtr (RepeatFrequenceField);
			FldGetSelection(fld, &startPos, &endPos);
			noSelection = (startPos == endPos);
			FldHandleEvent (fld, event);
			
			// There are 3 ways that the repeat every value can be 
			// updated
			//		1) by the soft keyboard (on silkscreen)
			//		2) via graffiti entry in the form with a selection range
			// 	3) via graffiti entry in the form with no selection range
			//			(ie just an insertion cursor)
			// Methods 1 & 2 result in a fldChangedEvent being posted so the
			// update will be handled in response to that event.  ONLY when 
			// there is no selection range replacement, do we do the update here 
			// to avoid a double redraw of the description field.
			if (noSelection)
				RepeatDrawDescription (FrmGetActiveForm());
			}
		handled = true;
		}
		
	else if (event->eType == fldChangedEvent)
		{
		// when the user changes the "repeat every" value via the 
		// soft keyboard or in the form via a selection replacement
		// a fldChangedEvent is posted. Update the description in
		// response
		RepeatDrawDescription (FrmGetActiveForm());
		handled = true;
		}
		
	else if (event->eType == popSelectEvent)
		{
		if (event->data.popSelect.listID == RepeatEndOnList)
			{
			RepeatSelectEndDate (event);			
			handled = true;
			}
		}

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		RepeatInit (frm);
		FrmDrawForm (frm);
		FrmSetFocus (frm, FrmGetObjectIndex (frm, RepeatFrequenceField));
		RepeatDrawDescription (frm);
		handled = true;
		}

	else if (event->eType == frmUpdateEvent)
		{
		frm = FrmGetActiveForm ();
		FrmDrawForm (frm);
		handled = true;
		}
	
	else if (event->eType == frmCloseEvent)
		{
		if (RepeatDetailsP)
			{
			MemPtrFree(RepeatDetailsP);
			RepeatDetailsP = 0;
			}
		}

	return (handled);
}


#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    DetailsAlarmOnOff
 *
 * DESCRIPTION: This routine shows or hides the alarm advance ui object.
 *              It is called when the alarm checkbox is turn on or off.
 *
 * PARAMETERS:  on - true to show object, false to hide
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/21/95	Initial Revision
 *
 ***********************************************************************/
 static void DetailsAlarmOnOff (Boolean on)
 {
 	UInt16 advance;
 	UInt16 advanceUnit;
 	Char* textP;
 	Char* label;
 	MemHandle textH;
 	FormPtr frm;
 	ListPtr lst;
 	FieldPtr fld;
 	ControlPtr ctl;
 
	frm = FrmGetActiveForm ();
	fld = GetObjectPtr (DetailsAlarmAdvanceField);

 	if (on)
 		{
		if (AlarmPreset.advance != apptNoAlarm)
			{
			advance = AlarmPreset.advance;
			advanceUnit = AlarmPreset.advanceUnit;
			}
		else
			{
			advance = defaultAlarmAdvance;
			advanceUnit = defaultAdvanceUnit;
			}
		

		// Set the value of the alarm advance field.
		textH = FldGetTextHandle (fld);
		if (textH) MemHandleFree (textH);
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, advance);
		MemPtrUnlock (textP);

		FldSetTextHandle (fld, textH);
		
		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = GetObjectPtr (DetailsAlarmAdvanceList);		
		LstSetSelection (lst, advanceUnit);
		label = LstGetSelectionText (lst, advanceUnit);

		ctl = GetObjectPtr (DetailsAlarmAdvanceSelector);
		CtlSetLabel (ctl, label);
		ShowObject (frm, DetailsAlarmAdvanceSelector);

		// Show the alarm advance ui objects. 		
 		ShowObject (frm, DetailsAlarmAdvanceField);
 		ShowObject (frm, DetailsAlarmAdvanceSelector);
 		}
 	else
 		{
		FldFreeMemory (fld);

		CtlSetValue (GetObjectPtr (DetailsAlarmCheckbox), false);

		// Hide the alarm advance ui objects. 		
 		HideObject (frm, DetailsAlarmAdvanceField);
 		HideObject (frm, DetailsAlarmAdvanceSelector);
 		}
 }


/***********************************************************************
 *
 * FUNCTION:    DetailsSetTimeTrigger
 *
 * DESCRIPTION: This routine sets the label of the trigger in a details
 *              dialog that displays the appointment's start and end 
 *              time.
 *
 * PARAMETERS:  start  - start time or minus one if no-time event
 *              end    - end time
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of 
 *      the time trigger is large enough to hold the largest posible
 *      label.  This label's memory is reserved by initializing the label
 *      in the resource file.     
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *
 ***********************************************************************/
static void DetailsSetTimeTrigger (TimeType start, TimeType end)
{
	Char* str;
	Char* label;
	ControlPtr ctl;
	Char* rscP;

	ctl = GetObjectPtr (DetailsTimeSelector);
	label = (Char *)CtlGetLabel (ctl);	// OK to cast; we call CtlSetLabel
	
	if (TimeToInt (start) == apptNoTime)
		{
		rscP = MemHandleLock (DmGetResource (strRsc, noTimeStrID));
		StrCopy (label, rscP);
		MemPtrUnlock (rscP);
		}

	else
		{
		str = label;
		TimeToAscii (start.hours, start.minutes, TimeFormat, str);
		str += StrLen (str);
		*str++ = spaceChr;
		*str++ = '-';
		*str++ = spaceChr;
		TimeToAscii (end.hours, end.minutes, TimeFormat, str);
		}
	
	CtlSetLabel (ctl, label);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsSelectTime
 *
 * DESCRIPTION: This routine selects the start date of an appointment.
 *
 * PARAMETERS:  startP - passed:   current start time
 *                       returned: selected start time
 *              endtP  - passed:   current end time
 *                       returned: selected end time
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *
 ***********************************************************************/
static void DetailsSelectTime (TimePtr startP, TimePtr endP)
{
	if (GetTime (startP, endP, setTimeTitleStrID))
		{
		DetailsSetTimeTrigger (*startP, *endP);
		if (TimeToInt (*startP) == apptNoTime)
			DetailsAlarmOnOff (false);
		}
}


/***********************************************************************
 *
 * FUNCTION:    DetailsSelectDate
 *
 * DESCRIPTION: This routine selects the start date of an appointment.
 *
 * PARAMETERS:  dateP - passed:   current start date
 *                      returned: selected start date, if selection was
 *                                made
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *       art	4/2/96	Adjusted end of if repeating event
 *
 ***********************************************************************/
static void DetailsSelectDate (DetailsPtr details)
{
	Int16 month, day, year;
	Char* label;
	Char* title;
	ControlPtr ctl;

	year = details->when.date.year + firstYear;
	month = details->when.date.month;
	day = details->when.date.day;
	
	title = MemHandleLock(DmGetResource (strRsc, startDateTitleStrID));

	if (SelectDay (selectDayByDay, &month, &day, &year, title))
		{
		// Set the label of the date selector.
		ctl = GetObjectPtr (DetailsDateSelector);
		label = (Char *)CtlGetLabel (ctl);	// OK to cast; we call CtlSetLabel
		DateToDOWDMFormat (month, day, year, ShortDateFormat, label);
		CtlSetLabel (ctl, label);

		// Return the date selected.
		details->when.date.day = day;
		details->when.date.month = month;
		details->when.date.year = year - firstYear;


		// If the event repeats monthly-by-day update the day the the 
		// month the event repeats on.
		if (details->repeat.repeatType == repeatMonthlyByDay)
			{
			details->repeat.repeatOn = DayOfMonth (month, day, year);
			}
		}

	MemPtrUnlock (title);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsDeleteAppointment
 *
 * DESCRIPTION: This routine deletes an appointment record. It is called 
 *              when the delete button in the details dialog is pressed.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *
 ***********************************************************************/
static Boolean DetailsDeleteAppointment (void)
{
	UInt16 recordNum;
		
	recordNum = CurrentRecord;

	// Clear the edit state, this will delete the current record if is 
	// blank.
	if (ClearEditState ())
		return (true);

	// Delete the record,  this routine will display an appropriate 
	// dialog to confirm the action.  If the dialog is canceled 
	// don't update the display.
	if (! DeleteRecord (recordNum))
		return (false);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsGet
 *
 * DESCRIPTION: This routine get the current setting of the details
 *              dialog.  
 *
 * PARAMETERS:  details - pointer to a structure that hold the info about
 *                        the current record.
 *
 * RETURNED:    true if the form info was all valid, otherwise false.
 *
 * NOTE:        Not all the members of the structure passed are filled
 *              in by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *
 ***********************************************************************/
static Boolean DetailsGet (DetailsPtr details)
{
	ListPtr lst;
	FieldPtr fld;
	ControlPtr ctl;
	Char* alarmStr;
	
	// Get the alarm settings.
	ctl = GetObjectPtr (DetailsAlarmCheckbox);
	if (CtlGetValue (ctl))
		{
		fld = GetObjectPtr (DetailsAlarmAdvanceField);
		alarmStr = FldGetTextPtr (fld);
		while (*alarmStr)
			{
			WChar curChar;
			alarmStr += TxtGetNextChar(alarmStr, 0, &curChar);
			if (!TxtCharIsDigit(curChar))
				{
				FrmAlert(AlarmInvalidAlert);
				return(false);
				}
			}
		
		details->alarm.advance = StrAToI (FldGetTextPtr (fld));

		lst = GetObjectPtr (DetailsAlarmAdvanceList);		
		details->alarm.advanceUnit = (AlarmUnitType) LstGetSelection (lst);
		}
	else
		details->alarm.advance = apptNoAlarm;		// no alarm is set

	// Get the private setting.
	details->secret = CtlGetValue (GetObjectPtr (DetailsPrivateCheckbox));
	
	return(true);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  details     - appointment details.
 *              attachNote  - true if pressing the note button being caused
 *                            the changes to be applied.
 *              updateCodeP - pointer to a code which indicates how the
 *									   appointment was changed,  this code is send
 *									   as the update code in a frmUpdate event.
 *
 * RETURNED:    true if the changes were applied, false if the changes
 *              were not applied.
 *
 *	NOTE:			 If a note is being attached to a repeating event the 
 *              user is prompted to determine if an exception should be
 *              created,  the exception is create be this routine,  the
 *              note is attached elsewhere.
 *
 *					 (GKG 1/27/99) Changing the repeat info of weekly repeating
 *					 appointments is tricky.  There are two ways to change which
 *					 days the appt repeats on - 1. change the date in the details dialog
 *					 and 2. change it directly in the repeat info dialog.  Both
 *					 ways should work now.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			kcr	10/09/95	added 'private records' alert
 *			gkg	01/27/99	fixed some problems with weekly repeating appointments
 *			gap	08/27/99	Replaced call of ExceptionAlert with call to RangeDialog.
 *			gap	09/14/99	Date change are now applied when changes are made to
 *								current and future occurrences of an event.
 *			gap	11/28/00	fixed some more problems with weekly repeating events.
 *
 ***********************************************************************/
static Boolean DetailsApply (DetailsPtr details, Boolean attachNote,
	UInt16* updateCodeP)
{
	Err					err = 0;
	UInt16				attr;
	UInt16				id;
	UInt16				repeatOn;
	UInt16 				updateCode	= 0;
	UInt16				alertButton;
	Int32					adjust;
	Boolean				dirty 		= false;
	Boolean				attrDirty 	= false;
	Boolean				exception 	= false;
	Boolean				pastRepeats	= false;
	FormPtr				alert;
	MemHandle 			recordH;
	ApptDateTimeType	when;
	ApptDBRecordType	apptRec;
	ApptDBRecordType	newRec;
	ApptDBRecordFlags	changedFields;
	

	// Get the ui setting for the appointment. If the alarm setting contains
	// invalid characters, bail out.
	if (!DetailsGet (details))
		return(false);
	
	when = details->when;

	// Compare the start date setting in the dialog with the date in the
	// current record.  Update the record if necessary.
	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);

	MemSet (&changedFields, sizeof (changedFields), 0);

	// Has the start time of the appointment been changed?
	if (TimeToInt(apptRec.when->startTime) != TimeToInt(details->when.startTime))
		{
		when.date = apptRec.when->date;
		newRec.when = &when;
		changedFields.when = true;
		updateCode |= updateTimeChanged;
		dirty = true;
		}


	// Has the end time of the appointment been changed?
	if (TimeToInt(apptRec.when->endTime) != TimeToInt(details->when.endTime))
		{
		when.date = apptRec.when->date;
		newRec.when = &when;
		changedFields.when = true;
		updateCode |= updateTimeChanged;
		dirty = true;
		}


	// Has the date of the appointment been changed?
	if (DateToInt(Date) != DateToInt(details->when.date))
		{
		when.date = details->when.date;
		newRec.when = &when;
		changedFields.when = true;
		updateCode |= updateDateChanged;
		dirty = true;
		}
	


	// Has an alarm been set?
	if ((! apptRec.alarm) && (details->alarm.advance != apptNoAlarm))
		{
		newRec.alarm = &details->alarm;
		changedFields.alarm = true;
		updateCode |= updateAlarmChanged;
		dirty = true;
		}

	// Has an alarm been canceled?
	else if ((apptRec.alarm) && (details->alarm.advance == apptNoAlarm))
		{
		newRec.alarm = NULL;
		changedFields.alarm = true;
		updateCode |= updateAlarmChanged;
		dirty = true;
		}

	// Has the alarm advance-notice-time been changed?
	else if ( (apptRec.alarm) &&
				((apptRec.alarm->advance != details->alarm.advance) ||
				 (apptRec.alarm->advanceUnit != details->alarm.advanceUnit)))
		{
		newRec.alarm = &details->alarm;
		changedFields.alarm = true;
		updateCode |= updateAlarmChanged;
		dirty = true;
		}
	


	// Has a non-repeating event been changed to a repeating event?
	if ((! apptRec.repeat) && (details->repeat.repeatType != repeatNone))
		{
		newRec.repeat = &details->repeat;
		changedFields.repeat = true;
		updateCode |= updateRepeatChanged;
		dirty = true;
		}

	// Has a repeating event been changed to a non-repeating event?
	else if ((apptRec.repeat) && (details->repeat.repeatType == repeatNone))
		{
		newRec.repeat = NULL;
		updateCode |= updateRepeatChanged;
		changedFields.repeat = true;
		dirty = true;
		}
	
	// Has any of the repeat info been changed?
	else if (apptRec.repeat &&
					(apptRec.repeat->repeatType != details->repeat.repeatType ||
			   	 DateToInt (apptRec.repeat->repeatEndDate) != DateToInt(details->repeat.repeatEndDate) ||
			   	 apptRec.repeat->repeatFrequency != details->repeat.repeatFrequency ||
					 (details->repeat.repeatType == repeatWeekly &&
			 			apptRec.repeat->repeatOn != details->repeat.repeatOn)))
		{
		newRec.repeat = &details->repeat;
		changedFields.repeat = true;
		updateCode |= updateRepeatChanged;
		dirty = true;
		}
	


	// Get the current setting of the secret checkbox and compare it the
	// the setting of the record.
	DmRecordInfo (ApptDB, CurrentRecord, &attr, NULL, NULL);	
	if (((attr & dmRecAttrSecret) == dmRecAttrSecret) != details->secret)
		{
		if (PrivateRecordVisualStatus > showPrivateRecords)
			{
			updateCode |= updateItemHide;
			}
		else if (details->secret)
			FrmAlert (privateRecordInfoAlert);
			
		attrDirty = true;
		}


	// If the record is a repeating appointment and the repeat info
	// has not been modified,  find out if we should create an exception.
	if ( (apptRec.repeat) && 
	     (!changedFields.repeat) && 
	     ((dirty || attrDirty) || (attachNote && (!apptRec.note))) )
		{

		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);


		// If the alert was canceled don't apply any of the changes.
		if (alertButton == RangeCancelButton)
			goto cancelExit;

		else if (alertButton == RangeCurrentButton)
			{
			updateCode |= updateException;
			exception = true;

			// If the date of the appointment has not be changed then set the
			// date of the exception to the current date.
			if ( ! (updateCode & updateDateChanged))
				when.date = Date;
			}
		else if (alertButton == RangeFutureButton) 
			// Split the repeating event into two event, one of past occurrences
			// and one for future occurreneces.
			{
			pastRepeats = true;
			if ( ! (updateCode & updateDateChanged))
				when.date = Date;
			newRec.when = &when;
			changedFields.when = true;
			dirty = true;
			}
			 // Currently no special code in needed to handle the case that the 
			 // user has selected RangeAllButton only because pastRepeats is initialized
			 // to false.  If the code changes in the future where this may not be the case,
			 // the following else case will need to be activated.
//		else //  (alertButton == RangeAllButton)
//			{
//			pastRepeats = false;
//			}

		}

	// If the record is a repeating appointment, and the repeat info has been
	// changed, and there are past occurrences of the event then we need
	// to duplicate the event so the past occurrence will continue to exist.
	if (apptRec.repeat && (changedFields.repeat))
		{
		// If any of the repeat info other than the end date has changed
		// then spawn a repeating event for all past occurrences.
		if (apptRec.repeat->repeatType != details->repeat.repeatType ||
			 apptRec.repeat->repeatFrequency != details->repeat.repeatFrequency ||
			 apptRec.repeat->repeatOn != details->repeat.repeatOn)
			{
			pastRepeats = true;

			// If the date of the appointment has not been changed then set the
			// start date of the repeating appointment to the current date.
			if ( ! (updateCode & updateDateChanged))
				{
				updateCode |= updateDateChanged;
				when.date = Date;
				newRec.when = &when;
				changedFields.when = true;
				}
			}
		}


	// If the record is a repeating appointment, and the repeat info has not
	// been changed but the start date has, then we need to adjust the 
	// end date and possibly split the event into two events.
	if ((! exception) && apptRec.repeat && (!changedFields.repeat) &&
		 (updateCode & updateDateChanged))
		{
		// If the event is a repeating event and it has an end date, move 
		// the end date such that the duration of the event is maintained.
		if ((DateToInt (apptRec.repeat->repeatEndDate) != apptNoEndDate))
			{
			adjust = DateToDays (apptRec.repeat->repeatEndDate) -
		         DateToDays (apptRec.when->date);
			details->repeat.repeatEndDate = details->when.date;
			DateAdjust (&details->repeat.repeatEndDate, adjust);
			}

		if (DateToInt (details->repeat.repeatEndDate) != 
			 (DateToInt(apptRec.repeat->repeatEndDate)))
			{
			newRec.repeat = &details->repeat;
			changedFields.repeat = true;
			updateCode |= updateRepeatChanged;
			dirty = true;
			}
		}
		

	// If the record is a repeating appointment, and the start date has, 
	// then delete the exceptions list.
	if ((! exception) && apptRec.repeat && (updateCode & updateDateChanged))
		{
		newRec.exceptions = NULL;
		changedFields.exceptions = true;
		dirty = true;
		}


	// If the repeat type is weekly and the start date of the event
	// has been changed, but the user has not explicitly changed
	// the days the event repeats on, determine if the 'repeat on'  
	// setting needs to be updated
	if ((! exception) &&
		 (details->repeat.repeatType == repeatWeekly) &&
		 (updateCode & updateDateChanged)  &&
		 ( (apptRec.repeat) && (apptRec.repeat->repeatOn == details->repeat.repeatOn) )  )
		{
		UInt16 newDayOfWeek;
		UInt8	newDayMask;

		// does the event only repeat once per week?
		ErrNonFatalDisplayIf(details->repeat.repeatOn == 0, "weekly appt. repeats on no days");
		
		newDayOfWeek = DayOfWeek (when.date.month, when.date.day, when.date.year + firstYear);
		newDayMask = (1 << newDayOfWeek);
		
		if (!(details->repeat.repeatOn & newDayMask))
			{
			// If the event only repeats on one day per week
			// update the 'repeat on' field, which contains
			// the days of the week the event repeats on, such that the 
			// event occurs on the start date.		
			if (OnlyRepeatsOncePerWeek(details->repeat))
				details->repeat.repeatOn = newDayMask;
				
			// If the event repeats on more than one day per week
			// and in the day of the newly selected date so that
			// it will be selected in the repeat on settings in the 
			// event the user selected a date that a occurs on a day
			// not already set. 
			else
				details->repeat.repeatOn |= newDayMask;		
										
			changedFields.repeat = true;
			newRec.repeat = &details->repeat;
			}
		}
		
		
	// If the repeat type is monthly by day and the start date of the event
	// has been changed, update the repeat on field which contains
	// the date of the month the event repeating on (ex: first friday).
	if ((! exception) && (details->repeat.repeatType == repeatMonthlyByDay) &&
		 ( (updateCode & updateDateChanged) || (! apptRec.repeat) ))
		{
		repeatOn = DayOfMonth (when.date.month,
			when.date.day,
			when.date.year + firstYear);
			
		// If we're in the fourth week, and the fourth week is also the last
		// week,  ask the user which week the event repeats in (fourth or last).
		if ( ((repeatOn / daysInWeek) == 3) &&
		      (when.date.day + daysInWeek > DaysInMonth (
		      	when.date.month, when.date.year + firstYear)))
			{
			alert = FrmInitForm (MonthlyRepeatDialog);
			
			// Set the 4th / last push button.
			if (apptRec.repeat && 
				 apptRec.repeat->repeatType == repeatMonthlyByDay &&
				 apptRec.repeat->repeatOn > dom4thSat)
				id = MonthlyRepeatLastButton;
			else
				id = MonthlyRepeatFourthButton;
			FrmSetControlGroupSelection (alert, MonthlyRepeatWeekGroup,id);

			#if WRISTPDA
			FrmSetEventHandler( alert, MonthlyRepeatHandleEvent );
			#endif

			alertButton = FrmDoDialog (alert);

			if (FrmGetObjectIndex (alert, MonthlyRepeatLastButton) ==
			    FrmGetControlGroupSelection (alert, MonthlyRepeatWeekGroup))
			   repeatOn += daysInWeek;

			FrmDeleteForm (alert);

			if (alertButton == MonthlyRepeatCancel)
				goto cancelExit;
			
			}

		details->repeat.repeatOn = repeatOn;
		newRec.repeat = &details->repeat;
		changedFields.repeat = true;
		}

	// If the alarm info has been changed, or if the appointment has 
	// an alarm and the date, time, or repeat info has been changed,
	// delete the item from the attention manager queue if present.
	if (changedFields.alarm || (apptRec.alarm && (changedFields.when || changedFields.repeat)) )
		DeleteAlarmIfPosted(CurrentRecord);


	// Unlock the appointment record.
	MemHandleUnlock (recordH);

	// Erase the details dialog before the changes are applied to the 
	// database.  It is necessary to do thing in this order because
	// erasing the details dialog may cause a frmUpdate event to be 
	// sent to list view, and the list view may not be able to redraw
	// properly once the database has been changed.
	FrmEraseForm (FrmGetActiveForm());


	// Write the changes to the database.
	if (exception)
		{
		err = ApptAddException (ApptDB, &CurrentRecord, Date);
		if (! err)
			{
			ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
			MemSet (&newRec, sizeof (newRec), 0);
			newRec.description = apptRec.description;
			newRec.note = apptRec.note;
			newRec.when = &when;
			if (details->alarm.advance != apptNoAlarm)
				newRec.alarm = &details->alarm;
	
			err = CreateException (&newRec, &CurrentRecord);
			MemHandleUnlock (recordH);
			}
		}
	else if (dirty)
		{
		// Create a copy of the repeating event and set its end date to yesterday.
		if (pastRepeats)
			err = SplitRepeatingEvent (&CurrentRecord);
			
		if (!err)
			err = ApptChangeRecord (ApptDB, &CurrentRecord, &newRec, changedFields);
		}		

	if (err)
		{
		FrmAlert (DeviceFullAlert);
		ClearEditState ();
		updateCode = 0;
		}

	// Save the new secret status if it's been changed.
	else if (attrDirty)
		{
		DmRecordInfo (ApptDB, CurrentRecord, &attr, NULL, NULL);	
		if (details->secret)
			attr |= dmRecAttrSecret;
		else
			attr &= ~dmRecAttrSecret;

		attr |= dmRecAttrDirty;
		DmSetRecordInfo (ApptDB, CurrentRecord, &attr, NULL);
		}

	// If the alarm info has been changed, or if the appointment has 
	// an alarm and the date, time, or repeat info has been changed,
	// reschedule the next alarm.
	if (changedFields.alarm || 
		(apptRec.alarm && (changedFields.when || changedFields.repeat)))
		RescheduleAlarms (ApptDB);

	//if record has been hidden, make sure to deselect it
	//if (updateCode & updateItemHide)
	//		CurrentRecord = noRecordSelected;

	*updateCodeP = updateCode;
	return (true);
	

	// We're here if the cancel button one of the confirmation dialogs
	// was pressed.	
cancelExit:
	MemHandleUnlock (recordH);
	return (false);	
}



/***********************************************************************
 *
 * FUNCTION:    DetailsExit
 *
 * DESCRIPTION: This routine is called when the details dialog is 
 *              applied or canceled.  It unlocks any memory locked 
 *              and frees any memory allocated by the Details Dialog routine.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/21/95	Initial Revision
 *			grant 6/28/99	Removed 'free' parameter - always free if DetailsP is non-NULL.
 *								Removed 'details' parameter - we use DetailsP instead.
 *
 ***********************************************************************/
static void DetailsExit(void)
{
	MemHandle rscH;

	// The label of the repeat trigger is locked - unlock it.
	// Note that we can't get the pointer to be unlocked by using
	// CtlGetLabel on the repeat trigger because it may actually be
	// pointing into the middle of the resource.
	rscH = DmGetResource (strRsc, repeatTypesStrID);
	MemHandleUnlock (rscH);
	
	// Free the block that holds the changes to the appointment record. 
	if (DetailsP)
		{
		MemPtrFree (DetailsP);
		DetailsP = 0;
		}
}


/***********************************************************************
 *
 * FUNCTION:    DetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog, and allocates
 *              and initialize a memory block that hold the record
 *              info to be edited by the details dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    details - memory block that hold info to be edited 
 *                        by the details dialog.
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of 
 *      the time trigger is large enough to hold the largest posible
 *      label.  This label's memory is reserved by initializing the label
 *      in the resource file.     
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *
 ***********************************************************************/
static DetailsPtr DetailsInit (void)
{
	UInt16 i;
	UInt16 attr;
	Char* rscP;
	Char* label;
	Char* textP;
	FormPtr frm;
	ListPtr lst;
	FieldPtr fld;
	ControlPtr ctl;
	DetailsPtr details;
	MemHandle textH;
	MemHandle recordH;
	ApptDBRecordType	apptRec;

	// Allocate and initialize a block to hold a temporary copy of the 
	// info from the appointment record, we don't want to apply any changes 
	// to the record until the "Details Dialog" is confirmed.
	if (! DetailsP)
		{
		DetailsP = MemPtrNew (sizeof (DetailsType));
		details = DetailsP;
		MemSet (details, sizeof (DetailsType), 0);
		
		// Get a pointer to the appointment record.
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
		
		DmRecordInfo (ApptDB, CurrentRecord, &attr, NULL, NULL);

		details->secret = (attr & dmRecAttrSecret) == dmRecAttrSecret;
		details->when = *apptRec.when;
		details->when.date = Date;
		if (apptRec.alarm)
			details->alarm = *apptRec.alarm;
		else
			details->alarm.advance = apptNoAlarm;

		if (apptRec.repeat)
			details->repeat = *apptRec.repeat;
		else
			{
			details->repeat.repeatType = repeatNone;
			//DateToInt (details->repeat.repeatEndDate) = apptNoEndDate;
                	details->repeat.repeatEndDate.year = 0x7f;
                	details->repeat.repeatEndDate.month = 0x0f;
                	details->repeat.repeatEndDate.day = 0x1f;
			}
		
		
		// Unlock the appointment record
		MemHandleUnlock (recordH);
		}
	else
		details = DetailsP;

	// Set the time selector label.
	DetailsSetTimeTrigger (details->when.startTime, details->when.endTime);
	

	// Set the start date selector label.
	ctl = GetObjectPtr (DetailsDateSelector);
	label = (Char *)CtlGetLabel (ctl);	// OK to cast; we call CtlSetLabel
	DateToDOWDMFormat (details->when.date.month, details->when.date.day, 
		details->when.date.year + firstYear, ShortDateFormat, label);
	CtlSetLabel (ctl, label);

	
	// Set the alarm check box.
	ctl = GetObjectPtr (DetailsAlarmCheckbox);
	CtlSetValue (ctl, (details->alarm.advance != apptNoAlarm));
	
	if (details->alarm.advance != apptNoAlarm)
		{
		frm = FrmGetActiveForm ();
		
		// Set the alarm advance value.
		fld = GetObjectPtr (DetailsAlarmAdvanceField);
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, details->alarm.advance);
		MemPtrUnlock (textP);
		FldSetTextHandle (fld, textH);
		ShowObject (frm, DetailsAlarmAdvanceField);
	
		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = GetObjectPtr (DetailsAlarmAdvanceList);		
		LstSetSelection (lst, details->alarm.advanceUnit);
		label = LstGetSelectionText (lst, details->alarm.advanceUnit);

		ctl = GetObjectPtr (DetailsAlarmAdvanceSelector);
		CtlSetLabel (ctl, label);
		ShowObject (frm, DetailsAlarmAdvanceSelector);
		}
	

	// Set the repeat type selector label.  The label will point
	// to a locked resouce string,  we'll need to unlock this 
	// resource when the dialog is dismissed.
	ctl = GetObjectPtr (DetailsRepeatSelector);
	rscP = MemHandleLock (DmGetResource (strRsc, repeatTypesStrID));
	for (i = 0; i < details->repeat.repeatType; i++)
		rscP += StrLen (rscP) + 1;
	CtlSetLabel (ctl, rscP);
	

	// If the record is mark secret, turn on the secret checkbox.
	ctl = GetObjectPtr (DetailsPrivateCheckbox);
	CtlSetValue (ctl, details->secret);


	return (details);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/20/95	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			jmp	09/17/99	Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
Boolean DetailsHandleEvent (EventType* event)
{
	static DetailsPtr	details;

	UInt16 updateCode = 0;
	FormPtr frm;
	Boolean handled = false;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case DetailsOkButton:
				if (DetailsApply (details, false, &updateCode))
					{
					DetailsExit();
					FrmReturnToForm (DayView);
					}
				FrmUpdateForm (DayView, updateCode);
				handled = true;
				break;

			case DetailsCancelButton:
				DetailsExit();
				FrmUpdateForm (DayView, updateCode);
				FrmReturnToForm (DayView);
				handled = true;
				break;
				
			case DetailsDeleteButton:
				DetailsExit();
				FrmReturnToForm (DayView);
				if ( DetailsDeleteAppointment ()) 
					updateCode = updateItemDelete;
				FrmUpdateForm (DayView, updateCode);
				handled = true;
				break;
				
			case DetailsNoteButton:
				if (DetailsApply (details, true, &PendingUpdate))
					{
					DetailsExit();
					FrmCloseAllForms ();
					if (CreateNote (false))
						FrmGotoForm (NewNoteView);
					else
						FrmGotoForm (DayView);
					}
				handled = true;
				break;
				
			case DetailsTimeSelector:
				DetailsSelectTime (&details->when.startTime, &details->when.endTime);
				handled = true;
				break;

			case DetailsDateSelector:
				DetailsSelectDate (details);
				handled = true;
				break;

			case DetailsAlarmCheckbox:
				DetailsAlarmOnOff (event->data.ctlSelect.on);
				handled = true;
				break;

			case DetailsRepeatSelector:
				if (DetailsGet (details))
					{
					// Give control of the details info block to the Repeat form
					RepeatDetailsP = DetailsP;
					DetailsP = 0;
					
					DetailsExit();
					FrmGotoForm (RepeatDialog);
					}
				
				handled = true;
				break;
			}
		}


	#if WRISTPDA
	else if	(event->eType == keyDownEvent) {
		EventType newEvent;
		newEvent = *event;
		newEvent.eType = ctlSelectEvent;
		newEvent.tapCount = 1;
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to an Ok button event.
			newEvent.data.ctlSelect.controlID = DetailsOkButton;
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to an Ok button event.
			newEvent.data.ctlSelect.controlID = DetailsCancelButton;
			EvtAddEventToQueue( &newEvent );
			return true;
		} else 	if	((!TxtCharIsHardKey(event->data.keyDown.modifiers,
										event->data.keyDown.chr))
				&& (!EvtKeydownIsVirtual(event)))
		{
		WChar chr = event->data.keyDown.chr;
		if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
			{
			FldHandleEvent (GetObjectPtr (DetailsAlarmAdvanceField), event);
			}
		handled = true;
		}
	}
	#else
	else if	(	(event->eType == keyDownEvent)
				&&	(!TxtCharIsHardKey(	event->data.keyDown.modifiers,
												event->data.keyDown.chr))
				&& (!EvtKeydownIsVirtual(event)))
		{
		WChar chr = event->data.keyDown.chr;
		if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
			{
			FldHandleEvent (GetObjectPtr (DetailsAlarmAdvanceField), event);
			}
		handled = true;
		}
	#endif

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		details = DetailsInit ();
		FrmDrawForm (frm);
		FrmSetFocus (frm, FrmGetObjectIndex (frm, DetailsAlarmAdvanceField));
		handled = true;
		}
	
	else if (event->eType == frmCloseEvent)
		{
		if (DetailsP)
			{
			MemPtrFree(DetailsP);
			DetailsP = 0;
			}
		}

	return (handled);
}


#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    NoteViewDrawTitleAndForm
 *
 * DESCRIPTION: This routine draws the form and title of the note view.
 *
 * PARAMETERS:  frm, FormPtr to the form to draw
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		08/03/95	art	Created by Art Lamb.
 *		08/04/99	kwk	Use FntWidthToOffset for text truncation.
 *    09/27/99 jmp   Square off the NoteView title so that it covers up
 *                   the blank Form title used to trigger the menu on taps
 *                   to the title area.  Also, set the NoteView title's color
 *                   to match the standard Form title colors.  Eventually, we
 *                   should add a variant to Forms that allows for NoteView
 *                   titles directly.  This "fixes" bug #21610.
 *		09/29/99	jmp	Fix bug #22413:  Ensure that we peform the font metrics
 *							AFTER we have set the font!
 *    12/02/99 jmp	Fix bug #24377.  Don't call WinScreenUnlock() if WinScreenLock()
 *                   fails.
 *                          
 ***********************************************************************/
 static void NoteViewDrawTitleAndForm (FormPtr frm)
 {
 	Coord x;
	Coord maxWidth;
   Coord formWidth;
   RectangleType r;
	FontID curFont;
	Char* desc;
	MemHandle recordH;
	RectangleType eraseRect, drawRect;
	ApptDBRecordType	apptRec;
	UInt16 descLen, ellipsisWidth;
	Char* linefeedP;
	Int16 descWidth;
	IndexedColorType curForeColor;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;
	UInt8 * lockedWinP;

	// Get current record and related info.
	//
	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
	desc = apptRec.description;
	
	// "Lock" the screen so that all drawing occurs offscreen to avoid
	// the anamolies associated with drawing the Form's title then drawing
	// the NoteView title.  We REALLY need to make a variant for doing
	// this in a more official way!
	//
	lockedWinP = WinScreenLock (winLockCopy);

	FrmDrawForm(frm);
	
	// Perform initial set up.
	//
   FrmGetFormBounds(frm, &r);
   formWidth = r.extent.x;
	maxWidth = formWidth - 8;
	
	linefeedP = StrChr (desc, linefeedChr);
	descLen = (linefeedP == NULL ? StrLen (desc) : linefeedP - desc);
	ellipsisWidth = 0;
			
	#if WRISTPDA
	curFont = FntSetFont ( FossilLargeFontID( WRISTPDA, noteTitleFont ) );
	#endif

	RctSetRectangle (&eraseRect, 0, 0, formWidth, FntLineHeight()+4);
	RctSetRectangle (&drawRect, 0, 0, formWidth, FntLineHeight()+2);
	
	// Save/Set window colors and font.  Do this after FrmDrawForm() is called
	// because FrmDrawForm() leaves the fore/back colors in a state that we
	// don't want here.
	//
 	curForeColor = WinSetForeColor (UIColorGetTableEntryIndex(UIFormFrame));
 	curBackColor = WinSetBackColor (UIColorGetTableEntryIndex(UIFormFill));
 	curTextColor = WinSetTextColor (UIColorGetTableEntryIndex(UIFormFrame));
	#if WRISTPDA
	// Font is set up above, prior to the calls to FntLineHeight.
	#else
	curFont = FntSetFont (noteTitleFont);
	#endif

	// Erase the Form's title area and draw the NoteView's.
	//
	WinEraseRectangle (&eraseRect, 0);
	WinDrawRectangle (&drawRect, 3);

	if (FntWidthToOffset (desc, descLen, maxWidth, NULL, &descWidth) != descLen)
		{
		ellipsisWidth = FntCharWidth(chrEllipsis);
		descLen = FntWidthToOffset (desc, descLen, maxWidth - ellipsisWidth, NULL, &descWidth);
		}
	
	x = (formWidth - descWidth - ellipsisWidth + 1) / 2;
	
	WinDrawInvertedChars (desc, descLen, x, 1);
	if (ellipsisWidth != 0)
		{
		Char buf[maxCharBytes + sizeOf7BitChar(chrNull)];
		buf[TxtSetNextChar(buf, 0, chrEllipsis)] = chrNull;
		WinDrawInvertedChars (buf, StrLen(buf), x + descWidth, 1);
		}

	// Now that we've drawn everything, blast it all back on the screen at once.
	//
	if (lockedWinP)
		WinScreenUnlock ();

	// Unlock the record that ApptGetRecord() implicitly locked.
	//
	MemHandleUnlock (recordH);
	
   // Restore window colors and font.
   //
   WinSetForeColor (curForeColor);
   WinSetBackColor (curBackColor);
   WinSetTextColor (curTextColor);
	FntSetFont (curFont);
}


#if WRISTPDA
/***********************************************************************
 *
 * FUNCTION:    NoteViewUpdateScrollButtons
 *
 * DESCRIPTION: This routine updates the WristPDA scroll buttons.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dmc		07/01/02	Initial Revision
 *
 ***********************************************************************/
static void NoteViewUpdateScrollButtons( void )
{

    Boolean    Down, Up, Visible;
	ControlPtr ctlUp, ctlDown;
	FieldPtr   fld;

	fld = GetObjectPtr (NoteField);

	// Update the Down button state.

	ctlDown = GetObjectPtr( NotePageDown );

	// Can scroll the field down?

	if ( FldScrollable( fld, winDown ) ) {
		Down = true;
	}

	CtlSetLabel( ctlDown, ( Down == true ) ? DownArrowEnabled : DownArrowDisabled );
	CtlSetEnabled( ctlDown, ( Down == true ) );

	// Update the Up button state.

	ctlUp = GetObjectPtr( NotePageUp );

	// Can scroll the field up?

	if ( FldScrollable( fld, winUp ) ) {
		Up = true;
	}

	CtlSetLabel( ctlUp, ( Up == true ) ? UpArrowEnabled : UpArrowDisabled );
	CtlSetEnabled( ctlUp, ( Up == true ) );

	// Buttons are only visible if we can scroll up and/or down.

	Visible = ( Down || Up );

	if ( Visible ) {
		CtlShowControl( ctlDown );
		CtlShowControl( ctlUp );
	} else {
		CtlSetLabel( ctlDown, DownArrowHidden );
		CtlSetLabel( ctlUp, UpArrowHidden );
		CtlHideControl( ctlDown );
		CtlHideControl( ctlUp );
	}

}
#endif


/***********************************************************************
 *
 * FUNCTION:    NoteViewUpdateScrollBar
 *
 * DESCRIPTION: This routine update the scroll bar.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/01/96	Initial Revision
 *			gap	11/02/96	Fix case where field and scroll bars get out of sync
 *
 ***********************************************************************/
static void NoteViewUpdateScrollBar (void)
{
#if WRISTPDA
	NoteViewUpdateScrollButtons();
#else
	UInt16 scrollPos;
	UInt16 textHeight;
	UInt16 fieldHeight;
	Int16 maxValue;
	FieldPtr fld;
	ScrollBarPtr bar;

	fld = GetObjectPtr (NoteField);
	bar = GetObjectPtr (NoteScrollBar);
	
	FldGetScrollValues (fld, &scrollPos, &textHeight,  &fieldHeight);

	if (textHeight > fieldHeight)
		{
		// On occasion, such as after deleting a multi-line selection of text,
		// the display might be the last few lines of a field followed by some
		// blank lines.  To keep the current position in place and allow the user
		// to "gracefully" scroll out of the blank area, the number of blank lines
		// visible needs to be added to max value.  Otherwise the scroll position
		// may be greater than maxValue, get pinned to maxvalue in SclSetScrollBar
		// resulting in the scroll bar and the display being out of sync.
		maxValue = (textHeight - fieldHeight) + FldGetNumberOfBlankLines (fld);
		}
	else if (scrollPos)
		maxValue = scrollPos;
	else
		maxValue = 0;

	SclSetScrollBar (bar, scrollPos, 0, maxValue, fieldHeight-1);
#endif
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewLoadRecord
 *
 * DESCRIPTION: This routine loads the note db field into the note edit 
 *              object.
 *
 * PARAMETERS:  frm - pointer to the Edit View form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void NoteViewLoadRecord (void)
{
	UInt16 offset;
	FieldPtr fld;
	MemHandle recordH;
	Char * ptr;
	ApptDBRecordType	apptRec;
	
	// Get a pointer to the note field.
	fld = GetObjectPtr (NoteField);
	
	// Set the font used in the note field.
	FldSetFont (fld, NoteFont);
	
	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
	ErrFatalDisplayIf (! apptRec.note, "Invalid record");

	// Compute the offset within the appointment record of the note string.
	// The field object will edit the note in place, its is not copied
	// to the dynamic heap.
	ptr = MemHandleLock(recordH);
	offset = apptRec.note - ptr;
	FldSetText (fld, recordH, offset, StrLen(apptRec.note)+1);
	
	MemHandleUnlock (recordH);
	MemHandleUnlock (recordH);		// was also locked by ApptGetRecord
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewSave
 *
 * DESCRIPTION: This routine removed any empty space that the field
 *              routines may a add to the note,  the field object
 *              modifies the note db field in place.  The current
 *              record is also mark dirty.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void NoteViewSave (void)
{
	FieldPtr fld;
	Boolean empty = false;

	fld = GetObjectPtr (NoteField);

	// Was the note string modified by the user.
	if (FldDirty (fld))
		{
		// Release any free space in the note field.
		FldCompactText (fld);

		// Mark the record dirty.	
		DirtyRecord (ApptDB, CurrentRecord);
		}
		
	empty = (FldGetTextLength (fld) == 0);


	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of,  this call also unlocks
	// the handle that contains the note string.
	FldSetTextHandle (fld, 0);
	

	// Remove the note from the record, if it is empty.
	if (empty)
		{
		DeleteNote (false, false);
		}
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewDeleteNote
 *
 * DESCRIPTION: This routine deletes a the note field from an appointment
 *              record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the note was deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/4/95	Initial Revision
 *
 ***********************************************************************/
static Boolean NoteViewDeleteNote (void)
{
	FormPtr alert;
	UInt16 alertButton;
	FieldPtr fld;
	Boolean exception = false;
	Boolean splitEvent = false;
	MemHandle recordH;
	ApptDBRecordType apptRec;
	
	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
	MemHandleUnlock (recordH);

	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if and exception is being created.
	if (apptRec.repeat)
		{
		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);
		
		if (alertButton == RangeCancelButton)
			return (false);
			
		else if (alertButton == RangeCurrentButton)
			exception = true;
			
		else if (alertButton == RangeFutureButton)
			splitEvent = true;
		}

	// Confirm the operation.
	else if (FrmAlert(DeleteNoteAlert) != DeleteNoteYes)
		return (false);


	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of.  This call also 
	// unlocks the handle that contains the note string.
	fld = GetObjectPtr (NoteField);
	FldCompactText (fld);
	FldSetTextHandle (fld, 0);	

	// Remove the note field form the record.
	if (DeleteNote (exception, splitEvent))
		return (true);

	// If we were unable to delete the note, restore the state of the 
	// note view.  Deletes can fails if the appointment repeats and
	// there not enough space to create an exception.
	NoteViewLoadRecord ();
	
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewDoCommand
 *
 * DESCRIPTION: This routine preforms the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/29/95	Initial Revision
 *			jmp	9/17/99	Eliminate the obsolete goto top/bottom menu items.
 *			jmp	11/04/99	To prevent other sublaunch issues, remind ourselves
 *								that we've sublaunched already into PhoneNumberLookup().
 *
 ***********************************************************************/
static Boolean NoteViewDoCommand (UInt16 command)
{
	FieldPtr fld;
	Boolean handled = true;
	
	switch (command)
		{
		case newNoteFontCmd:
			NoteFont = SelectFont (NoteFont);
			break; 
		
		case newNotePhoneLookupCmd:
			fld = GetObjectPtr (NoteField);
			InPhoneLookup = true;
			PhoneNumberLookup (fld);
			InPhoneLookup = false;
			break;

		default:
			handled = false;
		}	
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewScroll
 *
 * DESCRIPTION: This routine scrolls the Note View by the specified
 *					 number of lines.
 *
 * PARAMETERS:  linesToScroll - the number of lines to scroll,
 *						positive for down,
 *						negative for up
 *					 updateScrollbar - force a scrollbar update?
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant 2/2/99	Use NoteViewUpdateScrollBar()
 *
 ***********************************************************************/
static void NoteViewScroll (Int16 linesToScroll, Boolean updateScrollbar)
{
	UInt16			blankLines;
	FieldPtr			fld;
	
	fld = GetObjectPtr (NoteField);
	blankLines = FldGetNumberOfBlankLines (fld);

	if (linesToScroll < 0)
		FldScrollField (fld, -linesToScroll, winUp);
	else if (linesToScroll > 0)
		FldScrollField (fld, linesToScroll, winDown);
	
	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	//if ((blankLines && linesToScroll < 0) || updateScrollbar)
	if ((blankLines && linesToScroll < 0) || updateScrollbar) // XXX
		{
		NoteViewUpdateScrollBar();
		}
}

/***********************************************************************
 *
 * FUNCTION:    NoteViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page up or down.
 *
 * PARAMETERS:   direction     up or down
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant 2/9/99	use NoteViewScroll() to do actual scrolling
 *
 ***********************************************************************/
static void NoteViewPageScroll (WinDirectionType direction)
{
	UInt16 linesToScroll;
	FieldPtr fld;

	fld = GetObjectPtr (NoteField);
	
	if (FldScrollable (fld, direction))
		{
		linesToScroll = FldGetVisibleLines (fld) - 1;
		
		if (direction == winUp)
			linesToScroll = -linesToScroll;
			
		NoteViewScroll(linesToScroll, true);
		}
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewInit
 *
 * DESCRIPTION: This routine initializes the Note View form.
 *
 * PARAMETERS:  frm - pointer to the Note View form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/31/95	Initial Revision
 *			jmp	9/8/99	Make this routine more consistent with other
 *								built-in apps that have it.
 *			jmp	9/23/99	Eliminate the hiding of the old font controls now
 *								that we're using a form that doesn't contain them anyway.
 *			peter	09/20/00	Disable attention indicator because title is custom.
 *
 ***********************************************************************/
static void NoteViewInit (FormPtr frm)
{
	FieldPtr 		fld;
	FieldAttrType	attr;

	AttnIndicatorEnable(false);		// Custom title doesn't support attention indicator.
	NoteViewLoadRecord ();

	// Have the field send events to maintain the scroll bar.
	fld = GetObjectPtr (NoteField);
	FldGetAttributes (fld, &attr);
	attr.hasScrollBar = true;
	FldSetAttributes (fld, &attr);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Note View".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			jmp	9/27/99	Combined NoteViewDrawTitle() & FrmUpdateForm()
 *								into a single routine that is now called
 *								NoteViewDrawTitleAndForm().
 *			peter	09/15/00	Disable attention indicator because title is custom.
 *
 ***********************************************************************/
Boolean NoteViewHandleEvent (EventType* event)
{
	UInt16 pos;
	FormPtr frm;
	FieldPtr fld;
	Boolean handled = false;

	if (event->eType == keyDownEvent)
		{
		#if WRISTPDA
		frm = FrmGetActiveForm ();
		// Update scroll buttons to handle degenerate case where scrolling
		// is enabled when initially editing a memo, but then subsequently
		// disabled after some lines have been deleted.
		NoteViewUpdateScrollButtons();
		// Translate the Enter and Back keys to a Done button event.
		if ( ( event->data.keyDown.chr == vchrThumbWheelPush ) ||
			 ( event->data.keyDown.chr == vchrThumbWheelBack ) ) {
			EventType newEvent;
			MemSet( & newEvent, sizeof( EventType ), 0 );
			newEvent.eType = ctlSelectEvent;
			newEvent.data.ctlSelect.controlID = NoteDoneButton;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
			return true;
		} else
		// Translate the RockerUp key to a PageUp key event.
		if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
			EventType newEvent;
			MemSet( & newEvent, sizeof( EventType ), 0 );
			newEvent.eType = keyDownEvent;
			newEvent.data.keyDown.chr = vchrPageUp;
			newEvent.data.keyDown.modifiers = commandKeyMask;
			EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
			return true;
		} else
		// Translate the RockerDown key to a PageDown key event.
		if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
			EventType newEvent;
			MemSet( & newEvent, sizeof( EventType ), 0 );
			newEvent.eType = keyDownEvent;
			newEvent.data.keyDown.chr = vchrPageDown;
			newEvent.data.keyDown.modifiers = commandKeyMask;
			EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
			return true;
		} else
		#endif
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
			NoteViewSave ();
			ClearEditState ();
			PendingUpdate = 0;
			DateSecondsToDate (TimGetSeconds (), &Date);
			FrmGotoForm (DayView);
			handled = true;
			}
		else if (EvtKeydownIsVirtual(event))
			{
			if (event->data.keyDown.chr == vchrPageUp)
				{
				NoteViewPageScroll (winUp);
				handled = true;
				}
	
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				NoteViewPageScroll (winDown);
				handled = true;
				}
			}
		}

		#if WRISTPDA
		else if (event->eType == ctlRepeatEvent) {
			// Handle scroll button event.
			ControlPtr ctl;
			EventType  newEvent;
			// Redraw the control to eliminate inverted state.
			ctl = GetObjectPtr( event->data.ctlRepeat.controlID );
			CtlEraseControl( ctl );
			CtlDrawControl( ctl );
			// Translate the repeating button event to a PageUp/PageDown key event.
			newEvent = *event;
			newEvent.eType = keyDownEvent;
			newEvent.tapCount = 1;
			newEvent.data.keyDown.keyCode = 0;
			newEvent.data.keyDown.modifiers = 8;
			switch (event->data.ctlRepeat.controlID)
			{
			case NotePageUp:
				handled = true;
				newEvent.data.keyDown.chr = vchrPageUp;
				EvtAddEventToQueue( &newEvent );
				break;
			case NotePageDown:
				handled = true;
				newEvent.data.keyDown.chr = vchrPageDown;
				EvtAddEventToQueue( &newEvent );
				break;
			}
		}
	#endif

	else if (event->eType == ctlSelectEvent)
		{		
		switch (event->data.ctlSelect.controlID)
			{
			case NoteDoneButton:
				NoteViewSave ();
				FrmGotoForm (DayView);
				handled = true;
				break;

			case NoteDeleteButton:
				if (NoteViewDeleteNote())
					FrmGotoForm (DayView);
				handled = true;
				break;
			}
		}


	else if (event->eType == menuEvent)
		{
		handled = NoteViewDoCommand (event->data.menu.itemID);
		}
		

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		NoteViewInit (frm);
		NoteViewDrawTitleAndForm (frm);
		NoteViewUpdateScrollBar ();
		FrmSetFocus (frm, FrmGetObjectIndex (frm, NoteField));
		handled = true;
		}
		

	else if (event->eType == frmGotoEvent)
		{
		frm = FrmGetActiveForm ();
		CurrentRecord = event->data.frmGoto.recordNum;
		SetDateToNextOccurrence (CurrentRecord);
		NoteViewInit (frm);
		fld = GetObjectPtr (NoteField);
		pos = event->data.frmGoto.matchPos;
		FldSetScrollPosition (fld, pos);
		FldSetSelection (fld, pos, pos + event->data.frmGoto.matchLen);
		NoteViewDrawTitleAndForm (frm);
		NoteViewUpdateScrollBar ();
		FrmSetFocus (frm, FrmGetObjectIndex (frm, NoteField));
		handled = true;
		}
		

	else if (event->eType == fldChangedEvent)
		{
		frm = FrmGetActiveForm ();
		NoteViewUpdateScrollBar ();
		handled = true;
		}
		

	else if (event->eType == frmUpdateEvent)
		{
		if (event->data.frmUpdate.updateCode & updateFontChanged)
			{
			fld = GetObjectPtr (NoteField);
			FldSetFont (fld, NoteFont);
			NoteViewUpdateScrollBar ();
			}
		else
			{
			frm = FrmGetActiveForm ();
			NoteViewDrawTitleAndForm (frm);
			}
		handled = true;
		}


	else if (event->eType == frmCloseEvent)
		{
		AttnIndicatorEnable(true);		// Custom title doesn't support attention indicator.
		if ( FldGetTextHandle (GetObjectPtr (NoteField)))
			NoteViewSave ();
		}


	else if (event->eType == sclRepeatEvent)
		{
		NoteViewScroll (event->data.sclRepeat.newValue - 
			event->data.sclRepeat.value, false);
		}
	
	return (handled);
}


#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    DayViewRestoreEditState
 *
 * DESCRIPTION: This routine restores the edit state of the day view,
 *              if the view is in edit mode. This routine is 
 *              called when the time of an appointment is changed, or when
 *              returning from the details dialog or note view.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/28/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewRestoreEditState ()
{
	Int16 row;
	UInt32 uniqueID;
	FormPtr frm;
	TablePtr table;
	FieldPtr fld;

	if ( ! ItemSelected) return;

	// Find the row that the current record is in.  It's posible 
	// that the current record is no longer displayable (ex: the record
	// has marked private).
	table = GetObjectPtr (DayTable);
	DmRecordInfo (ApptDB, CurrentRecord, NULL, &uniqueID, NULL);

	if ( ! TblFindRowData (table, uniqueID, &row) )
		{
		ClearEditState ();
		return;
		}

	// The only time an item might be the current selection but still masked
	// is if the user is "going to" the event from the attention manager.  In 
	// that case we need to query for the password then unmask the event.
	if ( TblRowMasked(table,row))
		{
		if (SecVerifyPW (showPrivateRecords) == true)
			{
			// We only want to unmask this one record, so restore the preference.
			PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

			// Unmask just the current row.
			TblSetRowMasked(table, row, false);

			// Draw the row unmasked.
			TblMarkRowInvalid (table, row);
			TblRedrawTable(table);

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
			}
		else
			{
			ClearEditState ();
			return;
			}
		}

	frm = FrmGetActiveForm ();
	FrmSetFocus (frm, FrmGetObjectIndex (frm, DayTable));
	TblGrabFocus (table, row, descColumn);
	
	// Restore the insertion point position.
	fld = TblGetCurrentField (table);
	FldSetInsPtPosition (fld, DayEditPosition);
	if (DayEditSelectionLength)
		FldSetSelection (fld, DayEditPosition, DayEditPosition + DayEditSelectionLength);

	FldGrabFocus (fld);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewClearEditState
 *
 * DESCRIPTION: This routine clears the edit state of the day view.
 *              It is called whenever a table item is selected.
 *
 *              If the new item selected is in a different row than
 *              the current record the edit state is cleared,  and if 
 *              current record is empty it is deleted.
 *
 * PARAMETERS:  newRow - row number of newly table item
 *
 * RETURNED:    true if the current record is deleted and items
 *              of the table have been move around on the display
 *              as a result.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/28/95	Initial Revision
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *			peter	4/25/00	Re-mask private record when leaving it.
 *
 ***********************************************************************/
static Boolean DayViewClearEditState (void)
{
	Int16 row;
	UInt16 numAppts;
	UInt32 uniqueID;
	TablePtr table;
	FormPtr frm;
	UInt16 attr;
	Boolean found;

	if ( ! ItemSelected)
		{
		CurrentRecord = noRecordSelected;
		return (false);
		}

	frm = FrmGetFormPtr (DayView);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));
	DmRecordInfo (ApptDB, CurrentRecord, NULL, &uniqueID, NULL);
	found = TblFindRowData (table, uniqueID, &row);
	ErrNonFatalDisplayIf (!found, "Wrong record");
	
	TblReleaseFocus (table);
	
	// We're leaving a record. If it's secret and we're masking secret records
	// (but unmasked just this one), then re-mask it now.
	if (found && (CurrentRecordVisualStatus != PrivateRecordVisualStatus))
	{
		CurrentRecordVisualStatus = PrivateRecordVisualStatus;
		
		// Is the record still secret? It may have been changed from the
		// details dialog.
		DmRecordInfo (ApptDB, CurrentRecord, &attr, NULL, NULL);
		
		if (attr & dmRecAttrSecret)
		{
			// Re-mask the current row.
			TblSetRowMasked(table, row, true);

			// Draw the row masked.
			TblMarkRowInvalid (table, row);
			TblRedrawTable(table);
		}
	}
	
	// If a different row has been selected, clear the edit state, this 
	// will delete the current record if its empty.
	if (ClearEditState ())
		{
		numAppts = NumAppts;

		// Layout the day again, empty time slots may needed to be add to 
		// full-in the gap left by the deleted appointment.
		DayViewLayoutDay (true);
		
		DayViewLoadTable ();
		TblRedrawTable (table);
		DayViewDrawTimeBars ();

		// If the number of appointments has changed as a result of
		// deleting the currently selected empty appointment, then 
		// the item in the table have move around and tblEnter event
		// should be ignored.
		return (numAppts != NumAppts);
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewFindAppointment
 *
 * DESCRIPTION: Given the database index of a record, this routine 
 *              finds the record in the appointment list and returns
 *              the index of the appointment
 *
 * PARAMETERS:  recordNum - db index of record to fine 
 *              recordNum - 
 *
 * RETURNED:   appointment list index
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/27/95	Initial Revision
 *
 ***********************************************************************/
static UInt16 DayViewFindAppointment (UInt16 recordNum)
{
	UInt16 i;
	ApptInfoPtr appts;
	
	appts = MemHandleLock (ApptsH);

	for (i = 0; i < NumAppts; i++)
		{
		if (appts[i].recordNum == recordNum)
			{
			MemPtrUnlock (appts);
			return (i);
			}
		}

	MemPtrUnlock (appts);

	// If we're beyond the maximun number of appointment that can be 
	// shown on a day, then the record passed may not be in the list.
	if (NumApptsOnly >= apptMaxPerDay)
		{
		ClearEditState ();
		return (0);
		}

	
	ErrDisplay ("Record not on day");

	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSetTopAppointment
 *
 * DESCRIPTION: This routine determines the first appointment that should
 *              be visible on the current day.  For all dates other than
 *              today the fisrt time slot of the appointment list
 *              is the first visible appointment.  For today the time
 *              slot that stats before to the current time should be the top 
 *              visible time slot.
 *
 * PARAMETERS:  nothing.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewSetTopAppointment (void)
{
	UInt16 i;
	TimeType time;
	DateTimeType dateTime;
	ApptInfoPtr appts;

	TopVisibleAppt = 0;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);

	// If the current date is not today, then the first appointment 
	// is the first one visible.
	if ( (dateTime.year - firstYear != Date.year) ||
		  (dateTime.month != Date.month) ||
		  (dateTime.day != Date.day))
		{
		return;
		}

	// If the current date is today, then the top visible appointment is
	// the appointment with the greatest end time that is before the 
	// current time.
	time.hours = dateTime.hour;
	time.minutes = dateTime.minute;
	
	appts = MemHandleLock (ApptsH);
	for (i = 0; i < NumAppts; i++)
		{
		if (TimeToInt (appts[i].endTime) < TimeToInt (time))
			TopVisibleAppt = i;
		}

	MemPtrUnlock (appts);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDrawTime
 *
 * DESCRIPTION: This routine draws the start time of an appointment.  This
 *              routine is called by the table object as a callback 
 *              routine.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *			kcr	11/14/95	Display 'no time' char for untimed events
 *			rbb	6/4/99	Moved bulk of code to DrawTime, for sharing w/Agenda
 *
 ***********************************************************************/
static void DayViewDrawTime (void * table, Int16 row, Int16 column, 
	RectanglePtr bounds)
{
#pragma unused (column)

	UInt16 apptIndex;
	TimeType startTime;
	ApptInfoPtr appts;
	

	// Get the appointment index that corresponds to the table item.
	// The index of the appointment in the appointment list, is stored
	// as the row id.
	apptIndex =TblGetRowID (table, row);

	// Get the start time of the appointment.
	appts = MemHandleLock (ApptsH);
	startTime = appts[apptIndex].startTime;
	MemHandleUnlock (ApptsH);
	
	#if WRISTPDA
	bounds->extent.x -= 4;
	#endif

	DrawTime (startTime, TimeFormat, apptTimeFont, rightAlign, bounds);

	#if WRISTPDA
	bounds->extent.x += 4;
	#endif

}


/***********************************************************************
 *
 * FUNCTION:    DayViewDrawIcons
 *
 * DESCRIPTION: This routine draws the note, alarm and repeat icons.
 *              It is called by the table object as a callback 
 *              routine. 
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/16/96	Initial Revision
 *
 ***********************************************************************/
static void DayViewDrawIcons (void * table, Int16 row, Int16 column, 
	RectanglePtr bounds)
{
	Char					chr;
	UInt16					apptIndex;
	UInt16 					recordNum;
	Int16					x, y;
	FontID				curFont;
	MemHandle 			recordH;
	ApptInfoPtr 		appts;
	ApptDBRecordType	apptRec;

	// Get the appointment index that corresponds to the table item.
	// The index of the appointment in the appointment list, is stored
	// as the row id.
	apptIndex =TblGetRowID (table, row);

	// Get the start time of the appointment.
	appts = MemHandleLock (ApptsH);
	recordNum = appts[apptIndex].recordNum;
	MemHandleUnlock (ApptsH);

	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);

	x = bounds->topLeft.x + bounds->extent.x - TblGetItemInt (table, row, column);
	y = bounds->topLeft.y;
	curFont = FntSetFont (symbolFont);

	// Draw note icon
	if (apptRec.note)
		{
		chr = symbolNote;
		WinDrawChars (&chr, 1, x, y);
		x += FntCharWidth (chr) + 1;
		}

	// Draw alarm icon
	if (apptRec.alarm)
		{
		chr = symbolAlarm;
		WinDrawChars (&chr, 1, x, y);
		x += FntCharWidth (chr) + 1;
		}

	// Draw repeat icon
	if (apptRec.repeat)
		{
		chr = symbolRepeat;
		WinDrawChars (&chr, 1, x, y);
		x += FntCharWidth (chr) + 1;
		}

	FntSetFont (curFont);

	MemHandleUnlock (recordH);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewCheckForConflicts
 *
 * DESCRIPTION: This routine check the apointment list for conflicts
 *              (overlapping appointmens).
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    the number of columns of time bars necessary to display
 *              the conflicts
 *              
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/3/96	Initial Revision
 *
 ***********************************************************************/
static UInt16 DayViewCheckForConflicts (void)
{
	UInt16				i;
	UInt16				numColumns;
	UInt16				apptIndex;
	UInt16				width;
	TablePtr			table;
	TimeType			endTime [maxTimeBarColumns];
	ApptInfoPtr		appts;
	RectangleType	tableR;
	
	MemSet (endTime, sizeof (endTime), 0);

	numColumns = 1;

	appts = MemHandleLock (ApptsH);
	for (apptIndex = 0; apptIndex < NumAppts; apptIndex++)
		{
		if (appts[apptIndex].recordNum == emptySlot)
			continue;

		else if (TimeToInt (appts[apptIndex].startTime) == apptNoTime)
			continue;
		
		for (i = 0; i < maxTimeBarColumns; i++)
			{
			if (TimeToInt (appts[apptIndex].startTime) >= TimeToInt (endTime[i]))
				{
				endTime[i] = appts[apptIndex].endTime;
				if (i+1 > numColumns)
					numColumns = i+1;
				break;
				}
			}
		}
	MemPtrUnlock (appts);
	

	// Reserve spase for the time bars.  We will show time bars if the user
	// has requested them or if there are overlapping appointments.
	if (numColumns == 1 && (! ShowTimeBars))
		numColumns = 0;

	if (TimeBarColumns != numColumns)
		{
		TimeBarColumns = numColumns;
		table = GetObjectPtr (DayTable);
		
		// Set the width of the time bar table column.
		TblSetColumnWidth (table, timeBarColumn, numColumns * timeBarWidth);
		
		// Adjust the width of the description column.
		TblGetBounds (table, &tableR);
		width = tableR.extent.x - 
				  (numColumns * timeBarWidth) -
 				  TblGetColumnWidth (table, timeColumn) -
 				  TblGetColumnSpacing (table, timeColumn);
 		TblSetColumnWidth (table, descColumn, width);
 		
 		// Invalid the whole table since the positions of the time and 
 		// description columns has changed.
 		TblMarkTableInvalid (table);
		}

	return (numColumns);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDrawTimeBars
 *
 * DESCRIPTION: This routine draw the time bars the indicate the durations
 *              of apointments.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *              
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/3/96	Initial Revision
 *			rbb	4/9/99	Removed time bar for zero-duration appts
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22481.
 ***********************************************************************/
static void DayViewDrawTimeBars (void)
{
	Int16				i, j;
	Int16				apptIndex;
	Int16				numColumns;
	Int16				row;
	Int16				lineHeight;
	Int16				x, y1, y2;
	FontID			curFont;
	TablePtr			table;
	Boolean			drawTop;
	Boolean			drawBottom;
	Int16				endPoint [maxTimeBarColumns];
	TimeType			endTime [maxTimeBarColumns];
	ApptInfoPtr		appts;
	RectangleType	r;
	RectangleType	tableR;
	RectangleType	eraseR;
	FormPtr			frm;

	if (! TimeBarColumns) return;

	MemSet (endTime, sizeof (endTime), 0);
	numColumns = 1;

	frm = FrmGetFormPtr (DayView);
	if (! FrmVisible (frm)) return;
	
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));
	TblGetBounds (table, &tableR);

	for (i = 0; i < maxTimeBarColumns; i++)
		endPoint[i] = tableR.topLeft.y;

 	WinPushDrawState();
	curFont = FntSetFont (apptTimeFont);
	lineHeight = FntLineHeight ();
	FntSetFont (curFont);

	appts = MemHandleLock (ApptsH);
	for (apptIndex = 0; apptIndex < NumAppts; apptIndex++)
		{
		if (appts[apptIndex].recordNum == emptySlot)
			continue;
		else if (TimeToInt (appts[apptIndex].startTime) == apptNoTime)
			continue;
		
		for (i = 0; i < maxTimeBarColumns; i++)
			{
			if (i == 0)
				WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
			else
				WinSetForeColor(UIColorGetTableEntryIndex(UIWarning));
			
			if (TimeToInt (appts[apptIndex].startTime) >= TimeToInt (endTime[i]))
				{
				endTime[i] = appts[apptIndex].endTime;
				
				// Find the row that hold the appointment, it may not be
				// visible.
				if (TblFindRowID (table, apptIndex, &row))
					{
					TblGetItemBounds (table, row, descColumn, &r);
					y1 = r.topLeft.y + (lineHeight >> 1);
					drawTop = true;
					}

				// Is the appointment off the top of the display.
				else if (apptIndex < TopVisibleAppt)
					{
					y1 = tableR.topLeft.y;
					drawTop = false;
					}

				// If the appointment is below the bottom of the display we
				// don't draw anything.	
				else 
					break;

				// If the start time matches the end time we don't draw anything
				if ( TimeToInt (appts[apptIndex].startTime) ==
						TimeToInt (appts[apptIndex].endTime) )
					break;				

				// Find the row that contains the end time of the appointment.
				for (j = apptIndex + 1; j < NumAppts; j++)
					{
					// There may be more the one time slot with the time
					// we're searching for, get the last one.
					if (TimeToInt (appts[apptIndex].endTime) <=
						 TimeToInt (appts[j].startTime))
						break;
					}

				// Is the end-time visible.
				if (TblFindRowID (table, j, &row))
					{
					TblGetItemBounds (table, row, descColumn, &r);
					y2 = r.topLeft.y + (lineHeight >> 1);
					drawBottom = true;				
					}

				// Is the end of the appointment off the top of the display, if so
				// don't draw anything.
				else if (j < TopVisibleAppt)
					break;

				else
					{
					y2 = tableR.topLeft.y + tableR.extent.y - 1;
					drawBottom = false;				
					}

				x = tableR.topLeft.x + (i * timeBarWidth);

				// Erase the region between the top of the time bar we're
				// about to draw and the bottom of the previous time bar.
				if (y1 > endPoint[i])
					{
					eraseR.topLeft.x = x;
					eraseR.topLeft.y = endPoint[i];
					eraseR.extent.x = timeBarWidth;
					eraseR.extent.y = y1 - endPoint[i];
					WinEraseRectangle (&eraseR, 0);
					}
				endPoint[i] = y2 + 1;
				

				// Draw the time bar.
				WinEraseLine (x+1, y1+1, x+1, y2-2);

				if (drawTop) y1++;
				if (drawBottom) y2--;

				WinDrawLine (x, y1, x, y2);
				if (drawTop)
					WinDrawLine (x, y1, x + timeBarWidth - 1, y1);
				if (drawBottom)
					WinDrawLine (x, y2, x + timeBarWidth - 1, y2);
				
				if (i+1 > numColumns)
					numColumns = i+1;

				break;
				}
			}
		}
		
	// Erase the regions between the botton of the last time bar in 
	// each column and the bottom of the table.
	for (i = 0; i < numColumns; i++)
		{
		if (tableR.topLeft.y + tableR.extent.y > endPoint[i])
			{
			eraseR.topLeft.x = tableR.topLeft.x + (i * timeBarWidth);
			eraseR.topLeft.y = endPoint[i];
			eraseR.extent.x = timeBarWidth;
			eraseR.extent.y = tableR.topLeft.y + tableR.extent.y - endPoint[i];
			WinEraseRectangle (&eraseR, 0);
			}
		}
	
	MemPtrUnlock (appts);
 	WinPopDrawState();
	}


/***********************************************************************
 *
 * FUNCTION:    DayViewInsertAppointment
 *
 * DESCRIPTION: This routine inserts the record index of a new record 
 *              into the structure that keeps track of the appointment
 *              on the current day.
 *
 * PARAMETERS:  apptIndex - 
 *              recordNum - appointment record index
 *              row       - row in the table object of the new record
 *
 * RETURNED:   nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/28/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewInsertAppointment (UInt16 apptIndex, UInt16 recordNum, UInt16 row)
{
	UInt16 i;
	UInt32 uniqueID;
	TablePtr table;
	ApptInfoPtr appts;
	
	appts = MemHandleLock (ApptsH);

	// Adjust all the record index that are greater than the new record.
	for (i = 0; i < NumAppts; i++)
		{
		if ((appts[i].recordNum != emptySlot) && 
			 (appts[i].recordNum >= recordNum))
			appts[i].recordNum++;
		}

	appts[apptIndex].recordNum = recordNum;
//	appts[apptIndex].endTime.hours = appts[apptIndex].startTime.hours + 1;

	MemHandleUnlock (ApptsH);

	// Store the unique id of the record in the row.
	table = GetObjectPtr (DayTable);
	DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
	TblSetRowData (table, row, uniqueID);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewGetDescription
 *
 * DESCRIPTION: This routine returns a pointer to the description field
 *              of a appointment record.  This routine is called by 
 *              the table object as a callback routine when it wants to 
 *              display or edit the appointment's description.
 *
 * PARAMETERS:  table         - pointer to the Day View table (TablePtr)
 *              row           - row in the table
 *              column        - column in the table
 *              editable      - true if the field will be edited by the table
 *              textOffset    - offset within the record of the desc field (returned)
 *              textAllocSize - allocated size the the description field (returned)
 *
 * RETURNED:    handle of the appointment record
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *       ryw   2/18/00  Added casts to satisfy const cstring checking, should be safe
 *
 ***********************************************************************/
static Err DayViewGetDescription (void * table, Int16 row, Int16 column,
	Boolean editable, MemHandle * textH, Int16 * textOffset, 
	Int16 * textAllocSize, FieldPtr fld)
{
#pragma unused (column)

	Err error = 0;
	UInt16 apptIndex;
	UInt16 recordNum;
	Char* recordP;
	MemHandle recordH;
	Boolean redraw = false;
	FieldAttrType attr;
	ApptInfoPtr appts;
	ApptDBRecordType apptRec;
	ApptDateTimeType when;
//	UInt16 height;
//	UInt16 iconsWidth;
//	UInt16 tableHeight;
//	UInt16 columnWidth;
//	UInt32 uniqueID;
//	FontID fontID;
//	RectangleType r;
	
	*textH = 0;

	// Get the appoitment that corresponds to the table item.
	// The index of the appointment in the appointment list, is stored
	// as the row id.
	apptIndex = TblGetRowID (table, row);

	// Get the record index of the next appointment,  empty time slots 
	// have a minus one in the recordNum field.
	appts = MemHandleLock (ApptsH);
	recordNum = appts[apptIndex].recordNum;
	MemHandleUnlock (ApptsH);

	if (recordNum == emptySlot)
		{
		// If we're drawing the description, return a null MemHandle.
		if  (! editable) 
			return (0);

		// If we have reached the maximum number of displayable event, then
		// exit returning an error.
		if (NumApptsOnly >= apptMaxPerDay)
			return (-1);


		// If we're editing the description, create a new record.
		MemSet (&apptRec, sizeof (apptRec), 0);

		appts = MemHandleLock (ApptsH);
		when.startTime = appts[apptIndex].startTime;

		// If the start time is before 11:00 pm, the end time is one hour 
		// after the start time.
		if (when.startTime.hours < maxHours)
			{
			when.endTime.hours = when.startTime.hours + 1;
			when.endTime.minutes = when.startTime.minutes;
			}
			
		// If the start time is 11:00 pm or later, the end time is 11:55 pm. 
		else
			{
			when.endTime.hours = maxHours;
			when.endTime.minutes = maxMinutes;
			}

			// Don't let the new appointment overlap the next appointment.
		if (((apptIndex+1) < NumAppts) &&
			 (TimeToInt(when.endTime) > TimeToInt(appts[apptIndex+1].startTime)))
			{
			when.endTime = appts[apptIndex+1].startTime;
			appts[apptIndex].endTime = when.endTime;
			}

			// If the end time of the new event is not currently displayed, then
		// don't redraw the time bars.  We need to redraw the day, but we cannot
		// do it here.  We'll redraw the row on the tblSelect event.
		if (((apptIndex+1) == NumAppts) ||
			 (((apptIndex+1) < NumAppts) &&
				(TimeToInt(when.endTime) < TimeToInt(appts[apptIndex+1].startTime))))
			{
			redraw = true;
			TblMarkRowInvalid (table, row);
			}

		// If the description font is a different than the empty appointment font 
		// then we need to redraw the day, but we cannot
		// do it here.  We'll redraw the row on the tblSelect event.
		if (ApptDescFont != apptEmptyDescFont)
			{
			redraw = true;
			TblMarkRowInvalid (table, row);
			}

		MemHandleUnlock (ApptsH);

		when.date = Date;
		apptRec.when = &when;

		// Make sure the record has a description field so that we have
		// something to edit.
		apptRec.description = (char *)"";
		
		if (AlarmPreset.advance != apptNoAlarm)
			apptRec.alarm = &AlarmPreset;

		error = ApptNewRecord (ApptDB, &apptRec, &recordNum);

		if (error)
			{
			FrmAlert (DeviceFullAlert);
			return (error);
			}

		DayViewInsertAppointment (apptIndex, recordNum, row);

		// If the alarm preset preference is set we needed to reinitialize the
		// row so that the alarm icon will draw.  We don't redraw the row
		// here, we'll do that on the tblSelect event.
		if (AlarmPreset.advance != apptNoAlarm)
			{
			UInt32 ref;
			UInt32 trigger;
			UInt32 newAlarm;
			
			// If the new event's alarm will sound between now and the currently
			// registered alarm, the new one must be registered
			trigger = AlarmGetTrigger (&ref);
			newAlarm = ApptGetAlarmTime (&apptRec, TimGetSeconds (), true);

			if (newAlarm && ((newAlarm < trigger) || (trigger == 0)))
				{
				RescheduleAlarms (ApptDB);
				}

//			columnWidth = TblGetColumnWidth (table, descColumn);
//			TblGetBounds (table, &r);
//			tableHeight = r.extent.y;
//			height = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);
//			DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
//			appts = MemHandleLock (ApptsH);
//			DayViewInitRow (table, row, apptIndex, height, uniqueID, iconsWidth, fontID);
//			MemPtrUnlock (appts);

			TblMarkRowInvalid (table, row);
			}

		if (! redraw)
			DayViewDrawTimeBars ();
		}


	// Get the offset and length of the description field 
	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
	
	
	if (apptRec.description == NULL)
		{
		ApptDBRecordFlags changedFields;
		
		
		// Add the note to the record.
		MemSet (&changedFields, sizeof (changedFields), 0);
		changedFields.description = true;
		apptRec.description = (char *)"";
		error = ApptChangeRecord (ApptDB, &recordNum, &apptRec, changedFields);
		if (error) return error;
		
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
		}
	
	recordP = MemHandleLock(recordH);
	*textOffset = apptRec.description - recordP;
	*textAllocSize = StrLen (apptRec.description) + 1;  // one for null terminator
	*textH = recordH;
	MemHandleUnlock (recordH);
	MemHandleUnlock (recordH);  // MemHandle was also locked in ApptGetRecord

	// Set the field to support auto-shift.
	if (fld)
		{
		FldGetAttributes (fld, &attr);
		attr.autoShift = true;
		FldSetAttributes (fld, &attr);
		}

	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSaveDescription
 *
 * DESCRIPTION: This routine saves the description field of a appointment 
 *              to its db record.  This routine is called by the table 
 *              object, as a callback routine, when it wants to save
 *              the description.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw 
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static Boolean DayViewSaveDescription (void * table, Int16 row, Int16 column)
{
#pragma unused (column)

	UInt16 recordNum;
	UInt16 apptIndex;
	UInt16 selectStart, selectEnd;
	Boolean dirty;
	FieldPtr fld;
	ApptInfoPtr appts;
	
	
	// Get the appointment that corresponds to the table item passed.
	apptIndex = TblGetRowID (table, row);

	// Get the record index of the appointment,  empty time slots 
	// have a minus one in the recordNum field.
	appts = MemHandleLock (ApptsH);
	recordNum = appts[apptIndex].recordNum;
	MemHandleUnlock (ApptsH);

	ErrFatalDisplayIf ( (recordNum == emptySlot), "Error saving appointment");


	// If the description has been modified mark the record dirty,  any 
	// change made to the description were written directly to the 
	// record by the field object.
	fld = TblGetCurrentField (table);
	dirty = FldDirty (fld);
	if (dirty)
		DirtyRecord (ApptDB, recordNum);


	// Save the dirty state, we'll need it if we auto-delete an empty record.
	RecordDirty = dirty;

	// Check if the top of the description is scroll off the top of the 
	// field, if it is then redraw the field.
	if (FldGetScrollPosition (fld))
		{
		FldSetSelection (fld, 0, 0);
		FldSetScrollPosition (fld, 0);
		DayEditPosition = 0;
		DayEditSelectionLength = 0;
		}

	// Save the insertion point position, and length of the selection.  
	// We'll need the insertion point position an selection length
	// if we put the table back into edit mode.
	else
		{
		DayEditPosition = FldGetInsPtPosition (fld);
		
		FldGetSelection (fld, &selectStart, &selectEnd);
		DayEditSelectionLength = selectEnd - selectStart;
		if (DayEditSelectionLength)
			DayEditPosition = selectStart;
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewGetDescriptionHeight
 *
 * DESCRIPTION: This routine returns the height, in pixels, of the 
 *              description field of an appointment record.
 *
 * PARAMETERS:  apptIndex   - index in appointment list
 *              width       - width of the description column
 * 				 maxHeight   - the maximum height of the field
 *              iconsWidthP - space ot reserve for note, alarm and 
 *                            repeat icons
 *              fontIdP     - font id to draw the text with,
 *
 * RETURNED:    height in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *			peter	4/25/00	Allow multi-line masked records.
 *
 ***********************************************************************/
static Int16 DayViewGetDescriptionHeight (Int16 apptIndex, Int16 width, Int16 maxHeight,
	Int16 * iconsWidthP, FontID * fontIdP)
{
	Int16 height;
	Int16 iconsWidth;
	UInt16 recordNum;
	Int16 lineHeight;
	FontID curFont;
	MemHandle recordH;
	ApptInfoPtr	appts;
	ApptDBRecordType apptRec;
	UInt16 			attr;
	

	// Get the record index.
	appts = MemHandleLock (ApptsH);
	recordNum = appts[apptIndex].recordNum;
	MemPtrUnlock (appts);

	iconsWidth = 0;

	// Empty time slot?
	if (recordNum == emptySlot)
		{
		curFont = FntSetFont (apptTimeFont);
		height = FntLineHeight ();
		FntSetFont (curFont);
		*fontIdP = apptTimeFont;
		}
	else
		{	  
		// Get the appointment record.
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);


		DmRecordInfo (ApptDB, recordNum, &attr, NULL, NULL);
		// The following code is commented out since masked records are no longer limited
		// to one line. The reason for this is to keep masking and unmasking of individual
		// records from affecting the position of records on the screen.
		//		if (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
		//			{
		//			//masked
		//			curFont = FntSetFont (apptTimeFont);
		//			height = FntLineHeight ();
		//			*fontIdP = apptTimeFont;
		//			}
		//		else
			{
			//unmasked
			// Compute the width needed to draw the note, alarm and repeat icons.
			curFont = FntSetFont (symbolFont);

			if (apptRec.note)
				iconsWidth += FntCharWidth (symbolNote);

			if (apptRec.alarm)
				{
				if (iconsWidth) iconsWidth++;
				iconsWidth += FntCharWidth (symbolAlarm);
				}

			if (apptRec.repeat)
				{
				if (iconsWidth) iconsWidth++;
				iconsWidth += FntCharWidth (symbolRepeat);
				}


			// Compute the height of the appointment description.
			FntSetFont (ApptDescFont);
			
			height = FldCalcFieldHeight (apptRec.description, width - iconsWidth);
			lineHeight = FntLineHeight ();
			height = min (height, (maxHeight / lineHeight));
			height *= lineHeight;
			
			*fontIdP = ApptDescFont;
			}

		FntSetFont (curFont);

		MemHandleUnlock (recordH);
		}


	*iconsWidthP = iconsWidth;
	return (height);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the Day View scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frm             -  pointer to the to do Day form
 *              bottomAppt      -  record index of the last visible record
 *              lastItemClipped - true if the list is partially off the 
 *                                display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/28/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewUpdateScrollers (FormPtr frm, UInt16 bottomAppt,
	Boolean lastItemClipped)
{
	UInt16 upIndex;
	UInt16 downIndex;
	Boolean scrollableUp;
	Boolean scrollableDown;
	
	// If the first appointment displayed is not the fist appointment
	// of the day, enable the up scroller.
	scrollableUp = (TopVisibleAppt > 0);

	// If the last appointment displayed is not the last appointment
	// of the day or if it partially clipped, enable the down scroller.
	scrollableDown = ( lastItemClipped || (bottomAppt+1 < NumAppts) );

	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frm, DayUpButton);
	downIndex = FrmGetObjectIndex (frm, DayDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewInitRow
 *
 * DESCRIPTION: This routine initializes a row in the Day View table.
 *
 * PARAMETERS:  table      - pointer to the table of appointments
 *              row        - row number (first row is zero)
 *              apptIndex  - index in appointment list
 *              rowHeight  - height of the row in pixels
 *              uniqueID   - unique ID of appointment record
 *              iconsWidth - spase to reserve for note, alarm and repeat
 *                           icons
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewInitRow (TablePtr table, UInt16 row, UInt16 apptIndex, 
	Int16 rowHeight, UInt32 uniqueID, UInt16 iconsWidth, FontID fontID)
{
	UInt16 time;
	ApptInfoPtr	appts;	

	// Make the row usable.
	TblSetRowUsable (table, row, true);
	
	// Set the height of the row to the height of the description.
	TblSetRowHeight (table, row, rowHeight);
	
	// Store the record number as the row id.
	TblSetRowID (table, row, apptIndex);
	
	// Store the start time of the appointment in the table.
	appts = MemHandleLock(ApptsH);
	time = TimeToInt(appts[apptIndex].startTime);
	MemHandleUnlock(ApptsH);
	TblSetItemInt (table, row, timeColumn, time);
	#if WRISTPDA
	TblSetItemFont( table, row, timeColumn, FossilLargeFontID( WRISTPDA, fontID ) );
	#endif
	
	// Store the unique id of the record in the row.
	TblSetRowData (table, row, uniqueID);

	// Set the table item type for the description,  it will differ depending
	// on the presence of a note.
	if (! iconsWidth)
		{
		TblSetItemStyle (table, row, descColumn, textTableItem);		
		TblSetItemInt (table, row, descColumn, 0);
		}
	else
		{
		TblSetItemStyle (table, row, descColumn, narrowTextTableItem);
		TblSetItemInt (table, row, descColumn, iconsWidth);
		}


	// Set the font used to draw the text of the row.
	TblSetItemFont (table, row, descColumn, fontID);

	// Mark the row invalid so that it will draw when we call the 
	// draw routine.
	TblMarkRowInvalid (table, row);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewLoadTable
 *
 * DESCRIPTION: This routine reloads appointment database records into
 *              the Day view.  This routine is called when:
 *              	o A new item is inserted
 *              	o An item is deleted
 *              	o The time of an items is changed
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *			peter	4/25/00	Add support for unmasking only the selected record.
 ***********************************************************************/
static void DayViewLoadTable (void)
{
	Int16 apptIndex;
	Int16 row;
	UInt16 numRows;
	UInt16 recordNum;
	UInt16 lastAppt;
	Int16 iconsWidth;
	Int16 lineHeight;
	Int16 dataHeight;
	Int16 tableHeight;
	Int16 columnWidth;
	UInt16 pos, oldPos;
	UInt16 height, oldHeight;
	UInt32	uniqueID;
	FontID fontID;
	FontID currFont;
	FormPtr frm;
	TablePtr table;
	Boolean init;
	Boolean rowUsable;
	Boolean rowsInserted = false;
	Boolean lastItemClipped;
	ApptInfoPtr	appts;
	RectangleType r;
	UInt16 attr;
	Boolean masked;
	privateRecordViewEnum visualStatus;	

	appts = MemHandleLock (ApptsH);

	frm = FrmGetFormPtr (DayView);
	
	// Get the height of the table and the width of the description
	// column.
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (table, descColumn);


	// If we currently have a selected record, make sure that it is not
	// above the first visible record.
	if (CurrentRecord != noRecordSelected)
		{
		apptIndex = DayViewFindAppointment (CurrentRecord);
		if (apptIndex < TopVisibleAppt)
			TopVisibleAppt = apptIndex;
		}

	if (TopVisibleAppt >= NumAppts)			// Fix: sometimes, TopVisibleAppt is greater than NumAppt
		TopVisibleAppt = NumAppts - 1;		//      which generates an out-of-bounds crash later in code
		
	apptIndex = TopVisibleAppt;
	lastAppt = apptIndex;

	// Load records into the table.
	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;

	while (apptIndex < NumAppts)
		{		
		// Compute the height of the appointment's description.
		height = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);

		// Is there enought room for at least one line of the the decription.
		currFont = FntSetFont (fontID);
		lineHeight = FntLineHeight ();
		FntSetFont (currFont);
		if (tableHeight >= dataHeight + lineHeight)
			{
			// Get the height of the current row.
			rowUsable = TblRowUsable (table, row);
			if (rowUsable)
				oldHeight = TblGetRowHeight (table, row);
			else
				oldHeight = 0;


			// Determine if the row needs to be initialized.  We will initialize 
			// the row if: the row is not usable (not displayed),  the unique
			// id of the record does not match the unique id stored in the 
			// row, or if the start time of the appointment does not match the
			// start time stored in the table.
			init = (! rowUsable);
			uniqueID = 0;
			masked = false;
			recordNum = appts[apptIndex].recordNum;
			if (recordNum != emptySlot)	// empty time slot?
				{
				DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
				init |= TblGetRowData (table, row) != uniqueID;
				
				//Mask if appropriate
				visualStatus = recordNum == CurrentRecord
					? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
				DmRecordInfo (ApptDB, recordNum, &attr, NULL, NULL);
		   	masked = (((attr & dmRecAttrSecret) && visualStatus == maskPrivateRecords));	
				}
			else if (! init)
				init |= (TblGetRowData (table, row) != 0);	
			
			if (masked != TblRowMasked(table,row))
				TblMarkRowInvalid (table, row);
				
			TblSetRowMasked(table,row,masked);
		

			if (! init)
				init = TimeToInt (appts[apptIndex].startTime) != 
					    TblGetItemInt (table, row, timeColumn);
					    
			if (! init)
				init = TblGetItemInt (table, row, descColumn) != iconsWidth;

			if (! init)
				init = TblGetItemFont (table, row, descColumn) != fontID;

			// If the record is not already being displayed in the current 
			// row load the record into the table.
			if (init)
				{
				DayViewInitRow (table, row, apptIndex, height, uniqueID, iconsWidth, fontID);
				}

			// If the height or the position of the item has changed draw the item.
			else 
				{
				TblSetRowID (table, row, apptIndex);
				if (height != oldHeight)
					{
					TblSetRowHeight (table, row, height);
					TblMarkRowInvalid (table, row);
					}
				else if (pos != oldPos)
					{
					TblMarkRowInvalid (table, row);
					}
				}
				
			pos += height;
			oldPos += oldHeight;

			lastAppt = apptIndex;
			apptIndex++;
			row++;
			}
		
		dataHeight += height;

		// Is the table full?
		if (dataHeight >= tableHeight)		
			{
			// If we have a currently selected record, make sure that it is
			// not below the last visible record.  If the currently selected 
			// record is the last visible record, make sure the whole description 
			// is visible.
			if (CurrentRecord == noRecordSelected) break;

			apptIndex = DayViewFindAppointment (CurrentRecord);
			if (apptIndex < lastAppt)
				 break;

			// Last visible?
			else if (apptIndex == lastAppt)
				{
				if ((apptIndex == TopVisibleAppt) || (dataHeight == tableHeight))
					break;
					
				// Remove the top item from the table and reload the table again.
				TopVisibleAppt++;
				apptIndex = TopVisibleAppt;
				}
			// Below last visible.
			else
				TopVisibleAppt = apptIndex;
				
			row = 0;
			dataHeight = 0;
			oldPos = pos = 0;
			}
		}



	// Hide the items that don't have any data.
	numRows = TblGetNumberOfRows (table);
	while (row < numRows)
		{		
		TblSetRowUsable (table, row, false);
		row++;
		}


	// If the table is not full and the first visible record is 
	// not the first record	in the database, displays enough records
	// to fill out the table.
	while (dataHeight < tableHeight)
		{
		apptIndex = TopVisibleAppt;
		if (apptIndex == 0) break;
		apptIndex--;

		height = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);
			
		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;
		
		// Insert a row before the first row.
		TblInsertRow (table, 0);

		recordNum = appts[apptIndex].recordNum;
		masked = false;
		if (recordNum != emptySlot)	// empty time slot?
			{
			DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
			//mask if appropriate
			DmRecordInfo (ApptDB, recordNum, &attr, NULL, NULL);
	   		masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));
			}
		else
			uniqueID = 0;		
				
		TblSetRowMasked(table,0,masked);


		DayViewInitRow (table, 0, apptIndex, height, uniqueID, iconsWidth, fontID);
		
		TopVisibleAppt = apptIndex;
		
		rowsInserted = true;

		dataHeight += height;
		}
		
	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		TblMarkTableInvalid (table);

	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clip and the 
	// table is scrollable.
	lastItemClipped = (dataHeight > tableHeight);

	// Update the scroll arrows.
	DayViewUpdateScrollers (frm, lastAppt, lastItemClipped);

	MemPtrUnlock (appts);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewLayoutDay
 *
 * DESCRIPTION: This routine builds a list of: untimed appointment, 
 *              timed appointment, and empty time slots, for the 
 *              current day (the date store in the global variable Date).
 *
 * PARAMETERS:  retrieve - true if the list if appointment should be 
 *
 * RETURNED:    nothing
 *
 * NOTE:			the global variables ApptsH and NumAppts are set by
 *             this routine.
 *              
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/96	Initial Revision
 *			rbb	4/9/99	Don't use extra line when end time matches start
 *			rbb	5/18/99	Events ending on the hour caused display to skip an hour
 *			jmp	10/7/99	Replace GetObjectPtr() with FrmGetFormPtr() and 
 *								FrmGetObjectIndex(); fixes bug #22548.
 *
 ***********************************************************************/
static void DayViewLayoutDay (Boolean retieve)
{
	Int16				i, j;
	Int16				index;
	Int16				numRows;
	Int16				height;
	Int16				lineHeight;
	Int16				iconsWidth;
	Int16				tableHeight;
	Int16				columnWidth;
	FontID			fontID;
	FontID			currFont;
	TablePtr			table;
	TimeType			next;
	TimeType			endTime;
	Boolean			replace;
	Boolean			addEndTime;
	ApptInfoPtr		appts;
	ApptInfoPtr		apptsOnly;
	RectangleType 	r;
	FormPtr			frm;
	

	// Get a list of: untimed appointment and timed appointment for the
	// current day.
	if (retieve)
		{
		if (ApptsOnlyH) MemHandleFree (ApptsOnlyH);
		ApptGetAppointments (ApptDB, Date, 1, &ApptsOnlyH, &NumApptsOnly);
		}


	// Free the existing list.
	if (ApptsH)
		MemHandleFree (ApptsH);

	// If there are no appointsment on the day, fill in the appointment list
	// with empty time slots.
	if (! ApptsOnlyH)
		{
		NumAppts = DayEndHour - DayStartHour + 1;
		ApptsH = MemHandleNew (NumAppts * sizeof (ApptInfoType));
		appts = MemHandleLock (ApptsH);

		for (i = 0; i < NumAppts; i++)
			{
			appts[i].startTime.hours = DayStartHour + i;				
			appts[i].startTime.minutes = 0;				
			appts[i].endTime.hours = DayStartHour + i + 1;				
			appts[i].endTime.minutes = 0;				
			appts[i].recordNum = emptySlot;	
			}
			
		DayViewCheckForConflicts ();
		MemHandleUnlock (ApptsH);	
		return;
		}
	
	// Merge empty time slots into the appointment list.
	//
	// Allocate space for the maximun number of empty time slots that
	// we may need to add. 
	ApptsH = MemHandleNew ((NumApptsOnly+(hoursPerDay*2)) * sizeof (ApptInfoType));
	appts = MemHandleLock (ApptsH);
	NumAppts = 0;
	index = 0;

	// Add the untimed events, the timed events, and a blank time slot for 
	// the end-time of each timed event.
	apptsOnly = MemHandleLock (ApptsOnlyH);
	for (i = 0; i < NumApptsOnly; i++)
		{
		// Find the correct position at which to insert the current event.
		replace = false;
		for (j = index; j < NumAppts; j++)
			{
			if (appts[j].recordNum == emptySlot)
				{
				// If an empty time slot with the same start-time already exist, then
				// replace it.
				if (TimeToInt (appts[j].startTime) == TimeToInt (apptsOnly[i].startTime))
					{
					replace = true;
					break;
					}

				// If we find an empty time slot that has an start-time before
				// the start-time of the current event and an end-time after the 
				// the start-time of the current event, adjust the end-time of the 
				// empty time such that it is equal to the start of the current event.
			 	if (TimeToInt (appts[j].startTime) < TimeToInt (apptsOnly[i].startTime) &&
			 		 TimeToInt (appts[j].endTime)   > TimeToInt (apptsOnly[i].startTime))
			 		appts[j].endTime = apptsOnly[i].startTime;
			 	}
			
			if (TimeToInt (appts[j].startTime) > TimeToInt (apptsOnly[i].startTime))
				{
				// Make room for the empty time slot we're about to add.
				MemMove (&appts[j+1], &appts[j], (NumAppts-j) * sizeof (ApptInfoType));
				break;
				}
			}

		// Add the event to the list.
		appts[j] = apptsOnly[i];
		index = j + 1;
		if (! replace)
			NumAppts++;
			

		// If the event is a timed event add an empty time slot to display the end-time.
		// If the event has no duration, skip it to avoid displaying the same time twice.
		if ( (TimeToInt (apptsOnly[i].startTime) != apptNoTime)
				&& (TimeToInt (apptsOnly[i].startTime) != TimeToInt (apptsOnly[i].endTime)) )
			{
			// Find the correct position at which to insert the end-time time slot.
			addEndTime = true;
			for (j = index; j < NumAppts; j++)
				{
				// If an event already exist that has a start-time equal to the 
				// end-time of the current event then we don't need to add an 
				//  end-time time slot.
				if (TimeToInt (appts[j].startTime) == TimeToInt (apptsOnly[i].endTime))
					{
					addEndTime = false;
					break;
					}
					
				// We're found the position to insert the empty time slot when we find 
				// an appointment with a start-time greater than the end-time of the 
				// current event.
				if (TimeToInt (appts[j].startTime) > TimeToInt (apptsOnly[i].endTime))
					{
					// Make room for the empty time slot we're about to add.
					MemMove (&appts[j+1], &appts[j], (NumAppts-j) * sizeof (ApptInfoType));
					break;
					}
				}
			
			if (addEndTime)
				{
				// The end time of the empty time slot is the earlier of:
				//		o the start time plus one hour
				//		o 11:55 pm
				//		o the start time of the next event.
				if (apptsOnly[i].endTime.hours < 23)
					{
					endTime.hours = apptsOnly[i].endTime.hours + 1;
					endTime.minutes = 0;
//					endTime.minutes = apptsOnly[i].endTime.minutes;
					}
				else
					{
					endTime.hours = 23;		// max end time is 11:55 pm
					endTime.minutes = 55;
					}
	
				if (j < NumAppts && TimeToInt(endTime) > TimeToInt(appts[j+1].startTime))
					endTime = appts[j+1].startTime;
		
	
				appts[j].recordNum = emptySlot;
				appts[j].startTime = apptsOnly[i].endTime;
				appts[j].endTime = endTime;
				NumAppts++;
				}
			}
		}


	// Reserve spase for the time bars.  We will show time bars if the user
	// has requested them or if there are overlapping appointments.
	DayViewCheckForConflicts ();


	// Determine if that is space to add empty time slot, these time slot are in
	// addition to the empty time slot that represent end times of events.
	frm = FrmGetFormPtr (DayView);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));
	numRows = TblGetNumberOfRows (table);
	if (( ! CompressDayView) || (numRows > NumAppts))
		{
		TblGetBounds (table, &r);
		tableHeight = r.extent.y;
		height = 0;
		columnWidth = TblGetColumnWidth (table, descColumn);

		currFont = FntSetFont (apptEmptyDescFont);
		lineHeight = FntLineHeight ();
		FntSetFont (currFont);

		if (CompressDayView)
			{
			for (j = 0; j < NumAppts; j++)
				{
				height += DayViewGetDescriptionHeight (j, columnWidth, tableHeight, &iconsWidth, &fontID);
				if (height >= tableHeight)
					break;
				}
			}
		
		// Add empty time slots to the list of appointment until the table is full.
		next.hours = DayStartHour;
		next.minutes = 0;
		i = 0;

		while (( ! CompressDayView) || (height + lineHeight <= tableHeight))
			{
			if ((i < NumAppts) &&
				 (TimeToInt (next) >= TimeToInt (appts[i].startTime)))
				{
				if (TimeToInt (next) <= TimeToInt (appts[i].endTime) &&
					(appts[i].endTime.hours >= DayStartHour))
					{
					next = appts[i].endTime;
					if (next.minutes || (TimeToInt (appts[i].startTime)
													== TimeToInt (appts[i].endTime)))
						{
						next.hours++;
						next.minutes = 0;
						}
					}
				i++;
				}

			// Insert an empty time slot if we're not passed the end of the 
			// day.
			else if ((next.hours < DayEndHour) || 
						((next.hours == DayEndHour) && next.minutes == 0))
				{
				MemMove (&appts[i+1], &appts[i], (NumAppts-i) * sizeof (ApptInfoType));
				NumAppts++;
	
				appts[i].startTime = next;
				appts[i].recordNum = emptySlot;	
	
				// The end time is the beginning of the next hour or the 
				// start time of the next appointment, which ever is earliest.
				next.hours++;
				next.minutes = 0;
				if ( (i+1 < NumAppts) &&
					  (TimeToInt (next) > TimeToInt (appts[i+1].startTime)))
					next = appts[i+1].startTime;
	
				appts[i].endTime = next;
				
				height += DayViewGetDescriptionHeight (i, columnWidth, tableHeight, &iconsWidth, &fontID);
				i++;
				}

			else if (i < NumAppts)
				{
				next.hours++;
				next.minutes = 0;
				}
			
			else
				break;
			}
		}
				
	MemHandleUnlock (ApptsOnlyH);
			
	// Release any unused space in the appointment list;
	MemHandleUnlock (ApptsH);
	MemHandleResize (ApptsH, (NumAppts * sizeof (ApptInfoType)));
}


/***********************************************************************
 *
 * FUNCTION:    DayViewNewAppointment
 *
 * DESCRIPTION: This routine adds a new untimed appointment to the top of
 *              current day.
 *
 * PARAMETERS:  event - pointer to the keyDown event, or NULL
 *
 * RETURNED:    true if the key has handle by this routine
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/29/95		Initial Revision
 *
 ***********************************************************************/
static Boolean DayViewNewAppointment (EventType* event)
{
	Err error;
	Int16 row;
	Int16 apptIndex;
	UInt16 recordNum;
	Char desc [2];
	FormPtr frm;
	TablePtr table;
	TimeType startTime;
	TimeType endTime;
	Boolean handled = false;
	ApptDBRecordType newAppt;
	ApptDateTimeType when;
	

	//TimeToInt (startTime) = apptNoTime;
	startTime.hours = 0xff;
	startTime.minutes = 0xff;
	//TimeToInt (endTime) = apptNoTime;
	endTime.hours = 0xff;
	endTime.minutes = 0xff;
	*desc = 0;

	// We'll have an event if the appointment is being created as the result
	// writing a character when no appointment is selected.
	if (event)
		{
		if (TxtCharIsDigit (event->data.keyDown.chr))
			{
			EvtAddEventToQueue (event);
			
			startTime.hours = event->data.keyDown.chr - '0';
			startTime.minutes = 0;
			endTime.hours = startTime.hours + 1;
			endTime.minutes = 0;
			if (!GetTime (&startTime, &endTime, setTimeTitleStrID))
				return (true);
			}

		// Convert lower case alpha character to upper-case.
		else
			{
			desc[0] = event->data.keyDown.chr;
			desc[1] = 0;
			if ((UInt8)desc[0] >= 'a' && (UInt8)desc[0] <= 'z')
				desc[0] -= ('a' - 'A');
			}
		handled = true;
		}


	// Limit the number of appointments that can be enter on a day.
	if (NumApptsOnly >= apptMaxPerDay)
		return (true);

	// Create a untimed appointment on the current day.
	MemSet (&newAppt, sizeof (newAppt), 0);
	when.startTime = startTime;
	when.endTime = endTime;
	when.date = Date;
	newAppt.when = &when;
	newAppt.description = desc;
	if (AlarmPreset.advance != apptNoAlarm)
		newAppt.alarm = &AlarmPreset;
	error = ApptNewRecord (ApptDB, &newAppt, &recordNum);
	
	// If necessary display an alert that indicates that the new record could 
	// not be created.
	if (error)
		{
		FrmAlert (DeviceFullAlert);
		return (true);
		}

	CurrentRecord = recordNum;
	ItemSelected = true;

	frm = FrmGetActiveForm ();
	table = GetObjectPtr (DayTable);
	
	// Reload and redraw the appointment table.
	DayViewLayoutDay (true);
	DayViewLoadTable ();
	TblRedrawTable (table);
	DayViewDrawTimeBars ();


	// Give the focus to the new item.
	apptIndex = DayViewFindAppointment (recordNum);
	TblFindRowID (table, apptIndex, &row);
	FrmSetFocus (frm, FrmGetObjectIndex (frm, DayTable));
	TblGrabFocus (table, row, descColumn);
	FldGrabFocus (TblGetCurrentField (table));
	
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDeleteAppointment
 *
 * DESCRIPTION: This routine deletes the selected appointment.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95		Initial Revision
 *
 ***********************************************************************/
static void DayViewDeleteAppointment (void)
{
	Int16 row;
	Int16 column;
	Int16 apptIndex;
	UInt16 recordNum;
	TablePtr table;
	ApptInfoPtr	appts;

		
	// If appointment is selected, return.
	table = GetObjectPtr (DayTable);
	if (! TblEditing (table))
		return;

	TblGetSelection (table, &row, &column);
	TblReleaseFocus (table);


	// Get the record index.
	apptIndex = TblGetRowID (table, row);
	appts = MemHandleLock (ApptsH);
	recordNum = appts[apptIndex].recordNum;
	MemHandleUnlock (ApptsH);
	
	ErrFatalDisplayIf ((recordNum != CurrentRecord), "Wrong record");

	// Clear the edit state, this will delete the current record if is 
	// blank.
	if (! ClearEditState ())
		{
		// Delete the record,  this routine will display an appropriate 
		// dialog to confirm the action.  If the dialog is canceled 
		// don't update the display.
		if (! DeleteRecord (recordNum))
			{
				FrmUpdateForm (DayView, 0);		// Re-masks the record if necessary.
				return;
			}
		}

	// Layout the day again, empty time slots may needed to be add to 
	// full-in the gap left by the deleted appointment.
	DayViewLayoutDay (true);

	DayViewLoadTable ();
	TblRedrawTable (table);
	DayViewDrawTimeBars ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDeleteNote
 *
 * DESCRIPTION: This routine deletes the note attached to the selected 
 *              appointment.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void DayViewDeleteNote ()
{
	UInt16 i;
	Int16 row;
	Int16 column;
	Int16 height;
	Int16 tableHeight;
	Int16 iconsWidth;
	Int16 newHeight;
	Int16 apptIndex;
	FormPtr alert;
	UInt16 alertButton;
	Int16 columnWidth;
	UInt16 rowsInTable;
	FontID fontID;
	TablePtr table;
	Boolean empty;
	Boolean exception = false;
	Boolean splitEvent = false;
	MemHandle recordH;
	RectangleType r;
	ApptDBRecordType apptRec;
		
	table = GetObjectPtr (DayTable);

	// Check if we are editing an item.
	if (! TblEditing (table))
		return;

	// Check if the record has a note attached.
	ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
	empty = ( (! apptRec.note) || (*apptRec.note == 0));
	MemHandleUnlock (recordH);
	if (empty) return;


	// If we're changing a repeating appointmemt, check if all occurrences
	// are being changed, or if and exception is being created.
	if (apptRec.repeat)
		{
		alert = FrmInitForm (RangeDialog);
		alertButton = FrmDoDialog (alert);
		FrmDeleteForm (alert);
		
		if (alertButton == RangeCancelButton)
			return;
			
		else if (alertButton == RangeCurrentButton)
			exception = true;
			
		else if (alertButton == RangeFutureButton)
			splitEvent = true;
		}

	else if (FrmAlert(DeleteNoteAlert) != DeleteNoteYes)
		return;


	// Get the selected row.
	TblGetSelection (table, &row, &column);

	// Release the focus, this will unlock the current record.
	TblReleaseFocus (table);

	// Remove the note field from the database record.
	if (! DeleteNote (exception, splitEvent))
		return;

	// Mark the current row non-usable so the it will redraw.
	TblSetRowUsable (table, row, false);
	
	// Get the current height of the description.
	height = TblGetRowHeight (table, row);

	// Get the new height of the description, the desciption may be short
	// because we can draw in the space vacated by the note indicator.
	apptIndex = TblGetRowID (table, row);
	columnWidth = TblGetColumnWidth (table, descColumn);
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	newHeight = DayViewGetDescriptionHeight (apptIndex, columnWidth, tableHeight, &iconsWidth, &fontID);
	
	// If the height of the description has changed, invalid all the row
	// after the current row so that they be drawn.
	if (height != newHeight)
		{
		rowsInTable = TblGetNumberOfRows (table);
		for (i = row+1; i < rowsInTable; i++)
			TblSetRowUsable (table, i, false);
		}
	
	// Creating an exception will move records around, so we need to rebuild
	// the appointment list.
	if (exception)
		DayViewLayoutDay (true);

	DayViewLoadTable ();
	TblRedrawTable (table);
	DayViewDrawTimeBars ();

	DayViewRestoreEditState ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSelectTime
 *
 * DESCRIPTION: This routine is called when a time item in the day view
 *              table is selected.  The time picker is displayed, if 
 *              the start or end time of the appointment is changed, 
 *              the day's appointments are resorted and the appointment
 *              table is redrawn.
 *
 * PARAMETERS:  table - table of appointments
 *              row   - row selected
 *              
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95		Initial Revision
 *
 ***********************************************************************/
static void DayViewSelectTime (TablePtr table, UInt16 row)
{
	UInt16 apptIndex;
	UInt16 recordNum;
	Boolean userConfirmed;
	Boolean moved = false;
	TimeType startTime;
	TimeType endTime;
	ApptInfoPtr appts;

	// Get the record index, start time, and end time.
	apptIndex = TblGetRowID (table, row);
	appts = MemHandleLock (ApptsH);
	startTime = appts[apptIndex].startTime;
	endTime = appts[apptIndex].endTime;
	recordNum = appts[apptIndex].recordNum;
	MemHandleUnlock (ApptsH);

	// Display the time picker
	userConfirmed = GetTime (&startTime, &endTime, setTimeTitleStrID);

	// if the user has confirmed the dialog pass the information to MoveEvent
	if (userConfirmed)
		{
		if (MoveEvent (&recordNum, startTime, endTime, Date, true, &moved))
			return;
		}
	
	// The display needs to be udpated when the user just cancels the GetTime dialog
	// as well as when an event is actually changed because just tapping on the time
	// of an empty slot causes a new "empty" event to be created and the view needs 
	// to be updated to take care of time bars & alarm icon (when the preset is set)
	if ((!userConfirmed) || moved)
		{
		CurrentRecord = recordNum;	
		DayViewLayoutDay (true);
		DayViewLoadTable ();
		TblRedrawTable (table);
		DayViewDrawTimeBars ();
		}

	DayViewRestoreEditState ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSelectIconRect
 *
 * DESCRIPTION: This routine handles the selection of the note, alarm 
 *              and repeat icons.  If determains which icon the pen is
 *              on and tracks the pen until it is released.  If the pen 
 *              is released within the bounds of an icon we nagivate to
 *              apprpperiate form,
 *
 * PARAMETERS:  table	 - pointer to DayView table
 *					 row		 - row in table DayView icon is in
 *					 r			 - DayView's icon's bounds
 *					 selected - whether to draw the icon selected or unselected
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			jmp	10/31/99	Initial Revision
 *			jmp	11/06/99	Oops, was just doing a WinResetClip() instead of
 *								save/restore WinSetClip().
 *
 ***********************************************************************/
static void DayViewSelectIconRect(TablePtr table, Int16 row, RectangleType *r, Boolean selected)
{
	RectangleType savedClip;
	
	WinGetClip(&savedClip);
	
	// The table selection code will select or unselect the entire item as
	// the icons are part of a custom item for the Datebook.  So, we restrict
	// what the table-selection code will do by clipping the rectangle to
	// the specific icon we want selected or unselected.
	//
	WinSetClip(r);
	
	// Select or unselect the icon as requested.
	//
	if (selected)
		TblSelectItem(table, row, descColumn);
	else
		TblUnhighlightSelection(table);

	// Restore clipping.
	//
	WinSetClip(&savedClip);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSelectIcons
 *
 * DESCRIPTION: This routine handles the selection of the note, alarm 
 *              and repeat icons.  If determains which icon the pen is
 *              on and tracks the pen until it is released.  If the pen 
 *              is released within the bounds of an icon we nagivate to
 *              apprpperiate form,
 *
 * PARAMETERS:  event  - pointer to a tblEnter event.
 *
 * RETURNED:    true if an icon is selected.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/16/96	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *			jmp	10/31/99 Replace calls to WinInvertRectangle() with calls to
 *								DayViewSelectIconRect().
 *			peter	6/12/00	Don't allow icon selection in masked rows.
 *
 ***********************************************************************/
static Boolean DayViewSelectIcons (EventType* event)
{
	UInt16			apptIndex;
	UInt16			row;
	UInt16			recordNum;
	UInt16			iconsWidth;
	Int16				x, y;
	FontID			curFont;
	MemHandle 		recordH;
	TablePtr			table;
	Boolean			penDown = true;
	Boolean			selected = true;
	Boolean			inNoteIcon = false;
	Boolean			inAlarmIcon = false;
	Boolean			inRepeatIcon = false;
	ApptInfoPtr		appts;
	RectangleType	r;
	RectangleType	itemR;
	ApptDBRecordType apptRec;

	if (event->data.tblEnter.column != descColumn)
		return (false);

	// Check if there are any icons.
	table = (TablePtr) event->data.tblEnter.pTable;
	row = event->data.tblEnter.row;
	if (TblRowMasked(table,row))
		return (false);
	iconsWidth = TblGetItemInt (table, row, descColumn);
	if (! iconsWidth)
		return (false);
	
	// Check if the pen is within the bounds of the icons.
	TblGetItemBounds (table, row, descColumn, &itemR);

	r.topLeft.x = itemR.topLeft.x + itemR.extent.x - iconsWidth;
	r.topLeft.y = itemR.topLeft.y;
	r.extent.x = iconsWidth;
	curFont = FntSetFont (ApptDescFont);
	r.extent.y = FntLineHeight ();
	FntSetFont (curFont);
	
	x = event->screenX;
	y = event->screenY;
	
	if (RctPtInRectangle (x, y, &r))
		{
		// Get the record index of the selected appointment.
		apptIndex = TblGetRowID (table, row);
		appts = MemHandleLock (ApptsH);
		recordNum = appts[apptIndex].recordNum;
		MemPtrUnlock (appts);

		// Get the appointment record.
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);

		curFont = FntSetFont (symbolFont);
		
		// Check if the pen is on the note icon.
		if (apptRec.note)
			{
			r.extent.x = FntCharWidth (symbolNote);
			inNoteIcon = RctPtInRectangle (x, y, &r);
			if (! inNoteIcon)
				r.topLeft.x += r.extent.x + 1;
			}

		// Check if the pen is on the alarm icon.
		if (apptRec.alarm && (! inNoteIcon))
			{
			r.extent.x = FntCharWidth (symbolAlarm);
			inAlarmIcon = RctPtInRectangle (x, y, &r);
			if (! inAlarmIcon)
				r.topLeft.x += r.extent.x + 1;
			}

		// Check if the pen is on the repeat icon.
		if (apptRec.repeat && (! inNoteIcon) && (! inAlarmIcon) )
			{
			r.extent.x = FntCharWidth (symbolRepeat);
			inRepeatIcon = RctPtInRectangle (x, y, &r);
			if (! inRepeatIcon)
				r.topLeft.x += r.extent.x  + 1;
			}
			
		FntSetFont (curFont);
		MemHandleUnlock (recordH);

		TblReleaseFocus (table);

		// Highlight the icon.
		DayViewSelectIconRect(table, row, &r, selected);

		// Track the pen until it's released.
		do 
			{
			PenGetPoint (&x, &y, &penDown);
	
			if (RctPtInRectangle (x, y, &r))
				{
				if (!selected)
					{
					selected = true;
					DayViewSelectIconRect(table, row, &r, selected);
					}
				}
	
			else if (selected)
				{
				selected = false;
				DayViewSelectIconRect(table, row, &r, selected);
				}
	
			} while (penDown);

		if (selected)
			{
			selected = false;
			DayViewSelectIconRect(table, row, &r, selected);

			CurrentRecord = recordNum;

			if (inNoteIcon)
				FrmGotoForm (NewNoteView);
			else
				FrmPopupForm (DetailsDialog);
			return (true);
			}
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewItemSelected
 *
 * DESCRIPTION: This routine is called when an item in Day View table
 *              is selected.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *       ryw   2/18/00  Added cast to satisfy const cstring checking, should be safe
 *			peter	4/25/00	Add support for un-masking just the selected record.
 *
 ***********************************************************************/
static void DayViewItemSelected (EventType* event)
{
	Err error;
	UInt16 apptIndex;
	UInt16 row;
	UInt16 column;
	UInt16 recordNum;
	UInt32 uniqueID;
	TablePtr table;
	ApptInfoPtr appts;
	ApptDBRecordType newAppt;
	ApptDateTimeType when;
	EventType newEvent;
	UInt16 systemVolume, mutedVolume = 0;
	
	table = event->data.tblSelect.pTable;
	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;

	if (TblRowMasked(table,row))
		{
		if (SecVerifyPW (showPrivateRecords) == true)
			{
			// We only want to unmask this one record, so restore the preference.
			PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);
			
			// Unmask just the current row.
			TblSetRowMasked(table, row, false);

			// Draw the row unmasked.
			TblMarkRowInvalid (table, row);
			TblRedrawTable(table);

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
				// Leave PrivateRecordVisualStatus set to maskPrivateRecords

			// Now that the row is unmasked, let the table re-handle the table
			// enter event. This will cause the field to be made fully visible
			// and place the cursor at the start of the field. It is necessary
			// to put the cursor in the field so that tapping outside the field
			// can be used to re-mask the record.
			newEvent.eType = tblEnterEvent;
			newEvent.penDown = event->penDown;
			newEvent.tapCount = 0;						// don't select anything
			newEvent.screenX = 0;						// put cursor at start
			newEvent.screenY = 0;
			newEvent.data.tblEnter.tableID = table->id;
			newEvent.data.tblEnter.pTable = table;
			newEvent.data.tblEnter.row = row;
			newEvent.data.tblEnter.column = descColumn;
				// Never let this event view the note.

			// Rather than posting the event, handle it directly to avoid
			// the click produced by it, since a click was already produced.
			SndGetDefaultVolume (NULL, &systemVolume, NULL);
			SndSetDefaultVolume (NULL, &mutedVolume, NULL);
			TblHandleEvent (table, &newEvent);
			SndSetDefaultVolume (NULL, &systemVolume, NULL);
			}
		return;
		}
		
	// Was a time item selected?
	if (column == timeColumn)
		{
		// Unhighlight the time.
		TblUnhighlightSelection (table);
		
		// If an empty time slot has been selected then create a 
		// new appointment.
		uniqueID = TblGetRowData (table, event->data.tblEnter.row);
		if (uniqueID == 0)
			{
			// If we have reached the maximum number of displayable event, then
			// exit.
			if (NumApptsOnly >= apptMaxPerDay)
				return;

			// Get the start time, and end time of the row selected.
			apptIndex = TblGetRowID (table, row);
			appts = MemHandleLock (ApptsH);
		
			// Create a untimed appointment on the current day.
			MemSet (&newAppt, sizeof (newAppt), 0);
			when.startTime = appts[apptIndex].startTime;
			when.endTime = appts[apptIndex].endTime;
			when.date = Date;
			newAppt.when = &when;
			newAppt.description = (char *)""; 

			if (AlarmPreset.advance != apptNoAlarm)
				newAppt.alarm = &AlarmPreset;		

			error = ApptNewRecord (ApptDB, &newAppt, &recordNum);
			
			if (! error)
				{
				appts[apptIndex].recordNum =  recordNum;
		
				// Store the unique id of the record in the row.
				DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
				TblSetRowData (table, row, uniqueID);

				CurrentRecord = recordNum;
				ItemSelected = true;
				}

			MemHandleUnlock (ApptsH);

			// Display an alert that indicates that the new record could 
			// not be created.
			if (error)
				{
				FrmAlert (DeviceFullAlert);
				return;
				}
			}

		DayViewSelectTime (table, row);
		}


	// Was a description selected?
	else if (column == descColumn)
		{
		// Get the record index of the selected appointment.
		apptIndex = TblGetRowID (table, row);
		appts = MemHandleLock (ApptsH);
		CurrentRecord = appts[apptIndex].recordNum;
		MemPtrUnlock (appts);
		
		// If the table is in edit mode then the description field
		// was selected, otherwise the note indictor must have
		// been selected.
		if (TblEditing (table))
			{
			ItemSelected = true;

			// If the row will have been visually invalidate if: an empty slot
			// was selected and the alarm preset preference is on, or
			// the end time of the event is not displayed, or the font for 
			// drawing descriptions are a different height than the font for 
			// drawing empty time slots.

			// If the alarm preset preference is on then selecting an empty
			// appointment will visually invalidate that row.  We need to
			// redraw the table so the the alarm icon can be drawn.
			if (TblRowInvalid (table, row))
				{
				TblReleaseFocus (table);
				DayViewLayoutDay (true);
				DayViewLoadTable ();
				TblRedrawTable (table);
				DayViewDrawTimeBars ();
				DayViewRestoreEditState ();
				}
			}
		else
			FrmGotoForm (NewNoteView);
		}
}


/***********************************************************************
 *
 * FUNCTION:    DayViewResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of appointment's
 *              description is changed as a result of user input.
 *              If the new hieght of the field is shorter,  more items
 *              may need to be added to the bottom of the Day.
 *              
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/18/95		Initial Revision
 *
 ***********************************************************************/
static void DayViewResizeDescription (EventType* event)
{
	Int16 apptIndex;
	Int16 row;
	Int16 currentRow;
	Int16 currentColumn;
	FieldPtr fld;
	TablePtr table;
	RectangleType fieldR;
	
	// Get the current height of the field;
	fld = event->data.fldHeightChanged.pField;
	FldGetBounds (fld, &fieldR);

	// Have the table object resize the field and move the items below
	// the field up or down.
	table = GetObjectPtr (DayTable);
	TblHandleEvent (table, event);
	

	// If the field's height has been expanded , and we don't have items scrolled 
	// off the top of the table, just update the scrollers.

	if (event->data.fldHeightChanged.newHeight >= fieldR.extent.y)
		{
		TopVisibleAppt = TblGetRowID (table, 0);
		}
		

	// Relayout the day to add or remove empty time slots.  We don't need 
	// to rebuild the list of records.
	DayViewLayoutDay (true);
	DayViewLoadTable ();


	// Relaying-out the day migth move the row that has the focus, if so 
	// we need to correct the table's current row.
	apptIndex = DayViewFindAppointment (CurrentRecord);
	TblFindRowID (table, apptIndex, &row);
	TblGetSelection (table, &currentRow, &currentColumn);
	if (row != currentRow)
		// DOLATER ??? - write API routine to do this.
		table->currentRow = row;


	TblRedrawTable (table);		
	DayViewDrawTimeBars ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewSetTitle
 *
 * DESCRIPTION: Set the Day View form's title, based on the Day and
 *		LongDateFormat global variables.
 *
 * PARAMETERS:  frmP		Pointer to Day View form.
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		09/12/99	kwk	Created by Ken Krugler.
 *		10/22/99	kwk	Directly use short date format as index into the
 *							formatting strings.
 *		04/18/00 jmp	Added support for the dfMDYWithDashes format.
 *		08/31/00	kwk	Don't forget space for the terminating NULL for the
 *							title string.
 *		11/08/00	kwk	Use separate index for dfMDYWithDashes format.
 *
 ***********************************************************************/
static void DayViewSetTitle(FormType* frmP)
{
	Char title[longDateStrLength + 1];
	UInt16 dateFormatIndex;
	MemHandle templateH;
	Char *templateP;

	// We can't use the long date format to guess at the short date
	// format, since there's not a one-to-one mapping set up in the
	// Formats panel. We'll directly use the short date format.
	
	// We need to derive the appropriate date template string based on
	// the ShortDateFormat global, which is loaded from sys prefs (and
	// thus is set by the user in the Formats panel).
	//
	if (ShortDateFormat == dfMDYWithDashes)
		{
		dateFormatIndex = DayViewMDYWithDashesTitleStemplateStrID - DayViewFirstTitleTemplateStrID;
		}
	else if (ShortDateFormat > dfYMDWithDashes)
		{
		// DOLATER kwk - gross! If we add a new short date format,
		// this will trigger a fatal alert, but only if testing
		// actually runs this code with that format selected.
		ErrNonFatalDisplay("Unknown short date format");
		
		// Default to the dfMDYWithSlashes format.
		//
		dateFormatIndex = (UInt16)dfMDYWithSlashes;
		}
	else
		{
		dateFormatIndex = (UInt16)ShortDateFormat;
		}
	
	templateH = DmGetResource(strRsc, DayViewFirstTitleTemplateStrID + dateFormatIndex);
	templateP = (Char*)MemHandleLock(templateH);
	DateTemplateToAscii(templateP, Date.month, Date.day, (Date.year+firstYear), title, sizeof(title) - 1);
	MemHandleUnlock(templateH);

	FrmCopyTitle (frmP, title);
} // DayViewSetTitle


/***********************************************************************
 *
 * FUNCTION:    DayViewDrawTitle
 *
 * DESCRIPTION: This routine draws the day view title and highlights
 *              the current day's the day-of-week push button. 
 *
 * PARAMETERS:  void
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/1/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewDrawTitle (void)
{
	UInt16	dayOfWeek;
	FormPtr	frm;

	frm = FrmGetActiveForm ();
	DayViewSetTitle(frm);
	
	// Update the day-of-week push button to highlight the correct day of
	// the week.
	dayOfWeek = DatebookDayOfWeek (Date.month, Date.day, Date.year+firstYear);
	FrmSetControlGroupSelection (frm, DayOfWeekGroup, DayDOW1Button + dayOfWeek);

	TimeDisplayed = false;
}


/***********************************************************************
 *
 * FUNCTION:    DayViewShowTime
 *
 * DESCRIPTION: This routine display the current time in the title of the
 *              day view.
 *
 * PARAMETERS:  frm - pointer to the day view form.
 *
 * RETURNED:    nothing
 *
 * NOTE:        The global variables TimeDisplayed and TimeDisplayTick are
 *					 set by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/14/96	Initial Revision
 *			grant 2/2/99	update TimeDisplayTick (now matches WeekViewShowTime
 *								and MonthViewShowTime)
 *
 ***********************************************************************/
static void DayViewShowTime (void)
{
	Char				title[timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii (dateTime.hour, dateTime.minute, TimeFormat, title);
	FrmCopyTitle (FrmGetActiveForm (), title);
	
	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks () + timeDisplayTicks;
}


/***********************************************************************
 *
 * FUNCTION:    DayViewHideTime
 *
 * DESCRIPTION: If the title of the Day View is displaying the current 
 *              time, this routine will change the title to the standard
 *					 title (the current date).
 *
 * PARAMETERS:  nothing
 *
 * PARAMETERS:  hide - true to always hide, false hide only if
 *                     to time has been display for the require
 *                      length of time.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/14/96	Initial Revision
 *			grant	2/2/99	Use TimeToWait(), don't use EvtSetNullEventTick()
 *
 ***********************************************************************/
static void DayViewHideTime (Boolean hide)
{
	if (TimeDisplayed)
		{
		if (hide || TimeToWait() == 0)
			{
			// If the Day View is the draw window then redraw the title.
			if (WinGetDrawWindow () == FrmGetWindowHandle (FrmGetFormPtr (DayView)))
				DayViewDrawTitle ();
			}
		}
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDrawDate
 *
 * DESCRIPTION: This routine display the date passed.
 *
 * PARAMETERS:  date - date to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewDrawDate (DateType date)
{
	TablePtr table;

	// Adjust the current date.
	Date = date;

	DayViewDrawTitle ();
		
	// Get all the appointments and empty time slots on the new day.
	DayViewLayoutDay (true);

	// Determine the first appointment to display.
	DayViewSetTopAppointment ();

 	// Load the appointments and empty time slots into the table object 
	// that will display them.
	DayViewLoadTable ();
	
	// Draw the new day's events.
	table = GetObjectPtr (DayTable);
	TblRedrawTable (table);
	DayViewDrawTimeBars ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDayOfWeekSelected
 *
 * DESCRIPTION: This routine is called when of the day-of-week push buttons
 *              is pressed.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/27/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewDayOfWeekSelected (EventType* event)
{
	Int16			adjust;
	UInt16		dayOfWeek;
	UInt16		newDayOfWeek;

	// Adjust the current date.
	dayOfWeek = DatebookDayOfWeek (Date.month, Date.day, Date.year+firstYear);
	newDayOfWeek = event->data.ctlSelect.controlID - DayDOW1Button;
	if (dayOfWeek == newDayOfWeek) return;
	
	adjust = newDayOfWeek - dayOfWeek;
	DateAdjust (&Date, adjust);

	DayViewDrawDate (Date);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewGoToDate
 *
 * DESCRIPTION: This routine displays the date picker so that the 
 *              user can select a date to navigate to.  If the date
 *              picker is confirmed, the date selected is displayed.
 *
 *              This routine is called when a "go to" button is pressed.
 *              
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewGoToDate (void)
{
	Char* title;
	MemHandle titleH;
	Int16 month, day, year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// Display the date picker.
	if (SelectDay (selectDayByDay, &month, &day, &year, title))
		{
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;

		DayViewDrawDate (Date);
		}
		
	MemHandleUnlock (titleH);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewGotoAppointment
 *
 * DESCRIPTION: This routine sets gloabal variables such the Day View
 *              will display the text found by the text search
 *              command.
 *
 * PARAMETERS:  event - frmGotoEvent 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/24/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewGotoAppointment (EventType* event)
{
	TopVisibleAppt = 0;
	ItemSelected = true;
	CurrentRecord = event->data.frmGoto.recordNum;
	DayEditPosition = event->data.frmGoto.matchPos;
	DayEditSelectionLength = event->data.frmGoto.matchLen;
	
	SetDateToNextOccurrence (CurrentRecord);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewScroll
 *
 * DESCRIPTION: This routine scrolls the day of of appointments
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *              wrap      - if true the day is wrap to the first appointment
 *                          if we're at the end of the day.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void DayViewScroll (WinDirectionType direction, Boolean wrap)
{
	Int16				row;
	Int16				height;
	UInt16			apptIndex;
	Int16				iconsWidth;
	Int16 			columnWidth;
	Int16 			tableHeight;
	FontID			fontID;
	TablePtr			table;
	RectangleType	r;

	table = GetObjectPtr (DayTable);
	TblReleaseFocus (table);

	// Get the height of the table and the width of the description
	// column.
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (table, descColumn);

	apptIndex = TopVisibleAppt;

	// Scroll the table down.
	if (direction == winDown)
		{
		// If we're at the bottom of the day, and page wrapping is allowed,
		// go to the top of the page.
		if (wrap && ( ! CtlEnabled (GetObjectPtr (DayDownButton))))
			TopVisibleAppt = 0;
		
		else if (TopVisibleAppt+1 >= NumAppts) 
			return;

		else
			{
			row = TblGetLastUsableRow (table);
			apptIndex = TblGetRowID (table, row);				
	
			// If there is only one appointment visible, this is the case 
			// when a appointment occupies the whole screeen, and its not
			// the last appointment, move to the next appointment.
			if (row == 0)
				apptIndex++;
			}
		}


	// Scroll the table up.
	else
		{
		if (TopVisibleAppt == 0) return;
		
		// Scan the records starting with the first visible record to 
		// determine how many record we need to scroll.  Since the 
		// heights of the records vary,  we sum the heights of the 
		// records until we get a screen full.
		height = TblGetRowHeight (table, 0);
		if (height >= tableHeight)
			height = 0;							

		while ( (height < tableHeight) && (apptIndex > 0) )
			{
			height += DayViewGetDescriptionHeight (apptIndex-1, columnWidth, tableHeight, &iconsWidth, &fontID);
			if ((height <= tableHeight) || (apptIndex == TopVisibleAppt))
				apptIndex--;
			}
		}

	TblMarkTableInvalid (table);
				
	TopVisibleAppt = apptIndex;
	DayViewLoadTable ();	

	TblUnhighlightSelection (table);
	TblRedrawTable (table);
	DayViewDrawTimeBars ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewPurge
 *
 * DESCRIPTION: This routine deletes appointment that are before the 
 *              user specified date.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/1/95		Initial Revision
 *
 ***********************************************************************/
static void DayViewPurge (void)
{
	TablePtr table;
	Boolean redraw;

	redraw = PurgeRecords ();

	// If the current date is before the purge date redraw the day, we may
	// have deleted some of the appointments being displayed.
	if (redraw)
		{
		DayViewLayoutDay (true);		
		DayViewLoadTable ();	
	
		table = GetObjectPtr (DayTable);
		TblRedrawTable (table);
		DayViewDrawTimeBars ();
		}
}


/***********************************************************************
 *
 * FUNCTION:    DayViewDoCommand
 *
 * DESCRIPTION: This routine preforms the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    true if 
 *
 * HISTORY:
 *		03/29/95	art	Created by Art Lamb.
 *		09/17/99	jmp	Use NewNoteView instead of NoteView.
 *		11/04/99	jmp	To prevent other sublaunch issues, remind ourselves
 *							that we've sublaunched already into PhoneNumberLookup().
 *		08/28/00	kwk	Use new FrmGetActiveField trap.
 *
 ***********************************************************************/
static Boolean DayViewDoCommand (UInt16 command)
{
	Int16			row;
	Int16			column;
	UInt16 		pasteLen;
	MemHandle 	pasteCharsH;
	FontID		fontID;
	FieldPtr 	fld;
	TablePtr 	table;
	Boolean 		handled = true;
	
	// New
	if (command == NewItemCmd)
		{
		DayViewClearEditState ();
		DayViewNewAppointment (NULL);
		if (ItemSelected)
			{
			table = GetObjectPtr (DayTable);
			TblGetSelection (table, &row, &column);
			TblReleaseFocus (table);
			DayViewSelectTime (table, row);
			}
		}


	// Delete
	else if (command == DeleteCmd)
		{
		if (ItemSelected)
			DayViewDeleteAppointment ();
		else
			FrmAlert (SelectItemAlert);
		}


	// Create note
	else if (command == CreateNoteCmd)
		{
		if (ItemSelected)
			{
			TblReleaseFocus (GetObjectPtr (DayTable));
			if (CreateNote (true))
				{
				MenuEraseStatus (NULL);
				FrmGotoForm (NewNoteView);
				}
			else
				DayViewRestoreEditState ();
			}
		else
			FrmAlert (SelectItemAlert);
		}


	// Delete nore
	else if (command == DeleteNoteCmd)
		{
		if (ItemSelected)
			DayViewDeleteNote ();
		else
			FrmAlert (SelectItemAlert);
		}
		

	// Purge
	else if (command == PurgeCmd)
		{
		DayViewClearEditState ();
		DayViewPurge ();
		}
					

	// Beam Record
	else if (command == BeamRecordCmd)
		{
		if (ItemSelected)
			{
			TblReleaseFocus (GetObjectPtr (DayTable));
			DateSendRecord(ApptDB, CurrentRecord, exgBeamPrefix);
			DayViewRestoreEditState ();
			}
		else
			FrmAlert (SelectItemAlert);
		}
					

	// Send Record
	else if (command == SendRecordCmd)
		{
		if (ItemSelected)
			{
			TblReleaseFocus (GetObjectPtr (DayTable));
			DateSendRecord(ApptDB, CurrentRecord, exgSendPrefix);
			DayViewRestoreEditState ();
			}
		else
			FrmAlert (SelectItemAlert);
		}
					

	// Font Selector
	else if (command == DayFontsCmd)
		{
		TblReleaseFocus (GetObjectPtr (DayTable));
		fontID = SelectFont (ApptDescFont);
		if (fontID != ApptDescFont)
			{
			ApptDescFont = fontID;
			DayViewClearEditState ();
			}
		else
			DayViewRestoreEditState ();
		}

	// Preferences
	else if (command == DayPreferencesCmd)
		{
		DayViewClearEditState ();
		FrmPopupForm (PreferencesDialog);
		}

	// Display Options
	else if (command == DayDisplayOptionsCmd)
		{
		DayViewClearEditState ();
		FrmPopupForm (DisplayDialog);
		}

	// Phone Lookup
	else if (command == DayPhoneLookup)
		{
		fld = FrmGetActiveField (NULL);
		if (fld)
			{
			InPhoneLookup = true;
			PhoneNumberLookup (fld);
			InPhoneLookup = false;
			}
		else
			FrmAlert (SelectItemAlert);
		}
		
	// Security
	else if (command == DaySecurityCmd)
		{
		DayViewClearEditState();
		DoSecurity();
		DayViewUpdateDisplay(updateDisplayOptsChanged);
		}

	// About
	else if (command == DayGetInfoCmd)
		{
		DayViewClearEditState ();
		AbtShowAbout (sysFileCDatebook);
		}


	// Paste
	else if (command == sysEditMenuPasteCmd)
		{
			fld = FrmGetActiveField (NULL);
			if (! fld)
			{
			pasteCharsH = ClipboardGetItem (clipboardText, &pasteLen);
			if (pasteCharsH && pasteLen)
				{
				DayViewNewAppointment (NULL);
				GrfSetState (false, false, false);
				}
			}
		handled = false;
		}

	// Some the the edit menu commands are handled by FrmHandleEvent.
	else
		{
		handled = false;
		}
	
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    DayViewUpdateDisplay
 *
 * DESCRIPTION: This routine updates the display of the day View.
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the Day View.
 *                		
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95	Initial Revision
*			jmp	11/01/99	Fixed problem on frmRedrawUpdateCode events when
 *								we're still in the edit state but we weren't redrawing
 *								the edit indicator.  Fixes Datebook part of bug #23235.
 *
 ***********************************************************************/
static void DayViewUpdateDisplay (UInt16 updateCode)
{
	Int16 apptIndex;
	Int16 row, column;
	DateType next;
	TablePtr table;
	Boolean repeatsAgain;
	MemHandle recordH;
	ApptDBRecordType apptRec;
	
	table = GetObjectPtr (DayTable);

	// Was the UI unable to save an image of the day view when is 
	// obscured part of the day view with another dialog?  If not,
	// we'll handle it here.
	if (updateCode & frmRedrawUpdateCode)
		{
		FrmDrawForm (FrmGetActiveForm ());
		
		// If we're editing, then find out which row is being edited,
		// mark it invalid, and redraw the table.
		if (TblEditing(table))
			{
			TblGetSelection (table, &row, &column);
			TblMarkRowInvalid(table, row);
			TblRedrawTable (table);
			}
			
		DayViewDrawTimeBars ();
		return;
		}
		
	// Was the display options modified (Preferences dialog) or was the 
	// font changed.
	else if (updateCode & updateDisplayOptsChanged)
		{
		DayViewLayoutDay (true);
		DayViewSetTopAppointment ();
		}

	// Was an item deleted or marked sercet.
	else if ( (updateCode & updateItemDelete) ||
				 (updateCode & updateItemHide))
		{
		ClearEditState ();
		DayViewLayoutDay (true);
		}


	// Was the repeat infomation changed?
	else if (updateCode & updateRepeatChanged)
		{
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
		if (apptRec.repeat)
			{
			next = Date;
			repeatsAgain = ApptNextRepeat (&apptRec, &next, true);
			if (! repeatsAgain)
				{
				next = apptRec.when->date;
				repeatsAgain = ApptNextRepeat (&apptRec, &next, true);
				}
			MemHandleUnlock (recordH);

			if (repeatsAgain && (DateToInt (next) != DateToInt (Date)))
				{
				Date = next;
				DayViewDrawDate (Date);
				return;
				}
			else if (! repeatsAgain)
				ClearEditState ();
			}
		else
			{
			next = apptRec.when->date;
			MemHandleUnlock (recordH);
			// If a repeating event was changed to a non-repeating event
			// and the start date was also change then go to the new 
			// start date.
			if (DateToInt (next) != DateToInt (Date))
				{
				Date = next;
				DayViewDrawDate (Date);
				return;
				}
			}
			
		// Redraw the current record if it's still visible so the
		// the repeat indictor is draw or erased
		TblGetSelection (table, &row, &column);
		TblSetRowUsable (table, row, false);

		DayViewLayoutDay (true);
		}


	// Was the date of an appointment changed?
	else if (updateCode & updateDateChanged)
		{
		ApptGetRecord (ApptDB, CurrentRecord, &apptRec, &recordH);
		Date = apptRec.when->date;
		MemHandleUnlock (recordH);
			
		DayViewDrawDate (Date);
		return;
		}


	// Was the time of an appointment or the font changed?
	else if (updateCode & (updateTimeChanged | updateException | updateFontChanged))
		{
		DayViewLayoutDay (true);
		}

	// Was an alarm set or cleard?
	else if (updateCode & updateAlarmChanged)
		{
		DayViewLayoutDay (true);

		// Redraw the current record if it's still visible so the
		// the alarm indictor will be drawn or erased.
		apptIndex = DayViewFindAppointment (CurrentRecord);
		if (TblFindRowID (table, apptIndex, &row))
			TblSetRowUsable (table, row, false);
		}

	DayViewLoadTable ();
	TblRedrawTable (table);
	DayViewDrawTimeBars ();
}


/***********************************************************************
 *
 * FUNCTION:    DayViewInit
 *
 * DESCRIPTION: This routine initializes the "Day View" of the 
 *              Datebook application.
 *
 * PARAMETERS:  frm - pointer to the day view form.
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		06/12/95	art	Created by Art Lamb.
 *		08/04/99	kwk	Copy day-of-week pushbutton ptrs vs. just first byte
 *							of each label.
 *
 ***********************************************************************/
static void DayViewInit (FormPtr frm)
{
	UInt16 month;
	UInt16 day;
	UInt16 year;
	UInt16 dayOfWeek;
	UInt16 id;
	UInt16 row;
	UInt16 rowsInTable;
	TablePtr table;

	// Get the day we're displaying.
	day = Date.day;
	month = Date.month;
	year = Date.year+firstYear;

	DayViewSetTitle(frm);
	
	// If the start-day-of-week is monday rearrange the labels on the 
	// days-of-week push buttons.
	if (StartDayOfWeek == monday)
		{
		const Char* sundayLabel = CtlGetLabel (GetObjectPtr (DayDOW1Button));
		for (id = DayDOW1Button; id < DayDOW7Button; id++)
			{
			CtlSetLabel (GetObjectPtr (id), CtlGetLabel (GetObjectPtr (id + 1)));
			}
		CtlSetLabel (GetObjectPtr (DayDOW7Button), sundayLabel);
		}

	// Highlight the correct day-of-week push button.
	dayOfWeek = DatebookDayOfWeek (month, day, year);
	FrmSetControlGroupSelection (frm, DayOfWeekGroup, DayDOW1Button + dayOfWeek);

	// Highlight the Day View push button.
	FrmSetControlGroupSelection (frm, DayViewGroup, DayDayViewButton);

	// Initialize the table used to display the day's appointments.
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, DayTable));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
		{		
		TblSetItemStyle (table, row, timeBarColumn, customTableItem);
		TblSetItemStyle (table, row, timeColumn, customTableItem);
		TblSetItemStyle (table, row, descColumn, textTableItem);
		TblSetRowUsable (table, row, false);
		}

	TblSetColumnUsable (table, timeBarColumn, true);
	TblSetColumnUsable (table, timeColumn, true);
	TblSetColumnUsable (table, descColumn, true);
	
	TblSetColumnMasked (table, descColumn, true);

	TblSetColumnSpacing (table, timeBarColumn, spaceAfterTimeBarColumn);
	TblSetColumnSpacing (table, timeColumn, spaceAfterTimeColumn);
	
	TblSetColumnEditIndicator (table, timeBarColumn, false);

	// Set the callback routines that will load and save the 
	// description field.
	TblSetLoadDataProcedure (table, descColumn, DayViewGetDescription);
	TblSetSaveDataProcedure (table, descColumn, DayViewSaveDescription);
	
	// Set the callback routine that draws the time field.
	TblSetCustomDrawProcedure (table, timeColumn, DayViewDrawTime);

	// Set the callback routine that draws the note, alarm, and repeat icons.
	TblSetCustomDrawProcedure (table, descColumn, DayViewDrawIcons);

	// By default the list view assume no time bar are displayed.
	TimeBarColumns = 0;

	// Get all the appointments and empty time slots on the current day.
	DayViewLayoutDay (true);


	// If we do not have an appointment selected, then position the table
	// so the the first appointment of the day is visible, unless we're on
	// today, then make sure the next appointment is visible.
	if (! ItemSelected)
		DayViewSetTopAppointment ();

	// Load the appointments and empty time slots into the table object 
	// that will display them.
	if (PendingUpdate)
		DayViewUpdateDisplay (PendingUpdate);
	else
		DayViewLoadTable ();


	// Initial miscellaneous global variables.
	TimeDisplayed = false;
	PendingUpdate = 0;
}


/***********************************************************************
 *
 * FUNCTION:    DayViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the Day View
 *              of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 *	HISTORY:
 *		06/12/95	art	Created by Art Lamb.
 *		11/22/98	kwk	Handle command keys in separate code block so that
 *							TxtCharIsPrint doesn't get called w/virtual chars.
 *		06/22/99	CS		Standardized keyDownEvent handling
 *							(TxtCharIsHardKey, commandKeyMask, etc.)
 *		08/04/99	kwk	Tweak auto-entry generation w/keydown event so that
 *							it works when a FEP is active.
 *		09/23/99 jmp	On frmCloseEvent, don't call FrmSetFocus() with
 *							a NULL frm to prevent debug build from going ErrFatalDisplay
 *							crazy.
 *		11/04/99	jmp	Restored DayView edit state after beaming; fixes bug #23315.
 *		10/27/00	gap	change the command bar initialization to allow field
 *							code to add cut, copy, paste, & undo commands as 
 *							appropriate rather than adding a fixed set of selections.
 *
 ***********************************************************************/
Boolean DayViewHandleEvent (EventType* event)
{
	Int16 row, column;
	UInt32 uniqueID;
	DateType date;
	FormPtr frm;
	TablePtr table;
	UInt32 numLibs;
	Boolean handled = false;

	if (event->eType == keyDownEvent)
		{
		#if WRISTPDA
			EventType	newEvent;
			FormType*	frmP;
			extern UInt32  DebounceEnterKeyEndTime;
			extern UInt16  NewSndSysAmp, OldSndSysAmp;
			extern UInt16  NewSndDefAmp, OldSndDefAmp;
			extern UInt16  NewKeyInitDelay, OldKeyInitDelay;
			extern UInt16  NewKeyPeriod, OldKeyPeriod;
			extern UInt16  NewKeyDoubleTapDelay,  OldKeyDoubleTapDelay;
			extern Boolean NewKeyQueueAhead, OldKeyQueueAhead;

			frmP = FrmGetActiveForm();
			// On other than Enter key exit the debounce state.
			if ( event->data.keyDown.chr != vchrThumbWheelPush ) {
				if ( DebounceEnterKeyEndTime > 0 ) {
					DebounceEnterKeyEndTime = 0;
				}
			}
			MemSet( & newEvent, sizeof( EventType ), 0 );
			if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
				// If we need to debounce this Enter key event then just return.
				if ( DebounceEnterKeyEndTime > 0 ) {
					if ( TimGetTicks() < DebounceEnterKeyEndTime )
						return true;
				}
				DebounceEnterKeyEndTime = 0;
				// Translate the Enter key to a Done button event.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = DayWeekViewButton;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frmP,FrmGetObjectIndex(frmP, newEvent.data.ctlSelect.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
				// Translate the Back key to an open launcher event.
				newEvent.eType = keyDownEvent;
				newEvent.data.keyDown.chr = launchChr;
				newEvent.data.keyDown.modifiers = commandKeyMask;
				EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
				// Translate the RockerUp key to a DayDOWPrevButton event.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = DayDOWPrevButton;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frmP,FrmGetObjectIndex(frmP, newEvent.data.ctlSelect.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
				// Translate the RockerDown key to a DayDOWNextButton event.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = DayDOWNextButton;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frmP,FrmGetObjectIndex(frmP, newEvent.data.ctlSelect.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrPageUp ) {
				// Translate the PageUp key to a DayUpButton event.
				newEvent.eType = ctlRepeatEvent;
				newEvent.data.ctlRepeat.controlID = DayUpButton;
				newEvent.data.ctlRepeat.pControl =
					FrmGetObjectPtr(frmP,FrmGetObjectIndex(frmP, newEvent.data.ctlRepeat.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000005, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrPageDown ) {
				// Translate the PageDown key to a DayDownButton event.
				newEvent.eType = ctlRepeatEvent;
				newEvent.data.ctlRepeat.controlID = DayDownButton;
				newEvent.data.ctlRepeat.pControl =
					FrmGetObjectPtr(frmP,FrmGetObjectIndex(frmP, newEvent.data.ctlRepeat.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000006, true );
				return true;
			}
		#endif
		// Datebook key pressed?
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
			DayViewClearEditState ();
			DateSecondsToDate (TimGetSeconds (), &date);
			if (DateToInt (date) != DateToInt (Date) ||
				 ((event->data.keyDown.modifiers & poweredOnKeyMask)))
				DayViewDrawDate (date);
			else
				FrmGotoForm (WeekView);
			handled = true;
			}
		else if (EvtKeydownIsVirtual(event))
			{
	
			// Scroll up key pressed?
			if (event->data.keyDown.chr == vchrPageUp)
				{
				DayViewClearEditState ();
				DateAdjust (&Date, -1);
				DayViewDrawDate (Date);
				handled = true;
				}
	
			// Scroll down key pressed?
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				DayViewClearEditState ();
				DateAdjust (&Date, 1);
				DayViewDrawDate (Date);
				handled = true;
				}
	
			// Send data key pressed?
			else if (event->data.keyDown.chr == vchrSendData)
				{
				if (ItemSelected)
					{
					TblReleaseFocus (GetObjectPtr (DayTable));
					DateSendRecord(ApptDB, CurrentRecord, exgBeamPrefix);
					DayViewRestoreEditState ();
					}
				else
					FrmAlert (SelectItemAlert);
				handled = true;
				}
			
			// Confirm key pressed?
			else if (event->data.keyDown.chr == vchrConfirm)
			{
				ItemSelected = false;
				// Leave handled false so table releases focus.
			}
			
			else
				{
				handled = false;
				}
			}

		else if (TxtCharIsPrint (event->data.keyDown.chr))
			{
			if (!ItemSelected)
				{
				// If no item is selected and the character is display, 
				// create a new ToDo item.
				if (TxtCharIsDigit (event->data.keyDown.chr))
					{
					handled = DayViewNewAppointment (event);
					}
				else
					{
					// Create a new empty appointment, upper-shift the character,
					// and requeue it. This way it works even if an input method
					// is active.
					DayViewNewAppointment (NULL);
					if (ItemSelected)
						{
						event->data.keyDown.chr = TxtGlueUpperChar(event->data.keyDown.chr);
						EvtAddEventToQueue (event);
						handled = true;
						}
					}
				}
/*			else
				{
				// If we get a printable character, then we can assume that any Graffiti
				// auto-shifting is finished, so it's safe to turn it off. This solves the
				// problem of a re-queued keydown event (e.g. when in list view & writing
				// a character -> generate a new entry) not turning off the temp shift state
				// because it wasn't created by Graffiti.

				Boolean capsLock, numLock, autoShifted;
				UInt16 tempShift;
				
				if (GrfGetState (&capsLock, &numLock, &tempShift, &autoShifted) == 0
					 && autoShifted)
					{
					GrfSetState (capsLock, numLock, false);
					}
				}*/
			}
		}


	// If the pen is not in any of the object of the view, take the 
	// view out of edit mode.
	else if (event->eType == penDownEvent)
		{
		if (! FrmHandleEvent (FrmGetActiveForm (), event))
			DayViewClearEditState ();
		handled = true;		// Don't let FrmHandleEvent get this event again.
		}

	// Check for buttons that take us out of edit mode.
	else if (event->eType == ctlEnterEvent)
		{
		switch (event->data.ctlEnter.controlID)
			{
			case DayNextWeekButton:
			case DayDOW1Button:
			case DayDOW2Button:
			case DayDOW3Button:
			case DayDOW4Button:
			case DayDOW5Button:
			case DayDOW6Button:
			case DayDOW7Button:
			case DayPrevWeekButton:
			case DayDayViewButton:
			case DayWeekViewButton:
			case DayMonthViewButton:
			case DayAgendaViewButton:
			case DayNewButton:
			case DayGoToButton:
			case DayUpButton:
			case DayDownButton:
				DayViewClearEditState ();
				break;
			}
		}


	// Handle the button that was selected.
	else if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case DayWeekViewButton:
				FrmGotoForm (WeekView);
				handled = true;
				break;

			case DayMonthViewButton:
				FrmGotoForm (MonthView);
				handled = true;
				break;
			
			case DayAgendaViewButton:
				FrmGotoForm (AgendaView);
				handled = true;
				break;

			case DayDetailsButton:
				if (ItemSelected)
					FrmPopupForm (DetailsDialog);
				else
					FrmAlert (SelectItemAlert);
				handled = true;
				break;

			case DayNewButton:
				DayViewNewAppointment (NULL);
				if (ItemSelected)
					{
					table = GetObjectPtr (DayTable);
					TblGetSelection (table, &row, &column);
					TblReleaseFocus (table);
					DayViewSelectTime (table, row);
					}
				break;

			case DayGoToButton:
				DayViewGoToDate ();
				handled = true;
				break;

			case DayDOW1Button:
			case DayDOW2Button:
			case DayDOW3Button:
			case DayDOW4Button:
			case DayDOW5Button:
			case DayDOW6Button:
			case DayDOW7Button:
				DayViewDayOfWeekSelected (event);
				handled = true;
				break;
			#if WRISTPDA
			case DayDOWPrevButton:
				DayViewClearEditState ();
				DateAdjust (&Date, -1);
				DayViewDrawDate (Date);
				handled = true;
				break;
			case DayDOWNextButton:
				DayViewClearEditState ();
				DateAdjust (&Date, 1);
				DayViewDrawDate (Date);
				handled = true;
				break;
			#endif
			}
		}


	// If the pen when down in the details button but did not go up in it,
	// restore the edit state.
	else if (event->eType == ctlExitEvent)
		{
		if (event->data.ctlExit.controlID == DayDetailsButton)
			DayViewRestoreEditState ();
		}


	// Handle the scrolling controls.
	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case DayPrevWeekButton:
				DateAdjust (&Date, -daysInWeek);
				DayViewDrawDate (Date);
				break;

			case DayNextWeekButton:
				DateAdjust (&Date, daysInWeek);
				DayViewDrawDate (Date);
				break;

			case DayUpButton:
				#if WRISTPDA
				DayViewClearEditState();
				#endif
				DayViewScroll (winUp, false);
				break;
				
			case DayDownButton:
				#if WRISTPDA
				DayViewClearEditState();
				#endif
				DayViewScroll (winDown, false);
				break;
			}
		}


	// Check if we've changed row in the day view table, if so 
	// clear the edit state.
	else if (event->eType == tblEnterEvent)
		{
		table = (TablePtr) event->data.tblEnter.pTable;
		if (ItemSelected)
			{
			DmRecordInfo (ApptDB, CurrentRecord, NULL, &uniqueID, NULL);
			if (TblFindRowData (table, uniqueID, &row))
				{
				if (event->data.tblEnter.row != row)
					handled = DayViewClearEditState ();
				}
			}
		if (event->data.tblEnter.column == timeBarColumn)
			handled = true;
		else if (! handled)
			handled = DayViewSelectIcons (event);
		}
		

	// An item in the table has been selected.
	else if (event->eType == tblSelectEvent)
		{
		DayViewItemSelected (event);
		handled = true;
		}
		

	// Expand or compress the height of the appointments description.
	else if (event->eType == fldHeightChangedEvent)
		{
		DayViewResizeDescription (event);
		handled = true;
		}


	// Add the buttons that we want available on the command bar, based on the current context
	else if (event->eType == menuCmdBarOpenEvent)
	{

		if (ItemSelected) 
		{
			FieldType* fldP;
			UInt16 startPos, endPos;
			
			fldP = TblGetCurrentField(GetObjectPtr(DayTable));
			FldGetSelection(fldP, &startPos, &endPos);
			
			if (startPos == endPos)  // there's no highlighted text, but an item is chosen
			{
				// Call directly Field event handler so that system edit buttons are added if applicable
				FldHandleEvent(fldP, event);
				
				// Left: Beam & Secure
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, DaySecurityCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, BeamRecordCmd, 0);

				// Right: Delete
				MenuCmdBarAddButton(menuCmdBarOnRight, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteCmd, 0);

				// Prevent the field package to add edit buttons again
				event->data.menuCmdBarOpen.preventFieldButtons = true;
			}
		}
		else	// no item is chosen
		{
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, DaySecurityCmd, 0);
			// allow the field package to automatically add cut, copy, paste, and undo buttons as applicable
		}
		// don't set handled to true; this event must fall through to the system.
	}


	else if (event->eType == menuOpenEvent)
		{
		if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
			MenuHideItem(SendRecordCmd);
		else
			MenuShowItem(SendRecordCmd);
		// don't set handled = true
		}


	else if (event->eType == menuEvent)
		{
		handled = DayViewDoCommand (event->data.menu.itemID);
		}
		

	else if (event->eType == frmOpenEvent)
		{
		#if WRISTPDA
		extern UInt16  OldSndSysAmp;
		extern UInt16  OldSndDefAmp;
		extern UInt16  OldKeyInitDelay;
		extern UInt16  OldKeyPeriod;
		extern UInt16  OldKeyDoubleTapDelay;
		extern Boolean OldKeyQueueAhead;
		#endif
		frm = FrmGetActiveForm ();
		DayViewInit (frm);
		FrmDrawForm (frm);
		DayViewDrawTimeBars ();
		DayViewRestoreEditState ();
		handled = true;
		}


	else if (event->eType == frmGotoEvent)
		{
		frm = FrmGetActiveForm ();
		DayViewGotoAppointment (event);
		DayViewInit (frm);
		FrmDrawForm (frm);
		DayViewDrawTimeBars ();
		DayViewRestoreEditState ();
		handled = true;
		}


	else if (event->eType == frmUpdateEvent)
		{
		DayViewUpdateDisplay (event->data.frmUpdate.updateCode);
		if (event->data.frmUpdate.updateCode != frmRedrawUpdateCode)
			DayViewRestoreEditState ();
		handled = true;
		}


	else if (event->eType == frmSaveEvent)
		{
		DayViewClearEditState ();
		// This deletes empty events. It can do this because we don't do a FrmSaveAllForms
		// on a sysAppLaunchCmdSaveData launch.
		}


	else if (event->eType == frmTitleEnterEvent)
		{
		Boolean penDown;
		Coord x, y;
		
		//Wait 1/20 sec before displaying time, to avoid flicker before menu pops up
		SysTaskDelay (timeDisplayWaitTicks);
		PenGetPoint (&x, &y, &penDown);
		if (penDown)
			{
			// If the pen is in the title toggle between the current date
			// and the current time.
			if (TimeDisplayed)
				DayViewHideTime (true);
			else
				{
				DayViewShowTime ();
				}
			}
		}


	else if (event->eType == nilEvent)
		{
		DayViewHideTime (false);
		}

	else if (event->eType == frmCloseEvent)
		{
		// If necessary, release the focus before freeing ApptsH.  Releasing the focus causes some
		// data to be saved, and that action depends on ApptsH.  If the data isn't
		// saved now, trying to save it later will access a NULL ApptsH.
		frm = FrmGetActiveForm ();
		if (frm)
			FrmSetFocus(frm, noFocus);
		
		MemHandleFree (ApptsH);
		ApptsH = 0;
		
		// Also free ApptsOnlyH if necessary (was allocated in last call to DayViewLayoutDay)
		if (ApptsOnlyH)
			{
			MemHandleFree(ApptsOnlyH);
			ApptsOnlyH = 0;
			}
		}

	return (handled);
}
