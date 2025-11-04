/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DatePref.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This module contains the routines that handle the Datebook 
 *   applications's preferences.
 *
 * History:
 *		September 27, 1995	Created by Art Lamb
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "Datebook.h"

#define numRepeats 						5
#define numPlayEverys 					4
#define defaultRepeatsLevel			3				
#define defaultPlayEveryLevel			2	

// Max len of sound trigger label placeholder
#define soundTriggerLabelLen			32


/***********************************************************************
 *
 *	Protoypes
 *
 **********************************************************************/
static void SetSoundLabel(FormPtr formP, const char* labelP);
static void FreeAll(void);


/***********************************************************************
 *
 *	Global variables
 *
 **********************************************************************/
// Number of times to remind the person
static UInt16 RepeatCountMappings [numRepeats] =
	{	
	1, 2, 3, 5, 10
	};

// How many seconds between repeats
static UInt16 RepeatIntervalMappings [numPlayEverys] =
	{	
	1, 5, 10,  30
	};

// Placeholder for sound trigger label
static Char * soundTriggerLabelP;

// handle to the list containing names and DB info of MIDI tracks.
// Each entry is of type SndMidiListItemType.
static MemHandle	gMidiListH;
// number of entries in the MIDI list
static UInt16	gMidiCount;

// The following global variable are only valid while editng the datebook's
// preferences.
static UInt16	PrefDayStartHour;
static UInt16	PrefDayEndHour;

// The following globals are for the repeat rates of the alarms preferences.
static UInt16	PrefSoundRepeatCount;					// number of times to repeat alarm sound 
static UInt16	PrefSoundRepeatInterval;				// interval between repeat sounds, in seconds

// The following globals are for the repeat rates of the alarms preferences.
static UInt32	PrefSoundUniqueRecID;					// Alarm sound MIDI file unique ID record identifier


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
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
/*
static void * GetObjectPtr (UInt16 objectID)
{
	FormPtr frm;
	
	frm = FrmGetActiveForm ();
	return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));
}
*/


/***********************************************************************
 *
 * FUNCTION:    MidiPickListDrawItem
 *
 * DESCRIPTION: Draw a midi list item.
 *
 * PARAMETERS:  itemNum - which shortcut to draw
 *					 bounds - bounds in which to draw the text
 *					 unusedP - pointer to data (not used)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vnk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/18/97	Modified to truncate items in list with ...
 *
 ***********************************************************************/
static void MidiPickListDrawItem (Int16 itemNum, RectanglePtr bounds, 
	Char **unusedP)
{
#pragma unused (unusedP)

	Char *	itemTextP;
	Int16		itemTextLen;
	Int16		itemWidth;
	SndMidiListItemType*	listP;
	
	ErrNonFatalDisplayIf(itemNum >= gMidiCount, "index out of bounds");
	
	// Bail out if MIDI sound list is empty
	if (gMidiListH == NULL)
		return;
	
	listP = MemHandleLock(gMidiListH);

	itemTextP = listP[itemNum].name;

	// Truncate the item with an ellipsis if it doesnt fit in the list width.
	// Get the item text length
	itemTextLen = StrLen(itemTextP);
	// Get the width of the text
	itemWidth = FntCharsWidth(itemTextP, itemTextLen);
	// Does it fit?
	if (itemWidth <= bounds->extent.x)
		{
		// Draw entire item text as is
		WinDrawChars(itemTextP, itemTextLen, bounds->topLeft.x, bounds->topLeft.y);
		}
	else
		{
		// We're going to truncate the item text
		Boolean	ignored;
		char		ellipsisChar = chrEllipsis;
		// Set the new max item width
		itemWidth = bounds->extent.x - FntCharWidth(ellipsisChar);
		// Find the item length that fits in the bounds minus the ellipsis
		FntCharsInWidth(itemTextP, &itemWidth, &itemTextLen, &ignored);
		// Draw item text that fits
		WinDrawChars(itemTextP, itemTextLen, bounds->topLeft.x, bounds->topLeft.y);
		// Draw ellipsis char
		WinDrawChars(&ellipsisChar, 1, bounds->topLeft.x + itemWidth, bounds->topLeft.y);
		}

	// Unlock list items
	MemPtrUnlock(listP);
}


