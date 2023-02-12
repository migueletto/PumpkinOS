/******************************************************************************
 *
 * Copyright (c) 1996-2003 PalmSource, Inc. All rights reserved.
 *
 * File: Progress.h
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	  This header file defines a generic progress dialog interface
 *
 *****************************************************************************/

#ifndef	__PROGRESS_H__
#define	__PROGRESS_H__

#include <PalmTypes.h>					// Basic types
#include <CoreTraps.h>					// Trap Numbers.

#include <Form.h>
#include <Rect.h>
#include <Window.h>


#define progressMaxMessage	128
#define progressMaxTitle	31   	  // max size for title of progress dialog
#define progressMaxButtonText 7   // max size of text in OK/Cancel button

// Progress callback function
// The progress dialog will call this function to get the text to display for the
// current status. 
// stage - the current stage of progess as defined by your app
// message - text that can be sent from the protocol 
// cancel - true if the dialog is in cancel mode
// error - current error (func should return an error message in this case...
typedef struct
{
	UInt16 stage;					// <= current stage
	Char *textP;					// => buffer to hold text to display
	UInt16 textLen;				// <= length of text buffer
	Char 	*message;				// <= additional text for display
	Err error;						// <= current error
	UInt16 bitmapId;				// => resource ID of bitmap to display
	UInt16 canceled:1;			// <= true if user has pressed the cancel button			
	UInt16 showDetails:1;		// <= true if user pressed down arrow for more details
	UInt16 textChanged:1;		// => if true then update text (defaults to true)
	UInt16 timedOut:1;			// <= true if update caused by a timeout
	UInt32 timeout;				// <> timeout in ticks to force next update (for animation)
	
	//progress bar info (Not Implemented)
	UInt32 barMaxValue;			// the maximum value for the progress bar, if = 0 then the bar is
										// not visible
	UInt32 barCurValue;			// the current value of the progress bar, the bar will be drawn 
										// filled the percentage of maxValue \ value
	Char 	 *barMessage;			// additional text for display below the progress bar.
	UInt16	barFlags;			// reserved for future use.
	
	//
	// *** The following fields were added in PalmOS 3.2 ***
	//
	
	UInt16	delay:1;				// => if true delay 1 second after updating form icon/msg
	void *	userDataP;			// <= context pointer that caller passed to PrgStartDialog
	
} PrgCallbackData, *PrgCallbackDataPtr;

//typedef Boolean (*PrgCallbackFunc)  (UInt16 stage,Boolean showDetails,Char *message,Boolean cancel,UInt16 error,Char *textP, UInt16 maxtextLen,UInt16 *bitmapID);
typedef Boolean (*PrgCallbackFunc)  (PrgCallbackDataPtr cbP);



//---------------------------------------------------------------------------
// Structure of the Progress Info structure. This structure should be stored
//  in the interface's globals. Each of the routines in SerNetIFCommon.c 
//  take a pointer to this structure. 
//---------------------------------------------------------------------------


