/******************************************************************************
 *
 * Copyright (c) 1995-2002 PalmSource, Inc. All rights reserved.
 *
 * File: SampleMain.c
 *
 * Release: Palm OS 5 SDK (68K) R3.
 *
 * Description:
 *	 This app is based on PalmSource's Memo Pad sample app.
 *   It demonstrates how to use the palmOne document API.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <PalmCompatibility.h>
#include <PalmOSGlue.h>
#include <VFSMgr.h>

#include <FileBrowserLibCommon.h>
#include <PalmUtils.h>
#include <palmOne_68K.h>
#include <HsExt.h>
#include <HsNav.h>

#include "SampleDB.h"
#include "SampleRsc.h"
#include "SampleMain.h"



/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define sampleVersionNum					3
#define samplePrefsVersionNum				3
#define samplePrefID						0x00


#define newItemSize  					64

#define noRecordSelected				0xffff
#define fileSelected					0xfffe
#define noRecordSelectedID				-1

#define ftrPtrNumURL					0

#define kMaxFilenameLength				255

#define vfsVolumeAttrNonRemovable		(0x00000008UL)
	//<><> Remove when it becomes part of the SDK

// Update codes, used to determine how the to do list view should
// be redrawn.
#define updateRedrawAll					0x00
#define updateCategoryChanged			0x01
#define updateDisplayOptsChanged		0x02
#define updateFontChanged				0x04


//The listViewIndexStringSize is the size of the character array
//that holds the string representation of the index that is displayed to
//the left of the item title in the list view.  The string can have a
//range of 1 - 99,999 with the current value.
#define listViewIndexStringSize		7

/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/

typedef struct {
	UInt16			topVisibleRecord;
	UInt16			currentRecord;
	UInt16			currentView;
	UInt16			currentCategory;
	FontID			v20editFont;
	UInt8				reserved1;
	UInt16			editScrollPosition;
	Boolean			showAllCategories;
	UInt8				reserved2;
	UInt32			currentRecordID;
	Boolean			saveBackup;

	// Version 2 preferences
	FontID			v20listFont;

	// Version 3 preferences
	FontID			editFont;
	FontID			listFont;
} SamplePreferenceType;

typedef struct {
	DmOpenRef		db;
	Char *			categoryName;
	UInt16			categoryIndex;
} AcceptBeamType;

typedef enum {
	lineEndLF,
	lineEndCR,
	lineEndCRLF
} LineEndType;


/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/

static DmOpenRef				ItemDB;
static char						CategoryName [dmCategoryLength];
static UInt16					ItemsInCategory;
static privateRecordViewEnum	PrivateRecordVisualStatus;
static MenuBarPtr				CurrentMenu;

// The following global variable are saved to a state file.
static UInt16					CurrentVolRefNum = vfsInvalidVolRef;
static Char *					CurrentPath = NULL;
static LineEndType				CurrentLineEnd = lineEndLF;
static UInt16					TopVisibleRecord = 0;
static UInt16					CurrentRecord = noRecordSelected;
static UInt16					CurrentView = ListView;
static UInt16					CurrentCategory = dmAllCategories;
static Boolean					ShowAllCategories = true;
static FontID					ListFont = stdFont;
static FontID					EditFont = stdFont;
static UInt16					EditScrollPosition = 0;
static Boolean					SaveBackup = true;

//static Boolean					InPhoneLookup = false;

static UInt16					TopRowPositionInCategory;

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static Boolean EditViewDeleteRecord (void);
static void SampleLoadPrefs(UInt32*	currentRecordID);
static void SampleSavePrefs(UInt16 scrollPosition);
static void ListViewDisplayMask (RectanglePtr bounds);
static Boolean ListViewUpdateDisplay (UInt16 updateCode);
static void RegisterLocaleChangingNotification(void);
static Err OpenFile (UInt16 volRefNum, Char * path);

/***********************************************************************
 *
 * FUNCTION:     SetDBBackupBit
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database and set
 *					  the backup bit on it.
 *
 * PARAMETERS:   dbP -	the database to set backup bit,
 *								can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	4/1/99	Initial Revision
 *
 ***********************************************************************/
void SetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef localDBP;
	LocalID dbID;
	UInt16 cardNo;
	UInt16 attributes;
	Err error;
	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
	{
		localDBP = DmOpenDatabaseByTypeCreator (itemDBType, kAppCreator, dmModeReadWrite);
		if (localDBP == NULL)  return;
	}
	else
	{
		localDBP = dbP;
	}

	// now set the backup bit on localDBP
	error = DmOpenDatabaseInfo(localDBP, &dbID, NULL, NULL, &cardNo, NULL);
	if(error)
		return;
	error = DmDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				   NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if(error)
		return;
	attributes |= dmHdrAttrBackup;
	error = DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
					  NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	if(error)
		return;
	// close database if necessary
	if (dbP == NULL)
	{
		DmCloseDatabase(localDBP);
	}
}

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

			// Set the bakcup bit on the newly created DB.
			SetDBBackupBit(NULL);
		}
	}

	// If there is no default data, or we had a problem creating it,
	// then attempt to create an empty database.
	if (!resH || error)
	{
		error = SampleGetDatabase (&dbP, dmModeReadWrite);

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
 *			grant 4/6/99	Moved code to set backup bit into SetDBBackupBit
 *			jmp	10/2/99	Call new SampleGetDataBase() to create database
 *								if it doesn't already exist.
 *
 ***********************************************************************/
static UInt16 StartApplication (void)
{
	Err err = 0;
	UInt16 attr;
	UInt16 mode;
	UInt32 uniqueID;
	UInt32 currentRecordID = 0;
	Boolean recordFound = false;

#if EMULATION_LEVEL != EMULATION_NONE
	RegisterLocaleChangingNotification();
#endif
	// Determime if secret record should be shown.
	PrivateRecordVisualStatus = (privateRecordViewEnum)PrefGetPreference (prefShowPrivateRecords);
	if (PrivateRecordVisualStatus == hidePrivateRecords)
	{
		mode = dmModeReadWrite;
	}
	else
	{
		mode = dmModeReadWrite | dmModeShowSecret;
	}

	// Find the application's data file.  If it doesn't exist, create it.
	err = SampleGetDatabase (&ItemDB, mode);
	if (err)
		return err;

	// Read the preferences.
	SampleLoadPrefs(&currentRecordID);

	// The file may have been synchronized since the last time we used it,
	// check that the current record and the currrent category still
	// exist.  Also, if secret records are being hidden, check if the
	// the current record is marked secret.
	CategoryGetName (ItemDB, CurrentCategory, CategoryName);
	if (*CategoryName == 0)
	{
		CurrentCategory = dmAllCategories;
		ShowAllCategories = true;
	}

	if ( DmQueryRecord (ItemDB, CurrentRecord) != 0)
	{
		DmRecordInfo (ItemDB, CurrentRecord, &attr, &uniqueID, NULL);
		recordFound = (uniqueID == currentRecordID) &&
			((PrivateRecordVisualStatus == showPrivateRecords) ||
			 (!(attr & dmRecAttrSecret)));
	}

	if (! recordFound)
	{
		TopVisibleRecord = 0;
		CurrentRecord = noRecordSelected;
		CurrentView = ListView;
		EditScrollPosition = 0;
	}

	if (ShowAllCategories)
		ItemsInCategory = DmNumRecordsInCategory (ItemDB, dmAllCategories);
	else
		ItemsInCategory = DmNumRecordsInCategory (ItemDB, CurrentCategory);

	return (err);
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
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void StopApplication (void)
{
	UInt16 scrollPosition = 0;
	FormPtr frm;
	FieldPtr fld;

	// If we are in the "edit view", get the current scroll position.
	if ((CurrentView == EditView) && (CurrentRecord != noRecordSelected) &&
		(CurrentRecord != fileSelected))
	{
		frm = FrmGetFormPtr (EditView);
		fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, EditItemField));
		scrollPosition = FldGetScrollPosition (fld);
	}

	// Close all open forms,  this will force any unsaved data to
	// be written to the database.
	FrmCloseAllForms ();

	// Write the preferences / saved-state information.
	SampleSavePrefs(scrollPosition);

	// Close the application's data file.
	DmCloseDatabase (ItemDB);
	
	// Free the path, if any
	if (CurrentPath)
		MemPtrFree (CurrentPath);
}


/***********************************************************************
 *
 * FUNCTION:    SyncNotification
 *
 * DESCRIPTION: This routine is an entry point of the sample application.
 *              It is called when the application's database is
 *              synchronized.  This routine will resort the database.
 *
 * PARAMETERS:	 nothing
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/17/96	Initial Revision
 *		jmp	10/02/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								SampleGetDatabase().
 *
 ***********************************************************************/
static void SyncNotification (void)
{
	DmOpenRef dbP;
	Err err;

	// Find the application's data file.
	err = SampleGetDatabase(&dbP, dmModeReadWrite);
	if (err)
		return;

	// Resort the database.
	SampleSort (dbP);

	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:     RegisterLocaleChangingNotification

 *
 * DESCRIPTION:  Register for NotifyMgr notifications for locale chagning.
 */
 /*
 *
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
void RegisterLocaleChangingNotification(void)
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
 * FUNCTION:    RegisterData
 *
 * DESCRIPTION: Register with the Exchange Manager to receive .txt and
 *				text/plain objects.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		7/28/00		dje	Initial Revision.
 *
 ***********************************************************************/
static void RegisterData(void)
{
	MemHandle resH;
	void *desc;

	resH = DmGetResource(strRsc, ExgDescriptionStr);
	desc = MemHandleLock(resH);
	ExgRegisterDatatype(kAppCreator, exgRegExtensionID, sampleExtension, desc, 0);
	ExgRegisterDatatype(kAppCreator, exgRegTypeID, sampleMIMEType, desc, 0);
	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}


/***********************************************************************
 *
 * FUNCTION:    RegisterFileBrowser
 *
 * DESCRIPTION: Register with the Exchange Manager to edit .txt and
 *              text/plain objects (typically files).
 *
 * PARAMETERS:  none
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *		4/6/04		dje	Initial Revision.
 *
 ***********************************************************************/
static void RegisterFileBrowser(void)
{
	MemHandle resH;
	void *desc;

	resH = DmGetResource(strRsc, FileDescriptionStr);
	desc = MemHandleLock(resH);
	// The resource IDs for LargeDocumentIconBitmapFamily and SmallDocumentIconBitmapFamily are in this string
	ExgRegisterDatatype (kAppCreator, exgRegEditExtensionID, sampleExtension, desc, 0);
	ExgRegisterDatatype (kAppCreator, exgRegEditTypeID, sampleMIMEType, desc, 0);
	MemHandleUnlock(resH);
	DmReleaseResource(resH);
}


/***********************************************************************
 *
 * FUNCTION:    SampleLoadPrefs
 *
 * DESCRIPTION: Load the preferences and do any fixups needed for backwards
 *					 and forwards compatibility
 *
 * PARAMETERS:  currentRecordID <- returned record id from preferences.
 *
 * RETURNED:    nothing
 *
 *	HISTORY:
 *		01/13/98	BGT	Initial Revision.
 *		08/04/99	kwk	Cleaned up setting EditFont/ListFont from prefs.
 *
 ***********************************************************************/
void SampleLoadPrefs(UInt32*	currentRecordID)
{
	SamplePreferenceType prefs;
	UInt16 prefsSize;
	Int16 prefsVersion;
	Boolean needFontInfo = false;

	// Read the preferences / saved-state information.
	prefsSize = sizeof (SamplePreferenceType);
	prefsVersion = PrefGetAppPreferences (kAppCreator, samplePrefID, &prefs, &prefsSize, true);
	if (prefsVersion > samplePrefsVersionNum) {
		prefsVersion = noPreferenceFound;
	}

	if (prefsVersion > noPreferenceFound)
	{
		// Try to carry forward the version 2 preferences for the font
		if (prefsVersion < 2)
		{
			// No font data in original prefs
			needFontInfo = true;
		}
		else if (prefsVersion == 2)
		{
			prefs.editFont = prefs.v20editFont;
			prefs.listFont = prefs.v20listFont;

			// Use the 'better' large font if we've got it, since version 2
			// prefs would have been created on an older version of the OS
			// which didn't have the largeBoldFont available.
			if (prefs.editFont == largeFont)
				prefs.editFont = largeBoldFont;

			if (prefs.listFont == largeFont)
				prefs.listFont = largeBoldFont;
		}

		TopVisibleRecord = prefs.topVisibleRecord;
		CurrentRecord = prefs.currentRecord;
		CurrentView = prefs.currentView;
		CurrentCategory = prefs.currentCategory;
		EditScrollPosition = prefs.editScrollPosition;
		ShowAllCategories = prefs.showAllCategories;
		SaveBackup = prefs.saveBackup;
		*currentRecordID = prefs.currentRecordID;
	}
	else
	{
		needFontInfo = true;
	}

	// If the prefs didn't supply us with font info, we'll need to get it ourselves.
	if (needFontInfo)
	{
		UInt32 defaultFont;
		FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
		EditFont = (FontID)defaultFont;
		ListFont = (FontID)defaultFont;
	}
	else
	{
		EditFont = prefs.editFont;
		ListFont = prefs.listFont;
	}

	// The first time this app starts register to handle .txt and text/plain.
	if (prefsVersion != samplePrefsVersionNum)
	{
		RegisterData();
		RegisterFileBrowser();
	}
}


/***********************************************************************
 *
 * FUNCTION:    SampleSavePrefs
 *
 * DESCRIPTION: Save the preferences and do any fixups needed for backwards
 *					 and forwards compatibility
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			BGT	1/13/98	Initial Revision
 *
 ***********************************************************************/
void SampleSavePrefs(UInt16 scrollPosition)
{
	SamplePreferenceType prefs;
	UInt32 uniqueID;

	// Write the preferences / saved-state information.
	prefs.topVisibleRecord = TopVisibleRecord;
	prefs.currentRecord = CurrentRecord;
	prefs.currentView = CurrentView;
	prefs.currentCategory = CurrentCategory;
	prefs.showAllCategories = ShowAllCategories;
	prefs.editScrollPosition = scrollPosition;
	prefs.saveBackup = SaveBackup;
	prefs.editFont = EditFont;
	prefs.listFont = ListFont;

	prefs.v20editFont = stdFont;
	prefs.v20listFont = stdFont;

	// Clear reserved fields so prefs don't look "different" just from stack garbage!
	prefs.reserved1 = 0;
	prefs.reserved2 = 0;

	// Get the current record's unique id and save it with the state
	// information.
	if ( DmQueryRecord (ItemDB, CurrentRecord) != 0)
	{
		DmRecordInfo (ItemDB, CurrentRecord, NULL, &uniqueID, NULL);
		prefs.currentRecordID = uniqueID;
	}
	else
		prefs.currentRecordID = noRecordSelectedID;

	// Write the state information.
	PrefSetAppPreferences (kAppCreator, samplePrefID, samplePrefsVersionNum, &prefs,
						   sizeof (SamplePreferenceType), true);


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
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/
static void * GetObjectPtr (UInt16 objectID)
{
	FormPtr frm;

	frm = FrmGetActiveForm ();
	return (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, objectID)));

}