/***********************************************************************
 *
 * FUNCTION:    CreateMidiPickList
 *
 * DESCRIPTION: Create a list of midi sounds available.
 *
 * PARAMETERS:	listP	-- the list to contain the panel list
 *					funcP	-- item draw function
 *
 * RETURNED:    panelCount and panelIDsP are set
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			vmk		8/8/97	Initial version
 *			trev		8/14/97	Ported to dateBook App
 *			frigino	8/20/97	Added maximum widening of MIDI sound list
 *
 ***********************************************************************/
static void CreateMidiPickList(ListPtr listP, ListDrawDataFuncPtr funcP)
{
	SndMidiListItemType*	midiListP;
	UInt16		i;
	UInt16		listWidth;
	UInt16		maxListWidth;
	Boolean	bSuccess;
	RectangleType r;
		
	// Load list of midi record entries
	bSuccess = SndCreateMidiList(sysFileCSystem, false, &gMidiCount, &gMidiListH);
	if ( !bSuccess )
		{
		gMidiListH = 0;
		gMidiCount = 0;
		return;
		}
	
		
	// Now set the list to hold the number of sounds found.  There
	// is no array of text to use.
	LstSetListChoices(listP, NULL, gMidiCount);
	
	// Now resize the list to the number of panel found
	LstSetHeight (listP, gMidiCount);

	// Because there is no array of text to use, we need a function
	// to interpret the panelIDsP list and draw the list items.
	LstSetDrawFunction(listP, funcP);

	// Make the list as wide as possible to display the full sound names
	// when it is popped winUp.

	// Lock MIDI sound list
	midiListP = MemHandleLock(gMidiListH);
	// Initialize max width
	maxListWidth = 0;
	// Iterate through each item and get its width
	for (i = 0; i < gMidiCount; i++)
		{
			// Get the width of this item
			listWidth = FntCharsWidth(midiListP[i].name, StrLen(midiListP[i].name));
			// If item width is greater that max, swap it
			if (listWidth > maxListWidth)
				{
				maxListWidth = listWidth;
				}
		}
	// Unlock MIDI sound list
	MemPtrUnlock(midiListP);
	// Set list width to max width + left margin
	listP->bounds.extent.x = maxListWidth + 2;
	// Get pref dialog window extent
	FrmGetFormBounds(FrmGetActiveForm(), &r);
	// Make sure width is not more than window extent
	if (listP->bounds.extent.x > r.extent.x)
		{
		listP->bounds.extent.x = r.extent.x;
		}
	// Move list left if it doesnt fit in window
	if (listP->bounds.topLeft.x + listP->bounds.extent.x > r.extent.x)
		{
			listP->bounds.topLeft.x = r.extent.x - listP->bounds.extent.x;
		}
}

/***********************************************************************
 *
 * FUNCTION:    FreeMidiPickList
 *
 * DESCRIPTION: Free the list of midi sounds available.
 *
 * PARAMETERS:	none
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vmk	8/11/97	Initial version
 *			trev	08/14/97	Ported to dateBook App
 *
 ***********************************************************************/
static void FreeMidiPickList(void)
{
	if ( gMidiListH )
		{
		MemHandleFree(gMidiListH);
		gMidiListH = 0;
		gMidiCount = 0;
		}
}


/***********************************************************************
 *
 * FUNCTION:    MapToPosition
 *
 * DESCRIPTION:	Map a value to it's position in an array.  If the passed
 *						value is not found in the mappings array, a default
 *						mappings item will be returned.
 *
 * PARAMETERS:  value	- value to look for
 *
 * RETURNED:    position value found in
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			kcr		9/13/95	Initial Revision
 *			frigino	8/21/97	Converted all params to UInt16
 *
 ***********************************************************************/
static UInt16 MapToPosition (UInt16* mappingArray, UInt16 value,
									UInt16 mappings, UInt16 defaultItem)
	{
	UInt16 i;
	
	i = 0;
	while (mappingArray[i] != value && i < mappings)
		i++;
	if (i >= mappings)
		return defaultItem;

	return i;
	}	//	end of MapToPosition


