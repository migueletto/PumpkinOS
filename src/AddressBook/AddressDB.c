/******************************************************************************
 *
 * Copyright (c) 1995-2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressDB.c
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 * Description:
 *      Address Manager routines
 *
 *****************************************************************************/

#include "sec.h"

#include "AddressDB.h"
#include "AddrTools.h"
#include "AddressRsc.h"
#include "AddrDefines.h"

#include <UIResources.h>
#include <SysUtils.h>
#include <ErrorMgr.h>
#include <StringMgr.h>
#include <TextMgr.h>
#include <PalmUtils.h>

#include "SysDebug.h"


/***********************************************************************
 *
 *   Defines
 *
 ***********************************************************************/

// Max length of a field name found in the FieldNamesStrList string list.
#define maxFieldName		31

#define LocalizedAppInfoStr	1000

#define sortKeyFieldBits   (BitAtPosition(ad_name) | BitAtPosition(ad_firstName) | BitAtPosition(ad_company))

// Indexes into FieldNamesStrList string list.
enum {
	fieldNameStrListCity = 0,
	fieldNameStrListState,
	fieldNameStrListZip
};

// The following structure doesn't really exist.  The first field
// varies depending on the data present.  However, it is convient
// (and less error prone) to use when accessing the other information.
typedef struct {
	AddrOptionsType		options;        // Display by company or by name
	AddrDBRecordFlags	flags;
	UInt8				companyFieldOffset;   // Offset from firstField
	char				firstField;
} PrvAddrPackedDBRecord;

static const UInt32 bitPos[32] = {
  24, 25, 26, 27, 28, 29, 30, 31,
  16, 17, 18, 19, 20, 21, 22, 23,
   8,  9, 10, 11, 12, 13, 14, 15,
   0,  1,  2,  3,  4,  5,  6,  7
};

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/

static void		PrvAddrDBLocalizeAppInfo(AddrAppInfoPtr appInfoP) SEC("code1");
static void		PrvAddrDBFindKey(PrvAddrPackedDBRecord *r, char **key, UInt16 *whichKey, Int16 sortByCompany) SEC("code1");
static Int16	PrvAddrDBComparePackedRecords(PrvAddrPackedDBRecord *r1, PrvAddrPackedDBRecord *r2, Int16 sortByCompany, SortRecordInfoPtr /*info1*/, SortRecordInfoPtr /*info2*/, MemHandle /*appInfoH*/) SEC("code1");
static Int16	PrvAddrDBUnpackedSize(AddrDBRecordPtr r) SEC("code1");
static void		PrvAddrDBPack(AddrDBRecordPtr s, void * recordP) SEC("code1");
static void		PrvAddrDBUnpack(PrvAddrPackedDBRecord *src, AddrDBRecordPtr dest) SEC("code1");
static UInt16	PrvAddrDBFindSortPosition(DmOpenRef dbP, PrvAddrPackedDBRecord *newRecord) SEC("code1");
static UInt16	PrvAddrDBStrCmpMatches(const Char* s1, const Char* s2) SEC("code1");
static Boolean	PrvAddrDBRecordContainsField(PrvAddrPackedDBRecord *packedRecordP, AddressLookupFields field, Int16 * phoneP, Int16 direction, AddressFields lookupFieldMap[]) SEC("code1");
static Boolean	PrvAddrDBSeekVisibleRecordInCategory (DmOpenRef dbR, UInt16 * indexP, UInt16 offset, Int16 direction, UInt16 category, Boolean masked) SEC("code1");

UInt32 BitAtPosition(UInt32 pos) {
  return ((UInt32)1) << bitPos[pos];
}

/************************************************************
 *
 *  FUNCTION: AddrDBAppInfoGetPtr
 *
 *  DESCRIPTION: Return a locked pointer to the AddrAppInfo or NULL
 *
 *  PARAMETERS: dbP - open database pointer
 *
 *  RETURNS: locked ptr to the AddrAppInfo or NULL
 *
 *  CREATED: 6/13/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
AddrAppInfoPtr   AddrDBAppInfoGetPtr(DmOpenRef dbP)
{
	UInt16     cardNo;
	LocalID    dbID;
	LocalID    appInfoID;

	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return NULL;
	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
		return NULL;

	if (appInfoID == 0)
		return NULL;
	else
		return MemLocalIDToLockedPtr(appInfoID, cardNo);

}


/************************************************************
 *
 *  FUNCTION: AddrDBChangeCountry
 *
 *  DESCRIPTION: Set the field labels to those appropriate
 *  to the current country (based on system preferences).
 *
 *  PARAMETERS: application info ptr
 *
 *  RETURNS: nothing
 *
 * HISTORY:
 *		07/24/95	rsf	Created by Roger Flores
 *		07/29/99	kwk	Load field names from string list resource.
 *
 *************************************************************/
void AddrDBChangeCountry(AddrAppInfoPtr appInfoP)
{
	CountryType countryCurrent;
	//AddrAppInfoPtr   nilP = NULL;
	MemHandle textH;
	UInt16 strListID;
	Char fieldName[maxFieldName + sizeOf7BitChar(chrNull)];

	// Localize the field labels to the current country
	countryCurrent = (CountryType) PrefGetPreference(prefCountry);
	strListID = (UInt16)countryCurrent + FieldNamesStrList;

	textH = DmGetResource(strListRscType, strListID);
	if (textH != NULL)
	{
		AddrDBRecordFlags dirtyFieldLabels;

		//DmStrCopy(appInfoP, (Int32) nilP->fieldLabels[city],
		DmStrCopy(appInfoP,  OffsetOf(AddrAppInfoType, fieldLabels[ad_city]),
				  SysStringByIndex(strListID, fieldNameStrListCity, fieldName, maxFieldName));
		//DmStrCopy(appInfoP, (Int32) nilP->fieldLabels[state],
		DmStrCopy(appInfoP, OffsetOf(AddrAppInfoType, fieldLabels[ad_state]),
				  SysStringByIndex(strListID, fieldNameStrListState, fieldName, maxFieldName));
		//DmStrCopy(appInfoP, (Int32) nilP->fieldLabels[zipCode],
		DmStrCopy(appInfoP, OffsetOf(AddrAppInfoType, fieldLabels[ad_zipCode]),
				  SysStringByIndex(strListID, fieldNameStrListZip, fieldName, maxFieldName));

		dirtyFieldLabels.allBits = (appInfoP->dirtyFieldLabels.allBits) |
			BitAtPosition(ad_city) | BitAtPosition(ad_state) | BitAtPosition(ad_zipCode);

		//DmWrite(appInfoP, (Int32) &nilP->dirtyFieldLabels, &dirtyFieldLabels, sizeof dirtyFieldLabels);
		DmWrite(appInfoP, OffsetOf(AddrAppInfoType, dirtyFieldLabels), &dirtyFieldLabels, sizeof dirtyFieldLabels);
	}

	// Record the country.
	//DmWrite(appInfoP, (Int32) &nilP->country, &countryCurrent, sizeof(countryCurrent));
	DmWrite(appInfoP, OffsetOf(AddrAppInfoType, country), &countryCurrent, sizeof(countryCurrent));
}


/************************************************************
 *
 *  FUNCTION: AddrDBAppInfoInit
 *
 *  DESCRIPTION: Create an app info chunk if missing.  Set
 *      the strings to a default.
 *
 *  PARAMETERS: dbP - open database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/3/95
 *
 *  BY: Roger Flores
 *
 *  MODIFICATIONS:
 *      10/22/96   roger      Change to init data via code and resources to
 *                        remove global var use which wasn't always available.
 *************************************************************/
