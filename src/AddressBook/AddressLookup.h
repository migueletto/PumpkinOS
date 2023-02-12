/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressLookup.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESSLOOKUP_H
#define ADDRESSLOOKUP_H

#include "AddressDB.h"

#include <AppLaunchCmd.h>
#include <Form.h>
#include <PdiLib.h>

/***********************************************************************
 *   Internal Structures
 ***********************************************************************/

// The Lookup command is called while another app is running.  Because of
// of this the other app and not the Address Book has global variables.
// All the variables the lookup command needs as globals are therefore
// kept in this structure.

// 2/2/99 meg added the beepOnFail boolean because when a callng app sent in a
// multi char string and the string did not match, the lookup code would beep
// then remove the last char. It would then repeat this process, beeping for
// each char that was removed. I added this boolean that is initialized to true
// so it beeps the first time, then when it beeps it sets the var to false.
// Entering a char turns it back on, so a user entering into the lookup field
// will still get a beep per bad char, but it only will beep once for strings
// passed in.

typedef struct
{
	AddrLookupParamsPtr params;
	FormPtr frm;
	DmOpenRef dbP;
	Boolean hideSecretRecords;
	UInt8 reserved1;
	UInt16 currentRecord;
	Int16 currentPhone;
	UInt16 topVisibleRecord;
	Int16 topVisibleRecordPhone;
	Boolean sortByCompany;
	Boolean ignoreEmptyLookupField;
	AddressFields lookupFieldMap[addrLookupFieldCount];
	Char phoneLabelLetters[numPhoneLabels];
	Boolean beepOnFail;
	UInt8 reserved2;
} LookupVariablesType;

typedef LookupVariablesType *LookupVariablesPtr;


/************************************************************
 * Function Prototypes
 *************************************************************/

void Lookup (AddrLookupParamsType *params);

#endif // ADDRESSLOOKUP_H
