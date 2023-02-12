/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrDialList.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the Address Book application's dial list form module.
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <Chars.h>
#include <ErrorMgr.h>
#include <NotifyMgr.h>
#include <StringMgr.h>
#include <SysUtils.h>
#include <TraceMgr.h>
#include <UIResources.h>
#include <DataMgr.h>
#include <LocaleMgr.h>
#include <Form.h>
#include <Helper.h>
#include <HelperServiceClass.h>
#include <TextMgr.h>

#include <PalmUtils.h>

#include "sec.h"

#include "AddrDialList.h"
#include "Address.h"
#include "AddrTools.h"
#include "AddressRsc.h"
#include "AddressDB.h"
#include "AddrDefines.h"

#define kSpaceBetweenLabelAndNumber 6
#define kLeftAndRightSpace			2
#define	kMaxPhonesCount				25
#define kMaxCharsPhoneNumber		25

// Convenient access
#define gAddrP			(&(gDialListData->addr))
#define gAddrH			(gDialListData->addrH)
#define gAppInfoP		(gDialListData->appInfoP)
#define gDisplayName	(gDialListData->displayName)
#define gPhones			(gDialListData->phones)
#define gPhonesCount	(gDialListData->phonesCount)
#define gPhoneX			(gDialListData->phoneX)
#define gSelectedIndex	(gDialListData->selectedIndex)


/***********************************************************************
 *
 *	Internal types
 *
 ***********************************************************************/

typedef struct DialListPhoneTag
{
	Char*		label;
	Char*		number;
	Int16		numberLen;
} DialListPhoneType;

typedef struct DialListDataTag
{
	// Record
	MemHandle 			addrH;

	// Temp only accurate when drawing
	AddrDBRecordType	addr;
	AddrAppInfoType*	appInfoP;

	// Record description - allocated
	Char*				displayName;

	// Phone position - got from field position so that localization
	// can enable various position
	// Label will be aligned right + delta to that
	Coord				phoneX;
	Coord				phoneY;
	// X min is got from te Description label
	Coord				displayXMin;
	Coord				displayNameY;

	// list info
	Int16				topIndex;
	Int16				selectedIndex;

	// Array of data
	DialListPhoneType	phones[kMaxPhonesCount];
	UInt16				phonesCount;
} DialListDataType;


/***********************************************************************
 *
 *   Global variables
 *
 ***********************************************************************/

static DialListDataType* gDialListData;
static const Char gPhoneChars[] = "0123456789,+#*";


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

// Internal tools
static Boolean	PrvDialListCanBePhoneNumber( Char* text, Int16 textLen ) SEC("code2");
static void		PrvDialListPhoneNumberFilter( Char* outString, Int16* outLen, const Char* inString, Int16 inLen ) SEC("code2");
static Char*	PrvDialListAllocStringFrom( const Char* s1, const Char* s2, const Char* s3, Boolean checkLineFeed ) SEC("code2");
static void		PrvDialListSetFieldHandle( FieldType* fldP, MemHandle handle ) SEC("code2");

// Internal Dial List function
static void		PrvDialListBuildDescription( void ) SEC("code2");
//static Boolean	PrvDialListHandleDescriptionEvent( struct FormGadgetType *gadgetP, UInt16 cmd, void *paramP );
static Boolean	PrvDialListHandleDescriptionEvent( FormGadgetTypeInCallback *gadgetP, UInt16 cmd, void *paramP ) SEC("code2");
static void		PrvDialListInit( FormType* frmP ) SEC("code2");
static void		PrvDialListScroll( WinDirectionType direction ) SEC("code2");
static void		PrvDialListDrawPhoneItem( Int16 index, RectangleType *boundsP, Char **itemsText ) SEC("code2");
static void		PrvDialListUpdateAfterSelection( FormType* frmP ) SEC("code2");
static void		PrvDialListUpdatePhoneNumber( FieldType* fldP ) SEC("code2");
static void		PrvDialListFreeMemory( void ) SEC("code2");
static void		PrvDialListLeaveDialog( void ) SEC("code2");
static Boolean	PrvDialListDialSelected( FormType* frmP ) SEC("code2");


