/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateAlarm.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This module contains the routines that handle alarms.
 *
 * History:
 *		August 29, 1995	Created by Art Lamb
 *
 *			Name		Date			Description
 *			----		----			-----------
 *			gap		7/27/00		rewritten for attention manager support
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <AlarmMgr.h>
#include <FeatureMgr.h>

#include <PalmUtils.h>

#include "sections.h"
#include "Datebook.h"

extern void ECApptDBValidate (DmOpenRef dbP);



/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define alarmDescSeparatorY				9		// whitespace between title and description
#define alarmDetailDescYTextOffset		4
#define alarmDetailDescMaxLine			4
#define alarmListDescMaxLine				1
#define alarmListDescriptionOffset		5

#define alarmIconYOffset					8
#if		WRISTPDA
#define alarmDetailDescHTextOffset		33
#else
#define alarmDetailDescHTextOffset		37
#endif//	WRISTPDA

#define alarmPaddingSeconds				3		// alarms will never trigger more than 3 seconds early


/***********************************************************************
 *
 * FUNCTION:		FindRecordByID
 *
 * DESCRIPTION:	As DmFindRecordByID does not take into account the deleted
 *						any time we want to proccess an attention callback DmQueryRecord
 *						must alsp be called to determine if the unique ID is one
 *						the app needs to take action on.  I have combined both routines
 *						in this call to be sure both are done in all instances that the
 *						record number is obtained via the uniqueID.
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		dbP 			- the note pad database
 * 					uniqueID		- the uniqueID of the note
 *						recordNum	- the record number of the note				
 *
 * RETURNED:		err			- 0 if we get a valid "active" (ie not deleted) refnum
 *										- dmErrUniqueIDNotFound if the id is not found or if the
 *										  record is not currently usable  (ie it is marked deleted)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	09/13/00	Initial Revision
 *
 ***********************************************************************/
