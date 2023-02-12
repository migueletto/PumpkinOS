/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressLookup.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *     This is the Address Book's Lookup module.  This module
 *   handles looking up address book information for other apps.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include <TraceMgr.h>

#include <PalmUtils.h>

#include "sec.h"

#include "AddressLookup.h"
#include "AddressDB.h"
#include "AddrTools.h"
#include "AddressRsc.h"
#include "AddrDefines.h"

#ifdef PALMOS
typedef UInt32 UIntPtr;
#endif

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Address lookup table columns
#define field1Column						0
#define field2Column						1

#if		WRISTPDA
#define	kFontToUse						FossilBoldFont
#endif//	WRISTPDA


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static Boolean			PrvLookupViewHandleEvent (LookupVariablesPtr vars);
static void				PrvLookupViewInit (LookupVariablesPtr vars);
static void				PrvLookupDrawRecordFields (LookupVariablesPtr vars, AddrDBRecordPtr record, Int16 field, RectanglePtr bounds, Char * phoneLabelLetters);
static void				PrvLookupViewDrawRecord (void * table, Int16 row, Int16 column, RectanglePtr bounds);
static void				PrvLookupViewUpdateScrollButtons (LookupVariablesPtr vars);
static void				PrvLookupLoadTable (LookupVariablesPtr vars);
static void				PrvLookupViewScroll (LookupVariablesPtr vars, WinDirectionType direction, Boolean oneLine);
static void				PrvLookupViewSelectRecord (LookupVariablesPtr vars, UInt16 recordNum, UInt16 phoneNum);
static Boolean			PrvLookupViewLookupString (LookupVariablesPtr vars, EventType * event);
static void				PrvLookupClearLookupString ();
static Boolean			PrvLookupViewUseSelection (LookupVariablesPtr vars);
static AddressFields	PrvLookupFindPhoneField (LookupVariablesPtr vars, AddrDBRecordPtr recordP, AddressLookupFields lookupField, UInt16 phoneNum);
static Char* 			PrvLookupResizeResultString (MemHandle resultStringH, UInt16 newSize);
static MemHandle		PrvLookupCreateResultString (LookupVariablesPtr vars, UInt16 recordNum, UInt16 phoneNum);


/***********************************************************************
 *
 * FUNCTION:    Lookup
 *
 * DESCRIPTION: Present a list of records for the user to select and return
 * a string formatted to include information from the selected record.
 *
 * PARAMETERS:    params - address lookup launch command parameters
 *
 * RETURNED:    nothing
 *                The params will contain a string for the matching record.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/1/96   Initial Revision
 *			  meg		2/3/99	 added beep param to vars struct...default to true
 *			  peter	09/20/00	 Disable attention indicator because title is custom.
 ***********************************************************************/
void Lookup(AddrLookupParamsType * params)
{
	Err err;
	DmOpenRef dbP;
	AddrAppInfoPtr appInfoPtr;
	UInt16      cardNo=0;
	LocalID   dbID;
	DmSearchStateType   searchState;
	FormPtr frm;
	FormPtr originalForm;
	LookupVariablesType vars;
	Boolean uniqueMatch;
	Boolean completeMatch;
	UInt16 mode;

	TraceOutput(TL(appErrorClass,"Lookup() - title = %s, pasteButtonText = %s, lookupString = %s, field1 = %hu, field2 = %hu, field2Optional = %hu, userShouldInteract = %hu, formatStringP = %s, resultStringH = %lu, uniqueID = %lu", params->title, params->pasteButtonText, params->lookupString, params->field1, params->field2, params->field2Optional, params->userShouldInteract, params->formatStringP, params->resultStringH, params->uniqueID));
	
	// Check the parameters
	ErrFatalDisplayIf(params->field1 > addrLookupListPhone &&
					  params->field1 != addrLookupNoField, "Bad Lookup request - field1");
	ErrFatalDisplayIf(params->field2 > addrLookupListPhone &&
					  params->field2 != addrLookupNoField, "Bad Lookup request - field2");


	// Find the application's data file.
	err = DmGetNextDatabaseByTypeCreator (true, &searchState, addrDBType,
										  AddressBookCreator, true, &cardNo, &dbID);
	if (err)
	{
		params->resultStringH = 0;
		return;
	}

	// Obey the secret records setting.  Also, we only need to
	// read from the database.
	if (PrefGetPreference(prefHidePrivateRecordsV33))
		mode = dmModeReadOnly;
	else
		mode = dmModeReadOnly | dmModeShowSecret;

	// Open the address database.
	dbP = DmOpenDatabase(cardNo, dbID, mode);
	if (! dbP)
	{
		params->resultStringH = 0;
		params->uniqueID = 0;
		return;
	}

	// Initialize some of the lookup variables (those needed)
	vars.params = params;
	vars.dbP = dbP;
	vars.beepOnFail = true;

	// Find how the database is sorted.
	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
	vars.sortByCompany = appInfoPtr->misc.sortByCompany;
	ToolsInitPhoneLabelLetters(appInfoPtr, vars.phoneLabelLetters);
	MemPtrUnlock(appInfoPtr);

	// Set the mappings from AddressLookupFields to AddressFields.  It is
	// necessary to initialize the mappings at runtime because static
	// global variables are not available to this launch code routine.
	vars.lookupFieldMap[addrLookupName] = ad_name;
	vars.lookupFieldMap[addrLookupFirstName] = ad_firstName;
	vars.lookupFieldMap[addrLookupCompany] = ad_company;
	vars.lookupFieldMap[addrLookupAddress] = ad_address;
	vars.lookupFieldMap[addrLookupCity] = ad_city;
	vars.lookupFieldMap[addrLookupState] = ad_state;
	vars.lookupFieldMap[addrLookupZipCode] = ad_zipCode;
	vars.lookupFieldMap[addrLookupCountry] = ad_country;
	vars.lookupFieldMap[addrLookupTitle] = ad_title;
	vars.lookupFieldMap[addrLookupCustom1] = ad_custom1;
	vars.lookupFieldMap[addrLookupCustom2] = ad_custom2;
	vars.lookupFieldMap[addrLookupCustom3] = ad_custom3;
	vars.lookupFieldMap[addrLookupCustom4] = ad_custom4;
	vars.lookupFieldMap[addrLookupNote] = ad_note;

	// Check to see if the lookup string is sufficient for a unique match.
	// If so we skip presenting the user a lookup dialog and just use the match.
	AddrDBLookupLookupString(vars.dbP, params->lookupString, vars.sortByCompany,
						   vars.params->field1, vars.params->field2, &vars.currentRecord, &vars.currentPhone,
						   vars.lookupFieldMap, &completeMatch, &uniqueMatch);
	if (completeMatch && uniqueMatch)
	{
		PrvLookupCreateResultString(&vars, vars.currentRecord, vars.currentPhone + firstPhoneField);
		goto Exit;
	}

	// If the user isn't allowed to select a record then return without
	// a match.
	if (!params->userShouldInteract)
	{
		params->resultStringH = 0;
		params->uniqueID = 0;
		goto Exit;
	}


	// Initialize more of the lookup variables
	vars.currentRecord = noRecord;
	vars.currentPhone = 0;
	vars.topVisibleRecord = 0;
	vars.topVisibleRecordPhone = 0;
	vars.hideSecretRecords = PrefGetPreference(prefHidePrivateRecordsV33);


	// Custom title doesn't support attention indicator.
	// Disable indicator before switching forms.
	AttnIndicatorEnable(false);

	// Remember the original form
	originalForm =  FrmGetActiveForm();

	// Initialize the dialog.
	frm = FrmInitForm (LookupView);
	vars.frm = frm;

	// Set the title
	if (params->title)
		FrmSetTitle(frm, params->title);

	// Set the paste button
	if (params->pasteButtonText)
		CtlSetLabel (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, LookupPasteButton)), params->pasteButtonText);

	FrmSetActiveForm (frm);

	PrvLookupViewInit (&vars);

	// Enter the lookup string
	if (/*params->lookupString &&*/ *params->lookupString != '\0')
	{
		FldInsert (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, LookupLookupField)), params->lookupString, StrLen(params->lookupString));
		PrvLookupViewLookupString(&vars, NULL);
	}

	FrmDrawForm (frm);

	FrmSetFocus (frm, FrmGetObjectIndex (frm, LookupLookupField));

	// Handle events until the user picks a record or cancels
	if (PrvLookupViewHandleEvent (&vars))
	{
		PrvLookupCreateResultString(&vars, vars.currentRecord, vars.currentPhone);
	}
	else
	{
		params->resultStringH = 0;
		params->uniqueID = 0;
	}

	AttnIndicatorEnable(true);		// Custom title doesn't support attention indicator.

	FrmSetFocus (frm, noFocus);
	FrmEraseForm (frm);
	FrmDeleteForm (frm);
	FrmSetActiveForm (originalForm);


