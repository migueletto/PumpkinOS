/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateWeek.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This is the Datebook application's week view module.
 *
 * History:
 *		June 12, 1995	Created by Art Lamb
 *
 *****************************************************************************/

#include <PalmOS.h>

#include <PalmUtils.h>

#if WRISTPDA
#include <StringMgr.h>
#endif

#include "sections.h"
#include "Datebook.h"

//DOLATER - peter: remove this once a choice is made.
#define UNMASK_PRIVATE_RECORDS_IN_WEEK_VIEW		1

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

extern privateRecordViewEnum	CurrentRecordVisualStatus;	// applies to current record
extern privateRecordViewEnum	PrivateRecordVisualStatus;	// applies to all other records

//	WeekView globals:
static MemHandle				WeekObjectH;				//	The currently active
																	//	week

static CustomPatternType	regAppt = {0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55};
										//	50% gray pattern
static CustomPatternType	overLapAppt = {0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11};
										//	Diagonal stripes
static FontID					OrigFont;


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/


//	WeekView/Day object constants
#define weekViewDayHMargin					4		//	This is the horizontal space
 															//	between the edge of the day's
 															//	bounds and the visible event
 															//	bar.

#define weekViewTimeDisplayWidth			25		//	Width of column of times
#define timeHMargin							2		//	Horiz margin between times
															//	& first day bar.
#define weekHMargin							2		//	Horiz margin between days
															//	and right edge of screen.
#if WRISTPDA
#define weekViewDayDisplayHeight			14 		//	Height of day labels at top
													//	of view.
#define weekViewDateDisplayHeight			15 		//	Height of date labels under
													//	day labels at top of view.
#else
#define weekViewDayDisplayHeight			12		//	Height of day labels at top
															//	of view.
#define weekViewDateDisplayHeight		13		//	Height of date labels under
															//	day labels at top of view.
#endif
#define weekViewEventFrameWidth			3		//	Width of frame around

#define weekViewScrollInterval			4		//	Scroll interval, in hours.
															//	Must be >= 1.
#define weekViewNumDispHours				11		//	Number of hours that we display
															//	on one screenful of data.
#define weekViewHourDisplayInterval		2		//	Used for displaying the time
															//	bar.
															
#define dataMarkHMargin						4		//	Horizontal margins around
															//	data indicators.
#define dataMarkVMargin						2
#define outOfRangeDay						0		//	illegal date.
 
#define weekDescMarginX						3		// Margin in popup desc
#define weekDescMarginY						2		// Margin in popup desc

#define moveThreshold						8		// Number of pixels to drag an 
															// event before it starts moving

#define weekDescDisplayTicks				200	// number of tick to display the
															// description popup 
#define weekDescUnmaskedDisplayTicks	500	// number of tick to display the
															// description popup for unmasked
															// events
/***********************************************************************
 *
 *	Internal Structutes
 *
 ***********************************************************************/

//	These structures are used by the WeekView of the DateBook
typedef struct
	{
	RectangleType			bounds;
	DateType					date;					//	If the 'day' field is set to
														//	'outOfRangeDay', this day is
														//	outside the allowable system range
														//	and should not be drawn.
	UInt8						dayOfWeek;
	UInt8						reserved;
	MemHandle				apptsH;				//	Use ApptInfoPtr to examine
	UInt16					numAppts;
	UInt16					firstEventHour;
	UInt16					lastEventHour;		//	The limits of the events which
														//	exist on this day.  Meaningless
														//	if there are no appts.
	} DayType;
	
typedef struct
	{
	RectangleType			bounds;
	UInt16					startHour;			//	first currently displayed hour
	UInt16					endHour;				//	last currently displayed hour
	DayType					days[daysInWeek];
	DateType					startDate;			//	Start date of week
	UInt16					firstEventHour;	//	Hour of earliest event
														//	during the current week.
	UInt16					lastEventHour;		//	Hour of latest event
														//	during the current week.
	UInt16					hourHeight;			//	Height, in pixels, of one
														//	hour of time in a day bar.
	Int32						descTick;			// tick count when the popup 
														// description was displayed.
	WinHandle				behindDesc;			// bits behind popup description.
	WinHandle				behindHighlight;	// bits behind selection highlighting

	} WeekType;
	
typedef WeekType * WeekPtr;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void WeekViewDrawTimes (Int16 startYPos, UInt16 startHour, UInt16 endHour,
										 UInt16 hourHeight) EXTRA_SECTION_THREE;
static void WeekViewFree (void) EXTRA_SECTION_THREE;
static void WeekViewInit (FormPtr frm) EXTRA_SECTION_THREE;
static void WeekViewInitDays (FormPtr frm) EXTRA_SECTION_THREE;
static void WeekViewDrawDays (void) EXTRA_SECTION_THREE;
static void WeekViewDraw (FormPtr frm) EXTRA_SECTION_THREE;
static void WeekEraseDescription (Boolean eraseIt) EXTRA_SECTION_THREE;
static void WeekViewUpdateDisplay (UInt16 updateCode) EXTRA_SECTION_THREE;
 
// DOLATER ??? - move to time manager.
/***********************************************************************
 *
 * FUNCTION:    TimeDifference
 *
 * DESCRIPTION: Subtract pTime2 from pTime1 and place the result in
 *					 pTimeDiff.
 *
 *					 Note that TimeType is now unsigned so this doesn't
 *					 work for negative times (which no longer exist!).
 *
 * PARAMETERS:  pTime1   - pointer to HMSTime
 *              pTime2   - pointer to HMSTime
 *              pTimeDiff- pointer to HMSTime
 *
 * RETURNED:	 pTimeDiff is set to pTime1 - pTime2.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	12/2/94	Initial Revision
 *
 ***********************************************************************/

static void TimeDifference (TimePtr pTime1, TimePtr pTime2, TimePtr pTimeDiff) EXTRA_SECTION_THREE;
static void TimeDifference (TimePtr pTime1, TimePtr pTime2, TimePtr pTimeDiff)
	{
	pTimeDiff->hours = pTime1->hours - pTime2->hours;

	if (pTime1->minutes < pTime2->minutes)
		{
		pTimeDiff->minutes = pTime1->minutes + hoursInMinutes - pTime2->minutes;
		pTimeDiff->hours--;
		}
	else
		pTimeDiff->minutes = pTime1->minutes - pTime2->minutes;
	
	}

/***********************************************************************
 *
 * FUNCTION:    FirstDayOfYear
 *
 * DESCRIPTION: Return the number of day from 1/1/1904 of the first day 
 *              of the year passed.
 *
 *					 The first day of the year is always a Monday.  The rule 
 *					 for determining the first day of the year is:
 *				
 *					 New Years Day	First Day of the Year
 *					 ------------	---------------------
 *					 Monday			Monday Jan 1
 *					 Tuesday			Monday Dec 31
 *					 Wednesday		Monday Dec 30
 *					 Thursday		Monday Dec 29
 *					 Friday			Monday Jan 4 
 *					 Saturday		Monday Jan 3 
 *					 Sunday			Monday Jan 2
 *
 * PARAMETERS:	 year  - year (1904-2031)
 *
 * RETURNED:	 number of days since 1/1/1904
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/4/96	Initial Revision
 *
 ***********************************************************************/
static UInt32 FirstDayOfYear (UInt16 year) EXTRA_SECTION_THREE;
static UInt32 FirstDayOfYear (UInt16 year)
{
	UInt32 days;
	UInt16 dayOfWeek;
	DateType date;
	
	// Get days to January 1st of the year passed.
	date.day = 1;
	date.month = 1;
	date.year = year - firstYear;
	days = DateToDays (date);
	
	dayOfWeek = DayOfWeek (1, 1, year);

	// Move to monday.
	days++;
	days -= dayOfWeek;

	
	if (dayOfWeek >= friday)
		days += daysInWeek;
		
	return (days);
}


/***********************************************************************
 *
 * FUNCTION:    GetWeekNumber
 *
 * DESCRIPTION: Calculate the week number of the specified date.
 *
 * PARAMETERS:	 month - month (1-12)
 *              day   - day (1-31)
 *              year  - year (1904-2031)
 *
 * RETURNED:	 week number (1-53)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/4/96	Initial Revision
 *
 ***********************************************************************/
static UInt16 GetWeekNumber (UInt16 month, UInt16 day, UInt16 year) EXTRA_SECTION_THREE;
static UInt16 GetWeekNumber (UInt16 month, UInt16 day, UInt16 year)
	{
	UInt16 dow;
	UInt16 week;
	UInt32 days;
	UInt32 firstOfYear;
	UInt32 firstOfNextYear;
	DateType date;

	// Calculate the julian date of Monday in the same week as the 
	// specified date.
	date.day = day;
	date.month = month;
	date.year = year - firstYear;
	days = DateToDays (date);

	// Adjust the day of the week by the preference setting for the first day
	// of the week.
	dow = (DayOfWeek (month, day, year) - StartDayOfWeek + daysInWeek) 
				% daysInWeek;

	if (monday < StartDayOfWeek)
		days -= (Int16)dow - (monday + daysInWeek - StartDayOfWeek);
	else		
		days -= (Int16)dow - (monday - (Int16)StartDayOfWeek);


	firstOfYear = FirstDayOfYear (year);
	
	if (days < firstOfYear)
		{
		// The date passed is in a week that is part of the prior 
		//	year, so get the start of the prior year.
		if (year > firstYear)
			firstOfYear = FirstDayOfYear (--year);
		}
	else
		{
		// Make sure the date passed is not in a week that in part
		// of next year.
		if (year < lastYear)
			{  
			firstOfNextYear = FirstDayOfYear (year + 1);
			if (days == firstOfNextYear)
				firstOfYear = firstOfNextYear;
			}
		}
	
	week = ((Int16)(days - firstOfYear)) / daysInWeek + 1;		// one base
	
	return (week);
	}
	

/***********************************************************************
 *
 * FUNCTION:    WeekViewNewAppointment
 *
 * DESCRIPTION:	This routine create a new apointment.  It's called
 *                when the user selects an empty time slot.
 *
 * PARAMETERS:		startTime - start time of the new appointment
 *                endTime   - end time of the new appointment
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/25/95	Initial Revision
 *       ryw   2/18/00  Added cast to satisfy const cstring checking, should be safe
 *
 ***********************************************************************/