Err   AddrDBAppInfoInit(DmOpenRef dbP)
{
	UInt16         cardNo;
	LocalID        dbID;
	LocalID        appInfoID;
	MemHandle         h;
	AddrAppInfoPtr appInfoP;
	AddrAppInfoPtr defaultAddrApplicationInfoP;
	UInt8          i;


	appInfoP = AddrDBAppInfoGetPtr(dbP);

	// If there isn't an AddrApplicationInfo make space for one
	if (appInfoP == NULL)
	{
		if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
			return dmErrInvalidParam;
		if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
			return dmErrInvalidParam;

		h = DmNewHandle(dbP, sizeof(AddrAppInfoType));
		if (!h) return dmErrMemError;

		appInfoID = MemHandleToLocalID( h);
		if (DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
		{
			MemHandleFree(h);
			return 1;
		}

		appInfoP = MemHandleLock(h);
	}


	// Allocate & Clear the app info
	defaultAddrApplicationInfoP = MemPtrNew(sizeof(AddrAppInfoType));
	if (defaultAddrApplicationInfoP == NULL)
	{
		ErrDisplay("Unable to init AddressDB");
		return 1;
	}

	MemSet(defaultAddrApplicationInfoP, sizeof(AddrAppInfoType), 0);

	// Init the categories
	for (i = 0; i < dmRecNumCategories; i++)
	{
		defaultAddrApplicationInfoP->categoryUniqIDs[i] = i;
	}
	defaultAddrApplicationInfoP->lastUniqID = dmRecNumCategories - 1;

	// Set to sort by name
	defaultAddrApplicationInfoP->misc.sortByCompany = false;


	// copy in the defaults and free the default app info
	DmWrite(appInfoP, 0, defaultAddrApplicationInfoP,  sizeof(AddrAppInfoType));
	MemPtrFree(defaultAddrApplicationInfoP);


	// Try to use localized app info block strings.
	PrvAddrDBLocalizeAppInfo(appInfoP);

	// Localize the field labels to the current country
	AddrDBChangeCountry(appInfoP);


	// Unlock
	MemPtrUnlock(appInfoP);

	return 0;
}


/************************************************************
 *
 *  FUNCTION: AddrDBSetFieldLabel
 *
 *  DESCRIPTION: Set a field's label and mark it dirty.
 *
 *  PARAMETERS: dbP - open database pointer
 *                fieldNum - field label to change
 *                fieldLabel - new field label to use
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 6/28/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
void AddrDBSetFieldLabel(DmOpenRef dbP, UInt16 fieldNum, Char * fieldLabel)
{
	AddrAppInfoPtr    appInfoP;
	AddrAppInfoType   copy;


	ErrFatalDisplayIf(fieldNum >= lastLabel,
					  "fieldNum out of range");

	// Get a copy of the app info
	appInfoP = AddrDBAppInfoGetPtr(dbP);
	ErrFatalDisplayIf(appInfoP == NULL,
					  "Bad database (invalid or no app info block)");
	MemMove(&copy, appInfoP, sizeof(copy));

	// Make the changes
	StrCopy(copy.fieldLabels[fieldNum], fieldLabel); //lint !e661
	SetBitMacro(copy.dirtyFieldLabels.allBits, fieldNum);

	// Write changes to record
	DmWrite(appInfoP, 0, &copy, sizeof(copy));

	// Unlock app info
	MemPtrUnlock(appInfoP);
}

static void printRecord(PrvAddrPackedDBRecord *rec, int indent) {
#ifdef SYSDEBUG
  UInt16 index;
  UInt32 flags;
	char *p;

#ifdef PALMOS
  //SysDebug(1, "check", "%srecord 0x%08lX options=0x%08lX flags=%08lX", indent ? "  " : "", (UInt32)rec, rec->options, rec->flags);
#else
  //SysDebug(1, "check", "%srecord %p options=0x%08X flags=%08X", indent ? "  " : "", rec, rec->options, rec->flags);
#endif
  flags = rec->flags.allBits;
  p = (char *)rec;
  p = &p[9];

  for (index = firstAddressField; index < ad_addressFieldsCount; index++) {
    if (GetBitMacro(flags, index) != 0) {
      //SysDebug(1, "check", "%s  field %d: \"%s\"", indent ? "  " : "", index, p);
      p += StrLen(p) + 1;
    }
  }
#endif
}

static void printDatabase(DmOpenRef dbP) {
#ifdef SYSDEBUG
  UInt16 index, num;
  MemHandle h;
  PrvAddrPackedDBRecord *rec;

	num = DmNumRecords(dbP);
  //SysDebug(1, "check", "database %d records begin", num);
  for (index = 0; index < num; index++) {
    h = DmGetRecord(dbP, index);
    rec = MemHandleLock(h);
    printRecord(rec, 1);
    MemHandleUnlock(h);
    DmReleaseRecord(dbP, index, false);
  }
  //SysDebug(1, "check", "database %d records end", num);
#endif
}

static void printMove(DmOpenRef dbP, UInt16 from, UInt16 to) {
#ifdef SYSDEBUG
  printDatabase(dbP);
  //SysDebug(1, "check", "move record from %d to %d", from, to);
#endif
}

static void printPosition(UInt16 index, char *label) {
#ifdef SYSDEBUG
  //SysDebug(1, "check", "%s record at position %d", label, index);
  //SysDebug(1, "check", "-------------");
#endif
}

/************************************************************
 *
 *  FUNCTION: AddrDBNewRecord
 *
 *  DESCRIPTION: Create a new packed record in sorted position
 *
 *  PARAMETERS: database pointer - open db pointer
 *            address record   - pointer to a record to copy into the DB
 *            record index      - to be set to the new record's index
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *             index set if a new record is created.
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err AddrDBNewRecord(DmOpenRef dbP, AddrDBRecordPtr r, UInt16 *index)
{
	MemHandle               recordH;
	Err                   err;
	PrvAddrPackedDBRecord*   recordP;
	UInt16                   newIndex;


	// 1) and 2) (make a new chunk with the correct size)
	recordH = DmNewHandle(dbP, (Int32) PrvAddrDBUnpackedSize(r));
	if (recordH == NULL)
		return dmErrMemError;


	// 3) Copy the data from the unpacked record to the packed one.
	recordP = MemHandleLock(recordH);
	PrvAddrDBPack(r, recordP);

	// Get the index
	newIndex = PrvAddrDBFindSortPosition(dbP, recordP);
	MemPtrUnlock(recordP);

  printRecord(recordP, 0);
  printDatabase(dbP);
  printPosition(newIndex, "insert");

	// 4) attach in place
	err = DmAttachRecord(dbP, &newIndex, recordH, 0);
	if (err)
		MemHandleFree(recordH);
	else
		*index = newIndex;

	return err;
}


/************************************************************
 *
 *  FUNCTION: AddrDBChangeRecord
 *
 *  DESCRIPTION: Change a record in the Address Database
 *
 *  PARAMETERS: dbP - open database pointer
 *            database index
 *            address record
 *            changed fields
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *   COMMENTS:   Records are not stored with extra padding - they
 *   are always resized to their exact storage space.  This avoids
 *   a database compression issue.  The code works as follows:
 *
 *   1)   get the size of the new record
 *   2)   make the new record
 *   3)   pack the packed record plus the changes into the new record
 *   4)   if the sort position is changes move to the new position
 *   5)   attach in position
 *
 * The MemHandle to the record passed doesn't need to be unlocked
 * since that chunk is freed by this routine.  It should be discarded.
 *
 *************************************************************/
Err AddrDBChangeRecord(DmOpenRef dbP, UInt16 *index, AddrDBRecordPtr r, AddrDBRecordFlags changedFields)
{
	AddrDBRecordType    src;
	MemHandle             srcH;
	Err                result;
	MemHandle             recordH=0;
	MemHandle             cmpH, oldH;
	Int16                i, cmpr;
	UInt32             changes = changedFields.allBits;
	Int16                sortByCompany;
	AddrAppInfoPtr    appInfoPtr;
	Boolean            dontMove;
	UInt16                attributes;      // to contain the deleted flag

	PrvAddrPackedDBRecord*   cmpP;
	PrvAddrPackedDBRecord*   recordP;


	// We do not assume that r is completely valid so we get a valid
	// AddrDBRecordPtr...
	if ((result = AddrDBGetRecord(dbP, *index, &src, &srcH)) != 0)
		return result;

	// and we apply the changes to it.
	src.options = r->options;         // copy the phone info
	for (i = firstAddressField; i < ad_addressFieldsCount; i++)
	{
		// If the flag is set point to the string else NULL
		if (GetBitMacro(changes, i) != 0)
		{
			src.fields[i] = r->fields[i];
			RemoveBitMacro(changes, i);
		}
		if (changes == 0)
			break;      // no more changes
	}


	// 1) and 2) (make a new chunk with the correct size)
	recordH = DmNewHandle(dbP, PrvAddrDBUnpackedSize(&src));
	if (recordH == NULL)
	{
		MemHandleUnlock(srcH);      // undo lock from AddrGetRecord above
		return dmErrMemError;
	}
	recordP = MemHandleLock(recordH);


	// 3) Copy the data from the unpacked record to the packed one.
	PrvAddrDBPack(&src, recordP);
  //SysDebug(1, "Addr", "AddrDBChangeRecord record:");
  //SysDebugBytes(1, "Addr", recordP, MemHandleSize(recordH));

	// The original record is copied and no longer needed.
	MemHandleUnlock(srcH);


	// 4) if the sort position changes...
	// Check if any of the key fields have changed
	if ((changedFields.allBits & sortKeyFieldBits) == 0)
		goto attachRecord;

  //SysDebug(1, "Addr", "AddrDBChangeRecord key field has changed");

	// Make sure *index-1 < *index < *index+1, if so it's in sorted
	// order.  Leave it there.
	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
	sortByCompany = appInfoPtr->misc.sortByCompany;
	MemPtrUnlock(appInfoPtr);

	if (*index > 0)
	{
		// This record wasn't deleted and deleted records are at the end of the
		// database so the prior record may not be deleted!
    cmpH = DmQueryRecord(dbP, *index-1);
		cmpP = MemHandleLock(cmpH);
    //SysDebug(1, "Addr", "AddrDBChangeRecord previous:");
    //SysDebugBytes(1, "Addr", cmpP, MemHandleSize(cmpH));
		cmpr = PrvAddrDBComparePackedRecords (cmpP,  recordP, sortByCompany, NULL, NULL, 0);
		dontMove = cmpr == -1;
    //SysDebug(1, "Addr", "AddrDBChangeRecord comparison %d, %smove", cmpr, dontMove ? "dont " : "");
		MemPtrUnlock(cmpP);
	}
	else {
    //SysDebug(1, "Addr", "AddrDBChangeRecord first");
		dontMove = true;
  }


	if (*index+1 < DmNumRecords (dbP))
	{
		DmRecordInfo(dbP, *index+1, &attributes, NULL, NULL);
		if (attributes & dmRecAttrDelete)
			;      // don't move it after the deleted record!
		else {
      cmpH = DmQueryRecord(dbP, *index+1);
			cmpP = MemHandleLock(cmpH);
      //SysDebug(1, "Addr", "AddrDBChangeRecord next:");
      //SysDebugBytes(1, "Addr", cmpP, MemHandleSize(cmpH));
			cmpr = PrvAddrDBComparePackedRecords (recordP, cmpP, sortByCompany, NULL, NULL, 0);
			dontMove = dontMove && cmpr == -1;
      //SysDebug(1, "Addr", "AddrDBChangeRecord comparison %d, %smove", cmpr, dontMove ? "dont " : "");
			MemPtrUnlock(cmpP);
		}
	}

  printRecord(recordP, 0);

	if (dontMove) {
    //SysDebug(1, "Addr", "AddrDBChangeRecord dont move, attach directly");
		goto attachRecord;
  }

	// The record isn't in the right position.  Move it.
	i = PrvAddrDBFindSortPosition(dbP, recordP);
  //SysDebug(1, "Addr", "AddrDBChangeRecord move from %d to %d", *index, i);
  printMove(dbP, *index, i);
	DmMoveRecord(dbP, *index, i);
	if (i > *index) i--;
	*index = i;                  // return new position
  //SysDebug(1, "Addr", "AddrDBChangeRecord new index %d", *index);


	// Attach the new record to the old index,  the preserves the
	// category and record id.
attachRecord:

  printDatabase(dbP);
  printPosition(*index, "update");

  //SysDebug(1, "Addr", "AddrDBChangeRecord attach record at %d", *index);
	result = DmAttachRecord(dbP, index, recordH, &oldH);
	MemPtrUnlock(recordP);
	if (result) return result;

	MemHandleFree(oldH);
	return 0;
}


/************************************************************
 *
 *  FUNCTION: AddrDBGetRecord
 *
 *  DESCRIPTION: Get a record from the Address Database
 *
 *  PARAMETERS: database pointer - open db pointer
 *            database index - index of record to lock
 *            address record pointer - pointer address structure
 *            address record - MemHandle to unlock when done
 *
 *  RETURNS: ##0 if successful, errorcode if not
 *    The record's MemHandle is locked so that the pointer to
 *  strings within the record remain pointing to valid chunk
 *  versus the record randomly moving.  Unlock the MemHandle when
 *  AddrDBRecord is destroyed.
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP,
				  MemHandle *recordH)
{
	PrvAddrPackedDBRecord *src;

	*recordH = DmQueryRecord(dbP, index);
	src = (PrvAddrPackedDBRecord *) MemHandleLock(*recordH);
	if (src == NULL)
		return dmErrIndexOutOfRange;

	PrvAddrDBUnpack(src, recordP);

	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    AddrDBRecordContainsData
 *
 * DESCRIPTION: Checks the record returns true if it contains any data.
 *
 * PARAMETERS:  recordP  - a pointer to an address record
 *
 * RETURNED:    true if one of the fields has data
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         rsf   12/3/97   Initial Revision
 *
 ***********************************************************************/
Boolean AddrDBRecordContainsData (AddrDBRecordPtr recordP)
{
	UInt16 i;


	// Look for a field which isn't empty
	for (i = firstAddressField; i < ad_addressFieldsCount; i++)
	{
		if (recordP->fields[i] != NULL)
			return true;
	}

	return false;
}


/************************************************************
 *
 *  FUNCTION: AddrDBChangeSortOrder
 *
 *  DESCRIPTION: Change the Address Database's sort order
 *
 *  PARAMETERS: dbP - open database pointer
 *            TRUE if sort by company
 *
 *  RETURNS: nothing
 *
 *  CREATED: 1/17/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err AddrDBChangeSortOrder(DmOpenRef dbP, Boolean sortByCompany)
{
	AddrAppInfoPtr appInfoPtr;
	//AddrAppInfoPtr   nilP=0;
	AddrDBMisc      misc;


	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
	misc = appInfoPtr->misc;
	misc.sortByCompany = sortByCompany;
	//DmWrite(appInfoPtr, (Int32) &nilP->misc, &misc, sizeof(misc));
	DmWrite(appInfoPtr, OffsetOf(AddrAppInfoType, misc), &misc, sizeof(misc));
	MemPtrUnlock(appInfoPtr);

	DmQuickSort(dbP, (DmComparF *) PrvAddrDBComparePackedRecords, (Int16) sortByCompany);
	return 0;
}


/***********************************************************************
 *
 * FUNCTION:    AddrDBLookupSeekRecord
 *
 * DESCRIPTION: Given the index of a record, scan
 *              forewards or backwards for displayable records.
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
 * RETURNED:    true if a displayable record was found.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
Boolean AddrDBLookupSeekRecord (DmOpenRef dbP, UInt16 * indexP, Int16 * phoneP, Int16 offset, Int16 direction, AddressLookupFields field1, AddressLookupFields field2, AddressFields lookupFieldMap[])
{
	UInt16 index;
	UInt16 oldIndex;
	UInt16 count;
	UInt16 numRecords;
	MemHandle recordH;
	Boolean match;
	Int16 phone;
	Boolean searchPhones;
	PrvAddrPackedDBRecord *packedRecordP;


	ErrFatalDisplayIf ( (direction != dmSeekForward) && (direction != dmSeekBackward),
						"Bad Param");

	ErrFatalDisplayIf ( (offset < 0), "Bad param");


	index = *indexP;
	phone = *phoneP;

	searchPhones = IsPhoneLookupField(field1) || IsPhoneLookupField(field2);

	numRecords = DmNumRecords(dbP);

	if (index >= numRecords)
	{
		if (direction == dmSeekForward)
			return false;
		else
			index = numRecords - 1;
	}


	// Moving forward?
	if (direction == dmSeekForward )
		count = numRecords - index;
	else
		count = index + 1;


	// Loop through the records
	while (count--) {

		// Make sure the current record isn't hidden.  If so skip it and find the
		// next non hidden record.  Decrease the record count to search by the number
		// of records skipped.
		oldIndex = index;
		if (DmSeekRecordInCategory (dbP, &index, 0, direction, dmAllCategories))
		{
			// There are no more records.
			break;
		}
		if (index != oldIndex)
		{
			if (direction == dmSeekForward)
				count -= index - oldIndex;
			else
				count -= oldIndex - index;
		}

		recordH = DmQueryRecord(dbP, index);

		// If we have found a deleted record stop the search.
		if (!recordH) {
			break;
    }

		packedRecordP = MemHandleLock(recordH);
		if (!packedRecordP)
			goto Exit;

		match = PrvAddrDBRecordContainsField(packedRecordP, field1, &phone, direction, lookupFieldMap) &&
			PrvAddrDBRecordContainsField(packedRecordP, field2, &phone, direction, lookupFieldMap);

		MemHandleUnlock(recordH);

		if (match)
		{
			*indexP = index;
			*phoneP = phone;
			if (offset == 0) return true;
			offset--;
		}

		// Look for another phone in this record if one was found or
		// else look at the next record.
		if (searchPhones && match)
		{
			phone += direction;
			// We their are no more phones to search so advance to next record
			if (phone == -1 || numPhoneFields <= phone)
			{
				if (direction == dmSeekForward)
					phone = 0;
				else
					phone = numPhoneFields - 1;

				index += direction;
			}
			else
			{
				// Since we are going to search this record again bump the count up
				// by one.  This loop is supposed to loop once per record to search.
				count++;
			}
		}
		else
			index += direction;

	}

	return false;

Exit:
	ErrDisplay("Err seeking rec");

	return false;
}

/************************************************************
 *
 *  FUNCTION: AddrDBLookupString
 *
 *  DESCRIPTION: Return which record contains the most of
 *      the string passed.  If no string is passed or there
 *  aren't any records then false is returned.
 *
 *  PARAMETERS: address record
 *                key - string to lookup record with
 *                sortByCompany - how the db is sorted
 *                category -  the category to search in
 *                recordP - to contain the record found
 *                completeMatch -  true if a record contains all
 *                                 of the key
 *
 *  RETURNS: the record in recordP or false
 *             completeMatch -  true if a record contains all
 *                              of the key
 *
 *  CREATED: 6/15/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Boolean AddrDBLookupString(DmOpenRef dbP, Char * key, Boolean sortByCompany, UInt16 category, UInt16 * recordP, Boolean *completeMatch, Boolean masked)
{
	Int16                   numOfRecords;
	MemHandle                rH;
	PrvAddrPackedDBRecord*   r;
	UInt16                  kmin, probe, probe2, i;      // all positions in the database.
	Int16                   result;                     // result of comparing two records
	UInt16                   whichKey;
	char*                  recordKey;
	UInt16                   matches1, matches2;


	// If there isn't a key to search with stop the with the first record.
	if (key == NULL || *key == '\0')
	{
		*completeMatch = true;
		return false;
	}

	numOfRecords = DmNumRecords(dbP);
	if (numOfRecords == 0)
		return false;

	result = 0;
	kmin = probe = 0;
	rH = 0;


	while (numOfRecords > 0)
	{
		i = numOfRecords / 2;
		probe = kmin + i;


		// Compare the two records.  Treat deleted records as greater.
		// If the records are equal look at the following position.
		if (rH)
			MemHandleUnlock(rH);
		rH = DmQueryRecord(dbP, probe);
		if (rH == 0)
		{
			result = -1;      // Delete record is greater
		}
		else
		{
			r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
			ErrFatalDisplayIf(r == 0, "Addr bsearch: data somehow missing");


			// Compare the string to the first sort key only
			whichKey = 1;
			PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);

			if (recordKey == NULL)
				result = 1;
			else
				result = StrCaselessCompare(key, recordKey);


			// If equal stop here!  We don't want the position after.
			if (result == 0)
				goto findRecordInCategory;
		}


		ErrFatalDisplayIf(result == 0, "Impossible bsearch state");

		// More likely than < 0 because of deleted records
		if (result < 0)
			numOfRecords = i;
		else
		{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
		}
	}

	if (result >= 0)
		probe++;

findRecordInCategory:
	if (rH)
		MemHandleUnlock(rH);

	// At this point probe is the position where the string could be
	// inserted.  It is in between two entries.  Neither the record
	// before or after may have ANY letters in common, especially after
	// those records in other catergories are skipped.  Go with the
	// record that has the most letters in common.


	// Make sure the record returned is of the same category.
	// If not return the first prior record of the same category.
	probe2 = probe;
	if (!PrvAddrDBSeekVisibleRecordInCategory (dbP, &probe, 0, dmSeekForward, category, masked))
	{
		// Now count the number of matching characters in probe
		rH = DmQueryRecord(dbP, probe);      // No deleted record possible
		r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
		ErrFatalDisplayIf(r == 0, "Addr bsearch: data somehow missing");
		whichKey = 1;
		PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);
		if (recordKey == NULL)
			matches1 = 0;
		else
			matches1 = PrvAddrDBStrCmpMatches(key, recordKey);

		MemHandleUnlock(rH);
	}
	else
	{
		// No record in this category was found or probe is past all
		// records in this category.  Either way there aren't any matching
		// letters.
		matches1 = 0;
	}



	// Sometimes the record before has more matching letters. Check it.
	// Passing DmSeekRecordInCategory an offset of 1 doesn't work
	// when probe is at the end of the database and there isn't at least
	// one record to skip.
	probe2 = probe - 1;
	if (probe == 0 ||
		PrvAddrDBSeekVisibleRecordInCategory (dbP, &probe2, 0, dmSeekBackward, category, masked))
	{
		if (matches1 > 0)
		{
			// Go with probe because they have at least some letters in common.
			*recordP = probe;   //
			*completeMatch = (matches1 == StrLen(key));
			return true;
		}
		else
		{
			// probe has no letters in common and nothing earlier in this category
			// was found so this is a failed lookup.
			*completeMatch = false;
			return false;
		}
	}


	// Now count the number of matching characters in probe2
	rH = DmQueryRecord(dbP, probe2);      // No deleted record possible
	r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
	ErrFatalDisplayIf(r == 0, "Addr bsearch: data somehow missing");
	whichKey = 1;
	PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);
	if (recordKey == NULL)
		matches2 = 0;
	else
		matches2 = PrvAddrDBStrCmpMatches(key, recordKey);
	MemHandleUnlock(rH);


	// Now, return the probe which has the most letters in common.
	if (matches1 > matches2)
	{
		*completeMatch = (matches1 == StrLen(key));
		*recordP = probe;
	}
	else
		if (matches1 == 0 && matches2 == 0)
		{
			*completeMatch = false;
			return false;            // no item with same first letter found
		}
		else
		{
			// The first item matches as much or more as the second item
			*recordP = probe2;

			// If the prior item in the category has the same number of
			// matching letters use it instead.  Repeat to find the
			// earliest such match.
			while (!PrvAddrDBSeekVisibleRecordInCategory (dbP, &probe2, 1, dmSeekBackward, category, masked))
			{
				rH = DmQueryRecord(dbP, probe2);
				r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
				ErrFatalDisplayIf(r == 0, "Addr bsearch: data somehow missing");

				// Compare the string to the first sort key only
				whichKey = 1;
				PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);

				if (recordKey == NULL)
					matches1 = 0;
				else
					matches1 = PrvAddrDBStrCmpMatches(key, recordKey);

				MemHandleUnlock(rH);

				if (matches1 == matches2)
					*recordP = probe2;
				else
					break;
			}

			*completeMatch = (matches2 == StrLen(key));
		}

	return true;
}


/************************************************************
 *
 *  FUNCTION:    AddrDBLookupLookupString
 *
 *  DESCRIPTION: Return which record contains the most of
 *      the string passed.  If no string is passed or there
 *  aren't any records then false is returned.
 *
 *  PARAMETERS: address record
 *                key - string to lookup record with
 *                sortByCompany - how the db is sorted
 *                vars -  Lookup variables
 *                recordP - to contain the record found
 *                phoneP - to contain the phone found
 *                completeMatch -  true if a record contains all
 *                                 of the key
 *
 *  RETURNS: the record in recordP or false
 *             completeMatch -  true if a record contains all
 *                              of the key
 *
 * RETURNED:    false is return if a displayable record was not found.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/19/96   Initial Revision
 *
 *************************************************************/
Boolean AddrDBLookupLookupString(DmOpenRef dbP, Char * key, Boolean sortByCompany, AddressLookupFields field1, AddressLookupFields field2, UInt16 * recordP, Int16 * phoneP, AddressFields lookupFieldMap[], Boolean *completeMatch, Boolean *uniqueMatch)
{
	Int16                   numOfRecords;
	MemHandle                rH;
	PrvAddrPackedDBRecord*   r;
	//   UInt16                  kmin, i;                     // all positions in the database.
	UInt16                  probe, probe2         ;      // all positions in the database.
	Int16                  phoneProbe, phoneProbe2;
	//   Int16                   result;                     // result of comparing two records
	UInt16                   whichKey;
	char*                  recordKey;
	UInt16                   matches1, matches2;
	AddressFields         searchField;
	AddrDBRecordFlags      searchFieldFlag;


	*uniqueMatch = false;
	*completeMatch = false;

	// If there isn't a key to search with stop the with the first record.
	if (key == NULL || *key == '\0')
	{
		*completeMatch = true;
		return false;
	}

	numOfRecords = DmNumRecords(dbP);
	if (numOfRecords == 0)
		return false;

	// Performing a lookup on the sort field allows the use a binary search which
	// takes advantage of the ordered field.
	if (field1 == addrLookupSortField)
	{
		// Perform the standard lookup on the sort fields looking at all categories.
		if (!AddrDBLookupString(dbP, key, sortByCompany, dmAllCategories,
							  recordP, completeMatch, false))
			return false;   // nothing matched


		// At this point probe is the position where the string could be
		// inserted.  It is in between two entries.  Neither the record
		// before or after may have ANY letters in common, especially after
		// those records in other catergories are skipped.  Go with the
		// record that has the most letters in common.


		// Make sure the record returned is of the same category.
		// If not return the first prior record of the same category.
		probe2 = probe = *recordP;
		phoneProbe2 = phoneProbe = 0;
		if (AddrDBLookupSeekRecord (dbP, &probe, &phoneProbe, 0, dmSeekForward,
								  field1, field2, lookupFieldMap))
		{
			// Now count the number of matching characters in probe
			rH = DmQueryRecord(dbP, probe);      // No deleted record possible
			r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
			ErrFatalDisplayIf(r == 0, "AddrLookup bsearch: data somehow missing");
			whichKey = 1;
			PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);
			if (recordKey == NULL)
				matches1 = 0;
			else
				matches1 = PrvAddrDBStrCmpMatches(key, recordKey);

			MemHandleUnlock(rH);
		}
		else
		{
			// No record in this category was found or probe is past all
			// records in this category.  Either way there aren't any matching
			// letters.
			matches1 = 0;
		}


		*uniqueMatch = true;


		// Sometimes the record before has more matching letters. Check it.
		// Passing DmSeekRecordInCategory an offset of 1 doesn't work
		// when probe is at the end of the database and there isn't at least
		// one record to skip.
		probe2 = probe - 1;
		if (probe == 0 ||
			!AddrDBLookupSeekRecord (dbP, &probe2, &phoneProbe2, 0, dmSeekBackward,
								   field1, field2, lookupFieldMap))
		{
			// There isn't an earlier record.  Try to find a following record.
			probe2 = probe + 1;
			phoneProbe2 = phoneProbe;
			if (!AddrDBLookupSeekRecord (dbP, &probe2, &phoneProbe2, 0, dmSeekForward,
									   field1, field2, lookupFieldMap))
			{
				// There isn't a following record.  Try to use the probe.
				if (matches1 > 0)
				{
					// Go with probe because they have at least some letters in common.
					*recordP = probe;   //
					*phoneP = phoneProbe;
					*completeMatch = (matches1 == StrLen(key));
					return true;
				}
				else
				{
					// probe has no letters in common and nothing earlier in this category
					// was found so this is a failed lookup.
					*completeMatch = false;
					return false;
				}
			}
		}


		// Now count the number of matching characters in probe2
		rH = DmQueryRecord(dbP, probe2);      // No deleted record possible
		r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
		ErrFatalDisplayIf(r == 0, "AddrLookup bsearch: data somehow missing");
		whichKey = 1;
		PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);
		if (recordKey == NULL)
			matches2 = 0;
		else
			matches2 = PrvAddrDBStrCmpMatches(key, recordKey);
		MemHandleUnlock(rH);


		// Now, return the probe which has the most letters in common.
		if (matches1 > matches2)
		{
			*completeMatch = (matches1 == StrLen(key));
			*recordP = probe;
			*phoneP = phoneProbe;

			// If the next item has the same number of
			// matching letters then the match is not unique.
			probe2 = probe;
			phoneProbe2 = phoneProbe;
			if (AddrDBLookupSeekRecord (dbP, &probe2, &phoneProbe2, 1, dmSeekForward,
									  field1, field2, lookupFieldMap))
			{
				rH = DmQueryRecord(dbP, probe2);
				r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
				ErrFatalDisplayIf(r == 0, "AddrLookup bsearch: data somehow missing");

				// Compare the string to the first sort key only
				whichKey = 1;
				PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);

				if (recordKey == NULL)
					matches2 = 0;
				else
					matches2 = PrvAddrDBStrCmpMatches(key, recordKey);

				MemHandleUnlock(rH);

				if (matches1 <= matches2)
				{
					*uniqueMatch = false;
				}
			}
		}
		else
			if (matches1 == 0 && matches2 == 0)
			{
				*completeMatch = false;
				*uniqueMatch = false;
				return false;            // no item with same first letter found
			}
			else
			{
				// The first item matches as much or more as the second item
				*recordP = probe2;
				*phoneP = phoneProbe2;

				// If the prior item in the category has the same number of
				// matching letters use it instead.  Repeat to find the
				// earliest such match.
				while (AddrDBLookupSeekRecord (dbP, &probe2, &phoneProbe2, 1, dmSeekBackward,
											 field1, field2, lookupFieldMap))
				{
					rH = DmQueryRecord(dbP, probe2);
					r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
					ErrFatalDisplayIf(r == 0, "AddrLookup bsearch: data somehow missing");

					// Compare the string to the first sort key only
					whichKey = 1;
					PrvAddrDBFindKey(r, &recordKey, &whichKey, sortByCompany);

					if (recordKey == NULL)
						matches1 = 0;
					else
						matches1 = PrvAddrDBStrCmpMatches(key, recordKey);

					MemHandleUnlock(rH);

					if (matches1 == matches2)
					{
						*recordP = probe2;
						*phoneP = phoneProbe2;
					}
					else
						break;
				}

				*completeMatch = (matches2 == StrLen(key));
				*uniqueMatch = false;
			}

		return true;

	}
	else
	{
		// Peform a lookup based on unordered data.  This gets real slow with lots of data
		// Because to check for uniqueness we must search every record.  This means on average
		// this lookup is twice as slow as it would be it it could stop with the first match.
		AddrDBRecordType record;


		*completeMatch = false;

		matches1 = 0;         // treat this as the most matches

		// cache these values
		searchField = lookupFieldMap[field1];
		searchFieldFlag.allBits = BitAtPosition(field1);

		// Start with the first record and look at each record until there are no more.
		// Look for the record with the most number of matching records.  Even if we found
		// a record containing all the record we are searching for we must still look
		// for one more complete match to confirm or deny uniqueness of the match.
		probe2 = 0;
		phoneProbe2 = 0;
		while (AddrDBLookupSeekRecord (dbP, &probe2, &phoneProbe2, 1, dmSeekForward,
									 field1, field2, lookupFieldMap))
		{
			rH = DmQueryRecord(dbP, probe2);
			r = (PrvAddrPackedDBRecord *) MemHandleLock(rH);
			ErrFatalDisplayIf(r == 0, "AddrLookup bsearch: data somehow missing");

			// Compare the string to the search field
			if (r->flags.allBits & searchFieldFlag.allBits)
			{
				PrvAddrDBUnpack(r, &record);
				recordKey = record.fields[searchField];

				if (recordKey == NULL)
					matches2 = 0;
				else
					matches2 = PrvAddrDBStrCmpMatches(key, recordKey);
			}
			else
			{
				matches2 = 0;
			}

			MemHandleUnlock(rH);

			if (matches2 > matches1)
			{
				matches1 = matches2;      // the most matches so far

				*recordP = probe2;      // return the best record
				*phoneP = phoneProbe2;

				*completeMatch = (matches2 == StrLen(key));
			}
			// Did we find another record which is a complete match?
			else if (matches2 > 0 &&
					 matches1 == matches2 &&
					 *completeMatch)
			{
				*uniqueMatch = false;
				return true;
			}
			else
			{
				// The record is a matching failure.  Since AddrLookupSeekRecord is going
				// to return this record again for every phone field we cheat by specifying
				// the last phone field to skip all other entries.
				//            phoneProbe2 = numPhoneFields - 1;
			}
		}


		// Was at least one record found with at least one matching character?
		if (matches1 > 0)
		{
			// At this point every record was searched and no other match was found.
			*uniqueMatch = true;

			return true;
		}
	}

	return false;
}


