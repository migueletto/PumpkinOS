/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrDetails.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the Address Book application's pref screen
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrDetails.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddrNote.h"
#include "AddressRsc.h"

#include <Category.h>
#include <UIResources.h>


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static Boolean	PrvDetailsSelectCategory (UInt16 * category) SEC("code2");
static UInt16	PrvDetailsApply (UInt16 category, Boolean categoryEdited) SEC("code2");
static void		PrvDetailsInit (UInt16 * categoryP) SEC("code2");


/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/

// Valid after DetailsDialogInit
static Char * DetailsPhoneListChoices[numPhoneFields];


/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" of the Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art    6/5/95    Initial Revision
 *         jmp    9/17/99   Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
Boolean DetailsHandleEvent (EventType * event)
{
	static UInt16       category;
	static Boolean      categoryEdited;

	UInt16 updateCode;
	Boolean handled = false;
	FormPtr frm;


	#if WRISTPDA
	if (event->eType == keyDownEvent) {
		EventType newEvent;
		MemSet (&newEvent, sizeof(EventType), 0);
		newEvent.eType = ctlSelectEvent;
		newEvent.data.ctlSelect.on = true;
		frm = FrmGetActiveForm();
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to a Done button event.
			newEvent.data.ctlSelect.controlID = DetailsOkButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to a Cancel button event.
			newEvent.data.ctlSelect.controlID = DetailsCancelButton;
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
		case DetailsOkButton:
			updateCode = PrvDetailsApply (category, categoryEdited);
			ToolsLeaveForm ();
			if (updateCode)
				FrmUpdateForm (EditView, updateCode);
			handled = true;
			break;

		case DetailsCancelButton:
			if (categoryEdited)
				FrmUpdateForm (EditView, updateCategoryChanged);
			ToolsLeaveForm ();
			handled = true;
			break;

		case DetailsDeleteButton:
			if ( DetailsDeleteRecord ())
			{
				FrmCloseAllForms ();
				FrmGotoForm (ListView);
			}
			handled = true;
			break;

		case DetailsNoteButton:
			PrvDetailsApply (category, categoryEdited);
			FrmReturnToForm (EditView);
			if (NoteViewCreate())
			{
				FrmGotoForm (NewNoteView);
				RecordNeededAfterEditView = true;
			}
			handled = true;
			break;

		case DetailsCategoryTrigger:
			categoryEdited = PrvDetailsSelectCategory (&category) || categoryEdited;
			handled = true;
			break;
		}
	}


	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		PrvDetailsInit (&category);
		FrmDrawForm (frm);
		categoryEdited = false;
		handled = true;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsDeleteRecord
 *
 * DESCRIPTION: This routine deletes an address record. This routine is
 *              called when the delete button in the details dialog is
 *              pressed or when delete record is used from a menu.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95   Initial Revision
 *         art   9/30/95   Remember the "save backup" settting.
 *
 ***********************************************************************/
Boolean DetailsDeleteRecord (void)
{
	UInt16 ctlIndex;
	UInt16 buttonHit;
	FormPtr alert;
	Boolean archive;

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

	ToolsDeleteRecord(archive);

	return (true);
}

//#pragma mark -


/***********************************************************************
 *
 * FUNCTION:    PrvDetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the category was changed in a way that
 *              require the list view to be redrawn.
 *
 *              The following global variables are modified:
 *                     CategoryName
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *       art   06/05/95   Initial Revision
 *       art   09/28/95   Fixed problem with merging and renaming
 *			gap	08/13/99   Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
Boolean PrvDetailsSelectCategory (UInt16 * category)
{
	Boolean categoryEdited;

	categoryEdited = CategorySelect (AddrDB, FrmGetActiveForm (),
									 DetailsCategoryTrigger, DetailsCategoryList,
									 false, category, CategoryName, 1, categoryDefaultEditCategoryString);

	return (categoryEdited);
}


/***********************************************************************
 *
 * FUNCTION:    PrvDetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  category - new catagory
 *
 * RETURNED:    code which indicates how the to do list was changed,  this
 *              code is send as the update code, in the frmUpdate event.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95   Initial Revision
 *         kcr   10/9/95   added 'private records' alert
 *
 ***********************************************************************/
