/******************************************************************************
 *
 * Copyright (c) 1999-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: DateAgenda.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *	  Display and support code for Datebook's Agenda view.
 *
 * History:
 *		May 28, 1999	Created by Russell Brenner
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <FntGlue.h>

#include <PalmUtils.h>

#include "sections.h"
#include "Datebook.h"
#include "DateTime.h"
#include "ToDoDB.h"
#include <stddef.h>

#include "debug.h"

extern void ECApptDBValidate (DmOpenRef dbP);


/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/

#define minDatebookRows					1
#define maxDatebookRows					6
#define scrollerInset					1

#define minToDoRows						0

#define adjustSeparatorTop				2
#define adjustCategoryTop				2
#define adjustCategoryBottom			0

// DOLATER ??? - Copied from ToDo.c. Remove when obviated by ToDoLib

#define toDoPrefsVersionNum			3
#define todoPrefID						0x00
#define toDoDBName						"ToDoDB"
#define toDoDBType						'DATA'

// Columns in the ToDo table of the list view.
#define completedColumn					0
#define priorityColumn					1
#define descColumn						2
#define dueDateColumn					3
#define categoryColumn					4

#define spaceBeforeDesc					2
#define spaceBeforeCategory			2

#define maxDateTemplateLen				31


// Field numbers, used to indicate where search string was found.
#define descSearchFieldNum				0
#define noteSearchFieldNum				1

#define noAppointments					0xffff
#define noTasks							0xffff

#if WRISTPDA
// Values to indicate list selection focus.
#define AgendaFocusNone					0
#define AgendaFocusApptList				1
#define AgendaFocusToDoList				2
#endif

typedef struct {
	UInt16				currentCategory;
	FontID			v20NoteFont;		// For 2.0 compatibility (BGT)
	Boolean			showAllCategories;
	Boolean 			showCompletedItems;
	Boolean 			showOnlyDueItems;
	Boolean			showDueDates;
	Boolean			showPriorities;
	Boolean			showCategories;
	Boolean			saveBackup;
	Boolean			changeDueDate;
	
	// Version 3 preferences
	FontID			listFont;
	FontID			noteFont;		// For 3.0 and later units.	(BGT)
	
	UInt8				reserved;
} ToDoPreferenceType;


/***********************************************************************
 *
 *	Globals
 *
 ***********************************************************************/

static AgendaType					Agenda;
extern privateRecordViewEnum	PrivateRecordVisualStatus;
extern UInt16				PendingUpdate;					// code of pending day view update

#if WRISTPDA
static UInt16 SelectionFocus = AgendaFocusNone;
static UInt16 SelectedRecord = 0;
#endif

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

void					AgendaViewInitDatebook (
								FormPtr							frm );
void					AgendaViewInitToDo (
								FormPtr							frm );
static void			AgendaViewChangeDate (
								FormType*						inFormP,
								DateType							inDate ) EXTRA_SECTION_THREE;
static void AgendaViewCountDisplayLines (
								FormType*						inFormP,
								UInt16							inApptCount,
								UInt16							inToDoCount,
								UInt16*							outApptLinesP,
								UInt16*							outToDoLinesP ) EXTRA_SECTION_THREE;
static void 		AgendaViewLayout (
								FormType*						inFormP,
								UInt16							inApptCount,
								UInt16							inToDoCount ) EXTRA_SECTION_THREE;
static void			AgendaViewLayoutTables (
								FormType*						inFormP,
								UInt16							inApptLineCount,
								UInt16							inToDoLineCount ) EXTRA_SECTION_THREE;
static void 		AgendaViewSetTopAppointment ();
static void			AgendaViewSetTopTask ();
static void			AgendaViewLayoutAppointments (
								FormType*						inFormP,
								RectangleType*					ioBoundsP,
								UInt16							inApptLineCount ) EXTRA_SECTION_THREE;
static void			AgendaViewLayoutSeparator (
								FormType*						inFormP,
								RectangleType*					ioBoundsP ) EXTRA_SECTION_THREE;
static UInt16		AgendaViewObjectSetTop (
								FormType*						inFormP,
								UInt16							inObjectID,
								UInt16							inTop ) EXTRA_SECTION_THREE;
static void			AgendaViewLayoutTasks (
								FormType*						inFormP,
								RectangleType*					ioBoundsP,
								UInt16							inToDoLineCount ) EXTRA_SECTION_THREE;
static void			AgendaViewUpdateScroller (
								FormType*						inFormP,
								UInt16							inObjectID,
								UInt16							inAssocObjectID,
								UInt16							inRecordCount,
								UInt16							inVisibleCount,
								UInt16							inLineHeight ) EXTRA_SECTION_THREE;
static void			AgendaViewEraseObject (
								FormType*						inFormP,
								UInt16							inObjectID ) EXTRA_SECTION_THREE;
static void			AgendaViewSetScrollThumb (
								FormType*						inFormP,
								UInt16							inObjectID,
								UInt16							inValue );
