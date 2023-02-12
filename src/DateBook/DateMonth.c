/******************************************************************************
 *
 * Copyright (c) 1996-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateMonth.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This is the Datebook application's month view module.
 *
 * History:
 *		April 8, 1996	Created by Art Lamb
 *
 *****************************************************************************/

#include <PalmOS.h>

#include <DebugMgr.h>
//#include <StdIOPalm.h>

#include <PalmUtils.h>

#include "sections.h"
#include "Datebook.h"

extern void PrintApptRecord (DmOpenRef dbP, UInt16 index);


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define linesInMonthPlusTitle			7
#define maxWeeksInMonth					6

// Day number's margins
#define textLeftMargin					3
#define textTopMargin					1


// Event indicators
#define lunchStartTime					0x0b1e		// 11:30 am
#define lunchMidTime						0x0c0f		// 12:15 pm
#define lunchEndTime						0xd00			//  1:00 pm

#define timeIndicatorWidth				2
#define timeIndicatorTopMargin		3
#define timeIndicatorRightMargin		2

#define untimeIndicatorTopMargin		13
#define untimeIndicatorRightMargin	5

#define repeatIndicatorBottomMargin	2


#define morningHeight					4
#define lunchHeight						3
#define afternoonHeight					4

#define drawDaySelected					true
#define dontDrawDaySelected			false

#define TodayIsVisible(monthP,today) ((monthP->month == today.month) && (monthP->year == today.year))

#if WRISTPDA
// The current selected day.
static UInt16  SelectedDay = 1;
static Boolean InMonthViewHighlightSelectedDay = false;
#endif


/***********************************************************************
 *
 *	Internal Structutes
 *
 ***********************************************************************/
typedef struct {
	RectangleType			bounds;
	Int16						month;			// month displayed
	Int16						year;				// year displayed
	DateTimeType			selected;
} MonthType;

typedef MonthType * MonthPtr;


#if WRISTPDA
// Function Prototypes
static void MonthSelectDayRect( MonthPtr monthP, RectangleType *r, Boolean selected );
static void MonthViewHighlightSelectedDay( Boolean Highlight );
#endif


/***********************************************************************
 *
 * FUNCTION:    FirstDayOfMonth
 *
 * DESCRIPTION: This routine returns the day-of-the-week, adjusted by the 
 *              preference setting that specifies the first day-of-
 *              the-week.  If the date passed is a Tuesday and the 
 *              start day of week is Monday, this routine will return
 *              a value of one.
 *
 * PARAMETERS:  monthP     - pointer to MontnType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *
 ***********************************************************************/
static UInt16 FirstDayOfMonth (MonthPtr monthP) EXTRA_SECTION_FOUR;
static UInt16 FirstDayOfMonth (MonthPtr monthP)
	{
	return (DayOfWeek (monthP->month, 1, monthP->year) - 
			StartDayOfWeek + daysInWeek) % daysInWeek;
	}


/***********************************************************************
 *
 * FUNCTION:    MonthDrawInversionEffect
 *
 * DESCRIPTION: This routine does an inversion effect by swapping colors
 *					 this is NOT undoable by calling it a second time, rather
 *					 it just applies a selected look on top of already
 *					 rendered data.  (It's kind of a hack.)
 *
 * PARAMETERS:	 rP - pointer to a rectangle to 'invert'
 *					 cornerDiam - corner diameter
 *					 
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bob	10/28/99	Initial revision
 *			jmp	11/14/99	Modified for use with Datebook's MonthView (i.e., backcolor
 *								changed to UIObjectFill).
 *
 ***********************************************************************/
