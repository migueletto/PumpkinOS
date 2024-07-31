/*
 * $Id: genericfile.c,v 1.30 2003/12/29 20:21:39 nordstrom Exp $
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
 *
 * -*- mode: c; indent-tabs-mode: nil; -*-
 */

#include <PalmOS.h>
#include <VFSMgr.h>
#include <DLServer.h>   /* for def of dlkUserNameBufSize */

#include "cache.h"
#include "const.h"
#include "debug.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "session.h"
#include "uncompress.h"
#include "os.h"
#include "image.h"
#include "paragraph.h"
#include "ramfile.h"
#include "vfsfile.h"
#include "metadocument.h"
#include "metadata.h"
#include "libraryform.h"
#include "doclist.h"

#include "genericfile.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define INDEX_RECORD    1
#define SELECT_DELETE   0


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt16 reservedRecords[ MAX_RESERVED ];



/* Check the owner ID in the doc, if any, against the HotSync ID */
static Boolean ValidOwnerID( void )
    /* THROWS */
{
    Header*                 header;
    UInt16                  numOfRecords;
    MetadataElementHeader*  metadata;
    MemHandle               handle;
    Boolean                 result;

    /* default to showing the document */
    result = true;

    handle = FindRecord( METADATA_ID, NULL );
    if ( handle == NULL )
        return result;

    header = MemHandleLock( handle );
    THROW_IF( header->type != DATATYPE_METADATA, dmErrCorruptDatabase );

    numOfRecords = *( (UInt16*)( header + 1) );
    metadata     = (MetadataElementHeader*)( ( (UInt16*)( header + 1 ) ) + 1 );

    while ( 0 < numOfRecords-- ) {
        if ( metadata->typecode == METADATA_OWNER_ID ) {
            Err     err;
            Char    nameBuf[ dlkUserNameBufSize ];
            UInt32  docID;
            UInt32  deviceID;

            /* change default to not showing the document */
            result = false;

            err = DlkGetSyncInfo( NULL, NULL, NULL, nameBuf, NULL, NULL );
            if ( err != errNone )
                break;

            docID       = *( (UInt32*) ( metadata->argument ) );
            deviceID    = CRC32( 0L, nameBuf, StrLen( nameBuf ) );
            MSG( _( "document owner is 0x%lx, device owner is 0x%lx\n",
                    docID, deviceID ) );
            if ( docID == deviceID ) {
                Char   keyBuf[ OWNER_ID_HASH_LEN ];
                UInt16 i;             
                for ( i = 0;  i < 10;  i++ ) {
                    deviceID = CRC32( deviceID, nameBuf, StrLen( nameBuf ) );
                    /* MSG( _( "crc val %d is %08lx\n", i, deviceID) ); */
                    *( (UInt32*)( keyBuf + ( 4 * i ) ) ) = deviceID;
                }
                MSG( _( "key is %08lx %08lx %08lx %08lx %08lx %08lx %08lx "
                        "%08lx %08lx %08lx...\n",
                       *( (UInt32*)( keyBuf ) ),
                       *( (UInt32*)( keyBuf + 4 ) ),
                       *( (UInt32*)( keyBuf + 8 ) ),
                       *( (UInt32*)( keyBuf + 12 ) ),
                       *( (UInt32*)( keyBuf + 16 ) ),
                       *( (UInt32*)( keyBuf + 20 ) ),
                       *( (UInt32*)( keyBuf + 24 ) ),
                       *( (UInt32*)( keyBuf + 28 ) ),
                       *( (UInt32*)( keyBuf + 32 ) ),
                       *( (UInt32*)( keyBuf + 36 ) ) ) );
                SetUncompressKey( (UInt8 *)keyBuf );
                result = true;
            }
            else {
                result = false;
            }
            break;
        }
        else {
            /* advance to the next metadata record */
            metadata = NEXT_METADATA_RECORD( metadata );
        }
    }
    MemHandleUnlock( handle );

    return result;
}



