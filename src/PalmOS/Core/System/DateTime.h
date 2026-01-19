/******************************************************************************
 *
 * Copyright (c) 1995-2003 PalmSource, Inc. All rights reserved.
 *
 * File: DateTime.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *		Date and Time calculations
 *
 *****************************************************************************/

#ifndef __DATETIME_H__
#define	__DATETIME_H__


#include <CoreTraps.h>
#include <LocaleMgr.h>		// LmLocaleType


typedef enum 
	{
	tfColon,
	tfColonAMPM,			// 1:00 pm
	tfColon24h,				// 13:00
	tfDot,
	tfDotAMPM,				// 1.00 pm
	tfDot24h,				// 13.00
	tfHoursAMPM,			// 1 pm
	tfHours24h,				// 13
	tfComma24h				// 13,00
	} TimeFormatType;


typedef enum {
	dsNone,					// Daylight Savings Time not observed
	dsUSA,					// United States Daylight Savings Time
	dsAustralia,			// Australian Daylight Savings Time
	dsWesternEuropean,	// Western European Daylight Savings Time
	dsMiddleEuropean,		// Middle European Daylight Savings Time
	dsEasternEuropean,	// Eastern European Daylight Savings Time
	dsGreatBritain,		// Great Britain and Eire Daylight Savings Time
	dsRumania,				// Rumanian Daylight Savings Time
	dsTurkey,				// Turkish Daylight Savings Time
	dsAustraliaShifted	// Australian Daylight Savings Time with shift in 1986
	} DaylightSavingsTypes;


// pass a TimeFormatType	
#define Use24HourFormat(t) ((t) == tfColon24h || (t) == tfDot24h || (t) == tfHours24h || (t) == tfComma24h)
#define TimeSeparator(t) ((Char) ( t <= tfColon24h ? ':' : (t <= tfDot24h ? '.' : ',')))


typedef enum {
	dfMDYWithSlashes,		// 12/31/95
	dfDMYWithSlashes,		// 31/12/95
	dfDMYWithDots,			// 31.12.95
	dfDMYWithDashes,		// 31-12-95
	dfYMDWithSlashes,		// 95/12/31
	dfYMDWithDots,			// 95.12.31
	dfYMDWithDashes,		// 95-12-31

	dfMDYLongWithComma,	// Dec 31, 1995
	dfDMYLong,				// 31 Dec 1995
	dfDMYLongWithDot,		// 31. Dec 1995
	dfDMYLongNoDay,		// Dec 1995
	dfDMYLongWithComma,	//	31 Dec, 1995
	dfYMDLongWithDot,		//	1995.12.31
	dfYMDLongWithSpace,	//	1995 Dec 31

	dfMYMed,					//	Dec '95
	dfMYMedNoPost,			//	Dec 95		(added for French 2.0 ROM)
	dfMDYWithDashes		// 12-31-95		(added for 4.0 ROM)
	} DateFormatType;

typedef struct {
	Int16 second;
	Int16 minute;
	Int16 hour;
	Int16 day;
	Int16 month;
	Int16 year;
	Int16 weekDay;		// Days since Sunday (0 to 6)
	} DateTimeType;
	
typedef DateTimeType *DateTimePtr;


// This is the time format.  Times are treated as words so don't 
// change the order of the members in this structure.
//
typedef struct {
#ifdef PALMOS
	UInt8        hours;
	UInt8        minutes;
#else
	UInt8        minutes;
	UInt8        hours;
#endif
} TimeType;

typedef TimeType *TimePtr;

#define noTime	-1		// The entire TimeType is -1 if there isn't a time.


// This is the date format.  Dates are treated as words so don't 
// change the order of the members in this structure.
//
typedef struct {
#ifdef PALMOS
	UInt16 year  :7;                   // years since 1904 (MAC format)
	UInt16 month :4; 
	UInt16 day   :5;
#else
	UInt16 day   :5;
	UInt16 month :4; 
	UInt16 year  :7;                   // years since 1904 (MAC format)
#endif
} DateType;

typedef DateType *DatePtr;


/************************************************************
 * Date Time Constants
 *************************************************************/

// Maximum lengths of strings return by the date and time formating
// routine DateToAscii and TimeToAscii.
#define timeStringLength		9
#define dateStringLength		9
#define longDateStrLength		15
#define dowDateStringLength	19
#define dowLongDateStrLength	25
#define timeZoneStringLength	50


#define firstYear				1904
#define numberOfYears		128
#define lastYear				(firstYear + numberOfYears - 1)



// Constants for time calculations
// Could change these from xIny to yPerX
#define secondsInSeconds	1
#define minutesInSeconds	60
#define hoursInMinutes		60
#define hoursInSeconds		(hoursInMinutes * minutesInSeconds)
#define hoursPerDay			24
//#define daysInSeconds		((Int32)(hoursPerDay) * ((Int32)hoursInSeconds))
#define daysInSeconds		(0x15180)	// cc bug

#define daysInWeek			7
#define daysInYear			365
#define daysInLeapYear		366
#define daysInFourYears		(daysInLeapYear + 3 * daysInYear)

#define monthsInYear			12

#define maxDays				((UInt32) numberOfYears / 4 * daysInFourYears - 1)
#define maxSeconds			((UInt32) (maxDays + 1) * daysInSeconds - 1)

// Values returned by DayOfWeek routine.
#define sunday					0
#define monday					1
#define tuesday				2
#define wednesday				3
#define thursday				4
#define friday					5
#define saturday				6