static void MonthDrawInversionEffect (const RectangleType *rP, UInt16 cornerDiam) EXTRA_SECTION_FOUR;
static void MonthDrawInversionEffect (const RectangleType *rP, UInt16 cornerDiam) 
{
	WinPushDrawState();
	WinSetDrawMode(winSwap);
	WinSetPatternType (blackPattern);
	WinSetBackColor(UIColorGetTableEntryIndex(UIObjectFill));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
	WinPaintRectangle(rP, cornerDiam);
	
	if (UIColorGetTableEntryIndex(UIObjectSelectedFill) != UIColorGetTableEntryIndex(UIObjectForeground)) {
		WinSetBackColor(UIColorGetTableEntryIndex(UIObjectForeground));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
		WinPaintRectangle(rP, cornerDiam);
	}
	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    GetDayBounds
 *
 * DESCRIPTION: This routine returns the bounds of the specified day
 *
 * PARAMETERS:  monthP  - pointer to month object
 *              day     - day of month
 *              r       - returned: bounds of the day     
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/10/96	Initial Revision
 *
 ***********************************************************************/
static void GetDayBounds (MonthPtr monthP, UInt16 day, RectanglePtr r) EXTRA_SECTION_FOUR;
static void GetDayBounds (MonthPtr monthP, UInt16 day, RectanglePtr r)
	{
	UInt16 cell;
	Int16	cellWidth;
	Int16 cellHeight;

	cell = day - 1 + FirstDayOfMonth (monthP);
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonthPlusTitle;
	r->topLeft.x = monthP->bounds.topLeft.x + (cell % daysInWeek) * cellWidth;
	r->topLeft.y = monthP->bounds.topLeft.y + ((cell / daysInWeek) + 1) * cellHeight;
	r->extent.x = cellWidth;
	r->extent.y = cellHeight;
	}
	

/***********************************************************************
 *
 * FUNCTION:    GetDaySelectionBounds
 *
 * DESCRIPTION: This routine returns the selection bounds of the specified day
 *
 * PARAMETERS:  monthP  - pointer to month object
 *              day     - day of month
 *              r       - returned: bounds of the day selection 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			jmp	11/14/99	Initial Revision
 *			jmp	11/30/99	Grrr... swapped meaning of "cell" and "day" and
 *								broke selection of Sunday vs. Monday start-of-week
 *								selection.  Fixed that!
 *
 ***********************************************************************/
static void GetDaySelectionBounds(MonthPtr monthP, UInt16 day, RectanglePtr r) EXTRA_SECTION_FOUR;
static void GetDaySelectionBounds(MonthPtr monthP, UInt16 day, RectanglePtr r)
{
	UInt16 cell;
	Int16	cellWidth;
	Int16 cellHeight;
	Int16 charWidth;
	
	cell = day - 1 + FirstDayOfMonth (monthP);
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonthPlusTitle;
	#if WRISTPDA
	FntSetFont( FossilStdFont );
	#endif
	charWidth = FntCharWidth('0');
	r->topLeft.x = monthP->bounds.topLeft.x + (cell % daysInWeek) * cellWidth;
	r->topLeft.y = monthP->bounds.topLeft.y + ((cell / daysInWeek) + 1) * cellHeight;
	r->extent.x = ((day < 10) ? charWidth : charWidth * 2) + (textLeftMargin * 2);
	r->extent.y = FntLineHeight () + (textTopMargin * 2);
	RctInsetRectangle (r, 1);
}

/***********************************************************************
 *
 * FUNCTION:    DrawAppointment
 *
 * DESCRIPTION: This routine draw an appointment indicator.
 *
 * PARAMETERS:  dayR  - bounds of the day's drawing region
 *              start - appointment's start time
 *              end   - appointment's end time
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *
 ***********************************************************************/
static void DrawAppointment (RectanglePtr dayR, TimeType start, TimeType end) EXTRA_SECTION_FOUR;
static void DrawAppointment (RectanglePtr dayR, TimeType start, TimeType end)
{
	Int16 x, y;
	RectangleType r;
	
	// Draw the indicator for an untimed appointment.
	if (TimeToInt(start) == apptNoTime)
		{
		x = dayR->topLeft.x + untimeIndicatorRightMargin;
		y = dayR->topLeft.y + untimeIndicatorTopMargin;
		
		WinDrawLine (x - 1, y, x + 1, y);
		WinDrawLine (x, y - 1, x, y + 1);
		return;
		}

	// Draw the indicator for an timed appointment.
	x = dayR->topLeft.x + dayR->extent.x - timeIndicatorWidth - 
		timeIndicatorRightMargin;
	y = dayR->topLeft.y + timeIndicatorTopMargin;

	// Draw indicator for a morning appointment.
	if (TimeToInt(start) < lunchStartTime)
		{
		r.topLeft.x = x;
		r.topLeft.y = y;
		r.extent.x = timeIndicatorWidth;
		r.extent.y = morningHeight;
		WinDrawRectangle (&r, 0);
		}

	// Draw indicator for a lunch appointment.
	if ((TimeToInt(start) >= lunchStartTime && TimeToInt(start) < lunchEndTime) ||
		 (TimeToInt(start) < lunchStartTime && TimeToInt(end) > lunchMidTime))
		{
		r.topLeft.x = x;
		r.topLeft.y = y + morningHeight  + 1;
		r.extent.x = timeIndicatorWidth;
		r.extent.y = lunchHeight;
		WinDrawRectangle (&r, 0);
		}

	// Draw indicator for a afternoon appointment.
	if (TimeToInt(start) >= lunchEndTime ||
		(TimeToInt(start) < lunchEndTime && TimeToInt(end) > lunchEndTime))
		{
		r.topLeft.x = x;
		r.topLeft.y = y + morningHeight + lunchHeight + 2;
		r.extent.x = timeIndicatorWidth;
		r.extent.y = afternoonHeight;
		WinDrawRectangle (&r, 0);
		}
}

/***********************************************************************
 *
 * FUNCTION:    DrawTimedAppointments
 *
 * DESCRIPTION: This routine draws appointment indicators for all the 
 *              appointment in a month.
 *
 * PARAMETERS:  monthP  - month object
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/10/96	Initial Revision
 *
 ***********************************************************************/
static void DrawTimedAppointments (MonthPtr monthP) EXTRA_SECTION_FOUR;
static void DrawTimedAppointments (MonthPtr monthP)
{
	UInt16	recordNum;
	DateType temp;
	DateType date;
	Boolean untimed;
	Boolean sameMonth;
	MemHandle recordH;
	RectangleType dayR;
	ApptDBRecordType apptRec;


	date.day = 1;
	date.month = monthP->month;
	date.year = monthP->year - firstYear;
	
	// Find the first non-repeating appointment of the day.
	ApptFindFirst (ApptDB, date, &recordNum);
	while (true)
		{
		recordH = DmQueryNextInCategory (ApptDB, &recordNum, dmAllCategories);
		if (! recordH) break;

		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);

		// Check if the appointment is on the same month
		sameMonth = (apptRec.when->date.month == monthP->month) && 
						(apptRec.when->date.year == monthP->year - firstYear);

		untimed = TimeToInt(apptRec.when->startTime) == apptNoTime;

		if (sameMonth && 
			 ((ShowUntimedAppts && untimed) || (ShowTimedAppts && (! untimed))))
			{
			GetDayBounds (monthP, apptRec.when->date.day, &dayR);
			DrawAppointment (&dayR, apptRec.when->startTime,
				apptRec.when->endTime);
			}
		MemHandleUnlock (recordH);
		if (! sameMonth) break;

		// Get the next record.
		recordNum++;
		}


	// Add the repeating appointments to the list.  Repeating appointments
	// are stored at the beginning of the database.
	recordNum = 0;
	while (true)
		{
		recordH = DmQueryNextInCategory (ApptDB, &recordNum, dmAllCategories);
		if (! recordH) break;
		
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
		
		untimed = TimeToInt(apptRec.when->startTime) == apptNoTime;

		if (apptRec.repeat && 
			 ((ShowUntimedAppts && untimed) || (ShowTimedAppts && (! untimed))))
			{
			date.day = 1;
			date.month = monthP->month;
			date.year = monthP->year - firstYear;

		 	while (ApptNextRepeat (&apptRec, &date, true))
				{
				if ((date.month != monthP->month) || 
					(date.year != monthP->year - firstYear))
					break;
				
				GetDayBounds (monthP, date.day, &dayR);
				DrawAppointment (&dayR, apptRec.when->startTime, 
					apptRec.when->endTime);

				temp = date;
				DateAdjust (&date, 1);
				if (DateToInt (temp) == DateToInt (date)) break;
				}
			}
		MemHandleUnlock (recordH);

		// If the record has no repeating info we've reached the end of the 
		// repeating appointments.
		if (! apptRec.repeat) break;
		
		recordNum++;
		}
}


