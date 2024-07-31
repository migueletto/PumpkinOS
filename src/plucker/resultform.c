/*
 * $Id: resultform.c,v 1.51 2004/05/08 09:04:54 nordstrom Exp $
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
#include "control.h"
#include "debug.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "search.h"
#include "util.h"
#include "DIA.h"
#include "font.h"
#include "hires.h"
#include "grayfont.h"

#include "resultform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define DEFAULT_X           2
#define DEFAULT_Y           28
#define DEFAULT_EXTENT_X    156
#define MAX_RESULT_SONY_SILK_MIN   16
#define MAX_RESULT_NORMAL          9


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
static struct {
    RectangleType   coordinates;
    UInt16          record;
    UInt16          pos;
    UInt16          length;
} SearchResult[ MAX_RESULT_SONY_SILK_MIN ];


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void ClearSearchResult( void ) RESULTFORM_SECTION;
static void ShowCancel( void ) RESULTFORM_SECTION;
static void ShowCancelMore( void ) RESULTFORM_SECTION;
static void UpdateStatus( Boolean findMore ) RESULTFORM_SECTION;
static void ResultFormInit( void ) RESULTFORM_SECTION;
static Boolean SearchResultHandleEvent(
                Boolean startFromBeginning ) RESULTFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Coord            extentX;
static Coord            topLeftX;
static Coord            topLeftY;
static Coord            lineHeight;
static FormType*        resultForm;
static UInt16           resultFormID;
static Int16            maxResult;
static RectangleType    rect;
static Int16            count;
static Int16            lineMatch;
static Boolean          newLine;
static FieldType*       fldPtr;
static Char             matches[ MAX_PATTERN_LEN + 25 ];
static Char             noMatches[ MAX_PATTERN_LEN + 25 ];
static Char             searching[ 21 ];
static Char             pattern[ MAX_PATTERN_LEN + 1 ];
static FontID           initialFontStyle;
#ifdef HAVE_GRAY_FONT
static Char             oldOrientation;
#endif            



/* Clear result data variables */
static void ClearSearchResult( void )
{
    MemSet( &SearchResult, sizeof( SearchResult ), 0 );
    count           = 0;
    newLine         = false;
    rect.topLeft.y  = topLeftY;
}



/* Write header info for search result, return true if end of record has
   been reached */
Boolean DrawResultHeader
    (
    Char* header    /* header string */
    )
{
    UInt16 len;
    UInt16 deltaX;
    UInt16 deltaY;
    UInt16 coordY;
    FontID oldFontStyle;

    oldFontStyle = FntSetFont( stdFont );

    if ( newLine ) {
        newLine         = false;
        rect.topLeft.y += lineHeight;
        count++;
    }

    len     = FntCharsWidth( header, StrLen( header ) );
    deltaX  = ( extentX - len ) / 2;
    coordY  = rect.topLeft.y + lineHeight / 2;

    WinEraseRectangle( &rect, 0 );

    WinDrawLine( topLeftX, coordY, deltaX, coordY );
    deltaY = 5;
    HiResAdjust( (Int16 *)&deltaY, sonyHiRes | handeraHiRes );
    WinDrawChars( header, StrLen( header ),
        topLeftX + deltaX, coordY - deltaY );
    deltaY = 3;
    HiResAdjust( (Int16 *)&deltaY, sonyHiRes | handeraHiRes );
    WinDrawLine( topLeftX + deltaX + len + 1, coordY, extentX - deltaY,
        coordY );

    if ( maxResult - 1 <= count ) {
        count           = 0;
        newLine         = false;
        rect.topLeft.y  = topLeftY;
        return true;
    }

    FntSetFont( oldFontStyle );

    return false;
}



/* Write search result, return true if end of record has been reached */
Boolean DrawResultString
    (
    UInt16  record, /* record ID */
    UInt16  pos,    /* position of search pattern in record */
    UInt16  length, /* length of found area in bytes */
    Char*   result  /* result string */
    )
{
    count++;

    rect.topLeft.y += lineHeight;

    SearchResult[ count ].coordinates   = rect;
    SearchResult[ count ].record        = record;
    SearchResult[ count ].pos           = pos;
    SearchResult[ count ].length        = length;

    WinDrawChars( result, StrLen( result ), rect.topLeft.x, rect.topLeft.y );

    if ( maxResult - 1 <= count ) {
        count           = 0;
        newLine         = false;
        rect.topLeft.y  = topLeftY;
        return true;
    }
    newLine = true;

    return false;
}



