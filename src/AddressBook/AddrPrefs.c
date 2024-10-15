/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrPrefs.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the Address Book application's pref screen
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrPrefs.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddressTransfer.h"
#include "AddrDialList.h"
#include "AddressRsc.h"

#include <ErrorMgr.h>

/***********************************************************************
 *
 *	Defines
 *
 ***********************************************************************/

#define addrPrefVersionNum					0x04
#define addrPrefID							0x00


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void PrvPrefsSave (FormPtr frm) SEC("code2");
static void PrvPrefsInit (FormPtr frm) SEC("code2");


/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/

typedef struct {
	UInt16			currentCategory;
	FontID			v20NoteFont;				// For 2.0 compatibility (BGT)
	Boolean			showAllCategories;
	Boolean			saveBackup;
	Boolean			rememberLastCategory;

	// Version 3 preferences
	FontID			addrListFont;
	FontID			addrRecordFont;
	FontID			addrEditFont;
	UInt8 			reserved1;
	UInt32			businessCardRecordID;
	FontID			noteFont;
	UInt8 			reserved2;

	// Version 4 preferences
	Boolean			enableTapDialing;
} AddrPreferenceType;


/***********************************************************************
 *
 * FUNCTION:    AddressLoadPrefs
 *
 * DESCRIPTION: Load the application preferences and fix them up if
 *				there's a version mismatch.
 *
 * PARAMETERS:  appInfoPtr	-- Pointer to the app info structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         BGT   1/8/98     Initial revision
 *
 ***********************************************************************/

void PrefsLoad(AddrAppInfoPtr appInfoPtr)
{
	Int16 prefsVersion;
	UInt16 prefsSize;
	AddrPreferenceType prefs;

	// Read the preferences / saved-state information.  There is only one
	// version of the Address Book preferences so don't worry about multiple
	// versions.  Users appreciate the transferal of their preferences from prior
	// versions.
	prefsSize = sizeof (AddrPreferenceType);
	#if WRISTPDA
	// First read the state information from the Wrist PDA private preferences.
	prefsVersion = PrefGetAppPreferences (WPdaCreator, 'Ad', &prefs, &prefsSize, true);
	if (prefsVersion > addrPrefVersionNum) {
		prefsVersion = noPreferenceFound;
	}
	// If reading the private prefs failed then read the standard preferences.
	if (prefsVersion == noPreferenceFound) {
		prefsVersion = PrefGetAppPreferences (AddressBookCreator, addrPrefID, &prefs, &prefsSize, true);
	}
	#else
	prefsVersion = PrefGetAppPreferences (AddressBookCreator, addrPrefID, &prefs, &prefsSize, true);
	#endif
	if (prefsVersion > addrPrefVersionNum) {
		prefsVersion = noPreferenceFound;
	}
	if (prefsVersion > noPreferenceFound)
	{
		if (prefsVersion < addrPrefVersionNum) {
			prefs.noteFont = prefs.v20NoteFont;
		}
		SaveBackup = prefs.saveBackup;
		RememberLastCategory = prefs.rememberLastCategory;
		if (prefs.noteFont == largeFont)
			NoteFont = largeBoldFont;
		else
			NoteFont = prefs.noteFont;
		//NoteFont = FossilLargeFontID( WRISTPDA, NoteFont );

		// If the preferences are set to use the last category and if the
		// category hasn't been deleted then use the last category.
		if (RememberLastCategory &&
			prefs.currentCategory != dmAllCategories &&
			appInfoPtr->categoryLabels[prefs.currentCategory][0] != '\0')
		{
			CurrentCategory = prefs.currentCategory;
			ShowAllCategories = prefs.showAllCategories;
		}

		// Support transferal of preferences from the previous version of the software.
		if (prefsVersion >= 3)
		{
			// Values not set here are left at their default values
			//AddrListFont = FossilLargeFontID( WRISTPDA, prefs.addrListFont );
			//AddrRecordFont = FossilLargeFontID( WRISTPDA, prefs.addrRecordFont );
			//AddrEditFont = FossilLargeFontID( WRISTPDA, prefs.addrEditFont );
			AddrListFont = prefs.addrListFont;
			AddrRecordFont = prefs.addrRecordFont;
			AddrEditFont = prefs.addrEditFont;
			BusinessCardRecordID = prefs.businessCardRecordID;
		}

		// Support transferal of preferences from the previous version of the software.
		if (prefsVersion >= addrPrefVersionNum)
		{
			#if WRISTPDA
			EnableTapDialing = false;
			#else
			EnableTapDialing = prefs.enableTapDialing;
			#endif
		}
	}

	// The first time this app starts register to handle vCard data.
	if (prefsVersion != addrPrefVersionNum)
		TransferRegisterData();

	MemPtrUnlock(appInfoPtr);
}

