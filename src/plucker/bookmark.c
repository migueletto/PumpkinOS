/*
 * $Id: bookmark.c,v 1.48 2004/05/16 11:50:55 nordstrom Exp $
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

#include "control.h"
#include "debug.h"
#include "document.h"
#include "fullscreenform.h"
#include "genericfile.h"
#include "history.h"
#include "resourceids.h"
#include "util.h"
#include "metadocument.h"

#include "bookmark.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define ADD_BOOKMARK_LEN        25
#define EDIT_BOOKMARK_LEN       25
#define ADD_BOOKMARK            0
#define EDIT_BOOKMARK           1
#define NO_PARAGRAPHS           0
#define NO_BOOKMARKS            0
#define MAX_BOOKMARK_LIST_LEN   12
#define BOOKMARK_HEADER_LEN     ( 3 * sizeof( UInt16 ) )

#define GET_ENTRIES( ptr )         ( ( ptr )[ 2 ] * 256 + ( ptr )[ 3 ] )
#define GET_OFFSET( ptr )          ( ( ptr )[ 4 ] * 256 + ( ptr )[ 5 ] )
#define GET_EXTFIRSTFIELD( ptr )   ( ( ptr )[ 0 ] * 256 + ( ptr )[ 1 ] )
#define GET_EXTSECONDFIELD( ptr )  ( ( ptr )[ 2 ] * 256 + ( ptr )[ 3 ] )


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef HistoryData BookmarkData;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void DrawListItem( Int16 itemNum, RectangleType* bounds,
                    Char** itemsText ) BOOKMARK_SECTION;
static UInt16 InitBookmarkList( ListType* list ) BOOKMARK_SECTION;
static UInt16 InitExtBookmarkList( Char** nameList, UInt16 offset ) BOOKMARK_SECTION;

/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static MemHandle    nameListHandle;
static Boolean      isPopupList;
static Char         addBookmark[ ADD_BOOKMARK_LEN ];
static Char         editBookmark[ EDIT_BOOKMARK_LEN ];



/* Callback function that draws list items */
static void DrawListItem
    (
    Int16           itemNum,    /* item number */
    RectangleType*  bounds,     /* pointer to a RectangleType structure that
                                   specifies the bounds for the list item */
    Char**          itemsText   /* pointer to an array of text strings */
    )
{
    FontID oldFont;

    if ( isPopupList && ( itemNum == ADD_BOOKMARK ||
                          itemNum == EDIT_BOOKMARK ||
                          itemNum <= CountExtBookmarks() + 1 ) )
        oldFont = FntSetFont( boldFont );
    else if ( itemNum < CountExtBookmarks() )
        oldFont = FntSetFont( boldFont );
    else
        oldFont = FntSetFont( stdFont );

    if ( itemsText[ itemNum ] != NULL )
        WinDrawChars( itemsText[ itemNum ], StrLen( itemsText[ itemNum ] ), 
            bounds->topLeft.x, bounds->topLeft.y );

    FntSetFont( oldFont );
}



/* Populate list with bookmarks, return number of bookmarks in list */
static UInt16 InitBookmarkList
    (
    ListType* list  /* pointer to list */
    )
{
    UInt16      entries;
    UInt16      extEntries;
    UInt16      extraListItems;
    UInt16      i;
    MemHandle   handle;
    Char**      nameList;
    UInt8*      bookmarkPtr;

    handle          = NULL;
    nameList        = NULL;
    bookmarkPtr     = NULL;
    /* default is "Add bookmark" and "View bookmarks" */
    entries         = 0;
    extraListItems  = 2;

    extEntries = CountExtBookmarks();

    handle = ReturnMetaHandle( INTERNAL_BOOKMARKS_ID, NO_PARAGRAPHS );
    if ( handle != NULL ) {
        if ( isPopupList )
            extraListItems = 2;
        else
            extraListItems = 0;

        bookmarkPtr     = MemHandleLock( handle );
        entries        += GET_ENTRIES( bookmarkPtr );
        bookmarkPtr    += BOOKMARK_HEADER_LEN;
    }
    else {
        if ( ! isPopupList ) {
            extraListItems = 0;

            if ( extEntries == 0 ) {
                LstSetListChoices( list, nameList, NO_BOOKMARKS );
                LstSetDrawFunction( list, DrawListItem );

                return NO_BOOKMARKS;
            }
        }
    }
    entries += extraListItems + extEntries;

    /* Allocate arrays for name list */
    nameListHandle = MemHandleNew( entries * sizeof( *nameList ) );
    if ( nameListHandle == NULL ) {
        if ( extraListItems < entries - extEntries )
            MemHandleUnlock( handle );

        return NO_BOOKMARKS;
    }
    nameList = MemHandleLock( nameListHandle );

    if ( isPopupList ) {
        SysCopyStringResource( addBookmark, strMainAddBookmark );
        nameList[ ADD_BOOKMARK ] = addBookmark;
        SysCopyStringResource( editBookmark, strMainViewBookmark );
        nameList[ EDIT_BOOKMARK ] = editBookmark;
    }

    if ( 0 < extEntries )
        InitExtBookmarkList( nameList, extraListItems );

    if ( handle != NULL ) {
        for ( i = extraListItems + extEntries; i < entries; i++ ) {
            nameList[ i ]   = (Char *)bookmarkPtr;
            bookmarkPtr    += StrLen( (Char *)bookmarkPtr ) + 1;
        }
    }
    LstSetListChoices( list, nameList, entries );
    LstSetDrawFunction( list, DrawListItem );

    if ( isPopupList )
        LstSetHeight( list, entries );

    if ( extraListItems < entries - extEntries )
        MemHandleUnlock( handle );

    return entries;
}



