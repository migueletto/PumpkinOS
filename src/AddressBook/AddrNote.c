/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrNote.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the Address Book Note screen
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrNote.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddressRsc.h"

#include <Category.h>
#include <UIResources.h>
#include <StringMgr.h>
#include <PhoneLookup.h>
#include <TraceMgr.h>

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

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void		PrvNoteViewInit (FormPtr frm) SEC("code2");
static void		PrvNoteViewDrawTitleAndForm (FormPtr frm) SEC("code2");
static void		PrvNoteViewUpdateScrollBar (void) SEC("code2");
static void		PrvNoteViewLoadRecord (void) SEC("code2");
static void		PrvNoteViewSave (void) SEC("code2");
static Boolean	PrvNoteViewDeleteNote (void) SEC("code2");
static Boolean	PrvNoteViewDoCommand (UInt16 command) SEC("code2");
static void		PrvNoteViewScroll (Int16 linesToScroll, Boolean updateScrollbar) SEC("code2");
static void		PrvNoteViewPageScroll (WinDirectionType direction) SEC("code2");

#if WRISTPDA

static void ListViewUpdateScrollButtons( void );

static void NoteViewUpdateScrollButtons( void );

#endif


/***********************************************************************
 *
 * FUNCTION:    NoteViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the NoteView
 *              of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *       art  	6/5/95   	Initial Revision
 *			jmp	9/8/99		Made this routine more consistent with the
 *									other built-in apps that have it.
 *			jmp	9/27/99		Combined NoteViewDrawTitle() & FrmUpdateForm()
 *									into a single routine that is now called
 *									PrvNoteViewDrawTitleAndForm().
 *			peter	09/15/00		Disable attention indicator because title is custom.
 *
 ***********************************************************************/
