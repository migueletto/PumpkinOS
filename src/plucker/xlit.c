/*
 * $Id: xlit.c,v 1.10 2004/04/18 15:34:48 prussar Exp $
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

#include "util.h"
#include "list.h"
#include "const.h"
#include "resourceids.h"

#include "xlit.h"




/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    DmOpenRef   dbRef;
    UInt16      index;
    MemHandle   handle;
    TransliterationHeader*  header;
} XlitEntry;




/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/

static void ProcessXlitDB ( UInt16 cardNo, LocalID dbID ) XLIT_SECTION;
static Int16 XlitCompare ( void* a, void* b, Int32 other ) XLIT_SECTION;




/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static LinkedList xlitDataList    = NULL;
static UInt16     numXlits        = 0;
static Boolean    alreadyOpened   = false;

static XlitEntry*                xlitData;
static Char**                    xlitNames;


static void ProcessXlitDB
       (
       UInt16              cardNo,
       LocalID             dbID
       )
{
    UInt16        numRecords;
    UInt16        i;
    DmOpenRef     dbRef;

    dbRef      = DmOpenDatabase( cardNo, dbID, dmModeReadOnly );

    if ( dbRef == NULL )
        return;

    numRecords = DmNumRecords( dbRef );

    for ( i = 0 ; i < numRecords ; i++ ) {
        MemHandle              handle;

        handle = DmGetRecord( dbRef, i );

        if ( handle != NULL ) {
            XlitEntry*  entry;

            entry = SafeMemPtrNew( sizeof( XlitEntry ) );

            entry->dbRef  = dbRef;
            entry->index  = i;
            entry->handle = handle;
            entry->header = MemHandleLock( handle );

            ListAppend( xlitDataList, entry );
            numXlits++;
        }
    }
}



static Int16 XlitCompare
    (
    void*   a,
    void*   b,
    Int32   other
    )
{
    return StrCompare( ( ( XlitEntry* )a )->header->name,
                       ( ( XlitEntry* )b )->header->name );
}



/* Set up xlatTable in accordance with transliteration number
   xlitNum.  Return value: Is the transliteration symmetric.
   *multibyteCharFixP is set to true iff the transliteration
   specifies a value to transliterate multibyte chars to, and
   if so then *multibyteCharFixTo is set to that value. */
Boolean SetTransliteration
    (
    UInt16   xlitNum,
    Char*    xlatTable,
    Boolean* multibyteCharFixP,
    WChar*   multibyteCharFixTo
    )
{
    TransliterationHeader* header;
    UInt16                 i;
    if ( xlitNum == 0 || numXlits + 1 <= xlitNum ) {
        for ( i = 0 ; i < 256 ; i++ ) {
            xlatTable[ i ] = i;
        }
        *multibyteCharFixP = false;
        return true;
    }
    header = xlitData[ xlitNum - 1 ].header;
    for ( i = 0 ; i < 256 ; i++ ) {
        if ( header->firstGlyph <= i && i <= header->lastGlyph ) {
            xlatTable[ i ] = ( ( ( UInt8* )header ) + header->dataOffset )
                                                    [ i - header->firstGlyph ];
        }
        else {
            if ( header->flags &
                     TRANSLITERATION_FLAG_OUT_OF_RANGE_NO_CHANGE ) {
                xlatTable[ i ] = i;
            }
            else {
                xlatTable[ i ] = header->outOfRangeMapTo;
            }
        }
    }
    if ( header->flags & TRANSLITERATION_FLAG_OUT_OF_RANGE_NO_CHANGE ) {
        *multibyteCharFixP  = true;
        *multibyteCharFixTo = header->outOfRangeMapTo;
    }
    if ( header->flags & TRANSLITERATION_FLAG_SYMMETRIC ) {
        return true;
    }
    else {
        return false;
    }
}



/* Setup transliteration popup on search form */
void SetupXlitPopup( void )
{
    ListType*           list;

    list      = GetObjectPtr( frmSearchXlitList );

    LstSetListChoices( list, xlitNames, numXlits + 1 );
    LstSetHeight( list, numXlits + 1 );
}



void OpenTransliterations( void )
{
    DmSearchStateType   stateInfo;
    Err                 err;
    UInt16              i;
    UInt16              cardNo;
    LocalID             dbID;
    Char                temp[ 100 ];

    XlitEntry*          entry;

    if ( alreadyOpened )
        return;

    alreadyOpened = true;

    numXlits    = 0;

    xlitDataList = ListCreate();

    err = DmGetNextDatabaseByTypeCreator( true, &stateInfo,
            (UInt32) XlitDBType, (UInt32) ViewerAppID,
            false, &cardNo, &dbID );

    while ( err == errNone ) {
        ProcessXlitDB( cardNo, dbID );

        err = DmGetNextDatabaseByTypeCreator( false, &stateInfo,
                (UInt32) SkinResourceType, (UInt32) ViewerAppID, false,
                &cardNo, &dbID );
    }

    if ( 0 < numXlits ) {
        xlitData = SafeMemPtrNew( sizeof( XlitEntry ) * numXlits );
        i        = 0;

        entry  = ListFirst( xlitDataList );
        while ( entry != NULL ) {
            xlitData[ i++ ] = *entry;

            SafeMemPtrFree( entry );
            entry = ListNext( xlitDataList, entry );
        }
        SysInsertionSort( xlitData, numXlits, sizeof( XlitEntry ),
            XlitCompare, 0 );
    }
    ListDelete( xlitDataList );

    xlitNames = SafeMemPtrNew( ( numXlits + 1 ) * sizeof( Char* ) );

    SysCopyStringResource( temp, strNoTransliteration );
    xlitNames[ 0 ] = SafeMemPtrNew( 1 + StrLen( temp ) );
    StrCopy( xlitNames[ 0 ], temp );

    for ( i = 1 ; i < numXlits + 1 ; i++ ) {
        Char* name;
        name           = xlitData[ i - 1 ].header->name;
        xlitNames[ i ] = SafeMemPtrNew( 1 + StrLen( name ) );
        StrCopy( xlitNames[ i ], name );
    }
}



void CloseTransliterations( void )
{
    UInt16     i;

    if ( 0 < numXlits ) {
        for ( i = 0 ; i < numXlits ; i++ ) {
            MemHandleUnlock( xlitData[ i ].handle );
            DmReleaseRecord( xlitData[ i ].dbRef, xlitData[ i ].index, false );
        }
        for ( i = 0 ; i < numXlits ; i++ ) {
            DmOpenRef  dbRef;
            dbRef = xlitData[ i ].dbRef;
            if ( dbRef != NULL ) {
                UInt16 j;
                DmCloseDatabase( dbRef );
                for ( j = i + 1 ; j < numXlits ; j++ ) {
                    if ( xlitData[ j ].dbRef == dbRef )
                        xlitData[ j ].dbRef = NULL;
                }
            }
        }
        SafeMemPtrFree( xlitData );
    }

    for ( i = 0 ; i < numXlits + 1 ; i++ ) {
        SafeMemPtrFree( xlitNames[ i ] );
    }
    SafeMemPtrFree( xlitNames );
    numXlits = 0;
}
