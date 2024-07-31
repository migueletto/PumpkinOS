/*
 * $Id: uncompress.c,v 1.49 2004/02/03 17:53:09 chrish Exp $
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
#include <DLServer.h>   /* for def of dlkUserNameBufSize */

#define NOZLIBDEFS
#include "SysZLib.h"

#include "const.h"
#include "debug.h"
#include "document.h"
#include "list.h"
#include "paragraph.h"
#include "resourceids.h"
#include "util.h"

#include "uncompress.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define COUNT_BITS  3
#define BUFSIZE     4096
#define ZBUFSIZE    1024

static const Char uncompressName[] = "Plkr-UnCoMpReSS";



/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    UInt16    tableUID;
    UInt16    recordNo;
} TableIndex;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static MemHandle AllocateUncompressRecord( Header* record ) UNCOMPRESS_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static DmOpenRef    uncompressBuffer;
static MemHandle    uncompressTextHandle;
static MemHandle    uncompressImageHandle;
static MemHandle    uncompressHandle;
static MemHandle    ownerID;
static LinkedList   tableIndexList = NULL;


/* Create buffer used when uncompressing records */
Err CreateUncompressBuffer( void )
{
    Err err;

    err = errNone;

    uncompressBuffer = DmOpenDatabaseByTypeCreator( UncompressType,
                        ViewerAppID, dmModeReadWrite );
    if ( uncompressBuffer == NULL ) {
        MemHandle   handle;
        UInt16      dbIndex;
        UInt16      cardNo;
        UInt8       i;

        cardNo = 0;

        err = DmCreateDatabase( cardNo, uncompressName, ViewerAppID,
                UncompressType, false );
        if ( err == errNone ) {
            uncompressBuffer = DmOpenDatabaseByTypeCreator( UncompressType,
                                ViewerAppID, dmModeReadWrite );
            if ( uncompressBuffer != NULL ) {
                for ( i = 0; i < 3; i++ ) {
                    dbIndex = dmMaxRecordIndex;
                    handle  = DmNewRecord( uncompressBuffer, &dbIndex, 1 );
                    if ( handle == NULL ) {
                        err = DmGetLastErr();
                        break;
                    }
                    DmReleaseRecord( uncompressBuffer, dbIndex, true );
                }
            }
            else {
                err = DmGetLastErr();
            }
            if ( err != errNone )
                DeleteUncompressBuffer();
        }
    }
    if ( err == errNone )
        tableIndexList = ListCreate();

    return err;
}



/* Close and delete uncompress buffer */
void DeleteUncompressBuffer( void )
{
    UInt16  cardNo;
    LocalID dbID;

    if ( uncompressBuffer == NULL )
        return;

    ListRelease( tableIndexList );

    DmCloseDatabase( uncompressBuffer );

    cardNo  = 0;
    dbID    = DmFindDatabase( cardNo, uncompressName );
    if ( dbID != 0 )
        DmDeleteDatabase( cardNo, dbID );
    uncompressBuffer = NULL;
}



/* Free a table's UncompressRecord */
void FreeUncompressTableRecord( UInt16 uid )
{
    TableIndex  *index;

    index = ListFirst( tableIndexList );
    while ( index != NULL ) {
        if ( index->tableUID == uid ) {
            DmRemoveRecord( uncompressBuffer, index->recordNo );
            ListTakeOut( tableIndexList, index );
            MSG( _( "Table list take out #%d (uid %d)\n", index->recordNo, index->tableUID ) );
            SafeMemPtrFree( index );
            break;
        }
        index = ListNext( tableIndexList, index );
    }
}