static void WeekViewNewAppointment (TimePtr startTime, TimePtr endTime)
{
	Err error;
	UInt16 recordNum;
	ApptDBRecordType newAppt;
	ApptDateTimeType when;
	
	// Create a new appointment on the current day.
	MemSet (&newAppt, sizeof (newAppt), 0);

	when.startTime = *startTime;
	when.endTime = *endTime;
	if (TimeToInt(when.endTime) > apptMaxEndTime) {
		//TimeToInt (when.endTime) = apptMaxEndTime;
		when.endTime.hours = 0xff;
		when.endTime.minutes = 0xff;
	}
	when.date = Date;
	newAppt.when = &when;
	newAppt.description = (char *)""; 
	if (AlarmPreset.advance != apptNoAlarm)
		newAppt.alarm = &AlarmPreset;		

	error = ApptNewRecord (ApptDB, &newAppt, &recordNum);
	
	if (! error)
		{
		CurrentRecord = recordNum;
		ItemSelected = true;
		}

	// Display an alert that indicates that the new record could 
	// not be created.
	if (error)
		{
		// DOLATER - Add alert here
		}
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewUpdateScrollers
 *
 * DESCRIPTION:	This routine draws or erases the Week View scroll arrow
 *						buttons depending on whether there is more data to be
 *						displayed in either direction.
 *
 * PARAMETERS:		frm - pointer to the Week View form.
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	7/31/95	Initial Revision
 *			kcr	9/20/95	convert to use FrmUpdateScrollers
 *
 ***********************************************************************/
static void WeekViewUpdateScrollers (FormPtr frm)
	{
	WeekType		*week;
	UInt16 upIndex;
	UInt16 downIndex;
	Boolean scrollableUp;
	Boolean scrollableDown;
	
	week = MemHandleLock (WeekObjectH);

	scrollableUp = (week->startHour > week->firstEventHour);
	scrollableDown = (week->endHour < week->lastEventHour);
	upIndex = FrmGetObjectIndex (frm, WeekUpButton);
	downIndex = FrmGetObjectIndex (frm,WeekDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);
		
	MemPtrUnlock(week);
	}	//	end of WeekViewUpdateScrollers
	
/***********************************************************************
 *
 * FUNCTION:    DrawTopDataMark
 *
 * DESCRIPTION:	This routine draws a little mark above a given day's
 *						event bar.
 *
 * PARAMETERS:		r - the bounds of the day
 *               scrool - true of the view can be scrolled winUp
 *               noTime - true of the day has untimed events.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	4/19/95	initial version
 *
 ***********************************************************************/
static void DrawTopDataMark (RectanglePtr r, Boolean scroll, Boolean untimed)
	{
	Int16 x1, x2, y;
	
	x1 = r->topLeft.x + dataMarkHMargin;
	x2 = r->topLeft.x + r->extent.x - dataMarkHMargin;
	y = r->topLeft.y + weekViewDayDisplayHeight +
		weekViewDateDisplayHeight - dataMarkVMargin;

	if (scroll)
		{
		// Draw the scroll indicators.
		WinDrawLine (x1, y, x2, y);
		if (untimed)
			{
			// Draw the no-time indicators.
			WinDrawLine (x1, y-1, x1+1, y-1);
			}
		else
			{
			// Erase the no-time indicators.
			WinEraseLine (x1, y-1, x1+1, y-1);
			}
		}
	
	else
		{
		// Draw the no-time indicators and erase the scroll indicators.
		if (untimed)
			{
			WinDrawLine (x1, y-1, x1+1, y-1);
			WinDrawLine (x1, y, x1+1, y);
			WinEraseLine (x1+2, y, x2, y);
			}

		// Erase both the scroll and no-time indicators.
		else
			{
			WinEraseLine (x1, y-1, x1+1, y-1);
			WinEraseLine (x1, y, x2, y);
			}
		}
	}	//	end of DrawTopDataMark


#if 0
/***********************************************************************
 *
 * FUNCTION:    DrawTopDataMark
 *
 * DESCRIPTION:	This routine draws a little mark above a given day's
 *						event bar.
 *
 * PARAMETERS:		RectanglePtr				r - the bounds of the day
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	10/3/95	initial version
 *
 ***********************************************************************/
static void DrawTopDataMark (RectanglePtr r)
	{
	WinDrawLine (r->topLeft.x + dataMarkHMargin,
					 r->topLeft.y +
					 	weekViewDayDisplayHeight +
					 	weekViewDateDisplayHeight - dataMarkVMargin,
					 r->topLeft.x + r->extent.x - dataMarkHMargin,
					 r->topLeft.y + 
					 	weekViewDayDisplayHeight +
					 	weekViewDateDisplayHeight - dataMarkVMargin);
	}	//	end of DrawTopDataMark


/***********************************************************************
 *
 * FUNCTION:    EraseTopDataMark
 *
 * DESCRIPTION:	This routine erases a little mark above a given day's
 *						event bar.
 *
 * PARAMETERS:		RectanglePtr				r - the bounds of the day
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	10/3/95	initial version
 *
 ***********************************************************************/
static void EraseTopDataMark (RectanglePtr r)
	{
	WinEraseLine (r->topLeft.x + dataMarkHMargin,
					 r->topLeft.y +
					 	weekViewDayDisplayHeight +
					 	weekViewDateDisplayHeight - dataMarkVMargin,
					 r->topLeft.x + r->extent.x - dataMarkHMargin,
					 r->topLeft.y + 
					 	weekViewDayDisplayHeight +
					 	weekViewDateDisplayHeight - dataMarkVMargin);
	}	//	end of EraseTopDataMark
#endif

/***********************************************************************
 *
 * FUNCTION:    DrawBottomDataMark
 *
 * DESCRIPTION:	This routine draws a little mark below a given day's
 *						event bar.
 *
 * PARAMETERS:		RectanglePtr				r - the bounds of the day
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	10/3/95	initial version
 *
 ***********************************************************************/
static void DrawBottomDataMark (RectanglePtr r)
	{
	WinDrawLine (r->topLeft.x + dataMarkHMargin,
					 r->topLeft.y + r->extent.y + dataMarkVMargin,
					 r->topLeft.x + r->extent.x - dataMarkHMargin,
					 r->topLeft.y + r->extent.y + dataMarkVMargin);
	}	//	end of DrawBottomDataMark


/***********************************************************************
 *
 * FUNCTION:    EraseBottomDataMark
 *
 * DESCRIPTION:	This routine erases a little mark below a given day's
 *						event bar.
 *
 * PARAMETERS:		RectanglePtr				r - the bounds of the day
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	10/3/95	initial version
 *
 ***********************************************************************/
static void EraseBottomDataMark (RectanglePtr r)
	{
	WinEraseLine (r->topLeft.x + dataMarkHMargin,
					 r->topLeft.y + r->extent.y + dataMarkVMargin,
					 r->topLeft.x + r->extent.x - dataMarkHMargin,
					 r->topLeft.y + r->extent.y + dataMarkVMargin);
	}	//	end of EraseBottomDataMark


/***********************************************************************
 *
 * FUNCTION:    WeekViewDrawDataMarks
 *
 * DESCRIPTION:	This routine draws little marks above and below each
 *						day's event bar to show that there are more events
 *						in either direction for a given day.
 *
 *						This routine must be broken out separately from the
 *						standard draw routine, because it must function even
 *						when a clip region is set for drawing while scrolling.
 *
 * PARAMETERS:		nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	10/3/95	initial version
 *
 ***********************************************************************/
static void WeekViewDrawDataMarks (void)
	{
	int					i;
	Boolean				untimed;
	WeekType				*week;
 	ApptInfoPtr			appts;

	week = MemHandleLock (WeekObjectH);
	
	for (i = 0; i < daysInWeek; i++)
		{
		// Check if the are any untimed apppoints.  They'll be first in list
		// of appointment if they exist.
		untimed = false;
		if (week->days[i].numAppts)
			{
			appts = MemHandleLock (week->days[i].apptsH);
			untimed = (TimeToInt (appts[0].startTime) == apptNoTime);
	 		MemPtrUnlock(appts);
			}

		DrawTopDataMark (&(week->days[i].bounds),
							  week->days[i].firstEventHour < week->startHour,
							  untimed);
		
//		if (week->days[i].firstEventHour < week->startHour)
//			DrawTopDataMark (&(week->days[i].bounds));
//		else
//			EraseTopDataMark (&(week->days[i].bounds));

		if (week->days[i].lastEventHour > week->endHour)
			DrawBottomDataMark (&(week->days[i].bounds));
		else
			EraseBottomDataMark (&(week->days[i].bounds));
		}

	MemPtrUnlock(week);
	}	//	end of WeekViewDrawDataMarks


/***********************************************************************
 *
 * FUNCTION:    WeekViewDrawDescription
 *
 * DESCRIPTION:	This routine
 *
 * PARAMETERS:		
 *
 * RETURNED:    	handle of a window that contains the bits obscured 
 *                by the description.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	3/28/96	Initial Revision
 *			kwk	07/07/99	Use WinDrawTruncChars.
 *			gap	10/25/00	Fix redraw problem in 24hour time.
 *
 ***********************************************************************/
#define descWidthMax	20
static WinHandle WeekViewDrawDescription (TimePtr startTime, TimePtr endTime,
	DatePtr date, Char * desc, Boolean saveBits)
{
	UInt16 error;
	UInt16 len;
	UInt16 charsToDraw;
	UInt16 charsToErase;
	Char str [descWidthMax];
	#if WRISTPDA
	Char dateStr [descWidthMax];
	#endif
	Int16 maxDescWidth;
	Char * ptr;
	Int16 x, y;
	FontID curFont;
	WinHandle winH = 0;
	RectangleType r;	
	RectangleType eraseR;	
	
	//curFont = FntSetFont (FossilStdFont);
	curFont = FntSetFont (stdFont);
	
	// Compute the bounds of the drawing region for the description.
	WinGetWindowExtent (&r.extent.x, &r.extent.y);
	r.topLeft.x = 0;
	r.topLeft.y = 0;
	r.extent.y = (FntLineHeight () * 2) + (weekDescMarginY * 3) + 3;
	
	// Save the region that we're going to draw over.
	if (saveBits)
		{
		AttnIndicatorEnable(false);		// Get the indicator off before we save the bits
		winH = WinSaveBits (&r, &error);
		if (! winH) return (0);
		
		// Erase and frame the drawing region with a 3D frame.
		WinEraseRectangle (&r, 0);
		r.topLeft.x++;
		r.topLeft.y++;
		r.extent.x -= 3;
		r.extent.y -= 3;
		WinDrawRectangleFrame (popupFrame , &r);
		}
	else
		{
		r.topLeft.x++;
		r.topLeft.y++;
		r.extent.x -= 3;
		r.extent.y -= 3;
		}
	

	// Format and draw the appointments date.
	if (date)
		{
		DateToDOWDMFormat (date->month, date->day, date->year + firstYear,
			ShortDateFormat, str);

		#if WRISTPDA
		StrCopy( dateStr, str );
		#endif
	
		x = r.topLeft.x + weekDescMarginX;
		y = r.topLeft.y + weekDescMarginY;
		#if WRISTPDA
		eraseR.topLeft.x = x;
		eraseR.topLeft.y = y;
		eraseR.extent.x = 75;
		eraseR.extent.y = FntLineHeight ();
		WinEraseRectangle (&eraseR, 0);
		#endif
		WinDrawChars (str, StrLen (str), x, y);

		charsToErase = 0;
		if (date->month >= 1 && date->month <= 9)
			charsToErase++;

		if (date->day >= 1 && date->day <= 9)
			charsToErase++;

		#if WRISTPDA == 0
		// I don't know how mush wider the old day-name was than 
		// day-name just drawn, so I'll erase the width of two chatacter
		// just to be save.
		eraseR.topLeft.x = x + FntCharsWidth (str, StrLen (str));
		eraseR.topLeft.y = y;
		eraseR.extent.x = FntAverageCharWidth () * 2 + 
								FntCharWidth ('1') * charsToErase;
		eraseR.extent.y = FntLineHeight ();
		WinEraseRectangle (&eraseR, 0);
		#endif
		}


	// Format and draw the appointments start and end times
	if (startTime && endTime)
		{
		TimeToAscii (startTime->hours, startTime->minutes,
			TimeFormat, str);
		len = StrLen (str);
		str[len++] = spaceChr;
		str[len++] = '-';
		str[len++] = spaceChr;
		TimeToAscii (endTime->hours, endTime->minutes,
			TimeFormat, &str[len]);
			
		#if WRISTPDA
		x = r.topLeft.x + weekDescMarginX;
		y = r.topLeft.y + (weekDescMarginY << 1) + FntLineHeight();
		#else
		x = r.topLeft.x + (r.extent.x - weekDescMarginX - 
			FntCharsWidth (str, StrLen (str)));
		y = r.topLeft.y + weekDescMarginY;
		#endif
		
		charsToErase = 0;
		if ((/*startTime->hours >= 0 &&*/ startTime->hours <= 9) ||
			(startTime->hours >= 13 && startTime->hours <= 21))
			charsToErase++;

		if ((/*endTime->hours >= 0 &&*/ endTime->hours <= 9) ||
			(endTime->hours  >= 13 && endTime->hours <= 21))
			charsToErase++;
			
		#if WRISTPDA
		if ( 1 )
		#else
		if (charsToErase)
		#endif
			{
			#if WRISTPDA
			eraseR.topLeft.x = x;
			eraseR.topLeft.y = y;
			eraseR.extent.x = 150;
			eraseR.extent.y = FntLineHeight ();
			#else
			eraseR.topLeft.x = x - (FntCharWidth ('1') * charsToErase);
			eraseR.topLeft.y = y;
			eraseR.extent.x = x - eraseR.topLeft.x;
			eraseR.extent.y = FntLineHeight ();
			#endif
			WinEraseRectangle (&eraseR, 0);
			}
		
		WinDrawChars (str, StrLen (str), x, y);
		}


	// Draw the appointment's desciption.
	if (desc)
		{
		//DOLATER - peter: Add argument to this routine to specify
		// whether record is masked, and use here to draw mask instead
		// of description. Do this only if
		// UNMASK_PRIVATE_RECORDS_IN_WEEK_VIEW is false.
		#if WRISTPDA
		x = r.topLeft.x + (r.extent.x / 2);
		y = r.topLeft.y + weekDescMarginY;
		maxDescWidth = r.extent.x - x - weekDescMarginX;
		#else
		x = r.topLeft.x + weekDescMarginX;
		y = r.topLeft.y + (weekDescMarginY << 1) + FntLineHeight();
		maxDescWidth = r.extent.x - x - weekDescMarginX;
		#endif
	
		ptr = StrChr (desc, linefeedChr);

		charsToDraw = (ptr == NULL ? StrLen (desc) : ptr - desc);
		WinDrawTruncChars(desc, charsToDraw, x, y, maxDescWidth);
		}

	FntSetFont (curFont);
	
	return (winH);
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewScroll
 *
 * DESCRIPTION:	This routine scrolls the Week View in the direction 
 *						specified.  It is assumed that there is room to scroll
 *						in the specified direction.  If there is not, the 
 *						scroll operation will just redraw the screen in place.
 *
 *						The screen is scrolled on weekViewScrollInterval if
 *						byPage is FALSE, and by weekViewNumDispHours if
 *						byPage is TRUE, in the specified direction.
 *
 * PARAMETERS:	WinDirectionType		direction - winUp or dowm
 *					Boolean					byPage - TRUE if scrolling by page, FALSE
 *												if scrolling by weekViewScrollInterval
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	8/7/95	Initial Revision
 *			kcr	8/8/95	added page scrolling
 *			kcr	8/21/95	scroll to even intervals when not scrolling byPage
 *			grant	1/27/99	scroll by an even number of hours - otherwise the
 *								checkerboard fill patterns can get out of sync
 *			gap	11/14/00 fix checkin above to not allow the start time to 
 *								be decremented below 0
 *
 ***********************************************************************/
static void WeekViewScroll (WinDirectionType direction, Boolean byPage)
	{
	int					interval;
	RectangleType		r, vacated, origClip;
	Int16					lineYPos;
	WeekType				*week;
	UInt16				scrollDist;
	UInt16				dispStartHour;
	Boolean				forceFullRedraw = false;
	
	week = MemHandleLock (WeekObjectH);
	
	#if		WRISTPDA
	if ((direction == winDown) && (week->endHour >= week->lastEventHour))
		goto  Exit;
	if ((direction == winUp) && (week->startHour <= week->firstEventHour))
		goto  Exit;
	#endif//	WRISTPDA

	if (byPage)
		interval = weekViewNumDispHours;
	else
		interval = weekViewScrollInterval;
		
	//	Adjust week's displayed hour range in the proper direction
	if (direction == winUp)
		{
		if (byPage)
			scrollDist = weekViewNumDispHours;
		else
			scrollDist = week->startHour -
							 ((((week->startHour - interval - 1) / interval) + 1) *
							  interval);
		if ((week->startHour - week->firstEventHour) < scrollDist)
			scrollDist = week->startHour - week->firstEventHour;

		// only scroll by an even number of hours (otherwise the event fill
		// pattern gets out of sync)
		if (scrollDist % 2 != 0)
			{
			scrollDist++;
			
			// Don't scroll up (decrement) more than startHour. This field is an
			// unsigned int so wrapping around to the 655xx values is not desired. 
			// if the scroll distance is odd, just force a full update of the grid
			// area to keep the patterns in sync.   
			if (scrollDist > week->startHour)
				{
				scrollDist = week->startHour;
				forceFullRedraw = (scrollDist % 2 != 0);
				}
			}

		week->startHour -= scrollDist;
		week->endHour -= scrollDist;
		}
	else	//	winDown
		{
		if (byPage)
			scrollDist = weekViewNumDispHours;
		else
			scrollDist = (((week->startHour / interval) + 1) * interval) - 
							 week->startHour;
		if ((week->lastEventHour - week->endHour) < scrollDist)
			scrollDist = week->lastEventHour - week->endHour;

		// only scroll by an even number of hours (otherwise the event fill
		// pattern gets out of sync)
		if (scrollDist % 2 != 0)
			{
			scrollDist++;
			}

		week->startHour += scrollDist;
		week->endHour += scrollDist;
		}

	//	Redraw the hour markers:
	//	Determine where start of backround is:	
 	lineYPos = week->bounds.topLeft.y +
 						 weekViewDayDisplayHeight +
 						 weekViewDateDisplayHeight;
 	dispStartHour = week->startHour;

 	while ((dispStartHour % weekViewHourDisplayInterval) != 0)
 		{	//	Display starts on an odd hour
 		lineYPos += week->hourHeight;
 		dispStartHour++;
 		}

 	//	Draw times - vertically centered on background lines; 
 	//	horizontally centered in space between left edge of view and
 	//	left end of lines.  Erase background first.
	r = week->bounds;
	r.extent.x = weekViewTimeDisplayWidth;
	WinEraseRectangle (&r, 0);
 	WeekViewDrawTimes (lineYPos, dispStartHour, week->endHour, week->hourHeight);	

	//	We only want to scroll the 'event bar' area of the screen.
	//	Shrink the vertical size by 1 pix on top to keep the frame
	//	edges intact.
	r = week->bounds;
	r.topLeft.y += (weekViewDayDisplayHeight + weekViewDateDisplayHeight +1);
	r.extent.y = week->hourHeight*weekViewNumDispHours -1;
	r.topLeft.x += weekViewTimeDisplayWidth + 1;
	r.extent.x = r.extent.x - weekViewTimeDisplayWidth - 2;

	//	The actual scrolling dirty-work:
	// Normally the scroll distance will be even so that the fill pattern will not
	// be out of alignment when updating the vacated area of the screen after scrolling
	// the view.  Occasionally, when scrolling up, the scroll will be odd in order to pin
	// the minimum start hour to 0.  When this happens, erase & update the entire grid
	// area so that the fill pattern will remain in sync.
	if (forceFullRedraw)
		{
		vacated = r;
		}
	else
		{
		if (direction == winUp)
			WinScrollRectangle (&r, winDown, scrollDist*week->hourHeight, &vacated);
		else
			WinScrollRectangle (&r, winUp, scrollDist*week->hourHeight, &vacated);
		}
		
	// update the "vacated" area
	WinEraseRectangle (&vacated, 0);
	WinGetClip (&origClip);
	WinSetClip (&vacated);
	WeekViewDraw (FrmGetActiveForm ());
	WeekViewDrawDays ();
	WinSetClip (&origClip);
	WeekViewDrawDataMarks ();

	WeekViewUpdateScrollers (FrmGetActiveForm());

//Exit:
	MemPtrUnlock(week);
	}	//	end of WeekViewScroll


/***********************************************************************
 *
 * FUNCTION:    WeekViewGoToWeek
 *
 * DESCRIPTION:	This routine moves the Week View to the week
 *						including the current setting of Date.  It is assumed
 *						that the week view is already winUp; it must be erased,
 *						reinitialized, and redrawn.
 *
 * PARAMETERS:  	nothing
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	8/21/95	Initial Revision
 *			kcr	9/19/95	removed date calc.  Must be done by caller.
 *
 ***********************************************************************/
static void WeekViewGoToWeek (void)
	{
	WeekType				*week;

	WeekViewFree ();
	WeekViewInit (FrmGetActiveForm ());

	//	Erase the old week view before drawing the new one.
	week = MemHandleLock (WeekObjectH);
	WinEraseRectangle (&(week->bounds), 0);
	WeekViewDraw (FrmGetActiveForm ());

	WeekViewInitDays (FrmGetActiveForm ());
	WeekViewDrawDays ();
	WeekViewDrawDataMarks ();

	MemPtrUnlock(week);
	}	//	end of WeekViewGoToWeek
	

/***********************************************************************
 *
 * FUNCTION:    WeekViewDayInit
 *
 * DESCRIPTION:	This routine initializes a passed DayType structure.
 *						All fields of the DayType struct are filled in and
 *						the week->firstEventHour and week->lastEventHour vars are
 *						maintained.
 *
 * PARAMETERS:		DayType				*day - the structure to init.
 *						UInt8		dayOfWeek - matches date
 *						RectanglePtr		bounds
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	7/28/95		Initial Revision
 *			kcr	10/3/95		calc day's event range, as well as week's.
 *			kcr	10/12/95		don't calc data if date is out of range.
 *			kcr	11/13/95		remove date param; dates are set during week init
 *
 ***********************************************************************/
 static void WeekViewDayInit (DayType *day, UInt8 dayOfWeek,
 										RectanglePtr bounds,
 										UInt16 *firstEventHour, UInt16 *lastEventHour,
 										MemHandle apptsH, UInt16 numAppts)
 	{
 	ApptInfoPtr				appts = NULL;
 	int						i;

 	//	Load day bounds
 	day->bounds = *bounds;

 	//	Load the appt list
 	if (day->date.day != outOfRangeDay)
 		{
	 	day->apptsH = apptsH;
	 	day->numAppts = numAppts;
	 	day->dayOfWeek = dayOfWeek;
	 	}
	else	//	Non-displayable date.
		{
		day->apptsH = 0;
		day->numAppts = 0;
		day->dayOfWeek = 0;
		}
 	day->firstEventHour = 24;
 	day->lastEventHour = 0;

 	//	Stretch the bounds of the scrolling range if there are any events
 	//	outside the current range.  This must be performed for both
 	//	the day and the week.
	if (day->numAppts > 0)
		appts = MemHandleLock (day->apptsH);
 	for (i = 0; i < day->numAppts; i++)
 		{
 		if ((appts[i].startTime.hours != (UInt8) apptNoTime) &&
 			 (appts[i].startTime.hours < (*firstEventHour)))
 			{
 			//	Round winDown to the nearest hour <= start of appt.
 			(*firstEventHour) = appts[i].startTime.hours;
 			day->firstEventHour = appts[i].startTime.hours;
 			}
 		else if ((appts[i].startTime.hours != (UInt8) apptNoTime) &&
					(appts[i].startTime.hours < day->firstEventHour))
 			day->firstEventHour = appts[i].startTime.hours;


 		if ((appts[i].endTime.hours != (UInt8) apptNoTime) &&
 			 (appts[i].endTime.hours > (*lastEventHour) ||
 			  (appts[i].endTime.hours == (*lastEventHour) &&
 			   appts[i].endTime.minutes != 0)))
 			{
 			//	Round winUp to the next hour >= end of the appt.
 			(*lastEventHour) = appts[i].endTime.hours;
 			if (appts[i].endTime.minutes != 0)
 				(*lastEventHour)++;
 			day->lastEventHour = appts[i].endTime.hours;
 			if (appts[i].endTime.minutes != 0)
 				day->lastEventHour++;
 			}

 		else if ((appts[i].endTime.hours != (UInt8) apptNoTime) &&
 					(appts[i].endTime.hours > day->lastEventHour ||
 					 (appts[i].endTime.hours == day->lastEventHour &&
 					  appts[i].endTime.minutes != 0)))
 			{
 			//	Round winUp to the next hour >= end of the appt.
 			day->lastEventHour = appts[i].endTime.hours;
 			if (appts[i].endTime.minutes != 0)
 				day->lastEventHour++;
 			}
 		}
 	if (day->numAppts > 0)
 		MemPtrUnlock(appts);

 	}	//	end of WeekViewDayInit


/***********************************************************************
 *
 * FUNCTION:    WeekViewDayClose
 *
 * DESCRIPTION:	This routine frees all allocatd memory involved in
 *						a DayType structure.  It is generally called when a week
 *						is being torn winDown.
 *
 * PARAMETERS:		DayType				*day
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/9/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewDayClose (DayType *day)
 	{
 	if (day->numAppts != 0)
 		{
 		MemHandleFree (day->apptsH);
 		day->apptsH = 0;
 		}
 	}	//	end of WeekViewDayClose
 	

/***********************************************************************
 *
 * FUNCTION:    	WeekViewHoursToCoords
 *
 * DESCRIPTION:	Convert a time to a y-coordinate position, based on
 *						a starting hour.  'time' is assumed to be within the
 *						current hour display range.
 *
 * PARAMETERS:		TimeType				time - the time to convert
 *						UInt16					startHour - baseline of time to use
 *												as the zero-coordinate.
 *						UInt16					hourHeight - vertical size of an hour-long
 *												event.
 *
 * RETURNED:		UInt16					yPos - the difference, in y-coords,
 *												between startHour and time, based on
 *												week->hourHeight.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/3/95		Initial Revision
 *
 ***********************************************************************/
static UInt16 WeekViewHoursToCoords(TimeType time, UInt16 startHour,
											 UInt16 hourHeight)
	{
	UInt16				yPos;
	
	yPos = (time.hours - startHour) * hourHeight;
	yPos += (time.minutes * hourHeight) / 60;
	return (yPos);
	}	//	end of WeekViewHoursToCoords


/***********************************************************************
 *
 * FUNCTION:    	WeekViewGetEventBounds
 *
 * DESCRIPTION:	Determine the top & bottom bounds of an event.
 *
 * PARAMETERS:		TimeType				startTime
 *						TimeType				endTime
 *						UInt16					startHour - start of time display
 *						UInt16					endHour - end of time display
 *						RectanglePtr		r - points to rectangle holding whole
 *													event bar's coords, including top &
 *													bottom of frame.
 *						UInt16					hourHeight - height, in pix, of an hour-
 *													long event.
 *
 * RETURNED:		nothing				y-coords of r have been adjusted so that
 *													frame around event (top & bottom) will
 *													include start & end times.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/8/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewGetEventBounds(TimeType startTime, TimeType endTime,
											  UInt16 startHour, UInt16 endHour,
											  RectanglePtr r, UInt16 hourHeight)
	{
	Int16				startYOffset = 0;
	Int16				endYOffset;
	
	//	Determine start y coord, relative to top of r, inclusive of the
	//	startTime:
	if (startTime.hours > startHour ||
		 (startTime.hours == startHour &&
		  startTime.minutes != 0))
			{	//	Need to adjust start y coord.
			startYOffset = WeekViewHoursToCoords(startTime, startHour, hourHeight);
			r->topLeft.y += startYOffset;
			}
			
	//	Determine ending coord, relative to the top of the event bar,
	//	inclusive of the endTime:
	endYOffset = r->extent.y;
	if (endTime.hours < endHour)
		{	//	Need to shorten winUp end y-coord
		endYOffset = WeekViewHoursToCoords(endTime, startHour, hourHeight);
		endYOffset++;										//	Make the range inclusive
																//	of the endTime
		}

	r->extent.y = endYOffset - startYOffset;

	r->topLeft.y++;								//	Allow for top & bottom of frame
	r->extent.y -= 2;
	}	//	end of WeekViewGetEventBounds


/***********************************************************************
 *
 * FUNCTION:    	AdjustEventRight
 *
 * DESCRIPTION:	Shifts a full-width appt rect to the right side of 
 *						the event bar.  If the event bar is odd-width, we
 *						will put the smaller portion on the right.
 *
 * PARAMETERS:		RectanglePtr	event
 *
 * RETURNED:		Nothing; event is adjusted.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	10/5/95		initial version
 *
 ***********************************************************************/
static void AdjustEventRight (RectanglePtr event)
	{
	Int16				oldWidth;

	//	Make the bar an extra pix narrower to accomodate it's frame.
	oldWidth = event->extent.x;
	event->extent.x = oldWidth / 2 - 1;
	event->topLeft.x += (oldWidth - event->extent.x);
	}	//	end of AdjustEventRight


/***********************************************************************
 *
 * FUNCTION:    	AdjustEventLeft
 *
 * DESCRIPTION:	Shifts a full-width appt rect to the left side of 
 *						the event bar.  If the event bar is odd-width, the
 *						smaller half of the bar goes on the right.
 *
 * PARAMETERS:		RectanglePtr	event
 *
 * RETURNED:		Nothing; event is adjusted.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	10/5/95		initial version
 *
 ***********************************************************************/
static void AdjustEventLeft (RectanglePtr event)
	{
	//	Make the bar an extra pix narrower to accomodate the frame.
	event->extent.x = event->extent.x - (event->extent.x / 2) - 1;
	}	//	end of AdjustEventLeft


/***********************************************************************
 *
 * FUNCTION:    	UpdateEndTime
 *
 * DESCRIPTION:	Compares the passed time with the passed endTime.  If the
 *						time is later than the endTime, the endTime will be
 *						advanced to the earlier of either the time or the 
 *						timeLimit.
 *
 * PARAMETERS:		TimeType				*time - time to compare
 *						TimeType				*endTime - time to update
 *						UInt16					timeLimit - cannot advance *endTime
 *												past this hour value.
 *
 * RETURNED:		Nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	10/5/95		initial version
 *
 ***********************************************************************/
static void UpdateEndTime (TimeType *time, TimeType *endTime,
									UInt16 timeLimit)
	{
	if (TimeToInt(*time) > TimeToInt (*endTime))
		{
		if (time->hours >= timeLimit)
			{
			endTime->hours = timeLimit;
			endTime->minutes = 0;
			}
		else
			{
			endTime->hours = time->hours;
			endTime->minutes = time->minutes;
			}
		}
	}	//	end of UpdateEndTime


/***********************************************************************
 *
 * FUNCTION:    	WeekViewDayDrawAppts
 *
 * DESCRIPTION:	Draws the passed appt list within the passed 
 *						RectangleType.  This should only be called when there
 *						is a non-0 number of events and a non-0 apptsH MemHandle.
 *
 * PARAMETERS:		MemHandle				apptsH - MemHandle of mem chunk containing
 *												the ApptInfoPtr (list).
 *						UInt16					numAppts - number of entries in apptsH.
 *						RectanglePtr		r - the day's current event bar; this
 *													includes the top & bottom frame area.
 *													The width is set correctly for frame
 *													drawing over the existing frame, or
 *													rect-filling within the existing frame.
 *						UInt16					startHour - first displayed hour of the
 *												event bar.
 *						UInt16					endHour - end of the event bar.
 *						UInt16					hourHeight - vertical size of an hour-long
 *													event.
 *
 * RETURNED:		Nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/4/95		Initial Revision
 *			kcr	10/4/95		Draw overlapped events side-by-side
 *
 ***********************************************************************/
static void WeekViewDayDrawAppts (MemHandle apptsH, UInt16 numAppts,
											 RectanglePtr r, UInt16 startHour,
											 UInt16 endHour, UInt16 hourHeight)
	{
	ApptInfoPtr				appts;
	Boolean					overlapOnLeft;
	CustomPatternType		origPattern;
 	int						i;
 	RectangleType			apptRect;
 	TimeType					lastEndTimeLeft, lastEndTimeRight;

	appts = MemHandleLock (apptsH); 
	lastEndTimeLeft.hours = 0;
	lastEndTimeLeft.minutes = 0;
	lastEndTimeRight.hours = 0;
	lastEndTimeRight.minutes = 0;
	WinGetPattern(&origPattern);
	WinSetPattern((const CustomPatternType *)&regAppt);

	for (i = 0; i < numAppts; i++)
		{
		//	Only display timed appts.
		if (appts[i].startTime.hours == (UInt8) apptNoTime)
			continue;
		//	Don't display events which end <= startHour
		if (appts[i].endTime.hours < startHour ||
			 (appts[i].endTime.hours == startHour &&
			  appts[i].endTime.minutes == 0))
			continue;
		//	Don't display events which start >= endHour
		if (appts[i].startTime.hours >= endHour)
			continue;

		//	Check for overlap with previous appts on both sides:
		if (((appts[i].startTime.hours < lastEndTimeLeft.hours) ||
			  ((appts[i].startTime.hours == lastEndTimeLeft.hours) &&
			   (appts[i].startTime.minutes < lastEndTimeLeft.minutes))) &&
			 ((appts[i].startTime.hours < lastEndTimeRight.hours) ||
			  ((appts[i].startTime.hours == lastEndTimeRight.hours) &&
			   (appts[i].startTime.minutes < lastEndTimeRight.minutes))))
			{
			//	Overlapping appts need to be drawn in two parts - the overlapping
			//	part and the non-overlapping part, if any.
			apptRect = *r;

			//	Select side on which to overlap the bars:
			if (TimeToInt (lastEndTimeLeft) > TimeToInt (lastEndTimeRight))
				overlapOnLeft = false;
			else
				overlapOnLeft = true;

			//	The overlap area ends at the earlier of the appropriate
			//	lastEndTime or the appt's endTime.
			if (overlapOnLeft &&
				 (TimeToInt(appts[i].endTime) > TimeToInt (lastEndTimeLeft)))
				WeekViewGetEventBounds (appts[i].startTime, lastEndTimeLeft,
												startHour, endHour, &apptRect,
												hourHeight);
			else if (!overlapOnLeft &&
						(TimeToInt(appts[i].endTime) > TimeToInt (lastEndTimeRight)))
				WeekViewGetEventBounds (appts[i].startTime, lastEndTimeRight,
												startHour, endHour, &apptRect,
												hourHeight);
			else
				WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
												startHour, endHour, &apptRect,
												hourHeight);

			//	Select side to overlap
			if (!overlapOnLeft)
				AdjustEventRight (&apptRect);
			else
				AdjustEventLeft (&apptRect);

			WinSetPattern((const CustomPatternType *)&overLapAppt);
			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			WinSetPattern((const CustomPatternType *)&regAppt);
			
			//	Now do the non-overlapping part.
			if ((overlapOnLeft &&
				  TimeToInt(appts[i].endTime) <= TimeToInt (lastEndTimeLeft)) ||
				 (!overlapOnLeft &&
				  TimeToInt(appts[i].endTime) <= TimeToInt (lastEndTimeRight)))
				continue;			//	No non-overlapping part
			apptRect = *r;
			if (overlapOnLeft)
				{
				WeekViewGetEventBounds (lastEndTimeLeft, appts[i].endTime,
												startHour, endHour, &apptRect,
												hourHeight);
				AdjustEventLeft (&apptRect);
				UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
				}
			else
				{
				WeekViewGetEventBounds (lastEndTimeRight, appts[i].endTime,
												startHour, endHour, &apptRect,
												hourHeight);
				AdjustEventRight (&apptRect);
				UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
				}

			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			
			}	//	end of if <overlapping appt>

		//	Check for overlap with left side only:
		else if ((appts[i].startTime.hours < lastEndTimeLeft.hours) ||
					((appts[i].startTime.hours == lastEndTimeLeft.hours) &&
					 (appts[i].startTime.minutes < lastEndTimeLeft.minutes)))
			{	//	draw non-overlapping appt on right side; update right end time
			apptRect = *r;							//	Use pre-calculated x & y coords.
			WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
											startHour, endHour, &apptRect,
											hourHeight);
			AdjustEventRight (&apptRect);

			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
			}
			   
		//	Check for overlap with right side only:
		else if ((appts[i].startTime.hours < lastEndTimeRight.hours) ||
					((appts[i].startTime.hours == lastEndTimeRight.hours) &&
					 (appts[i].startTime.minutes < lastEndTimeRight.minutes)))
			{	//	draw non-overlapping appt on left side; update left end time
			apptRect = *r;							//	Use pre-calculated x & y coords.
			WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
											startHour, endHour, &apptRect,
											hourHeight);
			AdjustEventLeft (&apptRect);

			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
			}

		//	Appt does not overlap previous appt - check for overlap with
		//	next appt.
		else if (((i + 1) < numAppts) &&
					((appts[i+1].startTime.hours < appts[i].endTime.hours) ||
					 ((appts[i+1].startTime.hours == appts[i].endTime.hours) &&
					  (appts[i+1].startTime.minutes < appts[i].endTime.minutes))))
			{	//	draw non-overlapping appt on left side; update left end time.
			apptRect = *r;							//	Use pre-calculated x & y coords.
			WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
											startHour, endHour, &apptRect,
											hourHeight);
			AdjustEventLeft (&apptRect);

			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
			}	//	end of if <overlapping next appt>

		else	//	Appt does not overlap previous appts:
			{	//	Draw full width appt; update both left & right end times.
			apptRect = *r;							//	Use pre-calculated x & y coords.
			WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
											startHour, endHour, &apptRect,
											hourHeight);

			WinFillRectangle(&apptRect, 0);
			WinDrawRectangleFrame(simpleFrame, &apptRect);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeLeft, endHour);
			UpdateEndTime (&(appts[i].endTime), &lastEndTimeRight, endHour);
			}	//	end of if <non-overlapping appt>

		}	//	end of for <each appt in day>
		

	//	Clean up after drawing events
	MemPtrUnlock(appts);
	WinSetPattern((const CustomPatternType *)&origPattern);
	
	}	//	end of WeekViewDayDrawAppts
		