/***********************************************************************
 *
 * FUNCTION:     AddrDBGetDatabase
 *
 * DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     Err - zero if no error, else the error
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			jmp		10/01/99	Initial Revision
 *
 ***********************************************************************/
Err AddrDBGetDatabase (DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
	UInt16 cardNo;
	LocalID dbID;

	*dbPP = NULL;

	// Find the application's data file.  If it doesn't exist create it.
	dbP = DmOpenDatabaseByTypeCreator (addrDBType, AddressBookCreator, mode);
	if (!dbP)
	{
		error = DmCreateDatabase (0, addrDBName, AddressBookCreator, addrDBType, false);
		if (error)
			return error;

		dbP = DmOpenDatabaseByTypeCreator(addrDBType, AddressBookCreator, mode);
		if (!dbP)
			return (1);

		// Set the backup bit.  This is to aid syncs with non Palm software.
		ToolsSetDBAttrBits(dbP, dmHdrAttrBackup);

		error = AddrDBAppInfoInit (dbP);
		if (error)
		{
			DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL);
			DmCloseDatabase(dbP);
			DmDeleteDatabase(cardNo, dbID);
			return error;
		}
	}

	*dbPP = dbP;
	return 0;
}

//#pragma mark -

/************************************************************
 *
 *  FUNCTION: PrvAddrDBLocalizeAppInfo
 *
 *  DESCRIPTION: Look for localize app info strings and copy
 *  them into the app info block.
 *
 *  PARAMETERS: application info ptr
 *
 *  RETURNS: nothing
 *
 *  CREATED: 12/13/95
 *
 *  BY: Roger Flores
 *
 *  MODIFICATIONS:
 *      10/22/96   roger      Set flags when field modified
 *************************************************************/