Err FindRecordByID (DmOpenRef dbP, UInt32 uniqueID, UInt16 * recordNum)
{
	Err			err;
	MemHandle	recordH;

	// determine if the uniqueID exists in the specified DB
	err = DmFindRecordByID (dbP, uniqueID, recordNum);
	if (err) return err;
	
	// if the record is there, be sure if is one we can access
	recordH = DmQueryRecord (dbP, *recordNum);
	if (!recordH) return dmErrUniqueIDNotFound;
	
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    CondensedTimeToAscii
 *
 * DESCRIPTION:  Convert the time passed to an ascii string.
 *
 * PARAMETERS:  hours    - hours (0-23)
 *					 minutes	 - minutes (0-59)
 *					 timeFormat - how to format the time string
 *					 pString  - pointer to string which gets the result. The
 *									string must be of length timeStringLength
 *
 * RETURNED:	 pointer to the text of the current selection
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	12/2/94	Initial Revision
 *			roger 2/28/95	Changed and made available to system
 *			roger 8/5/96	Added new tfComma24h format
 *
 ***********************************************************************/
 /*
static void CondensedTimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, Char * pString)
{
	char t;
	
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
	// Fill up the destination as much as allowed with a value to reveal errors.
	MemSet(pString, timeStringLength, 0xfe);	
#endif
	
	t = hours;
	if ( (timeFormat == tfColon24h) || (timeFormat == tfDot24h) ||
		  (timeFormat == tfHours24h) || (timeFormat == tfComma24h) )
		{
		if (t >= 20)
			{
			t -= 20;
			*pString++ = '2';
			}
		}
	else
		{
		t %= 12;
	
		if (t == 0)
			{
			t = 2;
			*pString++ = '1';
			}
		}

	if (t >= 10)
		{
		t -= 10;
		*pString++ = '1';
		}
	*pString++ = '0' + t;


	// Now add the minutes
	if ( (timeFormat != tfHoursAMPM) && (timeFormat != tfHours24h) )
		{
		*pString++ = TimeSeparator(timeFormat);
	
		// Translate minutes in to text characters:
		*pString++ = '0' + minutes / 10;
		*pString++ = '0' + minutes % 10;
		}
			
	
	if ( (timeFormat == tfColonAMPM) || (timeFormat == tfDotAMPM) ||
		(timeFormat == tfHoursAMPM))
		{
		if (hours >= 12)
			*pString++ = 'p';
		else
			*pString++ = 'a';
		*pString++ = 'm';
		}

	*pString++ = '\0';
}
*/


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:		DeleteAlarm
 *
 * DESCRIPTION:	Deletes the specified alarm from the attention manager
 *						queue. 
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		uniqueID	- the uniqueID of the event
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	07/27/00	Initial Revision
 *
 ***********************************************************************/
static void DeleteAlarm (UInt32 uniqueID)
{
	UInt16 		cardNo;
	LocalID 		dbID;
	DmSearchStateType searchInfo;

	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
	AttnForgetIt(cardNo, dbID, uniqueID);
}


/***********************************************************************
 *
 * FUNCTION:		DrawListAlarm
 *
 * DESCRIPTION:	Draws the alarm info in attention manager list view. 
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		eventTime	- pointer to time of event 
 *						duration		- pointer to duration of event 
 *						untimed		- specifies if this is an untimed event
 *						description	- pointer to the text to be displayed to describe event
 *						paramsPtr	- pointer to attention manager structure containing info
 *										  as to where to draw			
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/04/00	Initial Revision
 *			CS		11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/
static void DrawListAlarm (UInt32 eventTime, UInt16 duration, char* description, Boolean untimed, AttnCommandArgsType *paramsPtr)
{
	Char 							dateStr[longDateStrLength];
	Char							timeStr [timeStringLength];
 	UInt16						lineCount = 0;
 	UInt16						length;
 	Int16							descLen;
	Int16							descFitWidth;
	Int16							descFitLen;
	UInt16						maxWidth;
 	Int16							x, y, iconOffset;
	FontID						curFont;
	MemHandle					resH;
	Char*							resP;
	Char*							ptr;
	DateTimeType 				startDateTime, endDateTime, today;
	DateFormatType				dateFormat;
	TimeFormatType				timeFormat;
	Boolean						fit;
 	Char							chr;
	MemHandle					smallIconH;
	BitmapPtr					smallIconP;


	// Get the date and time formats.
	dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
	timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Compute the maximum width of a line of the description.
	maxWidth = paramsPtr->drawList.bounds.extent.x;

	// Set the font used to draw the alarm info
	//curFont = FntSetFont (FossilStdFont);
	curFont = FntSetFont (stdFont);

	y = paramsPtr->drawList.bounds.topLeft.y;
	x = paramsPtr->drawList.bounds.topLeft.x;

	//draw the application's small icon
	smallIconH = DmGet1Resource(iconType, 1001);
	if (smallIconH) 
		{	
    Coord width;
		smallIconP = (BitmapPtr)(MemHandleLock(smallIconH));
    BmpGetDimensions(smallIconP, &width, NULL, NULL);
		iconOffset = (kAttnListMaxIconWidth - width)/2;
		WinDrawBitmap(smallIconP, x+iconOffset, y);
		MemHandleUnlock (smallIconH);
		DmReleaseResource(smallIconH);
		}
	
	x += kAttnListTextOffset;
#if	WRISTPDA
	y -= 2;  // Bring text a bit higher so bottom message "ggg" fits
#endif

	TimSecondsToDateTime (eventTime, &startDateTime);	
	
	// draw the time information for the event if the event has a time and the duration is > 0.
	if (!untimed)
		{
		// Draw the event's start time
		TimeToAscii (startDateTime.hour, startDateTime.minute, timeFormat, timeStr);
	
		WinDrawChars (timeStr, StrLen (timeStr), x, y);
		x += FntCharsWidth (timeStr, StrLen (timeStr));

		// draw the event's end time if its duration is > 0
		if (duration > 0)
			{
			x += (FntCharWidth(spaceChr) / 2);
			chr = '-';
			WinDrawChars (&chr, 1, x, y);
			x += FntCharWidth (chr) + (FntCharWidth(spaceChr) / 2);
		
			TimSecondsToDateTime (eventTime + (duration * minutesInSeconds), &endDateTime);
			TimeToAscii (endDateTime.hour, endDateTime.minute, timeFormat, timeStr);
			WinDrawChars (timeStr, StrLen (timeStr), x, y);
			x += FntCharsWidth (timeStr, StrLen (timeStr)) + FntCharWidth(spaceChr);
			}
		else
			x += FntCharWidth (spaceChr);
		}


	// Draw the event's date
	// If the event occurs today, draw the 
	TimSecondsToDateTime (TimGetSeconds(), &today);	

	if ( (today.day == startDateTime.day) && (today.month == startDateTime.month) && (today.year == startDateTime.year))
		{
		resH = DmGetResource (strRsc, alarmTodayStrID);
		resP = MemHandleLock(resH);
		WinDrawChars (resP, StrLen (resP), x, y);
		MemPtrUnlock (resP);
		}
	else
		{
		DateToAscii(startDateTime.month, startDateTime.day, startDateTime.year, dateFormat, dateStr);
		WinDrawChars (dateStr, StrLen (dateStr), x, y);
		}
	
	// Draw the event's description.
	x = paramsPtr->drawList.bounds.topLeft.x + kAttnListTextOffset;
	y += FntLineHeight();
#if	WRISTPDA
	y -= 1;  // Bring text a bit higher to leave more space between items
				// it is OK to use the space from the line above because it's
				// only numbers, "pm" and "Today" which does not go as low as "g"
				// Hopefully "Today" in other languages will fit? "Aujourd'hui" in French fits
#endif
	maxWidth = paramsPtr->drawList.bounds.extent.x - kAttnListTextOffset;
	
	ptr = description;
	descLen = StrLen(ptr);

	while(descLen)
		{
		descFitWidth = maxWidth;
		descFitLen = descLen;
		
		// Calculate how many characters will fit in the window bounds
		FntCharsInWidth (ptr, &descFitWidth, &descFitLen, &fit);
		if (!descFitLen)
			break;
			
		// Calculate the number of characters in full words that will fit in the bounds
		length = FldWordWrap(ptr, maxWidth);
		
		// Need to display the minimum of the two as FldWordWrap includes carriage returns, tabs, etc.
		descFitLen = min(descFitLen, length);

		if (++lineCount >= alarmListDescMaxLine) {
			if (descLen != descFitLen)
				descFitLen = descLen;
				
			// DOLATER еее - make sure chrEllipsis is international OK
			if (descFitWidth < maxWidth)
				descFitWidth += FntCharWidth(chrEllipsis);
			
			descFitWidth = min(descFitWidth, maxWidth);
			
			WinDrawTruncChars(ptr, descFitLen, x, y, descFitWidth);
			break;
			}
		else 
			WinDrawTruncChars(ptr, descFitLen, x, y, maxWidth);
			
		descLen -= length;
		ptr += length;
		
		y += FntLineHeight();
		}

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:		DrawDetailAlarm
 *
 * DESCRIPTION:	Draws the alarm info in attention manager detail view. 
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		eventTime	- pointer to time of event 
 *						duration		- pointer to duration of event 
 *						untimed		- specifies if this is an untimed event
 *						description	- pointer to the text to be displayed to describe event
 *						paramsPtr	- pointer to attention manager structure containing info
 *										  as to where to draw			
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/04/00	Initial Revision
 *			CS		11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/
static void DrawDetailAlarm (UInt32 eventTime, UInt16 duration, char* description, Boolean untimed, AttnCommandArgsType *paramsPtr)
{
	FontID			curFont;
 	Int16				x, y;
	DateTimeType	dateTime;
 	UInt16			length;
	UInt16			maxWidth;
 	Int16				descLen;
	Int16				descFitWidth;
	Int16				descFitLen;
	Boolean			fit;
 	UInt16			lineCount = 0;
	Char				dowNameStr[dowDateStringLength];
	Char 				dateStr[longDateStrLength];
	Char				timeStr [timeStringLength];
	MemHandle		resH;
	Char*				resP;
	Char*				ptr;
 	Char				chr;
	DateFormatType	dateFormat;
	TimeFormatType	timeFormat;

#if		WRISTPDA
	#define			alarmDateFont		FossilBoldFont
	#define			alarmTimeFont		FossilBoldFont
	#define			alarmDetailsFont	FossilBoldFont
#else
	#define			alarmDateFont		boldFont
	#define			alarmTimeFont		boldFont
	#define			alarmDetailsFont	boldFont
#endif//	WRISTPDA

	// Get the date and time formats.
	dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
	timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Draw the alarm clock icon
	resH = DmGetResource(bitmapRsc, AlarmClockIcon);
#if		WRISTPDA
	WinDrawBitmap(MemHandleLock(resH), paramsPtr->drawDetail.bounds.topLeft.x, paramsPtr->drawDetail.bounds.topLeft.y+alarmDetailDescYTextOffset);
#else
	WinDrawBitmap(MemHandleLock(resH), paramsPtr->drawDetail.bounds.topLeft.x,
				paramsPtr->drawDetail.bounds.topLeft.y +alarmIconYOffset);
#endif//	WRISTPDA
	MemHandleUnlock(resH);
	DmReleaseResource(resH);

	// Set the font used to draw the alarm info
	curFont = FntSetFont (alarmDateFont);

	// Draw the date - for English this will be "Monday, <date>".
	x = paramsPtr->drawDetail.bounds.topLeft.x + alarmDetailDescHTextOffset;
	y = paramsPtr->drawDetail.bounds.topLeft.y + alarmDetailDescYTextOffset;
	
	TimSecondsToDateTime (eventTime, &dateTime);

	// Get the day-of-week name and the system formatted date
	DateTemplateToAscii("^1l", dateTime.month, dateTime.day, dateTime.year, dowNameStr, sizeof(dowNameStr));

#if		WRISTPDA
	DateToAscii(dateTime.month, dateTime.day, 99, dateFormat, dateStr);

	// Remove the year from the string
	ptr = StrStr( dateStr, "99" );
	if (ptr)
	{
		length = StrLen(ptr);
		if (length == 2)
		{
			ptr--;
			ptr[0] = '\0';
		}
		else
		{
			MemMove( ptr, ptr +3, StrLen(ptr+3) +1 );
		}
	}
#else
	DateToAscii(dateTime.month, dateTime.day, dateTime.year, dateFormat, dateStr);
#endif//	WRISTPDA
	
	resH = DmGetResource (strRsc, drawAlarmDateTemplateStrID);
	resP = MemHandleLock (resH);
	ptr = TxtParamString(resP, dowNameStr, dateStr, NULL, NULL);
	MemPtrUnlock(resP);
	
	// DOLATER kwk - what should happen if the string needs to be truncated? Go
	// onto a following line (wrap it)?
	WinDrawChars(ptr, StrLen(ptr), x, y);
	MemPtrFree((MemPtr)ptr);

	y += FntLineHeight();

	if (alarmTimeFont != alarmDateFont)
		FntSetFont (alarmTimeFont);

	// the time of the event if the event has a time.
	if (!untimed)
		{
		// Draw the event's time and duration.
		TimeToAscii (dateTime.hour, dateTime.minute, timeFormat, timeStr);
	
		WinDrawChars (timeStr, StrLen (timeStr), x, y);
		
		if (duration > 0)
			{
			x += FntCharsWidth (timeStr, StrLen (timeStr)) + FntCharWidth (spaceChr);
			chr = '-';
			WinDrawChars (&chr, 1, x, y);
			
			TimSecondsToDateTime (eventTime + (duration * minutesInSeconds), &dateTime);
			TimeToAscii (dateTime.hour, dateTime.minute, timeFormat, timeStr);
			x += FntCharWidth (chr) + FntCharWidth (spaceChr);
			WinDrawChars (timeStr, StrLen (timeStr), x, y);
			}
		}

	if (alarmDetailsFont != alarmTimeFont)
		FntSetFont (alarmDetailsFont);

	// Draw the event's description.
	y += alarmDescSeparatorY;
	x = paramsPtr->drawDetail.bounds.topLeft.x + alarmDetailDescHTextOffset;
	maxWidth = paramsPtr->drawDetail.bounds.extent.x - x;
	
	ptr = description;
	descLen = StrLen(ptr);
	while(descLen)
		{
		descFitWidth = maxWidth;
		descFitLen = descLen;
		
		// Calculate how many characters will fit in the window bounds
		FntCharsInWidth (ptr, &descFitWidth, &descFitLen, &fit);
		if (!descFitLen)
			break;
			
		// Calculate the number of characters in full words that will fit in the bounds
		length = FldWordWrap  (ptr, maxWidth);
		
		// Need to display the minimum of the two as FldWordWrap includes carriage returns, tabs, etc.
		descFitLen = min(descFitLen, length);

		y += FntLineHeight();


		if (++lineCount >= alarmDetailDescMaxLine) {
			if (descLen != descFitLen)
				descFitLen = descLen;
				
			// DOLATER еее - make sure chrEllipsis is international OK
			if (descFitWidth < maxWidth)
				descFitWidth += FntCharWidth(chrEllipsis);
			
			descFitWidth = min(descFitWidth, maxWidth);
			
			WinDrawTruncChars(ptr, descFitLen, x, y, descFitWidth);
			break;
			}
		else 
			WinDrawTruncChars(ptr, descFitLen, x, y, maxWidth);
			
		descLen -= length;
		ptr += length;
		}

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:		DrawAlarm
 *
 * DESCRIPTION:	Does the initial validation an setup for alarm drawing then
 *						calls the appropriate routine for either list or detailed
 *						display. 
 *						THIS MAY BE CALLED AT INTERRUPT LEVEL, SO DONT USE GLOBALS!!
 *
 * PARAMETERS:		uniqueID		- the uniqueID of the event
 *						paramsPtr	- info provided by attention manager
 *						drawDetail	- boolean specifying Detail or list view				
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	10/04/00	Initial Revision
 *			peter	01/22/01	Add alarmPaddingSeconds to work around Alarm Mgr flaw
 *
 ***********************************************************************/
static void DrawAlarm (UInt32 inUniqueID, AttnCommandArgsType *paramsPtr, Boolean drawDetail)
{
	Err						err;
	DmOpenRef 				dbP;
	UInt16 					duration = 0;
	Int32 					adjust;
	UInt32 					eventTime;
	Boolean					untimed;
	MemHandle				resH = 0;
	MemHandle				recordH;
	Char*						description;
	ApptDBRecordType 		apptRec;
	UInt16 					attr;
	Boolean					displayPrivate = false;
	UInt16  					recordNum;


	// Open the appointment database.
	dbP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadOnly);
	if (!dbP) return;

	// Validate that uniqueID is still valid and get the associated record number.
	err = FindRecordByID (dbP, inUniqueID, &recordNum);
	if (err)
		{
		DeleteAlarm(inUniqueID);
		goto Exit;
		}

	ApptGetRecord (dbP, recordNum, &apptRec, &recordH);
		
	// If the device is locked, it is still possible for the app to 
	// get called by the attention manager in order to display information
	// for  an event whose alarm has just triggered.  In this case
	// DateBook must always display the "Private Record" string regardless
	// of the setting of the event's private bit.  Otherwise,
	// if the user has selected that private records be masked or hidden
	// determine if the private bit has been set for the record, if it is
	// we need to show the string "Private Appointment" in place of the 
	// actual appointment title.
	if (PrefGetPreference(prefDeviceLocked))
		displayPrivate = true;
	else if ( PrefGetPreference(prefShowPrivateRecords) != showPrivateRecords )
		{
		DmRecordInfo (dbP, recordNum, &attr, NULL, NULL);
		displayPrivate = (attr & dmRecAttrSecret);
		}

	if (displayPrivate)
		{
		resH = DmGetResource (strRsc, alarmPrivateApptStrID);
		description = MemHandleLock(resH);
		}
	else
		description = apptRec.description;

	
	// Calculate the event's date and time from the alarm time and the alarm
	// advance.  The date and time stored in the record will not be
	// the correct values to display, if the event is repeating.
	adjust = apptRec.alarm->advance;
	switch (apptRec.alarm->advanceUnit)
		{
		case aauMinutes:
			adjust *= minutesInSeconds;
			break;
		case aauHours:
			adjust *= hoursInSeconds;
			break;
		case aauDays:
			adjust *= daysInSeconds;
			break;
		}	
	
	// The alarm manager may trigger a bit early, so to be safe, search
	// backwards for the appropriate alarm time, not from the current time,
	// but from a bit into the future. Since the fastest repeat rate for
	// alarms is once a day, this won't cause any significant problems.
	// If displaying a repeating event alarm just before the next occurrence
	// is about to go off, you may see the new alarm time, but that's not
	// a serious problem.
	eventTime = ApptGetAlarmTime (&apptRec, TimGetSeconds() + alarmPaddingSeconds, false);
	ErrNonFatalDisplayIf(eventTime == 0, "No alarm before now");
	eventTime = eventTime + adjust;

	// Calculate the duration of the event.
	untimed = (TimeToInt (apptRec.when->startTime) == (UInt16)apptNoTime);
	if (!untimed)
		duration = (apptRec.when->endTime.hours * hoursInMinutes + apptRec.when->endTime.minutes) -
					  (apptRec.when->startTime.hours * hoursInMinutes + apptRec.when->startTime.minutes);


	if (drawDetail)
		DrawDetailAlarm (eventTime, duration, description, untimed, paramsPtr);
	else
		DrawListAlarm(eventTime, duration, description, untimed, paramsPtr);

	if (displayPrivate)
		MemHandleUnlock (resH);

	MemHandleUnlock (recordH);

Exit:
	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:		ValidateAlarm
 *
 * DESCRIPTION:	This routine validates the item with the specified 
 *						uniqueID to determine if it still exists as well as if 
 *						its alarm is still valid.  This routine is called to
 *						validate alarms in the attention manager queue following
 *						a device time change, a hotsync, and an attention manager 
 *						"tickle" 
 *
 * PARAMETERS:  	uniqueID  - the unique ID of the event
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	08/15/00	Initial Revision
 *
 ***********************************************************************/
static void ValidateAlarm (UInt32 uniqueID)
{
	DmOpenRef	dbP;
	Err			err;
	UInt16		recordNum;
	UInt16 		cardNo;
	LocalID 		dbID;
	Boolean		valid = true;
	MemHandle	recordH;
	ApptDBRecordType	apptRec;
	DmSearchStateType searchInfo;


	dbP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadOnly);
	if (!dbP) return;

	// Determine if the record still exists
	err = FindRecordByID (dbP, uniqueID, &recordNum);
	if (err) valid = false;

	// Now get the event's info to determine if the alarm time is <= the current time
	// if it is greater than the current time, forget it and let it ring again when
	// the device reaches its specified time.
	if (valid)
		{
		ApptGetRecord (dbP, recordNum, &apptRec, &recordH);
		ErrFatalDisplayIf ((!recordH), "Bad record");
		valid = ( (apptRec.alarm) && ApptGetAlarmTime (&apptRec, TimGetSeconds(), false) );
		MemHandleUnlock (recordH);
		}

	// If one of the previous tests failed, remove the alarm from the attention manager queue
	if (!valid)
		{
		DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
		AttnForgetIt(cardNo, dbID, uniqueID);
		}

	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:		GotoAlarm
 *
 * DESCRIPTION:	This code handle user specification in an attention
 *						manager dialog (both list view and details view).
 *
 * PARAMETERS:  	uniqueID  - the unique ID of the event
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	09/06/00	Initial Revision
 *
 ***********************************************************************/
static void GotoAlarm (UInt32 uniqueID)
{
	DmOpenRef	dbP;
	UInt16		recordNum;
	UInt16 		cardNo;
	LocalID 		dbID;
	DmSearchStateType searchInfo;
	UInt32		*gotoInfoP;
	Err			err;

	// get the application's card number an dbID
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);

	// verify that the specified uniqueID is still in the Date Book's database
	dbP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadOnly);
	if (!dbP) return;
	err = DmFindRecordByID(dbP, uniqueID, &recordNum);
	DmCloseDatabase (dbP);
	
	// if the unique ID no longer exists, just remove the attention from the attention manager queue and
	// return
	if (err == dmErrUniqueIDNotFound)
		{
		AttnForgetIt(cardNo, dbID, uniqueID);
		return;
		}
		
	// if we received a goto for a valid unique ID	
	// create the pointer to contain the goto information
	gotoInfoP = (UInt32*)MemPtrNew (sizeof(UInt32));
	ErrFatalDisplayIf ((!gotoInfoP), "Out of memory");
	MemPtrSetOwner(gotoInfoP, 0);

	// initialize the goto params structure so that datebook will open day view 
	// with the specified item selected
	*gotoInfoP = uniqueID;

