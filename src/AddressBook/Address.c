/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Address.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *   This is the Address Book application's main module.  This module
 *   starts the application, dispatches events, and stops
 *   the application.
 *
 *****************************************************************************/

#include "sec.h"

#include "Address.h"
#include "AddrList.h"
#include "AddrView.h"
#include "AddrEdit.h"
#include "AddrNote.h"
#include "AddrTools.h"
#include "AddrDetails.h"
#include "AddrCustom.h"
#include "AddrDialList.h"
#include "AddressTransfer.h"
#include "AddressLookup.h"
#include "AddrPrefs.h"
#include "AddressAutoFill.h"
#include "AddressRsc.h"

#include "SysDebug.h"

#include <Find.h>
#include <NotifyMgr.h>
#include <ErrorMgr.h>
#include <TimeMgr.h>
#include <KeyMgr.h>
#include <Menu.h>
#include <UIResources.h>
#include <SysEvtMgr.h>

#if WRISTPDA
#include <PalmOS.h>
#endif

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Time to depress the app's button to send a business card
#define AppButtonPushTimeout					(sysTicksPerSecond)


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

PUMPKIN_API;

DmOpenRef					AddrDB = NULL;
privateRecordViewEnum		PrivateRecordVisualStatus;
Char						CategoryName [dmCategoryLength];
UInt16        				TopVisibleRecord = 0;
UInt16						TopVisibleFieldIndex;
UInt16						EditFieldPosition;
UInt16          			CurrentRecord = noRecord;
UInt16           			ListViewSelectThisRecord = noRecord;		// This must
// be set whenever we leave a
// dialog because a frmSaveEvent
// happens whenever the focus is
// lost in the EditView and then
// a find and goto can happen
// causing a wrong selection to
// be used.
Boolean						SortByCompany;
UInt16						PriorAddressFormID;   // Used for NoteView

// These are used for controlling the display of the duplicated address records.
UInt16						NumCharsToHilite = 0;

UInt16						EditRowIDWhichHadFocus;
UInt16						EditLabelColumnWidth = 0;
UInt16						RecordLabelColumnWidth = 0;
Char *						UnnamedRecordStringPtr = 0;
MemHandle					UnnamedRecordStringH = 0;
Boolean						RecordNeededAfterEditView;

// The following global variable are saved to a state file.
UInt16						CurrentCategory = dmAllCategories;
Boolean						EnableTapDialing = false;	// tap dialing is not enabled by default
Boolean						ShowAllCategories = true;
Boolean						SaveBackup = true;
Boolean						RememberLastCategory = false;
#if WRISTPDA
FontID						NoteFont = FossilBoldFont;
FontID						AddrListFont = FossilBoldFont;
FontID						AddrRecordFont = FossilBoldFont;
FontID						AddrEditFont = FossilBoldFont;
#else
FontID						NoteFont = stdFont;
FontID						AddrListFont = stdFont;
FontID						AddrRecordFont = largeBoldFont;
FontID						AddrEditFont = largeBoldFont;
#endif
UInt32						BusinessCardRecordID = dmUnusedRecordID;

// For business card beaming
UInt32						TickAppButtonPushed;

// Valid after PrvAppStart
Char						PhoneLabelLetters[numPhoneLabels];

Boolean						DialerPresentChecked;
Boolean						DialerPresent;

// For business card beaming
static UInt16				AppButtonPushed = nullChr;
static UInt16				AppButtonPushedModifiers = 0;
static Boolean				BusinessCardSentForThisButtonPress = false;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static UInt32	PrvAppPilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);
static Err		PrvAppStart(void);
static void		PrvAppStop(void);
static void		PrvAppEventLoop (void);
static Boolean	PrvAppHandleKeyDown (EventType * event);
static Boolean	PrvAppHandleEvent (EventType * event);
static void		PrvAppSearch(FindParamsPtr findParams);
static void		PrvAppGoToItem (GoToParamsPtr goToParams, Boolean launchingApp);
//static void		PrvAppHandleSync(void);
static Boolean	PrvAppLaunchCmdDatabaseInit(DmOpenRef dbP);


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the Address
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * NOTE:        We need to create a branch island to PilotMain in order to
 *              successfully link this application for the device.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   7/24/95   Initial Revision
 *
 ***********************************************************************/