/***********************************************************************
 *
 * FUNCTION:    GetFocusObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to the field object, in
 *              the current form, that has the focus.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    pointer to a field object or NULL of there is no fucus
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static FieldPtr GetFocusObjectPtr (void)
{
	FormPtr frm;
	UInt16 focus;

	frm = FrmGetActiveForm ();
	focus = FrmGetFocus (frm);
	if (focus == noFocus)
		return (NULL);

	return (FrmGetObjectPtr (frm, focus));
}


/***********************************************************************
 *
 * FUNCTION:    SeekRecord
 *
 * DESCRIPTION: Given the index of a 'to do' record, this routine scans
 *              forwards or backwards for displayable 'to do' records.
 *
 * PARAMETERS:  indexP  - pointer to the index of a record to start from;
 *                        the index of the record sought is returned in
 *                        this parameter.
 *
 *              offset  - number of records to skip:
 *                        	0 - mean seek from the current record to the
 *                             next display record, if the current record is
 *                             a displayable record, its index is retuned.
 *                         1 - mean seek foreward, skipping one displayable
 *                             record
 *                        -1 - menas seek backwards, skipping one
 *                             displayable record
 *
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/5/95	Initial Revision
 *
 ***********************************************************************/
static Boolean SeekRecord (UInt16 * indexP, Int16 offset, Int16 direction)
{
	DmSeekRecordInCategory (ItemDB, indexP, offset, direction, CurrentCategory);
	if (DmGetLastErr()) return (false);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    ChangeCategory
 *
 * DESCRIPTION: This routine updates the global varibles that keep track
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
	if (ShowAllCategories)
		ItemsInCategory = DmNumRecordsInCategory (ItemDB, dmAllCategories);
	else
		ItemsInCategory = DmNumRecordsInCategory (ItemDB, category);

	CurrentCategory = category;
	TopVisibleRecord = 0;
}


/***********************************************************************
 *
 * FUNCTION:    DrawItemTitle
 *
 * DESCRIPTION: This routine draws the title of the specified item.
 *
 * PARAMETERS:	 item  - pointer to an item
 *              x     - draw position
 *              y     - draw position
 *              width - maximum width to draw.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	04/18/95	Initial Revision
 *			roger	07/27/95	Combined both cases
 *			kwk	05/15/99	Use Int'l code for truncation of title.
 *
 ***********************************************************************/
static void DrawItemTitle (Char * item, Int16 x, Int16 y, Int16 width)
{
	Char * ptr = StrChr (item, linefeedChr);
	UInt16 titleLen = (ptr == NULL ? StrLen (item) : (UInt16) (ptr - item));
	if (FntWidthToOffset (item, titleLen, width, NULL, NULL) == titleLen)
	{
		WinDrawChars (item, titleLen, x, y);
	}
	else
	{
		Int16 titleWidth;
		titleLen = FntWidthToOffset (item, titleLen, width - FntCharWidth (chrEllipsis), NULL, &titleWidth);
		WinDrawChars (item, titleLen, x, y);
		WinDrawChar (chrEllipsis, x + titleWidth, y);
	}
}


/***********************************************************************
 *
 * FUNCTION:    ReplaceTwoColors
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
static void ReplaceTwoColors (const RectangleType *rP, UInt16 cornerDiam,
							  UIColorTableEntries oldForeground, UIColorTableEntries oldBackground,
							  UIColorTableEntries newForeground, UIColorTableEntries newBackground)
{
	UInt8 oldForegroundIndex = UIColorGetTableEntryIndex(oldForeground);
	UInt8 oldBackgroundIndex = UIColorGetTableEntryIndex(oldBackground);
	UInt8 newForegroundIndex = UIColorGetTableEntryIndex(newForeground);
	UInt8 newBackgroundIndex = UIColorGetTableEntryIndex(newBackground);

	WinPushDrawState();
	WinSetDrawMode(winSwap);
	WinSetPatternType (blackPattern);

	if (newBackgroundIndex == oldForegroundIndex)
		if (newForegroundIndex == oldBackgroundIndex)
		{
			// Handle the case when foreground and background colors change places,
			// such as on black and white systems, with a single swap.
			WinSetBackColor(oldBackgroundIndex);
			WinSetForeColor(oldForegroundIndex);
			WinPaintRectangle(rP, cornerDiam);
		}
		else
		{
			// Handle the case when the old foreground and the new background
			// are the same, using two swaps.
			WinSetBackColor(oldForegroundIndex);
			WinSetForeColor(oldBackgroundIndex);
			WinPaintRectangle(rP, cornerDiam);
			WinSetBackColor(oldBackgroundIndex);
			WinSetForeColor(newForegroundIndex);
			WinPaintRectangle(rP, cornerDiam);
		}
	else if (oldBackgroundIndex == newForegroundIndex)
	{
		// Handle the case when the old background and the new foreground
		// are the same, using two swaps.
		WinSetBackColor(newForegroundIndex);
		WinSetForeColor(oldForegroundIndex);
		WinPaintRectangle(rP, cornerDiam);
		WinSetBackColor(newBackgroundIndex);
		WinSetForeColor(oldForegroundIndex);
		WinPaintRectangle(rP, cornerDiam);
	}
	else
	{
		// Handle the case when no two colors are the same, as is typically the case
		// on color systems, using two swaps.
		WinSetBackColor(oldBackgroundIndex);
		WinSetForeColor(newBackgroundIndex);
		WinPaintRectangle(rP, cornerDiam);
		WinSetBackColor(oldForegroundIndex);
		WinSetForeColor(newForegroundIndex);
		WinPaintRectangle(rP, cornerDiam);
	}

	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    Search
 *
 * DESCRIPTION: This routine searchs the sample database for records
 *              containing the string passed.
 *
 * PARAMETERS:	findParams - text search parameter block
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		04/18/95	Initial Revision
 *			roger	07/26/95	converted to modern search mechanism
 *			kwk		05/15/99	Use TxtFindString, save match length in custom param.
 *			jmp		10/01/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								SampleGetDatabase().
 *			jmp		10/21/99	Previous change caused bug #22965, but previous code
 *								caused yet another problem.  Fixed #22965 by using
 *								everyone else's way:  Call DmGetNextDatabaseByTypeCreator()
 *								first, then call DmOpenDatabase() if all is well.
 *
 ***********************************************************************/
static void Search (FindParamsPtr findParams)
{
	UInt16 pos;
	Char * header;
	UInt16 recordNum;
	MemHandle recordH;
	MemHandle headerStringH;
	RectangleType r;
	Boolean done;
	Boolean match;
	DmOpenRef dbP;
	DmSearchStateType searchState;
	Err err;
	UInt16 cardNo = 0;
	LocalID dbID;
	ItemDBRecordPtr itemRecP;
	UInt32 longPos;
	UInt16 matchLength;

	// Find the application's data file.
	err = DmGetNextDatabaseByTypeCreator (true, &searchState, itemDBType,
										  kAppCreator, true, &cardNo, &dbID);
	if (err)
	{
		findParams->more = false;
		return;
	}

	// Open the Sample database.
	dbP = DmOpenDatabase(cardNo, dbID, findParams->dbAccesMode);
	if (!dbP)
	{
		findParams->more = false;
		return;
	}

	// Display the heading line.
	headerStringH = DmGetResource(strRsc, FindItemHeaderStr);
	header = MemHandleLock(headerStringH);
	done = FindDrawHeader(findParams, header);
	MemHandleUnlock(headerStringH);
	DmReleaseResource(headerStringH);
	if (done)
		goto Exit;

	// Search the items for the "find" string.
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
		// secret the check doesn't happen.  Generally this shouldn't be a
		// problem since if most of the records are secret then the search
		// won't take long anyways!
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

		itemRecP = MemHandleLock (recordH);

		// Search for the string passed,  if it's found display the title
		// of the item.
		match = TxtFindString (&(itemRecP->note), findParams->strToFind, &longPos, &matchLength);
		pos = longPos;

		if (match)
		{
			// Add the match to the find paramter block,  if there is no room to
			// display the match the following function will return true.
			done = FindSaveMatch (findParams, recordNum, pos, 0, matchLength, cardNo, dbID);

			if (!done)
			{
				// Get the bounds of the region where we will draw the results.
				FindGetLineBounds (findParams, &r);

				// Display the title of the description.
				DrawItemTitle (&(itemRecP->note), r.topLeft.x+1, r.topLeft.y,
							   r.extent.x-2);

				findParams->lineNumber++;
			}
		}

		MemHandleUnlock(recordH);
		if (done) break;

		recordNum++;
	}

Exit:
	DmCloseDatabase (dbP);
}


/***********************************************************************
 *
 * FUNCTION:    IsAllWhitespace
 *
 * DESCRIPTION: This function checks if a string contains only white-
 *              space.
 *
 * PARAMETERS:  string - the string
 *
 * RETURNED:    whether the string is all whitespace
 *
 * REVISION HISTORY:
 *    Name  Date        Description
 *    ----  ----        -----------
 *    dje   2004.07.28  Initial revision
 *
 ***********************************************************************/
/*
static Boolean IsAllWhitespace(Char *string)
{
	UInt16 index = 0;
	WChar chr;
	
	while (true)
	{
		index += TxtGetNextChar(string, index, &chr);
		if (chr == chrNull)
			break;
		if (!TxtCharIsSpace(chr))
			return false;
	}
	return true;
}
*/

/***********************************************************************
 *
 * FUNCTION:    ParseFileURL
 *
 * DESCRIPTION: This routine parses a standard "file:" URL into a
 *              volRefNum and path. The caller should free the path with
 *              MemPtrFree.
 *
 * PARAMETERS:	 url - the URL to parse
 *               volRefNumP - pass back the volRefNum
 *
 * RETURNED:	 the path or NULL if the URL can't be parsed
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dje		06/04/04	Initial Revision
 *
 ***********************************************************************/
static Char *ParseFileURL (const Char *url, UInt16 *volRefNumP)
{
	Char *path = NULL;
	UInt16 refNum;
	Err err;
	
	err = SysLibFind(kFileBrowserLibName, &refNum);
	ErrFatalDisplayIf(err, "Can't find file browser lib");
	err = FileBrowserLibOpen(refNum);
	ErrFatalDisplayIf(err, "Can't open file browser lib");
	FileBrowserLibParseFileURL(refNum, url, volRefNumP, &path);
	FileBrowserLibClose(refNum);
	return path;
}


/***********************************************************************
 *
 * FUNCTION:    GoToItem
 *
 * DESCRIPTION: This routine is an entry point of the sample application.
 *              It is generally called as the result of hitting a
 *              "Go to" button in the text search dialog.  The record
 *              identifies by the parameter block passed will be display,
 *              with the character range specified highlighted.
 *
 * PARAMETERS:	 goToParams  - parameter block that identifies the record to
 *                             display.
 *              launchingApp - true if the application is being launched.
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	06/06/95	Initial Revision
 *			roger	07/26/95	converted to modern search mechanism
 *			kwk	05/15/99	Use saved match length in matchCustom field, so
 *								that it works with Japanese.
 *
 ***********************************************************************/
