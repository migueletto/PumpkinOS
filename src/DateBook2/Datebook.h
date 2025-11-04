/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Datebook.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This file defines the Datebook's Main modual's functions anf globals.
 *
 * History:
 *		August 10, 1995	Created by Art Lamb
 *		09/09/97	frigino	Moved DatebookPreferenceType into here
 *		04/22/99	rbb	Added support for snooze feature
 *		08/03/99	kwk	Deleted defaultApptDescFont & defaultNoteFont.
 *		09/15/99	gap	Changed names in MoveEvent parameters to match new implementation.
 *		11/04/99	jmp	Added InPhoneLookup.
 *		07/28/00	gap	Added preliminary attention manager support.
 *
 *****************************************************************************/

#include <IMCUtils.h>
#include <ExgMgr.h>

#include "DateDB.h"
#include "DateAlarm.h"
#include "DateDisplay.h"
#include "DatePref.h"
#include "DateDay.h"
#include "DateWeek.h"
#include "DateMonth.h"
#include "DateAgenda.h"
#include "DateTransfer.h"
#include "DatebookRsc.h"


#define DATEBOOK_DEBUG_NONE			0
#define DATEBOOK_DEBUG_FULL			1

#define DATEBOOK_DEBUG_LEVEL			DATEBOOK_DEBUG_NONE



/***********************************************************************
 *
 *	Constants
 *
 ***********************************************************************/
#define noRecordSelected				0xffff
#define maxDescSize						256

// Length of verious  
#define maxFrequenceFieldLen			3
#define defaultRepeatDecsLen			32
#define maxAdvanceFieldLen				3


#define datebookDBType					'DATA'

// Error conditions
#define datebookErrDuplicateAlarm	1		// Used only with AddPendingAlarm
#define datebookErrAlarmListFull		2		// Used only with AddPendingAlarm

// Feature numbers used by the datebook
#define alarmPendingListFeature				0		// List of pending alarms
#define alarmSnoozeTimeFeature				1
#define alarmPreviouslyDismissedFeature	2
#define recentFormFeature						3

#define datebookVersionNum						3
#define datebookPrefsVersionNum				4
#define datebookPrefID							0x00
#define datebookUnsavedPrefsVersionNum		2		// version 2 represents reformatted structure for attn mgr support
#define datebookUnsavedPrefID					0x00
#define datebookDBName							"DatebookDB"

#define dateExtension							"vcs"
#define dateMIMEType								"text/x-vCalendar"

// Column in the to do table on the day view.
#define timeBarColumn					0
#define timeColumn						1
#define descColumn						2

#define spaceAfterTimeBarColumn		0
#define spaceAfterTimeColumn			2

#define newEventSize  					16
#define maxNoteTitleLen					40

// Update codes, used to determine how the to do day view should 
// be redrawn.
#define updateRedrawAll					0x00
#define updateItemDelete				0x01
#define updateItemHide					0x02
#define updateDateChanged				0x04
#define updateTimeChanged				0x08
#define updateException					0x10
#define updateRepeatChanged			0x20
#define updateDisplayOptsChanged		0x40
#define updateAlarmChanged				0x80
#define updateFontChanged				0x100
#define updateCategoryChanged			0x200
#define updateGoTo						0x400
#define updateItemMove					0x800

// Fonts used by application
#define apptTimeFont 					stdFont
#define apptEmptyDescFont				stdFont
#define noteTitleFont					boldFont


// Token strings in repeating event description template.
#define frequenceToken					"^f"
#define dayOrdinalToken					"^x"
#define weekOrdinalToken				"^w"
#define monthNameToken					"^m"
#define dayNameToken						"^d"

// Default setting for the Details Dialog.
#define defaultAlarmAdvance			5
#define defaultAdvanceUnit				aauMinutes

// Default setting for the Repeating Events Dialog.
#define defaultRepeatFrequency		1
#define defaultRepeatEndDate			-1

// Repeat dialog "end on" popup list items.
#define repeatNoEndDateItem			0
#define repeatChooseDateItem			1

// End Date popup list chooses
#define noEndDateItem					0
#define selectEndDateItem				1

#define emptySlot							0xffff