static void			AgendaViewRefreshLayoutTasks (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
//static void			AgendaViewFillTitle (
								//FormPtr							frm );
//static void			AgendaViewFillAppointmentsTitle (
								//FormType*						frmP );
static void			AgendaViewFillAppointments (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
//static void			AgendaViewFillTasksTitle (
								//FormType*						inFormP,
								//RectangleType*					ioBoundsP );
static void			AgendaViewFillTasks (
								FormType*						inFormP );
static void			AgendaDayViewScroll (
								Int16								delta) EXTRA_SECTION_THREE;
static void			AgendaViewGoToDate ( );

static void			AgendaViewDrawDate (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
static void			AgendaViewDrawTime (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
static void			AgendaViewDrawSeparator (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
static void			AgendaDateToDOWDM (
								DateType							inDate,
								Char*								outAscii ) EXTRA_SECTION_THREE;

static Err			AgendaViewGetApptDesc (
								void*								table,
								Int16								row,
								Int16								column,
								Boolean							editable,
								MemHandle*						dataH,
								Int16*							dataOffset,
								Int16*							textAllocSize,
								FieldPtr							fld ) EXTRA_SECTION_THREE;
static Boolean		AgendaDividerDraw (
								//FormGadgetType*				inGadgetP,
								FormGadgetTypeInCallback*				inGadgetP,
								UInt16							inCommand,
								void*								inParamP ) EXTRA_SECTION_THREE;
//static void			AgendaTitleDraw (
								//void*								inTableP,
								//Int16								inRow,
								//Int16								inColumn,
								//RectangleType*					inBoundsP );
static void			AgendaApptDrawTime (
								void*								inTableP,
								Int16								inRow,
								Int16								inColumn,
								RectangleType*					inBoundsP ) EXTRA_SECTION_THREE;
static void			AgendaApptDrawDesc (
								void*								inTableP,
								Int16								inRow,
								Int16								inColumn,
								RectangleType*					inBoundsP ) EXTRA_SECTION_THREE;
	

Boolean				AgendaViewHandleTblEnter (
								EventType*							event ) EXTRA_SECTION_THREE;

static MemHandle	AgendaLoadAppointments (
								DateType							inDate ) EXTRA_SECTION_THREE;
static void			AgendaFreeAppointments ( );
static MemHandle	AgendaGetAppointments ( ) EXTRA_SECTION_THREE;
static void			AgendaSetAppointments (
								const MemHandle				inAppointmentsH,
								UInt16							inAppointmentCount ) EXTRA_SECTION_THREE;
static UInt16		AgendaGetAppointmentCount ( );
static void			AgendaViewGetScrollableRect (
								FormType*						inFormP,
								RectangleType*					outBoundsP ) EXTRA_SECTION_THREE;
static UInt16		AgendaViewGetSeparatorHeight (
								FormType*						inFormP ) EXTRA_SECTION_THREE;
static UInt16		AgendaViewGetDatebookDefaultHeight (
								FormType*						inFormP ) EXTRA_SECTION_THREE;

static Boolean		SeekRecord (
								DmOpenRef						inDB,
								UInt16*							inIndexP,
								UInt16							inOffset,
								Int16								inDirection,
								UInt16							inCategory,
								DateType							inDate,
								Boolean							inShowCompleted,
								Boolean							inShowDueOnly ) EXTRA_SECTION_THREE;
static UInt16		CountRecords (
								DmOpenRef						inDB,
								UInt16*							outIndices,
								UInt16							inIndexCount,
								UInt16							inCategory,
								DateType							inDate,
								Boolean							inShowCompleted,
								Boolean							inShowDueOnly ) EXTRA_SECTION_THREE;
static void ListInitTableRow (TablePtr table, UInt16 row, UInt16 recordNum, 
	Int16 rowHeight);

void					LaunchToDoWithRecord (
								DmOpenRef					inDB,
								UInt16						inRecordNum ) EXTRA_SECTION_THREE;
void					GoToAppointment (
								DmOpenRef					inDB,
								UInt16						inRecordNum ) EXTRA_SECTION_THREE;

#if WRISTPDA
static void AgendaViewMoveApptSelection ( Int16 Direction );
static void AgendaViewMoveToDoSelection ( Int16 Direction );
#endif


/***********************************************************************
 *
 *	Temporary Stuff
 *
 ***********************************************************************/

// DOLATER ??? - Temporary hack. Move prefs code into new To Do library.
// DOLATER ??? - Use new API instead of globals to access these settings.
//static FontID		NoteFont = FossilBoldFont;				// font used in note view
//FontID		NoteFont = boldFont;				// font used in note view
static UInt16		CurrentCategory = dmAllCategories;	// currently displayed category
static char			CategoryName [dmCategoryLength];		// name of the current category
static Boolean		ShowAllCategories = true;				// true if all categories are being displayed
static Boolean 	ShowCompletedItems = true;				// true if completed items are being displayed
static Boolean 	ShowOnlyDueItems = false;				// true if only due items are displayed
static Boolean		ShowDueDates = false;					// true if due dates are displayed in the list view
static Boolean		ShowPriorities = true;					// true if priorities are displayed in the list view
static Boolean		ShowCategories = false;					// true if categories are displayed in the list view
//static Boolean		SaveBackup = true;						// true if save backup to PC is the default
static Boolean		ChangeDueDate = false;					// true if due date is changed to completion date when completed
//static FontID		ListFont = FossilBoldFont;				// font used to draw to do item
static FontID		ListFont = boldFont;				// font used to draw to do item
static UInt16		TopVisibleRecord = 0;
// Number of system ticks (1/60 seconds) to display crossed out item
// before they're erased.
#define crossOutDelay					40


static UInt16 ListViewGetColumnWidth (UInt16 column) EXTRA_SECTION_THREE;
//static Err ListViewGetDescription (void* table, UInt16 row, UInt16 column,
	//Boolean editable, MemHandle* textH, UInt16* textOffset, UInt16* textAllocSize,
	//FieldPtr fld);
//static Boolean ListViewSaveDescription (void* table, UInt16 row, UInt16 column);
static void ListViewDrawDueDate (void* table, Int16 row, Int16 column, 
	RectanglePtr bounds) EXTRA_SECTION_THREE;
static void ListViewDrawCategory (void* table, Int16 row, Int16 column, 
	RectanglePtr bounds) EXTRA_SECTION_THREE;
static void			ListViewDrawDesc (
								void*								inTableP,
								Int16								inRow,
								Int16								inColumn,
								RectangleType*					inBoundsP ) EXTRA_SECTION_THREE;
//static void * GetObjectPtr (UInt16 objectID);
static UInt16 ListViewSelectCategory (void) EXTRA_SECTION_THREE;
static void ListViewDrawTable (UInt16 updateCode) EXTRA_SECTION_THREE;
static void ListViewScroll (Int16 delta) EXTRA_SECTION_THREE;
static void ChangeCategory (UInt16 category) EXTRA_SECTION_THREE;

Err	ToDoAppInfoInit(DmOpenRef dbP);

//static UInt16 CountMemoryChunks(UInt16 ownerID);
//static void WatchMemoryChunkCount(void);

Char* GetToDoNotePtr (ToDoDBRecordPtr recordP);
static void ListViewCrossOutItem (Int16 row) EXTRA_SECTION_THREE;
static void ListViewChangeCompleteStatus (Int16 row, UInt16 complete) EXTRA_SECTION_THREE;

extern void ECToDoDBValidate(DmOpenRef dbP);

extern void GoToItem (GoToParamsPtr goToParams, Boolean launchingApp);

//static void			CheckDBLeaks (
								//DmOpenRef						inDB );
static void ToDoLoadPrefs () EXTRA_SECTION_THREE;


// Checking for leaks takes a long time with large databases,
// so turn it off, by default. At some point, there may be a better
// mechanism for toggling settings like this from the build environment,
// but, for now, just unlock the file and comment out the unwanted setting.
// (Note: This used to be keyed of off ERROR_CHECK_LEVEL, but this is still
// set to ERROR_CHECK_FULL during alpha testing.)

//#define CHECK_DB_LEAKS(db)		CheckDBLeaks (db)
#define CHECK_DB_LEAKS(db)


/***********************************************************************
 *
 * FUNCTION:    AgendaViewInit
 *
 * DESCRIPTION: Initialize the Agenda view of the Datebook application.
 *
 * PARAMETERS:  frm - pointer to the day view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *			CS		11/14/00	Use PrefGetPreference instead of PrefGetPreferences.
 *
 ***********************************************************************/
void
AgendaViewInit (
	FormPtr							frm )
{
	UInt16 mode;
	FontID curFont;

#if EMULATION_LEVEL != EMULATION_NONE
	ECApptDBValidate (ApptDB);
#endif
	
	ToDoLoadPrefs();

	Agenda.apptDB = ApptDB;
	Agenda.apptTopVisibleIndex = 0;
	Agenda.apptFont = ApptDescFont;
	Agenda.apptLineCount = 0;
	
	curFont = FntSetFont (Agenda.apptFont);
	Agenda.apptLineHeight = FntLineHeight ();
	FntSetFont (curFont);
	
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadOnly : (dmModeReadOnly | dmModeShowSecret);
	
	ToDoGetDatabase (&Agenda.todoDB, mode);	// DOLATER ??? - What should be done on failure?
	Agenda.todoLineCount = 0;
	
#ifdef USE_NATIVE_TODO_FONT
	Agenda.todoFont = ListFont;
	curFont = FntSetFont (Agenda.todoFont);
	Agenda.todoLineHeight = FntLineHeight ();
	FntSetFont (curFont);
#else
	Agenda.todoFont = Agenda.apptFont;	// same as appointments
	Agenda.todoLineHeight = Agenda.apptLineHeight;
#endif

	//Agenda.timeFont = FossilBoldFont;
	Agenda.timeFont = boldFont;
	Agenda.timeFormat = (TimeFormatType)PrefGetPreference(prefTimeFormat);
	
//	Agenda.date = Date;
	//Agenda.dateFont = FossilBoldFont;
	Agenda.dateFont = boldFont;
	Agenda.dateFormat =		(DateFormatType) PrefGetPreference (prefDateFormat);
	Agenda.longDateFormat =	(DateFormatType) PrefGetPreference (prefLongDateFormat);
	
	AgendaViewInitDatebook (frm);
	AgendaViewInitToDo (frm);
	
	FrmSetGadgetHandler (frm, FrmGetObjectIndex (frm, AgendaDivider), AgendaDividerDraw);

	// Load the appointments and tasks into the table object that will display them.
	// DOLATER ??? - Not yet sure what details govern this stuff
	if (PendingUpdate && ItemSelected)
		{
		AgendaViewChangeDate (frm, Date);
		}
	else
		{
//		AgendaViewFillTable (frm);
		AgendaViewChangeDate (frm, Date);
		}

	// Highlight the Agenda View push button.
//	FrmSetControlGroupSelection (frm, AgendaViewGroup, AgendaAgendaViewButton);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewInitDatebook
 *
 * DESCRIPTION: Initialize the Datebook portion of the Agenda view.
 *
 * PARAMETERS:  frm - pointer to the agenda view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
void
AgendaViewInitDatebook (
	FormPtr							frm )
{
	UInt16 row;
	UInt16 rowsInTable;
	TablePtr table;

	// Initialize the table used to display the day's agenda.
	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, AgendaDatebookTable));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
		{		
		TblSetItemStyle (table, row, agendaApptTimeColumn, customTableItem);
		TblSetItemStyle (table, row, agendaApptDescColumn, customTableItem);
		#if WRISTPDA
		TblSetItemFont (table, row, agendaApptTimeColumn, FossilLargeFontID( WRISTPDA, ListFont ) );
		TblSetItemFont (table, row, agendaApptDescColumn, FossilLargeFontID( WRISTPDA, ListFont ) );
		#endif
		}

	TblSetColumnUsable (table, agendaApptTimeColumn, true);
	TblSetColumnUsable (table, agendaApptDescColumn, true);
	TblSetColumnMasked (table, agendaApptDescColumn, true);

	TblSetColumnSpacing (table, agendaApptTimeColumn, agendaApptTimeColumnSpacing);

	// Set the callback routine that will load the description field.
	// (No callback for saving, since this is a read-only view.)
	TblSetLoadDataProcedure (table, agendaApptDescColumn, AgendaViewGetApptDesc);
	
	// Set the callback routine that draws the time field.
	TblSetCustomDrawProcedure (table, agendaApptTimeColumn, AgendaApptDrawTime);
	TblSetCustomDrawProcedure (table, agendaApptDescColumn, AgendaApptDrawDesc);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewGetApptDesc
 *
 * DESCRIPTION: This routine returns a pointer to the description field
 *              of a appointment record.  This routine is called by 
 *              the table object as a callback routine when it wants to 
 *              display or edit the appointment's description.
 *
 * PARAMETERS:  inTable			- pointer to the Day View table (TablePtr)
 *              inRow			- row in the table
 *              inColumn		- column in the table
 *              inEditable		- true if the field will be edited by the table
 *					 outHandleP		- handle to the appointment record
 *              outOffsetP		- offset within the record of the desc field (returned)
 *              outSizeP		- allocated size the the description field (returned)
 *					 ioFieldP		- Table Mgr's field description
 *
 * RETURNED:    handle of the appointment record
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static Err AgendaViewGetApptDesc (
	void*								inTableP,
	Int16								inRow,
	Int16								inColumn,
	Boolean							inEditable,
	MemHandle*						outHandleP,
	Int16*							outOffsetP,
	Int16*							outSizeP,
	FieldType*						ioFieldP )
{
//#pragma unused (inColumn, inEditable)

	Err								err = errNone;
	UInt16							apptIndex;
	ApptInfoPtr						appts;
	UInt16							recordNum;
	MemHandle						recordH;
	ApptDBRecordType				apptRec;
	FieldAttrType					attributes;
	
	Assert (inColumn == agendaApptDescColumn );
	Assert (outHandleP);
	Assert (outOffsetP);
	Assert (outSizeP);
	Assert (ioFieldP);
	
	*outHandleP = 0;

	// Override the default field attributes
	FldGetAttributes (ioFieldP, &attributes);
	attributes.underlined = false;
	attributes.editable = false;
	FldSetAttributes (ioFieldP, &attributes);

	// Get the appointment that corresponds to the table item.
	// The index of the appointment in the appointment list is stored
	// as the row id.
	apptIndex = TblGetRowID (inTableP, inRow);
	
	// If there aren't any appointments, show a message to that effect
	if (apptIndex == noAppointments)
		{
		*outHandleP = DmGetResource (strRsc, agendaNoAppointmentsStrID);
		*outOffsetP = 0;
		*outSizeP = MemHandleSize (*outHandleP);
		goto Exit;
		}

	// Get the record index of the next appointment
	appts = MemHandleLock (AgendaGetAppointments ());
	recordNum = appts[apptIndex].recordNum;
	MemPtrUnlock (appts);

	// Get the offset and length of the description field 
	err = ApptGetRecord (Agenda.apptDB, recordNum, &apptRec, &recordH);
	
	if ( err == errNone )
		{
		*outHandleP = recordH;
		
		if ( apptRec.description != NULL )
			{
			//*outOffsetP = (UInt16) ((UInt32) apptRec.description - (UInt32) apptRec.when);
			*outOffsetP = (UInt16) (offsetof(ApptDBRecordType, description) - offsetof(ApptDBRecordType, when));
			*outSizeP = StrLen (apptRec.description) + 1;  // one for null terminator
			}
		else
			{
			*outOffsetP = 0;
			*outSizeP = 0;
			}
		}
		
	MemHandleUnlock (recordH);
	
Exit:
	return (err);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewChangeDate
 *
 * DESCRIPTION: This routine displays the date passed.
 *
 * PARAMETERS:  date - date to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void AgendaViewChangeDate (
	FormType*						inFormP,
	DateType							inDate )
{
	CHECK_DB_LEAKS (Agenda.todoDB);

	// Keep the new date for future reference
	Date = inDate;
	Agenda.timeSeconds = TimGetSeconds ();
	
	// Find out how many appointments and tasks to display
	AgendaLoadAppointments (inDate);		// sets Agenda.apptCount
	
	// Tally up the number of displayable tasks and store the index
	// of the first in TopVisibleRecord
	Agenda.todoCount = CountRecords (Agenda.todoDB, &TopVisibleRecord, 1, CurrentCategory,
										Date, ShowCompletedItems, ShowOnlyDueItems);

	AgendaViewDrawDate (inFormP);
	AgendaViewDrawTime (inFormP);
		
	// Get all the appointments and empty time slots on the new day.
	AgendaViewEraseObject (inFormP, AgendaDatebookTable);
	AgendaViewEraseObject (inFormP, AgendaToDoTable);
	AgendaViewLayout (inFormP, Agenda.apptCount, Agenda.todoCount);
	
	// Determine the top items to display
	AgendaViewSetTopAppointment ();
//	AgendaViewSetTopTask ();
	
	// Fill the display tables
	AgendaViewFillAppointments (inFormP);
	AgendaViewFillTasks (inFormP);

	// Update the scroll arrows
	AgendaViewSetScrollThumb (inFormP, AgendaDatebookScroller, Agenda.apptTopVisibleIndex);
	AgendaViewSetScrollThumb (inFormP, AgendaToDoScroller, 0);

	// Draw the new day's events
	FrmSetControlGroupSelection (inFormP, AgendaViewGroup, AgendaAgendaViewButton);
	FrmDrawForm (inFormP);
	
	#if WRISTPDA
	// Determine selection focus.
	if ( Agenda.apptCount > 0 )
		SelectionFocus = AgendaFocusApptList;
	else if ( Agenda.todoCount > 0 )
		SelectionFocus = AgendaFocusToDoList;
	else
		SelectionFocus = AgendaFocusNone;
	// Determine initial selected record.
	SelectedRecord = TopVisibleRecord = 0;
	if ( SelectionFocus == AgendaFocusApptList )
		SelectedRecord = TopVisibleRecord = Agenda.apptTopVisibleIndex;
	// Display initial selection highlight.
	if ( SelectionFocus == AgendaFocusApptList ) {
		Int16 SelectedRow;
		TablePtr Table;
		Table = GetObjectPtr( AgendaDatebookTable );
		TblFindRowID( Table, SelectedRecord, & SelectedRow );
		TblUnhighlightSelection( Table );
		ListViewDrawTable( updateItemMove );
		AgendaViewMoveApptSelection( 0 );
	} else if ( SelectionFocus == AgendaFocusToDoList ) {
		Int16 SelectedRow;
		TablePtr Table;
		Table = GetObjectPtr( AgendaToDoTable );
		TblFindRowID( Table, SelectedRecord, & SelectedRow );
		TblUnhighlightSelection( Table );
		ListViewDrawTable( updateItemMove );
		AgendaViewMoveToDoSelection( 0 );
	}
	#endif
	
	CHECK_DB_LEAKS (Agenda.todoDB);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewLayout
 *
 * DESCRIPTION: This routine displays the date passed.
 *
 * PARAMETERS:  date - date to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	9/13/99	Initial Revision
 *
 ***********************************************************************/
static void AgendaViewLayout (
	FormType*						inFormP,
	UInt16							inApptCount,
	UInt16							inToDoCount )
{
	UInt16							apptLineCount;
	UInt16							todoLineCount;
	
	AgendaViewCountDisplayLines (inFormP, inApptCount, inToDoCount,
											&apptLineCount, &todoLineCount);
	
	// DOLATER rbb - Enhance performance by processing events during screen refresh,
	// checking to see if the displayed date has changed and, if so, starting over.
	// This code should also use offscreen drawing so that the user will never have
	// to see a partial redraw.
	
	if (Agenda.apptLineCount != apptLineCount)
		{
//		AgendaViewEraseObject (inFormP, AgendaDatebookTable);
		AgendaViewEraseObject (inFormP, AgendaDatebookScroller);
		AgendaViewEraseObject (inFormP, AgendaDivider);
		AgendaViewEraseObject (inFormP, AgendaToDoCategoryTrigger);
		}
		
	if ( (Agenda.apptLineCount != apptLineCount) || (Agenda.todoLineCount != todoLineCount) )
		{
		// Erase To Do
//		AgendaViewEraseObject (inFormP, AgendaToDoTable);
		AgendaViewEraseObject (inFormP, AgendaToDoScroller);
		}

	AgendaViewLayoutTables (inFormP, apptLineCount, todoLineCount);

	AgendaViewUpdateScroller (inFormP, AgendaDatebookScroller,AgendaDatebookTable,
										inApptCount, apptLineCount, Agenda.apptLineHeight);

	AgendaViewUpdateScroller (inFormP, AgendaToDoScroller, AgendaToDoTable,
										inToDoCount, todoLineCount, Agenda.todoLineHeight);
	
	Agenda.apptLineCount = apptLineCount;
	Agenda.todoLineCount = todoLineCount;
}


static void AgendaViewCountDisplayLines (
	FormType*						inFormP,
	UInt16							inApptCount,
	UInt16							inToDoCount,
	UInt16*							outApptLinesP,
	UInt16*							outToDoLinesP )
{
	UInt16							apptCount;
	UInt16							todoCount;
	RectangleType					agendaBounds;
	UInt16							agendaHeight;	
	UInt16							separatorHeight;	
	UInt16							apptSectionHeight;	
	UInt16							todoSectionHeight;	
	UInt16							apptDefaultCount;
	UInt16							todoDefaultCount;
	UInt16							apptDisplayCount;
	UInt16							todoDisplayCount;
	UInt16							apptDisplayHeight;
	UInt16							todoDisplayHeight;
	UInt16							remainingDisplayHeight;
	UInt16							remainingDisplayCount;
	
	// Compensate for empty sections, which will show something like,
	// "No Appointments Today"
	apptCount = max (minDatebookRows, inApptCount);
	todoCount = max (minToDoRows, inToDoCount);
	
	// Get the default sizes for each of the view sections
	AgendaViewGetScrollableRect (inFormP, &agendaBounds);
	agendaHeight = agendaBounds.extent.y;
	separatorHeight = AgendaViewGetSeparatorHeight (inFormP);
	apptSectionHeight = AgendaViewGetDatebookDefaultHeight (inFormP);
	todoSectionHeight = agendaHeight - separatorHeight - apptSectionHeight;
	
	// Convert those sizes into line counts
	apptDefaultCount = apptSectionHeight / Agenda.apptLineHeight;
	todoDefaultCount = todoSectionHeight / Agenda.todoLineHeight;
	
	// Adjust the layout for optimum viewing
	if (apptCount <= apptDefaultCount)
		{
		// All appointments fit within the standard area
		apptDisplayCount = apptCount;
		apptDisplayHeight = apptDisplayCount * Agenda.apptLineHeight;
		
		if (todoCount <= todoDefaultCount)
			{
			// All tasks fit within the standard area
			todoDisplayCount = todoCount;
			todoDisplayHeight = todoDisplayCount * Agenda.todoLineHeight;
			}
		else
			{
			// Extra tasks can grow into the unused appointment area
			remainingDisplayHeight = agendaHeight - separatorHeight - apptDisplayHeight;
			remainingDisplayCount = remainingDisplayHeight / Agenda.todoLineHeight;
			todoDisplayCount = min (todoCount, remainingDisplayCount);
			todoDisplayHeight = todoDisplayCount * Agenda.todoLineHeight;
			}
		}
	else if (todoCount <= todoDefaultCount)
		{
		// All tasks fit within the standard area
		todoDisplayCount = todoCount;
		todoDisplayHeight = todoDisplayCount * Agenda.todoLineHeight;
		
		// Extra appointments can grow into the unused task area
		remainingDisplayHeight = agendaHeight - separatorHeight - todoDisplayHeight;
		remainingDisplayCount = remainingDisplayHeight / Agenda.apptLineHeight;
		apptDisplayCount = min (apptCount, remainingDisplayCount);
		apptDisplayHeight = apptDisplayCount * Agenda.apptLineHeight;
		}
	else
		{
		// Both the appointment and task sections are full, so use the default layout
		apptDisplayCount = apptDefaultCount;
		apptDisplayHeight = apptDisplayCount * Agenda.apptLineHeight;
		todoDisplayCount = todoDefaultCount;
		todoDisplayHeight = todoDisplayCount * Agenda.todoLineHeight;
		}
	
	*outApptLinesP = apptDisplayCount;
	*outToDoLinesP = todoDisplayCount;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewSetTopAppointment
 *
 * DESCRIPTION: This routine determines the first appointment that should
 *              be visible on the current day.  For all dates other than
 *              today the fisrt time slot of the appointment list
 *              is the first visible appointment.  For today the time
 *              slot that stats before to the current time should be the top 
 *              visible time slot.
 *
 * PARAMETERS:  nothing.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void AgendaViewSetTopAppointment ()
{
	UInt16 i;
	TimeType time;
	DateTimeType dateTime;
	MemHandle apptsH;
	ApptInfoPtr apptsP;

	Agenda.apptTopVisibleIndex = 0;

	TimSecondsToDateTime (TimGetSeconds (), &dateTime);

	// If the current date is not today, then the first appointment 
	// is the first one visible.
	if ( (dateTime.year - firstYear != Date.year) ||
		  (dateTime.month != Date.month) ||
		  (dateTime.day != Date.day))
		{
		return;
		}

	// If the current date is today, then the top visible appointment is
	// the appointment with the greatest end time that is before the 
	// current time.
	time.hours = dateTime.hour;
	time.minutes = dateTime.minute;
	
	apptsH = AgendaGetAppointments ();
	if (apptsH)
		{
		apptsP = MemHandleLock (apptsH);
		for (i = 0; i < Agenda.apptCount; i++)
			{
			if (TimeToInt (apptsP[i].endTime) < TimeToInt (time))
				Agenda.apptTopVisibleIndex = i;
			}

		MemPtrUnlock (apptsP);
		}
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewSetTopTask
 *
 * DESCRIPTION: This routine determines the first task that should
 *              be visible on the current day.  For all dates other than
 *              today the fisrt time slot of the appointment list
 *              is the first visible appointment.  For today the time
 *              slot that stats before to the current time should be the top 
 *              visible time slot.
 *
 * PARAMETERS:  nothing.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void AgendaViewSetTopTask ()
{
	TopVisibleRecord = 0;
}


static void
AgendaViewLayoutTables (
	FormType*						inFormP,
	UInt16							inApptLineCount,
	UInt16							inToDoLineCount )
{
	RectangleType					scrollableRect;
	RectangleType					apptRect;
	RectangleType					separatorRect;
	RectangleType					todoRect;

	AgendaViewGetScrollableRect (inFormP, &scrollableRect);
	
	apptRect = scrollableRect;
	
	AgendaViewLayoutAppointments (inFormP, &apptRect, inApptLineCount);
	
	separatorRect.topLeft.x = scrollableRect.topLeft.x;
	separatorRect.topLeft.y = apptRect.topLeft.y + apptRect.extent.y;
	separatorRect.extent.x = scrollableRect.extent.x;
	separatorRect.extent.y = scrollableRect.extent.y - apptRect.extent.y;
	
	AgendaViewLayoutSeparator (inFormP, &separatorRect);
	
	todoRect.topLeft.x = scrollableRect.topLeft.x;
	todoRect.topLeft.y = separatorRect.topLeft.y + separatorRect.extent.y;
	todoRect.extent.x = scrollableRect.extent.x;
	todoRect.extent.y = scrollableRect.extent.y - apptRect.extent.y - separatorRect.extent.y;
	
	AgendaViewLayoutTasks (inFormP, &todoRect, inToDoLineCount);
}


static void
AgendaViewLayoutAppointments (
	FormType*						inFormP,
	RectangleType*					ioBoundsP,
	UInt16							inApptLineCount )
{
//#pragma unused (inFormP)
	TableType*						tableP;
	UInt16							row;
	UInt16							rowCount;
	RectangleType					bounds;

	tableP = GetObjectPtr (AgendaDatebookTable);
	rowCount = TblGetNumberOfRows (tableP);

	// Set the usability for each row in the table. If there are fewer appointments
	// than rows, the remaining rows will be unusable
	for (row = 0; row < rowCount; row++)
		{
		if ((row < inApptLineCount)
				&& (!TblRowUsable (tableP, row)
						|| (TblGetRowHeight (tableP, row) != Agenda.apptLineHeight)) )
			{
			TblMarkRowInvalid (tableP, row);
			}
			
		TblSetRowUsable (tableP, row, row < inApptLineCount);
		TblSetRowHeight (tableP, row, Agenda.apptLineHeight);
		}
	
	// Adjust the table size so that it doesn't overlap the To Do table
	TblGetBounds (tableP, &bounds);
	bounds.extent.y = inApptLineCount * Agenda.apptLineHeight;
	TblSetBounds (tableP, &bounds);
	ioBoundsP->extent.y = inApptLineCount * Agenda.apptLineHeight;
}


static void
AgendaViewLayoutSeparator (
	FormType*						inFormP,
	RectangleType*					ioBoundsP )
{
	RectangleType					dividerBounds;
	UInt16							top = ioBoundsP->topLeft.y;
	
	AgendaViewObjectSetTop (inFormP, AgendaDivider, top);
	AgendaViewObjectSetTop (inFormP, AgendaToDoCategoryList, top + adjustCategoryTop);
	AgendaViewObjectSetTop (inFormP, AgendaToDoCategoryTrigger, top + adjustCategoryTop);
	
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex (inFormP, AgendaDivider), &dividerBounds);
	ioBoundsP->topLeft.y = dividerBounds.topLeft.y;
	ioBoundsP->extent.y = dividerBounds.extent.y;
}


static UInt16
AgendaViewObjectSetTop (
	FormType*						inFormP,
	UInt16							inObjectID,
	UInt16							inTop )
{
	RectangleType					bounds;
	UInt16							index;

	index = FrmGetObjectIndex (inFormP, inObjectID);
	FrmGetObjectBounds (inFormP, index, &bounds);
	bounds.topLeft.y = inTop;
	FrmSetObjectBounds (inFormP, index, &bounds);
	
	return bounds.topLeft.y + bounds.extent.y;
}

static void
AgendaViewLayoutTasks (
	FormType*						inFormP,
	RectangleType*					ioBoundsP,
	UInt16							inToDoLineCount )
{
//#pragma unused (inFormP)
	TableType*						tableP;
	UInt16							row;
	UInt16							rowCount;
	RectangleType					bounds;

	tableP = GetObjectPtr (AgendaToDoTable);
	rowCount = TblGetNumberOfRows (tableP);
	TblGetBounds (tableP, &bounds);
	
	// Set the usability for each row in the table. If there are fewer tasks
	// than rows, the remaining rows will be unusable
	for (row = 0; row < rowCount; row++)
		{
		if ((row < inToDoLineCount)
				&& (!TblRowUsable (tableP, row)
						|| (ioBoundsP->topLeft.y != bounds.topLeft.y)
						|| (TblGetRowHeight (tableP, row) != Agenda.todoLineHeight)) )
			{
			TblMarkRowInvalid (tableP, row);
			}
			
		TblSetRowUsable (tableP, row, row < inToDoLineCount);
		TblSetRowHeight (tableP, row, Agenda.todoLineHeight);
		}
	
	// For drawing purposes, the table should extend to the bottom of the
	// displayable area, regardless of the number of displayable lines.
	TblSetBounds (tableP, ioBoundsP);
}


static void
AgendaViewUpdateScroller (
	FormType*						inFormP,
	UInt16							inObjectID,
	UInt16							inAssocObjectID,
	UInt16							inRecordCount,
	UInt16							inVisibleCount,
	UInt16							inLineHeight )
{
	RectangleType					assocBounds;
	UInt16							scrollIndex;
	RectangleType					scrollBounds;
	UInt16							scrollMax;
	Int16								currentValue;
	Int16								currentMin;
	Int16								currentMax;
	Int16								currentPageSize;
	
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex (inFormP, inAssocObjectID), &assocBounds);
	
	scrollMax = max (0, (Int32) inRecordCount - inVisibleCount);	// without the typecast, max ignores the 0!!!
	
	// Resize the scroll bar to be slightly inset from the provided bounds.
	// To avoid runtime warnings (debug only), make sure that it is also
	// at least 1 pixel tall.
	scrollIndex = FrmGetObjectIndex (inFormP, inObjectID);
	FrmGetObjectBounds (inFormP, scrollIndex, &scrollBounds);
	
	scrollBounds.topLeft.y = assocBounds.topLeft.y + scrollerInset;
	scrollBounds.extent.y = max (1, (Int32) inVisibleCount * inLineHeight - (2 * scrollerInset));
	FrmSetObjectBounds (inFormP, scrollIndex, &scrollBounds);
	
	SclGetScrollBar (GetObjectPtr (inObjectID), &currentValue, &currentMin,
							&currentMax, &currentPageSize);
	currentValue = min (currentValue, scrollMax);
	SclSetScrollBar (GetObjectPtr (inObjectID), currentValue, 0, scrollMax, inVisibleCount);
}


static void
AgendaViewEraseObject (
	FormType*						inFormP,
	UInt16							inObjectID )
{
	RectangleType					bounds;
	
	if (FrmVisible(inFormP))
		{
		FrmGetObjectBounds (inFormP, FrmGetObjectIndex (inFormP, inObjectID), &bounds);
		WinEraseRectangle (&bounds, 0);
		}
}


static void
AgendaViewSetScrollThumb (
	FormType*						inFormP,
	UInt16							inObjectID,
	UInt16							inValue )
{
//#pragma unused (inFormP)
	ScrollBarType*					scrollBarP;
	Int16								scrollValue;
	Int16								scrollMin;
	Int16								scrollMax;
	Int16								scrollPage;

	scrollBarP = GetObjectPtr (inObjectID);
	SclGetScrollBar (scrollBarP, &scrollValue, &scrollMin, &scrollMax, &scrollPage);
	SclSetScrollBar (scrollBarP, inValue, scrollMin, scrollMax, scrollPage);
}


static void
AgendaViewRefreshLayoutTasks (
	FormType*						inFormP )
{
	UInt16							oldApptLineCount;

	// This routine is called when the To Do section of the agenda has been altered.
	// Even though only the To Do section has changed, the heuristic divider may
	// need to be repositioned if the Datebook section overflows its default size.

	oldApptLineCount = Agenda.apptLineCount;

	AgendaViewLayout (inFormP, Agenda.apptCount, Agenda.todoCount);
	
	if (Agenda.apptLineCount != oldApptLineCount)
		{
		AgendaViewFillAppointments (inFormP);
		AgendaViewSetScrollThumb (inFormP, AgendaDatebookScroller, Agenda.apptTopVisibleIndex);
		}

	AgendaViewFillTasks (inFormP);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaApptDrawTime
 *
 * DESCRIPTION: Draw the starting time of an appointment.  This
 *              routine is called by the table object as a callback 
 *              routine.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaApptDrawTime (
	void*								inTableP,
	Int16								inRow,
	Int16								inColumn,
	RectangleType*					inBoundsP )
{
//#pragma unused (inColumn)
	UInt16							apptIndex;
	ApptInfoPtr						appts;
	TimeType							startTime;

	// Get the appointment that corresponds to the table item.
	// The index of the appointment in the appointment list is stored
	// as the row id.
	apptIndex = TblGetRowID (inTableP, inRow);

	// When there are no appointments, a special index is stored. If this is the case, don't
	// try to draw the time; the description column will show "No Appointments"
	if (apptIndex != noAppointments)
		{
		// Get the record index of the next appointment
		appts = MemHandleLock (AgendaGetAppointments ());
		startTime = appts[apptIndex].startTime;
		MemPtrUnlock (appts);
		
		#if WRISTPDA
		inBoundsP->extent.x -= 4;
		#endif
		DrawTime (startTime, TimeFormat, apptTimeFont, rightAlign, inBoundsP);
		#if WRISTPDA
		inBoundsP->extent.x += 4;
		#endif
		}
}


/***********************************************************************
 *
 * FUNCTION:    AgendaApptDrawDesc
 *
 * DESCRIPTION: Draw the description of an appointment.  This
 *              routine is called by the table object as a callback 
 *              routine.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	8/30/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaApptDrawDesc (
	void*								inTableP,
	Int16								inRow,
	Int16								inColumn,
	RectangleType*					inBoundsP )
{
//#pragma unused (inColumn)
	Err								err = errNone;
	UInt16							apptIndex;
	ApptInfoPtr						apptsP;
	UInt16							recordNum;
	MemHandle						recordH = NULL;
	ApptDBRecordType				apptRec;
	MemHandle						textH = NULL;
	Char*								textP = NULL;
	FontID							curFont;
	Char*								tempP;
	UInt16							tempCount = 0;

	// Get the appointment that corresponds to the table item.
	// The index of the appointment in the appointment list is stored
	// as the row id.
	apptIndex = TblGetRowID (inTableP, inRow);

	// When there are no appointments, a special index is stored. If this is the case, don't
	// try to draw the time; the description column will show "No Appointments"
	if (apptIndex != noAppointments)
		{
		// Get the record index of the next appointment
		apptsP = MemHandleLock (AgendaGetAppointments ());
		recordNum = apptsP[apptIndex].recordNum;
		MemPtrUnlock (apptsP);

		// Get the offset and length of the description field 
		err = ApptGetRecord (Agenda.apptDB, recordNum, &apptRec, &recordH);
		
		if ( err == errNone )
			{
			textP = apptRec.description;
			}
		}
	else
		{
		textH = DmGetResource (strRsc, agendaNoAppointmentsStrID);
		textP = MemHandleLock (textH);
		}
	
	if (textP)
		{
		curFont = FntSetFont (Agenda.apptFont);
		
		tempP = textP;
		while (*tempP && *tempP != linefeedChr)
			{
			++tempCount;
			++tempP;
			}
		WinDrawTruncChars (textP, tempCount, inBoundsP->topLeft.x, inBoundsP->topLeft.y,
									inBoundsP->extent.x);
		FntSetFont (curFont);
		}
	
	if (recordH)
		{
		MemHandleUnlock (recordH);
		}
	
	if (textH)
		{
		MemHandleUnlock (textH);
		DmReleaseResource (textH);
		}
}


static Boolean
AgendaDividerDraw (
	//FormGadgetType*				inGadgetP,
	FormGadgetTypeInCallback*				inGadgetP,
	UInt16							inCommand,
	void*								inParamP )
{
	EventType * eventP;
	EventType newEvent;
	ControlType * categoryTriggerP;
	
	switch (inCommand)
		{
		case formGadgetDrawCmd:
			WinDrawLine (	inGadgetP->rect.topLeft.x,
								inGadgetP->rect.topLeft.y + adjustSeparatorTop,
								inGadgetP->rect.topLeft.x + inGadgetP->rect.extent.x - 1,
								inGadgetP->rect.topLeft.y + adjustSeparatorTop );
			return true;
		
		case formGadgetEraseCmd:
			WinEraseLine (	inGadgetP->rect.topLeft.x,
								inGadgetP->rect.topLeft.y + adjustSeparatorTop,
								inGadgetP->rect.topLeft.x + inGadgetP->rect.extent.x - 1,
								inGadgetP->rect.topLeft.y + adjustSeparatorTop );
			return true;
		
		case formGadgetHandleEventCmd:
			// Since the gadget can overlap the popup trigger for the to do category,
			// we have to take the gadget enter event generated for a pen down event
			// and convert it back to a pen down event to pass on to the control. If
			// the point is in the bounds of the popup trigger, it will then convert
			// it into a control enter event.
			eventP = (EventType *)inParamP;
			categoryTriggerP = GetObjectPtr(AgendaToDoCategoryTrigger);
			if (eventP->eType == frmGadgetEnterEvent)
			{
				newEvent = *eventP;
				newEvent.eType = penDownEvent;
				return CtlHandleEvent(categoryTriggerP, &newEvent);
			}
		}
	
	return false;
}


#if 0
/***********************************************************************
 *
 * FUNCTION:    AgendaTitleDraw
 *
 * DESCRIPTION: Draw the current date and time
 *
 * PARAMETERS:  frm - pointer to the agenda view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaTitleDraw (
	void*								inTableP,
	Int16								inRow,
	Int16								inColumn,
	RectangleType*					inBoundsP )
{
	DateTimeType					now;
	char								timeStr[timeStringLength];
	char								dateStr[longDateStrLength];
	FontID							origFont;
	UInt16							x;
	UInt16							len;

	TimSecondsToDateTime (TimGetSeconds (), &now);
	
	WinInvertRectangle (inBoundsP, 0);
	origFont = FntSetFont (TblGetItemFont (inTableP, inRow, inColumn));
	
	switch (inColumn)
		{
		case agendaTitleDateColumn:
			DateToAscii (now.month, now.day, now.year, Agenda.longDateFormat, dateStr);

			x = inBoundsP->topLeft.x + agendaTitleInset;
			WinInvertChars (dateStr, StrLen (dateStr), x, inBoundsP->topLeft.y);
			break;
		
		case agendaTitleTimeColumn:
			TimeToAscii (now.hour, now.minute, Agenda.timeFormat, timeStr);
			
			len = StrLen (timeStr);
			x = inBoundsP->topLeft.x + ( inBoundsP->extent.x - FntCharsWidth (timeStr, len) );
			x -= agendaTitleInset;
			WinInvertChars (timeStr,  len, x, inBoundsP->topLeft.y);
			break;
		}
	
	FntSetFont (origFont);
}
#endif


/***********************************************************************
 *
 * FUNCTION:    AgendaViewDrawDate
 *
 * DESCRIPTION: Draw the current date
 *
 * PARAMETERS:  inFormP - pointer to the agenda view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewDrawDate (
	FormType*						inFormP )
{
//#pragma unused (inFormP)

	// DOLATER rbb 7/8/99 - Add constants, in DateAgenda.h or DateTime.h, for
	// string length and trim offsets.
	static Char						titleP [dowLongDateStrLength];
	ControlType*					controlP;
	//Int8							trimOffset = 0;

	AgendaDateToDOWDM (Date, titleP);
	
	
	controlP = (ControlType*) GetObjectPtr (AgendaCurrentDayButton);
	CtlSetLabel (controlP, titleP);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewDrawTime
 *
 * DESCRIPTION: Draw the current time
 *
 * PARAMETERS:  inFormP - pointer to the agenda view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewDrawTime (
	FormType*						inFormP )
{
//#pragma unused (inFormP)
	static Char						timeStr [timeStringLength];
	DateTimeType 					dateTime;
	
	TimSecondsToDateTime (Agenda.timeSeconds, &dateTime);
	TimeToAscii (dateTime.hour, dateTime.minute, Agenda.timeFormat, timeStr);
	
	FrmCopyTitle (inFormP, timeStr);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewDrawSeparator
 *
 * DESCRIPTION: Draw the separator line between the calendar and task sections
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/4/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewDrawSeparator (
	FormType*						inFormP )
{
//#pragma unused (inFormP)
	FormGadgetType*					separatorP;
	RectangleType						bounds;
	FormGadgetType*					dividingLineP;
	ControlType*						categoryTriggerP;
	IndexedColorType					oldForeColor;
	
	separatorP = GetObjectPtr (AgendaSeparatorDefaultArea);
	dividingLineP = GetObjectPtr (AgendaDivider);
	
	bounds = separatorP->rect;
	bounds.topLeft.y = dividingLineP->rect.topLeft.y;
	bounds.extent.y = dividingLineP->rect.extent.y;
	WinEraseRectangle (&bounds, 0);
	
	oldForeColor = WinSetForeColor(UIColorGetTableEntryIndex(UIFormFrame));
	(dividingLineP->handler) ((FormGadgetTypeInCallback *)dividingLineP, formGadgetDrawCmd, 0);
	WinSetForeColor(oldForeColor);

	categoryTriggerP = GetObjectPtr (AgendaToDoCategoryTrigger);
	CtlDrawControl (categoryTriggerP);
	
}


/***********************************************************************
 *
 * FUNCTION:    AgendaDateToDOWDM
 *
 * DESCRIPTION: Similar to DateToAscii, but accepts DateType and masks
 *					 out century 
 *
 * PARAMETERS:  
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	06/28/99	Initial Revision
 *			gap	10/10/00 Added a format string list so that the format string
 *								in the agenda view title will respect user's format
 *								setting specified in formats panel.  
 *
 ***********************************************************************/
static void
AgendaDateToDOWDM (
	DateType			inDate,
	Char*				outAscii )
{
	UInt16			dateFormatIndex;
	Char 				templateBuffer[maxDateTemplateLen + 1];


	// Convert the current short day of week format selector into an index into the 
	// agenda title date formats list.
	if (Agenda.dateFormat > dfYMDWithDashes)
		{
		// The dfMDYWithDashes uses the dfMDYLongWithComma just as the dfMDYWithSlashes
		// does.
		//
		if (ShortDateFormat != dfMDYWithDashes)
			{
			// DOLATER kwk - gross! If we add a new short date format,
			// this will trigger a fatal alert, but only if testing
			// actually runs this code with that format selected.
			ErrNonFatalDisplay("Unknown short date format");
			}
		
		// Default to the dfMDYWithSlashes format.
		//
		dateFormatIndex = (UInt16)dfMDYWithSlashes;
		}
	else
		dateFormatIndex = (UInt16)Agenda.dateFormat;
	

	SysStringByIndex(agendaTitleDateFormatsListID, (UInt16)dateFormatIndex, templateBuffer,
							sizeof(templateBuffer) - 1);

	
	DateTemplateToAscii(templateBuffer, inDate.month, inDate.day, inDate.year + firstYear,
								outAscii, dowLongDateStrLength);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaDayViewScroll
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
 *			art	2/21/95	Initial Revision
 *			rbb	4/14/99	Uses new ListViewDrawTable
 *
 ***********************************************************************/
static void
AgendaDayViewScroll (
	Int16								delta)
{
	UInt16 							apptIndex;
	UInt16							apptCount;
	UInt16							rowCount;
	UInt16							newIndex;
	TableType* 						tableP;

	if (delta == 0)
		{
		return;
		}
	
	tableP = GetObjectPtr (AgendaDatebookTable);
	TblReleaseFocus (tableP);
	
	apptIndex = Agenda.apptTopVisibleIndex;
	apptCount = AgendaGetAppointmentCount();
	rowCount = TblGetLastUsableRow (tableP) + 1;
	
	ErrFatalDisplayIf (apptIndex == noAppointments, "AgendaDayViewScroll called with no appointments");
	
	newIndex = max (0, (Int32) apptIndex + delta);
	newIndex = min (newIndex, apptCount - rowCount);

	Agenda.apptTopVisibleIndex = newIndex;
	
	AgendaViewFillAppointments (FrmGetActiveForm());
	TblRedrawTable (tableP);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewGoToDate
 *
 * DESCRIPTION: This routine displays the date picker so that the 
 *              user can select a date to navigate to.  If the date
 *              picker is confirmed, the date selected is displayed.
 *
 *              This routine is called when a "go to" button is pressed.
 *              
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/29/95	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewGoToDate ( void )
{
	Char* title;
	MemHandle titleH;
	Int16 month, day, year;

	// Get the title for the date picker dialog box.
	titleH = DmGetResource (strRsc, goToDateTitleStrID);
	title = MemHandleLock (titleH);

	day = Date.day;
	month = Date.month;
	year = Date.year + firstYear;

	// Display the date picker.
	if (SelectDay (selectDayByDay, &month, &day, &year, title))
		{
		Date.day = day;
		Date.month = month;
		Date.year = year - firstYear;

		AgendaViewChangeDate (FrmGetActiveForm(), Date);
		}
		
	MemHandleUnlock (titleH);
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AgendaViewInitToDo
 *
 * DESCRIPTION: Initialize the To Do portion of the Agenda view.
 *
 * PARAMETERS:  frm - pointer to the day view form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
void
AgendaViewInitToDo (
	FormPtr							frm )
{
	// Get the To Do prefs
	UInt16 row;
	UInt16 rowsInTable;
	UInt16 width;
	FontID fontID;
	TablePtr table;
	RectangleType r;
	Boolean showCategories = ShowCategories && (CurrentCategory == dmAllCategories);
	ControlPtr ctl;
	

	table = FrmGetObjectPtr (frm, FrmGetObjectIndex (frm, AgendaToDoTable));

	rowsInTable = TblGetNumberOfRows (table);
	for (row = 0; row < rowsInTable; row++)
		{		
		TblSetItemStyle (table, row, completedColumn, checkboxTableItem);
		TblSetItemStyle (table, row, priorityColumn, numericTableItem);
		TblSetItemStyle (table, row, descColumn, customTableItem);
		TblSetItemStyle (table, row, dueDateColumn, customTableItem);
		TblSetItemStyle (table, row, categoryColumn, customTableItem);


		// Set the font used to draw the text of the row.
		//if (ListFont == FossilStdFont)
		if (ListFont == stdFont)
			//fontID = FossilBoldFont;
			fontID = boldFont;
		else
			fontID = ListFont;
		#if WRISTPDA
		// The table checkbox is not drawn with the correct size by the system extension
		// when running with large fonts, so even though we should set the correct font here,
		// the selection highlight is drawn too large for the checkbox if we do...
//		TblSetItemFont (table, row, completedColumn, FossilLargeFontID( WRISTPDA, ListFont ) );
		#endif
		TblSetItemFont (table, row, priorityColumn, fontID);
		TblSetItemFont (table, row, descColumn, ListFont);
		TblSetItemFont (table, row, dueDateColumn, ListFont);
		TblSetItemFont (table, row, categoryColumn, ListFont);

//		TblSetRowUsable (table, row, false);
		}


	TblSetColumnUsable (table, completedColumn, true);
	TblSetColumnUsable (table, priorityColumn, ShowPriorities);
	TblSetColumnUsable (table, descColumn, true);
	TblSetColumnUsable (table, dueDateColumn, ShowDueDates);
	TblSetColumnUsable (table, categoryColumn, showCategories);
	
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
//	TblSetLoadDataProcedure (table, descColumn, (TableLoadDataFuncPtr)ListViewGetDescription);
//	TblSetSaveDataProcedure (table, descColumn, (TableSaveDataFuncPtr)ListViewSaveDescription);

	// Set the callback routines that draws the various fields
	TblSetCustomDrawProcedure (table, dueDateColumn, ListViewDrawDueDate);
	TblSetCustomDrawProcedure (table, categoryColumn, ListViewDrawCategory);
	TblSetCustomDrawProcedure (table, descColumn, ListViewDrawDesc);


	// Set the label of the category trigger.
	ctl = GetObjectPtr (AgendaToDoCategoryTrigger);
	CategoryGetName (Agenda.todoDB, CurrentCategory, CategoryName);
	CategorySetTriggerLabel (ctl, CategoryName);
}


#if 0
/***********************************************************************
 *
 * FUNCTION:    AgendaViewFillTitle
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewFillTitle (
	FormPtr							frm )
{
//#pragma unused (frm)

}
#endif


#if 0
/***********************************************************************
 *
 * FUNCTION:    AgendaViewFillAppointmentsTitle
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewFillAppointmentsTitle (
	FormPtr							frm )
{
//#pragma unused (frm)

}
#endif


/***********************************************************************
 *
 * FUNCTION:    AgendaViewFillAppointments
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewFillAppointments (
	FormType*						inFormP )
{
//#pragma unused (inFormP)

	TableType*						tableP;
	RectangleType					tableBounds;
	UInt16							row;
	UInt16							lastRow;
	FontID							oldFont;
	UInt16							apptIndex;
	UInt16							apptCount;
	UInt16							rowHeight = 0;
	UInt16							displayableHeight = 0;
	//UInt16							virtualHeight = 0;
	UInt16 							attr;
	Boolean 							masked;
	UInt16							maxIndex;

	// Get the height of the table and the width of the description column.
	tableP = GetObjectPtr (AgendaDatebookTable);
	TblGetBounds (tableP, &tableBounds);
	
	row = 0;
	lastRow = TblGetLastUsableRow (tableP);
	
	oldFont = FntSetFont (Agenda.apptFont);
	rowHeight = FntLineHeight ();
	FntSetFont (oldFont);
	
	apptCount = AgendaGetAppointmentCount();
	maxIndex = max (0, (Int16) apptCount - (Int16) lastRow - 1);
	apptIndex = min (Agenda.apptTopVisibleIndex, maxIndex);
	Agenda.apptTopVisibleIndex = apptIndex;
	
	// Associate each row with the appropriate appointment
	while ( (apptIndex < apptCount) && (row <= lastRow) )
		{
		// Also ensure that we're within our drawing area
		if ( displayableHeight + rowHeight <= tableBounds.extent.y )
			{
			MemHandle				apptsH;
			ApptInfoType*			apptsP;
			
			TblSetRowID (tableP, row, apptIndex);
			TblMarkRowInvalid (tableP, row);
			
			//Mask if appropriate
			apptsH = AgendaGetAppointments ();
			if (apptsH)
				{
				apptsP = MemHandleLock (apptsH);
				DmRecordInfo (ApptDB, apptsP[apptIndex].recordNum, &attr, NULL, NULL);
			   	masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));	
				TblSetRowMasked(tableP,row,masked);
				
				MemPtrUnlock (apptsP);
				}
			
			apptIndex++;
			row++;
			displayableHeight += rowHeight;
			}
		else
			{
			break;
			}
		}
	
	if (apptCount == 0)
		{
		// Set the ID to a special value to trigger the display of the "no appointments" text
		TblSetRowID (tableP, 0, 0xffff);
		TblSetRowMasked (tableP, row, false);
		TblMarkRowInvalid (tableP, row);
		}
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AgendaLoadAppointments
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static MemHandle
AgendaLoadAppointments (
	DateType							inDate )
{
	MemHandle						appointmentsH;
	UInt16							count;
	
	// Load the day's appointments from the database
	ApptGetAppointments (Agenda.apptDB, inDate, 1, &appointmentsH, &count);
	
	// Replace the current appointments with our new data (frees any old data)
	AgendaSetAppointments (appointmentsH, count);
	
	return appointmentsH;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaFreeAppointments
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaFreeAppointments ( )
{
	if ( Agenda.apptsH != NULL )
		{
		MemHandleFree (Agenda.apptsH);
		}
	
	Agenda.apptsH = NULL;
	Agenda.apptCount = 0;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaGetAppointments
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static MemHandle
AgendaGetAppointments ( )
{
	return Agenda.apptsH;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaSetAppointments
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaSetAppointments (
	const MemHandle				inAppointmentsH,
	UInt16							inAppointmentCount )
{
	AgendaFreeAppointments ();
	
	Agenda.apptsH = inAppointmentsH;
	Agenda.apptCount = inAppointmentCount;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaGetAppointmentCount
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static UInt16
AgendaGetAppointmentCount ( )
{
	return Agenda.apptCount;
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AgendaViewGetScrollableRect
 *
 * DESCRIPTION: Returns the area into which the agenda can be drawn.
 *					 This is the whole screen, except for the title and
 *					 control areas.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    outBoundsP	- Screen area for agenda contents
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/14/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewGetScrollableRect (
	FormType*						inFormP,
	RectangleType*					outBoundsP )
{
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaScrollableArea), outBoundsP);
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewGetSeparatorHeight
 *
 * DESCRIPTION: Returns the height of the area between the datebook and
 *					 to do sections of the agenda view, into which the category
 *					 popup is drawn
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    outBoundsP	- Screen area for agenda contents
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/14/99	Initial Revision
 *
 ***********************************************************************/
static UInt16
AgendaViewGetSeparatorHeight (
	FormType*						inFormP )
{
	RectangleType					bounds;
	
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaSeparatorDefaultArea), &bounds);
	
	return bounds.extent.y;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewGetDatebookDefaultHeight
 *
 * DESCRIPTION: Returns the height of the area between the datebook and
 *					 to do sections of the agenda view, into which the category
 *					 popup is drawn
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    outBoundsP	- Screen area for agenda contents
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	6/14/99	Initial Revision
 *
 ***********************************************************************/
static UInt16
AgendaViewGetDatebookDefaultHeight (
	FormType*						inFormP )
{
	RectangleType					agendaBounds;
	RectangleType					separatorBounds;
	
	AgendaViewGetScrollableRect (inFormP, &agendaBounds);
	FrmGetObjectBounds (inFormP, FrmGetObjectIndex(inFormP, AgendaSeparatorDefaultArea),
								&separatorBounds);
	
	return separatorBounds.topLeft.y - agendaBounds.topLeft.y;
}


#if WRISTPDA

/***********************************************************************
 *
 * FUNCTION:    PrvListNumberOfRows
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
UInt16 PrvListNumberOfRows (TablePtr table);
UInt16 PrvListNumberOfRows (TablePtr table)
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
 * FUNCTION:    ToolsSeekRecord
 *
 * DESCRIPTION: Given the index of a to do record, this routine scans
 *              forewards or backwards for displayable to do records.
 *
 * PARAMETERS:  indexP  - pointer to the index of a record to start from;
 *                        the index of the record sought is returned in
 *                        this parameter.
 *
 *              offset  - number of records to skip:
 *                           0 - mean seek from the current record to the
 *                             next display record, if the current record is
 *                             a display record, its index is retuned.
 *                         1 - mean seek foreward, skipping one displayable
 *                             record
 *                        -1 - means seek backwards, skipping one
 *                             displayable record
 *
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
Boolean ToolsSeekRecord (DmOpenRef dbRef, UInt16 * indexP, Int16 offset, Int16 direction);
Boolean ToolsSeekRecord (DmOpenRef dbRef, UInt16 * indexP, Int16 offset, Int16 direction)
{

	if ( dbRef == Agenda.todoDB ) { 

		// For ToDo records handle the ShowCompletedItems and ShowOnlyDueItems states.

		Int32 dateL;
		Int32 todayL;
		MemHandle recordH;
		DateTimeType today;
		ToDoDBRecordPtr toDoRec;

		ErrFatalDisplayIf ( (offset < -1 || offset > 1) , "Invalid offset");

		while (true)
			{
			DmSeekRecordInCategory (Agenda.todoDB, indexP, offset, direction, CurrentCategory);
			if (DmGetLastErr()) return (false);
	
			if ( ShowCompletedItems && (! ShowOnlyDueItems))
				return (true);
		
			recordH = DmQueryRecord (Agenda.todoDB, *indexP);
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
	} else {
		// For DateBook records just seek in the current category.
		DmSeekRecordInCategory (dbRef, indexP, offset, direction, dmAllCategories);
		if (DmGetLastErr()) return (false);
	}

	return (true);

}


/***********************************************************************
 *
 * FUNCTION:    LastVisibleRecord
 *
 * DESCRIPTION: This routine returns the index of the last visible record.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Index of last visible record.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dmc		2/04/03		Initial Revision
 *
 ***********************************************************************/
static UInt16 LastVisibleRecord( DmOpenRef dbRef )
{
	UInt16 i, Last, NumRows;
	Last  = TopVisibleRecord;
	NumRows = TblGetLastUsableRow( GetObjectPtr( AgendaToDoTable ) );
	if ( NumRows == 0 )
	  return Last;
	for ( i = 0; i <= NumRows - 1; i++ ) {
		if ( ! ToolsSeekRecord( dbRef, & Last, 1, +1 ) )
		 	break;
	}
	return Last;
}


/***********************************************************************
 *
 * FUNCTION:    ToDoVisibleRecord
 *
 * DESCRIPTION: This routine returns status indicating if there is at
 *				least one visible ToDo record.
 *
 * PARAMETERS:  None
 *
 * RETURNED:    Boolean indicating if there is a visible ToDo record.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			dmc		3/29/03		Initial Revision
 *
 ***********************************************************************/
static Boolean ToDoVisibleRecord( void )
{

	Boolean Result1, Result2;
	UInt16 RecordsInCategory, Record;

	// If there are no records then there are no visible records.
	RecordsInCategory = DmNumRecordsInCategory( Agenda.todoDB, CurrentCategory );
	if ( RecordsInCategory == 0 )
		return false;

	// Filter the records based on ShowCompletedItems and ShowOnlyDueItems.
	Record = 0;
	Result1 = ToolsSeekRecord( Agenda.todoDB, & Record, 0, +1 );
	Result2 = ToolsSeekRecord( Agenda.todoDB, & Record, 0, -1 );
	// If we can't seek then there are no visible records.
	if ( ( Result1 == false ) && ( Result2 == false ) )
		return false;

	// There is at least one visible record.
	return true;

}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewMoveApptSelection
 *
 * DESCRIPTION: This routine moves the Datebook navigation selection
 *              highlight up or down.
 *
 * PARAMETERS:  Direction: -1 => Move up
 *                          0 => Don't move, highlight current selection
 *                         +1 => Move down
 *
 * RETURNED:    Nothing
 *
 * HISTORY:
 *
 *		02/13/03	dmc		Initial version.
 *
 ***********************************************************************/
static void AgendaViewMoveApptSelection ( Int16 Direction )
{
	TablePtr Table;
	Int16  OldSelectedRecord, SelectedRow, VisibleTopRecordIndex, VisibleBottomRecordIndex;
	UInt16 TableLastRecordIndex;
	// If there are no records then we can't move the selection.
	if ( Agenda.apptCount == 0 )
		return;
	OldSelectedRecord = SelectedRecord;
	if ( Direction != 0 ) {
		// Get the info we need to decide if we can move the selection.
		Table = GetObjectPtr( AgendaDatebookTable );
		TableLastRecordIndex = Agenda.apptCount - 1;
		VisibleTopRecordIndex = TopVisibleRecord;
		VisibleBottomRecordIndex = Agenda.apptTopVisibleIndex + Agenda.apptLineCount - 1;
		if ( VisibleBottomRecordIndex > TableLastRecordIndex )
			VisibleBottomRecordIndex = TableLastRecordIndex;
		if ( Direction == +1 ) {
			// Move the selection highlight down one record, scroll if necessary.
			if ( SelectedRecord < TableLastRecordIndex ) {
				if ( SelectedRecord >= VisibleBottomRecordIndex ) {
					if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) )
						TblUnhighlightSelection( Table );
					AgendaDayViewScroll( +1 );
				}
				ToolsSeekRecord( Agenda.apptDB, & SelectedRecord, 1, +1 );
				TopVisibleRecord = Agenda.apptTopVisibleIndex; // FIXIT: Seek?
			} else {
				// Scrolled off end of Appt list, attempt to switch focus to To Do list.
				if ( ToDoVisibleRecord() == true ) {
					TblUnhighlightSelection( Table );
					SelectionFocus = AgendaFocusToDoList;
					Table = GetObjectPtr( AgendaToDoTable );
					TopVisibleRecord = SelectedRecord = TblGetRowID( Table, 0 );
				}
			}
		}
	 	else if ( Direction == -1 ) {
			// Move the selection highlight up one record, scroll if necessary.
			if ( SelectedRecord > 0 ) {
				if ( SelectedRecord <= VisibleTopRecordIndex ) {
					if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) )
						TblUnhighlightSelection( Table );
					AgendaDayViewScroll( -1 );
				}
				ToolsSeekRecord( Agenda.apptDB, & SelectedRecord, 1, -1 );
				TopVisibleRecord = Agenda.apptTopVisibleIndex; // FIXIT: Seek?
			}
		}
	}
	// Highlight the current selection.
	if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) ) {
		TblUnhighlightSelection( Table );
		TblSelectItem( Table, SelectedRow, 0 );
	}
	// Update the scrollbar state.
	if ( SelectionFocus == AgendaFocusApptList )
		AgendaViewSetScrollThumb( FrmGetActiveForm(), AgendaDatebookScroller, TopVisibleRecord );
	else if ( SelectionFocus == AgendaFocusToDoList )
		AgendaViewSetScrollThumb( FrmGetActiveForm(), AgendaToDoScroller, TopVisibleRecord );
}

/***********************************************************************
 *
 * FUNCTION:    AgendaViewMoveToDoSelection
 *
 * DESCRIPTION: This routine moves the To Do  navigation selection
 *              highlight up or down.
 *
 * PARAMETERS:  Direction: -1 => Move up
 *                          0 => Don't move, highlight current selection
 *                         +1 => Move down
 *
 * RETURNED:    Nothing
 *
 * HISTORY:
 *
 *		02/13/03	dmc		Initial version.
 *
 ***********************************************************************/
static void AgendaViewMoveToDoSelection ( Int16 Direction )
{
	TablePtr Table;
	Int16  SelectedRow, VisibleTopRecordIndex, VisibleBottomRecordIndex;
	UInt16 RecordsInCategory, TableLastRecordIndex;
	// If there are no records then we can't move the selection.
	RecordsInCategory = DmNumRecordsInCategory( Agenda.todoDB, CurrentCategory );
	if ( RecordsInCategory == 0 )
		return;
	// If there are no visible records then attempt to switch focus to Appt list.
	if ( ToDoVisibleRecord() == false ) {
		if ( Agenda.apptCount > 0 ) {
			SelectionFocus = AgendaFocusApptList;
			Table = GetObjectPtr( AgendaDatebookTable );
			TopVisibleRecord = Agenda.apptTopVisibleIndex;
			SelectedRecord = Agenda.apptTopVisibleIndex + Agenda.apptLineCount - 1;
		}
		goto Exit;
	}
	Table = GetObjectPtr( AgendaToDoTable );
	if ( Direction != 0 ) {
		// Get the info we need to decide if we can move the selection.
		TableLastRecordIndex = dmMaxRecordIndex;
		ToolsSeekRecord( Agenda.todoDB, & TableLastRecordIndex, 0, -1 );
		VisibleTopRecordIndex = TopVisibleRecord;
		VisibleBottomRecordIndex = LastVisibleRecord( Agenda.todoDB );
		if ( VisibleBottomRecordIndex > TableLastRecordIndex )
			VisibleBottomRecordIndex = TableLastRecordIndex;
		if ( Direction == +1 ) {
			// Move the selection highlight down one record, scroll if necessary.
			if ( SelectedRecord < TableLastRecordIndex ) {
				if ( SelectedRecord >= VisibleBottomRecordIndex ) {
					if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) )
						TblUnhighlightSelection( Table );
					ListViewScroll( +1 );
				}
				ToolsSeekRecord( Agenda.todoDB, & SelectedRecord, 1, +1 );
			}
		}
	 	else if ( Direction == -1 ) {
			// Move the selection highlight up one record, scroll if necessary.
			if ( SelectedRecord > 0 ) {
				if ( SelectedRecord <= VisibleTopRecordIndex ) {
					if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) )
						TblUnhighlightSelection( Table );
					ListViewScroll( -1 );
				}
				ToolsSeekRecord( Agenda.todoDB, & SelectedRecord, 1, -1 );
			} else {
				// Scrolled off end of Appt list, attempt to switch focus to Appt list.
				if ( Agenda.apptCount > 0 ) {
					TblUnhighlightSelection( Table );
					SelectionFocus = AgendaFocusApptList;
					Table = GetObjectPtr( AgendaDatebookTable );
					TopVisibleRecord = Agenda.apptTopVisibleIndex;  // FIXIT: Seek?
					SelectedRecord = Agenda.apptTopVisibleIndex + Agenda.apptLineCount - 1;
				}
			}
		}
	} else {
		// Make sure the current selection is still valid, move if necessary.
		ToolsSeekRecord( Agenda.todoDB, & SelectedRecord, 0, +1 );
		ToolsSeekRecord( Agenda.todoDB, & SelectedRecord, 0, -1 );
	}
Exit:
	// Highlight the current selection.
	if ( TblFindRowID( Table, SelectedRecord, & SelectedRow ) )
		TblSelectItem( Table, SelectedRow, 0 );
	// Update the scrollbar state.
	if ( SelectionFocus == AgendaFocusApptList )
		AgendaViewSetScrollThumb( FrmGetActiveForm(), AgendaDatebookScroller, TopVisibleRecord );
	else if ( SelectionFocus == AgendaFocusToDoList )
		AgendaViewSetScrollThumb( FrmGetActiveForm(), AgendaToDoScroller, TopVisibleRecord );
}