void PrvAddrDBLocalizeAppInfo(AddrAppInfoPtr appInfoP)
{
	MemHandle       localizedAppInfoH;
	Char *          localizedAppInfoP;
	//AddrAppInfoPtr   nilP = 0;
	MemHandle       stringsH;
	Char *         *stringsP;
	int             i;
	UInt16            localRenamedCategories;
	UInt32            localDirtyFieldLabels;


	localizedAppInfoH = DmGetResource(appInfoStringsRsc, LocalizedAppInfoStr);
	if (!localizedAppInfoH)
		return;
	localizedAppInfoP = MemHandleLock(localizedAppInfoH);
	stringsH = SysFormPointerArrayToStrings(localizedAppInfoP,
											dmRecNumCategories + addrNumFields + numPhoneLabelsStoredSecond);
	stringsP = MemHandleLock(stringsH);


	// Copy each category
	localRenamedCategories = appInfoP->renamedCategories;
	for (i = 0; i < dmRecNumCategories; i++)
	{
		if (stringsP[i][0] != '\0')
		{
			//DmStrCopy(appInfoP, (Int32) nilP->categoryLabels[i], stringsP[i]);
			DmStrCopy(appInfoP, OffsetOf(AddrAppInfoType, categoryLabels[i]), stringsP[i]);
			SetBitMacro(localRenamedCategories, i);
		}
	}
	//DmWrite(appInfoP, (Int32) &nilP->renamedCategories, &localRenamedCategories,
	DmWrite(appInfoP, OffsetOf(AddrAppInfoType, renamedCategories), &localRenamedCategories,
			sizeof(localRenamedCategories));


	// Copy each field label
	localDirtyFieldLabels = appInfoP->dirtyFieldLabels.allBits;
	for (i = 0; i < (addrNumFields + numPhoneLabelsStoredSecond); i++)
	{
		if (stringsP[i + dmRecNumCategories][0] != '\0')
		{
			//DmStrCopy(appInfoP, (Int32) nilP->fieldLabels[i],
			DmStrCopy(appInfoP, OffsetOf(AddrAppInfoType, fieldLabels[i]),
					  stringsP[i + dmRecNumCategories]);
			SetBitMacro(localDirtyFieldLabels, i);
		}
	}
	//DmWrite(appInfoP, (Int32) &nilP->dirtyFieldLabels.allBits, &localDirtyFieldLabels,
	DmWrite(appInfoP, OffsetOf(AddrAppInfoType, dirtyFieldLabels.allBits), &localDirtyFieldLabels,
			sizeof(localDirtyFieldLabels));


	MemPtrFree(stringsP);
	MemPtrUnlock(localizedAppInfoP);
	DmReleaseResource(localizedAppInfoH);
}