/***********************************************************************
 *
 * FUNCTION:    PreferencesUpdateScrollers
 *
 * DESCRIPTION: This routine updates the day-start and day-end time 
 *              scrollers
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesUpdateScrollers ()
{
	FormPtr frm;
	UInt16 upIndex;
	UInt16 downIndex;
	
	// Update the start time scrollers.
	frm = FrmGetActiveForm ();
	upIndex = FrmGetObjectIndex (frm, PreferStartUpButton);
	downIndex = FrmGetObjectIndex (frm, PreferStartDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex,
		PrefDayStartHour<23, PrefDayStartHour>0);
	
	// Update the end time scrollers.
	upIndex = FrmGetObjectIndex (frm, PreferEndUpButton);
	downIndex = FrmGetObjectIndex (frm, PreferEndDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, 
		PrefDayEndHour<23, PrefDayEndHour>0);		
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesAlarmOnOff
 *
 * DESCRIPTION: This routine shows or hides the alarm preset ui object.
 *              It is call when the alarm preset check box is turn on 
 *              or off.
 *
 * PARAMETERS:  on - true to show alarm preset ui, false to hide.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *
 ***********************************************************************/
 static void PreferencesAlarmPresetOnOff (Boolean on)
 {
 	UInt16 fldIndex;
 	UInt16 ctlIndex;
 	Char * textP;
 	Char * label;
 	MemHandle textH;
 	FormPtr frm;
 	ListPtr lst;
 	FieldPtr fld;
 	ControlPtr ctl;
 
	frm = FrmGetActiveForm ();
	fld = GetObjectPtr (PreferAlarmField);

	fldIndex = FrmGetObjectIndex (frm, PreferAlarmField);
	ctlIndex = FrmGetObjectIndex (frm, PreferAlarmUnitTrigger);
	
 	if (on)
 		{
		// Set the value of the alarm advance field.
		textH = FldGetTextHandle (fld);
		if (textH) MemHandleFree (textH);
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, defaultAlarmAdvance);
		MemPtrUnlock (textP);

		FldSetTextHandle (fld, textH);
		
		// Set the alarm advance unit of measure (minutes, hours, or days).
		lst = GetObjectPtr (PreferAlarmList);		
		LstSetSelection (lst, defaultAdvanceUnit);
		label = LstGetSelectionText (lst, defaultAdvanceUnit);

		ctl = GetObjectPtr (PreferAlarmUnitTrigger);
		CtlSetLabel (ctl, label);

		// Show the alarm advance ui objects. 		
		FrmShowObject (frm, fldIndex);
		FrmShowObject (frm, ctlIndex);

		FrmSetFocus (frm, fldIndex);
 		}
 	else
 		{
		FrmSetFocus (frm, noFocus);

		FldFreeMemory (fld);

		// Hide the alarm advance ui objects. 		
		FrmHideObject (frm, fldIndex);
		FrmHideObject (frm, ctlIndex);
 		}
 }

/***********************************************************************
 *
 * FUNCTION:    PreferencesDrawTime
 *
 * DESCRIPTION: This routine draw the time passed at the location specified.
 *
 * PARAMETERS:  hour        - hour to draw, minutes is assumed to be zero
 *              isStartTime - true if we're drawing the start time
 *
 * RETURNED:    nothing
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/7/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesDrawTime (UInt8 hour, Boolean isStartTime)
{
	Char str[timeStringLength];
	UInt16 id;
	UInt16 len;
	UInt16 index;
	Int16 x, y;
	FontID curFont;
	FormPtr frm;
	RectangleType r;

	// Compute the drawing bounds.
	if (isStartTime)
		id = PreferStartUpButton;
	else
		id = PreferEndUpButton;

	frm = FrmGetActiveForm ();
	index = FrmGetObjectIndex (frm, id);
	FrmGetObjectPosition (frm, index, &x, &y);
	
	r.topLeft.x = x - dayRangeTimeWidth - 5;
	r.topLeft.y = y + 2;
	r.extent.x = dayRangeTimeWidth;
	r.extent.y = dayRangeTimeHeight;
	
	// Format the time into a string.
	TimeToAscii (hour, 0, TimeFormat, str);
	len = StrLen (str);
	
	// Draw a frame around the time.
	WinDrawRectangleFrame (simpleFrame, &r);
	
	WinEraseRectangle (&r, 0);
	
	// Draw the time.
	curFont = FntSetFont (boldFont);
	x = r.topLeft.x + (r.extent.x - FntCharsWidth (str, len)) - 3;
	y = r.topLeft.y + ((r.extent.y - FntLineHeight ()) / 2);
	WinDrawChars (str, len, x, y);
	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesApply
 *
 * DESCRIPTION: This routine applies the changes made in the Preferences Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/3/95	Initial Revision
 *			vmk	12/9/97	Set alarm sound and repeat/interval to Datebook globals
 *			vmk	12/9/97	Added a call to save app's preferences
 *
 ***********************************************************************/