#if EMULATION_LEVEL == EMULATION_NONE
	// Launch DateBookEMULATION_LEVEL with the corresponding launch code.
	SysUIAppSwitch(cardNo, dbID, appLaunchCmdAlarmEventGoto, gotoInfoP);
#else

	GoToAlarmItem(uniqueID);
#endif
	
}


/***********************************************************************
 *
 * FUNCTION:		AttentionBottleNeckProc
 *
 * DESCRIPTION:	Main bottleneck proc whihc processes attention manager 
 *						launch codes.
 *
 * PARAMETERS:  	paramP -  the launch code specific information supplied
 *						by the attention manager.
 *
 * RETURNED:    	true if the alarm was found/deleted
 *						false if alarm was not posted
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	07/31/00	Initial Revision
 *
 ***********************************************************************/
Boolean AttentionBottleNeckProc(AttnLaunchCodeArgsType * paramP)
{
	AttnCommandArgsType * argsP = paramP->commandArgsP;

	switch (paramP->command) 
		{
		case kAttnCommandDrawDetail:
			DrawAlarm ( paramP->userData, paramP->commandArgsP, true);
			break;			

		case kAttnCommandDrawList:
			DrawAlarm ( paramP->userData, paramP->commandArgsP, false);
			break;
		
		case kAttnCommandPlaySound:
			{
			DatebookPreferenceType prefs;
			
			// Load Date Book's prefs so we can get the user-specified alarm sound.
			DatebookLoadPrefs (&prefs);

			PlayAlarmSound(prefs.alarmSoundUniqueRecID);
			}
			break;		
						
		case kAttnCommandGotIt:
			{
			if (argsP->gotIt.dismissedByUser)
				DeleteAlarm(paramP->userData);
			}
			break;			

		case kAttnCommandGoThere:
			GotoAlarm(paramP->userData);		
			break;	

		case kAttnCommandIterate:
			// if the argument is nil, this is a "tickle from the attention manager
			// asking the application to validate the specified entry
			// this may happen at interrupt time - do not use globals here
			if (argsP == NULL)
				ValidateAlarm(paramP->userData);
			
			// otherwise, this launch code was received from attention manager in
			// response to an AttnIterate made by Date Book to update the posted alarms
			// with respect to one of the following occurrences
			else
				switch (argsP->iterate.iterationData) 
					{
					// When the user changes the nag parameters, assign the new value to each alarm currently
					// in the attention manager queue.
					// THIS WILL ONLY OCCUR WHEN APP IS RUNNING SO CACHED GLOBALS ARE USED FOR SPEED
					case SoundRepeatChanged:
						{
						UInt16 		cardNo;
						LocalID 		dbID;
						DmSearchStateType searchInfo;
	
						DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
						AttnUpdate(cardNo, dbID, paramP->userData, NULL, NULL, &AlarmSoundRepeatInterval, &AlarmSoundRepeatCount);
						break;			
						}
						
					default:
						ValidateAlarm(paramP->userData);
					}
			break;	
			
//		case AttnCommand_nag:		// ignored by DateBook
//			break;
		}

	return true;
}



