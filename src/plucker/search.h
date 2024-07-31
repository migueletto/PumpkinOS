/*
 * $Id: search.h,v 1.22 2003/07/23 04:27:11 prussar Exp $
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

#ifndef PLUCKER_SEARCH_H
#define PLUCKER_SEARCH_H

#include "viewer.h"

/* return the first/last record ID in the last sequence searched */
void GetLastSearchedSequenceBoundaryRecordIds( UInt16* first, UInt16* last )
            SEARCH_SECTION;

/* get record ID of last searched record */
UInt16 GetLastSearchedRecordId( void ) SEARCH_SECTION;

/* set record ID of last searched record */
void SetLastSearchedRecordId( UInt16 uid ) SEARCH_SECTION;

/* Search in all pages */
extern Boolean SearchDocument( Char* pattern, Boolean startFromBeginning,
            Boolean* findMore, SearchModeType searchMode ) SEARCH_SECTION;

/* Search again in current page */
extern void SearchAgain( void ) SEARCH_SECTION;

/* Set search position */
extern void SetSearchPosition( UInt16 pos ) SEARCH_SECTION;

/* Initialize format of percentage indicator */
extern void InitializeResultFormat( const Char* lang ) SEARCH_SECTION;

/* Release search queue for subpage search */
extern void ReleaseSearchQueue( void ) SEARCH_SECTION;

/* Search from here! */
extern void SetSearchFromHere( void ) SEARCH_SECTION;

/* Go to the result */
extern void GoToSearchResult( void ) SEARCH_SECTION;
#endif

