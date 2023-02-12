/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrList.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *   This is the Address Book application's list form module.
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrList.h"
#include "AddrView.h"
#include "AddrEdit.h"
#include "AddrDialList.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddrNote.h"
#include "AddrDefines.h"
#include "AddressRsc.h"
#include "AddressTransfer.h"

#include "SysDebug.h"

#include <TextMgr.h>
#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <TimeMgr.h>
#include <AboutBox.h>
#include <Category.h>
#include <Menu.h>
#include <UIResources.h>


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Address list table columns
#define nameAndNumColumn					0
#define noteColumn							1

// Scroll rate values
#define scrollDelay							2
#define scrollAcceleration					2
#define scrollSpeedLimit					5


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

// These are used for accelerated scrolling
static UInt16 				LastSeconds = 0;
static UInt16 				ScrollUnits = 0;

#if WRISTPDA
// Index of the current selected record.
static UInt16 SelectedRecord    = 0;
#endif


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void		PrvListInit( FormType* frmP ) SEC("code2");
static void		PrvListSelectRecord( FormType* frmP, UInt16 recordNum, Boolean forceSelection ) SEC("code2");
static void		PrvListScroll( WinDirectionType direction, UInt16 units, Boolean byLine ) SEC("code2");
static void		PrvListResetScrollRate(void) SEC("code2");
static void		PrvListAdjustScrollRate(void) SEC("code2");
static Boolean	PrvListLookupString (EventType * event) SEC("code2");
static void		PrvListDrawRecord (void * table, Int16 row, Int16 column, RectanglePtr bounds) SEC("code2");
static void		PrvListClearLookupString () SEC("code2");
static void		PrvListUpdateDisplay( UInt16 updateCode ) SEC("code2");
static UInt16	PrvListNumberOfRows (TablePtr table) SEC("code2");
static void		PrvListUpdateScrollButtons( FormType* frmP ) SEC("code2");
static void		PrvListLoadTable( FormType* frmP ) SEC("code2");
static UInt16	PrvListSelectCategory (void) SEC("code2");
static void		PrvListNextCategory (void) SEC("code2");
static Boolean	PrvListDeleteRecord (void) SEC("code2");
static Boolean	PrvListDoCommand (UInt16 command) SEC("code2");
static Boolean	PrvListHandleRecordSelection( EventType* event ) SEC("code2");
static void		PrvListReplaceTwoColors (const RectangleType *rP, UInt16 cornerDiam, UIColorTableEntries oldForeground, UIColorTableEntries oldBackground, UIColorTableEntries newForeground, UIColorTableEntries newBackground) SEC("code2");

#if WRISTPDA
static void RationalizeCurrentSelection( void );
#endif

#if WRISTPDA

/***********************************************************************
 *
 * FUNCTION:    LastVisibleRecord
 *
 * DESCRIPTION: This routine returns the index of the last visible record.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Index of last visible record.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dmc		2/04/03		Initial Revision
 *
 ***********************************************************************/