Exit:
	DmCloseDatabase (dbP);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewHandleEvent
 *
 * DESCRIPTION: This is the event handler for the "Lookup View"
 *              of the Address Book application.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if a record was selected and false if not.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/8/96   Initial Revision
 *			CSS		06/22/99	Standardized keyDownEvent handling
 *									(TxtCharIsHardKey, commandKeyMask, etc.)
 *
 ***********************************************************************/
Boolean PrvLookupViewHandleEvent (LookupVariablesPtr vars)
{
	EventType event;
	Boolean handled;


	while (true)
	{
		EvtGetEvent (&event, evtWaitForever);

		// Cancel if something is going to switch apps.
		if ( (event.eType == keyDownEvent)
			 &&	(!TxtCharIsHardKey(	event.data.keyDown.modifiers, event.data.keyDown.chr))
			 &&	(EvtKeydownIsVirtual(&event))
			 &&	(event.data.keyDown.chr == vchrFind))
		{
			EvtAddEventToQueue(&event);
			return false;
		}

		if (SysHandleEvent (&event))
			continue;


		handled = false;

		// Clear the lookup string because the user is selecting an item.
		if (event.eType == appStopEvent)
		{
			EvtAddEventToQueue(&event);
			return false;
		}

		else if (event.eType == tblSelectEvent)
		{
			// Doing the next call sends a fldChangedEvent which remove's the
			// table's selection.  Set a flag to not handle the event.
			PrvLookupClearLookupString ();
			vars->ignoreEmptyLookupField = true;

			handled = true;
		}

		else if (event.eType == ctlSelectEvent)
		{
			if (event.data.ctlSelect.controlID == LookupPasteButton)
				return PrvLookupViewUseSelection(vars);

			else if (event.data.ctlSelect.controlID == LookupCancelButton)
				return false;
		}

		else if (event.eType == ctlRepeatEvent)
		{
			if (event.data.ctlRepeat.controlID == LookupUpButton)
			{
				PrvLookupViewScroll (vars, winUp, false);
				PrvLookupClearLookupString ();
				// leave unhandled so the buttons can repeat
			}

			else if (event.data.ctlRepeat.controlID == LookupDownButton)
			{
				PrvLookupViewScroll (vars, winDown, false);
				PrvLookupClearLookupString ();
				// leave unhandled so the buttons can repeat
			}
		}

		else if (event.eType == keyDownEvent)
		{
#if		WRISTPDA
			if (event.data.keyDown.chr == vchrThumbWheelPush)
			{
				PrvLookupClearLookupString ();
				return PrvLookupViewUseSelection(vars);
			}
			else if (event.data.keyDown.chr == vchrThumbWheelBack)
			{	// Just like tapping Cancel
				return false;
			}
#endif//	WRISTPDA
/*
#if		WRISTPDA
			#warning this code does not work!
			if ((event.data.keyDown.chr == vchrThumbWheelUp)
			 || (event.data.keyDown.chr == vchrThumbWheelDown))
			{
				Int16			row, moveY = 0;
				Int16			column;
				TableType*	tableP;

				tableP = ToolsGetObjectPtr (LookupTable);
				if (TblGetSelection(tableP, &row, &column))
				{
					if (event.data.keyDown.chr == vchrThumbWheelUp)
					{
						if (row)	moveY = -1;
					}
					else
					{
						if (row < TblGetNumberOfRows( tableP ))	moveY = +1;
					}
					if (moveY)
					{
						Int16		topRow;
						Int16 	topRecord = vars->topVisibleRecord;
						Int16		numRows = TblGetNumberOfRows(tableP);

						PrvLookupClearLookupString();
						topRow = TblGetTopRow( tableP );

						#warning this code does not work!
						if (moveY > 0)
						{
							if (row -topRow < numRows /2)
							{
								row += moveY;
							}
							else
							{
								PrvLookupViewScroll(vars, winDown, true);
							}
						}
						else
						{
							if (row -topRow >= numRows /2)
							{
								row += moveY;
							}
							else
							{
								PrvLookupViewScroll(vars, winUp, true);
							}
						}
						TblSelectItem(tableP, row, 0);
					}
				}
			}
#endif//	WRISTPDA
*/
			if (TxtCharIsHardKey(event.data.keyDown.modifiers, event.data.keyDown.chr))
			{
				// SysHandleEvent saw these keys and is now switching apps.
				// Leave the Lookup function.
				return false;
			}
			else if (EvtKeydownIsVirtual(&event))
			{
				if (event.data.keyDown.chr == vchrPageUp)
				{
					PrvLookupViewScroll (vars, winUp, false);
					PrvLookupClearLookupString ();
					handled = true;
				}

				else if (event.data.keyDown.chr == vchrPageDown)
				{
					PrvLookupViewScroll (vars, winDown, false);
					PrvLookupClearLookupString ();
					handled = true;
				}

				else
				{
					handled = false;
				}
			}

			else if (event.data.keyDown.chr == chrLineFeed)
			{
				return PrvLookupViewUseSelection(vars);
			}

			else
			{
				//user entered a new char...beep if it's not valid
				vars->beepOnFail = true;
				handled = PrvLookupViewLookupString(vars, &event);

				// If the field becomes empty, handle it.
				vars->ignoreEmptyLookupField = false;
			}
		}

		else if (event.eType == fldChangedEvent)
		{
			if (!(vars->ignoreEmptyLookupField &&
				  FldGetTextLength(FrmGetObjectPtr (vars->frm,
													FrmGetObjectIndex(vars->frm, LookupLookupField))) == 0))
			{
				// dont set the beep...this way, if the calling app sent in a multiple char
				// string, we only beep once even if we will remove all chars from the lookup field
				PrvLookupViewLookupString(vars, NULL);
			}

			vars->ignoreEmptyLookupField = false;
			handled = true;
		}

		// Check if the form can handle the event
		if (!handled)
			FrmHandleEvent (vars->frm, &event);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewInit
 *
 * DESCRIPTION: This routine initializes the "Lookup View" of the
 *              Address application.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupViewInit (LookupVariablesPtr vars)
{
	UInt16 row;
	UInt16 rowsInTable;
	TablePtr table;
	RectangleType bounds;
#if		WRISTPDA
	UInt16			rowHeight;
	FontID			saveFont;
#endif//	WRISTPDA

	// Initialize the address list table.
	table = FrmGetObjectPtr (vars->frm, FrmGetObjectIndex (vars->frm, LookupTable));
	rowsInTable = TblGetNumberOfRows (table);

#if		WRISTPDA
	saveFont = FntSetFont( kFontToUse );
	rowHeight = FntLineHeight();
	FntSetFont( saveFont );
#endif//	WRISTPDA

	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle (table, row, field1Column, tallCustomTableItem);
		TblSetRowUsable (table, row, false);

#if		WRISTPDA
		TblSetRowHeight (table, row, rowHeight );
#endif//	WRISTPDA
	}

	TblSetColumnUsable (table, 0, true);


	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure (table, field1Column, PrvLookupViewDrawRecord);


	// Load records into the address list.
	PrvLookupLoadTable (vars);


	// Turn on the cursor in the lookup field.
	//   FrmSetFocus(vars->frm, FrmGetObjectIndex (vars->frm, LookupLookupField));

	// Set the bounds of the title, so that the title will draw across the
	// entire display.
	FrmGetFormBounds(vars->frm, &bounds);
	// Note: '0' should be FrmGetObjectIndex (vars->frm, LookupTitle), but that fails
	// because frmTitleObjects do not have the resource ID stored in them.
	FrmSetObjectBounds (vars->frm, 0, &bounds);

	// Respond to an empty lookup field.
	vars->ignoreEmptyLookupField = false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupDrawRecordFields
 *
 * DESCRIPTION: Draws the name and phone number (plus which phone)
 * within the screen bounds passed.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              record - record to draw
 *              field - phone to draw
 *              bounds - bounds of the draw region
 *              phoneLabelLetters - the first letter of each phone label
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
void PrvLookupDrawRecordFields(LookupVariablesPtr vars, AddrDBRecordPtr record, Int16 field, RectanglePtr bounds, Char * phoneLabelLetters)
{
	Char * name1;
	Char * name2;
	Char * field1;
	Char * field2;
	Int16 x;
	Int16 y;
	Int16 field1Length;
	Int16 field2Length;
	Int16 field1Width;
	Int16 field2Width;
	Int16 horizontalSpace;
	UInt16 phoneLabel;
	Int16 fieldSeparatorWidth;
	Int16 shortenedFieldWidth;
	UInt16 field1Num;
	UInt16 field2Num;
	Int16 name1Length;
	Int16 name2Length;
	Int16 name1Width;
	Int16 name2Width;
	Boolean ignored;
	Boolean everythingFits;
	Boolean name1HasPriority = true;
	UInt8 phoneLabelWidth;

	x = bounds->topLeft.x;
	y = bounds->topLeft.y;

	phoneLabelWidth = FntCharWidth('W') - 1;		// remove the blank trailing column

	TraceOutput(TL(appErrorClass,"PrvLookupDrawRecordFields() - *"));

	// Do we need to figure out the person's name?
	if (vars->params->field1 == addrLookupSortField || vars->params->field2 == addrLookupSortField)
		name1HasPriority = ToolsDetermineRecordName (record, &shortenedFieldWidth, &fieldSeparatorWidth, vars->sortByCompany, &name1, &name1Length, &name1Width, &name2, &name2Length, &name2Width, NULL, NULL, bounds->extent.x);

	TraceOutput(TL(appErrorClass,"PrvLookupDrawRecordFields() - **"));

	// Handle the first field to display
	field1Num = vars->params->field1;
	if (field1Num == addrLookupSortField)
		field1Width = name1Width + (name2 ? fieldSeparatorWidth : 0) + name2Width;
	else if (field1Num == addrLookupNoField)
	{
		field1Length = 0;
		field1Width = 0;
	}
	else
	{
		// Map from the lookup field requested to the Address's format
		if (IsPhoneLookupField(field1Num))
		{
			field1 = record->fields[field];
		}
		else if (field1Num == addrLookupListPhone)
		{
			field1 = record->fields[ad_phone1 + record->options.phones.displayPhoneForList];
		}
		else
		{
			field1 = record->fields[vars->lookupFieldMap[field1Num]];
		}

		if (field1)
		{
			// Only show text from the first line in the field
			field1Width = bounds->extent.x;
			field1Length = field1Width;      // more characters than we can expect
			FntCharsInWidth (field1, &field1Width, &field1Length, &ignored);//lint !e64

			// Leave room for a letter to indicate which phone is being displayed.
			if (field1Num == addrLookupListPhone)
				field1Width += phoneLabelWidth + 1;
		}
		else
		{
			field1Length = 0;
			field1Width = 0;
		}
	}


	// Handle the second field to display
	field2Num = vars->params->field2;
	if (field2Num == addrLookupSortField)
	{
		// This is the width of the name before any truncation.
		field2Width = name1Width + (name2 ? fieldSeparatorWidth : 0) + name2Width;
	}
	else if (field2Num == addrLookupNoField)
	{
		field2Length = 0;
		field2Width = 0;
	}
	else
	{
		// Map from the lookup field requested to the Address's format
		if (IsPhoneLookupField(field2Num))
		{
			field2 = record->fields[field];
		}
		else if (field2Num == addrLookupListPhone)
		{
			field2 = record->fields[ad_phone1 + record->options.phones.displayPhoneForList];
		}
		else
		{
			field2 = record->fields[vars->lookupFieldMap[field2Num]];
		}

		if (field2)
		{
			// Only show text from the first line in the field
			field2Width = bounds->extent.x;
			field2Length = field2Width;      // more characters than we can expect
			FntCharsInWidth (field2, &field2Width, &field2Length, &ignored);//lint !e64

			// Leave room for a letter to indicate which phone is being displayed.
			if (vars->params->field2 == addrLookupListPhone)
				field2Width += phoneLabelWidth + 1;
		}
		else
		{
			field2Length = 0;
			field2Width = 0;
		}
	}



	// Now check if everything can display without any truncation.
	everythingFits = bounds->extent.x >= field1Width +
		((field1Width && field2Width) ? spaceBetweenNamesAndPhoneNumbers : 0) +
		field2Width;
	/*   if (bounds->extent.x   >= name1Width + (name2 ? fieldSeparatorWidth : 0) +
	 name2Width + (phone ? spaceBetweenNamesAndPhoneNumbers : 0) + phoneWidth)
	 */      {
	 }

	 // Find out how much room the left side can use
	 if (everythingFits || field2Num == addrLookupNoField)
		 horizontalSpace = bounds->extent.x;
	 else
	 {
		 horizontalSpace = bounds->extent.x - spaceBetweenNamesAndPhoneNumbers;

		 // This allows the left side to use space on the right side if the
		 // right side is small enough.
		 horizontalSpace -= min(horizontalSpace / 2, field2Width);
	 }
	 x = bounds->topLeft.x;


	 // Now draw the left side.
	 if (vars->params->field1 == addrLookupSortField)
	 {
		 ToolsDrawRecordName (name1, name1Length, name1Width, name2, name2Length, name2Width,
						 horizontalSpace, &x, y, shortenedFieldWidth, fieldSeparatorWidth, false,
						 name1HasPriority || !vars->sortByCompany, false);

		 // Remember how much width was actually used for field1
		 field1Width = x - bounds->topLeft.x;
	 }
	 else if (vars->params->field1 == addrLookupListPhone)
	 {
		 if (field1Width > horizontalSpace)
		 {
			 everythingFits = false;
			 field1Width = horizontalSpace - phoneLabelWidth - 1 - shortenedFieldWidth;

			 FntCharsInWidth(field1, &field1Width, &field1Length, &ignored);
		 }
		 else
		 {
			 everythingFits = true;
			 field1Width -= phoneLabelWidth + 1;
		 }

		 x = bounds->topLeft.x;
		 if (!everythingFits)
			 x -= shortenedFieldWidth;
		 WinDrawChars(field1, field1Length, x, y);
		 x = bounds->topLeft.x + field1Width;

		 if (!everythingFits)
		 {
			 WinDrawChars(shortenedFieldString, shortenedFieldLength,
						  x, y);
			 x += shortenedFieldWidth;
		 }

		 // Draw the first letter of the phone field label
		 phoneLabel = GetPhoneLabel(record, firstPhoneField +
									record->options.phones.displayPhoneForList);
		 WinDrawChars (&phoneLabelLetters[phoneLabel], 1,
					   x + ((phoneLabelWidth - (FntCharWidth(phoneLabelLetters[phoneLabel]) - 1)) >> 1), //lint !e702
					   y);

		 // Remember how much width was actually used for field1
		 field1Width = x + phoneLabelWidth - bounds->topLeft.x;
	 }
	 else if (vars->params->field1 != addrLookupNoField)
	 {
		 if (field1Width > horizontalSpace)
		 {
			 everythingFits = false;
			 field1Width = horizontalSpace - shortenedFieldWidth;
			 FntCharsInWidth(field1, &field1Width, &field1Length, &ignored);//lint !e64
		 }
		 else
		 {
			 everythingFits = true;
		 }

		 x = bounds->topLeft.x;
		 WinDrawChars(field1, field1Length, x, y);
		 x += field1Width;

		 if (!everythingFits)
		 {
			 WinDrawChars(shortenedFieldString, shortenedFieldLength, x, y);
			 x += shortenedFieldWidth;
		 }

		 // Remember how much width was actually used for field1
		 field1Width = x - bounds->topLeft.x;
	 }


	 // Find out how much room the right side can use
	 horizontalSpace = bounds->extent.x - field1Width - spaceBetweenNamesAndPhoneNumbers;
	 x = bounds->topLeft.x + bounds->extent.x - field1Width;

	 // Now draw the right side.
	 if (vars->params->field2 == addrLookupSortField)
	 {
		 ToolsDrawRecordName (name1, name1Length, name1Width, name2, name2Length, name2Width,
						 horizontalSpace, &x, y, shortenedFieldWidth, fieldSeparatorWidth, false,
						 name1HasPriority || !vars->sortByCompany, false);
	 }
	 else if (vars->params->field2 == addrLookupListPhone)
	 {
		 if (field2Width > horizontalSpace)
		 {
			 everythingFits = false;
			 field2Width = horizontalSpace - phoneLabelWidth - 1 - shortenedFieldWidth;

			 FntCharsInWidth(field2, &field2Width, &field2Length, &ignored);//lint !e64
		 }
		 else
		 {
			 everythingFits = true;
			 field2Width -= phoneLabelWidth + 1;
		 }


		 x = bounds->topLeft.x + (bounds->extent.x - 1) - field2Width - phoneLabelWidth - 1;
		 if (!everythingFits)
			 x -= shortenedFieldWidth;
		 WinDrawChars(field2, field2Length, x, y);

		 if (!everythingFits)
		 {
			 WinDrawChars(shortenedFieldString, shortenedFieldLength,
						  bounds->topLeft.x + (bounds->extent.x - 1) - shortenedFieldWidth -
						  phoneLabelWidth - 1, y);
		 }

		 // Draw the first letter of the phone field label
		 phoneLabel = GetPhoneLabel(record, firstPhoneField +
									record->options.phones.displayPhoneForList);
		 WinDrawChars (&phoneLabelLetters[phoneLabel], 1,
					   bounds->topLeft.x + (bounds->extent.x - 1) - phoneLabelWidth +
					   ((phoneLabelWidth - (FntCharWidth(phoneLabelLetters[phoneLabel]) - 1)) >> 1), //lint !e702
					   y);
	 }
	 else if (vars->params->field2 != addrLookupNoField)
	 {
		 if (field2Width > horizontalSpace)
		 {
			 everythingFits = false;
			 field2Width = horizontalSpace - shortenedFieldWidth;
			 FntCharsInWidth(field2, &field2Width, &field2Length, &ignored);//lint !e64
		 }
		 else
		 {
			 everythingFits = true;
		 }

		 x = bounds->topLeft.x + bounds->extent.x - field2Width;
		 if (!everythingFits)
			 x -= shortenedFieldWidth;
		 WinDrawChars(field2, field2Length, x, y);

		 if (!everythingFits)
			 WinDrawChars(shortenedFieldString, shortenedFieldLength, bounds->topLeft.x + bounds->extent.x - shortenedFieldWidth, y);
	 }
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewDrawRecord
 *
 * DESCRIPTION: This routine draws an address book record.  It is called as
 *              a callback routine by the table object.
 *
 * PARAMETERS:  table  - pointer to the address list table
 *              row    - row number, in the table, of the item to draw
 *              column - column number, in the table, of the item to draw
 *              bounds - bounds of the draw region
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupViewDrawRecord (void * table, Int16 row, Int16 column, RectanglePtr bounds)
{
	UInt16 recordNum;
	Int16 fieldNum;
	Err error;
	AddrDBRecordType record;
	MemHandle recordH;
	LookupVariablesPtr vars;

	TraceOutput(TL(appErrorClass, "PrvLookupViewDrawRecord() - table = %lu, row = %hu, column = %hu",table, row, column));
	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	//
	recordNum = TblGetRowID (table, row);
	vars = (LookupVariablesPtr) TblGetRowData(table, row);

	error = AddrDBGetRecord (vars->dbP, recordNum, &record, &recordH);
	ErrNonFatalDisplayIf ((error), "Record not found");
	if (error) return;

	fieldNum = TblGetItemInt(table, row, column);

	#if WRISTPDA
	FntSetFont (kFontToUse);
	#else
	FntSetFont (stdFont);
	#endif

	PrvLookupDrawRecordFields (vars, &record, fieldNum, bounds, vars->phoneLabelLetters);

	MemHandleUnlock(recordH);
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewUpdateScrollButtons
 *
 * DESCRIPTION: Show or hide the list view scroll buttons.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupViewUpdateScrollButtons (LookupVariablesPtr vars)
{
	Int16   row;
	UInt16 upIndex;
	UInt16 downIndex;
	UInt16 recordNum;
	Int16 phoneNum;
	Boolean scrollableUp;
	Boolean scrollableDown;
	TablePtr table;


	// Update the buttons that scroll the list.
	//
	// If the first record displayed is not the fist record in the category,
	// enable the up scroller.
	recordNum = vars->topVisibleRecord;
	phoneNum = vars->topVisibleRecordPhone;
	scrollableUp = AddrDBLookupSeekRecord (vars->dbP, &recordNum, &phoneNum, 1, dmSeekBackward,
										 vars->params->field1, vars->params->field2, vars->lookupFieldMap);


	// Find the record in the last row of the table
	table = ToolsGetObjectPtr (LookupTable);
	for (row = TblGetNumberOfRows (table) - 1; row >= 0; row--)
	{
		// Make the row usable.
		if (TblRowUsable (table, row))
		{
			recordNum = TblGetRowID (table, row);
			phoneNum = TblGetItemInt (table, row, 0);
			break;
		}
	}


	// If the last record displayed is not the last record in the category,
	// enable the down scroller.
	scrollableDown = AddrDBLookupSeekRecord (vars->dbP, &recordNum, &phoneNum, 1, dmSeekForward,
										   vars->params->field1, vars->params->field2, vars->lookupFieldMap);


	// Update the scroll button.
	upIndex = FrmGetObjectIndex (vars->frm, LookupUpButton);
	downIndex = FrmGetObjectIndex (vars->frm, LookupDownButton);
	FrmUpdateScrollers (vars->frm, upIndex, downIndex, scrollableUp, scrollableDown);

}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupLoadTable
 *
 * DESCRIPTION: This routine loads address book database records into
 *              the lookup view form.  Note that the phone field may
 *                be set to the first or last field if it isn't field1 or
 *                field2 and the table is loaded either forward or backward.
 *                So ignore it if phone are not being displayed.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupLoadTable (LookupVariablesPtr vars)
{
	UInt16      row;
	UInt16      numRows;
	UInt16      recordNum;
	Int16      phoneNum;
	TablePtr table;
	Boolean searchPhones;


	// For each row in the table, store the record number as the row id.
	table = ToolsGetObjectPtr (LookupTable);


	// Make sure we haven't scrolled too far down the list of records
	// leaving blank lines in the table.

	// Try going forward to the last record that should be visible
	numRows = TblGetNumberOfRows (table);
	recordNum = vars->topVisibleRecord;
	phoneNum = vars->topVisibleRecordPhone;
	if (!AddrDBLookupSeekRecord (vars->dbP, &recordNum, &phoneNum, numRows - 1, dmSeekForward,
							   vars->params->field1, vars->params->field2, vars->lookupFieldMap))
	{
		// We have at least one line without a record.  Fix it.
		// Try going backwards one page from the last record
		vars->topVisibleRecord = dmMaxRecordIndex;
		vars->topVisibleRecordPhone = numPhoneFields - 1;
		if (!AddrDBLookupSeekRecord (vars->dbP, &vars->topVisibleRecord, &vars->topVisibleRecordPhone,
								   numRows - 1, dmSeekBackward, vars->params->field1, vars->params->field2,
								   vars->lookupFieldMap))
		{
			// Not enough records to fill one page.  Start with the first record
			vars->topVisibleRecord = 0;
			vars->topVisibleRecordPhone = 0;
			AddrDBLookupSeekRecord (vars->dbP, &vars->topVisibleRecord, &vars->topVisibleRecordPhone,
								  0, dmSeekForward,   vars->params->field1, vars->params->field2,
								  vars->lookupFieldMap);
		}
	}



	numRows = TblGetNumberOfRows (table);
	recordNum = vars->topVisibleRecord;
	phoneNum = vars->topVisibleRecordPhone;
	searchPhones = IsPhoneLookupField(vars->params->field1) ||
		IsPhoneLookupField(vars->params->field2);

	for (row = 0; row < numRows; row++)
	{
		if ( ! AddrDBLookupSeekRecord (vars->dbP, &recordNum, &phoneNum, 0, dmSeekForward,
									 vars->params->field1, vars->params->field2, vars->lookupFieldMap))
			break;

		// Make the row usable.
		TblSetRowUsable (table, row, true);

		// Mark the row invalid so that it will draw when we call the
		// draw routine.
		TblMarkRowInvalid (table, row);

		// Store the record number as the row id.
		TblSetRowID (table, row, recordNum);

		// Store a pointer to vars for the callback.  Ideally the table would
		// have one copy of this information but it doesn't so we keep
		// a pointer to vars in every row.
		//TblSetRowData(table, row, (UInt32) vars);
		TblSetRowData(table, row, (UIntPtr) vars);

		// Store the field used for a phone in the item int.
		// We need to do this because some records have multiple
		// occurances when they have multiple identical phone types
		// (i.e. two email address).  The type is AddressField.
		// Remember that we allow only one of the two fields to be
		// a phone field.
		TblSetItemInt (table, row, 0, firstPhoneField + phoneNum);
#if		WRISTPDA
		TblSetItemFont( table, row, 0, kFontToUse );
#endif//	WRISTPDA

		if (searchPhones)
		{
			phoneNum++;
			if (phoneNum >= numPhoneFields)
			{
				phoneNum = 0;
				recordNum++;
			}
		}
		else
			recordNum++;
	}


	// Hide the item that don't have any data.
	while (row < numRows)
	{
		TblSetRowUsable (table, row, false);
		row++;
	}

	TblUnhighlightSelection(table);

	PrvLookupViewUpdateScrollButtons(vars);
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of names and phone numbers
 *              in the direction specified.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              direction - up or dowm
 *              oneLine   - if true the list is scroll by a single line,
 *                          if false the list is scroll by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupViewScroll (LookupVariablesPtr vars, WinDirectionType direction, Boolean oneLine)
{
	TablePtr table;
	UInt16 rowsInTable;
	UInt16 newTopVisibleRecord;
	Int16 newTopVisibleRecordPhone;


	table = ToolsGetObjectPtr (LookupTable);
	rowsInTable = TblGetNumberOfRows (table);
	newTopVisibleRecord = vars->topVisibleRecord;
	newTopVisibleRecordPhone = vars->topVisibleRecordPhone;


	// Scroll the table down.
	if (direction == winDown)
	{
		// Scroll down a single line.  If we can't scroll down a line
		// then the scroll down arrow would not have been displayed.
		if (oneLine)
		{
			AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
								  1, dmSeekForward, vars->params->field1, vars->params->field2, vars->lookupFieldMap);
			ErrNonFatalDisplayIf (DmGetLastErr(), "Error scrolling");
		}

		// Scroll down a page (less one row).
		else
		{
			// Try going forward one page
			if (!AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
									   rowsInTable - 1, dmSeekForward, vars->params->field1, vars->params->field2,
									   vars->lookupFieldMap))
			{
				// Try going backwards one page from the last record
				newTopVisibleRecord = dmMaxRecordIndex;
				newTopVisibleRecordPhone = numPhoneFields - 1;
				if (!AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
										   rowsInTable - 1, dmSeekBackward, vars->params->field1,
										   vars->params->field2, vars->lookupFieldMap))
				{
					// Not enough records to fill one page.  Start with the first record
					newTopVisibleRecord = 0;
					newTopVisibleRecordPhone = 0;
					AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
										  0, dmSeekForward, vars->params->field1, vars->params->field2,
										  vars->lookupFieldMap);
				}
			}
		}
	}



	// Scroll the table up.
	else
	{
		// Scroll up a single line
		if (oneLine)
		{
			AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone, 1,
								  dmSeekBackward, vars->params->field1, vars->params->field2, vars->lookupFieldMap);
			ErrNonFatalDisplayIf (DmGetLastErr(), "Error scrolling");
		}

		// Scroll up a page (less one row).
		else
		{
			if (!AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
									   rowsInTable - 1, dmSeekBackward, vars->params->field1, vars->params->field2,
									   vars->lookupFieldMap))
			{
				// Not enough records to fill one page.  Start with the first record
				newTopVisibleRecord = 0;
				newTopVisibleRecordPhone = 0;
				AddrDBLookupSeekRecord (vars->dbP, &newTopVisibleRecord, &newTopVisibleRecordPhone,
									  0, dmSeekForward, vars->params->field1, vars->params->field2,
									  vars->lookupFieldMap);
			}
		}
	}


	// Avoid redraw if no change
	if (vars->topVisibleRecord != newTopVisibleRecord ||
		vars->topVisibleRecordPhone != newTopVisibleRecordPhone)
	{
		vars->topVisibleRecord = newTopVisibleRecord;
		vars->topVisibleRecordPhone = newTopVisibleRecordPhone;
		PrvLookupLoadTable(vars);
		TblRedrawTable(table);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewSelectRecord
 *
 * DESCRIPTION: Selects (highlights) a record on the table, scrolling
 *              the record if neccessary.  Also sets the CurrentRecord.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              recordNum - record to select
 *              phoneNum - phone in record to select
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupViewSelectRecord (LookupVariablesPtr vars, UInt16 recordNum, UInt16 phoneNum)
{
	Int16 row, column;
	TablePtr tableP;
	UInt16 attr;
	Boolean recordFound;
	Boolean searchPhones;


	ErrFatalDisplayIf (recordNum >= DmNumRecords(vars->dbP), "Record outside AddrDB");
	ErrFatalDisplayIf (phoneNum >= numPhoneFields, "Phone outside legal range");


	tableP = ToolsGetObjectPtr (LookupTable);


	// Don't change anything if the same record is selected
	if (TblGetSelection(tableP, &row, &column) &&
		recordNum == TblGetRowID (tableP, row) &&
		phoneNum == TblGetItemInt (tableP, row, 0))
	{
		return;
	}


	searchPhones = IsPhoneLookupField(vars->params->field1) ||
		IsPhoneLookupField(vars->params->field2);

	// See if the record is displayed by one of the rows in the table
	// A while is used because if TblFindRowID fails we need to
	// call it again to find the row in the reloaded table.
	while (true)
	{
		recordFound = false;
		if (TblFindRowID(tableP, recordNum, &row))
		{
			if (searchPhones)
			{
				for (; row < TblGetNumberOfRows(tableP); row++)
				{
					if (TblRowUsable(tableP, row))
					{
						if (phoneNum == TblGetItemInt(tableP, row, 0) - firstPhoneField)
						{
							recordFound = true;
							break; // match found
						}
					}
				}
			}
			else
			{
				recordFound = true;
				break; // match found
			}
		}

		// If the record is found in the existing table stop
		// and go select it.  Otherwise position that table to
		// start with the record, reload the table, and look
		// for it again.
		if (recordFound)
			break;

		if (vars->hideSecretRecords)
		{
			// If the record is hidden stop trying to show it.
			DmRecordInfo(vars->dbP, recordNum, &attr, NULL, NULL);
			if (attr & dmRecAttrSecret)
			{
				return;
			}
		}

		// Scroll the view down placing the item
		// on the top row
		vars->topVisibleRecord = recordNum;
		vars->topVisibleRecordPhone = phoneNum;

		PrvLookupLoadTable(vars);
		TblRedrawTable(tableP);
	}


	// Select the item
	TblSelectItem (tableP, row, 0);

	vars->currentRecord = recordNum;
	vars->currentPhone = phoneNum;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewLookupString
 *
 * DESCRIPTION: Adds a character to LookupLookupField, looks up the
 * string in the database and selects the item that matches.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *              event - EventPtr containing character to add to LookupLookupField
 *                        or NULL to use the text there
 *
 * RETURNED:    true if the field handled the event
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *			  meg		2/2/99	 added beepOnFail check before beeping
 ***********************************************************************/
Boolean PrvLookupViewLookupString (LookupVariablesPtr vars, EventType * event)
{
	FormPtr frm;
	UInt16 fldIndex;
	FieldPtr fldP;
	Char * fldTextP;
	TablePtr tableP;
	UInt16 foundRecord;
	Int16 foundRecordPhone;
	Boolean completeMatch;
	Boolean uniqueMatch;
	Int16 length;


	frm = FrmGetActiveForm();
	fldIndex = FrmGetObjectIndex(frm, LookupLookupField);
	FrmSetFocus(frm, fldIndex);
	fldP = FrmGetObjectPtr (frm, fldIndex);


	if (event == NULL ||
		FldHandleEvent (fldP, event))
	{
		fldTextP = FldGetTextPtr(fldP);
		tableP = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, LookupTable));

		foundRecordPhone = 0;
		if (!AddrDBLookupLookupString(vars->dbP, fldTextP, vars->sortByCompany,
									vars->params->field1, vars->params->field2, &foundRecord, &foundRecordPhone,
									vars->lookupFieldMap, &completeMatch, &uniqueMatch))  //foundRecordPhone
		{
			// If the user deleted the lookup text remove the
			// highlight.
			TblUnhighlightSelection(tableP);
		}
		else
		{
			PrvLookupViewSelectRecord(vars, foundRecord, foundRecordPhone);
		}


		if (!completeMatch)
		{
			// Delete the last character added.
			length = FldGetTextLength(fldP);
			FldDelete(fldP, length - 1, length);

			if (vars->beepOnFail)
			{
				SndPlaySystemSound (sndError);

				//ok, we beeped, if there are more chars in the field, we don;t want to beep again
				vars->beepOnFail = false;
			}
		}

		return true;
	}

	// Event not handled
	return false;

}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupClearLookupString
 *
 * DESCRIPTION: Clears the LookupLookupField.  Does not unhighlight the item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