// Field numbers used the id where search string was found
#define descSeacrchFieldNum			0
#define noteSeacrchFieldNum			1

// Preference Dialog.
#if WRISTPDA
#define dayRangeTimeWidth				55 // WRISTPDA Original value: 50
#define dayRangeTimeHeight				15 // WRISTPDA Original value: 13
#else
#define dayRangeTimeWidth				50
#define dayRangeTimeHeight				13
#endif

// Latest time
#define maxHours							23
#define maxMinutes						55

// Time bars
#define maxTimeBarColumns				5
#define timeBarWidth						2

// The duration to display the current time when the title of the 
// Day View is pressed. 
#define timeDisplayTicks 			((sysTicksPerSecond * 3) / 2)		// 1 1/2 seconds
#define timeDisplayWaitTicks 		(sysTicksPerSecond / 20)		// 1/20 second



// Default databook app preference values
#define defaultDayStartHour					(8)
#define defaultDayEndHour						(18)
#define defaultAlarmPresetAdvance			(apptNoAlarm)
#define defaultAlarmPresetUnit				(aauMinutes)
#define defaultSaveBackup						(true)
#define defaultShowTimeBars					(true)
#define defaultCompressDayView				(true)
#define defaultShowTimedAppts					(true)
#define defaultShowUntimedAppts				(false)
#define defaultShowDailyRepeatingAppts		(false)
#define defaultAlarmSoundRepeatCount		(3)
#define defaultAlarmSoundRepeatInterval	(300)
#define defaultAlarmSoundUniqueRecID		(0)
#define defaultAlarmSnooze						(300)
#define defaultRecentForm						(DayView)


/***********************************************************************
 *
 *	Debug Macros
 *
 ***********************************************************************/
#if (EMULATION_LEVEL != EMULATION_NONE)
	#ifndef Assert
		#define Assert(EX) ( (EX) ? (void) 0 : ErrDisplayFileLineMsg(__FILE__,__LINE__,#EX))
	#endif
#else
	#define Assert(EX)
#endif


/***********************************************************************
 *
 *	Datebook prefs structure
 *
 ***********************************************************************/

// This is the structure of the data that's saved to the state file.
typedef struct {
	UInt16				dayStartHour;
	UInt16				dayEndHour;
	AlarmInfoType		alarmPreset;
	FontID				v20NoteFont;		// Changed for 2.0 compatibility (BGT)
	Boolean				saveBackup;
	Boolean				showTimeBars;
	Boolean				compressDayView;
	Boolean				showTimedAppts;
	Boolean				showUntimedAppts;
	Boolean				showDailyRepeatingAppts;
	UInt8					reserved;
	
	// Version 3 preferences
	UInt16 				alarmSoundRepeatCount;
	UInt16 				alarmSoundRepeatInterval;
	UInt32				alarmSoundUniqueRecID;
	FontID				apptDescFont;
	FontID				noteFont;		// Changed for 2.0 compatibility (BGT)
	
	// Version 4 preferences
	UInt16					alarmSnooze;
} DatebookPreferenceType;


typedef enum {
	appLaunchCmdAlarmEventGoto = sysAppLaunchCmdCustomBase
} DateBookCustomLaunchCodes;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
extern 	DmOpenRef			ApptDB;						// datebook database

extern	DateType				Date;							// date currently displayed
extern	UInt16				StartDayOfWeek;
extern	UInt16				DayStartHour;				// start of the day 8:00am
extern	UInt16				DayEndHour;					// end of the day 6:00pm

extern	TimeFormatType		TimeFormat;
extern	DateFormatType		LongDateFormat;			// system preference
extern 	DateFormatType		ShortDateFormat;
extern 	UInt16				CurrentRecord;				// record being edited
extern	Boolean				ItemSelected;				// true if a day view item is selected
extern	UInt16				DayEditPosition;			// position of the insertion point in the desc field
extern 	RGBColorType	 	colorLine;					// Color to draw week and month view lines in

#if 0		// moved to DatePref.c	vmk 12/9/97
	// The following global variable are only valid while editng the datebook's
	// preferences.
	extern	UInt16					PrefDayStartHour;
	extern	UInt16					PrefDayEndHour;
#endif