UInt16 PrvDetailsApply (UInt16 category, Boolean categoryEdited)
{
	UInt16               attr;
	UInt16                updateCode = 0;
	Boolean           secret;
	Boolean            dirty = false;
	AddrDBRecordType currentRecord;
	MemHandle recordH;
	AddrDBRecordFlags changedFields;
	UInt16               newPhoneFieldToDisplay;
	Err               err;



	// Get the phone number to show at the list view.
	AddrDBGetRecord(AddrDB, CurrentRecord, &currentRecord, &recordH);
	newPhoneFieldToDisplay = LstGetSelection(ToolsGetObjectPtr (DetailsPhoneList));
	if (currentRecord.options.phones.displayPhoneForList != newPhoneFieldToDisplay)
	{
		currentRecord.options.phones.displayPhoneForList = newPhoneFieldToDisplay;
		changedFields.allBits = 0;
		err = AddrDBChangeRecord(AddrDB, &CurrentRecord, &currentRecord,
							   changedFields);
		if (err)
		{
			MemHandleUnlock(recordH);
			FrmAlert(DeviceFullAlert);
		}

		updateCode |= updateListViewPhoneChanged;
	}
	else
		MemHandleUnlock(recordH);


	// Get the category and the secret attribute of the current record.
	DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);


	// Get the current setting of the secret checkbox and compare it the
	// the setting of the record.  Update the record if the values
	// are different.  If the record is being set 'secret' for the
	//   first time, and the system 'hide secret records' setting is
	//   off, display an informational alert to the user.
	secret = CtlGetValue (ToolsGetObjectPtr (DetailsSecretCheckbox));
	if (((attr & dmRecAttrSecret) == dmRecAttrSecret) != secret)
	{
		if (PrivateRecordVisualStatus > showPrivateRecords)
		{
			updateCode |= updateItemHide;
		}
		else if (secret)
			FrmAlert (privateRecordInfoAlert);

		dirty = true;

		if (secret)
			attr |= dmRecAttrSecret;
		else
			attr &= ~dmRecAttrSecret;
	}


	// Compare the current category to the category setting of the dialog.
	// Update the record if the category are different.
	if ((attr & dmRecAttrCategoryMask) != category)
	{
		attr &= ~dmRecAttrCategoryMask;
		attr |= category;
		dirty = true;

		CurrentCategory = category;
		updateCode |= updateCategoryChanged;
	}

	// If current category was moved, deleted renamed, or merged with
	// another category, then the list view needs to be redrawn.
	if (categoryEdited)
	{
		CurrentCategory = category;
		updateCode |= updateCategoryChanged;
	}


	// Save the new category and/or secret status, and mark the record dirty.
	if (dirty)
	{
		attr |= dmRecAttrDirty;
		DmSetRecordInfo (AddrDB, CurrentRecord, &attr, NULL);
	}

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    PrvDetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
void PrvDetailsInit (UInt16 * categoryP)
{
	UInt16 attr;
	UInt16 category;
	ControlPtr ctl;
	ListPtr popupPhoneList;
	AddrDBRecordType currentRecord;
	MemHandle recordH;
	AddrAppInfoPtr appInfoPtr;
	UInt16 phoneLabel;
	UInt16 i;


	// Make a list of the phone labels used by this record
	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
	AddrDBGetRecord(AddrDB, CurrentRecord, &currentRecord, &recordH);

	for (i = 0; i < 5; i++)
	{
		phoneLabel = GetPhoneLabel(&currentRecord, firstPhoneField + i);
		DetailsPhoneListChoices[i] = appInfoPtr->fieldLabels[phoneLabel +
															 ((phoneLabel < numPhoneLabelsStoredFirst) ?
															  firstPhoneField : (ad_addressFieldsCount - numPhoneLabelsStoredFirst))];
	}


	// Set the default phone list to the list of phone labels just made
	popupPhoneList = ToolsGetObjectPtr (DetailsPhoneList);
	LstSetListChoices(popupPhoneList, DetailsPhoneListChoices, numPhoneFields);
	LstSetSelection (popupPhoneList, currentRecord.options.phones.displayPhoneForList);
	LstSetHeight (popupPhoneList, numPhoneFields);
	CtlSetLabel(ToolsGetObjectPtr (DetailsPhoneTrigger),
				LstGetSelectionText (popupPhoneList, currentRecord.options.phones.displayPhoneForList));

	// If the record is mark secret, turn on the secret checkbox.
	DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
	ctl = ToolsGetObjectPtr (DetailsSecretCheckbox);
	CtlSetValue (ctl, attr & dmRecAttrSecret);

	// Set the label of the category trigger.
	category = attr & dmRecAttrCategoryMask;
	CategoryGetName (AddrDB, category, CategoryName);
	ctl = ToolsGetObjectPtr (DetailsCategoryTrigger);
	CategorySetTriggerLabel (ctl, CategoryName);

	// Return the current category and due date.
	*categoryP = category;


	MemHandleUnlock(recordH);
	MemPtrUnlock(appInfoPtr);
}