PUBLIC UInt32   PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return PrvAppPilotMain(cmd, cmdPBP, launchFlags);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvAppPilotMain
 *
 * DESCRIPTION: This is the main entry point for the Address Book
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *				----	----		-----------
 *				art	6/5/95	Initial Revision
 *				jmp	10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *									AddrGetDatabase().
 *				jmp	10/02/99	Made the support for the sysAppLaunchCmdExgReceiveData
 *									sysAppLaunchCmdExgAskUser launch codes more like their
 *									counterparts in Datebook, Memo, and ToDo.
 *				jmp	10/13/99	Fix bug #22832:  Call AddrGetDatabase() on look-up
 *									sublaunch to create default database if it doesn't
 *									exists (at least the user can now see that nothing
 *									exists rather than just having nothing happen).
 *				jmp	10/14/99	Oops... wasn't closing the database when we opened it
 *									in the previous change!  Fixes bug #22944.
 *				jmp	10/16/99	Just create a database on hard reset if the default
 *									database doesn't exist.
 *				jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *									since it was already being done in sysAppLaunchCmdExgReceiveData if
 *									the user affirmed sysAppLaunchCmdExgAskUser.
 *
 ***********************************************************************/
// Note: We need to create a branch island to PilotMain in order to successfully
//  link this application for the device.
UInt32 PrvAppPilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error = errNone;
	DmOpenRef dbP;


 	switch (cmd)
	{

		// The only place the following define is used is in the Jamfile for this application.
		// It's sole purpose is to provide a way to check if cross segment calls are being made
		// when application globals are unavailable.
#ifndef SINGLE_SEGMENT_CHECK
	case sysAppLaunchCmdNormalLaunch:
		error = PrvAppStart ();
		if (error)
			return (error);

		#if WRISTPDA
		FossilBackKeyModeSet( kFossilBackKeyNoAction );
		FossilExtendedFontSelectSet( true );
		#endif
		FrmGotoForm (ListView);
		PrvAppEventLoop ();
		PrvAppStop ();
		#if WRISTPDA
		FossilExtendedFontSelectSet( false );
		FossilBackKeyModeSet( kFossilBackKeyLauncher );
		#endif
		break;
#endif

	case sysAppLaunchCmdFind:
		PrvAppSearch ((FindParamsPtr) cmdPBP);
		break;


		// This action code could be sent to the app when it's already running.
	case sysAppLaunchCmdGoTo:
		{
			Boolean launched;

			launched = launchFlags & sysAppLaunchFlagNewGlobals;

#ifndef SINGLE_SEGMENT_CHECK
			if (launched)
			{
				error = PrvAppStart ();
				if (error)
					return (error);
			}
#endif

			#if WRISTPDA
			FossilBackKeyModeSet( kFossilBackKeyNoAction );
			FossilExtendedFontSelectSet( true );
			#endif
			PrvAppGoToItem ((GoToParamsPtr)cmdPBP, launched);

#ifndef SINGLE_SEGMENT_CHECK
			if (launched)
			{
				PrvAppEventLoop ();
				PrvAppStop ();
			}
			#if WRISTPDA
			FossilExtendedFontSelectSet( false );
			FossilBackKeyModeSet( kFossilBackKeyLauncher );
			#endif
		}
#endif
		break;


#if 0
	case sysAppLaunchCmdSyncNotify:
		PrvAppHandleSync();
		break;
#endif


		// Launch code sent to running app before sysAppLaunchCmdFind
		// or other action codes that will cause data PrvAppSearches or manipulation.
	case sysAppLaunchCmdSaveData:
		FrmSaveAllForms ();
		break;


		// We are requested to initialize an empty database (by sync app).
	case sysAppLaunchCmdInitDatabase:
		PrvAppLaunchCmdDatabaseInit (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
		break;


#if 0
		// This launch code is sent after the system is reset.  We use this time
		// to create our default database.  If there is no default database image,
		// then we create an empty database.
	case sysAppLaunchCmdSystemReset:
		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
		{
			error = ToolsCreateDefaultDatabase();
			// Register to receive vcf files on hard reset.
			TransferRegisterData();
		}
		ToolsRegisterLocaleChangingNotification();
		break;
#endif


		// Present the user with ui to perform a lookup and return a string
		// with information from the selected record.
	case sysAppLaunchCmdLookup:
		Lookup((AddrLookupParamsPtr) cmdPBP);
		break;


	case sysAppLaunchCmdExgAskUser:
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			AddrDBGetDatabase (&dbP, dmModeReadWrite);
		}
		else
			dbP = AddrDB;

		if (dbP != NULL)
		{
			ToolsCustomAcceptBeamDialog (dbP, (ExgAskParamPtr) cmdPBP);

			if (!(launchFlags & sysAppLaunchFlagSubCall))
				DmCloseDatabase(dbP);
		}
		break;


	case sysAppLaunchCmdExgReceiveData:
		{
	      UInt32 currentUID;

			// if our app is not active, we need to open the database
			// the subcall flag is used here since this call can be made without launching the app
			if (!(launchFlags & sysAppLaunchFlagSubCall))
			{
				error = AddrDBGetDatabase (&dbP, dmModeReadWrite);
			}
			else
			{
				dbP = AddrDB;
				
				// We don't save the current record because the keyboard dialog could have
				// stolen the table's field's handle. There's no need anyway.
				
				// TransferReceiveData() inserts the received record in sorted order. This may change the
				// index of the current record. So we remember its UID here, and refresh our copy of its
				// index afterwards.
				if (CurrentRecord != noRecord)
					DmRecordInfo(dbP, CurrentRecord, NULL, &currentUID, NULL);
			}

			if (dbP != NULL)
			{
				error = TransferReceiveData(dbP, (ExgSocketPtr) cmdPBP);

				if (launchFlags & sysAppLaunchFlagSubCall)
				{
					if (CurrentRecord != noRecord)
					{
						if (DmFindRecordByID(dbP, currentUID, &CurrentRecord) != 0)
							CurrentRecord = noRecord;	// Can't happen, but...
						
						// DOLATER dje -
						//		To fix the off-by-one error, we can decrement exgSocketP->goToParams.recordNum
						//		if it's after the current empty record in order to compensate for the
						//		current empty record getting deleted when we exit before the goto launch.
					}
				}
				else
					DmCloseDatabase(dbP);
			}
			else
				error = exgErrAppError;	// DOLATER dje - use a new error code - "try again after switching apps"
			
			// If we can't open our database, return the error since it wasn't passed to ExgDisconnect
		}
		break;

	case sysAppLaunchCmdExgPreview:
		TransferPreview((ExgPreviewInfoType *)cmdPBP);
		break;

	case sysAppLaunchCmdNotify :
		{
			if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyLocaleChangedEvent)
			{
				DmSearchStateType	PrvAppSearchState;
				LocalID	dbID;
				UInt16	cardNo;

				// Since the locale has changed, delete the existing database
				// and re-create it for the new locale
				error = DmGetNextDatabaseByTypeCreator (true, &PrvAppSearchState, addrDBType,
														AddressBookCreator, true, &cardNo, &dbID);
				if (!error)
					DmDeleteDatabase(cardNo, dbID);
				error = ToolsCreateDefaultDatabase();
			}
		}
		break;
	}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:     PrvAppStart
 *
 * DESCRIPTION:  This routine opens the application's resource file and
 *               database.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err - standard error code
 *
 * REVISION HISTORY:
 * 			Name	Date		Description
 * 			----	----		-----------
 * 			art	6/5/95	Initial Revision
 * 			vmk	12/12/97	Get amplitude for scroll sound
 *				BGT	1/8/98	Use AddressLoadPrefs to load and fix up the
 *									application preferences
 *				grant	4/6/99	Moved code to set backup bit into SetDBAttrBits.
 *				jmp	10/1/99	Call new AddrGetDataBase() to create database
 *									if it doesn't already exist.
 *
 ***********************************************************************/