Boolean NoteViewHandleEvent (EventType * event)
{
	FormType* frmP;
	Boolean handled = false;
	FieldPtr fldP;


	switch (event->eType)
	{
		case frmOpenEvent:
			TraceOutput(TL(appErrorClass, "NoteViewHandleEvent() - frmOpenEvent"));
			frmP = FrmGetActiveForm ();
			PrvNoteViewInit (frmP);
			PrvNoteViewDrawTitleAndForm (frmP);
			PrvNoteViewUpdateScrollBar ();
			FrmSetFocus (frmP, FrmGetObjectIndex (frmP, NoteField));
			handled = true;
			break;
	
		case frmCloseEvent:
			TraceOutput(TL(appErrorClass, "NoteViewHandleEvent() - frmCloseEvent"));

			AttnIndicatorEnable(true);		// Custom title doesn't support attention indicator.

			if ( UnnamedRecordStringPtr != 0 )
			{
				MemPtrUnlock(UnnamedRecordStringPtr);
				UnnamedRecordStringPtr = NULL;
			}

			if ( UnnamedRecordStringH != 0 )
			{
				TraceOutput(TL(appErrorClass, "NoteViewHandleEvent() - frmCloseEvent - freeing UnnamedRecordStringH"));
				DmReleaseResource(UnnamedRecordStringH);
				UnnamedRecordStringH = NULL;
			}
			
			if ( FldGetTextHandle (ToolsGetObjectPtr (NoteField)))
				PrvNoteViewSave ();
			break;
	
		case keyDownEvent:
			#if WRISTPDA
			// Update scroll buttons to handle degenerate case where scrolling
			// is enabled when initially editing a memo, but then subsequently
			// disabled after some lines have been deleted.
			NoteViewUpdateScrollButtons();
			// Translate the Enter and Back keys to a Done button event.
			if ( ( event->data.keyDown.chr == vchrThumbWheelPush ) ||
				 ( event->data.keyDown.chr == vchrThumbWheelBack ) ) {
				EventType newEvent;
				FormPtr frm = FrmGetActiveForm();
				MemSet (&newEvent, sizeof(EventType), 0);
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.on = true;
				newEvent.data.ctlSelect.controlID = NoteDoneButton;
				newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
					FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
				EvtAddEventToQueue( &newEvent );
				return true;
			} else
			if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
				// Translate the RockerUp key to a PageUp event.
				event->data.keyDown.chr = vchrPageUp;
				EvtAddEventToQueue( event );
				return true;
			} else
			if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
				// Translate the RockerDown key to a PageDown event.
				event->data.keyDown.chr = vchrPageDown;
				EvtAddEventToQueue( event );
				return true;
			} else
			#endif
			if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
			{
				PrvNoteViewSave ();
				FrmGotoForm (ListView);
				handled = true;
			}
	
			else if (EvtKeydownIsVirtual(event))
			{
				if (event->data.keyDown.chr == vchrPageUp)
				{
					PrvNoteViewPageScroll (winUp);
					handled = true;
				}
				else if (event->data.keyDown.chr == vchrPageDown)
				{
					PrvNoteViewPageScroll (winDown);
					handled = true;
				}
			}
			break;
	
		#if WRISTPDA
		case ctlRepeatEvent:
			{
				// Handle scroll button event.
				ControlPtr ctl;
				EventType  newEvent;
				// Redraw the control to eliminate inverted state.
				ctl = ToolsGetObjectPtr( event->data.ctlRepeat.controlID );
				CtlEraseControl( ctl );
				CtlDrawControl( ctl );
				// Translate the repeating button event to a PageUp/PageDown key event.
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
			break;
		#endif

		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case NoteDoneButton:
					PrvNoteViewSave ();
		
					// When we return to the ListView highlight this record.
					if (PriorAddressFormID == ListView)
						ListViewSelectThisRecord = CurrentRecord;
		
					FrmGotoForm(PriorAddressFormID);
					handled = true;
					break;
		
				case NoteDeleteButton:
					if (PrvNoteViewDeleteNote ())
						FrmGotoForm (PriorAddressFormID);
		
					ListViewSelectThisRecord = noRecord;
					handled = true;
					break;
		
				default:
					break;
			}
			break;
	
		case fldChangedEvent:
			frmP = FrmGetActiveForm ();
			PrvNoteViewUpdateScrollBar ();
			handled = true;
			break;
	
		case menuEvent:
			return PrvNoteViewDoCommand (event->data.menu.itemID);
	
		case frmGotoEvent:
			frmP = FrmGetActiveForm ();
			CurrentRecord = event->data.frmGoto.recordNum;
			PrvNoteViewInit(frmP);
			fldP = ToolsGetFrmObjectPtr(frmP, NoteField);
			FldSetScrollPosition(fldP, event->data.frmGoto.matchPos);
			FldSetSelection(fldP, event->data.frmGoto.matchPos,
							event->data.frmGoto.matchPos + event->data.frmGoto.matchLen);
			PrvNoteViewDrawTitleAndForm (frmP);
			PrvNoteViewUpdateScrollBar();
			FrmSetFocus(frmP, FrmGetObjectIndex(frmP, NoteField));
			handled = true;
			break;
	
		case frmUpdateEvent:
			if (event->data.frmUpdate.updateCode & updateFontChanged)
			{
				fldP = ToolsGetObjectPtr(NoteField);
				FldSetFont(fldP, NoteFont);
				PrvNoteViewUpdateScrollBar();
			}
			else
			{
				// Handle the case that form is not active (frmRedrawUpdateCode)
				frmP = FrmGetFormPtr(NewNoteView);
				PrvNoteViewDrawTitleAndForm(frmP);
			}
			handled = true;
			break;
	
		case sclRepeatEvent:
			PrvNoteViewScroll (event->data.sclRepeat.newValue - event->data.sclRepeat.value, false);
			break;
	
		default:
			break;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewCreate
 *
 * DESCRIPTION: Make sure there is a note field to edit.  If one doesn't
 * exist make one.
 *
 *   The main reason this routine exists is to make sure we can edit a note
 * before we close the current form.
 *
 * PARAMETERS:  CurrentRecord set
 *
 * RETURNED:    true if a note field exists to edit
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   10/19/95   Initial Revision
 *         ryw      2/18/00  Added cast to satisfy const cstring checking, should be safe
 *
 ***********************************************************************/
