/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressAutoFill.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESSAUTOFILL_H
#define ADDRESSAUTOFILL_H


typedef struct {
	UInt32		time;						// time the entry was last accessed
	Char		text;						// null-terminated string
	UInt8		reserved;
} LookupRecordType;

typedef LookupRecordType *LookupRecordPtr;


/************************************************************
 * Function Prototypes
 *************************************************************/

Err		AutoFillInitDB (UInt32 type, UInt32 creator, const Char *dbName, UInt16 initRscID);
Boolean AutoFillLookupStringInDatabase (DmOpenRef dbP, Char *key, UInt16 *indexP);
void	AutoFillLookupSave (UInt32 type, UInt32 creator, Char *str);

#endif // ADDRESSAUTOFILL_H