Err PrvAppStart(void)
{
	Err err = 0;
	UInt16 mode;
	AddrAppInfoPtr appInfoPtr;

	#if WRISTPDA
	// Set automatic display update mode
	FossilDisplayRefreshRateSet( kFossilRefreshAuto );
	#endif

	// Determime if secret records should be shown.
	PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
		dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);


	// Find the application's data file.  If it doesn't exist create it.
	err = AddrDBGetDatabase(&AddrDB, mode);
	if (err)
		return err;


	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);
	ErrFatalDisplayIf(appInfoPtr == NULL, "Missing app info block");

	// Update the database to look and behave properly for the given country.
	if (appInfoPtr->country != PrefGetPreference(prefCountry))
		AddrDBChangeCountry(appInfoPtr);


	ToolsInitPhoneLabelLetters(appInfoPtr, PhoneLabelLetters);

	SortByCompany = appInfoPtr->misc.sortByCompany;

	// Load the application preferences and fix them up if need be.	(BGT)
	PrefsLoad(appInfoPtr);							// (BGT)

	// Initialize the default auto-fill databases
	AutoFillInitDB(titleDBType, AddressBookCreator, titleDBName, titleAFInitStr);
	AutoFillInitDB(companyDBType, AddressBookCreator, companyDBName, companyAFInitStr);
	AutoFillInitDB(cityDBType, AddressBookCreator, cityDBName, cityAFInitStr);
	AutoFillInitDB(stateDBType, AddressBookCreator, stateDBName, stateAFInitStr);
	AutoFillInitDB(countryDBType, AddressBookCreator, countryDBName, countryAFInitStr);

	// Start watching the button pressed to get into this app.  If it's held down
	// long enough then we need to send the business card.
	TickAppButtonPushed = TimGetTicks();

	// Mask off the key to avoid repeat keys causing clicking sounds
	KeySetMask(~KeyCurrentState());

	return (err);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppStop
 *
 * DESCRIPTION: This routine close the application's database
 *              and save the current state of the application.
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
void PrvAppStop(void)
{

	#if WRISTPDA
	// Set default display update mode
	FossilDisplayRefreshRateSet( kFossilRefreshDefault );
	#endif

	// Write the preferences / saved-state information.
	PrefsSave();

	// Send a frmSave event to all the open forms.
	FrmSaveAllForms ();

	// Close all the open forms.
	FrmCloseAllForms ();

	// Close the application's data file.
	DmCloseDatabase (AddrDB);

}


