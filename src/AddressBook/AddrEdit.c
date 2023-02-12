/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrEdit.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *   This is the Address Book application's edit form module.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <TraceMgr.h>
#include <TextMgr.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <AboutBox.h>
#include <Category.h>
#include <Graffiti.h>
#include <Menu.h>
#include <UIResources.h>

#include <PalmUtils.h>

#include "sec.h"

#include "AddrEdit.h"

#include "AddressDB.h"
#include "AddrDialList.h"
#include "Address.h"
#include "AddressAutoFill.h"
#include "AddressRsc.h"
#include "AddrDefines.h"
#include "AddrTools.h"
#include "AddrDetails.h"
#include "AddrNote.h"
#include "AddressTransfer.h"

#include "SysDebug.h"


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

#if WRISTPDA
#define addrEditLabelFont	FossilStdFont
#define addrEditBlankFont	FossilStdFont
#else
#define addrEditLabelFont	stdFont
#define addrEditBlankFont	stdFont
#endif

#define noFieldIndex		0xff

// Resource type used to specify order of fields in Edit view.
#define	fieldMapRscType		'fmap'

// Address edit table's rows and columns
#define editLabelColumn		0
#define editDataColumn		1

#define spaceBeforeDesc		2

#define editLastFieldIndex	17

#define isPhoneField(f)		(f >= firstPhoneField && f <= lastPhoneField)

#define editInvalidRow		((UInt16) 0xFFFF)


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

static UInt16				CurrentFieldIndex;

// global to remember the row of the last tapped tel label
static UInt16				CurrentTableRow;

// The following structure maps row in the edit table to fields in the
// address record.  This controls the order in which fields are edited.
// Valid after PrvEditInit.
static const AddressFields* FieldMap;
static MemHandle			FieldMapH;

// Valid after PrvEditInit
static Char * EditPhoneListChoices[numPhoneLabels];


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void		PrvEditInit( FormPtr frmP, Boolean leaveDataLocked ) SEC("code2");
static void		PrvEditInitTableRow( FormType* frmP, TablePtr table, UInt16 row, UInt16 fieldIndex, Int16 rowHeight, FontID fontID, AddrDBRecordPtr record, AddrAppInfoPtr appInfoPtr ) SEC("code2");
static Boolean	PrvEditHandleSelectField (Int16 row, Int16 column) SEC("code2");
static void		PrvEditRestoreEditState( FormType* frmP ) SEC("code2");
static void		PrvEditSetGraffitiMode (FieldPtr fld, UInt16 currentField) SEC("code2");
static Err		PrvEditGetRecordField (void * table, Int16 row, Int16 /*column*/, Boolean editing, MemHandle * textH, Int16 * textOffset, Int16 * textAllocSize, FieldPtr fld) SEC("code2");
static Boolean	PrvEditSaveRecordField (void * table, Int16 row, Int16 /*column*/) SEC("code2");
static UInt16	PrvEditSaveRecord () SEC("code2");
static void		PrvEditSelectCategory (void) SEC("code2");
static void		PrvEditUpdateScrollers (FormPtr frmP, UInt16 bottomFieldIndex, Boolean lastItemClipped) SEC("code2");
static UInt16	PrvEditGetFieldHeight (TablePtr table, UInt16 fieldIndex, Int16 columnWidth, Int16 maxHeight, AddrDBRecordPtr record, FontID * fontIdP) SEC("code2");
static void		PrvEditDrawBusinessCardIndicator (FormPtr formP) SEC("code2");
static void		PrvEditResizeDescription (EventType * event) SEC("code2");
static void		PrvEditScroll (WinDirectionType direction) SEC("code2");
static void		PrvEditNextField (WinDirectionType direction) SEC("code2");
static void		PrvEditUpdateCustomFieldLabels( FormType* frmP) SEC("code2");
static void		PrvEditUpdateDisplay( UInt16 updateCode ) SEC("code2");
static Boolean	PrvEditDoCommand (UInt16 command) SEC("code2");
static Boolean	PrvEditAutoFill (EventPtr event) SEC("code2");
static void		PrvEditDialCurrent( void ) SEC("code2");
static void		PrvEditLoadTable( FormType* frmP ) SEC("code2");

/***********************************************************************
 *
 * FUNCTION:    EditHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit View"
 *              of the Address Book application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 *	HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		art		06/05/95	Created by Art Lamb.
 *		kwk		07/29/99	Unlock FieldMap in frmCloseEvent block.
 *		jmp		09/17/99	Use NewNoteView instead of NoteView.
 *		fpa		10/23/00	Fixed bug #27480 - Font Style: The default font
 *							setting did not get refresh in the Address Edit
 *							View when entering text in the last name field
 *							after you entered the first name and tap done
 *		gap		10/27/00	change the command bar initialization to allow field
 *							code to add cut, copy, paste, & undo commands as 
 *							appropriate rather than adding a fixed set of selections.
 *
 ***********************************************************************/