/***********************************************************************
 *
 * FUNCTION:		DeleteAlarmIfPosted
 *
 * DESCRIPTION:	Get the unique ID of the current record number and call
 *						AttnForgetIt to remove it form the attention manager 
 *						queue.
 *
 * PARAMETERS:  	recordNum -  the event's record number
 *
 * RETURNED:    	true if the alarm was found/deleted
 *						false if alarm was not posted
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	07/31/00	Initial Revision
 *			gap	12/11/00	Add param check for recordnum to be sure it is not noRecordSelected
 *
 ***********************************************************************/
Boolean DeleteAlarmIfPosted (UInt16 recordNum)
{
	UInt16 		cardNo;
	LocalID 		dbID;
	DmSearchStateType searchInfo;
	UInt32 		uniqueID;
	Err			err;


	// Shouls always have a valid record number when this routine is called.
	ErrNonFatalDisplayIf(recordNum == noRecordSelected, "trying to delete recordNum of noRecordSelected from attn mgr queue ");
	if (recordNum == noRecordSelected)
		return false;
			
	// get the unique ID for the specified record
	err = DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL);
	if (err) return false;

	// get the card number & dataBase ID for the app
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
	
	// remove the alarm from the attention manager queue if it is present
	return (AttnForgetIt(cardNo, dbID, uniqueID));
}