/***********************************************************************
 *
 * FUNCTION:    DrawRepeatingAppointments
 *
 * DESCRIPTION: This routine draws appointment indicators for all the 
 *              daily repeating appointments in a month.
 *
 * PARAMETERS:  monthP  - month object
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/96	Initial Revision
 *			roger	6/25/98	Fix from developer to not always draw the first day mark.
 *
 ***********************************************************************/
static void DrawRepeatingAppointments (MonthPtr monthP) EXTRA_SECTION_FOUR;
static void DrawRepeatingAppointments (MonthPtr monthP)
{
	//UInt16	count = 0;
	UInt16	recordNum;
	Int16 x1, x2, y;
	DateType temp;
	DateType date;
	Boolean first;
	Boolean last;
	MemHandle recordH;
	RectangleType dayR;
	ApptDBRecordType apptRec;


	// Loop through the repeating event looking for daily repeating events.
	// Repeating appointments are stored at the beginning of the database.
	recordNum = 0;
	while (true)
		{
		recordH = DmQueryNextInCategory (ApptDB, &recordNum, dmAllCategories);
		if (! recordH) break;
		
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
		
		if (apptRec.repeat && 
			 apptRec.repeat->repeatType == repeatDaily &&
			 apptRec.repeat->repeatFrequency == 1)
			{
			date.day = 1;
			date.month = monthP->month;
			date.year = monthP->year - firstYear;

			// if the first day of this daily repeating event is in this month
			// then we draw a vertical tick.  If it's not (it's in the prior
			// month) then don't.
			if (DateToInt(apptRec.when->date) >= DateToInt(date))
				first = true;
			else
				first = false;
			
			
		 	while (ApptNextRepeat (&apptRec, &date, true))
		 		{
		 		if ((date.month != monthP->month) ||
				 	 (date.year != monthP->year - firstYear))
					break;
		
				GetDayBounds (monthP, date.day, &dayR);
				y = dayR.topLeft.y + dayR.extent.y - repeatIndicatorBottomMargin;
				x1 = dayR.topLeft.x;
				x2 = dayR.topLeft.x + dayR.extent.x;
				
				last = (DateToInt (date) == DateToInt (apptRec.repeat->repeatEndDate));

				if (first)
					{
					first = false;
					WinDrawLine (x1+1, y-1, x1+1, y+1);
					x1 += 2;
					}
				
				if (last)
					{
					WinDrawLine (x2-1, y-1, x2-1, y+1);
					x2 -= 2;
					}

				WinDrawGrayLine (x1, y, x2, y);

				temp = date;
				DateAdjust (&date, 1);
				if (DateToInt (temp) == DateToInt (date)) break;
				}
			}
		MemHandleUnlock (recordH);

		// If the record has no repeating info we've reached the end of the 
		// repeating appointments.
		if (! apptRec.repeat) break;
		
		recordNum++;
		}
}


/***********************************************************************
 *
 * FUNCTION:    DrawAppointmentsInMonth
 *
 * DESCRIPTION: This routine draws appointment indicators for all the 
 *              appointment in a month.
 *
 * PARAMETERS:  monthP  - month object
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/16/96	Initial Revision
 *
 ***********************************************************************/
static void DrawAppointmentsInMonth (MonthPtr monthP) EXTRA_SECTION_FOUR;
static void DrawAppointmentsInMonth (MonthPtr monthP)
{
	if (ShowTimedAppts || ShowUntimedAppts)
		DrawTimedAppointments (monthP);

	if (ShowDailyRepeatingAppts)
		DrawRepeatingAppointments (monthP);
}


/***********************************************************************
 *
 * FUNCTION: 	 DrawMonth
 *
 * DESCRIPTION: Draw the month object.
 *
 * PARAMETERS:  monthP - pointer to month object to draw
 *					 selectDay - if true, draw "today" selected if possible
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *		04/08/96	rsf	Created by Roger Flores.
 *		08/26/99	kwk	Use DateTemplateToAscii to get the DOW first letter.
 *		10/18/99	gap	Get DOW labels from daysOfWeekInitialsStrID resource and
 *							use StartDayOfWeek to determine which letter is start day.
 *		10/31/99	jmp	Eliminate WinInvertRectangle(); draw day as selected instead.
 *							Also, always call DrawAppointmentsInMonth(), as DrawMonth()
 *							was always followed by that call -- helps consolidate various
 *							drawing problems.
 *		11/14/99	jmp	Updated the drawing such that non-selectable areas of the MonthView
 *							are in standard Form colors while selectable areas use control-
 *							style colors.  Also, added selectDay argument to allow today's
 *							date to either by drawn selected or not.
 *
 ***********************************************************************/
