/*
 * $Id: link.c,v 1.26 2003/07/12 14:57:03 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2001, Mark Ian Lillywhite and Michael Nordstrom
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

#include "debug.h"
#include "metadocument.h"
#include "link.h"



/* Get the status for the link */
Boolean LinkVisited
    (
    UInt16 reference /* record number the link refers to */
    )
{
    return GetBitStatus( LINK_TABLE_ID, reference );
}



/* Set the status to visited for the link */
void SetVisitedLink
    (
    UInt16 reference /* record number the link refers to */
    )
{
    SetBitStatus( LINK_TABLE_ID, reference, true );
}



/* Set the status to not visited for the link */
void UnsetVisitedLink
    (
    UInt16 reference /* record number the link refers to */
    )
{
    SetBitStatus( LINK_TABLE_ID, reference, false );
}