#endif

/***********************************************************************
 *
 * FUNCTION:    AgendaViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the Agenda View
 *              of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			rbb	6/2/99	Added button for Agenda view
 *			rbb	11/15/99 Refresh clock, as needed, on nil events
 *			rbb	11/15/99	Enhanced usage of the Datebook hard key
 *
 ***********************************************************************/
Boolean
AgendaViewHandleEvent (
	EventType*							event)
{
	FormPtr frm;
	DateType date;
	Boolean handled = false;
//	MonthPtr monthP;

	#if WRISTPDA

	EventType newEvent;

	extern UInt32  DebounceEnterKeyEndTime;

	extern UInt16  NewSndSysAmp, OldSndSysAmp;
	extern UInt16  NewSndDefAmp, OldSndDefAmp;

	extern UInt16  NewKeyInitDelay, OldKeyInitDelay;
	extern UInt16  NewKeyPeriod, OldKeyPeriod;
	extern UInt16  NewKeyDoubleTapDelay,  OldKeyDoubleTapDelay;
	extern Boolean NewKeyQueueAhead, OldKeyQueueAhead;

	#endif

	if (event->eType == keyDownEvent)
		{

		#if WRISTPDA
			frm = FrmGetActiveForm();

			// On other than Enter key exit the debounce state,
			if ( event->data.keyDown.chr != vchrThumbWheelPush ) {
				if ( DebounceEnterKeyEndTime > 0 ) {
					DebounceEnterKeyEndTime = 0;
				}
			}
			MemSet( & newEvent, sizeof( EventType ), 0 );
			if ( event->data.keyDown.chr == vchrThumbWheelPush ) {
				UInt32 Duration, TicksPerSecond = SysTicksPerSecond();
				// If we need to debounce this Enter key event then just return.
				if ( DebounceEnterKeyEndTime > 0 ) {
					if ( TimGetTicks() < DebounceEnterKeyEndTime )
						return true;
				}
				DebounceEnterKeyEndTime = 0;
				// Determine how long the Enter key is depressed.
				Duration = CheckKeyDuration( keyBitEnter, TicksPerSecond );
				// We take different actions based on how long the Enter key was depressed.
				if ( Duration < TicksPerSecond ) {
					// < 1 sec: Translate the Enter key to an AgendaDayViewButton event.
					newEvent.eType = ctlSelectEvent;
					newEvent.data.ctlSelect.controlID = AgendaDayViewButton;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					// Indicate we don't need to debounce the Enter key.
					DebounceEnterKeyEndTime = 0;
					// Queue the event.
					EvtAddUniqueEventToQueue( &newEvent, 0x00000001, true );
					return true;
				} else {
					// > 1 sec: A data record has been selected, perform appropriate action.
					UInt16 attr;
					// Indicate that we need to debounce (i.e. ignore) Enter key
					// event(s) that occur too quickly after this one.
					DebounceEnterKeyEndTime = TimGetTicks() + TicksPerSecond / 2;
					// Beep to provide user feedback.
					SndPlaySystemSound( sndConfirmation );
					// If there is no selected record then there is nothing to do.
					if ( SelectedRecord == noRecordSelected )
						return true;
					// Get the category and secret attribute of the current record.
					DmRecordInfo( ( SelectionFocus == AgendaFocusApptList ) ? ApptDB : Agenda.todoDB,
					              SelectedRecord, &attr, NULL, NULL );
					// If this is a "private" record, then determine what is to be shown.
					if ( attr & dmRecAttrSecret ) {
						if ( SecVerifyPW( showPrivateRecords ) == true ) {
							Int16 row;
							TablePtr table = GetObjectPtr( ( SelectionFocus == AgendaFocusApptList ) ? AgendaDatebookTable : AgendaToDoTable );
							// We only want to unmask this one record, so restore the preference.
							PrefSetPreference( prefShowPrivateRecords, maskPrivateRecords );
							// Goto the selected record.
							if ( TblFindRowID( table, SelectedRecord, & row ) ) {
								EventType newEvent;
								newEvent.eType = tblEnterEvent;
								newEvent.data.tblEnter.tableID = ( SelectionFocus == AgendaFocusApptList ) ? AgendaDatebookTable : AgendaToDoTable ;
								newEvent.data.tblEnter.pTable = table;
								newEvent.data.tblEnter.row = row;
								newEvent.data.tblEnter.column = descColumn;
								EvtAddUniqueEventToQueue( &newEvent, 0x00000010, true );
								return true;
							}
						} else {
							// Password not entered correctly.
							return true;
						}
					}
					// Queue appropriate event based on which list has focus.
					newEvent.eType = ctlSelectEvent;
					if ( SelectionFocus == AgendaFocusApptList ) {
						newEvent.data.ctlSelect.controlID = AgendaApptListSelect;
						newEvent.data.ctlSelect.pControl =
							FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
						EvtAddUniqueEventToQueue( &newEvent, 0x00000002, true );
					} else if ( SelectionFocus == AgendaFocusToDoList ) {
						newEvent.data.ctlSelect.controlID = AgendaToDoListSelect;
						newEvent.data.ctlSelect.pControl =
							FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
						EvtAddUniqueEventToQueue( &newEvent, 0x00000003, true );
					}
					return true;
				}
			} else if ( event->data.keyDown.chr == vchrThumbWheelBack ) {
				// Translate the Back key to an open launcher event.
				newEvent.eType = keyDownEvent;
				newEvent.data.keyDown.chr = launchChr;
				newEvent.data.keyDown.modifiers = commandKeyMask;
				EvtAddUniqueEventToQueue( &newEvent, 0x00000004, true );
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelDown ) {
				// Move the selection highlight down one day, scroll if necessary.
				newEvent.eType = ctlSelectEvent;
				if ( SelectionFocus == AgendaFocusApptList ) {
					newEvent.data.ctlSelect.controlID = AgendaMoveApptSelectionDown;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000005, true );
				} else if ( SelectionFocus == AgendaFocusToDoList ) {
					newEvent.data.ctlSelect.controlID = AgendaMoveToDoSelectionDown;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000006, true );
				}
				return true;
			} else if ( event->data.keyDown.chr == vchrThumbWheelUp ) {
				// Move the selection highlight up one day, scroll if necessary.
				newEvent.eType = ctlSelectEvent;
				if ( SelectionFocus == AgendaFocusApptList ) {
					newEvent.data.ctlSelect.controlID = AgendaMoveApptSelectionUp;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000007, true );
				} else if ( SelectionFocus == AgendaFocusToDoList ) {
					newEvent.data.ctlSelect.controlID = AgendaMoveToDoSelectionUp;
					newEvent.data.ctlSelect.pControl =
						FrmGetObjectPtr(frm,FrmGetObjectIndex(frm, newEvent.data.ctlSelect.controlID));
					EvtAddUniqueEventToQueue( &newEvent, 0x00000008, true );
				}
				return true;
			}
		#endif

		if (EvtKeydownIsVirtual(event))
			{
			// Datebook key pressed? Only the Datebook button will reach this
			//	point.
			if (TxtCharIsHardKey(event->data.keyDown.modifiers, event->data.keyDown.chr))
				{
				// If a date other than today is being viewed, or if the user powered up
				// by pressing the Datebook hard key, show today in the agenda view
				DateSecondsToDate (TimGetSeconds (), &date);
				if (DateToInt (date) != DateToInt (Date) ||
					 ((event->data.keyDown.modifiers & poweredOnKeyMask)))
					{
					AgendaViewChangeDate (FrmGetActiveForm (), date);
					}
				
				// otherwise, cycle through to the day view
				else
					FrmGotoForm (DayView);
					
				handled = true;
				}
				
			else if (event->data.keyDown.chr == vchrPageUp)
				{
				frm = FrmGetActiveForm ();
				DateAdjust (&Date, -1);
				AgendaViewChangeDate (frm, Date);
				handled = true;
				}	
			else if (event->data.keyDown.chr == vchrPageDown)
				{
				frm = FrmGetActiveForm ();
				DateAdjust (&Date, +1);
				AgendaViewChangeDate (frm, Date);
				handled = true;
				}
			}
		}


	else if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case AgendaGoToButton:
			case AgendaCurrentDayButton:
				AgendaViewGoToDate ();
				handled = true;
				break;

			case AgendaDayViewButton:
				FrmGotoForm (DayView);
				handled = true;
				break;

			case AgendaWeekViewButton:
				FrmGotoForm (WeekView);
				handled = true;
				break;

			case AgendaMonthViewButton:
				FrmGotoForm (MonthView);
				handled = true;
				break;

			case AgendaToDoCategoryTrigger:
				ListViewSelectCategory ();
				handled = true;
				break;
			#if WRISTPDA
			case AgendaApptListSelect:
				// Go to Day View for the selected appointment.
				{
					TablePtr Table;
					Int16    apptIndex;
					UInt16   recordNum;
					// Beep to provide user feedback.
					SndPlaySystemSound( sndConfirmation );
					// Find the appointment index to go to.
					Table = GetObjectPtr( AgendaDatebookTable );
					TblFindRowID( Table, SelectedRecord, & apptIndex );
					apptIndex += Agenda.apptTopVisibleIndex;
					recordNum = noAppointments;
					if ( apptIndex != noAppointments ) {
						// Get the record index of the next appointment
						MemHandle apptsH = AgendaGetAppointments ();
						if ( apptsH ) {
							ApptInfoType * apptsP = MemHandleLock( apptsH );
							recordNum = apptsP[apptIndex].recordNum;
							MemPtrUnlock( apptsP );
						}
					}
					// Go to Day View.
					GoToAppointment( Agenda.apptDB, recordNum );
					return true;
				}
			case AgendaToDoListSelect:
				// Toggle the To Do item completed status.
				{
					Int16 on, SelectedRow;
					TablePtr Table;
					Table = GetObjectPtr( AgendaToDoTable );
					TblFindRowID( Table, SelectedRecord, & SelectedRow );
					on = TblGetItemInt (Table, SelectedRow, 0 );
					ListViewChangeCompleteStatus( SelectedRow, ( on == 0 ) ? 1 : 0 );
					TblUnhighlightSelection( Table );
					ListViewDrawTable( updateItemMove );
					AgendaViewMoveToDoSelection( 0 );
				}
				return true;
			case AgendaMoveApptSelectionDown:
				// Move the selection highlight down one day, scroll if necessary.
				AgendaViewMoveApptSelection( +1 );
				return true;
			case AgendaMoveApptSelectionUp:
				// Move the selection highlight down one day, scroll if necessary.
				AgendaViewMoveApptSelection( -1 );
				return true;
			case AgendaMoveToDoSelectionDown:
				// Move the selection highlight down one day, scroll if necessary.
				AgendaViewMoveToDoSelection( +1 );
				return true;
			case AgendaMoveToDoSelectionUp:
				// Move the selection highlight down one day, scroll if necessary.
				AgendaViewMoveToDoSelection( -1 );
				return true;
			#endif
			}
		}


	else if (event->eType == ctlRepeatEvent)
		{
		switch (event->data.ctlRepeat.controlID)
			{
			case AgendaPreviousDayButton:
				frm = FrmGetActiveForm ();
				DateAdjust (&Date, -1);
				AgendaViewChangeDate (frm, Date);
//				handled = true;
				break;

			case AgendaNextDayButton:
				frm = FrmGetActiveForm ();
				DateAdjust (&Date, +1);
				AgendaViewChangeDate (frm, Date);
//				handled = true;
				break;
			}
		}

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		AgendaViewInit (frm);
//		FrmDrawForm (frm);
		handled = true;
		}


	else if (event->eType == frmUpdateEvent)
		{
		FrmDrawForm (FrmGetActiveForm ());
		handled = true;		
		}

	else if (event->eType == frmCloseEvent)
		{
		AgendaFreeAppointments ();
		DmCloseDatabase (Agenda.todoDB);
		}

	else if (event->eType == sclRepeatEvent)
		{
		Int16 delta = event->data.sclRepeat.newValue - event->data.sclRepeat.value;

		if (event->data.sclRepeat.scrollBarID == AgendaToDoScroller)
			{
			ListViewScroll (delta);
			}
		else if (event->data.sclRepeat.scrollBarID == AgendaDatebookScroller)
			{
			AgendaDayViewScroll (delta);
			}
		}

	else if (event->eType == tblEnterEvent)
		{
		if (event->data.tblEnter.tableID == AgendaDatebookTable)
			{
			UInt16 apptIndex = TblGetRowID (event->data.tblEnter.pTable, event->data.tblEnter.row);
			UInt16 recordNum = noAppointments;
			
			if (apptIndex != noAppointments)
				{
				// Get the record index of the next appointment
				MemHandle apptsH = AgendaGetAppointments ();
				if (apptsH)
					{
					ApptInfoType* apptsP = MemHandleLock (apptsH);
					recordNum = apptsP[apptIndex].recordNum;
					MemPtrUnlock (apptsP);
					}
				}

			GoToAppointment (Agenda.apptDB, recordNum);
			handled = true;
			}
		else if (event->data.tblEnter.tableID == AgendaToDoTable)
			{
			if (event->data.tblEnter.column != completedColumn)
				{
				UInt16 recordNum = TblGetRowID (event->data.tblEnter.pTable, event->data.tblEnter.row);
				LaunchToDoWithRecord (Agenda.todoDB, recordNum);
				handled = true;
				}
			}
		}
		

	// An item in the table has been selected.
	else if (event->eType == tblSelectEvent)
		{
//		DayViewItemSelected (event);
//		handled = true;
		if (event->data.tblEnter.tableID == AgendaToDoTable)
			{
			if (event->data.tblEnter.column == completedColumn)
				{
				Int16 on = TblGetItemInt (event->data.tblEnter.pTable,
										  event->data.tblEnter.row,
										  event->data.tblEnter.column);
				ListViewChangeCompleteStatus (event->data.tblEnter.row, on);
				#if WRISTPDA
				// Make sure the current selection is still valid.
				AgendaViewMoveToDoSelection( 0 );
				#endif
				}
			}
		}
	
	
	// Refresh the clock, as needed
	else if (event->eType == nilEvent)
		{
		UInt32 now = TimGetSeconds ();
		
		if ((now / minutesInSeconds) != (Agenda.timeSeconds / minutesInSeconds))
			{
			Agenda.timeSeconds = now;
			AgendaViewDrawTime (FrmGetActiveForm ());
			}
		}


	return (handled);
}


