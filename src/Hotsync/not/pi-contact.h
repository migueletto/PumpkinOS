/*******************************************************************************
 *
 * $Id: pi-contact.h,v 1.12 2009/02/23 11:59:16 nicholas Exp $
 *
 * pi-contact.h:  Translate Palm contact data formats
 * Derived from a module of J-Pilot http://jpilot.org (jp-pi-contact.h 1.5)
 *
 * Rewrite Copyright 2006, 2007 Judd Montgomery
 * Rewrite Copyright 2004, 2005 Joseph Carter
 * Copyright 2003, 2004 Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ******************************************************************************/

/*
 * Hints for uses of this library <= 1.11:
 *
 * appinfo->internal has changed from a pi_buffer_t to char array of fixed length.
 *
 * appinfo->labels has changed from a pi_buffer_t to an array of char
 * arrays of fixed length, and is no longer 'opaque'.
 *
 * contact->reminder has changed from -1 for 'not set', to 0 for 'not
 * set' and the reminder/advance duration is now in contact->advance.
 *
 * contact->picture has changed from a pi_buffer_t to a ContactPicture
 *
 * There is no longer a contact->pictype (if the picture exists, it's a JPEG.)
 *
 */

#ifndef _PILOT_CONTACT_H_
#define _PILOT_CONTACT_H_

#include <pi-args.h>
#include <pi-appinfo.h>
#include <pi-buffer.h>
#include <time.h>

#define MAX_CONTACT_VERSION 11

#define NUM_CONTACT_ENTRIES 39

#define NUM_CONTACT_V10_LABELS 49
#define NUM_CONTACT_V11_LABELS 53

/* Blob types, or blob creator IDs, can range from BD00 - Bd09 for Contacts */
#define BLOB_TYPE_PICTURE_ID "Bd00"
#define MAX_CONTACT_BLOBS 10

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	contacts_v10,
	contacts_v11
} contactsType;

/* Entry fields */
enum { 
   contLastname, 
   contFirstname, 
   contCompany, 
   contTitle,
   contPhone1, 
   contPhone2,
   contPhone3,
   contPhone4,
   contPhone5,
   contPhone6,
   contPhone7,
   contIM1,
   contIM2,
   contWebsite,
   contCustom1,
   contCustom2,
   contCustom3,
   contCustom4,
   contCustom5,
   contCustom6,
   contCustom7,
   contCustom8,
   contCustom9,
   contAddress1,
   contCity1,
   contState1,
   contZip1,
   contCountry1,
   contAddress2,
   contCity2,
   contState2,
   contZip2,
   contCountry2,
   contAddress3,
   contCity3,
   contState3,
   contZip3,
   contCountry3,
   contNote
};

/* Non-entry fields */
enum { 
   contBirthday = contNote + 1,
   contPicture
};

struct ContactBlob {
   /* type ranges from "Bd00" - "Bd09" */
   char type[4];
   int length;
   unsigned char *data;
};

struct ContactPicture {
   /* The picture pointer is only for convienience and
    * will point to the 3rd byte of the last picture blob.
    * The data will not need to be freed.  The blob structure will. */
   unsigned int dirty;
   /* data points to blob data in jpeg format */
   unsigned int length;
   unsigned char *data;
};

struct Contact {
   int phoneLabel[7];
   int addressLabel[3];
   int IMLabel[2];
   int showPhone;
   int birthdayFlag;
   int reminder;
   int advance;
   int advanceUnits;    
   struct tm birthday;
   char *entry[39];
   struct ContactBlob *blob[MAX_CONTACT_BLOBS];
   struct ContactPicture *picture;
};

struct ContactAppInfo {
   contactsType type;
   int num_labels;
   struct CategoryAppInfo category;
   char internal[26];         /* Palm has not documented what this is */
   char labels[53][16];       /* Hairy to explain, obvious to look at */
   /*int labelRenamed[53];*/  /* list of booleans showing which labels were modified   */
   int country;
   int sortByCompany;
   int numCustoms;            /* Included for source compatibility with pi-contact.h <= 1.11 */
   char customLabels[9][16];  /* Duplication of some labels, to greatly reduce hair */
   char phoneLabels[8][16];   /* Duplication of some labels, to greatly reduce hair */
   char addrLabels[3][16];    /* Duplication of some labels, to greatly reduce hair */
   char IMLabels[5][16];      /* Duplication of some labels, to greatly reduce hair */
};

extern void free_Contact
    PI_ARGS((struct Contact *));
extern int unpack_Contact
    PI_ARGS((struct Contact *, pi_buffer_t *, contactsType));
extern int pack_Contact
    PI_ARGS((struct Contact *, pi_buffer_t *, contactsType));
extern int unpack_ContactAppInfo
    PI_ARGS((struct ContactAppInfo *, pi_buffer_t *));
extern int pack_ContactAppInfo
    PI_ARGS((struct ContactAppInfo *, pi_buffer_t *));

extern void free_ContactAppInfo
    PI_ARGS((struct ContactAppInfo *));

extern int Contact_add_blob
    PI_ARGS((struct Contact *, struct ContactBlob *));
extern int Contact_add_picture
    PI_ARGS((struct Contact *, struct ContactPicture *));

#ifdef __cplusplus
}

#endif            /* __cplusplus */
#endif            /* _PILOT_CONTACT_H_ */