/***********************************************************************
 *
 * FUNCTION:    PrvAppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the Address Book
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/5/95   Initial Revision
 *
 ***********************************************************************/
void PrvAppEventLoop (void)
{
	UInt16 error;
	EventType event;

	do
	{

		#if WRISTPDA
		// For now we want to see all keys.
		KeySetMask( 0xFFFFFFFF );
		#endif

		EvtGetEvent (&event, (TickAppButtonPushed == 0) ? evtWaitForever : 2);
    //SysDebug(1, "Addr", "EventLoop event %d", event.eType);

		if (! SysHandleEvent (&event))

			if (! PrvAppHandleKeyDown (&event))

				if (! MenuHandleEvent (0, &event, &error))

					if (! PrvAppHandleEvent (&event))

						FrmDispatchEvent (&event);

#if EMULATION_LEVEL != EMULATION_NONE
		//         MemHeapCheck(0);         // Check the dynamic heap after every event
		//         MemHeapCheck(1);         // Check the first heap after every event
#endif
	}
	while (event.eType != appStopEvent);
}



/***********************************************************************
 *
 * FUNCTION:    PrvAppHandleKeyDown
 *
 * DESCRIPTION: Handle the key being down.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed on
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  10/22/97  Initial Revision
 *
 ***********************************************************************/