/***********************************************************************
 *
 * FUNCTION:    WeekViewCreateEventBar
 *
 * DESCRIPTION:	This routine calculates the area of a given day's
 *						event bar.
 *
 * PARAMETERS:		RectanglePtr		day - full day's bounds
 *						RectanglePtr		r - points to rect to be filled with
 *													event bar's coords.  The resulting
 *													bar does not include the frame around
 *													it.  The top & bottom of the frame
 *													should be added before this is passed
 *													to GetEventBounds
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/10/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewCreateEventBar (RectanglePtr day, RectanglePtr r)
	{
 	//	NOTE: since this is a framed event bar, the bounds of the bar must
 	//	be shrunk 1 pix on top & sides to keep the frame (drawn outside of passed
 	//	rect) within the bounds.  This is only nesc because we are using 
 	//	a frame around the event bar.
 	r->topLeft.x = day->topLeft.x + weekViewDayHMargin + 1;
 	r->topLeft.y = day->topLeft.y +
 						weekViewDayDisplayHeight +
 						weekViewDateDisplayHeight + 1;
 	r->extent.x = day->extent.x - 2*weekViewDayHMargin - 1;
 	r->extent.y = day->extent.y -
 						weekViewDayDisplayHeight -
 						weekViewDateDisplayHeight - 1;
 	}	//	end of WeekViewCreateEventBar
 					

/***********************************************************************
 *
 * FUNCTION:    WeekViewDayDraw
 *
 * DESCRIPTION:	This routine draws a passed DayObject structure.
 *
 * PARAMETERS:		DayType				*day - the day to draw
 *						UInt16				startHour - first displayed hour of the
 *												day
 *						UInt16				endHour - end of event bar display
 *						UInt16				hourHeight - vertical size of hour-long
 *												event
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	7/31/95		Initial Revision
 *			kcr	10/3/95		remove frame around whole day.
 *			kcr	10/13/95		don't draw days which are outside of date range
 *
 ***********************************************************************/