static Err GoToItem (GoToParamsPtr goToParams, Boolean launchingApp)
{
	UInt16 recordNum;
	UInt16 attr;
	UInt32 uniqueID;
	EventType event;
	UInt32 value;
	UInt16 volRefNum;
	Char *path = NULL;
	Err err;


	// If we're editing a file, the URL will be in a feature pointer
	if (FtrGet (kAppCreator, ftrPtrNumURL, &value) == errNone)
	{
		//path = ParseFileURL ((Char *)value, &volRefNum);
    path = ParseFileURL(pumpkin_heap_base() + value, &volRefNum);
		if (path)
		{
			err = OpenFile (volRefNum, path);
			MemPtrFree(path);
		}
		else
		{
			err = kFileBrowserLibErrCantFind;
		}
		FtrPtrFree (kAppCreator, ftrPtrNumURL);
		return err;
	}
	
	recordNum = goToParams->recordNum;
	if (!DmQueryRecord(ItemDB, recordNum))
	{
	
		if (!SeekRecord(&recordNum, 0, dmSeekBackward))
			if (!SeekRecord(&recordNum, 0, dmSeekForward))
			{
				FrmAlert(secGotoInvalidRecordAlert);
				FrmGotoForm(ListView);
				return errNone;
			}
	}
	DmRecordInfo(ItemDB, recordNum, &attr, &uniqueID, NULL);
	if ((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords)
	{
		FrmAlert(secGotoInvalidRecordAlert);
		FrmGotoForm(ListView);
		return errNone;
	}

	// Make the item the first item displayed.
	TopVisibleRecord = recordNum;

	// Change the current category if necessary.
	if (CurrentCategory != dmAllCategories)
	{
		ChangeCategory (attr & dmRecAttrCategoryMask);
	}


	// If the application is already running, close all open forms.  This
	// may cause in the database record to be reordered, so we'll find the
	// records index by its unique id.
	if (! launchingApp)
	{
		FrmCloseAllForms ();
		DmFindRecordByID (ItemDB, uniqueID, &recordNum);
	}


	// Send an event to goto a form and select the matching text.
	MemSet (&event, sizeof(EventType), 0);

	event.eType = frmLoadEvent;
	event.data.frmLoad.formID = EditView;
	EvtAddEventToQueue (&event);

	event.eType = frmGotoEvent;
	event.data.frmGoto.recordNum = recordNum;
	event.data.frmGoto.matchPos = goToParams->matchPos;
	event.data.frmGoto.matchLen = goToParams->matchCustom;
	event.data.frmGoto.matchFieldNum = goToParams->matchFieldNum;
	event.data.frmGoto.formID = EditView;
	EvtAddEventToQueue (&event);

	CurrentView = EditView;
	
	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    CreateRecord
 *
 * DESCRIPTION: This routine creates a new item record.
 *
 * PARAMETERS:  none
 *
 * RETURNED:    true if the record was sucessfully created.
 *
 * HISTORY:
 *		10/03/95	art	Created by Art Lamb.
 *		10/11/95	kcr	Set initial graffiti upshift.
 *		09/25/99	kwk	No longer take initial character parameter.
 *		10/29/99	jmp	Eliminate compiler's "has no side-effect" warning.
 *
 ***********************************************************************/
static Boolean CreateRecord (void)
{
	MemPtr		p;
	Char			zero = 0;
	UInt32		offset = 0;
	UInt16		attr = dmUnfiledCategory;
	UInt16		index;
	MemHandle 	itemRec;

	// Add a new record at the end of the database.
	index = DmNumRecords (ItemDB);
	itemRec = DmNewRecord (ItemDB, &index, newItemSize);

	// If the allocate failed, display a warning.
	if (! itemRec)
	{
		FrmAlert (DeviceFullAlert);
		return (false);
	}


	p = MemHandleLock (itemRec);

	// Null terminate the new item string.
	DmWrite (p, offset, &zero, sizeof(Char));

	MemPtrUnlock (p);

	// Set the category of the new record to the current category.
	DmRecordInfo (ItemDB, index, &attr, NULL, NULL);
	attr &= ~dmRecAttrCategoryMask;
	if (CurrentCategory != dmAllCategories)
		attr |= CurrentCategory;
	DmSetRecordInfo (ItemDB, index, &attr, NULL);

	CurrentRecord = index;
	ItemsInCategory++;

	DmReleaseRecord (ItemDB, index, true);


	//	Set the graffiti state for an initial upshift of the
	//	first character of a new item.
	//GrfSetState (false, false, true);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    DeleteRecord
 *
 * DESCRIPTION: This routine deletes the specified item
 *
 * PARAMETERS:  index - index of record to delete
 *
 * RETURNED:    true if the record was sucessfully deleted.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	9/30/95	Initial Revision
 *
 ***********************************************************************/
/*
static Boolean DeleteRecord (UInt16 index)
{
	UInt16 ctlIndex;
	UInt16 buttonHit;
	FormPtr alert;
	Boolean saveBackup;

	// Display an alert to confirm the delete operation.
	alert = FrmInitForm (DeleteItemDialog);

	ctlIndex = FrmGetObjectIndex (alert, DeleteItemSaveBackup);
	FrmSetControlValue (alert, ctlIndex, SaveBackup);
	buttonHit = FrmDoDialog (alert);
	saveBackup = FrmGetControlValue (alert, ctlIndex);;

	FrmDeleteForm (alert);

	if (buttonHit != DeleteItemOk)
		return (false);

	SaveBackup = saveBackup;

	// Delete or archive the record.
	if (SaveBackup)
		DmArchiveRecord (ItemDB, index);
	else
		DmDeleteRecord (ItemDB, index);
	DmMoveRecord (ItemDB, index, DmNumRecords (ItemDB));

	return (true);
}
*/

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
 * FUNCTION:	GetFilename
 *
 * DESCRIPTION:	This function extracts the filename from a path.
 *
 * PARAMETERS:	path - the path
 *
 * RETURNED:	a pointer to the filename within the path
 *
 * REVISION HISTORY:
 *    Name  Date        Description
 *    ----  ----        -----------
 *    dje   2004.01.30  Initial revision
 *
 ***********************************************************************/
static Char * GetFilename (Char * path)
{
	Char * result = path;
	Char * nextSeparator;
	
	while (true)
	{
		nextSeparator = StrChr(result, '/');
		if (nextSeparator == NULL)
			break;
		result = nextSeparator + 1;
	}
	return result;
}


/***********************************************************************
 *
 * FUNCTION:	LineEndConvertToPalmOS
 *
 * DESCRIPTION:	This function replaces CR/LF pairs and CRs with LFs to
 *				conform to Palm OS conventions.
 *
 * PARAMETERS:	string - the string (modified in place)
 *
 * RETURNED:	original line end convention used
 *
 * REVISION HISTORY:
 *    Name  Date        Description
 *    ----  ----        -----------
 *    dje   2004.03.15  Initial revision
 *
 ***********************************************************************/
static LineEndType LineEndConvertToPalmOS (Char * string)
{
	LineEndType result = lineEndLF;
	Char * nextCR = string;
	
	while (true)
	{
		nextCR = StrChr (nextCR, chrCarriageReturn);
		if (nextCR == NULL)
			break;
		if (*(nextCR + 1) == chrLineFeed)
		{
			// Delete CR, leaving only LF
			MemMove (nextCR, nextCR + 1, StrLen(nextCR + 1) + 1);
			result = lineEndCRLF;
		}
		else
		{
			// Replace CR with LF
			*nextCR = chrLineFeed;
			result = lineEndCR;
		}
		nextCR++;
	}
	return result;
}


/***********************************************************************
 *
 * FUNCTION:	SaveFile
 *
 * DESCRIPTION:	This function saves an item, replacing LFs with CR/LF
 *				pairs or CRs, as necessary.
 *
 * PARAMETERS:	volRefNum - volume on which to save file
 *				path - path to save file to
 *				string - the string for the item to save (with LFs)
 *				lineEnd - line end convention to convert to
 *
 * RETURNED:	error code
 *
 * REVISION HISTORY:
 *    Name  Date        Description
 *    ----  ----        -----------
 *    dje   2004.03.17  Initial revision
 *
 ***********************************************************************/
static Err SaveFile (UInt16 volRefNum, Char *path, Char *string, LineEndType lineEnd)
{
	FileRef fileRef;
	UInt32 bytesWritten;
	Char * curPos = string;
	Char * nextLF;
	const Char crString[] = {chrCarriageReturn};
	const Char crlfString[] = {chrCarriageReturn, chrLineFeed};
	Err err;
	
	// Open the file, replacing the previous contents, if any
	err = VFSFileOpen (volRefNum, path, vfsModeWrite | vfsModeTruncate, &fileRef);
	if (!err)
	{
		if (lineEnd == lineEndLF)
		{
			// No line end conversion required
			err = VFSFileWrite (fileRef, StrLen (string), string, &bytesWritten);
		}
		else
		{ 
			while (true)
			{
				nextLF = StrChr (curPos, chrLineFeed);
				if (nextLF == NULL)
				{
					// Emit the final line
					err = VFSFileWrite (fileRef, StrLen (curPos), curPos, &bytesWritten);
					break;
				}
				
				// Emit the next line without the trailing LF
				err = VFSFileWrite (fileRef, nextLF - curPos, curPos, &bytesWritten);
				if (err)
					break;
				
				// Emit the appropriate line end
				if (lineEnd == lineEndCRLF)
					err = VFSFileWrite (fileRef, 2, crlfString, &bytesWritten);
				else
					err = VFSFileWrite (fileRef, 1, crString, &bytesWritten);
				if (err)
					break;
				
				curPos = nextLF + 1;
			}
		}
		VFSFileClose (fileRef);
	}
	
	// Report error to the user
	if (err)
		FrmAlert (SaveErrorAlert);
	
	return err;
}


#pragma mark ----
/***********************************************************************
 *
 * FUNCTION:    PreferencesApply
 *
 * DESCRIPTION: This routine applies the changes made in the Preferences
 *              Dialog
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    update code
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
static UInt16 PreferencesApply (void)
{
	UInt8		sortOrder;
	UInt16		updateCode = 0;

	// Update the sort order.  Reset the To Do list to the top.
	sortOrder = LstGetSelection (GetObjectPtr (PreferencesSortByList));

	if (SampleGetSortOrder (ItemDB) != sortOrder)
	{
		if (sortOrder == soAlphabetic)
		{
			if (FrmAlert (alphabeticSortAlert) == alphabeticSortNo)
				return (0);
		}

		SampleChangeSortOrder (ItemDB, sortOrder);

		updateCode = updateDisplayOptsChanged;
	}

	return (updateCode);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesInit
 *
 * DESCRIPTION: This routine initializes the Preferences Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
static void PreferencesInit (void)
{
	UInt16			sortOrder;
	Char *		label;
	ListPtr		lst;
	ControlPtr	ctl;


	// Set the trigger and popup list that indicates the sort order.
	sortOrder = SampleGetSortOrder (ItemDB);
	lst = GetObjectPtr (PreferencesSortByList);
	label = LstGetSelectionText (lst, sortOrder);
	ctl = GetObjectPtr (PreferencesSortByTrigger);
	CtlSetLabel (ctl, label);
	LstSetSelection (lst, sortOrder);
}


/***********************************************************************
 *
 * FUNCTION:    PreferencesHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Preferences
 *              Dialog Box" of the Sample application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/18/95	Initial Revision
 *
 ***********************************************************************/
static Boolean PreferencesHandleEvent (EventType * event)
{
	UInt16 updateCode;
	Boolean handled = false;
	FormPtr frm;

	if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
		case PreferencesOkButton:
			updateCode = PreferencesApply ();
			FrmReturnToForm (ListView);
			if (updateCode)
				FrmUpdateForm (ListView, updateCode);
			handled = true;
			break;

		case PreferencesCancelButton:
			FrmReturnToForm (ListView);
			handled = true;
			break;

		}
	}

	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		PreferencesInit ();
		FrmDrawForm (frm);
		handled = true;
	}

	return (handled);
}


#pragma mark ----
/***********************************************************************
 *
 * FUNCTION:    DetailsSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    index of the selected category.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static UInt16 DetailsSelectCategory (UInt16* category)
{
	Char* name;
	Boolean categoryEdited;

	name = (Char *)CtlGetLabel (GetObjectPtr (DetailsCategoryTrigger));

	categoryEdited = CategorySelect (ItemDB, FrmGetActiveForm (),
									 DetailsCategoryTrigger, DetailsCategoryList,
									 false, category, name, 1, categoryDefaultEditCategoryString);

	return (categoryEdited);
}



/***********************************************************************
 *
 * FUNCTION:    DetailsApply
 *
 * DESCRIPTION: This routine applies the changes made in the Details Dialog.
 *
 * PARAMETERS:  category - new catagory
 *
 * RETURNED:    code which indicates how the item was changed,  this
 *              code is sent as the update code, in the frmUpdate event.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/10/95	Initial Revision
 *			kcr	10/9/95	added 'private records' alert.
 *
 ***********************************************************************/
static UInt16 DetailsApply (UInt16 category, Boolean categoryEdited)
{
	UInt16		attr;
	UInt16		updateCode = 0;
	Boolean	dirty = false;
	Boolean	secret;


	// Get the category and secret attribute of the current record.
	DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);

	// Get the current setting of the secret checkbox.
	secret = (CtlGetValue (GetObjectPtr (DetailsSecretCheckbox)) != 0);

	// Has the secret attribute was been changed?
	if (((attr & dmRecAttrSecret) == dmRecAttrSecret) != secret)
	{
		if (secret)
		{
			attr |= dmRecAttrSecret;
			if (PrivateRecordVisualStatus == showPrivateRecords)
				FrmAlert (privateRecordInfoAlert);
		}
		else
			attr &= ~dmRecAttrSecret;
		dirty = true;
	}


	// Has the category been changed?
	if (CurrentCategory != category)
	{
		attr &= ~dmRecAttrCategoryMask;
		attr |= category;
		dirty = true;
		updateCode = updateCategoryChanged;
	}


	// If the current category was deleted, renamed, or merged with
	// another category, then the list view needs to be redrawn.
	if (categoryEdited)
	{
		CurrentCategory = category;
		updateCode |= updateCategoryChanged;
	}


	if (dirty)
	{
		attr |= dmRecAttrDirty;
		DmSetRecordInfo (ItemDB, CurrentRecord, &attr, NULL);
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
 *
 ***********************************************************************/
static void DetailsInit (void)
{
	UInt16		attr;
	UInt16 		category;
	Char*			name;
	Boolean 		secret;
	ControlPtr	ctl;

	// Get the category and secret attribute of the current record.
	DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;
	secret = attr & dmRecAttrSecret;


	// If the record is marked secret, turn on the secret checkbox.
	ctl = GetObjectPtr (DetailsSecretCheckbox);
	if (secret)
		CtlSetValue (ctl, true);
	else
		CtlSetValue (ctl, false);


	// Set the label of the category trigger.
	ctl = GetObjectPtr (DetailsCategoryTrigger);
	name = (Char *)CtlGetLabel (ctl);
	CategoryGetName (ItemDB, category, name);
	CategorySetTriggerLabel (ctl, name);
}


/***********************************************************************
 *
 * FUNCTION:    DetailsHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Details
 *              Dialog Box".
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *
 ***********************************************************************/

static Boolean DetailsHandleEvent (EventType * event)
{
	static UInt16 		category;
	static Boolean		categoryEdited;

	UInt16 					updateCode;
	FormPtr 				frm;
	Boolean 				handled = false;

	if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
		case DetailsOkButton:
			updateCode = DetailsApply (category, categoryEdited);
			FrmReturnToForm (EditView);
			if (updateCode)
				FrmUpdateForm (EditView, updateCode);
			handled = true;
			break;

		case DetailsCancelButton:
			if (categoryEdited)
				FrmUpdateForm (EditView, updateCategoryChanged);
			FrmReturnToForm (EditView);
			handled = true;
			break;

		case DetailsDeleteButton:
			if ( EditViewDeleteRecord ())
			{
				frm = FrmGetActiveForm();
				FrmEraseForm (frm);
				FrmDeleteForm (frm);
				FrmCloseAllForms ();
				FrmGotoForm (ListView);
			}
			handled = true;
			break;


		case DetailsCategoryTrigger:
			categoryEdited = DetailsSelectCategory (&category) || categoryEdited;
			handled = true;
			break;
		}
	}

	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		DetailsInit ();
		FrmDrawForm (frm);
		category = CurrentCategory;
		categoryEdited = false;
		handled = true;
	}

	return (handled);
}


#pragma mark ----
/***********************************************************************
 *
 * FUNCTION:    EditViewSetTitle
 *
 * DESCRIPTION: This routine formats and sets the title of the Edit View.
 *              If the Edit View is visible, the new title is drawn.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * HISTORY:
 *	02/21/95	art	Created by Art Labm.
 *	08/31/00	kwk	Re-wrote to use TxtParamString, versus hard-coding
 *					order of elements using '#' in template string.
 *	11/22/00	FPa	Added DmReleaseResource()
 *	11/28/00	CS	Use FrmSetTitle instead of FrmCopyTitle and leave it
 *					allocated until EditViewExit.  This removes the
 *					dependency that the form title in the resource has to
 *					be wide enough to accomodate the longest possible title.
 *	11/30/00	FPa	Fixed bug #46014
 *
 ***********************************************************************/