static void DrawMonth (MonthPtr monthP, Boolean selectDay) EXTRA_SECTION_FOUR;
static void DrawMonth (MonthPtr monthP, Boolean selectDay)
	{
	Int16				drawX, drawY;
	Int16				cellWidth, cellHeight;
	//Int16				charWidth;
	Int16				lastDay;
	//Char				dowTemplate[] = "^1s";
	Char				dayInAscii[3];
	UInt16			dow;
	UInt16			i;
	Int16				x, y;
	DateTimeType	today;
	RectangleType	r;
	UInt8 			dayOfWeek;
 	Char *			dayLabels;
	UInt16			labelLength;
	Char*				label;

	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFormFill));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
	//FntSetFont (FossilBoldFont);
	FntSetFont (boldFont);
	
	// Make sure the "unselectable" parts of the MonthView are
	// drawn in the proper colors first.
	WinEraseRectangle (&monthP->bounds, 0);
	
	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonthPlusTitle;

	// Get the resource that contains the first letter of each day.
 	dayLabels = MemHandleLock (DmGetResource (strRsc, daysOfWeekInitialsStrID));

	// Calculate length of one item in string */
	labelLength = StrLen (dayLabels) / daysInWeek;

	// Draw the days of the week labels right justified to the number columns	
	// Be sure to draw the labels with respect to current setting of StartDayOfWeek. Somes locales
	// consider Monday the first day while others use Sunday.  There is also a user preference 
	// selection in Prefs app/Formats panelthat will allow the user to change first day of week.
	drawY = 	monthP->bounds.topLeft.y + (cellHeight - FntLineHeight()) / 2;
	for (i = 0; i <= daysInWeek; i++)
		{
		dayOfWeek = (i + StartDayOfWeek) % daysInWeek;
		label = &dayLabels[labelLength * dayOfWeek];
		drawX = monthP->bounds.topLeft.x + (cellWidth * i) + ((cellWidth - FntCharsWidth (label, labelLength)) / 2);
		WinDrawChars (label, labelLength, drawX, drawY);
		}

	// Unlock the day of week label resource now that we are done with it.
	MemPtrUnlock (dayLabels);
	
	// Set the background color behind the grid in control-style colors since
	// the grid is selectable.
	WinSetBackColor(UIColorGetTableEntryIndex(UIObjectFill));
	RctCopyRectangle(&monthP->bounds, &r);
	r.topLeft.y += cellHeight;
	r.extent.y -= cellHeight;
	WinEraseRectangle (&r, 0);
	
	// Draw the grid.  Change the foreground color temporarily to get the right effect,
	// and put it back when we're done.
	WinSetForeColor(WinRGBToIndex(&colorLine));
	x = monthP->bounds.topLeft.x;
	y = monthP->bounds.topLeft.y + cellHeight;
	for (i = 0; i < daysInWeek + 1; i++)
		{
		WinDrawLine (x, y, x, y + (cellHeight * maxWeeksInMonth));
		x += cellWidth;
		}

	x = monthP->bounds.topLeft.x;
	for (i = 0; i < maxWeeksInMonth + 1; i++)
		{
		WinDrawLine (x, y, x + (cellWidth * daysInWeek) , y);
		y += cellHeight;
		}
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));

	// Draw the days of the month.
	//FntSetFont (FossilStdFont);
	FntSetFont (stdFont);
	//charWidth = FntCharWidth('0');
	
	dow = FirstDayOfMonth (monthP);
	drawX = monthP->bounds.topLeft.x + (dow * cellWidth) + textLeftMargin;
	drawY = monthP->bounds.topLeft.y + cellHeight + textTopMargin;
	lastDay = DaysInMonth(monthP->month, monthP->year);
	for (i=1; i <= lastDay; i++, dow++)
		{
		if (dow == daysInWeek)
			{
			drawX = monthP->bounds.topLeft.x + textLeftMargin;
			drawY += cellHeight;
			dow = 0;
			}
		StrIToA (dayInAscii, i);
		WinDrawChars (dayInAscii, 
						  (i < 10) ? 1 : 2, 
//						  (i < 10) ? drawX + charWidth : drawX,
						  drawX,
						  drawY);
		drawX += cellWidth;
		}

	// Display a rectangle around today's day if it's visible.
	TimSecondsToDateTime(TimGetSeconds(), &today);
	if (TodayIsVisible(monthP, today) && selectDay)
		{
		GetDaySelectionBounds(monthP, today.day, &r);
		#if WRISTPDA
		WinDrawGrayRectangleFrame( roundFrame, & r );
		#else
		MonthDrawInversionEffect (&r, 0);
		#endif
		}

	DrawAppointmentsInMonth (monthP);

	WinPopDrawState ();

	}


/***********************************************************************
 *
 * FUNCTION: 	 MapPointToItem
 *
 * DESCRIPTION: Return return the item at x, y.
 *
 * PARAMETERS:  x, y - coordinate
 *              r - the bounds of the item area (not the MTWTFSS area)
 *
 * RETURNED:	 item number (doesn't check for invalid bounds!)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	11/15/94	Initial Revision
 *
 ***********************************************************************/
static Int16 MapPointToItem (Int16 x, Int16 y, RectanglePtr r) EXTRA_SECTION_FOUR;
static Int16 MapPointToItem (Int16 x, Int16 y, RectanglePtr r)
	{
	Int16 itemNumber;

	itemNumber = daysInWeek * (((y - r->topLeft.y) /
		(r->extent.y / linesInMonthPlusTitle)) - 1);
	itemNumber += ((x - r->topLeft.x) / (r->extent.x / daysInWeek));
	return itemNumber;
	}


/***********************************************************************
 *
 * FUNCTION:    MonthViewGetMonthPtr
 *
 * DESCRIPTION: This routine returns a pointer to the month object.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:	 pointer to a MonthType structure
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/10/96	Initial Revision
 *			jmp	10/6/99	Replace FrmGetActiveForm() with FrmGetFormPtr();
 *
 ***********************************************************************/
static MonthPtr MonthViewGetMonthPtr (void) EXTRA_SECTION_FOUR;
static MonthPtr MonthViewGetMonthPtr (void)
	{
	FormPtr frm;
	
	frm = FrmGetFormPtr (MonthView);
	return (FrmGetGadgetData (frm, FrmGetObjectIndex (frm, MonthGadget)));
	}


/***********************************************************************
 *
 * FUNCTION:    MonthViewSetTitle
 *
 * DESCRIPTION: This routine set the title of the Month View
 *
 * PARAMETERS:  monthP  - pointer to month object (MonthType)
 *
 * RETURNED:	 nothing
 *
 * HISTORY:
 *		04/10/96	art	Created by Art Lamb.
 *		09/08/99	kwk	Use DateTemplateToAscii & load format from string resource.
 *		10/6/99	jmp	Replace FrmGetActiveForm() with FrmGetFormPtr();
 *
 ***********************************************************************/
static void MonthViewSetTitle (MonthPtr monthP) EXTRA_SECTION_FOUR;
static void MonthViewSetTitle (MonthPtr monthP)
	{
	Char title [longDateStrLength];
	MemHandle templateH;
	Char* templateP;
	
	templateH = DmGetResource(strRsc, MonthViewTitleStrID);
	templateP = (Char*)MemHandleLock(templateH);
	DateTemplateToAscii(templateP, monthP->month, 1, monthP->year, title, sizeof(title) - 1);
	MemHandleUnlock(templateH);
	
	FrmCopyTitle (FrmGetFormPtr (MonthView), title);
	TimeDisplayed = false;
	}

	
