/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: ToDo.c
 *
 * Release: Palm OS Developer Suite 5 SDK (68K) 4.0
 *
 * Description:
 *	  This is the ToDo application's main module.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <FntGlue.h>
#include <TraceMgr.h>

#include <PalmUtils.h>

#include "ToDo.h"
#include "ToDoDB.h"
#include "ToDoRsc.h"
#include "pumpkin_syscall.h"
//#include "debug.h"

// Error checking routines
extern void ECToDoDBValidate(DmOpenRef dbP);

// Exported routines
extern Char* GetToDoNotePtr (ToDoDBRecordPtr recordP);


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define toDoVersionNum					3
#define toDoPrefsVersionNum			3
#define todoPrefID						0x00
#define toDoDBName						"ToDoDB"
#define toDoDBType						'DATA'

// If the preferences version found is less than or equal to this
// value, we need to update the font info.
#define toDoPrefsVerUpdateFonts		2

// Fonts used by application
#define noteTitleFont					boldFont

// Columns in the ToDo table of the list view.
#define completedColumn					0
#define priorityColumn					1
#define descColumn						2
#define dueDateColumn					3
#define categoryColumn					4

#define spaceNoPriority					1
#define spaceBeforeDesc					2
#define spaceBeforeCategory			2

#define newToDoSize  					16
#define defaultPriority					1

#define maxNoteTitleLen					40

// Due Date popup list chooses
#define dueTodayItem						0
#define dueTomorrowItem					1
#define dueOneWeekLaterItem			2
#define noDueDateItem					3
#define selectDateItem					4

// Sort Order popup list choices
#define priorityDueDateItem			0
#define dueDatePriorityItem			1
#define categoryPriorityItem			2
#define categoryDueDateItem			3

// Update codes, used to determine how the ToDo list should
// be redrawn.
#define updateRedrawAll					0x00
#define updateItemDelete				0x01
#define updateItemMove					0x02
#define updateItemHide					0x04
#define updateCategoryChanged			0x08
#define updateDisplayOptsChanged		0x10
#define updateGoTo						0x20
#define updateFontChanged				0x40

// Field numbers, used to indicate where search string was found.
#define descSeacrchFieldNum			0
#define noteSeacrchFieldNum			1

#define noRecordSelected				0xffff

// Number of system ticks (1/60 seconds) to display crossed out item
// before they're erased.
#define crossOutDelay					40


/***********************************************************************
 *
 *	Internal Structutes
 *
 ***********************************************************************/

// This is the structure of the data stored in the state file.
typedef struct {
	UInt16			currentCategory;
	FontID			v20NoteFont;		// For 2.0 compatibility (BGT)
	Boolean			showAllCategories;
	Boolean 			showCompletedItems;
	Boolean 			showOnlyDueItems;
	Boolean			showDueDates;
	Boolean			showPriorities;
	Boolean			showCategories;	// added in version 2 preferences
	Boolean			saveBackup;
	Boolean			changeDueDate;		// added in version 2 preferences

	// Version 3 preferences
	FontID			listFont;
	FontID			noteFont;		// For 3.0 and later units.	(BGT)

	UInt8				reserved;
} ToDoPreferenceType;

typedef struct {
	DmOpenRef		db;
	Char *			categoryName;
	UInt16			categoryIndex;
} AcceptBeamType;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

pumpkin_system_call_t pumpkin_system_call_p;

static DmOpenRef			ToDoDB;										// ToDo database
static char					CategoryName [dmCategoryLength];		// name of the current category
static privateRecordViewEnum		CurrentRecordVisualStatus;	// applies to current record
static privateRecordViewEnum		PrivateRecordVisualStatus;	// applies to all other records
static MenuBarPtr			CurrentMenu;								// pointer to the current menu bar
static UInt16				TopVisibleRecord = 0;					// top visible record in list view
static UInt16				PendingUpdate = 0;						// code of pending list view update
static DateFormatType	DateFormat;
static DateType			Today;										// Date when the device was powered on.

// The following global variables are used to keep track of the edit
// state of the application.
static UInt16				CurrentRecord = noRecordSelected;	// record being edited
static Boolean				ItemSelected = false;					// true if a list view item is selected
static Boolean				RecordDirty = false;						// true if a record has been modified
static UInt16				ListEditPosition = 0;					// position of the insertion point in the desc field
static UInt16				ListEditSelectionLength;				// length of the current selection.

// The following global variables are saved to a state file.
static FontID				NoteFont = stdFont;						// font used in note view
static UInt16				CurrentCategory = dmAllCategories;	// currently displayed category
static Boolean				ShowAllCategories = true;				// true if all categories are being displayed
static Boolean 			ShowCompletedItems = true;				// true if completed items are being displayed
static Boolean 			ShowOnlyDueItems = false;				// true if only due items are displayed
static Boolean				ShowDueDates = false;					// true if due dates are displayed in the list view
static Boolean				ShowPriorities = true;					// true if priorities are displayed in the list view
static Boolean				ShowCategories = false;					// true if categories are displayed in the list view
static Boolean				SaveBackup = true;						// true if save backup to PC is the default
static Boolean				ChangeDueDate = false;					// true if due date is changed to completion date when completed
static FontID				ListFont = stdFont;						// font used to draw to do item

static Boolean				InPhoneLookup = false;					// true if we've called PhoneNumberLookup()

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void ListViewInit (FormPtr frm);
static void ListViewLoadTable (Boolean fillTable);
static void ListViewDrawTable (UInt16 updateCode);
static void ListViewRedrawTable (Boolean fillTable);
static Boolean ListViewUpdateDisplay (UInt16 updateCode);

static Boolean ClearEditState (void);

static void ToDoLoadPrefs(void);		// (BGT)
static void ToDoSavePrefs(void);		// (BGT)

static Boolean SeekRecord (UInt16* indexP, Int16 offset, Int16 direction);

/***********************************************************************
 *
 * FUNCTION:    ECToDoValidateAll
 *
 * DESCRIPTION: This routine preforms various edit checks on the ToDo
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
#if EMULATION_LEVEL != EMULATION_NONE

static void ECToDoValidateAll ()
{
	MemHandle recordH;


	// If a record in the list view is selected, make sure it exists.
	if (ItemSelected)
		{
		recordH = DmQueryRecord (ToDoDB, CurrentRecord);
		ErrFatalDisplayIf (!recordH, "Selected record does not exist");
		}

	else if (CurrentRecord == noRecordSelected)
		{
		// Check the in integrity of the ToDo database.
		#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
			ECToDoDBValidate (ToDoDB);
		#endif
		}

}
#endif

/***********************************************************************
 *
 * FUNCTION:     CreateDefaultDatabase
 *
 * DESCRIPTION:  This routine creates the default database from the
 *					  saved image in a resource in the application.
 *
 * PARAMETERS:   none
 *
 * RETURNED:     0 - if no error
 *					  otherwise appropriate error value
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vivek	8/17/00	Initial Revision
 *
 ***********************************************************************/
static Err CreateDefaultDatabase(void)
{
	MemHandle resH;
   DmOpenRef dbP;
	Err	error = errNone;

	// Attempt to get our default data image and create our
	// database.
	resH = DmGet1Resource(sysResTDefaultDB, sysResIDDefaultDB);
	if (resH)
		{
		error = DmCreateDatabaseFromImage(MemHandleLock(resH));

		if (!error)
			{
			MemHandleUnlock(resH);
			DmReleaseResource(resH);

			// set the backup bit on the new database
			ToDoSetDBBackupBit(NULL);
			}
		}

	// If there is no default data, or we had a problem creating it,
	// then attempt to create an empty database.
	if (!resH || error)
		{
		error = ToDoGetDatabase (&dbP, dmModeReadWrite);

		if (!error)
			DmCloseDatabase(dbP);
		}

	return error;
}

/***********************************************************************
 *
 * FUNCTION:     StartApplication
 *
 * DESCRIPTION:  This routine opens the application's database, loads the
 *               saved-state information and initializes global variables.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			grant	4/6/99	Move code to set backup bit into SetDBBackupBit
 *			jmp	10/4/99	Call ToDoGetDatabase() in place of similar in-line code.
 *
 ***********************************************************************/
static Err StartApplication (void)
{
	Err err = errNone;
	UInt16 mode;

	// Determime if secret record should be shown.
	PrivateRecordVisualStatus = CurrentRecordVisualStatus =
		(privateRecordViewEnum)PrefGetPreference(prefShowPrivateRecords);
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

	// Get the date format from the system preferences.
	DateFormat = (DateFormatType)PrefGetPreference(prefDateFormat);

	// Find the application's data file.  If it doesn't exist create it.
	err = ToDoGetDatabase(&ToDoDB, mode);
	if (err)
		return err;

	// Read the preferences.
	ToDoLoadPrefs();
	TopVisibleRecord = 0;
	ItemSelected = false;
	CurrentRecord = noRecordSelected;

	// Get today's date.  Will will use it to determine if passed due items
	// need to be redrawn when the device is powered on the next time.
	DateSecondsToDate (TimGetSeconds (), &Today);

	return err;
}


/***********************************************************************
 *
 * FUNCTION:    StopApplication
 *
 * DESCRIPTION: This routine closes the application's database
 *              and saves the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void StopApplication (void)
{
	ToDoSavePrefs();					// BGT

	// Send a frmSave event to all the open forms.
	FrmSaveAllForms ();

	// Close all the open forms.
	FrmCloseAllForms ();

	// Close the application's data file.
	DmCloseDatabase (ToDoDB);
}


/***********************************************************************
 *
 * FUNCTION:    SyncNotification
 *
 * DESCRIPTION: This routine is a entry point of the ToDo application.
 *              It is called when the ToDo application's database is
 *              synchronized.  This routine will re-sort the database and
 *              reset the state file info if necessary.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/6/95	Initial Revision
 *			jmp	10/4/99	Replaced call to DmOpenDatabaseByTypeCreator() with
 *								ToDoGetDatabase().
 *
 ***********************************************************************/
static void SyncNotification (void)
{
	Err err = errNone;
	char name [dmCategoryLength];
	DmOpenRef dbP;
	ToDoPreferenceType prefs;
	UInt16 prefsSize;

	// Open the application's data file.
	err = ToDoGetDatabase(&dbP, dmModeReadWrite);
	if (err)
		return;

	// Re-sort the database.
	ToDoSort (dbP);

	// Check if the currrent category still exists.
	prefsSize = sizeof (ToDoPreferenceType);
	if (PrefGetAppPreferences (sysFileCToDo, todoPrefID, &prefs, &prefsSize, true) != noPreferenceFound)
		{
		CategoryGetName (dbP, prefs.currentCategory, name);
		if (*name == 0)
			{
			prefs.currentCategory = dmAllCategories;
			prefs.showAllCategories = true;

			PrefSetAppPreferences (sysFileCToDo, todoPrefID, toDoVersionNum, &prefs,
				sizeof (ToDoPreferenceType), true);
			}
		}

	DmCloseDatabase (dbP);
}

/***********************************************************************
 *
 * FUNCTION:     RegisterLocaleChangingNotification

 *
 * DESCRIPTION:  Register for NotifyMgr notifications for locale chagning.
 */
 /*
 * PARAMETERS:   nothing
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			vivek	8/01/00	Initial Revision
 *
 ***********************************************************************/
static void RegisterLocaleChangingNotification(void)
{
	UInt16 cardNo;
	LocalID dbID;
	Err err;

	err = SysCurAppDatabase(&cardNo, &dbID);
	ErrNonFatalDisplayIf(err != errNone, "can't get app db info");
	if(err == errNone)
	{
		err = SysNotifyRegister(cardNo, dbID, sysNotifyLocaleChangedEvent,
						NULL, sysNotifyNormalPriority, NULL);

#if EMULATION_LEVEL == EMULATION_NONE
		ErrNonFatalDisplayIf((err != errNone) && (err != sysNotifyErrDuplicateEntry), "can't register");
#endif

	}

	return;
}


/***********************************************************************
 *
 * FUNCTION:    SearchDraw
 *
 * DESCRIPTION: This routine draws the description of a ToDo item found
 *              by the text search routine
 *
 * PARAMETERS:	 desc  - pointer to a description field
 *              x     - draw position
 *              y     - draw position
 *              width - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 * HISTORY:
 *		04/18/95	art	Created by Art Lamb.
 *		12/10/00	kwk	Use WinDrawTruncChars, versus byte-by-byte processing
 *							of text that doesn't work with Japanese.
 *
 ***********************************************************************/
static void SearchDraw (Char* desc, Int16 x, Int16 y, Int16 width)
{
	UInt16 textLen;
	const Char* lineFeedP;

	lineFeedP = StrChr(desc, chrLineFeed);
	if (lineFeedP != NULL)
	{
		textLen = lineFeedP - desc;
	}
	else
	{
		textLen = StrLen(desc);
	}

	WinDrawTruncChars(desc, textLen, x, y, width);
}


/***********************************************************************
 *
 * FUNCTION:    Search
 *
 * DESCRIPTION: This routine searchs the ToDo database for records
 *              that contain the findParams string.
 *
 * PARAMETERS:	 findParams - text search parameter block
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/18/95	Initial Revision
 *			jmp   10/21/99	Changed params to findParams to match other routines
 *								like this one.
 *
 ***********************************************************************/
static void Search (FindParamsPtr findParams)
{
	Err err;
	UInt16 pos;
	UInt16 fieldNum;
	Char* desc;
	Char* note;
	Char* header;
	UInt16 recordNum;
	MemHandle recordH;
	MemHandle headerH;
	RectangleType r;
	Boolean done;
	Boolean match;
	LocalID dbID;
	DmOpenRef dbP;
	ToDoDBRecordPtr toDoRec;
	UInt16 cardNo = 0;
	DmSearchStateType	searchState;
	UInt32 longPos;
	UInt16 matchLength;

	// Find the application's data file.
	err = DmGetNextDatabaseByTypeCreator (true, &searchState, toDoDBType,
					sysFileCToDo, true, &cardNo, &dbID);
	if (err)
		{
		findParams->more = false;
		return;
		}

	// Open the ToDo database.
	dbP = DmOpenDatabase(cardNo, dbID, findParams->dbAccesMode);
	if (!dbP)
		{
		findParams->more = false;
		return;
		}

	// Display the heading line.
	headerH = DmGetResource (strRsc, FindToDoHeaderStr);
	header = MemHandleLock (headerH);
	done = FindDrawHeader (findParams, header);
	MemHandleUnlock(headerH);
	if (done)
		goto Exit;

	// Search the description and note fields for the "find" string.
	recordNum = findParams->recordNum;
	while (true)
		{
		// Because applications can take a long time to finish a find when
		// the result may be on the screen or for other reasons, users like
		// to be able to stop the find.  Stop the find if an event is pending.
		// This stops if the user does something with the device.  Because
		// this call slows winDown the search we perform it every so many
		// records instead of every record.  The response time should still
		// be Int16 without introducing much extra work to the search.

		// Note that in the implementation below, if the next 16th record is
		// secret the check doesn't happen.  Generally, this shouldn't be a
		// problem if most of the records are secret then the search
		// won't take long.
		if ((recordNum & 0x000f) == 0 &&			// every 16th record
			EvtSysEventAvail(true))
			{
			// Stop the search process.
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

		toDoRec = MemHandleLock (recordH);

		// Search the description field,  if a match is not found search the
		// note field.
		fieldNum = descSeacrchFieldNum;
		desc = &toDoRec->description;
		match = TxtFindString (desc, findParams->strToFind, &longPos, &matchLength);
		pos = longPos;
		if (! match)
			{
			note = desc + StrLen (desc) + 1;
			if (*note)
				{
				fieldNum = noteSeacrchFieldNum;
				match = TxtFindString (note, findParams->strToFind, &longPos, &matchLength);
				pos = longPos;
				}
			}

		if (match)
			{
			// Add the match to the find paramter block,  if there is no room to
			// display the match the following function will return true.
			done = FindSaveMatch (findParams, recordNum, pos, fieldNum, matchLength, cardNo, dbID);
			if (done)
				{
				MemHandleUnlock (recordH);
				break;
				}

			// Get the bounds of the region where we will draw the results.
			FindGetLineBounds (findParams, &r);

			// Display the description.
			SearchDraw (desc, r.topLeft.x+1, r.topLeft.y, r.extent.x-2);

			findParams->lineNumber++;
			}

		MemHandleUnlock (recordH);
		recordNum++;
		}

Exit:
	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    GoToItem
 *
 * DESCRIPTION: This routine is called when the "Go to" button
 *              in the text search dialog is pressed.
 *
 * PARAMETERS:	 goToParams   - where to go to
 *              launchingApp - true is the application is being launched
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	06/06/95	Initial Revision
 *			kwk	12/03/98	Fixed param order in call to MemSet.
 *			jmp	09/17/99 Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	UInt16 formID;
	UInt16 recordNum;
	UInt16 attr;
	Int32 dateL;
	Int32 todayL;
	UInt32 uniqueID;
	MemHandle recordH;
	EventType event;
	DateTimeType today;
	ToDoDBRecordPtr toDoRec;

	recordNum = goToParams->recordNum;

	if (!DmQueryRecord(ToDoDB, recordNum))
	{

		if (!SeekRecord(&recordNum, 0, dmSeekBackward))
			if (!SeekRecord(&recordNum, 0, dmSeekForward))
			{
				FrmAlert(secGotoInvalidRecordAlert);
				FrmGotoForm(ListView);
				return;
			}
	}

	// If the application is already running, close all the open forms.  If
	// the current record is blank, then it will be deleted, so we'll get
	// the unique id of the record and use it to find the record index
	// after all the forms are closed.
	if (! launchingApp)
		{
		DmRecordInfo (ToDoDB, recordNum, NULL, &uniqueID, NULL);
		FrmCloseAllForms ();
		ClearEditState ();
		DmFindRecordByID (ToDoDB, uniqueID, &recordNum);
		}


	// Make the item the first item displayed.
	TopVisibleRecord = recordNum;

	// Change the current category if necessary.
	if (CurrentCategory != dmAllCategories)
		{
		DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
		if (CurrentCategory != (attr & dmRecAttrCategoryMask))
			{
			CurrentCategory = dmAllCategories;
			ShowAllCategories = true;
			}
		}


	// If the item is not displayable given the current display options,
	// change the display options so that it will be.
	recordH = DmQueryRecord (ToDoDB, recordNum);
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	// If only completed items are not being displayed, and the item is complete,
	// change the "show completed" option setting.
	if ((! ShowCompletedItems) && (toDoRec->priority & completeFlag))
		ShowCompletedItems = true;

	// If only due items are being show, and the item is not due,
	// change the "show only due items" option.
	if (ShowOnlyDueItems)
		if (DateToInt (toDoRec->dueDate) != toDoNoDueDate)
			{
			// Check if the item is due.
			TimSecondsToDateTime (TimGetSeconds(), &today);
			todayL = ( ((Int32) today.year) << 16) +
						( ((Int32) today.month) << 8) +
						  ((Int32) today.day);

			dateL = ( ((Int32) toDoRec->dueDate.year + firstYear) << 16) +
					  ( ((Int32) toDoRec->dueDate.month) << 8) +
						 ((Int32) toDoRec->dueDate.day);

			if (dateL > todayL)
				ShowOnlyDueItems = false;
			}

	MemHandleUnlock (recordH);



	if (goToParams->matchFieldNum == noteSeacrchFieldNum)
		formID = NewNoteView;
	else
		formID = ListView;

	// Send an event to goto a form and select the matching text.
	MemSet (&event, sizeof(EventType), 0);

	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = formID;
	EvtAddEventToQueue (&event);

	event.eType = frmGotoEvent;
	event.data.frmGoto.formID = formID;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchCustom;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	EvtAddEventToQueue (&event);
}


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
static void* GetObjectPtr (UInt16 objectID)
{
	FormPtr frm;

	frm = FrmGetActiveForm ();
	return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));

}


