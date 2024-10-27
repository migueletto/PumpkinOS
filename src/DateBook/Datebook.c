/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Datebook.c
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
 *
 *****************************************************************************/

#include <PalmOS.h>

#include <PalmUtils.h>

#include "sections.h"
#include "Datebook.h"

extern void ECApptDBValidate (DmOpenRef dbP);


/***********************************************************************
 *
 *	Global variables, declarded in DateGlobals.c.  Because of a bug in
 *  the Metrowerks compiler, we must compile the globals separately with
 *	 PC-relative strings turned off.
 *
 ***********************************************************************/

PUMPKIN_API;

extern	UInt16					TopVisibleAppt;
extern privateRecordViewEnum	CurrentRecordVisualStatus;	// applies to current record
extern privateRecordViewEnum	PrivateRecordVisualStatus;	// applies to all other records

// The following global variables are used to keep track of the edit
// state of the application.
extern 	UInt16					CurrentRecord;						// record being edited
extern 	Boolean					ItemSelected;						// true if a day view item is selected
extern 	UInt16					DayEditPosition;					// position of the insertion point in the desc field
extern	UInt16					DayEditSelectionLength;			// length of the current selection.
extern	Boolean					RecordDirty;						// true if a record has been modified


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void EventLoop (void);


/***********************************************************************
 *
 *	Test Code
 *
 ***********************************************************************/

// Useful structure field offset and size macros
#define prvFieldOffset(type, field)		((UInt32)(&((type*)0)->field))
#define prvFieldSize(type, field)		(sizeof(((type*)0)->field))


#if WRISTPDA

// Routine to check if a key is pressed

Boolean CheckKeyIsPressed( UInt32 KeyMask )
{
	UInt32 i, KeyState = 0;
	for ( i = 0; i < 4; i++ ) {
		SysTaskDelay( 1 );
		KeyState |= KeyCurrentState();
	}
	return ( ( KeyState & KeyMask ) != 0 );
}

// Routine to check how long a key is pressed

UInt32 CheckKeyDuration( UInt32 KeyMask, UInt32 MaxDuration )
{
	UInt32 StartTime, EndTime, Duration, TicksPerSecond;
	// Determine how long the Enter key is depressed.
	TicksPerSecond = SysTicksPerSecond();
	StartTime = TimGetTicks();
	Duration = 0;
	while ( CheckKeyIsPressed( KeyMask ) == true ) {
		Duration = TimGetTicks() - StartTime;
		if ( Duration > MaxDuration )
			break;
	}
	EndTime = TimGetTicks();
	Duration = EndTime - StartTime;
	return Duration;
}

#endif


/***********************************************************************
 *
 * FUNCTION:     SetDBBackupBit
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
static void SetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef localDBP;
	LocalID dbID;
	UInt16 cardNo;
	UInt16 attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
		{
		localDBP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadWrite);
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
 * FUNCTION:		RegisterData
 *
 * DESCRIPTION:	Register with the Exchange Manager to receive .vcs and
 *						text/x-vCalendar.
 *
 * PARAMETERS:		none
 *
 * RETURNED:		nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			dje		7/28/00	Initial Revision
 *
 ***********************************************************************/
static void RegisterData(void)
{
	MemHandle resH = DmGetResource(strRsc, exgDescriptionStrID);
	void *desc = MemHandleLock(resH);
	
	ExgRegisterDatatype(sysFileCDatebook, exgRegExtensionID, dateExtension, (Char *)desc, 0);
	ExgRegisterDatatype(sysFileCDatebook, exgRegTypeID, dateMIMEType, (Char *)desc, 0);
	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}


/***********************************************************************
 *
 * FUNCTION:     DateGetDatabase
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
 *			roger		12/3/97	Initial Revision
 *
 ***********************************************************************/