/* Allocate space for doc, image or special uncompress record */
static MemHandle AllocateUncompressRecord
    (
    Header* record  /* pointer to record */
    )
{
    MemHandle     hand;
    UInt16        index;
    TableIndex*   entry;

    hand = NULL;
    if ( record->type == DATATYPE_PHTML_COMPRESSED ) {
        uncompressTextHandle = DmResizeRecord( uncompressBuffer,
                                UNCOMPRESS_TEXT_ID, record->size );
        hand = uncompressTextHandle;
    }
    else if ( record->type == DATATYPE_TBMP_COMPRESSED ) {
        uncompressImageHandle = DmResizeRecord( uncompressBuffer,
                                    UNCOMPRESS_IMAGE_ID, record->size );
        hand = uncompressImageHandle;
    }
    else if ( record->type == DATATYPE_LINKS_COMPRESSED ) {
        uncompressHandle = DmResizeRecord( uncompressBuffer,
                            UNCOMPRESS_SPECIAL_ID, record->size );
        hand = uncompressHandle;
    }
    else if ( record->type == DATATYPE_TABLE_COMPRESSED ) {
        index = dmMaxRecordIndex;
        uncompressHandle  = DmNewRecord( uncompressBuffer, &index,
                                record->size );
        hand = uncompressHandle;
        entry = SafeMemPtrNew ( sizeof ( TableIndex ) );
        entry->recordNo = index;
        entry->tableUID = record->uid;
        MSG( _( "Table list add #%d (uid %d)\n", entry->recordNo, entry->tableUID ) );
        ListAppend ( tableIndexList, entry );
    }
    return hand;
}



/* Return handle to doc uncompress record */
MemHandle GetUncompressTextHandle( void )
{
    return uncompressTextHandle;
}



/* Return handle to image uncompress record */
MemHandle GetUncompressImageHandle( void )
{
    return uncompressImageHandle;
}



/* Uncompress DOC compressed text/image */
MemHandle UnDoc
    (
    Header* record  /* pointer to compressed record */
    )
    /* THROWS */
{
    MemHandle   uncompressHandle;
    UInt8*      inBuf;
    UInt8*      outBuf;
    UInt8*      uncompressPtr;
    UInt16      headerSize;
    UInt16      docSize;
    UInt16      i;
    UInt16      j;
    UInt16      k;

    headerSize  = sizeof( Header ) + record->paragraphs * sizeof( Paragraph );
    docSize     = MemPtrSize( record ) - headerSize;
    THROW_IF( record->size < docSize, dmErrCorruptDatabase );

    uncompressHandle = AllocateUncompressRecord( record );
    THROW_IF( uncompressHandle == NULL, dmErrMemError );

    j               = 0;
    k               = 0;
    inBuf           = GET_DATA( record );
    outBuf          = SafeMemPtrNew( BUFSIZE );
    uncompressPtr   = MemHandleLock( uncompressHandle );
    while ( j < docSize ) {
        i = 0;
        while ( i < BUFSIZE && j < docSize ) {
            UInt16 c;

            c = (UInt16) inBuf[ j++ ];
            if ( 0 < c && c < 9 ) {
                while ( 0 < c-- )
                    outBuf[ i++ ] = inBuf[ j++ ];
            }
            else if ( c < 0x80 )
                outBuf[ i++ ] = c;
            else if ( 0xc0 <= c ) {
                outBuf[ i++ ] = ' ';
                outBuf[ i++ ] = c ^ 0x80;
            }
            else {
                Int16 m;
                Int16 n;

                c <<= 8;
                c  += inBuf[ j++ ];

                m   = ( c & 0x3fff ) >> COUNT_BITS;
                n   = c & ( ( 1 << COUNT_BITS ) - 1 );
                n  += 2;

                do {
                    outBuf[ i ] = outBuf[ i - m ];
                    i++;
                } while ( 0 < n-- );
            }
        }
        DmWrite( uncompressPtr, k, outBuf, i );
//debug(1, "XXX", "uncompress %d bytes at %d", i, k);
//debug_bytes(1, "XXX", &uncompressPtr[k], i);
        k += BUFSIZE;
    }
    SafeMemPtrFree( outBuf );
    MemHandleUnlock( uncompressHandle );

    return uncompressHandle;
}