/***********************************************************************
 *
 * FUNCTION:    LaunchToDoWithRecord
 *
 * DESCRIPTION: Switch to the To Do application and display the given
 *				record. If there are any problems retrieving the record,
 *				launch the app without it.
 *
 * PARAMETERS:  inRecordNum  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			rbb	6/2/99	Added button for Agenda view
 *
 ***********************************************************************/
void
LaunchToDoWithRecord (
	DmOpenRef					inDB,
	UInt16						inRecordNum )
{
	MemHandle					recordH;
	ToDoDBRecordType*			toDoRecP;
	UInt16						length;
	GoToParamsType*				gotoParamsP;
	UInt16						launchCmd = sysAppLaunchCmdNormalLaunch;
	MemPtr						launchParamsP = NULL;

	// If we're emulating, do nothing so that Gremlins can be run
	// without quitting when we ask to switch applications
#if EMULATION_LEVEL != EMULATION_NONE
	return;
#endif

	recordH = DmQueryRecord( inDB, inRecordNum);
	
	if (recordH)
		{
		toDoRecP = (ToDoDBRecordType*) MemHandleLock (recordH);
		length = StrLen (&toDoRecP->description);
		MemHandleUnlock (recordH);
		
		// Create the param block (system is responsible for disposal)
		gotoParamsP = MemPtrNew(sizeof(GoToParamsType));
		if (gotoParamsP)
			{
			// Fill it in
			MemPtrSetOwner (gotoParamsP, 0);
			gotoParamsP->recordNum		= inRecordNum;			
			gotoParamsP->matchPos		= length;				// put cursor at end of string
			gotoParamsP->matchFieldNum	= descSearchFieldNum;	// same for Datebook and ToDo
			gotoParamsP->searchStrLen	= 0;	// length of match (for Datebook, ignored by ToDo)
			gotoParamsP->matchCustom	= 0;	// length of match (for ToDo, ignored by Datebook)

			DmOpenDatabaseInfo(inDB, &gotoParamsP->dbID, NULL, NULL, &gotoParamsP->dbCardNo, NULL);
		
			launchCmd = sysAppLaunchCmdGoTo;
			launchParamsP = gotoParamsP;
			}
		else
			{
			ErrNonFatalDisplay ("Not enough memory to go to requested record");
			}
		}
	else
		{
		ErrDisplay ("Couldn't find the displayed ToDo record!!!");
		}

	#if WRISTPDA
	// Set feature informing To Do that it was launched by Datebook's Agenda View ('AV').
	FtrSet( WPdaCreator, 'AV', true );
	#endif
	AppLaunchWithCommand (sysFileCToDo, launchCmd, launchParamsP);
}


