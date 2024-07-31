/*
 * $Id: cache.c,v 1.20 2004/05/16 11:50:55 nordstrom Exp $
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
#include "util.h"
#include "list.h"
#include "os.h"

#include "cache.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define FTRNUM_START           100
#define MINIMUM_FREE_SPACE     262144
#define MAX_DATA_SIZE          64000
#define STORAGE_HEAP_LOCATION  0,1

static const Char cacheDBName[] = "Plkr-Cache";

typedef struct {
    CacheType   type;
    Int32       refID;
    UInt16      size;
    MemHandle   recordHandle;
    void*       dataPtr;
    void        (*CleanerCallback)( void** dataPtr );
} CacheNode;


/***********************************************************************
 *
 *      Local Functions
 *
 ***********************************************************************/
static Err GetCacheNode( CacheType type, Int32 refID, CacheNode** refNode );
static void RemoveCacheNodeRecord( CacheNode* node );
static void RemoveCacheNode( CacheNode* node );
static UInt32 StorageHeapFree ( void );


/***********************************************************************
 *
 *      Private Variables
 *
 ***********************************************************************/
static LinkedList   cache;
static DmOpenRef    cacheDB;


/* Check if this refID has a cache node */
Boolean HasCacheNode
    (
    CacheType   type,
    Int32       refID   /* reference ID to search for */
    )
{
    CacheNode* refNode;

    return GetCacheNode( type, refID, &refNode ) == errNone;
}



/* check if this reference ID is cached, return it by the object pointer */
static Err GetCacheNode
    (
    CacheType   type,
    Int32       refID,  /* reference ID to search for */
    CacheNode** refNode /* return the reference node */
    )
{
    CacheNode* node;

    node = ListFirst( cache );
    while ( node != NULL )  {
        if ( node->type == type && node->refID == refID )
            break;
        node = ListNext( cache, node );
    }
    *refNode = node;

    if ( node == NULL )
        return errCacheNotFound;
    else
        return errNone;
}



/* Remove a record from the cache database identifying by node */
static void RemoveCacheNodeRecord
    (
    CacheNode* node
    )
{
    UInt16 index;

    if ( node->recordHandle == NULL )
        return;
    index = DmSearchRecord( node->recordHandle, &cacheDB );
    DmReleaseRecord( cacheDB, index, true );
    DmRemoveRecord( cacheDB, index );
    node->recordHandle = NULL;
}



/* remove node from cache */
static void RemoveCacheNode
    (
    CacheNode* node
    )
{
    if ( node != NULL ) {
        if ( node->CleanerCallback != NULL )
            node->CleanerCallback( &(node->dataPtr) );
        else
            RemoveCacheNodeRecord( node );
        ListTakeOut( cache, node );
        SafeMemPtrFree( node );
    }
}



/* recall a specific node out of cache by its reference ID */
MemPtr LoadFromCache
    (
    CacheType type,
    Int32     refID  /* reference ID to use */
    )
{
    Err         err;
    CacheNode*  node;

    MSG( _( "Loading from cache: refID %ld (type %d)\n", refID, type ) );

    /* Try to find this reference ID in our cache. If it's there,
       assign the memory pointer to its location */
    node    = NULL;
    err     = GetCacheNode( type, refID, &node );

    if ( err != errNone ) {
        MSG( _( "Not found\n" ) );
        return NULL;
    }

    MSG( _( "found list or ptr\n" ) );
    return node->dataPtr;
}



