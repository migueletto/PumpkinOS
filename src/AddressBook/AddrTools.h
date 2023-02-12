/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: addrtools.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRTOOLS_H
#define ADDRTOOLS_H

#include "AddressDB.h"

#include <UIColor.h>
#include <Form.h>
#include <ExgMgr.h>


/************************************************************
 * Function Prototypes
 *************************************************************/

Boolean	ToolsIsDialerPresent( void );
void	ToolsSetDBAttrBits(DmOpenRef dbP, UInt16 attrBits);
Err		ToolsCreateDefaultDatabase(void);
void	ToolsRegisterLocaleChangingNotification(void);
Boolean ToolsDetermineRecordName(AddrDBRecordPtr recordP, Int16 *shortenedFieldWidth, Int16 *fieldSeparatorWidth, Boolean sortByCompany, Char **name1, Int16 *name1Length, Int16 *name1Width, Char **name2, Int16 *name2Length, Int16 *name2Width, Char **unnamedRecordStringPtr, MemHandle* unnamedRecordH, Int16 nameExtent);
void	ToolsDrawRecordName (Char *name1, Int16 name1Length, Int16 name1Width, Char *name2, Int16 name2Length, Int16 name2Width, Int16 nameExtent, Int16 *x, Int16 y, Int16 shortenedFieldWidth, Int16 fieldSeparatorWidth, Boolean center, Boolean priorityIsName1, Boolean inTitle);
Int16	ToolsDrawRecordNameAndPhoneNumber (AddrDBRecordPtr record, RectanglePtr bounds, Char * phoneLabelLetters, Boolean sortByCompany, Char **unnamedRecordStringPtr, MemHandle* unnamedRecordStringH);
UInt16	ToolsGetLabelColumnWidth (AddrAppInfoPtr appInfoPtr, FontID labelFontID);
void	ToolsLeaveForm();
void	ToolsChangeCategory (UInt16 category);
Boolean	ToolsSeekRecord (UInt16 * indexP, Int16 offset, Int16 direction);
void	ToolsDirtyRecord (UInt16 index);
FontID	ToolsSelectFont (FontID currFontID);
void	ToolsDeleteRecord (Boolean archive);
Err		ToolsCustomAcceptBeamDialog(DmOpenRef dbP, ExgAskParamPtr askInfoP);
void*	ToolsGetObjectPtr (UInt16 objectID);
void*	ToolsGetFrmObjectPtr( FormType* frmP, DmResID id );
Boolean	ToolsAddrBeamBusinessCard (DmOpenRef dbP);
void	ToolsInitPhoneLabelLetters(AddrAppInfoPtr appInfoPtr, Char *phoneLabelLetters);
UInt16	ToolsDuplicateCurrentRecord (UInt16 *numCharsToHilite, Boolean deleteCurrentRecord);
char*	ToolsGetStringResource (UInt16 stringResource, char * stringP);
Boolean	ToolsIsPhoneIndexSupported( AddrDBRecordType* addrP, UInt16 phoneIndex );
UInt16	ToolsGetLineIndexAtOffset( Char* textP, UInt16 offset );

#endif // ADDRTOOLS_H