/* Populate list with bookmarks ( including Add/View Bookmarks ), return number
   of bookmarks in list */
UInt16 CreatePopupBookmarkList
    (
    ListType* list  /* pointer to list */
    )
{
    isPopupList = true;
    return InitBookmarkList( list );
}



/* Populate list with bookmarks ( skip Add/View Bookmarks ), return number 
   of bookmarks in list */
UInt16 CreateBookmarkList
    (
    ListType* list
    )
{
    isPopupList = false;
    return InitBookmarkList( list );
}



/* Unlock list handle and free allocated memory */
void ReleaseBookmarkList( void )
{
    if ( nameListHandle != NULL ) {
        MemHandleUnlock( nameListHandle );
        MemHandleFree( nameListHandle );
        nameListHandle = NULL;
    }
}



/* Restore data for current bookmark, return the record ID or if
   there are no bookmarks NO_RECORD */
UInt16 RestoreBookmarkData
    (
    UInt16 index     /* index in bookmarks list */
    )
{
    MetaRecord*     meta;
    BookmarkData    bookmarkData;
    UInt8*          bookmarkPtr;
    MemHandle       handle;
    UInt16          offset;

    handle = ReturnMetaHandle( INTERNAL_BOOKMARKS_ID, NO_PARAGRAPHS );
    if ( handle == NULL )
        return NO_RECORD;

    bookmarkPtr  = MemHandleLock( handle );
    offset       = GET_OFFSET( bookmarkPtr );
    bookmarkPtr += offset + index * sizeof( BookmarkData );
    MemMove( &bookmarkData, bookmarkPtr, sizeof( BookmarkData ) );

    meta = MemHandleLock( GetMetaHandle( bookmarkData.recordId, false ) );
    DmWrite( meta, OFFSETOF( MetaRecord, verticalOffset ), &bookmarkData,
        sizeof( BookmarkData ) - sizeof( UInt16 ) );

    /* Add to history */
    AddToHistory( bookmarkData.recordId );
    SetHistoryFirstParagraph( bookmarkData.firstVisibleParagraph,
        bookmarkData.firstParagraphY );
#ifdef STORE_LAST_VISIBLE
    SetHistoryLastParagraph( bookmarkData.lastVisibleParagraph,
        bookmarkData.lastParagraphY );
#endif
    SetHistoryVerticalOffset( bookmarkData.verticalOffset );
    SetHistoryCharacterPosition( bookmarkData.characterPosition );

    MemPtrUnlock( meta );
    MemHandleUnlock( handle );

    return bookmarkData.recordId;
}