Boolean PrvAppHandleKeyDown (EventType * event)
{
	// Check if a button being held down is released
	if (TickAppButtonPushed != 0)
	{
		// This is the case when the button is let up
		if ((KeyCurrentState() & (keyBitHard1 | keyBitHard2 | keyBitHard3 | keyBitHard4)) == 0)
		{
			if (BusinessCardSentForThisButtonPress)
			{
				BusinessCardSentForThisButtonPress = false;

				TickAppButtonPushed = 0;

				// Allow the masked off key to now send keyDownEvents.
				KeySetMask(keyBitsAll);
			}
			else if (event->eType == nilEvent)
			{
				// Send the keyDownEvent to the app.  It was stripped out
				// before but now it can be sent over the nullEvent.  It
				// may be nullChr from when the app was launched.  In that case
				// we don't need to send the app's key because the work expected,
				// which was switching to this app, has already been done.
				if (AppButtonPushed != nullChr)
				{
					event->eType = keyDownEvent;
					event->data.keyDown.chr = AppButtonPushed;
					event->data.keyDown.modifiers = AppButtonPushedModifiers;
				}

				TickAppButtonPushed = 0;

				// Allow the masked off key to now send keyDownEvents.
				KeySetMask(keyBitsAll);
			}
		}
		// This is the case when the button is depresed long enough to send the business card
		else if (TickAppButtonPushed + AppButtonPushTimeout <= TimGetTicks() &&
				 !BusinessCardSentForThisButtonPress)
		{
			BusinessCardSentForThisButtonPress = true;
			ToolsAddrBeamBusinessCard(AddrDB);
		}
	}


	else if (event->eType == keyDownEvent)
	{
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr) &&
			!(event->data.keyDown.modifiers & autoRepeatKeyMask))
		{
			// Remember which hard key is mapped to the Address Book
			// because it may need to be sent later.
			AppButtonPushed = event->data.keyDown.chr;
			AppButtonPushedModifiers = event->data.keyDown.modifiers;

			TickAppButtonPushed = TimGetTicks();

			// Mask off the key to avoid repeat keys causing clicking sounds
			KeySetMask(~KeyCurrentState());

			// Don't process the key
			return true;
		}
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art    9/11/95   Initial Revision
 *         jmp		9/17/99   Use NewNoteView instead of NoteView.
 *		   aro		06/22/00	Add AddrListDialog
 *
 ***********************************************************************/