/* compare UIDs */
Int16 CompareUID
    (
    void*       item,
    MemHandle   handle
    )
{
    UID*    currentUID;
    UInt16* wantedUID;
    Int16   result;

    wantedUID   = item;
    currentUID  = MemHandleLock( handle );

    if ( *wantedUID == currentUID->uid )
        result = 0;
    else if ( *wantedUID < currentUID->uid )
        result = -1;
    else
        result = 1;

    MemHandleUnlock( handle );
    
    return result;
}



/* compare names */
Int16 CompareNames
    (
    void*       item,
    MemHandle   handle
    )
{
    Char*   currentName;
    Char*   wantedName;
    Int16   result;

    wantedName  = item;
    currentName = MemHandleLock( handle );
    result      = StrCompare( wantedName, currentName );
    MemHandleUnlock( handle );

    return result;
}



/* Check the compression type and retrieve IDs for reserved records */
DocType GetIndexData( void )
    /* THROWS */
{
    DocType         docType;
    MemHandle       handle;
    IndexRecord*    indexRecord;
    UInt16*         reserved;
    UInt16          i;

    MemSet( &reservedRecords, MAX_RESERVED * sizeof( UInt16 ), 0 );

    handle = FindRecord( INDEX_RECORD, NULL );
    if ( handle == NULL )
        ErrThrow( dmErrCorruptDatabase );

    indexRecord = MemHandleLock( handle );
    docType     = (DocType) indexRecord->version;
    if ( docType != DOCTYPE_ZLIB && docType != DOCTYPE_DOC ) {
        MemHandleUnlock( handle );
        FreeRecordHandle( &handle );

        ErrThrow( dmErrCorruptDatabase );
    }

    reserved = indexRecord->reserved;
    for ( i = 0; i < indexRecord->records; i++ ) {
        UInt16 name;
        UInt16 id;

        name    = *reserved;
        id      = *( reserved + 1 );
        if ( name < MAX_RESERVED )
            reservedRecords[ name ] = id;
        reserved += sizeof( UInt16 );
    }
    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );

    return docType;
}



/* Return the reserved record's unique ID */
UInt16 GetReservedID
    (
    UInt16 name /* record "name" */
    )
{
    if ( name < MAX_RESERVED )
        return reservedRecords[ name ];
    else
        return 0;
}



/* open given document */
Err OpenDocument
    (
    DocumentInfo* docInfo
    )
{
    DocType docType;

    if ( docInfo == NULL )
       return dmErrInvalidParam;

    ResetCache();
    ResetRecordReferences();
    /* no key, by default */
    SetUncompressKey( NULL );

    ErrTry {
        Err err;
        if ( docInfo->location == RAM ) {
            err = OpenRAMDocument( docInfo );
        }
        else { /* on external card */
            err = OpenVFSDocument( docInfo );
        }
        THROW_IF( err != errNone, err );
        StrCopy( Prefs()->docName, docInfo->name );
        Prefs()->location = docInfo->location;

        docType = GetIndexData();
        if ( docType == DOCTYPE_ZLIB ) {
            if ( SupportZLib() )
                Uncompress = UnZip;
            else
                ErrThrow( errNoZLibSupport );
        }
        else if ( docType == DOCTYPE_DOC ) {
            Uncompress = UnDoc;
        }

        /* make sure the document contains a "home page" */
        if ( reservedRecords[ HOME_PAGE ] != 0 ) {
            MemHandle hand;

            hand = ReturnRecordHandle( reservedRecords[ HOME_PAGE ] );
            if ( hand == NULL )
                ErrThrow( dmErrCorruptDatabase );

            FreeRecordHandle( &hand );
        }
        else {
            ErrThrow( dmErrCantOpen );
        }
        /* if the document is "protected" then check the owner-ID */
        if ( reservedRecords[ METADATA ] != 0 && ! ValidOwnerID() ) {
            ErrThrow( errInvalidOwner );
        }
        OpenMetaDocument( docInfo->name, docInfo->location, docInfo->created );
        InitSessionData();
    }
    ErrCatch( err ) {
        CloseDocument();

        switch ( err ) {
            case errNoZLibSupport:
                MSG( "ZLib compressed documents not supported\n" );
                FrmCustomAlert( warnNoZLibSupport, docInfo->name, NULL, NULL );
                break;

            case dmErrCorruptDatabase:
                MSG( "not a valid Plucker document\n" );
                FrmCustomAlert( errUnknownType, docInfo->name, NULL, NULL );
                break;

            case errInvalidOwner:
                FrmCustomAlert( warnInvalidOwner, docInfo->name, NULL, NULL );
                break;

            case dmErrCantFind:
            case vfsErrFileNotFound:
                break;

            default:
                MSG( "couldn't open/initialize document\n" );
                FrmCustomAlert( warnBrokenDocument, docInfo->name, NULL, NULL );
                break;
        }
        return err;
    } ErrEndCatch

    return errNone;
}