/***********************************************************************
 *
 * FUNCTION:    MonthViewHideTime
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
static void MonthViewHideTime (Boolean hide) EXTRA_SECTION_FOUR;
static void MonthViewHideTime (Boolean hide)
{
	MonthPtr 	monthP;
	if (TimeDisplayed)
		{
		if (hide || TimeToWait() == 0)
			{
			monthP = MonthViewGetMonthPtr ();
			MonthViewSetTitle (monthP);
			}
		}
}


/***********************************************************************
 *
 * FUNCTION:    MonthSelectDayRect
 *
 * DESCRIPTION: This routine selects or unselects day in the month
 *					 object.
 *
 * PARAMETERS:	 monthP   - pointer to control object (ControlType)
 *              r        - pointer to day's rectangle
 *					 selected - says whether to draw the rectangle selected
 *									or unselected
 *
 * RETURNED:	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			jmp	10/31/99	Initial Revision.
 *			jmp	11/06/99	Oops, was just doing a WinResetClip() instead of
 *								save/restore WinSetClip().
 *			jmp	11/14/99	Added support for drawing the current day's selection
 *								unselected to get the right selection effect on
 *								"today."
 *			jmp	11/19/99	Use WinScreenLock()/WinScreenUnlock() to eliminate all of
 *								the ugly redrawing effects.
 *       jmp   12/02/99 Fix bug #24377.  Don't call WinScreenUnlock() if WinScreenLock()
 *                      fails.
 *                          
 ***********************************************************************/
static void MonthSelectDayRect (MonthPtr monthP, RectangleType *r, Boolean selected) EXTRA_SECTION_FOUR;
static void MonthSelectDayRect (MonthPtr monthP, RectangleType *r, Boolean selected)
{
	RectangleType savedClip;
	UInt8 * lockedWinP;
	
	// Draw everything offscreen to begin with -- we have a lot to redraw, and
	// it's somewhat unsightly to see it all happen onscreen.  But still restrict
	// the clipping rectangle to get the effect we want.
	//
	lockedWinP = WinScreenLock(winLockCopy);
	WinGetClip(&savedClip);
	WinSetClip(r);
	
	// Always redraw unselected first.
	//
	DrawMonth(monthP, !selected);

	// If selection is desired, do that now.
	//
	if (selected)
	{
		RectangleType todayRect;
		DateTimeType today;

		// First, draw the entire rectangle selected.
		//
		MonthDrawInversionEffect(r, 0);
		
		// If we're drawing "today," then draw the
		// day selection itself unselected.
		//
		TimSecondsToDateTime(TimGetSeconds(), &today);
		GetDaySelectionBounds(monthP, today.day, &todayRect);
		
		#if WRISTPDA
		if ( InMonthViewHighlightSelectedDay == false )
		#endif
		if (TodayIsVisible(monthP, today) && RctPtInRectangle(r->topLeft.x, r->topLeft.y, &todayRect))
		{
			WinSetClip(&todayRect);
			DrawMonth(monthP, dontDrawDaySelected);
			}
	}

	// Now that we're done drawing, reset the clipping rectangle, and put everything we've done back
	// onscreen.
	//
	WinSetClip(&savedClip);
	if (lockedWinP)
		WinScreenUnlock();
}


/***********************************************************************
 *
 * FUNCTION:    MonthSelectDay
 *
 * DESCRIPTION: This routine selects a day in the month object.
 *
 * PARAMETERS:	 monthP  - pointer to control object (ControlType)
 *              eventP  - pointer to an EventType structure.
 *              dateP   - returned: pointer to the date selected
 *
 * RETURNED:	true if the event was handle or false if it was not.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/9/96	Initial Revision
 *			jmp	10/31/99	Eliminate WinInvertRectangle() by calling
 *								MonthSelectDayRect() instead.
 *
 ***********************************************************************/
static Boolean MonthSelectDay (MonthPtr monthP, EventType* eventP, DatePtr dateP) EXTRA_SECTION_FOUR;
static Boolean MonthSelectDay (MonthPtr monthP, EventType* eventP, DatePtr dateP)
	{
	Int16				x, y;
	Int16       	item;
	Int16       	daysInMonth;
	Int16       	firstDayInMonth;
	Int16				cellWidth, cellHeight;
	Boolean			penDown;
	Boolean			selected;
	RectangleType	r;
	
	firstDayInMonth = FirstDayOfMonth (monthP);
	daysInMonth = DaysInMonth (monthP->month, monthP->year);

	// Restrict the selection bounds.  The top should be moved down
	// to exclude selecting the day of week titles.  The bottom should
	// be moved up to the last week with a day in it.
	RctCopyRectangle (&(monthP->bounds), &r);
	r.topLeft.y += r.extent.y / linesInMonthPlusTitle;
	r.extent.y = (r.extent.y / linesInMonthPlusTitle) *
		(((firstDayInMonth + daysInMonth - 1) / 7) + 1);

	if (! RctPtInRectangle (eventP->screenX, eventP->screenY, &r))
		return false;

	// Compute the bounds of the day the pen is on.
	item = MapPointToItem (eventP->screenX, eventP->screenY, &monthP->bounds);

	cellWidth = monthP->bounds.extent.x / daysInWeek;
	cellHeight = monthP->bounds.extent.y / linesInMonthPlusTitle;
	r.topLeft.x = monthP->bounds.topLeft.x + (item % daysInWeek) * cellWidth + 1;
	r.topLeft.y = monthP->bounds.topLeft.y + ((item / daysInWeek) + 1) * cellHeight + 1;
	r.extent.x = cellWidth - 1;
	r.extent.y = cellHeight - 1;

	// Draw the selected day.
	selected	= true;
	MonthSelectDayRect (monthP, &r, selected);

	// Track the pen until it's released, highlight or unhighlight the
	// item as the pen move in and out of it.
	do 
		{
		PenGetPoint (&x, &y, &penDown);

		if (RctPtInRectangle (x, y, &r))
			{
			if (!selected)
				{
				selected = true;
				MonthSelectDayRect (monthP, &r, selected);
				}
			}

		else if (selected)
			{
			selected = false;
			MonthSelectDayRect (monthP, &r, selected);
			}

		} while (penDown);


	// Exit if the pen was released outside the item.
	if (!selected)
		return (false);

	selected = false;
	MonthSelectDayRect (monthP, &r, selected);

	dateP->month = monthP->month;
	dateP->year = monthP->year - firstYear;

	// Is the selection is outside of the month?
	if ((unsigned) (item - firstDayInMonth) >= (unsigned) daysInMonth)
		{
		// Move to the month before or after the current month.
		dateP->day = 1;
		DateAdjust (dateP, (item - firstDayInMonth));
		}
	else
		{
		dateP->day = item - firstDayInMonth + 1;
		}

	return true;
	}