/***********************************************************************
 *
 * FUNCTION:		UpdatePostedAlarms
 *
 * DESCRIPTION:	Updates the sound and nag information in the posted
 *						alarms when the prefs are changed by the user
 *
 * PARAMETERS:  	none
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	08/02/00	Initial Revision
 *
 ***********************************************************************/
void UpdatePostedAlarms (AlarmUpdateType  updateType)
{
	UInt16 		cardNo;
	LocalID 		dbID;
	DmSearchStateType searchInfo;
	UInt32		iterateType = updateType;

	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
	AttnIterate(cardNo, dbID, iterateType);
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AlarmGetTrigger
 *
 * DESCRIPTION: This routine gets the time of the next scheduled
 *              alarm from the Alarm Manager.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    the time of the next scheduled alarm, or zero if 
 *              no alarm is scheduled.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/1/95	Initial Revision
 *			rbb	4/21/99	Was GetTimeOfNextAlarm. Renamed to avoid
 *								confusion with ApptGetTimeOfNextAlarm.
 *
 ***********************************************************************/
UInt32 AlarmGetTrigger (UInt32* refP)
{
	UInt16 		cardNo;
	LocalID 		dbID;
	UInt32		alarmTime = 0;	
	DmSearchStateType searchInfo;


	// get the card number & dataBase ID for the app
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);

	alarmTime = AlmGetAlarm (cardNo, dbID, refP);

	return (alarmTime);
}


