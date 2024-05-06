/*
 * $Id: mail.c,v 1.25 2006/10/12 14:21:22 desrod Exp $
 *
 * mail.c:  Translate Pilot mail data formats
 *
 * Copyright (c) 1997, Kenneth Albanowski
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

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "pi-macros.h"
#include "pi-mail.h"

char *MailSortTypeNames[] = { "Date", "Type", NULL };
char *MailSyncTypeNames[] = { "All", "Send", "Filter", NULL };

/***********************************************************************
 *
 * Function:    free_Mail
 *
 * Summary:     frees all the Mail_t structure members
 *
 * Parameters:  Mail_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Mail(Mail_t *mail)
{
	if (mail->from != NULL) {
		free(mail->from);
		mail->from = NULL;
	}

	if (mail->to != NULL) {
		free(mail->to);
		mail->to = NULL;
	}

	if (mail->subject != NULL) {
		free(mail->subject);
		mail->to = NULL;
	}

	if (mail->cc != NULL) {
		free(mail->cc);
		mail->cc = NULL;
	}

	if (mail->bcc != NULL) {
		free(mail->bcc);
		mail->bcc = NULL;
	}

	if (mail->replyTo) {
		free(mail->replyTo);
		mail->replyTo = NULL;
	}

	if (mail->sentTo) {
		free(mail->sentTo);
		mail->sentTo = NULL;
	}

	if (mail->body != NULL) {
		free(mail->body);
		mail->body = NULL;
	}
}


/***********************************************************************
 *
 * Function:    free_MailAppInfo
 *
 * Summary:     frees all MailAppInfo_t structure members
 *
 * Parameters:  MailAppInfo_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_MailAppInfo(MailAppInfo_t *appinfo)
{
	/* if (appinfo->signature)
	   free(appinfo->signature); */
}