static Err DateGetDatabase (DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
	UInt16 cardNo;
	LocalID dbID;
	
	
	*dbPP = 0;
	dbP = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
	if (! dbP)
		{
		error = DmCreateDatabase (0, datebookDBName, sysFileCDatebook,
								datebookDBType, false);
		if (error) return error;
		
		dbP = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
		if (! dbP) return ~0;

		// Set the backup bit.  This is to aid syncs with non Palm software.
		SetDBBackupBit(dbP);
		
		error = ApptAppInfoInit (dbP);
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


/***********************************************************************
 *
 * FUNCTION:     StartApplication
 *
 * DESCRIPTION:  This routine opens the application's database, loads the 
 *               saved-state information and initializes global variables.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		6/12/95	Initial Revision
 *			trm		8/5/97	adds variable alarm sound settings
 *			frigino	9/9/97	Add AlarmSoundUniqueRecID initialization
 *			vmk		12/9/97	Call DatebookLoadPrefs() to load and fixup prefs
 *			rbb		6/20/99	Removed multi-segment workaround
 *
 ***********************************************************************/
static UInt16 StartApplication (void)
{
	Err err = 0;
	UInt16 mode;
	DateTimeType dateTime;
	DatebookPreferenceType prefs;
	Int16 prefsVersion;
	
	#if WRISTPDA
	extern UInt32  DebounceEnterKeyEndTime;
	extern UInt16  OldSndSysAmp;
	extern UInt16  OldSndDefAmp;
	extern UInt16  OldKeyInitDelay;
	extern UInt16  OldKeyPeriod;
	extern UInt16  OldKeyDoubleTapDelay;
	extern Boolean OldKeyQueueAhead;
	// We start with Enter key debounce turned off.
	DebounceEnterKeyEndTime = 0;
	#endif
	
	#if WRISTPDA
	// Set automatic display update mode
	FossilDisplayRefreshRateSet( kFossilRefreshAuto );
	#endif

	// Determime if secret record should be shown.
	PrivateRecordVisualStatus = CurrentRecordVisualStatus =
		(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	

	// Get the time formats from the system preferences.
	TimeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

	// Get the date formats from the system preferences.
	LongDateFormat = (DateFormatType)PrefGetPreference(prefLongDateFormat);
	ShortDateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);

	// Get the starting day of the week from the system preferences.
	StartDayOfWeek = PrefGetPreference(prefWeekStartDay);
	

	// Get today's date.
	TimSecondsToDateTime (TimGetSeconds (), &dateTime);
	Date.year = dateTime.year - firstYear;
	Date.month = dateTime.month;
	Date.day = dateTime.day;


	// Find the application's data file.  If it don't exist create it.
	err = DateGetDatabase (&ApptDB, mode);
	if (err)	return err;
	
	
	// Read the preferences / saved-state information (will fix up incompatible versions).
	prefsVersion = DatebookLoadPrefs (&prefs);
	DayStartHour = prefs.dayStartHour;
	DayEndHour = prefs.dayEndHour;
	AlarmPreset = prefs.alarmPreset;
	NoteFont = prefs.noteFont;
	SaveBackup = prefs.saveBackup;
	ShowTimeBars = prefs.showTimeBars;
	CompressDayView = prefs.compressDayView;
	ShowTimedAppts = prefs.showTimedAppts;
	ShowUntimedAppts = prefs.showUntimedAppts;
	ShowDailyRepeatingAppts = prefs.showDailyRepeatingAppts;
	
	AlarmSoundRepeatCount = prefs.alarmSoundRepeatCount;
	AlarmSoundRepeatInterval = prefs.alarmSoundRepeatInterval;
	AlarmSoundUniqueRecID = prefs.alarmSoundUniqueRecID;
	ApptDescFont = prefs.apptDescFont;
	
	AlarmSnooze = prefs.alarmSnooze;

	// The first time this app starts register to handle vCard data.
	if (prefsVersion != datebookPrefsVersionNum)
		RegisterData();

	TopVisibleAppt = 0;
	CurrentRecord = noRecordSelected;	
	
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    StopApplication
 *
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		6/12/95		Initial Revision
 *			trm		8/5/97		adds variable alarm sound settings
 *			frigino	9/9/97		Add saving of AlarmSoundUniqueRecID
 *			vmk		12/9/97		Call DatebookSavePrefs() to write out the prefs
 *			rbb		6/20/99		Removed multi-segment workaround
 *
 ***********************************************************************/
static void StopApplication (void)
{

	#if WRISTPDA
	// Set default display update mode
	FossilDisplayRefreshRateSet( kFossilRefreshDefault );
	#endif

	DatebookSavePrefs();

	// Send a frmSave event to all the open forms.
	FrmSaveAllForms ();
	
	// Close all the open forms.
	FrmCloseAllForms ();

	// Close the application's data file.
	DmCloseDatabase (ApptDB);
}

/***********************************************************************
 *
 * FUNCTION:		DatebookLoadPrefs
 *
 * DESCRIPTION:	Loads app's preferences and fixes them up if they didn't exist
 *						or were of the wrong version.
 *
 * PARAMETERS:		prefsP		-- ptr to preferences structure to fill in
 *
 * RETURNED:		the version of preferences from which values were read
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk		12/9/97	Initial version
 *			vmk		12/11/97	Fix up note font
 *			rbb		4/23/99	Added alarmSnooze
 *
 ***********************************************************************/
Int16 DatebookLoadPrefs (DatebookPreferenceType* prefsP)
{
	UInt16	prefsSize;
	Int16	prefsVersion = noPreferenceFound;
	Boolean haveDefaultFont = false;
	UInt32 defaultFont;
	
	ErrNonFatalDisplayIf(!prefsP, "null prefP arg");
		
	// Read the preferences / saved-state information.  Fix-up if no prefs or older/newer version
	prefsSize = sizeof (DatebookPreferenceType);
	#if WRISTPDA
	// First read the state information from the Wrist PDA private preferences.
	prefsVersion = PrefGetAppPreferences (WPdaCreator, 'DB', prefsP, &prefsSize, true);
	if (prefsVersion > datebookPrefsVersionNum) {
		prefsVersion = noPreferenceFound;
	}
	// If reading the private prefs failed then read the standard preferences.
	if (prefsVersion == noPreferenceFound) {
		prefsVersion = PrefGetAppPreferences (sysFileCDatebook, datebookPrefID, prefsP, &prefsSize, true);
	}
	#else
	prefsVersion = PrefGetAppPreferences (sysFileCDatebook, datebookPrefID, prefsP, &prefsSize, true);
	#endif
	
	// If the preferences version is from a future release (as can happen when going back
	// and syncing to an older version of the device), treat it the same as "not found" because
	// it could be significantly different
	if ( prefsVersion > datebookPrefsVersionNum )
		prefsVersion = noPreferenceFound;
		
	if ( prefsVersion == noPreferenceFound )
		{
		// Version 1 and 2 preferences
		prefsP->dayStartHour = defaultDayStartHour;
		prefsP->dayEndHour = defaultDayEndHour;
		prefsP->alarmPreset.advance = defaultAlarmPresetAdvance;
		prefsP->alarmPreset.advanceUnit = defaultAlarmPresetUnit;
		prefsP->saveBackup = defaultSaveBackup;
		prefsP->showTimeBars = defaultShowTimeBars;
		prefsP->compressDayView = defaultCompressDayView;
		prefsP->showTimedAppts = defaultShowTimedAppts;
		prefsP->showUntimedAppts = defaultShowUntimedAppts;
		prefsP->showDailyRepeatingAppts = defaultShowDailyRepeatingAppts;
		
		// We need to set up the note font with a default value for the system.
		FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
		haveDefaultFont = true;
		#if WRISTPDA
		defaultFont = boldFont; // Converted to FossilBoldFont below.
		#endif		
		prefsP->v20NoteFont = (FontID)defaultFont;
		}
		
	if ((prefsVersion == noPreferenceFound) || (prefsVersion < datebookPrefsVersionNum))
		{
		// Version 3 preferences
		prefsP->alarmSoundRepeatCount = defaultAlarmSoundRepeatCount;
		prefsP->alarmSoundRepeatInterval = defaultAlarmSoundRepeatInterval;
		prefsP->alarmSoundUniqueRecID = defaultAlarmSoundUniqueRecID;
		prefsP->noteFont = prefsP->v20NoteFont;	// 2.0 compatibility (BGT)
		
		// Fix up the note font if we copied from older preferences.
		if ((prefsVersion != noPreferenceFound)
		 && (prefsP->noteFont == largeFont))
			//prefsP->noteFont = FossilLargeBoldFont;
			prefsP->noteFont = largeBoldFont;

		if (!haveDefaultFont)
		{
			FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
			#if WRISTPDA
			defaultFont = boldFont; // Converted to FossilBoldFont below.
			#endif		
		}
		
		prefsP->apptDescFont = (FontID)defaultFont;
		}
	
	if ((prefsVersion == noPreferenceFound) || (prefsVersion < datebookPrefsVersionNum))
		{
		// Version 4 preferences
		prefsP->alarmSnooze = defaultAlarmSnooze;
		}

	// Adapt for Fossil fonts
	//prefsP->noteFont = FossilLargeFontID( true, prefsP->noteFont );
	//prefsP->v20NoteFont = FossilLargeFontID( true, prefsP->v20NoteFont );
	//prefsP->apptDescFont = FossilLargeFontID( true, prefsP->apptDescFont );

	return prefsVersion;
}


/***********************************************************************
 *
 * FUNCTION:    DatebookSavePrefs
 *
 * DESCRIPTION: Saves the current preferences of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * CALLED:	from DatePref.c and Datebook.c
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk		12/9/97	Initial version
 *			rbb		4/23/99	Added alarmSnooze
 *
 ***********************************************************************/
void DatebookSavePrefs (void)
{
	DatebookPreferenceType prefs;
	

	// Write the preferences / saved-state information.
	prefs.dayStartHour = DayStartHour;
	prefs.dayEndHour = DayEndHour;
	prefs.alarmPreset = AlarmPreset;
	prefs.saveBackup = SaveBackup;
	prefs.showTimeBars = ShowTimeBars;
	prefs.compressDayView = CompressDayView;
	prefs.showTimedAppts = ShowTimedAppts;
	prefs.showUntimedAppts = ShowUntimedAppts;
	prefs.showDailyRepeatingAppts = ShowDailyRepeatingAppts;
	prefs.alarmSoundRepeatCount = AlarmSoundRepeatCount;
	prefs.alarmSoundRepeatInterval = AlarmSoundRepeatInterval;
	prefs.alarmSoundUniqueRecID = AlarmSoundUniqueRecID;
	//prefs.apptDescFont = FossilNormalFontID( true, ApptDescFont );
	prefs.apptDescFont = ApptDescFont;
	//prefs.noteFont = FossilNormalFontID( true, NoteFont );		
	prefs.noteFont = NoteFont;		
	prefs.alarmSnooze = AlarmSnooze;

	// Clear reserved field so prefs don't look "different" just from stack garbage!
	prefs.reserved = 0;

	// Handle 2.0 backwards compatibility for fonts.	(BGT)
	prefs.v20NoteFont = stdFont;

	#if WRISTPDA

	// Write the state information to Wrist PDA private preferences.
	PrefSetAppPreferences (WPdaCreator, 'DB', datebookPrefsVersionNum, &prefs,
						   sizeof (DatebookPreferenceType), true);

	// If necessary, map the FossilLarge8Font and FossilLargeBold8Font font ids
	// to Large and LargeBold before writing the standard preferences.
	
	if ( prefs.noteFont == FossilLarge8Font )
		prefs.noteFont = largeFont;

	if ( prefs.noteFont == FossilLargeBold8Font )
		prefs.noteFont = largeBoldFont;

	if ( prefs.apptDescFont == FossilLarge8Font )
		prefs.apptDescFont = largeFont;

	if ( prefs.apptDescFont == FossilLargeBold8Font )
		prefs.apptDescFont = largeBoldFont;

	#endif

	// Write the state information.
	PrefSetAppPreferences (sysFileCDatebook, datebookPrefID, datebookPrefsVersionNum, 
		&prefs, sizeof (DatebookPreferenceType), true);
}


/***********************************************************************
 *
 * FUNCTION:    InitDatabase
 *
 * DESCRIPTION: This routine initializes the datebook database.
	*
 * PARAMETERS:	 datebase
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/19/95	Initial Revision
 *			grant	6/23/99	Set the backup bit.
 *
 ***********************************************************************/
static void InitDatabase (DmOpenRef dbP)
{
	ApptAppInfoInit (dbP);
	
	// Set the backup bit.  This is to aid syncs with non-Palm software.
	SetDBBackupBit(dbP);
}


/***********************************************************************
 *
 * FUNCTION:    SyncNotification
 *
 * DESCRIPTION: This routine is called when the datebook database is 
 *              synchronized.  This routine will sort the database
 *              and schedule the next alarm.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/6/95	Initial Revision
 *
 ***********************************************************************/
static void SyncNotification (void)
{
	UInt16 mode;
	DmOpenRef dbP;

	// Find the application's data file.
	mode = dmModeReadWrite;
	dbP = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
	if (!dbP) return;

	// Resort the appointment database.
	ApptSort (dbP);

	// Remove any alarms from the attention manager queue
	// that might be associated with items deleted during the hotsync.
	UpdatePostedAlarms(PostHotsyncVerification);
	
	// Reschedule the next alarm.
	RescheduleAlarms (dbP);

	DmCloseDatabase (dbP);
}

/***********************************************************************
 *
 * FUNCTION:    SearchDraw
 *
 * DESCRIPTION: This routine draws the description, date, and time of
 *              an appointment found by the text search routine.
 *
 * PARAMETERS:	 apptRecP - pointer to an appointment record.
 *              x        - draw position
 *              y        - draw position
 *              width    - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *		04/18/95	art	Created by Art Lamb
 *		08/03/99	kwk	Use WinDrawTruncChars to handle truncation.
 *		11/14/00	CS		Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/

// DOLATER - these should be based on the screen width, not hard coded.

#define maxSearchRepeatDescWidth	100
#define maxSearchDateWidth			42
#define maxSearchTimeWidth			39

static void SearchDraw (ApptDBRecordPtr apptRecP, Int16 x, Int16 y, 
	Int16 width)
{
	UInt16 i;
	Char timeStr [timeStringLength];
	Char dateStr [dateStringLength];
	UInt16 len;
	UInt16 maxDescWidth;
	Coord drawX;
	Char* ptr;
	Char* desc;
	Char* rscP;
	MemHandle rscH;
	DateFormatType dateFormat;
	TimeFormatType timeFormat;
	
	
	if (apptRecP->repeat)
		maxDescWidth = maxSearchRepeatDescWidth;
	else
		maxDescWidth = width - maxSearchDateWidth - maxSearchTimeWidth;

	// Draw the appointment's desciption.
	desc = apptRecP->description;
	ptr = StrChr(desc, linefeedChr);
	len = (ptr == NULL ? StrLen(desc) : (UInt16)(ptr - desc));
	WinDrawTruncChars (desc, len, x, y, maxDescWidth);

	// If the event is repeating, draw the repeat type.
	if (apptRecP->repeat)
		{
		rscH = DmGetResource (strRsc, repeatTypesStrID);
		rscP = MemHandleLock (rscH);
		for (i = 0; i < apptRecP->repeat->repeatType; i++)
			rscP += StrLen(rscP) + 1;
		x += (width - FntCharsWidth (rscP, StrLen (rscP)));		
		WinDrawChars (rscP, StrLen (rscP), x, y);
		MemHandleUnlock (rscH);
		}
		
	// Draw the appointment's date and time.
	else 
		{
		// Get time and date formats from the system preferences.
		dateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);
		timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);

		if (TimeToInt (apptRecP->when->startTime) != apptNoTime)
			{
			TimeToAscii (apptRecP->when->startTime.hours, 
				apptRecP->when->startTime.minutes, timeFormat, timeStr);
			len = StrLen (timeStr);
			drawX = x + (width - FntCharsWidth (timeStr, len));		
			WinDrawChars (timeStr, len, drawX, y);
			}

		DateToAscii (apptRecP->when->date.month, 
				 		 apptRecP->when->date.day, 
				 		 apptRecP->when->date.year + firstYear, 
				 		 dateFormat, dateStr);
		len = StrLen (dateStr);
		drawX = x + (width - FntCharsWidth (dateStr, len) - maxSearchTimeWidth);		
		WinDrawChars (dateStr, len, drawX, y);
		}
}


/***********************************************************************
 *
 * FUNCTION:    Search
 *
 * DESCRIPTION: This routine searchs the datebook database for records 
 *              containing the string passed. 
 *
 * PARAMETERS:	 findParams - text search parameter block
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *		04/18/95	art	Created by Art Lamb.
 *		05/27/99	jaq	Two-pass process (future first)
 *		08/03/99	kwk	Use TxtFindString for source match length result.
 *		10/21/99	jmp	Made this routine a bit more like everyone else's.
 *
 ***********************************************************************/
static void Search (FindParamsPtr findParams)
{
	Err					err;
	UInt16 				cardNo = 0;
	UInt16				fieldNum;
	UInt16 				recordNum;
	Char* 				desc;
	Char* 				note;
	Char* 				header;
	Boolean 				done;
	Boolean 				match;
	MemHandle			headerStringH;
	LocalID 				dbID;
	DateType				date;
	MemHandle 			recordH;
	DmOpenRef 			dbP;
	RectangleType 		r;
	ApptDBRecordType	apptRec;
	DmSearchStateType searchInfo;
	UInt16				matchLength;
	UInt32				matchPos;

	// Display the heading line.
	headerStringH = DmGetResource(strRsc, findDatebookHeaderStrID);
	header = MemHandleLock(headerStringH);
	done = FindDrawHeader(findParams, header);
   MemHandleUnlock(headerStringH);   
   DmReleaseResource(headerStringH);   
   if (done) 
      return;

	// Find the application's data file.
	err =  DmGetNextDatabaseByTypeCreator (true, &searchInfo, datebookDBType, 
						sysFileCDatebook, true, &cardNo, &dbID);
	if (err)
		{
		findParams->more = false;
		return;
		}

	// Open the appointment database.
	dbP = DmOpenDatabase(cardNo, dbID, findParams->dbAccesMode);
	if (!dbP) 
		{
		findParams->more = false;
		return;
		}

	
	// Search the description and note fields for the "find" string.
	recordNum = findParams->recordNum;
	while (true)
		{
		// Because applications can take a long time to finish a find when
		// the result may be on the screen or for other reasons, users like
		// to be able to stop the find.  Stop the find if an event is pending.
		// This stops if the user does something with the device.  Because
		// this call slows down the search we perform it every so many 
		// records instead of every record.  The response time should still
		// be short without introducing much extra work to the search.
		
		// Note that in the implementation below, if the next 16th record is  
		// secret the check doesn't happen.  Generally this shouldn't be a 
		// problem since if most of the records are secret then the search 
		// won't take long anyways!
		if ((recordNum & 0x000f) == 0 &&			// every 16th record
			EvtSysEventAvail(true))
			{
			// Stop the search process.
			findParams->more = true;
			break;
			}
		
		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);

		//Should we wrap around to repeating and past events?
		if (! recordH)
			{
			findParams->more = false;			
			break;
			}

		ApptGetRecord (dbP, recordNum, &apptRec, &recordH);
		 
		// Search the description field,  if a match is not found search the
		// note field.
		fieldNum = descSeacrchFieldNum;
		desc = apptRec.description;
		match = TxtFindString (desc, findParams->strToFind, &matchPos, &matchLength);
		if (! match)
			{
			note = apptRec.note;
			if (note)
				{
				fieldNum = noteSeacrchFieldNum;
				match = TxtFindString (note, findParams->strToFind, &matchPos, &matchLength);
				}
			}

		// If a match occurred in a repeating event,  make sure there is 
		// a displayable occurrence of the event.
		if (match && apptRec.repeat)
			{
			date = apptRec.when->date;
			match = ApptNextRepeat (&apptRec, &date, true);
			}

		if (match)
			{
			// Add the match to the find paramter block,  if there is no room to
			// display the match the following function will return true.
			done = FindSaveMatch (findParams, recordNum, matchPos, fieldNum, matchLength, cardNo, dbID);
			if (done) 
				{
				MemHandleUnlock (recordH);
				break;
				}

			// Get the bounds of the region where we will draw the results.
			FindGetLineBounds (findParams, &r);
			
			// Display the appointment info.
			SearchDraw (&apptRec, r.topLeft.x+1, r.topLeft.y, r.extent.x-2);

			findParams->lineNumber++;
			}

		MemHandleUnlock (recordH);
		recordNum++;
		}
		
//Exit:
	DmCloseDatabase (dbP);	
}


/***********************************************************************
 *
 * FUNCTION:    GoToItem
 *
 * DESCRIPTION: This routine is called as the result of hitting the 
 *              "Go to" button in the text search dialog. 
 *
 * PARAMETERS:	 goToParams   - where to go to.
 *              launchingApp - true if the application is being launched
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *		08/23/95	art	Created by Art Lamb.
 *		08/03/99	kwk	Use custom param to set up match length.
 *		04/12/00	ryw	Fixed MemSet parameter ordering.
 *
 ***********************************************************************/
void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	UInt16 formID;
	UInt16 recordNum;
	EventType event;
	UInt32 uniqueID;

	recordNum = goToParams->recordNum;
	
	if (!DmQueryRecord(ApptDB, recordNum))
	{
		// Record isn't accessible. This can happen when receiving a beam with an
		// empty record. This prevents a fatal alert, but doesn't fix the off-by-one
		// error. (See DOLATER in sysAppLaunchCmdExgReceiveData case.)
		if (!SeekCurrentRecord(ApptDB, &recordNum, 0, dmSeekBackward))
			if (!SeekCurrentRecord(ApptDB, &recordNum, 0, dmSeekForward))
			{
				FrmAlert(secGotoInvalidRecordAlert);
				FrmGotoForm(DayView);
				return;
			}
	}

	// If the application is already running, destroy the UI, this may 
	// change the index of record we're going to.
	if (! launchingApp)
		{
		DmRecordInfo (ApptDB, recordNum, NULL, &uniqueID, NULL); 
		FrmCloseAllForms ();
		ClearEditState ();
		DmFindRecordByID (ApptDB, uniqueID, &recordNum);
		}
		
	// Go to day view or note view.
	if (goToParams->matchFieldNum == noteSeacrchFieldNum)
		formID = NewNoteView;
	else
		formID = DayView;

	MemSet (&event, sizeof(EventType), 0);

	// Send an event to load the form we want to goto.
	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = formID;
	EvtAddEventToQueue (&event);
 

	// Send an event to goto a form and select the matching text.
	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = formID;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchCustom;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);
}


