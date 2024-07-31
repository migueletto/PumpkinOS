/*
 * $Id: libraryform.c,v 1.107 2004/05/19 11:40:03 nordstrom Exp $
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

#include <PalmOS.h>
#include <VFSMgr.h>
#include <BmpGlue.h>
#include <TxtGlue.h>

#include "debug.h"
#include "dimensions.h"
#include "doclist.h"
#include "fiveway.h"
#include "font.h"
#include "genericfile.h"
#include "hires.h"
#include "jogdial.h"
#include "mainform.h"
#include "metadocument.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "screen.h"
#include "session.h"
#include "timeout.h"
#include "util.h"
#include "vfsfile.h"
#include "list.h"
#include "control.h"
#include "keyboard.h"
#include "grayfont.h"
#include "DIA.h"

#include "libraryform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define MAX_COLUMNS         4
#define DOC_OPEN            0
#define DOC_CATEGORIZE      1
#define DOC_DELETE          2
#define DOC_RENAME          3
#define DOC_UNREAD          4
#define DOC_BEAM            5
#define SELECT_OK           0

#define CATEGORY_MODES      2
#define CATEGORY_TRIGGER    0
#define CATEGORY_FILE       1

#define sizeStringLength    9
#define titleStringLength  10

#define POPUP_TIMEOUT      50  /* ticks */
#define MAX_TITLE_LENGTH   30

#define PROGRESS_NO_ERROR   0

#define MAX_LOOKUP_CHARS        10
#define LOOKUP_BUFFER_SIZE    ( 2 * MAX_LOOKUP_CHARS + 1 )

#define MSG_TOPLEFT_X         10
#define MSG_TOPLEFT_Y         45
#define MSG_EXTENT_X          140
#define MSG_EXTENT_Y          17

#define MAX_MESSAGE_LENGTH    160

/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    UInt16        index;
    RectangleType bounds;
} DisplayType;

typedef struct {
    Char          name[ dmDBNameLength ];
    Char          size[ sizeStringLength ];
    Char          date[ dateStringLength ];
} FormatType;

typedef struct {
    BitmapType* bitmap;
    MemHandle   handle;
    Coord       height;
    Coord       width;
} TypeIconNode;


typedef struct {
    UInt16      volumeRef;
    BitmapType* bitmap;
    MemHandle   handle;
    Coord       height;
    Coord       width;
} CardIconNode;

/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt16           libraryFormId = frmLibrary;
static FormType*        libraryForm;
static Char             formTitle[ MAX_TITLE_LENGTH ];
static LinkedList       docList;
static FontID           displayFont;
static FontID           displayFontBold;
static UInt16           typeColumnWidth;
static UInt16           nameColumnWidth;
static UInt16           dateColumnWidth;
static UInt16           sizeColumnWidth;
static UInt16           scrollColumnWidth;
static UInt16*          indexList;
static Int16            lastIndex       = -1;
static Int16            firstVisibleRow = 0;
static Int16            numberOfRows;
static Int16            selectedRow;
static UInt8            visibleRows;
static RectangleType    nameBounds;
static RectangleType    docBounds;
static Boolean          selectCategoryFilter;
static DisplayType*     displayList     = NULL;
static Int16            penSelectedRow;
static Int16            highlightedRow;
static Char             popupList[ 255 ];
static MemHandle        popupHandle;

static WinHandle        backBits;

static SortType         selectedSort;
static RectangleType    selectedBounds;
static Boolean          highlighted;

static LinkedList       typeIconList;
static LinkedList       cardIconList;

static Int16            selectorCurrentRow = NO_ROW;

static Boolean          initializing;

static Boolean          forceLibraryFormUpdate = false;

static Char*            categoryName[ CATEGORY_MODES ];

#ifdef HAVE_GRAY_FONT
static Char             oldOrientation;
#endif

static Char             lookupBuffer[ LOOKUP_BUFFER_SIZE ];
static UInt16           lookupCharPos;
static UInt16           lookupBytePos;

static Boolean          activateOnce = false;

/* These three variables are only valid if their respective
   Prefs().column value is true. */
static RectangleType    typeBounds;
static RectangleType    dateBounds;
static RectangleType    sizeBounds;

static Int16( *CompareItems ) ( DocumentInfo*, DocumentInfo* );

/* select type actions approved for libraryform use */
static SelectType librarySelectActions[] = {
    SELECT_PREFS,
    SELECT_BUTTON_ACTION,
    SELECT_TAP_ACTION,
    SELECT_GESTURE_ACTION,
    SELECT_FONT,
    SELECT_COMMAND_STROKE,
    SELECT_MENU,
    SELECT_BRIGHTNESS_ADJUST,
    SELECT_TOGGLE_BACKLIGHT,
    SELECT_CONTRAST_ADJUST
};
#define NUM_LIBRARY_SELECT_ACTIONS ( sizeof( librarySelectActions ) \
                                         / sizeof( SelectType ) )

/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void LibraryFormInit( void ) LIBRARYFORM_SECTION;
static void DrawLayout( void ) LIBRARYFORM_SECTION;
static void UpdateScrollbar(void) LIBRARYFORM_SECTION;
static void DisplayCategoryStyle( void ) LIBRARYFORM_SECTION;
static void ShowSortMessage( void ) LIBRARYFORM_SECTION;
static void UpdateSortMethod( void ) LIBRARYFORM_SECTION;
static Int16 CompareTypeAsc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareTypeDesc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareNameAsc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareNameDesc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareSizeAsc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareSizeDesc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareDateAsc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareDateDesc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareDateTimeAsc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Int16 CompareDateTimeDesc( DocumentInfo* d1,
                DocumentInfo* d2 ) LIBRARYFORM_SECTION;
static Boolean HandlePenOnSort( EventType* event ) LIBRARYFORM_SECTION;
static Boolean HandlePenOnRecord( EventType* event ) LIBRARYFORM_SECTION;
static void HighlightRow( Int16 newRow ) LIBRARYFORM_SECTION;
static void ScrollTo( const Int16 row ) LIBRARYFORM_SECTION;
static void DeleteAllDocuments( void ) LIBRARYFORM_SECTION;
static void DeleteReadDocuments( void ) LIBRARYFORM_SECTION;
static void DeleteOneDocument( Int16 index ) LIBRARYFORM_SECTION;
static void UnreadOneDocument( Int16 index ) LIBRARYFORM_SECTION;
static void DeleteListDocument( Int16 index, ProgressPtr prg )
                LIBRARYFORM_SECTION;
static Boolean IsReadOnlyDocument( Int16 index ) LIBRARYFORM_SECTION;
static Boolean IsCopyProtectedDocument( Int16 index ) LIBRARYFORM_SECTION;
static void InitializeIconPopupList( void ) LIBRARYFORM_SECTION;
static void SetCategoryListPosition( Coord x, Coord y ) LIBRARYFORM_SECTION;
static void DeAllocateIcons( void ) LIBRARYFORM_SECTION;
static void SyncDocList( void ) LIBRARYFORM_SECTION;
static void ReleaseAllocatedMemory(void) LIBRARYFORM_SECTION;
static void DrawVisibleLines( UInt16 start, UInt16 count ) LIBRARYFORM_SECTION;
static void AllocateTypeIcon(void) LIBRARYFORM_SECTION;
static void AllocateCardIcon(UInt16 volumeRef) LIBRARYFORM_SECTION;
static void ShowTypeIcon(UInt16 index, Coord startY) LIBRARYFORM_SECTION;
static Boolean SelectCategory(void) LIBRARYFORM_SECTION;
static void SetNextCategory( void ) LIBRARYFORM_SECTION;
static void SetCategoryTrigger(void) LIBRARYFORM_SECTION;
static void ReleaseCategoryName( void ) LIBRARYFORM_SECTION;
static void DrawRecords( UInt16 start, UInt16 count ) LIBRARYFORM_SECTION;
static void UpdateIndexList( void ) LIBRARYFORM_SECTION;
static UInt16 CategoryAtoC( UInt16 advanced ) LIBRARYFORM_SECTION;
static UInt16 CategoryCtoA( UInt16 classic ) LIBRARYFORM_SECTION;
static void LibraryTimeoutPopupHandler( void ) LIBRARYFORM_SECTION;
static Boolean LibraryHandleKeyboard( UInt16 key, UInt16 keyCode ) 
    LIBRARYFORM_SECTION;
static void FormatDocument( UInt16 i, FormatType* format ) LIBRARYFORM_SECTION;
static void CalculateColumnWidths( void ) LIBRARYFORM_SECTION;
static Boolean callbackDeleteDocument( PrgCallbackDataPtr ) LIBRARYFORM_SECTION;
static Boolean LookupByInitial( WChar ch ) LIBRARYFORM_SECTION;
static void ResetLookup( void ) LIBRARYFORM_SECTION;



/* Set library form to update next time it's entered */
void SetLibraryFormUpdate( void )
{
    forceLibraryFormUpdate = true;
}


/* Initialize the Document Library form */
static void LibraryFormInit( void )
{
    if ( SetValidForm( &libraryFormId ) )
        return;

    initializing = true;

    SetScreenMode();

    LoadUserFont( GetUserFontNumber( Prefs()->libraryUserFontName,
                      true, fontCacheLibrary ) );
    RefreshCustomFonts();

    InitializeViewportBoundaries();
    InitializeIconPopupList();

    libraryForm = FrmGetFormPtr( libraryFormId );
    CloseDocument();

    /* Move the library to the right, to make room for the help icon */
    if ( StrNCompare( FrmGetTitle( libraryForm ), "      ", 6 ) ) {
        StrPrintF( formTitle, "      %s", FrmGetTitle( libraryForm ) );
        FrmSetTitle( libraryForm, formTitle );
    }

    FrmDrawForm( libraryForm );

    displayFont     = GetLibraryStyleFont( DEFAULTSTYLE );
    displayFontBold = GetLibraryStyleFont( BOLDSTYLE );

    DrawHelpIcon( libraryForm, frmLibraryHelp );

    ShowSyncMessage();

    CalculateColumnWidths();
    InitializeDocInfoList();

    typeIconList = ListCreate();
    cardIconList = ListCreate();

    HideMessage();

    activateOnce    = true;

    penSelectedRow  = NO_ROW;
    highlightedRow  = NO_ROW;
    selectedRow     = NO_ROW;
    /* jogdial/fiveway/keyboard selector */
    selectorCurrentRow = NO_ROW;

    DisplayCategoryStyle();
    DrawLayout();
    ResetLookup();

    Prefs()->lastForm = libraryFormId;
    initializing = false;
}



static void ResetLookup( void )
{
    lookupCharPos   = 0;
    lookupBytePos   = 0;
}