/***********************************************************************
 *
 * FUNCTION:    MonthViewShowTime
 *
 * DESCRIPTION: This routine display the current time in the title of the
 *              month view.
 *
 * PARAMETERS:  frm - pointer to the day view form.
 *
 * RETURNED:    nothing
 *
 * NOTE:        The global variable TimeDisplayed is set to true by this
 *              routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/15/96	Initial Revision
 *			jmp	10/6/99	Replace FrmGetActiveForm() with FrmGetFormPtr();
 *
 ***********************************************************************/
static void MonthViewShowTime (void) EXTRA_SECTION_FOUR;
static void MonthViewShowTime (void)
{
	Char				title[timeStringLength];
	DateTimeType 	dateTime;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii (dateTime.hour, dateTime.minute, TimeFormat, title);
	FrmCopyTitle (FrmGetFormPtr (MonthView), title);
	
	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks () + timeDisplayTicks;
}


/***********************************************************************
 *
 * FUNCTION:		MonthViewGotoDate
 *
 * DESCRIPTION:	This routine displays the date picker so that the 
 *						user can select a month to navigate to.  If the date
 *						picker is confirmed, the month selected month is 
 *                confirmed
 *
 *						This routine is called when a "go to" button is pressed.
 *              
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/20/96	Initial Revision
 *			jmp	10/31/99 Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *								into DrawMonth() itself.
 *
 ***********************************************************************/
static void MonthViewGotoDate (void) EXTRA_SECTION_FOUR;
static void MonthViewGotoDate (void)
	{
	Char *		title;
	MemHandle	titleH;
	MonthPtr 	monthP;
	Int16 		day;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	monthP = MonthViewGetMonthPtr ();

	// Display the date picker.
	day = 1;
	if (SelectDay (selectDayByMonth, &monthP->month, &day, &monthP->year, title))
		{
		MonthViewSetTitle (monthP);
		DrawMonth (monthP, drawDaySelected);
		#if WRISTPDA
		SelectedDay = day;
		MonthViewHighlightSelectedDay( true );
		#endif
		}

	MemHandleUnlock (titleH);
	}



/***********************************************************************
 *
 * FUNCTION:    MonthViewScroll
 *
 * DESCRIPTION: This routine scrolls the month view foreward or backwards
 *              by a month.
 *
 * PARAMETERS:  monthP  - pointer to month object (MonthType)
 *              direction - winUp (backwards) or winDown (foreward)
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			jmp	10/31/99 Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *								into DrawMonth() itself.
 *
 ***********************************************************************/
static void MonthViewScroll (MonthPtr monthP, WinDirectionType direction) EXTRA_SECTION_FOUR;
static void MonthViewScroll (MonthPtr monthP, WinDirectionType direction)
{

	#if WRISTPDA
	
	// Move backward one month.
	if (direction == winUp)
		{
		if (monthP->month > january) {
			monthP->month--;
			SelectedDay = DaysInMonth( monthP->month, monthP->year ); // Last day of month
		} else if (monthP->year > firstYear)
			{
			monthP->month = december;
			monthP->year--;
			SelectedDay = DaysInMonth( monthP->month, monthP->year ); // Last day of month
			}
		else
			return;
		}
		
	// Move foreward one month.
	else
		{
		if (monthP->month < december) {
			monthP->month++;
			SelectedDay = 1;
		} else if (monthP->year < lastYear)
			{
			monthP->month = january;
			monthP->year++;
			SelectedDay = 1;
			}
		else
			return;
		}
	
	#else
	
	// Move backward one month.
	if (direction == winUp)
		{
		if (monthP->month > january)
			monthP->month--;
		else if (monthP->year > firstYear)
			{
			monthP->month = december;
			monthP->year--;
			}
		else
			return;
		}
		
	// Move foreward one month.
	else
		{
		if (monthP->month < december)
			monthP->month++;
		else if (monthP->year < lastYear)
			{
			monthP->month = january;
			monthP->year++;
			}
		else
			return;
		}

	#endif

	MonthViewSetTitle (monthP);
	DrawMonth (monthP, drawDaySelected);
	#if WRISTPDA
	MonthViewHighlightSelectedDay( true );
	#endif
	
}


/***********************************************************************
 *
 * FUNCTION:    MonthViewDoCommand
 *
 * DESCRIPTION: This routine preforms the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    true if 
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/96	Initial Revision
 *			jmp	10/31/99 Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *								into DrawMonth() itself.
 *
 ***********************************************************************/
