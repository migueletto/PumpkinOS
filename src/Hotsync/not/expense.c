/*
 * $Id: expense.c,v 1.29 2006/11/07 21:13:24 adridg Exp $
 *
 * expense.c:  Translate Pilot expense tracker data formats
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
#include "pi-expense.h"

char *ExpenseSortNames[] = { "Date", "Type", NULL };
char *ExpenseDistanceNames[] = { "Miles", "Kilometers", NULL };
char *ExpensePaymentNames[] =
    {	"AmEx", "Cash", "Check", "CreditCard", "MasterCard", "Prepaid",
	"VISA", "Unfiled"
};

char *ExpenseTypeNames[] =
    { 	"Airfare", "Breakfast", "Bus", "Business Meals", "Car Rental",
	"Dinner", "Entertainment", "Fax", "Gas", "Gifts", "Hotel",
	"Incidentals", "Laundry", "Limo", "Lodging", "Lunch", "Mileage",
	"Other", "Parking", "Postage", "Snack", "Subway", "Supplies",
	"Taxi", "Telephone", "Tips", "Tolls", "Train"
};


/***********************************************************************
 *
 * Function:	free_Expense
 *
 * Summary:     frees members of the Expense structure
 *
 * Parameters:  Expense_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Expense(Expense_t *expense)
{
	if (expense->note != NULL) {
		free(expense->note);
		expense->note = NULL;
	}

	if (expense->amount != NULL) {
		free(expense->amount);
		expense->amount = NULL;
	}

	if (expense->city != NULL) {
		free(expense->city);
		expense->city = NULL;
	}

	if (expense->vendor != NULL) {
		free(expense->vendor);
		expense->vendor = NULL;
	}

	if (expense->attendees != NULL) {
		free(expense->attendees);
		expense->attendees = NULL;
	}
}


/***********************************************************************
 *
 * Function:    unpack_Expense
 *
 * Summary:     unpack Expense records
 *
 * Parameters:  Expense_t*, char* to buffer, length of buffer
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
unpack_Expense(Expense_t *expense, unsigned char *buffer, int len)
{
	unsigned long d;
	unsigned char *start = buffer;

	if (len < 6)
		return 0;

	d = (unsigned short int) get_short(buffer);
	expense->date.tm_year 	= (d >> 9) + 4;
	expense->date.tm_mon 	= ((d >> 5) & 15) - 1;
	expense->date.tm_mday 	= d & 31;
	expense->date.tm_hour 	= 0;
	expense->date.tm_min 	= 0;
	expense->date.tm_sec 	= 0;
	expense->date.tm_isdst 	= -1;
	mktime(&expense->date);

	expense->type 	= (enum ExpenseType) get_byte(buffer + 2);
	expense->payment = (enum ExpensePayment) get_byte(buffer + 3);
	expense->currency = get_byte(buffer + 4);

	buffer 	+= 6;
	len 	-= 6;

	if (len < 1)
		return 0;

	if (*buffer) {
		expense->amount = strdup((char *)buffer);
		buffer += strlen(expense->amount);
		len -= strlen(expense->amount);
	} else {
		expense->amount = 0;
	}
	buffer++;
	len--;

	if (len < 1)
		return 0;

	if (*buffer) {
		expense->vendor = strdup((char *)buffer);
		buffer += strlen(expense->vendor);
		len -= strlen(expense->vendor);
	} else {
		expense->vendor = 0;
	}
	buffer++;
	len--;

	if (len < 1)
		return 0;

	if (*buffer) {
		expense->city = strdup((char *)buffer);
		buffer += strlen(expense->city);
		len -= strlen(expense->city);
	} else {
		expense->city = 0;
	}
	buffer++;
	len--;

	if (len < 1)
		return 0;

	if (*buffer) {
		expense->attendees = strdup((char *)buffer);
		buffer += strlen(expense->attendees);
		len -= strlen(expense->attendees);
	} else {
		expense->attendees = 0;
	}
	buffer++;
	len--;

	if (len < 1)
		return 0;

	if (*buffer) {
		expense->note = strdup((char *)buffer);
		buffer += strlen(expense->note);
		len -= strlen(expense->note);
	} else {
		expense->note = 0;
	}

	buffer++;
	len--;

	return (buffer - start);
}


/***********************************************************************
 *
 * Function:    pack_Expense
 *
 * Summary:     pack Expense records
 *
 * Parameters:  Expense_t*, char* to buffer, buffer length
 *
 * Returns:     effective buffer length
 *
 ***********************************************************************/
int
pack_Expense(Expense_t *expense, unsigned char *record, int len)
{
	int 	destlen = 6 + 1 + 1 + 1 + 1 + 1;
	unsigned char *buf = record;

	if (expense->amount)
		destlen += strlen(expense->amount);
	if (expense->vendor)
		destlen += strlen(expense->vendor);
	if (expense->city)
		destlen += strlen(expense->city);
	if (expense->attendees)
		destlen += strlen(expense->attendees);
	if (expense->note)
		destlen += strlen(expense->note);

	if (!record)
		return destlen;
	if (len < destlen)
		return 0;

	set_short(buf,
		  ((expense->date.tm_year - 4) << 9) | ((expense->date.tm_mon +
						   1) << 5) | expense->date.
		  tm_mday);
	buf += 2;
	set_byte(buf, expense->type);
	set_byte(buf + 1, expense->payment);
	set_byte(buf + 2, expense->currency);
	set_byte(buf + 3, 0);	/* gapfill */
	buf += 4;

	if (expense->amount) {
		strcpy((char *)buf, expense->amount);
		buf += strlen((char *)buf);
	} else {
		set_byte(buf, 0);
	}
	buf++;

	if (expense->vendor) {
		strcpy((char *)buf, expense->vendor);
		buf += strlen((char *)buf);
	} else {
		set_byte(buf, 0);
	}
	buf++;

	if (expense->city) {
		strcpy((char *)buf, expense->city);
		buf += strlen((char *)buf);
	} else {
		set_byte(buf, 0);
	}
	buf++;

	if (expense->attendees) {
		strcpy((char *)buf, expense->attendees);
		buf += strlen((char *)buf);
	} else {
		set_byte(buf, 0);
	}
	buf++;

	if (expense->note) {
		strcpy((char *)buf, expense->note);
		buf += strlen((char *)buf);
	} else {
		set_byte(buf, 0);
	}
	buf++;

	return (buf - record);
}