static Boolean LookupByInitial
    (
    WChar ch
    )
{
    UInt16 i;
    Int32  order;

    if ( ch == chrBackspace && lookupCharPos <= 1 ) {
        ResetLookup();
        if ( selectorCurrentRow != NO_ROW && visibleRows < numberOfRows )
            return LibrarySelectorHandler( librarySelectorLeft, 0 );
        else
            return true;
    }

    if ( ( ch != chrBackspace &&
           ( LOOKUP_BUFFER_SIZE - 1 < TxtGlueCharSize( ch ) + lookupBytePos ||
             MAX_LOOKUP_CHARS <= lookupCharPos) ) ) {
        return true;
    }
    if ( ch == chrBackspace ) {
        lookupBytePos -= TxtGlueGetPreviousChar( lookupBuffer, lookupBytePos,
                            NULL );
        lookupCharPos--;
    }
    else {
        lookupBytePos += TxtGlueSetNextChar( lookupBuffer, lookupBytePos, ch );
        lookupCharPos++;
    }
    lookupBuffer[ lookupBytePos ] = '\0';

    if ( Prefs()->sortOrder == SORTORDER_ASC ) {
        order = -1;
    }
    else {
        order = 1;
    }
    for ( i = 0 ; i < numberOfRows ; i++ ) {
        DocumentInfo* docInfo;
        docInfo = ListGet( docList, indexList[ i ] + 1 );
        if ( order * StrCaselessCompare( docInfo->name, lookupBuffer ) <= 0 )
            break;
    }
    if ( i == numberOfRows && 0 < i ) {
        i--;
    }
    LibrarySelectorHandler( librarySelectorInit, 0 );
    return LibrarySelectorHandler( librarySelectorToNumber, i );
}



/* handle keyboard/graffiti actions */
static Boolean LibraryHandleKeyboard
    (
    WChar  ch,   /* character to handle */
    UInt16 keyCode
    )
{
    SelectType action;
    Int16      i;

    /* We don't handle keys with keyCode yet. */
    /* MN: Minor change so that both the Treo600's keyboard and the fiveway
           on the T|C (and maybe also on other devices) will work until a
           proper solution has been implemented. */
    if ( 255 < keyCode ) {
        return false;
    }
#ifdef HAVE_FIVEWAY_SDK
    if ( vchrPalmMin <= ch && ch <= vchrPalmMax ) {
        return false;
    }
#endif

    if ( TxtGlueCharIsPrint( ch ) || ch == chrBackspace ) {
        if ( Prefs()->sortType == SORT_NAME ) {
            return LookupByInitial( ch );
        }
        else {
            return true;
        }
    }

    action = GetKeyboardAction( ch, 0 );
    if ( action == KEYBOARD_ACTION_NONE ) {
        return false;
    }

    /* check if this is an action for which we can call DoSelectTypeAction() */
    for ( i = 0 ; i < NUM_LIBRARY_SELECT_ACTIONS ; i++ ) {
         if ( librarySelectActions[ i ] == action ) {
             DoSelectTypeAction( action );
             return true;
         }
    }

    /* to be nice, handle these */
    switch ( action ) {
        case SELECT_GO_TO_TOP:
             LibrarySelectorHandler( librarySelectorInit, 0 );
             return LibrarySelectorHandler( librarySelectorToNumber,
                        LIBRARY_SELECTOR_GO_TO_TOP );

        case SELECT_GO_TO_BOTTOM:
             LibrarySelectorHandler( librarySelectorInit, 0 );
             return LibrarySelectorHandler( librarySelectorToNumber,
                        LIBRARY_SELECTOR_GO_TO_BOTTOM );

        case SELECT_NEXT_ANCHOR:
             LibrarySelectorHandler( librarySelectorInit, 0 );
             return LibrarySelectorHandler( librarySelectorDown, 0 );

        case SELECT_PREV_ANCHOR:
             LibrarySelectorHandler( librarySelectorInit, 0 );
             return LibrarySelectorHandler( librarySelectorUp, 0 );

        case SELECT_GO_TO_LINK:
             if ( selectorCurrentRow == NO_ROW ) {
                 LibrarySelectorHandler( librarySelectorInit, 0 );
                 if ( selectorCurrentRow == selectedRow )
                     return LibrarySelectorHandler( librarySelectorGo, 0 );
                 else
                     return true;
             }
             return LibrarySelectorHandler( librarySelectorGo, 0 );

        case SELECT_GO_BACK:
             return LibrarySelectorHandler( librarySelectorLeft, 0 );

        case SELECT_FULL_PAGE_UP:
        case SELECT_HALF_PAGE_UP: /* FIXME! */
             return LibrarySelectorHandler( librarySelectorPageUp, 0 );

        case SELECT_FULL_PAGE_DOWN:
        case SELECT_HALF_PAGE_DOWN: /* FIXME! */
             return LibrarySelectorHandler( librarySelectorPageDown, 0 );

        default:
             return false;
    }
}



/* The following functions are for the "selector".  This is the
   selecting device, currently either keyboard, fiveway or jogdial, or
   a combination of these. */
/* update selectorCurrentRow -- selectorCurrentRow uses absolute row values */
void SelectorSetRow
     (
     Int16 row
     )
{
    selectorCurrentRow = row;
}



Int16 SelectorGetRow( void )
{
    return selectorCurrentRow;
}



void SelectorHighlightRow( Boolean enable )
{
    if ( selectorCurrentRow == NO_ROW )
        return;

    HighlightRectangle( LibraryGetDisplayListBounds( selectorCurrentRow ),
        0, enable, TEXT);

    if ( ! enable )
        selectorCurrentRow = NO_ROW;
}



Boolean LibrarySelectorHandler
    (
    LibrarySelector movement,
    Int32           argument
    )
{
    Boolean        handled;
    Boolean        scroll;

    handled         = true;
    scroll          = false;

    if ( movement != librarySelectorInit &&
         ( movement != librarySelectorToNumber || argument < 0) ) {
        ResetLookup();
    }

    switch ( movement ) {
        case librarySelectorInit:
            if ( selectorCurrentRow == NO_ROW ) {
                if ( selectedRow < firstVisibleRow ||
                     firstVisibleRow + visibleRows < selectedRow ) {
                    selectorCurrentRow = visibleRows / 2 + firstVisibleRow;
                }
                else {
                    selectorCurrentRow = selectedRow;
                }
                LibraryHighlightRow( NO_ROW );
                HighlightRectangle(
                    LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                    true, TEXT );
            }
            break;

        case librarySelectorPageUp:
            if ( selectorCurrentRow != NO_ROW ) {
                HighlightRectangle(
                    LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                    false, TEXT );
                selectorCurrentRow -= visibleRows;
                if ( selectorCurrentRow < 0 )
                    selectorCurrentRow = 0;
                if ( firstVisibleRow == 0 ) {
                    HighlightRectangle(
                        LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                        true, TEXT );
                    break;
                }
            }
            ScrollUp( visibleRows );
            break;

        case librarySelectorPageDown:
            if ( selectorCurrentRow != NO_ROW ) {
                HighlightRectangle(
                    LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                    false, TEXT );
                selectorCurrentRow += visibleRows;
                if ( numberOfRows <= selectorCurrentRow )
                    selectorCurrentRow = numberOfRows - 1;
                if ( numberOfRows <= firstVisibleRow + visibleRows ) {
                    HighlightRectangle(
                        LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                        true, TEXT );
                    break;
                }
            }
            ScrollDown( visibleRows );
            break;

        case librarySelectorRight:
            if ( selectorCurrentRow != NO_ROW ) {
                lastIndex = LibraryGetDisplayListIndex( selectorCurrentRow );
                IconPopupList( selectorCurrentRow - firstVisibleRow );
            }
            break;

        case librarySelectorGo:
            if ( selectorCurrentRow != NO_ROW ) {
                OpenNewDocument( LibraryGetDisplayListIndex(
                                     selectorCurrentRow ));
                selectorCurrentRow = NO_ROW;
            }
            else {
                return LibrarySelectorHandler( librarySelectorInit, 0 );
            }
            break;

        case librarySelectorLeft:
            if ( selectorCurrentRow != NO_ROW &&
                 visibleRows < numberOfRows ) {
                HighlightRectangle( LibraryGetDisplayListBounds(
                                        selectorCurrentRow ), 0, false, TEXT);
                selectorCurrentRow = NO_ROW;
                LibraryHighlightRow( selectedRow - firstVisibleRow );
            }
            else {
                EventType stopEvent;

                stopEvent.eType = appStopEvent;
                EvtAddEventToQueue(&stopEvent);
            }
            break;

        case librarySelectorUp:
            if ( numberOfRows == 0 ) {
                handled = false;
                return handled;
            }
            if ( selectorCurrentRow == NO_ROW )
                return handled;
            if ( 0 < selectorCurrentRow ) {
                HighlightRectangle(
                    LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                    false, TEXT );
                if ( selectorCurrentRow == firstVisibleRow )
                    scroll = true;
                selectorCurrentRow--;
            }
            else {
                return handled;
            }
            if ( scroll ) {
                ScrollUp( ONE_ROW );
            }
            else {
                HighlightRectangle( LibraryGetDisplayListBounds(
                    selectorCurrentRow ), 0, true, TEXT);
            }
            break;

        case librarySelectorDown:
            if ( numberOfRows == 0 ) {
                handled = false;
                return handled;
            }
            if ( selectorCurrentRow == NO_ROW )
                return handled;
            if ( selectorCurrentRow < numberOfRows - 1) {
                HighlightRectangle(
                    LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                    false, TEXT );
                if ( selectorCurrentRow == visibleRows +
                                   firstVisibleRow - 1)
                    scroll = true;
                selectorCurrentRow++;
            }
            else {
                return handled;
            }
            if ( scroll ) {
                ScrollDown( ONE_ROW );
            }
            else {
                HighlightRectangle( LibraryGetDisplayListBounds(
                    selectorCurrentRow ), 0, true, TEXT);
            }
            break;

        case librarySelectorToNumber:
            if ( selectorCurrentRow == NO_ROW )
                return handled;
            if ( argument == LIBRARY_SELECTOR_GO_TO_TOP )
                argument = 0;
            else if ( argument == LIBRARY_SELECTOR_GO_TO_BOTTOM ||
                      numberOfRows <= argument )
                argument = numberOfRows - 1;
            else if ( argument < 0 )
                argument = 0;
            HighlightRectangle(
                LibraryGetDisplayListBounds( selectorCurrentRow ), 0,
                false, TEXT );
            if ( argument < firstVisibleRow ) {
                selectorCurrentRow = argument;
                ScrollUp( firstVisibleRow - argument );
            }
            else if ( visibleRows + firstVisibleRow <= argument ) {
                selectorCurrentRow = argument;
                ScrollDown( argument - ( visibleRows + firstVisibleRow - 1 ) );
            }
            else {
                selectorCurrentRow = argument;
                HighlightRectangle( LibraryGetDisplayListBounds(
                    selectorCurrentRow ), 0, true, TEXT);
            }
            break;

        case librarySelectorUndefined:
            handled = false;
            break;
    }
    return handled;
}