/***********************************************************************
 *
 * FUNCTION:    GoToAppointment
 *
 * DESCRIPTION: Switch to the To Do application and display the given
 *				record. If there are any problems retrieving the record,
 *				launch the app without it.
 *
 * PARAMETERS:  inRecordNum  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			rbb	6/2/99	Added button for Agenda view
 *
 ***********************************************************************/
void
GoToAppointment (
	DmOpenRef					inDB,
	UInt16						inRecordNum )
{
	MemHandle					recordH = NULL;
	ApptDBRecordType			apptRec;
	UInt16						length;
	GoToParamsType*				gotoParamsP;
	UInt16						launchCmd = sysAppLaunchCmdNormalLaunch;
	MemPtr						launchParamsP = NULL;
	//Err							err;
	
	if (inRecordNum != noAppointments)
		{
		/*err =*/ ApptGetRecord (inDB, inRecordNum, &apptRec, &recordH);
		
		if (recordH)
			{
			// recordH is locked by ApptGetRecord and referenced by apptRec
			length = StrLen (apptRec.description);
			MemHandleUnlock (recordH);
			
			// Create the param block (system is responsible for disposal)
			gotoParamsP = MemPtrNew(sizeof(GoToParamsType));
			if (gotoParamsP)
				{
				// Fill it in
				MemPtrSetOwner (gotoParamsP, 0);
				gotoParamsP->recordNum		= inRecordNum;			
				gotoParamsP->matchPos		= length;				// put cursor at end of string
				gotoParamsP->matchFieldNum	= descSearchFieldNum;	// same for Datebook and ToDo
				gotoParamsP->searchStrLen	= 0;	// length of match (for Datebook, ignored by ToDo)
				gotoParamsP->matchCustom	= 0;	// length of match (for ToDo, ignored by Datebook)

				DmOpenDatabaseInfo(inDB, &gotoParamsP->dbID, NULL, NULL, &gotoParamsP->dbCardNo, NULL);
				
				launchCmd = sysAppLaunchCmdGoTo;
				launchParamsP = gotoParamsP;

	#if EMULATION_LEVEL == EMULATION_NONE
				AppLaunchWithCommand (sysFileCDatebook, launchCmd, launchParamsP);
	#else
				GoToItem (gotoParamsP, false);
				MemPtrFree (gotoParamsP);
	#endif
				
				goto Exit;
				}
			else
				{
				ErrNonFatalDisplay ("Not enough memory to go to requested record");
				}
			}
		else
			{
			ErrDisplay ("Couldn't find the displayed Datebook record!!!");
			}
		}
	
	// If there are no records displayed or if there was an error, jump to the Day view
	// without making a selection
	FrmGotoForm (DayView);
	
Exit:
	return;
}