/***********************************************************************
 *
 * FUNCTION:    AddressSavePrefs
 *
 * DESCRIPTION: Save the Address preferences with fixups so that
 *				previous versions won't go crazy.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         BGT   1/8/98     Initial Revision
 *         SCL   2/12/99    Clear reserved fields before writing saved prefs
 *			  gap  12/06/00	 EnableTapDialing is now false by default, only
 *									 checked if turned on by user
 *
 ***********************************************************************/

void PrefsSave(void)
{
	AddrPreferenceType prefs;

  MemSet(&prefs, sizeof(AddrPreferenceType), 0);

	// Write the preferences / saved-state information.
	prefs.currentCategory = CurrentCategory;

	#if WRISTPDA
	prefs.noteFont = NoteFont;
	if (prefs.noteFont > FossilLargeFont) {
		prefs.v20NoteFont = FossilStdFont;
	}
	else {
		prefs.v20NoteFont = prefs.noteFont;
	}
	prefs.noteFont = FossilNormalFontID( WRISTPDA, prefs.noteFont  );
	prefs.v20NoteFont = FossilNormalFontID( WRISTPDA, prefs.v20NoteFont );
	#else
	ErrNonFatalDisplayIf(NoteFont > largeBoldFont, "Note font invalid.");
	prefs.noteFont = NoteFont;
	if (prefs.noteFont > largeFont) {
		prefs.v20NoteFont = stdFont;
	}
	else {
		prefs.v20NoteFont = prefs.noteFont;
	}
	#endif
	//prefs.addrListFont = FossilNormalFontID( WRISTPDA, AddrListFont );
	//prefs.addrRecordFont = FossilNormalFontID( WRISTPDA, AddrRecordFont );
	//prefs.addrEditFont = FossilNormalFontID( WRISTPDA, AddrEditFont );
	prefs.addrListFont = AddrListFont;
	prefs.addrRecordFont = AddrRecordFont;
	prefs.addrEditFont = AddrEditFont;
	prefs.showAllCategories = ShowAllCategories;
	prefs.saveBackup = SaveBackup;
	prefs.rememberLastCategory = RememberLastCategory;
	prefs.businessCardRecordID = BusinessCardRecordID;
//	if (!DialerPresent)
//		prefs.enableTapDialing = true;	// If we're installing telephony components on a non-telephony ROM, then "EnableTapDialing" check box will be checked by default
//	else
		prefs.enableTapDialing = EnableTapDialing;

	// Clear reserved fields so prefs don't look "different" just from stack garbage!
	prefs.reserved1 = 0;
	prefs.reserved2 = 0;

	#if WRISTPDA

	// Write the state information to Wrist PDA private preferences.
	PrefSetAppPreferences (WPdaCreator, 'Ad', addrPrefVersionNum, &prefs,
						   sizeof (AddrPreferenceType), true);

	// If necessary, map the FossilLarge8Font and FossilLargeBold8Font font ids
	// to Large and LargeBold before writing the standard preferences.
	
	if ( prefs.addrListFont == FossilLarge8Font )
		prefs.addrListFont = largeFont;

	if ( prefs.addrListFont == FossilLargeBold8Font )
		prefs.addrListFont = largeBoldFont;

	if ( prefs.addrRecordFont == FossilLarge8Font )
		prefs.addrRecordFont = largeFont;

	if ( prefs.addrRecordFont == FossilLargeBold8Font )
		prefs.addrRecordFont = largeBoldFont;

	if ( prefs.addrEditFont == FossilLarge8Font )
		prefs.addrEditFont = largeFont;

	if ( prefs.addrEditFont == FossilLargeBold8Font )
		prefs.addrEditFont = largeBoldFont;

	#endif

	// Write the state information.
	PrefSetAppPreferences (AddressBookCreator, addrPrefID, addrPrefVersionNum, &prefs,
						   sizeof (AddrPreferenceType), true);
}