static UInt16 LastVisibleRecord( void )
{
	UInt16 i, First, Last, NumRows;
	First = TopVisibleRecord;
	Last  = TopVisibleRecord;
	NumRows = PrvListNumberOfRows( ToolsGetObjectPtr( ListTable ) );
	for ( i = 0; i < NumRows; i++ ) {
		 if ( ! ToolsSeekRecord( & Last, 1, +1 ) )
		 	break;
	}
	return Last;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewMoveSelection
 *
 * DESCRIPTION: This routine moves the List View navigation selection
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
 *		02/03/03	dmc		Initial version.
 *
 ***********************************************************************/
static void ListViewMoveSelection ( Int16 Direction )
{
	TablePtr Table;
	Int16  VisibleTopRecordIndex, VisibleBottomRecordIndex;
	UInt16 RecordsInCategory, TableLastRecordIndex;
	// If there are no records then we can't move the selection.
	RecordsInCategory = DmNumRecordsInCategory( AddrDB, CurrentCategory );
	if ( RecordsInCategory == 0 )
		return;
	if ( Direction != 0 ) {
		// Get the info we need to decide if we can move the selection.
		Table = ToolsGetObjectPtr( ListTable );
		TableLastRecordIndex = dmMaxRecordIndex;
		ToolsSeekRecord( & TableLastRecordIndex, 0, -1 );
		VisibleTopRecordIndex = TopVisibleRecord;
		VisibleBottomRecordIndex = LastVisibleRecord();
		if ( VisibleBottomRecordIndex > TableLastRecordIndex )
			VisibleBottomRecordIndex = TableLastRecordIndex;
		if ( Direction == +1 ) {
			// Move the selection highlight down one record, scroll if necessary.
			if ( SelectedRecord < TableLastRecordIndex ) {
				if ( SelectedRecord > VisibleBottomRecordIndex )
					PrvListScroll( winDown, 1, true );
				else
					ToolsSeekRecord( & SelectedRecord, 1, +1 );
			}
		}
	 	else if ( Direction == -1 ) {
			// Move the selection highlight up one record, scroll if necessary.
			if ( SelectedRecord > 0 ) {
				if ( SelectedRecord < VisibleTopRecordIndex )
					PrvListScroll( winUp, 1, true );
				else
					ToolsSeekRecord( & SelectedRecord, 1, -1 );
			}
		}
	}
	// Highlight the current selection.
	PrvListSelectRecord( FrmGetActiveForm(), SelectedRecord, true );
}

#endif


/***********************************************************************
 *
 * FUNCTION:    ListHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the Address Book application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		6/5/95   Initial Revision
 *			frigino	8/15/97	Added scroll rate acceleration using
 *									PrvListResetScrollRate() and PrvListAdjustScrollRate()
 *			jmp		9/17/99	Use NewNoteView instead of NoteView.
 *			peter		4/25/00	Add support for un-masking just the selected record.
 *			peter		5/08/00	Add support for tapping on phone numbers to dial.
 *			aro			6/22/00	Add check for dialing abilities
 *			aro			9/25/00	use cmdTextH rather than cmdText to avoir unreleased resource
 *
 ***********************************************************************/
Boolean ListHandleEvent (EventType* event)
{
	FormType* frmP;
	Boolean handled = false;
	TablePtr table;
	Int16 row;
	Int16 column;
	MemHandle cmdTextH;
	UInt32 numLibs;

	switch (event->eType)
	{
	case frmOpenEvent:
    //SysDebug(1, "Addr", "ListHandleEvent frmOpenEvent");
		frmP = FrmGetActiveForm ();
		PrvListInit (frmP);

		#if WRISTPDA
		{
			// Initialize the SelectedRecord value and highlight selection.
			UInt16 numRecs = DmNumRecordsInCategory( AddrDB, CurrentCategory );
			if ( numRecs > 0 ) {
				if ( ListViewSelectThisRecord == noRecord )
					ListViewSelectThisRecord = 0;
				ToolsSeekRecord( & ListViewSelectThisRecord, 0, +1 );
			}
			SelectedRecord = ListViewSelectThisRecord;
			ListViewMoveSelection( 0 );
		}
		#endif

		// Make sure the record to be selected is one of the table's rows or
		// else it reloads the table with the record at the top.  Nothing is
		// drawn by this because the table isn't visible.
		if (ListViewSelectThisRecord != noRecord)
			PrvListSelectRecord(frmP, ListViewSelectThisRecord, false);

		FrmDrawForm (frmP);

		// Select the record.  This finds which row to select it and does it.
		if (ListViewSelectThisRecord != noRecord)
		{
			PrvListSelectRecord(frmP, ListViewSelectThisRecord, true);
			ListViewSelectThisRecord = noRecord;
		}

		// Set the focus in the lookup field so that the user can easily
		// bring up the keyboard.
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, ListLookupField));

		PriorAddressFormID = FrmGetFormId (frmP);

		// Check the dialing abilities
		// Only the first call is long and further called are fast
		// So it's better to do it the first time the form is drawn
		if (!ToolsIsDialerPresent())
			EnableTapDialing = false;

		handled = true;
		break;

	case frmCloseEvent:
    //SysDebug(1, "Addr", "ListHandleEvent frmCloseEvent");
		if (UnnamedRecordStringPtr)
		{
			MemPtrUnlock(UnnamedRecordStringPtr);
			UnnamedRecordStringPtr = NULL;
		}

		if (UnnamedRecordStringH)
		{
			DmReleaseResource(UnnamedRecordStringH);
			UnnamedRecordStringH = NULL;
		}
		break;

	case tblEnterEvent:
    //SysDebug(1, "Addr", "ListHandleEvent tblSelectEvent %d", event->data.tblEnter.row);
		handled = PrvListHandleRecordSelection (event);
		break;

	case tblSelectEvent:
    //SysDebug(1, "Addr", "ListHandleEvent tblSelectEvent %d", event->data.tblSelect.row);
		if (TblRowMasked(event->data.tblSelect.pTable,
						 event->data.tblSelect.row))
		{
			if (SecVerifyPW (showPrivateRecords) == true)
			{
				// We only want to unmask this one record, so restore the preference.
				PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

				event->data.tblSelect.column = nameAndNumColumn; //force non-note view
			}
			else
				break;
		}

		// An item in the list of names and phone numbers was selected, go to
		// the record view.
		CurrentRecord = TblGetRowID (event->data.tblSelect.pTable, event->data.tblSelect.row);
    //SysDebug(1, "Addr", "ListHandleEvent set CurrentRecord=%d", CurrentRecord);

		// Set the global variable that determines which field is the top visible
		// field in the edit view.  Also done when New is pressed.
		TopVisibleFieldIndex = 0;
		EditRowIDWhichHadFocus = editFirstFieldIndex;
		EditFieldPosition = 0;

		if (event->data.tblSelect.column == nameAndNumColumn)
			FrmGotoForm (RecordView);
		else
			if (NoteViewCreate())
				FrmGotoForm (NewNoteView);
		handled = true;
		break;

	case ctlEnterEvent:
    //SysDebug(1, "Addr", "ListHandleEvent ctlEnterEvent %d", event->data.ctlEnter.controlID);
		switch (event->data.ctlEnter.controlID)
		{
			case ListUpButton:
			case ListDownButton:
				// Reset scroll rate
				PrvListResetScrollRate();
				// Clear lookup string
				PrvListClearLookupString ();
				// leave unhandled so the buttons can repeat
				break;
		}
		break;

	case ctlSelectEvent:
    //SysDebug(1, "Addr", "ListHandleEvent ctlSelectEvent %d", event->data.ctlSelect.controlID);
		switch (event->data.ctlSelect.controlID)
		{
			case ListCategoryTrigger:
				PrvListSelectCategory ();
				handled = true;
				break;
	
			case ListNewButton:
				EditNewRecord();
				handled = true;
				break;
		}
		break;

	case ctlRepeatEvent:
    //SysDebug(1, "Addr", "ListHandleEvent ctlRepeatEvent %d", event->data.ctlRepeat.controlID);
		// Adjust the scroll rate if necessary
		PrvListAdjustScrollRate();

		switch (event->data.ctlRepeat.controlID)
		{
			case ListUpButton:
				PrvListScroll (winUp, ScrollUnits, false);
				// leave unhandled so the buttons can repeat
				break;
	
			case ListDownButton:
				PrvListScroll (winDown, ScrollUnits, false);
				// leave unhandled so the buttons can repeat
				break;
			default:
				break;
		}
		break;


	case keyDownEvent:
    //SysDebug(1, "Addr", "ListHandleEvent keyDownEvent %d", event->data.keyDown.chr);
		// Address Book key pressed for the first time?
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
		{
			if (! (event->data.keyDown.modifiers & poweredOnKeyMask))
			{
				PrvListClearLookupString ();
				PrvListNextCategory ();
				handled = true;
			}
		}

		else if (EvtKeydownIsVirtual(event))
		{
			switch (event->data.keyDown.chr)
			{
				case vchrPageUp:
					// Reset scroll rate if not auto repeating
					if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
					{
						PrvListResetScrollRate();
					}
					// Adjust the scroll rate if necessary
					PrvListAdjustScrollRate();
					PrvListScroll (winUp, ScrollUnits, false);
					PrvListClearLookupString ();
					handled = true;
					break;
	
				case vchrPageDown:
					// Reset scroll rate if not auto repeating
					if ((event->data.keyDown.modifiers & autoRepeatKeyMask) == 0)
					{
						PrvListResetScrollRate();
					}
					// Adjust the scroll rate if necessary
					PrvListAdjustScrollRate();
					PrvListScroll (winDown, ScrollUnits, false);
					PrvListClearLookupString ();
					handled = true;
					break;
	
				#if WRISTPDA

				case vchrThumbWheelBack:
					{
					// Translate the Back key to an open launcher event.
					EventType newEvent;
					MemSet (&newEvent, sizeof(EventType), 0);
					newEvent.eType = keyDownEvent;
					newEvent.data.keyDown.chr = launchChr;
					newEvent.data.keyDown.modifiers = commandKeyMask;
					EvtAddEventToQueue( &newEvent );
					}
					handled = true;
					break;

				case vchrThumbWheelDown:
					// Move the selection highlight down one record, scroll if necessary.
					ListViewMoveSelection( +1 );
					handled = true;
					break;

				case vchrThumbWheelUp:
					// Move the selection highlight up one record, scroll if necessary.
					ListViewMoveSelection( -1 );
					handled = true;
					break;

				case vchrThumbWheelPush:
					if (CurrentRecord == noRecord)
					{
						SndPlaySystemSound (sndError);
					}
					else
					{
						// Open the selected record in record view.
						// Set the global variable that determines which field
						// is the top visible field in the record view.
						UInt16 attr;
						TopVisibleFieldIndex = 0;
						EditRowIDWhichHadFocus = editFirstFieldIndex;
						EditFieldPosition = 0;
						// Get the category and secret attribute of the current record.
						DmRecordInfo( AddrDB, CurrentRecord, &attr, NULL, NULL );
						// If this is a "private" record, then determine what is to be shown.
						if ( attr & dmRecAttrSecret ) {
							switch ( PrivateRecordVisualStatus ) {
								case showPrivateRecords:
									FrmGotoForm( RecordView );
									break;
								case maskPrivateRecords:
									if ( SecVerifyPW( showPrivateRecords ) == true ) {
										// We only want to unmask this one record, so restore the preference.
										PrefSetPreference( prefShowPrivateRecords, maskPrivateRecords );
										FrmGotoForm( RecordView );
									}
									break;
								// This case should never be executed.
								case hidePrivateRecords:
								default:
									break;
							}
						} else {
							FrmGotoForm (RecordView);
						}
						handled = true;
					}
					break;

				#endif

				case vchrSendData:
					if (CurrentRecord != noRecord)
					{
						MenuEraseStatus (0);
						TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
					}
					else
						SndPlaySystemSound (sndError);
					handled = true;
					break;
			}
		}

		else if (event->data.keyDown.chr == linefeedChr)
		{
			frmP = FrmGetActiveForm ();
			table = FrmGetObjectPtr (frmP, FrmGetObjectIndex (frmP, ListTable));
			if (TblGetSelection (table, &row, &column))
			{
				// Set the global variable that determines which field is the top visible
				// field in the edit view.  Also done when New is pressed.
				TopVisibleFieldIndex = 0;

				FrmGotoForm (RecordView);
			}
			handled = true;
		}

		else
			handled = PrvListLookupString(event);
		break;

	case fldChangedEvent:
    //SysDebug(1, "Addr", "ListHandleEvent fldChangedEvent %d", event->data.fldChanged.fieldID);
		PrvListLookupString(event);
		handled = true;
		break;

	case menuEvent:
    //SysDebug(1, "Addr", "ListHandleEvent menuEvent");
		return PrvListDoCommand (event->data.menu.itemID);

	case menuCmdBarOpenEvent:
    //SysDebug(1, "Addr", "ListHandleEvent menuCmdBarOpenEvent");
		if (CurrentRecord != noRecord)
		{
			// because this isn't a real menu command, get the text for the button from a resource
			cmdTextH = DmGetResource (strRsc, DeleteRecordStr);
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarDeleteBitmap, menuCmdBarResultMenuItem, ListRecordDeleteRecordCmd, MemHandleLock(cmdTextH));
			MemHandleUnlock(cmdTextH);
			DmReleaseResource(cmdTextH);
		}

		MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, ListOptionsSecurityCmd, 0);

		if (CurrentRecord != noRecord)
		{
			// because this isn't a real menu command, get the text for the button from a resource
			cmdTextH = DmGetResource (strRsc, BeamRecordStr);
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, ListRecordBeamRecordCmd, MemHandleLock(cmdTextH));
			MemHandleUnlock(cmdTextH);
			DmReleaseResource(cmdTextH);
		}

		// tell the field package to not add cut/copy/paste buttons automatically; we
		// don't want it for the lookup field since it'd cause confusion.
		event->data.menuCmdBarOpen.preventFieldButtons = true;

		// don't set handled to true; this event must fall through to the system.
		break;

	case menuOpenEvent:
    //SysDebug(1, "Addr", "ListHandleEvent menuOpenEvent");
		if(!ToolsIsDialerPresent())
			MenuHideItem(ListRecordDialCmd);

		if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
			MenuHideItem(ListRecordSendCategoryCmd);
		else
			MenuShowItem(ListRecordSendCategoryCmd);
		// don't set handled = true
		break;

	case frmUpdateEvent:
    //SysDebug(1, "Addr", "ListHandleEvent frmUpdateEvent");
		PrvListUpdateDisplay (event->data.frmUpdate.updateCode);
		handled = true;
		break;

	default:
		break;
	}

  //SysDebug(1, "Addr", "ListHandleEvent event %d handled %d", event->eType, handled);
	return (handled);
}