/***********************************************************************
 *
 * FUNCTION:    AgendaViewHandleTblEnter
 *
 * DESCRIPTION: This routine is the event handler for the Agenda View
 *              of the Datebook application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	4/8/96	Initial Revision
 *			rbb	6/2/99	Added button for Agenda view
 *
 ***********************************************************************/
Boolean
AgendaViewHandleTblEnter (
	EventType*							event )
{
	Boolean							handled = false;
	
	if (event->data.tblEnter.tableID == AgendaDatebookTable)
		{
		FrmGotoForm (DayView);
		handled = true;
		}
	else if (event->data.tblEnter.tableID == AgendaToDoTable)
		{
		GoToParamsType*				gotoParamsP;
		UInt16						recordNum;
		MemHandle					recordH;
		ToDoDBRecordType*			toDoRecP;
		UInt16						length;
		//Err							err;
		
		UInt16						launchCmd = sysAppLaunchCmdNormalLaunch;
		MemPtr						launchParamsP = NULL;
		
		// If we're emulating, do nothing so that Gremlins can be run
		// without quitting when we ask to switch applications
#if EMULATION_LEVEL != EMULATION_NONE
		goto Exit;
#endif

		if (Agenda.todoDB)
			{
			recordNum = TblGetRowID (event->data.tblEnter.pTable, event->data.tblEnter.row);
			
			if (recordNum != noTasks)
				{
				recordH = DmQueryRecord( Agenda.todoDB, recordNum);
				
				if (recordH)
					{
					toDoRecP = (ToDoDBRecordType*) MemHandleLock (recordH);
					length = StrLen (&toDoRecP->description);
					MemHandleUnlock (recordH);
					
					// Create the param block (system is responsible for disposal)
					gotoParamsP = MemPtrNew(sizeof(GoToParamsType));
					if (!gotoParamsP) {
						//err = memErrNotEnoughSpace;
						//ErrNonFatalDisplayIf(err, "");
						goto Exit;
						}
						
					// Fill it in
					MemPtrSetOwner (gotoParamsP, 0);
					gotoParamsP->searchStrLen	= 0;	// Ignored for ToDo
					gotoParamsP->recordNum		= TblGetRowID ( event->data.tblEnter.pTable,
																event->data.tblEnter.row);
					gotoParamsP->matchPos		= length;
					gotoParamsP->matchFieldNum	= descSearchFieldNum;
					gotoParamsP->matchCustom	= 0;	// For ToDo, this is the length of the match
				
					launchCmd = sysAppLaunchCmdGoTo;
					launchParamsP = &gotoParamsP;
					}
				else
					{
					//err = dmErrCantFind;
					ErrDisplay ("Couldn't find the displayed ToDo record!!!");
					}
				}
			}
			
		#if WRISTPDA
		// Set feature informing To Do that it was launched by Datebook's Agenda View ('AV').
		FtrSet( WPdaCreator, 'AV', true );
		#endif
		AppLaunchWithCommand (sysFileCToDo, launchCmd, launchParamsP);
		handled = true;
		}
	else
		{
		ErrDisplay ("Unexpected value in event->data.tblEnter.tableID");
		}
	
Exit:
	return handled;
}