/* Display Cancel button */
static void ShowCancel( void )
{
    FrmHideObject( resultForm, FrmGetObjectIndex( resultForm, frmResultStop ) );
    FrmShowObject( resultForm, FrmGetObjectIndex( resultForm,
                                frmResultCancel ) );
}



/* Display Cancel and Find More buttons */
static void ShowCancelMore( void )
{
    ShowCancel();
    FrmShowObject( resultForm, FrmGetObjectIndex( resultForm,
                                frmResultFindMore ) );
}



/* Display current status: "Matches for <pattern>" or "No matches for
   <pattern>" */
static void UpdateStatus
    (
    Boolean findMore    /* true if more records remains to be searched */
    )
{
    if ( ! findMore && count == 0 )
        FldSetTextPtr( fldPtr, noMatches );
    else
        FldSetTextPtr( fldPtr, matches );

    FldDrawField( fldPtr );
}



/* Initialize the result form */
static void ResultFormInit( void )
{
    Char           str[ 30 ];
    RectangleType  stopBounds;
    Coord          actualExtent;

    initialFontStyle = FntGetFont();
    SetMainStyleFont( DEFAULTSTYLE );
    lineHeight       = FntLineHeight();

    topLeftX         = DEFAULT_X;
    topLeftY         = DEFAULT_Y;
    extentX          = DEFAULT_EXTENT_X;
    HiResAdjust( &topLeftX, sonyHiRes | handeraHiRes );
    HiResAdjust( &topLeftY, sonyHiRes | handeraHiRes );
    HiResAdjust( &extentX, sonyHiRes | handeraHiRes );
    rect.extent.y    = lineHeight;
    rect.topLeft.x   = topLeftX;
    rect.topLeft.y   = topLeftY;
    rect.extent.x    = extentX;

    resultFormID = GetValidForm( frmResult );
    resultForm   = FrmGetFormPtr( resultFormID );

    FrmGetObjectBounds( resultForm,
        FrmGetObjectIndex( resultForm, frmResultStop ),
        &stopBounds );
    actualExtent = stopBounds.topLeft.y - DEFAULT_Y;
    HiResAdjust( &actualExtent, sonyHiRes | handeraHiRes );
    maxResult = actualExtent / FntLineHeight();

    GetSearchString( pattern );

    SysCopyStringResource( searching, strResultSearching );

    SysCopyStringResource( str, strResultMatches );
    StrPrintF( matches, "%s \"%s\"", str, pattern );

    SysCopyStringResource( str, strResultNoMatches );
    StrPrintF( noMatches, "%s \"%s\"", str, pattern );

    fldPtr = GetObjectPtr( frmResultStatus );
    FldSetTextPtr( fldPtr, searching );

    FrmDrawForm( resultForm );

    ClearSearchResult();
}



/* Event handler for the result form */
static Boolean SearchResultHandleEvent
    (
    Boolean startFromBeginning  /* start from the first record in DB */
    )
{
    Boolean handled;
    Boolean done;
    Boolean findMore;

    handled     = false;
    done        = true;
    findMore    = false;

    for ( ;; ) {
        EventType event;

        do {
            EvtGetEvent( &event, evtWaitForever );

            if ( SysHandleEvent( &event ) )
                continue;

            handled = false;

            switch ( event.eType ) {
                case ctlSelectEvent:
                    if ( event.data.ctlEnter.controlID == frmResultStop ) {
                        ShowCancelMore();
                        UpdateStatus( false );
                        handled = true;
                    }
                    else
                        handled = false;
                    break;

                case penUpEvent:
                    EvtFlushPenQueue();
                    handled = false;
                    break;

                case keyDownEvent:
                    ShowCancelMore();
                    UpdateStatus( false );
                    handled = true;
                    break;

                case appStopEvent:
                    EvtAddEventToQueue( &event );
                    return false;

                default:
                    handled = false;
            }

            /* Check if the form can handle the event */
            if ( ! handled )
                FrmHandleEvent( resultForm, &event );
            else
                return true;

        } while ( EvtEventAvail() );

        if ( Prefs()->searchMode == SEARCH_DOWN_IN_DOC && startFromBeginning ) {
            SetSearchFromHere();
            startFromBeginning = false;
        }
        done = SearchDocument( pattern, startFromBeginning, &findMore,
                   Prefs()->searchMode );

        startFromBeginning = false;

        if ( done ) {
            if ( findMore )
                ShowCancelMore();
            else
                ShowCancel();
            UpdateStatus( findMore );

            return true;
        }
    }
}