/*
typedef struct {
  UInt16       renamedCategories;      // bitfield of categories with a different name
  char         categoryLabels[dmRecNumCategories][dmCategoryLength];
  UInt8        categoryUniqIDs[dmRecNumCategories];
  UInt8        lastUniqID;     // Uniq IDs generated by the device are between 0 - 127.  Those from the PC are 128 - 255.
  UInt8        reserved1;      // from the compiler word aligning things
  UInt16       reserved2;
  AddrDBFieldLabelsDirtyFlags dirtyFieldLabels;
  addressLabel fieldLabels[addrNumFields + numPhoneLabelsStoredSecond];
  CountryType  country;                // Country the database (labels) is formatted for
  UInt8        reserved;
  AddrDBMisc   misc;
} AddrAppInfoType;
*/


/************************************************************
 *
 *  FUNCTION: PrvAddrDBFindKey
 *
 *  DESCRIPTION: Return the next valid key
 *
 *  PARAMETERS: database packed record
 *            <-> key to use (ptr to string or NULL for uniq ID)
 *            <-> which key (incremented for use again, starts at 1)
 *            -> sortByCompany
 *
 *  RETURNS:
 *
 *  CREATED: 1/16/95
 *
 *  BY: Roger Flores
 *
 *   COMMENTS:   Returns the key which is asked for if possible and
 *   advances whichKey.  If the key is not available the key advances
 *   to the next one.  The order of keys is:
 *
 * if sortByCompany:
 *      companyKey, nameKey, firstNameKey, uniq ID
 *
 * if !sortByCompany:
 *      nameKey, firstNameKey, companyKey (if no name or first name), uniq ID
 *
 *
 *************************************************************/