/***********************************************************************
 *
 * FUNCTION:    AlarmSetTrigger
 *
 * DESCRIPTION: This routine set the time of the next scheduled
 *              alarm. 
 *
 * PARAMETERS:  the time of the next scheduled alarm, or zero if 
 *              no alarm is scheduled.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/1/95	Initial Revision
 *			rbb	4/21/99	Renamed from SetTimeOfNextAlarm, in parallel
 *								with rename of GetTimeOfNextAlarm
 *
 ***********************************************************************/
void AlarmSetTrigger (UInt32 alarmTime, UInt32 ref)
{
	UInt16 cardNo;
	LocalID dbID;
	DmSearchStateType searchInfo;


	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
	AlmSetAlarm (cardNo, dbID, ref, alarmTime, true);
}




/***********************************************************************
 *
 * FUNCTION:	AlarmTriggered
 *
 * DESCRIPTION:	This routine is called when the alarm manager informs the 
 *						datebook application that an alarm has triggered.
 *						THIS IS CALLED AT INTERRUPT LEVEL! DONT USE GLOBALS!!
 *
 * PARAMETERS:	time of the alarm.
 *
 * RETURNED:	time of the next alarm
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		9/20/95	Initial Revision
 *			frigino	9/9/97	Switched use of globals to use of prefs
 *									values instead.
 *			vmk		12/9/97	Call DatebookLoadPrefs() to load/fixup our app prefs
 *			rbb		04/22/99	Added snooze feature
 *			gap		08/08/00	Rewritten for attention manager support
 *			gap		08/08/00	Rewritten again for attention manager support
 *									now post alarms to attention manager during the 
 *									triggered launch code in stead of the display
 *
 ***********************************************************************/