static void EditViewSetTitle (void)
{
	MemHandle titleTemplateH = NULL;
	Char * titleTemplateP = NULL;
	Char * title;
	Char * filename;
	Char * oldTitle;
	Char posStr[maxStrIToALen + 1];
	Char totalStr[maxStrIToALen + 1];
	UInt16 pos;
	FormType* formP;

	formP = FrmGetFormPtr(EditView);
	
	// If there's an old title, we must free it; nevertheless FrmSetTitle() uses the old title -> we need to free it *after* having called FrmSetTitle()
	oldTitle = (Char*)FrmGetTitle(formP);

	if (CurrentRecord == fileSelected)
	{
		filename = GetFilename (CurrentPath);
		title = MemPtrNew (StrLen (filename) + 1);
		ErrFatalDisplayIf (title == NULL, "Can't alloc title");
		StrCopy(title, filename);
	}
	else
	{
		// Format as strings, the item's postion within its category, and
		// the total number of items in the category.
		pos = DmPositionInCategory(	ItemDB,CurrentRecord, ShowAllCategories ? dmAllCategories : CurrentCategory);
		StrIToA(posStr, pos+1);
		StrIToA(totalStr, ItemsInCategory);

		titleTemplateH = DmGetResource(strRsc, EditTitleString);
		titleTemplateP = MemHandleLock(titleTemplateH);
		title = TxtParamString(titleTemplateP, posStr, totalStr, NULL, NULL);	// title needs to be freed
	}
	
	FrmSetTitle(formP, title);

	if (oldTitle != 0)
		MemPtrFree(oldTitle);

	if (titleTemplateP)
		MemHandleUnlock(titleTemplateH);
	if (titleTemplateH)
		DmReleaseResource(titleTemplateH);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewSelectCategory
 *
 * DESCRIPTION: This routine  recategorizes an item in the "Edit View".
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 *              The following global variables are modified:
 *							CurrentCategory
 *							CategoryName
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/03/95	Initial Revision
 *			grant	86/29/99	Adjust ItemsInCategory when the record is private
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static void EditViewSelectCategory (void)
{
	UInt16 attr;
	FormPtr frm;
	UInt16 category;
	Boolean categorySelected;
	Boolean categoryEdited;


	// Get the current category.
	DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;

	// Process the category popup list.
	frm = FrmGetActiveForm();
	categoryEdited = CategorySelect (ItemDB, frm, EditCategoryTrigger,
									 EditCategoryList, false, &category, CategoryName, 1, categoryDefaultEditCategoryString);


	categorySelected = category != (attr & dmRecAttrCategoryMask);

	// If a different category was selected,  set the category field
	// in the new record.
	if (categorySelected)
	{
		// Change the category of the record.
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);
		attr &= ~dmRecAttrCategoryMask;
		attr |= category | dmRecAttrDirty;
		DmSetRecordInfo (ItemDB, CurrentRecord, &attr, NULL);
	}


	// If the current category was changed or the name of the category
	// was edited,  draw the title.
	if (categoryEdited || categorySelected)
	{
		ChangeCategory (category);

		// If the record is secret and secret records are hidden, then the record isn't
		// accounted for by ItemsInCategory.  Adjust it, and EditViewSaveRecord will
		// adjust when done with the record.
		if ((PrivateRecordVisualStatus == hidePrivateRecords) && (attr & dmRecAttrSecret))
			ItemsInCategory++;

		EditViewSetTitle ();
	}
}


/***********************************************************************
 *
 * FUNCTION:    EditViewUpdateScrollBar
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
static void EditViewUpdateScrollBar ()
{
	UInt16 scrollPos;
	UInt16 textHeight;
	UInt16 fieldHeight;
	Int16 maxValue;
	FieldPtr fld;
	ScrollBarPtr bar;

	fld = GetObjectPtr (EditItemField);
	bar = GetObjectPtr (EditItemScrollBar);

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
 * FUNCTION:    EditViewLoadRecord
 *
 * DESCRIPTION: This routine loads an item record into the Edit View form.
 *
 * PARAMETERS:  frm - pointer to the Edit View form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger	6/24/99	Fixup ItemsInCategory when the record is private
 *			peter	4/25/00	Add support for un-masking just the selected record.
 *
 ***********************************************************************/
static void EditViewLoadRecord (FormPtr frm)
{
	UInt16		attr;
	FieldPtr fld;
	ControlPtr ctl;
	MemHandle itemRec;
	Char * itemRecP;
	FileRef fileRef;
	UInt32 bytesRead;
	Err err;

	Boolean capsLock, numLock, autoShifted;
	UInt16 tempShift;
	if ((GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted) == 0)
		&& (autoShifted))
	{
		GrfSetState(capsLock, numLock, false);
	}

	// Get a pointer to the item field.
	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, EditItemField));

	if (CurrentRecord == fileSelected)
	{
		// Get the current file
		itemRec = MemHandleNew(itemMaxLength);
		ErrFatalDisplayIf(itemRec == NULL, "Can't alloc");
		itemRecP = MemHandleLock(itemRec);
		*itemRecP = chrNull;
		err = VFSFileOpen (CurrentVolRefNum, CurrentPath, vfsModeRead, &fileRef);
		if(err) 
			ErrFatalDisplayIf (err, "Can't open file");
		err = VFSFileRead(fileRef, itemMaxLength, itemRecP, &bytesRead);
		if(err)	
			ErrFatalDisplayIf (err, "Can't read file");
		VFSFileClose (fileRef);
				
		if (bytesRead == itemMaxLength)
			itemRecP[itemMaxLength - 1] = chrNull;  // trim to fit
		else
			itemRecP[bytesRead] = chrNull;
		CurrentLineEnd = LineEndConvertToPalmOS(itemRecP);
		MemHandleUnlock(itemRec);
	}
	else
	{
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);

		// Get the current record.
		itemRec = DmGetRecord (ItemDB, CurrentRecord);
		ErrFatalDisplayIf ((! itemRec), "Bad record");
	}

	// Set the font used by the item field.
	FldSetFont (fld, EditFont);

	FldSetTextHandle (fld, itemRec);
	FldSetScrollPosition (fld, EditScrollPosition);

	if (CurrentRecord != fileSelected)
	{
		// Set the global variable that keeps track of the current category
		// to the category of the current record.
		CurrentCategory = attr & dmRecAttrCategoryMask;

		// If the record is secret and secret records are hidden, then the record isn't
		// accounted for by ItemsInCategory.  Adjust it, and EditViewSaveRecord will
		// adjust when done with the record.
		if ((PrivateRecordVisualStatus == hidePrivateRecords) && (attr & dmRecAttrSecret))
			ItemsInCategory++;
	}

	// Set the view's title
	EditViewSetTitle ();

	if (CurrentRecord == fileSelected)
	{
		FrmHideObject (frm, FrmGetObjectIndex (frm, EditCategoryTrigger));
		FrmHideObject (frm, FrmGetObjectIndex (frm, EditDetailsButton));
	}
	else
	{
		// Set the label that contains the note's category.
		ctl = GetObjectPtr (EditCategoryTrigger);
		CategoryGetName (ItemDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel (ctl, CategoryName);
	}

	EditViewUpdateScrollBar ();
}


/***********************************************************************
 *
 * FUNCTION:    EditViewSaveRecord
 *
 * DESCRIPTION: This routine save an item record to the sample database or
 *				deletes it if it's empty.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			kcr	11/16/95	use DmReleaseRecord to set dirty attribute
 *			jmp	9/29/99	Use FrmGetFormPtr() & FrmGetObjectIndex() instead of
 *								GetObjectPtr() because GetObjectPtr() calls FrmGetActiveForm(),
 *								and FrmGetActiveForm() may return a form that isn't the one we
 *								want when other forms are up when we are called.
 *								Fixes bug #22418.
 *
 ***********************************************************************/
static void EditViewSaveRecord (void)
{
	UInt16	attr;
	Char *	ptr;
	Boolean 	empty;
	Boolean	dirty;
	FieldPtr fld;
	FormPtr	frm;
	//Err err;
	// Find out if the field has been modified or if it's empty.
	frm = FrmGetFormPtr (EditView);
	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, EditItemField));
	ptr = FldGetTextPtr (fld);
	dirty = FldDirty (fld);
	empty = (*ptr == 0);

	FldReleaseFocus (fld);

	// Release any free space in the item field.
	FldCompactText (fld);

	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of.
	FldSetTextHandle (fld, 0);

	// If we're editing a file, save the changes
	if (CurrentRecord == fileSelected)
	{
		if (dirty)
		{
			if (FrmAlert (SaveChangesAlert) == SaveChangesYesButton)
				/*err =*/ SaveFile (CurrentVolRefNum, CurrentPath, ptr, CurrentLineEnd);
		}
		MemPtrFree(ptr);
		return;
	}

	// If there's data in an existing record, mark it dirty if
	// necessary and release it.
	if (! empty)
	{
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);

		if (PrivateRecordVisualStatus == hidePrivateRecords && (attr & dmRecAttrSecret))
			ItemsInCategory--;

		DmReleaseRecord (ItemDB, CurrentRecord, dirty);

		// Move the current record to the correct sort position.
		if (dirty)
			SampleSortRecord (ItemDB, &CurrentRecord);
	}

	// If the record is empty, delete it.
	else
	{
		if (dirty)
		{
			DmDeleteRecord (ItemDB, CurrentRecord);
			DmMoveRecord (ItemDB, CurrentRecord, DmNumRecords (ItemDB));
		}
		else
			DmRemoveRecord (ItemDB, CurrentRecord);

		CurrentRecord = noRecordSelected;
		ItemsInCategory--;
	}

	ErrNonFatalDisplayIf(ItemsInCategory > DmNumRecords(ItemDB), "invalid ItemsInCategory");
}


/***********************************************************************
 *
 * FUNCTION:    EditViewChangeFont
 *
 * DESCRIPTION: This routine redisplay the item in the font specified.
 *              It is called when one of the font push-buttons is presed.
 *
 * PARAMETERS:  controlID - id to button pressed.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *			kcr	10/20/95		handles two-button font change mechanism
 *
 ***********************************************************************/
static void EditViewChangeFont (void)
{
	FontID	fontID;
	FieldPtr fld;

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (EditFont);

	if (fontID != EditFont)
	{
		EditFont = fontID;

		// FldSetFont will redraw the field if it is visible.
		fld = GetObjectPtr (EditItemField);
		FldSetFont (fld, fontID);
	}

	EditViewUpdateScrollBar ();
}



/***********************************************************************
 *
 * FUNCTION:    EditViewDeleteRecord
 *
 * DESCRIPTION: This routine deletes an item record..
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
static Boolean EditViewDeleteRecord (void)
{
	FormPtr  frm;
	FieldPtr fld;
	Char *  ptr;
	UInt16 ctlIndex;
	UInt16 buttonHit;
	FormPtr alert;
	Boolean empty;
	Boolean saveBackup;


	frm = FrmGetFormPtr (EditView);
	fld = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, EditItemField));

	// Find out if the field is empty.
	ptr = FldGetTextPtr (fld);
	empty = (*ptr == 0);

	// Display an alert to confirm the operation.
	if (!empty)
	{
		alert = FrmInitForm (DeleteItemDialog);

		ctlIndex = FrmGetObjectIndex (alert, DeleteItemSaveBackup);
		FrmSetControlValue (alert, ctlIndex, SaveBackup);
		buttonHit = FrmDoDialog (alert);
		saveBackup = FrmGetControlValue (alert, ctlIndex);;

		FrmDeleteForm (alert);

		if (buttonHit == DeleteItemCancel)
			return (false);

		SaveBackup = saveBackup;
	}

	// Clear the handle value in the field, otherwise the handle
	// will be free when the form is disposed of.
	FldSetTextHandle (fld, 0);

	// Delete or archive the record.
	if (empty && (! FldDirty (fld)))
	{
		DmRemoveRecord (ItemDB, CurrentRecord);
	}
	else
	{
		if (SaveBackup)
			DmArchiveRecord (ItemDB, CurrentRecord);
		else
			DmDeleteRecord (ItemDB, CurrentRecord);
		DmMoveRecord (ItemDB, CurrentRecord, DmNumRecords (ItemDB));
	}

	ItemsInCategory--;
	CurrentRecord = noRecordSelected;

	ErrNonFatalDisplayIf(ItemsInCategory > DmNumRecords(ItemDB), "invalid ItemsInCategory");

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:		EditViewDoCommand
 *
 * DESCRIPTION:	This routine performs the menu command specified.
 *
 * PARAMETERS:		command  - menu item id
 *
 * RETURNED:    	True if we handled the command.
 *
 *	HISTORY:
 *		03/29/95	art	Created by Art Lamb.
 *		11/07/95	kcr	converted to common about box
 *		08/21/99	kwk	Deleted page top/bottom commands.
 *		11/04/99	jmp	To prevent other sublaunch issues, remind ourselves
 *							that we've sublaunched already into PhoneNumberLookup().
 *
 ***********************************************************************/