Boolean EditHandleEvent (EventType * event)
{
	FormType* frmP;
	TablePtr tableP;
	Boolean handled = false;
	Int16 row;
	UInt32 numLibs;
	
	switch (event->eType)
	{
		case frmOpenEvent:
		{
			UInt16 tableIndex;
			FieldPtr fldP;
	
      //SysDebug(1, "Addr", "EditHandleEvent frmOpenEvent");
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - frmOpenEvent"));
			frmP = FrmGetActiveForm ();
			PrvEditInit (frmP, true);
			tableIndex = FrmGetObjectIndex(frmP, EditTable);
			tableP = FrmGetObjectPtr (frmP, tableIndex);
	
			// Make sure the field which will get the focus is visible
			while (!TblFindRowID (tableP, EditRowIDWhichHadFocus, &row))
			{
				TopVisibleFieldIndex = EditRowIDWhichHadFocus;
				CurrentFieldIndex = EditRowIDWhichHadFocus;
				PrvEditLoadTable(frmP);
			}
			FrmDrawForm (frmP);
			PrvEditDrawBusinessCardIndicator (frmP);
	
			// Now set the focus.
			FrmSetFocus(frmP, tableIndex);
			TblGrabFocus (tableP, row, editDataColumn);
			fldP = TblGetCurrentField(tableP);
			FldGrabFocus (fldP);
	
			// If NumCharsToHilite is not 0, then we know that we are displaying
			// a duplicated message for the first time and we must hilite the last
			// NumCharsToHilite of the field (first name) to indicate the modification
			// to that duplicated field.
			if (NumCharsToHilite > 0)
			{
				EditFieldPosition = FldGetTextLength (fldP);
	
				// Now hilite the chars added.
				FldSetSelection (fldP, EditFieldPosition - NumCharsToHilite, EditFieldPosition);
				NumCharsToHilite = 0;
			}
	
			FldSetInsPtPosition (fldP, EditFieldPosition);
	
			PriorAddressFormID = FrmGetFormId (frmP);
			
			// Simulate a tap in last name field in order to fix bug #27480
			PrvEditHandleSelectField(EditRowIDWhichHadFocus - TopVisibleFieldIndex, 1);
			
			handled = true;
			break;
		}
	
		case frmCloseEvent:
		{
			AddrAppInfoPtr appInfoPtr;
	
      //SysDebug(1, "Addr", "EditHandleEvent frmCloseEvent");
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - frmCloseEvent"));
			// Check if the record is empty and should be deleted.  This cannot
			// be done earlier because if the record is deleted there is nothing
			// to display in the table.
			PrvEditSaveRecord ();
	
			// We need to unlock the block containing the phone labels.
			appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
			MemPtrUnlock(appInfoPtr);	// Call to AddrAppInfoGetPtr did a lock
			MemPtrUnlock(appInfoPtr);   // Unlock lock in PrvEditInit
	
			// We need to unlock the FieldMap resource, which was also locked
			// in PrvEditInit.
			MemHandleUnlock(FieldMapH);
			DmReleaseResource(FieldMapH);
			FieldMap = 0;
			break;
		}
	
		case tblEnterEvent:
      //SysDebug(1, "Addr", "EditHandleEvent tblEnterEvent %d", event->data.tblEnter.row);
			// if a phone label is tapped: store current 
			if (isPhoneField(FieldMap[TblGetRowID(event->data.tblEnter.pTable, event->data.tblEnter.row)]))
			{
				CurrentTableRow = event->data.tblEnter.row;
			}
			break;
			
		case winExitEvent:
      //SysDebug(1, "Addr", "EditHandleEvent winExitEvent");
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - winExitEvent (%us)", CurrentFieldIndex));
			// if we exits a window and a phone label was tapped: redraw the associated row
			// to replace the semicolon character
			if (CurrentTableRow != editInvalidRow)
			{
				tableP = ToolsGetFrmObjectPtr(FrmGetActiveForm(), EditTable);
				TblMarkRowInvalid(tableP, CurrentTableRow);
				TblRedrawTable(tableP);				
				CurrentTableRow = editInvalidRow;
			}
			break;
			
		case tblSelectEvent:
      //SysDebug(1, "Addr", "EditHandleEvent tblSelectEvent %d", event->data.tblSelect.row);
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - tblSelectEvent"));
			// Select the field if it's different than the one selected before.  This means the selection
			// is on a different row or the selection is a phone label.
			if (CurrentFieldIndex != TblGetRowID (event->data.tblSelect.pTable, event->data.tblSelect.row) ||
				  (event->data.tblSelect.column == editLabelColumn && isPhoneField(FieldMap[TblGetRowID(event->data.tblSelect.pTable, event->data.tblSelect.row)])))
				PrvEditHandleSelectField (event->data.tblSelect.row, event->data.tblSelect.column);
			
			CurrentTableRow = editInvalidRow;
			break;
			
		case ctlSelectEvent:
      //SysDebug(1, "Addr", "EditHandleEvent ctlSelectEvent %d", event->data.ctlSelect.controlID);
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - ctlSelectEvent - controlID = %hu", event->data.ctlSelect.controlID));
			switch (event->data.ctlSelect.controlID)
			{
				case EditCategoryTrigger:
					PrvEditSelectCategory ();
					PrvEditRestoreEditState(FrmGetActiveForm());	// DOLATER: Can cause problems when no field is editable
					handled = true;
					break;
		
				case EditDoneButton:
					FrmGotoForm (ListView);
					handled = true;
					break;
		
				case EditDetailsButton:
					FrmPopupForm (DetailsDialog);
					handled = true;
					break;
		
				case EditNoteButton:
					if (NoteViewCreate())
					{
						RecordNeededAfterEditView = true;
						FrmGotoForm (NewNoteView);
					}
					handled = true;
					break;
				default:
					break;
			}
			break;
	
		case ctlRepeatEvent:
      //SysDebug(1, "Addr", "EditHandleEvent ctlRepeatEvent %d", event->data.ctlRepeat.controlID);
			switch (event->data.ctlRepeat.controlID)
			{
				case EditUpButton:
					PrvEditScroll (winUp);
					// leave unhandled so the buttons can repeat
					break;
		
				case EditDownButton:
					PrvEditScroll (winDown);
					// leave unhandled so the buttons can repeat
					break;
				default:
					break;
			}
			break;
	
		case menuEvent:
      //SysDebug(1, "Addr", "EditHandleEvent menuEvent");
			return PrvEditDoCommand (event->data.menu.itemID);
	
		case menuCmdBarOpenEvent:
		{
			FieldType* fldP;
			UInt16 startPos, endPos;

      //SysDebug(1, "Addr", "EditHandleEvent menuCmdBarOpenEvent");
			fldP = TblGetCurrentField(ToolsGetObjectPtr(EditTable));
			if (fldP)
				FldGetSelection(fldP, &startPos, &endPos);

			if ((fldP) && (startPos == endPos))  // there's no highlighted text
			{
				// Call directly the Field event handler so that edit buttons are added if applicable
				FldHandleEvent(fldP, event);
				
				MenuCmdBarAddButton(menuCmdBarOnRight, BarDeleteBitmap, menuCmdBarResultMenuItem, EditRecordDeleteRecordCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, EditRecordBeamRecordCmd, 0);

				// Prevent the field package to add edit buttons again
				event->data.menuCmdBarOpen.preventFieldButtons = true;
			}
			else if (fldP == NULL)	// there is no active text field (none have cursor visible)
			{
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarDeleteBitmap, menuCmdBarResultMenuItem, EditRecordDeleteRecordCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, EditRecordBeamRecordCmd, 0);

				// Prevent the field package to add edit buttons again
				event->data.menuCmdBarOpen.preventFieldButtons = true;
			}
			else
			{
			// When there is a selection range of text (ie startPos != endPos)
			// fall through to the field code to add the appropriate cut, copy, 
			// paste, and undo selections to the command bar.
				event->data.menuCmdBarOpen.preventFieldButtons = false;
			}
			
			// don't set handled to true; this event must fall through to the system.
			break;
		}
	
		case menuOpenEvent:
      //SysDebug(1, "Addr", "EditHandleEvent menuOpenEvent");
			if(!ToolsIsDialerPresent())
				MenuHideItem(EditRecordDialCmd);

			if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
				MenuHideItem(EditRecordSendRecordCmd);
			else
				MenuShowItem(EditRecordSendRecordCmd);
			// don't set handled = true
			break;
	
		case fldHeightChangedEvent:
      //SysDebug(1, "Addr", "EditHandleEvent fldHeightChangedEvent %d", event->data.fldChanged.fieldID);
			TraceOutput(TL(appErrorClass, "EditHandleEvent() - fldHeightChangedEvent"));
			PrvEditResizeDescription (event);
			handled = true;
			break;
	
		case keyDownEvent:
      //SysDebug(1, "Addr", "EditHandleEvent keyDownEvent %d", event->data.keyDown.chr);
			#if WRISTPDA
			if ( ( event->data.keyDown.chr == vchrThumbWheelPush ) ||
				 ( event->data.keyDown.chr == vchrThumbWheelBack ) ) {
				// Translate the Enter and Back keys to a Done button event.
				EventType newEvent;
				FormPtr frm = FrmGetActiveForm();
				MemSet (&newEvent, sizeof(EventType), 0);
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.on = true;
				newEvent.data.ctlSelect.controlID = EditDoneButton;
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
				TblReleaseFocus(ToolsGetObjectPtr(EditTable));
				TopVisibleRecord = 0;      // Same as when app switched to
				CurrentFieldIndex = noFieldIndex;
				FrmGotoForm (ListView);
				return (true);
			}
			else if (EvtKeydownIsVirtual(event))
			{
				switch (event->data.keyDown.chr)
				{
					case vchrPageUp:
						PrvEditScroll (winUp);
						handled = true;
						break;
		
					case vchrPageDown:
						PrvEditScroll (winDown);
						handled = true;
						break;
		
					case vchrNextField:
						PrvEditNextField (winDown);
						handled = true;
						break;
		
					case vchrPrevField:
						PrvEditNextField (winUp);
						handled = true;
						break;
		
					case vchrSendData:
						// Make sure the field being edited is saved
						frmP = FrmGetActiveForm ();
						tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
						TblReleaseFocus(tableP);
		
						MenuEraseStatus (0);
						TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
						handled = true;
						break;
		
					default:
						break;
				}
			}
			else
				handled = PrvEditAutoFill(event);
	
			break;
	
		case frmUpdateEvent:
      //SysDebug(1, "Addr", "EditHandleEvent frmUpdateEvent");
			PrvEditUpdateDisplay(event->data.frmUpdate.updateCode);
			handled = true;
			break;
	
		case frmSaveEvent:
			// Save the field being edited.  Do not delete the record if it's
			// empty because a frmSaveEvent can be sent without the form being
			// closed.  A canceled find does this.
	
      //SysDebug(1, "Addr", "EditHandleEvent frmSaveEvent");
			frmP = FrmGetFormPtr (EditView);
			tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
			TblReleaseFocus(tableP);
			break;
	
		default:
			break;
	}


  //SysDebug(1, "Addr", "EditHandleEvent event %d handled %d", event->eType, handled);
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    EditNewRecord
 *
 * DESCRIPTION: Makes a new record with some setup
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/13/95   Initial Revision
 *
 ***********************************************************************/
void EditNewRecord ()
{
	AddrDBRecordType newRecord;
	AddressFields i;
	UInt16 attr;
	Err err;


	// Set up the new record
	newRecord.options.phones.displayPhoneForList = 0;
	newRecord.options.phones.phone1 = workLabel;
	newRecord.options.phones.phone2 = homeLabel;
	newRecord.options.phones.phone3 = faxLabel;
	newRecord.options.phones.phone4 = otherLabel;
	newRecord.options.phones.phone5 = emailLabel;
	newRecord.options.phones.reserved = 0;

	for (i = firstAddressField; i < ad_addressFieldsCount; i++)
	{
		newRecord.fields[i] = NULL;
	}

	err = AddrDBNewRecord(AddrDB, &newRecord, &CurrentRecord);
  //SysDebug(1, "Addr", "EditNewRecord set CurrentRecord=%d", CurrentRecord);
	if (err)
	{
		FrmAlert(DeviceFullAlert);
		return;
	}


	// Set it's category to the category being viewed.
	// If the category is All then set the category to unfiled.
	DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
	attr &= ~dmRecAttrCategoryMask;
	attr |= ((CurrentCategory == dmAllCategories) ? dmUnfiledCategory :
			 CurrentCategory) | dmRecAttrDirty;
	DmSetRecordInfo (AddrDB, CurrentRecord, &attr, NULL);


	// Set the global variable that determines which field is the top visible
	// field in the edit view.  Also done when New is pressed.
	TopVisibleFieldIndex = 0;
	CurrentFieldIndex = editFirstFieldIndex;
	EditRowIDWhichHadFocus = editFirstFieldIndex;
	EditFieldPosition = 0;

	FrmGotoForm (EditView);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvEditInit
 *
 * DESCRIPTION: This routine initializes the "Edit View" of the
 *              Address application.
 *
 * PARAMETERS:	frmP					Pointer to the Edit form structure
 *					leaveDataLocked	T=>keep app info, form map data locked.
 *
 * RETURNED:	nothing
 *
 *	HISTORY:
 *		06/05/99	art	Created by Art Lamb.
 *		07/29/99	kwk	Set up locked FieldMap pointer.
 *		09/21/00	aro	GetObjectPtr => GetFrmObjectPtr
 *
 ***********************************************************************/
void PrvEditInit( FormPtr frmP, Boolean leaveDataLocked )
{
	UInt16 attr;
	UInt16 row;
	UInt16 rowsInTable;
	UInt16 category;
	UInt16 dataColumnWidth;
	TablePtr table;
	AddrAppInfoPtr appInfoPtr;
	ListPtr popupPhoneList;
	FontID   currFont;
	RectangleType bounds;

  //SysDebug(1, "Addr", "PrvEditInit begin");

	#if WRISTPDA
	currFont = FntSetFont (FossilStdFont);
	#else
	currFont = FntSetFont (stdFont);
	#endif
	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
	FieldMapH = DmGetResource(fieldMapRscType, FieldMapID);	
	FieldMap = (const AddressFields*)MemHandleLock(FieldMapH);
	
	CurrentTableRow = editInvalidRow;

	// Set the choices to the phone list
	EditPhoneListChoices[0] = appInfoPtr->fieldLabels[firstPhoneField];
	EditPhoneListChoices[1] = appInfoPtr->fieldLabels[firstPhoneField + 1];
	EditPhoneListChoices[2] = appInfoPtr->fieldLabels[firstPhoneField + 2];
	EditPhoneListChoices[3] = appInfoPtr->fieldLabels[firstPhoneField + 3];
	EditPhoneListChoices[4] = appInfoPtr->fieldLabels[firstPhoneField + 4];
	EditPhoneListChoices[5] = appInfoPtr->fieldLabels[ad_addressFieldsCount];
	EditPhoneListChoices[6] = appInfoPtr->fieldLabels[ad_addressFieldsCount + 1];
	EditPhoneListChoices[7] = appInfoPtr->fieldLabels[ad_addressFieldsCount + 2];
	popupPhoneList = ToolsGetFrmObjectPtr(frmP, EditPhoneList);
	LstSetListChoices(popupPhoneList, EditPhoneListChoices, numPhoneLabels);
	LstSetHeight (popupPhoneList, numPhoneLabels);



	// Initialize the address list table.
	table = ToolsGetFrmObjectPtr(frmP, EditTable);
	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
	{
		// This sets the data column
		TblSetItemStyle (table, row, editDataColumn, textTableItem);
		TblSetRowUsable (table, row, false);
	}

	TblSetColumnUsable (table, editLabelColumn, true);
	TblSetColumnUsable (table, editDataColumn, true);

	TblSetColumnSpacing (table, editLabelColumn, spaceBeforeDesc);


	// Set the callback routines that will load and save the
	// description field.
	TblSetLoadDataProcedure (table, editDataColumn, PrvEditGetRecordField);
	TblSetSaveDataProcedure (table, editDataColumn, PrvEditSaveRecordField);


	// Set the column widths so that the label column contents fit exactly.
	// Those labels change as the country changes.
	if (EditLabelColumnWidth == 0)
		#if WRISTPDA
		EditLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, FossilStdFont);
		#else
		EditLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, stdFont);
		#endif

	// Compute the width of the data column, account for the table column gutter.
	TblGetBounds (table, &bounds);
	dataColumnWidth = bounds.extent.x - spaceBeforeDesc - EditLabelColumnWidth;

	TblSetColumnWidth(table, editLabelColumn, EditLabelColumnWidth);
	TblSetColumnWidth(table, editDataColumn, dataColumnWidth);


  //SysDebug(1, "Addr", "PrvEditInit PrvEditLoadTable");
	PrvEditLoadTable(frmP);


	// Set the label of the category trigger.
	if (CurrentCategory == dmAllCategories)
	{
		DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
		category = attr & dmRecAttrCategoryMask;
	}
	else
		category = CurrentCategory;
	CategoryGetName (AddrDB, category, CategoryName);
	CategorySetTriggerLabel(ToolsGetFrmObjectPtr(frmP, EditCategoryTrigger), CategoryName);


	FntSetFont (currFont);

	// if the caller is using us to reset the form, then we don't want
	// to repeatedly lock down the app info block.
	if (!leaveDataLocked)
	{
		MemPtrUnlock(appInfoPtr);
		MemHandleUnlock(FieldMapH);
		DmReleaseResource(FieldMapH);
	}

	// In general, the record isn't needed after this form is closed.
	// It is if the user is going to the Note View.  In that case
	// we must keep the record.
	RecordNeededAfterEditView = false;

  //SysDebug(1, "Addr", "PrvEditInit end");
}