static Boolean MonthViewDoCommand (UInt16 command) EXTRA_SECTION_FOUR;
static Boolean MonthViewDoCommand (UInt16 command)
{
	MonthPtr monthP = MonthViewGetMonthPtr();
	
	// Preferences
	if (command == MonthPreferencesCmd)
		FrmPopupForm (PreferencesDialog);

	// Display Options
	else if (command == MonthDisplayOptionsCmd)
		FrmPopupForm (DisplayDialog);

	// About
	else if (command == MonthSecurityCmd)
		{
		DoSecurity();
		DrawMonth (monthP, drawDaySelected);
		#if WRISTPDA
		MonthViewHighlightSelectedDay( true );
		#endif
		}
		
	// About
	else if (command == MonthGetInfoCmd)
		AbtShowAbout (sysFileCDatebook);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    MonthViewInit
 *
 * DESCRIPTION: This routine initialize the month view.
 *
 * PARAMETERS:  frm - pointer of the Month View form
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *
 ***********************************************************************/
static void MonthViewInit (FormPtr frm) EXTRA_SECTION_FOUR;
static void MonthViewInit (FormPtr frm)
{
	UInt16 objIndex;
	Int16 width;
	MonthPtr monthP;

	// Create a month object, initialize it, and store a pointer to it in the 
	// month gadget of the Month View.
	monthP = MemPtrNew (sizeof (MonthType));
	if (monthP)
		{
		objIndex = FrmGetObjectIndex (frm, MonthGadget);

		FrmGetObjectBounds (frm, objIndex, &monthP->bounds);
		width = monthP->bounds.extent.x;
		monthP->bounds.extent.x = (width / daysInWeek * daysInWeek) + 1;
		monthP->bounds.topLeft.x = (width - monthP->bounds.extent.x) / 2;

		monthP->month = Date.month;
		monthP->year = Date.year + firstYear;

		FrmSetGadgetData (frm, objIndex, monthP);

		MonthViewSetTitle (monthP);

		// Highlight the Month View push button.
		FrmSetControlGroupSelection (frm, DayViewGroup, MonthMonthViewButton);
		}

	TimeDisplayed = false;
}


#if WRISTPDA

/***********************************************************************
 *
 * FUNCTION:    MonthViewHighlightSelectedDay
 *
 * DESCRIPTION: This routine highlights or unhighlights the selected day.
 *
 * PARAMETERS:  Highlight: true to highlight, false to unhighlight
 *
 * RETURNED:    Nothing
 *
 * HISTORY:
 *
 *		02/11/03	dmc		Initial version.
 *
 ***********************************************************************/
static void MonthViewHighlightSelectedDay( Boolean Highlight )
{
	MonthPtr      monthP;
	RectangleType r;
	InMonthViewHighlightSelectedDay = true;
	monthP = MonthViewGetMonthPtr();
	// Highlight or unhighlight the selected day.
	GetDaySelectionBounds( monthP, SelectedDay, & r );
	MonthSelectDayRect( monthP, & r, Highlight );
//	MonthDrawInversionEffect (&r, 0);
	InMonthViewHighlightSelectedDay = false;
}


/***********************************************************************
 *
 * FUNCTION:    MonthViewMoveSelection
 *
 * DESCRIPTION: This routine moves the Month View day selection
 *              highlight up or down.
 *
 * PARAMETERS:  Direction: -1 => Move up
 *                          0 => Don't move, highlight current selection
 *                         +1 => Move down
 *
 * RETURNED:    Nothing
 *
 * HISTORY:
 *
 *		02/11/03	dmc		Initial version.
 *
 ***********************************************************************/
static void MonthViewMoveSelection ( Int16 Direction )
{
	Int16         LastDayOfMonth;
	MonthPtr      monthP;
	
	monthP = MonthViewGetMonthPtr();

	// Unhighlight the current selected day.
	MonthViewHighlightSelectedDay( false );

	LastDayOfMonth = DaysInMonth( monthP->month, monthP->year );

	if ( Direction != 0 ) {
		if ( Direction == +1 ) {
			// Move the selection highlight down one record, scroll if necessary.
			if ( SelectedDay < LastDayOfMonth ) {
				SelectedDay++;
			} else {
				// Scroll the month.
				MonthViewScroll( monthP, winDown );
				SelectedDay = 1;
			}
		}
	 	else if ( Direction == -1 ) {
			// Move the selection highlight up one record, scroll if necessary.
			if ( SelectedDay > 1 ) {
				SelectedDay--;
			} else {
				// Scroll the month.
				MonthViewScroll( monthP, winUp );
				monthP = MonthViewGetMonthPtr();
				LastDayOfMonth = DaysInMonth( monthP->month, monthP->year );
				SelectedDay = LastDayOfMonth;
			}
		}
	}
	// Highlight the new selected day.
	MonthViewHighlightSelectedDay( true );
}

#endif


/***********************************************************************
 *
 * FUNCTION:    MonthViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the Month View
 *              of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *			rbb	6/2/99	Added button for Agenda view
 *			jmp	10/6/99	Fix bug #22497:  Perform a FrmDrawForm() before
 *								updating the rest of the form on frmUpdateEvents;
 *								also, replace FrmGetActiveForm() with FrmGetFormPtr()
 *								as that is more correct in these modern times of
 *								generating update events when bits fail to be allocated.
 *			jmp	10/31/99 Move WinEraseRectangle() & DrawAppointmentsInMonth()
 *								into DrawMonth() itself.
 *
 ***********************************************************************/

Boolean MonthViewHandleEvent (EventType* event)
{
	UInt16 daysInMonth;
	FormPtr frm;
	DateType date;
	Boolean handled = false;
	MonthPtr monthP;

	#if WRISTPDA

	EventType newEvent;

	extern UInt32  DebounceEnterKeyEndTime;

	extern UInt16  NewSndSysAmp, OldSndSysAmp;
	extern UInt16  NewSndDefAmp, OldSndDefAmp;

	extern UInt16  NewKeyInitDelay, OldKeyInitDelay;
	extern UInt16  NewKeyPeriod, OldKeyPeriod;
	extern UInt16  NewKeyDoubleTapDelay,  OldKeyDoubleTapDelay;
	extern Boolean NewKeyQueueAhead, OldKeyQueueAhead;

	#endif


	if (event->eType == penDownEvent)
		{
		monthP = MonthViewGetMonthPtr ();
		if (MonthSelectDay (monthP, event, &date))
			{
			Date = date;
			FrmGotoForm (DayView);
			handled = true;
			}
		}
	
	else if (event->eType == keyDownEvent)
		{

		#if WRISTPDA
			frm = FrmGetFormPtr (MonthView);
			// On other than Enter key exit the debounce state.
			if ( event->data.keyDown.chr != vchrThumbWheelPush ) {
				if ( DebounceEnterKeyEndTime > 0 ) {
					DebounceEnterKeyEndTime = 0;
				}
			}
			MemSet( & newEvent, sizeof( EventType ), 0 );
			if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
				UInt32 Duration, TicksPerSecond = SysTicksPerSecond();
				// If we need to debounce this Enter key event then just return.
				if ( DebounceEnterKeyEndTime > 0 ) {
					if ( TimGetTicks() < DebounceEnterKeyEndTime )
						return true;
				}
				DebounceEnterKeyEndTime = 0;
				// Determine how long the Enter key is depressed.
				Duration = CheckKeyDuration( keyBitEnter, TicksPerSecond );
				// We take different actions based on how long the Enter key was depressed.
				if ( Duration < SysTicksPerSecond() ) {
					// < 1 sec: Translate the Enter key to a MonthAgendaViewButton event.
					newEvent.eType = ctlSelectEvent;
					newEvent.data.ctlSelect.controlID = MonthAgendaViewButton;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					// Indicate we don't need to debounce the Enter key.
					DebounceEnterKeyEndTime = 0;
					// Queue the event.
					EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
					return true;
				} else {
					// > 1 sec: Go to Day View for the selected day.
					newEvent.eType = ctlSelectEvent;
					newEvent.data.ctlSelect.controlID = MonthGoToSelectedDay;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					// Indicate that we need to debounce (i.e. ignore) Enter key
					// event(s) that occur too quickly after this one.
					DebounceEnterKeyEndTime = TimGetTicks() + TicksPerSecond / 2;
					// Turn off key repeat etc.
					NewKeyInitDelay = NewKeyPeriod = NewKeyDoubleTapDelay = 0xFFFF;
					NewKeyQueueAhead = false;
					// Beep to provide user feedback.
					SndPlaySystemSound( sndConfirmation );
					// We must wait a while before setting volumes to zero, or no sound...
					SysTaskDelay( TicksPerSecond / 10 );
					// Queue appropriate event based on which list has focus.
					EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
					return true;
				}
			} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
				// Translate the Back key to an open launcher event.
				newEvent.eType = keyDownEvent;
				newEvent.data.keyDown.chr = launchChr;
				newEvent.data.keyDown.modifiers = commandKeyMask;
				EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
				// Move the selection highlight down one day, scroll if necessary.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = MonthMoveSelectionDown;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
				// Move the selection highlight up one day, scroll if necessary.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = MonthMoveSelectionUp;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000005, true );
				return true;
			}
		#endif

		MonthViewHideTime (true);

		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
			DateSecondsToDate (TimGetSeconds (), &date);
			Date = date;
			if (EventInCurrentView ||
				(event->data.keyDown.modifiers & poweredOnKeyMask))
				FrmGotoForm (DayView);
			else
				FrmGotoForm (AgendaView);				
			handled = true;
			EventInCurrentView = true;
			}
		else if (EvtKeydownIsVirtual(event))
			{
			if (event->data.keyDown.chr == vchrPageUp)
				{
				monthP = MonthViewGetMonthPtr ();
				MonthViewScroll (monthP, winUp);
				handled = true;
				}	
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				monthP = MonthViewGetMonthPtr ();
				MonthViewScroll (monthP, winDown);
				handled = true;
				}
			
			EventInCurrentView = true;
			}
		}

	
	else if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case MonthDayViewButton:
			case MonthWeekViewButton:
			case MonthAgendaViewButton:
				monthP = MonthViewGetMonthPtr ();
				Date.year = monthP->year - firstYear;
				Date.month = monthP->month;
				daysInMonth = DaysInMonth (Date.month, Date.year + firstYear);
				Date.day = min (Date.day, daysInMonth);
				break;
			#if WRISTPDA
			case MonthMoveSelectionDown:
				// Move the selection highlight down one day, scroll if necessary.
				MonthViewMoveSelection( +1 );
				return true;
			case MonthMoveSelectionUp:
				// Move the selection highlight up one day, scroll if necessary.
				MonthViewMoveSelection( -1 );
				return true;
			case MonthGoToSelectedDay:
				// Go to Day View for the selected day.
				monthP = MonthViewGetMonthPtr();
				MemSet( & Date, sizeof( Date ), 0 );
				Date.day   = SelectedDay;
				Date.month = monthP->month;
				Date.year  = monthP->year - firstYear;
				FrmGotoForm( DayView );
				return true;
			#endif

			}
		switch (event->data.ctlSelect.controlID)
			{
			case MonthGoToButton:
				MonthViewGotoDate ();
				handled = true;
				break;

			case MonthDayViewButton:
				FrmGotoForm (DayView);
				handled = true;
				break;

			case MonthWeekViewButton:
				FrmGotoForm (WeekView);
				handled = true;
				break;

			case MonthAgendaViewButton:
				FrmGotoForm (AgendaView);
				handled = true;
				break;
			}
		}


	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case MonthPrevButton:
				monthP = MonthViewGetMonthPtr ();
				MonthViewScroll (monthP, winUp);
				break;
			
			case MonthNextButton:
				monthP = MonthViewGetMonthPtr ();
				MonthViewScroll (monthP, winDown);
				break;
			}
		}


	else if (event->eType == menuEvent)
		{
		handled = MonthViewDoCommand (event->data.menu.itemID);
		}
		

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetFormPtr (MonthView);
		MonthViewInit (frm);
		FrmDrawForm (frm);

		#if WRISTPDA
		{
			// Init the selected day.
			DateTimeType DateTime;
			TimSecondsToDateTime( TimGetSeconds(), & DateTime );
			SelectedDay = Date.day;
		}
		#endif

		monthP = MonthViewGetMonthPtr ();
		DrawMonth (monthP, drawDaySelected);
		#if WRISTPDA
		// Highlight the selected day.
		MonthViewHighlightSelectedDay( true );
		#endif
		EventInCurrentView = false;
		handled = true;
		}


	else if (event->eType == frmUpdateEvent)
		{
		frm = FrmGetFormPtr (MonthView);
		FrmDrawForm (frm);
		
		monthP = MonthViewGetMonthPtr ();
		DrawMonth (monthP, drawDaySelected);
		#if WRISTPDA
		MonthViewHighlightSelectedDay( true );
		#endif
		handled = true;		
		}

	else if (event->eType == frmCloseEvent)
		{
		monthP = MonthViewGetMonthPtr ();
		if (monthP)
			MemPtrFree (monthP);
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
				MonthViewHideTime (true);
			else
				MonthViewShowTime ();
			}
		}


	else if (event->eType == nilEvent)
		{
		MonthViewHideTime (false);
		}

	return (handled);
}