/* draw the layout of the form, fit it properly within the display */
static void DrawLayout( void )
{
    FontID        oldFont;
    Char          nameTitle[ titleStringLength ];
    Char          dateTitle[ titleStringLength ];
    Char          sizeTitle[ titleStringLength ];
    RectangleType bounds;
    UInt16        adjustNameColumn = 0;

    oldFont = FntSetFont( displayFontBold );

    SysCopyStringResource( nameTitle, strLibraryName );
    SysCopyStringResource( dateTitle, strLibraryDate );
    SysCopyStringResource( sizeTitle, strLibrarySize );

    bounds.topLeft.x = 0;

    if ( IsHiResTypeSony( HiResType() ) )
        bounds.topLeft.y = TITLEBAR_HEIGHT_SONY;
    else if ( IsHiResTypeHandera( HiResType() ) )
        bounds.topLeft.y = TITLEBAR_HEIGHT_HANDERA;
    else
        bounds.topLeft.y = TITLEBAR_HEIGHT;

    bounds.extent.x = MaxExtentX();

    /* figure out row sizes on... */
    /* title row */
    bounds.extent.y = FntCharHeight() + 1;

    /* document rows */
    FntSetFont( displayFont );
    visibleRows = ( MaxExtentY() - bounds.topLeft.y - bounds.extent.y ) /
                  FntCharHeight();

    bounds.extent.y += visibleRows * FntCharHeight();

    FntSetFont( displayFontBold );

    /* figure out column sizes on...*/
    /* date column */
    if ( Prefs()->column.date == SHOW ) {
        UInt16 titleWidth;

        titleWidth = FntCharsWidth( dateTitle, StrLen( dateTitle ) ) + 2;

        if ( dateColumnWidth < titleWidth )
            dateColumnWidth = titleWidth;
    }
    else {
        dateColumnWidth = 0;
    }

    /* size column */
    if ( Prefs()->column.size == SHOW ) {
        UInt16 titleWidth;

        titleWidth = FntCharsWidth( sizeTitle, StrLen( sizeTitle ) ) + 2;

        if ( sizeColumnWidth < titleWidth )
            sizeColumnWidth = titleWidth;
    }
    else {
        sizeColumnWidth = 0;
    }

    if ( Prefs()->column.type == SHOW ) {
        /* keep typeColumnWidth's previous value */
    }
    else {
        typeColumnWidth = 0;
    }

    scrollColumnWidth = SCROLLBAR_WIDTH + 1;
    HiResAdjust( (Int16 *)&scrollColumnWidth, sonyHiRes );

    /* name column */
    nameColumnWidth = MaxExtentX() - 1 -
        ( typeColumnWidth + dateColumnWidth +
          sizeColumnWidth + scrollColumnWidth );

    bounds.extent.x  -= scrollColumnWidth;
    if ( Prefs()->scrollbar == SCROLLBAR_LEFT ) {
        Coord y;

        bounds.topLeft.x += scrollColumnWidth;
        if ( IsHiResTypeHandera( HiResType() ) )
            y = TITLEBAR_HEIGHT_HANDERA;
        else
            y = TITLEBAR_HEIGHT;

        FrmSetObjectPosition( libraryForm, FrmGetObjectIndex( libraryForm,
            frmLibraryScrollBar ), 0, y );
    }

    /* Draw the frame for the titlebar... */
    /* Left line */
    WinDrawLine( bounds.topLeft.x,
        bounds.topLeft.y,
        bounds.topLeft.x,
        bounds.topLeft.y + FntCharHeight() );
    /* Bottom line */
    WinDrawLine( bounds.topLeft.x,
        bounds.topLeft.y + FntCharHeight(),
        bounds.topLeft.x + bounds.extent.x,
        bounds.topLeft.y + FntCharHeight() );
    /* Right line */
    WinDrawLine( bounds.topLeft.x + bounds.extent.x,
        bounds.topLeft.y,
        bounds.topLeft.x + bounds.extent.x,
        bounds.topLeft.y + FntCharHeight() );
    /* No top, because we're flushed up against the regular UI label */


    /* define docBounds as the area to draw the documents */
    docBounds       = bounds;
    docBounds.topLeft.y += FntCharHeight() + 1;
    docBounds.extent.y  -= FntCharHeight() + 1;

    bounds.topLeft.x += bounds.extent.x;

    if ( Prefs()->column.size == SHOW ) {
        bounds.topLeft.x -= sizeColumnWidth;
        bounds.extent.x   = sizeColumnWidth;

        WinDrawChars( sizeTitle, StrLen( sizeTitle ),
            bounds.topLeft.x + 2, bounds.topLeft.y );
        WinDrawLine( bounds.topLeft.x,
            bounds.topLeft.y,
            bounds.topLeft.x,
            bounds.topLeft.y + FntCharHeight() );

        sizeBounds = bounds;
        sizeBounds.topLeft.x++;
        sizeBounds.extent.x--;
        sizeBounds.extent.y = FntCharHeight();
    }
    else {
        adjustNameColumn += sizeColumnWidth;
    }

    if ( Prefs()->column.date == SHOW ) {
        bounds.topLeft.x -= dateColumnWidth;
        bounds.extent.x   = dateColumnWidth;

        WinDrawChars( dateTitle, StrLen( dateTitle ),
            bounds.topLeft.x + 2, bounds.topLeft.y );
        WinDrawLine( bounds.topLeft.x,
            bounds.topLeft.y,
            bounds.topLeft.x,
            bounds.topLeft.y + FntCharHeight() );

        dateBounds = bounds;
        dateBounds.topLeft.x++;
        dateBounds.extent.x--;
        dateBounds.extent.y = FntCharHeight();
    }
    else {
        adjustNameColumn += dateColumnWidth;
    }

    bounds.topLeft.x -= nameColumnWidth + adjustNameColumn;
    bounds.extent.x   = nameColumnWidth + adjustNameColumn;

    WinDrawChars( nameTitle, StrLen( nameTitle ),
        bounds.topLeft.x + 2, bounds.topLeft.y );

    nameBounds = bounds;
    nameBounds.topLeft.x++;
    nameBounds.extent.x--;
    nameBounds.extent.y = FntCharHeight();

    if ( Prefs()->column.type == SHOW ) {
        WinDrawLine( bounds.topLeft.x,
            bounds.topLeft.y,
            bounds.topLeft.x,
            bounds.topLeft.y + FntCharHeight() );

        bounds.topLeft.x -= typeColumnWidth;
        bounds.extent.x   = typeColumnWidth;

        typeBounds = bounds;
        typeBounds.topLeft.x++;
        typeBounds.extent.x--;
        typeBounds.extent.y = FntCharHeight();
    }

    FntSetFont( oldFont );

    UpdateSortMethod();
    ShowSortMessage();
    UpdateIndexList();
    HideMessage();
    DrawRecords( 0, visibleRows );
}



static void UpdateScrollbar(void)
{
    UInt16 maxValue;

    if (visibleRows < numberOfRows) {
        if (numberOfRows < firstVisibleRow + visibleRows)
            firstVisibleRow = numberOfRows - visibleRows;

        maxValue = numberOfRows - visibleRows;
    } else {
        maxValue = 0;
    }
    SclSetScrollBar(GetObjectPtr(frmLibraryScrollBar), firstVisibleRow,
                    0, maxValue, visibleRows);
}



/* uses absolute row values */
UInt16 LibraryGetDisplayListIndex(Int16 row)
{
    if ( displayList != NULL && firstVisibleRow <= row &&
        row < visibleRows + firstVisibleRow )
        return displayList[ row - firstVisibleRow ].index;
    else
        return -1;
}


RectangleType NullBounds = { { 0, 0 }, { 0, 0 } };

/* uses absolute row values */
RectangleType* LibraryGetDisplayListBounds(Int16 row)
{
    if ( displayList != NULL && firstVisibleRow <= row &&
        row < visibleRows + firstVisibleRow )
        return &displayList[ row - firstVisibleRow ].bounds;
    else
        return &NullBounds;
}


static void ShowTypeIcon(UInt16 index, Coord startY)
{
    RectangleType clip;
    TypeIconNode *typeNode;
    DocumentInfo *docInfo;

    docInfo = ListGet(docList, index + 1);
    clip = typeBounds;
    clip.topLeft.y = startY;
    clip.extent.y  = FntCharHeight();
    WinSetClip(&clip);

    /* draw the icon for this type in the centre of the
       area defined by typeBounds, current y coordinate,
       and line height  */
    typeNode = ListFirst(typeIconList);
    if (typeNode != NULL) {
        Coord x;
        Coord y;

        x = typeBounds.extent.x / 2 + typeBounds.topLeft.x -
            typeNode->width / 2;
        y = FntCharHeight() / 2 - typeNode->height / 2 + startY;

        WinDrawBitmap(typeNode->bitmap, x, y);
    }

    /* draw any supplemental vfs icon centred left */
    if (docInfo->location != RAM) {
        CardIconNode *cardNode;

        cardNode = ListFirst(cardIconList);
        while (cardNode != NULL) {
            if (cardNode->volumeRef == docInfo->volumeRef) {
                Coord x;
                Coord y;

                x = typeBounds.topLeft.x + 1;
                y = FntCharHeight() / 2 - cardNode->height / 2 + startY;

                WinDrawBitmap(cardNode->bitmap, x, y);
                break;
            }
            cardNode = ListNext(cardIconList, cardNode);
        }
    }
    WinResetClip();
}