Boolean PrvAppHandleEvent (EventType * event)
{
	UInt16 formId;
	FormPtr frm;

	if (event->eType == frmLoadEvent)
	{

		// Load the form resource.
		formId = event->data.frmLoad.formID;
		frm = FrmInitForm (formId);
		FrmSetActiveForm (frm);
    //SysDebug(1, "Addr", "AppHandleEvent frmLoadEvent %d", formId);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
		{
		case ListView:
			FrmSetEventHandler(frm, ListHandleEvent);
			break;

		case RecordView:
			FrmSetEventHandler(frm, ViewHandleEvent);
			break;

		case EditView:
			FrmSetEventHandler(frm, EditHandleEvent);
			break;

		case NewNoteView:
			FrmSetEventHandler(frm, NoteViewHandleEvent);
			break;

		case DetailsDialog:
			FrmSetEventHandler(frm, DetailsHandleEvent);
			break;

		case CustomEditDialog:
			FrmSetEventHandler(frm, CustomEditHandleEvent);
			break;

		case PreferencesDialog:
			FrmSetEventHandler(frm, PrefsHandleEvent);
			break;

		case DialListDialog:
			FrmSetEventHandler(frm, DialListHandleEvent);
			break;

		default:
			ErrNonFatalDisplay("Invalid Form Load Event");
			break;
		}

		return (true);
	}
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppSearch
 *
 * DESCRIPTION: This routine searches the the address database for records
 *              contains the string passed.
 *
 * PARAMETERS:  findParams
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		06/05/95	art	Created by Art Lamb.
 *		10/21/99	jmp	Changed params to findParams to match other routines
 *							like this one.
 *		11/30/00	kwk	Use TxtFindString to avoid trap dispatch from FindStrInStr.
 *							Save returned match length as appCustom in FindSaveMatch.
 *
 ***********************************************************************/
void PrvAppSearch(FindParamsPtr findParams)
{
	AddrAppInfoPtr       appInfoPtr;
	AddrDBRecordType     record;
	Boolean              done;
	Boolean              match;
	char                 PrvAppSearchPhoneLabelLetters[numPhoneLabels];
	Char *               header;
	Char *               unnamedRecordStringPtr = NULL;
	MemHandle			 unnamedRecordStringH = NULL;
	DmOpenRef            dbP;
	DmSearchStateType    PrvAppSearchState;
	Err                  err;
	MemHandle            headerStringH;
	LocalID              dbID;
	RectangleType        r;
	UInt16               cardNo=0;
	UInt16               recordNum;
	MemHandle            recordH;
	UInt16               i;
	UInt32               matchPos;
	UInt16					matchLen;
	

	// Find the application's data file.
	err = DmGetNextDatabaseByTypeCreator (true, &PrvAppSearchState, addrDBType,
										  sysFileCAddress, true, &cardNo, &dbID);
	if (err)
	{
		findParams->more = false;
		return;
	}

	// Open the address database.
	dbP = DmOpenDatabase(cardNo, dbID, findParams->dbAccesMode);
	if (!dbP)
	{
		findParams->more = false;
		return;
	}

	// Display the heading line.
	headerStringH = DmGetResource(strRsc, FindAddrHeaderStr);
	header = MemHandleLock(headerStringH);
	done = FindDrawHeader (findParams, header);
	MemHandleUnlock(headerStringH);
	DmReleaseResource(headerStringH);
	if (done)
		goto Exit;

	// PrvAppSearch the description and note fields for the "find" string.
	recordNum = findParams->recordNum;
	while (true)
	{
		// Because applications can take a long time to finish a find when
		// the result may be on the screen or for other reasons, users like
		// to be able to stop the find.  Stop the find if an event is pending.
		// This stops if the user does something with the device.  Because
		// this call slows down the PrvAppSearch we perform it every so many
		// records instead of every record.  The response time should still
		// be Int16 without introducing much extra work to the PrvAppSearch.

		// Note that in the implementation below, if the next 16th record is
		// secret the check doesn't happen.  Generally this shouldn't be a
		// problem since if most of the records are secret then the PrvAppSearch
		// won't take long anyways!
		if ((recordNum & 0x000f) == 0 &&         // every 16th record
			EvtSysEventAvail(true))
		{
			// Stop the PrvAppSearch process.
			findParams->more = true;
			break;
		}

		recordH = DmQueryNextInCategory (dbP, &recordNum, dmAllCategories);

		// Have we run out of records?
		if (! recordH)
		{
			findParams->more = false;
			break;
		}

		// PrvAppSearch all the fields of the address record.
		AddrDBGetRecord (dbP, recordNum, &record, &recordH);
		match = false;
		for (i = 0; i < addrNumFields; i++)
		{
			if (record.fields[i])
			{
				match = TxtFindString(record.fields[i], findParams->strToFind, &matchPos, &matchLen);
				if (match)
				{
					break;
				}
			}
		}

		if (match)
		{
			// Add the match to the find paramter block, if there is no room to
			// display the match the following function will return true.
			done = FindSaveMatch (findParams, recordNum, matchPos, i, matchLen, cardNo, dbID);
			if (done)
			{
				MemHandleUnlock(recordH);
				break;
			}

			// Get the bounds of the region where we will draw the results.
			FindGetLineBounds (findParams, &r);

			appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
			ToolsInitPhoneLabelLetters(appInfoPtr, PrvAppSearchPhoneLabelLetters);

			// Display the title of the description.
			// Warning: WRISTPDA can not change font size because the space allocated by the OS
			//				is fixed.
			FntSetFont (stdFont);
			ToolsDrawRecordNameAndPhoneNumber (&record, &r, PrvAppSearchPhoneLabelLetters, appInfoPtr->misc.sortByCompany, &unnamedRecordStringPtr, &unnamedRecordStringH);

			MemPtrUnlock(appInfoPtr);

			findParams->lineNumber++;
		}

		MemHandleUnlock(recordH);
		recordNum++;
	}

	if ( unnamedRecordStringPtr != 0 )
		MemPtrUnlock(unnamedRecordStringPtr);

	if ( unnamedRecordStringH != 0 )
		DmReleaseResource(unnamedRecordStringH);

Exit:
	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAppGoToItem
 *
 * DESCRIPTION: This routine is a entry point of this application.
 *              It is generally call as the result of hiting of
 *              "Go to" button in the text PrvAppSearch dialog.
 *
 * PARAMETERS:  recordNum -
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		07/12/95	rsf	Created by Roger Flores.
 *		09/17/99	jmp	Use NewNoteView instead of NoteView.
 *		11/30/00	kwk	Set frmGoto.matchLen to be matchCustom, since we
 *							pass the match length returned by TxtFindString to
 *							FindSaveMatch in the appCustom parameter.
 *
 ***********************************************************************/
void PrvAppGoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	UInt16 formID;
	UInt16 recordNum;
	UInt16 attr;
	UInt32 uniqueID;
	EventType event;


	recordNum = goToParams->recordNum;
	if (!DmQueryRecord(AddrDB, recordNum))
	{
		// Record isn't accessible. This can happen when receiving a beam while in the
		// Address New form. This prevents a fatal alert, but doesn't fix the off-by-one
		// error. (See DOLATER in sysAppLaunchCmdExgReceiveData case.)
		if (!ToolsSeekRecord(&recordNum, 0, dmSeekBackward))
			if (!ToolsSeekRecord(&recordNum, 0, dmSeekForward))
			{
				FrmAlert(secGotoInvalidRecordAlert);
				FrmGotoForm(ListView);
				return;
			}
	}
	DmRecordInfo (AddrDB, recordNum, &attr, &uniqueID, NULL);

	// Change the current category if necessary.
	if (CurrentCategory != dmAllCategories)
	{
		CurrentCategory = attr & dmRecAttrCategoryMask;
	}


	// If the application is already running, close all the open forms.  If
	// the current record is blank, then it will be deleted, so we'll
	// the record's unique id to find the record index again, after all
	// the forms are closed.
	if (! launchingApp)
	{
		FrmCloseAllForms ();
		DmFindRecordByID (AddrDB, uniqueID, &recordNum);
	}


	// Set global variables that keep track of the currently record.
	CurrentRecord = recordNum;
  //SysDebug(1, "Addr", "PrvAppGoToItem set CurrentRecord=%d", CurrentRecord);

	// Set PriorAddressFormID so the Note View returns to the List View
	PriorAddressFormID = ListView;


	if (goToParams->matchFieldNum == ad_note)
		formID = NewNoteView;
	else
		formID = RecordView;

	MemSet (&event, sizeof(EventType), 0);

	// Send an event to load the form we want to goto.
	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = formID;
	EvtAddEventToQueue (&event);

	// Send an event to goto a form and select the matching text.
	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = formID;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchCustom;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);

}