/***********************************************************************
 *
 * FUNCTION:    PrefsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List By
 *              Dialog" of the Address application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/2/95   Initial Revision
 *
 ***********************************************************************/
Boolean PrefsHandleEvent (EventType * event)
{
	Boolean handled = false;
	FormPtr frm;

	#if WRISTPDA
	if (event->eType == keyDownEvent) {
		EventType newEvent;
		FormPtr frm = FrmGetActiveForm();
		MemSet (&newEvent, sizeof(EventType), 0);
		newEvent.eType = ctlSelectEvent;
		newEvent.data.ctlSelect.on = true;
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to a Done button event.
			newEvent.data.ctlSelect.controlID = PreferencesOkButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to a Cancel button event.
			newEvent.data.ctlSelect.controlID = PreferencesCancelButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelUp ) ||
					( event->data.keyDown.chr == vchrPageUp ) ) {
			// Select first list item on RockerUp/PageUp.
			UInt16 Obj;
			frm = FrmGetActiveForm();
			Obj =  FrmGetObjectIndex( frm, PreferencesLastName );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), true );
			Obj =  FrmGetObjectIndex( frm, PreferencesCompanyName );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), false );
			return true;
		} else if ( ( event->data.keyDown.chr == vchrThumbWheelDown ) ||
					( event->data.keyDown.chr == vchrPageDown ) ) {
			// Select second list item on RockerDown/PageDown.
			UInt16 Obj;
			frm = FrmGetActiveForm();
			Obj =  FrmGetObjectIndex( frm, PreferencesLastName );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), false );
			Obj =  FrmGetObjectIndex( frm, PreferencesCompanyName );
			CtlSetValue( FrmGetObjectPtr( frm, Obj ), true );
			return true;
		}
	} else
	#endif

	if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
		case PreferencesOkButton:
			frm = FrmGetActiveForm();
			PrvPrefsSave(frm);
			ToolsLeaveForm();
			handled = true;
			break;

		case PreferencesCancelButton:
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
		PrvPrefsInit (frm);
		FrmDrawForm (frm);
		handled = true;
	}

	return (handled);
}


//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvPrefsInit
 *
 * DESCRIPTION: Initialize the dialog's ui.  Sets the database sort by
 * buttons.
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date     	 Description
 *         ----   ----       -----------
 *         roger  08/02/95   Initial Revision
 *         FPa    11/23/00   Added PreferencesEnableTapDialingHeightGadget
 *							 handling
 *
 ***********************************************************************/