static void DrawVisibleLines
    (
    UInt16  start,
    UInt16  count
    )
{
    Coord   startY;
    Int16   row;
    UInt16  stop;

    if ( count == visibleRows ) {
        HighlightRow( NO_ROW );
        WinEraseRectangle( &docBounds, 0 );
    }

    stop = start + count;

    startY      = docBounds.topLeft.y;
    for (row = firstVisibleRow;
         row < numberOfRows && row - firstVisibleRow < visibleRows;
         row++) {
        Int16       index;
        FormatType  format;
        Int16       displayRow;
        Char        text[ dmDBNameLength ];

        index = indexList[ row ];

        displayRow                                  = row - firstVisibleRow;

        displayList[ displayRow ].index             = index;
        displayList[ displayRow ].bounds.topLeft.x  = docBounds.topLeft.x;
        displayList[ displayRow ].bounds.topLeft.y  = startY;
        displayList[ displayRow ].bounds.extent.x   = docBounds.extent.x;
        displayList[ displayRow ].bounds.extent.y   = FntCharHeight();

        FormatDocument( index, &format );

        if ( start <= displayRow && displayRow < stop ) {

            /* Set clipping so that kerned text doesn't get out of bounds. */
            WinSetClip( &displayList[ displayRow ].bounds );

            /* always display the document name */
            StrNCopy( text, format.name, dmDBNameLength );
            TrimText( text, nameBounds.extent.x - 1 );
            WinDrawChars( text, StrLen(text), nameBounds.topLeft.x + 1
                - GrayFntMinLeftKerning( FntGetFont() ), startY );

            if ( Prefs()->column.type == SHOW )
                ShowTypeIcon(index, startY);

            if ( Prefs()->column.date == SHOW )
                WinDrawChars(format.date, StrLen(format.date),
                             dateBounds.topLeft.x + 1, startY);

            if ( Prefs()->column.size == SHOW )
                WinDrawChars(format.size, StrLen(format.size),
                     sizeBounds.topLeft.x + (sizeBounds.extent.x -
                                             FntCharsWidth(format.size,
                                                           StrLen(format.
                                                                  size))),
                     startY);

            if ( Prefs()->indicateOpened ){
                DocumentInfo*   docInfo;

                docInfo = ListGet( docList, index + 1 );
                if ( docInfo->timestamp != 0 ){
                    WinDrawLine(
                        docBounds.topLeft.x,
                        startY + FntCharHeight() / 2,
                        docBounds.topLeft.x + docBounds.extent.x,
                        startY + FntCharHeight() / 2);
                }
            }

            WinResetClip();
            /* Found row with last accessed document, highlight/set it
               accordingly. */
            if ( selectorCurrentRow == NO_ROW &&
                 STREQ( Prefs()->docName, format.name )) {
                /* If the list of documents is only one page or if
                   we have just returned to the library then activate
                   fiveway/jogdial (if available). */
                if ( ( numberOfRows <= visibleRows || activateOnce ) &&
                     ( HaveFiveWay() || HaveJogdial() ) ) {
                    selectorCurrentRow = row;
                }
                else {
                    HighlightRow( row - firstVisibleRow );
                    selectedRow = row;
                }
            }
            /* Found the jog row, highlight/set it accordingly */
            if ( selectorCurrentRow == row ) {
                SelectorHighlightRow( true );
            }
        }

        startY += FntCharHeight();
    }
    /* If the list of documents is only one page and the
       last accessed document isn't in the list, then
       select the document in the middle of the list and
       activate fiveway/jogdial from the very beginning. */
    if ( count == visibleRows && selectorCurrentRow == NO_ROW &&
         numberOfRows <= visibleRows &&
         ( HaveFiveWay() || HaveJogdial()) ) {
        selectorCurrentRow = numberOfRows / 2;
        SelectorHighlightRow( true );
    }
    /* We only check this once during a library session */
    activateOnce = false;
}



/* actually draw the records within docBounds */
void DrawRecords
     (
     UInt16   start,
     UInt16   count
     )
{
    UInt32        memSize;
    FontID        oldFont;

    if ( ListIsEmpty( docList ) ) {
        WinEraseRectangle( &docBounds, 0 );
        FrmShowObject( libraryForm, FrmGetObjectIndex( libraryForm,
            frmLibraryNoDocuments ) );
        return;
    }
    else {
        FrmHideObject( libraryForm, FrmGetObjectIndex( libraryForm,
            frmLibraryNoDocuments ) );
    }
    memSize = visibleRows * sizeof( DisplayType );
    if ( displayList != NULL &&
         MemPtrSize( displayList ) != memSize ) {
        SafeMemPtrFree( displayList );
        displayList = NULL;
    }
    if ( displayList == NULL )
        displayList = SafeMemPtrNew( memSize );

    oldFont = FntSetFont( displayFont );

    UpdateScrollbar();
    DrawVisibleLines( start, count );

    FntSetFont( oldFont );
}



static void DisplayCategoryStyle( void )
{
    UInt16 showObject;

    switch ( Prefs()->categoryStyle ) {
        case CATEGORY_ADVANCED:
            showObject = frmLibraryCategoryBtn;
            break;

        case CATEGORY_CLASSIC:
            showObject = frmLibraryCategoryPopup;
            SetCategoryTrigger();
            Prefs()->filterMode = FILTER_OR;
            break;

        default:
            showObject = frmLibraryCategoryBtn;
            break;
    }
    FrmShowObject( libraryForm, FrmGetObjectIndex( libraryForm, showObject ) );
}



void ShowMessage
     (
     UInt16 stringID
     )
{
    UInt16    error;
    UInt16    oldCoordSys;
    Coord     width;
    Coord     height;
    Coord     normalScreenWidth;
    Coord     deltaX;
    Char      message[ MAX_MESSAGE_LENGTH + 1 ];
    FontID    oldFont;

    RectangleType bounds = {
        { MSG_TOPLEFT_X, MSG_TOPLEFT_Y },
        { MSG_EXTENT_X, MSG_EXTENT_Y }
    };

    oldCoordSys = PalmSetCoordinateSystem( STANDARD );

    HiResAdjustBounds( &bounds, sonyHiRes | handeraHiRes );

    normalScreenWidth = 160;
    
    HiResAdjust( &normalScreenWidth, sonyHiRes | handeraHiRes );

    if ( normalScreenWidth < MaxExtentX() ) {
        deltaX = ( MaxExtentX() - normalScreenWidth ) / 2;
    }
    else {
        deltaX = 0;
    }
    
    bounds.topLeft.x += deltaX;    

    backBits = WinSaveBits( &bounds, &error );

    if ( backBits != NULL ) {
        RectangleType innerBounds;

        innerBounds.topLeft.x = bounds.topLeft.x + 2;
        innerBounds.topLeft.y = bounds.topLeft.y + 2;
        innerBounds.extent.x  = bounds.extent.x - 4;
        innerBounds.extent.y  = bounds.extent.y - 4;

        WinEraseRectangle( &innerBounds, 0 );
        WinDrawRectangleFrame( boldRoundFrame, &innerBounds );

        SysCopyStringResource( message, stringID );

        oldFont = FntSetFont( HiResFont( boldFont ) );

        width   = FntCharsWidth( message, StrLen( message ) );
        height  = FntCharHeight();

        WinDrawChars( message,
                      StrLen( message ),
                      bounds.topLeft.x + bounds.extent.x / 2 - width / 2,
                      bounds.topLeft.y + bounds.extent.y / 2 - height / 2 );

        FntSetFont( oldFont );
    }

    PalmSetCoordinateSystem( oldCoordSys );
}




void HideMessage( void )
{
    UInt16    oldCoordSys;
    Coord     x;
    Coord     y;

    if ( backBits == NULL )
        return;

    x = MSG_TOPLEFT_X;
    y = MSG_TOPLEFT_Y;

    HiResAdjust( &x, sonyHiRes | handeraHiRes );
    HiResAdjust( &y, sonyHiRes | handeraHiRes );

    oldCoordSys = PalmSetCoordinateSystem( STANDARD );

    WinRestoreBits( backBits, x, y );
    backBits = NULL;

    PalmSetCoordinateSystem( oldCoordSys );
}





/* Display sorting message */
static void ShowSortMessage( void )
{
    ShowMessage( strSortDoc );
}



/* Display Sync message */
void ShowSyncMessage( void )
{
    if ( Prefs()->syncPolicy == SYNC_MANUAL )
        ShowMessage( strOpenDocList );
    else
        ShowMessage( strSyncDocList );
}



/* Update settings for selected sort method */
static void UpdateSortMethod( void )
{
    switch ( Prefs()->sortType ) {
        case SORT_TYPE:
            if ( Prefs()->sortOrder == SORTORDER_ASC )
                CompareItems = CompareTypeAsc;
            else
                CompareItems = CompareTypeDesc;
            break;

        case SORT_NAME:
            if ( Prefs()->sortOrder == SORTORDER_ASC )
                CompareItems = CompareNameAsc;
            else
                CompareItems = CompareNameDesc;
            break;

        case SORT_DATE:
            if ( Prefs()->useDateTime ) {
                if ( Prefs()->sortOrder == SORTORDER_ASC ) {
                    CompareItems = CompareDateTimeAsc;
                }
                else {
                    CompareItems = CompareDateTimeDesc;
                }
            }
            else {
                if ( Prefs()->sortOrder == SORTORDER_ASC ) {
                    CompareItems = CompareDateAsc;
                }
                else {
                    CompareItems = CompareDateDesc;
                }
            }
            break;

        case SORT_SIZE:
            if ( Prefs()->sortOrder == SORTORDER_ASC )
                CompareItems = CompareSizeAsc;
            else
                CompareItems = CompareSizeDesc;
            break;

        default:
            CompareItems = CompareNameAsc;
            break;
    }
}



/* Open the specified document */
void OpenNewDocument
    (
    UInt16 index
    )
{
    Err err;
    DocumentInfo* docInfo;

    docInfo = ListGet(docList, index + 1);
    err     = OpenDocument( docInfo );
    if ( err != errNone ) {
        FrmAlert( errCannotFind );
        return;
    }

    docInfo->timestamp = TimGetSeconds();
    StoreReadTimeInDocumentList( docInfo->name, docInfo->timestamp );

    StrCopy( Prefs()->docName, docInfo->name );

    InitSessionData();
    FrmGotoForm( GetMainFormId() );
}



/* Compare types ( ascending ), if match go by name ( ascending )  */
static Int16 CompareTypeAsc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.volumeRef = d2.volumeRef */
    /* < 0 if d1.volumeRef < d2.volumeRef */
    /* > 0 if d1.volumeRef > d2.volumeRef */
    if ( d2->volumeRef < d1->volumeRef )
        return 1;
    else if ( d1->volumeRef < d2->volumeRef )
        return -1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Compare types ( descending ), if match go by name ( ascending )  */
static Int16 CompareTypeDesc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.volumeRef = d2.volumeRef */
    /* < 0 if d1.volumeRef > d2.volumeRef */
    /* > 0 if d1.volumeRef < d2.volumeRef */
    if ( d2->volumeRef < d1->volumeRef )
        return -1;
    else if ( d1->volumeRef < d2->volumeRef )
        return 1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Compare names ( ascending ) */
static Int16 CompareNameAsc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*     0 if d1.name = d2.name */
    /*   < 0 if d1.name < d2.name */
    /*   > 0 if d1.name > d2.name */
    return StrCompare( d1->name, d2->name );
}



/* Compare names ( descending ) */
static Int16 CompareNameDesc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*     0 if d1.name = d2.name */
    /*   < 0 if d1.name > d2.name */
    /*   > 0 if d1.name < d2.name */
    return StrCompare( d2->name, d1->name );
}



/* Compare size ( ascending ) */
static Int16 CompareSizeAsc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.size = d2.size */
    /* < 0 if d1.size < d2.size */
    /* > 0 if d1.size > d2.size */
    if ( d2->size < d1->size )
        return 1;
    else if ( d1->size < d2->size )
        return -1;
    else
        return 0;
}



/* Compare size ( descending ) */
static Int16 CompareSizeDesc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.size = d2.size */
    /* < 0 if d1.size > d2.size */
    /* > 0 if d1.size < d2.size */
    if ( d2->size < d1->size )
        return -1;
    else if ( d1->size < d2->size )
        return 1;
    else
        return 0;
}