//#pragma mark -
/***********************************************************************
 *
 * FUNCTION:    AgendaLoadTasks
 *
 * DESCRIPTION: 
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	5/28/99	Initial Revision
 *
 ***********************************************************************/
static void
AgendaViewFillTasks (
	FormType*						inFormP )
{
//#pragma unused (inFormP)

	UInt16							row;
	UInt16							recordNum;
	UInt32							uniqueID;
	TableType*						tableP;
	UInt16 							attr;
	Boolean 							masked;
	
	CHECK_DB_LEAKS (Agenda.todoDB);
	// Get the height of the table and the width of the description
	// column.
	tableP = GetObjectPtr (AgendaToDoTable);

	row = 0;
	recordNum = TopVisibleRecord;
	
	// Load records into the table.
	while (row < Agenda.todoLineCount)
		{
		// Get the next record in the current category.
		if ( !SeekRecord (Agenda.todoDB, &recordNum, 0, dmSeekForward,
								CurrentCategory, Date, ShowCompletedItems, ShowOnlyDueItems ))
			{
			ErrDisplay ("Record not found");
			break;
			}
		
		// Redraw the row if the unique id of the record does not match the unique id
		// stored in the row, or if the row has otherwise been invalidated
		DmRecordInfo (Agenda.todoDB, recordNum, NULL, &uniqueID, NULL);
		if ( (TblGetRowData (tableP, row) != uniqueID) || (TblRowInvalid (tableP, row)) )
			{
			ListInitTableRow (tableP, row, recordNum, Agenda.todoLineHeight);
			}

		else 
			{
			// If the contents of the item or its display characteristics have changed,
			// invalidate the item
			Boolean redraw = false;
			
			if (recordNum != TblGetRowID (tableP, row))
				{
				TblSetRowID (tableP, row, recordNum);
				redraw = true;
				}
			
			//Mask if appropriate
			DmRecordInfo (Agenda.todoDB, recordNum, &attr, NULL, NULL);
		   masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));	
		   
		   if (masked != TblRowMasked (tableP, row))
		   	{
				TblSetRowMasked (tableP, row, masked);
				redraw = true;
				}
			
			if (redraw)
				{
				TblMarkRowInvalid (tableP, row);
				}
			}
			
		row++;
		recordNum++;
		}
	
	CHECK_DB_LEAKS (Agenda.todoDB);
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
static Boolean
SeekRecord (
	DmOpenRef						inDB,
	UInt16*							ioIndexP,
	UInt16							inOffset,
	Int16								inDirection,
	UInt16							inCategory,
	DateType							inDate,
	Boolean							inShowCompleted,
	Boolean							inShowDueOnly )
{
	MemHandle						recordH = NULL;
	ToDoDBRecordType*				toDoRec;
	Boolean							found = false;
	Err								err;

	ErrFatalDisplayIf ( (inOffset > 1), "Invalid offset");

	while (!found)
		{
		err = DmSeekRecordInCategory (inDB, ioIndexP, inOffset, inDirection, inCategory);
		if (err)
			{
			break;
			}
	
		if ( inShowCompleted && (! inShowDueOnly))
			{
			found = true;
			break;
			}
		
		if (inOffset == 0)
			{
			inOffset = 1;
			}

		recordH = DmQueryRecord (inDB, *ioIndexP);
		
		// Since we got the index from DmSeekRecordInCategory, the query should
		// always succeed
		ErrFatalDisplayIf (!recordH, "Couldn't query record");
		
		if (recordH)
			{
			toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);
			
			if ( (inShowCompleted) || (! (toDoRec->priority & completeFlag)))
				{
				if (!inShowDueOnly
						|| (DateToInt (toDoRec->dueDate) == toDoNoDueDate)
						|| (DateToInt (toDoRec->dueDate) <= DateToInt (inDate)) )
					{
					found = true;
					}
				}
			
			MemHandleUnlock (recordH);	
			}
		}

	CHECK_DB_LEAKS (Agenda.todoDB);
	return (found);
}


/***********************************************************************
 *
 * FUNCTION:    SeekCurrentRecord
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
 *			dje	10/25/00	Initial Revision
 *
 ***********************************************************************/
Boolean
SeekCurrentRecord (
	DmOpenRef						inDB,
	UInt16*							ioIndexP,
	UInt16							inOffset,
	Int16								inDirection )
{
	return SeekRecord (
		inDB,
		ioIndexP,
		inOffset,
		inDirection,
		CurrentCategory,
		Date,
		ShowCompletedItems,
		ShowOnlyDueItems);
}


