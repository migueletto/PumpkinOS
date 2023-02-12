/******************************************************************************
 *
 * Copyright (c) 2000 Palm, Inc. or its subsidiaries.
 * All rights reserved.
 *
 * File: AddrDefines.h
 *
 * Release: Palm OS SDK 4.0 (63220)
 *
 *****************************************************************************/

#ifndef ADDRDEFINES_H
#define ADDRDEFINES_H

#include <PalmOS.h>
#include <Event.h>


#ifdef PALMOS
#define AddressBookCreator        'AdTe'
#else
#define AddressBookCreator        sysFileCAddress
#endif

// Update codes, used to determine how the address list view should
// be redrawn.
#define updateRedrawAll						0x01
#define updateGrabFocus						0x02
#define updateItemHide						0x04
#define updateCategoryChanged				0x08
#define updateFontChanged					0x10
#define updateListViewPhoneChanged			0x20
#define updateCustomFieldLabelChanged		0x40
#define updateSelectCurrentRecord			0x80

// AutoFill database types and names
// Note that we prefix with "Address" to avoid name conflicts with Expense app

#define titleDBType							'titl'
#define companyDBType						'cmpy'
#define cityDBType							'city'
#define stateDBType							'stat'
#define countryDBType						'cnty'
#define addrDBType							'DATA'

#ifdef PALMOS
#define titleDBName							"AddressTitlesDBTe"
#define companyDBName						"AddressCompaniesDBTe"
#define cityDBName							"AddressCitiesDBTe"
#define stateDBName							"AddressStatesDBTe"
#define countryDBName						"AddressCountriesDBTe"
#define addrDBName							"AddressDBTe"
#else
#define titleDBName							"AddressTitlesDB"
#define companyDBName						"AddressCompaniesDB"
#define cityDBName							"AddressCitiesDB"
#define stateDBName							"AddressStatesDB"
#define countryDBName						"AddressCountriesDB"
#define addrDBName							"AddressDB"
#endif

#define shortenedFieldString				"..."
#define shortenedFieldLength				3
#define fieldSeparatorString				", "
#define fieldSeparatorLength				2
#define spaceBetweenNamesAndPhoneNumbers	6

#define editFirstFieldIndex					0

#define	kDialListShowInListPhoneIndex		((UInt16)(-1))

#define addrNumFields						19
#define numPhoneLabels						8

#define noRecord							0xffff

#define GetPhoneLabel(r, p)					(((r)->options.phoneBits >> (((p) - firstPhoneField) << 2)) & 0xF)

#define SetPhoneLabel(r, p, pl)				((r)->options.phoneBits = \
											((r)->options.phoneBits & ~((UInt32) 0x0000000F << (((p) - firstPhoneField) << 2))) | \
											((UInt32) pl << (((p) - firstPhoneField) << 2)))

#define kFrmCustomUpdateEvent				firstUserEvent


#endif	// ADDRDEFINES_H
