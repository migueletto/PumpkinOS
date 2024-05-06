/*
 * $Id: pi-foto.h,v 1.4 2006/10/17 13:24:07 desrod Exp $
 *
 * pi-foto.h - Macro definitions for the palmOne Photo application
 * 
 * Copyright (C) 2004 Steve Ratcliffe,  23 Feb 2004
 *
 * This latches early in the sync to determine serial sync speeds
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

#ifndef _PI_FOTO_H_
#define _PI_FOTO_H_

/** @file pi-foto.h
 *  @brief Macros for the Palm Photo application's data
 *
 */

/*
 * Offsets into the thumbnail record header.
 */
#define FOTO_IMAGE_WIDTH  0x04 /* Width of the image jpg. 2 bytes		*/
#define FOTO_IMAGE_HEIGHT 0x06 /* Height of the image jpg. 2 bytes		*/
#define FOTO_MODIFY_DATE  0x08 /* Date modified. 4 bytes			*/
#define FOTO_IMAGE_SIZE   0x0c /* (Related to) the size of the jpg pdb, 4b	*/
#define FOTO_THUMB_WIDTH  0x10 /* Width of thumbnail. 1 byte			*/
#define FOTO_THUMB_HEIGHT 0x11 /* Height of thumbnail. 1 byte 			*/
#define FOTO_NAME_LEN     0x16 /* Length of name. 1 byte(?) 			*/
#define FOTO_IMAGE_DATE   0x1c /* Date displayed with image. 4 bytes 		*/
#define FOTO_THUMB_SIZE   0x26 /* Size of thumbnail. 2 bytes 			*/

#endif /* _PI_FOTO_H_ */