#if		WRISTPDA
/***********************************************************************
 *
 * FUNCTION:    PrvDrawLabel
 *
 * DESCRIPTION: Draws the labels to the left of the table
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         MT   6/26/95      Initial Revision
 *
 ***********************************************************************/
TableDrawItemFuncType	PrvDrawLabel;
void					PrvDrawLabel( 
	void*				tableP,
	Int16				row,
	Int16				column,
	RectangleType*	boundsP )
{
	FontID			savedFont;
	Char*				textP;
	UInt16			textS;
	Int16				width;
	AddrAppInfoPtr appInfoPtr;

	savedFont = FntSetFont( addrEditLabelFont );

	// Write text
	appInfoPtr = (AddrAppInfoPtr)AddrDBAppInfoGetPtr(AddrDB);
	textP = appInfoPtr->fieldLabels[FieldMap[row+TopVisibleFieldIndex]];

	textS = StrLen( textP );
	WinDrawTruncChars( textP, textS, boundsP->topLeft.x,
												boundsP->topLeft.y,
												boundsP->extent.x - FntCharsWidth(": ", 2));
	// Add Write ":"
	width = FntCharsWidth( textP, textS );
	WinDrawChars( ": ", 2, min(boundsP->topLeft.x + width, boundsP->extent.x - FntCharsWidth(": ", 2)), boundsP->topLeft.y );
	
	FntSetFont( savedFont );
	MemPtrUnlock(appInfoPtr);
}	//		PrvDrawLabel
#endif//	WRISTPDA


/***********************************************************************
 *
 * FUNCTION:    PrvEditInitTableRow
 *
 * DESCRIPTION: This routine initialize a row in the edit view.
 *
 * PARAMETERS:  table       - pointer to the table of to do items
 *              row         - row number (first row is zero)
 *              fieldIndex  - the index of the field displayed in the row
 *              rowHeight   - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/26/95      Initial Revision
 *
 ***********************************************************************/
static void PrvEditInitTableRow( FormType* frmP, TablePtr table, UInt16 row, UInt16 fieldIndex, Int16 rowHeight, FontID fontID, AddrDBRecordPtr record, AddrAppInfoPtr appInfoPtr )
{

	// Make the row usable.
	TblSetRowUsable (table, row, true);

	// Set the height of the row to the height of the desc
	TblSetRowHeight (table, row, rowHeight);

	// Store the record number as the row id.
	TblSetRowID (table, row, fieldIndex);

	// Mark the row invalid so that it will draw when we call the
	// draw routine.
	TblMarkRowInvalid (table, row);

	// Set the text font.
	TblSetItemFont (table, row, editDataColumn, fontID);

	// The label is either a text label or a popup menu (of phones)
	if (! isPhoneField(FieldMap[fieldIndex]))
	{
		#if		WRISTPDA
		TblSetItemStyle (table, row, editLabelColumn, tallCustomTableItem);
		TblSetCustomDrawProcedure( table, editLabelColumn, &PrvDrawLabel );
		#else
		TblSetItemStyle (table, row, editLabelColumn, labelTableItem);
		TblSetItemPtr (table, row, editLabelColumn,
					   appInfoPtr->fieldLabels[FieldMap[fieldIndex]]);
		#endif//	WRISTPDA
	}
	else
	{
		// The label is a popup list
		TblSetItemStyle (table, row, editLabelColumn, popupTriggerTableItem);
		TblSetItemInt (table, row, editLabelColumn, GetPhoneLabel(record, FieldMap[fieldIndex]));
		TblSetItemPtr (table, row, editLabelColumn, ToolsGetFrmObjectPtr(frmP, EditPhoneList));
	}

	#if WRISTPDA
	TblSetItemFont (table, row, editLabelColumn, addrEditLabelFont);
	#endif

}


/***********************************************************************
 *
 * FUNCTION:    PrvEditHandleSelectField
 *
 * DESCRIPTION: Handle the user tapping an edit view field label.
 *   Either the a phone label is changed or the user wants to edit
 * a field by tapping on it's label.
 *
 * PARAMETERS:  row    - row of the item to select (zero based)
 *              column - column of the item to select (zero based)
 *
 * RETURNED:    true if the event was handled and nothing else should
 *              be done
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	11/27/95	Cut from EditViewHandleEvent
 *			art		09/02/97	Add multi-font support
 *			roger	11/04/97	Changed parameters to support another routine
 *			jmp		04/18/00	Fixed bug #23237:  When changing an editLabelColumn,
 *								mark the row invalid so that we redraw everything.
 *								If we don't do this, the edit indicator's colors
 *								don't come out correctly.
 *			fpa		10/23/00	Fixed bug #42762 - Can't select text when cursor
 *								is in a blank field
 *			gap		10/25/00	fix above did not take into account that there are
 *								occasions where fldP is NULL.
 *			fpa		11/06/00	Fixed bug #23088 - Cannot insert a cursor between 2
 *								characters. Undid gap above modification because it
 *								was reopening bug #42762
 *
 ***********************************************************************/
