/*
 * $Id: vfsfile.h,v 1.16 2004/01/04 10:35:05 nordstrom Exp $
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

#ifndef PLUCKER_VFSFILE_H
#define PLUCKER_VFSFILE_H

#include "viewer.h"
#include "genericfile.h"

#define PATH_LEN            256


typedef enum {
    ENUMERATECARD_DOCS  = 1,
    ENUMERATECARD_FONTS = 2
} EnumerateCardType;


/* return UID for last record in document, i.e. the max UID */
extern UInt16 GetVFSMaxUID( void ) VFSFILE_SECTION;

/* find volume ref with given volume label */
extern UInt16 FindVolRefNum( Char* label ) VFSFILE_SECTION;

/* Find record with given uid in document, returns true if successful */
extern MemHandle FindVFSRecord( UInt16 uid, Int16* index ) VFSFILE_SECTION;

/* Return a handle to a record */
extern MemHandle ReturnVFSRecordHandle( UInt16 uid ) VFSFILE_SECTION;

/* Return a handle to next record */
extern MemHandle ReturnNextVFSRecordHandle( UInt16* index ) VFSFILE_SECTION;

/* Open the database so we can get a fileRef */
extern Err OpenVFSDocument( DocumentInfo* docInfo ) VFSFILE_SECTION;

/* Close the specified file */
extern void CloseVFSDocument() VFSFILE_SECTION;

/* Delete specified document */
extern void DeleteVFSDocument( DocumentInfo* docInfo ) VFSFILE_SECTION;

/* enumerate mounted volumes */
extern void EnumerateCards( EnumerateCardType enumerateWhat ) VFSFILE_SECTION;

/* Rename document and its associated meta document */
extern void RenameVFSDocument( Char* newName,
                DocumentInfo* docInfo, Char* newFilename ) VFSFILE_SECTION;

/* Beam Plucker document located on external card */
extern void BeamVFSDocument( DocumentInfo* docInfo ) VFSFILE_SECTION;

/* Return number of records in document */
extern UInt16 GetVFSNumOfRecords( void ) VFSFILE_SECTION;

/* Free allocated handle */
extern void FreeVFSRecordHandle( MemHandle* handle ) VFSFILE_SECTION;

/* Get a handle by index number */
extern MemHandle ReturnVFSRecordHandleByIndex( const UInt16 index ) 
                    VFSFILE_SECTION;

#endif