void PrvAddrDBFindKey(PrvAddrPackedDBRecord *r, char **key, UInt16 *whichKey, Int16 sortByCompany)
{
	AddrDBRecordFlags fieldFlags;

//SysDebug(1, "Addr", "PrvAddrDBFindKey begin");
	fieldFlags.allBits = r->flags.allBits;
//SysDebug(1, "Addr", "PrvAddrDBFindKey allBits 0x%08lX", fieldFlags.allBits);

	ErrFatalDisplayIf(*whichKey == 0 || *whichKey == 5, "Bad addr key");

	if (sortByCompany)
	{
//SysDebug(1, "Addr", "PrvAddrDBFindKey sortByCompany");
		//if (*whichKey == 1 && fieldFlags.bits.company)
		if (*whichKey == 1 && GetBitMacro(fieldFlags.allBits, ad_company))
		{
			*whichKey = 2;
			goto returnCompanyKey;
		}

		//if (*whichKey <= 2 && fieldFlags.bits.name)
		if (*whichKey <= 2 && GetBitMacro(fieldFlags.allBits, ad_name))
		{
			*whichKey = 3;
			goto returnNameKey;
		}

		//if (*whichKey <= 3 && fieldFlags.bits.firstName)
		if (*whichKey <= 3 && GetBitMacro(fieldFlags.allBits, ad_firstName))
		{
			*whichKey = 4;
			goto returnFirstNameKey;
		}
	}
	else
	{
//SysDebug(1, "Addr", "PrvAddrDBFindKey not sortByCompany");
		//if (*whichKey == 1 && fieldFlags.bits.name)
		if (*whichKey == 1 && GetBitMacro(fieldFlags.allBits, ad_name))
		{
			*whichKey = 2;
//SysDebug(1, "Addr", "PrvAddrDBFindKey whichKey 1");
			goto returnNameKey;
		}

		//if (*whichKey <= 2 && fieldFlags.bits.firstName)
		if (*whichKey <= 2 && GetBitMacro(fieldFlags.allBits, ad_firstName))
		{
			*whichKey = 3;
//SysDebug(1, "Addr", "PrvAddrDBFindKey whichKey 2");
			goto returnFirstNameKey;
		}

//SysDebug(1, "Addr", "PrvAddrDBFindKey whichKey >= 3");

		// For now don't consider company name when sorting by person name
		// unless there isn't a name or firstName
		//if (*whichKey <= 3 && fieldFlags.bits.company &&
			//!(fieldFlags.bits.name || fieldFlags.bits.firstName))
		if (*whichKey <= 3 && GetBitMacro(fieldFlags.allBits, ad_company) &&
		        !(GetBitMacro(fieldFlags.allBits, ad_name) || !GetBitMacro(fieldFlags.allBits, ad_firstName)))
		{
			*whichKey = 4;
			goto returnCompanyKey;
		}

	}

	// All possible fields have been tried so return NULL so that
	// the uniq ID is compared.
	*whichKey = 5;
	*key = NULL;
//SysDebug(1, "Addr", "PrvAddrDBFindKey NULL end");
	return;



returnCompanyKey:
	*key = (char *) &r->companyFieldOffset + r->companyFieldOffset;
//SysDebug(1, "Addr", "PrvAddrDBFindKey returnCompanyKey end");
	return;


returnNameKey:
	*key = &r->firstField;
//SysDebug(1, "Addr", "PrvAddrDBFindKey returnNameKey \"%s\" end", *key ? *key : "NULL");
	return;


returnFirstNameKey:
	*key = &r->firstField;
//SysDebug(1, "Addr", "PrvAddrDBFindKey returnFirstNameKey firstField \"%s\"", *key ? *key : "NULL");
	//if (r->flags.bits.name)
	if (GetBitMacro(r->flags.allBits, ad_name))
	{
		*key += StrLen(*key) + 1;
//SysDebug(1, "Addr", "PrvAddrDBFindKey returnFirstNameKey nextField \"%s\"", *key ? *key : "NULL");
	}
//SysDebug(1, "Addr", "PrvAddrDBFindKey returnFirstNameKey \"%s\" end", *key ? *key : "NULL");
	return;

}


