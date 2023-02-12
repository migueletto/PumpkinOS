/******************************************************************************
 *
 * Copyright (c) 1996-1999 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateDisplay.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  This module contains the routines that handle the Datebook 
 *   applications's display optiions.
 *
 * History:
 *		July 17, 1996	Created by Art Lamb
 *
 *****************************************************************************/

#include <PalmOS.h>
#include "sections.h"
#include "Datebook.h"


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
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void * GetObjectPtr (UInt16 objectID)
{
	FormPtr frm;
	
	frm = FrmGetActiveForm ();
	return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));

}
#endif

/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Display
 *              Options Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/95	Initial Revision
 *
 ***********************************************************************/
static UInt16 DisplayOptionsApply (void)
{	
	UInt16 updateCode = 0;
	Boolean on;
		
	// Get the "Show Time Bars" setting.
	on = (CtlGetValue (GetObjectPtr (DisplayShowTimeBarsCheckbox)) != 0);
	if (on != ShowTimeBars)
		{
		ShowTimeBars = on;
		updateCode = updateDisplayOptsChanged;
		}		

	// Get the "Compress Day View" setting.
	on = (CtlGetValue (GetObjectPtr (DisplayCompressDayViewCheckbox)) != 0);
	if (on != CompressDayView)
		{
		CompressDayView = on;
		updateCode = updateDisplayOptsChanged;
		}		

	// Get the "Show Timed Events" setting
	on = (CtlGetValue (GetObjectPtr (DisplayShowTimedCheckbox)) != 0);
	if (on != ShowTimedAppts)
		{
		ShowTimedAppts = on;
		updateCode = updateDisplayOptsChanged;
		}		

	// Get the "Show Untimed Events" setting
	on = (CtlGetValue (GetObjectPtr (DisplayShowUntimedCheckbox)) != 0);
	if (on != ShowUntimedAppts)
		{
		ShowUntimedAppts = on;
		updateCode = updateDisplayOptsChanged;
		}		

	// Get the "Show Daily repeating Events" setting
	on = (CtlGetValue (GetObjectPtr (DisplayShowRepeatingCheckbox)) != 0);
	if (on != ShowDailyRepeatingAppts)
		{
		ShowDailyRepeatingAppts = on;
		updateCode = updateDisplayOptsChanged;
		}		

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsInit
 *
 * DESCRIPTION: This routine initializes the DisplayOptions Dialog.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/96	Initial Revision
 *
 ***********************************************************************/
static void DisplayOptionsInit (void)
{
	// Set the "Show Time Bars" setting.
	CtlSetValue (GetObjectPtr (DisplayShowTimeBarsCheckbox), ShowTimeBars);

	// Set the "Compress Day View" setting.
	CtlSetValue (GetObjectPtr (DisplayCompressDayViewCheckbox), CompressDayView);
	
	// Set the "Show Timed Events" setting
	CtlSetValue (GetObjectPtr (DisplayShowTimedCheckbox), ShowTimedAppts);
	
	// Set the "Show Untimed Events" setting
	CtlSetValue (GetObjectPtr (DisplayShowUntimedCheckbox), ShowUntimedAppts);

	// Set the "Show Daily repeating Events" setting
	CtlSetValue (GetObjectPtr (DisplayShowRepeatingCheckbox), ShowDailyRepeatingAppts);
	}


/***********************************************************************
 *
 * FUNCTION:    DisplayOptionsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Display Options
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/96	Initial Revision
 *
 ***********************************************************************/
Boolean DisplayOptionsHandleEvent (EventType * event)
{
	UInt16 updateCode;
	FormPtr frm;
	Boolean handled = false;

	#if WRISTPDA
	if	(event->eType == keyDownEvent) {
		EventType newEvent;
		MemSet (&newEvent, sizeof(EventType), 0);
		newEvent.eType = ctlSelectEvent;
		newEvent.data.ctlSelect.on = true;
		frm = FrmGetActiveForm();
		if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
			// Translate the Enter key to an Ok button event.
			newEvent.data.ctlSelect.controlID = DisplayOkButton;
			newEvent.data.ctlSelect.pControl = FrmGetObjectPtr( frm,
				FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID) );
			EvtAddEventToQueue( &newEvent );
			return true;
		} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
			// Translate the Back key to an Ok button event.
			newEvent.data.ctlSelect.controlID = DisplayCancelButton;
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
			case DisplayOkButton:
				updateCode = DisplayOptionsApply ();
				FrmReturnToForm (0);
				if (updateCode)
					FrmUpdateForm (FrmGetFormId (FrmGetActiveForm()), updateCode);
				handled = true;
				break;

			case DisplayCancelButton:
				FrmReturnToForm (0);
				handled = true;
				break;
			}
		}


	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		DisplayOptionsInit ();
		FrmDrawForm (frm);
		handled = true;
		}

	return (handled);
}