static UInt16 PreferencesApply (UInt16 dayStartHour, UInt16 dayEndHour)
{	
	UInt16		updateCode = 0;
	ListPtr		lst;
	FieldPtr 	fld;
	ControlPtr	ctl;
	Boolean		updateAlarms;
		
	if ((dayStartHour != DayStartHour) || (dayEndHour != DayEndHour))
		{
		DayStartHour = dayStartHour;
		DayEndHour = dayEndHour;
		updateCode = updateDisplayOptsChanged;
		}

	// Get the alarm preset settings.
	ctl = GetObjectPtr (PreferAlarmCheckbox);
	if (CtlGetValue (ctl))
		{
		fld = GetObjectPtr (PreferAlarmField);
		AlarmPreset.advance = StrAToI (FldGetTextPtr (fld));

		lst = GetObjectPtr (PreferAlarmList);		
		AlarmPreset.advanceUnit = (AlarmUnitType) LstGetSelection (lst);
		}
	else
		AlarmPreset.advance = -1;			// no alarm is set

	
	// If the sound, or nag information is changed, all alarms posted
	// to the attention manager will need to be udpated after the prefs
	// are saved.
	updateAlarms = ((AlarmSoundRepeatCount != PrefSoundRepeatCount) ||
		 			(AlarmSoundRepeatInterval != PrefSoundRepeatInterval) ||
		 			(AlarmSoundUniqueRecID != PrefSoundUniqueRecID) );

	// Get the alarm sound and interval settings
	AlarmSoundRepeatCount = PrefSoundRepeatCount;
	AlarmSoundRepeatInterval = PrefSoundRepeatInterval;
	AlarmSoundUniqueRecID = PrefSoundUniqueRecID;
	
	// Save app's preferences -- this is needed in case alarm settings
	// have been changed and an alarm occurs before we leave the app, since
	// the alarm notification handlers cannot rely on globals and always get
	// fresh settings from app's preferences.
	DatebookSavePrefs ();
	
	if (updateAlarms)
		UpdatePostedAlarms(SoundRepeatChanged);

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesInit
 *
 * DESCRIPTION: This routine initializes the Preferences Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		08/03/95	Initial Revision
 *			trev		08/14/97	Added MIDI support for alarms
 *       frigino	08/20/97	Modified sound name trigger initialization
 *			frigino	08/21/97	Removed typecasts in MapToPosition
 *			vmk		12/09/97	Check gMidiListH for null before locking
 *			kwk		07/07/99	Use default sound name str to find default entry.
 *
 ***********************************************************************/
static void PreferencesInit (void)
{
	FormPtr		formP;
	Char *		label;
	Char *		textP;
	MemHandle		textH;
	FieldPtr		fld;
	ControlPtr	ctl;
	ListPtr		listP;
	UInt16			item;
	UInt16			i;
	UInt16			fldIndex;
	SndMidiListItemType*	midiListP;
	Char *		defaultName;
	
	formP = FrmGetActiveForm ();

	// Set the alarm preset values.
	if (AlarmPreset.advance != apptNoAlarm)
		{
		// Turn the preset checkbox on.
		CtlSetValue (GetObjectPtr (PreferAlarmCheckbox), true);
		
		// Set the alarm advance value.
		fld = GetObjectPtr (PreferAlarmField);
		textH = MemHandleNew (maxAdvanceFieldLen);
		textP = MemHandleLock (textH);
		StrIToA (textP, AlarmPreset.advance);
		MemPtrUnlock (textP);
		FldSetTextHandle (fld, textH);
		fldIndex = FrmGetObjectIndex (formP, PreferAlarmField);
		FrmShowObject (formP, fldIndex);
		FrmSetFocus (formP, fldIndex);
	
		// Set the alarm advance unit of measure (minutes, hours, or days).
		listP = GetObjectPtr (PreferAlarmList);		
		LstSetSelection (listP, AlarmPreset.advanceUnit);
		label = LstGetSelectionText (listP, AlarmPreset.advanceUnit);

		ctl = GetObjectPtr (PreferAlarmUnitTrigger);
		CtlSetLabel (ctl, label);
		FrmShowObject (formP, FrmGetObjectIndex (formP, PreferAlarmUnitTrigger));
		}
	
	// Set the Remind Me trigger and list
	listP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferRemindMeList));

	//	Convert the preference setting to it's UI list position:
	item = MapToPosition (RepeatCountMappings, PrefSoundRepeatCount,
								 numRepeats, defaultRepeatsLevel);
	LstSetSelection (listP, item);
	CtlSetLabel (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferRemindMeTrigger)),
											LstGetSelectionText (listP, item));

	// Set the Play Every trigger and list
	listP = FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferPlayEveryList));

	//	Convert the preference setting to it's UI list position:
	item = MapToPosition (RepeatIntervalMappings,
								PrefSoundRepeatInterval / minutesInSeconds,
								numRepeats, defaultPlayEveryLevel);
	LstSetSelection (listP, item);
	CtlSetLabel (FrmGetObjectPtr (formP, FrmGetObjectIndex (formP, PreferPlayEveryTrigger)),
											LstGetSelectionText (listP, item));
	
	listP = FrmGetObjectPtr(formP, FrmGetObjectIndex(formP, PreferAlarmSoundList));

	CreateMidiPickList(listP, MidiPickListDrawItem);

	// Traverse MIDI pick list and find the item whose unique ID matches our
	// saved unique ID and use its index as our list selection index. If we
	// don't find a match, then we want to use the MIDI sound that corresponds
	// to our default sound name; if that's not found, use item 0.

	// Default to first sound in list
	item = 0;

	// Lock MIDI sound list
	if ( gMidiListH )
		{
		midiListP = MemHandleLock(gMidiListH);
		defaultName = MemHandleLock(DmGetResource(strRsc, defaultAlarmSoundNameID));

		// Iterate through each item and get its unique ID
		for (i = 0; i < gMidiCount; i++)
			{
			if (midiListP[i].uniqueRecID == PrefSoundUniqueRecID)
				{
				item = i;
				break;		// exit for loop
				}
			else if (StrCompare(midiListP[i].name, defaultName) == 0)
				{
				item = i;
				}
			}
		
		MemPtrUnlock(defaultName);
		
		// Set the list selection
		LstSetSelection (listP, item);

		// Init the sound trigger label
		// Create a new ptr to hold the label
		soundTriggerLabelP = MemPtrNew(soundTriggerLabelLen);
		// Check for mem failure
		ErrFatalDisplayIf(soundTriggerLabelP == NULL, "Out of memory");
		// Set the trigger label
		SetSoundLabel(formP, midiListP[item].name);

		// Unlock MIDI sound list
		MemPtrUnlock(midiListP);
		}

	PreferencesUpdateScrollers ();
}