/* add pointer's address as a new entry into the cache */
Err AddPtrToCache
    (
    CacheType type,
    Int32     refID,    /* reference ID this data represents */
    void*     dataPtr,
    void*     cleanerCallbackFunc
    )
{
    CacheNode* node;

    while ( StorageHeapFree() < MINIMUM_FREE_SPACE && ! ListIsEmpty( cache ) )
        RemoveCacheNode( ListFirst( cache ) );

    if ( StorageHeapFree() < MINIMUM_FREE_SPACE )
        return errNone;

    node = SafeMemPtrNew( sizeof *node );
    MemSet( node, sizeof *node, 0 );

    node->type            = type;
    node->refID           = refID;
    node->dataPtr         = dataPtr;
    node->CleanerCallback = cleanerCallbackFunc;

    ListAppend( cache, node );

    MSG( _( "Adding into cache refID: %ld (type %d)\n", refID, type ) );
    return errNone;
}



/* Allocate space in the storage heap for cache-related data */
MemHandle AllocateCacheRecord
    (
    CacheType type,
    Int32     refID,
    UInt32    size
    )
{
    Err         err;
    CacheNode*  node;
    UInt16      index;

    node    = NULL;
    err     = GetCacheNode( type, refID, &node );

    if ( err != errNone )
        return NULL;

    if ( node->recordHandle != NULL ) {
        index = DmSearchRecord( node->recordHandle, &cacheDB );
        node->recordHandle = DmResizeRecord( cacheDB, index, size );
    }
    else {
        index  = dmMaxRecordIndex;
        node->recordHandle = DmNewRecord( cacheDB, &index, size );
        if ( node->recordHandle == NULL ) {
            RemoveCacheNode( node );
            return NULL;
        }
    }

    return node->recordHandle;
}



/* Remove from the storage heap the cache-related data */
void RemoveCacheRecord
    (
    CacheType type,
    Int32     refID
    )
{
    CacheNode* node;
    Err        err;

    node = NULL;
    err  = GetCacheNode( type, refID, &node );

    if ( err == errNone )
        RemoveCacheNodeRecord( node );
}



/* Initialize the Cache. Needed to be called only when plucker starts */
Err InitializeCache( void )
{
    UInt16 cardNo;
    Err    err;

    /* create new cache */
    cacheDB = DmOpenDatabaseByTypeCreator( CacheDBType, ViewerAppID,
        dmModeReadWrite );
    if ( cacheDB != NULL )
        RemoveCache();

    cardNo = 0;
    err = DmCreateDatabase( cardNo, cacheDBName, ViewerAppID,
        CacheDBType, false );
    if ( err == errNone ) {
        cacheDB = DmOpenDatabaseByTypeCreator( CacheDBType, ViewerAppID,
            dmModeReadWrite );
        err = DmGetLastErr();
        if ( err != errNone )
            RemoveCache();
        else
            cache = ListCreate();
    }
/*    DebugDatabaseSize( cacheDB ); */
    return err;
}



/* Clear the cache. Called either when Plucker is shutting down, or another
   document is loaded in memory */
void ResetCache( void )
{
    CacheNode* node;

    /* clear any data in the cache */
    node = ListFirst( cache );
    while ( node != NULL ) {
        CacheNode *next;

        next = ListNext( cache, node );
        RemoveCacheNode( node );
        node = next;
    }
}



/* Remove the cache. Called when Plucker is shutting down */
void RemoveCache( void )
{
    UInt16  cardNo;
    LocalID dbID;

    ResetCache();
    ListDelete( cache );
    cache = NULL;

    DmCloseDatabase( cacheDB );
    cacheDB = NULL;
    cardNo = 0;
    dbID   = DmFindDatabase( cardNo, cacheDBName );
    if ( dbID != 0 )
        DmDeleteDatabase( cardNo, dbID );
}



static UInt32 StorageHeapFree ( void )
{
    UInt16  heapID;
    //UInt32  total;
    UInt32  free;
    UInt32  maxsize;

    heapID  = MemHeapID( STORAGE_HEAP_LOCATION );
    //total   = MemHeapSize( heapID );
    MemHeapFreeBytes( heapID, &free, &maxsize );
    MSG( _( "Cache storage heap: %ld total, %ld used, %ld avail\n",
            total, total - free, maxsize ) );

    return free;
}