/* Compare date ( ascending ), if match go by name ( ascending )  */
static Int16 CompareDateAsc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    DateType date1, date2;
    UInt32   days1, days2;

    /* Because we're only printing dates (year, month, day) to the viewer,
       sorting by time ( d#->created ) skews the results making everything
       look 'unsorted'. Instead, we convert our actual time to number of
       days, and sort by that. End result is that what's actually being
       displayed is sorted properly */

    DateSecondsToDate( d1->created, &date1 );
    DateSecondsToDate( d2->created, &date2 );
    days1 = DateToDays( date1 );
    days2 = DateToDays( date2 );

    /*   0 if days1 = days2 */
    /* < 0 if days1 < days2 */
    /* > 0 if days1 > days2 */
    if ( days2 < days1 )
        return 1;
    else if ( days1 < days2 )
        return -1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Compare date ( descending ), if match go by name ( ascending )  */
static Int16 CompareDateDesc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    DateType date1, date2;
    UInt32   days1, days2;

    /* See CompareDateAsc() comment */
    DateSecondsToDate( d1->created, &date1 );
    DateSecondsToDate( d2->created, &date2 );
    days1 = DateToDays( date1 );
    days2 = DateToDays( date2 );

    /*   0 if days1 = days2 */
    /* < 0 if days1 > days2 */
    /* > 0 if days1 < days2 */
    if ( days2 < days1 )
        return -1;
    else if ( days1 < days2 )
        return 1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Compare date and time ( ascending ), if match go by name ( ascending )  */
static Int16 CompareDateTimeAsc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.created = d2.created */
    /* < 0 if d1.created < d2.created */
    /* > 0 if d1.created > d2.created */
    if ( d2->created < d1->created )
        return 1;
    else if ( d1->created < d2->created )
        return -1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Compare date and time ( descending ), if match go by name ( ascending )  */
static Int16 CompareDateTimeDesc
    (
    DocumentInfo* d1,   /* pointer to the record to sort */
    DocumentInfo* d2    /* pointer to the record to sort */
    )
{
    /*   0 if d1.created = d2.created */
    /* < 0 if d1.created > d2.created */
    /* > 0 if d1.created < d2.created */
    if ( d2->created < d1->created )
        return -1;
    else if ( d1->created < d2->created )
        return 1;
    else
        return CompareNameAsc( d1, d2 );
}