static void WeekViewDayDraw (DayType *day, UInt16 startHour, UInt16 endHour,
									  UInt16 hourHeight)
 	{
 	RectangleType			r;

	//	Only draw days which are in range.
	if (day->date.day == outOfRangeDay)
		return;

 	//	Use day bounds to create event bar bounds
 	WeekViewCreateEventBar (&(day->bounds), &r);

 	//	Draw the appointments - including overlaps.  The current appointment
 	//	is highlighted specially.  Only show portions of events which fall
 	//	between startHour & endHour.
	if (day->numAppts != 0)
		{
		//	Pass event bar to the event drawing routine, including the top &
		//	bottom frame areas.
		r.topLeft.y--;
		r.extent.y += 2;
		WeekViewDayDrawAppts (day->apptsH, day->numAppts, &r, startHour,
									 endHour, hourHeight);
 		}
  	}	//	end of WeekViewDayDraw


/***********************************************************************
 *
 * FUNCTION:    WeekViewInitDays
 *
 * DESCRIPTION:	Check the data of the days in the current week and
 *						extend the week's time range if nesc.  Assumes the
 *						week has already been initialized.
 *
 *						This routine is performing all of the heavy data access
 *						work, so that the screen can be drawn quickly without
 *						the burden of the database.
 *
 * PARAMETERS:  FormPtr			frm - The currently active (WeekView) form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	11/13/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewInitDays (FormPtr frm)
	{
 	int					i;
 	RectangleType		dayBounds;
	Int16					dayWidth;
 	WeekType				*week;
	UInt16 				counts[daysInWeek];
	MemHandle 			apptLists[daysInWeek];

 	ErrFatalDisplayIf(!WeekObjectH, "Unallocated Week View object");
	week = MemHandleLock (WeekObjectH);

 	//	Initialize the days:
 	dayWidth = (week->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
 	dayBounds.topLeft.x = week->bounds.topLeft.x + weekViewTimeDisplayWidth;
 	dayBounds.topLeft.y = week->bounds.topLeft.y;
 	dayBounds.extent.x = dayWidth;
 	dayBounds.extent.y = week->hourHeight * weekViewNumDispHours +
 								weekViewDayDisplayHeight +
 								weekViewDateDisplayHeight;


	// Get all the appointments in the week.
	ApptGetAppointments (ApptDB, week->startDate, daysInWeek, 
		apptLists, counts);

 	for (i = 0; i < daysInWeek; i++)
 		{
 		WeekViewDayInit(&(week->days[i]),
 							 (i + StartDayOfWeek) % daysInWeek,
 							 &dayBounds,
 							 &(week->firstEventHour),
 							 &(week->lastEventHour),
 							 apptLists[i], counts[i]);

 		dayBounds.topLeft.x += dayWidth;					//	Next day bounds
 		}

 	//	Initailize scroll controls
	WeekViewUpdateScrollers(frm);

 	MemPtrUnlock (week);
	}	//	end of WeekViewInitDays


/***********************************************************************
 *
 * FUNCTION:    WeekViewSetTitle
 *
 * DESCRIPTION: This routine set the title of the Week View form
 *
 * PARAMETERS:  week  - pointer to week object (WeekType)
 *
 * RETURNED:	 nothing
 *
 * HISTORY:
 *		07/15/96	art	Created by Art Lamb
 *		03/11/97	scl	added dfMYMedNoPost for French 2.0 ROM
 *		07/07/99	kwk	use soft constant for date format.
 *		09/12/99	kwk	Re-wrote to use templates for all dates.
 *		10/05/99	kwk	Let localizers control location of full date (w/year)
 *							in the "same year, spans months" layout.
 *		10/7/99		EL	fixed title computation for date dec 31 2031.
 *
 ***********************************************************************/