void PrvPrefsInit (FormPtr frm)
{
	UInt16 objNumber;
	UInt16 rememberCategoryIndex;
	#ifndef WRISTPDA
	UInt16 enableTapDialingIndex;
	#endif

	rememberCategoryIndex = FrmGetObjectIndex (frm, PreferencesRememberCategoryCheckbox);
	CtlSetValue(FrmGetObjectPtr (frm, rememberCategoryIndex), RememberLastCategory);

	#ifndef WRISTPDA // No tap dialing on WristPDA

	enableTapDialingIndex = FrmGetObjectIndex (frm, PreferencesEnableTapDialingCheckbox);
	
	if (!ToolsIsDialerPresent())
	{
		ControlType* enableTapDialingP;
		Coord x,y;
		UInt16 numberOfObjects;
		UInt16 index;
		RectangleType rect;
		WinHandle windowH;
		UInt16 enableTapDialingHeightIndex;
		UInt16 prefsHeightDelta;
	
		enableTapDialingHeightIndex = FrmGetObjectIndex (frm, PreferencesEnableTapDialingHeightGadget);
	
		// Hide "Enable tap-dialing" control
		enableTapDialingP = FrmGetObjectPtr (frm, enableTapDialingIndex);
		CtlHideControl(enableTapDialingP);

		// Move "Remember last category" control down
		FrmGetObjectPosition(frm, enableTapDialingIndex, &x, &y);
		FrmSetObjectPosition(frm, rememberCategoryIndex, x, y);

		// Change form size
		FrmGetObjectBounds(frm, enableTapDialingHeightIndex, &rect);	// PreferencesEnableTapDialingHeightGadget goal is only to provide the number of pixel that we should used to move all components up when Dial App is not active
		prefsHeightDelta = rect.extent.y;
		windowH = FrmGetWindowHandle(frm);
		WinSetDrawWindow(windowH);
		WinGetDrawWindowBounds(&rect);
		rect.topLeft.y += prefsHeightDelta;
		rect.extent.y -= prefsHeightDelta;
		WinSetWindowBounds(windowH, &rect);


		// Move all controls up
		numberOfObjects = FrmGetNumberOfObjects(frm);
		for (index = 0 ; index < numberOfObjects ; index++)
		{
			FrmGetObjectBounds(frm, index, &rect);
			rect.topLeft.y -= prefsHeightDelta;
			FrmSetObjectBounds(frm, index, &rect);
		}
	}
	else
		// Check or uncheck "Enable Tap Dialing" checkbox
		CtlSetValue(FrmGetObjectPtr (frm, enableTapDialingIndex), EnableTapDialing);

	#endif

	// Set the current sort by setting
	if (SortByCompany)
		objNumber = PreferencesCompanyName;
	else
		objNumber = PreferencesLastName;

	CtlSetValue(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objNumber)),
				true);

}


/***********************************************************************
 *
 * FUNCTION:    PrvPrefsSave
 *
 * DESCRIPTION: Write the renamed field labels
 *
 * PARAMETERS:  frm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name		Date		Description
 *				----		----		-----------
 *				roger		8/2/95	Initial Revision
 *				jmp		11/02/99	Don't reset CurrentRecord to zero if it's been set to
 *										noRecord.  Fixes bug #23571.
 *				gap		10/27/00	actually, when the records are resorted, the current
 *										selection should always be cleared.
 *
 ***********************************************************************/
void PrvPrefsSave (FormPtr frm)
{
	FormPtr curFormP;
	FormPtr formP;
	Boolean sortByCompany;

	RememberLastCategory = CtlGetValue(FrmGetObjectPtr (frm,
														FrmGetObjectIndex (frm, PreferencesRememberCategoryCheckbox)));

	#if WRISTPDA
	EnableTapDialing = false;
	#else
	EnableTapDialing = CtlGetValue(FrmGetObjectPtr (frm,
													FrmGetObjectIndex (frm, PreferencesEnableTapDialingCheckbox)));
	#endif

	sortByCompany = CtlGetValue(FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, PreferencesCompanyName)));

	if (sortByCompany != SortByCompany)
	{
		// Put up the sorting message dialog so the user knows what's going on
		// while the sort locks up the device.
		curFormP = FrmGetActiveForm ();
		formP = FrmInitForm (SortingMessageDialog);
		FrmSetActiveForm (formP);
		FrmDrawForm (formP);

		// Peform the sort.
		SortByCompany = sortByCompany;
		AddrDBChangeSortOrder(AddrDB, SortByCompany);
		
		// clear the current selection 
		CurrentRecord = noRecord;
		TopVisibleRecord = 0;
		
		// post an event to update the form
		FrmUpdateForm (ListView, updateRedrawAll);

		// Remove the sorting message.
		FrmEraseForm (formP);
		FrmDeleteForm (formP);
		FrmSetActiveForm (curFormP);
	}
}