//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvListInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
void PrvListInit( FormType* frmP )
{
	UInt16 row;
	UInt16 rowsInTable;
	TableType* tblP;
	ControlPtr ctl;

	if (ShowAllCategories)
		CurrentCategory = dmAllCategories;


	// Initialize the address list table.
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	rowsInTable = TblGetNumberOfRows(tblP);
	for (row = 0; row < rowsInTable; row++)
	{
		TblSetItemStyle(tblP, row, nameAndNumColumn, customTableItem);
		TblSetItemStyle(tblP, row, noteColumn, customTableItem);
		TblSetRowUsable(tblP, row, false);
	}

	TblSetColumnUsable(tblP, nameAndNumColumn, true);
	TblSetColumnUsable(tblP, noteColumn, true);

	TblSetColumnMasked(tblP, nameAndNumColumn, true);
	TblSetColumnMasked(tblP, noteColumn, true);


	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure (tblP, nameAndNumColumn, PrvListDrawRecord);
	TblSetCustomDrawProcedure (tblP, noteColumn, PrvListDrawRecord);


	// Load records into the address list.
	PrvListLoadTable(frmP);


	// Set the label of the category trigger.
	ctl = ToolsGetFrmObjectPtr(frmP, ListCategoryTrigger);
	CategoryGetName (AddrDB, CurrentCategory, CategoryName);
	CategorySetTriggerLabel (ctl, CategoryName);

}


/***********************************************************************
 *
 * FUNCTION:    PrvListResetScrollRate
 *
 * DESCRIPTION: This routine resets the scroll rate
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino	8/14/97	Initial Revision
 *
 ***********************************************************************/
void PrvListResetScrollRate(void)
{
	// Reset last seconds
	LastSeconds = TimGetSeconds();
	// Reset scroll units
	ScrollUnits = 1;
}


/***********************************************************************
 *
 * FUNCTION:    PrvListAdjustScrollRate
 *
 * DESCRIPTION: This routine adjusts the scroll rate based on the current
 *              scroll rate, given a certain delay, and plays a sound
 *              to notify the user of the change
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			frigino		8/14/97		Initial Revision
 *			vmk			12/2/97		Fix crash from uninitialized sndCmd and
 *									derive sound amplitude from system amplitude
 *
 ***********************************************************************/
void PrvListAdjustScrollRate(void)
{
	// Accelerate the scroll rate every 3 seconds if not already at max scroll speed
	UInt16 newSeconds = TimGetSeconds();
	if ((ScrollUnits < scrollSpeedLimit) && ((newSeconds - LastSeconds) > scrollDelay))
	{
		// Save new seconds
		LastSeconds = newSeconds;

		// increase scroll units
		ScrollUnits += scrollAcceleration;
	}

}


/***********************************************************************
 *
 * FUNCTION:    PrvListLookupString
 *
 * DESCRIPTION: Adds a character to ListLookupField, looks up the
 * string in the database and selects the item that matches.
 *
 * PARAMETERS:  event - EventType* containing character to add to ListLookupField
 *
 * RETURNED:    true if the field handled the event
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/15/95   Initial Revision
 *
 ***********************************************************************/
Boolean PrvListLookupString (EventType * event)
{
	FormType* frmP;
	UInt16 fldIndex;
	FieldPtr fldP;
	Char * fldTextP;
	TablePtr tableP;
	UInt16 foundRecord;
	Boolean completeMatch;
	Int16 length;


	frmP = FrmGetActiveForm();
	fldIndex = FrmGetObjectIndex(frmP, ListLookupField);
	FrmSetFocus(frmP, fldIndex);
	fldP = FrmGetObjectPtr (frmP, fldIndex);


	if (FldHandleEvent (fldP, event) || event->eType == fldChangedEvent)
	{
		fldTextP = FldGetTextPtr(fldP);
		tableP = ToolsGetFrmObjectPtr(frmP, ListTable);

		if (!AddrDBLookupString(AddrDB, fldTextP, SortByCompany,
							  CurrentCategory, &foundRecord, &completeMatch,
							  (PrivateRecordVisualStatus == maskPrivateRecords)))
		{
			// If the user deleted the lookup text remove the
			// highlight.
			CurrentRecord = noRecord;
			TblUnhighlightSelection(tableP);
		}
		else
		{
			PrvListSelectRecord(frmP, foundRecord, true);
		}


		if (!completeMatch)
		{
			// Delete the last character added.
			length = FldGetTextLength(fldP);
			FldDelete(fldP, length - 1, length);

			SndPlaySystemSound (sndError);
		}

		return true;
	}

	// Event not handled
	return false;

}


/***********************************************************************
 *
 * FUNCTION:    PrvListDrawRecord
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
 *         roger   6/21/95   Initial Revision
 *			  peter	 5/09/00	  Store phoneX in row data for tap test
 *
 ***********************************************************************/
