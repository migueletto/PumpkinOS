/*
 * $Id: pi-buffer.h,v 1.6 2006/10/17 13:24:06 desrod Exp $
 *
 * pi-buffer.h:  simple data block management for variable data storage
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
 * * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. 
 */

/** @file pi-buffer.h
 *  @brief Variable size buffer management interface
 *  @author Florent Pillet
 *
 * pi-buffer provides for a reliable and easy to use variable size buffer
 * management, allowing for buffers that grow as needed to store
 * variable-length data.
 *
 * When you create a buffer with pi_buffer_new(), you indicate an initial
 * capacity that is allocated. The number of used bytes is set to 0. To
 * append data to the buffer, use pi_buffer_append(). This ensures that the
 * buffer grows as needed.
 *
 * You can access data in the buffer using the @a buffer->data member. The
 * number of bytes used is always accessible using @a buffer->used.
 *
 * It is possible to use the pi-buffer functions on static buffers. In this
 * case, you won't call pi_buffer_free() on the structure. You'll dispose of
 * the memory yourself instead. Here is an example:
 *
 * @code
 *	pi_buffer_t mybuf;
 *	mybuf.data = (unsigned char *) malloc(256);
 *	mybuf.allocated = 256;
 *	mybuf.used = 0;
 *
 *	// ... perform your tasks here ....
 *	pi_buffer_append(&mybuf, somedata, somedatasize);
 *	// ...
 *
 *	free(mybuf.data);
 * @endcode
 */

#ifndef _PILOT_BUFFER_H_
#define _PILOT_BUFFER_H_

#include "pi-args.h"

#ifdef __cplusplus
extern "C" {
#endif
	/** @brief Variable buffer structure */
	typedef struct pi_buffer_t {
		unsigned char *data;	/**< Pointer to the data */
		size_t allocated;	/**< Number of bytes allocated */
		size_t used;		/**< Number of allocated bytes actually used */
	} pi_buffer_t;

	/** @brief Create a new variable size buffer
	 *
	 * Dispose of this buffer with pi_buffer_free()
	 *
	 * @param capacity Initial size to allocate
	 * @return A newly allocated pi_buffer_t structure
	 */
	extern pi_buffer_t* pi_buffer_new
		PI_ARGS((size_t capacity));
	
	/** @brief Ensure the buffer is large enough to store @p capacity bytes of data
	 *
	 * This grows the allocated buffer as needed and updates the @a allocated
	 * member. Doesn't touch the @a used member. After this call succeeds, you
	 * can directly use the @a buffer->data pointer to store up to
	 * @a buffer->allocated bytes using direct memory access.
	 *
	 * @param buf The buffer to grow
	 * @param new_capacity The total number of bytes the buffer is expected to contain
	 * @return The @p buf buffer on success, NULL if a memory error happened
	 */
	extern pi_buffer_t* pi_buffer_expect
		PI_ARGS((pi_buffer_t *buf, size_t new_capacity));

	/** @brief Append data to the buffer
	 *
	 * Grow the buffer if needed.
	 *
	 * @param buf The buffer to grow
	 * @param data Pointer to the data to append
	 * @param len Length of the data to append
	 * @return The @p buf buffer on success, NULL if a memory error happened
	 */
	extern pi_buffer_t* pi_buffer_append
		PI_ARGS((pi_buffer_t *buf, PI_CONST void *data, size_t len));

	/** @brief Append a buffer to another buffer
	 *
	 * @param dest The buffer to append to
	 * @param src Buffer whose data will be appended to @p dest
	 * @return The @p dest buffer on success, NULL if a memory error happened
	 */
	extern pi_buffer_t* pi_buffer_append_buffer
		PI_ARGS((pi_buffer_t *dest, PI_CONST pi_buffer_t *src));

	/** @brief Reset the @a used member of a buffer
	 *
	 * The @p used member is set to 0. If the actual allocated bytes is large,
	 * the allocation may shrink to a reasonable value to prevent unneeded
	 * memory use.
	 *
	 * @param buf The buffer to clear
	 * @return The @p buf parameter
	 */
	extern void pi_buffer_clear
		PI_ARGS((pi_buffer_t *buf));

	/** @brief Dispose of all memory used by a buffer allocated with pi_buffer_new()
	 *
	 * After this call, the @p buf structure itself will have been freed as well.
	 * Do not reuse the pointer.
	 *
	 * @param buf The buffer to dispose of
	 */
	extern void pi_buffer_free
		PI_ARGS((pi_buffer_t *buf));

#ifdef __cplusplus
}
#endif

#endif