Boolean PrvEditHandleSelectField (Int16 row, Int16 column)
{
	Err					err;
	Int16				currRow;
	UInt16				fieldNum;
	UInt16				fieldIndex;
	MemHandle			currentRecordH;
	UInt16				i;
	UInt16				currentField;
	FontID				currFont;
	Boolean				redraw = false;
	FormType*			frmP;
	TablePtr			tableP;
	FieldPtr			fldP;
	AddrDBRecordType	currentRecord;
	AddrDBRecordFlags	changedFields;
	UInt16				startPosition;
	UInt16				stopPosition;

	frmP = FrmGetActiveForm();
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);
	fldP = NULL;

	TraceOutput(TL(appErrorClass, "PrvEditHandleSelectField"));
	// If a phone label was changed then modify the record
	currentField = FieldMap[TblGetRowID(tableP, row)];
	if (column == editLabelColumn)
	{
		if (isPhoneField(currentField))
		{
			i = TblGetItemInt(tableP, row, editLabelColumn);
			AddrDBGetRecord(AddrDB, CurrentRecord, &currentRecord, &currentRecordH);

			switch (currentField)
			{
				case firstPhoneField:
					currentRecord.options.phones.phone1 = i;
					break;
	
				case firstPhoneField + 1:
					currentRecord.options.phones.phone2 = i;
					break;
	
				case firstPhoneField + 2:
					currentRecord.options.phones.phone3 = i;
					break;
	
				case firstPhoneField + 3:
					currentRecord.options.phones.phone4 = i;
					break;
	
				case firstPhoneField + 4:
					currentRecord.options.phones.phone5 = i;
					break;
			}

			changedFields.allBits = 0;
			err = AddrDBChangeRecord(AddrDB, &CurrentRecord, &currentRecord, changedFields);
      //SysDebug(1, "Addr", "PrvEditHandleSelectField set CurrentRecord=%d", CurrentRecord);
			if ( err != errNone )
			{
				MemHandleUnlock(currentRecordH);
				FrmAlert(DeviceFullAlert);

				// Redraw the table without the change.  The phone label
				// is unchanged in the record but the screen and the table row
				// are changed.  Reinit the table to fix it.  Mark the row
				// invalid and redraw it.
				PrvEditInit(frmP, false);
				TblMarkRowInvalid(tableP, row);
				TblRedrawTable(tableP);

				return true;
			}
		}

		// The user selected the label of a field.  So, set the table to edit the field to
		// the right of the label.  Also, mark the row invalid and say that we want to redraw
		// it so all the colors and such come out right.
		TblReleaseFocus(tableP);
		TblUnhighlightSelection(tableP);
		TblMarkRowInvalid (tableP, row);
		redraw = true;
	}

	// Make sure the the heights the the field we are exiting and the
	// that we are entering are correct.  They may be incorrect if the
	// font used to display blank line is a different height then the
	// font used to display field text.
	fieldIndex = TblGetRowID (tableP, row);

	if (fieldIndex != CurrentFieldIndex || TblGetCurrentField(tableP) == NULL)
	{
		AddrDBGetRecord (AddrDB, CurrentRecord, &currentRecord, &currentRecordH);

		currFont = FntGetFont ();

		// Is there a current field and is it empty?
		if (CurrentFieldIndex != noFieldIndex &&
			!currentRecord.fields[FieldMap[CurrentFieldIndex]])
		{
			if (TblFindRowID (tableP, CurrentFieldIndex, &currRow))
			{
				// Is the height of the field correct?
				FntSetFont (addrEditBlankFont);
				if (FntLineHeight () != TblGetRowHeight (tableP, currRow))
				{
					TblMarkRowInvalid (tableP, currRow);
					redraw = true;
				}
			}
		}

		CurrentFieldIndex = fieldIndex;

		// Is the newly selected field empty?
		fieldNum = FieldMap[fieldIndex];
		if (!currentRecord.fields[fieldNum])
		{
			// Is the height of the field correct?
			FntSetFont (AddrEditFont);
			if (FntLineHeight () != TblGetRowHeight (tableP, row))
			{
				TblMarkRowInvalid (tableP, row);
				redraw = true;
			}
		}

		// Do before the table focus is released and the record is saved.
		MemHandleUnlock (currentRecordH);

		if (redraw)
		{
			fldP = TblGetCurrentField(tableP);
			if ( fldP != NULL )
				FldGetSelection(fldP, &startPosition, &stopPosition);	// Save the selection of the field

			TblReleaseFocus (tableP);
			PrvEditLoadTable(frmP);
			TblFindRowID (tableP, fieldIndex, &row);

			TblRedrawTable (tableP);
		}

		FntSetFont (currFont);
	}

	// Set the focus on the field if necessary
	if ( TblGetCurrentField(tableP) == NULL )
	{
		FieldPtr fldTempP;
		
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditTable));
		TblGrabFocus (tableP, row, editDataColumn);
		fldTempP = TblGetCurrentField(tableP);
		FldGrabFocus(fldTempP);
		FldMakeFullyVisible (fldTempP);
	}

	// Restore the selection of the field or restore the insertion point
	if ( redraw && (fldP != NULL) )
	{
		if ( startPosition != stopPosition )
			FldSetSelection(fldP, startPosition, stopPosition);
		else
			FldSetInsPtPosition(fldP, EditFieldPosition);
	}
		
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditRestoreEditState
 *
 * DESCRIPTION: This routine restores the edit state of the Edit
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/12/97	Initial Revision
 *
 ***********************************************************************/
void PrvEditRestoreEditState( FormType* frmP )
{
	Int16			row;
	TablePtr		table;
	FieldPtr		fld;

	if (CurrentFieldIndex == noFieldIndex) return;

	// Find the row that the current field is in.
	table = ToolsGetFrmObjectPtr(frmP, EditTable);
	if ( ! TblFindRowID (table, CurrentFieldIndex, &row) )
		return;

	FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditTable));
	TblGrabFocus (table, row, editDataColumn);

	// Restore the insertion point position.
	fld = TblGetCurrentField (table);
	FldSetInsPtPosition (fld, EditFieldPosition);
	FldGrabFocus (fld);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditSetGraffitiMode
 *
 * DESCRIPTION: Set the graffiti mode based on the field being edited.
 *
 * PARAMETERS:  currentField - the field being edited.
 *
 * RETURNED:    the graffiti mode is set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   9/20/95   Initial Revision
 *
 ***********************************************************************/
void PrvEditSetGraffitiMode (FieldPtr fld, UInt16 currentField)
{
	MemHandle currentRecordH;
	Boolean autoShift;
	FieldAttrType attr;
	AddrDBRecordType currentRecord;


	AddrDBGetRecord(AddrDB, CurrentRecord, &currentRecord, &currentRecordH);

	if (! isPhoneField(currentField))
	{
		// Set the field to support auto-shift.
		autoShift = true;
	}
	else
	{
		GrfSetState(false, true, false);
		autoShift = false;
	}

	if (fld)
	{
		FldGetAttributes (fld, &attr);
		attr.autoShift = autoShift;
		FldSetAttributes (fld, &attr);
	}


	MemHandleUnlock(currentRecordH);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditGetRecordField
 *
 * DESCRIPTION: This routine returns a pointer to a field of the
 *              address record.  This routine is called by the table
 *              object as a callback routine when it wants to display or
 *              edit a field.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/6/95      Initial Revision
 *
 ***********************************************************************/
Err PrvEditGetRecordField (void * table, Int16 row, Int16 UNUSED_PARAM(column), Boolean editing, MemHandle * textH, Int16 * textOffset, Int16 * textAllocSize, FieldPtr fld)
{
	UInt16 fieldNum;
	UInt16  fieldIndex;
	Char * recordP;
	Char * fieldP;
	MemHandle recordH, fieldH;
	UInt16 fieldSize;
	AddrDBRecordType record;

  //SysDebug(1, "Addr", "PrvEditGetRecordField row %d CurrentRecord %d", row, CurrentRecord);

	// Get the field number that corresponds to the table item.
	// The field number is stored as the row id.
	//
	fieldIndex = TblGetRowID (table, row);
	fieldNum = FieldMap[fieldIndex];

	AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);

	if (editing)
	{
		PrvEditSetGraffitiMode(fld, fieldNum);
		if (record.fields[fieldNum])
		{
			fieldSize = StrLen(record.fields[fieldNum]) + 1;
      //SysDebug(1, "Addr", "PrvEditGetRecordField editing MemHandleNew %d", fieldSize);
			fieldH = MemHandleNew(fieldSize);	// Handle freeing done into PrvEditSaveRecordField() by calling FldFreeMemory() function
			fieldP = MemHandleLock(fieldH);
			MemMove(fieldP, record.fields[fieldNum], fieldSize);
			*textAllocSize = fieldSize;
			MemHandleUnlock(fieldH);
		}
		else
		{
			fieldH = 0;
			*textAllocSize = 0;
		}
		MemHandleUnlock (recordH);
		*textOffset = 0;         // only one string
		*textH = fieldH;
		return (0);

	}
	else
	{
		// Calculate the offset from the start of the record.
		recordP = MemHandleLock (recordH);   // record now locked twice

		if (record.fields[fieldNum])
		{
			*textOffset = record.fields[fieldNum] - recordP;
			*textAllocSize = StrLen (record.fields[fieldNum]) + 1;  // one for null terminator
		}
		else
		{
			do
			{
				fieldNum++;
			} while (fieldNum < ad_addressFieldsCount &&
					 record.fields[fieldNum] == NULL);

			if (fieldNum < ad_addressFieldsCount)
				*textOffset = record.fields[fieldNum] - recordP;
			else
				// Place the new field at the end of the text.
				*textOffset = MemHandleSize(recordH);

			*textAllocSize = 0;  // one for null terminator
		}
		MemHandleUnlock (recordH);   // unlock the second lock
	}

	MemHandleUnlock (recordH);      // unlock the AddrGetRecord lock

	*textH = recordH;
	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditSaveRecordField
 *
 * DESCRIPTION: This routine saves a field of an address to the
 *              database.  This routine is called by the table
 *              object, as a callback routine, when it wants to save
 *              an item.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   2/21/95   Initial Revision
 *
 ***********************************************************************/
Boolean PrvEditSaveRecordField (void * table, Int16 row, Int16 UNUSED_PARAM(column))
{
	UInt16 fieldNum;
	UInt16 fieldIndex;
	FieldPtr fld;
	AddrDBRecordType record;
	MemHandle recordH;
	MemHandle textH;
	Char * textP;
	AddrDBRecordFlags bit;
	UInt16 i;
	Err err;
	Boolean redraw = false;
	UInt16 numOfRows;
	Int16 newSize;

	fld = TblGetCurrentField (table);
	textH = FldGetTextHandle(fld);

	// Get the field number that corresponds to the table item to save.
	fieldIndex = TblGetRowID (table, row);
	fieldNum = FieldMap[fieldIndex];
  //SysDebug(1, "Addr", "PrvEditSaveRecordField row=%d col=%d index=%d num=%d fld=%d CurrentRecord=%d", row, column, fieldIndex, fieldNum, fld ? 1 : 0, CurrentRecord);

	// Save the field last edited.
	EditRowIDWhichHadFocus = fieldIndex;

	// Save the cursor position of the field last edited.
	// Check if the top of the text is scroll off the top of the
	// field, if it is then redraw the field.
	if (FldGetScrollPosition (fld))
	{
		FldSetScrollPosition (fld, 0);
		EditFieldPosition = 0;
	}
	else
		EditFieldPosition = FldGetInsPtPosition (fld);

	// Make sure there any selection is removed since we will free
	// the text memory before the callee can remove the selection.
	FldSetSelection (fld, 0, 0);


  //SysDebug(1, "Addr", "PrvEditSaveRecordField dirty=%d", FldDirty (fld));
	if (FldDirty (fld))
	{
		// Since the field is dirty, mark the record as dirty
		ToolsDirtyRecord (CurrentRecord);

		// Get a pointer to the text of the field.
		if (textH == 0)
			textP = NULL;
		else
		{
			textP = MemHandleLock(textH);
			if (textP[0] == '\0')
				textP = NULL;
		}

    //SysDebug(1, "Addr", "PrvEditSaveRecordField text=%d", textP ? 1 : 0);
		// If we have text, and saving an auto-fill field, save the data to the proper database
		if (textP) {
			UInt32	dbType;

			// Select the proper database for the field we are editing,
			// or skip if not an autofill enabled field
			switch (fieldNum) {
			case ad_title:		dbType = titleDBType; break;
			case ad_company:	dbType = companyDBType; break;
			case ad_city:		dbType = cityDBType; break;
			case ad_state:		dbType = stateDBType; break;
			case ad_country:	dbType = countryDBType; break;
			default:			dbType = 0;
			}

      //SysDebug(1, "Addr", "PrvEditSaveRecordField dbType=%ld", dbType);
			if (dbType) AutoFillLookupSave(dbType, AddressBookCreator, textP);
		}

		AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);
		record.fields[fieldNum] = textP;

		// If we have changed a phone field and if the show if
		// list view phone is blank set it to the first non blank phone
		// This rule should allow:
		// 1. Showing a blank field is possible
		// 2. Deleting the shown field switches to another
		// 3. Adding a field when there isn't one shows it.
    //SysDebug(1, "Addr", "PrvEditSaveRecordField isPhone=%d", isPhoneField(fieldNum));
		if (isPhoneField(fieldNum) &&
			record.fields[firstPhoneField + record.options.phones.displayPhoneForList] == NULL)
		{
			for (i = firstPhoneField; i <= lastPhoneField; i++)
			{
				if (record.fields[i] != NULL)
				{
					record.options.phones.displayPhoneForList = i - firstPhoneField;
					break;
				}
			}
		}


		//bit.allBits = (UInt32)1 << fieldNum;
		bit.allBits = BitAtPosition(fieldNum);
    //SysDebug(1, "Addr", "PrvEditSaveRecordField allBits=0x%08lX", bit.allBits);
		err = AddrDBChangeRecord(AddrDB, &CurrentRecord, &record, bit);
    //SysDebug(1, "Addr", "PrvEditSaveRecordField set CurrentRecord=%d", CurrentRecord);

		// The new field has been copied into the new record.  Unlock it.
		if (textP)
			MemPtrUnlock(textP);

		// The change was not made (probably storage out of memory)
		if (err)
		{
			// Because the storage is full the text in the text field differs
			// from the text in the record.  PrvEditGetFieldHeight uses
			// the text in the field (because it's being edited).
			// Make the text in the field the same as the text in the record.
			// Resizing should always be possible.
			MemHandleUnlock(recordH);      // Get original text
			AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);

			if (record.fields[fieldNum] == NULL)
				newSize = 1;
			else
				newSize = StrLen(record.fields[fieldNum]) + 1;

			// Have the field stop using the chunk to unlock it.  Otherwise the resize can't
			// move the chunk if more space is needed and no adjacent free space exists.
			FldSetTextHandle (fld, 0);
			if (!MemHandleResize(textH, newSize))
			{
				textP = MemHandleLock(textH);
				if (newSize > 1)
					StrCopy(textP, record.fields[fieldNum]);
				else
					textP[0] = '\0';
				MemPtrUnlock(textP);
			}
			else
			{
				ErrNonFatalDisplay("Resize failed.");
			}


			// Update the text field to use whatever text we have.
			FldSetTextHandle (fld, textH);

			MemHandleUnlock(recordH);
			FrmAlert(DeviceFullAlert);

			// The field may no longer be the same height.  This row and those
			// below may need to be recalced. Mark this row and those
			// below it not usable and reload the table.
			numOfRows = TblGetNumberOfRows(table);
			while (row < numOfRows)
			{
				TblSetRowUsable(table, row, false);
				row++;
			}
			PrvEditLoadTable(FrmGetActiveForm());
			redraw = true;                  // redraw the table showing change lost
		}

	}

	// Free the memory used for the field's text because the table suppresses it.
	FldFreeMemory (fld);

  //SysDebug(1, "Addr", "PrvEditSaveRecordField redraw=0x%08X", redraw);
	return redraw;
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditSaveRecord
 *
 * DESCRIPTION: Checks the record and saves it if it's OK
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    The view that should be switched to.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         rsf   9/20/95   Initial Revision
 *
 ***********************************************************************/