/***********************************************************************
 *
 * Function:    free_MailSyncPref
 *
 * Summary:     frees all MailSyncPref_t structure members 
 *
 * Parameters:  MailSyncPref_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_MailSyncPref(MailSyncPref_t *pref)
{
	if (pref->filterTo != NULL) {
		free(pref->filterTo);
		pref->filterTo = NULL;
	}

	if (pref->filterFrom != NULL) {
		free(pref->filterFrom);
		pref->filterFrom = NULL;
	}

	if (pref->filterSubject != NULL) {
		free(pref->filterSubject);
		pref->filterSubject = NULL;
	}
}


/***********************************************************************
 *
 * Function:    free_MailSignaturePref
 *
 * Summary:     frees all MailSignaturePref_t structure members
 *
 * Parameters:  MailSignaturePref_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_MailSignaturePref(MailSignaturePref_t *pref)
{
	if (pref->signature != NULL) {
		free(pref->signature);
		pref->signature = NULL;
	}
}


/***********************************************************************
 *
 * Function:    unpack_Mail
 *
 * Summary:     unpacks Mail
 *
 * Parameters:  Mail_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_Mail(Mail_t *mail, unsigned char *buffer, size_t len)
{
	int 	flags;
	unsigned long d;
	unsigned char *start = buffer;

	if (len < 6)
		return 0;

	d = (unsigned short int) get_short(buffer);
	mail->date.tm_year 	= (d >> 9) + 4;
	mail->date.tm_mon 	= ((d >> 5) & 15) - 1;
	mail->date.tm_mday 	= d & 31;
	mail->date.tm_hour 	= get_byte(buffer + 2);
	mail->date.tm_min 	= get_byte(buffer + 3);
	mail->date.tm_sec 	= 0;
	mail->date.tm_isdst 	= -1;
	mktime(&mail->date);

	if (d)
		mail->dated = 1;
	else
		mail->dated = 0;

	flags = get_byte(buffer + 4);

	mail->read 		= (flags & (1 << 7)) ? 1 : 0;
	mail->signature 		= (flags & (1 << 6)) ? 1 : 0;
	mail->confirmRead 		= (flags & (1 << 5)) ? 1 : 0;
	mail->confirmDelivery 	= (flags & (1 << 4)) ? 1 : 0;
	mail->priority 		= (flags & (3 << 2)) >> 2;
	mail->addressing 		= (flags & 3);

	buffer 	+= 6;
	len 	-= 6;

	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->subject = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->subject = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->from = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->from = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->to = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->to = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->cc = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->cc = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->bcc = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->bcc = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->replyTo = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->replyTo = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->sentTo = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->sentTo = 0;
	buffer++;
	len--;
	if (len < 1)
		return 0;
	if (get_byte(buffer)) {
		mail->body = strdup((char *)buffer);
		buffer += strlen((char *)buffer);
		len -= strlen((char *)buffer);
	} else
		mail->body = 0;
	buffer++;
	len--;

	return (buffer - start);
}


/***********************************************************************
 *
 * Function:    pack_Mail
 *
 * Summary:     packs Mail
 *
 * Parameters:  Mail_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_Mail(struct Mail *mail, unsigned char *buffer, size_t len)
{
	size_t 	destlen = 6 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1;
	unsigned char *start = buffer;

	if (mail->subject)
		destlen += strlen(mail->subject);
	if (mail->from)
		destlen += strlen(mail->from);
	if (mail->to)
		destlen += strlen(mail->to);
	if (mail->cc)
		destlen += strlen(mail->cc);
	if (mail->bcc)
		destlen += strlen(mail->bcc);
	if (mail->replyTo)
		destlen += strlen(mail->replyTo);
	if (mail->sentTo)
		destlen += strlen(mail->sentTo);
	if (mail->body)
		destlen += strlen(mail->body);

	if (!buffer)
		return destlen;
	if (len < destlen)
		return 0;

	set_short(buffer,
		  ((mail->date.tm_year - 4) << 9) | ((mail->date.tm_mon +
						   1) << 5) | mail->date.
		  tm_mday);
	set_byte(buffer + 2, mail->date.tm_hour);
	set_byte(buffer + 3, mail->date.tm_min);

	if (!mail->dated)
		set_long(buffer, 0);

	set_byte(buffer + 4, (mail->read ? (1 << 7) : 0) |
		 (mail->signature ? (1 << 6) : 0) | (mail->
						  confirmRead ? (1 << 5) :
						  0) | (mail->
							confirmDelivery
							? (1 << 4) : 0) |
		 ((mail->priority & 3) << 2) | (mail->addressing & 3));
	set_byte(buffer + 5, 0);

	buffer += 6;

	if (mail->subject) {
		strcpy((char *)buffer, mail->subject);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->from) {
		strcpy((char *)buffer, mail->from);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->to) {
		strcpy((char *)buffer, mail->to);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->cc) {
		strcpy((char *)buffer, mail->cc);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->bcc) {
		strcpy((char *)buffer, mail->bcc);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->replyTo) {
		strcpy((char *)buffer, mail->replyTo);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->sentTo) {
		strcpy((char *)buffer, mail->sentTo);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;
	if (mail->body) {
		strcpy((char *)buffer, mail->body);
		buffer += strlen((char *)buffer);
	} else
		set_byte(buffer, 0);
	buffer++;

	return (buffer - start);
}


/***********************************************************************
 *
 * Function:    unpack_MailAppInfo
 *
 * Summary:     unpacks MailAppInfo
 *
 * Parameters:  MailAppInfo_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_MailAppInfo(MailAppInfo_t *appinfo, unsigned char *record, size_t len)
{
	int 	i;
	unsigned char *start = record;

	i = unpack_CategoryAppInfo(&appinfo->category, record, len);
	if (!i)
		return i;
	record += i;
	len -= i;
	if (len < 11)
		return 0;
	appinfo->dirty = get_short(record);
	record += 2;
	appinfo->sortOrder = get_byte(record);
	record += 2;
	appinfo->unsentMessage = get_long(record);
	record += 4;

/* ai->signature = 0; 			*/
/* strdup(start + get_short(record)); 	*/
	record += 3;

	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_MailAppInfo
 *
 * Summary:     packs MailAppInfo
 *
 * Parameters:  MailAppInfo_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_MailAppInfo(struct MailAppInfo *appinfo, unsigned char *record, size_t len)
{
	int 	i;
	unsigned char *start = record;

	i = pack_CategoryAppInfo(&appinfo->category, record, len);
	if (!record)
		return i + 11;
	if (!i)
		return i;
	record += i;
	len -= i;
	if (len < 8)
		return 0;
	set_short(record, appinfo->dirty);
	record += 2;
	set_short(record, 0);	/* gapfill */
	set_byte(record, appinfo->sortOrder);
	record += 2;
	set_long(record, appinfo->unsentMessage);
	record += 4;

	set_short(record, (record - start + 2));
	record += 2;

	/* if (appinfo->signature)
	   strcpy(record, appinfo->signature);
	   else
	   set_byte(record, 0);
	   record += strlen(record); */
	set_byte(record, 0);
	record++;

	return (record - start);
}