void AlarmTriggered (SysAlarmTriggeredParamType * cmdPBP)
{
	DmOpenRef					dbP;
	UInt32						alarmTime;
	
	// all triggered alarms are sent to attention manager for display so there is
	// no need for alarm manager to send the sysAppLaunchCmdDisplayAlarm launchcode
	cmdPBP->purgeAlarm = true;

	// Open the appointment database.
	dbP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadOnly);
	if (!dbP) return;
	
	// Establish the time for which alarms need to be retrieved.
	alarmTime= cmdPBP->alarmSeconds;
	
	ApptPostTriggeredAlarms(dbP, alarmTime);

	// Set the alarm trigger for the time of the next alarm to ring.
	AlarmSetTrigger(ApptGetTimeOfNextAlarm (dbP, alarmTime + minutesInSeconds), 0);

	#if	EMULATION_LEVEL != EMULATION_NONE
		ECApptDBValidate (dbP);
	#endif
	
	// Close the appointment database.
	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    RescheduleAlarms
 *
 * DESCRIPTION: This routine computes the time of the next alarm and 
 *              compares it to the time of the alarm scheduled with
 *              Alarm Manager,  if they are different it reschedules
 *              the next alarm with the Alarm Manager.
 *
 * PARAMETERS:  dbP - the appointment database
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/5/95	Initial Revision
 *			rbb	4/22/99	Snoozing now disables other rescheduling of alarms
 *			gap	9/25/00	removed snoozing & alarm internals
 *
 ***********************************************************************/