typedef struct {

#ifdef ALLOW_ACCESS_TO_INTERNALS_OF_PROGRESS
	// This field contains a pointer to the open progress dialog
	FormPtr				frmP;						// Our progress dialog ptr
	
	// This field is set a maximum time for the action to take place. A cancel
	// will be generated if this timeout is reached
	UInt32				timeout;					// max time to wait in ticks
	
	
	// This boolean is set by either the protocol (through PrgUpdateDialog()) or UI 
	//  task to inform the UI that it needs to update it's progress dialog with new 
	//  information as stored in the error, stage, and message fields. 
	UInt16				needUpdate:1;			// true if UI update required.


	// The following boolean is set by the UI task when the user hits the cancel button.
	// When the user cancels, the UI changes to display "Cancelling..." and then waits
	// for the protocol task to notice the user cancel and set the error field to
	//  netErrUserCancel before disposing the dialog. The SerIFUserCancel() which is
	//  called from the protocol task checks this boolean.
	UInt16				cancel:1;				// true if cancelling 
	
	
	// This boolean is set by PrvCheckEvents() after we've displayed an error message
	//  in the progress dialog and changed the "cancel" button to an "OK" button. 
	//  This tells the dialog event handling code in PrvCheckEvents() that it should
	//  dispose of the dialog on the next hit of the cancel/OK button.
	UInt16				waitingForOK:1;		// true if waiting for OK button hit.


	// This boolean gets set if the user hits the down button while the UI is up. It
	//  causes more detailed progress to be shown
	UInt16				showDetails:1;			// show progress details. 
	
	// This is set to true whenever the message text is changed. This allows the
	// display to be more efficient by not redrawing when not needed
	UInt16  				messageChanged: 1;
	
	
	//-----------------------------------------------------------------------
	// The following fields are set by PrgUpdateDialog() and used by PrgHandleEvent()
	//  to figure out what to display in the progress dialog
	//-----------------------------------------------------------------------
	
	// This word is set by the protocol task (through PrgUpdateDialog()) when an 
	//  error occurs during connection establishment. If this error is non-nil 
	//  and not equal to netErrUserCancel, the UI task will display the appropriate
	//  error message and change the cancel button to an OK button, set the waitingForOK
	//  boolean and wait for the user to  hit the OK button before disposing 
	//  the dialog. 
	UInt16					error;					// error set by interface

	// This enum is set by the protocol task (through PrgUpdateDialog()) as it 
	//  progresses through the  connection establishment and is checked by 
	//  PrgHandleEvent() when needUpate is true. It is used to determine what 
	//  string to display in the progress dialog.
	UInt16					stage;					// which stage of the connection we're in


	// This is an additional string that is displayed in the progress dialog for
	//  certain stages. The netConStageSending stage for example uses this string
	//  for holding the text string that it is sending. It is set by
	//  PrgUpdateDialog().
	Char						message[progressMaxMessage+1];	// connection stage message.
	
	UInt8						reserved1;
	
	// Used to cache current icon number so we don't unnecessarily redraw it
	UInt16					lastBitmapID;
	
	// Text array used to hold control title for the OK/Cancel button. This
	//  must be kept around while the control is present in case of updates.
	Char						ctlLabel[progressMaxButtonText+1];
	
	Char *					serviceNameP;

	//progress bar stuff (Not implemented)
	UInt32 lastBarMaxValue;
	UInt32 lastBarCurValue;
	
	// stuff for saving old window state
	WinHandle 		oldDrawWinH;
	WinHandle 		oldActiveWinH;
	FormPtr			oldFrmP;
	Boolean 			oldInsPtState;
	UInt8				reserved2;
	PointType 		oldInsPtPos;

	PrgCallbackFunc textCallback;

	char 				title[progressMaxTitle+1];

	//
	// *** The following field was added in PalmOS 3.2 ***
	//
	
	void 				*userDataP;
	
#else
	void*			opaque1;
	UInt32			opaque2;

	UInt16			opaque3:1;

	// The following boolean is set by the UI task when the user hits the cancel button.
	// When the user cancels, the UI changes to display "Cancelling..." and then waits
	// for the protocol task to notice the user cancel and set the error field to
	//  netErrUserCancel before disposing the dialog. The SerIFUserCancel() which is
	//  called from the protocol task checks this boolean.
	UInt16			cancel:1;				// true if cancelling
	
	UInt16			opaque4:3;

	// This word is set by the protocol task (through PrgUpdateDialog()) when an 
	//  error occurs during connection establishment. If this error is non-nil 
	//  and not equal to netErrUserCancel, the UI task will display the appropriate
	//  error message and change the cancel button to an OK button, set the waitingForOK
	//  boolean and wait for the user to  hit the OK button before disposing 
	//  the dialog. 
	UInt16			error;					// error set by interface

	// This enum is set by the protocol task (through PrgUpdateDialog()) as it 
	//  progresses through the  connection establishment and is checked by 
	//  PrgHandleEvent() when needUpate is true. It is used to determine what 
	//  string to display in the progress dialog.
	UInt16			stage;					// which stage of the connection we're in

	// This is an additional string that is displayed in the progress dialog for
	//  certain stages. The netConStageSending stage for example uses this string
	//  for holding the text string that it is sending. It is set by
	//  PrgUpdateDialog().
	Char			opaque5[progressMaxMessage+1];	// message
	
	UInt8			opaque6; // reserved1
	UInt16			opaque7; // lastBitmapID
	Char			opaque8[progressMaxButtonText+1]; // ctlLabel
	Char *			opaque9; // serviceNameP
	UInt32			opaque10; // lastBarMaxValue
	UInt32			opaque11;	// lastBarCurValue
	WinHandle 		opaque12;	// oldDrawWinH
	WinHandle 		opaque13;	// oldActiveWinH
	void*			opaque14;	// oldFrmP;
	Boolean 		opaque15;	// oldInsPtState;
	UInt8			opaque16;	// reserved2;
	PointType 		opaque17;	// oldInsPtPos;
	PrgCallbackFunc	opaque18;	// textCallback;
	char 			opaque19[progressMaxTitle+1];	// title[progressMaxTitle+1];
	void*			opaque20;	// userDataP;
#endif

	} ProgressType, *ProgressPtr;

// Warning:  In the future, the ProgressType will be opaque.  So, please don't
// write code that depends on its internals; you'll just pass it around as a
// "cookie," and that's how you should be treating it now.

// macro to test if the user has canceled
#define PrgUserCancel(prgP) (prgP)->cancel

//-----------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

ProgressPtr	PrgStartDialogV31(const Char *title, PrgCallbackFunc textCallback)
		SYS_TRAP(sysTrapPrgStartDialogV31);

ProgressPtr	PrgStartDialog(const Char *title, PrgCallbackFunc textCallback, void *userDataP)
		SYS_TRAP(sysTrapPrgStartDialog);

void PrgStopDialog(ProgressPtr prgP,Boolean force)
		SYS_TRAP(sysTrapPrgStopDialog);

void	PrgUpdateDialog(ProgressPtr prgGP, UInt16 err, UInt16 stage,
					const Char *	messageP,Boolean updateNow)
		SYS_TRAP(sysTrapPrgUpdateDialog);

Boolean	PrgHandleEvent(ProgressPtr prgGP,EventType *eventP)
		SYS_TRAP(sysTrapPrgHandleEvent);

#ifdef __cplusplus 
}
#endif

#endif //__PROGRESS_H__
