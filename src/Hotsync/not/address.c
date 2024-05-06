/*
 * $Id: address.c,v 1.30 2006/11/22 22:52:25 adridg Exp $
 *
 * address.c:  Translate Pilot address book data formats
 * (c) 1996, Kenneth Albanowski
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


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi-macros.h"
#include "pi-address.h"

#define hi(x) (((x) >> 4) & 0x0f)
#define lo(x) ((x) & 0x0f)
#define pair(x,y) (((x) << 4) | (y))

/***********************************************************************
 *
 * Function:    free_Address
 *
 * Summary:	Free the members of an address structure
 *
 * Parameters:  Address_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Address(Address_t *addr)
{
	int 	i;

	for (i = 0; i < 19; i++)
		if (addr->entry[i]) {
			free(addr->entry[i]);
			addr->entry[i] = NULL;
		}
}


/***********************************************************************
 *
 * Function:    unpack_Address
 *
 * Summary:     Fill in the address structure based on the raw record 
 *		data
 *
 * Parameters:  Address_t*, pi_buffer_t *buf
 *
 * Returns:     -1 on error, 0 on success
 *
 ***********************************************************************/
int
unpack_Address(Address_t *addr, const pi_buffer_t *buf, addressType type)
{
	unsigned long	contents,
			v;
	size_t		ofs;

	if (type != address_v1)
		/* Don't support anything else yet */
		return -1;

	if (buf == NULL || buf->data == NULL || buf->used < 9)
		return -1;

	/* get_byte(buffer); gapfill */
	addr->showPhone     = hi(get_byte(buf->data + 1));
	addr->phoneLabel[4] = lo(get_byte(buf->data + 1));
	addr->phoneLabel[3] = hi(get_byte(buf->data + 2));
	addr->phoneLabel[2] = lo(get_byte(buf->data + 2));
	addr->phoneLabel[1] = hi(get_byte(buf->data + 3));
	addr->phoneLabel[0] = lo(get_byte(buf->data + 3));

	contents = get_long(buf->data + 4);

	/* get_byte(buf->data+8) offset */

	ofs = 9;

	/* if(flag & 0x1) { 
	   addr->lastname = strdup((buf->data + ofs);
	   ofs += strlen((buf->data + ofs)) + 1;
	   } else {
	   addr->lastname = 0;
	   } */

	for (v = 0; v < 19; v++) {
		if (contents & (1 << v)) {
			if ((buf->used - ofs) < 1)
				return 0;
			addr->entry[v] = strdup((char *) (buf->data + ofs));
                  	ofs += strlen(addr->entry[v]) + 1;
		} else {
			addr->entry[v] = 0;
		}
	}

	return 0;
}


/***********************************************************************
 *
 * Function:    pack_Address
 *
 * Summary:     Fill in the raw address record data based on the 
 *		address structure
 *
 * Parameters:  Address_t*, pi_buffer_t *buf of record, record type
 *
 * Returns:     -1 on error, 0 on success.
 *
 ***********************************************************************/
int
pack_Address(const Address_t *addr, pi_buffer_t *buf, addressType type)
{
	unsigned int	l,
			destlen = 9;

	unsigned char *buffer;
	unsigned long 	contents,
			v,
			phoneflag;

	unsigned char offset;

	if (addr == NULL || buf == NULL)
		return -1;

	if (type != address_v1)
		/* Don't support anything else yet */
		return -1;

	for (v = 0; v < 19; v++)
		if (addr->entry[v] && strlen(addr->entry[v]))
			destlen += strlen(addr->entry[v]) + 1;

	pi_buffer_expect (buf, destlen);
	buf->used = destlen;

	buffer 		= buf->data + 9;
	phoneflag 	= 0;
	contents 	= 0;
	offset 		= 0;

	for (v = 0; v < 19; v++) {
		if (addr->entry[v] && strlen(addr->entry[v])) {
			if (v == entryCompany)
				offset =
				    (unsigned char) (buffer - buf->data) - 8;
			contents |= (1 << v);
			l = strlen(addr->entry[v]) + 1;
			memcpy(buffer, addr->entry[v], l);
			buffer += l;
		}
	}

	phoneflag = ((unsigned long) addr->phoneLabel[0]) << 0;
	phoneflag |= ((unsigned long) addr->phoneLabel[1]) << 4;
	phoneflag |= ((unsigned long) addr->phoneLabel[2]) << 8;
	phoneflag |= ((unsigned long) addr->phoneLabel[3]) << 12;
	phoneflag |= ((unsigned long) addr->phoneLabel[4]) << 16;
	phoneflag |= ((unsigned long) addr->showPhone) << 20;

	set_long(buf->data, phoneflag);
	set_long(buf->data + 4, contents);
	set_byte(buf->data + 8, offset);

	return 0;
}