Boolean NoteViewCreate (void)
{
	AddrDBRecordType record;
	AddrDBRecordFlags bit;
	MemHandle recordH;
	Err err;


	AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);


	// Since we are going to edit in place, add a note field if there
	// isn't one
	if (!record.fields[ad_note])
	{
		record.fields[ad_note] = (char *)"";
		//bit.allBits = (UInt32)1 << ad_note;
		bit.allBits = BitAtPosition(ad_note);
		err = AddrDBChangeRecord(AddrDB, &CurrentRecord, &record, bit);
		if (err)
		{
			MemHandleUnlock(recordH);
			FrmAlert(DeviceFullAlert);
			return false;            // can't make an note field.
		}

	}
	else
	{
		MemHandleUnlock(recordH);
	}

	return true;                  // a note field exists.
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewDelete
 *
 * DESCRIPTION: Deletes the note field from the current record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
void NoteViewDelete (void)
{
	AddrDBRecordType record;
	MemHandle recordH;
	AddrDBRecordFlags changedField;
	Err err;


	AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);
	record.fields[ad_note] = NULL;
	//changedField.allBits = (UInt32)1 << ad_note;
	changedField.allBits = BitAtPosition(ad_note);
	err = AddrDBChangeRecord(AddrDB, &CurrentRecord, &record, changedField);
	if (err)
	{
		MemHandleUnlock(recordH);
		FrmAlert(DeviceFullAlert);
		return;
	}


	// Mark the record dirty.
	ToolsDirtyRecord (CurrentRecord);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewInit
 *
 * DESCRIPTION: This routine initials the Edit View form.
 *
 * PARAMETERS:  frm - pointer to the Edit View form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *	        	----	----		-----------
 *				art   6/5/95	Initial Revision
 *				jmp	9/23/99	Eliminate code to hide unused font controls
 *									now that we're using a NoteView form that doesn't
 *									have them anymore.
 *				peter	09/20/00	Disable attention indicator because title is custom.
 *
 ***********************************************************************/
