/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrView.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *   This is the Address Book application's view form module.
 *
 *****************************************************************************/

#include "sec.h"

#include "AddrView.h"
#include "AddrDialList.h"
#include "AddrEdit.h"
#include "AddrDetails.h"
#include "AddrNote.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddressRsc.h"
#include "AddrDefines.h"
#include "AddressTransfer.h"

#include <TextMgr.h>
#include <ErrorMgr.h>
#include <SoundMgr.h>
#include <StringMgr.h>
#include <AboutBox.h>
#include <Category.h>
#include <Menu.h>
#include <UIResources.h>
#include <TraceMgr.h>
#if WRISTPDA
#include <palmUtils.h>
#endif

// number of record view lines to store
#define recordViewLinesMax		55
#define recordViewBlankLine		0xffff   // Half height if the next line.x == 0

// Resource type used to specify order of fields in Edit view.
#define	fieldMapRscType			'fmap'

/***********************************************************************
 *
 *   Internal Structures
 *
 ***********************************************************************/

// Info on how to draw the record view
typedef struct
{
	UInt16			fieldNum;
	UInt16			length;
	UInt16			offset;
	UInt16			x;
} RecordViewLineType;


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

static AddrDBRecordType		recordViewRecord;
static MemHandle			recordViewRecordH = 0;
static RecordViewLineType	*RecordViewLines;
static UInt16				RecordViewLastLine;   // Line after last one containing data
static UInt16				TopRecordViewLine;
static UInt16				RecordViewFirstPlainLine;


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void		PrvViewOpen(void);
static void		PrvViewClose(void);
static void		PrvViewInit( FormType* frm );
static void		PrvViewNewLine (UInt16 *width);
static void		PrvViewAddSpaceForText (const Char * const string, UInt16 *width);
static void		PrvViewPositionTextAt (UInt16 *width, const UInt16 position);
static void		PrvViewAddField (const UInt16 fieldNum, UInt16 *width, const UInt16 maxWidth, const UInt16 indentation);
static void		PrvViewErase( FormType* frmP );
static UInt16	PrvViewCalcNextLine(UInt16 i, UInt16 oneLine);
static void		PrvViewDrawSelectedText (UInt16 currentField, UInt16 selectPos, UInt16 selectLen, UInt16 textY);
static void		PrvViewDraw ( FormType* frmP, UInt16 selectFieldNum, UInt16 selectPos, UInt16 selectLen, Boolean drawOnlySelectField );
static void		PrvViewDrawBusinessCardIndicator (FormPtr formP);
static void		PrvViewUpdate( FormType* frmP );
static UInt16	PrvViewScrollOnePage (Int16 newTopRecordViewLine, WinDirectionType direction);
static void		PrvViewScroll (WinDirectionType direction);
static void		PrvViewMakeVisible (UInt16 selectFieldNum, UInt16 selectPos, UInt16 selectLen);
static Boolean	PrvViewPhoneNumberAt (Int16 x, Int16 y, UInt16 *fieldNumP, UInt16 *offsetP, UInt16 *lengthP);
static Boolean 	PrvViewHandleTapOnPhoneNumber (UInt16 fieldNum, UInt16 offset, UInt16 length);
static Boolean 	PrvViewHandlePen (EventType * event);
static Boolean 	PrvViewDoCommand (UInt16 command);

/***********************************************************************
 *
 * FUNCTION:    ViewHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Address View"
 *              of the Address Book application.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *				Name	Date		Description
 *				----	----		-----------
 *				art		6/5/95		Initial Revision
 *				jmp		10/01/99	Fix frmUpdateEvent so that it redraws the form
 *									and updated the RecordView now that we can get
 *									into a situation where the bits might not necessarily
 *									be restored except through and update event itself.
 * 				FPa		11/28/00	kFrmCustomUpdateEvent handling
 *
 ***********************************************************************/
Boolean ViewHandleEvent(EventType * event)
{
	UInt32 numLibs;
	Boolean handled = false;
	FormType* frmP;

	#if WRISTPDA
	EventType newEvent;
	FormPtr frm;
	#endif

	switch (event->eType)
	{

		case frmOpenEvent:
			PrvViewOpen();
			handled = true;
			break;
	
		case frmCloseEvent:
			PrvViewClose();
			break;
	
		case ctlSelectEvent:
			switch (event->data.ctlSelect.controlID)
			{
				case RecordDoneButton:
					// When we return to the ListView highlight this record.
					ListViewSelectThisRecord = CurrentRecord;
					FrmGotoForm (ListView);
					handled = true;
					break;
		
				case RecordEditButton:
					FrmGotoForm (EditView);
					handled = true;
					break;
		
				case RecordNewButton:
					EditNewRecord();
					handled = true;
					break;
				default:
					break;
			}
			break;
	
	
		case penDownEvent:
			handled = PrvViewHandlePen(event);
			break;
	
	
		case ctlRepeatEvent:
			switch (event->data.ctlRepeat.controlID)
			{
				case RecordUpButton:
					PrvViewScroll(winUp);
					// leave unhandled so the buttons can repeat
					break;
		
				case RecordDownButton:
					PrvViewScroll(winDown);
					// leave unhandled so the buttons can repeat
					break;
				default:
					break;
			}
			break;
	
	
		case keyDownEvent:

			#if WRISTPDA
			if ( ( event->data.keyDown.chr == vchrThumbWheelPush ) ||
				 ( event->data.keyDown.chr == vchrThumbWheelBack ) ) {
				// Translate the Enter and Back keys to a Done button event.
				MemSet (&newEvent, sizeof(EventType), 0);
				frm = FrmGetActiveForm();
				newEvent.eType = ctlSelectEvent;
				newEvent.data.ctlSelect.on = true;
				newEvent.data.ctlSelect.controlID = RecordDoneButton;
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
				FrmGotoForm (ListView);
				handled = true;
			}
	
			else if (EvtKeydownIsVirtual(event))
			{
				switch (event->data.keyDown.chr)
				{
					case vchrPageUp:
						PrvViewScroll (winUp);
						handled = true;
						break;
		
					case vchrPageDown:
						PrvViewScroll (winDown);
						handled = true;
						break;
		
					case vchrSendData:
						TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
						handled = true;
						break;
		
					default:
						break;
				}
			}
			break;
	
		case menuEvent:
			return PrvViewDoCommand (event->data.menu.itemID);
	
	
		case menuCmdBarOpenEvent:
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarDeleteBitmap, menuCmdBarResultMenuItem, RecordRecordDeleteRecordCmd, 0);
			MenuCmdBarAddButton(menuCmdBarOnLeft, BarBeamBitmap, menuCmdBarResultMenuItem, RecordRecordBeamRecordCmd, 0);
	
			// tell the field package to not add cut/copy/paste buttons automatically
			event->data.menuCmdBarOpen.preventFieldButtons = true;
	
			// don't set handled to true; this event must fall through to the system.
			break;
	
		case menuOpenEvent:
			if(!ToolsIsDialerPresent())
				MenuHideItem(RecordRecordDialCmd);

			if (ExgGetRegisteredApplications(NULL, &numLibs, NULL, NULL, exgRegSchemeID, exgSendScheme) || !numLibs)
				MenuHideItem(RecordRecordSendRecordCmd);
			else
				MenuShowItem(RecordRecordSendRecordCmd);
			// don't set handled = true
			break;
	
		case frmUpdateEvent:
			// Do not use ActiveForm here since the update event is broadcasted to all open forms
			TraceOutput(TL(appErrorClass, "ViewHandleEvent() - frmUpdateEvent"));
			frmP = FrmGetFormPtr(RecordView);
			FrmDrawForm(frmP);
			PrvViewUpdate(frmP);
			handled = true;
			break;
	
		case kFrmCustomUpdateEvent:
			// Event sent by Custom view because when custom fields are renamed, it can be necessary to recalculate view screen display: if the width of the custom field is enlarged (and if its content can only be displayed using 2 lines), its content can be displayed on the next line
			PrvViewClose();
			PrvViewOpen();
			handled = true;
			break;
			
		case frmGotoEvent:
			TraceOutput(TL(appErrorClass, "ViewHandleEvent() - event->data.frmGoto.matchLen = %hu", event->data.frmGoto.matchLen));
			frmP = FrmGetActiveForm ();
			CurrentRecord = event->data.frmGoto.recordNum;
			PrvViewInit(frmP);
			PrvViewMakeVisible(event->data.frmGoto.matchFieldNum, event->data.frmGoto.matchPos, event->data.frmGoto.matchLen);
			FrmDrawForm(frmP);
			PrvViewDraw(frmP, event->data.frmGoto.matchFieldNum, event->data.frmGoto.matchPos, event->data.frmGoto.matchLen, false);
			PriorAddressFormID = FrmGetFormId(frmP);
			handled = true;
			break;
	
		default:
			break;
	}

	return (handled);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvViewOpen
 *
 * DESCRIPTION: This routine is called when frmOpenEvent is received
 *
 * PARAMETERS:  None
 *
 * RETURNED:    None
 *
 * REVISION HISTORY:
 *        	Name   	Date      	Description
 *        	----   	----      	-----------
 *			FPa		11/28/00	Initial Revision
 *
 ***********************************************************************/