/* Handle events if the pen is on the titles (to sort them) */
static Boolean HandlePenOnSort
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Coord   x;
    Coord   y;
    Boolean handled;

    x        = event->screenX;
    y        = event->screenY;
    handled  = false;

    HiResAdjust( &x, sonyHiRes );
    HiResAdjust( &y, sonyHiRes );

    switch ( event->eType ) {
        case penDownEvent:
        {
            if ( RctPtInRectangle( x, y, &nameBounds ) ) {
                selectedSort   = SORT_NAME;
                selectedBounds = nameBounds;
            }
            else if ( Prefs()->column.type == SHOW &&
                      RctPtInRectangle( x, y, &typeBounds ) ) {
                selectedSort   = SORT_TYPE;
                selectedBounds = typeBounds;
            }
            else if ( Prefs()->column.date == SHOW &&
                      RctPtInRectangle( x, y, &dateBounds ) ) {
                selectedSort   = SORT_DATE;
                selectedBounds = dateBounds;
            }
            else if ( Prefs()->column.size == SHOW &&
                      RctPtInRectangle( x, y, &sizeBounds ) ) {
                selectedSort   = SORT_SIZE;
                selectedBounds = sizeBounds;
            }
            else {
                selectedSort   = SORT_INVALID;
                selectedBounds = NullBounds;
                break;
            }
            HighlightRectangle( &selectedBounds, 0, true, BUTTON );
            highlighted = true;
            handled = true;
            break;
        }

        case penMoveEvent:
            if ( selectedSort == SORT_INVALID )
                break;

            if ( ( ! RctPtInRectangle( x, y, &selectedBounds ) &&
                   highlighted ) ||
                 ( RctPtInRectangle( x, y, &selectedBounds ) &&
                   ! highlighted ) ) {
                HighlightRectangle( &selectedBounds, 0, ! highlighted, BUTTON );
                highlighted = ! highlighted;
            }
            handled = true;
            break;

        case penUpEvent:
            if ( selectedSort == SORT_INVALID )
                break;

            if ( highlighted && RctPtInRectangle( x, y, &selectedBounds ) ) {
                if ( Prefs()->sortType == selectedSort ) {
                    if ( Prefs()->sortOrder == SORTORDER_ASC )
                        Prefs()->sortOrder = SORTORDER_DESC;
                    else
                        Prefs()->sortOrder = SORTORDER_ASC;
                }
                else {
                    Prefs()->sortOrder = SORTORDER_ASC;
                    Prefs()->sortType  = selectedSort;
                }
                handled = true;
            }

            if ( highlighted )
                HighlightRectangle( &selectedBounds, 0, false, BUTTON );

            selectedSort = SORT_INVALID;

            if ( handled ) {
                UpdateSortMethod();
                ShowSortMessage();
                UpdateIndexList();
                HideMessage();
                DrawRecords( 0, visibleRows );
            }
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Handle events if the pen is on a record */
static Boolean HandlePenOnRecord
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Coord         x;
    Coord         y;
    Boolean       handled;
    RectangleType activeArea;
    Int16         i;

    x            = event->screenX;
    y            = event->screenY;
    handled      = false;

    /* if we're displaying a low number of records, (not enough to need
       a scrollbar) activeArea is adjusted to only span the space that is
       actually being used by documents. Ie. if the stylus were to move
       into an area that a document isn't listed, the highlighting row
       simply disappears. Obviously, this stops people from accidently
       clicking and opening something that isn't there! */
    activeArea = docBounds;
    if ( numberOfRows < visibleRows )
        activeArea.extent.y = numberOfRows *
            displayList[ 0 ].bounds.extent.y - 1;

    HiResAdjust(&x, sonyHiRes);
    HiResAdjust(&y, sonyHiRes);

    if ( displayList == NULL )
        return handled;

    switch ( event->eType ) {
        case penDownEvent:
            if ( ! RctPtInRectangle( x, y, &activeArea ) )
                break;

            SelectorHighlightRow( false );

            for ( i = 0; i < visibleRows; i++ ) {
                if ( RctPtInRectangle( x, y, &displayList[ i ].bounds ) ) {
                    penSelectedRow = i;
                    HighlightRow( penSelectedRow );
                    lastIndex = displayList[ i ].index;
                    TimeoutSet( POPUP_TIMEOUT, LibraryTimeoutPopupHandler );
                    break;
                }
            }
            if ( Prefs()->column.type == SHOW && penSelectedRow != NO_ROW ) {
                RectangleType area;

                area.topLeft.y = displayList[ penSelectedRow ].bounds.topLeft.y;
                area.topLeft.x = typeBounds.topLeft.x;
                area.extent.x  = typeBounds.extent.x;
                area.extent.y  = typeBounds.extent.y;
                if ( RctPtInRectangle( x, y, &area ) ) {
                    TimeoutRelease();
                    IconPopupList( penSelectedRow );
                    penSelectedRow = NO_ROW;
                }
            }
            handled = true;
            break;

        case penMoveEvent:
            if ( penSelectedRow == NO_ROW ) {
                TimeoutRelease();
                break;
            }

            if ( y < activeArea.topLeft.y ||
                 activeArea.topLeft.y + activeArea.extent.y < y ) {
                Boolean isPenDown;

                TimeoutRelease();

                penSelectedRow = NO_ROW;
                HighlightRow( penSelectedRow );

                isPenDown = false;

                if ( y < activeArea.topLeft.y ) {
                    do {
                        ScrollUp( ONE_ROW );
                        EvtGetPen( &x, &y, &isPenDown );
                        HiResAdjust( &y, sonyHiRes );
                    } while ( isPenDown && y < activeArea.topLeft.y );
                    if ( isPenDown )
                        penSelectedRow = 0;
                }
                else {
                    do {
                        ScrollDown( ONE_ROW );
                        EvtGetPen( &x, &y, &isPenDown );
                        HiResAdjust( &y, sonyHiRes );
                    } while ( isPenDown &&
                              activeArea.topLeft.y + activeArea.extent.y < y );
                    if ( isPenDown )
                        penSelectedRow = visibleRows - 1;
                }
                TimeoutSet( POPUP_TIMEOUT, LibraryTimeoutPopupHandler );
                HighlightRow( penSelectedRow );
                handled = true;
                break;
            }

            if ( ! RctPtInRectangle( x, y,
                    &displayList[ penSelectedRow ].bounds ) ) {
                for ( i = 0; i < visibleRows; i++ ) {
                    if ( RctPtInRectangle( x, y,
                             &displayList[ i ].bounds ) ) {
                        penSelectedRow = i;
                        HighlightRow( penSelectedRow );
                        lastIndex = displayList[ i ].index;
                        TimeoutReset();
                        break;
                    }
                }
            }
            handled = true;
            break;

        case penUpEvent:
            TimeoutRelease();

            if ( penSelectedRow == NO_ROW )
                break;

            if ( RctPtInRectangle( x, y,
                    &displayList[ penSelectedRow ].bounds ) ) {
                OpenNewDocument( displayList[ penSelectedRow ].index );
            }
            penSelectedRow = NO_ROW;
            HighlightRow( penSelectedRow );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}



/* Highlight a specific row of data after unhighlighting the previous row */
static void HighlightRow
    (
    Int16 newRow
    )
{
    if ( highlightedRow != NO_ROW && 0 <= highlightedRow &&
         highlightedRow < visibleRows ) {
            HighlightRectangle( &displayList[ highlightedRow ].bounds,
                0, false, BUTTON );
    }

    if ( newRow != NO_ROW && 0 <= newRow && newRow < visibleRows )
        HighlightRectangle( &displayList[ newRow ].bounds, 0, true, BUTTON );

    highlightedRow = newRow;
}



/* Scroll to a given row */
static void ScrollTo
    (
    const Int16 row   /* row to scroll to */
    )
{
    Int16  maxScroll;
    Int16  delta;
    UInt16 absDelta;
    Int16  newRow;

    maxScroll   = numberOfRows - visibleRows;

    /* if there's nothing to move... */
    if ( ( maxScroll < 0 ) ||
         /* or if we can't go any further up... */
         ( row <= firstVisibleRow && firstVisibleRow == 0 ) ||
         /* or if we can't go any futher down... */
         ( maxScroll <= row && firstVisibleRow == maxScroll ) )
        return;

    if ( row <= 0 )
        newRow = 0;
    else if ( maxScroll <= row )
        newRow = maxScroll;
    else
        newRow = row;

    delta           = newRow - firstVisibleRow;

    if ( delta < 0 )
        absDelta = -delta;
    else
        absDelta = delta;

    if ( delta != 0 ) {
        if ( visibleRows <= absDelta ) {
            firstVisibleRow = newRow;
            DrawRecords( 0, visibleRows );
        }
        else {
            Coord            distance;
            WinDirectionType direction;
            RectangleType    vacatedRectangle;

            firstVisibleRow = newRow;
            distance        = absDelta * displayList[ 0 ].bounds.extent.y;
            direction       = ( delta < 0 ) ? winDown : winUp;
            WinScrollRectangle( &docBounds, direction, distance,
                &vacatedRectangle );
            WinEraseRectangle( &vacatedRectangle, 0 );
            if ( highlightedRow != NO_ROW ) {
                highlightedRow -= delta;
                if ( highlightedRow < 0 ||
                     visibleRows <= highlightedRow ) {
                    highlightedRow = NO_ROW;
                }
            }    
            if ( delta < 0 ) {
                DrawRecords( 0, absDelta );
            }
            else {
                DrawRecords( visibleRows - absDelta, absDelta );
            }
        }
    }
}



/* Scroll up */
void ScrollUp
    (
    Int16 amount    /* number of rows to scroll */
    )
{
    ScrollTo( firstVisibleRow - amount );
}



/* Scroll down */
void ScrollDown
    (
    Int16 amount    /* number of rows to scroll */
    )
{
    ScrollTo( firstVisibleRow + amount );
}



/* Delete all displayed documents. Revived from dbmngform.c */
static void DeleteAllDocuments( void )
{
    ProgressPtr prg;

    if ( FrmAlert( confirmDeleteAllDoc ) == SELECT_OK ) {
        volatile UInt16 failed;
        PrgCallbackData data;

        failed = 0;
        if ( Support33() )
            prg = PrgStartDialog( "Deleting All Documents",
                      callbackDeleteDocument, &data );
        else
            prg = NULL;

        while ( numberOfRows-- ) {
            Int16 index;

            index = indexList[ numberOfRows ];
            ErrTry {
                DeleteListDocument( index, prg );
                if ( prg != NULL && PrgUserCancel( prg ) )
                    break;
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                failed++;
            } ErrEndCatch
        }
        if ( prg != NULL )
            PrgStopDialog(prg, false);
        ScrollTo( 0 );
        if ( failed != 0 )
            FrmAlert( errCannotDeleteAllDoc );
    }
}



/* Delete read documents. Revived from dbmngform.c */
static void DeleteReadDocuments( void )
{
    if ( FrmAlert( confirmDeleteReadDoc ) == SELECT_OK ) {
        volatile UInt16 failed;
        ProgressPtr prg;
        PrgCallbackData data;

        failed = 0;
        if ( Support33() )
            prg = PrgStartDialog( "Deleting Read Documents", 
                      callbackDeleteDocument, &data );
        else
            prg = NULL;

        while ( numberOfRows-- ) {
            Int16 index;
            DocumentInfo* docInfo;

            index = indexList[ numberOfRows ];
            docInfo = ListGet( docList, index + 1 );

            if ( docInfo->timestamp != 0 ){
                ErrTry {
                    DeleteListDocument( index, prg );
                    if ( prg != NULL && PrgUserCancel( prg ) )
                        break;
                }
                ErrCatch( UNUSED_PARAM( err ) ) {
                    failed++;
                } ErrEndCatch
            }
        }
        if ( prg != NULL )
            PrgStopDialog(prg, false);
        ScrollTo( 0 );
        if ( failed != 0 )
            FrmAlert( errCannotDeleteReadDoc );
    }
}



/* Delete the document specified by the user */
static void DeleteOneDocument
    (
    Int16 index /* item number in list */
    )
{
    DocumentInfo* docInfo;

    docInfo = ListGet( docList, index + 1 );
    if ( FrmCustomAlert( confirmDelete, docInfo->name,
            NULL, NULL ) == SELECT_OK ) {
        ErrTry {
            DeleteListDocument( index, NULL );

            ScrollUp( ONE_ROW );
            DrawRecords( 0, visibleRows );
        }
        ErrCatch( err ) {
            if ( err == vfsErrFilePermissionDenied || err == dmErrReadOnly )
                FrmAlert( errReadOnly );
            else
                FrmAlert( errCannotDeleteDoc );
        } ErrEndCatch
    }
}



/* Unread the document, and delete meta document if confirmed */
static void UnreadOneDocument
    (
    Int16 index /* item number in list */
    )
{
    DocumentInfo* docInfo;

    docInfo = ListGet( docList, index + 1 );
    if ( FrmCustomAlert( confirmDeleteMetaDoc, docInfo->name,
                         NULL, NULL ) == SELECT_OK ) {
        DeleteMetaDocument( docInfo->name, docInfo->location );
    }
    docInfo->timestamp = 0;
    StoreReadTimeInDocumentList( docInfo->name, docInfo->timestamp );

    DrawRecords( 0, visibleRows );
}



/* Delete document and remove it from the list of documents */
static void DeleteListDocument
    (
    Int16       index, /* item number in list */
    ProgressPtr prg    /* progress dialog */
    )
    /* THROWS */
{
    Char* label;
    DocumentInfo*   docInfo;
    
    
    docInfo = ListGet( docList, index + 1 );
    label   = docInfo->name;
    if ( prg != NULL ){
        PrgUpdateDialog(prg, PROGRESS_NO_ERROR, 0, label, true);
    }

    DeleteDocument( docInfo );
    DeleteMetaDocument( label, docInfo->location );
    RemoveDocInfoRecord( index );

    if ( STREQ( Prefs()->docName, docInfo->name ) && 
         Prefs()->location == docInfo->location ) {
        Prefs()->docName[ 0 ]   = '\0';
        Prefs()->location       = RAM;
    }
    ListTakeOut( docList, docInfo );
    SafeMemPtrFree( docInfo->filename );
    SafeMemPtrFree( docInfo );
    UpdateIndexList();
}



/* Function to call the appropriate category style */
static Boolean SelectCategory(void)
{
    Boolean changed;
    UInt8   categoryType;
    Boolean title;
    UInt16  advanced;
    UInt16  classic;

    changed = false;

    if ( IsSelectingCategoryFilter() ) {
        categoryType = CATEGORY_TRIGGER;
        title        = true;
        advanced     = Prefs()->categories;
    }
    else {
        MemHandle     handle;
        DocumentInfo* recordPtr;

        categoryType = CATEGORY_FILE;
        title        = false;

        handle = FindDocData( DocInfo(ReturnLastIndex())->name,
                    ALL_ELEMENTS, NULL );
        if ( handle == NULL ) {
            FrmAlert( infoCopyProtected );
            return changed;
        }

        recordPtr = MemHandleLock( handle );
        advanced = recordPtr->categories;
        MemHandleUnlock( handle );
    }

    if ( categoryName[ categoryType ] == NULL )
        categoryName[ categoryType ] = SafeMemPtrNew( dmCategoryLength );

    classic  = CategoryAtoC( advanced );
    advanced = CategoryCtoA( classic );

    GetCategoryName(classic, categoryName[ categoryType ]);
    changed = ProcessSelectCategory(libraryForm, title, &classic,
                categoryName[ categoryType ]);
    if ( ! IsSelectingCategoryFilter() )
        SetCategoryTrigger();
    advanced = CategoryCtoA( classic );

    if ( IsSelectingCategoryFilter() ) {
        Prefs()->categories = advanced;
    }
    else {
        StoreCategoriesInDocumentList( DocInfo(ReturnLastIndex())->name,
            advanced );
        DocInfo(ReturnLastIndex())->categories = advanced;
    }

    return changed;
}



/* Sets the category to the next one in the list */
static void SetNextCategory( void )
{
    UInt16  classic;

    classic = CategoryAtoC( Prefs()->categories );
    Prefs()->categories = CategoryCtoA( GetNextCategory( classic ) );
}



/* Sets the pulldown trigger to coincide with a 'classic' category style */
static void SetCategoryTrigger(void)
{
    UInt16       classic;
    ControlType* ctl;

    if ( categoryName[ CATEGORY_TRIGGER ] == NULL )
        categoryName[ CATEGORY_TRIGGER ] = SafeMemPtrNew( dmCategoryLength );

    classic = CategoryAtoC( Prefs()->categories );
    GetCategoryName( classic, categoryName[ CATEGORY_TRIGGER ] );
    ctl = FrmGetObjectPtr( libraryForm, FrmGetObjectIndex( libraryForm,
                                            frmLibraryCategoryPopup ) );
    CategorySetTriggerLabel( ctl, categoryName[ CATEGORY_TRIGGER ] );

    /* Converting back to advanced grabs the GCF of what was enabled initially.
       This is needed when moving directly from advanced to classic and
       multiple categories were enabled. Either way, doing this ensures that
       the proper data being represented, is displayed */
    Prefs()->categories = CategoryCtoA( classic );
}



/* Release memory allocated for categoryName used in 'classic' mode */
static void ReleaseCategoryName( void )
{
    UInt8 i;

    for ( i = 0; i < CATEGORY_MODES; i++ ) {
        SafeMemPtrFree( categoryName[ i ] );
        categoryName[ i ] = NULL;
    }
}



void SelectNextCategory(void)
{
    SetNextCategory();
    SetCategoryTrigger();
    UpdateIndexList();
    DrawRecords( 0, visibleRows );
}



/* convert from an 'advanced' category value to a 'classic' value */
static UInt16 CategoryAtoC
    (
    UInt16 advanced
    )
{
    UInt16 classic;

    if ( advanced == dmAllCategoriesAdvanced ) {
        classic = dmAllCategories;
    }
    else {
        classic = 0;
        while ( 1 < advanced ) {
            advanced >>= 1;
            classic++;
        }
    }

    return classic;
}



/* convert from a 'classic' category value to an 'advanced' value */
static UInt16 CategoryCtoA
    (
    UInt16 classic
    )
{
    UInt16 advanced;

    if ( classic == dmAllCategories )
        advanced = dmAllCategoriesAdvanced;
    else
        advanced = 1 << classic;

    return advanced;
}



/* Check if document is read only */
static Boolean IsReadOnlyDocument
    (
    Int16 index /* item number in list */
    )
{
    DocumentInfo* docInfo;

    docInfo = ListGet( docList, index + 1 );
    return ( ( docInfo->attributes & dmHdrAttrReadOnly ) ==
             dmHdrAttrReadOnly );
}



/* Check if document is copy protected */
static Boolean IsCopyProtectedDocument
    (
    Int16 index /* item number in list */
    )
{
    DocumentInfo* docInfo;

    docInfo = ListGet( docList, index + 1 );
    return ( ( docInfo->attributes & dmHdrAttrCopyPrevention ) ==
             dmHdrAttrCopyPrevention );
}



static void InitializeIconPopupList( void )
{
    Char*            temp;
    Char**           popupOptions;
    UInt16           entries;
    ListType*        list;

    MemSet( &popupList, sizeof( popupList ), 0 );
    temp  = popupList;
    SysCopyStringResource( temp, strLibraryOpen );
    temp += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strLibraryCategorize );
    temp += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strLibraryDelete );
    temp += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strLibraryRename );
    temp += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strLibraryUnread );
    temp += StrLen( temp ) + 1;
    entries = 5;
    if ( SupportBeam() ) {
        SysCopyStringResource( temp, strLibraryBeam );
        temp += StrLen( temp ) + 1;
        entries++;
    }

    popupHandle = SysFormPointerArrayToStrings( popupList, entries );
    popupOptions = MemHandleLock( popupHandle );

    list = GetObjectPtr( frmLibraryList );
    LstSetListChoices( list, popupOptions, entries );
    LstSetHeight( list, entries );
}