void PrvNoteViewInit( FormType* frmP )
{
	FieldPtr       fld;
	FieldAttrType  attr;

	AttnIndicatorEnable(false);		// Custom title doesn't support attention indicator.
	PrvNoteViewLoadRecord ();

	// Have the field send events to maintain the scroll bar.
	fld = ToolsGetFrmObjectPtr(frmP, NoteField);
	FldGetAttributes (fld, &attr);
	attr.hasScrollBar = true;
	FldSetAttributes (fld, &attr);
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewDrawTitleAndForm
 *
 * DESCRIPTION: Draw the form and the title of the note view.  The title should be
 * the names that appear for the record on the list view.
 *
 * PARAMETERS:  frm, FormPtr to the form to draw
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  6/21/95   Initial Revision
 *         jmp    9/27/99   Square off the NoteView title so that it covers up
 *                          the blank Form title used to trigger the menu on taps
 *                          to the title area.  Also, set the NoteView title's color
 *                          to match the standard Form title colors.  Eventually, we
 *                          should add a variant to Forms that allows for NoteView
 *                          titles directly.  This "fixes" bug #21610.
 *         jmp    9/29/99   Fix bug #22412.  Ensure that title-length metrics are
 *                          computed AFTER the NoteView's title font has been set.
 *         jmp   12/02/99   Fix bug #24377.  Don't call WinScreenUnlock() if WinScreenLock()
 *                          fails.
 *			  peter 05/26/00	 Ensure font height isn't used until font is set.
 *
 ***********************************************************************/
void PrvNoteViewDrawTitleAndForm (FormPtr frm)
{
	Coord x, y;
	Int16 fieldSeparatorWidth;
	Int16 shortenedFieldWidth;
	Char * name1;
	Char * name2;
	Int16 name1Length;
	Int16 name2Length;
	Int16 name1Width;
	Int16 name2Width;
	Int16 nameExtent;
	Coord formWidth;
	RectangleType r;
	FontID curFont;
	RectangleType eraseRect,drawRect;
	Boolean name1HasPriority;
	IndexedColorType curForeColor;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;
	AddrDBRecordType record;
	MemHandle recordH;
	UInt8 * lockedWinP;
	//Err error;

	// "Lock" the screen so that all drawing occurs offscreen to avoid
	// the anamolies associated with drawing the Form's title then drawing
	// the NoteView title.  We REALLY need to make a variant for doing
	// this in a more official way!
	//
	lockedWinP = WinScreenLock (winLockCopy);

	FrmDrawForm (frm);

	// Peform initial set up.
	//
	FrmGetFormBounds (frm, &r);
	formWidth = r.extent.x;
	x = 2;
	y = 1;
	nameExtent = formWidth - 4;

	// Save/Set window colors and font.  Do this after FrmDrawForm() is called
	// because FrmDrawForm() leaves the fore/back colors in a state that we
	// don't want here.
	//
	curForeColor = WinSetForeColor (UIColorGetTableEntryIndex(UIFormFrame));
	curBackColor = WinSetBackColor (UIColorGetTableEntryIndex(UIFormFill));
	curTextColor = WinSetTextColor (UIColorGetTableEntryIndex(UIFormFrame));
	#if WRISTPDA
	curFont = FntSetFont (FossilBoldFont);
	#else
	curFont = FntSetFont (boldFont);
	#endif

	RctSetRectangle (&eraseRect, 0, 0, formWidth, FntLineHeight()+4);
	RctSetRectangle (&drawRect, 0, 0, formWidth, FntLineHeight()+2);

	// Erase the Form's title area and draw the NoteView's.
	//
	WinEraseRectangle (&eraseRect, 0);
	WinDrawRectangle (&drawRect, 3);

	/*error =*/ AddrDBGetRecord(AddrDB, CurrentRecord, &record, &recordH);
	//ErrNonFatalDisplayIf(error, "Record not found");
	name1HasPriority = ToolsDetermineRecordName(&record, &shortenedFieldWidth, &fieldSeparatorWidth, SortByCompany, &name1, &name1Length, &name1Width, &name2, &name2Length, &name2Width, &UnnamedRecordStringPtr, &UnnamedRecordStringH, nameExtent);

	ToolsDrawRecordName(name1, name1Length, name1Width, name2, name2Length, name2Width,
				   nameExtent, &x, y, shortenedFieldWidth, fieldSeparatorWidth, true,
				   name1HasPriority || !SortByCompany, true);

	// Now that we've drawn everything, blast it all back on the screen at once.
	//
	if (lockedWinP)
		WinScreenUnlock ();

	// Unlock the record that AddrGetRecord() implicitly locked.
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

	fld = ToolsGetObjectPtr (NoteField);

	// Update the Down button state.

	ctlDown = ToolsGetObjectPtr( NotePageDown );

	// Can scroll the field down?

	if ( FldScrollable( fld, winDown ) ) {
		Down = true;
	}

	CtlSetLabel( ctlDown, ( Down == true ) ? DownArrowEnabled : DownArrowDisabled );
	CtlSetEnabled( ctlDown, ( Down == true ) );

	// Update the Up button state.

	ctlUp = ToolsGetObjectPtr( NotePageUp );

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
 * FUNCTION:    PrvNoteViewUpdateScrollBar
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
void PrvNoteViewUpdateScrollBar (void)
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

	fld = ToolsGetObjectPtr (NoteField);
	bar = ToolsGetObjectPtr (NoteScrollBar);

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
 * FUNCTION:    PrvNoteViewLoadRecord
 *
 * DESCRIPTION: Load the record's note field into the field object
 * for editing in place.  The note field is too big (4K) to edit in
 * the heap.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			roger		6/21/95	Initial Revision
 *			roger		8/25/95	Changed to edit in place
 *			jmp		10/11/99	Replaced private MemDeref() call with MemHandleLock() so
 *									that the SDK build will work.
 *
 ***********************************************************************/
void PrvNoteViewLoadRecord (void)
{
	UInt16 offset;
	FieldPtr fld;
	MemHandle recordH;
	Char * ptr;
	AddrDBRecordType record;

	// Get a pointer to the memo field.
	fld = ToolsGetObjectPtr (NoteField);

	// Set the font used in the memo field.
	FldSetFont (fld, NoteFont);

	AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);

	// CreateNote will have been called before the NoteView was switched
	// to.  It will have insured that a note field exists.

	// Find out where the note field is to edit it
	ptr = MemHandleLock (recordH);
	offset = record.fields[ad_note] - ptr;
	FldSetText (fld, recordH, offset, StrLen(record.fields[ad_note])+1);

	// Unlock recordH twice because AddrGetRecord() locks it, and we had to lock
	// it to deref it.  Whacky.
	MemHandleUnlock(recordH);
	MemHandleUnlock(recordH);
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewSave
 *
 * DESCRIPTION: This routine
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   	6/5/95    Initial Revision
 *         roger  8/25/95   Changed to edit in place
 *
 ***********************************************************************/
void PrvNoteViewSave (void)
{
	FieldPtr fld;
	int textLength;


	fld = ToolsGetObjectPtr (NoteField);


	// If the field wasn't modified then don't do anything
	if (FldDirty (fld))
	{
		// Release any free space in the note field.
		FldCompactText (fld);

		ToolsDirtyRecord (CurrentRecord);
	}


	textLength = FldGetTextLength(fld);

	// Clear the handle value in the field, otherwise the handle
	// will be freed when the form is disposed of,  this call also unlocks
	// the handle that contains the note string.
	FldSetTextHandle (fld, 0);


	// Empty fields are not allowed because they cause problems
	if (textLength == 0)
		NoteViewDelete();
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewDeleteNote
 *
 * DESCRIPTION: This routine deletes a the note field from a to do record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the note was deleted.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
Boolean PrvNoteViewDeleteNote (void)
{
	FieldPtr fld;

	TraceOutput(TL(appErrorClass, "PrvNoteViewDeleteNote() - UnnamedRecordStringH = %hu - Before FrmAlert()", UnnamedRecordStringH));

	// CodeWarrior in Debug creates a problem... FrmAlert() sets UnnamedRecordStringH to 0 -> memory leak
	if (FrmAlert(DeleteNoteAlert) != DeleteNoteYes)
		return (false);

	TraceOutput(TL(appErrorClass, "PrvNoteViewDeleteNote() - UnnamedRecordStringH = %hu - After FrmAlert()", UnnamedRecordStringH));

	// Unlock the handle that contains the text of the memo.
	fld = ToolsGetObjectPtr (NoteField);
	ErrFatalDisplayIf ((! fld), "Bad field");

	// Clear the handle value in the field, otherwise the handle
	// will be freed when the form is disposed of. this call also
	// unlocks the handle the contains the note string.
	FldCompactText (fld);
	FldSetTextHandle (fld, 0);


	NoteViewDelete();

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewDoCommand
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
 *       art   6/5/95   Initial Revision
 *			jmp	9/8/99	Moved the noteFontCmd case to where it is in this
 *								routine in all the other NoteView implementations.
 *			jmp	9/17/99	Eliminate the goto top/bottom of page menu items
 *								as NewNoteView no longer supports them.
 *
 ***********************************************************************/
Boolean PrvNoteViewDoCommand (UInt16 command)
{
	FieldPtr fld;
	Boolean handled = true;

	switch (command)
	{
	case newNoteFontCmd:
		NoteFont = ToolsSelectFont (NoteFont);
		break;

	case newNotePhoneLookupCmd:
		fld = ToolsGetObjectPtr (NoteField);
		PhoneNumberLookup (fld);
		break;

	default:
		handled = false;
	}
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewScroll
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
 * 		Name	Date		Description
 * 		----	----		-----------
 * 		art	7/1/96	Initial Revision
 *			grant	2/2/99	Use PrvNoteViewUpdateScrollBar()
 *
 ***********************************************************************/
void PrvNoteViewScroll (Int16 linesToScroll, Boolean updateScrollbar)
{
	UInt16           blankLines;
	FieldPtr         fld;

	fld = ToolsGetObjectPtr (NoteField);
	blankLines = FldGetNumberOfBlankLines (fld);

	if (linesToScroll < 0)
		FldScrollField (fld, -linesToScroll, winUp);
	else if (linesToScroll > 0)
		FldScrollField (fld, linesToScroll, winDown);

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if ((blankLines && linesToScroll < 0) || updateScrollbar)
	{
		PrvNoteViewUpdateScrollBar();
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvNoteViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page up or down.
 *
 * PARAMETERS:   direction     up or down
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   7/1/96   Initial Revision
 *			  grant 2/2/99		Use PrvNoteViewScroll() to do actual scrolling
 *
 ***********************************************************************/
void PrvNoteViewPageScroll (WinDirectionType direction)
{
	Int16 linesToScroll;
	FieldPtr fld;

	fld = ToolsGetObjectPtr (NoteField);

	if (FldScrollable (fld, direction))
	{
		linesToScroll = FldGetVisibleLines (fld) - 1;

		if (direction == winUp)
			linesToScroll = -linesToScroll;

		PrvNoteViewScroll(linesToScroll, true);
	}
}