void PrvLookupClearLookupString ()
{
	FormPtr frm;
	UInt16 fldIndex;
	FieldPtr fldP;
	Int16 length;


	frm = FrmGetActiveForm();
	//   FrmSetFocus(frm, noFocus);
	fldIndex = FrmGetObjectIndex(frm, LookupLookupField);
	fldP = FrmGetObjectPtr (frm, fldIndex);

	length = FldGetTextLength(fldP);
	if (length > 0)
		FldDelete(fldP, 0, length);
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupViewUseSelection
 *
 * DESCRIPTION: Use the record currently selected.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *
 * RETURNED:    true if a record was selected and false if not.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   11/11/96   Initial Revision
 *
 ***********************************************************************/
Boolean PrvLookupViewUseSelection (LookupVariablesPtr vars)
{
	TablePtr table;
	Int16 row;
	Int16 column;


	// If a row is selected return the record number else
	// return noRecord.
	table = FrmGetObjectPtr (vars->frm, FrmGetObjectIndex (vars->frm, LookupTable));
	if (TblGetSelection (table, &row, &column))
	{
		vars->currentRecord = TblGetRowID(table, row);
		vars->currentPhone = TblGetItemInt(table, row, column);
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupFindPhoneField
 *
 * DESCRIPTION: Find a phone field from the record.  The first match
 * is used unless it's one of the fields displayed.  In that case the
 * field displayed is used.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *                recordP - the record to find the field in
 *                lookupField - the phone field to lookup
 *                phoneNum - the phone field in the record that matches
 *                           (used when field 1 or field2 is a phone field).
 *
 * RETURNED:    The field to use or lookupNoField if none found
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/18/96   Initial Revision
 *
 ***********************************************************************/
AddressFields PrvLookupFindPhoneField (LookupVariablesPtr vars, AddrDBRecordPtr recordP, AddressLookupFields lookupField, UInt16 phoneNum)
{
	int index;
	int phoneType;


	if (vars->params->field1 == lookupField || vars->params->field2 == lookupField)
		return (AddressFields) phoneNum;
	else
	{
		phoneType = lookupField - addrLookupWork;

		// Scan through the phone fields looking for a phone of the right type
		// which also contains data.
		for (index = firstPhoneField; index <= lastPhoneField; index++)
		{
			if (GetPhoneLabel(recordP, index) == phoneType &&
				recordP->fields[index] != NULL)
			{
				return (AddressFields) (ad_phone1 + index - firstPhoneField);
			}
		}

	}

	return (AddressFields) addrLookupNoField;
}


/***********************************************************************
 *
 * FUNCTION:    PrvLookupResizeResultString
 *
 * DESCRIPTION: Resize the lookup a result string
 *
 * PARAMETERS:  resultStringP - result string
 *              newSize       - new size
 *
 * RETURNED:    pointer to the resized result of zero if the resize failed.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Art   12/11/96   Initial Revision
 *
 ***********************************************************************/
Char * PrvLookupResizeResultString (MemHandle resultStringH, UInt16 newSize)
{
	Err err;

	MemHandleUnlock (resultStringH);

	err = MemHandleResize (resultStringH, newSize);
	if (err)
		return (0);

	return (MemHandleLock (resultStringH));
}

/***********************************************************************
 *
 * FUNCTION:    PrvLookupCreateResultString
 *
 * DESCRIPTION: Create a result string which includes data from the record.
 *
 * PARAMETERS:  vars - variables used by the lookup code.
 *                recordNum - the record create a result string from
 *                phoneNum - the phone field in the record that matches
 *                           (used when field 1 or field2 is a phone field).
 *
 * RETURNED:    The MemHandle to the string created or NULL.
 *                vars->params->resultStringH is also set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *         Ludovic 4/27/00  Fix bug - formatStringP increment invalid for some fields
 *
 ***********************************************************************/
MemHandle PrvLookupCreateResultString (LookupVariablesPtr vars, UInt16 recordNum, UInt16 phoneNum)
{
	Char * formatStringP;
	MemHandle resultStringH;
	Char * resultStringP;
	Int16 resultSize = 0;
	Int16 nextChunkSize;
	Char * nextFieldP;
	Int16 field;
	AddrDBRecordType record;
	MemHandle recordH;
	Char * fieldP;
	Err error;
	UInt16 separatorLength;
	UInt16 phoneLabel;


	// Return the record's unique ID
	DmRecordInfo(vars->dbP, recordNum, NULL, &vars->params->uniqueID, NULL);

	// Check if a format string was specified
	formatStringP = vars->params->formatStringP;
	if (formatStringP == NULL)
		return 0;

	// Allocate the string on the dynamic heap
	resultStringH = MemHandleNew(32);      // Allocate extra so there's room to grow
	if (!resultStringH)
		return 0;            // not enough memory?

	error = AddrDBGetRecord (vars->dbP, recordNum, &record, &recordH);
	ErrFatalDisplayIf ((error), "Record not found");

	resultStringP = MemHandleLock(resultStringH);
	while (*formatStringP != '\0')
	{
		// Copy the next chunk (the string until a '^' or '\0'
		nextFieldP = StrChr(formatStringP, '^');
		if (nextFieldP)
			nextChunkSize = nextFieldP - formatStringP;
		else
			nextChunkSize = StrLen(formatStringP);

		if (nextChunkSize > 0)
		{
			resultStringP = PrvLookupResizeResultString (resultStringH,
													  resultSize + nextChunkSize);
			if (! resultStringP) goto exit;

			MemMove(resultStringP + resultSize, formatStringP, nextChunkSize);

			resultSize += nextChunkSize;
			formatStringP += nextChunkSize;
			nextChunkSize = 0;
		}

		// determine which field to copy next
		if (*formatStringP == '^')
		{
			formatStringP++;
			field = (AddressFields) addrLookupNoField;

			// Decode which field to copy next.

			// Remember that the strings below can't be put into a table
			// because the lookup function runs without global variables
			// available with which we would store the table.
			if (StrNCompare(formatStringP, "name", 4) == 0)
			{
				field = ad_name;
				formatStringP += 4;
			}
			else if (StrNCompare(formatStringP, "first", 5) == 0)
			{
				field = ad_firstName;
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "company", 7) == 0)
			{
				field = ad_company;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "address", 7) == 0)
			{
				field = ad_address;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "city", 4) == 0)
			{
				field = ad_city;
				formatStringP += 4;
			}
			else if (StrNCompare(formatStringP, "state", 5) == 0)
			{
				field = ad_state;
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "zipcode", 7) == 0)
			{
				field = ad_zipCode;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "country", 7) == 0)
			{
				field = ad_country;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "title", 5) == 0)
			{
				field = ad_title;
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "custom1", 7) == 0)
			{
				field = ad_custom1;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "custom2", 7) == 0)
			{
				field = ad_custom2;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "custom3", 7) == 0)
			{
				field = ad_custom3;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "custom4", 7) == 0)
			{
				field = ad_custom4;
				formatStringP += 7;
			}
			else if (StrNCompare(formatStringP, "work", 4) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupWork, phoneNum);
				formatStringP += 4;
			}
			else if (StrNCompare(formatStringP, "home", 4) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupHome, phoneNum);
				formatStringP += 4;
			}
			else if (StrNCompare(formatStringP, "fax", 3) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupFax, phoneNum);
				formatStringP += 3;
			}
			else if (StrNCompare(formatStringP, "other", 5) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupOther, phoneNum);
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "email", 5) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupEmail, phoneNum);
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "main", 4) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupMain, phoneNum);
				formatStringP += 4;
			}
			else if (StrNCompare(formatStringP, "pager", 5) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupPager, phoneNum);
				formatStringP += 5;
			}
			else if (StrNCompare(formatStringP, "mobile", 6) == 0)
			{
				field = PrvLookupFindPhoneField(vars, &record, addrLookupMobile, phoneNum);
				formatStringP += 6;
			}
			else if (StrNCompare(formatStringP, "listname", 8) == 0)
			{
				formatStringP += 8;
				separatorLength = 0;

				// Add the company name
				if ((vars->sortByCompany) || (!record.fields[ad_name] && !record.fields[ad_firstName]))
				{
					fieldP = record.fields[ad_company];
					if (fieldP)
					{
						nextChunkSize = StrLen(fieldP);

						if (nextChunkSize > 0)
						{
							resultStringP = PrvLookupResizeResultString (resultStringH,
																	  resultSize + nextChunkSize);
							if (! resultStringP) goto exit;

							MemMove(resultStringP + resultSize, fieldP, nextChunkSize);

							resultSize += nextChunkSize;
							nextChunkSize = 0;
							separatorLength = 2;
						}
					}
				}

				// Add the name field
				fieldP = record.fields[ad_name];
				if (fieldP)
				{
					nextChunkSize = StrLen(fieldP);

					if (nextChunkSize > 0)
					{
						resultStringP = PrvLookupResizeResultString (resultStringH, resultSize + nextChunkSize + separatorLength);
						if (! resultStringP) goto exit;
						if (separatorLength > 0)
						{
							MemMove(resultStringP + resultSize, ", ", separatorLength);
							resultSize += separatorLength;
						}
						MemMove(resultStringP + resultSize, fieldP, nextChunkSize);

						resultSize += nextChunkSize;
						nextChunkSize = 0;
						separatorLength = 2;
					}
				}

				// Add the first name field
				if (!vars->sortByCompany ||
					record.fields[ad_company] == NULL)
				{
					fieldP = record.fields[ad_firstName];
					if (fieldP)
					{
						nextChunkSize = StrLen(fieldP);

						if (nextChunkSize > 0)
						{
							resultStringP = PrvLookupResizeResultString (resultStringH, resultSize + nextChunkSize + separatorLength);
							if (! resultStringP) goto exit;
							if (separatorLength > 0)
							{
								MemMove(resultStringP + resultSize, ", ", separatorLength);
								resultSize += separatorLength;
							}
							MemMove(resultStringP + resultSize, fieldP, nextChunkSize);

							resultSize += nextChunkSize;
							nextChunkSize = 0;
						}
					}
				}

				// We are done adding the data requested.  Continue to the next
				// chunk
				continue;
			}
			else if (StrNCompare(formatStringP, "listphone", 9) == 0)
			{
				formatStringP += 9;
				separatorLength = 0;

				// Add the list phone number with a letter after it
				fieldP = record.fields[firstPhoneField +
									   record.options.phones.displayPhoneForList];
				if (fieldP)
				{
					nextChunkSize = StrLen(fieldP);
					if (nextChunkSize > 0)
					{
						resultStringP = PrvLookupResizeResultString (resultStringH, resultSize + nextChunkSize + 2);
						if (! resultStringP) goto exit;

						MemMove(resultStringP + resultSize, fieldP, nextChunkSize);

						resultSize += nextChunkSize;

						resultStringP[resultSize] = ' ';
						phoneLabel = GetPhoneLabel(&record, firstPhoneField +
												   record.options.phones.displayPhoneForList);
						resultStringP[resultSize + 1] = vars->phoneLabelLetters[phoneLabel];
						resultSize += 2;

						nextChunkSize = 0;
						separatorLength = 2;
					}
				}
			}



			// Now copy in the correct field.  lookupNoField can result from
			// asking for a phone which isn't used.
			if (field != (AddressFields) addrLookupNoField)
			{
				fieldP = record.fields[field];
				if (fieldP)
				{
					nextChunkSize = StrLen(fieldP);

					if (nextChunkSize > 0)
					{
						resultStringP = PrvLookupResizeResultString (resultStringH, resultSize + nextChunkSize);
						if (! resultStringP) goto exit;
						MemMove(resultStringP + resultSize, fieldP, nextChunkSize);

						resultSize += nextChunkSize;
						nextChunkSize = 0;
					}
				}
			}

		}

	}

	// Now null terminate the result string
	resultStringP = PrvLookupResizeResultString (resultStringH, resultSize + 1);
	if (! resultStringP) goto exit;

	resultStringP[resultSize] = '\0';

	vars->params->resultStringH = resultStringH;
	MemHandleUnlock(recordH);
	MemHandleUnlock(resultStringH);

	return resultStringH;


exit:
	// Error return
	MemHandleUnlock(recordH);
	MemHandleFree (resultStringH);
	vars->params->resultStringH = 0;
	return (0);
}