void IconPopupList
    (
    UInt16 row
    )
{
    ListType*       list;
    Coord           popupX;
    Coord           popupY;
    Int16           selection;
    UInt16          index;
    RectangleType*  bounds;

    index   = displayList[ row ].index;
    bounds  = &displayList[ row ].bounds;
    popupX = bounds->topLeft.x + 2;
    popupY = bounds->topLeft.y + bounds->extent.y + 2;

    list = GetObjectPtr( frmLibraryList );
    if ( IsHiResTypeSony( HiResType() ) )
        LstSetPosition( list, popupX / 2, popupY / 2 );
    else
        LstSetPosition( list, popupX, popupY );

    LstSetSelection( list, 0 );
    selection = LstPopupList( list );

    penSelectedRow = NO_ROW;
    HighlightRow( penSelectedRow );

    if ( selection != noListSelection ) {
        switch ( selection ) {
            case DOC_DELETE:
                DeleteOneDocument( index );
                break;

            case DOC_OPEN:
                OpenNewDocument( index );
                break;

            case DOC_RENAME:
                if ( IsReadOnlyDocument( index ) )
                    FrmAlert( errReadOnly );
                else
                    FrmPopupForm( frmRenameDoc );
                break;

            case DOC_CATEGORIZE:
                selectCategoryFilter = false;
                if ( Prefs()->categoryStyle ==
                     CATEGORY_ADVANCED ) {
                    FrmGotoForm( frmCategory );
                }
                else if ( Prefs()->categoryStyle == 
                            CATEGORY_CLASSIC ) {
                    SetCategoryListPosition( popupX, popupY );
                    SelectCategory();
                    UpdateIndexList();
                    DrawRecords( 0, visibleRows );
                }
                break;

            case DOC_UNREAD:
                UnreadOneDocument( index );
                break;

            case DOC_BEAM:
                if ( SupportBeam() ) {
                    if ( IsCopyProtectedDocument( index ) ) {
                        FrmAlert( infoCopyProtected );
                    }
                    else {
                        DocumentInfo* docInfo;

                        docInfo = ListGet( docList, index + 1 );
                        BeamDocument( docInfo );
                    }
                }
                else {
                    FrmAlert( infoNoBeamSupport );
                }
                break;

            default:
                break;
        }
    }
}



/* Return pointer to doc info structure */
DocumentInfo* DocInfo
    (
    Int16 index /* item number in list */
    )
{
    if ( 0 <= index )
        return ListGet( docList, index + 1 );
    else
        return GetLastDocInfo();
}



/* Return index for selected document */
Int16 ReturnLastIndex( void )
{
    return lastIndex;
}



/* Check if in select categories mode */
Boolean IsSelectingCategoryFilter( void )
{
    return selectCategoryFilter;
}


void SetCategoryListPosition
    (
    Coord x,
    Coord y
    )
{
    UInt16        index;
    RectangleType bounds;

    if ( IsHiResTypeSony( HiResType() ) ) {
        x /= 2;
        y /= 2;
    }

    index = FrmGetObjectIndex( libraryForm, frmLibraryCategoryList );
    FrmGetObjectBounds( libraryForm, index, &bounds );
    if ( selectCategoryFilter ) {
        bounds.topLeft.x = MaxExtentX() - bounds.extent.x - 1;
        bounds.topLeft.y = 0;
    }
    else {
        bounds.topLeft.x = x;
        bounds.topLeft.y = y;
    }
    FrmSetObjectBounds( libraryForm, index, &bounds );
}



/* Populate table with documents */
void InitializeDocInfoList( void )
    /* THROWS */
{
    if ( docList == NULL ) {
        UInt16 entries;

        /* make sure the list of documents is available */
        OpenDocList();

        /* Allocate arrays for document info table */
        entries = GetNumOfDocuments();
        if ( 0 < entries ) {
            UInt16 i;

            docList = ListCreate();

            /* Initialize doc info structure */
            for ( i = 0; i < entries; i++ ) {
                DocumentData*   recordPtr;
                MemHandle       handle;

                handle = ReturnDocInfoHandle( i );
                if ( handle != NULL ) {
                    DocumentInfo* docInfo;

                    docInfo     = NULL;
                    recordPtr   = MemHandleLock( handle );
                    docInfo = SafeMemPtrNew( sizeof *docInfo );

                    StrNCopy( docInfo->name, recordPtr->name,
                        dmDBNameLength );
                    docInfo->cardNo     = recordPtr->cardNo;
                    docInfo->created    = recordPtr->created;
                    docInfo->attributes = recordPtr->attributes;
                    docInfo->size       = recordPtr->size;
                    docInfo->active     = recordPtr->active;
                    docInfo->categories = recordPtr->categories;
                    docInfo->timestamp  = recordPtr->timestamp;
                    docInfo->location   = recordPtr->location;
                    if ( docInfo->location != RAM ) {
                        UInt16 fileLength;

                        fileLength          = StrLen( recordPtr->data ) + 1;
                        docInfo->filename   = SafeMemPtrNew( fileLength );
                        StrNCopy( docInfo->filename, recordPtr->data,
                            fileLength );
                        docInfo->volumeRef  = FindVolRefNum(
                                                recordPtr->data +
                                                fileLength );
                    }
                    else {
                        docInfo->filename = NULL;
                        docInfo->volumeRef = 0;
                    }
                    ListAppend( docList, docInfo );
                }
                MemHandleUnlock( handle );
            }
            if ( 0 < ListSize( docList ) )
                indexList = SafeMemPtrNew( ListSize( docList ) *
                                sizeof *indexList);
        }
    }
}



/* Release allocated memory */
void ReleaseDocInfoList( void )
{
    DocumentInfo* docInfo;
    DocumentInfo* nextDocInfo;

    /* clean up allocated memory for filenames */
    docInfo = ListFirst( docList );
    while ( docInfo != NULL ) {
        nextDocInfo = ListNext( docList, docInfo );
        ListTakeOut( docList, docInfo );
        SafeMemPtrFree( docInfo->filename );
        SafeMemPtrFree( docInfo );
        docInfo = nextDocInfo;
    }
    ListDelete( docList );
    docList = NULL;

    SafeMemPtrFree( indexList );
    indexList = NULL;
}



static void AllocateTypeIcon(void)
{
    TypeIconNode*   newIcon;
    DmResID         bmpResID;

    /* only one type is supported at the moment */
    if ( ! ListIsEmpty( typeIconList ))
        return;

    /* include correct version of the plucker icon (the only
       supported type at the moment) */
    bmpResID = HiResFontImage( displayFont, bmpSmallPluckerIcon,
        bmpSmallPluckerIcon_half, bmpSmallPluckerIcon_x2 );
    newIcon = SafeMemPtrNew( sizeof( TypeIconNode ) );
    newIcon->handle = DmGetResource( bitmapRsc, bmpResID );
    newIcon->bitmap = MemHandleLock( newIcon->handle );
    BmpGlueGetDimensions( newIcon->bitmap, &newIcon->width,
        &newIcon->height, NULL );
    DmReleaseResource( newIcon->handle );
    ListAppend( typeIconList, newIcon );
}



static void AllocateCardIcon(UInt16 volumeRef)
{
    CardIconNode*   newIcon;
    CardIconNode*   cardNode;
    DmResID         bmpResID;
    VolumeInfoType  volInfo;

    cardNode = ListFirst(cardIconList);
    while (cardNode != NULL) {
        if (cardNode->volumeRef == volumeRef)
            return;
        cardNode = ListNext(cardIconList, cardNode);
    }

    /* include all supported VFS icons */
    VFSVolumeInfo(volumeRef, &volInfo);
    switch (volInfo.mediaType) {
    case expMediaType_MemoryStick:
        bmpResID = HiResFontImage( displayFont, bmpMemoryStick,
            bmpMemoryStick_half, bmpMemoryStick_x2 );
        break;

    case expMediaType_CompactFlash:
        bmpResID = HiResFontImage( displayFont, bmpCompactFlash,
            bmpCompactFlash_half, bmpCompactFlash_x2 );
        break;

    case expMediaType_SecureDigital:
    default:
        bmpResID = HiResFontImage( displayFont, bmpSecureDigital,
            bmpSecureDigital_half, bmpSecureDigital_x2 );
        break;
    }
    newIcon = SafeMemPtrNew(sizeof(CardIconNode));
    newIcon->handle = DmGetResource(bitmapRsc, bmpResID);
    newIcon->bitmap = MemHandleLock( newIcon->handle );
    BmpGlueGetDimensions(newIcon->bitmap, &newIcon->width,
        &newIcon->height, NULL);
    DmReleaseResource( newIcon->handle );
    newIcon->volumeRef = volumeRef;
    ListAppend(cardIconList, newIcon);
}



static void DeAllocateIcons(void)
{
    TypeIconNode *typeNode;
    CardIconNode *cardNode;

    typeNode = ListFirst(typeIconList);
    while ( typeNode != NULL ) {
        TypeIconNode *next;

        next = ListNext( typeIconList, typeNode );

        MemHandleUnlock( typeNode->handle );
        SafeMemPtrFree(typeNode);

        typeNode = next;
    }
    cardNode = ListFirst(cardIconList);
    while (cardNode != NULL) {
        CardIconNode *next;

        next = ListNext( cardIconList, cardNode );

        MemHandleUnlock( cardNode->handle );
        SafeMemPtrFree(cardNode);

        cardNode = next;
    }
}


static void FormatDocument( UInt16 i, FormatType* format )
{
    DocumentInfo*   docInfo;
    DateTimeType    dateTime;
    UInt32          size;
    
    docInfo = ListGet( docList, i + 1 );

    /* Prepare the document icon */
    AllocateTypeIcon();
    if (docInfo->location != RAM)
        AllocateCardIcon(docInfo->volumeRef);

    /* Add the document name */
    StrNCopy( format->name, docInfo->name, dmDBNameLength );

    /* Format the document date */
    TimSecondsToDateTime( docInfo->created, &dateTime );
    DateToAscii( dateTime.month, dateTime.day, dateTime.year,
        (DateFormatType) PrefGetPreference( prefDateFormat ),
        format->date );

    /* Format the document size */
    size = ( docInfo->size + 512 ) / 1024;
    if ( size != 0 )
        StrPrintF( format->size, "%ldk", size );
    else
        StrPrintF( format->size, "%ldb", docInfo->size );
}


