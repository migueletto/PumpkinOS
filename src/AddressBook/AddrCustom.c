/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrCustom.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the Address Book "Rename Custom Fields" screen
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrCustom.h"
#include "AddressDB.h"
#include "AddressRsc.h"
#include "AddrTools.h"
#include "Address.h"

#include <Form.h>
#include <StringMgr.h>


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void CustomEditSave (FormPtr frm) SEC("code2");
static void CustomEditInit (FormPtr frm) SEC("code2");


/***********************************************************************
 *
 * FUNCTION:    CustomEditSave
 *
 * DESCRIPTION: Write the renamed field labels
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   2/23/95   Initial Revision
 *
 ***********************************************************************/
void CustomEditSave (FormPtr frm)
{
	UInt16      index;
	FieldPtr   fld;
	UInt16      objNumber;
	Char * textP;
	AddrAppInfoPtr appInfoPtr;
	Boolean sendUpdate = false;


	// Get the object number of the first field.
	objNumber = FrmGetObjectIndex (frm, CustomEditFirstField);


	// For each dirty field update the corresponding label.
	for (index = firstRenameableLabel; index <= lastRenameableLabel; index++)
	{
		fld = FrmGetObjectPtr (frm, objNumber++);
		if (FldDirty(fld))
		{
			sendUpdate = true;
			textP = FldGetTextPtr(fld);
			if (textP)
				AddrDBSetFieldLabel(AddrDB, index, textP);
		}
	}

	if (sendUpdate)
	{
		// Update the column width since a label changed.
		appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
		#if WRISTPDA
		EditLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, FossilStdFont);
		#else
		EditLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, stdFont);
		#endif
		RecordLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, AddrRecordFont);
		MemPtrUnlock(appInfoPtr);

		FrmUpdateForm (0, updateCustomFieldLabelChanged);
	}

}


/***********************************************************************
 *
 * FUNCTION:    CustomEditInit
 *
 * DESCRIPTION: Load field labels for editing.
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   2/23/95   Initial Revision
 *
 ***********************************************************************/
void CustomEditInit (FormPtr frm)
{
	UInt16      index;
	UInt16      length;
	FieldPtr   fld;
	UInt16      objNumber;
	MemHandle textH;
	Char * textP;
	AddrAppInfoPtr appInfoPtr;
	addressLabel *fieldLabels;


	// Get the object number of the first field.
	objNumber = FrmGetObjectIndex (frm, CustomEditFirstField);

	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
	fieldLabels = appInfoPtr->fieldLabels;

	// For each label, allocate some global heap space and copy the
	// the string to the global heap.  Then set a field to use the
	// copied string for editing.  If the field is unused no space is
	// allocated.  The field will allocate space if text is typed in.
	for (index = firstRenameableLabel; index <= lastRenameableLabel; index++)
	{
		fld = FrmGetObjectPtr (frm, objNumber++);
		length = StrLen(fieldLabels[index]);
		if (length > 0)
		{
			length += 1;         // include space for a null terminator
			textH = MemHandleNew(length);
			if (textH)
			{
				textP = MemHandleLock(textH);
				MemMove(textP, fieldLabels[index], length);
				FldSetTextHandle (fld, textH);
				MemHandleUnlock(textH);
			}
		}
	}

	MemPtrUnlock(appInfoPtr);
}


/***********************************************************************
 *
 * FUNCTION:    CustomEditHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit Custom
 *              Fields" of the Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   06/23/95   Initial Revision
 *         FPa     11/28/00   Sends an event when OK button pressed
 *
 ***********************************************************************/
Boolean CustomEditHandleEvent (EventType * event)
{
	Boolean handled = false;
	FormPtr frm;


	#if WRISTPDA
	if (event->eType == keyDownEvent) {
		EventType newEvent;
		frm = FrmGetActiveForm();
		MemSet (&newEvent, sizeof(EventType), 0);
		newEvent.eType = ctlSelectEvent;
		newEvent.data.ctlSelect.on = true;
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to a Done button event.
			newEvent.data.ctlSelect.controlID = CustomEditOkButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to a Cancel button event.
			newEvent.data.ctlSelect.controlID = CustomEditCancelButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		}
	} else
	#endif

	if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
			case CustomEditOkButton:
			{
				EventType evt;
				
				frm = FrmGetActiveForm();
				CustomEditSave(frm);
				ToolsLeaveForm();

				evt.eType = kFrmCustomUpdateEvent;
				EvtAddEventToQueue(&evt);	// We send this event because View screen needs to recalculate its display when Custom fields are renamed
				handled = true;
				break;
			}
	
			case CustomEditCancelButton:
				ToolsLeaveForm();
				handled = true;
				break;
			default:
				break;

		}
	}


	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		CustomEditInit (frm);
		FrmDrawForm (frm);
		handled = true;
	}

	return (handled);
}