/***********************************************************************
 *
 * Function:    unpack_AddressAppInfo
 *
 * Summary:     Fill in the app info structure based on the raw app 
 *		info data
 *
 * Parameters:  AddressAppInfo_t*, char * to record, record length
 *
 * Returns:     The necessary length of the buffer if record is NULL,
 *		or 0 on error, the length of the data used from the 
 *		buffer otherwise
 *
 ***********************************************************************/
int
unpack_AddressAppInfo(AddressAppInfo_t *ai, const unsigned char *record, size_t len)
{
	size_t 	i,
		destlen = 4 + 16 * 22 + 2 + 2;

	unsigned char *start = (unsigned char *)record;
	unsigned long r;

	ai->type = address_v1;

	i = unpack_CategoryAppInfo(&ai->category, record, len);
	if (!record)
		return i + destlen;
	if (!i)
		return i;
	record += i;
	len -= i;

	if (len < destlen)
		return 0;

	r = get_long(record);
	for (i = 0; i < 22; i++)
		ai->labelRenamed[i] = !!(r & (1 << i));

	record += 4;
	memcpy(ai->labels, record, 16 * 22);
	record += 16 * 22;
	ai->country = get_short(record);
	record += 2;
	ai->sortByCompany = get_byte(record);
	record += 2;

	for (i = 3; i < 8; i++)
		strcpy(ai->phoneLabels[i - 3], ai->labels[i]);
	for (i = 19; i < 22; i++)
		strcpy(ai->phoneLabels[i - 19 + 5], ai->labels[i]);

	return (record - start);
}

/***********************************************************************
 *
 * Function:    pack_AddressAppInfo
 *
 * Summary:     Fill in the raw app info record data based on the app
 *		info structure
 *
 * Parameters:  AddressAppInfo_t*, char * to record, record length
 *
 * Returns:     The length of the buffer required if record is NULL,
 *		or 0 on error, the length of the data used from the
 *		buffer otherwise
 *
 ***********************************************************************/
int
pack_AddressAppInfo(const AddressAppInfo_t *ai, unsigned char *record, size_t len)
{
	int 	i;
	size_t	destlen = 4 + 16 * 22 + 2 + 2;
	unsigned char *pos = record;
	unsigned long r;

	i = pack_CategoryAppInfo(&ai->category, record, len);
	if (!record)
		return destlen + i;
	if (!i)
		return i;

	pos += i;
	len -= i;

	for (i = 3; i < 8; i++)
		strcpy((char *)ai->phoneLabels[i - 3], ai->labels[i]);
	for (i = 19; i < 22; i++)
		strcpy((char *)ai->phoneLabels[i - 19 + 5], ai->labels[i]);

	memset(pos, 0, destlen);

	r = 0;
	for (i = 0; i < 22; i++)
		if (ai->labelRenamed[i])
			r |= (1 << i);
	set_long(pos, r);
	pos += 4;

	memcpy(pos, ai->labels, 16 * 22);
	pos += 16 * 22;
	set_short(pos, ai->country);
	pos += 2;
	set_byte(pos, ai->sortByCompany);
	pos += 2;

	for (i = 3; i < 8; i++)
		strcpy((char *)ai->phoneLabels[i - 3], ai->labels[i]);
	for (i = 19; i < 22; i++)
		strcpy((char *)ai->phoneLabels[i - 19 + 5], ai->labels[i]);

	return (pos - record);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