#if 0
/***********************************************************************
 *
 * FUNCTION:    PrvAppHandleSync
 *
 * DESCRIPTION: MemHandle details after the database has been synchronized.
 * This app resorts the database.
 *
 * PARAMETERS:    findParams
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name   Date			Description
 *			----   ----			-----------
 *			roger	8/31/95		Initial Revision
 *			vmk	10/17/95		Changed to open db read/write
 *			jmp	10/01/99		Changed call to DmOpenDatabaseByTypeCreator() to
 *									AddrGetDatabase().
 *
 ***********************************************************************/
void PrvAppHandleSync(void)
{
	DmOpenRef dbP;
	AddrAppInfoPtr appInfoPtr;
	Err err;

	// Find the application's data file.
	err = AddrDBGetDatabase (&dbP, dmModeReadWrite);
	if (err)
		return;

	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);

	AddrDBChangeSortOrder(dbP, appInfoPtr->misc.sortByCompany);

	MemPtrUnlock(appInfoPtr);

	DmCloseDatabase (dbP);
}
#endif


/***********************************************************************
 *
 * FUNCTION:    PrvAppLaunchCmdDatabaseInit
 *
 * DESCRIPTION: Initialize an empty database.
 *
 * PARAMETERS:    dbP - pointer to database opened for read & write
 *
 * RETURNED:    true if successful
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   10/19/95   Initial Revision
 *
 ***********************************************************************/
Boolean PrvAppLaunchCmdDatabaseInit(DmOpenRef dbP)
{
	Err err;
	if (!dbP)
		return false;

	// Set the backup bit.  This is to aid syncs with non Palm software.
	ToolsSetDBAttrBits(dbP, dmHdrAttrBackup);

	// Initialize the database's app info block
	err = AddrDBAppInfoInit (dbP);
	if (err)
		return false;
	return true;
}