/***********************************************************************
 *
 * FUNCTION:    GoToAlarmItem
 *
 * DESCRIPTION: This routine is called as the result of hitting the 
 *              "Go to" button in the attention manager dialog.
 *
 * PARAMETERS:	 UniqueID   - uniqueID of the item to go to.
 *
 * RETURNED:	 nothing
 *
 *	HISTORY:
 *		09/07/00 gap	Initial reviesion.
 *
 ***********************************************************************/
void GoToAlarmItem (UInt32 uniqueID)
{
	UInt16	attributes;
	UInt16	recordNum;
	Err		err;
	EventType	event;
	UInt16	cardNo;
	LocalID	dbID;
	DmSearchStateType searchInfo;



#if EMULATION_LEVEL != EMULATION_NONE
		FrmCloseAllForms ();
		ClearEditState ();
#endif	

	// remove the item from the attention manager queue
	DmGetNextDatabaseByTypeCreator (true, &searchInfo, sysFileTApplication, sysFileCDatebook, true, &cardNo, &dbID);
	AttnForgetIt(cardNo, dbID, uniqueID);
	
	// verify that the specified uniqueID is still in the Date Book's database
	// one reason this needs to be done is to prevent crashing should the alarm
	// have been on an untitled event with no note attached.  in the process of 
	// executing the goto, the original record may have been deleted as all 
	// untitled events w/o a note are as soon as the item is deselected.
	err = FindRecordByID (ApptDB, uniqueID, &recordNum);
	if (err)
		{
		FrmGotoForm(DayView);
		return;
		}
	
	// Get the attribute information for the record
	DmRecordInfo (ApptDB, recordNum, &attributes, NULL, NULL);


	// if the user does a "go to" from an attention manager dialog while the
	// private items are hidden and the selected item happens to have the 
	// private bit set, the user must submit his password so that the 
	// security level can be changed from hide to show before the goto to the 
	// specified record occurs
	if ( (attributes & dmRecAttrSecret) && (PrivateRecordVisualStatus == hidePrivateRecords) )
		{		
		if (SecVerifyPW(showPrivateRecords))
			{
			DmCloseDatabase (ApptDB);	
			ApptDB = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, (dmModeReadWrite | dmModeShowSecret));
			PrivateRecordVisualStatus = CurrentRecordVisualStatus = showPrivateRecords;
			}
		else
			{
			FrmGotoForm(DayView);
			return;
			}
		}
		
	MemSet (&event, sizeof(EventType), 0);

	// Send an event to load the form we want to goto.
	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = DayView;
	EvtAddEventToQueue (&event);


	// Send an event to goto a form and select the matching text.
	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = DayView;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = 0;
	event.data.frmGoto.matchLen = 0;
	event.data.frmGoto.matchFieldNum = 0;
	EvtAddEventToQueue (&event);
}


