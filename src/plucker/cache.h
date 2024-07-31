/*
 * $Id: cache.h,v 1.9 2004/02/14 01:13:40 chrish Exp $
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

#ifndef PLUCKER_CACHE_H
#define PLUCKER_CACHE_H

#include "viewer.h"

#define errCacheDisabled  1
#define errDataTooLarge   2
#define errCacheFull      3
#define errCacheNotFound  4

typedef enum {
    IMAGE       = 0x01,
    IMAGEHANDLE = 0x02,
    IMAGELIST   = 0x04,
    ANCHORLIST  = 0x08
    /* must continue with ... 0x10, 0x20, 0x40, 0x80, etc */
} CacheType;

/* Check if this refID has a cache node */
extern Boolean HasCacheNode ( CacheType type, Int32 refID );

/* recall a specific element out of cache by its reference ID */
extern void* LoadFromCache( CacheType type, Int32 refID );

/* add pointer's address as a new entry into the cache */
extern Err AddPtrToCache( CacheType type, Int32 refID, void* dataPtr,
    void* cleanerCallbackFunc );

/* Allocate space in the storage heap for cache-related data */
extern MemHandle AllocateCacheRecord( CacheType type, Int32 refID,
    UInt32 size );

/* Remove from the storage heap the cache-related data */
void RemoveCacheRecord( CacheType type, Int32 refID );

/* Initialize the Cache. Needed to be called only when plucker starts */
extern Err InitializeCache( void );

/* Clear the cache. Called either when plucker shutting down, or another
   document is loaded in memory */
extern void ResetCache( void );

/* Remove the cache. Called when Plucker is shutting down */
extern void RemoveCache( void );

#endif