// The following global variable are saved to a state file.
extern 	UInt16				DayStartHour;				// start of the day 8:00am
extern 	UInt16				DayEndHour;					// end of the day 6:00pm
extern  	FontID				NoteFont;					// font used in note view
extern  	AlarmInfoType		AlarmPreset;				// default alarm settings.
extern	Boolean				SaveBackup;					// default setting "Backuo tp PC" checkbox
extern	Boolean				ShowTimeBars;				// show time bars in the day view
extern	Boolean				CompressDayView;			// remove empty time slot to prevent scrolling
extern	Boolean				ShowTimedAppts;			// show timed appointments in month view
extern	Boolean				ShowUntimedAppts;			// show untimed appointments in month view
extern	Boolean				ShowDailyRepeatingAppts;// show daily repeating appointments in month view

extern	Boolean				EventInCurrentView;		// true if pen or key event has occurred in the
																	// current view (used only by Week and Month views)
extern	FontID				ApptDescFont;				// font for drawing event description


// The following global variable is used to control the displaying of the
// current time in the title of a view.
extern	Boolean				TimeDisplayed;				// True if time in been displayed
extern	UInt32				TimeDisplayTick;			// Tick count when we stop showing time


// The following globals are for the repeat rates of the alarms.
extern	UInt16				AlarmSoundRepeatCount;	// number of times to repeat alarm sound 
extern	UInt16				AlarmSoundRepeatInterval;// interval between repeat sounds, in seconds

extern	UInt32				AlarmSoundUniqueRecID;	// Unique record ID of desired alarm sound

extern	UInt16				AlarmSnooze;			// snooze duration, in seconds

extern	Boolean				InPhoneLookup;			// true if we've called PhoneNumberLookup()

/***********************************************************************
 *
 *	Functions
 *
 ***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif


#if WRISTPDA
Boolean CheckKeyIsPressed( UInt32 KeyMask );
UInt32 CheckKeyDuration( UInt32 KeyMask, UInt32 MaxDuration );
#endif

extern void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp);

extern void GoToAlarmItem (UInt32 uniqueID);

extern void* GetObjectPtr (UInt16 objectID);

extern void DrawTime (
	TimeType							inTime,
	TimeFormatType					inFormat,
	FontID							inFont,
	JustificationType				inJustification,
	RectangleType*					inBounds );

extern void ShowInfo (void);

extern void DirtyRecord (DmOpenRef dbP, UInt16 index);

extern  Err MoveEvent (UInt16* recordNumP, TimeType startTime, TimeType endTime,
 		DateType startDate, Boolean timeChangeOnly, Boolean* moved);

extern Int16 DatebookLoadPrefs (DatebookPreferenceType* prefsP);

extern void DatebookSavePrefs (void);

extern Int32 TimeToWait (void);

extern Char* DateParamString(const Char* inTemplate, const Char* param0,
			const Char* param1, const Char* param2, const Char* param3);

extern void DoSecurity (void);

extern Boolean ClearEditState (void);

#ifdef __cplusplus 
}
#endif

/***********************************************************************
 *
 *   ParamBlock definition for plug-in routines
 *
 ***********************************************************************/

typedef enum
	{
	DateSendRecordCall,
	DateReceiveDataCall
	} MultiSegmentCalls;
	
typedef struct 
	{
	DmOpenRef dbP;
	UInt16 recordNum;
	} DateSendRecordParams;

typedef struct 
	{
	DmOpenRef dbP;
	ExgSocketPtr exgSocketP;
	} DateReceiveDataParams;

typedef union
	{
	DateSendRecordParams dsr;
	DateReceiveDataParams drd;
	} ParamsType;

typedef struct 
	{
	MultiSegmentCalls call;
	UInt8 reserved;
	ParamsType params;
	} myPlugInParamBlockType;

typedef myPlugInParamBlockType* myPlugInParamBlockPtr;

/***********************************************************************
 *
 *   Function prototype for plug-in routines
 *
 ***********************************************************************/
typedef UInt32 PlugInMainF (myPlugInParamBlockPtr plugInParamsP);


/***********************************************************************
 *
 *   Entry Points
 *
 ***********************************************************************/

extern UInt32 PlugIn1Main( myPlugInParamBlockPtr plugInParamsP );