/* Event handler for the result form */
Boolean ResultFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    RectangleType resultRect;

    Boolean handled;

    resultRect.topLeft.x = topLeftX;
    resultRect.topLeft.y = topLeftY;
    resultRect.extent.x  = extentX;
    resultRect.extent.y  = maxResult * lineHeight;

    handled = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmResultFindMore ) {
                WinEraseRectangle( &resultRect, 0 );

                FldSetTextPtr( fldPtr, searching );
                FldDrawField( fldPtr );

                FrmHideObject( resultForm, FrmGetObjectIndex( resultForm,
                                            frmResultCancel ) );
                FrmHideObject( resultForm, FrmGetObjectIndex( resultForm,
                                            frmResultFindMore ) );
                FrmShowObject( resultForm, FrmGetObjectIndex( resultForm,
                                            frmResultStop ) );

                ClearSearchResult();

                handled = SearchResultHandleEvent( false );
                break;
            }
            else if ( event->data.ctlEnter.controlID != frmResultCancel )
                break;

            FntSetFont( initialFontStyle );
            FrmReturnToForm( GetMainFormId() );
            FrmUpdateForm( GetMainFormId(), frmRedrawUpdateCode );
            handled = true;
            break;

        case penDownEvent: {
            Coord x;
            Coord y;
            x = event->screenX;
            y = event->screenY;
            HiResAdjust( &x, sonyHiRes );
            HiResAdjust( &y, sonyHiRes );
            if ( TopLeftY() < y &&
                 y < resultRect.topLeft.y + resultRect.extent.y
               ) {
                Int16 i;

                lineMatch = NOT_FOUND;

                for ( i = 0; i < maxResult; i++ ) {
                    if ( RctPtInRectangle( x, y,
                             &SearchResult[ i ].coordinates )
                       ) {
                        WinInvertRectangle( &SearchResult[ i ].coordinates, 0 );
                        lineMatch = i;
                        break;
                    }
                    handled = true;
                }
            }
            break;
        }

        case penUpEvent: {
            Coord x;
            Coord y;
            x = event->screenX;
            y = event->screenY;
            HiResAdjust( &x, sonyHiRes );
            HiResAdjust( &y, sonyHiRes );
            if ( lineMatch != NOT_FOUND ) {
                WinInvertRectangle( &SearchResult[ lineMatch ].coordinates,
                    0 );

                if ( RctPtInRectangle( x, y,
                        &SearchResult[ lineMatch ].coordinates ) ) {
                    EventType match;

                    FrmReturnToForm( GetMainFormId() );

                    MemSet( &match, sizeof( EventType ), 0 );

                    match.eType                         = frmGotoEvent;
                    match.data.frmGoto.formID           = GetMainFormId();
                    match.data.frmGoto.recordNum        =
                                SearchResult[ lineMatch ].record;
                    match.data.frmGoto.matchPos         =
                                SearchResult[ lineMatch ].pos;
                    match.data.frmGoto.matchLen         =
                                SearchResult[ lineMatch ].length;
                    match.data.frmGoto.matchFieldNum    = 0;
                    match.data.frmGoto.matchCustom      = 0;

                    EvtAddEventToQueue( &match );
                }
                SndPlaySystemSound( sndClick );
                handled = true;
            }
            break;
        }

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
#ifdef HAVE_GRAY_FONT
            oldOrientation = GrayFntSetOrientation( GRAY_FONT_NORMAL );
#endif
            ResultFormInit();
            handled = SearchResultHandleEvent( true );
            break;

        case frmCloseEvent:
#ifdef HAVE_GRAY_FONT
            GrayFntSetOrientation( oldOrientation );
#endif
            FntSetFont( initialFontStyle );
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif            
            handled = false;
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}