static void WeekViewSetTitle (WeekType * week)
{
	Char	startDate[longDateStrLength];
	Char	endDate[longDateStrLength];
	MemHandle templateH;
	Char* templateP;
	Char* title;
	UInt16 startDateID, endDateID, templateID;
	UInt16 weekLastDayIndex;
	
	// Find the last valid day (out of range days are number outOfRangeDay)
	
	weekLastDayIndex = daysInWeek - 1;
	while (weekLastDayIndex > 0) {
		if (week->days[weekLastDayIndex].date.day != outOfRangeDay)
			break;
		else
			weekLastDayIndex--;
	}

 	//	Set the title of the form to be the localized month/year date of the
 	//	StartDayOfWeek.  The format shall be "Mmm 'YY - Mmm 'YY", or
 	//	"Mmm - Mmm 'YY" if the year is the same, or "Mmm 'YY" if the month
 	//	is the same.

	if (week->startDate.year != week->days[weekLastDayIndex].date.year)
		{
		// Start/end year is different, so go for "Mmm 'YY - Mmm 'YY".
		startDateID = WeekViewTitleFullDateStrID;
		endDateID = WeekViewTitleFullDateStrID;
		templateID = WeekViewTitleTwoDatesStrID;
		}
	else if ((week->startDate.month != week->days[weekLastDayIndex].date.month) &&
 		 	(week->days[weekLastDayIndex].date.day != outOfRangeDay))
		{
		// Start/end year is the same, but month is different, so use "Mmm - Mmm 'YY"
		// or "Mmm 'YY - Mmm", depending on weekViewYearFirst soft constant.
		if (ResLoadConstant(weekViewYearFirst) == false)
			{
			startDateID = WeekViewTitleShortDateStrID;
			endDateID = WeekViewTitleFullDateStrID;
			}
		else
			{
			startDateID = WeekViewTitleFullDateStrID;
			endDateID = WeekViewTitleShortDateStrID;
			}
		
		templateID = WeekViewTitleTwoDatesStrID;
		}
	else
		{
		// Start/end year & month are the same, so use "Mmm 'YY"
		startDateID = WeekViewTitleFullDateStrID;
		endDateID = WeekViewTitleEmptyDateStrID;
		templateID = WeekViewTitleOneDateStrID;
		}

	templateH = DmGetResource(strRsc, startDateID);
	templateP = (Char*)MemHandleLock(templateH);
	DateTemplateToAscii(templateP, week->startDate.month, week->startDate.day,
 			week->startDate.year + firstYear, startDate, sizeof(startDate) - 1);
 	
 	if (startDateID != endDateID)
 		{
		MemPtrUnlock((MemPtr)templateP);
		templateH = DmGetResource(strRsc, endDateID);
		templateP = (Char*)MemHandleLock(templateH);
		}
	
	DateTemplateToAscii(templateP, week->days[weekLastDayIndex].date.month,
		week->days[weekLastDayIndex].date.day, week->days[weekLastDayIndex].date.year + firstYear,
		endDate, sizeof(endDate) - 1);
	MemPtrUnlock((MemPtr)templateP);

	templateH = DmGetResource(strRsc, templateID);
	templateP = (Char*)MemHandleLock(templateH);

	title = TxtParamString(templateP, startDate, endDate, NULL, NULL);
	MemPtrUnlock((MemPtr)templateP);
	FrmCopyTitle (FrmGetActiveForm (), title);
	MemPtrFree((MemPtr)title);
	
	TimeDisplayed = false;
	AttnIndicatorEnable(true);
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewInit
 *
 * DESCRIPTION:	This routine initializes the structures used for the
 *						WeekView.  The start date will be set and the start
 *						and end of the display range will be set.
 *
 * PARAMETERS:  FormPtr			frm - The currently active (WeekView) form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	7/27/95		Initial Revision
 *			kcr	10/4/95		New title date format
 *			kcr	10/9/95		Make sure lastEventHour does not get adjusted past
 *									midnight. Bug #260.
 *			kcr	10/12/95		handle beginning of date range & end of date
 *									range.
 *			kwk	07/07/99		Create date range title in localizable manner.
 *
 ***********************************************************************/
static void WeekViewInit (FormPtr frm)
 	{
 	int					i, skipFirstDays = 0, skipLastDays = 0;
 	DateType				tempDate;
 	WeekType				*week;
	UInt16				index;
 	UInt16				startDay;
 	UInt16				weekNumber;
	Char 					weekNumStr[16];
	Char					*templateP;
	Char					*weekString;

 	//	Allocate currWeek object & day objects
 	WeekObjectH = MemHandleNew (sizeof (WeekType));
 	ErrFatalDisplayIf(!WeekObjectH, "Allocation error");
	week = MemHandleLock (WeekObjectH);

 	//	Set bounds of WeekView
	index  = FrmGetObjectIndex (frm, WeekDayDisplay);
	FrmGetObjectBounds (frm, index, &(week->bounds));
	week->bounds.extent.x -= weekHMargin;

 	//	Determine the start date for this week based on the current 'Date' and
 	//	the StartDayOfWeek
 	week->startDate.year = Date.year;
 	week->startDate.month = Date.month;
 	week->startDate.day = Date.day;

 	startDay = DayOfWeek (Date.month, Date.day, Date.year+firstYear);
 																//	Current day of week
 	//	Adjust startDate backwards to previous occurrance of StartDayOfWeek
 	if (startDay > StartDayOfWeek)
 		DateAdjust (&(week->startDate), - ((Int32) (startDay - StartDayOfWeek)));
 	else if (StartDayOfWeek > startDay)
 		DateAdjust (&(week->startDate),
 								- ((Int32)(startDay + daysInWeek - StartDayOfWeek)));


 	//	Make sure we have not run into the start of the legal date
 	//	range.  If this has happened, we will skip the
 	//	dates betweeen the StartDayOfWeek and the beginning of the legal
 	//	date range.
	if ((week->startDate.year == 0) &&
		 (week->startDate.month == 1) &&
		 (week->startDate.day == 1))
		{
		startDay = DayOfWeek (1, 1, firstYear);
		if (startDay > StartDayOfWeek)
			skipFirstDays = startDay - StartDayOfWeek;
		else if (StartDayOfWeek > startDay)
			skipFirstDays = daysInWeek + startDay - StartDayOfWeek;
		}
	//	Make sure we have not run into the end of the legal date
	//	range.  If this happens, we will skip the days between the
	//	end of the date range and the week->startDate+daysInWeek.
	else if ((week->startDate.year == numberOfYears - 1) &&
				(week->startDate.month == 12) &&
				(week->startDate.day > (31 - (daysInWeek - 1))))
		{
		startDay = DayOfWeek (1, 1, lastYear);
		if (startDay >= StartDayOfWeek)
			skipLastDays = daysInWeek - (startDay - StartDayOfWeek) - 1;
		else
			skipLastDays = daysInWeek - (startDay + daysInWeek - StartDayOfWeek)
								- 1;
		}


 	//	week->startDate is now the date of the first instance of 
 	//	StartDayOfWeek <= the current Date.  An exception occurs when
 	//	the beginning of the legal date range has been hit.  In this case,
 	//	week->startDate falls on StartDayOfWeek+skipFirstDays.

 	//	Determine the scrolling range, and the initial displayed time range:
 	//
 	//	These vars hold the elligible scrolling range for the current week.
 	//	The range will be expanded as necessary as the days' events are
 	//	examined.
 	week->firstEventHour = DayStartHour;
 	week->lastEventHour = DayEndHour;

 	//	If range is still less than a screenful, stretch it.  End of display
 	//	may not extend past midnight, however.
 	if ((week->lastEventHour - week->firstEventHour < weekViewNumDispHours) &&
 		 (week->firstEventHour + weekViewNumDispHours > 24))
 		{	//	Have to stretch range at both ends
 		week->lastEventHour = 24;
 		week->firstEventHour = week->lastEventHour - weekViewNumDispHours;
 		}
 	else if (week->lastEventHour - week->firstEventHour < weekViewNumDispHours)
 		week->lastEventHour = week->firstEventHour + weekViewNumDispHours;

 	week->hourHeight = (week->bounds.extent.y -
 								 weekViewDayDisplayHeight -
 								 weekViewDateDisplayHeight) / weekViewNumDispHours;
 	//	The display will initially show all events from the start of the
 	//	user's day range to the end of the vertical screen space.  If there
 	//	is room for more hours of data than the user's day range specified,
 	//	a full screenful will be shown anyway.
 	week->startHour = week->firstEventHour;
 	week->endHour = week->firstEventHour + weekViewNumDispHours;


	//	Set the dates of each day so they can be drawn before initializing the
	//	days.
 	tempDate = week->startDate;
	for (i = 0; i < daysInWeek; i++)
		{
 		if ((i < skipFirstDays) ||
 			 (i >= (daysInWeek - skipLastDays)))
 			{
 			week->days[i].date.day = outOfRangeDay;
 			}
 		else
 			{
			week->days[i].date = tempDate;
 			DateAdjust (&tempDate, 1);						//	Next date
			}
		}

 	//	Set the title of the form
 	WeekViewSetTitle (week);

	// Create the week number string. We'll load the template string
	// and then substitute in the number string.
	
	weekNumber = GetWeekNumber (Date.month, Date.day, Date.year+firstYear);
	StrIToA(weekNumStr, weekNumber);
	if (StrLen(weekNumStr) == 1)
		{
		weekNumStr[1] = weekNumStr[0];
		weekNumStr[0] = chrNumericSpace;
		weekNumStr[2] = '\0';
		}
	
	templateP = (Char*)MemHandleLock(DmGetResource(strRsc, weekNumberTemplateStrID));
	weekString = DateParamString(templateP, NULL, weekNumStr, NULL, NULL);
	FrmCopyLabel(frm, WeekNumberLabel, weekString);
	MemPtrFree(weekString);
	MemPtrUnlock(templateP);
	
	// Highlight the Week View push button.
	FrmSetControlGroupSelection (frm, DayViewGroup, WeekWeekViewButton);

 	week->behindDesc = NULL;
 	
 	MemPtrUnlock (week);
 	} //	end of WeekViewInit


/***********************************************************************
 *
 * FUNCTION:    WeekViewFree
 *
 * DESCRIPTION:	This routine is called when the WeekView is being torn
 *						winDown.  It's primary function is to free allocated
 *						memory.
 *
 * PARAMETERS:  	nothing
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/9/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewFree (void)
 	{
 	int					i;
 	WeekType				*week;
 	
	WeekEraseDescription (true);

	week = MemHandleLock (WeekObjectH);
	//	Close each day structure
	for (i = 0; i < daysInWeek; i++)
		WeekViewDayClose (&(week->days[i]));
		
	//	Free WeekType obj.
	MemHandleFree (WeekObjectH);
	WeekObjectH = NULL;
 	}	//	end of WeekViewFree


/***********************************************************************
 *
 * FUNCTION:    WeekViewShowTime
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
 *			grant	2/2/99	Don't use EvtSetNullEventTick()
 *
 ***********************************************************************/
static void WeekViewShowTime (void)
{
	Char				title[timeStringLength];
	DateTimeType 	dateTime;

	AttnIndicatorEnable(false);
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	TimeToAscii (dateTime.hour, dateTime.minute, TimeFormat, title);
	FrmCopyTitle (FrmGetActiveForm (), title);
	
	TimeDisplayed = true;
	TimeDisplayTick = TimGetTicks () + timeDisplayTicks;
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewHideTime
 *
 * DESCRIPTION: If the title of the Week View is displaying the current 
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
static void WeekViewHideTime (Boolean hide)
{
	WeekPtr 	week;

	if (TimeDisplayed)
		{
		if (hide || TimeToWait() == 0)
			{
			week = MemHandleLock (WeekObjectH);
			WeekViewSetTitle (week);
			MemPtrUnlock (week);
			}
		}
}


/***********************************************************************
 *
 * FUNCTION:     WeekViewDayLabels
 *
 * DESCRIPTION:  Draw the day of week and day number labels along the 
 *					  top of the week view display.
 *
 * PARAMETERS:	  week - pointer the week object (WeekType)
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	4/23/96		Initial Revision
 *			kwk	07/07/99		Rolled in Karma code to handle day label correctly.
 *			EL    10/7/99		fixed bug to not break out when week starts with out of range days
 *
 ***********************************************************************/
static void WeekViewDayLabels (WeekType *week)
	{
	Int16				i;
 	Char				dateStr[3];				//	temp storage for string 
 													//	representing the date number.
	UInt8 			dayOfWeek;
	Int16				x, y;
	Int16				dayWidth;
	Int16				labelWidth;
	FontID 			origFont;
 	Char *			dayLabels;
	DayType 			*day;
 	DateTimeType	today;
	UInt16			labelLength;
	Char*				label;
	

	origFont = FntGetFont();

	// Get the resource that contains the first letter of each day.
 	dayLabels = MemHandleLock (DmGetResource (strRsc, daysOfWeekInitialsStrID));

	// Calculate length of one item in string */
	labelLength = StrLen (dayLabels) / 7;

	// Get today's date.
 	TimSecondsToDateTime(TimGetSeconds(), &today);

 	dayWidth = (week->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
 	x = week->bounds.topLeft.x + weekViewTimeDisplayWidth;
 	y = week->bounds.topLeft.y;

 	for (i = 0; i < daysInWeek; i++)
 		{
		day = &(week->days[i]);

		//	Only draw days which are in range.
		if (day->date.day != outOfRangeDay) {
			
		 	//	If this day's date matches the system date, then we will use
		 	//	FossilBoldFont to draw the day & date labels.
		 	if (day->date.year+firstYear == today.year &&
		 		 day->date.month == today.month &&
		 		 day->date.day == today.day)
		 		//FntSetFont (FossilBoldFont);
		 		FntSetFont (boldFont);
		 	else
		 		//FntSetFont (FossilStdFont);
		 		FntSetFont (stdFont);
		 	
		 	//	Draw the day labels over the centers of the bars:
			dayOfWeek = (i + StartDayOfWeek) % daysInWeek;
			label = &dayLabels[labelLength * dayOfWeek];
		 	labelWidth = FntCharsWidth (label, labelLength);
		 	WinDrawChars (label, labelLength, x + (dayWidth - labelWidth) / 2 + 1, y);
		 	
		 	//	Draw the date labels under the day labels:
		 	StrIToA (dateStr, day->date.day);
		 	labelWidth = FntCharsWidth (dateStr, StrLen(dateStr));
		 	WinDrawChars (dateStr,
		 					  StrLen(dateStr),
		 					  x + (dayWidth - labelWidth)/2+1,
		 					  y + weekViewDayDisplayHeight);
			}
		x += dayWidth;
		}

	MemPtrUnlock (dayLabels);
	
 	//	Restore the original font
 	FntSetFont (origFont);
	}
	

/***********************************************************************
 *
 * FUNCTION:    WeekViewDrawTimes
 *
 * DESCRIPTION:	Draw the time markers winDown the left-hand edge of 
 *						the week view display.
 *
 * PARAMETERS:		Int16			startYPos - Y position of first hour marker
 *						UInt16			startHour - first hour marker to draw
 *						UInt16			endHour - end of display range; may or may not
 *											be the last visible marker.
 *						UInt16			hourHeight - vertical size of an hour-long event.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	8/8/95		Initial Revision
 *			kcr	10/3/95		include minutes in times; right justify times.
 *
 ***********************************************************************/
static void WeekViewDrawTimes (Int16 startYPos, UInt16 startHour, UInt16 endHour,
										 UInt16 hourHeight)
	{
 	char					hourStr[timeStringLength];	//	Holds string representing
 																//	hours.
	int					j;
 	Int16					hourWidth, hourYPos;
 	UInt16					nextHour;
 	FontID					currFont;

	
	//currFont =  FntSetFont (FossilStdFont);
	currFont =  FntSetFont (stdFont);
 	hourYPos = startYPos - FntLineHeight() / 2;

	//	If we are using the 24 hours format, never write 24:00
	if (Use24HourFormat(TimeFormat) && endHour > 24)
		endHour = 24;
		
	for (nextHour = startHour;
		  nextHour < endHour;
		  nextHour += weekViewHourDisplayInterval,
		  hourYPos += hourHeight*weekViewHourDisplayInterval)
		{
		TimeToAscii (nextHour, 0, TimeFormat, hourStr);
		if (!Use24HourFormat(TimeFormat))
			{	//	If we are using the 12-hour/Am/Pm format, strip off the
				//	trailing am/pm
			for (j = 0; hourStr[j] != ' ' && j < timeStringLength - 1; j++)
				;
			hourStr[j] = '\0';
			}
		hourWidth = FntCharsWidth (hourStr, StrLen (hourStr));
		WinDrawChars (hourStr,
	 					  StrLen (hourStr),
	 					  weekViewTimeDisplayWidth - timeHMargin - hourWidth,
						  hourYPos);
		}	//	end of for <each hour marker>

	FntSetFont (currFont);
	}	//	end of WeekViewDrawTimes


/***********************************************************************
 *
 * FUNCTION:    WeekViewDraw
 *
 * DESCRIPTION:	This routine draws the background for the WeekView - the
 *						dotted lines, the hour markers, and then draws each
 *						day.
 *
 * PARAMETERS:  FormPtr			frm - The currently active (WeekView) form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	7/27/95		Initial Revision
 *			kcr	8/8/95		can draw starting on odd or even hours
 *			kcr	10/3/95		draw frame around all day bars; dividing lines
 *									between days.
 *			kcr	11/13/95		break out day-drawing to another routine
 *
 ***********************************************************************/
static void WeekViewDraw (FormPtr UNUSED_PARAM(frm))
 	{
 	Int16					i;
 	Int16					x;
 	Int16					dayWidth;
 	RectangleType		r;
 	Int16					lineYPos;
 	WeekType				*week;
 	UInt16				dispStartHour;

	WinPushDrawState();
	week = MemHandleLock (WeekObjectH);

	//	Determine where start of backround is:
 	lineYPos = week->bounds.topLeft.y +
 						 weekViewDayDisplayHeight +
 						 weekViewDateDisplayHeight;
 	dispStartHour = week->startHour;
 	
	WinSetForeColor(WinRGBToIndex(&colorLine));
	
	//	Draw frame around whole day-area.  Cut rect winDown by one pix on each
	//	side so frame will be in week area, not outside.
	r.topLeft.x = week->bounds.topLeft.x + weekViewTimeDisplayWidth + 1;
	r.topLeft.y = lineYPos + 1;		//	Want frame to be on line, not outside.
	r.extent.x = week->bounds.extent.x - weekViewTimeDisplayWidth - 2;
	r.extent.y = weekViewNumDispHours * week->hourHeight - 1;
	WinDrawRectangleFrame (simpleFrame, &r);

 	//	Draw background (solid lines).
 	x = week->bounds.topLeft.x + weekViewTimeDisplayWidth;
 	dayWidth = (week->bounds.extent.x - weekViewTimeDisplayWidth) / daysInWeek;
 	for (i = 1; i < daysInWeek; i++)
 		{
 		x += dayWidth;
 		WinDrawLine (x, r.topLeft.y, x, r.topLeft.y + r.extent.y);
		}

	//	Adjust start hour for times & lines to be on a display interval.
 	while ((dispStartHour % weekViewHourDisplayInterval) != 0)
 		{	//	Display starts on an odd hour
 		lineYPos += week->hourHeight;
 		dispStartHour++;
 		}

	// Draw the day numbers and doy of week labels.
	WeekViewDayLabels (week);

 	//	Draw times
 	WeekViewDrawTimes (lineYPos, dispStartHour, week->endHour, week->hourHeight);

 	//	Draw background (dotted lines).  The dotted lines are drawn at
 	//	specific hour intervals from the top of the bar area to the end of
 	//	the hour display.
 	//	Lines will not be drawn for the first & last hours - the frame should
 	//	include this.
 	if (week->startHour == dispStartHour)
 		{
 		lineYPos += week->hourHeight*weekViewHourDisplayInterval;
 		dispStartHour += weekViewHourDisplayInterval;
 		}
 	for (;
 		  dispStartHour < week->endHour;
 		  dispStartHour += weekViewHourDisplayInterval)
 		{
 		WinDrawGrayLine (week->bounds.topLeft.x + weekViewTimeDisplayWidth + 1,
 						 	  lineYPos,
 						 	  week->bounds.topLeft.x + week->bounds.extent.x - 2,
 						 	  lineYPos);

		//	Position of next line
 		lineYPos += week->hourHeight*weekViewHourDisplayInterval;
  		}	//	end of for <each hour interval>
  	
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));


 	MemPtrUnlock (week);
	WinPopDrawState();
 	} //	end of WeekViewDraw
 	

/***********************************************************************
 *
 * FUNCTION:    WeekViewDrawDays
 *
 * DESCRIPTION:	This routine draws each
 *						day.  This should be called after WeekViewInitDays.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	11/13/95		Initial Revision
 *
 ***********************************************************************/
static void WeekViewDrawDays (void)
	{
	int	i;
 	WeekType				*week;
	
	week = MemHandleLock (WeekObjectH);

	for (i = 0; i < daysInWeek; i++)
 		{
		/*
 		if (i != 0)
 			WinDrawLine (week->days[i].bounds.topLeft.x,
 							 week->days[i].bounds.topLeft.y +
 						 		weekViewDayDisplayHeight +
 						 		weekViewDateDisplayHeight,
 							 week->days[i].bounds.topLeft.x,
 							 week->days[i].bounds.topLeft.y +
 							 	week->days[i].bounds.extent.y);
 		*/

 		WeekViewDayDraw(&(week->days[i]), week->startHour, week->endHour,
 						 week->hourHeight);
		}

 	MemPtrUnlock (week);
	}	//	end of WeekViewDrawDays


/***********************************************************************
 *
 * FUNCTION:    WeekViewEraseDay
 *
 * DESCRIPTION:  This routine erase the appointment on the specified day.
 *
 * PARAMETERS:  day  - day of the week
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	4/16/96		Initial Revision
 *
 ***********************************************************************/
static void WeekViewEraseDay (WeekType *week, Int16 dayOfWeek)
	{
	RectangleType dayR;
	
	dayR = week->days[dayOfWeek].bounds;
	dayR.topLeft.x += 1;
	dayR.extent.x -= 1;
	dayR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight + 1;
	dayR.extent.y -= weekViewDayDisplayHeight + weekViewDateDisplayHeight + 1;
	WinEraseRectangle (&dayR, 0);
	}


/***********************************************************************
 *
 * FUNCTION:    WeekEraseDescription
 *
 * DESCRIPTION: This routine will will erase the description
 *              popup if been visible.
 *
 * PARAMETERS:  eraseIt - true to always erase, false erase only if
 *                        to popup has been display for the require
 *                        length of time.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/7/96	Initial Revision
 *			grant 2/2/99	Set TimeDisplayed to false to indicate that
 *								the temporary display has been erased.
 *
 ***********************************************************************/
static void WeekEraseDescription (Boolean eraseIt)
	{
 	WeekType	*week;
		
	week = MemHandleLock (WeekObjectH);
	
	if (week->behindDesc)
		{
		if ((eraseIt) || TimeToWait() == 0)
			{
			WinRestoreBits (week->behindDesc, 0, 0);
			week->behindDesc = NULL;
			TimeDisplayed = false;
			AttnIndicatorEnable(true);
			}
		}

 	MemPtrUnlock (week);
}

/***********************************************************************
 *
 * FUNCTION:    WeekViewPointInDescription
 *
 * DESCRIPTION: This routine returns true if the piont passed is within
 *              the bounds if an appointment description popup.
 *
 * PARAMETERS:  x, y - pen location
 *
 * RETURNED:    true if point is in description popup.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/15/96	Initial Revision
 *
 ***********************************************************************/
static Boolean WeekViewPointInDescription (Int16 x, Int16 y)
	{
 	WeekType	*week;
	Boolean inDesc = false;
	WinHandle win;
	RectangleType r;
	
	week = MemHandleLock (WeekObjectH);
	
	if (week->behindDesc)
		{
		win = WinSetDrawWindow (week->behindDesc);		
		r.topLeft.x = 0;
		r.topLeft.y = 0;
		WinGetWindowExtent (&r.extent.x, &r.extent.y);
		WinSetDrawWindow (win);

		inDesc = RctPtInRectangle (x, y, &r);
		}

 	MemPtrUnlock (week);

	return (inDesc);
	}


/***********************************************************************
 *
 * FUNCTION:    WeekViewSelectEvent
 *
 * DESCRIPTION:  This routine draws the selection highlight around and 
 *               move the event.
 *
 * PARAMETERS:  week
 *              dayOfWeek
 *              event
 *              recordNum
 *					 displayTicks
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	3/29/96		Initial Revision
 *			grant	2/2/99		Update TimeDisplayed and TimeDisplayTick to
 *									indicate that a temporary display is winUp.
 *									Don't use EvtSetNullEventTick().
 *			peter	4/27/00		Add argument for displayTicks
 *
 ***********************************************************************/
static void WeekViewSelectEvent (WeekType* week, Int16 dayOfWeek, 
	 EventType* event, UInt16 recordNum, Int32 displayTicks)

	{
	Int16					i;
	Int16					newDayOfWeek;
	Int16					savedDayOfWeek;
	UInt16				error;
	UInt16				hourHeight;
	Int16					x, y;
	Int16 				posInDay;
	Int16					posInAppt;
	Boolean 				moving = false;
	Boolean				penDown = true;
	Boolean				moved;
	Boolean				sameTime;
	Boolean				sameDate;
	TimeType 			newStart, timeDiff;
	TimeType	 			start, end;
	DateType				date;
	WinHandle 			savedBits;
	WinHandle			bitsBehindDesc;
	MemHandle			recordH;
	RectangleType		moveR, dayR, weekR, eventR;
	FrameBitsType		eventFrame;
	ApptDBRecordType	apptRec;

	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);

	start = apptRec.when->startTime;
	end = apptRec.when->endTime;
	date = week->days[dayOfWeek].date;
	TimeDifference (&end, &start, &timeDiff);

	moveR.topLeft.x = event->screenX - moveThreshold;
	moveR.topLeft.y = event->screenY - moveThreshold;
	moveR.extent.x = moveThreshold << 1;
	moveR.extent.y = moveThreshold << 1;
	
	dayR = week->days[dayOfWeek].bounds;
 	dayR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 	dayR.extent.y -= weekViewDayDisplayHeight + weekViewDateDisplayHeight;

	weekR = week->bounds;
	weekR.topLeft.x = week->days[0].bounds.topLeft.x;
	weekR.extent.x = week->days[0].bounds.extent.x * daysInWeek;
 	weekR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 	weekR.extent.y = week->hourHeight * weekViewNumDispHours;

	hourHeight = week->hourHeight;

	eventFrame.word = 0;
	eventFrame.bits.width = weekViewEventFrameWidth;

	newDayOfWeek = dayOfWeek;
	savedDayOfWeek = dayOfWeek;

	// Compute the bounds of the event.
	WeekViewCreateEventBar (&(week->days[dayOfWeek].bounds), &eventR);
	eventR.topLeft.y--;		//	Include the top & bottom of frame in bar
	eventR.extent.y += 2;
	WeekViewGetEventBounds (start, end, week->startHour, week->endHour, 
								   &eventR, hourHeight);
	// Highlight the event.
	RctInsetRectangle (&eventR, -weekViewEventFrameWidth);
	savedBits = WinSaveBits (&eventR, &error);
	RctInsetRectangle (&eventR, weekViewEventFrameWidth);
	WinDrawRectangleFrame (eventFrame.word, &eventR);

	// Draw the event description, time, and date.
	bitsBehindDesc = WeekViewDrawDescription (
									&apptRec.when->startTime,
									&apptRec.when->endTime,
									&date,
									apptRec.description,
									true);

	posInAppt = event->screenY - dayR.topLeft.y - 
					((start.hours - week->startHour) * hourHeight) -
					((start.minutes * hourHeight) / hoursInMinutes);		
	do 
		{
		PenGetPoint (&x, &y, &penDown);

		// The pen must be move pass a threshold before we will start
		// moving the event.  This threshold is greater than the distance
		// that would normally be traveled to move an event.
		if (! moving)
			{
			if (RctPtInRectangle (x, y, &moveR)) continue;
			moving = true;
			}
			
		// If we're moved outside the bound of the week, highlight to 
		// original event again.
		if (! RctPtInRectangle (x, y, &weekR))
			{
			if ((TimeToInt (start) == TimeToInt(apptRec.when->startTime)) && 
				 (DateToInt (date) == DateToInt(week->days[dayOfWeek].date)))
				continue;
			start = apptRec.when->startTime;
			end = apptRec.when->endTime;
			dayOfWeek = savedDayOfWeek;
			date = week->days[dayOfWeek].date;
			
			// Reset the day rect back to the original event so if the pen 
			// returns to the week, and that's a different day than the 
			// original, the difference will be noted and the date display 
			// updated.
			dayR = week->days[savedDayOfWeek].bounds;
		 	dayR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
		 	dayR.extent.y -= weekViewDayDisplayHeight + weekViewDateDisplayHeight;
			}
			
		// Have we moved the event to another day.
		else if (! RctPtInRectangle (x, y, &dayR))
			{
			for (newDayOfWeek = 0; newDayOfWeek < daysInWeek; newDayOfWeek++)
				{
				if (RctPtInRectangle (x, y, &week->days[newDayOfWeek].bounds))
					break;
				}
			dayR = week->days[newDayOfWeek].bounds;
		 	dayR.topLeft.y += weekViewDayDisplayHeight + weekViewDateDisplayHeight;
 			dayR.extent.y -= weekViewDayDisplayHeight + weekViewDateDisplayHeight;

			date = week->days[savedDayOfWeek].date;
			DateAdjust (&date, (Int32)newDayOfWeek - (Int32)savedDayOfWeek);
			}
		

		// Based on the position of the pen compute the time of the event.
		if (RctPtInRectangle (x, y, &weekR))
			{
			// Rounded the y coordinate to an even value,  this is done to
			// reduce the bounding of the selection highlight caused by 
			// digitizor.
			posInDay = week->startHour * hourHeight + 
				((y & (~1)) - dayR.topLeft.y - posInAppt);
			if (posInDay < 0) posInDay = 0;
			newStart.hours = (posInDay / hourHeight);
			newStart.minutes =  (2 * (posInDay % hourHeight) / 
				hourHeight) * (hoursInMinutes / 2);
			if (newStart.hours >= hoursPerDay)
				{
				newStart.hours = hoursPerDay - 1;
				newStart.minutes = hoursInMinutes - 5;
				}


			if (TimeToInt (start) == TimeToInt(newStart) && 
				 newDayOfWeek == dayOfWeek)
				continue;
			
			if (week->days[newDayOfWeek].date.day != outOfRangeDay)
				{
				dayOfWeek = newDayOfWeek;

				// The start time has changed so update the end time.
				start = newStart;
				end.hours = start.hours + timeDiff.hours;
				end.minutes = start.minutes + timeDiff.minutes;
				if (end.minutes >= hoursInMinutes)
					{
					end.hours++;
					end.minutes -= hoursInMinutes;
					}
				if (end.hours > maxHours)
					{
					end.hours = maxHours;
					end.minutes = maxMinutes;
					}
				}

			// We've move passed the last valid day (12/31/1931)
			else
				{
				if ((TimeToInt (start) == TimeToInt(apptRec.when->startTime)) && 
					 (DateToInt (date) == DateToInt(week->days[dayOfWeek].date)))
					continue;
				start = apptRec.when->startTime;
				end = apptRec.when->endTime;
				dayOfWeek = savedDayOfWeek;
				date = week->days[dayOfWeek].date;
				}
			}

		
		// Unhighlight the current selection.
		WinRestoreBits (savedBits, 
							 eventR.topLeft.x - weekViewEventFrameWidth,
							 eventR.topLeft.y - weekViewEventFrameWidth);
				
		WeekViewCreateEventBar (&(week->days[dayOfWeek].bounds), &eventR);
		eventR.topLeft.y--;		//	Include the top & bottom of frame in bar
		eventR.extent.y += 2;
		WeekViewGetEventBounds (start, end, week->startHour, week->endHour,
					                &eventR, hourHeight);
		// Highlight the event.
		RctInsetRectangle (&eventR, -weekViewEventFrameWidth);
		savedBits = WinSaveBits (&eventR, &error);
		RctInsetRectangle (&eventR, weekViewEventFrameWidth);
		WinDrawRectangleFrame (eventFrame.word, &eventR);
		
		// Draw the new date and time.
		WeekViewDrawDescription (&start, &end, &date, NULL, false);

		} while (penDown);

	WinRestoreBits (savedBits, 
						 eventR.topLeft.x - weekViewEventFrameWidth,
						 eventR.topLeft.y - weekViewEventFrameWidth);
						 


	// Check if the date or time of the appointment has been changed.
	sameTime = ((TimeToInt(start) == TimeToInt(apptRec.when->startTime)) &&
		         (TimeToInt(end) == TimeToInt(apptRec.when->endTime)));
	sameDate = (DateToInt(date) == DateToInt(week->days[savedDayOfWeek].date));

	MemHandleUnlock (recordH);
	

	// If the time hasn't been changed leave the description popup display
	// for a Int16 period of time.
	if (sameTime && sameDate) 
		{
		week->descTick = TimGetTicks () + displayTicks;
		week->behindDesc = bitsBehindDesc;
		TimeDisplayTick = week->descTick;
		TimeDisplayed = true;
		return;
		}

	WinRestoreBits (bitsBehindDesc, 0, 0);
	AttnIndicatorEnable(true);
	
	// If the date or time was changed move the appointment and redraw the view.
	// If only the time have changed then split the event into two event.
	MoveEvent (&recordNum, start, end, date, sameDate, &moved);
	if (moved)
		{
		ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
		MemHandleUnlock (recordH);

		if (apptRec.repeat)
			{
			for (i = 0; i < daysInWeek; i++)
				WeekViewEraseDay (week, i);
			}
		else
			{
			WeekViewEraseDay (week, dayOfWeek);
			WeekViewEraseDay (week, savedDayOfWeek);
			}
		WeekViewDraw (FrmGetActiveForm ());

		for (i = 0; i < daysInWeek; i++)
			WeekViewDayClose (&(week->days[i]));
		WeekViewInitDays (FrmGetActiveForm());
		WeekViewDrawDays ();
		WeekViewDrawDataMarks ();
		}
	}