/* Add data for bookmark */
void AddBookmark
    (
    Char* name      /* name of bookmark */
    )
    /* THROWS */
{
    MetaRecord*     meta;
    BookmarkData    bookmarkData;
    UInt8*          bookmarkPtr;
    MemHandle       handle;
    UInt32          endOfRecord;
    UInt16          entries;
    UInt16          offset;
    UInt16          newSize;
    UInt16          nameLength;
    UInt16          i;

    THROW_IF( name == NULL || *name == '\0', errNoBookmarkName );

    handle = ReturnMetaHandle( INTERNAL_BOOKMARKS_ID, NO_PARAGRAPHS );
    if ( handle == NULL )
        AddBookmarkRecord( &handle );

    endOfRecord = MemHandleSize( handle );
    TrimText( name, BOOKMARKLISTWIDTH );
    nameLength  = StrLen( name ) + 1;
        
    newSize     = endOfRecord + sizeof( BookmarkData ) + nameLength;

    ResizeMetaRecord( INTERNAL_BOOKMARKS_ID, newSize, &handle );

    bookmarkPtr = MemHandleLock( handle );

    entries = GET_ENTRIES( bookmarkPtr ) + 1;
    offset  = GET_OFFSET( bookmarkPtr ) + nameLength;
    DmWrite( bookmarkPtr, sizeof( UInt16 ), &entries, sizeof( UInt16 ) );
    DmWrite( bookmarkPtr, 2 * sizeof( UInt16 ), &offset, sizeof( UInt16 ) );

    meta = MemHandleLock( GetMetaRecord() );

    bookmarkData.verticalOffset         = meta->verticalOffset;
    bookmarkData.characterPosition      = meta->characterPosition;
    bookmarkData.firstVisibleParagraph  = meta->firstVisibleParagraph;
    bookmarkData.firstParagraphY        = meta->firstParagraphY;
#ifdef STORE_LAST_VISIBLE
    bookmarkData.lastVisibleParagraph   = meta->lastVisibleParagraph;
    bookmarkData.lastParagraphY         = meta->lastParagraphY;
#endif
    bookmarkData.recordId               = GetHistoryCurrent();

    /* Write new block of bookmark data */
    DmWrite( bookmarkPtr, endOfRecord + nameLength, &bookmarkData,
        sizeof( BookmarkData ) );
    endOfRecord -= sizeof( BookmarkData );

    /* Reshuffle old blocks with bookmark data */
    for ( i = 1; i < entries; i++ ) {
        MemMove( &bookmarkData, bookmarkPtr + endOfRecord,
            sizeof( BookmarkData ) );
        DmWrite( bookmarkPtr, endOfRecord + nameLength, &bookmarkData,
            sizeof( BookmarkData ) );
        endOfRecord -= sizeof( BookmarkData );
    }

    /* Write new bookmark name */
    DmStrCopy( bookmarkPtr, offset - nameLength, name );

    MemHandleUnlock( GetMetaRecord() );
    MemHandleUnlock( handle );
}



/* Delete bookmark, return true if list is empty */
void DeleteBookmark
    (
    UInt16 index     /* index in bookmarks list */
    )
{
    BookmarkData  bookmarkData;
    MemHandle     handle;
    UInt16        entries;
    UInt16        offset;
    UInt16        tempOffset;
    UInt16        newSize;
    UInt16        nameLength;
    UInt16        i;
    UInt8*        bookmarkPtr;
    UInt8*        readPtr;

    handle = ReturnMetaHandle( INTERNAL_BOOKMARKS_ID, NO_PARAGRAPHS );
    if ( handle == NULL )
        return;

    bookmarkPtr = MemHandleLock( handle );
    entries     = GET_ENTRIES( bookmarkPtr );
    if ( entries <= 1 ) {
        MemHandleUnlock( handle );
        ReleaseBookmarkList();
        RemoveBookmarkRecord();
        return;
    }

    /* Find name string for bookmark  */
    readPtr = bookmarkPtr + BOOKMARK_HEADER_LEN;
    for ( i = 0; i < index; i++ )
        readPtr += StrLen( (Char *)readPtr ) + 1;

    tempOffset  = readPtr - bookmarkPtr;
    nameLength  = StrLen( (Char *)readPtr ) + 1;
    readPtr    += nameLength;
    for ( i = 0; i < entries - index - 1; i++ ) {
        UInt16  length;
        Char    tempString[ MAX_BOOKMARK_LEN + 1 ];

        MemMove( tempString, readPtr, StrLen( (Char *)readPtr ) + 1 );
        length = StrLen( tempString ) + 1;

        DmWrite( bookmarkPtr, tempOffset, tempString, length );

        tempOffset  += length;
        readPtr     += length;
    }

    /* Reshuffle blocks with bookmark data */
    offset  = GET_OFFSET( bookmarkPtr );
    readPtr = bookmarkPtr + offset;
    for ( i = 0; i < index; i++ ) {
        MemMove( &bookmarkData, readPtr, sizeof( BookmarkData ) );

        DmWrite( bookmarkPtr, tempOffset, &bookmarkData,
            sizeof( BookmarkData ) );
        tempOffset  += sizeof( BookmarkData );
        readPtr     += sizeof( BookmarkData );
    }
    readPtr += sizeof( BookmarkData );
    for ( i = index + 1; i < entries; i++ ) {
        MemMove( &bookmarkData, readPtr, sizeof( BookmarkData ) );

        DmWrite( bookmarkPtr, tempOffset, &bookmarkData,
            sizeof( BookmarkData ) );
        tempOffset  += sizeof( BookmarkData );
        readPtr     += sizeof( BookmarkData );
    }

    entries--;
    DmWrite( bookmarkPtr, sizeof( UInt16 ), &entries, sizeof( UInt16 ) );
    offset -= nameLength;
    DmWrite( bookmarkPtr, 2 * sizeof( UInt16 ), &offset, sizeof( UInt16 ) );

    newSize = MemHandleSize( handle ) - sizeof( BookmarkData ) - nameLength;

    MemHandleUnlock( handle );

    ResizeMetaRecord( INTERNAL_BOOKMARKS_ID, newSize, &handle );
}



