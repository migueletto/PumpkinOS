/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddressDB.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRESSDB_H
#define ADDRESSDB_H

#include "AddrDefines.h"

#include <AppLaunchCmd.h>
#include <CoreTraps.h>
#include <LocaleMgr.h>
#include <DataMgr.h>


/***********************************************************************
 *   Defines
 ***********************************************************************/

#define firstAddressField			ad_name
#define firstPhoneField				ad_phone1
#define lastPhoneField				ad_phone5
#define numPhoneFields				(lastPhoneField - firstPhoneField + 1)
#define numPhoneLabelsStoredFirst	numPhoneFields
#define numPhoneLabelsStoredSecond	(numPhoneLabels - numPhoneLabelsStoredFirst)

#define firstRenameableLabel		ad_custom1
#define lastRenameableLabel			ad_custom4
#define lastLabel					(ad_addressFieldsCount + numPhoneLabelsStoredSecond)

#define IsPhoneLookupField(p)		(addrLookupWork <= (p) && (p) <= addrLookupMobile)

#define addrLabelLength				16


#define GetBitMacro(bitfield, index)      ((bitfield) & BitAtPosition(index))
#define SetBitMacro(bitfield, index)      ((bitfield) |= BitAtPosition(index))
#define RemoveBitMacro(bitfield, index)   ((bitfield) &= ~BitAtPosition(index))

/***********************************************************************
 *   Internal Structures
 ***********************************************************************/

/*
typedef union {
	struct {
#if 0
		unsigned reserved	:13;
		unsigned note		:1;	// set if record contains a note handle
		unsigned custom4	:1;	// set if record contains a custom4
		unsigned custom3	:1;	// set if record contains a custom3
		unsigned custom2	:1;	// set if record contains a custom2
		unsigned custom1	:1;	// set if record contains a custom1
		unsigned title		:1;	// set if record contains a title
		unsigned country	:1;	// set if record contains a birthday
		unsigned zipCode	:1;	// set if record contains a birthday
		unsigned state		:1;	// set if record contains a birthday
		unsigned city		:1;	// set if record contains a birthday
		unsigned address	:1;	// set if record contains a address
		unsigned phone5		:1;	// set if record contains a phone5
		unsigned phone4		:1;	// set if record contains a phone4
		unsigned phone3		:1;	// set if record contains a phone3
		unsigned phone2		:1;	// set if record contains a phone2
		unsigned phone1		:1;	// set if record contains a phone1
		unsigned company	:1;	// set if record contains a company
		unsigned firstName	:1;	// set if record contains a firstName
		unsigned name		:1;	// set if record contains a name (bit 0)
#endif
		UInt32 name		:1;	// set if record contains a name (bit 0)
		UInt32 firstName	:1;	// set if record contains a firstName
		UInt32 company		:1;	// set if record contains a company
		UInt32 phone1		:1;	// set if record contains a phone1
		UInt32 phone2		:1;	// set if record contains a phone2
		UInt32 phone3		:1;	// set if record contains a phone2
		UInt32 phone4		:1;	// set if record contains a phone2
		UInt32 address		:1;	// set if record contains a address
		UInt32 city		:1;	// set if record contains a birthday
		UInt32 state		:1;	// set if record contains a birthday
		UInt32 zipCode		:1;	// set if record contains a birthday
		UInt32 country		:1;	// set if record contains a birthday
		UInt32 title		:1;	// set if record contains a title
		UInt32 custom1		:1;	// set if record contains a custom1
		UInt32 custom2		:1;	// set if record contains a custom2
		UInt32 custom3		:1;	// set if record contains a custom3
		UInt32 custom4		:1;	// set if record contains a custom4
		UInt32 note		:1;	// set if record contains a note handle
		UInt32 reserved		:13;

	} bits;
	UInt32 allBits;
} AddrDBRecordFlags;
*/

typedef struct {
  UInt32 allBits;
} AddrDBRecordFlags;

