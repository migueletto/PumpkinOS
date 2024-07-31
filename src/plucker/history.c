/*
 * $Id: history.c,v 1.34 2003/12/29 20:22:29 nordstrom Exp $
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

#include "debug.h"
#include "document.h"
#include "genericfile.h"
#include "util.h"

#include "history.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void RollHistoryIndex( Int16* index ) HISTORY_SECTION;
static void RestoreData( void ) HISTORY_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static History history;



/* Return next history index */
static void RollHistoryIndex
    (
    Int16* index    /* pointer to history index ( upon return, it will
                       contain the next history index ) */
    )
{
    ( *index )++;
    if ( HISTORYSIZE <= *index )
        *index = 0;
}



/* Restore data for current record from history */
static void RestoreData( void )
{
    MetaRecord* meta;
    MemHandle   handle;
    UInt16      recordId;

    recordId    = history.records[ history.currentRecord ].recordId;
    handle      = GetMetaHandle( recordId, false );
    meta        = MemHandleLock( handle );

    DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ),
        &history.records[ history.currentRecord ],
        sizeof( HistoryData ) - sizeof( Int16 ) );

    MemHandleUnlock( handle );
}



/* Initialize the history structure */
void InitHistory( void )
{
    MemSet( &history, sizeof( History ), 0 );

    history.addHistory              = false;
    history.records[ 0 ].recordId   = HOME_PAGE_ID;
}



/* Read history from session record */
void ReadHistory
    (
    History* recordPtr,     /* pointer to session record */
    UInt16   availableSize  /* amount of data available */
    )
{
    MemSet( &history, sizeof( History ), 0 );
    if ( sizeof( History ) < availableSize )
        availableSize = sizeof( History );
    if ( 0 < availableSize )
        MemMove( &history, recordPtr, availableSize );
}



/* Save history in session record */
void SaveHistory
    (
    History* recordPtr /* pointer to session record */
    )
{
    DmWrite( recordPtr, 0, &history, sizeof( History ) );
}



/* Add record to history */
void AddToHistory
    (
    const UInt16 recordId  /* record ID */
    )
{
    if ( ! history.addHistory ) {
        history.addHistory = true;
        return;
    }

    RollHistoryIndex( &history.currentRecord );
    history.lastRecord = history.currentRecord;

    if ( history.currentRecord == history.firstRecord )
        RollHistoryIndex( &history.firstRecord );

    history.records[ history.currentRecord ].recordId = recordId;
}



/* Store data for first paragraph in current record from history */
void SetHistoryFirstParagraph
    (
    const Int16   offset, /* offset to paragraph in record */
    const YOffset y       /* paragraph's y coordinate within record */
    )
{
    history.records[ history.currentRecord ].firstVisibleParagraph  = offset;
    history.records[ history.currentRecord ].firstParagraphY        = y;
}



#ifdef STORE_LAST_VISIBLE
/* Store data for last paragraph in current record from history */
void SetHistoryLastParagraph
    (
    const Int16 offset,   /* offset to paragraph in record */
    const YOffset y       /* paragraph's y coordinate within record */
    )
{
    history.records[ history.currentRecord ].lastVisibleParagraph   = offset;
    history.records[ history.currentRecord ].lastParagraphY         = y;
}
#endif



/* Store vertical offset for current record from history */
void SetHistoryVerticalOffset
    (
    const YOffset offset  /* vertical offset in record */
    )
{
    history.records[ history.currentRecord ].verticalOffset = offset;
}



/* Get the add history status */
const Boolean GetAddHistory( void )
{
    return history.addHistory;
}



/* Get the current record ID from the history */
const UInt16 GetHistoryCurrent( void )
{
    return history.records[ history.currentRecord ].recordId;
}



/* Get the next record from the history */
const UInt16 GetHistoryNext( void )
{
    if ( history.currentRecord == history.lastRecord )
        return NO_RECORD;

    RollHistoryIndex( &history.currentRecord );

    RestoreData();

    return history.records[ history.currentRecord ].recordId;
}



/* Get the previous record from the history */
const UInt16 GetHistoryPrev( void )
{
    if ( history.currentRecord == history.firstRecord )
        return NO_RECORD;

    if ( --history.currentRecord < 0 )
        history.currentRecord = HISTORYSIZE - 1;

    RestoreData();

    return history.records[ history.currentRecord ].recordId;
}



/* get the history data */
History* GetHistoryPtr( void )
{
    return &history;
}



/* reset verticalOffsets in the history */
void ResetHistoryVerticalOffsets( void )
{
    Int16 i;
    for ( i = 0 ; i < HISTORYSIZE ; i++ )
        history.records[ i ].verticalOffset = NO_VERTICAL_OFFSET;
}



/* set character position of start */
void SetHistoryCharacterPosition( Int16 position )
{
    history.records[ history.currentRecord ].characterPosition = position;
}



/* set character position of start */
Int16 GetHistoryCharacterPosition( void )
{
    return history.records[ history.currentRecord ].characterPosition;
}



/* set current record */
void SetHistoryRecordId( UInt16 recordId )
{
    history.records[ history.currentRecord ].recordId = recordId;
}