/***********************************************************************
 *
 * FUNCTION:    	InvertDate
 *
 * DESCRIPTION:	This routine draws the selected Day/Date header with a
 *						selected appearance.
 *
 * PARAMETERS:		The bounds of the area to be highlighted
 *
 * RETURNED:    	nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			gap	11/16/00	Initial Revision
 *
 ***********************************************************************/
static void InvertDate (const RectangleType *r) 
{
	RectangleType		origClip;
	WeekPtr 				week;

	WinGetClip (&origClip);
	WinSetClip (r);

	WinPushDrawState();
	WinSetPatternType (blackPattern);
	WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
	
	// Erase the hilite bounds to fill it with selected fill color
	WinEraseRectangle(r, 0);
	
	// Draw the text.  As we have set the clip to the hilite rect
	// only the selected day/date will be drawn to the screen.
	week = MemHandleLock (WeekObjectH);
	WeekViewDayLabels (week);
	MemPtrUnlock (week);

	// Draw any data marks that may exist in the inverted area
	WeekViewDrawDataMarks();
	
	WinPopDrawState();

	WinSetClip (&origClip);
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewDayHandleSelection
 *
 * DESCRIPTION:	This routine is called when a pen winDown event occurs in
 *						a week object.  This routine will check to see if 
 *						the pen-winDown occurred within the passed day and track
 *						the pen until it is released.
 *
 * PARAMETERS:		DayType			*day
 *						EventType 		* event
 *						UInt16			startHour - current starting hour of event
 *											display.
 *						UInt16			endHour - current ending hour of event display
 *						UInt16			hourHeight - current vertical size of an hour
 *
 * RETURNED:    	Boolean			true if this day contained the passed
 *											event/point and handled it.  False if
 *											the event/point was outside this day.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	8/9/95	Initial Revision
 *			kcr	8/18/95	invert day/date area instead of framing
 *			kcr	10/13/95	don't handle points if day is out of date range.
 *			gap	10/31/00	don't allow the user to create an event in the 
 *								"empty" row that occasionally exists at the bottom
 *								of the week view grid
 *			gap	11/16/00	Use correct colors for day/date selection
 *								better fix to prevent event creation in "empty 
 *								row
 *
 ***********************************************************************/
static Boolean WeekViewDayHandleSelection (WeekType *week, Int16 dayOfWeek, 
	EventType* event)

	{
	DayType *			day;
	UInt16				startHour;
	UInt16				endHour;
	UInt16				hourHeight;
	ApptInfoPtr			appts = NULL;
	Boolean				apptSelected;
	Boolean				invertHighlight = true;
	FrameBitsType		eventFrame;
	int					i;
	Int16					x, y;
	TimeType				newStart, newEnd;
	UInt16				error;
	UInt16				recordNum = 0;
	WinHandle			savedBits;
	Boolean				newAppt = false;
	Boolean				penDown = true;
	Boolean				selected = true;
	RectangleType		pickR, saveR, highR;			//	pickR is the picking area,
																//	highR is the highlighted
																//		region
																//	saveR is the saved window
																//		(framed highlights only)

	day = &week->days[dayOfWeek];
	
	startHour = week->startHour;
	endHour = week->endHour;
	hourHeight = week->hourHeight;


	//	If this is the wrong day, give winUp
	if (! RctPtInRectangle (event->screenX, event->screenY, &(day->bounds)))
		return (false);
		
	//	If this day is out of the day range, and therefore inelligible for
	//	events, skip it:
	if (day->date.day == outOfRangeDay)
		return (true);
		
	//	Determine the part of the day that is selected:
	//	1.	Day/Date - go to day
	//	2.	single appt. - go to day, edit existing event
	//	3.	empty appt. slot - go to day edit new event

	//	NOTE: we are sometimes using different rectangles for picking &
	//	drawing highlights.  This is so that we can give the user as wide a
	//	margin for picking error as possible.
	
	apptSelected = false;
	Date = day->date;
	if (event->screenY <= (day->bounds.topLeft.y + weekViewDayDisplayHeight +
								  weekViewDateDisplayHeight))
		{	//	Day/Date is selected - highlight day/date
		pickR = day->bounds;
		pickR.extent.y = weekViewDayDisplayHeight + weekViewDateDisplayHeight;
		highR = pickR;
		highR.topLeft.x += weekViewDayHMargin-1;
		highR.extent.x -= 2*(weekViewDayHMargin-1);
		//	no saveR needed for this case
		}
	else
		{	//	Check through all appts
		if (day->numAppts > 0)
			appts = MemHandleLock (day->apptsH); 

		for (i = day->numAppts -1; i >= 0; i--)
			{
			//	Get the bounds of each appointment and check selected point
			//	against them.  We traverse the list from bottom to top
			//	so that we will find the event with the latest start time
			//	in the event of event overlap.
		 	WeekViewCreateEventBar (&(day->bounds), &highR);
			highR.topLeft.y--;		//	Include the top & bottom of frame in bar
			highR.extent.y += 2;
			WeekViewGetEventBounds (appts[i].startTime, appts[i].endTime,
											startHour, endHour,
											&highR, hourHeight);
			pickR = highR;
			pickR.topLeft.x = day->bounds.topLeft.x;
			pickR.extent.x = day->bounds.extent.x;
			pickR.topLeft.y--;
			pickR.extent.y += 2;			//	Include top & bottom of frame in pick
												//	area.
			if (RctPtInRectangle (event->screenX, event->screenY, &pickR))
				{
				apptSelected = true;
				recordNum = appts[i].recordNum;
				break;
				}
			}
		if (i < 0)
			{	
			//	New appt has been selected.  Convert y-coord of pt to
			//	time of new appt & visible region of new appt.
			newStart.hours = (event->screenY -
									day->bounds.topLeft.y -
									weekViewDayDisplayHeight -
									weekViewDateDisplayHeight) / hourHeight;
			newStart.hours += startHour;
			newStart.minutes = 0;
			
			newEnd = newStart;
			newEnd.hours++;
			
			// There are occasions, depending on the start/end time settings in 
			// preferences, as well as the times of existing events that there will 
			// be an "empty" row  at the bottom of the week view grid.  Be sure 
			// that the user is not allowed to create an appointment in this area.
			// Times for new events should always have a start hour between 0 and 23.
			if (newStart.hours < hoursPerDay)
				{
			 	WeekViewCreateEventBar (&(day->bounds), &highR);
				highR.topLeft.y--;		//	Include the top & bottom of frame in bar
				highR.extent.y += 2;
				WeekViewGetEventBounds (newStart, newEnd,
												startHour, endHour,
												&highR, hourHeight);
				pickR = highR;
				pickR.topLeft.x = day->bounds.topLeft.x;
				pickR.extent.x = day->bounds.extent.x;
				pickR.topLeft.y--;
				pickR.extent.y += 2;			//	Include top & bottom of frame in pick
													//	area.
				
				newAppt = true;
				}
			else
				{
				// just clean up & return true now as the user has tapped in
				// an "invalid' time & there is nothing to track
				if (day->numAppts > 0)
					MemPtrUnlock(appts);
				return (true);
				}
			}

		if (day->numAppts > 0)
			MemPtrUnlock(appts);
		
		invertHighlight = false;
		}	//	end of else <pt in event bar>

	// Draw the date, time and description of the apointment.
	if (apptSelected)
		{
		UInt16 			attr;
		
		//Mask if appropriate
		DmRecordInfo (ApptDB, recordNum, &attr, NULL, NULL);
   	if (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))	
			{
			//masked
			
#if UNMASK_PRIVATE_RECORDS_IN_WEEK_VIEW
			if (SecVerifyPW (showPrivateRecords) == true)
				{
				//DOLATER - peter: The following code is added for testing.
				//						 It won't be needed once the new security APIs are used.
				//						 It resets the security to mask private records.
				//						 This may be optional.
				PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

				// Only change the visual status of this record, leaving all others masked.
				CurrentRecordVisualStatus = showPrivateRecords;

				WeekViewHideTime (true);
				WeekViewSelectEvent (week, dayOfWeek, event, recordNum, weekDescUnmaskedDisplayTicks);
				}
			return (true);
#else
			WeekViewHideTime (true);
			WeekViewSelectEvent (week, dayOfWeek, event, recordNum, weekDescDisplayTicks);
			return (true);
#endif

			}
		else
			{
		
			WeekViewHideTime (true);
			WeekViewSelectEvent (week, dayOfWeek, event, recordNum, weekDescDisplayTicks);
			return (true);
			}
		}


	//	Handle all of the fancy dragging & highlighting:
	//	If invertHighlight, then highlight the selection by inverting the
	//	rectangle; else, highlight by drawing a frame.
	saveR = highR;
	if (!invertHighlight)
		{
		eventFrame.word = 0;
		eventFrame.bits.width = weekViewEventFrameWidth;
		//	Expand the drawR to include the frame:
		saveR.topLeft.x -= weekViewEventFrameWidth;
		saveR.topLeft.y -= weekViewEventFrameWidth;
		saveR.extent.x += 2*weekViewEventFrameWidth;
		saveR.extent.y += 2*weekViewEventFrameWidth;

		savedBits = WinSaveBits (&saveR, &error);
		WinDrawRectangleFrame (eventFrame.word, &highR);
		}
	else	//	inverting
		{
		savedBits = WinSaveBits (&saveR, &error);
		InvertDate(&saveR);
		}


	do 
		{
		PenGetPoint (&x, &y, &penDown);

		if (RctPtInRectangle (x, y, &pickR))
			{
			if (! selected)
				{
				savedBits = WinSaveBits (&saveR, &error);
				if (! invertHighlight)
					WinDrawRectangleFrame (eventFrame.word, &highR);
				else
					InvertDate(&saveR);
				selected = true;
				}
			}

		else if (selected)
			{
			WinRestoreBits (savedBits, saveR.topLeft.x, saveR.topLeft.y);				
			selected = false;
			}

		} while (penDown);
		

	if (selected)
		{
		WinRestoreBits (savedBits, saveR.topLeft.x, saveR.topLeft.y);
		
		if (newAppt)
			WeekViewNewAppointment (&newStart, &newEnd);
		FrmGotoForm (DayView);
		}

	return (true);
	}	//	end of WeekViewDayHandleSelection
	

