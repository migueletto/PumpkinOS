/*
 * $Id: blob.c,v 1.1 2009/02/22 08:09:00 nicholas Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
#include "pi-blob.h"

/***********************************************************************
 *
 * Function:    free_Blob
 *
 * Summary:     Frees members of the blob structure
 *
 * Parameters:  Blob_t*
 *
 * Returns:     void
 *
 ***********************************************************************/
void
free_Blob(Blob_t *blob)
{
	if(NULL != blob->data) {
		free(blob->data);
		blob->data = NULL;
	}
}

/***********************************************************************
 *
 * Function:    dup_Blob
 *
 * Summary:     Allocate memory for a new blob that is a duplicate of this one and copy the data into it
 *
 * Parameters:  Blob_t*
 *
 * Returns:     Blob_t* or NULL if there isn't enough memory and errno is set to ENOMEM
 *
 ***********************************************************************/
Blob_t*
dup_Blob(const Blob_t *blob)
{
	if(NULL == blob) {
		return NULL;
	}
	
	Blob_t *retval = (Blob_t*)malloc(sizeof(Blob_t));
	if(NULL == retval) {
		errno = ENOMEM;
		return NULL;
	}
	memcpy(retval->type, blob->type, 4);
	retval->length = blob->length;
	if(blob->length > 0) {
		retval->data = (uint8_t *)malloc(blob->length);
		if(NULL == retval->data) {
			errno = ENOMEM;
			return NULL;
		} else {
			memcpy(retval->data, blob->data, blob->length);
		}
	} else {
		retval->data = NULL;
	}
	return retval;
}


/***********************************************************************
 *
 * Function:    unpack_Blob
 *
 * Summary:     Unpack a blob starting at position in data
 *
 * Parameters:  Blob_t*, unsigned char*, size_t
 *
 * Returns:     the number of bytes read or -1 on error
 *
 ***********************************************************************/
int
unpack_Blob_p(Blob_t *blob, const unsigned char *data, const size_t position) {
	size_t localPosition = position;
  
	memcpy(blob->type, (char *)data+localPosition, 4);
	localPosition += 4;
	blob->length = get_short(data+localPosition);
	localPosition += 2;
	if(blob->length > 0) {
		blob->data = (uint8_t *)malloc(blob->length);
		if(NULL == blob->data) {
			return -1;
		} else {
			memcpy(blob->data, data+localPosition, blob->length);
		}
		localPosition += blob->length;
	}

	return localPosition - position;
}

/***********************************************************************
 *
 * Function:    pack_Blob
 *
 * Summary:     Pack a blob into the specified buffer 
 *
 * Parameters:  Blob_t*, pi_buffer_t*
 *
 * Returns:     0 on success, -1 on failure
 *
 ***********************************************************************/
int
pack_Blob(const Blob_t *blob, pi_buffer_t *buf) {
	size_t offset;
  
	offset = buf->used;
	pi_buffer_expect(buf, buf->used + 6 + blob->length);
	buf->used = buf->used + 6 + blob->length;
  
	memcpy(buf->data+offset, blob->type, 4);
	offset += 4;
  
	set_short(buf->data+offset, blob->length);
	offset += 2;
  
	memcpy(buf->data+offset, blob->data, blob->length);
	offset += blob->length;

	return 0;
}

/* vi: set ts=8 sw=4 sts=4 noexpandtab: cin */
/* ex: set tabstop=4 expandtab: */
/* Local Variables: */
/* indent-tabs-mode: t */
/* c-basic-offset: 8 */
/* End: */