/* calculate the widths of columns so everything fits properly */
static void CalculateColumnWidths( void )
{
    Char    size[ sizeStringLength ];
    Char    date[ dateStringLength ];
    FontID  oldFont;

    typeColumnWidth = 0;
    dateColumnWidth = 0;
    sizeColumnWidth = 0;

    /* pre-calculate typeColumnWidth */

    if ( Prefs()->column.type == SHOW ) {
        BitmapType*     bitmap;
        DmResID         bmpResID;
        MemHandle       bitmapH;

        bmpResID = HiResFontImage( displayFont, bmpSmallPluckerIcon,
                    bmpSmallPluckerIcon_half, bmpSmallPluckerIcon_x2 );
        bitmapH = DmGetResource( bitmapRsc, bmpResID );
        if ( bitmapH != NULL ) {
            bitmap  = MemHandleLock( bitmapH );
            if ( bitmap != NULL )
            {
                BmpGlueGetDimensions( bitmap, (Int16 *)&typeColumnWidth, NULL, NULL );
                typeColumnWidth += 2;
                MemHandleUnlock( bitmapH );
            }
            DmReleaseResource( bitmapH );
        }
    }

    oldFont = FntSetFont( displayFont );

    /* pre-calculate dateColumnWidth */
    DateToAscii( 12, 31, 2003,
        (DateFormatType) PrefGetPreference( prefDateFormat ), date );
    dateColumnWidth = FntCharsWidth( date, StrLen( date ) ) + 2;

    /* pre-calculate sizeColumnWidth */
    StrPrintF( size, "%ldk", 99999L );
    sizeColumnWidth = FntCharsWidth( size, StrLen( size ) ) + 1;

    FntSetFont( oldFont );
}


/* Update list of documents sorting them according to the selected sort
   method */
void UpdateIndexList( void )
{
    UInt16          i;
    DocumentInfo*   docInfo;

    i               =  0;
    lastIndex       = -1;
    numberOfRows    =  0;

    /* reset current index list */
    SafeMemPtrFree( indexList );
    if ( 0 < ListSize( docList ) ) {
        indexList = SafeMemPtrNew( ListSize( docList ) * sizeof *indexList);
    }
    else {
        indexList = NULL;
        return;
    }

    docInfo = ListFirst( docList );
    while ( docInfo != NULL ) {
        Int16   j;
        UInt16  category;

        category = docInfo->categories & Prefs()->categories;
        if ( docInfo->active && ( ( Prefs()->filterMode == FILTER_OR &&
                                    category ) ||
                                  ( Prefs()->filterMode == FILTER_AND && 
                                    category == Prefs()->categories ) ) ) {

            /* Put items in sorted order */
            j = numberOfRows;
            if ( j != 0 ) {
                while ( j != 0 ) {
                    Int16           result;
                    UInt16          k;
                    DocumentInfo*   prevDocInfo;

                    k           = indexList[ j - 1 ];
                    prevDocInfo = ListGet( docList, k + 1 );
                    result      = CompareItems( prevDocInfo, docInfo );
                    if ( result <= 0 ) {
                        break;
                    }

                    indexList[ j ] = indexList[ j - 1 ];
                    j--;
                }
            }
            indexList[ j ] = i;
            numberOfRows++;
        }
        i++;
        docInfo = ListNext( docList, docInfo );
    }
}



static void ReleaseAllocatedMemory(void)
{
    SafeMemPtrFree(indexList);
    indexList = NULL;

    SafeMemPtrFree(displayList);
    displayList = NULL;

    DeAllocateIcons();
    ReleaseCategoryName();

    ListDelete(cardIconList);
    cardIconList = NULL;

    ListDelete(typeIconList);
    typeIconList = NULL;

    if ( popupHandle != NULL ) {
        MemHandleUnlock( popupHandle );
        MemHandleFree( popupHandle );
        popupHandle = NULL;
    }
}



void SyncDocList(void)
{
    Boolean syncPolicy;

    syncPolicy = Prefs()->syncPolicy;
    Prefs()->syncPolicy = SYNC_AUTOMATIC;

    ShowSyncMessage();

    CloseDocList();
    ReleaseDocInfoList();
    InitializeDocInfoList();

    HideMessage();

    Prefs()->syncPolicy = syncPolicy;

    FrmUpdateForm(libraryFormId, frmRedrawUpdateCode);
}


/* Event handler for the library form */
Boolean LibraryFormHandleEvent
(
    EventType* event  /* pointer to an EventType structure */
)
{
    Boolean handled;

    handled = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            ResetLookup();
            switch ( event->data.ctlEnter.controlID ) {
                case frmLibraryCategoryBtn:
                    selectCategoryFilter = true;
                    FrmGotoForm( frmCategory );

                    firstVisibleRow = 0;
                    handled = true;
                    break;

                case frmLibraryHelp:
                    FrmHelp( strLibraryHelp );
                    handled = true;
                    break;
            }
            switch ( event->data.ctlSelect.controlID ) {
                case frmLibraryCategoryPopup:
                    selectCategoryFilter = true;
                    //SetCategoryListPosition( NULL, NULL );
                    SetCategoryListPosition( 0, 0 );
                    SelectCategory();
                    firstVisibleRow = 0;
                    penSelectedRow = NO_ROW;
                    UpdateIndexList();
                    DrawRecords( 0, visibleRows );
                    handled = true;
                    break;
            }
            break;

        case sclRepeatEvent:
            SelectorHighlightRow( false );
            ScrollTo( event->data.sclRepeat.newValue );
            handled = false;
            break;

        case keyDownEvent:
            switch ( event->data.keyDown.chr ) {
                case pageUpChr:
#ifdef HAVE_HANDSPRING_SDK
                    if (( JogdialType() == handspringJogdial ) &&
                        HsEvtMetaEvent ( event ) )
                        break;
#endif
                    if ( IsFiveWayEvent( event ) &&
                         SelectorGetRow() != NO_ROW )
                        break;
                    if ( numberOfRows == 0 )
                        break;
                    handled = LibrarySelectorHandler( librarySelectorPageUp,
                                  0 );
                    break;

                case pageDownChr:
#ifdef HAVE_HANDSPRING_SDK
                    if (( JogdialType() == handspringJogdial ) &&
                        HsEvtMetaEvent ( event ) )
                        break;
#endif
                    if ( IsFiveWayEvent( event ) &&
                         SelectorGetRow() != NO_ROW )
                        break;
                    if ( numberOfRows == 0 )
                        break;
                    handled = LibrarySelectorHandler( librarySelectorPageDown,
                                  0 );
                    break;

#ifdef RETURN_TO_PREV_APP
            case vchrLaunch:
                DoAutoscrollToggle( AUTOSCROLL_OFF );
                SendAppStopEvent();
                handled = true;
                break;
#endif

                default:
                    break;
            }
            if ( ! handled && HaveJogdial() )
                handled = JogdialLibraryHandler( event );
            if ( ! handled && HaveFiveWay() )
                handled = FiveWayLibraryHandler( event );
            if ( ! handled )
                handled = LibraryHandleKeyboard( event->data.keyDown.chr, 
                              event->data.keyDown.keyCode );
            break;

        case penDownEvent:
        case penMoveEvent:
        case penUpEvent:
            ResetLookup();
            if ( ! ListIsEmpty( docList ) ) {
                handled = HandlePenOnSort( event );
                if ( ! handled )
                    handled = HandlePenOnRecord( event );
            }
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
#ifdef HAVE_GRAY_FONT
            oldOrientation = GrayFntSetOrientation( GRAY_FONT_NORMAL );
#endif
            LibraryFormInit();
            handled = true;
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            if ( forceLibraryFormUpdate ) {
                forceLibraryFormUpdate = false;
                FrmGotoForm( GetValidForm( libraryFormId ) );
            }
            handled = true;
            break;

        case frmUpdateEvent:
            switch ( event->data.frmUpdate.updateCode ) {
                case frmRedrawUpdateCode:
                    if ( ! initializing ) {
#ifdef HAVE_SILKSCREEN
                        ResizeHandleFrmRedrawUpdateCode();
#endif
                        ReleaseAllocatedMemory();
                        LibraryFormInit();
                    }
                    handled = true;
                    break;

                case frmUpdateList:
                    ReleaseDocInfoList();
                    InitializeDocInfoList();
                    ShowSortMessage();
                    UpdateIndexList();
                    HideMessage();
                    DrawRecords( 0, visibleRows );
                    handled = true;
                    break;
            }
            break;

        case frmCloseEvent:
#ifdef HAVE_GRAY_FONT
            GrayFntSetOrientation( oldOrientation );
#endif
            ReleaseAllocatedMemory();
            LoadUserFont( GetUserFontNumber( Prefs()->mainUserFontName,
                              true, fontCacheDoc ) );
            RefreshCustomFonts();
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

         case menuCmdBarOpenEvent:
             ResetLookup();
             MenuCmdBarAddButton( menuCmdBarOnLeft, BarDeleteBitmap,
                menuCmdBarResultMenuItem, mOptionsDeleteAll, NULL );
             MenuCmdBarAddButton( menuCmdBarOnLeft, bmpSettings,
                menuCmdBarResultMenuItem, mOptionsPref, NULL );
             MenuCmdBarAddButton( menuCmdBarOnLeft, bmpSyncList,
                menuCmdBarResultMenuItem, mOptionsSyncList, NULL );

             handled = false;
             break;

        case menuEvent:
            /* Refresh display to get rid of command text in bottom
               left corner */
            MenuEraseStatus( NULL );
            switch ( event->data.menu.itemID ) {
                case mOptionsDeleteAll:
                    DeleteAllDocuments();
                    FrmUpdateForm( libraryFormId, frmRedrawUpdateCode );
                    handled = true;
                    break;

                case mOptionsDeleteRead:
                    DeleteReadDocuments();
                    FrmUpdateForm( libraryFormId, frmRedrawUpdateCode );
                    handled = true;
                    break;

                case mOptionsType:
                    /* toggle show type column */
                    Prefs()->column.type ^= SHOW;
                    FrmUpdateForm( libraryFormId, frmRedrawUpdateCode );
                    handled = true;
                    break;

                case mOptionsDate:
                    /* toggle show date column */
                    Prefs()->column.date ^= SHOW;
                    FrmUpdateForm( libraryFormId, frmRedrawUpdateCode );
                    handled = true;
                    break;

                case mOptionsSize:
                    /* toggle show date column */
                    Prefs()->column.size ^= SHOW;
                    FrmUpdateForm( libraryFormId, frmRedrawUpdateCode );
                    handled = true;
                    break;

                case mOptionsSyncList:
                    SyncDocList();
                    handled = true;
                    break;

                default:
                    handled = HandleCommonMenuItems( event->data.menu.itemID );
                    break;
            }
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}



/* Handle the calling of the popup whenever the timeout period has been met */
static void LibraryTimeoutPopupHandler( void )
{
    TimeoutRelease();
    IconPopupList( penSelectedRow );
}



Int16 LibraryGetNumberOfRows( void )
{
    return numberOfRows;
}



Int16 LibraryGetFirstVisibleRow( void )
{
    return firstVisibleRow;
}



void LibrarySetLastIndexForRow
    (
    UInt16 row
    )
{
    lastIndex = LibraryGetDisplayListIndex(row);
}



void LibraryHighlightRow( Int16 row )
{
    HighlightRow( row );
}



Boolean callbackDeleteDocument(PrgCallbackDataPtr cbP)
{
    StrCopy(cbP->textP,cbP->message);
    cbP->textChanged = true;
    return true;
}