/***********************************************************************
 *
 * FUNCTION:    DrawTime
 *
 * DESCRIPTION: Draws the given time. Used by the custom draw routines
 *					 for the date columns of the agenda and day views.
 *
 * PARAMETERS:  inTime		- the time
 *              inFormat	- time-to-ascii conversion format
 *					 inBoundsP	- drawing area
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
void DrawTime (
	TimeType							inTime,
	TimeFormatType					inFormat,
	FontID							inFont,
	JustificationType				inJustification,
	RectangleType*					inBoundsP )
{
	FontID							curFont;
	char								timeStr [timeStringLength];
	TimeFormatType					format;
	UInt16							len;
	UInt16							width;
	Int16								x;

	// No-time appointment?
	if (TimeToInt(inTime) == apptNoTime)
		{
		// Show a centered diamond symbol, overriding the font params
		//inFont = FossilSymbolFont;
		inFont = symbolFont;
		inJustification  = centerAlign;

		timeStr[0] = symbolNoTime;
		timeStr[1] = '\0';
		}
	else
		{
		if (inFormat == tfColonAMPM)
			format = tfColon;
		 else if (inFormat == tfDotAMPM)
		 	format = tfDot;
		 else
			format = inFormat;

		TimeToAscii (inTime.hours, inTime.minutes, format, timeStr);

		}	
	
	// Use the string width and alignment to compute its starting point
	len = StrLen (timeStr);
	width = FntCharsWidth (timeStr, len);
	x = inBoundsP->topLeft.x;
	switch (inJustification)
		{
		case rightAlign:
			x += inBoundsP->extent.x - width;
			break;
		
		case centerAlign:
			x += (inBoundsP->extent.x - width) / 2;
			break;
		default:
			break;
		}
	
	x = max (inBoundsP->topLeft.x, x);
	
	// Draw the time
	curFont = FntSetFont (inFont);
	WinDrawChars (timeStr, len, x, inBoundsP->topLeft.y);	
	FntSetFont (curFont);
}


#if 0
/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
void* GetObjectPtr (UInt16 objectID)
{
	FormPtr frm;
	
	frm = FrmGetActiveForm ();
	return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));

}
#endif


/***********************************************************************
 *
 * FUNCTION:    DirtyRecord
 *
 * DESCRIPTION: Mark a record dirty (modified).  Record marked dirty 
 *              will be synchronized.
 *
 * PARAMETERS:  record index
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/15/95	Initial Revision
 *
 ***********************************************************************/
