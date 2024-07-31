/*
 * $Id: metadocument.c,v 1.28 2003/12/29 20:33:42 nordstrom Exp $
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

#include "const.h"
#include "debug.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "session.h"
#include "history.h"
#include "os.h"
#include "paragraph.h"
#include "ramfile.h"
#include "genericfile.h"
#include "font.h"
#include "hires.h"
#include "bookmark.h"
#include "rotate.h"
#include "util.h"

#include "metadocument.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define REFERENCE_RECORD_SIZE   ( 64 + sizeof( UID ) )
#define SESSION_RECORD_SIZE     sizeof( History )
#define BOOKMARK_RECORD_SIZE    3 * sizeof( UInt16 )
#define BIT_ON                  1
#define INDEX_RECORD            1
#define MAX_DOCNAME_LEN         dmDBNameLength - 6 /* "Plkr-" + <LOCATION> */


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Boolean TextImageRecord( UInt16 uid ) METADOCUMENT_SECTION;
static void AddRecord( UInt16 uid, UInt32 size ) METADOCUMENT_SECTION;
static void InitializeMetaDocument( void ) METADOCUMENT_SECTION;
static void MetaCheckBitSize( const UInt16 id, UInt16 reference,
                MemHandle handle ) METADOCUMENT_SECTION;
static void InitializeBookmarkRecord( MemHandle handle ) METADOCUMENT_SECTION;
static void InitializeMetaRecord( MemHandle handle, UInt16 uid,
                UInt16 size ) METADOCUMENT_SECTION;
static void InitializeIndexRecord( MemHandle handle ) METADOCUMENT_SECTION;
static void InitializeReferenceRecord( MemHandle handle,
                UInt16 uid ) METADOCUMENT_SECTION;
static void InitializeSessionRecord( MemHandle handle ) METADOCUMENT_SECTION;
static void InitializeRecord( MemHandle handle, UInt16 uid,
                UInt32 size) METADOCUMENT_SECTION;
static MemHandle FindMetaRecord( UInt16 item,
                    Int16 *index ) METADOCUMENT_SECTION;
static void GetInternalIndexData( void ) METADOCUMENT_SECTION;
static Int16 CompareMetaUID( void *rec1, void *rec2, Int16 other,
                SortRecordInfoPtr rec1SortInfo,
                SortRecordInfoPtr rec2SortInfo, MemHandle appInfoH );



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static DmOpenRef        metaDocument;
static UInt16           internalRecords[ MAX_INTERNAL ];
static Boolean          enableReRender = true;



/* Callback function that compares unique IDs */
static Int16 CompareMetaUID
    (
    void*               rec1,           /* record to sort */
    void*               rec2,           /* record to sort */
    Int16               other,          /* any other custom information you want
                                           passed to the comparison function */
    SortRecordInfoPtr   rec1SortInfo,   /* structure that specify unique
                                           sorting information */
    SortRecordInfoPtr   rec2SortInfo,   /* structure that specify unique
                                           sorting information */
    MemHandle           appInfoH        /* handle to the document's application
                                           info block */
    )
{
    Int16 result;

    /*   0 if rec1.uid = rec2.uid */
    /* < 0 if rec1.uid < rec2.uid */
    /* > 0 if rec1.uid > rec2.uid */
    if ( ( (UID*) rec1 )->uid < ( (UID*) rec2 )->uid )
        result = -1;
    else if ( ( (UID*) rec2 )->uid < ( (UID*) rec1 )->uid )
        result = 1;
    else
        result = 0;

    return result;
}



/* Find record in document */
static MemHandle FindMetaRecord
    (
    UInt16      item,   /* UID to search for */
    Int16*      index   /* upon successful return, the index of the record,
                           pass NULL for this parameter if you don't want to
                           retrieve this value */
    )
    /* THROWS */
{
    UInt16  numRecords;

    THROW_IF( metaDocument == NULL, dmErrNoOpenDatabase );
    THROW_IF( item == 0, dmErrInvalidParam );

    numRecords = DmNumRecords( metaDocument );

    return SearchRAMDocument( metaDocument, CompareUID, &item, numRecords,
            index );
}



