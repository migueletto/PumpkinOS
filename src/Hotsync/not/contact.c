/* $Id: contact.c,v 1.20 2009/02/23 11:59:16 nicholas Exp $ */

/*******************************************************************************
 * contact.c:  Translate Palm contact data formats
 * Derived from a module of J-Pilot http://jpilot.org (jp-contact.c 1.10)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi-macros.h"
#include "pi-contact.h"

#include "pi-debug.h"
#include "debug.h"
 

/***********************************************************************
 *
 * Function:   free_Contact
 *
 * Summary:    Free the members of a contact structure
 *
 * Parameters: None
 *
 * Returns:    Nothing
 *
 ***********************************************************************/
void free_Contact(struct Contact *c)
{
   int i;

   for (i = 0; i < NUM_CONTACT_ENTRIES; i++)
      if (c->entry[i])
         free(c->entry[i]);
   for (i = 0; i < MAX_CONTACT_BLOBS; i++) {
      if (c->blob[i]) {
         if (c->blob[i]->data)
            free(c->blob[i]->data);
 
         free(c->blob[i]);
      }
   }

   if (c->picture) free(c->picture);
}

#define hi(x) (((x) >> 4) & 0x0f)
#define lo(x) ((x) & 0x0f)
#define pair(x,y) (((x) << 4) | (y))

/***********************************************************************
 *
 * Function:   unpack_Contact
 *
 * Summary:    Fill in the contact structure based on the raw record
 *             data
 *
 * Parameters: None
 *
 * Returns:    -1 on error, 
 *             The length of the data used from the buffer on success
 *
 ***********************************************************************/
int unpack_Contact(struct Contact *c, pi_buffer_t *buf, contactsType type)
{
   unsigned long contents1;
   unsigned long contents2;
   unsigned char *Pbuf, *record;
   int i, field_num, len;
   unsigned int packed_date;
   unsigned int blob_count;

   if (buf == NULL || buf->data == NULL || buf->used < 17)
      return -1;
   
   if (type != contacts_v10 && type != contacts_v11) {
     /* Don't support anything else yet */
     return -1;
   }

   record = Pbuf = buf->data;
   len = buf->used;

   for (i=0; i<MAX_CONTACT_BLOBS; i++) {
      c->blob[i]=NULL;
   }
   c->picture=NULL;

   c->showPhone     = hi(get_byte(Pbuf));
   c->phoneLabel[6] = lo(get_byte(Pbuf));
   c->phoneLabel[5] = hi(get_byte(Pbuf + 1));
   c->phoneLabel[4] = lo(get_byte(Pbuf + 1));
   c->phoneLabel[3] = hi(get_byte(Pbuf + 2));
   c->phoneLabel[2] = lo(get_byte(Pbuf + 2));
   c->phoneLabel[1] = hi(get_byte(Pbuf + 3));
   c->phoneLabel[0] = lo(get_byte(Pbuf + 3));

   c->addressLabel[2] = lo(get_byte(Pbuf + 4));
   c->addressLabel[1] = hi(get_byte(Pbuf + 5));
   c->addressLabel[0] = lo(get_byte(Pbuf + 5));

   c->IMLabel[1] = hi(get_byte(Pbuf + 7));
   c->IMLabel[0] = lo(get_byte(Pbuf + 7));

   contents1 = get_long(record + 8);
   contents2 = get_long(record + 12);

   /* c->companyOffset = get_byte(record + 16); */

   Pbuf  += 17;
   len   -= 17;

   field_num=0;

   for (i = 0; i < 28; i++, field_num++) {
      if (contents1 & (1 << i)) {
         if (len < 1)
            return 0;
         c->entry[field_num] = strdup((char *) Pbuf);
         Pbuf += strlen((char *) Pbuf) + 1;
         len -= strlen(c->entry[field_num]) + 1;
      } else {
         c->entry[field_num] = 0;
      }
   }
   for (i = 0; i < 11; i++, field_num++) {
      if (contents2 & (1 << i)) {
         if (len < 1)
            return 0;
         c->entry[field_num] = strdup((char *) Pbuf);
         Pbuf += strlen((char *) Pbuf) + 1;
         len -= strlen(c->entry[field_num]) + 1;
      } else {
         c->entry[field_num] = 0;
      }
   }

   /* I think one of these is a birthday flag and one is an alarm flag.
    * Since both are always set there is no way to know which is which.
    * It could be something like a flag for advanceUnits also.  */
   if ((contents2 & 0x0800) || (contents2 & 0x1000)) {
      c->birthdayFlag = 1;
      if (len < 1)
         return 0;
      packed_date = get_short(Pbuf);
      c->birthday.tm_year = ((packed_date & 0xFE00) >> 9) + 4;
      c->birthday.tm_mon  = ((packed_date & 0x01E0) >> 5) - 1;
      c->birthday.tm_mday = (packed_date & 0x001F);
      c->birthday.tm_hour = 0;
      c->birthday.tm_min  = 0;
      c->birthday.tm_sec  = 0;
      c->birthday.tm_isdst= -1;
      mktime(&c->birthday);
      /* 2 bytes and a zero padding byte */
      len -= 3;
      Pbuf += 3;
      c->advanceUnits = get_byte(Pbuf);
      len--;
      Pbuf++;
   } else {
      c->birthdayFlag = 0;
   }

   if (contents2 & 0x2000) {
      c->reminder = 1;
      if (len < 1)
         return 0;
      c->advance = get_byte(Pbuf);
      len -= 1;
      Pbuf += 1;
   } else {
      c->reminder = 0;
      c->advance = 0;
   }

   /* A blob of size zero would take 6 bytes */
   blob_count=0;
   while (len >= 6) {
      if (blob_count >= MAX_CONTACT_BLOBS) {
         /* Too many blobs were found. */
         return (Pbuf - record);
      }
      c->blob[blob_count] = malloc(sizeof(struct ContactBlob));
      strncpy(c->blob[blob_count]->type, (char *)Pbuf, 4);
      c->blob[blob_count]->length = get_short(Pbuf+4);
      c->blob[blob_count]->data = malloc(c->blob[blob_count]->length);
      if (c->blob[blob_count]->data) {
         memcpy(c->blob[blob_count]->data, Pbuf+6, c->blob[blob_count]->length);
      }
      if (! strncmp(c->blob[blob_count]->type, BLOB_TYPE_PICTURE_ID, 4)) {
         if (!(c->picture)) {
            c->picture = malloc(sizeof(struct ContactPicture));
         }
         c->picture->dirty = get_short(c->blob[blob_count]->data);
         c->picture->length = c->blob[blob_count]->length - 2;
         c->picture->data = c->blob[blob_count]->data + 2;
      }

      Pbuf += 6;
      len  -= 6;
      Pbuf += c->blob[blob_count]->length;
      len  -= c->blob[blob_count]->length;
      blob_count++;
   }

   return (Pbuf - record);
}