/* open last used document in previous session */
Err OpenLastDocument( void )
{
    UInt16          index;
    DocumentInfo*   docInfo;
    
    ErrTry { 
        FindDocData( Prefs()->docName, ALL_ELEMENTS, &index );
        docInfo = DocInfo( index );
    } 
    ErrCatch( UNUSED_PARAM( err ) ) {
        docInfo = GetLastDocInfo();
    } ErrEndCatch

    return OpenDocument( docInfo );
}



/* close current document */
void CloseDocument( void )
{
    ReleaseUncompressKey();
    CloseMetaDocument();
    CloseRAMDocument();
    CloseVFSDocument();

    MemSet( &reservedRecords, MAX_RESERVED * sizeof( UInt16 ), 0 );
}



/* remove current document */
static void DeleteCurrentDocument( void )
    /* THROWS */
{
    DeleteDocument( DocInfo( ReturnLastIndex() ) );
}



/* remove given document */
void DeleteDocument
    (
    DocumentInfo* docInfo
    )
    /* THROWS */
{
    THROW_IF( docInfo == NULL, dmErrInvalidParam );

    if ( docInfo->location == RAM )
        DeleteRAMDocument( docInfo );
    else
        DeleteVFSDocument( docInfo );
}



/* beam given document */
void BeamDocument
    (
    DocumentInfo* docInfo
    )
    /* THROWS */
{
    THROW_IF( docInfo == NULL, dmErrInvalidParam );

    if ( docInfo->location == RAM )
        BeamRAMDocument( docInfo );
    else
        BeamVFSDocument( docInfo );
}



/* rename given document */
void RenameDocument
    (
    Char*           newName,
    DocumentInfo*   docInfo,
    Char*           filename  /* new filename if on VFS */
    )
    /* THROWS */
{
    THROW_IF( docInfo == NULL, dmErrInvalidParam );

    if ( docInfo->location == RAM )
        RenameRAMDocument( newName, docInfo );
    else
        RenameVFSDocument( newName, docInfo, filename );
}


void DoDeleteDocument()
{
    UInt16 selectedButton;
    
    selectedButton = FrmCustomAlert( confirmDelete,
                                     Prefs()->docName, NULL, NULL );
    if ( selectedButton == SELECT_DELETE ) {
        CloseDocument();
        ErrTry {
            DeleteCurrentDocument();
            DeleteMetaDocument( Prefs()->docName,
                                Prefs()->location );
            CloseDocList();
            ReleaseDocInfoList();
            FrmGotoForm( GetValidForm( frmLibrary ) );
        }
        ErrCatch( UNUSED_PARAM( err ) ) {
            FrmAlert( errCannotDeleteDoc );
            OpenLastDocument();
            InitSessionData();
            FrmGotoForm( GetMainFormId() );
        } ErrEndCatch
    }
}

