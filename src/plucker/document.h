/*
 * $Id: document.h,v 1.69 2004/02/05 00:40:36 prussar Exp $
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

#ifndef PLUCKER_DOCUMENT_H
#define PLUCKER_DOCUMENT_H

#include "prefsdata.h"
#include "util.h"


#define ADD_TO_HISTORY  true
#define FROM_HISTORY    false


typedef enum {
    DIRECTION_DOWN = 0,
    DIRECTION_UP,
    DIRECTION_NONE
} DirectionType;

typedef enum {
    DATATYPE_PHTML              = 0,
    DATATYPE_PHTML_COMPRESSED   = 1,
    DATATYPE_TBMP               = 2,
    DATATYPE_TBMP_COMPRESSED    = 3,
    DATATYPE_MAILTO             = 4,
    DATATYPE_LINK_INDEX         = 5,
    DATATYPE_LINKS              = 6,
    DATATYPE_LINKS_COMPRESSED   = 7,
    DATATYPE_BOOKMARKS          = 8,
    DATATYPE_CATEGORY           = 9,
    DATATYPE_METADATA           = 10,
    DATATYPE_STYLE_SHEET        = 11,
    DATATYPE_FONT_PAGE          = 12,
    DATATYPE_TABLE              = 13,
    DATATYPE_TABLE_COMPRESSED   = 14,
    DATATYPE_MULTIIMAGE         = 15
} PluckerDataType;


/* records in a Plucker document always starts with a header */
typedef struct {
    UInt16          uid;
    UInt16          paragraphs;
    UInt16          size;
    //PluckerDataType type;
    UInt8           type;
    UInt8           flags;
} Header;

#define HEADER_FLAG_CONTINUATION  1


/* Entries 2-6 in this type must correspond to the first five entries
   in HistoryData */
/* the meta record is stored in the temporary files */
typedef struct {
    //UInt16          uid;
    UInt32          uid;
    YOffset         verticalOffset;
    Int16           characterPosition;
    Int16           firstVisibleParagraph;
    YOffset         firstParagraphY;
    YOffset         height;
} MetaRecord;


/* Call whenever the last uncompressed PHTML record might be overwritten */
void ResetLastUncompressedPHTML( void ) DOCUMENT_SECTION;

/* View record ( or image ) */
extern Boolean ViewRecord( UInt16 recordId, Boolean newPage,
            Int16 pOffset, Int16 cOffset, WriteModeType mode ) 
            DOCUMENT_SECTION;

/* Find the necessary adjustment for the search pattern to be visible in the record */
extern YOffset GetSearchAdjustment( void ) DOCUMENT_SECTION;

/* Return handle to current record */
extern MemHandle GetCurrentRecord( void ) DOCUMENT_SECTION;

/* Return a handle to a record */
extern MemHandle GetRecordHandle( const UInt16 recordId ) DOCUMENT_SECTION;

/* Return handle to meta record */
extern MemHandle GetMetaRecord( void ) DOCUMENT_SECTION;

/* Return a handle to the meta data */
extern MemHandle GetMetaHandle( const UInt16 recordId,
                    Boolean update ) DOCUMENT_SECTION;

/* Move page up/down */
extern void DoPageMove( const YOffset pixels ) DOCUMENT_SECTION;

/* Set height of record to zero */
extern void ResetHeight( void ) DOCUMENT_SECTION;

/* reset the record references (e.g. when the document is closed) */
extern void ResetRecordReferences( void ) DOCUMENT_SECTION;

/* Initialize format of percentage indicator */
extern void InitializeOffsetFormat( const Char* lang ) DOCUMENT_SECTION;

/* Update percentage value for the vertical offset */
extern void UpdateVerticalOffset( const UInt16 percent ) DOCUMENT_SECTION;

extern void CopyRecord( Char* buf, Int16 length ) DOCUMENT_SECTION;
extern void TextifyRecord( Char* buf, Int16 length ) DOCUMENT_SECTION;
extern void DoFindSelectedWord ( Coord x, Coord y ) DOCUMENT_SECTION;

/* Are we on the last page? */
extern Boolean OnLastScreen( void ) DOCUMENT_SECTION;

/* Are we on the first page? */
extern Boolean OnFirstScreen( void ) DOCUMENT_SECTION;

/* Scale scrollbar data */
extern YOffset ScrollbarScale( Int16 value ) DOCUMENT_SECTION;

extern void SetWorking( void ) DOCUMENT_SECTION;

extern YOffset GetVerticalOffset( void ) DOCUMENT_SECTION;

extern YOffset GetMinVerticalOffset( void ) DOCUMENT_SECTION;

/* get uid of current record */
extern UInt16 GetCurrentRecordId( void ) DOCUMENT_SECTION;

/* get record id corresponding to delta from current position */
extern UInt16 DeltaToRecordId( YOffset delta, Int16* cOffsetPtr )
                  DOCUMENT_SECTION;

/* Get the (estimated) height of a sequence of records */
extern YOffset GetSequenceHeight( void ) DOCUMENT_SECTION;

/* Get the next/previous record in a sequence of records */
extern UInt16 GetSequentialRecordId( DirectionType direction )
                DOCUMENT_SECTION;

/* Like GetVerticalOffset(), but within sequence */
extern YOffset GetSequenceOffset( void ) DOCUMENT_SECTION;

/* Get the first/last record in the sequence */
extern UInt16 GetSequenceBoundaryRecordId( UInt16 recordId,
        DirectionType direction, UInt16* recordNumPtr ) DOCUMENT_SECTION;

/* get the next/prev record in a sequence */
extern MemHandle GetSequentialRecordHandle ( UInt16* recordNumPtr,
            DirectionType direction ) DOCUMENT_SECTION;

/* Get the first visible record id in a sequence */
extern UInt16 GetFirstVisibleRecordId( void ) DOCUMENT_SECTION;

/* Set the scrollbar direction */
#ifdef HAVE_ROTATE
void SetScrollbarDirection( DirectionType direction ) DOCUMENT_SECTION;
#endif

#endif

void select_utf8(int on);