static Boolean EditViewDoCommand (UInt16 command)
{
	FieldPtr fld;
	FormPtr frm;
	Char * ptr;
	UInt16 refNum;
	const Char * extensions[] = {sampleExtension};
	UInt16 volRefNum;
	Char * path;
	Boolean handled = true;
	Err err;

	switch (command)
	{
	case NewItemCmd:
		EditViewSaveRecord ();
		err = CreateRecord ();
		EditScrollPosition = 0;
		if (CurrentRecord != noRecordSelected)
		{
			EditViewLoadRecord (FrmGetActiveForm ());
			fld = GetFocusObjectPtr ();
			if (fld) FldGrabFocus (fld);
		}
		else
			FrmGotoForm (ListView);
		break;

	case SaveItemCmd:
		fld = GetObjectPtr (EditItemField);
		ptr = FldGetTextPtr (fld);
		err = SaveFile (CurrentVolRefNum, CurrentPath, ptr, CurrentLineEnd);
		if (!err)
			FldSetDirty (fld, false);
		break;

	case SaveItemAsCmd:
		err = SysLibFind (kFileBrowserLibName, &refNum);
		ErrFatalDisplayIf (err, "Can't find file browser lib");
		err = FileBrowserLibOpen (refNum);
		ErrFatalDisplayIf (err, "Can't open file browser lib");
		volRefNum = CurrentVolRefNum;
		path = MemPtrNew (kFileBrowserLibPathBufferSize);
		ErrFatalDisplayIf (path == NULL, "Can't alloc path");
		path[0] = chrNull;
		if (CurrentPath)
			StrCopy (path, CurrentPath);
		if (FileBrowserLibShowSaveAsDialog (refNum, &volRefNum, path,
			1, extensions, sampleExtension, sampleMIMEType, NULL,
			kFileBrowserLibFlagPromptOverwrite | kFileBrowserLibFlagRequireExtension))
		{
			fld = GetObjectPtr (EditItemField);
			ptr = FldGetTextPtr (fld);
			err = SaveFile (volRefNum, path, ptr, CurrentLineEnd);
			if (!err)
			{
				CurrentRecord = fileSelected;
				CurrentVolRefNum = volRefNum;
				if (CurrentPath == NULL)
				{
					CurrentPath = MemPtrNew (kFileBrowserLibPathBufferSize);
					if (CurrentPath == NULL)
					{
						ErrNonFatalDisplay ("Can't alloc path");
						break;
					}
				}
				ErrNonFatalDisplayIf (StrLen (path) > kFileBrowserLibMaxPathLength, "Path too long");
				StrCopy (CurrentPath, path);
				FldSetDirty (fld, false);
				EditViewSetTitle ();
			}
		}
		MemPtrFree (path);
		FileBrowserLibClose (refNum);
		break;

	case DeleteItemCmd:
		if (EditViewDeleteRecord ())
			FrmGotoForm (ListView);
		break;

	case BeamItemCmd:
	case SendItemCmd:
		fld = GetObjectPtr (EditItemField);
		if (FldGetTextLength(fld) > 0)
		{
			EditViewSaveRecord();
			SampleSendRecord(ItemDB, CurrentRecord, (command == BeamItemCmd ? exgBeamPrefix : exgSendPrefix));

			// Redisplay the record.  If the IR loopback mechanism sends the
			// record to this app the goto action code closes all forms and
			// send a frmGotoEvent.  Load the record again only if the form
			// still exits.
			frm = FrmGetActiveForm ();
			if (frm)
				EditViewLoadRecord (frm);
		}
		else
			FrmAlert(NoDataToBeamAlert);
		break;
		/*
		 case BeamCategoryCmd:
		 case SendCategoryCmd:
		 fld = GetObjectPtr (EditItemField);
		 if (FldGetTextLength(fld) > 0)
		 {
		 FldCompactText (fld);
		 SampleSendCategory(ItemDB, CurrentCategory, (command == BeamCategoryCmd ? exgBeamPrefix : exgSendPrefix), (command == BeamCategoryCmd ? NoDataToBeamAlert : NoDataToSendAlert));
		 }
		 else
		 FrmAlert(NoDataToBeamAlert);
		 break;
		 */

	case EditOptionsFontsCmd:
		EditViewChangeFont ();
		break;

	case EditOptionPhoneLookupCmd:
		fld = GetObjectPtr (EditItemField);
		if (fld)
		{
			//InPhoneLookup = true;
			PhoneNumberLookup (fld);
			//InPhoneLookup = false;
		}
		break;

	case EditOptionsAboutCmd:
		AbtShowAbout (kAppCreator);
		break;

	default:
		handled = false;
	}

	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewScroll
 *
 * DESCRIPTION: This routine scrolls the item edit view a page or a
 *              line at a time.
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
 *			grant 2/4/99	Use EditViewUpdateScrollBar()
 *
 ***********************************************************************/
static void EditViewScroll (Int16 linesToScroll, Boolean updateScrollbar)
{
	UInt16				blankLines;
	FieldPtr			fld;

	fld = GetObjectPtr (EditItemField);
	blankLines = FldGetNumberOfBlankLines (fld);

	if (linesToScroll < 0)
		FldScrollField (fld, -linesToScroll, winUp);
	else if (linesToScroll > 0)
		FldScrollField (fld, linesToScroll, winDown);

	// If there were blank lines visible at the end of the field
	// then we need to update the scroll bar.
	if (blankLines || updateScrollbar)
	{
		ErrNonFatalDisplayIf(blankLines && linesToScroll > 0, "blank lines when scrolling winDown");

		EditViewUpdateScrollBar();
	}
}


/***********************************************************************
 *
 * FUNCTION:    EditViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the message a page winUp or winDown.
 *					 When the top of an item is visible, scrolling up will
 *              display the display the botton of the previous item.
 *              If the bottom of an item is visible, scrolling down will
 *              display the top of the next item.
 *
 * PARAMETERS:   direction     winUp or winDown
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/1/96	Initial Revision
 *			grant 2/4/99	Use EditViewScroll() to do actual scrolling.
 *
 ***********************************************************************/
static void EditViewPageScroll (WinDirectionType direction)
{
	Int16 seekDirection;
	UInt16 category;
	UInt16 recordNum;
	UInt32 uniqueID;
	UInt16 linesToScroll;
	FieldPtr fld;
	UInt16	attr;

	fld = GetObjectPtr (EditItemField);

	if (FldScrollable (fld, direction))
	{
		linesToScroll = FldGetVisibleLines (fld) - 1;

		if (direction == winUp)
			linesToScroll = -linesToScroll;

		EditViewScroll(linesToScroll, true);

		return;
	}


	// Move to the next or previous item.
	if (direction == winUp)
	{
		seekDirection = dmSeekBackward;
		EditScrollPosition = maxFieldTextLen;
	}
	else
	{
		seekDirection = dmSeekForward;
		EditScrollPosition = 0;
	}

	if (ShowAllCategories)
		category = dmAllCategories;
	else
		category = CurrentCategory;

	recordNum = CurrentRecord;

	//while to skip masked records. Even if the body never executes, we'll have done a DmSeekRecordInCategory
	while (!DmSeekRecordInCategory (ItemDB, &recordNum, 1, seekDirection, category) &&
		   !DmRecordInfo (ItemDB, recordNum, &attr, NULL, NULL) &&
		   ((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
	{
	}

	if (recordNum == CurrentRecord) return;

	// Don't show first/last record if it's private and we're masking.
	if (!DmRecordInfo (ItemDB, recordNum, &attr, NULL, NULL) &&
		((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
		return;

	SndPlaySystemSound (sndInfo);

	// Saving the current record may cause it to move if the records are
	// sorted alphabeticly.
	DmRecordInfo (ItemDB, recordNum, NULL, &uniqueID, NULL);
	EditViewSaveRecord ();
	DmFindRecordByID (ItemDB, uniqueID, &CurrentRecord);

	EditViewLoadRecord (FrmGetActiveForm ());
	FldGrabFocus (fld);

}


/***********************************************************************
 *
 * FUNCTION:    EditViewExit
 *
 * DESCRIPTION: This routine is call when the Edit View is exited.  It
 *              releases any memory allocated for the Edit View.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	3/17/95		Initial Revision
 *
 ***********************************************************************/
static void EditViewExit (void)
{
	FormPtr  frm;
	void * title;

	// Free the title string.
	frm = FrmGetActiveForm ();
	title = (void *)FrmGetTitle (frm);
	if (title)
	{
		FrmSetTitle (frm, NULL);
		MemPtrFree(title);
	}
}


/***********************************************************************
 *
 * FUNCTION:    EditViewInit
 *
 * DESCRIPTION: This routine initials the Edit View form.
 *
 * PARAMETERS:  frm - pointer to the Edit View form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art		3/31/95		Initial Revision
 *			art		7/1/96		Set field hasScrollBar attribute
 *			FPa		11/30/96	Set title to 0
 *
 ***********************************************************************/
static void EditViewInit (FormPtr frm)
{
	FieldPtr 		fld;
	FieldAttrType	attr;


	// Have the field send event to maintain the scroll bar.
	fld = GetObjectPtr (EditItemField);
	FldGetAttributes (fld, &attr);
	attr.hasScrollBar = true;
	FldSetAttributes (fld, &attr);

	FrmSetTitle(frm, 0);	// To avoid a crash into EditViewSetTitle when trying to free the old title

	EditViewLoadRecord (frm);

	CurrentView = EditView;
}


/***********************************************************************
 *
 * FUNCTION:    EditViewUpdateDisplay
 *
 * DESCRIPTION: This routine update the display of the edit view
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the view.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	10/3/95	Initial Revision
 *
 ***********************************************************************/
static Boolean EditViewUpdateDisplay (UInt16 updateCode)
{
	UInt16 attr;
	UInt16 category;

	if (updateCode == updateCategoryChanged)
	{
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);
		category = attr & dmRecAttrCategoryMask;
		ChangeCategory (category);

		// If we are editing a secret record and secret records are hidden
		// then increment the record count so that the correct record count
		// is shown.
		if (PrivateRecordVisualStatus == hidePrivateRecords && (attr & dmRecAttrSecret))
			ItemsInCategory++;

		// Set the title of the edit view.
		EditViewSetTitle ();

		// Set the label of the category trigger.
		CategoryGetName (ItemDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel (GetObjectPtr (EditCategoryTrigger),
								 CategoryName);
		return (true);
	}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    EditViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Edit View"
 *              of the Sample application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	02/21/95	Initial Revision.
 *			kwk	10/04/98	Explicitly disable Graffiti auto-shift if we get
 *								a printable key-down event.
 *			kwk	11/21/98	Handle cmd keys in separate code block, so TxtCharIsPrint
 *								doesn't get called w/virtual key codes.
 *			gap	10/27/00	change the command bar initialization to allow field
 *								code to add cut, copy, paste, & undo commands as
 *								appropriate rather than adding a fixed set of selections.
 *			CS		11/28/00	Call EditViewExit in response to frmCloseEvent.
 *
 ***********************************************************************/
static Boolean EditViewHandleEvent (EventType * event)
{
	FormPtr frm;
	Boolean handled = false;
	FieldPtr fldP;
	UInt32 numLibs;


	if (event->eType == keyDownEvent)
	{
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
		{
			EditViewSaveRecord ();
			FrmGotoForm (ListView);
			handled = true;
		}

		else if (EvtKeydownIsVirtual(event))
		{
			if (event->data.keyDown.chr == vchrPageUp)
			{
				EditViewPageScroll (winUp);
				handled = true;
			}

			else if (event->data.keyDown.chr == vchrPageDown)
			{
				EditViewPageScroll (winDown);
				handled = true;
			}

			// Send data key presed?
			else if (event->data.keyDown.chr == vchrSendData)
			{
				if (FldGetTextLength(GetObjectPtr (EditItemField)) > 0)
				{
					EditViewSaveRecord();
					SampleSendRecord(ItemDB, CurrentRecord, exgBeamPrefix);

					// Redisplay the record.  If the IR loopback mechanism sends the
					// record to this app the goto action code closes all forms and
					// send a frmGotoEvent.  Load the record again only if the form
					// still exits.
					frm = FrmGetActiveForm ();
					if (frm)
						EditViewLoadRecord (frm);
				}
				else
					FrmAlert(NoDataToBeamAlert);
				handled = true;
			}
		}

		// If we get a printable character, then we can assume that any Graffiti
		// auto-shifting is finished, so it's safe to turn it off. This solves the
		// problem of a re-queued keydown event (e.g. when in list view & writing
		// a character -> generate a new item) not turning off the temp shift state
		// because it wasn't created by Graffiti.

		/*
		 else if (TxtCharIsPrint(event->data.keyDown.chr))
		 {
		 Boolean capsLock, numLock, autoShifted;
		 UInt16 tempShift;

		 if ((GrfGetState(&capsLock, &numLock, &tempShift, &autoShifted) == 0)
		 && (autoShifted))
		 {
		 GrfSetState(capsLock, numLock, false);
		 }
		 }
		 */
	}

	else if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
		case EditCategoryTrigger:
			EditViewSelectCategory ();
			handled = true;
			break;

		case EditDoneButton:
			EditViewSaveRecord ();
			FrmGotoForm (ListView);
			handled = true;
			break;

		case EditDetailsButton:
			FrmPopupForm (DetailsDialog);
			handled = true;
			break;
		}
	}


	else if (event->eType == menuOpenEvent)
	{
		if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
			MenuHideItem(SendItemCmd);
		else
			MenuShowItem(SendItemCmd);
		// don't set handled = true
		
		// Hide the Save and Save As menu items when editing a record
		if (CurrentRecord != fileSelected)
		{
			MenuHideItem(SaveItemCmd);
			MenuHideItem(SaveItemAsCmd);
		}
	}

	else if (event->eType == menuEvent)
	{
		handled = EditViewDoCommand (event->data.menu.itemID);
	}


	// Add the buttons that we want available on the command bar, based on the current context
	else if (event->eType == menuCmdBarOpenEvent)
	{
		UInt16 startPos, endPos;
		FieldType* fldP;

		fldP = GetObjectPtr (EditItemField);
		FldGetSelection(fldP, &startPos, &endPos);

		if (startPos == endPos)  // there's no highlighted text
		{
			// Call directly Field event handler so that System Edit buttons are added
			FldHandleEvent(fldP, event);

			// Beam on the left
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, BeamItemCmd, 0);

			// Delete on the Right
			MenuCmdBarAddButton(menuCmdBarOnRight, BarDeleteBitmap, menuCmdBarResultMenuItem, DeleteItemCmd, 0);

			// Prevent the field package to automatically add cut, copy, paste, and undo buttons as applicable
			// since it was done previously
			event->data.menuCmdBarOpen.preventFieldButtons = true;
		}

		// don't set handled to true; this event must fall through to the system.
	}


	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		EditViewInit (frm);
		FrmDrawForm (frm);
		FrmSetFocus (frm, FrmGetObjectIndex (frm, EditItemField));
		handled = true;
	}


	else if (event->eType == frmGotoEvent)
	{
		frm = FrmGetActiveForm ();
		CurrentRecord = event->data.frmGoto.recordNum;
		EditViewInit (frm);
		fldP = GetObjectPtr (EditItemField);
		FldSetScrollPosition(fldP, event->data.frmGoto.matchPos);
		FldSetSelection(fldP, event->data.frmGoto.matchPos,
						event->data.frmGoto.matchPos + event->data.frmGoto.matchLen);
		EditViewUpdateScrollBar ();
		FrmDrawForm (frm);
		FrmSetFocus (frm, FrmGetObjectIndex (frm, EditItemField));
		handled = true;
	}


	else if (event->eType == frmUpdateEvent)
	{
		handled =  EditViewUpdateDisplay (event->data.frmUpdate.updateCode);
	}

	else if (event->eType == fldChangedEvent)
	{
		//frm = FrmGetActiveForm ();
		FrmGetActiveForm ();
		EditViewUpdateScrollBar ();
		handled = true;
	}


	else if (event->eType == frmCloseEvent)
	{
		FormPtr frm;

		frm = FrmGetFormPtr (EditView);
		if ( FldGetTextHandle (FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, EditItemField))))
			EditViewSaveRecord (); // This deletes empty items.
		EditViewExit();
	}


	else if (event->eType == sclRepeatEvent)
	{
		EditViewScroll (event->data.sclRepeat.newValue -
						event->data.sclRepeat.value, false);
	}

	return (handled);
}


#pragma mark ----
/***********************************************************************
 *
 * FUNCTION:    ListViewDoCommand
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
 *			kcr	11/7/95	converted to common about box
 *			jmp	10/02/99	Changed call to DmOpenDatabaseByTypeCreator() to
 *								SampleGetDatabase().
 *			jmp	03/19/00	Fixed bug #23669:  Adjust the number of items currently
 *								available so that scrollbar will be updated correctly
 *								after a change in security level.
 *
 ***********************************************************************/
static void ListViewDoCommand (UInt16 command)
{
	Boolean			wasHiding;
	UInt32			currentRecordID;
	UInt16		 	mode;
	UInt16			refNum;
	UInt16			volRefNum;
	Char *			path;
	const Char *	extensions[] = {sampleExtension};
	Err				err;

	switch (command)
	{
	case ListOptionsFontsCmd:
		ListFont = SelectFont(ListFont);
		break;

	case EditMenuCtl:
	case ListRecordOpenCmd:
		err = SysLibFind (kFileBrowserLibName, &refNum);
		//ErrFatalDisplayIf (err, "Can't find file browser lib");
		if (err)
		{
			FrmCustomAlert (InformationAlert,"Alert", "Can't find file browser lib. Please run it on Life Drive / Tungsten T5", NULL);
			break;
		}
		
		err = FileBrowserLibOpen (refNum);
		ErrFatalDisplayIf (err, "Can't open file browser lib");
		volRefNum = vfsInvalidVolRef;
		path = MemPtrNew (kFileBrowserLibPathBufferSize);
		ErrFatalDisplayIf (path == NULL, "Can't alloc path");
		path[0] = chrNull;
		if (FileBrowserLibShowOpenDialog (refNum, &volRefNum, path,
			1, extensions, sampleMIMEType, "Select Item", kFileBrowserLibFlagNoFolders))
		{
			CurrentRecord = fileSelected;
			CurrentVolRefNum = volRefNum;
			if (CurrentPath == NULL)
			{
				CurrentPath = MemPtrNew (kFileBrowserLibPathBufferSize);
				if (CurrentPath == NULL)
				{
					ErrNonFatalDisplay ("Can't alloc path");
					break;
				}
			}
			ErrNonFatalDisplayIf (StrLen (path) > kFileBrowserLibMaxPathLength, "Path too long");
			StrCopy (CurrentPath, path);
			EditScrollPosition = 0;
			FrmGotoForm (EditView);
		}
		MemPtrFree (path);
		FileBrowserLibClose (refNum);
		break;

	case ListRecordBeamCategoryCmd:
		SampleSendCategory(ItemDB, CurrentCategory, exgBeamPrefix, NoDataToBeamAlert);
		break;

	case ListRecordSendCategoryCmd:
		SampleSendCategory(ItemDB, CurrentCategory, exgSendPrefix, NoDataToSendAlert);
		break;

	case ListOptionsSecurityCmd:
		wasHiding = (PrivateRecordVisualStatus == hidePrivateRecords);

		PrivateRecordVisualStatus = SecSelectViewStatus();

		if (wasHiding ^ (PrivateRecordVisualStatus == hidePrivateRecords)) //xor on two logical values - mode to open DB has changed
		{
			// Close the application's data file.
			SampleSavePrefs(0);
			DmCloseDatabase(ItemDB);

			mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
				dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);

			SampleGetDatabase(&ItemDB, mode);
			ErrFatalDisplayIf(!ItemDB,"Can't reopen DB");
			// Read the preferences.
			SampleLoadPrefs(&currentRecordID);

			// Adjust the number of items currently available so the scrollbar will be updated correctly.
			if (ShowAllCategories)
				ItemsInCategory = DmNumRecordsInCategory(ItemDB, dmAllCategories);
			else
				ItemsInCategory = DmNumRecordsInCategory(ItemDB, CurrentCategory);
		}

		//For safety, simply reset the currentRecord
		CurrentRecord = noRecordSelected;
		ListViewUpdateDisplay(updateDisplayOptsChanged);
		break;

	case ListOptionsPreferencesCmd:
		FrmPopupForm (PreferencesDialog);
		break;

	case ListOptionsAboutCmd:
		MenuEraseStatus(CurrentMenu);
		AbtShowAbout(kAppCreator);
		break;
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewNumberOfRows
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
static UInt16 ListViewNumberOfRows (TablePtr table)
{
	UInt16				rows;
	UInt16				rowsInTable;
	UInt16				tableHeight;
	FontID			currFont;
	RectangleType	r;


	rowsInTable = TblGetNumberOfRows (table);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (ListFont);
	rows = tableHeight / FntLineHeight ();
	FntSetFont (currFont);

	if (rows <= rowsInTable)
		return (rows);
	else
		return (rowsInTable);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewDrawRecord
 *
 * DESCRIPTION: This routine draws the item number in the list
 *              view.  This routine is called by the table routine,
 *              TblDrawTable, each time a line of the table needs to
 *              be drawn.
 *
 * PARAMETERS:  table  - pointer to the item list table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - bound to the draw region
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			ADH	7/21/99	Increased the size of the posStr character array
 *								to allow for display of five digit numbers.
 *								Previously only four digit numbers could be
 *								displayed.
 *			ryw	1/11/01	use global TopRowPositionInCategory to determine numbering
 *
 ***********************************************************************/
static void ListViewDrawRecord (void * table, Int16 row, Int16 UNUSED_PARAM(column),
								RectanglePtr bounds)
{
	UInt16 len;
	//UInt16 category;
	UInt16 recordNum;
	MemHandle itemH;
	Int16 x, y;
	Char * itemP;
	UInt16 pos;
	// this string should handle up to "99999." but does not have to be 0 terminated when the . is added
	char posStr[6];
	UInt16 attr;
	RectangleType maskRectangle;

	// Get the record number that corresponds to the table item to draw.
	// The record number is stored in the "intValue" field of the item.
	recordNum = TblGetRowID (table, row);

	DmRecordInfo (ItemDB, recordNum, &attr, NULL, NULL);
	// If the record is private and we are to hide private records, then get out of here.
	// This should be taken care of by the calling function, but we will go ahead and
	// take care of it here also.
	if ((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == hidePrivateRecords)
	{
		return;
	}

	x = bounds->topLeft.x + 1;
	y = bounds->topLeft.y;

	FntSetFont (ListFont);

	// Format the item's postion, within its category, an draw it.
/*
	if (ShowAllCategories)
		category = dmAllCategories;
	else
		category = CurrentCategory;
*/

	//pos = DmPositionInCategory (ItemDB, recordNum, category);
	pos = TopRowPositionInCategory + row;
	StrIToA (posStr, pos+1);
	len = StrLen(posStr);
	// Warning if more than 99999 record (5 chars)
	ErrNonFatalDisplayIf(len > sizeof(posStr) - 1, "Too many records");
	posStr[len++] = '.';
	// This string does not need 0 termination for up to the end of the function. So the size is ok

	if (len < 3) x += FntCharWidth ('1');
	WinDrawChars (posStr, len, x, y);
	x += FntCharsWidth (posStr, len) + 4;

	// If we are here then we either we either mask the item out or display the
	// item title.
	if (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
	{
		MemMove (&maskRectangle, bounds, sizeof (RectangleType));
		maskRectangle.topLeft.x = x;
		maskRectangle.extent.x = bounds->extent.x - x;

		//If next row is masked, thicken rect so as to keep boundary at 1 pixel.
		/*    // THIS CODE REMOVED because people didn't like combining the masks together.
		 if (TblGetLastUsableRow(table) > row)
		 {
		 DmRecordInfo (ItemDB, TblGetRowID (table, row+1), &attr, NULL, NULL);
		 if (attr & dmRecAttrSecret)
		 maskRectangle.extent.y++;
		 }
		 */
		ListViewDisplayMask (&maskRectangle);
	}
	else
	{
		// Display the item's title, the title is the first line of the item.
		itemH = DmQueryRecord(ItemDB, recordNum);
		itemP = MemHandleLock(itemH);
		DrawItemTitle (itemP, x, y, bounds->extent.x - x);
		MemHandleUnlock(itemH);
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewDisplayMask
 *
 * DESCRIPTION: Draws the masked display for the record.
 *
 * PARAMETERS:  bounds (Input):  The bounds of the table item to display.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         css    06/21/99   Initial Revision
 *
 ***********************************************************************/
static void ListViewDisplayMask (RectanglePtr bounds)
{
	RectangleType tempRect;
	CustomPatternType origPattern;
	MemHandle	bitmapH;
	BitmapType * bitmapP;

	CustomPatternType pattern = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55};

MemMove (&tempRect, bounds, sizeof (RectangleType));
// Make sure it fits nicely into the display.
tempRect.topLeft.y++;
tempRect.extent.y --;
tempRect.extent.x -= SecLockWidth + 1;

WinGetPattern(&origPattern);
WinSetPattern((const CustomPatternType *)&pattern);
WinFillRectangle (&tempRect, 0);
WinSetPattern((const CustomPatternType *)&origPattern);

//draw lock icon
bitmapH = DmGetResource (bitmapRsc, SecLockBitmap);
if (bitmapH)
{
	bitmapP = MemHandleLock (bitmapH);
	WinDrawBitmap (bitmapP, tempRect.topLeft.x + tempRect.extent.x + 1,
				   tempRect.topLeft.y + ((tempRect.extent.y - SecLockHeight) / 2));
	MemPtrUnlock (bitmapP);
}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateScrollers
 *
 * DESCRIPTION: This routine draws or erases the list view scroll arrow
 *              buttons.
 *
 * PARAMETERS:  frm          -  pointer to the to do list form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	5/1/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewUpdateScrollers (FormPtr UNUSED_PARAM(frm))
{
	UInt16 pos;
	Int16 rows;
	UInt16 maxValue;

	rows = ListViewNumberOfRows (GetObjectPtr(ListTable));
	if (ItemsInCategory > rows)
	{
		pos = DmPositionInCategory (ItemDB, TopVisibleRecord, CurrentCategory);
		maxValue = ItemsInCategory - rows;
	}
	else
	{
		pos = 0;
		maxValue = 0;
	}

	SclSetScrollBar (GetObjectPtr (ListScrollBar), pos, 0, maxValue, rows);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadTable
 *
 * DESCRIPTION: This routine loads sample database records into
 *              the list view form.
 *
 * PARAMETERS:  recordNum index of the first record to display.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/16/95	Initial Revision
 *			grant	1/29/99	Set the heights of unused rows
 *			ryw	1/11/01	update global TopRowPositionInCategory on table load
 *
 ***********************************************************************/
static void ListViewLoadTable (FormPtr frm)
{
	UInt16			row;
	UInt16			recordNum;
	UInt16			lineHeight;
	UInt16			dataHeight;
	UInt16			tableHeight;
	UInt16			numRows;
	UInt32			uniqueID;
	FontID			currFont;
	TablePtr 		table;
	MemHandle			recordH;
	RectangleType	r;


	table = GetObjectPtr (ListTable);

	TblGetBounds (table, &r);
	tableHeight = r.extent.y;

	currFont = FntSetFont (ListFont);
	lineHeight = FntLineHeight ();
	FntSetFont (currFont);

	dataHeight = 0;

	recordNum = TopVisibleRecord;

	// For each row in the table, store the record number in the table item
	// that will dispaly the record.
	numRows = TblGetNumberOfRows (table);
	for (row = 0; row < numRows; row++)
	{
		// Get the next record in the currunt category.
		recordH = DmQueryNextInCategory (ItemDB, &recordNum, CurrentCategory);
		if(row == 0)
		{
			// store the position of the first row so we can use TopRowPositionInCategory+row 
			// when drawing
			TopRowPositionInCategory = recordH ? DmPositionInCategory(ItemDB, recordNum, CurrentCategory) : 0;
		}

		// If the record was found, store the record number in the table item,
		// otherwise set the table row unusable.
		if (recordH && (tableHeight >= dataHeight + lineHeight))
		{
			TblSetRowID (table, row, recordNum);
			TblSetItemStyle (table, row, 0, customTableItem);
			TblSetItemFont (table, row, 0, ListFont);

			TblSetRowHeight (table, row, lineHeight);

			DmRecordInfo (ItemDB, recordNum, NULL, &uniqueID, NULL);
			if ((TblGetRowData (table, row) != uniqueID) ||
				( ! TblRowUsable (table, row)))
			{
				TblSetRowUsable (table, row, true);

				// Store the unique id of the record in the row.
				TblSetRowData (table, row, uniqueID);

				// Mark the row invalid so that it will draw when we call the
				// draw routine.
				TblMarkRowInvalid (table, row);
			}

			if (row+1 < numRows) recordNum++;

			dataHeight += lineHeight;
		}
		else
		{
			// Set the row height - when scrolling winDown, the heights of the last rows of
			// the table are used to determine how far to scroll.  As rows are deleted
			// from the top of the table, formerly unused rows scroll into view, and the
			// height is used before the next call to ListViewLoadTable (which would set
			// the height correctly).
			TblSetRowHeight (table, row, lineHeight);

			TblSetRowUsable (table, row, false);
		}
	}


	// Update the scroll arrows.
	ListViewUpdateScrollers (frm);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewLoadRecords
 *
 * DESCRIPTION: This routine loads sample database records into
 *              the list view form.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/16/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewLoadRecords (FormPtr frm)
{
	TablePtr 	table;
	UInt16			recordNum;
	UInt16			rowsInTable;

	if (ShowAllCategories)
		CurrentCategory = dmAllCategories;

	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, ListTable));
	rowsInTable = ListViewNumberOfRows (table);

	// Is the current record before the first visible record?
	if (CurrentRecord != noRecordSelected)
	{
		if (TopVisibleRecord > CurrentRecord)
			TopVisibleRecord = CurrentRecord;

		// Is the current record after the last visible record?
		else
		{
			recordNum = TopVisibleRecord;
			DmSeekRecordInCategory (ItemDB, &recordNum, rowsInTable-1,
									dmSeekForward, CurrentCategory);
			if (recordNum < CurrentRecord)
				TopVisibleRecord = CurrentRecord;
		}
	}


	// Make sure we show a full display of records.
	if (ItemsInCategory)
	{
		recordNum = dmMaxRecordIndex;
		DmSeekRecordInCategory (ItemDB, &recordNum, (rowsInTable-1),
								dmSeekBackward, CurrentCategory);
		TopVisibleRecord = min (TopVisibleRecord, recordNum);
	}
	else
		TopVisibleRecord = 0;

	ListViewLoadTable (frm);

	// Set the callback routine that will draw the records.
	TblSetCustomDrawProcedure (table, 0, ListViewDrawRecord);

	TblSetColumnUsable (table, 0, true);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectCategory
 *
 * DESCRIPTION: This routine handles selection, creation and deletion of
 *              categories form the Details Dialog.
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
 *			art	03/10/95	Initial Revision
 *			gap	08/13/99	Update to use new constant categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static UInt16 ListViewSelectCategory (void)
{
	FormPtr frm;
	TablePtr table;
	UInt16 category;
	Boolean categoryEdited;


	// Process the category popup list.
	category = CurrentCategory;

	frm = FrmGetActiveForm();
	categoryEdited = CategorySelect (ItemDB, frm, ListCategoryTrigger,
									 ListCategoryList, true, &category, CategoryName, 1, categoryDefaultEditCategoryString);

	if (category == dmAllCategories)
		ShowAllCategories = true;
	else
		ShowAllCategories = false;

	if (categoryEdited || (category != CurrentCategory))
	{
		ChangeCategory (category);

		// Display the new category.
		ListViewLoadRecords (frm);
		table = GetObjectPtr (ListTable);
		TblEraseTable (table);
		TblDrawTable (table);
	}

	return (category);
}

/***********************************************************************
 *
 * FUNCTION:    ListViewNextCategory
 *
 * DESCRIPTION: This routine display the next category,  if the last
 *              catagory isn't being displayed
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
 *			art	9/15/95	Initial Revision
 *
 ***********************************************************************/
static void ListViewNextCategory (void)
{
	UInt16 category;
	FormPtr frm;
	TablePtr table;
	ControlPtr ctl;

	category = CategoryGetNext (ItemDB, CurrentCategory);

	if (category != CurrentCategory)
	{
		if (category == dmAllCategories)
			ShowAllCategories = true;
		else
			ShowAllCategories = false;

		ChangeCategory (category);

		// Set the label of the category trigger.
		ctl = GetObjectPtr (ListCategoryTrigger);
		CategoryGetName (ItemDB, CurrentCategory, CategoryName);
		CategorySetTriggerLabel (ctl, CategoryName);


		// Display the new category.
		TopVisibleRecord = 0;
		frm = FrmGetActiveForm ();
		ListViewLoadTable (frm);
		table = GetObjectPtr (ListTable);
		TblEraseTable (table);
		TblDrawTable (table);
	}
}


/***********************************************************************
 *
 * FUNCTION:    ListViewPageScroll
 *
 * DESCRIPTION: This routine scrolls the list of of item titles
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger 7/27/95	Copied fixed code from Address Book
 *
 ***********************************************************************/
static void ListViewPageScroll (WinDirectionType direction)
{
	TablePtr table;
	Int16 rowsInTable;
	UInt16 newTopVisibleRecord;

	table = GetObjectPtr (ListTable);
	rowsInTable = ListViewNumberOfRows (table);

	newTopVisibleRecord = TopVisibleRecord;
	CurrentRecord = noRecordSelected;

	// Scroll the table winDown a page (less one row).
	if (direction == winDown)
	{
		// Try going forward one page
		if (!SeekRecord (&newTopVisibleRecord, rowsInTable - 1, dmSeekForward))
		{
			// Try going backwards one page from the last record
			newTopVisibleRecord = dmMaxRecordIndex;
			if (!SeekRecord (&newTopVisibleRecord, rowsInTable - 1, dmSeekBackward))
			{
				// Not enough records to fill one page.  Start with the first record
				newTopVisibleRecord = 0;
				SeekRecord (&newTopVisibleRecord, 0, dmSeekForward);
			}
		}
	}

	// Scroll up a page (less one row).
	else
	{
		if (!SeekRecord (&newTopVisibleRecord, rowsInTable - 1, dmSeekBackward))
		{
			// Not enough records to fill one page.  Start with the first record
			newTopVisibleRecord = 0;
			SeekRecord (&newTopVisibleRecord, 0, dmSeekForward);
		}
	}



	// Avoid redraw if no change
	if (TopVisibleRecord != newTopVisibleRecord)
	{
		TopVisibleRecord = newTopVisibleRecord;
		ListViewLoadRecords (FrmGetActiveForm ());
		TblRedrawTable(table);
	}
}



/***********************************************************************
 *
 * FUNCTION:    ListViewScroll
 *
 * DESCRIPTION: This routine scrolls the list of of item titles
 *              in the direction specified.
 *
 * PARAMETERS:  direction - winUp or dowm
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			roger 7/27/95	Copied fixed code from Address Book
 *
 ***********************************************************************/
static void ListViewScroll (Int16 linesToScroll)
{
	Int16 				i;
	UInt16				rows;
	UInt16				lastRow;
	UInt16 				scrollAmount;
	UInt16 				newTopVisibleRecord;
	TablePtr 			table;
	RectangleType		scrollR;
	RectangleType		vacated;
	WinDirectionType	direction;


	table = GetObjectPtr (ListTable);
	CurrentRecord = noRecordSelected;


	// Find the new top visible record
	newTopVisibleRecord = TopVisibleRecord;

	// Scroll down.
	if (linesToScroll > 0)
		SeekRecord (&newTopVisibleRecord, linesToScroll, dmSeekForward);

	// Scroll up.
	else if (linesToScroll < 0)
		SeekRecord (&newTopVisibleRecord, -linesToScroll, dmSeekBackward);

	ErrFatalDisplayIf (TopVisibleRecord == newTopVisibleRecord,
					   "Invalid scroll value");

	TopVisibleRecord = newTopVisibleRecord;


	// Move the bits that will remain visible.
	rows = ListViewNumberOfRows (table);
	if (((linesToScroll > 0) && (linesToScroll < rows)) ||
		((linesToScroll < 0) && (-linesToScroll < rows)))
	{
		scrollAmount = 0;

		if (linesToScroll > 0)
		{
			lastRow = TblGetLastUsableRow (table) - 1;
			for (i = 0; i < linesToScroll; i++)
			{
				scrollAmount += TblGetRowHeight (table, lastRow);
				TblRemoveRow (table, 0);
			}
			direction = winUp;
		}
		else
		{
			for (i = 0; i < -linesToScroll; i++)
			{
				scrollAmount += TblGetRowHeight (table, 0);
				TblInsertRow (table, 0);
			}
			direction = winDown;
		}

		TblGetBounds (table, &scrollR);
		WinScrollRectangle (&scrollR, direction, scrollAmount, &vacated);
		WinEraseRectangle (&vacated, 0);
	}


	ListViewLoadTable (FrmGetActiveForm ());
	TblRedrawTable(table);
}



/***********************************************************************
 *
 * FUNCTION:    ListViewInit
 *
 * DESCRIPTION: This routine initializes the "List View" of the
 *              Sample application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    nothing.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95		Initial Revision
 *
 ***********************************************************************/
static void ListViewInit (FormPtr frm)
{
	ControlPtr ctl;

	ListViewLoadRecords (frm);

	// Set the label of the category trigger.
	ctl = GetObjectPtr (ListCategoryTrigger);
	CategoryGetName (ItemDB, CurrentCategory, CategoryName);
	CategorySetTriggerLabel (ctl, CategoryName);

	CurrentView = ListView;
}


/***********************************************************************
 *
 * FUNCTION:    ListViewInvertMoveIndicator
 *
 * DESCRIPTION:   If draw is true, then save the area behind the rectangle,
 *					then draw the indicator there. If draw is false, then restore
 *					the screen bits.
 *
 *
 * PARAMETERS:	 itemR - bounds of the move indicator
 *					 savedBits - if draw is true, then restore this window of bits at
 *					 	itemR.
 *					 draw - draw or erase the move indicator.
 *
 * RETURNED:	 WinHandle - handle to a saved window of screen bits, if the move
 *				indicator is visible. Otherwise, the value is 0.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/29/96	Initial Revision
 *
 ***********************************************************************/
static WinHandle ListViewInvertMoveIndicator (RectanglePtr itemR, WinHandle savedBits,
											  Boolean draw)
{
	UInt16 i;
	UInt16 err;
	WinHandle winH = 0;
	RectangleType indictatorR;
	CustomPatternType pattern;
	CustomPatternType savedPattern;


	indictatorR.topLeft.x = itemR->topLeft.x;
	indictatorR.topLeft.y = itemR->topLeft.y + itemR->extent.y - 2;
	indictatorR.extent.x = itemR->extent.x;
	indictatorR.extent.y = 2;

	if (draw)
	{
		WinGetPattern (&savedPattern);

		for (i = 0; i < sizeof (CustomPatternType) / sizeof (*pattern); i++)
			pattern[i]= (UInt8)0xAA55;

		WinSetPattern ((const CustomPatternType *)&pattern);

		winH = WinSaveBits (&indictatorR, &err);

		WinFillRectangle (&indictatorR, 0);

		WinSetPattern ((const CustomPatternType *)&savedPattern);
	}

	else
	{
		WinRestoreBits (savedBits, indictatorR.topLeft.x, indictatorR.topLeft.y);
	}

	return (winH);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectTableItem
 *
 * DESCRIPTION: This routine either selects or unselects the specified
 *					 table item.
 *
 * PARAMETERS:	 selected - specifies whether an item should be selected or
 *									unselected
 *					 table	 - pointer to a table object
 *              row      - row of the item (zero based)
 *              column   - column of the item (zero based)
 *              rP       - pointer to a structure that will hold the bound
 *                         of the item
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			jmp	10/29/99	Initial Revision
 *			jmp	11/12/99	While a table item is "on the move," having it be selected
 *								can cause the Table code grief.  So, instead of using
 *								the TblSelectItem()/TblUnhighlightSelect() calls, we now
 *								manually select/unselect the table's row.  Before color,
 *								only WinInvertRectangle() was called, so this is now in line
 *								again with the way things used to work.  Sigh.
 *
 ***********************************************************************/

static void ListViewSelectTableItem (Boolean selected, TablePtr table, Int16 row, Int16 column, RectangleType *r)
{
	// Get the item's rectangle.
	//
	TblGetItemBounds (table, row, column, r);

	// Set up the drawing state the way we want it.
	//
	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

	// Erase and (re)draw the item.
	//
	WinEraseRectangle(r, 0);
	ListViewDrawRecord(table, row, column, r);

	// If selected, make it look that way.
	//
	if (selected)
		ReplaceTwoColors (r, 0,
						  UIObjectForeground, UIFieldBackground,
						  UIObjectSelectedForeground, UIObjectSelectedFill);

	// Restore the previous drawing state.
	//
	WinPopDrawState();
}


/***********************************************************************
 *
 * FUNCTION:    ListViewSelectItem
 *
 * DESCRIPTION: This routine tracks an item for either selection
 *					 to go to EditView, or movement in the ListView.
 *
 *
 * PARAMETERS:	 event
 *
 * RETURNED:	 nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	1/29/96	Initial Revision
 *			jmp	10/29/99	Replaced WinInvertRectangle() calls with calls to
 *								ListViewSelectTableItem() to eliminate general
 *								color inversion problem.
 *			peter	4/25/00	Add support for un-masking just the selected record.
 *			gap	06/06/00	Correct what appears to be a copy/paste error.  When
 *								row < 0, the row was properly incremented but the
 *								y value of the move indicator was decremented causing the
 *								tracking line to draw up across the title area of the form.
 *
 ***********************************************************************/

static void ListViewSelectItem (EventType * event)
{
	Int16 sortOrder;
	Int16 row;
	Int16 selectedRow;
	Int16 column;
	UInt16 recordNum;
	Int16 selectedRecord;
	Coord x, y;
	Boolean penDown = true;
	Boolean moving = false;
	Boolean selected = true;
	TablePtr table;
	WinHandle savedBits = 0;
	RectangleType r;
	UInt16 attr;

	sortOrder = SampleGetSortOrder (ItemDB);

	row = event->data.tblSelect.row;
	column = event->data.tblSelect.column;
	table = event->data.tblSelect.pTable;

	// Highlight the item the pen when winDown on.
	selectedRecord = TblGetRowID (table, row);
	ListViewSelectTableItem (selected, table, row, column, &r);

	// Trace the pen until it move enough to constitute a move operation or until
	// the pen it released.
	while (true)
	{
		PenGetPoint (&x, &y, &penDown);
		if (! penDown) break;

		if (! moving)
		{
			if (sortOrder != soAlphabetic)
			{
				// Is the pen still within the bounds of the item it went winDown on,
				// if not draw the move indicator.
				if (! RctPtInRectangle (x, y, &r))
				{
					moving = true;

					TblGetItemBounds (table, row, column, &r);
					savedBits = ListViewInvertMoveIndicator (&r, 0, true);
				}
			}
			else
				selected = RctPtInRectangle (x, y, &r);
		}

		else if (! RctPtInRectangle (x, y, &r))
		{
			// Above the first item ?
			if (row < 0)
			{
				if (y >= r.topLeft.y)
				{
					row++;
					ListViewInvertMoveIndicator (&r, savedBits, false);
					r.topLeft.y += r.extent.y;
					savedBits = ListViewInvertMoveIndicator (&r, 0, true);
				}
			}

			// Move winUp.
			else if (y < r.topLeft.y)
			{
				recordNum = TblGetRowID (table, row);
				if (SeekRecord (&recordNum, 1, dmSeekBackward))
				{
					ListViewInvertMoveIndicator (&r, savedBits, false);
					if (row)
						row--;
					else
					{
						ListViewScroll (-1);
						if (TblFindRowID (table, selectedRecord, &selectedRow))
							ListViewSelectTableItem (selected, table, selectedRow, column, &r);
					}
					TblGetItemBounds (table, row, column, &r);
					savedBits = ListViewInvertMoveIndicator (&r, 0, true);
				}
				else if (row == 0)
				{
					row--;
					ListViewInvertMoveIndicator (&r, savedBits, false);
					r.topLeft.y -= r.extent.y;
					savedBits = ListViewInvertMoveIndicator (&r, 0, true);
				}
			}

			// Move winDown
			else
			{
				recordNum = TblGetRowID (table, row);
				if (SeekRecord (&recordNum, 1, dmSeekForward))
				{
					ListViewInvertMoveIndicator (&r, savedBits, false);
					if (row < TblGetLastUsableRow (table))
						row++;
					else
					{
						ListViewScroll (1);
						if (TblFindRowID (table, selectedRecord, &selectedRow))
							ListViewSelectTableItem (selected, table, selectedRow, column, &r);
					}
					TblGetItemBounds (table, row, column, &r);
					savedBits = ListViewInvertMoveIndicator (&r, 0, true);
				}
			}
		}
	}


	// Turn off the move indicator, if it is on.
	if (moving)
	{
		//savedBits = ListViewInvertMoveIndicator (&r, savedBits, false);
		ListViewInvertMoveIndicator (&r, savedBits, false);
	}

	// If the highlighted item is visible, unhighlight it.
	if (TblFindRowID (table, selectedRecord, &selectedRow))
		ListViewSelectTableItem (false, table, selectedRow, column, &r);

	if (moving)
	{
		if (row >= 0)
		{
			recordNum = TblGetRowID (table, row);
			if (selectedRecord == recordNum)
				return;

			recordNum++;
		}
		else
		{
			recordNum = TblGetRowID (table, 0);;
		}

		DmMoveRecord (ItemDB, selectedRecord, recordNum);
		/* Was
		 if (selectedRecord < TopVisibleRecord)
		 TopVisibleRecord--;
		 CurrentRecord = recordNum;
		 ListViewLoadTable (FrmGetActiveForm());
		 */
		ListViewLoadRecords (FrmGetActiveForm());
		TblRedrawTable (table);
	}

	// If we didn't move the item then it's been selected for editing, go to the
	// edit view.
	else if (sortOrder != soAlphabetic || selected)
	{
		CurrentRecord = TblGetRowID (event->data.tblSelect.pTable,
									 event->data.tblSelect.row);
		EditScrollPosition = 0;

		// Get the category and secret attribute of the current record.
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);

		// If this is a "private" record, then determine what is to be shown.
		if (attr & dmRecAttrSecret)
		{
			switch (PrivateRecordVisualStatus)
			{
			case showPrivateRecords:
				FrmGotoForm (EditView);
				break;

			case maskPrivateRecords:
				if (SecVerifyPW (showPrivateRecords) == true)
				{
					// We only want to unmask this one record, so restore the preference.
					PrefSetPreference (prefShowPrivateRecords, maskPrivateRecords);

					FrmGotoForm (EditView);
				}
				break;

				// This case should never be executed!!!!!!!
			case hidePrivateRecords:
			default:
				break;
			}
		}
		else
		{
			FrmGotoForm (EditView);
		}
	}
}

/***********************************************************************
 *
 * FUNCTION:    ListViewUpdateDisplay
 *
 * DESCRIPTION: This routine update the display of the list view
 *
 * PARAMETERS:  updateCode - a code that indicated what changes have been
 *                           made to the view.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	7/19/95	Initial Revision
 *
 ***********************************************************************/
static Boolean ListViewUpdateDisplay (UInt16 updateCode)
{
	TablePtr table;

	if (updateCode & (updateDisplayOptsChanged | updateFontChanged))
	{
		if (updateCode & updateDisplayOptsChanged)
			TopVisibleRecord = 0;

		ListViewLoadRecords (FrmGetActiveForm());
		table = GetObjectPtr (ListTable);
		TblEraseTable (table);
		TblDrawTable (table);

		return (true);
	}

	return (false);
}


/***********************************************************************
 *
 * FUNCTION:    ListViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "List View"
 *              of the Sample application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * HISTORY:
 *		02/21/95	art	Created by Art Lamb.
 *		11/22/98	kwk	Handle command keys in separate code block so that
 *							TxtCharIsPrint doesn't get called w/virtual chars.
 *		09/25/99	kwk	Use TxtGlueUpperChar to capitalize initial char for
 *							item that's autocreated by writing a printable char.
 *
 ***********************************************************************/
static Boolean ListViewHandleEvent (EventPtr event)
{
	FormPtr	frm;
	Boolean	handled = false;
	UInt16	attr;
	UInt32 numLibs;

	if (event->eType == keyDownEvent)
	{
		// Sample button pressed?
		if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
		{
			if (!(event->data.keyDown.modifiers & poweredOnKeyMask))
				ListViewNextCategory ();
			handled = true;
		}

		else if (EvtKeydownIsVirtual(event))
		{
			// Scroll up key presed?
			if (event->data.keyDown.chr == vchrPageUp)
			{
				ListViewPageScroll (winUp);
				handled = true;
			}

			// Scroll down key presed?
			else if (event->data.keyDown.chr == vchrPageDown)
			{
				ListViewPageScroll (winDown);
				handled = true;
			}

			// Send Data key presed?
			else if (event->data.keyDown.chr == vchrSendData)
			{
				ListViewDoCommand(ListRecordBeamCategoryCmd);
				handled = true;
			}
		}

		// If printable character, create a new record.
		else if (TxtCharIsPrint (event->data.keyDown.chr))
		{
			if (CreateRecord ())
			{
				FrmGotoForm (EditView);

				//event->data.keyDown.chr = TxtGlueUpperChar(event->data.keyDown.chr);
				EvtAddEventToQueue (event);
			}

			handled = true;
		}
	}


	else if (event->eType == ctlSelectEvent)
	{
		switch (event->data.ctlSelect.controlID)
		{
		case ListNewButton:
			if (CreateRecord ())
				FrmGotoForm (EditView);
			handled = true;
			break;

		case ListCategoryTrigger:
			ListViewSelectCategory ();
			handled = true;
			break;
		}
	}


	else if (event->eType == tblEnterEvent)
	{
		ListViewSelectItem (event);
		handled = true;
	}


	else if (event->eType == tblSelectEvent)
	{
		// An item in the list of items was selected, display it.
		CurrentRecord = TblGetRowID (event->data.tblSelect.pTable,
									 event->data.tblSelect.row);
		EditScrollPosition = 0;

		// Get the category and secret attribute of the current record.
		DmRecordInfo (ItemDB, CurrentRecord, &attr, NULL, NULL);

		// If this is a "private" record, then determine what is to be shown.
		if (attr & dmRecAttrSecret)
		{
			switch (PrivateRecordVisualStatus)
			{
			case showPrivateRecords:
				FrmGotoForm (EditView);
				break;

			case maskPrivateRecords:
				//					FrmGotoForm ();
				break;

				// This case should never be executed!!!!!!!
			case hidePrivateRecords:
			default:
				break;
			}
		}
		else
		{
			FrmGotoForm (EditView);
		}

		handled = true;
	}


	else if (event->eType == menuOpenEvent)
	{
		if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
			MenuHideItem(ListRecordSendCategoryCmd);
		else
			MenuShowItem(ListRecordSendCategoryCmd);
		// don't set handled = true
	}

	else if (event->eType == menuEvent)
	{
		ListViewDoCommand (event->data.menu.itemID);
		return (true);
	}


	else if (event->eType == frmOpenEvent)
	{
		frm = FrmGetActiveForm ();
		ListViewInit (frm);
		FrmDrawForm (frm);
		FrmNavObjectTakeFocus(frm, ListNewButton);
		handled = true;
	}

	else if (event->eType == menuCmdBarOpenEvent)
	{
		MenuCmdBarAddButton(menuCmdBarOnLeft, BarSecureBitmap, menuCmdBarResultMenuItem, ListOptionsSecurityCmd, 0);

		// tell the field package to not add buttons automatically; we've done it all ourselves.
		event->data.menuCmdBarOpen.preventFieldButtons = true;

		// don't set handled to true; this event must fall through to the system.
	}

	else if (event->eType == frmUpdateEvent)
	{
		handled =  ListViewUpdateDisplay (event->data.frmUpdate.updateCode);
	}

	else if (event->eType == sclRepeatEvent)
	{
		ListViewScroll (event->data.sclRepeat.newValue -
						event->data.sclRepeat.value);
	}

	return (handled);
}


#pragma mark ----


/***********************************************************************
 *
 * FUNCTION:    OpenFile
 *
 * DESCRIPTION: This routine opens a text file on a VFS volume.
 *
 * PARAMETERS:  volRefNum - the VFS volume
 *				path - the path to the file
 *
 * RETURNED:    Error if any
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dje		3/15/04		Initial Revision
 *
 ***********************************************************************/
static Err OpenFile (UInt16 volRefNum, Char * path)
{
	Err err = errNone;
	
	CurrentRecord = fileSelected;
	CurrentVolRefNum = volRefNum;
	CurrentPath = MemPtrNew (kFileBrowserLibPathBufferSize);
	if (CurrentPath == NULL)
	{
		ErrNonFatalDisplay ("Can't alloc path");
		return memErrNotEnoughSpace;
	}
	ErrNonFatalDisplayIf (StrLen (path) > kFileBrowserLibMaxPathLength, "Path too long");
	StrCopy (CurrentPath, path);
	EditScrollPosition = 0;
	FrmGotoForm (EditView);
	return err;
}


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
 *			gavin   11/9/99  Rewritten to use new ExgDoDialog function
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
 *			art	9/11/95		Initial Revision
 *
 ***********************************************************************/
static Boolean ApplicationHandleEvent (EventType * event)
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
		// active form is called by FrmHandleEvent each time it receives an
		// event.
		switch (formID)
		{
		case ListView:
			FrmSetEventHandler (frm, ListViewHandleEvent);
			break;

		case EditView:
			FrmSetEventHandler (frm, EditViewHandleEvent);
			break;

		case DetailsDialog:
			FrmSetEventHandler (frm, DetailsHandleEvent);
			break;

		case PreferencesDialog:
			FrmSetEventHandler (frm, PreferencesHandleEvent);
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
 * DESCRIPTION: This routine is the event loop for the Sample
 *              application.
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
 * DESCRIPTION: This is the main entry point for the Sample
 *              application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	2/21/95	Initial Revision
 *			art	1/3098	Removed sysAppLaunchCmdSaveData logic
 *			grant	6/25/99	In sysAppLaunchCmdInitDatabase, set the backup bit on the DB.
 *								In sysAppLaunchCmdExgReceiveData, update ItemsInCategory.
 *			jmp	10/02/99	Made the support for the sysAppLaunchCmdExgReceiveData
 *								sysAppLaunchCmdExgAskUser launch codes more like their
 *								counterparts in Address, Databook, and ToDo.
 *			jmp	10/18/99	If the default "demo" database image doesn't exist, then
 *								create an empty database instead.
 *			jmp	11/04/99	Eliminate extraneous FrmSaveAllForms() call from sysAppLaunchCmdExgAskUser
 *								since it was already being done in sysAppLaunchCmdExgReceiveData if
 *								the user affirmed sysAppLaunchCmdExgAskUser.  Also, in sysAppLaunchCmdExgReceiveData
 *								prevent call FrmSaveAllForms() if we're being call back through
 *								PhoneNumberLookup() as the two tasks are incompatible with each other.
 *
 ***********************************************************************/
UInt32	PilotMain (UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	UInt16 error;
	DmOpenRef dbP;
	UInt32 value;
	UInt32 deviceID = 0;
	
	if (cmd == sysAppLaunchCmdNormalLaunch)
	{
	    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &deviceID);

        // This app is only supported on LifeDrive and Tungsten T5 devices
	    if (deviceID != kPalmOneDeviceIDTungstenX  &&
	        deviceID != kPalmOneDeviceIDAngus &&
	        deviceID != kPalmOneDeviceIDTungstenT5 &&
	        deviceID != 'Pmpk')
	    {
	        FrmAlert(UnsupportedAlert);
	        return 1;	 
	    } 
	
	
		// Register for convenience when running via the debugger
		RegisterData();
		RegisterFileBrowser();
		
		// Clean up if we never get the goto launch for a "file:" URL
		if (FtrGet (kAppCreator, ftrPtrNumURL, &value) == errNone)
			FtrPtrFree (kAppCreator, ftrPtrNumURL);
		
		error = StartApplication ();
		if (error)
			return (error);

		FrmGotoForm (CurrentView);
		EventLoop ();
		StopApplication ();
	}

	else if (cmd == sysAppLaunchCmdFind)
	{
		Search ((FindParamsPtr)cmdPBP);
	}


	// This action code might be sent to the app when it's already running
	//  if the use hits the Find soft key next to the Graffiti area.
	else if (cmd == sysAppLaunchCmdGoTo)
	{
		if (launchFlags & sysAppLaunchFlagNewGlobals)
		{
			error = StartApplication ();
			if (error) return (error);

			error = GoToItem ((GoToParamsPtr) cmdPBP, true);

			if (!error)
				EventLoop ();
			StopApplication ();
		}
		else
			error = GoToItem ((GoToParamsPtr) cmdPBP, false);
	}


	// Launch code sent to running app before sysAppLaunchCmdFind
	// or other action codes that will cause data searches or manipulation.
	// We don't need to respond to this launch code because items are
	// edited in place.
	//	else if (cmd == sysAppLaunchCmdSaveData)
	//		{
	//		FrmSaveAllForms ();
	//		}


	// This launch code is sent after the system is reset.  We use this time
	// to create our default database.  If there is no default database image,
	// then we create an empty database.
	else if (cmd == sysAppLaunchCmdSystemReset)
	{
		if (((SysAppLaunchCmdSystemResetType*)cmdPBP)->createDefaultDB)
		{
			error = CreateDefaultDatabase();
			// Register to receive and edit .txt and text/plain on hard reset.
			RegisterData();
			RegisterFileBrowser();
		}
		RegisterLocaleChangingNotification();
	}


	else if (cmd == sysAppLaunchCmdSyncNotify)
	{
		SyncNotification ();
	}


	else if (cmd == sysAppLaunchCmdExgAskUser)
	{
		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
		{
			error = SampleGetDatabase (&dbP, dmModeReadWrite);
		}
		else
			dbP = ItemDB;

		if (dbP != NULL)
		{
			CustomAcceptBeamDialog (dbP, (ExgAskParamPtr) cmdPBP);

			if (!(launchFlags & sysAppLaunchFlagSubCall))
				error = DmCloseDatabase(dbP);
		}
	}


	// Present the user with ui to perform a lookup and return a string
	// with information from the selected record.
	else if (cmd == sysAppLaunchCmdExgReceiveData)
	{
		UInt16 numReceived = 0;
		UInt32 currentUID;
		ExgSocketType *socketP = cmdPBP;
		void *ftrP;

		// If this is a "file:" URL, stuff it into a feature pointer and arrange for a goto
		if (socketP->name && StrNCaselessCompare (socketP->name, "file:", 5) == 0)
		{
			error = FtrPtrNew (kAppCreator, ftrPtrNumURL, StrLen(socketP->name) + 1, &ftrP);
			if (!error)
			{
				DmStrCopy (ftrP, 0, socketP->name);
				socketP->goToCreator = kAppCreator;
				MemSet (&socketP->goToParams, sizeof(socketP->goToParams), 0);
			}
			return error;
		}

		// if our app is not active, we need to open the database
		// the subcall flag is used here since this call can be made without launching the app
		if (!(launchFlags & sysAppLaunchFlagSubCall))
			error = SampleGetDatabase (&dbP, dmModeReadWrite);
		else
		{
			dbP = ItemDB;

			// We don't delete the current record if it's empty because the user
			// could cancel the beam receive.

			// SampleReceiveData() calls SampleSort(), which may change the index of
			// the current record. So we remember its UID here, and refresh our
			// copy of its index afterwards.
			if (CurrentRecord != noRecordSelected)
				DmRecordInfo(dbP, CurrentRecord, NULL, &currentUID, NULL);
		}

		if (dbP != NULL)
		{
			error = SampleReceiveData(dbP, socketP, &numReceived);

			// We may have just added some items to the current category.
			// If the app is currently running, update ItemsInCategory to reflect this.
			if (launchFlags & sysAppLaunchFlagSubCall)
			{
				ItemsInCategory += numReceived;

				if (CurrentRecord != noRecordSelected)
				{
					if (DmFindRecordByID(dbP, currentUID, &CurrentRecord) != 0)
						CurrentRecord = noRecordSelected;	// Can't happen, but...
				}
			}
			else
				DmCloseDatabase(dbP);
		}
		else
			error = exgErrAppError;	
		// If we can't open our database, return the error since it wasn't passed to ExgDisconnect
		return error;
	}

	else if(cmd == sysAppLaunchCmdExgPreview)
	{
		SampleTransferPreview((ExgPreviewInfoType *)cmdPBP);
	}

	// This action code is sent by the DesktopLink server when it create
	// a new database.  We will initializes the new database.
	else if (cmd == sysAppLaunchCmdInitDatabase)
	{
		SampleAppInfoInit (((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);

		// Set the backup bit.  This is to aid syncs with non Palm software.
		SetDBBackupBit(((SysAppLaunchCmdInitDatabaseType*)cmdPBP)->dbP);
	}

	else if (cmd == sysAppLaunchCmdNotify)
	{
		if (((SysNotifyParamType*)cmdPBP)->notifyType == sysNotifyLocaleChangedEvent)
		{
			DmSearchStateType	searchState;
			LocalID	dbID;
			UInt16	cardNo;

			// Since the locale has changed, delete the existing database
			// and re-create it for the new locale
			error = DmGetNextDatabaseByTypeCreator (true, &searchState, itemDBType,
													kAppCreator, true, &cardNo, &dbID);
			if (!error)
				DmDeleteDatabase(cardNo, dbID);

			error = CreateDefaultDatabase();

		}
	}

	return (errNone);
}