/***********************************************************************
 *
 * Function:    unpack_MailSyncPref
 *
 * Summary:     unpacks Mail
 *
 * Parameters:  MailSyncPref_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_MailSyncPref(MailSyncPref_t *pref, unsigned char *record, size_t len)
{
	unsigned char *start = record;

	pref->syncType = get_byte(record);
	record += 1;
	pref->getHigh = get_byte(record);
	record += 1;
	pref->getContaining = get_byte(record);
	record += 2;
	pref->truncate = get_short(record);
	record += 2;

	if (get_byte(record)) {
		pref->filterTo = strdup((char *)record);
		record += strlen((char *)record);
	} else
		pref->filterTo = 0;
	record++;
	if (get_byte(record)) {
		pref->filterFrom = strdup((char *)record);
		record += strlen((char *)record);
	} else
		pref->filterFrom = 0;
	record++;
	if (get_byte(record)) {
		pref->filterSubject = strdup((char *)record);
		record += strlen((char *)record);
	} else
		pref->filterSubject = 0;
	record++;

	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_MailSyncPref
 *
 * Summary:     packs Mail
 *
 * Parameters:  MailSyncPref_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_MailSyncPref(MailSyncPref_t *pref, unsigned char *record, size_t len)
{
	size_t 	destlen = 6 + 1 + 1 + 1;
	unsigned char *start = record;

	if (pref->filterTo)
		destlen += strlen(pref->filterTo);
	if (pref->filterSubject)
		destlen += strlen(pref->filterSubject);
	if (pref->filterFrom)
		destlen += strlen(pref->filterFrom);

	if (!record)
		return destlen;
	if (len < destlen)
		return 0;

	set_byte(record, pref->syncType);
	record++;
	set_byte(record, pref->getHigh);
	record++;
	set_byte(record, pref->getContaining);
	record++;
	set_byte(record, 0);
	record++;		/* gapfill */
	set_short(record, pref->truncate);
	record += 2;

	if (pref->filterTo) {
		strcpy((char *)record, pref->filterTo);
		record += strlen(pref->filterTo);
	}
	*record++ = 0;

	if (pref->filterFrom) {
		strcpy((char *)record, pref->filterFrom);
		record += strlen(pref->filterFrom);
	}
	*record++ = 0;

	if (pref->filterSubject) {
		strcpy((char *)record, pref->filterSubject);
		record += strlen(pref->filterSubject);
	}
	*record++ = 0;

	return (record - start);
}


/***********************************************************************
 *
 * Function:    unpack_MailSignaturePref
 *
 * Summary:     unpacks MailSignaturePref
 *
 * Parameters:  MailSignaturePref_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_MailSignaturePref(MailSignaturePref_t *pref,
			 unsigned char *record, size_t len)
{
	unsigned char *start = record;

	if (len < 1)
		return 0;

	pref->signature = strdup((char *)record);

	record += strlen(pref->signature) + 1;

	return (record - start);
}


/***********************************************************************
 *
 * Function:    pack_MailSignaturePref
 *
 * Summary:     packs MailSignaturePref
 *
 * Parameters:  MailSignaturePref_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_MailSignaturePref(struct MailSignaturePref *pref, unsigned char *record,
		       size_t len)
{
	size_t 	destlen = 1;
	unsigned char *start = record;

	if (pref->signature)
		destlen += strlen(pref->signature);

	if (!record)
		return destlen;
	if (len < destlen)
		return 0;
	if (pref->signature) {
		strcpy((char *)record, pref->signature);
		record += strlen(pref->signature);
	}
	*record = 0;
	record++;

	return (record - start);
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