/*
typedef union {
	struct {
#if 0
		unsigned reserved	 :10;
		unsigned phone8      :1;	// set if phone8 label is dirty
		unsigned phone7      :1;	// set if phone7 label is dirty
		unsigned phone6      :1;	// set if phone6 label is dirty
		unsigned note        :1;	// set if note label is dirty
		unsigned custom4     :1;	// set if custom4 label is dirty
		unsigned custom3     :1;	// set if custom3 label is dirty
		unsigned custom2     :1;	// set if custom2 label is dirty
		unsigned custom1     :1;	// set if custom1 label is dirty
		unsigned title       :1;	// set if title label is dirty
		unsigned country	 :1;	// set if country label is dirty
		unsigned zipCode	 :1;	// set if zipCode label is dirty
		unsigned state		 :1;	// set if state label is dirty
		unsigned city		 :1;	// set if city label is dirty
		unsigned address     :1;	// set if address label is dirty
		unsigned phone5      :1;	// set if phone5 label is dirty
		unsigned phone4      :1;	// set if phone4 label is dirty
		unsigned phone3      :1;	// set if phone3 label is dirty
		unsigned phone2      :1;	// set if phone2 label is dirty
		unsigned phone1      :1;	// set if phone1 label is dirty
		unsigned company     :1;	// set if company label is dirty
		unsigned firstName   :1;	// set if firstName label is dirty
		unsigned name        :1;	// set if name label is dirty (bit 0)
#endiF

		UInt32 name        :1;	// set if name label is dirty (bit 0)
		UInt32 firstName   :1;	// set if firstName label is dirty
		UInt32 company     :1;	// set if company label is dirty
		UInt32 phone1      :1;	// set if phone1 label is dirty
		UInt32 phone2      :1;	// set if phone2 label is dirty
		UInt32 phone3      :1;	// set if phone3 label is dirty
		UInt32 phone4      :1;	// set if phone4 label is dirty
		UInt32 phone5      :1;	// set if phone5 label is dirty
		UInt32 address     :1;	// set if address label is dirty
		UInt32 city		 :1;	// set if city label is dirty
		UInt32 state		 :1;	// set if state label is dirty
		UInt32 zipCode	 :1;	// set if zipCode label is dirty
		UInt32 country	 :1;	// set if country label is dirty
		UInt32 title       :1;	// set if title label is dirty
		UInt32 custom1     :1;	// set if custom1 label is dirty
		UInt32 custom2     :1;	// set if custom2 label is dirty
		UInt32 custom3     :1;	// set if custom3 label is dirty
		UInt32 custom4     :1;	// set if custom4 label is dirty
		UInt32 note        :1;	// set if note label is dirty
		UInt32 phone6      :1;	// set if phone6 label is dirty
		UInt32 phone7      :1;	// set if phone7 label is dirty
		UInt32 phone8      :1;	// set if phone8 label is dirty
		UInt32 reserved	 :10;

	} bits;
	UInt32 allBits;
} AddrDBFieldLabelsDirtyFlags;
*/

typedef struct {
  UInt32 allBits;
} AddrDBFieldLabelsDirtyFlags;

typedef struct
{
	//unsigned reserved:7;
	//unsigned sortByCompany	:1;
	UInt8 sortByCompany	:1;
	UInt8 reserved:7;
} AddrDBMisc;

/*
typedef enum
{
	name,
	firstName,
	company,
	phone1,
	phone2,
	phone3,
	phone4,
	phone5,
	address,
	city,
	state,
	zipCode,
	country,
	title,
	custom1,
	custom2,
	custom3,
	custom4,
	note,			// This field is assumed to be < 4K
	addressFieldsCount
} AddressFields;
*/

#define ad_name      0
#define ad_firstName 1
#define ad_company   2
#define ad_phone1    3
#define ad_phone2    4
#define ad_phone3    5
#define ad_phone4    6
#define ad_phone5    7
#define ad_address   8
#define ad_city      9
#define ad_state     10
#define ad_zipCode   11
#define ad_country   12
#define ad_title     13
#define ad_custom1   14
#define ad_custom2   15
#define ad_custom3   16
#define ad_custom4   17
#define ad_note      18
#define ad_addressFieldsCount 19

typedef UInt8 AddressFields;


/*
// This structure is only for the exchange of address records.
typedef union
{
	struct
	{
		unsigned reserved		:8;

		// Typically only one of these are set
		unsigned email			:1;	// set if data is an email address
		unsigned fax			:1;	// set if data is a fax
		unsigned pager			:1;	// set if data is a pager
		unsigned voice			:1;	// set if data is a phone

		unsigned mobile			:1;	// set if data is a mobile phone

		// These are set in addition to other flags.
		unsigned work			:1;	// set if phone is at work
		unsigned home			:1;	// set if phone is at home

		// Set if this number is preferred over others.  May be preferred
		// over all others.  May be preferred over other emails.  One
		// preferred number should be listed next to the person's name.
		unsigned preferred   	:1;	// set if this phone is preferred (bit 0)
	} bits;
	UInt32 allBits;
} AddrDBPhoneFlags;
*/

