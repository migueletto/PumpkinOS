/*
 * $Id: pi-blob.h,v 1.1 2009/02/22 08:08:59 nicholas Exp $
 *
 * pi-blob.h - Support for blobs that appear in some palm databases.
 * 
 * (c) 2008, Jon Schewe & Judd Montgomery
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
#ifndef _PILOT_BLOB_H_
#define _PILOT_BLOB_H_

#include <stdint.h>

#include <pi-appinfo.h>
#include <pi-buffer.h>

/* This is the blob that has the timezone data in it */
#define BLOB_TYPE_CALENDAR_TIMEZONE_ID "Bd00"
/* Not sure what this blob type is, but it's just some extra data that appears in some calendar records, when it exists it is always 4 bytes long */
#define BLOB_TYPE_CALENDAR_UNKNOWN_ID "Bd01"

/* Maximum number of blobs that can exist in a record, this is just a guess at the upper bound */
#define MAX_BLOBS 10

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct {
		/* type ranges from "Bd00" - "Bd09" */
		char type[4];
		int16_t length;
		uint8_t *data;
      } Blob_t;

	extern void free_Blob
	PI_ARGS((Blob_t *blob));

	extern int unpack_Blob_p
	PI_ARGS((Blob_t *blob, const unsigned char *data, const size_t position));

	extern int pack_Blob
	PI_ARGS((const Blob_t *blob, pi_buffer_t *buf));

	extern Blob_t *dup_Blob
	PI_ARGS((const Blob_t *blob));
	
	
#ifdef __cplusplus
};
#endif

  
#endif /* _PILOT_BLOB_H */

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */

