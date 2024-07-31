/*
 * $Id: history.h,v 1.30 2003/08/11 00:21:25 prussar Exp $
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

#ifndef PLUCKER_HISTORY_H
#define PLUCKER_HISTORY_H

#include "viewer.h"
#include "prefsdata.h"


#define HISTORYSIZE 32


/* The first four entries in this type must correspond to
   entries 2-5 in MetaRecord. */
typedef struct {
    YOffset verticalOffset;
    Int16   characterPosition;
    Int16   firstVisibleParagraph;
    YOffset firstParagraphY;
    UInt16  recordId;
} HistoryData;


typedef struct {
    UInt16          uid;
    Int16           firstRecord;
    Int16           lastRecord;
    Int16           currentRecord;
    Boolean         addHistory;
    HistoryData     records[ HISTORYSIZE ];
    UInt16          depth;
    FontModeType    font;
    ToolbarType     toolbar;
    ScrollbarType   scrollbar;
    Int16           lineSpacing;
    Int16           paragraphSpacing;
    Char            userFontName[ dmDBNameLength ];
    UInt16          maxExtentX;
    RotateType      rotate;
    UInt16          extentX;
} History;


/* Initialize the history structure */
extern void InitHistory( void ) HISTORY_SECTION;

/* Read history from session record */
extern void ReadHistory( History* recordPtr, UInt16 availableSize ) 
        HISTORY_SECTION;

/* Save history in session record */
extern void SaveHistory( History* recordPtr ) HISTORY_SECTION;

/* Store data for first paragraph in current record from history */
extern void SetHistoryFirstParagraph( const Int16 offset,
        const YOffset y ) HISTORY_SECTION;

#ifdef STORE_LAST_VISIBLE
/* Store data for last paragraph in current record from history */
extern void SetHistoryLastParagraph( const Int16 offset,
        const YOffset y ) HISTORY_SECTION;
#endif

/* Store vertical offset for current record from history */
extern void SetHistoryVerticalOffset( const YOffset offset ) HISTORY_SECTION;

/* Add record to history */
extern void AddToHistory( const UInt16 recordId ) HISTORY_SECTION;

/* Get the previous record from the history */
extern const UInt16 GetHistoryPrev( void ) HISTORY_SECTION;

/* Get the next record from the history */
extern const UInt16 GetHistoryNext( void ) HISTORY_SECTION;

/* Get the current record ID from the history */
extern const UInt16 GetHistoryCurrent( void ) HISTORY_SECTION;

/* Get the add history status */
extern const Boolean GetAddHistory( void ) HISTORY_SECTION;

/* Get the history structure */
extern History* GetHistoryPtr( void ) HISTORY_SECTION;

/* reset verticalOffsets in the history */
extern void ResetHistoryVerticalOffsets( void ) HISTORY_SECTION;

/* set character position of start */
extern void SetHistoryCharacterPosition( Int16 position ) HISTORY_SECTION;

/* set character position of start */
extern Int16 GetHistoryCharacterPosition( void ) HISTORY_SECTION;

/* set the current recordid */
extern void SetHistoryRecordId( UInt16 recordId ) HISTORY_SECTION;

#endif