/***********************************************************************
 *
 * Function:   pack_Contact
 *
 * Summary:    Fill in the raw contact record data based on the 
 *             contact structure
 *
 * Parameters: None
 *
 * Returns:    -1 on error
 *             The length of the buffer used on success
 *
 ***********************************************************************/
int pack_Contact(struct Contact *c, pi_buffer_t *buf, contactsType type)
{
   int l, destlen = 17;

   unsigned char *Pbuf, *record;
   unsigned long contents1, contents2;
   int i;
   unsigned int  field_i;
   unsigned long phoneflag;
   unsigned long typesflag;
   unsigned short packed_date;
   int companyOffset = 0;

   if (c == NULL || buf == NULL)
      return -1;

   if (type != contacts_v10 && type != contacts_v11) {
     /* Don't support anything else yet */
     return -1;
   }

   for (i = 0; i < NUM_CONTACT_ENTRIES; i++) {
      if (c->entry[i]) {
         destlen += (strlen(c->entry[i]) + 1);
      }
   }
   if (c->birthdayFlag) {
      destlen += 3;
      if (c->reminder) {
	 destlen += 2;
      } else {
	 destlen += 1;
      }
   }

   /* Check for blobs */
   for (i=0; i<MAX_CONTACT_BLOBS; i++) {
      if (c->blob[i]) {
         destlen += c->blob[i]->length + 6;
      }
   }

   pi_buffer_expect(buf, destlen);

   record = buf->data;

   Pbuf = record + 17;
   phoneflag = 0;
   typesflag = 0;
   contents1 = contents2 = 0;

   field_i = 0;
   for (i = 0; i < 28; i++, field_i++) {
      if (c->entry[field_i] && strlen(c->entry[field_i])) {
         contents1 |= (1 << i);
         l = strlen(c->entry[field_i]) + 1;
         memcpy(Pbuf, c->entry[field_i], l);
         Pbuf += l;
      }
   }
   for (i = 0; i < 11; i++, field_i++) {
      if (c->entry[field_i] && strlen(c->entry[field_i])) {
         contents2 |= (1 << i);
         l = strlen(c->entry[field_i]) + 1;
         memcpy(Pbuf, c->entry[field_i], l);
         Pbuf += l;
      }
   }

   phoneflag  = (((unsigned long) c->phoneLabel[0]) & 0xF) << 0;
   phoneflag |= (((unsigned long) c->phoneLabel[1]) & 0xF) << 4;
   phoneflag |= (((unsigned long) c->phoneLabel[2]) & 0xF) << 8;
   phoneflag |= (((unsigned long) c->phoneLabel[3]) & 0xF) << 12;
   phoneflag |= (((unsigned long) c->phoneLabel[4]) & 0xF) << 16;
   phoneflag |= (((unsigned long) c->phoneLabel[5]) & 0xF) << 20;
   phoneflag |= (((unsigned long) c->phoneLabel[6]) & 0xF) << 24;
   phoneflag |= (((unsigned long) c->showPhone) & 0xF) << 28;

   typesflag   = (((unsigned long) c->IMLabel[0]) & 0xF) << 0;
   typesflag  |= (((unsigned long) c->IMLabel[1]) & 0xF) << 4;
   typesflag  |= (((unsigned long) c->addressLabel[0]) & 0xF) << 16;
   typesflag  |= (((unsigned long) c->addressLabel[1]) & 0xF) << 20;
   typesflag  |= (((unsigned long) c->addressLabel[2]) & 0xF) << 24;

   if (c->birthdayFlag) {
      contents2 |= 0x1800;
      packed_date = (((c->birthday.tm_year - 4) << 9) & 0xFE00) |
                        (((c->birthday.tm_mon+1) << 5) & 0x01E0) |
         (c->birthday.tm_mday & 0x001F);
      set_short(Pbuf, packed_date);
      Pbuf += 2;
      set_byte(Pbuf, 0);  /* padding byte in birthday date */
      Pbuf += 1;
      if (c->reminder) {
         contents2 |= 0x2000;
         set_byte(Pbuf, c->advanceUnits);
         Pbuf += 1;
         set_byte(Pbuf, c->advance);
         Pbuf += 1;
      } else {
         set_byte(Pbuf, 0);
         Pbuf += 1;
      }
   }

   set_long(record, phoneflag);
   set_long(record + 4, typesflag);
   set_long(record + 8, contents1);
   set_long(record + 12, contents2);
   /* companyOffset is the offset from itself to the company field,
    * or zero if no company field.  Its not useful to us at all.  */
   if (c->entry[2]) {
      companyOffset++;
      if (c->entry[0]) companyOffset += strlen(c->entry[0]) + 1;
      if (c->entry[1]) companyOffset += strlen(c->entry[1]) + 1;
   }
   set_byte(record + 16, companyOffset);

   /* Pack blobs */
   for (i=0; i<MAX_CONTACT_BLOBS; i++) {
      if (c->blob[i]) {
         memcpy(Pbuf, c->blob[i]->type, 4);
         Pbuf += 4;
         set_short(Pbuf, c->blob[i]->length);
         Pbuf += 2;
         memcpy(Pbuf, c->blob[i]->data, c->blob[i]->length);
         Pbuf += c->blob[i]->length;
      }
   }
   buf->used = Pbuf - record;

   return (buf->used);
}