/* Initialize the meta record */
static void InitializeMetaRecord
    (
    MemHandle   handle, /* handle to record */
    UInt16      uid,    /* unique ID */
    UInt16      size    /* record size */
    )
{
    MetaRecord*     meta;

    meta = MemHandleLock( handle );

    DmSet( meta, 0, size, 0 );
    DmWrite( meta, OFFSETOF( MetaRecord, uid ), &uid, sizeof( UInt16 ) );

    MemHandleUnlock( handle );
}



/* Initialize the session data */
static void InitializeSessionRecord
    (
    MemHandle handle    /* handle to record */
    )
{
    History*    sessionPtr;
    History     history;
    UInt16      prevCoordSys;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );
    sessionPtr   = MemHandleLock( handle );

    MemSet( &history, sizeof( History ), 0 );
    history.uid                     = SESSION_DATA_ID;
    history.addHistory              = false;
    history.records[ 0 ].recordId   = HOME_PAGE_ID;
    history.font                    = Prefs()->defaultFontModeMain;
    history.lineSpacing             = Prefs()->defaultLineSpacing;
    history.paragraphSpacing        = Prefs()->defaultParagraphSpacing;
    history.rotate                  = Prefs()->defaultRotate;
    history.depth                   = Prefs()->screenDepth;
    history.toolbar                 = Prefs()->toolbar;
    history.scrollbar               = Prefs()->scrollbar;
    history.maxExtentX              = RotMaxExtentX();
    history.extentX                 = RotExtentX();
    StrCopy( history.userFontName, Prefs()->defaultMainUserFontName );

    DmWrite( sessionPtr, 0, &history, sizeof( History ) );

    MemHandleUnlock( handle );
    PalmSetCoordinateSystem( prevCoordSys );
}



/* Initialize the show images or link data record */
static void InitializeReferenceRecord
    (
    MemHandle   handle, /* handle to show images or link record */
    UInt16      uid     /* unique ID */
    )
{
    void* referencePtr;

    referencePtr = MemHandleLock( handle );

    DmSet( referencePtr, 0, REFERENCE_RECORD_SIZE, 0 );
    DmWrite( referencePtr, 0, &uid, sizeof( UInt16 ) );

    MemHandleUnlock( handle );
}



/* Initialize the bookmarks record */
static void InitializeBookmarkRecord
    (
    MemHandle handle    /* handle to record */
    )
{
    UInt8*  bookmarkPtr;
    UInt16  uid;
    UInt16  offset;

    bookmarkPtr = MemHandleLock( handle );

    uid     = INTERNAL_BOOKMARKS_ID;
    offset  = BOOKMARK_RECORD_SIZE;
    DmSet( bookmarkPtr, 0, BOOKMARK_RECORD_SIZE, 0 );
    DmWrite( bookmarkPtr, 0, &uid, sizeof( UInt16 ) );
    DmWrite( bookmarkPtr, 2 * sizeof( UInt16 ), &offset, sizeof( UInt16 ) );

    MemHandleUnlock( handle );
}



/* Initialize the index record */
static void InitializeIndexRecord
    (
    MemHandle handle    /* handle to record */
    )
{
    MetaIndexRecord*    indexPtr;
    UInt16              uid;
    UInt16              records;

    indexPtr = MemHandleLock( handle );

    uid     = INDEX_RECORD;
    records = MAX_INTERNAL;
    DmWrite( indexPtr, OFFSETOF( MetaIndexRecord, uid ), &uid,
        sizeof( UInt16 ) );
    DmWrite( indexPtr, OFFSETOF( MetaIndexRecord, records ), &records,
        sizeof( UInt16 ) );
    DmWrite( indexPtr, OFFSETOF( MetaIndexRecord, internal ), &internalRecords,
        MAX_INTERNAL * sizeof( UInt16 ) );

    MemHandleUnlock( handle );
}