/* Set the username that is used to key the zlib uncompress routine */
void SetUncompressKey
    (
    const UInt8* key     /* pointer to 40 byte string, or NULL */
    )
    /* THROWS */
{
    ReleaseUncompressKey();

    if ( key != NULL ) {
        UInt8* newMem;

        ownerID = MemHandleNew( OWNER_ID_HASH_LEN );
        THROW_IF( ownerID == NULL, dmErrMemError );

        newMem = MemHandleLock( ownerID );
        MemMove ( newMem, key, OWNER_ID_HASH_LEN );
        MemHandleUnlock( ownerID );
    }
}



/* Release allocated memory for username */
void ReleaseUncompressKey( void )
{
    if ( ownerID != NULL ) {
        MemHandleFree( ownerID );
        ownerID     = NULL;
    }
}



/* Uncompress ZLib compressed text/image */
MemHandle UnZip
    (
    Header* record  /* pointer to compressed record */
    )
    /* THROWS */
{
    MemHandle   uncompressHandle;
    z_stream    z;
    UInt32      outSize;
    UInt8*      inBuf;
    UInt8*      outBuf;
    UInt8*      uncompressPtr;
    Int16       err;
    UInt16      docSize;
    UInt16      headerSize;
    UInt8       keyBuf[ OWNER_ID_HASH_LEN ];
    UInt16      keyLen;

    headerSize  = sizeof( Header ) + record->paragraphs * sizeof( Paragraph );
    docSize     = MemPtrSize( record ) - headerSize;
    keyLen      = (ownerID == NULL) ? 0 : min( docSize, OWNER_ID_HASH_LEN );
    THROW_IF( record->size < docSize, dmErrCorruptDatabase );

    uncompressHandle = AllocateUncompressRecord( record );
    THROW_IF( uncompressHandle == NULL, dmErrMemError );
                                                                            
    inBuf = GET_DATA( record );
    MemSet( &z, sizeof( z ), 0 );
    if ( ownerID != NULL ) {
        UInt16 i;
        MemPtr ownerIDPtr;

        MSG( "ownerid is not NULL\n" );

        ownerIDPtr = MemHandleLock( ownerID );
        for ( i = 0;  i < keyLen;  i++ )
            keyBuf[ i ] = inBuf[ i ] ^ ((UInt8*) ownerIDPtr)[ i ];
        MemHandleUnlock ( ownerID );
        z.next_in   = keyBuf;
        z.avail_in  = keyLen;
    }
    else {
        z.next_in   = inBuf;
        z.avail_in  = docSize;
    }
    outBuf = SafeMemPtrNew( ZBUFSIZE );

    err = inflateInit( &z );
    if ( err != Z_OK ) {
        SafeMemPtrFree( outBuf );
        ErrThrow( errZLibMemError );
    }

    outSize         = 0;
    uncompressPtr   = MemHandleLock( uncompressHandle );
    do {
        if ( z.avail_in == 0 && 0 < keyLen ) {
            z.next_in   = inBuf + keyLen;
            z.avail_in  = docSize - keyLen;
            keyLen      = 0;
        }
        z.next_out  = outBuf;
        z.avail_out = ZBUFSIZE;

        err = inflate( &z, Z_SYNC_FLUSH );
        DmWrite( uncompressPtr, outSize, outBuf, ZBUFSIZE - z.avail_out );

        outSize += ZBUFSIZE - z.avail_out;
    } while ( err == Z_OK );
    inflateEnd( &z );

    MemHandleUnlock( uncompressHandle );
    SafeMemPtrFree( outBuf );

    if ( err != Z_STREAM_END )
        ErrThrow( errZLibMemError );

    return uncompressHandle;
}



/* Return CRC-32 of buflen bytes in specified buffer */
UInt32 CRC32
  (
   UInt32       seed,
   const Char*  buf,
   UInt16       buflen
   )
{
    UInt32 crc;

    crc = crc32( seed, (UInt8 *)buf, buflen );

    return crc;
}