/***********************************************************************
 *
 * Function:   Contact_add_blob
 *
 * Summary:    Add a blob record to a Contact Record
 *
 * Parameters: None
 *
 * Returns:    0 on success
 *             1 on other error
 *
 ***********************************************************************/
int Contact_add_blob(struct Contact *c, struct ContactBlob *blob)
{
   int i;

   for (i=0; i<MAX_CONTACT_BLOBS; i++) {
      if (c->blob[i]) {
         continue;
      }

      c->blob[i] = malloc(sizeof(struct ContactBlob));
      if (!c->blob[i]) return EXIT_FAILURE;

      c->blob[i]->data = malloc(blob->length);
      strncpy(c->blob[i]->type, blob->type, 4);
      c->blob[i]->length = blob->length;
      strncpy((char *)c->blob[i]->data, (char *)blob->data, blob->length);
      return EXIT_SUCCESS;
   }

   return EXIT_FAILURE;
}


/***********************************************************************
 *
 * Function:   Contact_add_picture
 *
 * Summary:    Add a picture blob record to a Contact Record
 *             This will add a blob, but not touch the picture structure
 *             of the contact record
 *
 * Parameters: None
 *
 * Returns:    0 on success
 *             1 on other error
 *
 ***********************************************************************/
int Contact_add_picture(struct Contact *c, struct ContactPicture *p)
{
   int i;

   if ((!p) || (p->length<1) || (!p->data)) {
      return EXIT_FAILURE;
   }
   for (i=0; i<MAX_CONTACT_BLOBS; i++) {
      if (c->blob[i]) {
         continue;
      }

      c->blob[i] = malloc(sizeof(struct ContactBlob));
      if (!c->blob[i]) return EXIT_FAILURE;

      c->blob[i]->data = malloc(p->length + 2);
      //strncpy(c->blob[i]->type, BLOB_TYPE_PICTURE_ID, 4);
      memcpy(c->blob[i]->type, BLOB_TYPE_PICTURE_ID, 4);
      c->blob[i]->length = p->length + 2;
      set_short(c->blob[i]->data, p->dirty);
      memcpy(c->blob[i]->data + 2, p->data, p->length);
      return EXIT_SUCCESS;
   }

   return EXIT_FAILURE;
}