static UInt16
CountRecords (
	DmOpenRef						inDB,
	UInt16*							outIndices,
	UInt16							inIndexCount,
	UInt16							inCategory,
	DateType							inDate,
	Boolean							inShowCompleted,
	Boolean							inShowDueOnly )
{
	UInt16							recordNum = 0;
	UInt16							count = 0;
	
	while (SeekRecord (inDB, &recordNum, 0, dmSeekForward, inCategory, inDate,
								inShowCompleted, inShowDueOnly) )
		{
		if (count < inIndexCount)
			{
			outIndices[count] = recordNum;
			}
			
		++recordNum;
		++count;
		}
	
	return count;
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
static void ListInitTableRow (TablePtr table, UInt16 row, UInt16 recordNum, 
	Int16 rowHeight)
{
	UInt32							uniqueID;
	MemHandle						recordH;
	ToDoDBRecordType*				toDoRec;
	UInt16 							attr;
	Boolean 						masked;

debug(1, "XXX", "ListInitTableRow row %d", row);
	CHECK_DB_LEAKS (Agenda.todoDB);
	// Get a pointer to the ToDo record.
	recordH = DmQueryRecord( Agenda.todoDB, recordNum);
	toDoRec = (ToDoDBRecordType*) MemHandleLock (recordH);

	// Make the row usable.
	TblSetRowUsable (table, row, true);
	
	// Set the height of the row to the height of the description.
	TblSetRowHeight (table, row, rowHeight);
	
	// Store the record number as the row id.
	TblSetRowID (table, row, recordNum);
	
	// Store the unique id of the record in the table.
	DmRecordInfo (Agenda.todoDB, recordNum, NULL, &uniqueID, NULL);
	TblSetRowData (table, row, uniqueID);
	
	//Mask if appropriate
	DmRecordInfo (Agenda.todoDB, recordNum, &attr, NULL, NULL);
   	masked = (((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords));	
	TblSetRowMasked(table,row,masked);

	// Set the checkbox that indicates the completion status.
	TblSetItemInt (table, row, completedColumn, 
		(toDoRec->priority & completeFlag) == completeFlag);

	// Store the priority in the table.
	TblSetItemInt (table, row, priorityColumn, 
		toDoRec->priority & priorityOnly);
	
	// Store the due date in the table.
	TblSetItemInt (table, row, dueDateColumn, (*(Int16 *) &toDoRec->dueDate));

	// Mark the row invalid so that it will drawn when we call the 
	// draw routine.
	TblMarkRowInvalid (table, row);
	
	MemHandleUnlock (recordH);
	CHECK_DB_LEAKS (Agenda.todoDB);
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
static UInt16 ListViewGetColumnWidth (UInt16 column)
{
	Char		chr;
	Char		dateBuffer [longDateStrLength];
	UInt16		width = 0;
	FontID	curFont;
	Char*	dateStr;


	curFont = FntSetFont (Agenda.todoFont);


	if (column == priorityColumn)
		{
		//if (Agenda.todoFont == FossilStdFont)
			//FntSetFont (FossilBoldFont);
		if (Agenda.todoFont == stdFont)
			FntSetFont (boldFont);
		chr = '1';
		width = (FntCharWidth (chr) - 1) + 6;
		}

	else if (column == dueDateColumn)
		{
		DateToAscii (12, 31, 1997,	Agenda.dateFormat, dateBuffer);

		// Remove the year from the date string.
		dateStr = dateBuffer;
		if ((Agenda.dateFormat == dfYMDWithSlashes) ||
			 (Agenda.dateFormat == dfYMDWithDots) ||
			 (Agenda.dateFormat == dfYMDWithDashes))
			dateStr += 3;
		else
			dateStr[StrLen(dateStr) - 3] = 0;

		width = FntCharsWidth (dateStr, StrLen (dateStr));

		// Get the width of the character that indicates the item is due.
		// Don't count the whitespace in the character.
		//if (Agenda.todoFont == FossilStdFont)
			//FntSetFont (FossilBoldFont);
		if (Agenda.todoFont == stdFont)
			FntSetFont (boldFont);
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


#if 0
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
static Err ListViewGetDescription (void* table, UInt16 row, UInt16 column,
	Boolean editable, MemHandle* textH, UInt16* textOffset, UInt16* textAllocSize,
	FieldPtr fld)
{
//#pragma unused (column, editable)

	UInt16 recordNum;
	MemHandle recordH;
	FieldAttrType attr;
	ToDoDBRecordPtr toDoRec;
	
	// Get the record number that corresponds to the table item.
	// The record number is stored as the row id.
	recordNum = TblGetRowID (table, row);
	recordH = DmQueryRecord( Agenda.todoDB, recordNum);
	ErrFatalDisplayIf ((! recordH), "Record not found");
	
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	*textOffset = &toDoRec->description - ((Char *) toDoRec);
	*textAllocSize = StrLen (&toDoRec->description) + 1;  // one for null terminator
	*textH = recordH;
	
	MemHandleUnlock (recordH);

	// Set the field to support auto-shift.
	if (fld)
		{
		FldGetAttributes (fld, &attr);
		attr.underlined = false;
		attr.editable = false;
		attr.singleLine = true;
		FldSetAttributes (fld, &attr);
		}


	return (0);
}
#endif


#if 0
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
static Boolean		RecordDirty = false;			// true if a record has been modified
static UInt16		ListEditPosition = 0;		// position of the insertion point in the desc field
static UInt16		ListEditSelectionLength;	// length of the current selection.

static Boolean ListViewSaveDescription (void* table, UInt16 row, UInt16 column)
{
//#pragma unused (column)

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
		DirtyRecord (Agenda.todoDB, recordNum);

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
#endif


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
static void ListViewDrawDueDate (void* table, Int16 row, Int16 column, 
	RectanglePtr bounds)
{
//#pragma unused (column)

	char dueChr;
	char dateBuffer [longDateStrLength];
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
	if (DateToInt (date) == -1)
		{
		//curFont = FntSetFont (FossilStdFont);
		curFont = FntSetFont (stdFont);
		drawX = bounds->topLeft.x + ((bounds->extent.x - 5) >> 1);
		drawY = bounds->topLeft.y + ((FntLineHeight () + 1) / 2);
		WinDrawLine (drawX, drawY, drawX+5, drawY);		
		FntSetFont (curFont);
		return;
		}
	
	// Get the width of the character that indicates the item is due.  Don't
	// count the whitespace in the character.
	//if (Agenda.todoFont == FossilStdFont)
		//fontID = FossilBoldFont;
	if (Agenda.todoFont == stdFont)
		fontID = boldFont;
	else
		fontID = Agenda.todoFont;
	curFont = FntSetFont (fontID);
	dueChr = '!';
	dueChrWidth = FntCharWidth (dueChr) - 1;

	FntSetFont (Agenda.todoFont);
	
	DateToAscii (date.month, date.day, date.year + firstYear, 
					Agenda.dateFormat, dateBuffer);

	// Remove the year from the date string.
	dateStr = dateBuffer;
	if ((Agenda.dateFormat == dfYMDWithSlashes) ||
		 (Agenda.dateFormat == dfYMDWithDots) ||
		 (Agenda.dateFormat == dfYMDWithDashes))
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
static void ListViewDrawCategory (void* table, Int16 row, Int16 column, 
	RectanglePtr bounds)
{
//#pragma unused (column)

	Int16 width;
	Int16 length;
	UInt16 attr;
	UInt16 category;
	UInt16 recordNum;
	Boolean fits;
	Char categoryName [dmCategoryLength];
	FontID curFont;

	curFont = FntSetFont (Agenda.todoFont);

	// Get the category of the item in the specified row.
	recordNum = TblGetRowID (table, row);
	DmRecordInfo (Agenda.todoDB, recordNum, &attr, NULL, NULL);
	category = attr & dmRecAttrCategoryMask;

	// Get the name of the category and trunctae it to fix the the 
	// column passed.
	CategoryGetName (Agenda.todoDB, category, categoryName);
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
 * FUNCTION:    ListViewDrawDesc
 *
 * DESCRIPTION: Draw the description of a task.  This
 *              routine is called by the table object as a callback 
 *              routine.
 *
 * PARAMETERS:  table  - pointer to the memo Day table (TablePtr)
 *              row    - row of the table to draw
 *              column - column of the table to draw
 *              bounds - region to draw in 
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			rbb	8/30/99	Initial Revision
 *
 ***********************************************************************/
static void
ListViewDrawDesc (
	void*								inTableP,
	Int16								inRow,
	Int16								inColumn,
	RectangleType*					inBoundsP )
{
//#pragma unused (inColumn)
	UInt16							recordNum;
	MemHandle						recordH = NULL;
	ToDoDBRecordType*				todoRecP;
	FontID							curFont;
	Char*								tempP;
	UInt16							tempCount = 0;

	// Get a pointer to the ToDo record.
	recordNum = TblGetRowID (inTableP, inRow);
	recordH = DmQueryRecord( Agenda.todoDB, recordNum);
	
	ErrFatalDisplayIf (!recordH, "Missing record");
	
	if (recordH)
		{
		todoRecP = (ToDoDBRecordType*) MemHandleLock (recordH);
		
		curFont = FntSetFont (Agenda.todoFont);

		tempP = &todoRecP->description;
		while (*tempP && *tempP != linefeedChr)
			{
			++tempCount;
			++tempP;
			}
		WinDrawTruncChars (&todoRecP->description, tempCount,
									inBoundsP->topLeft.x, inBoundsP->topLeft.y,
									inBoundsP->extent.x);
		FntSetFont (curFont);
		
		MemHandleUnlock (recordH);
		}
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
 *			Name	Date			Description
 *			----	----			-----------
 *			art	03/10/95		Initial Revision
 *			rbb	04/14/99		Uses new ListViewDrawTable
 *			gap	08/13/99		Update to use new constants categoryHideEditCategory & categoryDefaultEditCategoryString.
 *
 ***********************************************************************/
static UInt16 ListViewSelectCategory (void)
{
	FormPtr frm;
	UInt16 category;
	Boolean categoryEdited;
	UInt16 updateCode = updateCategoryChanged;
	
	// Process the category popup list.  
	category = CurrentCategory;

	frm = FrmGetActiveForm();
	categoryEdited = CategorySelect (Agenda.todoDB, frm, AgendaToDoCategoryTrigger,
  					    AgendaToDoCategoryList, true, &category, CategoryName, 1, categoryHideEditCategory);
	
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

		AgendaViewSetScrollThumb (frm, AgendaToDoScroller, 0);
				
		Agenda.todoCount = CountRecords (Agenda.todoDB, &TopVisibleRecord, 1, CurrentCategory,
											Date, ShowCompletedItems, ShowOnlyDueItems);

		// Display the new category.
		ListViewDrawTable (updateCode);
		}

	#if WRISTPDA
	if ( SelectionFocus == AgendaFocusToDoList ) {
		TablePtr Table;
		// Make sure the selection stays valid.
		if ( ToDoVisibleRecord() == false ) {
			// There are no visible ToDo records.  If there are Appt	
			// records then force the selection focus to the Appt list.
			if ( Agenda.apptCount > 0 ) {
				SelectionFocus = AgendaFocusApptList;
				Table = GetObjectPtr( AgendaDatebookTable );
				TopVisibleRecord = Agenda.apptTopVisibleIndex;
				SelectedRecord = Agenda.apptTopVisibleIndex + Agenda.apptLineCount - 1;
				AgendaViewMoveApptSelection( +1 );
				AgendaViewMoveApptSelection( -1 );
			}
		} else {
			// Redraw the ToDo table.
			Table = GetObjectPtr( AgendaToDoTable );
			TblUnhighlightSelection( Table );
			TblMarkTableInvalid( Table );
			TblRedrawTable( Table );
			// When changing category set SelectedRecord to the first record
			// of the category (if there is one), otherwise set to noRecord.
			TopVisibleRecord = 0;
			if ( ! ToolsSeekRecord( Agenda.todoDB, & TopVisibleRecord, 0, +1 ) )
				TopVisibleRecord = noRecordSelected;
			SelectedRecord = CurrentRecord = TopVisibleRecord;
			AgendaViewMoveToDoSelection( 0 );
		}
	}
	#endif

	return (category);
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
 *			rbb		4/14/99		Initial Revision
 *
 ***********************************************************************/
static void ListViewDrawTable (UInt16 updateCode)
{
	FormType*							formP;
	
	
	formP = FrmGetActiveForm ();
	
	// If the display options or font were changed, the attributes for each
	// cell need to be regenerated
	if ( (updateCode == updateDisplayOptsChanged) 
			|| (updateCode == updateFontChanged)
			|| (updateCode == updateItemMove) )
		{
		AgendaViewInitToDo (formP);
		TblMarkTableInvalid (GetObjectPtr (AgendaToDoTable));
		}
	
	// There are other valid update codes used by the ToDo app, but I'm
	// not expecting to see them here.
	ErrFatalDisplayIf (		(updateCode != updateDisplayOptsChanged)
								&& (updateCode != updateFontChanged)
								&& (updateCode != updateCategoryChanged)
								&& (updateCode != updateGoTo)
								&& (updateCode != updateItemMove)
							, "Unexpected update code"
							);
	
	AgendaViewRefreshLayoutTasks (formP);
		
	TblRedrawTable (GetObjectPtr (AgendaDatebookTable));
	AgendaViewDrawSeparator (formP);
	TblRedrawTable (GetObjectPtr (AgendaToDoTable));
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
 *			art		2/21/95		Initial Revision
 *			rbb		4/14/99		Uses new ListViewDrawTable
 *
 ***********************************************************************/
static void ListViewScroll (Int16 delta)
{
	TablePtr 			table;
	Int16					seekDirection;
	UInt16				count;
	Int16					scrollValue;
	Int16					scrollMin;
	Int16					scrollMax;
	Int16					pageSize;
	UInt16				recordNum;

	if (delta == 0)
		{
		return;
		}
	
	table = GetObjectPtr (AgendaToDoTable);
	TblReleaseFocus (table);
	
	seekDirection = ( delta > 0 ? dmSeekForward : dmSeekBackward );
	count = ( delta > 0 ? delta : -delta );
	
	SclGetScrollBar (GetObjectPtr (AgendaToDoScroller), &scrollValue, &scrollMin,
							&scrollMax, &pageSize);
							
	// Find the new top visible record
	recordNum = TopVisibleRecord;
	while (count)
		{
		if (!SeekRecord (Agenda.todoDB, &recordNum, 1, seekDirection,
						CurrentCategory, Date, ShowCompletedItems, ShowOnlyDueItems))
			{
			break;
			}
		
		TopVisibleRecord = recordNum;
		--count;
		}
	
	ListViewDrawTable(updateGoTo);
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
 *			art	 3/21/96	Initial Revision
 *			gap	11/17/00	Use FntCharsInWidth instead of FldWordWrap to calculate 
 *								the number of characters to strikethrough as FntCharsInWidth
 *								missed truncated words.
 *			gap	11/20/00	clean up above fix a little more to correcly handle tabs.
 *
 ***********************************************************************/
static void ListViewCrossOutItem (Int16 row)
{
	Int16 length;
	UInt16 maxWidth;
	UInt16 recordNum;
	Int16 x, y;
	//Int16 lineHeight;
	FontID curFont;
	Char* chars;
	TablePtr table;
	RectangleType r;
	RectangleType tableR;
	MemHandle recordH;
	ToDoDBRecordPtr toDoRec;
	Int16 charsWidth;
	Boolean truncated;

	table = GetObjectPtr (AgendaToDoTable);

	// Get a pointer to the ToDo record.
	recordNum = TblGetRowID (table, row);
	recordH = DmQueryRecord( Agenda.todoDB, recordNum);
	toDoRec = (ToDoDBRecordPtr) MemHandleLock (recordH);

	curFont = FntSetFont (Agenda.todoFont);
	//lineHeight = FntLineHeight ();	

	TblGetBounds (table, &tableR);

	TblGetItemBounds (table, row, descColumn, &r);
	maxWidth = r.extent.x;
	
	// If the record has a note, leave space for the note indicator.
	if (*GetToDoNotePtr (toDoRec))
		maxWidth -= tableNoteIndicatorWidth;

	chars = &toDoRec->description;
	length = 0;

	y = r.topLeft.y + (Agenda.todoLineHeight >> 1);

	if (*chars)
		{
		
		// Get the number of character on each line.
		charsWidth = maxWidth;
		length = StrLen(chars);
		FntCharsInWidth (chars, &charsWidth, &length, &truncated);

		x = r.topLeft.x;
		while (charsWidth)
			{
			WinDrawLine (x, y, x, y);
			x++;
			charsWidth--;
			}
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
	UInt32 ticks = 0;
	TablePtr table;
	DateType dueDate;
	DateTimeType today;
	UInt32 mode;
	DmOpenRef todoDB;
	
	// Open a read/write copy of the database (the rest of the agenda view
	// uses read-only)
	mode = (PrivateRecordVisualStatus == hidePrivateRecords) ?
					dmModeReadWrite : (dmModeReadWrite | dmModeShowSecret);
	ToDoGetDatabase (&todoDB, mode);

	table = GetObjectPtr (AgendaToDoTable);
	recordNum = TblGetRowID (table, row);
	
	// If completed item are not shown then display an animation 
	// of a line being drawn through the item.
	if (! ShowCompletedItems)
		{
		ticks = TimGetTicks ();
		ListViewCrossOutItem (row);
		}

	// Update the record to reflect the new completion status.
	ToDoChangeRecord (todoDB, &recordNum, toDoComplete, &complete);

	// Should the due date be changed to the completion date?
	if (complete && ChangeDueDate)
		{
		TimSecondsToDateTime (TimGetSeconds (), &today);
		dueDate.year = today.year - firstYear;
		dueDate.month = today.month;
		dueDate.day = today.day;
		ToDoChangeRecord (todoDB, &recordNum, toDoDueDate, &dueDate);
//		CurrentRecord = recordNum;
		}

	// Mark the record dirty.
	DirtyRecord (todoDB, recordNum);



	// If completed items are shown and the due date was changed, redraw the list
	if (ShowCompletedItems)
		{
		if (complete && ChangeDueDate)
			{
			AgendaViewSetTopTask ();
			ListViewDrawTable (updateItemMove);
			}
		}

	// If completed items are hidden, update the table.
	else
		{
		UInt16	distance = 0;
		Int16		seekDirection = dmSeekForward;
		Int16		scrollValue;
		Int16		scrollMin;
		Int16		scrollMax;
		Int16		pageSize;
		
		// Wait long enough for the user to see the cross-out animation
		while (TimGetTicks () - ticks < crossOutDelay)
			{
			}
		
		// Remove this record from the count of displayable records
		Agenda.todoCount -= 1;
		
		// Reconfigure the scroll bar for one less record
		SclGetScrollBar (GetObjectPtr (AgendaToDoScroller), &scrollValue, &scrollMin,
								&scrollMax, &pageSize);
		scrollMax = max (scrollMax - 1, 0);
		
		// If we're already at the bottom of the scroller, look backward to find
		// a new record for the top of the list
		if ((scrollValue >= scrollMax) && scrollValue != 0)
			{
			distance = 1;
			seekDirection = dmSeekBackward;
			scrollValue = scrollMax;
			}
		
		SclSetScrollBar (GetObjectPtr (AgendaToDoScroller), scrollValue, scrollMin,
								scrollMax, pageSize);
			
		// Find the new top visible record
		SeekRecord (todoDB, &TopVisibleRecord, distance, seekDirection,
							CurrentCategory, Date, ShowCompletedItems, ShowOnlyDueItems );
		
		// Refresh the appointment and task tables
		ListViewDrawTable (updateItemMove);
		}

	DmCloseDatabase (todoDB);
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


#if 0
/************************************************************
 *
 *  FUNCTION: ToDoAppInfoInit
 *
 *  DESCRIPTION: Create an app info chunk if missing.  Set
 *		the strings to a default.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/20/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
#define LocalizedAppInfoStr			1000

Err	ToDoAppInfoInit(DmOpenRef dbP)
{
	UInt16 cardNo;
	UInt16 wordValue;
	MemHandle h;
	LocalID dbID;
	LocalID appInfoID;
	ToDoAppInfoPtr	nilP = 0;
	ToDoAppInfoPtr appInfoP;
	
	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;

	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
		 NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;
	
	if (appInfoID == NULL)
		{
		h = DmNewHandle(dbP, sizeof (ToDoAppInfoType));
		if (! h) return dmErrMemError;

		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, 
			NULL, &appInfoID, NULL, NULL, NULL);
		}
	
	appInfoP = MemLocalIDToLockedPtr(appInfoID, cardNo);

	// Clear the app info block.
	DmSet (appInfoP, 0, sizeof(ToDoAppInfoType), 0);

	// Initialize the categories.
	CategoryInitialize ((AppInfoPtr) appInfoP, LocalizedAppInfoStr);

	// I don't know what this field is used for.
	wordValue = 0xFFFF;
	DmWrite (appInfoP, (UInt32)&nilP->dirtyAppInfo, &wordValue,
		sizeof(appInfoP->dirtyAppInfo));

	// Initialize the sort order.
	DmSet (appInfoP, (UInt32)&nilP->sortOrder, sizeof(appInfoP->sortOrder), 
		soPriorityDueDate);

	MemPtrUnlock (appInfoP);

	return 0;
}
#endif


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
	if (prefsVersion > toDoPrefsVersionNum) {
		prefsVersion = noPreferenceFound;
	}
	if (prefsVersion > noPreferenceFound)
		{
		if (prefsVersion < toDoPrefsVersionNum) {
			prefs.noteFont = prefs.v20NoteFont;
		}
		CurrentCategory = prefs.currentCategory;
		
		// Since the FontSelect call only lets the user pick the std, stdBold, and largeBold
		// fonts, remap the FossilLargeFont to FossilLargeBoldFont. DOLATER kwk - this code makes assumptions
		// about how the FontSelect call works, which might not be true in the future, especially
		// for other character encodings.
		//if (prefs.noteFont == FossilLargeFont)
			//NoteFont = FossilLargeBoldFont;
		if (prefs.noteFont == largeFont)
			NoteFont = largeBoldFont;
		else
			NoteFont = prefs.noteFont;
		
		ShowAllCategories = prefs.showAllCategories;
		ShowCompletedItems = prefs.showCompletedItems;
		ShowOnlyDueItems = prefs.showOnlyDueItems;
		ShowDueDates = prefs.showDueDates;
		ShowPriorities = prefs.showPriorities;
		ShowCategories = prefs.showCategories;
		ChangeDueDate = prefs.changeDueDate;
		SaveBackup = prefs.saveBackup;
		
		// Agenda view does not honor ToDo font preferences
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
		NoteFont = FntGlueGetDefaultFontID(defaultSystemFont);
		}
	#if WRISTPDA
	ListFont = FossilLargeFontID( WRISTPDA, ListFont );
	NoteFont = FossilLargeFontID( WRISTPDA, NoteFont );
	#endif
}





#if 0
static void
CheckDBLeaks (
	DmOpenRef						inDB )
{
	UInt8								highest;
	UInt32							count;
	UInt32							busy;

	DmGetDatabaseLockState (inDB, &highest, &count, &busy);
	ErrNonFatalDisplayIf (highest > 0, "Records left locked");
	ErrNonFatalDisplayIf (busy > 0, "Records left busy");
}
#endif
