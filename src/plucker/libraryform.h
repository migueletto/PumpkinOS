/*
 * $Id: libraryform.h,v 1.18 2004/05/19 11:40:03 nordstrom Exp $
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

#ifndef PLUCKER_LIBRARYFORM_H
#define PLUCKER_LIBRARYFORM_H

#include "viewer.h"
#include "genericfile.h"

#define dmAllCategoriesAdvanced    0xFFFF
#define NO_ROW                    -1
#define ONE_ROW                    1

#define LIBRARY_SELECTOR_GO_TO_TOP             -1
#define LIBRARY_SELECTOR_GO_TO_BOTTOM          -2

typedef enum {
    librarySelectorUndefined = 0,
    librarySelectorInit,
    librarySelectorGo,
    librarySelectorUp,
    librarySelectorDown,
    librarySelectorLeft,
    librarySelectorRight,
    librarySelectorPageUp,
    librarySelectorPageDown,
    librarySelectorToNumber
} LibrarySelector;

/* initialize document info list */
extern void InitializeDocInfoList( void ) LIBRARYFORM_SECTION;

/* Return pointer to docinfo structure */
extern DocumentInfo* DocInfo( Int16 index ) LIBRARYFORM_SECTION;

/* Return index for selected document */
extern Int16 ReturnLastIndex( void ) LIBRARYFORM_SECTION;

/* Release allocated memory */
extern void ReleaseDocInfoList( void ) LIBRARYFORM_SECTION;

/* Check if in select categories mode */
extern Boolean IsSelectingCategoryFilter( void ) LIBRARYFORM_SECTION;

extern Boolean LibrarySelectorHandler ( LibrarySelector movement,
    Int32 argument ) LIBRARYFORM_SECTION;
extern RectangleType *LibraryGetDisplayListBounds(Int16 row) LIBRARYFORM_SECTION;
extern UInt16 LibraryGetDisplayListIndex(Int16 row) LIBRARYFORM_SECTION;
extern void OpenNewDocument( UInt16 index ) LIBRARYFORM_SECTION;
extern void IconPopupList(UInt16 row) LIBRARYFORM_SECTION;
extern void ScrollUp( Int16 amount ) LIBRARYFORM_SECTION;
extern void ScrollDown( Int16 amount ) LIBRARYFORM_SECTION;
extern Int16 LibraryGetNumberOfRows() LIBRARYFORM_SECTION;
extern void LibrarySetLastIndexForRow(UInt16 row) LIBRARYFORM_SECTION;
extern Int16 LibraryGetFirstVisibleRow(void) LIBRARYFORM_SECTION;
extern void LibraryHighlightRow( Int16 row ) LIBRARYFORM_SECTION;
extern void SelectNextCategory(void) LIBRARYFORM_SECTION;
extern void SelectorSetRow( Int16 row ) LIBRARYFORM_SECTION;
extern Int16 SelectorGetRow( void ) LIBRARYFORM_SECTION;
extern void SelectorHighlightRow( Boolean enable ) LIBRARYFORM_SECTION;
extern void SetLibraryFormUpdate( void ) LIBRARYFORM_SECTION;
extern void ShowSyncMessage( void ) LIBRARYFORM_SECTION;
extern void HideMessage( void ) LIBRARYFORM_SECTION;

/* Event handler for the document manager form */
extern Boolean LibraryFormHandleEvent( EventType* event );

extern Boolean StatusFormHandleEvent( EventType* event );
#endif