/***********************************************************************
 *
 * Function:   unpack_ContactAppInfo
 *
 * Summary:    Fill in the app info structure based on the raw app 
 *    info data
 *
 * Parameters: None
 *
 * Returns:    -1 on error
 *             The length of data used from the buffer on success
 *
 ***********************************************************************/
int unpack_ContactAppInfo(struct ContactAppInfo *ai, pi_buffer_t *buf)
{
   int i, j, destlen;
   unsigned char *start, *Pbuf;
   int len;

   start = Pbuf = buf->data;
   len = buf->used;
   if (len == 1092) {
      ai->type = contacts_v10;
      ai->num_labels = NUM_CONTACT_V10_LABELS;
      ai->numCustoms =  9; /* not sure - but  pi-contact.h <= 1.11 had 9 all the time */
   } else if (len == 1156) {
      ai->type = contacts_v11;
      ai->num_labels = NUM_CONTACT_V11_LABELS;
      ai->numCustoms =  9; /* not sure - but  pi-contact.h <= 1.11 had 9 all the time */
   } else {
      debug(DEBUG_ERROR, SYS_DEBUG, "contact.c: unpack_ContactAppInfo: ContactAppInfo size of %d incorrect", len);
      return -1;
   }

   /* 278 app info, 26 unknown, labels, county, sortBy */
   destlen = 278 + 26 + (16 * ai->num_labels) + 2 + 2;
   if (buf->used < destlen)
      return -1;

   i = unpack_CategoryAppInfo(&ai->category, start, len);
   if (!i)
      return i;
   Pbuf += i;

   memcpy(ai->internal, Pbuf, 26);
   Pbuf += 26;
   memcpy(ai->labels, Pbuf, 16 * ai->num_labels);
   Pbuf += 16 * ai->num_labels;
   ai->country = get_byte(Pbuf);
   Pbuf += 2;
   ai->sortByCompany = get_byte(Pbuf);
   Pbuf += 2;

   /* These are the fields that go in drop down menus */
   for (i = 4, j = 0; i < 11; i++, j++) {
      strcpy(ai->phoneLabels[j], ai->labels[i]);
   }
   strcpy(ai->phoneLabels[j], ai->labels[40]);

   for (i = 0; i < ai->numCustoms; i++) {
      strcpy(ai->customLabels[i], ai->labels[14 + i]);
   }

   strcpy(ai->addrLabels[0], ai->labels[23]);
   strcpy(ai->addrLabels[1], ai->labels[28]);
   strcpy(ai->addrLabels[2], ai->labels[33]);

   strcpy(ai->IMLabels[0], ai->labels[41]);
   strcpy(ai->IMLabels[1], ai->labels[42]);
   strcpy(ai->IMLabels[2], ai->labels[43]);
   strcpy(ai->IMLabels[3], ai->labels[44]);
   strcpy(ai->IMLabels[4], ai->labels[45]);

   return (Pbuf - start);
}


/***********************************************************************
 *
 * Function:   free_ContactAppInfo
 *
 * Summary:    Unallocate dynamically sized parts of ContactAppInfo
 *             (However, there are now none - provided for source compatibility)
 *
 * Parameters: struct ContactAppInfo *ai
 *
 * Returns:    None
 *
 ***********************************************************************/
void free_ContactAppInfo (struct ContactAppInfo *ai)
{
}

/***********************************************************************
 *
 * Function:   pack_ContactAppInfo
 *
 * Summary:    Fill in the raw app info record data based on the
 *             ContactAppInfo structure
 *
 * Parameters: None
 *
 * Returns:    -1 on error
 *             The length of the data used from the buffer on success.
 *
 ***********************************************************************/
int pack_ContactAppInfo(struct ContactAppInfo *ai, pi_buffer_t *buf)
{
   int destlen;

   if (buf == NULL || buf->data == NULL)
      return -1;

   /* 278 app info, 26 unknown, labels, country, sortBy */
   destlen = 278 + 26 + (16 * ai->num_labels) + 2 + 2;

   pi_buffer_expect(buf, destlen);

   buf->used = pack_CategoryAppInfo(&ai->category, buf->data, buf->allocated);
   if (buf->used != 278)
      return -1;

   pi_buffer_append(buf, ai->internal, 26);

   pi_buffer_append(buf, ai->labels, 16 * ai->num_labels);

   set_byte(buf->data + buf->used++, ai->country);
   /* Unknown field */
   set_byte(buf->data + buf->used++, 0x00);

   set_byte(buf->data + buf->used++, ai->sortByCompany);
   /* Unknown field */
   set_byte(buf->data + buf->used++, 0x00);

   return (buf->used);
}

