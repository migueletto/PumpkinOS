/*
 * $Id: uncompress.h,v 1.21 2003/02/26 01:46:24 chrish Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef PLUCKER_UNCOMPRESS_H
#define PLUCKER_UNCOMPRESS_H

#include "viewer.h"
#include "document.h"


/* Local record IDs in UncompressDB */
#define UNCOMPRESS_TEXT_ID      0
#define UNCOMPRESS_IMAGE_ID     1
#define UNCOMPRESS_SPECIAL_ID   2
#define UNCOMPRESS_TABLE_ID     3


/* Create buffer used when uncompressing records */
extern Err CreateUncompressBuffer( void ) UNCOMPRESS_SECTION;

/* Close and delete uncompress buffer */
extern void DeleteUncompressBuffer( void ) UNCOMPRESS_SECTION;

/* Return handle to doc uncompress record */
extern MemHandle GetUncompressTextHandle( void ) UNCOMPRESS_SECTION;

/* Return handle to image uncompress record */
extern MemHandle GetUncompressImageHandle( void ) UNCOMPRESS_SECTION;

/* Uncompress DOC compressed text/image */
extern MemHandle UnDoc( Header* record ) UNCOMPRESS_SECTION;

/* Uncompress ZLib compressed text/image */
extern MemHandle UnZip( Header* record ) UNCOMPRESS_SECTION;

/* Return CRC-32 of buflen bytes in specified buffer */
extern UInt32 CRC32 ( UInt32 seed, const Char* buf,
                UInt16 bufLen ) UNCOMPRESS_SECTION;

/* Set the username that is used to key the zlib uncompress routine */
extern void SetUncompressKey ( const UInt8* key ) UNCOMPRESS_SECTION;

/* Release allocated memory for username */
extern void ReleaseUncompressKey( void ) UNCOMPRESS_SECTION;

/* Free table UncompressRecord */
void FreeUncompressTableRecord( UInt16 uid ) UNCOMPRESS_SECTION;

PLKR_GLOBAL MemHandle (*Uncompress)( Header* );

#endif