static void InitializeRecord
    (
    MemHandle   handle,
    UInt16      uid,
    UInt32      size
    )
{
    if ( uid == INDEX_RECORD )
        InitializeIndexRecord( handle );
    else if ( TextImageRecord( uid ) )
        InitializeMetaRecord( handle, uid, size );
    else if ( uid == SESSION_DATA_ID )
        InitializeSessionRecord( handle );
    else if ( uid == SHOW_IMAGES_ID || uid == LINK_TABLE_ID )
        InitializeReferenceRecord( handle, uid );
    else if ( uid == INTERNAL_BOOKMARKS_ID )
        InitializeBookmarkRecord( handle );
    else
        ErrThrow( dmErrNotValidRecord );
}



/* Add record to document */
static void AddRecord
    (
    UInt16 uid, /* unique ID of record to create */
    UInt32 size /* record size */
    )
    /* THROWS */
{
    Err         err;
    MemHandle   handle;
    UInt16      docIndex;

    docIndex    = dmMaxRecordIndex;
    handle      = DmNewRecord( metaDocument, &docIndex, size );
    THROW_IF( handle == NULL, DmGetLastErr() );

    InitializeRecord( handle, uid, size );

    err = DmReleaseRecord( metaDocument, docIndex, true );
    THROW_IF( err != errNone, err );

    DmInsertionSort( metaDocument, CompareMetaUID, 0 );
}



/* Check if referenced record is a text/image record */
static Boolean TextImageRecord
    (
    UInt16 uid   /* unique ID to check */
    )
{
    UInt16 i;

    if ( uid == 0 || uid == INDEX_RECORD )
        return false;

    /* if it is not one of the reserved internal records (except for the home 
       page) then it is a text/image record */
    for ( i = 1; i < MAX_INTERNAL; i++ )
        if ( uid == internalRecords[ i ] )
            return false;
    return true;
}



/* Retrieve ID's for internal records */
static void GetInternalIndexData( void )
    /* THROWS */
{
    MemHandle handle;

    handle = FindMetaRecord( INDEX_RECORD, NULL );
    if ( handle != NULL ) {
        UInt16              i;
        UInt16*             internal;
        MetaIndexRecord*    indexRecord;

        indexRecord = MemHandleLock( handle );
        internal    = indexRecord->internal;
        for ( i = 0; i < indexRecord->records; i++ ) {
            internalRecords[ i ] = *internal;
            internal++;
        }
        MemHandleUnlock( handle );
    }
}



/* Initialize the document containing meta data */
static void InitializeMetaDocument( void )
    /* THROWS */
{
    UInt16      i;
    UInt16      maxUID;

    /* to avoid conflicts the UID for the first internal record should be
       higher than the UID for the last record in the "real" document, the
       home page should use the same UID, though */
    internalRecords[ _HOME_PAGE ]   = HOME_PAGE_ID;
    maxUID                          = GetMaxUID();
    for ( i = 1; i < MAX_INTERNAL; i++ )
        internalRecords[ i ] = maxUID + i;

    AddRecord( INDEX_RECORD, ( MAX_INTERNAL + 2 ) * sizeof( UInt16 ) );
    AddRecord( SHOW_IMAGES_ID, REFERENCE_RECORD_SIZE );
    AddRecord( SESSION_DATA_ID, SESSION_RECORD_SIZE );
    AddRecord( LINK_TABLE_ID, REFERENCE_RECORD_SIZE );
}



/* Remove meta record */
void RemoveMetaRecord
    (
    const UInt16 id /* unique ID to remove */
    )
    /* THROWS */
{
    MemHandle   handle;
    Int16       docIndex;

    handle = FindMetaRecord( id, &docIndex );
    if ( handle != NULL )
        DmRemoveRecord( metaDocument, docIndex );
}



/* Check if record has required length, if not resize it */
void MetaCheckByteSize
    (
    const UInt16    id,         /* record ID to check */
    UInt16          size,       /* required length */
    MemHandle       handle      /* handle to record */
    )
{
    UInt16 origSize;

    origSize  = MemHandleSize( handle );

    if ( origSize < size ) {
        void* recPtr;

        ResizeMetaRecord( id, size, &handle );

        recPtr = MemHandleLock( handle );
        DmSet( recPtr, origSize, size - origSize, 0 );
        MemHandleUnlock( handle );
    }
}



