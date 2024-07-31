/*
 * $Id: metadata.h,v 1.4 2002/09/26 15:26:54 nordstrom Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2002, Mark Ian Lillywhite and Michael Nordstrom
 * and Bill Janssen
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

#ifndef PLUCKER_METADATA_H
#define PLUCKER_METADATA_H

#include "viewer.h"


/* metadata "names" (type codes) */

#define METADATA_CHARSET                1
#define METADATA_EXCEPTIONAL_CHARSETS   2
#define METADATA_OWNER_ID               3

typedef struct {
    UInt16  typecode;
    UInt16  length;
    UInt16  argument[ 0 ];
} MetadataElementHeader;

#define NEXT_METADATA_RECORD( record )  ( (MetadataElementHeader*)( ( (UInt16*)( ( record )->argument ) ) + ( record )->length ) )

#endif