/***********************************************************************
 *
 * Function:    unpack_ExpenseAppInfo
 *
 * Summary:   	unpacks ExpenseAppInfo record
 *
 * Parameters:  ExpenseAppInfo_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
unpack_ExpenseAppInfo(ExpenseAppInfo_t *appinfo, unsigned char *record,
		      size_t len)
{
	int 	i;
	unsigned char *start = record;

	i = unpack_CategoryAppInfo(&appinfo->category, record, len);
	if (!i)
		return 0;
	record += i;
	len -= i;

	appinfo->sortOrder = (enum ExpenseSort) get_byte(record);
	record += 2;
	for (i = 0; i < 4; i++) {
		memcpy(appinfo->currencies[i].name, record, 16);
		record += 16;
		memcpy(appinfo->currencies[i].symbol, record, 4);
		record += 4;
		memcpy(appinfo->currencies[i].rate, record, 8);
		record += 8;
	}
	return (record - start);
}

/***********************************************************************
 *
 * Function:    pack_ExpenseAppInfo
 *
 * Summary:     packs ExpenseAppInfo record
 *
 * Parameters:  ExpenseAppInfo_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
pack_ExpenseAppInfo(ExpenseAppInfo_t *appinfo, unsigned char *record,
		    size_t len)
{
	int 	i,
		destlen = 2 + (16 + 4 + 8) * 4;
	unsigned char *start = record;

	i = pack_CategoryAppInfo(&appinfo->category, record, len);
	if (!record)
		return i + destlen;
	if (!i)
		return i;
	record 	+= i;
	len 	-= i;
	if (len < destlen)
		return 0;
	set_byte(record, appinfo->sortOrder);
	set_byte(record + 1, 0);	/* gapfill */
	record += 2;
	for (i = 0; i < 4; i++) {
		memcpy(record, appinfo->currencies[i].name, 16);
		record += 16;
		memcpy(record, appinfo->currencies[i].symbol, 4);
		record += 4;
		memcpy(record, appinfo->currencies[i].rate, 8);
		record += 8;
	}

	return (record - start);
}

/***********************************************************************
 *
 * Function:    unpack_ExpensePref
 *
 * Summary:     unpacks ExpensePref record
 *
 * Parameters:  ExpensePref_t*, char* to record, record length
 *
 * Returns:     effective record length
 *
 ***********************************************************************/
int
unpack_ExpensePref(ExpensePref_t *pref, unsigned char *record, int len)
{
	int 	i;
	unsigned char *start = record;

	(void) len;

	pref->currentCategory 	= get_short(record);
	record += 2;
	pref->defaultCurrency 	= get_short(record);
	record += 2;
	pref->attendeeFont      = get_byte(record);
	record++;
	pref->showAllCategories = get_byte(record);
	record++;
	pref->showCurrency 	= get_byte(record);
	record++;
	pref->saveBackup 	= get_byte(record);
	record++;
	pref->allowQuickFill 	= get_byte(record);
	record++;
	pref->unitOfDistance 	= (enum ExpenseDistance) get_byte(record);
	record++;

	for (i = 0; i < 5; i++) {
		pref->currencies[i] = get_byte(record);
		record++;
	}

	for (i = 0; i < 2; i++) {
		pref->unknown[i] = get_byte(record);
		record++;
	}

	pref->noteFont = get_byte(record);
	record++;

	return (record - start);
}

/***********************************************************************
 *
 * Function:    pack_ExpensePref
 *
 * Summary:     packs ExpensePref record
 *
 * Parameters:  ExpensePref_t*, char* to record, record length
 *
 * Returns:     Nothing
 *
 ***********************************************************************/
int pack_ExpensePref(ExpensePref_t *p, unsigned char *record, int len)
{
	int 	i;
	unsigned char *start = record;

	(void) len;

	set_short(record, p->currentCategory);
	record += 2;
	set_short(record, p->defaultCurrency);
	record += 2;
	set_byte(record, p->attendeeFont);
	record++;
	set_byte(record, p->showAllCategories);
	record++;
	set_byte(record, p->showCurrency);
	record++;
	set_byte(record, p->saveBackup);
	record++;
	set_byte(record, p->allowQuickFill);
	record++;
	set_byte(record, p->unitOfDistance);
	record++;
	for (i = 0; i < 5; i++) {
		set_byte(record, p->currencies[i]);
		record++;
	}
	/* Unknown values */
	set_byte(record, 0xff);
	record++;
	set_byte(record, 0xff);
	record++;

	set_byte(record, p->noteFont);
	record++;

	return record - start;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */

