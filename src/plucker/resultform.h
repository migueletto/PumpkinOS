/*
 * $Id: resultform.h,v 1.16 2004/01/01 15:27:30 prussar Exp $
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

#ifndef PLUCKER_RESULTFORM_H
#define PLUCKER_RESULTFORM_H

#include "viewer.h"


/* Event handler for the result form */
extern Boolean ResultFormHandleEvent( EventType* event );

/* Write header info for search result, return true if end of record has been reached */
extern Boolean DrawResultHeader( Char* header ) RESULTFORM_SECTION;

/* Write search result, return true if end of record has been reached */
extern Boolean DrawResultString( UInt16 record, UInt16 pos, UInt16 endPos,
                Char* result ) RESULTFORM_SECTION;

#endif