void PrvViewOpen(void)
{
	FormType* frmP;
	
	frmP = FrmGetActiveForm ();
	PrvViewInit(frmP);
	FrmDrawForm(frmP);
	PrvViewDraw(frmP, 0, 0, 0, false);
	PrvViewDrawBusinessCardIndicator(frmP);
	PriorAddressFormID = FrmGetFormId(frmP);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewClose
 *
 * DESCRIPTION: This routine is called when frmCloseEvent is received
 *
 * PARAMETERS:  None
 *
 * RETURNED:    None
 *
 * REVISION HISTORY:
 *        	Name   	Date      	Description
 *        	----   	----      	-----------
 *			FPa		11/28/00	Initial Revision
 *			FPa		12/05/00	Added MemHandleUnlock
 *
 ***********************************************************************/
void PrvViewClose(void)
{
	MemHandle handle;
	
	if (recordViewRecordH)
	{
		MemHandleUnlock(recordViewRecordH);
		recordViewRecordH = 0;
	}
	
	handle = MemPtrRecoverHandle(RecordViewLines);
	MemHandleUnlock(handle);
	MemHandleFree(handle);
	
	RecordViewLines = 0;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewInit
 *
 * DESCRIPTION: This routine initializes the "Record View" of the
 *              Address application.  Most importantly it lays out the
 *                record and decides how the record is drawn.
 *
 * PARAMETERS:  frm - pointer to the view form.
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *        	Name   	Date      	Description
 *        	----   	----      	-----------
 *        	roger  	6/21/95   	Initial Revision
 *			aro		09/25/00	Add fieldMapH to release the resource properly
 *			FPa		11/27/00	Do not add company field if this field is blank
 *
 ***********************************************************************/
void PrvViewInit( FormType* frm )
{
	UInt16 attr;
	UInt16 index;
	UInt16 category;
	AddrAppInfoPtr appInfoPtr;
	MemHandle RecordViewLinesH;
	UInt16 width = 0;
	RectangleType r;
	UInt16 maxWidth;
	FontID curFont;
	UInt16 i;
	UInt16 fieldIndex;
	UInt16 phoneLabelNum;
	const AddressFields* fieldMap;
	MemHandle fieldMapH;
	Int16 cityIndex = -1;
	Int16 zipIndex = -1;


	// Set the category label.
	if (CurrentCategory == dmAllCategories)
	{
		DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);
		category = attr & dmRecAttrCategoryMask;
	}
	else
		category = CurrentCategory;

	CategoryGetName (AddrDB, category, CategoryName);
	index = FrmGetObjectIndex (frm, RecordCategoryLabel);
	FrmSetCategoryLabel (frm, index, CategoryName);

	// Allocate the record view lines
	RecordViewLinesH = MemHandleNew(sizeof(RecordViewLineType) * recordViewLinesMax);
	ErrFatalDisplayIf (!RecordViewLinesH, "Out of memory");

	RecordViewLines = MemHandleLock(RecordViewLinesH);
	RecordViewLastLine = 0;
	TopRecordViewLine = 0;

	FrmGetFormBounds(frm, &r);
	maxWidth = r.extent.x;

	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);

	#if WRISTPDA
	// Always update RecordLabelColumnWidth based on current font
	#else
	if (RecordLabelColumnWidth == 0)
	#endif
		RecordLabelColumnWidth = ToolsGetLabelColumnWidth (appInfoPtr, AddrRecordFont);


	// Get the record to display.  recordViewRecordH may have data if
	// we are redisplaying the record (custom fields changed).
	if (recordViewRecordH)
		MemHandleUnlock(recordViewRecordH);
	AddrDBGetRecord (AddrDB, CurrentRecord, &recordViewRecord, &recordViewRecordH);

	// Here we construct the recordViewLines info by laying out
	// the record
	#if WRISTPDA
	curFont = FntSetFont (FossilBoldFont);
	#else
	curFont = FntSetFont (largeBoldFont);
	#endif
	if (recordViewRecord.fields[ad_name] == NULL &&
		recordViewRecord.fields[ad_firstName] == NULL &&
		recordViewRecord.fields[ad_company] != NULL)
	{
		PrvViewAddField(ad_company, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}
	else
	{
		if (recordViewRecord.fields[ad_firstName] != NULL)
		{
			PrvViewAddField(ad_firstName, &width, maxWidth, 0);
			
			// Separate the last name from the first name as long
			// as they are together on the same line.
			if (width > 0)
				PrvViewAddSpaceForText (" ", &width);
			#if WRISTPDA
			PrvViewNewLine(&width);
			#endif
		}
		PrvViewAddField(ad_name, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}
	RecordViewFirstPlainLine = RecordViewLastLine;
	FntSetFont (AddrRecordFont);

	if (recordViewRecord.fields[ad_title])
	{
		PrvViewAddField(ad_title, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}
	if (recordViewRecord.fields[ad_company] != NULL &&
		(recordViewRecord.fields[ad_name] != NULL ||
		 recordViewRecord.fields[ad_firstName] != NULL))
	{
		PrvViewAddField(ad_company, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}

	// If the line above isn't blank then add a blank line
	if (RecordViewLastLine > 0 &&
		RecordViewLines[RecordViewLastLine - 1].fieldNum != recordViewBlankLine)
	{
		PrvViewNewLine(&width);
	}



	// Layout the phone numbers.  Start each number on its own line.
	// Put the label first, followed by ": " and then the number
	for (fieldIndex = firstPhoneField; fieldIndex <= lastPhoneField; fieldIndex++)
	{
		if (recordViewRecord.fields[fieldIndex])
		{
			phoneLabelNum = GetPhoneLabel(&recordViewRecord, fieldIndex);
			PrvViewAddSpaceForText (appInfoPtr->fieldLabels[phoneLabelNum + ((phoneLabelNum < numPhoneLabelsStoredFirst) ? firstPhoneField : (ad_addressFieldsCount - numPhoneLabelsStoredFirst))], &width);
			PrvViewAddSpaceForText (": ", &width);
			PrvViewPositionTextAt(&width, RecordLabelColumnWidth);
			PrvViewAddField(fieldIndex, &width, maxWidth, RecordLabelColumnWidth);
			PrvViewNewLine(&width);
		}
	}


	// If the line above isn't blank then add a blank line
	if (RecordViewLastLine > 0 &&
		RecordViewLines[RecordViewLastLine - 1].fieldNum != recordViewBlankLine)
	{
		PrvViewNewLine(&width);
	}



	// Now do the address information
	if (recordViewRecord.fields[ad_address])
	{
		PrvViewAddField(ad_address, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}

	// We need to format the city, state, and zip code differently depending
	// on which country it is. For now, assume that if the city comes first,
	// we use the standard US formatting of [city, ][state   ][zip]<cr>,
	// otherwise we'll use the "int'l" format of [zip ][city]<cr>[state]<cr>.
	// DOLATER kwk - A better way of handling this would be to use a formatting
	// resource, which had N records, one for each line, where each record
	// had M entries, one for each field, and each entry had a field id and
	// suffix text.
	i = 0;
	fieldMapH = DmGetResource(fieldMapRscType, FieldMapID);
	fieldMap = (const AddressFields*)MemHandleLock(fieldMapH);
	while ((cityIndex == -1) || (zipIndex == -1))
	{
		if (fieldMap[i] == ad_city)
		{
			cityIndex = i;
		}
		else if (fieldMap[i] == ad_zipCode)
		{
			zipIndex = i;
		}

		i++;
	}

	MemHandleUnlock(fieldMapH);
	DmReleaseResource(fieldMapH);

	// Decide if we're formatting it US-style, or int'l-style
	if (cityIndex < zipIndex)
	{
		if (recordViewRecord.fields[ad_city])
		{
			PrvViewAddField(ad_city, &width, maxWidth, 0);
		}
		if (recordViewRecord.fields[ad_state])
		{
			if (width > 0)
				PrvViewAddSpaceForText (", ", &width);
			PrvViewAddField(ad_state, &width, maxWidth, 0);
		}
		if (recordViewRecord.fields[ad_zipCode])
		{
			if (width > 0)
				PrvViewAddSpaceForText ("   ", &width);
			PrvViewAddField(ad_zipCode, &width, maxWidth, 0);
		}
		if (recordViewRecord.fields[ad_city] ||
			recordViewRecord.fields[ad_state] ||
			recordViewRecord.fields[ad_zipCode])
		{
			PrvViewNewLine(&width);
		}
	}
	else
	{
		if (recordViewRecord.fields[ad_zipCode])
		{
			PrvViewAddField(ad_zipCode, &width, maxWidth, 0);
		}
		if (recordViewRecord.fields[ad_city])
		{
			if (width > 0)
				PrvViewAddSpaceForText (" ", &width);
			PrvViewAddField(ad_city, &width, maxWidth, 0);
		}
		if (recordViewRecord.fields[ad_zipCode] ||
			recordViewRecord.fields[ad_city])
		{
			PrvViewNewLine(&width);
		}
		if (recordViewRecord.fields[ad_state])
		{
			PrvViewAddField(ad_state, &width, maxWidth, 0);
			PrvViewNewLine(&width);
		}
	}

	if (recordViewRecord.fields[ad_country])
	{
		PrvViewAddField(ad_country, &width, maxWidth, 0);
		PrvViewNewLine(&width);
	}


	// If the line above isn't blank then add a blank line
	if (RecordViewLastLine > 0 &&
		RecordViewLines[RecordViewLastLine - 1].fieldNum != recordViewBlankLine)
	{
		PrvViewNewLine(&width);
	}


	// Do the custom fields
	for (i = ad_custom1; i < ad_addressFieldsCount - 1; i++)
	{
		if (recordViewRecord.fields[i])
		{
			PrvViewAddSpaceForText (appInfoPtr->fieldLabels[i], &width);
			PrvViewAddSpaceForText (": ", &width);
			PrvViewPositionTextAt(&width, RecordLabelColumnWidth);
			PrvViewAddField(i, &width, maxWidth, RecordLabelColumnWidth);
			PrvViewNewLine(&width);
			PrvViewNewLine(&width);      // leave a blank line
		}
	}

	// Show the note field.
	if (recordViewRecord.fields[ad_note])
	{
		PrvViewAddField(ad_note, &width, maxWidth, 0);
	}


	// Now remove trailing blank lines
	while (RecordViewLastLine > 0 &&
		   RecordViewLines[RecordViewLastLine - 1].fieldNum == recordViewBlankLine)
	{
		RecordViewLastLine--;
	}


	MemPtrUnlock(appInfoPtr);
	FntSetFont (curFont);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewNewLine
 *
 * DESCRIPTION: Adds the next field at the start of a new line
 *
 * PARAMETERS:  width - width already occupied on the line
 *
 * RETURNED:    width is set
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
void PrvViewNewLine (UInt16 *width)
{
	if (RecordViewLastLine >= recordViewLinesMax)
		return;

	if (*width == 0)
	{
		RecordViewLines[RecordViewLastLine].fieldNum = recordViewBlankLine;
		RecordViewLines[RecordViewLastLine].x = 0;
		RecordViewLastLine++;
	}
	else
		*width = 0;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewAddSpaceForText
 *
 * DESCRIPTION: Adds space for text to the RecordViewLines info.
 *
 * PARAMETERS:  string - Char * to text to leave space for
 *                width - width already occupied on the line
 *
 * RETURNED:    width is increased by the width of the text
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/21/95   Initial Revision
 *
 ***********************************************************************/
void PrvViewAddSpaceForText (const Char * const string, UInt16 *width)
{
	*width += FntCharsWidth(string, StrLen(string));
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewPositionTextAt
 *
 * DESCRIPTION: Position the following text at the given position.
 *
 * PARAMETERS:  position - position to indent to
 *                width - width already occupied on the line
 *
 * RETURNED:    width is increased if the position is greater
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/2/95   Initial Revision
 *
 ***********************************************************************/
void PrvViewPositionTextAt (UInt16 *width, const UInt16 position)
{
	if (*width < position)
		*width = position;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewAddField
 *
 * DESCRIPTION: Adds a field to the RecordViewLines info.
 *
 * PARAMETERS:  fieldNum - field to add
 *                width - width already occupied on the line
 *                maxWidth - can't add words past this width
 *                indentation - the amounnt of indentation wrapped lines of
 *                              text should begin with (except the last)
 *
 * RETURNED:    width is set to the width of the last line added
 *
 * HISTORY:
 *		06/21/95	rsf	Created by Roger Flores
 *		10/25/99	kwk	Fix sign extension w/calling TxtCharIsSpace
 *
 ***********************************************************************/
void PrvViewAddField (const UInt16 fieldNum, UInt16 *width, const UInt16 maxWidth, const UInt16 indentation)
{
	UInt16 length;
	UInt16 offset = 0;
	UInt16 newOffset;


	if (recordViewRecord.fields[fieldNum] == NULL ||
		RecordViewLastLine >= recordViewLinesMax)
		return;

	// If we're past the maxWidth already then start at the beginning
	// of the next line.
	if (*width >= maxWidth)
		*width = indentation;

	do {
		if (RecordViewLastLine >= recordViewLinesMax)
			break;

		// Check if we word wrapped in the middle of a word which could
		// fit on the next line.  Word wrap doesn't work well for use
		// when we call it twice on the same line.
		// The first part checks to see if we stopped in the middle of a line
		// The second part check to see if we didn't stop after a word break
		// The third part checks if this line wasn't a wide as it could be
		// because some other text used up space.
		// DOLATER kwk - Japanese version is completely different...decide
		// if it can be used for English & other languages. This code below
		// isn't exactly correct because it's checking the last byte of
		// the string in the call to TxtCharIsSpace, which might be the
		// low byte of a double-byte character.
		length = FldWordWrap(&recordViewRecord.fields[fieldNum][offset], maxWidth - *width);
		if (recordViewRecord.fields[fieldNum][offset + length] != '\0'
			&& !TxtCharIsSpace((UInt8)recordViewRecord.fields[fieldNum][offset + length - 1])
			&& (*width > indentation))
		{
			length = 0;            // don't word wrap - try next line
		}

		// Lines returned from FldWordWrap may include a '\n' at the
		// end.  If present remove it to keep it from being drawn.
		// The alternative is to not draw linefeeds at draw time.  That
		// seem more complex (there's many WinDrawChars) and slower as well.
		// This way is faster but makes catching word wrapping problems
		// less obvious (length 0 also happens when word wrap fails).
		newOffset = offset + length;
		if (newOffset > 0 && recordViewRecord.fields[fieldNum][newOffset - 1] == linefeedChr)
			length--;

		RecordViewLines[RecordViewLastLine].fieldNum = fieldNum;
		RecordViewLines[RecordViewLastLine].offset = offset;
		RecordViewLines[RecordViewLastLine].x = *width;
		RecordViewLines[RecordViewLastLine].length = length;
		RecordViewLastLine++;
		offset = newOffset;

		// Wrap to the start of the next line if there's still more text
		// to draw (so we must have run out of room) or wrap if we
		// encountered a line feed character.
		if (recordViewRecord.fields[fieldNum][offset] != '\0')
			*width = indentation;
		else
			break;

	} while (true);


	// If the last character was a new line then there is no width.
	// Otherwise the width is the width of the characters on the last line.
	if (recordViewRecord.fields[fieldNum][offset - 1] == linefeedChr)
		*width = 0;
	else
		*width += FntCharsWidth(&recordViewRecord.fields[fieldNum][RecordViewLines[RecordViewLastLine - 1].offset], RecordViewLines[RecordViewLastLine - 1].length);
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewErase
 *
 * DESCRIPTION: Erases the record view
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/30/95   Initial Revision
 *
 ***********************************************************************/
void PrvViewErase( FormType* frmP )
{
	RectangleType r;
	FrmGetObjectBounds(frmP, FrmGetObjectIndex(frmP, RecordViewDisplay), &r);
	WinEraseRectangle (&r, 0);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewCalcNextLine
 *
 * DESCRIPTION: This routine returns the amount of extra vertical space
 *					 required due to the item at this index in the lines array.
 *					 If there are multiple items in the line array which go on
 *					 the same y coordinate, the first is the one which
 *					 contributes the vertical space requirement. Text which
 *					 begins to the left of text on the previous line starts a
 *					 new line. Blank lines use only a half line to save space.
 *
 * PARAMETERS:  i - the line to base how far to advance
 *                oneLine - the amount which advance one line down.
 *
 * RETURNED:    the amount to advance.  Typically oneLine or oneLine / 2.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   9/28/95   Initial Revision
 *			  peter	10/06/00	  Replaced description above.
 *
 ***********************************************************************/
UInt16 PrvViewCalcNextLine(UInt16 i, UInt16 oneLine)
{
	// Advance down if the text starts before the text of the current line.
	if (RecordViewLines[i].x == 0 || 
	   (i > 0 &&
	   (RecordViewLines[i].x <= RecordViewLines[i - 1].x || RecordViewLines[i - 1].fieldNum == recordViewBlankLine)))
	{
		// A non blank line moves down a full line.
		if (RecordViewLines[i].fieldNum != recordViewBlankLine)
		{
			return oneLine;
		}
		else
		{
			// A recordViewBlankLine is half-height.
			return oneLine / 2;
		}
	}
	return 0;      // Stay on the same line.
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawSelectedText
 *
 * DESCRIPTION: Inverts text which is considered selected.
 *
 * PARAMETERS:  currentField - field containing the selected text
 *              selectPos 	  - offset into field for start of selected text
 *              selectLen 	  - length of selected text.  This field
 *                  			    should be zero if selected text isn't desired.
 *              textY 		  - where on the screen the text was drawn
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  11/27/95  Cut from PrvViewDraw
 *         jmp    04-21-00  Fixed bug #23860:  This routine was calling
 *                          WInvertRectangle(), which produced somewhat unpredicatable
 *                          results in a color environment.  Changed the routine
 *                          to call WinDrawInvertChars() instead, which swaps
 *                          the foreground and background colors appropriately.
 *			  peter	05/05/00	 Remove code that tried but failed to extend selection rectangle 1 pixel.
 *									 This code could be fixed, but we'd also need to erase the extra pixel.
 *			  peter	05/17/00	 Change to display text with selected object colors.
 *
 ***********************************************************************/
void PrvViewDrawSelectedText (UInt16 currentField, UInt16 selectPos, UInt16 selectLen, UInt16 textY)
{
	UInt16 selectXLeft = 0;
	UInt16 selectXRight = 0;
	
	// If the start of the selected region is on this line, calc an x.
	if ( (RecordViewLines[currentField].offset <= selectPos) && (selectPos < RecordViewLines[currentField].offset + RecordViewLines[currentField].length) )
	{
		selectXLeft = FntCharsWidth(&recordViewRecord.fields[RecordViewLines[currentField].fieldNum][RecordViewLines[currentField].offset], selectPos - RecordViewLines[currentField].offset);
	}
	// If the end of the selected region is on this line, calc an x.
	if ( (RecordViewLines[currentField].offset <= selectPos + selectLen) &&	(selectPos + selectLen <= RecordViewLines[currentField].offset + RecordViewLines[currentField].length))
	{
		selectXRight = FntCharsWidth(&recordViewRecord.fields[RecordViewLines[currentField].fieldNum][RecordViewLines[currentField].offset], selectPos + selectLen - RecordViewLines[currentField].offset);
	}

	// If either the left or right have been set then some
	// text needs to be selected.
	if (selectXLeft | selectXRight)
	{
		// Switch to selected object colors.
		WinPushDrawState();
		WinSetBackColor(UIColorGetTableEntryIndex(UIObjectSelectedFill));
		WinSetForeColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));
		WinSetTextColor(UIColorGetTableEntryIndex(UIObjectSelectedForeground));

		// Draw the text with the selection colors.
		WinDrawChars(&recordViewRecord.fields[RecordViewLines[currentField].fieldNum][selectPos], selectLen, selectXLeft += RecordViewLines[currentField].x, textY);

		// Restore non-selected object colors.
		WinPopDrawState();
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDraw
 *
 * DESCRIPTION: This routine initializes the "Record View"
 *
 * PARAMETERS:  selectFieldNum - field to show selected text
 *                selectPos - offset into field for start of selected text
 *                selectLen - length of selected text.  This field
 *                  should be zero if selected text isn't desired.
 *						drawOnlySelectField	- whether one or all fields are drawn
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * HISTORY:
 *		06/21/95	rsf		Created by Roger Flores
 *		02/06/98	tlw		Change test below "If we are past bottom stop drawing" from
 *							 if >= bottomOfRecordViewDisplay - FntLineHeight() to
 *							 if > bottomOfRecordViewDisplay - FntLineHeight()
 *							 to allow last line to be drawn.
 *		07/29/99	kwk		When drawing zip code, load prefix (# of spaces) from resource.
 *		05/17/00	peter	Explicitly set colors to draw in.
 *		09/25/00	aro		Adding frmP as an argument for the frmUpdateEvent
 *		10/06/00	peter & danny	When first line is blank, leave the 1/2 line gap used for blank lines.
 *		11/28/00	FPa		Fixed bug #45991
 *		11/29/00	FPa		Fixed bug #46272
 *
 ***********************************************************************/
void PrvViewDraw ( FormType* frmP, UInt16 selectFieldNum, UInt16 selectPos, UInt16 selectLen, Boolean drawOnlySelectField )
{
	AddrAppInfoPtr appInfoPtr;
	UInt16 y;
	UInt16 previousNonZeroHeight;
	UInt16 currentHeight;
	UInt16 i;
	FontID curFont;
	UInt16 phoneLabelNum;
	Char * fieldLabelString;
	UInt16 fieldLabelLength;
	UInt16 upIndex;
	UInt16 downIndex;
	Boolean scrollableUp;
	Boolean scrollableDown;
	RectangleType r;
	int bottomOfRecordViewDisplay;
#if WRISTPDA
	char*  str;
	Boolean customTitle[4] = { false, false, false, false };
	UInt16  idx;
#endif
	
	TraceOutput(TL(appErrorClass, "PrvViewDraw()"));

	TraceOutput(TL(appErrorClass, "PrvViewDraw() - selectPos = %hu, selectLen = %hu", selectPos, selectLen));

	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(AddrDB);

	FrmGetObjectBounds(frmP, FrmGetObjectIndex(frmP, RecordViewDisplay), &r);
	bottomOfRecordViewDisplay = r.topLeft.y +  r.extent.y;

	// Set the background color in control-style colors since the text is selectable.
	WinPushDrawState();
	WinSetBackColor(UIColorGetTableEntryIndex(UIFieldBackground));
	WinSetForeColor(UIColorGetTableEntryIndex(UIObjectForeground));
	WinSetTextColor(UIColorGetTableEntryIndex(UIObjectForeground));

	// Fill the entire record view display with the background color.
	if (! drawOnlySelectField)
		WinEraseRectangle(&r, 0);

	if (TopRecordViewLine < RecordViewFirstPlainLine)
		#if WRISTPDA
		curFont = FntSetFont (AddrRecordFont);
		#else
		curFont = FntSetFont (largeBoldFont);
		#endif
	else
		curFont = FntSetFont (AddrRecordFont);

	y = r.topLeft.y;
	previousNonZeroHeight = 0;

	for (i = TopRecordViewLine; i < RecordViewLastLine; i++)
	{
		// Switch fonts if necessary before computing the extra vertical
		// space needed for this element of the array.
		if (i == RecordViewFirstPlainLine)
			FntSetFont (AddrRecordFont);
		currentHeight = PrvViewCalcNextLine(i, FntLineHeight());
		
		// Since the above function returns zero for all but the first
		// item when several should be drawn at the same y coordinate,
		// we need to delay adding the result until we get to the next
		// non-zero result.
		if (currentHeight != 0)
		{
			y += previousNonZeroHeight;
			previousNonZeroHeight = currentHeight;
		}

		// If we are past the bottom stop drawing
		if (y > bottomOfRecordViewDisplay - FntLineHeight())
			break;

		ErrNonFatalDisplayIf(y < r.topLeft.y, "Drawing record out of gadget");

		if (!drawOnlySelectField || RecordViewLines[i].fieldNum == selectFieldNum)
		{
			if (RecordViewLines[i].offset == 0)
			{
				switch (RecordViewLines[i].fieldNum)
				{
					case recordViewBlankLine:
						break;
	
					case ad_phone1:
					case ad_phone2:
					case ad_phone3:
					case ad_phone4:
					case ad_phone5:
						phoneLabelNum = GetPhoneLabel(&recordViewRecord, RecordViewLines[i].fieldNum);
						fieldLabelString = appInfoPtr->fieldLabels[phoneLabelNum +
																   ((phoneLabelNum < numPhoneLabelsStoredFirst) ? firstPhoneField : (ad_addressFieldsCount - numPhoneLabelsStoredFirst))];
						fieldLabelLength = StrLen(fieldLabelString);
						WinDrawChars(fieldLabelString, fieldLabelLength, 0, y);
						WinDrawChars(": ", 2, FntCharsWidth(fieldLabelString, fieldLabelLength), y);
	
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						break;
	
					case ad_custom1:
					case ad_custom2:
					case ad_custom3:
					case ad_custom4:
					{
						Int16 fieldLabelWidth;
						Int16 fieldLabelCompleteWidth;

						fieldLabelString = appInfoPtr->fieldLabels[RecordViewLines[i].fieldNum];
						fieldLabelLength = StrLen(fieldLabelString);
						fieldLabelWidth = FntCharsWidth(fieldLabelString, fieldLabelLength);
						fieldLabelCompleteWidth = fieldLabelWidth + FntCharsWidth(": ", 2);
								
						#if WRISTPDA
						// PrvViewAddField() might send one or two records, depending on the width
						// of the current font and length of the custom name. To avoid displaying the Label
						// string twice always ignore a 0 length record.
						idx = RecordViewLines[i].fieldNum - custom1;
						if (!customTitle[idx])
						{
							WinDrawTruncChars(fieldLabelString, StrLen(fieldLabelString), 0, y, RecordViewLines[i].x - FntCharsWidth(": ", 2));
							WinDrawChars(": ", 2, min( fieldLabelWidth, RecordViewLines[i].x - FntCharsWidth(": ", 2))/*RecordViewLines[i].x - FntCharsWidth(": ", 2)*/, y);
							customTitle[idx] = true;
						}
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						#else
						if (RecordViewLines[i].length == 0 ||	// If the custom label is displayed on a line and its content on the next line (because the custom label has been renamed using a long name and because it content is multi-line)
						    fieldLabelCompleteWidth <= RecordViewLines[i].x)	// If the label name is not too width, then we display the label name and its content on the same line
						{
							WinDrawChars(fieldLabelString, fieldLabelLength, 0, y);
							WinDrawChars(": ", 2, fieldLabelWidth, y);
						}
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						#endif
						break;
					}
	
					case ad_state:
						if (RecordViewLines[i].x > 0)
							WinDrawChars(", ", 2, RecordViewLines[i].x - FntCharsWidth(", ", 2), y);
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						break;
	
					case ad_zipCode:
						if (RecordViewLines[i].x > 0)
						{
							const Char* textP;
							MemHandle zipCodePrefixH;
							
							zipCodePrefixH = DmGetResource(strRsc, ZipCodePrefixStr);
							textP = (const Char*)MemHandleLock(zipCodePrefixH);
							WinDrawChars(textP, StrLen(textP), RecordViewLines[i].x - FntCharsWidth(textP, StrLen(textP)), y);
							MemPtrUnlock((MemPtr)textP);
							DmReleaseResource(zipCodePrefixH);
						}
	
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						break;

					#if WRISTPDA	
					case firstName:
					case name:
						str = &recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset];
						WinDrawTruncChars(str, StrLen(str), RecordViewLines[i].x, y, r.extent.x);
						break;
					#endif

					default:
						WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
						break;
				}
			}
			else
			{
				// Draw the remainder of the fields' lines without any
				// other special handling.
				if (RecordViewLines[i].fieldNum != recordViewBlankLine)
				{
					WinDrawChars(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset], RecordViewLines[i].length, RecordViewLines[i].x, y);
				}
			}


			// Highlight text if it is within the selection bounds.  This is
			// used to select found text and phone numbers when the user taps on them.
			if ( (RecordViewLines[i].fieldNum == selectFieldNum) && (selectLen > 0) )
			{
				if ( selectPos < RecordViewLines[i].offset + RecordViewLines[i].length )	// If there's a selection to draw on this line (if the beginning of the selection is on this line or one of the previous lines)
				{
					UInt16 pos;
					UInt16 len;
					UInt16 posOffsetFromBeginningOfLine;

					if ( selectPos >= RecordViewLines[i].offset )	// If the beginning of the selection is within the current line
					{
						pos = selectPos;
						posOffsetFromBeginningOfLine = selectPos - RecordViewLines[i].offset;
					}
					else	// The beginning of the selection is within one of the previous lines
					{
						pos = RecordViewLines[i].offset;
						posOffsetFromBeginningOfLine = 0;
					}
					
					if ( selectPos + selectLen < RecordViewLines[i].offset + RecordViewLines[i].length )	// If the end of the selection is within the current line or within one of the previous lines
					{
						if ( selectPos + selectLen >=  RecordViewLines[i].offset + posOffsetFromBeginningOfLine )	// <=> selectPos + selectLen - RecordViewLines[i].offset - posOffsetFromBeginningOfLine (Not to have len < 0)
							len = selectPos + selectLen - RecordViewLines[i].offset - posOffsetFromBeginningOfLine;
						else
							len = 0;
					}
					else	// The end of the selection is within one of the next lines
					{
						len = RecordViewLines[i].length - posOffsetFromBeginningOfLine;
					}
					
					TraceOutput(TL(appErrorClass, "PrvViewDraw() - offset = %hu, length = %hu, selectPos = %hu, selectLen = %hu, pos = %hu, len = %hu, posOffsetFromBeginningOfLine = %hu", RecordViewLines[i].offset, RecordViewLines[i].length, selectPos, selectLen, pos, len, posOffsetFromBeginningOfLine));
					PrvViewDrawSelectedText(i, pos, len, y);
				}
			}
		}
	}

	MemPtrUnlock(appInfoPtr);
	FntSetFont (curFont);
	WinPopDrawState ();

	if (!drawOnlySelectField)
	{
		// Now show/hide the scroll arrows
		scrollableUp = TopRecordViewLine != 0;
		scrollableDown = i < RecordViewLastLine;


		// Update the scroll button.
		upIndex = FrmGetObjectIndex(frmP, RecordUpButton);
		downIndex = FrmGetObjectIndex(frmP, RecordDownButton);
		FrmUpdateScrollers(frmP, upIndex, downIndex, scrollableUp, scrollableDown);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDrawBusinessCardIndicator
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
 *         roger  10/22/97  Initial Revision
 *
 ***********************************************************************/
void PrvViewDrawBusinessCardIndicator (FormPtr formP)
{
	UInt32 uniqueID;

	DmRecordInfo (AddrDB, CurrentRecord, NULL, &uniqueID, NULL);
	if (BusinessCardRecordID == uniqueID)
		FrmShowObject(formP, FrmGetObjectIndex (formP, RecordViewBusinessCardBmp));
	else
		FrmHideObject(formP, FrmGetObjectIndex (formP, RecordViewBusinessCardBmp));

}


/***********************************************************************
 *
 * FUNCTION:    PrvViewUpdate
 *
 * DESCRIPTION: Update the record view and redraw it.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *		Name   Date      	Description
 *		----   ----      	-----------
 *		roger   10/18/95  	Initial Revision
 *		aro		09/26/00	Add frmP as an argument
 *		FPa		11/15/00	Fixed bug #44838
 *
 ***********************************************************************/
void PrvViewUpdate( FormType* frmP )
{
	PrvViewErase(frmP);
	PrvViewDraw(frmP, 0, 0, 0, false);
	PrvViewDrawBusinessCardIndicator(frmP);
}

/***********************************************************************
 *
 * FUNCTION:    PrvViewScrollOnePage
 *
 * DESCRIPTION: Scrolls the record view by one page less one line unless
 * we scroll from RecordViewLastLine (used by scroll code).
 *
 * PARAMETERS:  newTopRecordViewLine - top line of the display
 *              direction - up or dowm
 *
 * RETURNED:    new newTopRecordViewLine one page away
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *			roger		6/22/95	Initial Revision
 *			roger		8/2/95	Reworked to handle half height blank lines.
 *			roger		10/30/95	Reworked to obey FntLineHeight
 *			roger		10/31/95	Broke out of PrvViewScroll
 *
 ***********************************************************************/
UInt16 PrvViewScrollOnePage (Int16 newTopRecordViewLine, WinDirectionType direction)
{
	Int16 offset;
	FontID curFont;
	FormPtr frm;
	Int16 largeFontLineHeight;
	Int16 stdFontLineHeight;
	Int16 currentLineHeight;
	RectangleType r;
	Int16 recordViewDisplayHeight;


	// setup stuff
	#if WRISTPDA
	curFont = FntSetFont (FossilBoldFont);
	#else
	curFont = FntSetFont (largeBoldFont);
	#endif
	largeFontLineHeight = FntLineHeight();
	FntSetFont (AddrRecordFont);
	stdFontLineHeight = FntLineHeight();
	FntSetFont (curFont);

	frm = FrmGetActiveForm();
	FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, RecordViewDisplay), &r);
	recordViewDisplayHeight = r.extent.y;
	if (newTopRecordViewLine != RecordViewLastLine)
		recordViewDisplayHeight -= stdFontLineHeight;   // less one one line


	if (direction == winUp)
		offset = -1;
	else
		offset = 1;


	while (recordViewDisplayHeight >= 0 &&
		   (newTopRecordViewLine > 0 || direction == winDown) &&
		   (newTopRecordViewLine < (RecordViewLastLine - 1) || direction == winUp))
	{
		newTopRecordViewLine += offset;
		if (newTopRecordViewLine < RecordViewFirstPlainLine)
			currentLineHeight = largeFontLineHeight;
		else
			currentLineHeight = stdFontLineHeight;

		recordViewDisplayHeight -= PrvViewCalcNextLine(newTopRecordViewLine,
													   currentLineHeight);
	};

// Did we go too far?
if (recordViewDisplayHeight < 0)
{
	// The last line was too much so remove it
	newTopRecordViewLine -= offset;

	// Also remove any lines which don't have a height
	while (PrvViewCalcNextLine(newTopRecordViewLine, 2) == 0)
	{
		newTopRecordViewLine -= offset;   // skip it
	}
}

return newTopRecordViewLine;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewScroll
 *
 * DESCRIPTION: Scrolls the record view
 *
 * PARAMETERS:  direction - up or dowm
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	06/22/95	Initial Revision
 *			roger	08/02/95	Reworked to handle half height blank lines.
 *			roger	10/30/95	Reworked to obey FntLineHeight
 *			gap		10/12/99	Close command bar before processing scroll
 *			FPa		11/15/00	Fixed bug #44838
 *
 ***********************************************************************/
void PrvViewScroll (WinDirectionType direction)
{
	Int16 lastRecordViewLine;
	UInt16 newTopRecordViewLine;
	UInt16 category;
	UInt16 recordNum;
	Int16 seekDirection;
	UInt16	attr;
	FormType* frmP;

	// Before processing the scroll, be sure that the command bar has been closed.
	MenuEraseStatus (0);
	newTopRecordViewLine = TopRecordViewLine;

	if (direction == winUp)
	{
		newTopRecordViewLine = PrvViewScrollOnePage (newTopRecordViewLine, direction);
	}
	else
	{
		// Simple two part algorithm.
		// 1) Scroll down one page
		// 2) Scroll up one page from the bottom
		// Use the higher of the two positions
		// Find the line one page down

		newTopRecordViewLine = PrvViewScrollOnePage (newTopRecordViewLine, direction);

		// Find the line at the top of the last page
		// (code copied to PrvViewMakeVisible).
		lastRecordViewLine = PrvViewScrollOnePage (RecordViewLastLine, winUp);

		// We shouldn't be past the top line of the last page
		if (newTopRecordViewLine > lastRecordViewLine)
			newTopRecordViewLine = lastRecordViewLine;
	}

	// Get the active form
	frmP = FrmGetActiveForm();

	if (newTopRecordViewLine != TopRecordViewLine)
	{
		TopRecordViewLine = newTopRecordViewLine;

		PrvViewErase(frmP);
		PrvViewDraw(frmP, 0, 0, 0, false);
	}

	// If we couldn't scroll then scroll to the next record.
	else
	{
		// Move to the next or previous record.
		if (direction == winUp)
		{
			seekDirection = dmSeekBackward;
		}
		else
		{
			seekDirection = dmSeekForward;
		}

		if (ShowAllCategories)
			category = dmAllCategories;
		else
			category = CurrentCategory;

		recordNum = CurrentRecord;

		//skip masked records.
		while (!DmSeekRecordInCategory (AddrDB, &recordNum, 1, seekDirection, category) &&
			   !DmRecordInfo (AddrDB, recordNum, &attr, NULL, NULL) &&
			   ((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
		{
		}
		if (recordNum == CurrentRecord) return;
		
		// Don't show first/last record if it's private and we're masking.
		if (!DmRecordInfo (AddrDB, recordNum, &attr, NULL, NULL) &&
			   ((attr & dmRecAttrSecret) && PrivateRecordVisualStatus == maskPrivateRecords))
			return;

		SndPlaySystemSound (sndInfo);

		CurrentRecord = recordNum;

	  	MemHandleFree(MemPtrRecoverHandle(RecordViewLines));
	  	RecordViewLines = 0;
	  	PrvViewInit(frmP);
  	  	
		PrvViewUpdate(frmP);
	}
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewMakeVisible
 *
 * DESCRIPTION: Make a selection range visible
 *
 * PARAMETERS:  selectFieldNum - field to show selected text
 *                selectPos - offset into field for start of selected text
 *                selectLen - length of selected text
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   8/3/95   Initial Revision
 *
 ***********************************************************************/
void PrvViewMakeVisible (UInt16 selectFieldNum, UInt16 selectPos, UInt16 selectLen)
{
	UInt16 newTopRecordViewLine;
	UInt16 i;


	newTopRecordViewLine = RecordViewLastLine;
	for (i = 0; i < RecordViewLastLine; i++)
	{
		// Does the selected range end here?
		if (RecordViewLines[i].fieldNum == selectFieldNum &&
			RecordViewLines[i].offset <= selectPos + selectLen &&
			selectPos + selectLen <= RecordViewLines[i].offset +
			RecordViewLines[i].length)
		{
			newTopRecordViewLine = i;
		}
	}


	// If the selected range doesn't seem to exist then
	// we shouldn't scroll the view.
	if (newTopRecordViewLine == RecordViewLastLine)
		return;


	// Display as much before the selected text as possible
	newTopRecordViewLine = PrvViewScrollOnePage (newTopRecordViewLine, winUp);

	if (newTopRecordViewLine != TopRecordViewLine)
		TopRecordViewLine = newTopRecordViewLine;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewPhoneNumberAt
 *
 * DESCRIPTION: Given a point on the screen in the RecordViewDisplay,
 *					 determine whether that point is in a phone number, and if
 *					 so, which one. Phone numbers are defined as linefeed
 *					 separated.
 *
 * PARAMETERS:	x				- x coordinate of point to look at
 *					y				- y coordinate of point to look at
 *					fieldNumP	- result: which field the phone number is in
 *					offsetP		- result: where phone number starts in field
 *					lengthP		- result: how long phone number is
 *
 * RETURNED:	whether there is a phone number at the given point
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		peter	05/05/00	Initial Revision
 *		peter	05/26/00	Fix bug: Restore font.
 *		aro		06/27/00	Fix bug for non phone field
 *
 ***********************************************************************/
Boolean PrvViewPhoneNumberAt (Int16 x, Int16 y, UInt16 *fieldNumP, UInt16 *offsetP, UInt16 *lengthP)
{
	FormPtr			frm;
	FontID			curFont;
	RectangleType	r;
	Int16				lineY, bottomOfRecordViewDisplay, width, height;
	UInt16			previousNonZeroHeight, currentHeight;
	UInt16			i, j;

	frm = FrmGetActiveForm();
	FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, RecordViewDisplay), &r);

	if (TopRecordViewLine < RecordViewFirstPlainLine)
		#if WRISTPDA
		curFont = FntSetFont (FossilBoldFont);
		#else
		curFont = FntSetFont (largeBoldFont);
		#endif
	else
		curFont = FntSetFont (AddrRecordFont);

	lineY = r.topLeft.y;
	previousNonZeroHeight = 0;
	bottomOfRecordViewDisplay = r.topLeft.y +  r.extent.y;

	for (i = TopRecordViewLine; i < RecordViewLastLine; i++)
	{
		// Switch fonts if necessary before computing the extra vertical
		// space needed for this element of the array.
		if (i == RecordViewFirstPlainLine)
			FntSetFont (AddrRecordFont);
		currentHeight = PrvViewCalcNextLine(i, FntLineHeight());

		// Since the above function returns zero for all but the first
		// item when several should be drawn at the same y coordinate,
		// we need to delay adding the result until we get to the next
		// non-zero result.
		if (currentHeight != 0)
		{
			lineY += previousNonZeroHeight;
			previousNonZeroHeight = currentHeight;
		}

		// If we are past the bottom stop drawing
		if (lineY > bottomOfRecordViewDisplay - FntLineHeight())
			break;

		ErrNonFatalDisplayIf(lineY < r.topLeft.y, "Searching for record out of gadget");

		// The remainder of the fields' lines were drawn without any other special
		// handling. These may include continuations of phone numbers that don't
		// fit on one line as well as entire phone numbers which were included in
		// a field, separated by the return stroke.

		// Check if this is a dialable phone
		if ((RecordViewLines[i].fieldNum >= firstPhoneField)
			&& (RecordViewLines[i].fieldNum <= lastPhoneField)
			&& (ToolsIsPhoneIndexSupported(&recordViewRecord, RecordViewLines[i].fieldNum - firstPhoneField)))
		{
			// Dial the number tapped on.
			width = FntCharsWidth
				(&recordViewRecord.fields[RecordViewLines[i].fieldNum][RecordViewLines[i].offset],
				 RecordViewLines[i].length);
			height = FntCharHeight();
			RctSetRectangle(&r, RecordViewLines[i].x, lineY, width, height);
			if (RctPtInRectangle (x, y, &r))
			{
				*fieldNumP = RecordViewLines[i].fieldNum;

				// Look to see if this phone number started on a previous line.
				for (; i != 0; i--)
				{
					if (RecordViewLines[i - 1].fieldNum != *fieldNumP)
						break;
					if (recordViewRecord.fields[*fieldNumP][RecordViewLines[i].offset - 1] == linefeedChr)
						break;
				}
				*offsetP = RecordViewLines[i].offset;
				*lengthP = RecordViewLines[i].length;

				// Look to see if this phone number continues on subsequent lines.
				for (j = i + 1; j < RecordViewLastLine; j++)
				{
					if (RecordViewLines[j].fieldNum != *fieldNumP)
						break;
					if (recordViewRecord.fields[*fieldNumP][RecordViewLines[j].offset - 1] == linefeedChr)
						break;
					*lengthP += RecordViewLines[j].length;
				}

				FntSetFont (curFont);
				return true;
			}
		}
	}

	FntSetFont (curFont);
	return false;		// Given point isn't in a phone number.
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewHandleTapOnPhoneNumber
 *
 * DESCRIPTION: Handle a tap on a phone number in the RecordViewDisplay.
 *					 Highlight the phone number while the pen is over it, and
 *					 dial if the pen is released over it.
 *
 * PARAMETERS:  	fieldNum	- which field
 *					offset		- start of phone number in field
 *					length		- length of phone number in field
 *
 * RETURNED:    Whether Dial Number screen has been displayed
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		peter	05/05/00	Initial Revision
 *		aro		06/27/00	Add dialing
 *		fpa		10/19/00	Returns a boolean
 *
 ***********************************************************************/
Boolean PrvViewHandleTapOnPhoneNumber (UInt16 fieldNum, UInt16 offset, UInt16 length)
{
	UInt16			testFieldNum, testOffset, testLength;
	Int16				testX, testY;
	Boolean			isPenDown, wasSelected, isSelected;
	FormType*		frmP;

	frmP = FrmGetActiveForm();
	wasSelected = true;
	PrvViewDraw(frmP, fieldNum, offset, length, true);

	do
	{
		PenGetPoint (&testX, &testY, &isPenDown);
		isSelected = PrvViewPhoneNumberAt(testX, testY, &testFieldNum, &testOffset, &testLength) &&
			testFieldNum == fieldNum && testOffset == offset;
		if (isSelected != wasSelected)
		{
			PrvViewDraw(frmP,fieldNum, offset, isSelected ? length : 0, true);
			wasSelected = isSelected;
		}
	} while (isPenDown);

	if (isSelected)
	{
		UInt16 lineIndex;
		PrvViewDraw(frmP, fieldNum, offset, 0, true);

		lineIndex = ToolsGetLineIndexAtOffset(recordViewRecord.fields[fieldNum], offset);
		return DialListShowDialog(CurrentRecord, fieldNum - firstPhoneField, lineIndex);
	}
	else
		return false;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewHandlePen
 *
 * DESCRIPTION: Handle pen movement in the RecordViewDisplay. If the user
 *					 taps in the RecordViewDisplay take them to the Edit View
 *					 unless they tap on a phone number. In that case, arrange
 *					 to dial the selected number.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if handled.
 *
 * REVISION HISTORY:
 *		Name	Date		Description
 *		----	----		-----------
 *		roger	11/27/95	Cut from RecordViewHandleEvent
 *		peter	05/03/00	Add support for tapping on phone numbers to dial
 *      aro     06/27/00    Check for dialing abilities
 *
 ***********************************************************************/
Boolean PrvViewHandlePen (EventType * event)
{
	FormPtr			frm;
	RectangleType	r;
	Int16				x, y;
	Boolean			isPenDown;
	UInt16			fieldNum, offset, length;

	frm = FrmGetActiveForm();
	FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, RecordViewDisplay), &r);
	if (! RctPtInRectangle (event->screenX, event->screenY, &r))
		return false;

	// Check if the user tapped on a phone number.
	if (EnableTapDialing)
	{
		if (PrvViewPhoneNumberAt (event->screenX, event->screenY, &fieldNum, &offset, &length))
		{
			// The user tapped on this phone number. Wait for the pen up, highlighting the
			// phone number when the pen is over the number.
			if ( PrvViewHandleTapOnPhoneNumber (fieldNum, offset, length) )
				return true;
		}
	}

	// The user tapped in the record view display, but not on a phone number,
	// so wait for the pen to be released and if it's released inside the
	// record view display, edit the record.
	do
	{
		PenGetPoint (&x, &y, &isPenDown);
	} while (isPenDown);
	if (RctPtInRectangle (x, y, &r))
		FrmGotoForm (EditView);

	return true;
}


/***********************************************************************
 *
 * FUNCTION:    PrvViewDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date       Description
 *         ----   ----       -----------
 *         roger  06/27/95   Initial Revision
 *         jmp    09/17/99   Use NewNoteView instead of NoteView.
 *         FPa    11/20/00   Fixed a memory leak when deleting a note
 *         FPa    01/26/00   Fixed bug #51545
 *
 ***********************************************************************/
Boolean PrvViewDoCommand (UInt16 command)
{
	UInt16 newRecord;
	UInt16 numCharsToHilite;
	FormType *frmP;
	FontID oldFont;

	switch (command)
	{
	case RecordRecordDeleteRecordCmd:
		if (DetailsDeleteRecord ())
		{
			recordViewRecordH = 0;      // freed by the last routine
			FrmGotoForm (ListView);
		}
		return true;

	case RecordRecordDuplicateAddressCmd:
		newRecord = ToolsDuplicateCurrentRecord (&numCharsToHilite, false);
		// If we have a new record take the user to be able to edit it
		// automatically.
		if (newRecord != noRecord)
		{
			NumCharsToHilite = numCharsToHilite;
			CurrentRecord = newRecord;
			FrmGotoForm (EditView);
		}
		return true;

	case RecordRecordDialCmd:
		MenuEraseStatus (0);
		DialListShowDialog(CurrentRecord, kDialListShowInListPhoneIndex, 0);
		return true;

	case RecordRecordAttachNoteCmd:
		if (NoteViewCreate())
			FrmGotoForm (NewNoteView);
		// CreateNote may or may not have freed the record.  Compare
		// the record's handle to recordViewRecordH.  If they differ
		// the record is new and recordViewRecordH shouldn't be freed
		// by the frmClose.
		if (recordViewRecordH != DmQueryRecord(AddrDB, CurrentRecord))
			recordViewRecordH = 0;
		return true;

	case RecordRecordDeleteNoteCmd:
		if (recordViewRecord.fields[ad_note] != NULL &&
			FrmAlert(DeleteNoteAlert) == DeleteNoteYes)
		{
			FormType* frmP;
		
			// Free recordViewRecordH because recordViewRecordH() calls AddrDBGetRecord()
			if (recordViewRecordH)
			{
				MemHandleUnlock(recordViewRecordH);
				recordViewRecordH = 0;
			}

			NoteViewDelete ();
			// Deleting the note caused the record to be unlocked
			// Get it again for the record view's usage
			AddrDBGetRecord (AddrDB, CurrentRecord, &recordViewRecord, &recordViewRecordH);

			// Initialize RecordViewLines, RecordViewLastLine... so that the Note won't be drawn
		  	frmP = FrmGetActiveForm();
		  	MemHandleFree(MemPtrRecoverHandle(RecordViewLines));
		  	RecordViewLines = 0;
	  		PrvViewInit(frmP);

			PrvViewUpdate(frmP);
		}
		return true;

	case RecordRecordSelectBusinessCardCmd:
		MenuEraseStatus (0);
		if (FrmAlert(SelectBusinessCardAlert) == SelectBusinessCardYes)
		{
			DmRecordInfo (AddrDB, CurrentRecord, NULL, &BusinessCardRecordID, NULL);
			PrvViewDrawBusinessCardIndicator (FrmGetActiveForm());
		}
		return true;

	case RecordRecordBeamBusinessCardCmd:
		MenuEraseStatus (0);
		ToolsAddrBeamBusinessCard(AddrDB);
		return true;

	case RecordRecordBeamRecordCmd:
		MenuEraseStatus (0);
		TransferSendRecord(AddrDB, CurrentRecord, exgBeamPrefix, NoDataToBeamAlert);
		return true;

	case RecordRecordSendRecordCmd:
		MenuEraseStatus (0);
		TransferSendRecord(AddrDB, CurrentRecord, exgSendPrefix, NoDataToSendAlert);
		return true;

	case RecordOptionsFontCmd:
		MenuEraseStatus (0);
		oldFont = AddrRecordFont;
		AddrRecordFont = ToolsSelectFont (AddrRecordFont);
		if (oldFont != AddrRecordFont)
		{
			PrvViewClose();
			frmP = FrmGetFormPtr(RecordView);
			PrvViewInit(frmP);
		}
		return true;

	case RecordOptionsEditCustomFldsCmd:
		MenuEraseStatus (0);
		FrmPopupForm (CustomEditDialog);
		return true;

	case RecordOptionsAboutCmd:
		MenuEraseStatus (0);
		AbtShowAbout (AddressBookCreator);
		return true;

	}

	return false;
}