/***********************************************************************
 *
 * FUNCTION:
 *	DialListShowDialog
 *
 * DESCRIPTION:
 *	This routine show the dialog of the given record
 *	if the phoneIndex is not a phone number, first phone number is selected
 *
 * PARAMETERS:
 *	recordIndex		IN		index of the record
 *	phoneIndex		IN		index of the phone, kDialListShowInListPhoneIndex
 *							to use default show in list
 *	lineIndex		IN		index of the line
 *
 * RETURNED:
 *	false if the form must not be displayed
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
Boolean DialListShowDialog( UInt16 recordIndex, UInt16 phoneIndex, UInt16 lineIndex )
{
	Err			err;
	Int16		fieldIndex;

	gDialListData = MemPtrNew(sizeof(DialListDataType));
	if (!gDialListData)
		return false;
	MemSet(gDialListData, sizeof(DialListDataType), 0);

	// TraceInit();
	TraceOutput(TL(appErrorClass, "DialListShowDialog() - recordIndex = %hu, phoneIndex = %hu, lineIndex = %hu", recordIndex, phoneIndex, lineIndex));

	// Get the current record
	err = AddrDBGetRecord(AddrDB, recordIndex, gAddrP, &gAddrH);
	if (err)
		goto exit;

	// Check default phone index
	// If type is not a supported one, continue
	if (phoneIndex == kDialListShowInListPhoneIndex)
		phoneIndex = gAddrP->options.phones.displayPhoneForList;

	gAppInfoP = (AddrAppInfoType*)AddrDBAppInfoGetPtr(AddrDB);

	// Build the phone array
	// Splitting each phone field per line
	gPhonesCount = 0;
	for (fieldIndex = firstPhoneField; fieldIndex <= lastPhoneField; fieldIndex++)
	{
		Char* text = gAddrP->fields[fieldIndex];
		Char* next = text;
		Int16 length;
		Int16 phoneLineIndex = 0;

		if (text && ToolsIsPhoneIndexSupported(gAddrP, fieldIndex - firstPhoneField))
		{
			do
			{
				gPhones[gPhonesCount].number = text;

				// Check if a another line is available
				next = StrChr(text, chrLineFeed);

				// If a line feed is found
				if (next)
					length = next - text;
				else
					length = StrLen(text);

				// Check that the phone is a phone number (ie at least one phone character)
				if (PrvDialListCanBePhoneNumber(text, length))
				{
					Int16 phoneLabelIndex;

					gPhones[gPhonesCount].number = text;
					gPhones[gPhonesCount].numberLen = length;

					phoneLabelIndex = GetPhoneLabel(gAddrP, fieldIndex); // 0 = Work... 7 = Mobile
					// gAppInfoP->fieldLabels stored name of fields in a strange way
					if (phoneLabelIndex < 5) // Work, Home, Fax, Other, Email are stored as phone1... phone5
						phoneLabelIndex += ad_phone1;
					else	// Main, Pager and Mobile are stored after Note.
						phoneLabelIndex += ad_note - 4;
					gPhones[gPhonesCount].label = gAppInfoP->fieldLabels[phoneLabelIndex];

					// Is is the selected one
					if ((phoneIndex == fieldIndex - firstPhoneField) && (phoneLineIndex == lineIndex))
						gSelectedIndex = gPhonesCount;

					gPhonesCount++;
					if (gPhonesCount == kMaxPhonesCount)
						break;
				}
				text = next + 1;
				phoneLineIndex++;
			}
			while (next);
		}
		if (gPhonesCount == kMaxPhonesCount)
			break;
	}

	// Exit if no phone are available for this record
	if (!(gPhonesCount))
		goto exit;

	// Ok so no show the dialog...
	FrmPopupForm(DialListDialog);

	return true;

exit:
	TraceOutput(TL(appErrorClass, "DialListShowDialog() - exit"));
	PrvDialListFreeMemory();
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    DialListHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the "Dial List"
 *              of the Address Book application.
 *
 * PARAMETERS:  evtP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event was handled and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
Boolean DialListHandleEvent( EventType * evtP )
{
	FormType* frmP;
	Boolean handled = false;

	switch (evtP->eType)
	{
	case lstSelectEvent:
		{
			// Update phone type & number
			// Set focus to the field
			gSelectedIndex = evtP->data.lstSelect.selection;
			frmP = FrmGetActiveForm();
			PrvDialListUpdateAfterSelection(frmP);
			handled = true;
			break;
		}
	case ctlSelectEvent:
		{
			switch (evtP->data.ctlSelect.controlID)
			{
			case DialListDialButton:
				if (PrvDialListDialSelected(FrmGetActiveForm()))
					PrvDialListLeaveDialog();
				handled = true;
				break;
			case DialListCancelButton:
				PrvDialListLeaveDialog();
				handled = true;
				break;
			}
			break;
		}
	case frmOpenEvent:
		{
			frmP = FrmGetActiveForm();
			PrvDialListInit(frmP);
			FrmDrawForm(frmP);
			LstSetSelection(ToolsGetFrmObjectPtr(frmP, DialListList), gSelectedIndex);
			handled = true;
			break;
		}
	case frmCloseEvent:
		{
			TraceOutput(TL(appErrorClass, "Closing form"));
			PrvDialListSetFieldHandle(ToolsGetObjectPtr(DialListNumberField), 0);
			PrvDialListFreeMemory();
			// TraceClose();
			break;
		}
	case keyDownEvent:
		{
			if (TxtCharIsHardKey(evtP->data.keyDown.modifiers, evtP->data.keyDown.chr))
			{
				PrvDialListLeaveDialog();
				handled = true;
			}
			else if (EvtKeydownIsVirtual(evtP))
			{
				// up and down scroll bar without updating the selection
				// Scroll up key presed?
				if (evtP->data.keyDown.chr == vchrPageUp)
				{
					PrvDialListScroll(winUp);
				}
				// Scroll down key presed?
				else if (evtP->data.keyDown.chr == vchrPageDown)
				{
					PrvDialListScroll(winDown);
				}
			}

			break;
		}
		default:
			break;
	}


	return (handled);
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListCanBePhoneNumber
 *
 * DESCRIPTION:
 *	This routine check if a text could be a phone number
 *	ie if it contains phone chars
 *
 * PARAMETERS:
 *	text	IN	text string to parse
 *	textLen	IN	text len
 *
 * RETURNED:
 *	true if acceptable
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *	kwk			07/26/00	Modified to use Text Mgr, avoid sign extension
 *							problem calling StrChr with signed Char value.
 *
 ***********************************************************************/
