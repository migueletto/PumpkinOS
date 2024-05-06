/*
 * $Id: pi-address.h,v 1.21 2006/11/22 22:52:25 adridg Exp $
 *
 * pi-address.h: Macros for Palm "Classic" Address support
 * see pi-contact.h for the "Extended" Contacts support
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
  
#ifndef _PILOT_ADDRESS_H_
#define _PILOT_ADDRESS_H_

#include "pi-appinfo.h"
#include "pi-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		address_v1,
	} addressType;
  
	typedef enum {
		entryLastname, 
		entryFirstname, 
		entryCompany, 
		entryPhone1, 
		entryPhone2,
		entryPhone3,
		entryPhone4,
		entryPhone5,
		entryAddress,
		entryCity,
		entryState,
		entryZip,
		entryCountry,
		entryTitle,
		entryCustom1,
		entryCustom2,
		entryCustom3,
		entryCustom4,
		entryNote,
		entryCategory
	} AddressField_t;

	typedef struct Address {
		int phoneLabel[5];
		int showPhone;

		char *entry[19];
	} Address_t;

	typedef struct AddressAppInfo {
		addressType type;
		struct CategoryAppInfo category;
		char labels[19 + 3][16];	/* Hairy to explain, obvious to look	*/
		int labelRenamed[19 + 3];	/* Booleans show labels modified		*/
		char phoneLabels[8][16];	/* Dup some labels, reduce hair			*/
		int country;
		int sortByCompany;
	} AddressAppInfo_t;

	extern void free_Address
	  PI_ARGS((Address_t *));
	extern int unpack_Address
	  PI_ARGS((Address_t *, const pi_buffer_t *buf, addressType type));
	extern int pack_Address
	  PI_ARGS((const Address_t *, pi_buffer_t *buf, addressType type));
	extern int unpack_AddressAppInfo
	  PI_ARGS((AddressAppInfo_t *, const unsigned char *AppInfo, size_t len));
	extern int pack_AddressAppInfo
	  PI_ARGS((const AddressAppInfo_t *, unsigned char *AppInfo, size_t len));

#ifdef __cplusplus
  };
#endif

#endif				/* _PILOT_ADDRESS_H_ */