/************************************************************
 *
 *	FUNCTION:	PrvAddrDBComparePackedRecords
 *
 *	DESCRIPTION: Compare two packed records  key by key until
 *		there is a difference.  Return -1 if r1 is less or 1 if r2
 *		is less.  A zero may be returned if two records seem
 *		identical. NULL fields are considered less than others.
 *
 *	PARAMETERS:	address record 1
 *            address record 2
 *
 *	RETURNS: -1 if record one is less
 *           1 if record two is less
 *
 *	HISTORY:
 *		01/14/95	rsf	Created by Roger Flores.
 *		11/30/00	kwk	Only call StrCompare, not StrCaselessCompare
 *							first and then StrCompare. Also use TxtCompare
 *							instead of StrCompare, to skip a trap call.
 *
 *************************************************************/
Int16 PrvAddrDBComparePackedRecords(PrvAddrPackedDBRecord *r1,
												PrvAddrPackedDBRecord *r2,
												Int16 sortByCompany,
												SortRecordInfoPtr UNUSED_PARAM(info1),
												SortRecordInfoPtr UNUSED_PARAM(info2),
												MemHandle UNUSED_PARAM(appInfoH))
{
	UInt16 whichKey1, whichKey2;
	char *key1, *key2;
	Int16 result;

//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords begin");
	whichKey1 = 1;
	whichKey2 = 1;

	do {
#ifdef PALMOS
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords r1=0x%08lX r2=0x%08lX", ((UInt32)r1), ((UInt32)r2));
#else
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords r1=%p r2=%p", r1, r2);
#endif
//SysDebugBytes(1, "Addr", r1, MemHandleSize(MemPtrRecoverHandle(r1)));
//SysDebugBytes(1, "Addr", r2, MemHandleSize(MemPtrRecoverHandle(r2)));
		PrvAddrDBFindKey(r1, &key1, &whichKey1, sortByCompany);
		PrvAddrDBFindKey(r2, &key2, &whichKey2, sortByCompany);
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords key1=\"%s\" key2=\"%s\"", key1 ? key1 : "NULL", key2 ? key2 : "NULL");

		// A key with NULL loses the StrCompare.
		if (key1 == NULL) {
			// If both are NULL then return them as equal
			if (key2 == NULL) {
				result = 0;
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords key1 null, key2 null return %d", result);
				return result;
			} else {
				result = -1;
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords key1 null, key2 not null result %d", result);
      }
		} else
			if (key2 == NULL) {
				result = 1;
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords key1 not null, key2 null result %d", result);
      } else {
				// With Palm OS 4.0, StrCompare will try a caseless
				// comparison first, then a case-sensitive, so we
				// only need to call StrCompare. Also, we can call
				// TxtCompare to avoid one extra trap dispatch.
				
				// result = StrCaselessCompare(key1, key2);
				// if (result == 0)
				//		result = StrCompare(key1, key2);
				
				result = TxtCompare(	key1,		// const Char *s1,
											0xFFFF,	// UInt16 s1Len,
											NULL,		// UInt16 *s1MatchLen,
											key2,		// const Char *s2,
											0xFFFF,	// UInt16 s2Len,
											NULL);	// UInt16 *s2MatchLen
//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords TxtCompare \"%s\" \"%s\" result %d", key1 ? key1 : "NULL", key2 ? key2 : "NULL", result);
			}

	} while (!result);


//SysDebug(1, "Addr", "PrvAddrDBComparePackedRecords return %d", result);
	return result;
}


/************************************************************
 *
 *  FUNCTION: PrvAddrDBUnpackedSize
 *
 *  DESCRIPTION: Return the size of an AddrDBRecordType
 *
 *  PARAMETERS: address record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Int16 PrvAddrDBUnpackedSize(AddrDBRecordPtr r)
{
	Int16 size;
	Int16   index;

	//size = sizeof (PrvAddrPackedDBRecord) - sizeof (char);   // correct
	size = 4 + 4 + 1;
	for (index = firstAddressField; index < ad_addressFieldsCount; index++)
	{
		if (r->fields[index] != NULL)
			size += StrLen(r->fields[index]) + 1;
	}
	return size;
}


/************************************************************
 *
 *  FUNCTION: PrvAddrDBPack
 *
 *  DESCRIPTION: Pack an AddrDBRecordType.  Doesn't pack empty strings.
 *
 *  PARAMETERS: address record to pack
 *                address record to pack into
 *
 *  RETURNS: the PrvAddrPackedDBRecord is packed
 *
 *  CREATED: 1/10/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
void PrvAddrDBPack(AddrDBRecordPtr s, void * recordP)
{
	Int32                offset;
	AddrDBRecordFlags    flags;
	Int16                index;
	//PrvAddrPackedDBRecord*  d=0;
	Int16                len;
	void *               srcP;
	UInt8                companyFieldOffset;

	flags.allBits = 0;

//SysDebug(1, "Addr", "PrvAddrDBPack begin");
	//DmWrite(recordP, (Int32)&d->options, &s->options, sizeof(s->options));
//SysDebug(1, "Addr", "PrvAddrDBPack options 0x%08lX offset %ld", s->options, OffsetOf(PrvAddrPackedDBRecord, options));
	DmWrite(recordP, OffsetOf(PrvAddrPackedDBRecord, options), &s->options, sizeof(s->options));
	//offset = (Int32)&d->firstField;
	offset = OffsetOf(PrvAddrPackedDBRecord, firstField);

	for (index = firstAddressField; index < ad_addressFieldsCount; index++) {
		if (s->fields[index] != NULL)
			/*         if (s->fields[index][0] == '\0')
			 {
			 // so set the companyFieldOffset or clear it code doesn't fail
			 s->fields[index] = NULL;
			 }
			 else
			 */
		{
			ErrFatalDisplayIf(s->fields[index][0] == '\0' && index != ad_note,
							  "Empty field being added");
			srcP = s->fields[index];
			len = StrLen(srcP) + 1;
//SysDebug(1, "Addr", "PrvAddrDBPack field %d len %d \"%s\" offset %ld", index, len, srcP ? srcP : "NULL", offset);
			DmWrite(recordP, offset, srcP, len);
			offset += len;
//SysDebug(1, "Addr", "PrvAddrDBPack before bits 0x%08lX", flags.allBits);
			SetBitMacro(flags.allBits, index);
//SysDebug(1, "Addr", "PrvAddrDBPack after  bits 0x%08lX", flags.allBits);
		}
	}

	// Set the flags indicating which fields are used
	//DmWrite(recordP, (Int32)&d->flags.allBits, &flags.allBits, sizeof(flags.allBits));
