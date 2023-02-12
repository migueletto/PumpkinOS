/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrTools.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *  This is the place for all misc functions
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <Form.h>
#include <StringMgr.h>
#include <ErrorMgr.h>
#include <NotifyMgr.h>
#include <TextMgr.h>
#include <FontSelect.h>
#include <KeyMgr.h>
#include <TimeMgr.h>
#include <Helper.h>
#include <HelperServiceClass.h>
#include <TraceMgr.h>

#include <PalmUtils.h>

#include "sec.h"

#include "AddrCustom.h"
#include "AddressDB.h"
#include "AddressRsc.h"
#include "AddrTools.h"
#include "Address.h"
#include "AddressTransfer.h"
#include "AddrDefines.h"
#include "UIResources.h"
#include "Address.h"

/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Max length for first name field
#define maxNameLength			255

#define maxDuplicatedIndString	20

// Maximum label column width in Edit and Record views.
//  (Would be nice if this was based on window size or screen size, do 1/2 screen for now)
#define maxLabelColumnWidth		80

#define maxPhoneColumnWidth		82 // (415)-000-0000x...


/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

Boolean	PrvToolsPhoneIsANumber( Char* phone );


/***********************************************************************
 *
 * FUNCTION:
 *	ToolsIsDialerPresent
 *
 * DESCRIPTION:
 *	This routine check if a dialer is present
 *	Once check has been made, a global stores this info so that further
 *	callss are immediate
 *
 * PARAMETERS:
 *	none
 *
 * RETURNED:
 *  true if a dialer is present
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/12/00		Initial Revision
 *
 ***********************************************************************/

Boolean	ToolsIsDialerPresent( void )
{
#if WRISTPDA
	return false;
#else
	if (!DialerPresentChecked)
	{
		SysNotifyParamType param;
		HelperNotifyEventType details;
		HelperNotifyValidateType validate;

		param.notifyType = sysNotifyHelperEvent;
		param.broadcaster = AddressBookCreator;
		param.notifyDetailsP = &details;
		param.handled = false;

		details.version = kHelperNotifyCurrentVersion;
		details.actionCode = kHelperNotifyActionCodeValidate;
		details.data.validateP = &validate;

		validate.serviceClassID = kHelperServiceClassIDVoiceDial;
		validate.helperAppID = 0;

		SysNotifyBroadcast(&param);
		if (param.handled)
			DialerPresent = true;
		else
			DialerPresent = false;

		DialerPresentChecked = true;
	}
	return DialerPresent;
#endif
}


/***********************************************************************
 *
 * FUNCTION:     ToolsSetDBAttrBits
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
 *			bhall	7/9/99	made non-static for access in AddressAutoFill.c
 *
 ***********************************************************************/
void ToolsSetDBAttrBits(DmOpenRef dbP, UInt16 attrBits)
{
	DmOpenRef localDBP;
	LocalID dbID;
	UInt16 cardNo;
	UInt16 attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
	{
		localDBP = DmOpenDatabaseByTypeCreator (addrDBType, AddressBookCreator, dmModeReadWrite);
		if (localDBP == NULL)  return;
	}
	else
	{
		localDBP = dbP;
	}

	// now set the backup bit on localDBP
	DmOpenDatabaseInfo(localDBP, &dbID, NULL, NULL, &cardNo, NULL);
	DmDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				   NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	attributes |= attrBits;
	DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
					  NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	// close database if necessary
	if (dbP == NULL)
	{
		DmCloseDatabase(localDBP);
	}
}

/***********************************************************************
 *
 * FUNCTION:     ToolsCreateDefaultDatabase
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
Err ToolsCreateDefaultDatabase(void)
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

		// Set the backup bit on the new database.
		if (!error)
			ToolsSetDBAttrBits(NULL, dmHdrAttrBackup);
		
		MemHandleUnlock(resH);
		DmReleaseResource(resH);
	}

	// If there is no default data, or we had a problem creating it,
	// then attempt to create an empty database.
	if (!resH || error)
	{
		error = AddrDBGetDatabase (&dbP, dmModeReadWrite);

		if (!error)
			DmCloseDatabase(dbP);
	}

	return error;
}



/***********************************************************************
 *
 * FUNCTION:     ToolsRegisterLocaleChangingNotification

 *
 * DESCRIPTION:  Register for NotifyMgr notifications for locale chagning.
 *				DOLATER : This function and the one above can be rolled into one.
 *
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
void ToolsRegisterLocaleChangingNotification(void)
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
 * FUNCTION:    ToolsDetermineRecordName
 *
 * DESCRIPTION: Determines an address book record's name.  The name
 * varies based on which fields exist and what the sort order is.
 *
 * PARAMETERS:  name1, name2 - first and seconds names to draw
 *              name1Length, name2Length - length of the names in chars
 *              name1Width, name2Width - width of the names when drawn
 *              nameExtent - the space the names must be drawn in
 *              *x, y - where the names are drawn
 *              shortenedFieldWidth - the width in the current font
 *
 *
 * RETURNED:    x is set after the last char drawn
 *					 Boolean - name1/name2 priority based on sortByCompany
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			roger		06/20/95	Initial Revision
 *			frigino		08/13/97	Added priority return value
 *			fpa			11/02/00	Added unnamedRecordStringH parameter in order to prevent memory leaks
 *
 ***********************************************************************/