Boolean PrvDialListCanBePhoneNumber( Char* text, Int16 textLen )
{
	UInt16 offset = 0;

	while (offset < textLen)
	{
		WChar curChar;
		offset += TxtGetNextChar(text, offset, &curChar);
		if ( StrChr(gPhoneChars, curChar) != NULL )
			return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListPhoneNumberFilter
 *
 * DESCRIPTION:
 *	This routine filter a phone number
 *
 * PARAMETERS:
 *	outString	OUT	filterd phone number
 *	outLen		IN	max text len for outString
 *			 	OUT	phone number len
 *	inString	IN	text to filter
 *	inLen		IN	text len
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			06/12/00	Initial Revision
 *	fpa			11/11/00	Fixed a coherency problem with SMS: now, +++123+++456 -> + 123 456
 *
 ***********************************************************************/
void PrvDialListPhoneNumberFilter( Char* outString, Int16* outLen, const Char* inString, Int16 inLen )
{
	UInt16 inOffset;
	UInt16 outOffset;
	Boolean fLastWasSpace;

	inOffset = 0;
	outOffset = 0;
	fLastWasSpace = false;

	while ( (inOffset < inLen) && (outOffset < *outLen) )
	{
		WChar curChar;

		inOffset += TxtGetNextChar(inString, inOffset, &curChar);
		if (StrChr(gPhoneChars, curChar))
		{
			// Only + at the beginning
			if ( (curChar == chrPlusSign) && (outOffset > 0) )
			{
				outOffset += TxtSetNextChar(outString, outOffset, chrSpace);
				fLastWasSpace = true;
			}
			else
			{
				outOffset += TxtSetNextChar(outString, outOffset, curChar);
				fLastWasSpace = false;
			}
		}
		else if ( !fLastWasSpace && (outOffset > 0) )	// No space at the beginning
		{
			outOffset += TxtSetNextChar(outString, outOffset, chrSpace);
			fLastWasSpace = true;
		}
	}

	// No space at the end
	if (fLastWasSpace)
		outOffset--;

	TxtSetNextChar(outString, outOffset, chrNull);

	*outLen = outOffset;
}


/***********************************************************************
 *
 * FUNCTION:
 *		PrvDialListAllocStringFrom
 *
 * DESCRIPTION:
 *		This routine build a string from 3 string
 *		It cut after the first lineFeed according to checkLineFeed
 *
 * PARAMETERS:
 *  	s1, s2, s3		IN 	the 3 string that can be null...
 *      checkLineFeed	IN  check lineFeed?
 *
 * RETURNED:
 *		pointer to the string allocated
 *
 * REVISION HISTORY:
 *		Name		Date		Description
 *		----		----		-----------
 *		aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
Char* PrvDialListAllocStringFrom( const Char* s1, const Char* s2, const Char* s3, Boolean checkLineFeed )
{

#define SafeStrLen(string) ( (Int16) ( (string)? StrLen(string) : 0 ) )
#define CopyString(base, string, length) \
	if (length) { MemMove(base, string, length + 1); base += length; }

	Int16 size = 1;
	Int16 length1;
	Int16 length2;
	Int16 length3;
	Char* string;
	Char* tmpStr;

	// Concatenate all non null & non empty string
	// Cut it at first lineFeed

	length1 = SafeStrLen(s1);
	length2 = SafeStrLen(s2);
	length3 = SafeStrLen(s3);

	size = length1 + length2 + length3 + 1;
	if (size == 1)
		return 0;

	string = MemPtrNew(size);
	if (!string)
		return 0;

	tmpStr = string;
	CopyString(tmpStr, s1, length1);
	CopyString(tmpStr, s2, length2);
	CopyString(tmpStr, s3, length3);

	if (checkLineFeed)
	{
		tmpStr = StrChr(string, chrLineFeed);
		if (tmpStr)
		{
			length1 = tmpStr - string;
			string[length1] = chrNull;
			// Shrink so it can't fail
			MemPtrResize(string, length1 + 1);
		}
	}

	return string;
}


/***********************************************************************
 *
 * FUNCTION:
 *	Safe set field handle, previous one is freed if needed
 *
 * DESCRIPTION:
 *	Safe set field handle, previous one is freed if needed
 *
 * PARAMETERS:
 *	fldP	IN	field
 *	handle	IN	handle
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			8/3/00		Initial Revision
 *
 ***********************************************************************/
void	PrvDialListSetFieldHandle( FieldType* fldP, MemHandle handle )
{
	MemHandle oldH;
	oldH = FldGetTextHandle(fldP);
	FldSetTextHandle(fldP, handle);
	if (oldH)
	{
		MemHandleFree(oldH);
	}
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListBuildDescription
 *
 * DESCRIPTION:
 *	This routine build the description
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListBuildDescription( void )
{
	// Initialize description, sorted by default choice
	// - firstName name
	// - name
	// - firstName
	// - Company
	// Then cut at the first line feed
	if (gAddrP->fields[ad_firstName])
	{
		if (gAddrP->fields[ad_name])
		{
			gDisplayName = PrvDialListAllocStringFrom(gAddrP->fields[ad_firstName], " ", gAddrP->fields[ad_name], true);
		}
		else
		{
			// first name only
			gDisplayName = PrvDialListAllocStringFrom(gAddrP->fields[ad_firstName], 0, 0, true);
		}
	}
	else
	{
		if (gAddrP->fields[ad_name])
		{
			// name only
			gDisplayName = PrvDialListAllocStringFrom(gAddrP->fields[ad_name], 0, 0, true);
		}
		else if (gAddrP->fields[ad_company])
		{
			// company only
			gDisplayName = PrvDialListAllocStringFrom(gAddrP->fields[ad_company], 0, 0, true);
		}
	}

	if (!gDisplayName)
	{
		MemHandle unnamedH;
		// - unnamed - (need allocation)
		unnamedH = DmGetResource(strRsc, UnnamedRecordStr);
		gDisplayName = PrvDialListAllocStringFrom(MemHandleLock(unnamedH), 0, 0, true);
		MemHandleUnlock(unnamedH);
	}
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListHandleDescriptionEvent
 *
 * DESCRIPTION:
 *	This routine handle gadget event for descrption (mainly drawing)
 *
 * PARAMETERS
 *	gadgetP	IN  gadget pointer
 *	cmd		IN	command
 *	paramP	IN	param (unused)
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	aro		08/02/00	Initial Revision
 *
 ***********************************************************************/
//Boolean PrvDialListHandleDescriptionEvent( struct FormGadgetType *gadgetP, UInt16 cmd, void *paramP )
Boolean PrvDialListHandleDescriptionEvent( FormGadgetTypeInCallback *gadgetP, UInt16 cmd, void *paramP )
{
//#pragma unused(paramP)

	Boolean handled = false;

	switch (cmd)
	{
	case formGadgetDeleteCmd:
		// Free the display Name
		break;
	case formGadgetDrawCmd:
		{
			FontID fontId;

			// The displayName is left-aligned and truncated to fit in the gadget
			#if WRISTPDA
			fontId = FntSetFont(FossilBoldFont);
			#else
			fontId = FntSetFont(largeBoldFont);
			#endif
			WinDrawTruncChars(gDisplayName, StrLen(gDisplayName), gadgetP->rect.topLeft.x, gadgetP->rect.topLeft.y,
							  gadgetP->rect.extent.x);
			FntSetFont(fontId);
			handled = true;
			break;
		}
	}
	return handled;
}


/***********************************************************************
 *
 * FUNCTION:    PrvDialListInit
 *
 * DESCRIPTION: This routine initializes the "Dial List" dialog of the
 *              Address application.
 *
 * PARAMETERS:  frmP  - a pointer to the Form
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListInit( FormType* frmP )
{
	ListType*	lstP;
	UInt16		fldIndex;
	FieldType*	fldP;
	//const Char*	description = "";
	Coord		dummy;
	Int16		middle;
	Int16		visibleItems;
	Int16		topIndex;

	// Build the description for drawing in the form
	PrvDialListBuildDescription();
	FrmSetGadgetHandler(frmP, FrmGetObjectIndex(frmP, DialListDescriptionGadget), PrvDialListHandleDescriptionEvent);

	// Get the gadget phone rectangle position, since phone in the list will be aligned to that
	FrmGetObjectPosition(frmP, FrmGetObjectIndex(frmP, DialListPhoneRectangleGadget),
						 &gPhoneX, &dummy);

	// Initialize the address list.
	lstP = ToolsGetFrmObjectPtr(frmP, DialListList);
	LstSetListChoices(lstP, 0, gPhonesCount);
	LstSetDrawFunction(lstP, PrvDialListDrawPhoneItem);

	// Set the top item to avoid flickering
	// Try to have the selected one in the middle
	visibleItems = LstGetVisibleItems(lstP);
	middle = ((visibleItems - 1) / 2);
	if ((gPhonesCount <= visibleItems) || (gSelectedIndex <= middle))
	{
		// top aligned
		topIndex = 0;
	}
	else if (gSelectedIndex >= (gPhonesCount - (visibleItems - middle)))
	{
		// bottom aligned
		topIndex = gPhonesCount - visibleItems;
	}
	else
	{
		// centered
		topIndex = gSelectedIndex - middle;
	}
	LstSetTopItem(lstP, topIndex);

	// initiate phone number field
	fldIndex = FrmGetObjectIndex(frmP, DialListNumberField);
	fldP = FrmGetObjectPtr(frmP, fldIndex);
	FldSetMaxChars(fldP, kMaxCharsPhoneNumber);
	PrvDialListUpdatePhoneNumber(fldP);
	FrmSetFocus(frmP, fldIndex);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListScroll
 *
 * DESCRIPTION:
 *	This routine scroll the list up or down (page per page)
 *
 * PARAMETERS
 *	direction	IN	direction to scroll (winUp or winDown)
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name	Date		Description
 *	----	----		-----------
 *	aro		06/19/00	Initial Revision
 *
 ***********************************************************************/
void PrvDialListScroll( WinDirectionType direction )
{
	ListType* lstP = ToolsGetFrmObjectPtr(FrmGetActiveForm(), DialListList);
	Int16 count = LstGetVisibleItems(lstP);
	LstScrollList(lstP, direction, count - 1);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListDrawPhoneItem
 *
 * DESCRIPTION:
 *	This routine draws a phone item line (label & number)
 *	It is called as a callback routine by the list object.
 *
 * PARAMETERS:
 *	itenNum		IN	index of the item to draw
 *	boundsP		IN	boundsP of rectangle to draw in
 *	itemsText	IN	data if any
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListDrawPhoneItem( Int16 index, RectangleType *boundsP, Char **itemsText )
{
//#pragma unused(itemsText)

	Char* number;
	Char* label;
	Int16 numberLen;
	Int16 labelLen;
	Int16 dummyLen;
	Coord labelMaxWidth;
	Coord dummyWidth;
	Boolean	fit;

	// retrieve the name and the label
	number = gPhones[index].number;
	numberLen = gPhones[index].numberLen;
	label = gPhones[index].label;
	labelLen = StrLen(label);
	dummyLen = labelLen;

	// Draw the label on the left (truncated if needed) + ":")
	labelMaxWidth = gPhoneX - boundsP->topLeft.x - kSpaceBetweenLabelAndNumber;
	dummyWidth = labelMaxWidth;
	FntCharsInWidth(label, &labelMaxWidth, &dummyLen, &fit);
	WinDrawTruncChars(label, labelLen, boundsP->topLeft.x, boundsP->topLeft.y, dummyWidth);
	WinDrawChars(":", 1, boundsP->topLeft.x + labelMaxWidth + 1, boundsP->topLeft.y);

	WinDrawTruncChars(number, numberLen, gPhoneX, boundsP->topLeft.y,
					  boundsP->extent.x + boundsP->topLeft.x - gPhoneX);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListUpdateAfterSelection
 *
 * DESCRIPTION:
 *	This routine update the number
 *	according to the new or current selection
 *	Set focus to the field
 *
 * PARAMETERS:
 *	frmP	IN	form
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListUpdateAfterSelection( FormType* frmP )
{
	FieldType* 	fldP;
	UInt16		fieldIndex;

	// Set the number in the field
	// Number is parse according to characters allowed
	fieldIndex = FrmGetObjectIndex(frmP, DialListNumberField);
	fldP = FrmGetObjectPtr(frmP, fieldIndex);
	PrvDialListUpdatePhoneNumber(fldP);
	FldDrawField(fldP);
	FrmSetFocus(frmP, fieldIndex);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListUpdatePhoneNumber
 *
 * DESCRIPTION:
 *	This routine update the number
 *	in the field according to current selection
 *	No drawn is made
 *
 * PARAMETERS:
 *	fldP	IN	field
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListUpdatePhoneNumber( FieldType* fldP )
{
	MemHandle	numberH;
	Char*		numberP;
	Int16		len;

	len = (Int16)FldGetMaxChars(fldP);

	len = min(len, gPhones[gSelectedIndex].numberLen);

	numberH = MemHandleNew(len + 1);
	if (!numberH)
		return;

	numberP = MemHandleLock(numberH);
	PrvDialListPhoneNumberFilter(numberP, &len, gPhones[gSelectedIndex].number, gPhones[gSelectedIndex].numberLen);
	numberP[len] = chrNull;
	MemHandleUnlock(numberH);
	MemHandleResize(numberH, len + 1);

	PrvDialListSetFieldHandle(fldP, numberH);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListFreeMemory
 *
 * DESCRIPTION:
 *	This routine frees memory allocated by the dialog
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListFreeMemory( void )
{
	TraceOutput(TL(appErrorClass, "PrvDialListFreeMemory()"));

	if (gAppInfoP)
		MemPtrUnlock(gAppInfoP);
	if (gAddrH)
		MemHandleUnlock(gAddrH);

	if (gDisplayName)
		MemPtrFree(gDisplayName);
		
	if (gDialListData)
		MemPtrFree(gDialListData);

	gDialListData = 0;
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListLeaveDialog
 *
 * DESCRIPTION:
 *	This routine leave the dialog
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
void PrvDialListLeaveDialog( void )
{
	TraceOutput(TL(appErrorClass, "PrvDialListLeaveDialog"));

	PrvDialListSetFieldHandle(ToolsGetObjectPtr(DialListNumberField), 0);
	PrvDialListFreeMemory();

	FrmReturnToForm(0);
	FrmUpdateForm(0, updateSelectCurrentRecord);
}


/***********************************************************************
 *
 * FUNCTION:
 *	PrvDialListDialSelected
 *
 * DESCRIPTION:
 *	This routine dial selected number
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *	nothing
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/
Boolean PrvDialListDialSelected( FormType* frmP )
{
	SysNotifyParamType param;
	HelperNotifyEventType details;
	HelperNotifyExecuteType execute;

	param.notifyType = sysNotifyHelperEvent;
	param.broadcaster = AddressBookCreator;
	param.notifyDetailsP = &details;
	param.handled = false;

	details.version = kHelperNotifyCurrentVersion;
	details.actionCode = kHelperNotifyActionCodeExecute;
	details.data.executeP = &execute;

	execute.serviceClassID = kHelperServiceClassIDVoiceDial;
	execute.helperAppID = 0;
	execute.dataP = FldGetTextPtr(ToolsGetFrmObjectPtr(frmP, DialListNumberField));
	execute.displayedName = gDisplayName;
	execute.detailsP = 0;
	execute.err = errNone;

	SysNotifyBroadcast(&param);

	// Check error code
	if (!param.handled)
		// Not handled so exit the list - Unexepcted error
		return true;
	else
	{
		TraceOutput(TL(appErrorClass, "Dial result: %hx", execute.err));
		return (execute.err == errNone);
	}
}
