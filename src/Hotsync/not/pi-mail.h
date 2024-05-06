/*
 * $Id: pi-mail.h,v 1.19 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-mail.h: Palm Mail application macros
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
 
#ifndef _PILOT_MAIL_H_
#define _PILOT_MAIL_H_

#include <time.h>
#include "pi-appinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct Mail {
		int read;
		int signature;
		int confirmRead;
		int confirmDelivery;
		int priority;
		int addressing;

		int dated;
		struct tm date;

		char *subject;
		char *from;
		char *to;
		char *cc;
		char *bcc;
		char *replyTo;
		char *sentTo;
		char *body;
	} Mail_t;

	typedef struct MailAppInfo {
		struct CategoryAppInfo category;
		int dirty;			/* boolean */
		int sortOrder;
		unsigned long unsentMessage;	/* UniqueID of unsent message */

		/* not implemented by Palm
		char *signature; */
	} MailAppInfo_t;

	typedef struct MailSyncPref {
		int syncType;
		int getHigh;
		int getContaining;
		int truncate;
		char *filterTo;
		char *filterFrom;
		char *filterSubject;
	} MailSyncPref_t;

	typedef struct MailSignaturePref {
		char *signature;
	} MailSignaturePref_t;

	extern char *MailSyncTypeNames[];
	extern char *MailSortTypeNames[];

	typedef enum {
		mailCtgInbox = 0,
		mailCtgOutbox,
		mailCtgDeleted,
		mailCtgFiled,
		mailCtgDraft
	} MailCategory;

	typedef enum {
		mailSyncAll = 0,
		mailSyncSend,
		mailSyncFilter,
		mailSyncUnread = 3
	} MailSyncType;

	typedef enum {
		/* XXX 0? */
		mailPrefLocal = 1,
		mailPrefRemote,
		mailPrefSig
	} MailPrefId;

	extern void free_Mail PI_ARGS((struct Mail *));
	extern void free_MailAppInfo PI_ARGS((struct MailAppInfo *));
	extern void free_MailSyncPref PI_ARGS((struct MailSyncPref *));
	extern void free_MailSignaturePref
	    PI_ARGS((struct MailSignaturePref *));

	extern int unpack_Mail
	    PI_ARGS((struct Mail *, unsigned char *record, size_t len));

	extern int pack_Mail
	    PI_ARGS((struct Mail *, unsigned char *record, size_t len));

	extern int unpack_MailAppInfo
	    PI_ARGS((struct MailAppInfo *, unsigned char *AppInfo,
		     size_t len));

	extern int pack_MailAppInfo
	    PI_ARGS((struct MailAppInfo *, unsigned char *AppInfo,
		     size_t len));

	extern int unpack_MailSyncPref
	    PI_ARGS((struct MailSyncPref *, unsigned char *record,
		     size_t len));

	extern int unpack_MailSignaturePref
	    PI_ARGS((struct MailSignaturePref *, unsigned char *record,
		     size_t len));

	extern int pack_MailSyncPref
	    PI_ARGS((struct MailSyncPref *, unsigned char *record,
		     size_t len));

	extern int pack_MailSignaturePref
	    PI_ARGS((struct MailSignaturePref *, unsigned char *record,
		     size_t len));

#ifdef __cplusplus
}
#endif
#endif				/* _PILOT_MAIL_H_ */