Boolean ToolsDetermineRecordName (AddrDBRecordPtr recordP, Int16 *shortenedFieldWidth, Int16 *fieldSeparatorWidth, Boolean sortByCompany, Char **name1, Int16 *name1Length, Int16 *name1Width, Char **name2, Int16 *name2Length, Int16 *name2Width, Char **unnamedRecordStringPtr, MemHandle* unnamedRecordStringH, Int16 nameExtent)
{
	UInt16 fieldNameChoiceList[4];
	UInt16 fieldNameChoice;
	Boolean ignored;
	Boolean name1HasPriority;

	*shortenedFieldWidth = (FntCharWidth('.') * shortenedFieldLength);
	*fieldSeparatorWidth = FntCharsWidth (fieldSeparatorString, fieldSeparatorLength);
	*name1 = NULL;
	*name2 = NULL;
	
	if ( unnamedRecordStringH != NULL )
		*unnamedRecordStringH = NULL;

	if (sortByCompany)
	{
		// When sorting by company, always treat name2 as priority.
		name1HasPriority = false;

		fieldNameChoiceList[3] = ad_addressFieldsCount;
		fieldNameChoiceList[2] = ad_firstName;
		fieldNameChoiceList[1] = ad_name;
		fieldNameChoiceList[0] = ad_company;
		fieldNameChoice = 0;

		while (*name1 == NULL &&
			   fieldNameChoiceList[fieldNameChoice] != ad_addressFieldsCount)
		{
			*name1 = recordP->fields[fieldNameChoiceList[fieldNameChoice++]];
		}

		// When sorting by company, treat name2 as priority if we
		// succeed in getting the company name as the name1
		// Did we get the company name?
		if (fieldNameChoice > 1) {
			// No. We got a last name, first name, or nothing. Priority switches to name1
			name1HasPriority = true;
		}

		while (*name2 == NULL &&
			   fieldNameChoiceList[fieldNameChoice] != ad_addressFieldsCount)
		{
			*name2 = recordP->fields[fieldNameChoiceList[fieldNameChoice++]];
		}

	}
	else
	{
		// When not sorting by company, always treat name1 as priority.
		name1HasPriority = true;

		fieldNameChoiceList[3] = ad_addressFieldsCount;
		fieldNameChoiceList[2] = ad_addressFieldsCount;
		fieldNameChoiceList[1] = ad_firstName;
		fieldNameChoiceList[0] = ad_name;
		fieldNameChoice = 0;

		while (*name1 == NULL &&
			   fieldNameChoiceList[fieldNameChoice] != ad_addressFieldsCount)
		{
			*name1 = recordP->fields[fieldNameChoiceList[fieldNameChoice++]];
		}

		if (*name1 == NULL)
		{
			*name1 = recordP->fields[ad_company];
			*name2 = NULL;
		}
		else
		{
			while (*name2 == NULL &&
				   fieldNameChoiceList[fieldNameChoice] != ad_addressFieldsCount)
			{
				*name2 = recordP->fields[fieldNameChoiceList[fieldNameChoice++]];
			}
		}
	}

	if (*name1)
	{
		// Only show text from the first line in the field
		*name1Length = nameExtent;            // longer than possible
		*name1Width = nameExtent;            // wider than possible
		FntCharsInWidth (*name1, name1Width, name1Length, &ignored); //lint !e64
	}
	else
	{
		// Set the name to the unnamed string
		if (*unnamedRecordStringPtr == NULL)
		{
			*unnamedRecordStringH = DmGetResource(strRsc, UnnamedRecordStr);
			*unnamedRecordStringPtr = MemHandleLock(*unnamedRecordStringH);
		}

		// The unnamed string is assumed to be well chosen to not need clipping.
		*name1 = *unnamedRecordStringPtr;
		*name1Length = StrLen(*unnamedRecordStringPtr);
		*name1Width = FntCharsWidth (*unnamedRecordStringPtr, *name1Length);
	}

	if (*name2)
	{
		// Only show text from the first line in the field
		*name2Length = nameExtent;            // longer than possible
		*name2Width = nameExtent;            // wider than possible
		FntCharsInWidth (*name2, name2Width, name2Length, &ignored);//lint !e64
	}
	else
	{
		*name2Length = 0;
		*name2Width = 0;
	}

	// Return priority status
	return name1HasPriority;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsDrawRecordName
 *
 * DESCRIPTION: Draws an address book record name.  It is used
 * for the list view and note view.
 *
 * PARAMETERS:  name1, name2 - first and seconds names to draw
 *              name1Length, name2Length - length of the names in chars
 *              name1Width, name2Width - width of the names when drawn
 *              nameExtent - the space the names must be drawn in
 *              *x, y - where the names are drawn
 *              shortenedFieldWidth - the width in the current font
 *
 *
 * RETURNED:    x is set after the last char drawn
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			roger		6/20/95	Initial Revision
 *			frigino	970813	Rewritten. Now includes a variable ratio for
 *									name1/name2 width allocation, a prioritization
 *									parameter, and a word break search to allow
 *									reclaiming of space from the low priority name.
 *
 ***********************************************************************/
void ToolsDrawRecordName (Char * name1, Int16 name1Length, Int16 name1Width, Char * name2, Int16 name2Length, Int16 name2Width, Int16 nameExtent, Int16 *x, Int16 y, Int16 shortenedFieldWidth, Int16 fieldSeparatorWidth, Boolean center, Boolean priorityIsName1, Boolean inTitle)
{
	Int16		name1MaxWidth;
	Int16		name2MaxWidth;
	Boolean	ignored;
	Int16		totalWidth;
	//	Char *	highPriName;
	Char *	lowPriName;
	Int16		highPriNameWidth;
	Int16		lowPriNameWidth;


	// Check if both names fit
	totalWidth = name1Width + (name2 ? fieldSeparatorWidth : 0) + name2Width;

	// If we are supposed to center the names then move in the x position
	// by the amount that centers the text
	if (center && (nameExtent > totalWidth))
	{
		*x += (nameExtent - totalWidth) / 2;
	}

	// Special case if only name1 is given
	if (name2 == NULL)
	{
		// Draw name portion that fits in extent
		FntCharsInWidth(name1, (Int16*)&nameExtent, &name1Length, &ignored);
		if (inTitle)
			WinDrawInvertedChars(name1, name1Length, *x, y);
		else
			WinDrawChars(name1, name1Length, *x, y);
		// Add width of characters actually drawn
		*x += FntCharsWidth(name1, name1Length);
		return;
	}

	// Remove name separator width
	nameExtent -= fieldSeparatorWidth;

	// Test if both names fit
	if ((name1Width + name2Width) <= nameExtent)
	{
		name1MaxWidth = name1Width;
		name2MaxWidth = name2Width;
	}
	else
	{
		// They dont fit. One or both needs truncation
		// Establish name priorities and their allowed widths
		// Change this to alter the ratio of the low and high priority name spaces
		Int16	highPriMaxWidth = (nameExtent * 2) / 3;	// 1/3 to low and 2/3 to high
		Int16	lowPriMaxWidth = nameExtent - highPriMaxWidth;

		// Save working copies of names and widths based on priority
		if (priorityIsName1)
		{
			// Priority is name1
			//			highPriName = name1;
			highPriNameWidth = name1Width;
			lowPriName = name2;
			lowPriNameWidth = name2Width;
		}
		else
		{
			// Priority is name2
			//			highPriName = name2;
			highPriNameWidth = name2Width;
			lowPriName = name1;
			lowPriNameWidth = name1Width;
		}

		// Does high priority name fit in high priority max width?
		if (highPriNameWidth > highPriMaxWidth)
		{
			// No. Look for word break in low priority name
			Char * spaceP = StrChr(lowPriName, spaceChr);
			if (spaceP != NULL)
			{
				// Found break. Set low priority name width to break width
				lowPriNameWidth = FntCharsWidth(lowPriName, spaceP - lowPriName);
				// Reclaim width from low pri name width to low pri max width, if smaller
				if (lowPriNameWidth < lowPriMaxWidth)
				{
					lowPriMaxWidth = lowPriNameWidth;
					// Set new high pri max width
					highPriMaxWidth = nameExtent - lowPriMaxWidth;
				}
			}
		}
		else
		{
			// Yes. Adjust maximum widths
			highPriMaxWidth = highPriNameWidth;
			lowPriMaxWidth = nameExtent - highPriMaxWidth;
		}

		// Convert priority widths back to name widths
		if (priorityIsName1)
		{
			// Priority is name1
			name1Width = highPriNameWidth;
			name2Width = lowPriNameWidth;
			name1MaxWidth = highPriMaxWidth;
			name2MaxWidth = lowPriMaxWidth;
		}
		else
		{
			// Priority is name2
			name1Width = lowPriNameWidth;
			name2Width = highPriNameWidth;
			name1MaxWidth = lowPriMaxWidth;
			name2MaxWidth = highPriMaxWidth;
		}
	}

	// Does name1 fit in its maximum width?
	if (name1Width > name1MaxWidth)
	{
		// No. Draw it to max width minus the ellipsis
		name1Width = name1MaxWidth - shortenedFieldWidth;
		FntCharsInWidth(name1, &name1Width, &name1Length, &ignored);
		if (inTitle)
			WinDrawInvertedChars(name1, name1Length, *x, y);
		else
			WinDrawChars(name1, name1Length, *x, y);
		*x += name1Width;

		// Draw ellipsis
		if (inTitle)
			WinDrawInvertedChars(shortenedFieldString, shortenedFieldLength, *x, y);
		else
			WinDrawChars(shortenedFieldString, shortenedFieldLength, *x, y);
		*x += shortenedFieldWidth;
	}
	else
	{
		// Yes. Draw name1 within its width
		FntCharsInWidth(name1, &name1Width, &name1Length, &ignored);
		if (inTitle)
			WinDrawInvertedChars(name1, name1Length, *x, y);
		else
			WinDrawChars(name1, name1Length, *x, y);
		*x += name1Width;
	}

	// Draw name separator
	if (inTitle)
		WinDrawInvertedChars(fieldSeparatorString, fieldSeparatorLength, *x, y);
	else
		WinDrawChars(fieldSeparatorString, fieldSeparatorLength, *x, y);
	*x += fieldSeparatorWidth;

	// Draw name2 within its maximum width
	FntCharsInWidth(name2, &name2MaxWidth, &name2Length, &ignored);
	if (inTitle)
		WinDrawInvertedChars(name2, name2Length, *x, y);
	else
		WinDrawChars(name2, name2Length, *x, y);
	*x += name2MaxWidth;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsDrawRecordNameAndPhoneNumber
 *
 * DESCRIPTION: Draws the name and phone number (plus which phone)
 *					 within the screen bounds passed.
 *
 * PARAMETERS:  record - record to draw
 *              bounds - bounds of the draw region
 *              phoneLabelLetters - the first letter of each phone label
 *              sortByCompany - true if the database is sorted by company
 *              unnamedRecordStringPtr - string to use for unnamed records
 *
 * RETURNED:    x coordinate where phone number starts
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			roger	6/21/95		Initial Revision
 *			peter	5/09/00		Added result and eliminated destructive change to bounds
 *			fpa		11/02/00	Added unnamedRecordStringH parameter in order to prevent memory leaks
 *
 ***********************************************************************/
Int16 ToolsDrawRecordNameAndPhoneNumber(AddrDBRecordPtr record, RectanglePtr bounds, Char * phoneLabelLetters, Boolean sortByCompany, Char **unnamedRecordStringPtr, MemHandle* unnamedRecordStringH)
{
	Int16 x, y, phoneX, widthWithoutPhoneLabel;
	UInt16 phoneLabel;
	Int16 fieldSeparatorWidth;
	Int16 shortenedFieldWidth;
	Char * name1;
	Char * name2;
	Char * phone;
	Int16 name1Length;
	Int16 name2Length;
	Int16 phoneLength;
	Int16 name1Width;
	Int16 name2Width;
	Int16 phoneWidth;
	UInt16 nameExtent;
	Boolean ignored;
	Boolean name1HasPriority;
	UInt8 phoneLabelWidth;
	const Int16 phoneColumnWidth = maxPhoneColumnWidth;


	x = bounds->topLeft.x;
	y = bounds->topLeft.y;

	phoneLabelWidth = FntCharWidth('W') - 1;		// remove the blank trailing column
	widthWithoutPhoneLabel = bounds->extent.x - (phoneLabelWidth + 1);

	name1HasPriority = ToolsDetermineRecordName(record, &shortenedFieldWidth, &fieldSeparatorWidth, sortByCompany, &name1, &name1Length, &name1Width, &name2, &name2Length, &name2Width, unnamedRecordStringPtr, unnamedRecordStringH, widthWithoutPhoneLabel);


	phone = record->fields[ad_phone1 + record->options.phones.displayPhoneForList];
	if (phone)
	{
		// Only show text from the first line in the field
		phoneWidth = widthWithoutPhoneLabel;
		phoneLength = phoneWidth;         // more characters than we can expect
		FntCharsInWidth (phone, &phoneWidth, &phoneLength, &ignored);
	}
	else
	{
		phoneLength = 0;
		phoneWidth = 0;
	}
	phoneX = bounds->topLeft.x + widthWithoutPhoneLabel - phoneWidth;

	if (widthWithoutPhoneLabel >= name1Width + (name2 ? fieldSeparatorWidth : 0) +
		name2Width + (phone ? spaceBetweenNamesAndPhoneNumbers : 0) + phoneWidth)
	{
		// we can draw it all!
		WinDrawChars(name1, name1Length, x, y);
		x += name1Width;

		// Is there a second name?
		if (name2)
		{
			if (name1)
			{
				WinDrawChars(fieldSeparatorString, fieldSeparatorLength, x, y);
				x += fieldSeparatorWidth;
			}

			// draw name2
			WinDrawChars(name2, name2Length, x, y);
			x += name2Width;
		}

		if (phone)
			WinDrawChars(phone, phoneLength, phoneX, y);
	}
	else
	{
		// Shortened math (970812 maf)
		nameExtent = widthWithoutPhoneLabel - min(phoneWidth, phoneColumnWidth);

		// Leave some space between names and numbers if there is a phone number
		if (phone)
			nameExtent -= spaceBetweenNamesAndPhoneNumbers;

		ToolsDrawRecordName (name1, name1Length, name1Width, name2, name2Length, name2Width,
						nameExtent, &x, y, shortenedFieldWidth, fieldSeparatorWidth, false,
						name1HasPriority || !sortByCompany, false);

		if (phone)
		{
			x += spaceBetweenNamesAndPhoneNumbers;
			nameExtent = x - bounds->topLeft.x;

			// Now draw the phone number
			if (widthWithoutPhoneLabel - nameExtent >= phoneWidth)
				// We can draw it all
				WinDrawChars(phone, phoneLength, phoneX, y);
			else
			{
				// The phone number should be right justified instead of using
				// x from above because the string printed may be shorter
				// than we expect (CharsInWidth chops off space chars).
				phoneWidth = widthWithoutPhoneLabel - nameExtent - shortenedFieldWidth;
				FntCharsInWidth(phone, &phoneWidth, &phoneLength, &ignored);
				phoneX = bounds->topLeft.x + widthWithoutPhoneLabel - shortenedFieldWidth - phoneWidth;
				WinDrawChars(phone, phoneLength, phoneX, y);

				WinDrawChars(shortenedFieldString, shortenedFieldLength,
							 bounds->topLeft.x + widthWithoutPhoneLabel - shortenedFieldWidth, y);
			}
		}
	}


	if (phone)
	{
		// Draw the first letter of the phone field label
		phoneLabel = GetPhoneLabel(record, firstPhoneField +
								   record->options.phones.displayPhoneForList);

		// find out if the first letter of the phone label is an O(ther) or
		// E(mail). If it is email don't draw the letter. If it is other, and the
		// contents of the phone field is not a number, don't draw the letter.
		if ( phoneLabel != emailLabel )
		{
			if ( (phoneLabel != otherLabel) || PrvToolsPhoneIsANumber (phone) )
			{
				WinDrawChars (&phoneLabelLetters[phoneLabel], 1,
							  bounds->topLeft.x + widthWithoutPhoneLabel + 1 + ((phoneLabelWidth -
																				 (FntCharWidth(phoneLabelLetters[phoneLabel]) - 1)) >> 1), y);//lint !e702
			}
		}
	}

	return phoneX;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsGetLabelColumnWidth
 *
 * DESCRIPTION: Calculate the width of the widest field label plus a ':'.
 *
 * PARAMETERS:  appInfoPtr  - pointer to the app info block for field labels
 *              labelFontID - font
 *
 * RETURNED:    width of the widest label.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   1/30/98   Initial Revision
 *
 ***********************************************************************/
UInt16 ToolsGetLabelColumnWidth (AddrAppInfoPtr appInfoPtr, FontID labelFontID)
{
	Int16		i;
	UInt16	labelWidth;     // Width of a field label
	UInt16	columnWidth;    // Width of the label column (fits all label)
	FontID	curFont;
	Char *	label;


	// Calculate column width of the label column which is used by the Record View and the
	// Edit View.
	curFont = FntSetFont (labelFontID);

	columnWidth = 0;

	for (i = firstAddressField; i < lastLabel; i ++)
	{
		label = appInfoPtr->fieldLabels[i];
		labelWidth = FntCharsWidth(label, StrLen(label));
		columnWidth = max(columnWidth, labelWidth);
	}
	columnWidth += 1 + FntCharWidth(':');

	FntSetFont (curFont);

	if (columnWidth > maxLabelColumnWidth)
		columnWidth = maxLabelColumnWidth;

	return columnWidth;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsLeaveForm
 *
 * DESCRIPTION: Leaves the current popup form and returns to the prior one.
 *
 * PARAMETERS:  formID  - resource id of form to return to
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   6/30/95   Initial Revision
 *
 ***********************************************************************/
void ToolsLeaveForm  ()
{
	FormPtr frm;

	frm = FrmGetActiveForm();
	FrmEraseForm (frm);
	FrmDeleteForm (frm);
	FrmSetActiveForm (FrmGetFirstForm ());
}



/***********************************************************************
 *
 * FUNCTION:    ToolsChangeCategory
 *
 * DESCRIPTION: This routine updates the global varibles that keep track
 *              of category information.
 *
 * PARAMETERS:  category  - new category (index)
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
void ToolsChangeCategory (UInt16 category)
{
	CurrentCategory = category;
	TopVisibleRecord = 0;
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
Boolean ToolsSeekRecord (UInt16 * indexP, Int16 offset, Int16 direction)
{
	DmSeekRecordInCategory (AddrDB, indexP, offset, direction, CurrentCategory);
	if (DmGetLastErr()) return (false);

	return (true);
}


/***********************************************************************
 *
 * FUNCTION:    ToolsDirtyRecord
 *
 * DESCRIPTION: Mark a record dirty (modified).  Record marked dirty
 *              will be synchronized.
 *
 * PARAMETERS:
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/
void ToolsDirtyRecord (UInt16 index)
{
	UInt16      attr;

	DmRecordInfo (AddrDB, index, &attr, NULL, NULL);
	attr |= dmRecAttrDirty;
	DmSetRecordInfo (AddrDB, index, &attr, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    ToolsSelectFont
 *
 * DESCRIPTION: This routine handles selection of a font
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
FontID ToolsSelectFont (FontID currFontID)
{
	UInt16 formID;
	FontID fontID;

	formID = (FrmGetFormId (FrmGetActiveForm ()));

	// Call the OS font selector to get the id of a font.
	fontID = FontSelect (currFontID);

	#if WRISTPDA
	fontID = FossilLargeFontID( WRISTPDA, fontID );
	#endif

	if (fontID != currFontID)
		FrmUpdateForm (formID, updateFontChanged);

	return (fontID);
}


/***********************************************************************
 *
 * FUNCTION:    ToolsDeleteRecord
 *
 * DESCRIPTION: Deletes an address record.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger	6/13/95	Initial Revision
 *			  grant	6/21/99	Unlock the record if it is being archived.
 *         jwm		1999-10-8 Swap forward/backward so users can work
 *									through deleting records from top to bottom.
 *
 ***********************************************************************/
void ToolsDeleteRecord (Boolean archive)
{
	// Show the following record.  Users want to see where the record was and
	// they also want to return to the same location in the database because
	// they might be working their way through the records.  If there isn't
	// a following record show the prior record.  If there isn't a prior
	// record then don't show a record.
	ListViewSelectThisRecord = CurrentRecord;
	if (!ToolsSeekRecord(&ListViewSelectThisRecord, 1, dmSeekForward))
		if (!ToolsSeekRecord(&ListViewSelectThisRecord, 1, dmSeekBackward))
			ListViewSelectThisRecord = noRecord;

	// Delete or archive the record.
	if (archive)
	{
		DmArchiveRecord (AddrDB, CurrentRecord);
	}
	else
		DmDeleteRecord (AddrDB, CurrentRecord);

	// Deleted records are stored at the end of the database
	DmMoveRecord (AddrDB, CurrentRecord, DmNumRecords (AddrDB));

	// Since we just moved the CurrentRecord to the end the
	// ListViewSelectThisRecord may have been moved up one position.
	if (ListViewSelectThisRecord >= CurrentRecord &&
		ListViewSelectThisRecord != noRecord)
		ListViewSelectThisRecord--;


	// Use whatever record we found to select.
	CurrentRecord = ListViewSelectThisRecord;
}


/***********************************************************************
 *
 * FUNCTION:    ToolsCustomAcceptBeamDialog
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
Err ToolsCustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP)
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
 * FUNCTION:    ToolsGetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         art   6/5/95      Initial Revision
 *
 ***********************************************************************/

void* ToolsGetObjectPtr  (UInt16 objectID)
{
	return ToolsGetFrmObjectPtr(FrmGetActiveForm(), objectID);
}

/***********************************************************************
 *
 * FUNCTION:    ToolsGetFrmObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object
 *
 * PARAMETERS:  frmP - form
 *				Id - id of the object to display,
 *
 * RETURNED:    pointer to the object
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			aro			6/12/00		Initial Revision
 *
 ***********************************************************************/

void* ToolsGetFrmObjectPtr( FormType* frmP, DmResID id )
{
	return (FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
}


/***********************************************************************
 *
 * FUNCTION:    ToolsAddrBeamBusinessCard
 *
 * DESCRIPTION: Send the Business Card record or complain if none selected.
 *
 * PARAMETERS:  dbP - the database
 *
 * RETURNED:    true if the record is found and sent
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger  10/20/97  Initial Revision
 *
 ***********************************************************************/
Boolean ToolsAddrBeamBusinessCard (DmOpenRef dbP)
{
	UInt16 recordNum;


	if (DmFindRecordByID (AddrDB, BusinessCardRecordID, &recordNum) == dmErrUniqueIDNotFound ||
		DmQueryRecord(dbP, recordNum) == 0)
		FrmAlert(SendBusinessCardAlert);
	else
	{
		TransferSendRecord(dbP, recordNum, exgBeamPrefix, NoDataToBeamAlert);
		return true;
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:     ToolsInitPhoneLabelLetters
 *
 * DESCRIPTION:  Init the list of first letters of phone labels.  Used
 * in the list view and for find.
 *
 * PARAMETERS:   appInfoPtr - contains the field labels
 *                 phoneLabelLetters - array of characters (one for each phone label)
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         roger   7/24/95   Initial Revision
 *
 ***********************************************************************/
void ToolsInitPhoneLabelLetters(AddrAppInfoPtr appInfoPtr, Char * phoneLabelLetters)
{
	UInt16 i;


	// Get the first char of the phone field labels for the list view.
	for (i = 0; i < numPhoneLabels; i++){
		phoneLabelLetters[i] = appInfoPtr->fieldLabels[i +
													   ((i < numPhoneLabelsStoredFirst) ? firstPhoneField :
														(ad_addressFieldsCount - numPhoneLabelsStoredFirst))][0];
	}
}


/***********************************************************************
 *
 * FUNCTION:    ToolsDuplicateCurrentRecord
 *
 * DESCRIPTION: Duplicates a new record from the current record.
 *
 * PARAMETERS:  numCharsToHilite (Output):  The number of characters added to the
 *				first name field to indicate that it was a duplicated record.
 *
 * RETURNED:    The number of the new duplicated record.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         css	  6/13/99   Initial Revision
 *
 ***********************************************************************/
UInt16 ToolsDuplicateCurrentRecord (UInt16 *numCharsToHilite, Boolean deleteCurrentRecord)
{
	AddrDBRecordType recordToDup;
	UInt16 attr;
	Err err;
	UInt16 newRecordNum;
	MemHandle recordH;
	char *newFirstName = NULL;
	char duplicatedRecordIndicator [maxDuplicatedIndString + 1];
	UInt16 sizeToGet;
	UInt16 oldFirstNameLen;
	AddressFields fieldToAdd = ad_firstName;

	AddrDBGetRecord (AddrDB, CurrentRecord, &recordToDup, &recordH);

	// Now we must add the "duplicated indicator" to the end of the First Name so that people
	// know that this was the duplicated record.
	ToolsGetStringResource (DuplicatedRecordIndicatorStr, duplicatedRecordIndicator);
	*numCharsToHilite = StrLen (duplicatedRecordIndicator);

	// Find the first non-empty field from (first name, last name, company) to add "copy" to.
	fieldToAdd = ad_firstName;
	if (recordToDup.fields[fieldToAdd] == NULL)
		fieldToAdd = ad_name;
	if (recordToDup.fields[fieldToAdd] == NULL)
		fieldToAdd = ad_company;
	// revert to last name if no relevant fields exist
	if (recordToDup.fields[fieldToAdd] == NULL)
		fieldToAdd = ad_name;

	if (recordToDup.fields[fieldToAdd] == NULL)
	{
		recordToDup.fields[fieldToAdd] = duplicatedRecordIndicator;
	}
	else
	{
		// Get enough space for current string, one blank and duplicated record
		// indicator string & end of string char.
		oldFirstNameLen = StrLen (recordToDup.fields[fieldToAdd]);
		sizeToGet = oldFirstNameLen + sizeOf7BitChar(spaceChr)+ StrLen (duplicatedRecordIndicator) + sizeOf7BitChar(nullChr);
		newFirstName = MemPtrNew (sizeToGet);

		if (newFirstName == NULL)
		{
			FrmAlert (DeviceFullAlert);
			newRecordNum = noRecord;
			goto Exit;
		}

		// make the new first name string with what was already there followed by
		// a space and the duplicate record indicator string.

		StrPrintF (newFirstName, "%s %s", recordToDup.fields[fieldToAdd], duplicatedRecordIndicator);

		recordToDup.fields[fieldToAdd] = newFirstName;
		// Must increment for the blank space that we add.
		(*numCharsToHilite)++;

		// Make sure that this string is less than or equal to the maximum allowed for
		// the field.
		if (StrLen (newFirstName) > maxNameLength)
		{
			newFirstName [maxNameLength] = '\0';
			(*numCharsToHilite) = maxNameLength - oldFirstNameLen;
		}
	}

	EditRowIDWhichHadFocus = fieldToAdd; //this is a lucky coincidence, that the first two
	//enums are the first two rows, but:
	if (EditRowIDWhichHadFocus == ad_company) //the third one's not
		EditRowIDWhichHadFocus++;

	MemHandleUnlock(recordH);

	// Make sure the attributes of the new record are the same.
	DmRecordInfo (AddrDB, CurrentRecord, &attr, NULL, NULL);

	// If we are to delete the current record, then lets do that now.  We have
	// all the information from the record that we need to duplicate it correctly.
	if (deleteCurrentRecord)
	{
		ToolsDeleteRecord (false);
	}

	// Now create the new record that has been duplicated from the current record.
	err = AddrDBNewRecord(AddrDB, &recordToDup, &newRecordNum);
	if (err)
	{
		FrmAlert(DeviceFullAlert);
		newRecordNum = noRecord;
		goto Exit;
	}

	// This includes the catagory so the catagories are the same between the original and
	// duplicated record.
	attr |= dmRecAttrDirty;
	DmSetRecordInfo (AddrDB, newRecordNum, &attr, NULL);


Exit:
	if (newFirstName)
	{
		MemPtrFree (newFirstName);
	}

	return (newRecordNum);
}


/*****************************************************************************
 * Function:			ToolsGetStringResource
 *
 * Description:			Reads the string associated with the resource into the passed
 *						in string.
 *
 * Notes:				None.
 *
 * Parameters:			stringResource:	(Input) The string resource to be read.
 *						stringP:		(Output) The string that represents the resource.
 *
 * Return Value(s):		The address of the string that contains a copy of the resource string.
 ******************************************************************************/
char* ToolsGetStringResource (UInt16 stringResource, char * stringP)
{
	MemHandle 	nameH;

	nameH = DmGetResource(strRsc, stringResource);
	StrCopy (stringP, (Char *) MemHandleLock (nameH));
	MemHandleUnlock (nameH);
	DmReleaseResource (nameH);

	return (stringP);
}

/***********************************************************************
 *
 * FUNCTION:
 *	ToolsIsPhoneIndexSupported
 *
 * DESCRIPTION:
 *	This routine check if a the phone index of the given record has
 *	a type that allows dialing (ie for now all except email)
 *
 * PARAMETERS:
 *	recordP		IN	record pointer
 *	phoneIndex	IN	index of the phone to check, kDialListShowInListPhoneIndex
 *					for show in list
 *
 * RETURNED:
 *  true if the type is supported and if its content is not null
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/27/00		Initial Revision
 *
 ***********************************************************************/
Boolean	ToolsIsPhoneIndexSupported( AddrDBRecordType* addrP, UInt16 phoneIndex )
{
	UInt16 label;
	UInt16	fieldIndex;

	if (phoneIndex == kDialListShowInListPhoneIndex)
	{
		phoneIndex = addrP->options.phones.displayPhoneForList;
	}
	fieldIndex = firstPhoneField + phoneIndex;

	if (!(addrP->fields[fieldIndex]))
		return false;

	switch (fieldIndex)
	{
	case ad_phone1:
		label = (AddressPhoneLabels)addrP->options.phones.phone1;
		break;
	case ad_phone2:
		label = (AddressPhoneLabels)addrP->options.phones.phone2;
		break;
	case ad_phone3:
		label = (AddressPhoneLabels)addrP->options.phones.phone3;
		break;
	case ad_phone4:
		label = (AddressPhoneLabels)addrP->options.phones.phone4;
		break;
	case ad_phone5:
		label = (AddressPhoneLabels)addrP->options.phones.phone5;
		break;
	default:
		return false;
	}

	// Check whether this is a phone number and if so, whether it's one that's
	// appropriate to be dialed.
	switch (label)
	{
		// These are the phone numbers which are dialable.
	case workLabel:
	case homeLabel:
	case otherLabel:
	case mainLabel:
	case pagerLabel:
	case mobileLabel:
	case faxLabel:
		return true;
	default:
		return false;
	}
}


/***********************************************************************
 *
 * FUNCTION:
 *	ToolsGetLineIndexAtOffset
 *
 * DESCRIPTION:
 *	This routine gets the line index of a string at a specified offset
 *
 * PARAMETERS:
 *	textP	IN	text to parse
 *	offset	IN	char offset
 *
 * RETURNED:
 *	index of the line
 *
 * REVISION HISTORY:
 *	Name		Date		Description
 *	----		----		-----------
 *	aro			6/27/00		Initial Revision
 *
 ***********************************************************************/
UInt16	ToolsGetLineIndexAtOffset( Char* textP, UInt16 offset )
{
	Char* nextP;
	UInt16 lineIndex = 0;

	while (offset)
	{
		nextP = StrChr(textP, chrLineFeed);
		if (nextP)
		{
			UInt16 diff = nextP - textP;
			if (offset <= diff)
				break;
			else
			{
				offset -= (diff + 1);
				textP = nextP + 1;
				lineIndex++;
			}
		}
		else
			break;
	}
	return lineIndex;
}

//#pragma mark -

/***********************************************************************
 *
 * FUNCTION:    PrvToolsPhoneIsANumber
 *
 * DESCRIPTION: Determines whether the phone field contains a number or a
 * 				 string using the following heuristic: if the string contains
 *					 more numeric characters than non-numeric, it is a number.
 *
 * PARAMETERS:  phone - pointer to phone string
 *
 * RETURNED:    true if the string is a number.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         jeff   4/15/99   Initial Revision
 *
 ***********************************************************************/

Boolean PrvToolsPhoneIsANumber( Char* phone )
{
	UInt16	digitCount = 0;
	UInt16	charCount = 0;
	UInt32 byteLength = 0;
	WChar	ch;

	byteLength += TxtGetNextChar( phone, byteLength, &ch );
	while ( ch != 0 )
	{
		charCount++;
		if ( TxtCharIsDigit( ch ) ) digitCount++;
		byteLength += TxtGetNextChar( phone, byteLength, &ch );
	}

	return ( digitCount > ( charCount / 2 ) );
}
