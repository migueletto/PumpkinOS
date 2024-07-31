/*
 * $Id: genericfile.h,v 1.16 2003/12/20 14:00:29 nordstrom Exp $
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

#ifndef PLUCKER_GENERICFILE_H
#define PLUCKER_GENERICFILE_H

#include "viewer.h"


#define RAM  0


/* structure used for data saved in Palm database */
typedef struct {
    Char    name[ dmDBNameLength ];
    UInt16  cardNo;
    UInt32  size;
    UInt32  created;
    UInt16  attributes;
    UInt16  categories;
    UInt16  location;
    Boolean active;
    UInt8   reserved;
    UInt32  timestamp;
    Char    data[ 0 ];
} DocumentData;


/* structure used for internal document list */
typedef struct {
    Char    name[ dmDBNameLength ];
    UInt16  cardNo;
    UInt32  size;
    UInt32  created;
    UInt16  attributes;
    UInt16  categories;
    UInt16  location;
    Boolean active;
    UInt8   reserved;
    UInt32  timestamp;
    UInt16  volumeRef;
    UInt16  numRecords;
    Char*   filename;
} DocumentInfo;


/* Reserved record "names" */
#define MAX_RESERVED        5

#define HOME_PAGE           0
#define EXTERNAL_BOOKMARKS  1
#define PLUCKER_LINKS       2
#define CATEGORY            3
#define METADATA            4

typedef enum {
    DOCTYPE_UNKNOWN = 0,
    DOCTYPE_DOC     = 1,
    DOCTYPE_ZLIB    = 2 
} DocType;


typedef struct {
    UInt16 uid;
} UID;


typedef struct {
    UInt16 uid;
    UInt16 version;
    UInt16 records;
    UInt16 reserved[ 0 ];
} IndexRecord;


typedef Int16 CompareFunc( void* item, MemHandle handle );

/* compare UIDs */
extern Int16 CompareUID( void* item, MemHandle handle );

/* conmpare names */
extern Int16 CompareNames( void* item, MemHandle handle );

/* Check the compression type and retrieve IDs for reserved records */
extern DocType GetIndexData( void ) GENERICFILE_SECTION;

/* Return the reserved record's unique ID */
extern UInt16 GetReservedID( UInt16 name ) GENERICFILE_SECTION;

#define HOME_PAGE_ID            GetReservedID( HOME_PAGE )
#define EXTERNAL_BOOKMARKS_ID   GetReservedID( EXTERNAL_BOOKMARKS )
#define PLUCKER_LINKS_ID        GetReservedID( PLUCKER_LINKS )
#define CATEGORY_ID             GetReservedID( CATEGORY )
#define METADATA_ID             GetReservedID( METADATA )

/* open given document */
extern Err OpenDocument( DocumentInfo* docInfo ) GENERICFILE_SECTION;

/* close current document */
extern void CloseDocument( void ) GENERICFILE_SECTION;

/* open last used document in previous session */
extern Err OpenLastDocument( void ) GENERICFILE_SECTION;

/* remove given document */
extern void DeleteDocument( DocumentInfo* docInfo ) GENERICFILE_SECTION;

/* beam given document */
extern void BeamDocument( DocumentInfo* docInfo ) GENERICFILE_SECTION;

/* rename given document */
extern void RenameDocument( Char* newName, DocumentInfo* docInfo,
    Char* newFilename ) GENERICFILE_SECTION;

/* show dialog, delete document, remove from doclist, etc.*/
extern void DoDeleteDocument();


PLKR_GLOBAL MemHandle (*FindRecord)( UInt16 uid, Int16* index );
PLKR_GLOBAL UInt16 (*GetNumOfRecords)( void );
PLKR_GLOBAL MemHandle (*ReturnRecordHandle)( const UInt16 uid );
PLKR_GLOBAL MemHandle (*ReturnRecordHandleByIndex)( const UInt16 index );
PLKR_GLOBAL MemHandle (*ReturnNextRecordHandle)( UInt16* index );
PLKR_GLOBAL UInt16 (*GetMaxUID)( void );
PLKR_GLOBAL void (*FreeRecordHandle)( MemHandle* handle );

#endif