void PrvListDrawRecord (void * table, Int16 row, Int16 column, RectanglePtr bounds)
{
	UInt16 recordNum;
	Err error;
	AddrDBRecordType record;
	MemHandle recordH;
	char noteChar;
	FontID currFont;
	Int16 phoneX;

	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	//
	recordNum = TblGetRowID (table, row);

	error = AddrDBGetRecord (AddrDB, recordNum, &record, &recordH);
	if (error)
	{
		ErrNonFatalDisplay ("Record not found");
		return;
	}

	switch (column)
	{
		case nameAndNumColumn:
			currFont = FntSetFont (AddrListFont);
			#if WRISTPDA
			{
			RectangleType newBounds = * bounds;
			newBounds.extent.x -= 4;
			phoneX = ToolsDrawRecordNameAndPhoneNumber (&record, & newBounds, PhoneLabelLetters, SortByCompany, &UnnamedRecordStringPtr, &UnnamedRecordStringH);
			}
			#else
			phoneX = ToolsDrawRecordNameAndPhoneNumber (&record, bounds, PhoneLabelLetters, SortByCompany, &UnnamedRecordStringPtr, &UnnamedRecordStringH);
			#endif
			FntSetFont (currFont);
			TblSetRowData(table, row, phoneX);			// Store in table for later tap testing
			break;
		case noteColumn:
			// Draw a note symbol if the field has a note.
			if (record.fields[ad_note])
			{
				#if WRISTPDA
				currFont = FntSetFont (FossilSymbolFont);
				#else
				currFont = FntSetFont (symbolFont);
				#endif
				noteChar = symbolNote;
				#if WRISTPDA
				{
				RectangleType newBounds = * bounds;
				newBounds.topLeft.x -= 1;
				WinDrawChars (&noteChar, 1, newBounds.topLeft.x, newBounds.topLeft.y);
				}
				#else
				WinDrawChars (&noteChar, 1, bounds->topLeft.x, bounds->topLeft.y);
				#endif
				FntSetFont (currFont);
			}
		break;
	}

	MemHandleUnlock(recordH);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListClearLookupString
 *
 * DESCRIPTION: Clears the ListLookupField.  Does not unhighlight the item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/16/95   Initial Revision
 *
 ***********************************************************************/
void PrvListClearLookupString ()
{
	FormType* frmP;
	FieldType* fldP;
	Int16 length;

	frmP = FrmGetActiveForm();
	FrmSetFocus(frmP, noFocus);
	fldP = ToolsGetFrmObjectPtr(frmP, ListLookupField);

	length = FldGetTextLength(fldP);
	if (length > 0)
	{
		// Clear it this way instead of with FldDelete to avoid sending a
		// fldChangedEvent (which would undesirably unhighlight the item).
		FldFreeMemory (fldP);
		FldDrawField (fldP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvListNumberOfRows
 *
 * DESCRIPTION: This routine return the maximun number of visible rows,
 *              with the current list view font setting.
 *
 * PARAMETERS:  table - List View table
 *
 * RETURNED:    maximun number of displayable rows
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	8/28/97	Initial Revision
 *
 ***********************************************************************/
UInt16 PrvListNumberOfRows (TablePtr table)
{
	UInt16				rows;
	UInt16				rowsInTable;
	UInt16				tableHeight;
	FontID			currFont;
	RectangleType	r;


	rowsInTable = TblGetNumberOfRows (table);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (AddrListFont);
	rows = tableHeight / FntLineHeight ();
	FntSetFont (currFont);

	if (rows <= rowsInTable)
		return (rows);
	else
		return (rowsInTable);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListUpdateScrollButtons
 *
 * DESCRIPTION: Show or hide the list view scroll buttons.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *        	Name   Date      	Description
 *        	----   ----      	-----------
 *       	roger   6/21/95   	Initial Revision
 *			aro		9/25/00		Adding frmP as a parameter for frmUpdateEvent
 *
 ***********************************************************************/
void PrvListUpdateScrollButtons( FormType* frmP )
{
	UInt16 row;
	UInt16 upIndex;
	UInt16 downIndex;
	UInt16 recordNum;
	Boolean scrollableUp;
	Boolean scrollableDown;
	TableType* tblP;

	// Update the button that scroll the list.
	//
	// If the first record displayed is not the fist record in the category,
	// enable the up scroller.
	recordNum = TopVisibleRecord;
	scrollableUp = ToolsSeekRecord (&recordNum, 1, dmSeekBackward);


	// Find the record in the last row of the table
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	row = TblGetLastUsableRow(tblP);
	if (row != tblUnusableRow)
		recordNum = TblGetRowID(tblP, row);


	// If the last record displayed is not the last record in the category,
	// enable the down scroller.
	scrollableDown = ToolsSeekRecord (&recordNum, 1, dmSeekForward);

	// Update the scroll button.
	upIndex = FrmGetObjectIndex(frmP, ListUpButton);
	downIndex = FrmGetObjectIndex(frmP, ListDownButton);
	FrmUpdateScrollers(frmP, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListLoadTable
 *
 * DESCRIPTION: This routine loads address book database records into
 *              the list view form.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *      Name   	Date      	Description
 *      ----   	----      	-----------
 *      art  	6/5/95      Initial Revision
 *		aro		9/25/00		Adding frmP as a parameter for frmUpdateEvent
 *
 ***********************************************************************/
void PrvListLoadTable( FormType* frmP )
{
	UInt16      row;
	UInt16      numRows;
	UInt16		lineHeight;
	UInt16		recordNum;
	UInt16		visibleRows;
	FontID		currFont;
	TableType* 	tblP;
	UInt16 		attr;
	Boolean		masked;


	// For each row in the table, store the record number as the row id.
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);

	TblUnhighlightSelection(tblP);

	// Make sure we haven't scrolled too far down the list of records
	// leaving blank lines in the table.

	// Try going forward to the last record that should be visible
	visibleRows = PrvListNumberOfRows(tblP);
	recordNum = TopVisibleRecord;
	if (!ToolsSeekRecord (&recordNum, visibleRows - 1, dmSeekForward))
	{
		// We have at least one line without a record.  Fix it.
		// Try going backwards one page from the last record
		TopVisibleRecord = dmMaxRecordIndex;
		if (!ToolsSeekRecord (&TopVisibleRecord, visibleRows - 1, dmSeekBackward))
		{
			// Not enough records to fill one page.  Start with the first record
			TopVisibleRecord = 0;
			ToolsSeekRecord (&TopVisibleRecord, 0, dmSeekForward);
		}
	}


	currFont = FntSetFont (AddrListFont);
	lineHeight = FntLineHeight ();
	FntSetFont (currFont);

	numRows = TblGetNumberOfRows(tblP);
	recordNum = TopVisibleRecord;

	for (row = 0; row < visibleRows; row++)
	{
		if ( ! ToolsSeekRecord (&recordNum, 0, dmSeekForward))
			break;

		// Make the row usable.
		TblSetRowUsable (tblP, row, true);

		DmRecordInfo (AddrDB, recordNum, &attr, NULL, NULL);
		masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));
		TblSetRowMasked(tblP,row,masked);

		#if WRISTPDA
		TblSetRowSelectable(tblP,row, true);
		#endif

		// Mark the row invalid so that it will draw when we call the
		// draw routine.
		TblMarkRowInvalid (tblP, row);

		// Store the record number as the row id.
		TblSetRowID (tblP, row, recordNum);

		TblSetItemFont (tblP, row, nameAndNumColumn, AddrListFont);
		TblSetRowHeight (tblP, row, lineHeight);

		recordNum++;
	}


	// Hide the item that don't have any data.
	while (row < numRows)
	{
		TblSetRowUsable (tblP, row, false);
		row++;
	}

	PrvListUpdateScrollButtons(frmP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    The index of the new category.
 *
 *              The following global variables are modified:
 *                     CurrentCategory
 *                     ShowAllCategories
 *                     CategoryName
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   06/05/95   Initial Revision
 *			  gap	  08/13/99   Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
UInt16 PrvListSelectCategory (void)
{
	FormType* frmP;
	TableType* tblP;
	UInt16 category;
	Boolean categoryEdited;

	// Process the category popup list.
	category = CurrentCategory;

	frmP = FrmGetActiveForm();
	categoryEdited = CategorySelect (AddrDB, frmP, ListCategoryTrigger,
									 ListCategoryList, true, &category, CategoryName, 1, categoryDefaultEditCategoryString);

	if (category == dmAllCategories)
		ShowAllCategories = true;
	else
		ShowAllCategories = false;

	if ( categoryEdited || (category != CurrentCategory))
	{
		ToolsChangeCategory (category);

		// Display the new category.
		PrvListLoadTable(frmP);
		tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
		TblEraseTable(tblP);
		TblDrawTable(tblP);

		PrvListClearLookupString();

		// By changing the category the current record is lost.
		CurrentRecord = noRecord;
	}

	#if WRISTPDA
	// When changing category set SelectedRecord to the first record
	// of the category (if there is on), otherwise set to noRecord.
	TopVisibleRecord = 0;
	if ( ! ToolsSeekRecord( & TopVisibleRecord, 0, +1 ) )
		TopVisibleRecord = noRecord;
	SelectedRecord = CurrentRecord = TopVisibleRecord;
	RationalizeCurrentSelection();
	#endif

	return (category);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListNextCategory
 *
 * DESCRIPTION: This routine display the next category,  if the last
 *              catagory is being displayed
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *                     CurrentCategory
 *                     ShowAllCategories
 *                     CategoryName
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   9/15/95   Initial Revision
 *         rsf   9/20/95   Copied from To Do
 *
 ***********************************************************************/
void PrvListNextCategory (void)
{
	UInt16 category;
	TableType* tblP;
	ControlType* ctlP;
	FormType* frmP;

	category = CategoryGetNext (AddrDB, CurrentCategory);

	if (category != CurrentCategory)
	{
		if (category == dmAllCategories)
			ShowAllCategories = true;
		else
			ShowAllCategories = false;

		ToolsChangeCategory (category);

		// Set the label of the category trigger.
		frmP = FrmGetActiveForm();
		ctlP = ToolsGetFrmObjectPtr(frmP, ListCategoryTrigger);
		CategoryGetName (AddrDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel(ctlP, CategoryName);


		// Display the new category.
		PrvListLoadTable(frmP);
		tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
		TblEraseTable(tblP);
		TblDrawTable(tblP);

		// By changing the category the current record is lost.
		CurrentRecord = noRecord;
	}
}


#if WRISTPDA

/***********************************************************************
 *
 * FUNCTION:    RationalizeCurrentSelection
 *
 * DESCRIPTION: This routine makes the current selection values reasonable
 *              after a large view change such as a page scroll.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dmc		2/04/03		Initial Revision
 *
 ***********************************************************************/
static void RationalizeCurrentSelection( void )
{
	UInt16 MinVal, MaxVal;
	UInt16 RecordsInCategory = DmNumRecordsInCategory( AddrDB, CurrentCategory );
	if ( RecordsInCategory == 0 ) {
		SelectedRecord = CurrentRecord = noRecord;
    //SysDebug(1, "Addr", "RationalizeCurrentSelection set CurrentRecord=%d", CurrentRecord);
		return;
	}
	// Force CurrentRecord and SelectedRecord to reasonable values.
	MinVal = TopVisibleRecord;
	MaxVal = LastVisibleRecord();
	SelectedRecord = ( SelectedRecord < MinVal ) ? MinVal : SelectedRecord;
	SelectedRecord = ( SelectedRecord > MaxVal ) ? MaxVal : SelectedRecord;
	CurrentRecord = SelectedRecord;
  //SysDebug(1, "Addr", "RationalizeCurrentSelection set CurrentRecord=%d", CurrentRecord);
	PrvListSelectRecord( FrmGetActiveForm(), SelectedRecord, true );
}

#endif

/***********************************************************************
 *
 * FUNCTION:    PrvListScroll
 *
 * DESCRIPTION: This routine scrolls the list of names and phone numbers
 *              in the direction specified.
 *
 * PARAMETERS:  direction	- up or dowm
 *              units		- unit amount to scroll
 *              byLine		- if true, list scrolls in line units
 *									- if false, list scrolls in page units
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			art		6/5/95	Initial Revision
 *       frigino	8/14/97	Modified to scroll by line or page in units
 *       gap   	10/12/99	Close command bar before processing scroll
 *       gap   	10/15/99	Clean up selection handling after scroll
 *       gap   	10/25/99	Optimized scrolling to only redraw if item position changed
 *
 ***********************************************************************/
void PrvListScroll (WinDirectionType direction, UInt16 units, Boolean byLine)
{
	TableType* tblP;
	FormType* frmP;
	UInt16 rowsInPage;
	UInt16 newTopVisibleRecord;
	UInt16 prevTopVisibleRecord = TopVisibleRecord;

	#if WRISTPDA
	UInt16 OldTopVisibleRecord = TopVisibleRecord;
	#endif

	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus(0);

	frmP = FrmGetActiveForm();
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);
	// Safe. There must be at least one row in the table.
	rowsInPage = PrvListNumberOfRows(tblP) - 1;
	newTopVisibleRecord = TopVisibleRecord;

	// Scroll the table down.
	if (direction == winDown)
	{
		// Scroll down by line units
		if (byLine)
		{
			// Scroll down by the requested number of lines
			if (!ToolsSeekRecord (&newTopVisibleRecord, units, dmSeekForward))
			{
				// Tried to scroll past bottom. Goto last record
				newTopVisibleRecord = dmMaxRecordIndex;
				ToolsSeekRecord (&newTopVisibleRecord, 1, dmSeekBackward);
			}
		}
		// Scroll in page units
		else
		{
			// Try scrolling down by the requested number of pages
			if (!ToolsSeekRecord (&newTopVisibleRecord, units * rowsInPage, dmSeekForward))
			{
				// Hit bottom. Try going backwards one page from the last record
				newTopVisibleRecord = dmMaxRecordIndex;
				if (!ToolsSeekRecord (&newTopVisibleRecord, rowsInPage, dmSeekBackward))
				{
					// Not enough records to fill one page. Goto the first record
					newTopVisibleRecord = 0;
					ToolsSeekRecord (&newTopVisibleRecord, 0, dmSeekForward);
				}
			}
		}
	}
	// Scroll the table up
	else
	{
		// Scroll up by line units
		if (byLine)
		{
			// Scroll up by the requested number of lines
			if (!ToolsSeekRecord (&newTopVisibleRecord, units, dmSeekBackward))
			{
				// Tried to scroll past top. Goto first record
				newTopVisibleRecord = 0;
				ToolsSeekRecord (&newTopVisibleRecord, 0, dmSeekForward);
			}
		}
		// Scroll in page units
		else
		{
			// Try scrolling up by the requested number of pages
			if (!ToolsSeekRecord (&newTopVisibleRecord, units * rowsInPage, dmSeekBackward))
			{
				// Hit top. Goto the first record
				newTopVisibleRecord = 0;
				ToolsSeekRecord (&newTopVisibleRecord, 0, dmSeekForward);
			}
		}
	}

	// Avoid redraw if no change
	if (TopVisibleRecord != newTopVisibleRecord)
	{
		TopVisibleRecord = newTopVisibleRecord;
		CurrentRecord = noRecord;  	// scrolling always deselects current selection
		PrvListLoadTable(frmP);

		// Need to compare the previous top record to the current after PrvListLoadTable
		// as it will adjust TopVisibleRecord if drawing from newTopVisibleRecord will
		// not fill the whole screen with items.
		if (TopVisibleRecord != prevTopVisibleRecord)
			TblRedrawTable(tblP);
	}

	#if WRISTPDA
	if ( DmNumRecordsInCategory( AddrDB, CurrentCategory ) == 0 ) {
		// No records means no selection.
		SelectedRecord = CurrentRecord = noRecord;
	} else {
		// If we are on the first or last page and we get a
		// PageUp or PageDown then we will move the selection.
		Int16  VisibleTopRecordIndex, VisibleBottomRecordIndex;
		UInt16 TableFirstRecordIndex, TableLastRecordIndex;
		// Did we page scroll on this call to this routine?
		if ( OldTopVisibleRecord == TopVisibleRecord ) {
			// No, we did not, so we will check if we should move the selection.
			TableFirstRecordIndex = 0;
			ToolsSeekRecord( & TableFirstRecordIndex, 0, +1 );
			TableLastRecordIndex = dmMaxRecordIndex;
			ToolsSeekRecord( & TableLastRecordIndex, 0, -1 );
			VisibleTopRecordIndex = TopVisibleRecord;
			VisibleBottomRecordIndex = LastVisibleRecord();
			if ( VisibleBottomRecordIndex > TableLastRecordIndex )
				VisibleBottomRecordIndex = TableLastRecordIndex;
			// Check if we should move the selection.
			if ( (direction == winDown) ) {
				// Should we move the selection?
				if ( SelectedRecord < TableLastRecordIndex ) {
					if ( SelectedRecord < VisibleBottomRecordIndex ) {
						// Yes, we are on the last page, so force selection to last record.
						SelectedRecord = CurrentRecord = TableLastRecordIndex;
					}
				}
			} else
			// Did we get two PageUp keys in a row?
			if ( (direction == winUp) ) {
				// Should we move the selection?
				if ( SelectedRecord > TableFirstRecordIndex ) {
					if ( SelectedRecord > VisibleTopRecordIndex ) {
						// Yes, we are on the first page, so force selection to first record.
						SelectedRecord = CurrentRecord = TableFirstRecordIndex;
					}
				}
			}
		} else {
			// Yes, we did scroll, so we must update the SelectedRecord value.
			Int16  ActualDistance, RawDistance;
			UInt16 i, MatchRecord;
			// ActualDistance will be set to the number records actually scrolled.
			// This variable is not used by this code, but it useful to have
			// available when tracing and debugging this code.
			ActualDistance = 0;
			// RawDistance is the difference between the new and old top record indexes.
			// Note that this is NOT necessarily the number of records scrolled!
			RawDistance = TopVisibleRecord - OldTopVisibleRecord;
			if ( RawDistance > 0 ) {
				// We scrolled forward (down).
				MatchRecord = OldTopVisibleRecord;
				// Move the selection down by the actual number records we scrolled.
				for ( i = OldTopVisibleRecord; i < TopVisibleRecord; i++ ) {
					if ( ! ToolsSeekRecord( & MatchRecord, 1, +1 ) )
						break;
					ActualDistance++;
					ToolsSeekRecord( & SelectedRecord, 1, +1 );
					if ( MatchRecord == TopVisibleRecord )
						break;
				}
			} else if ( RawDistance < 0 ) {
				// We scrolled backward (up).
				MatchRecord = OldTopVisibleRecord;
				// Move the selection up by the actual number records we scrolled.
				for ( i = OldTopVisibleRecord; i > TopVisibleRecord; i-- ) {
					if ( ! ToolsSeekRecord( & MatchRecord, 1, -1 ) )
						break;
					ActualDistance--;
					ToolsSeekRecord( & SelectedRecord, 1, -1 );
					if ( MatchRecord == TopVisibleRecord )
						break;
				}
			}
		}
	}
	// Highlight the current selected record.
	PrvListSelectRecord( FrmGetActiveForm(), SelectedRecord, true );
	#endif

}


/***********************************************************************
 *
 * FUNCTION:    PrvListSelectRecord
 *
 * DESCRIPTION: Selects (highlights) a record on the table, scrolling
 *              the record if neccessary.  Also sets the CurrentRecord.
 *
 * PARAMETERS:  frmP			IN	form
 *				recordNum 		IN 	record to select
 *				forceSelection	IN	force selection
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         	Name   	Date      	Description
 *        	----   	----      	-----------
 *        	roger  	6/30/95   	Initial Revision
 *			aro		9/25/00		Add frmP as a parameter for frmUpdateEvent
 *								Add a boolean to force selection
 *
 ***********************************************************************/
void PrvListSelectRecord( FormType* frmP, UInt16 recordNum, Boolean forceSelection )
{
	Int16 row, column;
	TableType* tblP;
	UInt16 attr;

	if (recordNum == noRecord)
		return;
	ErrFatalDisplayIf (recordNum >= DmNumRecords(AddrDB), "Record outside AddrDB");


	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);

	#if WRISTPDA
	if (PrivateRecordVisualStatus > maskPrivateRecords)
	#else
	if (PrivateRecordVisualStatus > showPrivateRecords)
	#endif
	{
		// If the record is hidden stop trying to show it.
		if (!DmRecordInfo(AddrDB, recordNum, &attr, NULL, NULL) && (attr & dmRecAttrSecret))
		{
			CurrentRecord = noRecord;
      //SysDebug(1, "Addr", "PrvListSelectRecord set CurrentRecord=%d", CurrentRecord);
			TblUnhighlightSelection(tblP);
			return;
		}
	}



	// Don't change anything if the same record is selected
	if ((TblGetSelection(tblP, &row, &column)) &&
		(recordNum == TblGetRowID (tblP, row)) &&
		(!forceSelection))
	{
		return;
	}


	#if WRISTPDA
	// Remove old selection on masked row
	if ( TblRowMasked( tblP, row ) ) {
		TblMarkRowInvalid( tblP, row );
		TblRedrawTable(tblP);
	}
	#endif

	// See if the record is displayed by one of the rows in the table
	// A while is used because if TblFindRowID fails we need to
	// call it again to find the row in the reloaded table.
	while (!TblFindRowID(tblP, recordNum, &row))
	{

		// Scroll the view down placing the item
		// on the top row
		TopVisibleRecord = recordNum;

		// Make sure that TopVisibleRecord is visible in CurrentCategory
		if (CurrentCategory != dmAllCategories)
		{
			// Get the category and the secret attribute of the current record.
			DmRecordInfo (AddrDB, TopVisibleRecord, &attr, NULL, NULL);
			if ((attr & dmRecAttrCategoryMask) != CurrentCategory)
			{
				ErrNonFatalDisplay("Record not in CurrentCategory");
				CurrentCategory = (attr & dmRecAttrCategoryMask);
			}
		}

		PrvListLoadTable(frmP);
		TblRedrawTable(tblP);
	}


	// Select the item
	if (forceSelection)
	{
		TblUnhighlightSelection(tblP);
		TblSelectItem (tblP, row, nameAndNumColumn);
	}

	CurrentRecord = recordNum;
  //SysDebug(1, "Addr", "PrvListSelectRecord set CurrentRecord=%d", CurrentRecord);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListUpdateDisplay
 *
 * DESCRIPTION: This routine update the display of the list view
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the to do list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *				----	----		-----------
 *				art		6/5/95		Initial Revision
 *				ppo		10/13/99	Fixed bug #22753 (selection not redrawn)
 *				jmp		10/19/99	Changed previous fix to actually to set everything
 *									back up.  The previous change caused bug #23053, and
 *									didn't work in several cases anyway!  Also, optimized
 *									this routine space-wise.
 *				aro		9/26/00		Don't use GetActiveForm for frmRedrawUpdateCode
 *									Fix bug in debug ROM: selection was not restored after Dial or About
 *				fpa		10/26/00	Fixed bug #44352 (Selected line display problem when tapping
 *									menu | Dial, then cancel into Dial Number screen)
 *
 ***********************************************************************/
void PrvListUpdateDisplay( UInt16 updateCode )
{
	TableType* tblP;
	FormType* frmP;

	// Do not use active form here since the update event is broadcasted
	frmP = FrmGetFormPtr(ListView);
	tblP = ToolsGetFrmObjectPtr(frmP, ListTable);

	if (updateCode == frmRedrawUpdateCode)
	{
		TblUnhighlightSelection(tblP);	// Fixed bug #44352. If we don't do that, using a Debug rom, selection is too width when tapping Menu | Dial, then cancel into Dial Number screen (note is selected)
				
		FrmDrawForm(frmP);
		if (CurrentRecord != noRecord)
			PrvListSelectRecord(frmP, CurrentRecord, true);

	}

	if (updateCode & updateRedrawAll ||
		updateCode & updateFontChanged)
	{
		PrvListLoadTable(frmP);
		TblRedrawTable(tblP);
	}

	// Ensure selection is done event when everything is drawn again
	if (updateCode & updateRedrawAll ||
	    updateCode & updateFontChanged ||
		updateCode & updateSelectCurrentRecord)
	{
		if (CurrentRecord != noRecord)
			PrvListSelectRecord(frmP, CurrentRecord, true);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvListDeleteRecord
 *
 * DESCRIPTION: This routine deletes an address record. This routine is
 *              called when the delete button in the command bar is
 *              pressed when address book is in list view.  The benefit
 *					 this routine proides over DetailsDeleteRecord is that the
 *					 selection is maintained right up to the point where the address
 *					 is being deleted.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *         	Name	Date			Description
 *         	----	----			-----------
 *				gap	11/01/99		new
 *
 ***********************************************************************/
Boolean PrvListDeleteRecord (void)
{
	UInt16	ctlIndex;
	UInt16	buttonHit;
	FormType*	alert;
	Boolean	archive;
	TablePtr	table;


	// Display an alert to comfirm the operation.
	alert = FrmInitForm (DeleteAddrDialog);

	// Set the "save backup" checkbox to its previous setting.
	ctlIndex = FrmGetObjectIndex (alert, DeleteAddrSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);

	buttonHit = FrmDoDialog (alert);

	archive = FrmGetControlValue (alert, ctlIndex);

	FrmDeleteForm (alert);
	if (buttonHit == DeleteAddrCancel)
		return (false);

	// Remember the "save backup" checkbox setting.
	SaveBackup = archive;

	// Clear the highlight on the selection before deleting the item.
	table = ToolsGetObjectPtr (ListTable);
	TblUnhighlightSelection(table);

	ToolsDeleteRecord(archive);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    PrvListDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name		Date		Description
 *				----		----		-----------
 *				art		6/5/95	Initial Revision
 *				jmp		10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *										AddrGetDatabase().
 *				jwm	 1999-10-8	After deleting a record, highlight the new selection and
 *										clear the now possibly completely wrong lookup field.
 *				hou  2000-11-28 bug #44076 correction: ListOptionsSecurityCmd returns true
 *
 ***********************************************************************/
Boolean PrvListDoCommand (UInt16 command)
{
	UInt16 	newRecord;
	UInt16 	numCharsToHilite;
	Boolean	wasHiding;
	UInt16 	mode;

	switch (command)
	{
	case ListRecordBeamBusinessCardCmd:
		MenuEraseStatus (0);
		ToolsAddrBeamBusinessCard(AddrDB);
		return true;

	case ListRecordBeamCategoryCmd:
		MenuEraseStatus (0);
		TransferSendCategory(AddrDB, CurrentCategory, exgBeamPrefix, NoDataToBeamAlert);
		return true;

	case ListRecordSendCategoryCmd:
		MenuEraseStatus (0);
		TransferSendCategory(AddrDB, CurrentCategory, exgSendPrefix, NoDataToSendAlert);
		return true;

	case ListRecordDuplicateAddressCmd:
		if (CurrentRecord == noRecord)
		{
			//FrmAlert ();
			return true;
		}
		newRecord = ToolsDuplicateCurrentRecord (&numCharsToHilite, false);

		// If we have a new record take the user to be able to edit it
		// automatically.
		if (newRecord != noRecord)
		{
			NumCharsToHilite = numCharsToHilite;
			CurrentRecord = newRecord;
			FrmGotoForm (EditView);
		}
		return true;

	case ListRecordDialCmd:
		{
			UInt16 phoneIndex;
			Err error;
			AddrDBRecordType record;
			MemHandle recordH;

			if (CurrentRecord == noRecord)
			{
				SndPlaySystemSound (sndError);
				return true;
			}

			error = AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);
			if (error)
			{
				ErrNonFatalDisplay ("Record not found");
				return false;
			}
			phoneIndex = record.options.phones.displayPhoneForList;

			// Release the record.
			MemHandleUnlock(recordH);

			DialListShowDialog(CurrentRecord, phoneIndex, 0);
			return true;
		}

	case ListRecordDeleteRecordCmd:
		if (CurrentRecord != noRecord)
		{
			if (PrvListDeleteRecord ())
			{
				PrvListClearLookupString ();
				PrvListUpdateDisplay (updateRedrawAll | updateSelectCurrentRecord);
			}
		}
		else
			SndPlaySystemSound (sndError);
		return true;

	case ListRecordBeamRecordCmd:
		if (CurrentRecord != noRecord)
		{
			MenuEraseStatus (0);
			TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
		}
		else
			SndPlaySystemSound (sndError);
		return true;

	case ListOptionsFontCmd:
		{
			FontID newFont;
			
			MenuEraseStatus(0);
			newFont = ToolsSelectFont(AddrListFont);
			
			// The update event for font changed is post so when the
			// item is highlighted in the updateEvent handler, the drawing was made with a bad font
			// So force unhighlight here
			if (newFont != AddrListFont)
			{
				TblUnhighlightSelection(ToolsGetObjectPtr(ListTable));

				// now set the new font
				AddrListFont = newFont;
			}
			return true;
		}

	case ListOptionsListByCmd:
		MenuEraseStatus (0);
		PrvListClearLookupString();
		FrmPopupForm (PreferencesDialog);
		return true;

	case ListOptionsEditCustomFldsCmd:
		MenuEraseStatus (0);
		FrmPopupForm (CustomEditDialog);
		return true;

	case ListOptionsSecurityCmd:
		wasHiding = (PrivateRecordVisualStatus == hidePrivateRecords);

		PrivateRecordVisualStatus = SecSelectViewStatus();

		if (wasHiding != (PrivateRecordVisualStatus == hidePrivateRecords))
		{

			// Close the application's data file.
			DmCloseDatabase (AddrDB);

			mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
				dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

			AddrDBGetDatabase(&AddrDB, mode);
			ErrFatalDisplayIf(!AddrDB,"Can't reopen DB");
		}

		//For safety, simply reset the currentRecord
		TblReleaseFocus (ToolsGetObjectPtr (ListTable));
		PrvListUpdateDisplay (updateRedrawAll | updateSelectCurrentRecord);
		#if WRISTPDA
		// When changing security set SelectedRecord to the first record
		// of the category (if there is one), otherwise set to noRecord.
		TopVisibleRecord = 0;
		if ( ! ToolsSeekRecord( & TopVisibleRecord, 0, +1 ) )
			TopVisibleRecord = noRecord;
		SelectedRecord = CurrentRecord = TopVisibleRecord;
		RationalizeCurrentSelection();
		#endif
		//updateSelectCurrentRecord will cause currentRecord to be reset to noRecord if hidden or masked
		return true; // Hou: bug #44076 correction: used to be "break;" -> caused the event to not be handled

	case ListOptionsAboutCmd:
		MenuEraseStatus (0);
		AbtShowAbout (AddressBookCreator);
		return true;

	default:
		break;
	}
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvListHandleRecordSelection
 *
 * DESCRIPTION: This routine handles table selection in the list view,
 *					 either selecting the name to go to RecordView, or selecting
 *					 the phone number to dial.
 *
 *
 * PARAMETERS:	 event	- pointer to the table enter event
 *
 * RETURNED:	 whether the event was handled
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			peter	05/08/00	Initial Revision
 *			aro		06/22/00	Add dialing checking and feature
 *			aro		9/25/00		Disable selection when entering the table
 *
 ***********************************************************************/
Boolean PrvListHandleRecordSelection( EventType* event )
{
	TablePtr table;
	Int16 row, column, phoneX;
	Boolean isSelected, isPenDown;
	UInt16 recordNum;
	Err error;
	AddrDBRecordType record;
	MemHandle recordH;
	FontID currFont;
	Char * phone;
	RectangleType bounds, numberBounds;
	Coord x, y;
	UInt16 phoneIndex;

	// Disable the current selection
	CurrentRecord = noRecord;
  //SysDebug(1, "Addr", "PrvListHandleRecordSelection set CurrentRecord=%d", CurrentRecord);

	// Check if "Enable tap-dialing" is set in prefs
	if (!EnableTapDialing)
		return false;

	table = event->data.tblSelect.pTable;
	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;

	// If the column being tapped on isn't the name and number column,
	// let the table handle selection to view the note.
	if (column != nameAndNumColumn)
		return false;

	// If the record is masked, dialing is impossible, so just let
	//	the table handle selection.
	if (TblRowMasked(table, row))
		return false;

	// Extract the x coordinate of the start of the phone number for this row.
	// This was computed and stored in the row data when the row was drawn.
	phoneX = TblGetRowData(table, row);

	// If the user tapped on the name rather than the number, or on the space between them,
	// let the table handle selection so the user gets to view the record.
	if (event->screenX < phoneX)
		return false;

	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	recordNum = TblGetRowID (table, row);
	error = AddrDBGetRecord (AddrDB, recordNum, &record, &recordH);
	if (error)
	{
		ErrNonFatalDisplay ("Record not found");
		return false;
	}
	phoneIndex = record.options.phones.displayPhoneForList;
	phone = record.fields[ad_phone1 + phoneIndex];

	// If there is no phone number, if the phone is not part of supported
	// dial phone number, let the table handle selection.
	if ((phone == chrNull) || (!ToolsIsPhoneIndexSupported(&record, phoneIndex)) )
		goto CleanUpAndLeaveUnhandled;

	// The user tapped on the phone number, so instead of letting the
	// table handle selection, we highlight just the phone number.

	// First, deselect the row if it is selected.
	CurrentRecord = noRecord;
  //SysDebug(1, "Addr", "PrvListHandleRecordSelection set CurrentRecord=%d", CurrentRecord);
	TblUnhighlightSelection(table);

	// Set up the drawing state the way we want it.
	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));
	currFont = FntSetFont (AddrListFont);

	TblGetItemBounds (table, row, column, &bounds);
	numberBounds = bounds;
	numberBounds.extent.x -= phoneX - numberBounds.topLeft.x;		// Maintain same right side.
	numberBounds.topLeft.x = phoneX;

	// Extend left side of selection rectangle one pixel since we have the room.
	// It'd be great if we could extend the right side as well, so that the 'W'
	// for Work numbers wouldn't touch the edge of the selection rectangle, but
	// then it'd hit up against the note icon.
	numberBounds.topLeft.x--;
	numberBounds.extent.x ++;

	// Draw the phone number selected.
	PrvListReplaceTwoColors(&numberBounds, 0,
							UIObjectForeground, UIFieldBackground, UIObjectSelectedForeground, UIObjectSelectedFill);

	isSelected = true;
	do {
		PenGetPoint (&x, &y, &isPenDown);
		if (RctPtInRectangle (x, y, &numberBounds))
		{
			if (! isSelected)
			{
				isSelected = true;
				PrvListReplaceTwoColors(&numberBounds, 0,
										UIObjectForeground, UIFieldBackground, UIObjectSelectedForeground, UIObjectSelectedFill);
			}
		}
		else if (isSelected)
		{
			isSelected = false;
			PrvListReplaceTwoColors(&numberBounds, 0,
									UIObjectSelectedForeground, UIObjectSelectedFill, UIObjectForeground, UIFieldBackground);
		}
	} while (isPenDown);

	if (isSelected)
		PrvListReplaceTwoColors(&numberBounds, 0,
								UIObjectSelectedForeground, UIObjectSelectedFill, UIObjectForeground, UIFieldBackground);

	// Restore the previous drawing state.
	FntSetFont (currFont);
	WinPopDrawState();

	// Release the record.
	MemHandleUnlock(recordH);

	if (isSelected)
	{
		// Make it the current item
		CurrentRecord = recordNum;
    //SysDebug(1, "Addr", "PrvListHandleRecordSelection set CurrentRecord=%d", CurrentRecord);

		// Dial the number.
		return DialListShowDialog(recordNum, phoneIndex, 0);
	}

	return true;		// Don't let the table do any selection

CleanUpAndLeaveUnhandled:
	// Release the record.
	MemHandleUnlock(recordH);

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvListReplaceTwoColors
 *
 * DESCRIPTION: This routine does a selection or deselection effect by
 *					 replacing foreground and background colors with a new pair
 *					 of colors. In order to reverse the process, you must pass
 *					 the colors in the opposite order, so that the current
 *					 and new colors are known to this routine. This routine
 *					 correctly handling the cases when two or more of these
 *					 four colors are the same, but it requires that the
 *					 affected area of the screen contains neither of the
 *					 given NEW colors, unless these colors are the same as
 *					 one of the old colors.
 *
 * PARAMETERS:	 rP  - pointer to a rectangle to 'invert'
 *					 cornerDiam	- corner diameter
 *					 oldForeground	- UI color currently used for foreground
 *					 oldBackground	- UI color currently used for background
 *					 newForeground	- UI color that you want for foreground
 *					 newBackground	- UI color that you want for background
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			peter	05/19/00	Initial Revision
 *
 ***********************************************************************/
void PrvListReplaceTwoColors (const RectangleType *rP, UInt16 cornerDiam, UIColorTableEntries oldForeground, UIColorTableEntries oldBackground, UIColorTableEntries newForeground, UIColorTableEntries newBackground)
{
	WinPushDrawState();
	WinSetDrawMode(winSwap);
	WinSetPatternType (blackPattern);

	if (UIColorGetTableEntryIndex(newBackground) == UIColorGetTableEntryIndex(oldForeground))
		if (UIColorGetTableEntryIndex(newForeground) == UIColorGetTableEntryIndex(oldBackground))
		{
			// Handle the case when foreground and background colors change places,
			// such as on black and white systems, with a single swap.
			WinSetBackColor(UIColorGetTableEntryIndex(oldBackground));
			WinSetForeColor(UIColorGetTableEntryIndex(oldForeground));
			WinPaintRectangle(rP, cornerDiam);
		}
		else
		{
			// Handle the case when the old foreground and the new background
			// are the same, using two swaps.
			WinSetBackColor(UIColorGetTableEntryIndex(oldForeground));
			WinSetForeColor(UIColorGetTableEntryIndex(oldBackground));
			WinPaintRectangle(rP, cornerDiam);
			WinSetBackColor(UIColorGetTableEntryIndex(oldBackground));
			WinSetForeColor(UIColorGetTableEntryIndex(newForeground));
			WinPaintRectangle(rP, cornerDiam);
		}
	else if (UIColorGetTableEntryIndex(oldBackground) == UIColorGetTableEntryIndex(newForeground))
	{
		// Handle the case when the old background and the new foreground
		// are the same, using two swaps.
		WinSetBackColor(UIColorGetTableEntryIndex(newForeground));
		WinSetForeColor(UIColorGetTableEntryIndex(oldForeground));
		WinPaintRectangle(rP, cornerDiam);
		WinSetBackColor(UIColorGetTableEntryIndex(newBackground));
		WinSetForeColor(UIColorGetTableEntryIndex(oldForeground));
		WinPaintRectangle(rP, cornerDiam);
	}
	else
	{
		// Handle the case when no two colors are the same, as is typically the case
		// on color systems, using two swaps.
		WinSetBackColor(UIColorGetTableEntryIndex(oldBackground));
		WinSetForeColor(UIColorGetTableEntryIndex(newBackground));
		WinPaintRectangle(rP, cornerDiam);
		WinSetBackColor(UIColorGetTableEntryIndex(oldForeground));
		WinSetForeColor(UIColorGetTableEntryIndex(newForeground));
		WinPaintRectangle(rP, cornerDiam);
	}

	WinPopDrawState();
}
