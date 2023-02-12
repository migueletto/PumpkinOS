/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: Address.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESS_H
#define ADDRESS_H

#include <PalmOS.h>
#include <IMCUtils.h>
#include <ExgMgr.h>
#include <PrivateRecords.h>
#include <UIColor.h>
#include <PdiLib.h>
#include <Form.h>

#include "AddressDB.h"
#include "AddrDefines.h"


/************************************************************
 * Global Variables
 *************************************************************/

extern privateRecordViewEnum	PrivateRecordVisualStatus;
extern Boolean					SortByCompany;
extern DmOpenRef				AddrDB;
extern Char						CategoryName[dmCategoryLength];
extern UInt16       			TopVisibleRecord;
extern UInt16					TopVisibleFieldIndex;
extern UInt16					EditFieldPosition;
extern UInt16          			CurrentRecord;
extern UInt16          		 	ListViewSelectThisRecord;
extern UInt16					PriorAddressFormID;   // Used for NoteView
extern Boolean					RecordNeededAfterEditView;
extern Char *					UnnamedRecordStringPtr;
extern MemHandle				UnnamedRecordStringH;
extern UInt16					RecordLabelColumnWidth;
extern UInt16					EditLabelColumnWidth;
extern UInt16					EditRowIDWhichHadFocus;

// These are used for controlling the display of the duplicated address records.
extern UInt16					NumCharsToHilite;

// The following global variable are saved to a state file.
extern UInt16					CurrentCategory;
extern Boolean					EnableTapDialing;
extern Boolean					ShowAllCategories;
extern Boolean					SaveBackup;
extern Boolean					RememberLastCategory;
extern FontID					NoteFont;
extern FontID					AddrListFont;
extern FontID					AddrRecordFont;
extern FontID					AddrEditFont;
extern UInt32					BusinessCardRecordID;

// For business card beaming
extern UInt32					TickAppButtonPushed;

// Valid after StartApplication
extern Char						PhoneLabelLetters[numPhoneLabels];

extern Boolean					DialerPresentChecked;
extern Boolean					DialerPresent;

#endif // ADDRESS_H