// Months of the year
#define january				1
#define february				2
#define march					3
#define april					4
#define may						5
#define june					6
#define july					7
#define august					8
#define september				9
#define october				10
#define november				11
#define december				12

// It would have been cool to have a real DayOfWeekType, but we #define the
// following for compatibility with existing code.  Please use the new name
// (DayOfMonthType).
#define DayOfWeekType DayOfMonthType

// Values returned by DayOfMonth routine.
typedef enum {
	dom1stSun, dom1stMon, dom1stTue, dom1stWen, dom1stThu, dom1stFri, dom1stSat,
	dom2ndSun, dom2ndMon, dom2ndTue, dom2ndWen, dom2ndThu, dom2ndFri, dom2ndSat,
	dom3rdSun, dom3rdMon, dom3rdTue, dom3rdWen, dom3rdThu, dom3rdFri, dom3rdSat,
	dom4thSun, dom4thMon, dom4thTue, dom4thWen, dom4thThu, dom4thFri, dom4thSat,
	domLastSun, domLastMon, domLastTue, domLastWen, domLastThu, domLastFri, 
	domLastSat
	} DayOfMonthType;

// Values used by DateTemplateToAscii routine.
#define dateTemplateChar					chrCircumflexAccent

enum {
	dateTemplateDayNum = '0',
	dateTemplateDOWName,
	dateTemplateMonthName,
	dateTemplateMonthNum,
	dateTemplateYearNum
};

#define	dateTemplateShortModifier		's'
#define	dateTemplateRegularModifier	'r'
#define	dateTemplateLongModifier		'l'
#define	dateTemplateLeadZeroModifier	'z'

//************************************************************
//* Date and Time macros
//***********************************************************

// Convert a date in a DateType structure to an UInt16.
#ifdef PALMOS
#define DateToInt(date) (*(UInt16 *) &date)
#else
#define DateToInt(date) ((UInt16)(((date).year << 9) | ((date).month << 5) | (date).day))
#define IntToDate(date, i) do { UInt16 d = i; date.day = d & 0x1F; date.month = (d >> 5) & 0x0F; date.year = (d >> 9) & 0x7F; } while (0)
#endif
 
 
// Convert a date in a DateType structure to a signed int.
#ifdef PALMOS
#define TimeToInt(time) (*(Int16 *) &time)
#else
#define TimeToInt(time) ((UInt16)(((time).hours << 8) | (time).minutes))
#endif
 
 

//************************************************************
//* Date Time procedures
//************************************************************


#ifdef __cplusplus
extern "C" {
#endif


void TimSecondsToDateTime(UInt32 seconds, DateTimeType *dateTimeP)
			SYS_TRAP(sysTrapTimSecondsToDateTime);

UInt32 TimDateTimeToSeconds(const DateTimeType *dateTimeP)
			SYS_TRAP(sysTrapTimDateTimeToSeconds);

void TimAdjust(DateTimeType *dateTimeP, Int32 adjustment)
			SYS_TRAP(sysTrapTimAdjust);

void TimeToAscii(UInt8 hours, UInt8 minutes, TimeFormatType timeFormat, 
				Char *pString)
			SYS_TRAP(sysTrapTimeToAscii);

UInt32 TimTimeZoneToUTC (UInt32 seconds, Int16 timeZone,
				Int16 daylightSavingAdjustment)
			SYS_TRAP(sysTrapTimTimeZoneToUTC);

UInt32 TimUTCToTimeZone (UInt32 seconds, Int16 timeZone,
				Int16 daylightSavingAdjustment)
			SYS_TRAP(sysTrapTimUTCToTimeZone);

void TimeZoneToAscii(Int16 timeZone, const LmLocaleType *localeP, Char *string)
			SYS_TRAP(sysTrapTimeZoneToAscii);


Int16 DaysInMonth(Int16 month, Int16 year)
			SYS_TRAP(sysTrapDaysInMonth);

Int16 DayOfWeek (Int16 month, Int16 day, Int16 year)
			SYS_TRAP(sysTrapDayOfWeek);

Int16 DayOfMonth (Int16 month, Int16 day, Int16 year)
			SYS_TRAP(sysTrapDayOfMonth);



// Date routines.
void DateSecondsToDate (UInt32 seconds, DateType *dateP)
			SYS_TRAP(sysTrapDateSecondsToDate);

void DateDaysToDate (UInt32 days, DateType *dateP)
			SYS_TRAP(sysTrapDateDaysToDate);

UInt32 DateToDays (DateType date)
			SYS_TRAP(sysTrapDateToDays);

void DateAdjust (DateType *dateP, Int32 adjustment)
			SYS_TRAP(sysTrapDateAdjust);

void DateToAscii(UInt8 months, UInt8 days, UInt16 years, 
				DateFormatType dateFormat, Char *pString)
			SYS_TRAP(sysTrapDateToAscii);

void DateToDOWDMFormat(UInt8 months, UInt8 days, UInt16 years,
				DateFormatType dateFormat, Char *pString)
			SYS_TRAP(sysTrapDateToDOWDMFormat);

UInt16 DateTemplateToAscii(const Char *templateP, UInt8 months,
				UInt8 days, UInt16 years, Char *stringP, Int16 stringLen)
			SYS_TRAP(sysTrapDateTemplateToAscii);


#ifdef __cplusplus 
}
#endif


#endif //__DATETIME_H__