void RescheduleAlarms (DmOpenRef dbP)
{
	UInt32 ref;
	UInt32 timeInSeconds;
	UInt32 nextAlarmTime;
	UInt32 scheduledAlarmTime;
	
	
	scheduledAlarmTime = AlarmGetTrigger (&ref);
	timeInSeconds = TimGetSeconds();
	if ((timeInSeconds < scheduledAlarmTime) || (scheduledAlarmTime == 0))
		scheduledAlarmTime = timeInSeconds;
	
	nextAlarmTime = ApptGetTimeOfNextAlarm (dbP, scheduledAlarmTime);
	
	// If the scheduled time of the next alarm is not equal to the
	// calculated time of the next alarm,  reschedule the alarm with 
	// the alarm manager.
	if (scheduledAlarmTime != nextAlarmTime)
		AlarmSetTrigger (nextAlarmTime, 0);
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AlarmReset
 *
 * DESCRIPTION: This routine is called when the system time is changed
 *              by the Preference application, or when the device is reset.
 *                
 * PARAMETERS:  newerOnly - If true, we will not reset the alarm if the 
 *              time of the next (as calculated) is greater then the 
 *              currently scheduled alarm.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/21/95	Initial Revision
 *       art	11/18.96	Reseting time will nolong trigger alarms.
 *			rbb	4/22/99	Reset snooze and any pending alarms
 *
 ***********************************************************************/
void AlarmReset (Boolean newerOnly)
{
	UInt32 ref;
	UInt32 currentTime;
	UInt32 alarmTime;
	DmOpenRef dbP;
		

	if (newerOnly)
		{
		alarmTime = AlarmGetTrigger (&ref);
		currentTime = TimGetSeconds ();
		if ( alarmTime && (alarmTime <= currentTime))
			return;
		}

	// Clear any pending alarms.
	AlarmSetTrigger (0, 0);
	
	// Open the appointment database.
	dbP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadOnly);
	if (!dbP) return;

	RescheduleAlarms (dbP);

	#if	EMULATION_LEVEL != EMULATION_NONE
		ECApptDBValidate (dbP);
	#endif

	DmCloseDatabase (dbP);
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    PlayAlarmSound
 *
 * DESCRIPTION:	Play a MIDI sound given a unique record ID of the MIDI record in the System
 *						MIDI database.  If the sound is not found, then the default alarm sound will
 *						be played.
 *
 * PARAMETERS:  uniqueRecID	-- unique record ID of the MIDI record in the System
 *										   MIDI database.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	8/10/97	Initial version
 *			trev	08/14/97	Ported to dateBook App
 *			vmk	12/11/97	Prevent call to DmFindRecordByID if DmOpenDatabase failed
 *
 ***********************************************************************/
void PlayAlarmSound(UInt32 uniqueRecID)
{
	Err			err;
	MemHandle	midiH;							// handle of MIDI record
	SndMidiRecHdrType*	midiHdrP;			// pointer to MIDI record header
	UInt8*		midiStreamP;					// pointer to MIDI stream beginning with the 'MThd'
														// SMF header chunk
	UInt16		cardNo;							// card number of System MIDI database
	LocalID		dbID;								// Local ID of System MIDI database
	DmOpenRef	dbP = NULL;						// reference to open database
	UInt16		recIndex;						// record index of the MIDI record to play
	SndSmfOptionsType	smfOpt;					// SMF play options
	DmSearchStateType	searchState;			// search state for finding the System MIDI database
	Boolean		bError = false;				// set to true if we couldn't find the MIDI record
	
		
	// Find the system MIDI database
	err = DmGetNextDatabaseByTypeCreator(true, &searchState,
			 		sysFileTMidi, sysFileCSystem, true, 
			 		&cardNo, &dbID);
	if ( err )
		bError = true;														// DB not found
	
	// Open the MIDI database in read-only mode
	if ( !bError )
		{
		dbP = DmOpenDatabase (cardNo, dbID, dmModeReadOnly);
		if ( !dbP )
			bError = true;													// couldn't open
		}
	
	// Find the MIDI track record
	if ( !bError )
		{
		err = DmFindRecordByID (dbP, uniqueRecID, &recIndex);
		if ( err )
			bError = true;														// record not found
		}
		
	// Lock the record and play the sound
	if ( !bError )
		{
		// Find the record handle and lock the record
		midiH = DmQueryRecord(dbP, recIndex);
		midiHdrP = MemHandleLock(midiH);
		
		// Get a pointer to the SMF stream
		midiStreamP = (UInt8*)midiHdrP + midiHdrP->bDataOffset;
		
		// Play the sound (ignore the error code)
		// The sound can be interrupted by a key/digitizer event
		smfOpt.dwStartMilliSec = 0;
		smfOpt.dwEndMilliSec = sndSmfPlayAllMilliSec;
		smfOpt.amplitude = PrefGetPreference(prefAlarmSoundVolume);
		smfOpt.interruptible = true;
		smfOpt.reserved = 0;
		err = SndPlaySmf (NULL, sndSmfCmdPlay, midiStreamP, &smfOpt, NULL, NULL, false);
		
		// Unlock the record
		MemPtrUnlock (midiHdrP);
		}
	
	// Close the MIDI database
	if ( dbP )
		DmCloseDatabase (dbP);
		
	
	// If there was an error, play the alarm sound resource stored with the app.
	// This is equivalent to SndPlaySystemSound(sndAlarm), but the sound is
	// interruptible.
	if ( bError )
		SndPlaySmfResource(midiRsc, alarmSoundID, prefAlarmSoundVolume);
}
