/*
 * $Id: pi-buffer.c,v 1.10 2006/10/12 14:21:22 desrod Exp $
 *
 * pi-buffer.c:  simple data block management for variable data storage
 *
 * Copyright (c) 2004-2005, Florent Pillet.
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
 *
 * -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pi-buffer.h"

pi_buffer_t*
pi_buffer_new (size_t capacity) 
{
	pi_buffer_t* buf;
	buf = (struct pi_buffer_t *) malloc (sizeof (struct pi_buffer_t));
	if (buf == NULL)
		return NULL;

	if (capacity <= 0)
		capacity = 16;	/* allocating 0 byte is illegal - use a small value instead */

	buf->data = (unsigned char *) malloc (capacity);
	if (buf->data == NULL) {
		free (buf);
		return NULL;
	}

	buf->allocated = capacity;
	buf->used = 0;
	return buf;
}

pi_buffer_t*
pi_buffer_expect (pi_buffer_t *buf, size_t expect)
{
	if ((buf->allocated - buf->used) >= expect)
		return buf;

	if (buf->data)
		buf->data = (unsigned char *) realloc (buf->data, buf->used + expect);
	else
		buf->data = (unsigned char *) malloc (expect);

	if (buf->data == NULL) {
		buf->allocated = 0;
		buf->used = 0;
		return NULL;
	}

	buf->allocated = buf->used + expect;
	return buf;
}

pi_buffer_t*
pi_buffer_append (pi_buffer_t *buf, const void *data, size_t len)
{
	if (pi_buffer_expect (buf, len) == NULL)
		return NULL;

	memcpy (buf->data + buf->used, data, len);
	buf->used += len;

	return buf;
}

pi_buffer_t *
pi_buffer_append_buffer (pi_buffer_t *dest, const pi_buffer_t *src)
{
	return pi_buffer_append (dest, src->data, src->used);
}

void
pi_buffer_clear (pi_buffer_t *buf)
{
	buf->used = 0;
	if (buf->allocated > (size_t)65535)
	{
		buf->data = (unsigned char *) realloc (buf->data, 65535);
		buf->allocated = (buf->data == NULL) ? 0 : 65535;
	}
}

void
pi_buffer_free (pi_buffer_t* buf)
{
	if (buf) {
		if (buf->data)
			free (buf->data);
		free (buf);
	}
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