/***********************************************************************
 *
 * FUNCTION:		WeekViewGotoDate
 *
 * DESCRIPTION:	This routine dispay the date picker so that the 
 *						user can select a week to navigate to.  If the date
 *						picker is confirmed, the week containing the
 *						day selected is displayed.
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
 *			kcr	9/19/95	Initial Revision
 *			kcr	9/19/95	navigates to a week instead of a day.
 *			kcr	10/5/95	preserve font around calls to DatePicker
 *
 ***********************************************************************/
static void WeekViewGotoDate (void)
	{
	Char * title;
	FontID	origFont;
	MemHandle titleH;
	Int16 month, day, year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// Display the date picker.
	origFont = FntGetFont ();
	if (SelectDay (selectDayByWeek, &month, &day, &year, title))
		{
		FntSetFont (origFont);
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;

		WeekViewGoToWeek ();
		}
	else
		FntSetFont (origFont);
	MemHandleUnlock (titleH);
	}


#if 0
/***********************************************************************
 *
 * FUNCTION:    WeekViewGotoToday
 *
 * DESCRIPTION: This routine navigates to today in the day view.
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
 *			art	9/15/95	Initial Revision
 *
 ***********************************************************************/
static void WeekViewGotoToday (void)
{
	DateTimeType dateTime;
	
	// Get today's date.
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	Date.year = dateTime.year - firstYear;
	Date.month = dateTime.month;
	Date.day = dateTime.day;

	FrmGotoForm (DayView);
}
#endif


/***********************************************************************
 *
 * FUNCTION:    WeekViewDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/10/95	Initial Revision
 *
 ***********************************************************************/
static void WeekViewDoCommand (UInt16 command)
{

	switch (command)
		{
		case WeekGetInfoCmd:
			AbtShowAbout (sysFileCDatebook);
			break;
						
		case WeekPreferencesCmd:			
			FrmPopupForm (PreferencesDialog);
			break;
			
		case WeekSecurityCmd:			
			DoSecurity();
			WeekViewUpdateDisplay(updateDisplayOptsChanged);
			break;
			
		}
}


/***********************************************************************
 *
 * FUNCTION:    WeekViewHandleSelection
 *
 * DESCRIPTION: This routine is called when a pen winDown event occurs in
 *              a week object.  This routine will track
 *              the pen until it is released.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kcr	8/9/95	Initial Revision
 *
 ***********************************************************************/
static Boolean WeekViewHandleSelection (EventType* event)
	{
	int					i;
 	WeekType				*week;
		
	week = MemHandleLock (WeekObjectH);
	
	if (!RctPtInRectangle (event->screenX, event->screenY, &(week->bounds)))
		{
		MemPtrUnlock (week);
		return (false);
		}
		
	//	Let each day have a crack at the event until one handles it
	for (i = 0; i < daysInWeek; i++)
		{
		if (WeekViewDayHandleSelection (week, i, event))
			break;
		}
			
 	MemPtrUnlock (week);
	return (true);
}


/***********************************************************************
 *
 * FUNCTION:		WeekViewUpdateDisplay
 *
 * DESCRIPTION:	Update the Week View display according to the passed
 *						updateCode.
 *
 * PARAMETERS:		UInt16				updateCode - indicates which changes have
 *												been made to the view.
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			kcr	10/9/95		initial version
 *
 ***********************************************************************/
static void WeekViewUpdateDisplay (UInt16 updateCode)
	{
	if (updateCode & updateDisplayOptsChanged)
		{	//	Start & end times have changed
		WeekViewGoToWeek ();
		}
		
	else if (updateCode & frmRedrawUpdateCode)
		{
		FrmDrawForm (FrmGetActiveForm ());
		WeekViewDraw (FrmGetActiveForm ());
		WeekViewDrawDays ();
		WeekViewDrawDataMarks ();
		}

	}	//	end of WeekViewUpdateDisplay


/***********************************************************************
 *
 * FUNCTION:    WeekViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Week View"
 *              of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	7/24/95		Initial Revision
 *			kcr	8/21/95		added prev & next week buttons
 *			kcr	9/19/95		added handler for 'Day' button
 *			kcr	10/2/95		changed prev & next week buttons to repeat.
 *			grant	2/2/99		remove frmTitleSelectEvent handler
 *			CSS	06/22/99		Standardized keyDownEvent handling
 *									(TxtCharIsHardKey, commandKeyMask, etc.)
 *			rbb	6/2/99		Added button for Agenda view.
 *			gap	9/29/00		Ignore the TitleChanged event when the event
 *									description is open to prevent attention manager
 *									indicator from drawing over it.
 *
 ***********************************************************************/
Boolean WeekViewHandleEvent (EventType* event)
	{
	FormPtr frm;
	DateType date;
	Boolean handled = false;

	#if WRISTPDA

	extern UInt32  DebounceEnterKeyEndTime;

	extern UInt16  NewSndSysAmp, OldSndSysAmp;
	extern UInt16  NewSndDefAmp, OldSndDefAmp;

	extern UInt16  NewKeyInitDelay, OldKeyInitDelay;
	extern UInt16  NewKeyPeriod, OldKeyPeriod;
	extern UInt16  NewKeyDoubleTapDelay,  OldKeyDoubleTapDelay;
	extern Boolean NewKeyQueueAhead, OldKeyQueueAhead;

	#endif

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case WeekGoToButton:
				WeekViewGotoDate ();
				handled = true;
				break;

			case WeekDayViewButton:
				//	'Date' var should already be set.
				//	This can only happen when navigating from week to week,
				//	with prev/next arrows, or when navigating with the date
				//	picker.
				FrmGotoForm (DayView);
				handled = true;
				break;

			case WeekMonthViewButton:
				//	'Date' var should already be set.
				//	This can only happen when navigating from week to week,
				//	with prev/next arrows, or when navigating with the date
				//	picker.
				FrmGotoForm (MonthView);
				handled = true;
				break;

			case WeekAgendaViewButton:
				//	'Date' var should already be set.
				//	This can only happen when navigating from week to week,
				//	with prev/next arrows, or when navigating with the date
				//	picker.
				FrmGotoForm (AgendaView);
				handled = true;
				break;

			}
		}

	// Handle the repeating scroll controls.  NOTE: it
	//	is important not to 'handle' repeating controls so
	//	that the next event will be generated correctly.
	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case WeekUpButton:
				WeekViewScroll (winUp, false);
				break;
				
			case WeekDownButton:
				WeekViewScroll (winDown, false);
				break;

			case WeekPrevButton:
				DateAdjust (&Date, -daysInWeek);
				WeekViewGoToWeek ();
				break;
			
			case WeekNextButton:
				DateAdjust (&Date, daysInWeek);
				WeekViewGoToWeek ();
				break;

			}
		}


	else if (event->eType == penDownEvent)
		{
		handled = WeekViewPointInDescription (event->screenX, event->screenY);
		WeekEraseDescription (true);
		if (! handled)
			handled = WeekViewHandleSelection (event);
		EventInCurrentView = true;
		}


	else if (event->eType == keyDownEvent)
		{

		#if WRISTPDA
			EventType newEvent;

			frm = FrmGetActiveForm ();
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
				// Translate the Enter key to a WeekMonthViewButton event.
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.controlID = WeekMonthViewButton;
				newEvent.data.ctlSelect.pControl =
					FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
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
				// Translate the RockerUp key to a WeekPrevButton event.
				newEvent.eType = ctlRepeatEvent;
				newEvent.data.ctlRepeat.controlID = WeekPrevButton;
				newEvent.data.ctlRepeat.pControl =
					FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
				// Translate the RockerDown key to a WeekNextButton event.
				newEvent.eType = ctlRepeatEvent;
				newEvent.data.ctlRepeat.controlID = WeekNextButton;
				newEvent.data.ctlRepeat.pControl =
					FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
				EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
				return true;
			}
		#endif

		WeekEraseDescription (true);
		WeekViewHideTime (true);

		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{	
			DateSecondsToDate (TimGetSeconds (), &date);
			Date = date;
			if (EventInCurrentView ||
				(event->data.keyDown.modifiers & poweredOnKeyMask))
				FrmGotoForm (DayView);
			else
				FrmGotoForm (MonthView);				
			handled = true;
			EventInCurrentView = true;
			}
		else if (EvtKeydownIsVirtual(event))
			{
			if (event->data.keyDown.chr == vchrPageUp)
				{
				#if WRISTPDA
				// Same as WeekUpButton...
				WeekViewScroll (winUp, false);
				#else
				DateAdjust (&Date, -daysInWeek);
				WeekViewGoToWeek ();
				#endif
				handled = true;
				}	
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				#if WRISTPDA
				// Same as WeekDownButton...
				WeekViewScroll (winDown, false);
				#else
				DateAdjust (&Date, daysInWeek);
				WeekViewGoToWeek ();
				#endif
				handled = true;
				}
			
			EventInCurrentView = true;
			}
		}

	else if (event->eType == menuEvent)
		{
		WeekViewDoCommand (event->data.menu.itemID);
		return (true);
		}


	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		//OrigFont = FntSetFont (FossilStdFont);
		OrigFont = FntSetFont (stdFont);
		WeekViewInit (frm);
		FrmDrawForm (frm);
		WeekViewDraw (frm);
		
		// Note:  WeekViewInitDays(), unlike WeekViewInit(), relies
		// on the fact that FrmDrawForm() has already been called.
		WeekViewInitDays (frm);
		WeekViewDrawDays ();
		WeekViewDrawDataMarks();		
		EventInCurrentView = false;
		handled = true;
		}

	else if (event->eType == frmUpdateEvent)
		{
		WeekViewUpdateDisplay (event->data.frmUpdate.updateCode);
		handled = true;
		}

	else if (event->eType == frmCloseEvent)
		{
		WeekViewFree ();
		FntSetFont (OrigFont);
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
				WeekViewHideTime (true);
			else
				WeekViewShowTime ();
			}
		}
	else if (event->eType == nilEvent)
		{
		// Do WeekEraseDescription first - these both depend on the global TimeDisplayed,
		// but WeekEraseDescription also checks week view internal data before doing anything.
		WeekEraseDescription (false);
		WeekViewHideTime (false);
		}

	return (handled);
	}	//	end of WeekViewHandleEvent