//SysDebug(1, "Addr", "PrvAddrDBPack flags 0x%08lX offset %ld", flags.allBits, OffsetOf(PrvAddrPackedDBRecord, flags.allBits));
	DmWrite(recordP, OffsetOf(PrvAddrPackedDBRecord, flags.allBits), &flags.allBits, sizeof(flags.allBits));

	// Set the companyFieldOffset or clear it
	if (s->fields[ad_company] == NULL)
		companyFieldOffset = 0;
	else {
		index = 1;
		if (s->fields[ad_name] != NULL)
			index += StrLen(s->fields[ad_name]) + 1;
		if (s->fields[ad_firstName] != NULL)
			index += StrLen(s->fields[ad_firstName]) + 1;
		companyFieldOffset = (UInt8) index;
	}
	//DmWrite(recordP, (Int32)(&d->companyFieldOffset), &companyFieldOffset, sizeof(companyFieldOffset));
	DmWrite(recordP, OffsetOf(PrvAddrPackedDBRecord, companyFieldOffset), &companyFieldOffset, sizeof(companyFieldOffset));
//SysDebug(1, "Addr", "PrvAddrDBPack end");
}


/************************************************************
 *
 *  FUNCTION: PrvAddrDBUnpack
 *
 *  DESCRIPTION: Fills in the AddrDBRecord structure
 *
 *  PARAMETERS: address record to unpack
 *                the address record to unpack into
 *
 *  RETURNS: the record unpacked
 *
 *  CREATED: 1/14/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
void PrvAddrDBUnpack(PrvAddrPackedDBRecord *src, AddrDBRecordPtr dest)
{
	Int16   index;
	UInt32 flags;
	char *p;


	dest->options = src->options;
	flags = src->flags.allBits;
	p = &src->firstField;


	for (index = firstAddressField; index < ad_addressFieldsCount; index++)
	{
		// If the flag is set point to the string else NULL
		if (GetBitMacro(flags, index) != 0)
		{
			dest->fields[index] = p;
			p += StrLen(p) + 1;
		}
		else
			dest->fields[index] = NULL;
	}
}


/************************************************************
 *
 *  FUNCTION: PrvAddrDBFindSortPosition
 *
 *  DESCRIPTION: Return where a record is or should be
 *      Useful to find or find where to insert a record.
 *
 *  PARAMETERS: address record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/11/95
 *
 *  BY: Roger Flores
 *
 *************************************************************/
UInt16 PrvAddrDBFindSortPosition(DmOpenRef dbP, PrvAddrPackedDBRecord *newRecord)
{
	Int16 sortByCompany;
	AddrAppInfoPtr appInfoPtr;
Int16 r;


	appInfoPtr = (AddrAppInfoPtr) AddrDBAppInfoGetPtr(dbP);
	sortByCompany = appInfoPtr->misc.sortByCompany;
	MemPtrUnlock(appInfoPtr);

#ifdef PALMOS
  //SysDebug(1, "Addr", "PrvAddrDBFindSortPosition DmFindSortPosition 0x%08lX ...", ((UInt32)newRecord));
#else
  //SysDebug(1, "Addr", "PrvAddrDBFindSortPosition DmFindSortPosition %p ...", newRecord);
#endif
	r = DmFindSortPosition(dbP, (void *) newRecord, NULL, (DmComparF *) PrvAddrDBComparePackedRecords, (Int16) sortByCompany);
  //SysDebug(1, "Addr", "PrvAddrDBFindSortPosition DmFindSortPosition: %d", r);
  return r;
}



/************************************************************
 *
 *  FUNCTION: PrvAddrDBStrCmpMatches
 *
 *  DESCRIPTION: Compares two strings and reports the number
 *  of matching bytes from the start of <s1>.
 *
 *  PARAMETERS: 2 string pointers
 *
 *  RETURNS: number of matching bytes from <s1>.
 *
 *  CREATED: 6/15/95
 *
 *  BY: Roger Flores
 *
 * HISTORY:
 *		06/15/95	rsf	Created by Roger Flores.
 *		05/16/99	kwk	Use TxtCaselessCompare routine.
 *		11/30/00	kwk	Speed things up by not calling StrLen().
 *
 *************************************************************/
UInt16 PrvAddrDBStrCmpMatches(const Char* s1, const Char* s2)
{
	UInt16 matches;

	ErrFatalDisplayIf ( s1 == NULL, "Error NULL string parameter");
	ErrFatalDisplayIf ( s2 == NULL, "Error NULL string parameter");

	// In Palm OS 4.0, TxtCaselessCompare now handles null-terminated
	// strings, so the actual string lengths don't need to be passed in.
	// Instead we'll pass kMaxUInt16 (doesn't exist, use 0xFFFF).
	// On pre-4.0 ROMs, calls to StrLen(s1) & StrLen(s2) would still
	// be necessary.
	TxtCaselessCompare(s1, 0xFFFF, &matches, s2, 0xFFFF, NULL);
	return (matches);
}


/***********************************************************************
 *
 * FUNCTION:    PrvAddrDBRecordContainsField
 *
 * DESCRIPTION: Check if a packed record contains a desired field.
 *
 * PARAMETERS:  recordP  - pointer to the record to search
 *              field - type of field to find.
 *
 * RETURNED:    true if the record contains the field.
 *
 * REVISION HISTORY:
 *         Name   Date      Description
 *         ----   ----      -----------
 *         Roger   7/9/96   Initial Revision
 *
 ***********************************************************************/
Boolean PrvAddrDBRecordContainsField(PrvAddrPackedDBRecord *packedRecordP, AddressLookupFields field, Int16 * phoneP, Int16 direction, AddressFields lookupFieldMap[])
{
	int index;
	int stopIndex;
	int phoneType;


	switch (field)
	{
	case addrLookupSortField:
		return packedRecordP->flags.allBits & sortKeyFieldBits;

	case addrLookupListPhone:
		return GetBitMacro(packedRecordP->flags.allBits, firstPhoneField +
						   packedRecordP->options.phones.displayPhoneForList);

	case addrLookupNoField:
		return true;

	default:
		if (!IsPhoneLookupField(field))
			return GetBitMacro(packedRecordP->flags.allBits, lookupFieldMap[field]) != 0;

		phoneType = field - addrLookupWork;
		index = firstPhoneField + *phoneP;
		if (direction == dmSeekForward)
			stopIndex = lastPhoneField + direction;
		else
			stopIndex = firstPhoneField + direction;

		while (index != stopIndex)
		{
			// If the phone field is the type requested and it's not empty
			// return it.
			if (GetPhoneLabel(packedRecordP, index) == phoneType &&
				GetBitMacro(packedRecordP->flags.allBits, index))
			{
				*phoneP = index - firstPhoneField;
				return true;
			}
			index += direction;
		}

		// The phone type wasn't used.
		if (direction == dmSeekForward)
			*phoneP = 0; 						     // Reset for the next record
		else
			*phoneP = numPhoneFields - 1;      // Reset for the next record

		return false;
	}
}



/************************************************************
 *
 *  FUNCTION: PrvAddrDBSeekVisibleRecordInCategory
 *
 *  DESCRIPTION: Like DmSeekRecordInCategory, but if masked is true
 *						also explicitly skips past private records
 *
 *  PARAMETERS: masked - indicates that database is opened in show secret mode
 *							but should be hide secret.
 *
 *  RETURNS: as DmSeekRecordInCategory
 *
 *  CREATED: 6/15/99
 *
 *  BY: Jameson Quinn
 *
 *************************************************************/
Boolean PrvAddrDBSeekVisibleRecordInCategory (DmOpenRef dbR, UInt16 * indexP, UInt16 offset, Int16 direction, UInt16 category, Boolean masked)
{
	UInt16		attr;
	Boolean result;

	result = DmSeekRecordInCategory(dbR,indexP,offset,direction,category);

	if (result != errNone)
	{
		goto Exit;
	}

	DmRecordInfo (dbR, *indexP, &attr, NULL, NULL);

	while (masked && (attr & dmRecAttrSecret))
	{
		result = DmSeekRecordInCategory(dbR,indexP,1,direction,category);

		if (result != errNone)
		{
			goto Exit;
		}

		DmRecordInfo (dbR, *indexP, &attr, NULL, NULL);
	}

Exit:
	return result;
}
