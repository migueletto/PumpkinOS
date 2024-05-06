/*
 * $Id: money.c,v 1.16 2006/10/12 14:21:22 desrod Exp $
 *
 * money.c:  Translate Pilot MoneyManager data formats
 *
 * Copyright (c) 1998, Rui Oliveira
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi-macros.h"
#include "pi-money.h"

/***********************************************************************
 *
 * Function:    unpack_Transaction
 *
 * Summary:     unpacks Transaction_t data
 *
 * Parameters:  Transaction_t*, char* to buffer, length of buffer
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_Transaction(Transaction_t *trans, unsigned char *buffer, size_t len)
{

	unsigned char *p;

	if (len < 46)
		return 0;

	p = buffer;
	trans->flags 	= get_byte(p);
	p += 2;			/* gap */
	trans->checknum 	= get_short(p);
	p += 2;
	trans->amount 	= get_slong(p);
	p += 4;
	trans->total 	= get_slong(p);
	p += 4;
	trans->amountc 	= get_sshort(p);
	p += 2;
	trans->totalc 	= get_sshort(p);
	p += 2;

	trans->second 	= get_sshort(p);
	p += 2;
	trans->minute 	= get_sshort(p);
	p += 2;
	trans->hour 	= get_sshort(p);
	p += 2;
	trans->day 		= get_sshort(p);
	p += 2;
	trans->month 	= get_sshort(p);
	p += 2;
	trans->year 	= get_sshort(p);
	p += 2;
	trans->wday 	= get_sshort(p);
	p += 2;

	trans->repeat 	= get_byte(p);
	p += 1;
	trans->flags2 	= get_byte(p);
	p += 1;
	trans->type 	= get_byte(p);
	p += 1;

	memcpy(trans->reserved, p, 2);
	p += 2;

	trans->xfer = get_byte(p);
	p += 1;

	strcpy(trans->description, (char *)p);
	p += 19;
	strcpy(trans->note, (char *)p);
	p += strlen((char *)p) + 1;

	return (p - buffer);
}


/***********************************************************************
 *
 * Function:    pack_Transaction
 *
 * Summary:     unpacks Transaction_t data
 *
 * Parameters:  Transaction_t*, char* to buffer, length of buffer
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int pack_Transaction(struct Transaction *trans, unsigned char *buffer,
		 	size_t len)
{
	size_t 	destlen = 46 + strlen(trans->note) + 1;
	unsigned char *p;

	if (!buffer)
		return destlen;
	if (len < destlen)
		return 0;

	p = buffer;
	set_byte(p, trans->flags);
	p += 1;
	set_byte(p, 0);
	p += 1;			/* gap fill */
	set_short(p, trans->checknum);
	p += 2;
	set_slong(p, trans->amount);
	p += 4;
	set_slong(p, trans->total);
	p += 4;
	set_sshort(p, trans->amountc);
	p += 2;
	set_sshort(p, trans->totalc);
	p += 2;

	set_sshort(p, trans->second);
	p += 2;
	set_sshort(p, trans->minute);
	p += 2;
	set_sshort(p, trans->hour);
	p += 2;
	set_sshort(p, trans->day);
	p += 2;
	set_sshort(p, trans->month);
	p += 2;
	set_sshort(p, trans->year);
	p += 2;
	set_sshort(p, trans->wday);
	p += 2;

	set_byte(p, trans->repeat);
	p += 1;
	set_byte(p, trans->flags2);
	p += 1;
	set_byte(p, trans->type);
	p += 1;

	/* gap fill */
	set_short(p, 0);
	p += 2;

	set_byte(p, trans->xfer);
	p += 1;

	strcpy((char *)p, trans->description);
	p += 19;
	strcpy((char *)p, trans->note);
	p += strlen((char *)p) + 1;

	return (p - buffer);
}


/***********************************************************************
 *
 * Function:    unpack_MoneyAppInfo
 *
 * Summary:     unpacks MoneyAppInfo_t data
 *
 * Parameters:  MoneyAppInfo_t*, char* to buffer, length of buffer
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_MoneyAppInfo(MoneyAppInfo_t *appinfo, unsigned char *buffer, size_t len)
{
	int 	i,
		j;

	unsigned char *p;

	i = unpack_CategoryAppInfo(&appinfo->category, buffer, len);
	if (!i)
		return 0;

	p = (unsigned char *) (buffer + i);

	len -= i;
	if (len < 603)
		return 0;

	for (j = 0; j < 20; j++) {
		memcpy(appinfo->typeLabels[j], p, 10);
		p += 10;
	}

	for (j = 0; j < 20; j++) {
		memcpy(appinfo->tranLabels[j], p, 20);
		p += 20;
	}

	return i + 603;
}


/***********************************************************************
 *
 * Function:    pack_MoneyAppInfo
 *
 * Summary:     packs MoneyAppInfo_t data
 *
 * Parameters:  MoneyAppInfo_t*, char* to buffer, length of buffer
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_MoneyAppInfo(MoneyAppInfo_t *appinfo, unsigned char *buffer, size_t len)
{
	int 	i,
		j;
	unsigned char *p;

	i = pack_CategoryAppInfo(&appinfo->category, buffer, len);

	if (!buffer)
		return i + 603;
	if (!i)
		return i;

	p = (unsigned char *) (buffer + i);
	len -= i;
	if (i < 603)
		return 0;

	for (j = 0; j < 20; j++) {
		memcpy(p, appinfo->typeLabels[j], 10);
		p += 10;
	}

	for (j = 0; j < 20; j++) {
		memcpy(p, appinfo->tranLabels[j], 20);
		p += 20;
	}

	return (i + 603);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