/***********************************************************************
 *
 * FUNCTION:    SetObjectValue
 *
 * DESCRIPTION: Assign a value to the object with the given ID
 *
 * PARAMETERS:  objectID  - id of the object to change
 *              value     - new value of the object
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
static void SetObjectValue (UInt16 objectID, Int16 value)
{
	ControlPtr ctl;

	ctl = GetObjectPtr (objectID);
	CtlSetValue (ctl, value);
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectValue
 *
 * DESCRIPTION: Return the value of the object with the given ID
 *
 * PARAMETERS:  objectID  - id of the object to change
 *
 * RETURNED:    value of the object
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
static Int16 GetObjectValue (UInt16 objectID)
{
	ControlPtr ctl;

	ctl = GetObjectPtr (objectID);
	return CtlGetValue (ctl);
}


/***********************************************************************
 *
 * FUNCTION:    ChangeCategory
 *
 * DESCRIPTION: This routine updates the global variables that keep track
 *              of category information.
 *
 * PARAMETERS:  category  - new category (index)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *
 ***********************************************************************/
static void ChangeCategory (UInt16 category)
{
	CurrentCategory = category;
	TopVisibleRecord = 0;
}


/***********************************************************************
 *
 * FUNCTION:    SelectFont
 *
 * DESCRIPTION: This routine handles selection of a font in the List
 *              View.
 *
 * PARAMETERS:  currFontID - id of current font
 *
 * RETURNED:    id of new font
 *
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/10/97	Initial Revision
 *
 ***********************************************************************/
static FontID SelectFont (FontID currFontID)
{
	UInt16 formID;
	FontID fontID;

	formID = (FrmGetFormId (FrmGetActiveForm ()));

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	if (fontID != currFontID)
		FrmUpdateForm (formID, updateFontChanged);

	return (fontID);
}


/***********************************************************************
 *
 * FUNCTION:    SeekRecord
 *
 * DESCRIPTION: Given the index of a ToDo record, this routine scans
 *              forwards or backwards for displayable ToDo records.
 *
 * PARAMETERS:  indexP  - pointer to the index of a record to start from;
 *                        the index of the record sought is returned in
 *                        this parameter.
 *
 *              offset  - number of records to skip:
 *                        	0 - seek from the current record to the
 *                             next display record, if the current record is
 *                             a display record, its index is retuned.
 *                         1 - seek foreward, skipping one displayable
 *                             record
 *                        -1 - seek backwards, skipping one displayable
 *                             record
 *
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/11/95	Initial Revision
 *
 ***********************************************************************/
