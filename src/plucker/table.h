/*
 * $Id: table.h,v 1.13 2004/02/04 16:09:06 chrish Exp $
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

#ifndef PLUCKER_TABLE_H
#define PLUCKER_TABLE_H

#include "viewer.h"
#include "document.h"


/* Return table size */
extern Boolean GetTableSize( UInt16 reference, Coord* tablew,
                Coord* tableh ) TABLE_SECTION;

/* Draw 3D icon for table link */
extern void DrawIcon( Coord x, Coord y ) TABLE_SECTION;

/* Get Handle to large table */
extern MemHandle GetFullscreenTableHandle( void ) TABLE_SECTION;

/* Delete table window, table images */
extern void ReleaseFsTableHandle( void ) TABLE_SECTION;

/* Load table */
extern Boolean LoadTable( Header* record, const Boolean newPage ) TABLE_SECTION;

/* Check if a large table is displayed */
extern Boolean IsLargeTable( void ) TABLE_SECTION;

/* Draw table inline */
extern Boolean InlineTable( UInt16 reference, Coord tableX,
                Coord tableY ) TABLE_SECTION;

/* Copy visible anchors from large table to screen */
extern void CopyTableAnchors( Int32 reference, Int16 x, Int16 y ) TABLE_SECTION;

#endif