void DirtyRecord (DmOpenRef dbP, UInt16 index)
{
	UInt16		attr;
	Err			err;

	err = DmRecordInfo (dbP, index, &attr, NULL, NULL);
	ErrFatalDisplayIf (err, "Invalid index");
	
	attr |= dmRecAttrDirty;
	DmSetRecordInfo (dbP, index, &attr, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    ClearEditState
 *
 * DESCRIPTION: This routine take the application out of edit mode.
 *              The edit state of the current record is remember until
 *              this routine is called.  
 *
 *              If the current record is empty its deleted by this
 *              routine.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true is current record is deleted by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/28/95	Initial Revision
 *			gap	8/01/00	add attention manager support
 *
 ***********************************************************************/
Boolean ClearEditState (void)
{
	UInt16 recordNum;
	Boolean empty;
	Boolean hasAlarm;
	MemHandle recordH;
	ApptDBRecordType apptRec;

	if ( ! ItemSelected)
		{
		CurrentRecord = noRecordSelected;
		return (false);
		}
	
	recordNum = CurrentRecord;

	// Clear the global variables that keep track of the edit state of the
	// current record.
	ItemSelected = false;
	CurrentRecord = noRecordSelected;
	DayEditPosition = 0;
	DayEditSelectionLength = 0;
	
	// If the description field is empty there is no note, delete
	// the record.
	ApptGetRecord (ApptDB, recordNum, &apptRec, &recordH);
	hasAlarm = (apptRec.alarm != NULL);	
	empty = true;
	if (apptRec.description && *apptRec.description) {
		empty = false;
	} else if (apptRec.note && *apptRec.note) {
		empty = false;
	} else {
	}
	MemHandleUnlock (recordH);


	if (empty)
		{
		UInt32		alarmRef;	// Ignored. Needed for AlarmGetTrigger()

		// if the event to be delete had an alarm, be sure to remove it
		// from the posted alarm queue before the event is deleted.
		if (hasAlarm)
			DeleteAlarmIfPosted(recordNum);
		
		// If the record was not modified, and the description and 
		// note fields are empty, remove the record from the database.
		// This can occur when a new empty record is deleted.
		if (RecordDirty)
			{
			DmDeleteRecord (ApptDB, recordNum);
			DmMoveRecord (ApptDB, recordNum, DmNumRecords (ApptDB));
			}
		else {
			DmRemoveRecord (ApptDB, recordNum);
		}
		
		// If the appointment had the currently scheduled alarm, reschedule the next alarm
		if (hasAlarm)
			{
			UInt32 alarmTrigger = AlarmGetTrigger (&alarmRef);
			
			if (alarmTrigger && (alarmTrigger == ApptGetAlarmTime (&apptRec, TimGetSeconds(), true)))
				{
				RescheduleAlarms (ApptDB);
				}
			}

		return (true);
		}

	return (false);
}


// Branch island to get to the glue code, linked in first.

Char* DateParamString(const Char* inTemplate, const Char* param0,
			const Char* param1, const Char* param2, const Char* param3)
{
	return(TxtParamString(inTemplate, param0, param1, param2, param3));
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the Datebook
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95	Initial Revision
 *			vmk	10/22/97	Moved MIDI db creation to general panel
 *			art	1/7/98	Save current record before receiving a beamed record
 *			rbb	4/22/99	Moved code to link w/ 16-bit jumps
 *			rbb	6/20/99	Removed multi-segment workaround
 *			jmp	10/02/99 Made the support for the sysAppLaunchCmdExgReceiveData
 *								launch code more like its counterparts in Address,
 *								Memo, and ToDo.
 *			jmp	10/18/99	Note that Datebook doesn't attempt to create an empty
 *								database if a default ("demo") database doesn't exist.
 *			jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *								since it was already being done in sysAppLaunchCmdExgReceiveData if
 *								the user affirmed sysAppLaunchCmdExgAskUser.  Also, in sysAppLaunchCmdExgReceiveData
 *								prevent call FrmSaveAllForms() if we're being call back through
 *								PhoneNumberLookup() as the two tasks are incompatible with each other.
 *			rbb	11/12/99	Added recentFormFeature to pick default view on launch
 *
 ***********************************************************************/
PUBLIC UInt32	PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 error;
	UInt32 defaultForm;
   DmOpenRef dbP;
       
	// The only place the following define is used is in the Jamfile for this application.
	// It's sole purpose is to provide a way to check if cross segment calls are being made
	// when application globals are unavailable.
#ifndef SINGLE_SEGMENT_CHECK
	Boolean launched;

	#if WRISTPDA
	// Clear feature informing To Do that it was launched by Datebook's Agenda View ('AV').
	FtrSet( WPdaCreator, 'AV', false );
	#endif

	// Launch code sent by the launcher or the datebook button.
	if (cmd == sysAppLaunchCmdNormalLaunch)
		{
		error = StartApplication ();
		if (error) return (error);
		
		// If the user previously left the Datebook while viewing the agenda,
		// return there. Otherwise, go to the day view
		error = FtrGet (sysFileCDatebook, recentFormFeature, &defaultForm);
		if (error)
			{
			defaultForm = defaultRecentForm;
			}
			
		#if WRISTPDA
		FossilBackKeyModeSet( kFossilBackKeyNoAction );
		FossilExtendedFontSelectSet( true );
		#endif
		FrmGotoForm (defaultForm);
		EventLoop ();
		StopApplication ();
		#if WRISTPDA
		FossilExtendedFontSelectSet( false );
		FossilBackKeyModeSet( kFossilBackKeyLauncher );
		#endif
		}


	// This action code is a DateBook specific custom launch code.
	// It will always require that the app launches as it is a result
	// of a SysUIAppSwitch call.
	else if (cmd == appLaunchCmdAlarmEventGoto)
		{
			error = StartApplication ();
			if (error) return (error);

			#if WRISTPDA
			FossilBackKeyModeSet( kFossilBackKeyNoAction );
			FossilExtendedFontSelectSet( true );
			#endif

			GoToAlarmItem (*((UInt32*)cmdPBP));

			EventLoop ();
			StopApplication ();	

			#if WRISTPDA
			FossilExtendedFontSelectSet( false );
			FossilBackKeyModeSet( kFossilBackKeyLauncher );
			#endif
		}

	

	// This action code might be sent to the app when it's already running
	// if the use hits the "Go To" button in the Find Results dialog box.
	else if (cmd == sysAppLaunchCmdGoTo)
		{
		launched = launchFlags & sysAppLaunchFlagNewGlobals;
		if (launched)
			{
			error = StartApplication ();
			if (error) return (error);

			#if WRISTPDA
			FossilBackKeyModeSet( kFossilBackKeyNoAction );
			FossilExtendedFontSelectSet( true );
			#endif

			GoToItem ((GoToParamsPtr) cmdPBP, launched);

			EventLoop ();
			StopApplication ();	

			#if WRISTPDA
			FossilExtendedFontSelectSet( false );
			FossilBackKeyModeSet( kFossilBackKeyLauncher );
			#endif
			}
		else
			GoToItem ((GoToParamsPtr) cmdPBP, launched);
		}
#else
	if (false) {}
#endif

	//////////////////////////////////////////////////////////////////////////////////
	//
	// WARNING! Launch codes below this point can be called from a context where
	// 			application globals are unavailable.
	//
	//////////////////////////////////////////////////////////////////////////////////

	// Launch code sent to running app before sysAppLaunchCmdFind
	// or other action codes that will cause data searches or manipulation.
	// We don't need to respond to this launch code because events are
	// edited in place.
//	else if (cmd == sysAppLaunchCmdSaveData)
//		{
//		FrmSaveAllForms ();
//		}
	

	// Launch code sent by text search.
	else if (cmd == sysAppLaunchCmdFind)
		{
		Search ((FindParamsPtr)cmdPBP);
		}
	

	// Launch code sent by sync application to notify the datebook 
	// application that its database was been synced.
	else if (cmd == sysAppLaunchCmdSyncNotify)
		{
		SyncNotification ();
		}
	

	// Launch code sent by Alarm Manager to notify the datebook 
	// application that an alarm has triggered.
	else if (cmd == sysAppLaunchCmdAlarmTriggered)
		{
		AlarmTriggered ((SysAlarmTriggeredParamType *)cmdPBP);
		}
	

	// Launch code sent when the system time is changed.
	else if (cmd == sysAppLaunchCmdTimeChange)
		{
		// reset the trigger for the next alarm to fire
		AlarmReset (false);
		
		// Remove any "future" alarms from the attention manager queue
		// (ie alarms that will trigger after the new time setting)
		UpdatePostedAlarms(DeviceTimeChanged);
		}
		

	// This action code is sent after the system is reset.  We use this time
	// to create our default database.  Note:  Unlike several other built-in
	// apps, we do not attempt to create an empty database if the default
	// database image doesn't exist.
	else if (cmd == sysAppLaunchCmdSystemReset) 
		{
		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
			{
			MemHandle resH;
			
			resH = DmGet1Resource(sysResTDefaultDB, sysResIDDefaultDB);
			if (resH) 
				{
				DmCreateDatabaseFromImage(MemHandleLock(resH));
				MemHandleUnlock(resH);
				DmReleaseResource(resH);
				
				// set the backup bit on the new database
				SetDBBackupBit(NULL);
				}

			// Register to receive vcs files on hard reset.
			RegisterData();
			}

		if (! ((SysAppLaunchCmdSystemResetType*)cmdPBP)->hardReset)
			{
			AlarmReset (false);		// Find and install next upcoming alarm
			}
		}


   // Receive the record.  The app will parse the data and add it to the database.
   // This data should be displayed by the app.
	else if (cmd == sysAppLaunchCmdExgReceiveData) 
   	{
      UInt32 currentUID;
      
      // if our app is not active, we need to open the database 
      // the subcall flag is used here since this call can be made without launching the app
      if (!(launchFlags & sysAppLaunchFlagSubCall))
      	{
      	error = DateGetDatabase(&dbP, dmModeReadWrite);
      	}
      else
      	{
      	dbP = ApptDB;
      	
			// We don't delete the current record if it's empty because the user
			// could cancel the beam receive.
			
			// DateReceiveData() inserts the received record in sorted order. This may may change
			// the index of the current record. So we remember its UID here, and refresh our copy
			// of its index afterwards.
			if (CurrentRecord != noRecordSelected)
				DmRecordInfo(dbP, CurrentRecord, NULL, &currentUID, NULL);
      	}
      
      if (dbP != NULL)
      	{
			error = DateReceiveData(dbP, (ExgSocketPtr) cmdPBP);
			
	      if (launchFlags & sysAppLaunchFlagSubCall)
	      	{
				if (CurrentRecord != noRecordSelected)
					{
					if (DmFindRecordByID(dbP, currentUID, &CurrentRecord) != 0)
						CurrentRecord = noRecordSelected;	// Can't happen, but...
					
					// DOLATER dje -
					//		To fix the off-by-one error, we can decrement exgSocketP->goToParams.recordNum
					//		if it's after the current empty record in order to compensate for the
					//		current empty record getting deleted when we exit before the goto launch.
	      		}
	      	}
	      
			// The received event may have an alarm, reschedule the next alarm.
			RescheduleAlarms (dbP);

         if (!(launchFlags & sysAppLaunchFlagSubCall))
				DmCloseDatabase(dbP);
			}
		else
			error = exgErrAppError;	// DOLATER dje - use a new error code - "try again after switching apps"
		
		// If we can't open our database, return the error since it wasn't passed to ExgDisconnect
		return error;
		}

	else if (cmd == sysAppLaunchCmdExgPreview)
	{
		DateTransferPreview((ExgPreviewInfoType *)cmdPBP);
	}      
	// This action code is sent by the DesktopLink server when it create 
	// a new database.  We will initializes the new database.
	else if (cmd == sysAppLaunchCmdInitDatabase)
		{
		InitDatabase (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
		}
		
	else if (cmd == sysAppLaunchCmdAttention)
		{
		AttentionBottleNeckProc((AttnLaunchCodeArgsType*)cmdPBP);
		}
		

	return (0);
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    TimeToWait
 *
 * DESCRIPTION:	This routine calculates the number of ticks until the
 *						time display should be hidden.  If the time is not being
 *						displayed, we return evtWaitForever - a good default for
 *						EvtGetEvent.
 *
 *						If the agenda view is on display, return a delay of one
 *						second, allowing the title bar clock to update.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    number of ticks to wait
 *						== evtWaitForever if we are not showing the time
 *						== 0 if time is up
 *
 *	NOTES:		Uses the global variables TimeDisplayed and TimeDisplayTick.
 *					TimeDisplayed is true when some temporary information is on
 *					display.  In that case, TimeDisplayTick is the time when the
 *					temporary display should go away.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	2/2/99	Initial Revision
 *			rbb	11/15/99	Added timeout for the agenda view 
 *
 ***********************************************************************/
Int32 TimeToWait()
{
	Int32 timeRemaining;
	
	if (FrmGetActiveFormID () == AgendaView)
		{
		return sysTicksPerSecond;
		}
	
	if (!TimeDisplayed)
		{
		return evtWaitForever;
		}
	
	timeRemaining = TimeDisplayTick - TimGetTicks();
	if (timeRemaining < 0)
		{
		timeRemaining = 0;
		}
	
	return timeRemaining;
}


/***********************************************************************
 *
 * FUNCTION:    ApplicationHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/11/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteVie instead of NoteView.
 *			rbb	11/12/99	Added recentFormFeature to pick default view on launch
 *
 ***********************************************************************/
static Boolean ApplicationHandleEvent (EventType* event)
{
	UInt16 formId;
	FormPtr frm;

	if (event->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = event->data.frmLoad.formID;
		frm = FrmInitForm (formId);
		FrmSetActiveForm (frm);		
		
		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmDispatchEvent each time is receives an
		// event.
		if (formId == DayView)
			FrmSetEventHandler (frm, DayViewHandleEvent);
			
		else if (formId == WeekView)
			FrmSetEventHandler (frm, WeekViewHandleEvent);
	
		else if (formId == MonthView)
			FrmSetEventHandler (frm, MonthViewHandleEvent);
	
		else if (formId == AgendaView)
			FrmSetEventHandler (frm, AgendaViewHandleEvent);
	
		else if (formId == NewNoteView)
			FrmSetEventHandler (frm, NoteViewHandleEvent);
	
		else if (formId == DetailsDialog)
			FrmSetEventHandler (frm, DetailsHandleEvent);

		else if (formId == RepeatDialog)
			FrmSetEventHandler (frm, RepeatHandleEvent);
	
		else if (formId == PreferencesDialog)
			FrmSetEventHandler (frm, PreferencesHandleEvent);

		else if (formId == DisplayDialog)
			FrmSetEventHandler (frm, DisplayOptionsHandleEvent);
		
		FtrSet (sysFileCDatebook, recentFormFeature,
					formId == AgendaView ? AgendaView : DayView);

		return (true);
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    PreprocessEvent
 *
 * DESCRIPTION: This routine handles special event processing that
 *              needs to occur before the System Event Handler get 
 *              and event.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/10/97	Initial Revision
 *			CSS	06/22/99	Standardized keyDownEvent handling
 *								(TxtCharIsHardKey, commandKeyMask, etc.)
 *
 ***********************************************************************/
static void PreprocessEvent (EventType* event)
{

	// We want to allow the datebook event handled see the command keys
	// before the any other event handler get them.  Some of the databook
	// views display UI that dispappears automaticial (the time in the 
	// title, or an event's descripition in the week view).  This UI
	// needs to be dismissed before the System Event Handler or the 
	// Menu Event Handler displays any UI objects.  
	if (event->eType == keyDownEvent) 
		{
		if	(	(!TxtCharIsHardKey(	event->data.keyDown.modifiers,
											event->data.keyDown.chr))
			&&	(EvtKeydownIsVirtual(event))
			#if WRISTPDA
			&&	(event->data.keyDown.chr != vchrThumbWheelBack)
			&&	(event->data.keyDown.chr != vchrThumbWheelPush)
			&&	(event->data.keyDown.chr != vchrThumbWheelUp)
			&&	(event->data.keyDown.chr != vchrThumbWheelDown)
			#endif
			&&	(event->data.keyDown.chr != vchrPageUp)
			&&	(event->data.keyDown.chr != vchrPageDown)
			&&	(event->data.keyDown.chr != vchrSendData))
			{
			FrmDispatchEvent (event);
			}
		}
}

//extern void EvtPrintEvent(char *op, EventType *event);

/***********************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: This routine is the event loop for the Datebook
 *              aplication.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95	Initial Revision
 *			grant 2/2/99	Use TimeToWait() for time to wait for next event
 *
 ***********************************************************************/
static void EventLoop (void)
{
	UInt16 error;
	EventType event;

	#if WRISTPDA
	KeySetMask( 0xFFFFFFFF );
	#endif

	do
		{
		EvtGetEvent (&event, TimeToWait());
//EvtPrintEvent("EventLoop", &event);
		
		PreprocessEvent (&event);
		
		if (! SysHandleEvent (&event))

			if (! MenuHandleEvent (NULL, &event, &error))

				if (! ApplicationHandleEvent (&event))

					FrmDispatchEvent (&event); 

//		#if	EMULATION_LEVEL != EMULATION_NONE
//		ECApptDBValidate (ApptDB);
//		#endif
		}
	while (event.eType != appStopEvent);
}