UInt16 PrvEditSaveRecord ()
{
	MemHandle currentRecordH;
	AddrDBRecordType currentRecord;
	FormPtr frmP;
	TablePtr tableP;
	Boolean hasData;


	// Make sure the field being edited is saved
	frmP = FrmGetFormPtr (EditView);
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	TblReleaseFocus(tableP);


	// If this record is needed then leave.  This is a good time because
	// the data is saved and this is before the record could be deleted.
	if (RecordNeededAfterEditView)
	{
		ListViewSelectThisRecord = noRecord;
		return ListView;
	}


	// The record may have already been delete by the Delete menu command
	// or the details dialog.  If there isn't a CurrentRecord assume the
	// record has been deleted.
	if (CurrentRecord == noRecord)
	{
		ListViewSelectThisRecord = noRecord;
		return ListView;
	}

	// If there is no data then then delete the record.
	// If there is data but no name data then demand some.

	AddrDBGetRecord(AddrDB, CurrentRecord, &currentRecord, &currentRecordH);

	hasData = AddrDBRecordContainsData(&currentRecord);

	// Unlock before the DeleteRecord.   We can only rely on
	// NULL pointers from here on out.
	MemHandleUnlock(currentRecordH);


	// If none are the fields contained anything then
	// delete the field.
	if (!hasData)
	{
		ToolsDeleteRecord(false);   // uniq ID wasted?  Yes. We don't care.
		return ListView;
	}


	// The record's category may have been changed.  The CurrentCategory
	// isn't supposed to change in this case.  Make sure the CurrentRecord
	// is still visible in this category or pick another one near it.
	if (!ToolsSeekRecord(&CurrentRecord, 0, dmSeekBackward))
		if (!ToolsSeekRecord(&CurrentRecord, 0, dmSeekForward))
			CurrentRecord = noRecord;
  //SysDebug(1, "Addr", "PrvEditSaveRecord set CurrentRecord=%d", CurrentRecord);


	ListViewSelectThisRecord = CurrentRecord;

	return ListView;
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories from the "Edit View".
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
void PrvEditSelectCategory (void)
{
	UInt16 attr;
	FormType* frmP;
	UInt16 category;
	Boolean categoryEdited;


	// Process the category popup list.
	DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;

	frmP = FrmGetActiveForm();
	categoryEdited = CategorySelect (AddrDB, frmP, EditCategoryTrigger,
									 EditCategoryList, false, &category, CategoryName, 1, categoryDefaultEditCategoryString);

	if (categoryEdited || (category != (attr & dmRecAttrCategoryMask)))
	{
		// Change the category of the record.
		DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
		attr &= ~dmRecAttrCategoryMask;
		attr |= category | dmRecAttrDirty;
		DmSetRecordInfo (AddrDB, CurrentRecord, &attr, NULL);

		ToolsChangeCategory (category);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the edit view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frmP             -  pointer to the address edit form
 *              bottomField     -  field index of the last visible row
 *              lastItemClipped - true if the last visible row is clip at
 *                                 the bottom
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/26/95   Initial Revision
 *
 ***********************************************************************/
void PrvEditUpdateScrollers (FormPtr frmP, UInt16 bottomFieldIndex,
							 Boolean lastItemClipped)
{
	UInt16 upIndex;
	UInt16 downIndex;
	Boolean scrollableUp;
	Boolean scrollableDown;

	// If the first field displayed is not the fist field in the record,
	// enable the up scroller.
	scrollableUp = TopVisibleFieldIndex > 0;

	// If the last field displayed is not the last field in the record,
	// enable the down scroller.
	scrollableDown = (lastItemClipped || (bottomFieldIndex < editLastFieldIndex));


	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frmP, EditUpButton);
	downIndex = FrmGetObjectIndex (frmP, EditDownButton);
	FrmUpdateScrollers (frmP, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditGetFieldHeight
 *
 * DESCRIPTION: This routine initialize a row in the to do list.
 *
 * PARAMETERS:  table        - pointer to the table of to do items
 *              fieldIndex   - the index of the field displayed in the row
 *              columnWidth  - height of the row in pixels
 *
 * RETURNED:    height of the field in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/26/95	Initial Revision
 *			art	9/11/97	Add font support.
 *
 ***********************************************************************/
UInt16 PrvEditGetFieldHeight (TablePtr table, UInt16 fieldIndex, Int16 columnWidth, Int16 maxHeight, AddrDBRecordPtr record, FontID * fontIdP)
{
	Int16 row;
	Int16 column;
	UInt16 index;
	Int16 height;
	UInt16 lineHeight;
	FontID currFont;
	Char * str;
	FieldPtr fld;

	if (TblEditing (table))
	{
		TblGetSelection (table, &row, &column);
		if (fieldIndex == TblGetRowID (table, row))
		{
			fld = TblGetCurrentField (table);
			str = FldGetTextPtr (fld);
		}
		else
		{
			index = FieldMap[fieldIndex];
			str = record->fields[index];
		}
	}
	else
	{
		index = FieldMap[fieldIndex];
		str = record->fields[index];
	}


	// If the field has text empty, or the field is the current field, or
	// the font used to display blank lines is the same as the font used
	// to display text then used the view's current font setting.
	if ( (str && *str) ||
		 (CurrentFieldIndex == fieldIndex) ||
		 (AddrEditFont == addrEditBlankFont))
	{
		*fontIdP = AddrEditFont;
		currFont = FntSetFont (*fontIdP);
	}

	// If the height of the font used to display blank lines is the same
	// height as the font used to display text then used the view's
	// current font setting.
	else
	{
		currFont = FntSetFont (addrEditBlankFont);
		lineHeight = FntLineHeight ();

		FntSetFont (AddrEditFont);
		if (lineHeight == FntLineHeight ())
			*fontIdP = AddrEditFont;
		else
		{
			*fontIdP = addrEditBlankFont;
			FntSetFont (addrEditBlankFont);
		}
	}

	height = FldCalcFieldHeight (str, columnWidth);
	lineHeight = FntLineHeight ();
	height = min (height, (maxHeight / lineHeight));
	height *= lineHeight;

	FntSetFont (currFont);


	return (height);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditDrawBusinessCardIndicator
 *
 * DESCRIPTION: Draw the business card indicator if the current record is
 * the business card.
 *
 * PARAMETERS:  formP - the form containing the business card indicator
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  12/3/97  Initial Revision
 *
 ***********************************************************************/
void PrvEditDrawBusinessCardIndicator (FormPtr formP)
{
	UInt32 uniqueID;

	DmRecordInfo (AddrDB, CurrentRecord, NULL, &uniqueID, NULL);
	if (BusinessCardRecordID == uniqueID)
		FrmShowObject(formP, FrmGetObjectIndex (formP, EditViewBusinessCardBmp));
	else
		FrmHideObject(formP, FrmGetObjectIndex (formP, EditViewBusinessCardBmp));

}


/***********************************************************************
 *
 * FUNCTION:    PrvEditResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of address
 *              field is changed as a result of user input.
 *              If the new height of the field is shorter, more items
 *              may need to be added to the bottom of the list.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/26/95      Initial Revision
 *
 ***********************************************************************/
void PrvEditResizeDescription (EventType * event)
{
	UInt16 pos;
	Int16 row;
	Int16 column;
	Int16 lastRow;
	UInt16 fieldIndex;
	UInt16 lastFieldIndex;
	UInt16 topFieldIndex;
	FieldPtr fld;
	TablePtr table;
	Boolean restoreFocus = false;
	Boolean lastItemClipped;
	RectangleType itemR;
	RectangleType tableR;
	RectangleType fieldR;
	FormType* frmP;


	frmP = FrmGetActiveForm();
	// Get the current height of the field;
	fld = event->data.fldHeightChanged.pField;
	FldGetBounds (fld, &fieldR);

	// Have the table object resize the field and move the items below
	// the field up or down.
	table = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblHandleEvent (table, event);

	// If the field's height has expanded , we're done.
	if (event->data.fldHeightChanged.newHeight >= fieldR.extent.y)
	{
		topFieldIndex = TblGetRowID (table, 0);
		if (topFieldIndex != TopVisibleFieldIndex)
			TopVisibleFieldIndex = topFieldIndex;
		else
		{
			// Since the table has expanded we may be able to scroll
			// when before we might not have.
			lastRow = TblGetLastUsableRow (table);
			TblGetBounds (table, &tableR);
			TblGetItemBounds (table, lastRow, editDataColumn, &itemR);
			lastItemClipped = (itemR.topLeft.y + itemR.extent.y >
							   tableR.topLeft.y + tableR.extent.y);
			lastFieldIndex = TblGetRowID (table, lastRow);

			PrvEditUpdateScrollers(frmP, lastFieldIndex,
								   lastItemClipped);

			return;
		}
	}

	// If the field's height has contracted and the field edit field
	// is not visible then the table may be scrolled.  Release the
	// focus,  which will force the saving of the field we are editing.
	else if (TblGetRowID (table, 0) != editFirstFieldIndex)
	{
		TblGetSelection (table, &row, &column);
		fieldIndex = TblGetRowID (table, row);

		fld = TblGetCurrentField (table);
		pos = FldGetInsPtPosition (fld);
		TblReleaseFocus (table);

		restoreFocus = true;
	}

	// Add items to the table to fill in the space made available by the
	// shorting the field.
	PrvEditLoadTable(frmP);
	TblRedrawTable (table);

	// Restore the insertion point position.
	if (restoreFocus)
	{
		TblFindRowID (table, fieldIndex, &row);
		TblGrabFocus (table, row, column);
		FldSetInsPtPosition (fld, pos);
		FldGrabFocus (fld);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditScroll
 *
 * DESCRIPTION: This routine scrolls the list of editable fields
 *              in the direction specified.
 *
 * PARAMETERS:  direction - up or dowm
 *              oneLine   - if true the list is scroll by a single line,
 *                          if false the list is scroll by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   06/26/95	Initial Revision
 *         vmk   02/20/98	Move TblUnhighlightSelection before PrvEditLoadTable
 *         gap   10/12/99	Close command bar before processing scroll
 *
 ***********************************************************************/
void PrvEditScroll (WinDirectionType direction)
{
	UInt16				row;
	UInt16				height;
	UInt16				fieldIndex;
	UInt16				columnWidth;
	UInt16				tableHeight;
	TablePtr				table;
	FontID				curFont;
	RectangleType		r;
	AddrDBRecordType	record;
	MemHandle			recordH;
	FormType*			frmP;


	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus (0);
	frmP = FrmGetActiveForm();

	#if WRISTPDA
	curFont = FntSetFont (FossilStdFont);
	#else
	curFont = FntSetFont (stdFont);
	#endif

	table = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblReleaseFocus (table);

	// Get the height of the table and the width of the description
	// column.
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (table, editDataColumn);

	// Scroll the table down.
	if (direction == winDown)
	{
		// Get the index of the last visible field, this will become
		// the index of the top visible field, unless it occupies the
		// whole screeen, in which case the next field will be the
		// top filed.

		row = TblGetLastUsableRow (table);
		fieldIndex = TblGetRowID (table, row);

		// If the last visible field is also the first visible field
		// then it occupies the whole screeen.
		if (row == 0)
			fieldIndex = min (editLastFieldIndex, fieldIndex+1);
	}

	// Scroll the table up.
	else
	{
		// Scan the fields before the first visible field to determine
		// how many fields we need to scroll.  Since the heights of the
		// fields vary,  we sum the height of the records until we get
		// a screen full.

		fieldIndex = TblGetRowID (table, 0);
		ErrFatalDisplayIf(fieldIndex > editLastFieldIndex, "Invalid field Index");
		if (fieldIndex == 0)
			goto exit;

		// Get the current record
		AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);

		height = TblGetRowHeight (table, 0);
		if (height >= tableHeight)
			height = 0;

		while (height < tableHeight && fieldIndex > 0)
		{
			height += FldCalcFieldHeight (record.fields[FieldMap[fieldIndex-1]],
										  columnWidth) * FntLineHeight ();
			if ((height <= tableHeight) || (fieldIndex == TblGetRowID (table, 0)))
				fieldIndex--;
		}
		MemHandleUnlock(recordH);
	}

	TblMarkTableInvalid (table);
	CurrentFieldIndex = noFieldIndex;
	TopVisibleFieldIndex = fieldIndex;
	EditRowIDWhichHadFocus = editFirstFieldIndex;
	EditFieldPosition = 0;

	TblUnhighlightSelection (table);		// remove the highlight before reloading the table to avoid
	// having an out of bounds selection information in case
	// the newly loaded data doesn't have as many rows as the old
	// data.  This fixes the bug
	// "File: Table.c, Line: 2599, currentRow violated constraint!"
	// (fix suggested by Art) vmk 2/20/98
	PrvEditLoadTable(frmP);

	//TblUnhighlightSelection (table);	// moved call before PrvEditLoadTable vmk 2/20/98
	TblRedrawTable (table);

	exit:
		FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditNextField
 *
 * DESCRIPTION: If a field is being edited, advance the focus to the
 * edit view table's next field.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   7/27/95   Initial Revision
 *
 ***********************************************************************/
void PrvEditNextField (WinDirectionType direction)
{
	TablePtr tableP;
	Int16 row;
	Int16 column;
	UInt16 nextFieldNumIndex;
	FormType* frmP;

	frmP = FrmGetActiveForm();
	tableP = ToolsGetFrmObjectPtr(frmP, EditTable);

	if (!TblEditing(tableP))
		return;

	// Find out which field is being edited.
	TblGetSelection (tableP, &row, &column);
	nextFieldNumIndex = TblGetRowID (tableP, row);
	if (direction == winDown)
	{
		if (nextFieldNumIndex >= editLastFieldIndex)
			nextFieldNumIndex = 0;
		else
			nextFieldNumIndex++;
	}
	else
	{
		if (nextFieldNumIndex == 0)
			nextFieldNumIndex = editLastFieldIndex;
		else
			nextFieldNumIndex--;
	}
	TblReleaseFocus (tableP);

	CurrentFieldIndex = nextFieldNumIndex;

	// If the new field isn't visible move the edit view and then
	// find the row where the next field is.
	while (!TblFindRowID(tableP, nextFieldNumIndex, &row))
	{
		// Scroll the view down placing the item
		// on the top row
		TopVisibleFieldIndex = nextFieldNumIndex;
		PrvEditLoadTable(frmP);
		TblRedrawTable(tableP);
	}

	PrvEditHandleSelectField(row, editDataColumn);
}




/***********************************************************************
 *
 * FUNCTION:    PrvEditUpdateCustomFieldLabels
 *
 * DESCRIPTION: Update the custom field labels by reloading those rows
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date      Description
 *			----	----      -----------
 *			art		6/5/95      Initial Revision
 *			aro		9/26/00		Adding frmP as an argument for the updateEvent
 *
 ***********************************************************************/
static void PrvEditUpdateCustomFieldLabels( FormType* frmP)
{
	UInt16 row;
	UInt16 rowsInTable;
	TablePtr table;
	AddrAppInfoPtr appInfoPtr;
	UInt16 fieldIndex;
	UInt16 fieldNum;
	AddrDBRecordType record;
	MemHandle recordH;
	Boolean redraw = false;


	appInfoPtr = (AddrAppInfoPtr)AddrDBAppInfoGetPtr(AddrDB);
	table = ToolsGetFrmObjectPtr(frmP, EditTable);

	if (TblGetColumnWidth(table, editLabelColumn) != EditLabelColumnWidth)
	{
		PrvEditInit (frmP, false);
		redraw = true;
	}
	else
	{


		// Get the current record
		AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);


		rowsInTable = TblGetNumberOfRows(table);

		// Reload any renameable fields
		for (row = 0; row < rowsInTable; row++)
		{
			if (TblRowUsable (table, row))
			{
				fieldIndex = TblGetRowID (table, row);
				fieldNum = FieldMap[fieldIndex];
				if (fieldNum >= firstRenameableLabel &&
					fieldNum <= lastRenameableLabel)
				{
					PrvEditInitTableRow(frmP, table, row, fieldIndex,
										TblGetRowHeight (table, row),
										TblGetItemFont (table, row, editDataColumn),
										&record, appInfoPtr);
					redraw = true;

					// Mark the row invalid so that it will draw when we call the
					// draw routine.
					TblMarkRowInvalid (table, row);
				}
			}
		}


		MemHandleUnlock(recordH);
	}


	if (redraw)
		TblRedrawTable(table);


	MemPtrUnlock(appInfoPtr);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditUpdateDisplay
 *
 * DESCRIPTION: This routine update the display of the edit view
 *
 * PARAMETERS:  updateCode - a code that indicated what changes  been
 *                           have made to the view.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/12/97	Initial Revision
 *			jmp	11/02/99	Fixed problem on frmRedrawUpdateCode events when
 *								we're still in the edit state but we weren't redrawing
 *								the edit indicator.  Fixes Address part of bug #23235.
 *			jmp	12/09/99	Fix bug #23914:  On frmRedrawUpdateCode, don't reload
 *								the table, just redraw it!
 *			aro 09/21		removed FrmGetActiveForm since it can be wrong
 *								Also don't return a Boolean anymore and should handle all updateCode
 *
 ***********************************************************************/
void PrvEditUpdateDisplay( UInt16 updateCode )
{
	TableType* table;
	FormType* frmP;

	// Get form by Id since it might now be the active form
	frmP = FrmGetFormPtr(EditView);

	if (updateCode & updateCustomFieldLabelChanged)
	{
		PrvEditUpdateCustomFieldLabels(frmP);
	}

	if (updateCode & updateCategoryChanged)
	{
		// Set the label of the category trigger.
		CategoryGetName (AddrDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel(ToolsGetFrmObjectPtr(frmP, EditCategoryTrigger), CategoryName);
	}

	// Perform common tasks as necessary, and in the proper order.
	if ((updateCode & updateFontChanged) || (updateCode & updateGrabFocus) || (updateCode & frmRedrawUpdateCode))
	{
		if (updateCode & frmRedrawUpdateCode)
			FrmDrawForm(frmP);

		if ((updateCode & updateFontChanged) || (updateCode & frmRedrawUpdateCode))
			table = ToolsGetFrmObjectPtr(frmP, EditTable);

		if (updateCode & updateFontChanged)
			TblReleaseFocus (table);

		if (updateCode & frmRedrawUpdateCode)
		{
			// If we're editing, then find out which row is being edited,
			// mark it invalid, and redraw the table (below).
			if (TblEditing(table))
			{
				Int16 row;
				Int16 column;

				TblGetSelection (table, &row, &column);
				TblMarkRowInvalid(table, row);
				TblRedrawTable (table);
			}
		}

		if ((updateCode & updateFontChanged))
		{
			PrvEditLoadTable(frmP);
			TblRedrawTable (table);
		}

		if ((updateCode & updateFontChanged) || (updateCode & updateGrabFocus))
			PrvEditRestoreEditState(frmP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  6/27/95   Initial Revision
 *         jmp    9/17/99   Use NoteView instead of NoteView.
 *         aro    6/27/00	Add dialing
 *
 ***********************************************************************/
Boolean PrvEditDoCommand (UInt16 command)
{
	AddrDBRecordType record;
	MemHandle recordH;
	Boolean hasNote;
	UInt16 newRecord;
	UInt16 numCharsToHilite;
	Boolean hasData;
	FormType* frmP;

	TraceOutput(TL(appErrorClass, "PrvEditDoCommand() - command = %hu", command));

	frmP = FrmGetActiveForm();

	switch (command)
	{
		case EditRecordDeleteRecordCmd:
			MenuEraseStatus (0);
			FrmSetFocus(frmP, noFocus);   // save the field
			if (DetailsDeleteRecord ())
				FrmGotoForm (ListView);
			else
				PrvEditRestoreEditState(frmP);
			return true;
	
		case EditRecordDuplicateAddressCmd:
			// Make sure the field being edited is saved
			FrmSetFocus(frmP, noFocus);
	
			AddrDBGetRecord(AddrDB, CurrentRecord, &record, &recordH);
	
			hasData = AddrDBRecordContainsData(&record);
			MemHandleUnlock(recordH);
	
			newRecord = ToolsDuplicateCurrentRecord (&numCharsToHilite, !hasData);
			if (newRecord != noRecord)
			{
				NumCharsToHilite = numCharsToHilite;
				CurrentRecord = newRecord;
        //SysDebug(1, "Addr", "PrvEditDoCommand set CurrentRecord=%d", CurrentRecord);
				FrmGotoForm (EditView);
			}
			return true;
	
		case EditRecordDialCmd:
			MenuEraseStatus (0);
			PrvEditDialCurrent();
			return true;
	
		case EditRecordAttachNoteCmd:
			MenuEraseStatus (0);
			TblReleaseFocus(ToolsGetObjectPtr(EditTable));   // save the field
			if (NoteViewCreate())
			{
				RecordNeededAfterEditView = true;
				FrmGotoForm (NewNoteView);
			}
			return true;
	
		case EditRecordDeleteNoteCmd:
			MenuEraseStatus (0);
			AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);
			hasNote = (record.fields[ad_note] != NULL);
			MemHandleUnlock(recordH);
	
			if (hasNote && FrmAlert(DeleteNoteAlert) == DeleteNoteYes)
				NoteViewDelete ();
			return true;
	
		case EditRecordSelectBusinessCardCmd:
			MenuEraseStatus (0);
			if (FrmAlert(SelectBusinessCardAlert) == SelectBusinessCardYes)
			{
				DmRecordInfo (AddrDB, CurrentRecord, NULL, &BusinessCardRecordID, NULL);
				PrvEditDrawBusinessCardIndicator (frmP);
			}
			return true;
	
		case EditRecordBeamBusinessCardCmd:
			// Make sure the field being edited is saved
			FrmSetFocus(frmP, noFocus);
	
			MenuEraseStatus (0);
			ToolsAddrBeamBusinessCard(AddrDB);
			return true;
	
		case EditRecordBeamRecordCmd:
			// Make sure the field being edited is saved
			FrmSetFocus(frmP, noFocus);
	
			MenuEraseStatus (0);
			TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
			return true;
	
		case EditRecordSendRecordCmd:
			// Make sure the field being edited is saved
			FrmSetFocus(frmP, noFocus);
	
			MenuEraseStatus (0);
			TransferSendRecord(AddrDB, CurrentRecord, exgSendPrefix, NoDataToSendAlert);
			return true;
	
		case EditOptionsFontCmd:
			MenuEraseStatus (0);
			AddrEditFont = ToolsSelectFont (AddrEditFont);
			return true;
	
		case EditOptionsEditCustomFldsCmd:
			MenuEraseStatus (0);
			FrmSetFocus(frmP, noFocus);   // save the field
			FrmPopupForm (CustomEditDialog);
			return true;
	
		case EditOptionsAboutCmd:
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
 * FUNCTION:    RecordViewAutoFill
 *
 * DESCRIPTION: This routine handles auto-filling the vendor or city
 *              fields.
 *
 * PARAMETERS:  event  - pointer to a keyDownEvent.
 *
 * RETURNED:    true if the event has been handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	01/15/96		Initial Revision
 *			trm	07/20/97		Fixed Autofill bug
 *			kwk	11/20/98		Check for command key, return false if set.
 *			CSS	06/22/99		Standardized keyDownEvent handling
 *									(TxtCharIsHardKey, commandKeyMask, etc.)
 *			bhall	07/12/99		Modified from Expense.c/DetailsAutoFill
 *			bob	11/17/99		double check that table has a field before using it
 *
 ***********************************************************************/
Boolean PrvEditAutoFill (EventPtr event)
{
	UInt16		index;
	UInt16		focus;
	Char			*key;
	UInt32		dbType;
	FormPtr		frmP;
	FieldPtr		fld;
	DmOpenRef	dbP;
	TablePtr		table;
	UInt16 		fieldIndex;
	UInt16		fieldNum;
	UInt16		pos;

	if	(	TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr)
			||	(EvtKeydownIsVirtual(event))
			|| (!TxtCharIsPrint(event->data.keyDown.chr)))
		return false;

	frmP = FrmGetActiveForm();
	focus = FrmGetFocus(frmP);
	if (focus == noFocus)
		return false;

	// Find the table
	table = ToolsGetFrmObjectPtr(frmP, EditTable);

	// check to see if there really is a field before continuing.
	// in case table has stopped editing but form still thinks table is the
	// focus.
	if (TblGetCurrentField(table) == NULL)
		return false;

	// Get the field number that corresponds to the table item to save.
	{
		Int16 row;
		Int16 column;

		TblGetSelection(table, &row, &column);
		fieldIndex = TblGetRowID(table, row);
	}
	fieldNum = FieldMap[fieldIndex];

	// Select the proper database for the field we are editing,
	// or return right away if not an autofill enabled field
	switch (fieldNum) {
	case ad_title:		dbType = titleDBType; break;
	case ad_company:	dbType = companyDBType; break;
	case ad_city:		dbType = cityDBType; break;
	case ad_state:		dbType = stateDBType; break;
	case ad_country:	dbType = countryDBType; break;
	default:	return false;
	}

	// Let the OS insert the character into the field.
	FrmHandleEvent(frmP, event);

	// The current value of the field with the focus.
	fld = TblGetCurrentField(table);
	key = FldGetTextPtr(fld);
	pos = FldGetInsPtPosition(fld);
	
	// Only auto-fill if the insertion point is at the end.
	if (pos != FldGetTextLength(fld))
		return true;

	// Open the database
	dbP = DmOpenDatabaseByTypeCreator(dbType, AddressBookCreator, dmModeReadOnly);
	if (!dbP) return true;

	// Check for a match.
	if (AutoFillLookupStringInDatabase(dbP, key, &index)) {
		MemHandle				h;
		LookupRecordPtr	r;
		UInt16				len;
		Char 					*ptr;

		h = DmQueryRecord(dbP, index);
		r = MemHandleLock(h);

		// Auto-fill.
		ptr = &r->text + StrLen (key);
		len = StrLen(ptr);

		FldInsert(fld, ptr, StrLen(ptr));

		// Highlight the inserted text.
		FldSetSelection(fld, pos, pos + len);

		MemHandleUnlock(h);
	}

	// Close the database
	DmCloseDatabase(dbP);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvEditDialCurrent
 *
 * DESCRIPTION:
 *	This routine number in current field, or default one (show in list)
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/27/00		Initial Revision
 *
 ***********************************************************************/
void PrvEditDialCurrent( void )
{
	TableType* tblP;
	MemHandle	addrH;
	AddrDBRecordType	addr;
	Err			err;
	UInt16	phoneIndex = kDialListShowInListPhoneIndex;
	UInt16	lineIndex = 0;
	UInt16	fieldPosition = 0;

	// if focus on a phone, dial current number
	// else dial show in list

	tblP = ToolsGetObjectPtr(EditTable);

	if (TblEditing(tblP))
	{
		Int16		row;
		Int16 		column;
		UInt16 		fieldIndex;
		FieldType* 	fldP;

		// Find out which field is being edited.
		TblGetSelection(tblP, &row, &column);
		fldP = TblGetCurrentField(tblP);
		if (fldP)
			fieldPosition = FldGetInsPtPosition(fldP);

		// phoneIndex = TblGetRowID(tblP, row) - firstPhoneField;
		TblReleaseFocus(tblP);

		fieldIndex = FieldMap[TblGetRowID(tblP, row)];
		if (isPhoneField(fieldIndex))
			phoneIndex = fieldIndex - firstPhoneField;
	}

	err = AddrDBGetRecord(AddrDB, CurrentRecord, &addr, &addrH);
	if ( err != errNone )
		return;

	if (phoneIndex != kDialListShowInListPhoneIndex)
	{
		if (ToolsIsPhoneIndexSupported(&addr, phoneIndex))
			lineIndex = ToolsGetLineIndexAtOffset(addr.fields[phoneIndex + firstPhoneField], fieldPosition);
		else
			// This mean like something like mail was edited
			phoneIndex = kDialListShowInListPhoneIndex;
	}
	MemHandleUnlock(addrH);
	DialListShowDialog(CurrentRecord, phoneIndex, lineIndex);
}


/***********************************************************************
 *
 * FUNCTION:    PrvEditLoadTable
 *
 * DESCRIPTION: This routine reloads to do database records into
 *              the edit view.  This routine is called when:
 *                 o A field height changes (Typed text wraps to the next line)
 *                 o Scrolling
 *                 o Advancing to the next field causes scrolling
 *                 o The focus moves to another field
 *                 o A custom label changes
 *                 o The form is first opened.
 *
 *                The row ID is an index into FieldMap.
 *
 * PARAMETERS:  startingRow - index of the first row to redisplay.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *			art	9/10/97	Rewrote to support user selectable fonts.
 *			aro	9/21/00	Add frmP as an argument for the updateEvent
 *
 ***********************************************************************/
void PrvEditLoadTable( FormType* frmP )
{
	UInt16 row;
	UInt16 numRows;
	UInt16 lineHeight;
	UInt16 fieldIndex;
	UInt16 lastFieldIndex;
	UInt16 dataHeight;
	UInt16 tableHeight;
	UInt16 columnWidth;
	UInt16 pos, oldPos;
	UInt16 height, oldHeight;
	FontID fontID;
	FontID currFont;
	TablePtr table;
	Boolean rowUsable;
	Boolean rowsInserted = false;
	Boolean lastItemClipped;
	RectangleType r;
	AddrDBRecordType record;
	MemHandle recordH;
	AddrAppInfoPtr appInfoPtr;


  //SysDebug(1, "Addr", "PrvEditLoadTable begin");
	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);

	// Get the current record
	AddrDBGetRecord (AddrDB, CurrentRecord, &record, &recordH);
  //SysDebug(1, "Addr", "PrvEditLoadTable CurrentRecord %d", CurrentRecord);

	// Get the height of the table and the width of the description
	// column.
	table = ToolsGetFrmObjectPtr(frmP, EditTable);
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (table, editDataColumn);

	// If we currently have a selected record, make sure that it is not
	// above the first visible record.
	if (CurrentFieldIndex != noFieldIndex)
	{
		if (CurrentFieldIndex < TopVisibleFieldIndex)
			TopVisibleFieldIndex = CurrentFieldIndex;
	}

	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;
	fieldIndex = TopVisibleFieldIndex;
	lastFieldIndex = fieldIndex;

	// Load records into the table.
	while (fieldIndex <= editLastFieldIndex)
	{
		// Compute the height of the field's text string.
		height = PrvEditGetFieldHeight (table, fieldIndex, columnWidth, tableHeight, &record, &fontID);

		// Is there enought room for at least one line of the the decription.
		currFont = FntSetFont (fontID);
		lineHeight = FntLineHeight ();
		FntSetFont (currFont);
		if (tableHeight >= dataHeight + lineHeight)
		{
			rowUsable = TblRowUsable (table, row);

			// Get the height of the current row.
			if (rowUsable)
				oldHeight = TblGetRowHeight (table, row);
			else
				oldHeight = 0;

			// If the field is not already being displayed in the current
			// row, load the field into the table.
			if ((! rowUsable) ||
				(TblGetRowID (table, row) != fieldIndex) ||
				(TblGetItemFont (table, row, editDataColumn) != fontID))
			{
				PrvEditInitTableRow(frmP, table, row, fieldIndex, height, fontID,
									&record, appInfoPtr);
			}

			// If the height or the position of the item has changed draw the item.
			else if (height != oldHeight)
			{
				TblSetRowHeight (table, row, height);
				TblMarkRowInvalid (table, row);
			}
			else if (pos != oldPos)
			{
				TblMarkRowInvalid (table, row);
			}

			pos += height;
			oldPos += oldHeight;
			lastFieldIndex = fieldIndex;
			fieldIndex++;
			row++;
		}

		dataHeight += height;


		// Is the table full?
		if (dataHeight >= tableHeight)
		{
			// If we have a currently selected field, make sure that it is
			// not below the last visible field.  If the currently selected
			// field is the last visible record, make sure the whole field
			// is visible.
			if (CurrentFieldIndex == noFieldIndex)
				break;

			// Above last visible?
			else if  (CurrentFieldIndex < fieldIndex)
				break;

			// Last visible?
			else if (fieldIndex == lastFieldIndex)
			{
				if ((fieldIndex == TopVisibleFieldIndex) || (dataHeight == tableHeight))
					break;
			}

			// Remove the top item from the table and reload the table again.
			TopVisibleFieldIndex++;
			fieldIndex = TopVisibleFieldIndex;


			// Below last visible.
			//			else
			//				TopVisibleFieldIndex = CurrentFieldIndex;

			row = 0;
			dataHeight = 0;
			oldPos = pos = 0;
		}
	}


	// Hide the item that don't have any data.
	numRows = TblGetNumberOfRows (table);
	while (row < numRows)
	{
		TblSetRowUsable (table, row, false);
		row++;
	}

	// If the table is not full and the first visible field is
	// not the first field	in the record, displays enough fields
	// to fill out the table by adding fields to the top of the table.
	while (dataHeight < tableHeight)
	{
		fieldIndex = TopVisibleFieldIndex;
		if (fieldIndex == 0) break;
		fieldIndex--;

		// Compute the height of the field.
		height = PrvEditGetFieldHeight (table, fieldIndex,
										columnWidth, tableHeight, &record, &fontID);


		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;

		// Insert a row before the first row.
		TblInsertRow (table, 0);

		PrvEditInitTableRow(frmP, table, 0, fieldIndex, height, fontID, &record, appInfoPtr);

		TopVisibleFieldIndex = fieldIndex;

		rowsInserted = true;

		dataHeight += height;
	}

	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		TblMarkTableInvalid (table);

	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clip and the
	// table is scrollable.
	lastItemClipped = (dataHeight > tableHeight);

	// Update the scroll arrows.
	PrvEditUpdateScrollers(frmP, lastFieldIndex, lastItemClipped);


	MemHandleUnlock(recordH);
	MemPtrUnlock(appInfoPtr);
  //SysDebug(1, "Addr", "PrvEditLoadTable end");
}