static Boolean SeekRecord (UInt16* indexP, Int16 offset, Int16 direction)
{
	Int32 dateL;
	Int32 todayL;
	MemHandle recordH;
	DateTimeType today;
	ToDoDBRecordPtr toDoRec;
	Err err;

	ErrFatalDisplayIf ( (offset < -1 || offset > 1) , "Invalid offset");

	while (true)
		{
		//debug(DEBUG_INFO, "SYS", "DmSeekRecordInCategory n=%d i=%u o=%d d=%d c=%u", DmNumRecords(ToDoDB), *indexP, offset, direction, CurrentCategory);
		err = DmSeekRecordInCategory (ToDoDB, indexP, offset, direction, CurrentCategory);
		//debug(DEBUG_INFO, "SYS", "DmSeekRecordInCategory i=%u err=%d", *indexP, err);
		if (err) return (false);

		if ( ShowCompletedItems && (! ShowOnlyDueItems))
			return (true);

		recordH = DmQueryRecord (ToDoDB, *indexP);
		toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

		if ( (ShowCompletedItems) || (! (toDoRec->priority & completeFlag)))
			{
			if (! ShowOnlyDueItems) break;

			if (DateToInt (toDoRec->dueDate) == toDoNoDueDate) break;

			// Check if the item is due.
			TimSecondsToDateTime (TimGetSeconds(), &today);
			todayL = ( ((Int32) today.year) << 16) +
						( ((Int32) today.month) << 8) +
						  ((Int32) today.day);

			dateL = ( ((Int32) toDoRec->dueDate.year + firstYear) << 16) +
					  ( ((Int32) toDoRec->dueDate.month) << 8) +
						 ((Int32) toDoRec->dueDate.day);

			if (dateL <= todayL)	break;
			}

		if (offset == 0) offset = 1;

		MemHandleUnlock (recordH);
		}

	MemHandleUnlock (recordH);
	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    GetToDoNotePtr
 *
 * DESCRIPTION: This routine returns a pointer to the note field in a to
 *              do record.
 *
 * PARAMETERS:  recordP - pointer to a ToDo record
 *
 * RETURNED:    pointer to a null-terminated note
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/15/95	Initial Revision
 *
 ***********************************************************************/
Char* GetToDoNotePtr (ToDoDBRecordPtr recordP)
{
	return (&recordP->description + StrLen (&recordP->description) + 1);
}


/***********************************************************************
 *
 * FUNCTION:    DirtyRecord
 *
 * DESCRIPTION: Mark a record dirty (modified).  Record marked dirty
 *              will be synchronized.
 *
 * PARAMETERS:  index
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/15/95	Initial Revision
 *
 ***********************************************************************/
static void DirtyRecord (UInt16 index)
{
	UInt16		attr;

	DmRecordInfo (ToDoDB, index, &attr, NULL, NULL);
	attr |= dmRecAttrDirty;
	DmSetRecordInfo (ToDoDB, index, &attr, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    DeleteRecord
 *
 * DESCRIPTION: This routine deletes the selected ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the delete occurred,  false if it was canceled.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/30/95		Initial Revision
 *
 ***********************************************************************/
static Boolean DeleteRecord (UInt16 index)
{
	UInt16 ctlIndex;
	UInt16 buttonHit;
	FormPtr alert;
	Boolean saveBackup;

	// Display an alert to comfirm the operation.
	alert = FrmInitForm (DeleteToDoDialog);

	ctlIndex = FrmGetObjectIndex (alert, DeleteToDoSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);
	buttonHit = FrmDoDialog (alert);
	saveBackup = FrmGetControlValue (alert, ctlIndex);;

	FrmDeleteForm (alert);

	if (buttonHit == DeleteToDoCancel)
		return (false);

	SaveBackup = saveBackup;

	// Delete or archive the record.
	if (SaveBackup)
		DmArchiveRecord (ToDoDB, index);
	else
		DmDeleteRecord (ToDoDB, index);
	DmMoveRecord (ToDoDB, index, DmNumRecords (ToDoDB));

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    ClearEditState
 *
 * DESCRIPTION: This routine take the application out of edit mode.
 *              The edit state of the current record is remember until
 *              this routine is called.
 *
 *              If the current record is empty, it will be deleted
 *              by this routine.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true is current record is deleted by this routine.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/14/95	Initial Revision
 *
 ***********************************************************************/
static Boolean ClearEditState (void)
{
	UInt16 recordNum;
	Boolean empty;
	MemHandle recordH;
	ToDoDBRecordPtr recordP;

	if (!ItemSelected)
		{
		ItemSelected = false;
		CurrentRecord = noRecordSelected;
		return (false);
		}

	recordNum = CurrentRecord;

	// Clear the global variables that keeps track of the edit start of the
	// current record.
	ItemSelected = false;
	CurrentRecord = noRecordSelected;
	ListEditPosition = 0;
	ListEditSelectionLength = 0;
	PendingUpdate = 0;

	// If the description field is empty and the note field is empty, delete
	// the ToDo record.
	recordH = DmQueryRecord (ToDoDB, recordNum);
	recordP = MemHandleLock (recordH);
	empty = (! recordP->description) && (! *GetToDoNotePtr(recordP));
	MemHandleUnlock (recordH);

	if (empty)
		{
		// If the description was not modified, and the description and
		// note fields are empty, remove the record from the database.
		// This can occur when a new empty record is deleted.
		if (RecordDirty)
			{
			DmDeleteRecord (ToDoDB, recordNum);
			DmMoveRecord (ToDoDB, recordNum, DmNumRecords (ToDoDB));
			}
		else
			DmRemoveRecord (ToDoDB, recordNum);

		return (true);
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    DetemineDueDate
 *
 * DESCRIPTION: This routine is called when an item of the "due date"
 *              popup list is selected.  For items such as "today" and
 *              "end of week" the due date is computed,  for "select
 *              date" the date picker is displayed.
 *
 * PARAMETERS:  item selected in due date popup list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/31/95	Initial Revision
 *
 ***********************************************************************/
static void DetermineDueDate (UInt16 itemSelected, DateType * dueDateP)
{
	Int16 month, day, year;
	Int32 adjustment = 0;
	UInt32 timeInSeconds;
	Char* titleP;
	MemHandle titleH;
	DateTimeType date;

	// "No due date" items selected?
	if (itemSelected == noDueDateItem)
		{
		*((Int16*) dueDateP) = -1;
		return;
		}

	// "Select date" item selected?
	else if (itemSelected == selectDateItem)
		{
		if ( *((Int16*) dueDateP) == -1)
			{
			timeInSeconds = TimGetSeconds ();
			TimSecondsToDateTime (timeInSeconds, &date);
			year = date.year;
			month = date.month;
			day = date.day;
			}
		else
			{
			year = dueDateP->year + firstYear;
			month = dueDateP->month;
			day = dueDateP->day;
			}

		titleH = DmGetResource (strRsc, DueDateTitleStr);
		titleP = (Char*) MemHandleLock (titleH);

		if (SelectDay (selectDayByDay, &month, &day, &year, titleP))
			{
			dueDateP->day = day;
			dueDateP->month = month;
			dueDateP->year = year - firstYear;
			}

		MemHandleUnlock (titleH);
		return;
		}


	// "Today" item seleted?
	else if (itemSelected == dueTodayItem)
		adjustment = 0;

	// "Tomorrow" item selected?
	else if (itemSelected == dueTomorrowItem)
		adjustment = daysInSeconds;

	// "One week later" item selected?
	else if (itemSelected == dueOneWeekLaterItem)
		{
		adjustment = ((Int32) daysInSeconds) * ((Int32) daysInWeek);
		}

	timeInSeconds = TimGetSeconds ();
	TimSecondsToDateTime (timeInSeconds, &date);
	TimAdjust (&date, adjustment);

	dueDateP->year = date.year - firstYear;
	dueDateP->month = date.month;
	dueDateP->day = date.day;
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    OptionsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Options Dialog
 *              (aka Preferences).
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			rbb	4/14/99	Uses GetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void OptionsApply (void)
{
	UInt8 sortOrder;
	UInt16 listItem;
	Int16 val;

	// Update the sort order.  Reset the ToDo list to the top.
	listItem = LstGetSelection (GetObjectPtr (OptionsSortByList));
	switch (listItem)
		{
		case priorityDueDateItem:	sortOrder = soPriorityDueDate;	break;
		case dueDatePriorityItem:	sortOrder = soDueDatePriority;	break;
		case categoryPriorityItem:	sortOrder = soCategoryPriority;	break;
		case categoryDueDateItem:	sortOrder = soCategoryDueDate;	break;
		}


	if (ToDoGetSortOrder (ToDoDB) != sortOrder)
		{
		ToDoChangeSortOrder (ToDoDB, sortOrder);
		TopVisibleRecord = 0;
		}

	// Show or hide items marked complete.  Reset the list to the top.
	val = GetObjectValue (OptionsShowCompleted);
	if (ShowCompletedItems != val)
		{
		ShowCompletedItems = val;
		TopVisibleRecord = 0;
		}

	// Show only items due today or show all items (in the current
	// category).
	val = GetObjectValue (OptionsShowDueItems);
	if (ShowOnlyDueItems != val)
		{
		ShowOnlyDueItems = val;
		TopVisibleRecord = 0;
		}

	// Change the due date field, in the record, to the completion
	// date when the item is mark complete.
	ChangeDueDate	= GetObjectValue(OptionsChangeDueDate);

	// Show or hide the due date, priorities, and categories columns
	ShowDueDates	= GetObjectValue(OptionsShowDueDates);
	ShowPriorities	= GetObjectValue(OptionsShowPriorities);
	ShowCategories	= GetObjectValue(OptionsShowCategories);
}


/***********************************************************************
 *
 * FUNCTION:    OptionsInit
 *
 * DESCRIPTION: This routine initializes the Options Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95	Initial Revision
 *			rbb	4/14/99	Uses SetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void OptionsInit (void)
{
	UInt8 sortOrder;
	UInt16 listItem;
	Char* label;
	ListPtr lst;
	ControlPtr ctl;

	// Set the trigger and popup list that indicates the sort order.
	sortOrder = ToDoGetSortOrder (ToDoDB);

	if (sortOrder == soPriorityDueDate)
		listItem = priorityDueDateItem;
	else if (sortOrder == soDueDatePriority)
		listItem = dueDatePriorityItem;
	else if (sortOrder == soCategoryPriority)
		listItem = categoryPriorityItem;
	else
		listItem = categoryDueDateItem;

	lst = GetObjectPtr (OptionsSortByList);
	label = LstGetSelectionText (lst, listItem);
	ctl = GetObjectPtr (OptionsSortByTrigger);
	CtlSetLabel (ctl, label);
	LstSetSelection (lst, listItem);


	// Initialize the checkboxes in the dialog box.
	SetObjectValue (OptionsShowCompleted, ShowCompletedItems);
	SetObjectValue (OptionsShowDueItems, ShowOnlyDueItems);
	SetObjectValue (OptionsChangeDueDate, ChangeDueDate);
	SetObjectValue (OptionsShowDueDates, ShowDueDates);
	SetObjectValue (OptionsShowPriorities, ShowPriorities);
	SetObjectValue (OptionsShowCategories, ShowCategories);
}


/***********************************************************************
 *
 * FUNCTION:    OptionsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Options
 *              Dialog Box" of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/23/95		Initial Revision
 *
 ***********************************************************************/
static Boolean OptionsHandleEvent (EventPtr event)
{
	Boolean handled = false;
	FormPtr frm;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case OptionsOkButton:
				OptionsApply ();
				FrmReturnToForm (ListView);
				FrmUpdateForm (ListView, updateDisplayOptsChanged);
				handled = true;
				break;

			case OptionsCancelButton:
				FrmReturnToForm (ListView);
				handled = true;
				break;

			}
		}

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		OptionsInit ();
		FrmDrawForm (frm);
		handled = true;
		}

	return (handled);
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    DetailsSetDateTrigger
 *
 * DESCRIPTION: This routine sets the date trigger, in the details dialog,
 *              to the specified date.
 *
 * PARAMETERS:  year	  - years (since 1904)
 *              month  - months (1-12)
 *              day    - days (1-31)
 *
 * RETURNED:    nothing
 *
 * NOTES:
 *      This routine assumes that the memory allocated for the label of
 *      the due date trigger is large enough to hold the largest possible
 *      label.  This label memory is reserved by initializing the label
 *      in the resource file.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/31/95	Initial Revision
 *
 ***********************************************************************/
static void DetailsSetDateTrigger (DateType date)
{
	//Int16 dayOfWeek;
	Char * str;
	Char* label;
	ListPtr lst;
	ControlPtr ctl;

	lst = GetObjectPtr (DetailsDueDateList);
	LstSetSelection (lst, noDueDateItem);

	ctl = GetObjectPtr (DetailsDueDateTrigger);
	label = (Char *)CtlGetLabel (ctl);

	// Minus one means no date.  Set the label of the trigger to the
	// first choice of the popup list, which is "no date".
	if (DateToInt (date) == toDoNoDueDate)
		{
		str = LstGetSelectionText (lst, noDueDateItem);
		StrCopy (label, str);
		CtlSetLabel (ctl, label);
		LstSetSelection (lst, noDueDateItem);
		}

	// Format the date into a string and set it as the label of the trigger.
	else
		{
		// Format the date into a string.
		/*dayOfWeek =*/ DayOfWeek (date.month, date.day, date.year+firstYear);
		DateToDOWDMFormat (date.month, date.day, date.year+firstYear,
			DateFormat, label);

		CtlSetLabel (ctl, label);
		LstSetSelection (lst, selectDateItem);
		}
}


/***********************************************************************
 *
 * FUNCTION:    DetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories in the Details Dialog.
 *
 * PARAMETERS:  category - the current catagory, returns to new
 *                         category
 *
 * RETURNED:    true if the category was changed in a way that
 *              require the list view to be redrawn.
 *
 *              The following global variables are modified:
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *			mchen 12/20/00 Changed to sort after CategorySelect() returns
 *
 ***********************************************************************/
static Boolean DetailsSelectCategory (UInt16* category)
{
	Char* name;
	Boolean categoryEdited;

	name = (Char *)CtlGetLabel (GetObjectPtr (DetailsCategoryTrigger));

	categoryEdited = CategorySelect (ToDoDB, FrmGetActiveForm (),
		DetailsCategoryTrigger, DetailsCategoryList,
		false, category, name, 1, categoryDefaultEditCategoryString);

	// we can't tell if other categories have been changed so we MUST
	// resort no matter what because we use category names as one of
	// our sort fields.  CategorySelect() only returns true if the
	// current category has changed, but we care about ALL categories.
	ToDoSort(ToDoDB);

	return (categoryEdited);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsDeleteToDo
 *
 * DESCRIPTION: This routine deletes a ToDo item.  This routine is called
 *              when the delete button in the details dialog is pressed.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the record was delete or archived.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static Boolean DetailsDeleteToDo (void)
{
	UInt16 recordNum;
	Boolean empty;
	MemHandle recordH;
	ToDoDBRecordPtr recordP;

	recordNum = CurrentRecord;

	// Check if the record is empty. If it is, clear the edit state,
	// this will delete the current record if it is blank.
	recordH = DmQueryRecord (ToDoDB, recordNum);
	recordP = MemHandleLock (recordH);
	empty = (! recordP->description) && (! *GetToDoNotePtr(recordP));
	MemHandleUnlock (recordH);

	if (empty)
		{
		ClearEditState ();
		return (true);
		}

	// Display an alert to confirm the delete operation, and delete the
	// record if the alert is confirmed.
	if (!  DeleteRecord (recordNum) )
		return (false);

	ItemSelected = false;

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  category        - new catagory
 *              dueDateP        - new due date
 *              categoryEdited  - true if current category has been moved,
 *              deleted, renamed, or merged with another category
 *
 * RETURNED:    code which indicates how the ToDo list was changed,  this
 *              code is sent as the update code in a frmUpdate event.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			kcr	10/9/95	Added 'private records' alert
 *			rbb	4/14/99	Uses GetObjectValue (trimming a few bytes of code)
 *			jmp	11/15/99	Don't clear the edit state here as we could be
 *								going into NoteView, and NoteView needs to be
 *								associated with a record.  Instead, just return
 *								update code, and let ListViewUpdateDisplay() handle
 *								clearing the edit state if necessary.
 *
 ***********************************************************************/
static UInt16 DetailsApply (UInt16 category, DateType * dueDateP,
	Boolean categoryEdited)
{
	FormPtr				frm;
	UInt16				attr;
	UInt16 				index;
	UInt16 				updateCode = 0;
	UInt16 				curPriority;
	UInt16 				newPriority;
	Boolean  			secret;
	Boolean				dirty = false;
	MemHandle 			recordH;
	DateType				curDueDate;
	ToDoDBRecordPtr	toDoRec;

	// Get the secret attribute of the current record.
	DmRecordInfo (ToDoDB, CurrentRecord, &attr, NULL, NULL);

	// Get the priority setting from the dialog and compare it with the
	// priortity value in the current record.
	frm = FrmGetActiveForm ();
	index = FrmGetControlGroupSelection (frm, DetailsPrioritiesGroup);

	recordH = DmQueryRecord (ToDoDB, CurrentRecord);
	ErrFatalDisplayIf ((! recordH), "Record not found");

	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);
	curPriority = toDoRec->priority & priorityOnly;
	MemHandleUnlock (recordH);

	newPriority = FrmGetObjectId (frm, index) - DetailsPriority1Trigger + 1;

	if (curPriority != newPriority)
		{
		ToDoChangeRecord (ToDoDB, &CurrentRecord, toDoPriority, &newPriority);
		updateCode |= updateItemMove;
		dirty = true;
		}

	// Compare the due date setting in the dialog with the due date in the
	// current record.  Update the record if necessary.
	recordH = DmQueryRecord (ToDoDB, CurrentRecord);
	ErrFatalDisplayIf ((! recordH), "Record not found");

	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);
	curDueDate = toDoRec->dueDate;
	MemHandleUnlock (recordH);

	if (MemCmp (dueDateP, &curDueDate, sizeof (DateType)))
		{
		ToDoChangeRecord (ToDoDB, &CurrentRecord, toDoDueDate, dueDateP);
		updateCode |= updateItemMove;
		dirty = true;
		}

	// Compare the current category to the category setting of the dialog.
	// Update the record if the categories are different.
	if ((attr & dmRecAttrCategoryMask) != category)
		{
		ToDoChangeRecord (ToDoDB, &CurrentRecord, toDoCategory, &category);

		attr &= ~dmRecAttrCategoryMask;
		attr |= category;
		dirty = true;

		updateCode |= updateItemMove;
		}

	// Get the current setting of the secret checkbox and compare it the
	// the setting of the record.  Update the record if the values
	// are different.
	secret = GetObjectValue (DetailsSecretCheckbox);
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

	// If the current category was deleted, renamed, or merged with
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
		DmSetRecordInfo (ToDoDB, CurrentRecord, &attr, NULL);
		}

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsInit
 *
 * DESCRIPTION: This routine initializes the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			rbb	4/14/99	Uses SetObjectValue (trimming a few bytes of code)
 *
 ***********************************************************************/
static void DetailsInit (UInt16* categoryP, DateType * dueDateP)
{
	UInt16 attr;
	UInt16 category;
	UInt16 priority;
	Char* name;
	ControlPtr ctl;
	MemHandle recordH;
	ToDoDBRecordPtr	toDoRec;

	// If the record is marked secret, turn on the secret checkbox.
	DmRecordInfo (ToDoDB, CurrentRecord, &attr, NULL, NULL);
	SetObjectValue (DetailsSecretCheckbox, (attr & dmRecAttrSecret) != 0);

	// Set the label of the category trigger.
	category = attr & dmRecAttrCategoryMask;
	ctl = GetObjectPtr (DetailsCategoryTrigger);
	name = (Char *)CtlGetLabel (ctl);
	CategoryGetName (ToDoDB, category, name);
	CategorySetTriggerLabel (ctl, name);

	// Get a pointer to the ToDo record.
	recordH = DmQueryRecord (ToDoDB, CurrentRecord);
	ErrFatalDisplayIf ((! recordH), "Record not found");

	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	// Set the priority push button.
	priority = toDoRec->priority & priorityOnly;
	SetObjectValue ((DetailsPriority1Trigger + priority - 1), true);

	// Set the due date trigger.
	DetailsSetDateTrigger (toDoRec->dueDate);

	// Return the current category and due date.
	*categoryP = category;
	*dueDateP = toDoRec->dueDate;

	// Unlock the ToDo record
	MemHandleUnlock (recordH);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box" of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static Boolean DetailsHandleEvent (EventPtr event)
{
	static UInt16 		category;
	static DateType 	dueDate;
	static Boolean		categoryEdited;

	UInt16 updateCode;
	Boolean handled = false;
	FormPtr frm;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case DetailsOkButton:
				updateCode = DetailsApply (category, &dueDate, categoryEdited);
				FrmReturnToForm (ListView);
				FrmUpdateForm (ListView, updateCode);
				handled = true;
				break;

			case DetailsCancelButton:
				if (categoryEdited)
					updateCode = updateCategoryChanged;
				else
					updateCode = 0;
				FrmUpdateForm (ListView, updateCode);
				FrmReturnToForm (ListView);
				handled = true;
				break;

			case DetailsDeleteButton:
				FrmReturnToForm (ListView);
				if ( DetailsDeleteToDo ())
					FrmUpdateForm (ListView, updateItemDelete);
				else
					FrmUpdateForm (ListView, 0);
				handled = true;
				break;

			case DetailsNoteButton:
				PendingUpdate = DetailsApply (category, &dueDate, categoryEdited);
				FrmCloseAllForms ();
				FrmGotoForm (NewNoteView);
				handled = true;
				break;

			case DetailsCategoryTrigger:
				categoryEdited = DetailsSelectCategory (&category) || categoryEdited;
				handled = true;
				break;
			}
		}


	else if (event->eType == popSelectEvent)
		{
		if (event->data.popSelect.listID == DetailsDueDateList)
			{
			DetermineDueDate (event->data.popSelect.selection, &dueDate);
			DetailsSetDateTrigger (dueDate);
			handled = true;
			}
		}

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		DetailsInit (&category, &dueDate);
		FrmDrawForm (frm);
		categoryEdited = false;
		handled = true;
		}

	return (handled);
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    NoteViewDrawTitleAndForm
 *
 * DESCRIPTION: This routine draws the form and title of the note view.
 *
 * PARAMETERS:  frm, FormPtr to the form to draw
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *   		jmp   09/27/99	Square off the NoteView title so that it covers up
 *                   	the blank Form title used to trigger the menu on taps
 *                  	 	to the title area.  Also, set the NoteView title's color
 *                   	to match the standard Form title colors.  Eventually, we
 *                   	should add a variant to Forms that allows for NoteView
 *                   	titles directly.  This "fixes" bug #21610.
 *			jmp	09/29/99	Fix bug #22413:  Ensure that we peform the font metrics
 *								AFTER we have set the font!
 *       jmp   12/02/99 Fix bug #24377.  Don't call WinScreenUnlock() if WinScreenLock()
 *                      fails.
 *
 ***********************************************************************/
 static void NoteViewDrawTitleAndForm (FormPtr frm)
 {
 	Coord x;
	Coord maxWidth;
   Coord formWidth;
   RectangleType r;
	FontID curFont;
	Char* desc;
	Char* eolP;
	MemHandle recordH;
	RectangleType eraseRect, drawRect;
	ToDoDBRecordPtr recordP;
	Int16 descWidth;
	UInt16 descLen;
	UInt16 ellipsisWidth;
	IndexedColorType curForeColor;
	IndexedColorType curBackColor;
	IndexedColorType curTextColor;
	UInt8 * lockedWinP;

	// Get current record and related info.
	//
	recordH = DmQueryRecord (ToDoDB, CurrentRecord);
	ErrNonFatalDisplayIf ((! recordH), "Record not found");

	recordP = MemHandleLock (recordH);
	desc = &recordP->description;

	// "Lock" the screen so that all drawing occurs offscreen to avoid
	// the anamolies associated with drawing the Form's title then drawing
	// the NoteView title.  We REALLY need to make a variant for doing
	// this in a more official way!
	//
	lockedWinP = WinScreenLock(winLockCopy);

	FrmDrawForm(frm);

	// Perform initial set up.
	//
   FrmGetFormBounds(frm, &r);
   formWidth = r.extent.x;
	maxWidth = formWidth - 8;

	eolP = StrChr (desc, linefeedChr);
	descLen = (eolP == NULL ? StrLen (desc) : eolP - desc);
	ellipsisWidth = 0;

	RctSetRectangle (&eraseRect, 0, 0, formWidth, FntLineHeight()+4);
	RctSetRectangle (&drawRect, 0, 0, formWidth, FntLineHeight()+2);

	// Save/Set window colors and font.  Do this after FrmDrawForm() is called
	// because FrmDrawForm() leaves the fore/back colors in a state that we
	// don't want here.
	//
 	curForeColor = WinSetForeColor (UIColorGetTableEntryIndex(UIFormFrame));
 	curBackColor = WinSetBackColor (UIColorGetTableEntryIndex(UIFormFill));
 	curTextColor = WinSetTextColor (UIColorGetTableEntryIndex(UIFormFrame));
	curFont = FntSetFont (noteTitleFont);

	// Erase the Form's title area and draw the NoteView's.
	//
	WinEraseRectangle (&eraseRect, 0);
	WinDrawRectangle (&drawRect, 3);

	if (FntWidthToOffset (desc, descLen, maxWidth, NULL, &descWidth) != descLen)
		{
		ellipsisWidth = FntCharWidth (chrEllipsis);
		descLen = FntWidthToOffset (desc, descLen, maxWidth - ellipsisWidth, NULL, &descWidth);
		}

	x = (formWidth - descWidth - ellipsisWidth + 1) / 2;

	WinDrawInvertedChars (desc, descLen, x, 1);
	if (ellipsisWidth != 0)
		{
		Char buf[1];
		buf[0] = chrEllipsis;
		WinDrawInvertedChars (buf, 1, x + descWidth, 1);
		}

	// Now that we've drawn everything, blast it all back on the screen at once.
	//
   if (lockedWinP)
		WinScreenUnlock();

	// Unlock the record that we locked above.
	//
	MemHandleUnlock (recordH);

   // Restore window colors and font.
   //
   WinSetForeColor (curForeColor);
   WinSetBackColor (curBackColor);
   WinSetTextColor (curTextColor);
	FntSetFont (curFont);
 }


/***********************************************************************
 *
 * FUNCTION:    NoteViewUpdateScrollBar
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
static void NoteViewUpdateScrollBar (void)
{
	UInt16 scrollPos;
	UInt16 textHeight;
	UInt16 fieldHeight;
	Int16 maxValue;
	FieldPtr fld;
	ScrollBarPtr bar;

	fld = GetObjectPtr (NoteField);
	bar = GetObjectPtr (NoteScrollBar);

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
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewLoadRecord
 *
 * DESCRIPTION: This routine loads a note from a ToDo record into
 *              the note edit field.
 *
 * PARAMETERS:  frm - pointer to the Note View form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	9/8/99	In the Great Substitution, MemPtr became a local
 *								variable; changed MemPtr back to ptr.
 *
 ***********************************************************************/
static void NoteViewLoadRecord (void)
{
	UInt16 offset;
	FieldPtr fld;
	MemHandle recordH;
	Char * ptr;
	ToDoDBRecordPtr recordP;

	// Get a pointer to the note field.
	fld = GetObjectPtr (NoteField);

	// Set the font used in the note field.
	FldSetFont (fld, NoteFont);

	recordH = DmQueryRecord (ToDoDB, CurrentRecord);
	ErrFatalDisplayIf ((! recordH), "Bad record");

	// Compute the offset within the do to record of the note string, the
	// note string follows the description string.
	recordP = MemHandleLock (recordH);
	ptr = &recordP->description;
	ptr += StrLen (ptr) + 1;
	offset = ptr - (Char*)recordP;

	FldSetText (fld, recordH, offset, StrLen(ptr)+1);
	MemHandleUnlock (recordH);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewSave
 *
 * DESCRIPTION: This routine release any unused memory allocated for
 *              the note and mark the ToDo record dirty.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void NoteViewSave (void)
{
	FieldPtr fld;

	fld = GetObjectPtr (NoteField);

	// Was the note string modified by the user.
	if (FldDirty (fld))
		{
		// Release any free space in the note field.
		FldCompactText (fld);

		// Mark the record dirty.
		DirtyRecord (CurrentRecord);
		}


	// Clear the handle value in the field, otherwise the handle
	// will be freed when the form is disposed of,  this call also unlocks
	// the handle that contains the note string.
	FldSetTextHandle (fld, 0);

	#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
		ECToDoDBValidate (ToDoDB);
	#endif
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewDeleteNote
 *
 * DESCRIPTION: This routine deletes a note in a ToDo record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the note was deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static Boolean NoteViewDeleteNote (void)
{
	FieldPtr fld;

	if (FrmAlert(DeleteNoteAlert) != DeleteNoteYes)
		return (false);

	// Unlock the handle that contains the text of the note.
	fld = GetObjectPtr (NoteField);
	ErrFatalDisplayIf ((! fld), "Bad field");

	// Clear the handle value in the field, otherwise the handle
	// will be freed when the form is disposed of. this call also
	// unlocks the MemHandle the contains the note string.
	FldSetTextHandle (fld, 0);

	ToDoChangeRecord (ToDoDB, &CurrentRecord, toDoNote, "");

	// Mark the record dirty.
	DirtyRecord (CurrentRecord);

	return (true);
}



/***********************************************************************
 *
 * FUNCTION:    NoteViewDoCommand
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
 *			art	3/29/95	Initial Revision
 *			jmp	9/17/99	Eliminate old goto top/bottom menu items.
 *			jmp	11/04/99	To prevent other sublaunch issues, remind ourselves
 *								that we've sublaunched already into PhoneNumberLookup().
 *
 ***********************************************************************/
static Boolean NoteViewDoCommand (UInt16 command)
{
	FieldPtr fld;
	Boolean handled = true;

	switch (command)
		{
		case newNoteFontCmd:
			NoteFont = SelectFont (NoteFont);
			break;

		case newNotePhoneLookupCmd:
			fld = GetObjectPtr (NoteField);
			InPhoneLookup = true;
			PhoneNumberLookup (fld);
			InPhoneLookup = false;
			break;

		default:
			handled = false;
		}
	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewScroll
 *
 * DESCRIPTION: This routine scrolls the Note View by the specified
 *					 number of lines.
 *
 * PARAMETERS:  linesToScroll - the number of lines to scroll,
 *						positive for winDown,
 *						negative for winUp
 *					 updateScrollbar - force a scrollbar update?
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant	2/2/99	Use NoteViewUpdateScrollBar()
 *
 ***********************************************************************/
static void NoteViewScroll (Int16 linesToScroll, Boolean updateScrollbar)
{
	UInt16			blankLines;
	FieldPtr			fld;

	fld = GetObjectPtr (NoteField);
	blankLines = FldGetNumberOfBlankLines (fld);

	if (linesToScroll < 0)
		FldScrollField (fld, -linesToScroll, winUp);
	else if (linesToScroll > 0)
		FldScrollField (fld, linesToScroll, winDown);

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if ((blankLines && linesToScroll < 0) || updateScrollbar)
		{
		NoteViewUpdateScrollBar();
		}
}

/***********************************************************************
 *
 * FUNCTION:    NoteViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page winUp or winDown.
 *
 * PARAMETERS:   direction     winUp or winDown
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant	2/2/99	Use NoteViewScroll() to do actual scrolling
 *
 ***********************************************************************/
static void NoteViewPageScroll (WinDirectionType direction)
{
	UInt16 linesToScroll;
	FieldPtr fld;

	fld = GetObjectPtr (NoteField);

	if (FldScrollable (fld, direction))
		{
		linesToScroll = FldGetVisibleLines (fld) - 1;

		if (direction == winUp)
			linesToScroll = -linesToScroll;

		NoteViewScroll(linesToScroll, true);
		}
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewInit
 *
 * DESCRIPTION: This routine initializes the Note View form.
 *
 * PARAMETERS:  frm - pointer to the Note View form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/31/95	Initial Revision
 *			jmp	9/8/99	Make this routine more consistent with other
 *								built-in apps that have it.
 *			jmp	9/23/99	Eliminate code to hide old font controls since
 *								they are no longer in the NewNoteView form anyway.
 *			peter	09/20/00	Disable attention indicator because title is custom.
 *
 ***********************************************************************/
static void NoteViewInit (FormPtr frm)
{
	FieldPtr 		fld;
	FieldAttrType	attr;

	AttnIndicatorEnable(false);		// Custom title doesn't support attention indicator.
	NoteViewLoadRecord ();

	// Have the field send events to maintain the scroll bar.
	fld = GetObjectPtr (NoteField);
	FldGetAttributes (fld, &attr);
	attr.hasScrollBar = true;
	FldSetAttributes (fld, &attr);
}


/***********************************************************************
 *
 * FUNCTION:    NoteViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Note View"
 *              of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *		Name		Date	Description
 *		----		----	-----------
 *		02/21/95	art	Initial Revision
 *		10/11/95	kcr	Set initial shift state when entering an empty note field.
 *		05/16/99	kwk	Check command bit before processing special characters.
 *		9/8/99	jmp	Made this routine more consistent with the
 *							other built-in apps that have it.
 */

 /*		9/27/99	jmp	Combined NoteViewDrawTitle() & FrmUpdateForm()
 *							into a single routine that is now called
 *							NoteViewDrawTitleAndForm().
 *		09/15/00	peter	Disable attention indicator because title is custom.
 *
 ***********************************************************************/
static Boolean NoteViewHandleEvent (EventPtr event)
{
	UInt16 pos;
	FormPtr frm;
	FieldPtr fld;
	Boolean handled = false;


	if (event->eType == keyDownEvent)
		{
		if (EvtKeydownIsVirtual(event))
			{
			if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
				{
				NoteViewSave ();
				ClearEditState ();
				FrmGotoForm (ListView);
				handled = true;
				}

			else if (event->data.keyDown.chr == vchrPageUp)
				{
				NoteViewPageScroll (winUp);
				handled = true;
				}

			else if (event->data.keyDown.chr == vchrPageDown)
				{
				NoteViewPageScroll (winDown);
				handled = true;
				}
			}
		}

	else if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case NoteDoneButton:
				NoteViewSave ();
				FrmGotoForm (ListView);
				handled = true;
				break;

			case NoteDeleteButton:
				if (NoteViewDeleteNote())
					FrmGotoForm (ListView);
				handled = true;
				break;
			}
		}


	else if (event->eType == fldChangedEvent)
		{
		frm = FrmGetActiveForm ();
		NoteViewUpdateScrollBar ();
		handled = true;
		}


	else if (event->eType == menuEvent)
		{
		handled = NoteViewDoCommand (event->data.menu.itemID);
		}


	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		NoteViewInit (frm);
		NoteViewDrawTitleAndForm (frm);
		NoteViewUpdateScrollBar ();
		FrmSetFocus (frm, FrmGetObjectIndex (frm, NoteField));
		handled = true;
		}


	else if (event->eType == frmGotoEvent)
		{
		frm = FrmGetActiveForm ();

		ItemSelected = true;
		CurrentRecord = event->data.frmGoto.recordNum;
		NoteViewInit (frm);

		fld = GetObjectPtr (NoteField);
		pos = event->data.frmGoto.matchPos;
		FldSetScrollPosition (fld, pos);
		FldSetSelection (fld, pos, pos + event->data.frmGoto.matchLen);
		NoteViewDrawTitleAndForm (frm);
		NoteViewUpdateScrollBar ();
		FrmSetFocus (frm, FrmGetObjectIndex (frm, NoteField));
		handled = true;
		}

	else if (event->eType == frmUpdateEvent)
		{
		if (event->data.frmUpdate.updateCode & updateFontChanged)
			{
			fld = GetObjectPtr (NoteField);
			FldSetFont (fld, NoteFont);
			NoteViewUpdateScrollBar ();
			}
		else
			{
			frm = FrmGetActiveForm ();
			NoteViewDrawTitleAndForm (frm);
			}
		handled = true;
		}


	else if (event->eType == frmCloseEvent)
		{
		AttnIndicatorEnable(true);		// Custom title doesn't support attention indicator.
		if ( FldGetTextHandle (GetObjectPtr (NoteField)))
			NoteViewSave ();
		}


	else if (event->eType == sclRepeatEvent)
		{
		NoteViewScroll (event->data.sclRepeat.newValue -
			event->data.sclRepeat.value, false);
		}

	return (handled);
}


//#pragma mark ----------------
/***********************************************************************
 *
 * FUNCTION:    ListViewRestoreEditState
 *
 * DESCRIPTION: This routine restores the edit state of the ToDo list
 *              if the list is in edit mode. This routine is
 *              called after the priority or due date of an item is
 *              changed, or after returning from the details dialog
 *              or note view.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/17/95	Initial Revision
 *			kwk	12/10/98	Constrain field selection values to mask app bug.
 *
 ***********************************************************************/
static void ListViewRestoreEditState ()
{
	Int16 row;
	FormPtr frm;
	TablePtr table;
	FieldPtr fld;
	UInt16 fldLen;

	if (!ItemSelected) return;

	// Find the row that the current record is in.  Its possible
	// that the current record is no longer displayable (ex: only due
	// item are being display and the due date was changed
	// such that the record don't display).
	table = GetObjectPtr (ListTable);
	if (!TblFindRowID (table, CurrentRecord, &row) || TblRowMasked(table,row))
		{
		ClearEditState ();
		return;
		}

	frm = FrmGetActiveForm();
	FrmSetFocus (frm, FrmGetObjectIndex (frm, ListTable));
	TblGrabFocus (table, row, descColumn);

	// Restore the insertion point position.
	fld = TblGetCurrentField (table);

	// Make sure saved position/length are valid for the curren field.


	fldLen = FldGetTextLength (fld);
	if (ListEditPosition > fldLen)
		ListEditPosition = fldLen;

	if (ListEditPosition + ListEditSelectionLength > fldLen)
		ListEditSelectionLength = fldLen - ListEditPosition;

	FldSetInsPtPosition (fld, ListEditPosition);
	if (ListEditSelectionLength)
		FldSetSelection (fld, ListEditPosition,
			ListEditPosition + ListEditSelectionLength);

	FldGrabFocus (fld);
}



/***********************************************************************
 *
 * FUNCTION:    ListViewClearEditState
 *
 * DESCRIPTION: This routine clears the edit state of the ToDo list.
 *              It is called whenever a table item is selected.
 *
 *              If the new item selected is in a different row than
 *              the current record, the edit state is cleared,  and if
 *              current record is empty it is deleted.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the current record is deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/17/95	Initial Revision
 *			peter	4/24/00	Re-mask private record when leaving it.
 *
 ***********************************************************************/
static Boolean ListViewClearEditState (void)
{
	Int16 row;
	Int16 rowsInTable;
	FormPtr frm;
	TablePtr table;
	FieldPtr field;
	UInt16 attr;

	if (!ItemSelected) return (false);

	frm = FrmGetFormPtr (ListView);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ListTable));
	if (!TblFindRowID (table, CurrentRecord, &row) )
		return (false);

	// Check if the keyboard dialog stole the table's field's handle. If it did,
	// don't delete the record yet. We'll get another chance when the app quits
	// to goto the received record.
	field = TblGetCurrentField(table);
	if (field && !FldGetTextHandle(field))
		return (false);

	TblReleaseFocus (table);

	// We're leaving a record. If it's secret and we're masking secret records
	// (but unmasked just this one), then remask it now.
	if (CurrentRecordVisualStatus != PrivateRecordVisualStatus)
	{
		CurrentRecordVisualStatus = PrivateRecordVisualStatus;

		// Is the record still secret? It may have been changed from the
		// details dialog.
		DmRecordInfo (ToDoDB, CurrentRecord, &attr, NULL, NULL);

		if (attr & dmRecAttrSecret)
		{
			// Re-mask the current row.
			TblSetRowMasked(table, row, true);

			// Draw the row masked.
			TblMarkRowInvalid (table, row);
			TblRedrawTable(table);
		}
	}

	// If a different row has been selected, clear the edit state, this
	// will delete the current record if it's empty.
	if (ClearEditState ())
		{
		rowsInTable = TblGetNumberOfRows (table);
		for (; row < rowsInTable; row++)
			TblSetRowUsable (table, row, false);

		ListViewRedrawTable	(true);

		return (true);
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGetDescription
 *
 * DESCRIPTION: This routine returns a pointer to the description field
 *              of a ToDo record.  This routine is called by the table
 *              object as a callback routine when it wants to display or
 *              edit a ToDo description.
 *
 * PARAMETERS:  table  - pointer to the ToDo list table (TablePtr)
 *              row    - row of the table
 *              column - column of the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static Err ListViewGetDescription (void * table, Int16 row, UInt16 UNUSED_PARAM(column),
	Boolean UNUSED_PARAM(editable), MemHandle *textHP, UInt16 * textOffset, UInt16 * textAllocSize,
	FieldPtr fld)
{
	UInt16 recordNum;
	MemHandle recordH;
	FieldAttrType attr;
	ToDoDBRecordPtr toDoRec;

	// Get the record number that corresponds to the table item.
	// The record number is stored as the row id.
	recordNum = TblGetRowID (table, row);
	recordH = DmQueryRecord( ToDoDB, recordNum);
	ErrFatalDisplayIf ((! recordH), "Record not found");

	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	*textOffset = &toDoRec->description - ((Char *) toDoRec);
	*textAllocSize = StrLen (&toDoRec->description) + 1;  // one for null terminator
	*textHP = recordH;

	MemHandleUnlock (recordH);

	// Set the field to support auto-shift.
	if (fld)
		{
		FldGetAttributes (fld, &attr);
		attr.autoShift = true;
		FldSetAttributes (fld, &attr);
		}


	return (0);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSaveDescription
 *
 * DESCRIPTION: This routine is called by the table object, as a callback
 *              routine, when it wants to save a ToDo description.
 *              The description is edit in place (directly in the database
 *              record),  so we don't need to save it here,  we do however
 *              want to capture the current edit state.
 *
 * PARAMETERS:  table  - pointer to the memo list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *
 * RETURNED:    true if the table needs to be redrawn
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static Boolean ListViewSaveDescription (void * table, Int16 row, UInt16 UNUSED_PARAM(column))
{
	UInt16 recordNum;
	UInt16 selectStart;
	UInt16 selectEnd;
	Boolean dirty;
	FieldPtr fld;


	// Get the record number that corresponds to the table item to save.
	recordNum = TblGetRowID (table, row);

	// If the description has been modified mark the record dirty, any
	// change make to the ToDo's description were written directly
	// to the ToDo record.
	fld = TblGetCurrentField (table);
	dirty = FldDirty (fld);
	if (dirty)
		DirtyRecord (recordNum);

	// Save the dirty state, we're need it if we auto-delete an empty record.
	RecordDirty = dirty;

	// Check if the top of the description is scroll off the top of the
	// field, if it is then redraw the field.
	if (FldGetScrollPosition (fld))
		{
		FldSetSelection (fld, 0, 0);
		FldSetScrollPosition (fld, 0);
		ListEditPosition = 0;
		ListEditSelectionLength = 0;
		}

	// Save the insertion point position, and length of the selection.
	// We'll need the insertion point position an selection length
	// if we put the table back into edit mode.
	else
		{
		ListEditPosition = FldGetInsPtPosition (fld);

		FldGetSelection (fld, &selectStart, &selectEnd);
		ListEditSelectionLength = selectEnd - selectStart;
		if (ListEditSelectionLength)
			ListEditPosition = selectStart;
		}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGetDescriptionHeight
 *
 * DESCRIPTION: This routine returns the height, in pixels, of a ToDo
 *              description.
 *
 * PARAMETERS:  recordNum - record index
 *              width     - width of description
 *
 * RETURNED:    height in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	10/27/99	Restore current font in masked-records case; fixes
 *								bug #23276.
 *			peter	4/24/00	Allow multi-line masked records.
 *
 ***********************************************************************/
static UInt16 ListViewGetDescriptionHeight (UInt16 recordNum, UInt16 width, UInt16 maxHeight)
{
	UInt16 height;
	UInt16 lineHeight;
	Char* note;
	FontID curFont;
	MemHandle recordH;
	ToDoDBRecordPtr toDoRec;
	//Boolean masked;
	UInt16 attr;
	//privateRecordViewEnum visualStatus;

	//mask if appropriate
	DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
	//visualStatus = recordNum == CurrentRecord
		//? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
   //masked = (((attr & dmRecAttrSecret) && visualStatus == maskPrivateRecords));

	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();

	// The following code is commented out since masked records are no longer limited
	// to one line. The reason for this is to keep masking and unmasking of individual
	// records from affecting the position of records on the screen.
	//		if (masked)
	//			{
	//			FntSetFont (curFont);
	//			return lineHeight;
	//			}

	// Get a pointer to the ToDo record.
	recordH = DmQueryRecord( ToDoDB, recordNum);
	ErrFatalDisplayIf ((! recordH), "Record not found");

	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	// If the record has a note, leave space for the note indicator.
	note = GetToDoNotePtr (toDoRec);
	if (*note)
		width -= tableNoteIndicatorWidth;

	// Compute the height of the ToDo item's description.

	height = FldCalcFieldHeight (&toDoRec->description, width);
	height = min (height, (maxHeight / lineHeight));
	height *= lineHeight;

	FntSetFont (curFont);

	MemHandleUnlock (recordH);

	return (height);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewPriorityFontID
 *
 * DESCRIPTION: This routine is called to determine the correct font to
 *						use for drawing the list view priority number - we
 *						want to bold the list view font.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    Font id for list view priority number.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			kwk	06/23/99	New today.
 *
 ***********************************************************************/
static FontID ListViewPriorityFontID (void)
{
	if (ListFont == stdFont)
		return (boldFont);
	else if (ListFont == largeFont)
		return (largeBoldFont);
	else
		return (ListFont);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewDrawDueDate
 *
 * DESCRIPTION: This routine draws a ToDo items due date.
 *
 * PARAMETERS:	 table  - pointer to a table object
 *              row    - row the item is in
 *              column - column the item is in
 *              bounds - region to draw in
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/14/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewDrawDueDate (void * table, Int16 row, Int16 UNUSED_PARAM(column),
	RectanglePtr bounds)
{
	char dueChr;
	char dateBuffer [dateStringLength];
	Char* dateStr;
	UInt16 dateStrLen;
	UInt16 dueChrWidth;
	Int16 drawX, drawY;
	FontID curFont;
	FontID fontID;
	DateType date;
	DateTimeType today;
	Int32 todayL, dateL;


	// Get the due date to the item being drawn.
	*((Int16 *) (&date)) = TblGetItemInt (table, row, dueDateColumn);


	// If there is no date draw a dash to indicate such.
	if (DateToInt (date) == toDoNoDueDate)
		{
		curFont = FntSetFont (stdFont);
		drawX = bounds->topLeft.x + ((bounds->extent.x - 5) >> 1);
		drawY = bounds->topLeft.y + ((FntLineHeight () + 1) / 2);
		WinDrawLine (drawX, drawY, drawX+5, drawY);
		FntSetFont (curFont);
		return;
		}

	// Get the width of the character that indicates the item is due.  Don't
	// count the whitespace in the character.
	fontID = ListViewPriorityFontID();
	curFont = FntSetFont (fontID);
	dueChr = '!';
	dueChrWidth = FntCharWidth (dueChr) - 1;

	FntSetFont (ListFont);

	DateToAscii (date.month, date.day, date.year + firstYear,
					DateFormat, dateBuffer);

	// Remove the year from the date string.
	dateStr = dateBuffer;
	if ((DateFormat == dfYMDWithSlashes) ||
		 (DateFormat == dfYMDWithDots) ||
		 (DateFormat == dfYMDWithDashes))
		dateStr += 3;
	else
		{
		dateStr[StrLen(dateStr) - 3] = 0;
		}


	// Draw the due date, right aligned.
	dateStrLen = StrLen (dateStr);
	drawX = bounds->topLeft.x + bounds->extent.x - dueChrWidth -
		FntCharsWidth (dateStr, dateStrLen);
	drawY = bounds->topLeft.y ;
	WinDrawChars (dateStr, dateStrLen, drawX, drawY);


	// If the date is on or before today draw an exclamation mark.
	TimSecondsToDateTime (TimGetSeconds(), &today);

	todayL = ( ((Int32) today.year) << 16) +
				( ((Int32) today.month) << 8) +
				  ((Int32) today.day);

	dateL = ( ((Int32) date.year + firstYear) << 16) +
			  ( ((Int32) date.month) << 8) +
				 ((Int32) date.day);

	if (dateL < todayL)
		{
		drawX = bounds->topLeft.x + bounds->extent.x - dueChrWidth;
		FntSetFont (fontID);
		WinDrawChars (&dueChr, 1, drawX, drawY);
		}

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawCategory
 *
 * DESCRIPTION: This routine draws a ToDo item's category name.
 *
 * PARAMETERS:	 table  - pointer to a table object
 *              row    - row the item is in
 *              column - column the item is in
 *              bounds - region to draw in
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/21/96	Initial Revision
 *
 ***********************************************************************/
static void ListViewDrawCategory (void * table, Int16 row, Int16 UNUSED_PARAM(column),
	RectanglePtr bounds)
{
	Int16 width;
	Int16 length;
	UInt16 attr;
	UInt16 category;
	UInt16 recordNum;
	Boolean fits;
	Char categoryName [dmCategoryLength];
	FontID curFont;

	curFont = FntSetFont (ListFont);

	// Get the category of the item in the specified row.
	recordNum = TblGetRowID (table, row);
	DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;

	// Get the name of the category and trunctae it to fix the the
	// column passed.
	CategoryGetName (ToDoDB, category, categoryName);
	width = bounds->extent.x;
	length = StrLen(categoryName);
	FntCharsInWidth (categoryName, &width, &length, &fits);

	// Draw the category name.
	WinDrawChars (categoryName, length, bounds->topLeft.x,
		bounds->topLeft.y);

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the list view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frm             - pointer to the ToDo list form
 *              bottomRecord    - record index of the last visible record
 *              lastItemClipped - true if the last item display is not fully
 *                                visible
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewUpdateScrollers (FormPtr frm, UInt16 bottomRecord,
	Boolean lastItemClipped)
{
	UInt16 upIndex;
	UInt16 downIndex;
	UInt16 recordNum;
	Boolean scrollableUp;
	Boolean scrollableDown;

	// If the first record displayed is not the first record in the category,
	// enable the winUp scroller.
	recordNum = TopVisibleRecord;
	scrollableUp = SeekRecord (&recordNum, 1, dmSeekBackward);


	// If the last record displayed is not the last record in the category,
	// or the list item is clipped, enable the winDown scroller.
	recordNum = bottomRecord;
	scrollableDown = SeekRecord (&recordNum, 1, dmSeekForward) || lastItemClipped;


	// Update the scroll button.
	upIndex = FrmGetObjectIndex (frm, ListUpButton);
	downIndex = FrmGetObjectIndex (frm, ListDownButton);
	FrmUpdateScrollers (frm, upIndex, downIndex, scrollableUp, scrollableDown);
}


/***********************************************************************
 *
 * FUNCTION:    ListInitTableRow
 *
 * DESCRIPTION: This routine initialize a row in the ToDo list.
 *
 * PARAMETERS:  table      - pointer to the table of ToDo items
 *              row        - row number (first row is zero)
 *              recordNum  - the index of the record display in the row
 *              rowHeight  - height of the row in pixels
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListInitTableRow (TablePtr table, Int16 row, UInt16 recordNum,
	Int16 rowHeight)
{
	Char* note;
	UInt32	uniqueID;
	MemHandle recordH;
	ToDoDBRecordPtr toDoRec;

	// Get a pointer to the ToDo record.
	recordH = DmQueryRecord( ToDoDB, recordNum);
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	// Make the row usable.
	TblSetRowUsable (table, row, true);

	// Set the height of the row to the height of the description.
	TblSetRowHeight (table, row, rowHeight);

	// Store the record number as the row id.
	TblSetRowID (table, row, recordNum);

	// Store the unique id of the record in the table.
	DmRecordInfo (ToDoDB, recordNum, NULL, &uniqueID, NULL);
	TblSetRowData (table, row, uniqueID);

	// Set the checkbox that indicates the completion status.
	TblSetItemInt (table, row, completedColumn,
		(toDoRec->priority & completeFlag) == completeFlag);

	// Store the priority in the table.
	TblSetItemInt (table, row, priorityColumn,
		toDoRec->priority & priorityOnly);

	// Store the due date in the table.
	TblSetItemInt (table, row, dueDateColumn, (*(Int16 *) &toDoRec->dueDate));

	// Set the table item type for the description, it will differ depending
	// on the presents of a note.
	note = GetToDoNotePtr (toDoRec);
	if (*note)
		TblSetItemStyle (table, row, descColumn, textWithNoteTableItem);
	else
		TblSetItemStyle (table, row, descColumn, textTableItem);


	// Mark the row invalid so that it will drawn when we call the
	// draw routine.
	TblMarkRowInvalid (table, row);

	MemHandleUnlock (recordH);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadTable
 *
 * DESCRIPTION: This routine reloads ToDo database records into
 *              the list view.  This routine is called when:
 *              	o A new item is inserted
 *              	o An item is deleted
 *              	o The priority or due date of an items is changed
 *              	o An item is marked complete
 *              	o Hidden items are shown
 *              	o Completed items are hidden
 *
 * PARAMETERS:  fillTable - if true the top visible item will be scroll winDown
 *                          such that a full table is displayed
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
  *		jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *			peter	4/24/00	Add support for unmasking only the selected record.
 *
 ***********************************************************************/
static void ListViewLoadTable (Boolean fillTable)
{
	UInt16			row;
	UInt16			numRows;
	UInt16			recordNum;
	UInt16			lastRecordNum;
	UInt16			dataHeight;
	UInt16			lineHeight;
	UInt16			tableHeight;
	UInt16			columnWidth;
	UInt16			pos, oldPos;
	UInt16			height, oldHeight;
	UInt32			uniqueID;
	FontID			curFont;
	Boolean			rowUsable;
	Boolean			rowsInserted = false;
	Boolean			lastItemClipped;
	FormPtr			frm;
	TablePtr			table;
	RectangleType	r;
	UInt16 			attr;
	Boolean 			masked;
	privateRecordViewEnum visualStatus;

	frm = FrmGetFormPtr (ListView);

	// Make sure the global variable that holds the index of the
	// first visible record has a valid value.
	if (! SeekRecord (&TopVisibleRecord, 0, dmSeekForward))
		if (! SeekRecord (&TopVisibleRecord, 0, dmSeekBackward))
			TopVisibleRecord = 0;

	// If we have a currently selected record, make sure that it is not
	// above the first visible record.
	if (CurrentRecord != noRecordSelected)
		if (CurrentRecord < TopVisibleRecord) {
			CurrentRecord = TopVisibleRecord;
		}


	// Get the height of the table and the width of the description
	// column.
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ListTable));
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	columnWidth = TblGetColumnWidth (table, descColumn);

	// Get the height one one line.
	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();
	FntSetFont (curFont);

	row = 0;
	dataHeight = 0;
	oldPos = pos = 0;
	recordNum = TopVisibleRecord;
	lastRecordNum = recordNum;

	// Load records into the table.
	while (true)
		{
		// Get the next record in the currunt category.
		if ( ! SeekRecord (&recordNum, 0, dmSeekForward))
			break;

		// Compute the height of the ToDo item's description.
		height = ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);

		// Is there enought room for at least one line of the the decription.
		if (tableHeight >= dataHeight + lineHeight)
			{
			// Get the height of the current row.
			rowUsable = TblRowUsable (table, row);
			if (rowUsable)
				oldHeight = TblGetRowHeight (table, row);
			else
				oldHeight = 0;

			DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
			visualStatus = recordNum == CurrentRecord
				? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
	   	masked = (((attr & dmRecAttrSecret) && visualStatus == maskPrivateRecords));

			if (masked != TblRowMasked (table, row))
				TblMarkRowInvalid (table, row);

			TblSetRowMasked (table, row, masked);

			// Determine if the row needs to be initialized.  We will initialize
			// the row if: the row is not usable (not displayed),  the unique
			// id of the record does not match the unique id stored in the
			// row.
			DmRecordInfo (ToDoDB, recordNum, NULL, &uniqueID, NULL);
			if ((TblGetRowData (table, row) != uniqueID) ||
				 (! TblRowUsable (table, row)) ||
				 (TblRowInvalid (table, row)))
				{
				ListInitTableRow (table, row, recordNum, height);
				}

			// If the height or the position of the item has changed draw the item.
			else
				{
				TblSetRowID (table, row, recordNum);
				if (height != oldHeight)
					{
					TblSetRowHeight (table, row, height);
					TblMarkRowInvalid (table, row);
					}
				else if (pos != oldPos)
					{
					TblMarkRowInvalid (table, row);
					}
				}

			pos += height;
			oldPos += oldHeight;

			lastRecordNum = recordNum;
			row++;
			recordNum++;
			}

		dataHeight += height;

		// Is the table full?
		if (dataHeight >= tableHeight)
			{
			// If we have a currently selected record, make sure that it is
			// not below  the last visible record.
			if ((CurrentRecord == noRecordSelected) ||
				 (CurrentRecord <= lastRecordNum)) break;

			TopVisibleRecord = recordNum;
			row = 0;
			dataHeight = 0;
			}
		}

	// Hide the items that don't have any data.
	numRows = TblGetNumberOfRows (table);
	while (row < numRows)
		{
		TblSetRowUsable (table, row, false);
		row++;
		}

	// If the table is not full and the first visible record is
	// not the first record	in the database, displays enough records
	// to fill out the table.
	while (dataHeight < tableHeight)
		{
		if (! fillTable)
			break;

		recordNum = TopVisibleRecord;
		if ( ! SeekRecord (&recordNum, 1, dmSeekBackward))
			break;

		// Compute the height of the ToDo item's description.
		height = ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);

		// If adding the item to the table will overflow the height of
		// the table, don't add the item.
		if (dataHeight + height > tableHeight)
			break;

		// Insert a row before the first row.
		TblInsertRow (table, 0);

		ListInitTableRow (table, 0, recordNum, height);
		//mask if appropriate
		DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
		visualStatus = recordNum == CurrentRecord
			? CurrentRecordVisualStatus : PrivateRecordVisualStatus;
   	masked = (((attr & dmRecAttrSecret) && visualStatus == maskPrivateRecords));
		TblSetRowMasked(table,0,masked);

		TopVisibleRecord = recordNum;

		rowsInserted = true;

		dataHeight += height;
		}

	// If rows were inserted to full out the page, invalidate the whole
	// table, it all needs to be redrawn.
	if (rowsInserted)
		TblMarkTableInvalid (table);

	// If the height of the data in the table is greater than the height
	// of the table, then the bottom of the last row is clipped and the
	// table is scrollable.
	lastItemClipped = (dataHeight > tableHeight);

	// Update the scroll arrows.
	ListViewUpdateScrollers (frm, lastRecordNum, lastItemClipped);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawTable
 *
 * DESCRIPTION: Updates the entire list view, such as when changing categories
 *
 * PARAMETERS:  updateCode - indicates how (or whether) to rebuild the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *
 ***********************************************************************/
static void ListViewDrawTable (UInt16 updateCode)
{
	TablePtr table = GetObjectPtr (ListTable);

	TblEraseTable (table);

	switch (updateCode)
		{
		case updateDisplayOptsChanged:
		case updateFontChanged:
			ListViewInit (FrmGetActiveForm ());
			break;

		case updateCategoryChanged:
		case updateGoTo:
			ListViewLoadTable (true);
			break;
		}

	TblDrawTable (table);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewRedrawTable
 *
 * DESCRIPTION: Redraw the rows of the table that are marked invalid
 *
 * PARAMETERS:  fillTable - if true the top visible item will be scroll down
 *                          such that a full table is displayed
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	4/14/99	Initial Revision
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *
 ***********************************************************************/
static void ListViewRedrawTable (Boolean fillTable)
{
	TablePtr table;
	FormPtr frm;

	frm = FrmGetFormPtr (ListView);
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ListTable));

	ListViewLoadTable (fillTable);
	TblRedrawTable (table);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNewToDo
 *
 * DESCRIPTION: This routine adds a new ToDo item to the ToDo list.
 *              If a ToDo item is currently selected, the new item
 *              will be added after the selected item.  If not, the
 *              new item will be added after the last priority "one" item.
 *
 * PARAMETERS:  event - pointer to the keyDown event, or NULL
 *
 * RETURNED:    true if the event has handled
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	5/1/95		Initial Revision
 *			peter	4/24/00		Add support for re-masking private record.
 *
 ***********************************************************************/
static Boolean ListViewNewToDo (EventPtr event)
{
	Err error;
	UInt16 i;
	Int16 row;
	Int16 column;
	UInt16 recordNum;
	Int16 rowsInTable;
	UInt16 category;
	Char desc [2];
	FontID curFont;
	FormPtr frm;
	TablePtr table;
	Boolean empty;
	Boolean handled = false;
	//Boolean found;
	MemHandle recordH;
	ToDoItemType newToDo;
	ToDoDBRecordPtr recordP;

	table = GetObjectPtr (ListTable);


	// We'll have an event if the ToDo item is being created as the result
	// writing a character.
	if (event)
		{
		// Convert lower case alpha character to upper-case.
		desc[0] = event->data.keyDown.chr;
		desc[1] = 0;
		if ((UInt8)desc[0] >= 'a' && (UInt8)desc[0] <= 'z')
			desc[0] -= ('a' - 'A');
		handled = true;
		}
	else
		*desc = 0;


	// If a ToDo item is selected, insert the new item after the current
	// selection.
	if (ItemSelected)
		{
		TblGetSelection (table, &row, &column);
		recordNum = TblGetRowID (table, row);

		// Check if the current record is empty, if it is, don't insert
		// a new record.
		recordH = DmQueryRecord (ToDoDB, recordNum);
		recordP = MemHandleLock (recordH);
		empty = (! recordP->description) && (! *GetToDoNotePtr(recordP));
		MemHandleUnlock (recordH);
		if (empty)
			{
			ListViewRestoreEditState ();
			return (false);
			}

		// Save the record.
		// This was done by calling TblReleaseFocus, but that doesn't deal with
		// the possibility that the focus is currently in an unmasked private
		// record which needs to be re-masked.
		ListViewClearEditState ();

		error = ToDoInsertNewRecord (ToDoDB, &recordNum);
		if ((! error) && *desc)
			error = ToDoChangeRecord (ToDoDB, &recordNum, toDoDescription, desc);


		// Display an alert that indicates that the new record could
		// not be created.
		if (error)
			{
			FrmAlert (DeviceFullAlert);
			return (false);
			}

		// Insert a row into the table, after the currently selected row.
		rowsInTable = TblGetNumberOfRows (table);
		if (row != TblGetLastUsableRow (table))
			{
			TblInsertRow (table, row);
			curFont = FntSetFont (ListFont);
			ListInitTableRow (table, row+1, recordNum, FntLineHeight ());
			FntSetFont (curFont);

			// Invalidate all the rows from the inserted row to the end of the
			// table so that they will be redrawn.
			for (i = row; i < rowsInTable; i++)
				TblMarkRowInvalid (table, i);
			}

		// If we're inserting after the last visible row, force the table
		// to scroll up one row.
		else if (rowsInTable > 1)
			{
			if (row == 0)
				TopVisibleRecord = recordNum;
			else
				TopVisibleRecord = TblGetRowID (table, 1);
			}
		}


	// Add a new ToDo item after all the priority "one" items.
	else
		{
		newToDo.priority = defaultPriority;
		*((UInt16 *) &newToDo.dueDate) = toDoNoDueDate;
		newToDo.description = desc;
		newToDo.note = NULL;

		// If we're showing all categories the new item will be uncategorized.
		if (CurrentCategory == dmAllCategories)
			category = dmUnfiledCategory;
		else
			category = CurrentCategory;

		error = ToDoNewRecord (ToDoDB, &newToDo, category, &recordNum);

		// Display an alert that indicates that the new record could
		// not be created.
		if (error)
			{
			FrmAlert (DeviceFullAlert);
			return (false);
			}

		if (TopVisibleRecord == recordNum)
			{
			// Invalidate all the rows so that they will be drawn.
			rowsInTable = TblGetNumberOfRows (table);
			for (i = 0; i < rowsInTable; i++)
				TblSetRowUsable (table, i, false);
			}
		else
			TopVisibleRecord = recordNum;
		}

	CurrentRecord = recordNum;

	ListViewRedrawTable (true);

	// Give the focus to the new item.
	/*found =*/ TblFindRowID (table, recordNum, &row);
	//ErrNonFatalDisplayIf(!found, "New record not in table");
	frm = FrmGetActiveForm ();
	FrmSetFocus (frm, FrmGetObjectIndex (frm, ListTable));
	TblGrabFocus (table, row, descColumn);
	FldGrabFocus (TblGetCurrentField (table));

	ItemSelected = true;

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteToDo
 *
 * DESCRIPTION: This routine deletes the selected ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteToDo (void)
{
	UInt16 i;
	Int16 row;
	Int16 column;
	UInt16 numRows;
	UInt16 recordNum;
	TablePtr table;

	// If no ToDo item is selected, return.
	table = GetObjectPtr (ListTable);

	// Check if we are editing an item.
	if (! TblEditing (table))
		return;

	TblGetSelection (table, &row, &column);
	TblReleaseFocus (table);

	// Check if the record is empty, if it is, clear the edit state,
	// this will delete the current record when it's blank.
	recordNum = TblGetRowID (table, row);

	if (! ClearEditState ())
		{
		// Display an alert to confirm the delete operation, and delete the
		// record if the alert is confirmed.
		if (! DeleteRecord (recordNum))
			{
				FrmUpdateForm (ListView, updateRedrawAll);		// Re-masks the record if necessary.
				return;
			}
		}

	// Invalid the row deleted and all the row following the deleted record so
	// that they will redraw.
	numRows = TblGetNumberOfRows (table);
	for (i = row; i < numRows; i++)
		TblSetRowUsable (table, i, false);

	ListViewRedrawTable (true);

	ItemSelected = false;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteNote
 *
 * DESCRIPTION: This routine deletes the note attached to the selected
 *              ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteNote ()
{
	UInt16 i;
	Int16 row;
	Int16 column;
	UInt16 height;
	UInt16 newHeight;
	UInt16 tableHeight;
	Int16 columnWidth;
	UInt16 recordNum;
	Int16 rowsInTable;
	TablePtr table;
	Boolean empty;
	RectangleType r;
	ToDoDBRecordPtr recordP;

	table = GetObjectPtr (ListTable);

	// Check if we are editing an item.
	if (! TblEditing (table))
		return;

	// Get the selected record.
	TblGetSelection (table, &row, &column);
	recordNum = TblGetRowID (table, row);

	// Check if the record has a note attached.
	recordP = MemHandleLock (DmQueryRecord (ToDoDB, recordNum));
	empty = (! *GetToDoNotePtr(recordP));
	MemPtrUnlock (recordP);
	if (empty) return;

	// Confirm that the note should be deleted.
	if (FrmAlert(DeleteNoteAlert) != DeleteNoteYes)
		return;

	TblReleaseFocus (table);

	// Get the current height of the description.
	height = TblGetRowHeight (table, row);

	// Remove the note from the record.
	ToDoChangeRecord (ToDoDB, &recordNum, toDoNote, "");

	// Mark the record dirty.
	DirtyRecord (recordNum);

	// Mark the current row non-usable so the it will redraw.
	TblSetRowUsable (table, row, false);

	// Get the new height of the description, the desciption may be short
	// because we can draw in the space vacated by the note indicator.
	columnWidth = TblGetColumnWidth (table, descColumn);
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	newHeight =  ListViewGetDescriptionHeight (recordNum, columnWidth, tableHeight);

	// If the height of the description has changed, invalid all the row
	// after the current row so that they be redrawn.
	if (height != newHeight)
		{
		rowsInTable = TblGetNumberOfRows (table);
		for (i = row+1; i < rowsInTable; i++)
			TblSetRowUsable (table, i, false);
		}

	ListViewRedrawTable (true);

	ListViewRestoreEditState ();
}

/***********************************************************************
 *
 * FUNCTION:    ListViewCrossOutItem
 *
 * DESCRIPTION: This routine is called when a ToDo item is marked
 *              complete.  If completed item are not display then
 *              we display an animation of a line being drawn through
 *              the ToDo item.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/21/96	Initial Revision
 *
 ***********************************************************************/
static void ListViewCrossOutItem (Int16 row)
{
	UInt16 width;
	UInt16 length;
	UInt16 maxWidth;
	UInt16 recordNum;
	UInt16 charsToDraw;
	Int16 x, y;
	Int16 lineHeight;
	FontID curFont;
	Char* chars;
	TablePtr table;
	RectangleType r;
	RectangleType tableR;
	MemHandle recordH;
	ToDoDBRecordPtr toDoRec;

	table = GetObjectPtr (ListTable);

	// Get a pointer to the ToDo record.
	recordNum = TblGetRowID (table, row);
	recordH = DmQueryRecord( ToDoDB, recordNum);
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	curFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();

	TblGetBounds (table, &tableR);

	TblGetItemBounds (table, row, descColumn, &r);
	maxWidth = r.extent.x;

	// If the record has a note, leave space for the note indicator.
	if (*GetToDoNotePtr (toDoRec))
		maxWidth -= tableNoteIndicatorWidth;

	chars = &toDoRec->description;
	length = 0;

	y = r.topLeft.y + (lineHeight >> 1);
	while (*chars)
		{
		// Get the number of character on each line.
		length = FldWordWrap (chars, maxWidth);
		charsToDraw = length;

		// Don't draw the linefeedChr character.
		if ((charsToDraw) && chars[charsToDraw-1] == linefeedChr)
			charsToDraw--;

		// Don't draw trailing spaces to tabs.
		while (charsToDraw && (chars[charsToDraw-1] == spaceChr ||
			chars[charsToDraw-1] == tabChr))
			charsToDraw--;

		// Draw a line over the character.
		width = FntLineWidth (chars, charsToDraw);
		x = r.topLeft.x;
		while (width)
			{
			WinDrawLine (x, y, x, y);
			x++;
			width--;
			}
		chars += length;
		y += FntLineHeight ();

		if (y > tableR.topLeft.y + tableR.extent.y)
			break;
		}

	MemHandleUnlock (recordH);

	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewChangeCompleteStatus
 *
 * DESCRIPTION: This routine is called when a ToDo item is marked
 *              complete.  If completed items are not displayed
 *              (a preference setting),  this routine will remove the
 *              item from the list.
 *
 * PARAMETERS:  row      - row in the table
 *              complete - true if the item is marked complete
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewChangeCompleteStatus (Int16 row, UInt16 complete)
{
	UInt16 recordNum;
	UInt32 ticks;
	Boolean deleted;
	TablePtr table;
	DateType dueDate;
	DateTimeType today;


	table = GetObjectPtr (ListTable);
	recordNum = TblGetRowID (table, row);

	// If completed item are not shown then display an animation
	// of a line being drawn through the item.
	if (! ShowCompletedItems)
		{
		ticks = TimGetTicks ();
		ListViewCrossOutItem (row);
		}

	// Update the record to reflect the new completion status.
	ToDoChangeRecord (ToDoDB, &recordNum, toDoComplete, &complete);

	// Should the due date be changed to the completion date?
	if (complete && ChangeDueDate)
		{
		TimSecondsToDateTime (TimGetSeconds (), &today);
		dueDate.year = today.year - firstYear;
		dueDate.month = today.month;
		dueDate.day = today.day;
		ToDoChangeRecord (ToDoDB, &recordNum, toDoDueDate, &dueDate);
		CurrentRecord = recordNum;
		}

	// Mark the record dirty.
	DirtyRecord (recordNum);


	// If completed items are shown and the dae date was change, redraw
	// the list.
	if (ShowCompletedItems)
		{
		if (complete && ChangeDueDate)
			FrmUpdateForm (ListView, updateItemMove);
		else
			ListViewRestoreEditState ();
		}

	// If completed items are hidden, update the table.
	else
		{
		deleted = ClearEditState ();

		// If the current record wasn't empty, delay before redrawing the
		// table so that the crossout animation may be seen.
		if (! deleted)
			{
			while (TimGetTicks () - ticks < crossOutDelay)
				;
			}

		ListViewRedrawTable (false);
		}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectDueDate
 *
 * DESCRIPTION: This routine is called when a "due date" item in the do to list
 *              is selected.  The due date  popup list is displayed; if
 *              the due date of the item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95	Initial Revision
 *			peter	10/10/00	Don't move record if due date isn't changed.
 *			peter	1/12/01	Restore edit state if due date isn't changed.
 *
 ***********************************************************************/
static void ListViewSelectDueDate (TablePtr table, Int16 row)
{
	Int16 itemSeleted;
	UInt16 recordNum;
	UInt16 newRecordNum;
	ListPtr lst;
	DateType oldDueDate, newDueDate;
	MemHandle recordH;
	RectangleType r;
	ToDoDBRecordPtr toDoRec;

	lst = GetObjectPtr (ListDueDateList);

	// Unhighlight the selected item.
	TblUnhighlightSelection (table);

	// Get the due date from the ToDo record.
	recordNum = TblGetRowID (table, row);
	recordH = DmQueryRecord (ToDoDB, recordNum);
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);
	oldDueDate = newDueDate = toDoRec->dueDate;
	MemHandleUnlock (recordH);


	// Set the popup list's selection.
	if (DateToInt (oldDueDate) == toDoNoDueDate)
		LstSetSelection (lst, noDueDateItem);
	else
		LstSetSelection (lst, selectDateItem);


	// Position the list.
	TblGetItemBounds (table, row, dueDateColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);

	// Display the list until a selection is made.
	itemSeleted = LstPopupList (lst);

	// Minus one indicates the popup list was dismissed without a selection
	// being made.
	if (itemSeleted == -1)
		goto NoChange;

	DetermineDueDate (itemSeleted, &newDueDate);

	// Don't update the record if the due date selected is the same as before.
	if (!MemCmp (&oldDueDate, &newDueDate, sizeof (DateType)))
		goto NoChange;

	// Update the database record.
	newRecordNum = recordNum;
	ToDoChangeRecord (ToDoDB, &newRecordNum, toDoDueDate, &newDueDate);

	// Changing the due date may change the record's index.
	CurrentRecord = newRecordNum;

	// Mark the record dirty.
	DirtyRecord (newRecordNum);

	// Make sure the row is redrawn.
	TblMarkRowInvalid (table, row);

	// Send an event that will cause the view to be redrawn.
	FrmUpdateForm (ListView, updateItemMove);
	return;

NoChange:
	ListViewRestoreEditState ();
	return;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectPriority
 *
 * DESCRIPTION: This routine is called when a "priority" item in the do to list
 *              is selected.  A popup list of priority is displayed; if
 *              the priority of an item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewSelectPriority (TablePtr table, Int16 row)
{
	Int16 priority;
	Int16 newPriority;
	UInt16 recordNum;
	UInt16 newRecordNum;
	ListPtr lst;
	RectangleType r;

	lst = GetObjectPtr (ListPriorityList);

	// Unhighlight the priority.
	TblUnhighlightSelection (table);

	// Set the list's selection to the current priority.
	priority = TblGetItemInt (table, row, priorityColumn);
	LstSetSelection (lst, priority-1);

	// Position the list.
	TblGetItemBounds (table, row, priorityColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);

	newPriority = LstPopupList (lst);

	// Minus one indicates the popup list was dismissed without a selection
	// being made.
	if ((newPriority == -1) || (newPriority+1 == priority))
		{
		ListViewRestoreEditState ();
		return;
		}

	// Update the database record.
	newPriority++;									// one base the priority
	recordNum = TblGetRowID (table, row);
	newRecordNum = recordNum;
	ToDoChangeRecord (ToDoDB, &newRecordNum, toDoPriority, &newPriority);

	// Changing the priority may change the record's index.
	CurrentRecord = newRecordNum;

	// Mark the record dirty.
	DirtyRecord (newRecordNum);

	// Make sure the row is redrawn.
	TblMarkRowInvalid (table, row);

	// Send an event that will cause the view to be redrawn.
	FrmUpdateForm (ListView, updateItemMove);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectItemsCategory
 *
 * DESCRIPTION: This routine is called when a "category" item in the do to
 *              list is selected.  A popup list of categories is displayed,
 *              if the category of an item is changed, the record is updated
 *              and the re-sorted list is redrawn.
 *
 * PARAMETERS:  table - ToDo table
 *              row   - row in the table
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	07/12/96	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryHideEditCategory
 *
 ***********************************************************************/
static void ListViewSelectItemsCategory (TablePtr table, Int16 row)
{
	Int16				curSelection;
	Int16				newSelection;
	UInt16 			attr;
	UInt16			category;
	UInt16			recordNum;
	ListPtr			lst;
	Char*				name;
	RectangleType	r;

	lst = GetObjectPtr (ListItemsCategoryList);

	// Get the category of the item in the specified row.
	recordNum = TblGetRowID (table, row);
	DmRecordInfo (ToDoDB, recordNum, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;

	// Unhighlight the priority.
	TblUnhighlightSelection (table);

	LstSetPosition (lst, 0, 0);

	// Create a list of categories.
	CategoryCreateList (ToDoDB, lst, category, false, true, 1, categoryHideEditCategory, true);

	// Position the list.
	TblGetItemBounds (table, row, categoryColumn, &r);
	LstSetPosition (lst, r.topLeft.x, r.topLeft.y);


	// Display the category list.
	curSelection = LstGetSelection (lst);
	newSelection = LstPopupList (lst);

	// Was a new category selected?
	if ((newSelection != curSelection) && (newSelection != -1))
		{
		name = LstGetSelectionText (lst, newSelection);
		category = CategoryFind (ToDoDB, name);

		// Update the database record's category.
		ToDoChangeRecord (ToDoDB, &recordNum, toDoCategory, &category);

		// Changing the category may change the record's index
		CurrentRecord = recordNum;

		// Mark the record dirty.
		DirtyRecord (recordNum);

		// Make sure the row is redrawn.
		TblMarkRowInvalid (table, row);

		// Send an event that will cause the view to be redrawn.
		FrmUpdateForm (ListView, updateItemMove);
		}

	else
		{
		ListViewRestoreEditState ();
		}

	CategoryFreeList (ToDoDB, lst, false, false);
	}


/***********************************************************************
 *
 * FUNCTION:    ListViewItemSelected
 *
 * DESCRIPTION: This routine is called when an item in the do to list
 *              is selected.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/9/95	Initial Revision
 *			peter	4/24/00	Add support for un-masking just the selected record.
 *
 ***********************************************************************/
static void ListViewItemSelected (EventPtr event)
{
	UInt16 on;
	Int16 row;
	Int16 column;
	UInt16 tableID;
	TablePtr table;
	EventType newEvent;
	UInt16 systemVolume, mutedVolume = 0;

	table = event->data.tblSelect.pTable;
	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;
	tableID = event->data.tblSelect.tableID;

	if (TblRowMasked(table,row))
		{
		if (SecVerifyPW (showPrivateRecords) == true)
			{
			// We only want to unmask this one record, so restore the preference.
			PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

			// Unmask just the current row.
			TblSetRowMasked (table, row, false);

			// Draw the row unmasked.
			TblMarkRowInvalid (table, row);
			TblRedrawTable(table);

			// Only change the visual status of this record, leaving all others masked.
			CurrentRecordVisualStatus = showPrivateRecords;
				// Leave PrivateRecordVisualStatus set to maskPrivateRecords

			// Now that the row is unmasked, let the table re-handle the table
			// enter event. This will cause the field to be made fully visible
			// and place the cursor at the start of the field. It is necessary
			// to put the cursor in the field so that tapping outside the field
			// can be used to re-mask the record.

			newEvent.eType = tblEnterEvent;
			newEvent.penDown = event->penDown;
			newEvent.tapCount = 0;						// don't select anything
			newEvent.screenX = 0;						// put cursor at start
			newEvent.screenY = 0;
			newEvent.data.tblEnter.tableID = tableID;
			newEvent.data.tblEnter.pTable = table;
			newEvent.data.tblEnter.row = row;
			newEvent.data.tblEnter.column = descColumn;
				// Never let this event check off the item or view the note.

			// Rather than posting the event, handle it directly to avoid
			// the click produced by it, since a click was already produced.
			SndGetDefaultVolume (NULL, &systemVolume, NULL);
			SndSetDefaultVolume (NULL, &mutedVolume, NULL);
			TblHandleEvent (table, &newEvent);
			SndSetDefaultVolume (NULL, &systemVolume, NULL);
			}
		}
	else
		{
		if (column == completedColumn)
			{
			on = TblGetItemInt (table, row, column);
			ListViewChangeCompleteStatus (row, on);
			}

		else if (column == priorityColumn)
			{
			ListViewSelectPriority (table, row);
			}

		else if (column == descColumn)
			{
			CurrentRecord = TblGetRowID (table, row);
			// If the table is in edit mode then the description field
			// was selected, otherwise the note indicator must have
			// been selected.
			if (TblEditing (table))
				{
				ItemSelected = true;
				}

			else
				FrmGotoForm (NewNoteView);
			}

		else if (column == dueDateColumn)
			{
			ListViewSelectDueDate (table, row);
			}

		else if (column == categoryColumn)
			{
			ListViewSelectItemsCategory (table, row);
			}
		}

}


/***********************************************************************
 *
 * FUNCTION:    ListViewResizeDescription
 *
 * DESCRIPTION: This routine is called when the height of a ToDo item's
 *              description is changed as a result of user input.
 *              If the new height of the field is shorter,  more items
 *              may need to be added to the bottom of the list.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/18/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewResizeDescription (EventPtr event)
{
	UInt16 lastRow;
	UInt16 lastRecord;
	UInt16 topRecord;
	FieldPtr fld;
	TablePtr table;
	Boolean lastItemClipped;
	RectangleType itemR;
	RectangleType tableR;
	RectangleType fieldR;

	// Get the current height of the field;
	fld = event->data.fldHeightChanged.pField;
	FldGetBounds (fld, &fieldR);

	// Have the table object resize the field and move the items below
	// the field winUp or winDown.
	table = GetObjectPtr (ListTable);
	TblHandleEvent (table, event);


	// If the field's height has expanded , and there are no items scrolled
	// off the top of the table, just update the scrollers.
	if (event->data.fldHeightChanged.newHeight >= fieldR.extent.y)
		{
		topRecord = TblGetRowID (table, 0);
		if (topRecord != TopVisibleRecord)
			TopVisibleRecord = topRecord;
		else
			{
			// Update the scroll arrows.
			lastRow = TblGetLastUsableRow (table);
			TblGetBounds (table, &tableR);
			TblGetItemBounds (table, lastRow, descColumn, &itemR);
			lastItemClipped = (itemR.topLeft.y + itemR.extent.y >
			 	tableR.topLeft.y + tableR.extent.y);
			lastRecord = TblGetRowID (table, lastRow);
			ListViewUpdateScrollers (FrmGetActiveForm (), lastRecord,
				lastItemClipped);

			return;
			}
		}

	// Add items to the table to fill in the space made available by the
	// shortening the field.
	ListViewRedrawTable (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories in the List View.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    The index of the new category.
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95 Initial Revision
 *			rbb	04/14/99 Uses new ListViewDrawTable
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *			mchen	12/20/00 Resort after CategorySelect() because categories
 *								may have changed
 *
 ***********************************************************************/
static UInt16 ListViewSelectCategory (void)
{
	FormPtr frm;
	UInt16 category;
	Boolean categoryEdited;
	UInt8 updateCode = updateCategoryChanged;

	// Process the category popup list.
	category = CurrentCategory;

	frm = FrmGetActiveForm();
	categoryEdited = CategorySelect (ToDoDB, frm, ListCategoryTrigger,
					    ListCategoryList, true, &category, CategoryName, 1, categoryDefaultEditCategoryString);

	// if the categories changed, we need to resort.  however, we can't tell
	// if a category that is not the current one has changed (CategorySelect() only
	// returnes true of the current category has changed) so for now we ALWAYS
	// resort
	ToDoSort(ToDoDB);

	// If the option for category column is set and we switched to/from "All",
	// the table will need to be rebuilt with/without the column
	if ( ShowCategories && (CurrentCategory != category) &&
			( (category == dmAllCategories) || (CurrentCategory == dmAllCategories) ))
		{
		updateCode = updateDisplayOptsChanged;
		}

	if (category == dmAllCategories)
		ShowAllCategories = true;
	else
		ShowAllCategories = false;

	if ( (categoryEdited) || (CurrentCategory != category) || ShowCategories)
		{
		ChangeCategory (category);

		// Display the new category.
		ListViewDrawTable (updateCode);
		}

	return (category);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNextCategory
 *
 * DESCRIPTION: This routine displays the next category, if the last
 *              catagory is being displayed we wrap to the first category.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							ShowAllCategories
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		9/15/95		Initial Revision
 *			rbb		4/14/99		Uses new ListViewDrawTable
 *
 ***********************************************************************/
static void ListViewNextCategory (void)
{
	UInt16 index;
	UInt16 category;
	//FormPtr frm;
	ControlPtr ctl;
	UInt16 updateCode = updateCategoryChanged;

	category = CurrentCategory;

	// Find the next category that has displayable items.
	do
		{
		CurrentCategory = CategoryGetNext (ToDoDB, CurrentCategory);

		index = 0;
		if (SeekRecord (&index, 0, dmSeekForward))
			break;
		}
	while (CurrentCategory != dmAllCategories);


	if (category == CurrentCategory) return;

	// If the option for category column is set and we switched to/from "All",
	// the table will need to be rebuilt with/without the column
	if ( ShowCategories &&
			( (category == dmAllCategories) || (CurrentCategory == dmAllCategories) ))
		{
		updateCode = updateDisplayOptsChanged;
		}

	if (CurrentCategory == dmAllCategories)
		ShowAllCategories = true;
	else
		ShowAllCategories = false;

	ChangeCategory (CurrentCategory);

	// Set the label of the category trigger.
	//frm = FrmGetActiveForm ();
	ctl = GetObjectPtr (ListCategoryTrigger);
	CategoryGetName (ToDoDB, CurrentCategory, CategoryName);
	CategorySetTriggerLabel (ctl, CategoryName);


	// Display the new category.
	ListViewDrawTable (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGotoAppointment
 *
 * DESCRIPTION: This routine sets winUp the global variables such that the
 *              list view will display the text found by the text search
 *              command.
 *
 * PARAMETERS:  event - frmGotoEvent
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/11/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewGotoItem (EventPtr event)
{
	ItemSelected = true;
	CurrentRecord = event->data.frmGoto.recordNum;
	TopVisibleRecord = CurrentRecord;
	ListEditPosition = event->data.frmGoto.matchPos;
	ListEditSelectionLength = event->data.frmGoto.matchLen;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of ToDo items
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *              oneLine   - if true the list is scrolled by a single line,
 *                          if false the list is scrolled by a full screen.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	02/21/95	Initial Revision
 *			rbb	04/14/99	Uses new ListViewDrawTable
 *			gap	10/25/99	Optimized scrolling to only redraw if item position changed.
 *
 ***********************************************************************/
static void ListViewScroll (WinDirectionType direction)
{
	UInt16			row;
	UInt16			index;
	UInt16			height;
	UInt16 			recordNum;
	UInt16 			columnWidth;
	UInt16 			tableHeight;
	TablePtr 		table;
	RectangleType	r;
	UInt16			prevTopVisibleRecord = TopVisibleRecord;


	table = GetObjectPtr (ListTable);
	TblReleaseFocus (table);

	ItemSelected = false;
	CurrentRecord = noRecordSelected;

	// Get the height of the table and the width of the description
	// column.
	TblGetBounds (table, &r);
	tableHeight = r.extent.y;
	height = 0;
	columnWidth = TblGetColumnWidth (table, descColumn);

	// Scroll the table down.
	if (direction == winDown)
		{
		// Get the record index of the last visible record.  A row
		// number of minus one indicates that there are no visible rows.
		row = TblGetLastUsableRow (table);
		if (row == tblUnusableRow) return;

		recordNum = TblGetRowID (table, row);

		// If there is only one record visible, this is the case
		// when a record occupies the whole screeen, move to the
		// next record.
		if (row == 0)
			SeekRecord (&recordNum, 1, dmSeekForward);
		}

	// Scroll the table up.
	else
		{
		// Scan the records before the first visible record to determine
		// how many record we need to scroll.  Since the heights of the
		// records vary,  we sum the heights of the records until we get
		// a screen full.
		recordNum = TblGetRowID (table, 0);
		height = TblGetRowHeight (table, 0);
		if (height >= tableHeight)
			height = 0;

		while (height < tableHeight)
			{
			index = recordNum;
			if ( ! SeekRecord (&index, 1, dmSeekBackward) ) break;
			height += ListViewGetDescriptionHeight (index, columnWidth, tableHeight);
			if ((height <= tableHeight) || (recordNum == TblGetRowID (table, 0)))
				recordNum = index;
			}
		}

	TopVisibleRecord = recordNum;
	ListViewLoadTable (true);

	// Need to compare the previous top record to the current after ListViewLoadTable
	// as it will adjust TopVisibleRecord if drawing from recordNum will not fill the
	// whole screen with items.
	if (TopVisibleRecord != prevTopVisibleRecord)
		TblRedrawTable (table);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDeleteCompleted
 *
 * DESCRIPTION: This routine deletes ToDo items that are marked
 *              complete.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/29/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewDeleteCompleted (void)
{
	UInt16 i;
	UInt16 index;
	UInt16 ctlIndex;
	UInt16 numRecord;
	Int16 rowsInTable;
	UInt16 buttonHit;
	FormPtr alert;
	TablePtr table;
	Boolean complete;
	Boolean saveBackup;
	MemHandle recordH;
	ToDoDBRecordPtr recordP;


	table = GetObjectPtr (ListTable);
	TblReleaseFocus (table);

	// Display an alert to comfirm the operation.
	alert = FrmInitForm (DeleteCompletedDialog);

	ctlIndex = FrmGetObjectIndex (alert, DeleteCompletedSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);
	buttonHit = FrmDoDialog (alert);
	saveBackup = FrmGetControlValue (alert, ctlIndex);

	FrmDeleteForm (alert);

	if (buttonHit == DeleteCompletedCancel)
		return;

	SaveBackup = saveBackup;

	// Delete records marked complete.
	numRecord = DmNumRecords (ToDoDB);
	if (! numRecord) return;

	for (index = numRecord-1; (Int16) index >= 0; index--)
		{
		recordH = DmGetRecord (ToDoDB, index);
		if (recordH == 0) continue;

		recordP = MemHandleLock (recordH);
		complete = ((recordP->priority & completeFlag) == completeFlag);
		MemHandleUnlock (recordH);
		DmReleaseRecord (ToDoDB, index, complete);

		if (complete)
			{
			if (SaveBackup)
				DmArchiveRecord (ToDoDB, index);
			else
				DmDeleteRecord (ToDoDB, index);

			// Move deleted record to the end of the index so that the
			// sorting routine will work.
			DmMoveRecord (ToDoDB, index, numRecord);
			}
		}

	// Redraw the ToDo list.
	rowsInTable = TblGetNumberOfRows (table);
	for (i = 0; i < rowsInTable; i++)
		TblSetRowUsable (table, i, false);

	ListViewRedrawTable (true);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDoCommand
 *
 * DESCRIPTION: This routine preforms the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		03/29/95	art	Created by Art Lamb.
 *		09/17/99	jmp	Use NewNoteView instead of NoteView.
 *		10/4/99	jmp	Replaced call to DmOpenDatabaseByTypeCreator() with
 *							ToDoGetDatabase().
 *		11/04/99	jmp	To prevent other sublaunch issues, remind ourselves
 *							that we've sublaunched already into PhoneNumberLookup().
 *		11/16/99	jmp	Release the table focus around the send item and the send
 *							category commands.  This fixes bug #24067 and makes this
 *							code consistent with what the Datebook does.
 *		08/28/00	kwk	Use new FrmGetActiveField call.
 *
 ***********************************************************************/
static Boolean ListViewDoCommand (UInt16 command)
{
	UInt16 		pasteLen;
	MemHandle 	pasteCharsH;
	FieldPtr 	fld;
	Boolean 		handled = true;
	Boolean		wasHiding;
	UInt16 		mode;

	switch (command)
		{
		case DeleteCmd:
			if (ItemSelected)
				ListViewDeleteToDo ();
			else
				FrmAlert (SelectItemAlert);
			break;

		case CreateNoteCmd:
			if (ItemSelected)
				FrmGotoForm (NewNoteView);
			else
				FrmAlert (SelectItemAlert);
			break;

		case DeleteNoteCmd:
			if (ItemSelected)
				ListViewDeleteNote ();
			else
				FrmAlert (SelectItemAlert);
			break;

		case BeamRecordCmd:
			if (ItemSelected)
				{
				TblReleaseFocus (GetObjectPtr (ListTable));
				ToDoSendRecord (ToDoDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
				ListViewRestoreEditState ();
				}
			else
				FrmAlert (SelectItemAlert);
			break;

		case SendRecordCmd:
			if (ItemSelected)
				{
				TblReleaseFocus (GetObjectPtr (ListTable));
				ToDoSendRecord (ToDoDB, CurrentRecord, exgSendPrefix, NoDataToSendAlert);
				ListViewRestoreEditState ();
				}
			else
				FrmAlert (SelectItemAlert);
			break;

		case BeamCategoryCmd:
			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ListTable));

			ToDoSendCategory(ToDoDB, CurrentCategory, exgBeamPrefix, NoDataToBeamAlert);

			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case SendCategoryCmd:
			if (ItemSelected)
				TblReleaseFocus (GetObjectPtr (ListTable));

			ToDoSendCategory(ToDoDB, CurrentCategory, exgSendPrefix, NoDataToSendAlert);

			if (ItemSelected)
				ListViewRestoreEditState ();
			break;

		case FontCmd:
			ListViewClearEditState ();
			ListFont = SelectFont (ListFont);
			break;

		case PhoneLookupCmd:
			if (ItemSelected)
				{
				fld = FrmGetActiveField (NULL);
				if (fld)
					{
					InPhoneLookup = true;
					PhoneNumberLookup (fld);
					InPhoneLookup = false;
					}
				}
			else
				FrmAlert (SelectItemAlert);
			break;

		case SecurityCmd:
			ListViewClearEditState();
			wasHiding = (PrivateRecordVisualStatus == hidePrivateRecords);

			PrivateRecordVisualStatus = CurrentRecordVisualStatus = SecSelectViewStatus();

			if (wasHiding ^ (PrivateRecordVisualStatus == hidePrivateRecords)) //xor on two logical values - mode to open DB has changed
				{
				// Close the application's data file.
				ToDoSavePrefs();					// BGT
				DmCloseDatabase (ToDoDB);

				mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

				ToDoGetDatabase(&ToDoDB, mode);
				ErrFatalDisplayIf(!ToDoDB,"Can't reopen DB");
				// Read the preferences.
				ToDoLoadPrefs();
				}

			ListViewUpdateDisplay (updateDisplayOptsChanged);
			break;

		case AboutCmd:
			MenuEraseStatus (0);
			ListViewClearEditState ();
			AbtShowAbout (sysFileCToDo);
			break;

		case DeleteCompletedCmd:
			ListViewClearEditState ();
			ListViewDeleteCompleted ();
			break;

		case sysEditMenuPasteCmd:
			fld = FrmGetActiveField (NULL);
			if (! fld)
				{
				pasteCharsH = ClipboardGetItem (clipboardText, &pasteLen);
				if (pasteCharsH && pasteLen)
					{
					ListViewNewToDo (NULL);
					}
				}
			handled = false;
			break;

		default:
			handled = false;
		}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewGetColumnWidth
 *
 * DESCRIPTION: This routine returns the width of the specified
 *              column.
 *
 * PARAMETERS:	 column - column of the list table
 *
 * RETURNED:	 width of the column in pixels
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/10/97	Initial Revision
 *
 ***********************************************************************/
static UInt16 ListViewGetColumnWidth (Int16 column)
{
	Char		chr;
	Char		dateBuffer [dateStringLength];
	UInt16	width = 0;
	FontID	curFont;
	Char*		dateStr;


	if (column == priorityColumn)
		{
		curFont = FntSetFont (ListViewPriorityFontID());
		}
	else
		{
		curFont = FntSetFont (ListFont);
		}

	if (column == priorityColumn)
		{
		chr = '1';
		width = (FntCharWidth (chr) - 1) + 6;
		}

	else if (column == dueDateColumn)
		{
		DateToAscii (12, 31, 1997,	DateFormat, dateBuffer);

		// Remove the year from the date string.
		dateStr = dateBuffer;
		if ((DateFormat == dfYMDWithSlashes) ||
			 (DateFormat == dfYMDWithDots) ||
			 (DateFormat == dfYMDWithDashes))
			dateStr += 3;
		else
			dateStr[StrLen(dateStr) - 3] = 0;

		width = FntCharsWidth (dateStr, StrLen (dateStr));

		// Get the width of the character that indicates the item is due.
		// Don't count the whitespace in the character. Handle auto-bolding
		// of list font for the priority number.
		FntSetFont (ListViewPriorityFontID());
		chr = '!';
		width += FntCharWidth (chr) - 1;
		}

	// Size the category column such that is can display about five
	// characters.
	else if (column == categoryColumn)
		{
		chr = '1';
		width = (FntCharWidth (chr) * 5) - 1;
		}

	FntSetFont (curFont);

	return (width);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			rbb	4/14/99	Only show category column when viewing "All"
 *			jmp	11/27/99	Fix long-standing bug where priority column's spacing
 *								would change depending on whether we were being initialized
 *								for the first time or we were being re-inited (as during
 *								an update event or such).
 *			peter	5/26/00	Mask only the description, not the other columns.
 *			peter	6/12/00	Mask all the columns: Marketing requirement for 3.5
 *
 ***********************************************************************/
static void ListViewInit (FormPtr frm)
{
	Int16 row;
	Int16 rowsInTable;
	UInt16 width;
	FontID fontID;
	TablePtr table;
	ControlPtr ctl;
	RectangleType r;
	Boolean showCategories = ShowCategories && (CurrentCategory == dmAllCategories);

	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ListTable));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
		{
		TblSetItemStyle (table, row, completedColumn, checkboxTableItem);
		TblSetItemStyle (table, row, priorityColumn, numericTableItem);
		TblSetItemStyle (table, row, descColumn, textTableItem);
		TblSetItemStyle (table, row, dueDateColumn, customTableItem);
		TblSetItemStyle (table, row, categoryColumn, customTableItem);

		// Set the font used to draw the text of the row. We automatically
		// bold priority number.
		fontID = ListViewPriorityFontID();

		TblSetItemFont (table, row, priorityColumn, fontID);
		TblSetItemFont (table, row, descColumn, ListFont);
		TblSetItemFont (table, row, dueDateColumn, ListFont);
		TblSetItemFont (table, row, categoryColumn, ListFont);

		TblSetRowUsable (table, row, false);
		}

	TblSetColumnUsable (table, completedColumn, true);
	TblSetColumnUsable (table, priorityColumn, ShowPriorities);
	TblSetColumnUsable (table, descColumn, true);
	TblSetColumnUsable (table, dueDateColumn, ShowDueDates);
	TblSetColumnUsable (table, categoryColumn, showCategories);

	// Set up to mask all columns
	TblSetColumnMasked (table, completedColumn, true);
	TblSetColumnMasked (table, priorityColumn, true);
	TblSetColumnMasked (table, descColumn, true);
	TblSetColumnMasked (table, dueDateColumn, true);
	TblSetColumnMasked (table, categoryColumn, true);

	// Set the spacing after the complete column.
	if (ShowPriorities)
		{
		TblSetColumnSpacing (table, completedColumn, 0);
		TblSetColumnSpacing (table, priorityColumn, spaceBeforeDesc);
		}
	else
		{
		TblSetColumnSpacing (table, completedColumn, spaceBeforeDesc);
		TblSetColumnSpacing (table, priorityColumn, spaceNoPriority);
		}

	if (ShowDueDates && showCategories)
		{
		TblSetColumnSpacing (table, dueDateColumn, spaceBeforeCategory);
		}


	// Set the width of the priorities column.
	if (ShowPriorities)
		{
		width = ListViewGetColumnWidth (priorityColumn);
		TblSetColumnWidth (table, priorityColumn, width);
		}

	// Set the width of the due date column.
	if (ShowDueDates)
		{
		width = ListViewGetColumnWidth (dueDateColumn);
		TblSetColumnWidth (table, dueDateColumn, width);
		}

	// Set the width of the category column.
	if (showCategories)
		{
		width = ListViewGetColumnWidth (categoryColumn);
		TblSetColumnWidth (table, categoryColumn, width);
		}

	// Set the width of the description column.
	TblGetBounds (table, &r);
	width = r.extent.x;
	width -= TblGetColumnWidth (table, completedColumn) +
				TblGetColumnSpacing (table, completedColumn);
	width -= TblGetColumnSpacing (table, descColumn);
	if (ShowPriorities)
		width -= TblGetColumnWidth (table, priorityColumn) +
				   TblGetColumnSpacing (table, priorityColumn);
	if (ShowDueDates)
		width -= TblGetColumnWidth (table, dueDateColumn) +
				   TblGetColumnSpacing (table, dueDateColumn);
	if (showCategories)
		width -= TblGetColumnWidth (table, categoryColumn) +
				   TblGetColumnSpacing (table, categoryColumn);


	TblSetColumnWidth (table, descColumn, width);


	// Set the callback routines that will load and save the
	// description field.
	TblSetLoadDataProcedure (table, descColumn, (TableLoadDataFuncPtr)ListViewGetDescription);
	TblSetSaveDataProcedure (table, descColumn, (TableSaveDataFuncPtr)ListViewSaveDescription);

	// Set the callback routine that draws the due date field.
	TblSetCustomDrawProcedure (table, dueDateColumn, ListViewDrawDueDate);

	// Set the callback routine that draws the category field.
	TblSetCustomDrawProcedure (table, categoryColumn, ListViewDrawCategory);

	ListViewLoadTable (true);

	// Set the label of the category trigger.
	ctl = GetObjectPtr (ListCategoryTrigger);
	CategoryGetName (ToDoDB, CurrentCategory, CategoryName);
	CategorySetTriggerLabel (ctl, CategoryName);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateDisplay
 *
 * DESCRIPTION: This routine updates the display of the List View
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the ToDo list.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/2/95	Initial Revision
 *			rbb	4/14/99	Uses new ListViewDrawTable and ListViewRedrawTable
 *			jmp	11/01/99	Fixed problem on frmRedrawUpdateCode events when
 *								we're still in the edit state but we weren't redrawing
 *								the edit indicator.  Fixes ToDo part of bug #23235.
 *			jmp	11/15/99	Make sure to clear the edit state on updateItemDelete and
 *								updateItemHide!
 *			jmp	11/27/99	Make sure we call ListViewInit() on frmRedrawUpdateCodes,
 *								otherwise column spacing may not be correct.
 */


 /*			peter	12/22/00	Reload the table if ClearEditState deletes item when handling
 *								updateItemMove updateCode, but only redraw the table once.
 *
 ***********************************************************************/
static Boolean ListViewUpdateDisplay (UInt16 updateCode)
{
	UInt16 attr;
	Int16 row;
	Int16 column;
	UInt16 numRows;
	UInt16 recordNum;
	UInt32 uniqueID;
	TablePtr table;
	ControlPtr ctl;
	RectangleType itemBounds;
	RectangleType tableBounds;
	Boolean rowIsEntirelyVisible;

	table = GetObjectPtr (ListTable);

	// Was the UI unable to save an image of the ListView when it
	// obscured part of the view with another dialog?  If not,
	// we'll handle the event here.
	if (updateCode & frmRedrawUpdateCode)
		{
		FrmDrawForm(FrmGetActiveForm());

		// If we're editing, then find out which row is being edited,
		// mark it invalid, and redraw the table.
		if (TblEditing(table))
			{
			TblGetSelection(table, &row, &column);
			TblMarkRowInvalid(table, row);
			TblRedrawTable(table);
			}
		return (true);
		}

	// Were the display options modified (ToDoOption dialog) or was the
	// font changed?
	if (updateCode & (updateDisplayOptsChanged | updateFontChanged))
		{
		ListViewDrawTable (updateCode);
		return (true);
		}

	// Was the category of an item changed?
	else if (updateCode & updateCategoryChanged)
		{
		if (ShowAllCategories)
			CurrentCategory = dmAllCategories;
		else
			{
			DmRecordInfo (ToDoDB, CurrentRecord, &attr, NULL, NULL);
			CurrentCategory = attr & dmRecAttrCategoryMask;
			}
		// Set the label of the category trigger.
		ctl = GetObjectPtr (ListCategoryTrigger);
		CategoryGetName (ToDoDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel (ctl, CategoryName);

		TopVisibleRecord = CurrentRecord;
		}

	// Was an item deleted or marked secret? If so, invalidate all the rows
	// following the deleted/secret record.  Also, make sure the edit
	// state is now clear.
	if ( (updateCode & updateItemDelete) || (updateCode & updateItemHide))
		{
		TblGetSelection (table, &row, &column);
		numRows = TblGetNumberOfRows (table);
		for ( ; row < numRows; row++)
			TblSetRowUsable (table, row, false);

		ClearEditState ();
		}

	// Was the item moved?
	// Items are moved when their priority or due date is changed.
	else if (updateCode & updateItemMove)
		{
		// Always redraw the current record
		DmRecordInfo (ToDoDB, CurrentRecord, NULL, &uniqueID, NULL);
		if (TblFindRowData (table, uniqueID, &row))
			TblSetRowUsable (table, row, false);

		// We don't want to scroll the current record into view.
		recordNum = CurrentRecord;
		ItemSelected = false;
		CurrentRecord = noRecordSelected;

		ListViewLoadTable (true);

		// If the item is still entirely visible we will restore the edit state, but
		// If the item is only partially visible, we won't.
		CurrentRecord = recordNum;
		if (!TblFindRowData (table, uniqueID, &row))
			rowIsEntirelyVisible = false;		// Row isn't visible at all.
		else
		{
			// Row is at least partially visible, but is all of it visible?
			if (row < TblGetLastUsableRow (table))
				rowIsEntirelyVisible = true;	// Row isn't last so all must be visible.
			else
			{
				// Row is last, so it may not be entirely visible.
				TblGetBounds (table, &tableBounds);
				TblGetItemBounds (table, row, descColumn, &itemBounds);
				rowIsEntirelyVisible = itemBounds.topLeft.y + itemBounds.extent.y <=
					tableBounds.topLeft.y + tableBounds.extent.y;
			}
		}
		if (!rowIsEntirelyVisible)
		{
			// Entire row must be visible in order to safely maintain selection.
			// Since this is not the case, clear the edit state.
			// This will delete the current record if it's empty.
			if (ClearEditState ())
				{
				// The current record was empty, so it was deleted. This means the
				// record numbers stored in the table rows are no longer valid, so
				// reload and redraw the table.
				ListViewLoadTable (true);
				}
		}

		// Only redraw the table once.
		TblRedrawTable (table);
		return (true);
		}

	ListViewRedrawTable (true);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the ToDo application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date			Description
 *			----	----			-----------
 *			art	2/21/95		Initial Revision
 *			kcr	10/11/95		set initial shift for new todos created via
 *									'New' button, and for re-edit of empty desc field.
 *			kwk	11/21/98		Handle command keys in separate code block, so
 *									TxtCharIsPrint doesn't get called w/virtual chars.
 *			rbb	4/14/99		Uses new ListViewDrawTable
 *			jmp	11/01/99		Don't call ListViewRestoreEditState() at frmUpdateForm
 *									time when the updateCode is frmRedrawUpdateCode because
 *									the edit state is either still valid or it has been
 *									cleared, and we've handled either item elsewhere.
 *			jmp	11/16/99		Release the table focus around the send item event.
 *									This fixes bug #24067 and makes this code consistent with
 *									what the Datebook does.
 *			gap	10/27/00		change the command bar initialization to allow field
 *									code to add cut, copy, paste, & undo commands as
 *									appropriate rather than adding a fixed set of selections.
 *
 ***********************************************************************/
static Boolean ListViewHandleEvent (EventPtr event)
{
	Int16 row;
	FormPtr frm;
	TablePtr table;
	DateType date;
	UInt32 numLibs;
	Boolean handled = false;

	if (event->eType == keyDownEvent)
		{
		if (EvtKeydownIsVirtual(event))
			{
			// ToDo key pressed?
			if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
				{
				if ((event->data.keyDown.modifiers & poweredOnKeyMask))
					{
					// If the date has changed since the last time the device
					// was powered on then redraw the ToDo list so that pass
					// due item will be display correctly.
					DateSecondsToDate (TimGetSeconds (), &date);
					if (DateToInt (date) != DateToInt (Today))
						{
						Today = date;
						ListViewDrawTable (updateRedrawAll);
						}

					}
				// Display the next category
				else
					{
					ListViewClearEditState ();
					ListViewNextCategory ();
					}
				handled = true;
				}

			// Scroll up key pressed?
			else if (event->data.keyDown.chr == vchrPageUp)
				{
				ListViewClearEditState ();
				ListViewScroll (winUp);
				handled = true;
				}

			// Scroll down key pressed?
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				ListViewClearEditState ();
				ListViewScroll (winDown);
				handled = true;
				}

			// Send Data key pressed?
			else if (event->data.keyDown.chr == vchrSendData)
				{
				if (ItemSelected)
					{
					table = GetObjectPtr (ListTable);
					TblReleaseFocus (table);
					ToDoSendRecord (ToDoDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
					ListViewRestoreEditState ();
					}
				else
					FrmAlert (SelectItemAlert);
				handled = true;
				}

			// Confirm key pressed?
			else if (event->data.keyDown.chr == vchrConfirm)
				{
				ItemSelected = false;
				// Leave handled false so table releases focus.
				}
			}

		// If the character is printable, then either auto-create a new entry,
		// or fix up Graffiti auto-shifting.

		else if (TxtCharIsPrint (event->data.keyDown.chr))
			{
			if (!ItemSelected)
				{
				// If no item is selected and the character is display,
				// create a new ToDo item.
				ListViewNewToDo (NULL);
				if (ItemSelected)
					{
					Char buffer[maxCharBytes];
					UInt16 length = TxtSetNextChar (buffer, 0, event->data.keyDown.chr);
					if (TxtTransliterate (buffer, length, buffer, &length, translitOpUpperCase) == 0)
						{
						TxtGetNextChar (buffer, 0, &event->data.keyDown.chr);
						}
					EvtAddEventToQueue (event);
					handled = true;
					}
				}
			}
		}


	else if (event->eType == penDownEvent)
		{
		if (! FrmHandleEvent (FrmGetActiveForm (), event))
			ListViewClearEditState ();
		handled = true;		// Don't let FrmHandleEvent get this event again.
		}


	else if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case ListNewToDoButton:
				ListViewNewToDo (NULL);
				handled = true;
				break;

			case ListDetailsButton:
				if (ItemSelected)
					FrmPopupForm (DetailsDialog);
				else
					FrmAlert (SelectItemAlert);
				handled = true;
				break;

			case ListShowButton:
				FrmPopupForm (OptionsDialog);
				handled = true;
				break;

			case ListUpButton:
				ListViewScroll (winUp);
				handled = true;
				break;

			case ListDownButton:
				ListViewScroll (winDown);
				handled = true;
				break;

			case ListCategoryTrigger:
				ListViewSelectCategory ();
				handled = true;
				break;
			}
		}


	else if (event->eType == ctlEnterEvent)
		{
		switch (event->data.ctlEnter.controlID)
			{
			case ListShowButton:
			case ListUpButton:
			case ListDownButton:
			case ListCategoryTrigger:
				ListViewClearEditState ();
				break;
			}
		}

	else if (event->eType == ctlExitEvent)
		{
		switch (event->data.ctlExit.controlID)
			{
			case ListNewToDoButton:
			case ListDetailsButton:
				ListViewRestoreEditState ();
				break;
			}
		}


	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case ListUpButton:
				ListViewScroll (winUp);
				break;

			case ListDownButton:
				ListViewScroll (winDown);
				break;
			}
		}


	else if (event->eType == tblSelectEvent)
		{
		ListViewItemSelected (event);
		handled = true;
		}


	else if (event->eType == tblEnterEvent)
		{
		if (ItemSelected)
			{
			table = GetObjectPtr (ListTable);
			if (TblFindRowID (table, CurrentRecord, &row))
				{
				if (event->data.tblEnter.row != row)
					handled = ListViewClearEditState ();
				}
			}
		}


	else if (event->eType == tblExitEvent)
		{
		ListViewClearEditState ();
		handled = true;
		}

	else if (event->eType == fldHeightChangedEvent)
		{
		ListViewResizeDescription (event);
		handled = true;
		}


	else if (event->eType == menuOpenEvent)
		{
		if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
			{
			MenuHideItem(SendRecordCmd);
			MenuHideItem(SendCategoryCmd);
			}
		else
			{
			MenuShowItem(SendRecordCmd);
			MenuShowItem(SendCategoryCmd);
			}
		// don't set handled = true
		}

	else if (event->eType == menuEvent)
		{
		handled = ListViewDoCommand (event->data.menu.itemID);
		}


	// Add the buttons that we want available on the command bar, based on the current context
	else if (event->eType == menuCmdBarOpenEvent)
	{

		if (ItemSelected)
		{
			FieldType* fldP;
			UInt16 startPos, endPos;

			fldP = TblGetCurrentField(GetObjectPtr(ListTable));
			FldGetSelection(fldP, &startPos, &endPos);

			if (startPos == endPos)  // there's no highlighted text, but an item is chosen
			{
				// Call directly Field event handler so that system edit buttons are added if applicable
				FldHandleEvent(fldP, event);

				MenuCmdBarAddButton(menuCmdBarOnRight, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, SecurityCmd, 0);
				MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, BeamRecordCmd, 0);

				// Prevent the field package to add edit buttons again
				event->data.menuCmdBarOpen.preventFieldButtons = true;
			}
		}
		else	// no item is chosen
		{
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, SecurityCmd, 0);
		}

		// don't set handled to true; this event must fall through to the system.
	}


	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		ListViewInit (frm);
		FrmDrawForm (frm);
		if (PendingUpdate)
			{
			ListViewUpdateDisplay (PendingUpdate);
			PendingUpdate = 0;
			}
		ListViewRestoreEditState ();
		handled = true;
		}


	else if (event->eType == frmGotoEvent)
		{
		frm = FrmGetActiveForm ();
		ListViewGotoItem (event);
		ListViewInit (frm);
		FrmDrawForm (frm);
		ListViewRestoreEditState ();
		handled = true;
		}


	else if (event->eType == frmUpdateEvent)
		{
		handled = ListViewUpdateDisplay (event->data.frmUpdate.updateCode);
		if (handled && (event->data.frmUpdate.updateCode != frmRedrawUpdateCode))
			ListViewRestoreEditState ();
		}


	else if (event->eType == frmSaveEvent)
		{
		ListViewClearEditState ();
		// This deletes empty items. It can do this because we don't do a FrmSaveAllForms
		// on a sysAppLaunchCmdSaveData launch.
		}

	return (handled);
}



//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    CustomAcceptBeamDialog
 *
 * DESCRIPTION: This routine uses uses a new exchange manager function to
 *				Ask the user if they want to accept the data as well as set
 *				the category to put the data in. By default all data will go
 *				to the unfiled category, but the user can select another one.
 *				We store the selected category index in the appData field of
 *				the exchange socket so we have it at the when we get the receive
 *				data launch code later.
 *
 * PARAMETERS:  dbP - open database that holds category information
 *				askInfoP - structure passed on exchange ask launchcode
 *
 * RETURNED:    Error if any
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			bhall	9/7/99	Initial Revision
 *			gavin 11/9/99  Rewritten to use new ExgDoDialog function
 *
 ***********************************************************************/
static Err CustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
{
	ExgDialogInfoType	exgInfo;
	Err err;
	Boolean result;

	// set default category to unfiled
	exgInfo.categoryIndex = dmUnfiledCategory;
	// Store the database ref into a gadget for use by the event handler
	exgInfo.db = dbP;

	// Let the exchange manager run the dialog for us
	result = ExgDoDialog(askInfoP->socketP, &exgInfo, &err);


	if (!err && result) {

		// pretend as if user hit OK, we'll now accept the data
		askInfoP->result = exgAskOk;

		// Stuff the category index into the appData field
		askInfoP->socketP->appData = exgInfo.categoryIndex;
	} else {
		// pretend as if user hit cancel, we won't accept the data
		askInfoP->result = exgAskCancel;
	}

	return err;
}


//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    ApplicationHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and sets the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/11/95	Initial Revision
 *			jmp	9/17/99	Use NewNoteView instead of NoteView.
 *
 ***********************************************************************/
static Boolean ApplicationHandleEvent (EventPtr event)
{
	UInt16 formID;
	FormPtr frm;

	if (event->eType == frmLoadEvent)
		{
		// Load the form resource.
		formID = event->data.frmLoad.formID;
		frm = FrmInitForm (formID);
		FrmSetActiveForm (frm);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmDispatchEvent each time is receives an
		// event.
		switch (formID)
			{
			case ListView:
				FrmSetEventHandler (frm, ListViewHandleEvent);
				break;

			case NewNoteView:
				FrmSetEventHandler (frm, NoteViewHandleEvent);
				break;

			case DetailsDialog:
				FrmSetEventHandler (frm, DetailsHandleEvent);
				break;

			case OptionsDialog:
				FrmSetEventHandler (frm, OptionsHandleEvent);
				break;
			}
		return (true);
		}
	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    EventLoop
 *
 * DESCRIPTION: This routine is the event loop for the ToDo
 *              aplication.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void EventLoop (void)
{
	UInt16 error;
	EventType event;

	do
		{
		EvtGetEvent (&event, evtWaitForever);

		if (! SysHandleEvent (&event))

			if (! MenuHandleEvent (CurrentMenu, &event, &error))

				if (! ApplicationHandleEvent (&event))

					FrmDispatchEvent (&event);
		}

	while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the ToDo
 *              application.
 *
 * PARAMETERS:  cmd			 - launch code
 *              cmdPBP      - paramenter block (launch code specific)
 *              launchFlags - SysAppLaunch flags (ses SystemMgr.h)
 *
 * RETURNED:    error code
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			jmp	10/4/99	Replace calls to DmOpenDatabaseByTypeCreator() with
 *								ToDoGetDatabase().
 *			jmp	10/18/99	If the default database image doesn't exist, then create
 *								an empty database.
 *			jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *								since it was already being done in sysAppLaunchCmdExgReceiveData if
 *								the user affirmed sysAppLaunchCmdExgAskUser.  Also, in sysAppLaunchCmdExgReceiveData
 *								prevent call FrmSaveAllForms() if we're being call back through
 *								PhoneNumberLookup() as the two tasks are incompatible with each other.
 *			ABa 07/04/00	Add the todoLaunchCmdImportVObject launch code (DateBook uses it to import vtodo)
 *
 ***********************************************************************/
UInt32	PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;
	DmOpenRef dbP;

	// Normal Launch
	if (cmd == sysAppLaunchCmdNormalLaunch)
		{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdNormalLaunch"));
		//debug_setsyslevel(NULL, DEBUG_INFO);
		//debug_init("log.txt");
		//debug(DEBUG_INFO, "SYS", "NormalLaunch begin");
		
		error = StartApplication ();
		if (error != errNone)
			return (error);

		FrmGotoForm (ListView);
		EventLoop ();
		StopApplication ();
		
		//debug(DEBUG_INFO, "SYS", "NormalLaunch end");
		//debug_close();
		}

	else if (cmd == sysAppLaunchCmdFind)
		{
		Search ((FindParamsPtr)cmdPBP);
		}


	// This action code is sent to the app when the user hit the "Go To"
	// button in the Find Results dialog.
	else if (cmd == sysAppLaunchCmdGoTo)
		{
		Boolean	launched;
		launched = launchFlags & sysAppLaunchFlagNewGlobals;

		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdGoTo"));

		if (launched)
			{
			error = StartApplication ();
			if (error != errNone)
				return (error);

			GoToItem ((GoToParamsPtr) cmdPBP, launched);

			EventLoop ();
			StopApplication ();
			}
		else
			GoToItem ((GoToParamsPtr) cmdPBP, launched);
		}


	// Launch code sent to running app before sysAppLaunchCmdFind
	// or other action codes that will cause data searches or manipulation.
	// We don't need to respond to this launch code because items are
	// edited in place.
//	else if (cmd == sysAppLaunchCmdSaveData)
//		{
//		FrmSaveAllForms ();
//		}


	else if (cmd == sysAppLaunchCmdSyncNotify)
		{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdSyncNotify"));

		SyncNotification ();
		}


   // This launch code is sent after the system is reset.  We use this time
   // to create our default database.  If there is no default database image,
   // then we create an empty database.
	else if (cmd == sysAppLaunchCmdSystemReset)
		{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdSystemReset"));

		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
			{
				error = CreateDefaultDatabase();
			}
		RegisterLocaleChangingNotification();
		}


	else if (cmd == sysAppLaunchCmdExgAskUser)
   	{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgAskUser"));

      // if our app is not active, we need to open the database
      // the subcall flag is used here since this call can be made without launching the app
      if (!(launchFlags & sysAppLaunchFlagSubCall))
      	{
      	error = ToDoGetDatabase (&dbP, dmModeReadWrite);
      	}
      else
      	dbP = ToDoDB;

      if (dbP != NULL)
      	{
      	CustomAcceptBeamDialog (dbP, (ExgAskParamPtr) cmdPBP);

         if (!(launchFlags & sysAppLaunchFlagSubCall))
				error = DmCloseDatabase(dbP);
			}
		}


   // Receive the record.  The app will parse the data and add it to the database.
   // This data should be displayed by the app.
   // ABa: todoLaunchCmdImportVObject added in Palm OS 4.0.
	else if (cmd == sysAppLaunchCmdExgReceiveData || cmd == todoLaunchCmdImportVObject)
   	{
      UInt32 currentUID;

		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdExgReceiveData or todoLaunchCmdImportVObject"));

      // if our app is not active, we need to open the database
      // the subcall flag is used here since this call can be made without launching the app
      if (!(launchFlags & sysAppLaunchFlagSubCall))
      	{
        	error = ToDoGetDatabase (&dbP, dmModeReadWrite);
      	}
      else
      	{
      	dbP = ToDoDB;

			// We don't delete the current record if it's empty because the user
			// could cancel the beam receive.

			// ToDoReceiveData() inserts the received record in sorted order. This may change the
			// index of the current record. So we remember its UID here, and refresh our copy of
			// its index afterwards.
			if (CurrentRecord != noRecordSelected)
				DmRecordInfo(dbP, CurrentRecord, NULL, &currentUID, NULL);
			}

      if (dbP != NULL)
      	{
      	if (cmd != todoLaunchCmdImportVObject)
				error = ToDoReceiveData(dbP, (ExgSocketPtr) cmdPBP);
			else
				{
					UInt32 uniqueID;
					PdiReaderType* reader = (PdiReaderType*) cmdPBP;
					// ABa here is code to import just one VTODO
					TraceOutput(TL(appErrorClass, "ToDo App:PilotMain:todoLaunchCmdImportVObject"));
					ToDoImportVToDo(dbP, reader->pdiRefNum, reader, false, true, &uniqueID);
					ToDoSetGoToParams (dbP, reader->appData, uniqueID);
				}

	      if (launchFlags & sysAppLaunchFlagSubCall)
	      	{
				if (CurrentRecord != noRecordSelected)
					{
					if (DmFindRecordByID(dbP, currentUID, &CurrentRecord) != 0) {
						ItemSelected = false;
						CurrentRecord = noRecordSelected;	// Can't happen, but...
					}
	      		}
	      	}

			if (!(launchFlags & sysAppLaunchFlagSubCall))
				DmCloseDatabase(dbP);
			}
		else
			error = exgErrAppError;
		// If we can't open our database, return the error since it wasn't passed to ExgDisconnect
		return error;
		}


	// This action code is sent by the DesktopLink server when it creates
	// a new database.  We will initializes the new database.
	else if (cmd == sysAppLaunchCmdInitDatabase)
		{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdInitDatabase"));

		ToDoAppInfoInit (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);

		// Set the backup bit.  This is to aid syncs with non Palm software.
		ToDoSetDBBackupBit(((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
		}
	else if (cmd == sysAppLaunchCmdNotify)
		{
		TraceOutput(TL(appErrorClass, "PilotMain() - sysAppLaunchCmdNotify"));

			if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyLocaleChangedEvent)
				{
				DmSearchStateType	searchState;
				LocalID	dbID;
				UInt16	cardNo;

				// Since the locale has changed, delete the existing database
				// and re-create it for the new locale
				error = DmGetNextDatabaseByTypeCreator (true, &searchState, toDoDBType,
			          		sysFileCToDo, true, &cardNo, &dbID);
				if (error == errNone)
					DmDeleteDatabase(cardNo, dbID);
				error = CreateDefaultDatabase();

				}
		}


	return (errNone);
}


/***********************************************************************
 *
 * FUNCTION:    ToDoLoadPrefs
 *
 * DESCRIPTION: Read the preferences and handle previous and future
 *					 versions of the prefs.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	01/08/98	Initial Revision
 *			kwk	06/23/99	Use glue code for default Note/ListFont if prefs
 *								don't provide that information.
 *			grant	01/18/01	Only change noteFont from largeFont to largeBoldFont
 *								if we are updating prefs from an earlier version.
 *
 ***********************************************************************/
void ToDoLoadPrefs(void)
{
	ToDoPreferenceType prefs;
	UInt16 prefsSize;
	Int16 prefsVersion;

	// Read the preferences / saved-state information. If we get an older version of
	// the prefs, sync our new note font field with the original pref field.
	prefsSize = sizeof (ToDoPreferenceType);
	prefsVersion = PrefGetAppPreferences (sysFileCToDo, todoPrefID, &prefs, &prefsSize, true);
	if (prefsVersion > toDoPrefsVersionNum)
	{
		prefsVersion = noPreferenceFound;
	}
	if (prefsVersion > noPreferenceFound)
	{
		if (prefsVersion <= toDoPrefsVerUpdateFonts)
		{
			prefs.noteFont = prefs.v20NoteFont;

			// Use the 'better' large font if we've got it, since version 2
			// prefs would have been created on an older version of the OS
			// which didn't have the largeBoldFont available.
			if (prefs.noteFont == largeFont)
				prefs.noteFont = largeBoldFont;
		}

		CurrentCategory = prefs.currentCategory;
		NoteFont = prefs.noteFont;
		ShowAllCategories = prefs.showAllCategories;
		ShowCompletedItems = prefs.showCompletedItems;
		ShowOnlyDueItems = prefs.showOnlyDueItems;
		ShowDueDates = prefs.showDueDates;
		ShowPriorities = prefs.showPriorities;
		ShowCategories = prefs.showCategories;
		ChangeDueDate = prefs.changeDueDate;
		SaveBackup = prefs.saveBackup;


		// Support transferal of preferences from the second version of the preferences.
		if (prefsVersion == toDoPrefsVersionNum)
		{
			ListFont = prefs.listFont;
		}
		else
		{
			ListFont = FntGlueGetDefaultFontID(defaultSystemFont);
		}
	}
	else
	{
		ListFont = FntGlueGetDefaultFontID(defaultSystemFont);
		NoteFont = FntGlueGetDefaultFontID(defaultSystemFont);
	}
}

/***********************************************************************
 *
 * FUNCTION:    ToDoSavePrefs
 *
 * DESCRIPTION: Save the preferences and handle previous and future
 *					 versions of the prefs.
 *
 * PARAMETERS:  Nothing.
 *
 * RETURNED:    Nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	1/8/98	Initial Revision
 *
 ***********************************************************************/
void ToDoSavePrefs(void)
{
	ToDoPreferenceType prefs;

	// Write the preferences / saved-state information.
	prefs.currentCategory = CurrentCategory;
	prefs.noteFont = NoteFont;
	if (prefs.noteFont > largeFont) {
		prefs.v20NoteFont = stdFont;
	}
	else {
		prefs.v20NoteFont = prefs.noteFont;
	}
	prefs.showAllCategories = ShowAllCategories;
	prefs.showCompletedItems = ShowCompletedItems;
	prefs.showOnlyDueItems = ShowOnlyDueItems;
	prefs.showDueDates = ShowDueDates;
	prefs.showPriorities = ShowPriorities;
	prefs.showCategories = ShowCategories;
	prefs.changeDueDate = ChangeDueDate;
	prefs.saveBackup = SaveBackup;
	prefs.listFont = ListFont;

	// Clear reserved field so prefs don't look "different" just from stack garbage!
	prefs.reserved = 0;

	PrefSetAppPreferences (sysFileCToDo, todoPrefID, toDoPrefsVersionNum, &prefs,
		sizeof (ToDoPreferenceType), true);
}