typedef enum
{
	workLabel,
	homeLabel,
	faxLabel,
	otherLabel,
	emailLabel,
	mainLabel,
	pagerLabel,
	mobileLabel
} AddressPhoneLabels;


typedef union
{
	struct
	{
/*
		unsigned reserved:8;
		unsigned displayPhoneForList:4;	// The phone displayed for the list view 0 - 4
		unsigned phone5:4;				// Which phone (home, work, car, ...)
		unsigned phone4:4;
		unsigned phone3:4;
		unsigned phone2:4;
		unsigned phone1:4;
*/
		UInt32 reserved:8;
		UInt32 phone5:4;
		UInt32 displayPhoneForList:4;	// The phone displayed for the list view 0 - 4
		UInt32 phone3:4;
		UInt32 phone4:4;
		UInt32 phone1:4;
		UInt32 phone2:4;
	} phones;
	UInt32 phoneBits;
} AddrOptionsType;



// AddrDBRecord.
//
// This is the unpacked record form as used by the app.  Pointers are
// either NULL or point to strings elsewhere on the card.  All strings
// are null character terminated.

typedef struct
{
	AddrOptionsType	options;        // Display by company or by name
	Char *			fields[ad_addressFieldsCount];
} AddrDBRecordType;

typedef AddrDBRecordType *AddrDBRecordPtr;


// The labels for phone fields are stored specially.  Each phone field
// can use one of eight labels.  Part of those eight labels are stored
// where the phone field labels are.  The remainder (phoneLabelsStoredAtEnd)
// are stored after the labels for all the fields.

typedef char addressLabel[addrLabelLength];

typedef struct
{
	UInt16				renamedCategories;	// bitfield of categories with a different name
	char 					categoryLabels[dmRecNumCategories][dmCategoryLength];
	UInt8 				categoryUniqIDs[dmRecNumCategories];
	UInt8					lastUniqID;	// Uniq IDs generated by the device are between
	// 0 - 127.  Those from the PC are 128 - 255.
	UInt8					reserved1;	// from the compiler word aligning things
	UInt16				reserved2;
	AddrDBFieldLabelsDirtyFlags dirtyFieldLabels;
	addressLabel 		fieldLabels[addrNumFields + numPhoneLabelsStoredSecond];
	CountryType 		country;		// Country the database (labels) is formatted for
	UInt8 				reserved;
	AddrDBMisc			misc;
} AddrAppInfoType;

typedef AddrAppInfoType *AddrAppInfoPtr;


/************************************************************
 * Function Prototypes
 *************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

UInt32 BitAtPosition(UInt32 pos);

AddrAppInfoPtr	AddrDBAppInfoGetPtr(DmOpenRef dbP) SEC("code1");
void			AddrDBChangeCountry(AddrAppInfoPtr appInfoP) SEC("code1");
Err				AddrDBAppInfoInit(DmOpenRef dbP) SEC("code1");
void			AddrDBSetFieldLabel(DmOpenRef dbP, UInt16 fieldNum, Char *fieldLabel) SEC("code1");
Err				AddrDBNewRecord(DmOpenRef dbP, AddrDBRecordPtr r, UInt16 *index) SEC("code1");
Err				AddrDBChangeRecord(DmOpenRef dbP, UInt16 *index, AddrDBRecordPtr r, AddrDBRecordFlags changedFields) SEC("code1");
Err				AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP, MemHandle *recordH) SEC("code1");
Boolean			AddrDBRecordContainsData (AddrDBRecordPtr recordP) SEC("code1");
Err				AddrDBChangeSortOrder(DmOpenRef dbP, Boolean sortByCompany) SEC("code1");
Boolean			AddrDBLookupSeekRecord (DmOpenRef dbP, UInt16 *indexP, Int16 *phoneP, Int16 offset, Int16 direction, AddressLookupFields field1, AddressLookupFields field2, AddressFields lookupFieldMap[]) SEC("code1");
Boolean			AddrDBLookupString(DmOpenRef dbP, Char *key, Boolean sortByCompany, UInt16 category, UInt16 *recordP, Boolean *completeMatch,Boolean masked) SEC("code1");
Boolean			AddrDBLookupLookupString(DmOpenRef dbP, Char *key, Boolean sortByCompany, AddressLookupFields field1, AddressLookupFields field2, UInt16 *recordP, Int16 *phoneP, AddressFields lookupFieldMap[], Boolean *completeMatch, Boolean *uniqueMatch) SEC("code1");
Err				AddrDBGetDatabase (DmOpenRef *addrPP, UInt16 mode) SEC("code1");

#ifdef __cplusplus
}
#endif

#endif // ADDRESSDB_H