/***********************************************************************
 *
 * FUNCTION:    SetSoundLabel
 *
 * DESCRIPTION: Sets the sound trigger label, using truncation
 *
 * PARAMETERS:  formP - the form ptr
 *              labelP - ptr to original label text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/18/97	Initial Revision
 *
 ***********************************************************************/

static void SetSoundLabel(FormPtr formP, const char* labelP)
{
	ControlPtr	triggerP;
	UInt16			triggerIdx;

	// Copy the label, winUp to the max into the ptr
	StrNCopy(soundTriggerLabelP, labelP, soundTriggerLabelLen);
	// Terminate string at max len
	soundTriggerLabelP[soundTriggerLabelLen - 1] = '\0';
	// Get trigger idx
	triggerIdx = FrmGetObjectIndex(formP, PreferAlarmSoundTrigger);
	// Get trigger control ptr
	triggerP = FrmGetObjectPtr(formP, triggerIdx);
	// Use category routines to truncate it
	CategoryTruncateName(soundTriggerLabelP, ResLoadConstant(soundTriggerLabelWidth));
	// Set the label
	CtlSetLabel(triggerP, soundTriggerLabelP);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Preferences
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		8/3/95	Initial Revision
 *			trev		8/14/97	Added MIDI support for alarms
 *			frigino	9/9/97	Added PlayAlarmSound call when selecting new
 *									alarm sound from popup list
 *			vmk		12/9/97	Added initialization of alarm prefs in frmOpenEvent
 *			gap		10/15/99	Added added handling of frmUpdateEvent
 *
 ***********************************************************************/
Boolean PreferencesHandleEvent (EventType * event)
{
	UInt16 updateCode;
	FormPtr frm = 0;
	Boolean handled = false;
	SndMidiListItemType*	listP;
	UInt16 item;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case PreferOkButton:
				updateCode = PreferencesApply (PrefDayStartHour, PrefDayEndHour);
				FrmReturnToForm (0);
				if (updateCode)
					FrmUpdateForm (FrmGetFormId (FrmGetActiveForm()), updateCode);
				handled = true;
				// Free all data before we return to underlying form
				FreeAll();
				break;

			case PreferCancelButton:
				FrmReturnToForm (0);
				handled = true;
				// Free all data before we return to underlying form
				FreeAll();
				break;
			
			case PreferAlarmCheckbox:
				PreferencesAlarmPresetOnOff (event->data.ctlSelect.on);
				handled = true;
				break;
			}
		}

	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case PreferStartDownButton:
				if (PrefDayStartHour > 0)
					PrefDayStartHour--;
				PreferencesDrawTime (PrefDayStartHour, true);
				break;

			case PreferStartUpButton:
				if (PrefDayStartHour < 23)
					PrefDayStartHour++;
				PreferencesDrawTime (PrefDayStartHour, true);
				if (PrefDayEndHour < PrefDayStartHour)
					{
					PrefDayEndHour = PrefDayStartHour;
					PreferencesDrawTime (PrefDayEndHour, false);
					}
				break;

			case PreferEndDownButton:
				if (PrefDayEndHour > 0)
					PrefDayEndHour--;
				if (PrefDayEndHour < PrefDayStartHour)
					{
					PrefDayStartHour = PrefDayEndHour;
					PreferencesDrawTime (PrefDayStartHour, true);
					}
				PreferencesDrawTime (PrefDayEndHour, false);				
				break;

			case PreferEndUpButton:
				if (PrefDayEndHour < 23)
					PrefDayEndHour++;
				PreferencesDrawTime (PrefDayEndHour, false);
				break;
			}
		PreferencesUpdateScrollers ();
		}

	#if WRISTPDA
	else if	(event->eType == keyDownEvent) {
		EventType newEvent;
		frm = FrmGetActiveForm ();
		MemSet( & newEvent, sizeof( EventType ), 0 );
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to an Ok button event.
			newEvent.eType = ctlSelectEvent;
			newEvent.data.ctlSelect.controlID = PreferOkButton;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to an Ok button event.
			newEvent.eType = ctlSelectEvent;
			newEvent.data.ctlSelect.controlID = PreferCancelButton;
			newEvent.data.ctlSelect.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
			// Translate the RockerUp key to a PreferStartUpButton event.
			newEvent.eType = ctlRepeatEvent;
			newEvent.data.ctlRepeat.controlID = PreferStartUpButton;
			newEvent.data.ctlRepeat.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
			// Translate the RockerDown key to a PreferStartDownButton event.
			newEvent.eType = ctlRepeatEvent;
			newEvent.data.ctlRepeat.controlID = PreferStartDownButton;
			newEvent.data.ctlRepeat.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrPageUp ) {
			// Translate the PageUp key to a PreferEndUpButton event.
			newEvent.eType = ctlRepeatEvent;
			newEvent.data.ctlRepeat.controlID = PreferEndUpButton;
			newEvent.data.ctlRepeat.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000005, true );
			return true;
		} else if ( event->data.keyDown.chr == vchrPageDown ) {
			// Translate the PageDown key to a PreferEndDownButton event.
			newEvent.eType = ctlRepeatEvent;
			newEvent.data.ctlRepeat.controlID = PreferEndDownButton;
			newEvent.data.ctlRepeat.pControl =
				FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlRepeat.controlID));
			EvtAddUniqueEventToQueue( &newEvent, 0x00000006, true );
			return true;
		}
		if (!EvtKeydownIsVirtual(event))
		{
			WChar chr = event->data.keyDown.chr;
			if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
				{
				// Redirect numeric input to the Alarm Preset input field
				FldHandleEvent (GetObjectPtr (PreferAlarmField), event);
				}
			handled = true;
		}
	}
	#else
	else if ((event->eType == keyDownEvent)
	&& (!EvtKeydownIsVirtual(event)))
		{
		WChar chr = event->data.keyDown.chr;
		if (TxtCharIsDigit (chr) || TxtCharIsCntrl (chr))
			{
			// Redirect numeric input to the Alarm Preset input field
			FldHandleEvent (GetObjectPtr (PreferAlarmField), event);
			}
		handled = true;
		}
	#endif

	else if (event->eType == frmOpenEvent)
		{
		PrefDayStartHour = DayStartHour;
		PrefDayEndHour = DayEndHour;
		PrefSoundRepeatCount = AlarmSoundRepeatCount;
		PrefSoundRepeatInterval = AlarmSoundRepeatInterval;
		PrefSoundUniqueRecID = AlarmSoundUniqueRecID;

		frm = FrmGetActiveForm ();
		PreferencesInit ();
		FrmDrawForm (frm);
		PreferencesDrawTime (DayStartHour, true);
		PreferencesDrawTime (DayEndHour, false);

		handled = true;
		}
		
	else if (event->eType == frmUpdateEvent)
		{
		frm = FrmGetActiveForm ();
		FrmDrawForm (frm);
		PreferencesDrawTime (DayStartHour, true);
		PreferencesDrawTime (DayEndHour, false);
		handled = true;
		}
		
	else if (event->eType == popSelectEvent)
		{
		switch (event->data.popSelect.listID)
			{
			case PreferRemindMeList:
				item = event->data.popSelect.selection;
				
				PrefSoundRepeatCount = RepeatCountMappings[item];
				
				break;
			case PreferPlayEveryList:
				item = event->data.popSelect.selection;
				
				PrefSoundRepeatInterval = RepeatIntervalMappings[item] * minutesInSeconds;
				
				break;	
			case PreferAlarmSoundList:
				// Get new selected item
				item = event->data.popSelect.selection;
				// Get active form
				frm = FrmGetActiveForm ();
				// Lock MIDI list
				listP = MemHandleLock(gMidiListH);
				// Save alarm sound unique rec ID
				PrefSoundUniqueRecID = listP[item].uniqueRecID;
				// Erase control
				CtlEraseControl (event->data.popSelect.controlP);
				// Set new trigger label
				SetSoundLabel(frm, listP[item].name);
				// Redraw control
				CtlDrawControl (event->data.popSelect.controlP);
				// Unlock MIDI list
				MemPtrUnlock(listP);
				// Play new alarm sound
				PlayAlarmSound (PrefSoundUniqueRecID);
				// Mark event as handled
				handled = true;
				break;
			}
		}	

	else if (event->eType == frmCloseEvent)
		{
		// Free all data before we return to underlying form
		FreeAll();
		}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    FreeAll
 *
 * DESCRIPTION: Frees all data allocated for the duration of the dialog
 *
 * PARAMETERS:  none
 *
 * RETURNED:    none
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/20/97	Initial Revision
 *			vmk		12/9/97	Check soundTriggerLabelP for null before deleting
 *
 ***********************************************************************/
static void FreeAll(void)
{
	// Free the MIDI pick list
	FreeMidiPickList();

	// Free the sound trigger label placeholder
	if ( soundTriggerLabelP )
		{
		MemPtrFree(soundTriggerLabelP);
		soundTriggerLabelP = NULL;
		}
}