/* Reset the verticalOffset fields in all the bookmarks */
void ResetBookmarkVerticalOffsets( void )
{
    UInt8*          bookmarkPtr;
    MemHandle       handle;
    UInt16          offset;
    Int16           entries;

    handle = ReturnMetaHandle( INTERNAL_BOOKMARKS_ID, NO_PARAGRAPHS );
    if ( handle == NULL )
        return;

    bookmarkPtr  = MemHandleLock( handle );
    offset       = GET_OFFSET( bookmarkPtr );
    entries      = GET_ENTRIES( bookmarkPtr );

    while ( entries-- ) {
        YOffset verticalOffset;
        verticalOffset = NO_VERTICAL_OFFSET;
        DmWrite( bookmarkPtr, offset + OFFSETOF( BookmarkData, verticalOffset ),
            &verticalOffset, sizeof( YOffset ) );
        offset += sizeof( BookmarkData );
    }

    MemHandleUnlock( handle );
}



/* Populate array with external bookmarks, return number of bookmarks */
static UInt16 InitExtBookmarkList
    (
    Char** nameList,
    UInt16 offset     /* where to start in the list */
    )
{
    UInt16      entries;
    UInt16      i;
    MemHandle   handle;
    UInt8*      bookmarkPtr;

    handle          = NULL;
    bookmarkPtr     = NULL;

    handle  = ReturnRecordHandle( EXTERNAL_BOOKMARKS_ID );

    if ( handle == NULL )
        return NO_BOOKMARKS;

    bookmarkPtr     = MemHandleLock( handle );
    bookmarkPtr    += sizeof( Header );
    entries         = GET_EXTFIRSTFIELD( bookmarkPtr );
    bookmarkPtr    += 4;

    for ( i = offset; i < entries + offset; i++ ) {
        nameList[ i ]   = (Char *)bookmarkPtr;
        bookmarkPtr    += StrLen( (Char *)bookmarkPtr ) + 1;
    }

    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );

    return entries;
}



/* Return the number of external bookmarks */
UInt16 CountExtBookmarks( void )
{
    UInt16      entries;
    MemHandle   handle;
    UInt8*      bookmarkPtr;

    handle          = NULL;
    bookmarkPtr     = NULL;
    entries         = 0;

    ErrTry {
        handle  = ReturnRecordHandle( EXTERNAL_BOOKMARKS_ID );
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        FreeRecordHandle( &handle );
        return 0;
    } ErrEndCatch

    if ( handle == NULL )
        return NO_BOOKMARKS;

    bookmarkPtr     = MemHandleLock( handle );
    bookmarkPtr    += sizeof( Header );
    entries         = GET_EXTFIRSTFIELD( bookmarkPtr );

    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );
    return entries;
}



/* Restore data for external bookmark, return the record ID or
   NO_RECORD if it doesn't exist */
UInt16 GotoExtBookmark
    (
    UInt16 index     /* index in bookmarks list */
    )
{

    UInt16      uid;
    UInt16      offset;
    MemHandle   handle;
    UInt8*      bookmarkPtr;

    handle          = NULL;
    bookmarkPtr     = NULL;
    offset          = 0;

    handle  = ReturnRecordHandle( EXTERNAL_BOOKMARKS_ID );

    if ( handle == NULL )
        return NO_RECORD;

    bookmarkPtr     = MemHandleLock( handle );
    bookmarkPtr    += sizeof( Header );
    bookmarkPtr    += GET_EXTSECONDFIELD( bookmarkPtr ) - sizeof( Header );
    bookmarkPtr    += index * 4;

    uid             = GET_EXTFIRSTFIELD( bookmarkPtr );
    offset          = GET_EXTSECONDFIELD( bookmarkPtr );

    if ( IsFullscreenformActive() )
        FsFrmGotoForm ( GetMainFormId() );
    JumpToRecord( uid, offset, NO_OFFSET );
    MemHandleUnlock( handle );

    return uid;
}