/* Check that the size of the record is enough to hold the required data,
   otherwise resize it */
static void MetaCheckBitSize
    (
    const UInt16    id,         /* record ID to check */
    UInt16          reference,  /* reference for the bit to check */
    MemHandle       handle      /* handle to record */
    )
{
  MetaCheckByteSize( id, reference / 8 + 1 +sizeof( UID ), handle );
}



/* Return the internal record's unique ID */
UInt16 GetInternalID
    (
    UInt16 name /* record "name" */
    )
{
    if ( name < MAX_INTERNAL )
        return internalRecords[ name ];
    else
        return 0;
}



/* Open meta document */
void OpenMetaDocument
    (
    Char*   name,       /* name of the associated document */
    UInt16  location,   /* location of the associated document */
    UInt32  date        /* creation date of the associated document */
    )
    /* THROWS */
{
    Err     err;
    UInt16  metaVersion;
    UInt16  cardNo;
    LocalID metaID;
    Char    metaName[ dmDBNameLength ];

    metaVersion = 0;
    cardNo      = 0;

    MemSet( &internalRecords, MAX_INTERNAL * sizeof( UInt16 ), 0 );

    StrCopy( metaName, "Plkr" );
    if ( location != RAM ) {
        Char locationString[ 2 ];

        StrIToA( locationString, location );
        StrCat( metaName, locationString );
    }
    StrCat( metaName, "-" );
    StrNCat( metaName, name, MAX_DOCNAME_LEN );
    metaID = DmFindDatabase( cardNo, metaName );
    if ( metaID != 0 ) {
        UInt32 metaDate;

        err = DmDatabaseInfo( cardNo, metaID, NULL, NULL, &metaVersion,
                &metaDate, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
        THROW_IF( err != errNone, err );

        if ( metaDate < date || metaVersion != MetaDocumentVersion )
            DmDeleteDatabase( cardNo, metaID );
        else {
            metaDocument = DmOpenDatabase( cardNo, metaID, dmModeReadWrite );
            GetInternalIndexData();
            return;
        }
    }
    err = DmCreateDatabase( cardNo, metaName, ViewerAppID, MetaDocumentType,
            false );
    THROW_IF( err != errNone, err );

    metaID       = DmFindDatabase( cardNo, metaName );
    metaVersion  = MetaDocumentVersion;
    err = DmSetDatabaseInfo( cardNo, metaID, NULL, NULL, &metaVersion,
            &date, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
    THROW_IF( err != errNone, err );

    metaDocument = DmOpenDatabase( cardNo, metaID, dmModeReadWrite );

    InitializeMetaDocument();
}



/* Close document */
void CloseMetaDocument( void )
{
    if ( metaDocument == NULL )
        return;

    SaveSessionData();
    DmCloseDatabase( metaDocument );
    metaDocument = NULL;
}



/* Delete meta document */
void DeleteMetaDocument
    (
    Char*   name,       /* name of the associated document */
    UInt16  location    /* location of the associated document */
    )
{
    UInt16  cardNo;
    LocalID dbID;
    Char    metaName[ dmDBNameLength ];

    if ( metaDocument != NULL )
        return;

    StrCopy( metaName, "Plkr" );
    if ( location != RAM ) {
        Char locationString[ 2 ];

        StrIToA( locationString, location );
        StrCat( metaName, locationString );
    }
    StrCat( metaName, "-" );
    StrNCat( metaName, name, MAX_DOCNAME_LEN );

    cardNo  = 0;
    dbID    = DmFindDatabase( cardNo, metaName );
    if ( dbID != 0 )
        DmDeleteDatabase( cardNo, dbID );
}



/* Get a handle to a meta record */
MemHandle ReturnMetaHandle
    (
    UInt16          id,     /* unique ID to search for */
    const UInt16    num     /* number of paragraphs in record */
    )
    /* THROWS */
{
    MemHandle handle;

    if ( metaDocument == NULL )
        return NULL;

    handle = FindMetaRecord( id, NULL );
    if ( handle == NULL && id != internalRecords[ _INTERNAL_BOOKMARKS ] ) {
        UInt32 size;

        size = sizeof( MetaRecord ) + num * sizeof( MetaParagraph );
        AddRecord( id, size );
        handle = FindMetaRecord( id, NULL );
    }
    return handle;
}



/* Resize record */
void ResizeMetaRecord
    (
    const UInt16    id,     /* unique ID to resize */
    const UInt16    size,   /* new size */
    MemHandle*      handle  /* upon successful return, the handle to the
                               record */
    )
{
    Int16   docIndex;

    *handle = FindMetaRecord( id, &docIndex );
    if ( *handle != NULL ) {
        *handle = DmResizeRecord( metaDocument, docIndex, size );
        DmInsertionSort( metaDocument, CompareMetaUID, 0 );
    }
}



/* Get the status for a specific bit in the given record */
Boolean GetBitStatus
    (
    const UInt16    id,         /* record ID to check */
    UInt16          reference   /* reference for the bit to check */
    )
{
    MemHandle handle;
    UInt8*    recordPtr;
    Int16     value;

    handle = NULL;
    ErrTry {
        handle = ReturnMetaHandle( id, 0 );
        MetaCheckBitSize( id, reference, handle );
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        return false;
    } ErrEndCatch

    recordPtr   = MemHandleLock( handle );
    recordPtr  += sizeof( UID );
    reference  -= 1;
    value       = ( *( recordPtr + reference / 8 ) >> reference % 8 ) & 0x01;
    MemHandleUnlock( handle );

    return ( value == BIT_ON );
}



/* Set the status of a bit in the specified record */
void SetBitStatus
    (
    const UInt16    id,         /* record ID to set */
    UInt16          reference,  /* reference to set */
    Boolean         setStatus   /* indicates whether the bit should be set
                                   or unset */
    )
{
    MemHandle   handle;
    UInt8*      recordPtr;
    UInt8       bitStatus;
    UInt8       byte;
    UInt8       bit;
    Int16       byteOffset;

    handle = NULL;
    ErrTry {
        handle = ReturnMetaHandle( id, 0 );
        MetaCheckBitSize( id, reference, handle );
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        return;
    } ErrEndCatch

    recordPtr   = MemHandleLock( handle );
    reference  -= 1;
    byteOffset  = reference / 8 + sizeof( UID );
    byte        = *( recordPtr + byteOffset );
    bit         = 0x01 << reference % 8;

    if ( setStatus )
        bitStatus = byte | bit;
    else
        bitStatus = byte & ~bit;

    DmWrite( recordPtr, byteOffset, &bitStatus, 1 );
    MemHandleUnlock( handle );
}



/* Add bookmark record to document */
void AddBookmarkRecord
    (
    MemHandle* handle /* upon successful return, the handle to the record */
    )
    /* THROWS */
{
    AddRecord( internalRecords[ _INTERNAL_BOOKMARKS ], BOOKMARK_RECORD_SIZE );
    *handle = ReturnMetaHandle( internalRecords[ _INTERNAL_BOOKMARKS ], 0 );
}



/* Remove bookmark record from document */
void RemoveBookmarkRecord( void )
    /* THROWS */
{
    RemoveMetaRecord( internalRecords[ _INTERNAL_BOOKMARKS ] );
}



/* Rename meta database */
void RenameMetaDocument
    (
    Char*           newName,
    UInt16          oldLocation,
    DocumentInfo*   docInfo
    )
{
    Err     err;
    Char    metaOldName[ dmDBNameLength ];
    Char    locationString[ 2 ];
    LocalID metaID;
    UInt16  metaCardNo;

    err = errNone;

    /* Remove any "left-overs" */
    metaCardNo  = 0;
    metaID      = DmFindDatabase( metaCardNo, newName );
    if ( metaID != 0 ) {
        err = DmDeleteDatabase( metaCardNo, metaID );
    }
    if ( err == errNone ) {
        /* Rename meta document ( if it exists ) */
        StrCopy( metaOldName, "Plkr" );
        if ( oldLocation != 0 ) {
            StrIToA( locationString, oldLocation );
            StrCat( metaOldName, locationString );
        }
        StrCat( metaOldName, "-" );
        StrNCat( metaOldName, docInfo->name, MAX_DOCNAME_LEN );
        metaID = DmFindDatabase( metaCardNo, metaOldName );
        if ( metaID != 0 )
            err = DmSetDatabaseInfo( metaCardNo, metaID, newName, NULL,
                                    NULL, NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL );
        else
            err = DmGetLastErr();
    }
    THROW_IF( err != errNone && err != dmErrCantFind, err );
}



/* Force re-rendering of all records */
void ReRenderAll( void )
{
    MemHandle   handle;
    UID*        r;
    UInt16      index;
    UInt16      numRecords;
    YOffset     pos;

    if ( metaDocument == NULL )
        return;

    numRecords = DmNumRecords( metaDocument );

    for ( index = 0 ; index < numRecords ; index++ ) {
         handle = DmQueryRecord( metaDocument, index );
         if ( handle == NULL )
            break;
         r = MemHandleLock( handle );
         if ( TextImageRecord( r->uid ) ) {
             pos = 0;
             DmWrite( r, OFFSETOF( MetaRecord, height ), &pos,
                 sizeof( YOffset ) );
             pos = NO_VERTICAL_OFFSET;
             DmWrite( r, OFFSETOF( MetaRecord, verticalOffset ), &pos,
                 sizeof( YOffset ) );
         }
         MemHandleUnlock( handle );
    }
    UpdateMargins();
}



/* Check if we need to re-render */
void ReRenderAllIfNeeded( void )
{
    UInt16 prevCoordSys;

    if ( ! enableReRender )
        return;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    if ( GetHistoryPtr()->depth       != Prefs()->screenDepth ||
         GetHistoryPtr()->scrollbar   != Prefs()->scrollbar ||
         GetHistoryPtr()->font        != Prefs()->fontModeMain ||
         GetHistoryPtr()->lineSpacing != Prefs()->lineSpacing ||
         GetHistoryPtr()->paragraphSpacing != Prefs()->paragraphSpacing ||
         GetHistoryPtr()->maxExtentX  != RotMaxExtentX() ||
         GetHistoryPtr()->rotate      != Prefs()->rotate ||
         GetHistoryPtr()->extentX     != RotExtentX() ||
        ( Prefs()->fontModeMain == FONT_USER &&
             StrCompare( GetHistoryPtr()->userFontName,
                Prefs()->mainUserFontName )
        )
      ) {
        ReRenderAll();
        ResetBookmarkVerticalOffsets();
        ResetHistoryVerticalOffsets();

        GetHistoryPtr()->depth      = Prefs()->screenDepth;
        GetHistoryPtr()->scrollbar  = Prefs()->scrollbar;
        GetHistoryPtr()->maxExtentX = RotMaxExtentX();
        GetHistoryPtr()->extentX    = RotExtentX();

        if ( Prefs()->individualDocumentFonts ) {
          Prefs()->fontModeMain     = GetHistoryPtr()->font;
          Prefs()->lineSpacing      = GetHistoryPtr()->lineSpacing;
          Prefs()->paragraphSpacing = GetHistoryPtr()->paragraphSpacing;
          Prefs()->rotate           = GetHistoryPtr()->rotate;
          StrCopy( Prefs()->mainUserFontName, GetHistoryPtr()->userFontName );
          LoadUserFont( GetUserFontNumber( Prefs()->mainUserFontName, 
                            true, fontCacheDoc ) );
          RefreshCustomFonts();
          SetFontStyles();
        }
        else {
          GetHistoryPtr()->font             = Prefs()->fontModeMain;
          GetHistoryPtr()->lineSpacing      = Prefs()->lineSpacing;
          GetHistoryPtr()->paragraphSpacing = Prefs()->paragraphSpacing;
          GetHistoryPtr()->rotate           = Prefs()->rotate;
          StrCopy( GetHistoryPtr()->userFontName, Prefs()->mainUserFontName );
        }
    }

    PalmSetCoordinateSystem( prevCoordSys );
}



/* activate/deactive check for re-rendering */
void SetReRenderCheck
      (
      Boolean check
      )
{
    enableReRender = check;
}

