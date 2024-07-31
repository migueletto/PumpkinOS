/*
 * $Id: control.c,v 1.132 2004/04/24 20:36:55 prussar Exp $
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

#include "anchor.h"
#include "bookmark.h"
#include "debug.h"
#include "document.h"
#include "externalform.h"
#include "fullscreenform.h"
#include "genericfile.h"
#include "hardcopyform.h"
#include "hires.h"
#include "history.h"
#include "image.h"
#include "link.h"
#include "os.h"
#include "paragraph.h"
#include "prefsbutton.h"
#include "prefscontrol.h"
#include "prefsgesture.h"
#include "resourceids.h"
#include "search.h"
#include "table.h"
#include "DIA.h"
#include "dimensions.h"
#include "font.h"
#include "rotate.h"

#include "control.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define NUMBER_OF_TOOLBAR_OBJECTS   13
#define ADD_BOOKMARK                0
#define VIEW_BOOKMARK               1
#define TOP                         0
#define BOTTOM                      10
#define FULLSCREEN_TOP              10000
#define FULLSCREEN_BOTTOM           -10000


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef enum {
    REGION1,
    REGION2,
    REGION3,
    REGION4
} RegionType;


static struct {
    Int16 penDownY;
    Int16 penDownX;
    Int16 penMoveX;
    Int16 penMoveY;
    Int16 moveFromX;
    Int16 moveFromY;
    Int16 selectedControl;
} PenData;


typedef struct {
    RectangleType   bounds;
    AnchorStateType state;
} ControlDataType;



typedef Int16 ( *AnchorFinder )( void );


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void ClearPenData( void ) CONTROL_SECTION;
static void HighlightGraphicalControl( const Int16 controlID,
                const AnchorStateType state ) CONTROL_SECTION;
static void HighlightControl( const Int16 controlID,
                const AnchorStateType state ) CONTROL_SECTION;
static Int16 FindGraphicalControl( const Coord x, const Coord y ) CONTROL_SECTION;
static Int16 FindControlObject( const Coord x, const Coord y ) CONTROL_SECTION;
static void DoScreenAction( RegionType region ) CONTROL_SECTION;
static void HandleNextControl( void ) CONTROL_SECTION;
static void HandlePrevControl( void ) CONTROL_SECTION;
static void GoToFirstAnchor( void ) CONTROL_SECTION;
static void GoToLastAnchor( void ) CONTROL_SECTION;
#ifdef SUPPORT_WORD_LOOKUP
static Int16 FindSelectedWord( const Coord x, const Coord y ) CONTROL_SECTION;
static void ShowSelectWordTapIcon(void) CONTROL_SECTION;
static void HighlightSelectedWord( const AnchorStateType state ) CONTROL_SECTION;
static Boolean InSelectedWord ( const Coord x, const Coord y ) CONTROL_SECTION;
#endif


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/

static ControlDataType controls[ NUMBER_OF_TOOLBAR_OBJECTS ];
static Int16 actualAnchor = 0;
#ifdef SUPPORT_WORD_LOOKUP
static ControlDataType  selectedWord;
static Boolean          isSelectWordTapMode = false;
#endif



#ifdef SUPPORT_WORD_LOOKUP
/* Indicate that the next tap looks things up in the selected word */
static void ShowSelectWordTapIcon( void )
{
    FormType* mainForm;
    UInt16    prevCoordSys;

    if ( Prefs()->toolbar == TOOLBAR_NONE )
        return;

    mainForm = FrmGetFormPtr( GetMainFormId() );
    prevCoordSys = PalmSetCoordinateSystem( STANDARD );

    if ( Prefs()->toolbar == TOOLBAR_SILK ) {
        /* FIXME: figure this out */
    }
    else {
        FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpWait ) );

        if ( isSelectWordTapMode ) {
            FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpHome ) );
            FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpLookup ) );
        }
        else {
            FrmHideObject( mainForm, FrmGetObjectIndex( mainForm, bmpLookup ) );
            FrmShowObject( mainForm, FrmGetObjectIndex( mainForm, bmpHome ) );
        }
    }
    PalmSetCoordinateSystem( prevCoordSys );
}
#endif



/* Reset the prev/next anchor */
void ResetActualAnchor( void )
{
    actualAnchor = 0;
}



/* Unhighlight and reset the anchor for the prev/next controls */
void UnselectActualAnchor( void )
{
    if ( 0 < actualAnchor ) {
        HighlightAnchor( actualAnchor - 1, ANCHOR_UNSELECTED );
        actualAnchor = 0;
    }
}



static void HandleNextControl( void )
{
    Int16 controlObject;

    DeleteUnusedAnchors();

    controlObject = FindNextVisibleAnchor();
    if ( controlObject == NOT_FOUND ) {
        if ( ( IsFullscreenformActive() ) || ! OnLastScreen() ) {
            DoPageMove( -RotGetScrollValue() );
            controlObject = FindFirstVisibleAnchor();
        }
        else {
            controlObject = FindLastVisibleAnchor();
        }
        if ( controlObject != NOT_FOUND ) {
            HighlightAnchor( controlObject, ANCHOR_SELECTED );
        }
    }
    actualAnchor = controlObject + 1;
}



static void HandlePrevControl( void )
{
    Int16 controlObject;

    DeleteUnusedAnchors();

    controlObject = FindPrevVisibleAnchor();
    if ( controlObject == NOT_FOUND ) {
        if ( ( IsFullscreenformActive() ) || ! OnFirstScreen() ) {
            DoPageMove( RotGetScrollValue() );
            controlObject = FindLastVisibleAnchor();
        }
        else {
            controlObject = FindFirstVisibleAnchor();
        }
        if ( controlObject != NOT_FOUND ) {
            HighlightAnchor( controlObject, ANCHOR_SELECTED );
        }
    }
    actualAnchor = controlObject + 1;
}



/* Go to the first anchor on the screen */
static void GoToFirstAnchor( void )
{
    Int16 actualAnchor;

    DeleteUnusedAnchors();

    actualAnchor = FindFirstVisibleAnchor();

    if ( actualAnchor == NOT_FOUND ) return;

    HighlightAnchor( actualAnchor, ANCHOR_SELECTED );

    actualAnchor++;

    if ( IsFullscreenformActive() )
        FsDoControlAction( actualAnchor );
    else
        DoControlAction( actualAnchor );
}



/* Go to the last anchor on the screen */
static void GoToLastAnchor( void )
{
    Int16 actualAnchor;

    DeleteUnusedAnchors();

    actualAnchor = FindLastVisibleAnchor();

    if ( actualAnchor == NOT_FOUND ) return;

    HighlightAnchor( actualAnchor, ANCHOR_SELECTED );

    actualAnchor++;

    if ( IsFullscreenformActive() )
        FsDoControlAction( actualAnchor );
    else
        DoControlAction( actualAnchor );
}




/* Clear the structure holding pen data between events */
static void ClearPenData( void )
{
    MemSet( &PenData, sizeof( PenData ), 0 );
}



/* Set highlight status for a graphical control object */
static void HighlightGraphicalControl
    (
    const Int16             controlID,  /* control ID of the object */
    const AnchorStateType   state       /* anchor state ( ANCHOR_SELECTED or
                                           ANCHOR_UNSELECTED ) */
    )
{
    RectangleType bounds;

    bounds.topLeft.x = controls[ controlID ].bounds.topLeft.x;
    bounds.topLeft.y = controls[ controlID ].bounds.topLeft.y;
    bounds.extent.x = controls[ controlID ].bounds.extent.x;
    bounds.extent.y = controls[ controlID ].bounds.extent.y;

    /* the bounds need to be twice the default for sony devices */
    HiResAdjustBounds( &bounds, sonyHiRes );

    if ( controls[ controlID ].state != state ) {
        HighlightRectangle( &bounds, 0,
            ( state == ANCHOR_SELECTED ) ? true : false, BUTTON );
    }
}



#ifdef SUPPORT_WORD_LOOKUP
/* Set highlight status for a graphical control object */
static void HighlightSelectedWord
    (
    const AnchorStateType state       /* anchor state ( ANCHOR_SELECTED or
                                         ANCHOR_UNSELECTED ) */
    )
{
    if ( selectedWord.state != state ) {
        HighlightRectangle( &selectedWord.bounds, 0,
            ( state == ANCHOR_SELECTED ) ? true : false, BUTTON );
    }
}
#endif



/* Determine whether an anchor or control object should change highlight
   status */
static void HighlightControl
    (
    const Int16             controlID,  /* control ID of the object */
    const AnchorStateType   state       /* anchor state ( ANCHOR_SELECTED or
                                           ANCHOR_UNSELECTED ) */
    )
{
    if ( 0 < controlID )
        HighlightAnchor( controlID - 1, state );
    else if ( controlID < 0 ) {
#ifdef SUPPORT_WORD_LOOKUP
        if ( controlID == - ( SELECTEDWORDCONTROL + 1 ) )
        {
            HighlightSelectedWord( state );
            return;
        }
#endif
        HighlightGraphicalControl( - ( controlID + 1 ), state );
    }
}



/* Find control object at given location, return control ID, or NOT_FOUND
   if no object was found at the given location */
static Int16 FindGraphicalControl
    (
    const Coord x,
    const Coord y
    )
{
    Int16 controlID;

    for ( controlID = 0 ; controlID < NUMBER_OF_TOOLBAR_OBJECTS ; controlID++ )
        if ( RctPtInRectangle( x, y, &controls[ controlID ].bounds ) )
            return controlID;

    return NOT_FOUND;
}



/* Find control object at pen location */
static Int16 FindControlObject
    (
    const Coord x,
    const Coord y
    )
{
    Int16 controlObject;

    controlObject = FindGraphicalControl( x, y );

    /* event occurred on a graphical control */
    if ( controlObject != NOT_FOUND )
        return -( controlObject + 1 );

    /* event did not occur on anything in the toolbar */
    if ( y <= TopLeftY() || TopLeftY() + ExtentY() <= y )
        return 0;

    controlObject = AnchorIndex( x, y );

    /* event occurred on an anchor */
    if ( controlObject != NOT_FOUND )
        return controlObject + 1;

    /* event did not occur on anything */
    return 0;
}



#ifdef SUPPORT_WORD_LOOKUP
/* Is tap in last selected word? */
static Boolean InSelectedWord
    (
    const Coord x,
    const Coord y
    )
{
    return RctPtInRectangle( x, y, &selectedWord.bounds );
}



/* Find control object at pen location */
static Int16 FindSelectedWord
    (
    const Coord x,
    const Coord y
    )
{
    /* event occured in the toolbar */
    if ( y <= TopLeftY() || TopLeftY() + ExtentY() <= y )
        return 0;

    ShowSelectWordTapIcon();
    DoFindSelectedWord( x, y );
    if ( GetSelectedWord( &selectedWord.bounds ) != NULL ) {
        return -( SELECTEDWORDCONTROL + 1 );
    }

    /* event did not occur on anything */
    return 0;
}
#endif



/* Perform action for specified screen region */
static void DoScreenAction
    (
    RegionType region   /* screen region */
    )
{
    DoSelectTypeAction( Prefs()->select[ Prefs()->controlMode ][ region ] );
}



/* Jump to requested record */
void JumpToRecord
    (
    const UInt16  recordId, /* record ID */
    const Int16   pOffset,  /* offset to first paragraph */
    const Int16   cOffset   /* offset to first character */
    )
{
    ErrTry {
        ViewRecord( recordId, ADD_TO_HISTORY, pOffset, cOffset,
            WRITEMODE_DRAW_CHAR );
        SetVisitedLink( recordId );
    }
    ErrCatch( UNUSED_PARAM( err ) ) {
        ViewRecord( GetHistoryPrev(), FROM_HISTORY, NO_OFFSET, NO_OFFSET,
            WRITEMODE_DRAW_CHAR );
    } ErrEndCatch
}



/* Clear the structure holding control boundaries */
void ClearControlBounds( void )
{
    MemSet( &controls, sizeof( controls ), 0 );
}



/* Perform action assigned to given control object */
void DoControlAction
    (
    const Int16 control /* control value of the object */
    )
{
    UInt16  newRecord;
    Int16   controlID;

    /* Clear location of find pattern */
    ClearFindPatternData();

    if ( 0 < control ) {
        Int16   anchorIndex;
        UInt16  reference;
        Int16   offset;
        UInt16  image;

        anchorIndex = control - 1;
        reference   = GetVisibleReference( anchorIndex );
        offset      = GetVisibleOffset( anchorIndex );
        image       = GetVisibleImage( anchorIndex );

        SndPlaySystemSound( sndClick );

        if ( image != 0 ) {
            RectangleType   bounds;
            ListType*       list;
            Int16           x;
            Int16           y;
            UInt16          prevCoordSys;
            Int16           selection;

            bounds  = GetVisibleImagePosition( anchorIndex );
            list    = GetObjectPtr( frmMainImageDialog );

            x = ( bounds.extent.x - 60 ) / 2;
            if ( x < 0 )
                x = 0;
            y = ( bounds.extent.y - 22 ) / 2;
            if ( y < 0 )
                y = 0;
            x += bounds.topLeft.x;
            y += bounds.topLeft.y;
            LstSetPosition( list, x, y );

            prevCoordSys    = PalmSetCoordinateSystem( STANDARD );
            selection       = LstPopupList( list );
            PalmSetCoordinateSystem( prevCoordSys );
            if ( selection == noListSelection ) {
                return;
            }
            else if ( selection == 1 ) {
                reference = image;
            }
        }
        JumpToRecord( reference, offset, NO_OFFSET );

        return;
    }

    if ( control != 0 )
        SndPlaySystemSound( sndClick );

    controlID = -( control + 1 );
    switch ( controlID ) {
        case HOMECONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            ViewRecord( HOME_PAGE_ID, ADD_TO_HISTORY, NO_OFFSET, NO_OFFSET,
                WRITEMODE_DRAW_CHAR );
            SetVisitedLink( HOME_PAGE_ID );
            break;

        case LEFTCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            newRecord = GetHistoryPrev();
            if ( newRecord != NO_RECORD ) {
                ViewRecord( newRecord, FROM_HISTORY, NO_OFFSET, NO_OFFSET,
                    WRITEMODE_DRAW_CHAR );
                SetVisitedLink( newRecord );
            }
            else {
                FrmGotoForm( GetValidForm( frmLibrary ) );
            }
            break;

        case RIGHTCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            newRecord = GetHistoryNext();
            if ( newRecord != NO_RECORD ) {
                ViewRecord( newRecord, FROM_HISTORY, NO_OFFSET, NO_OFFSET,
                    WRITEMODE_DRAW_CHAR );
                SetVisitedLink( newRecord );
            }
            break;

        case LIBRARYCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            CloseDocument();
            FrmGotoForm( GetValidForm( frmLibrary ) );
            break;

        case FINDCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            FrmPopupForm( frmSearch );
            break;

        case AGAINCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            SearchAgain();
            break;

        case MENUCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            EvtEnqueueKey( menuChr, 0, commandKeyMask );
            break;

        case OFFSETCONTROL:
        {
            UInt16 prevCoordSys;
            Int16  selection;

            DoAutoscrollToggle( AUTOSCROLL_OFF );
            prevCoordSys = PalmSetCoordinateSystem( STANDARD );
            selection = LstPopupList( GetObjectPtr( frmMainPercentList ) );
            PalmSetCoordinateSystem( prevCoordSys );
            if ( selection != noListSelection )
                GotoLocation( selection );

            break;
        }

        case BOOKMARKCONTROL:
        {
            UInt16      prevCoordSys;
            ListType*   list;
            Int16       selection;
            Int16       extEntries;
            UInt16      numOfBookmarks;

            DoAutoscrollToggle( AUTOSCROLL_OFF );

            prevCoordSys = PalmSetCoordinateSystem( STANDARD );

            list            = GetObjectPtr( frmMainBookmarkList );
            numOfBookmarks  = CreatePopupBookmarkList( list );
            if ( numOfBookmarks == 0 ) {
                PalmSetCoordinateSystem( prevCoordSys );
                break;
            }

            selection = LstPopupList( GetObjectPtr( frmMainBookmarkList ) );
            ReleaseBookmarkList();
            PalmSetCoordinateSystem( prevCoordSys );

            if ( selection == ADD_BOOKMARK ) {
                FrmPopupForm( frmAddBookmark );
            }
            else if ( selection == VIEW_BOOKMARK ) {
                FrmPopupForm( frmBookmarks );
            }
            else if ( selection != noListSelection ) {
                selection -= 2;
                extEntries = CountExtBookmarks();
                if ( selection < extEntries )
                    GotoExtBookmark( selection );
                else {
                    selection -= extEntries;
                    newRecord = RestoreBookmarkData( selection );
                    if ( newRecord != NO_RECORD ) {
                        ViewRecord( newRecord, FROM_HISTORY, NO_OFFSET, NO_OFFSET,
                            WRITEMODE_DRAW_CHAR );
                        SetVisitedLink( newRecord );
                    }
                }
            }
            break;
        }

        case AUTOSCROLLSTARTCONTROL:
        case AUTOSCROLLSTOPCONTROL:
            DoAutoscrollToggle( AUTOSCROLL_TOGGLE );
            break;

        case AUTOSCROLLINCRCONTROL:
            DoAutoscrollIncr();
            break;

        case AUTOSCROLLDECRCONTROL:
            DoAutoscrollDecr();
            break;

        case COPYTOMEMOCONTROL:
            DoHardcopy();
            break;

#ifdef SUPPORT_WORD_LOOKUP
        case SELECTEDWORDCONTROL: {
            EventType newEvent;

            MemSet( &newEvent, sizeof( newEvent ), 0 );
            newEvent.eType = pluckerSelectedWordEvent;
            EvtAddEventToQueue( &newEvent );

            break;
        }
#endif

        default:
            break;
    }
}



/* Emit a key */
void EmitKey( UInt16 key, UInt16 modifiers )
{
    EventType newEvent;
    MemSet(&newEvent, sizeof(newEvent), 0);
    newEvent.eType = keyDownEvent;
    newEvent.data.keyDown.chr = key;
    newEvent.data.keyDown.modifiers = modifiers;
    EvtAddEventToQueue(&newEvent);
}



/* Perform action for specified select type */
void DoSelectTypeAction
    (
    SelectType selection    /* selection type */
    )
{
    if ( IsMainFormWinActive() || IsFullscreenformActive()
         || FrmGetActiveFormID() == frmLibrary ) {
        if ( IsMainFormWinActive() &&
             selection != SELECT_TOGGLE_AUTOSCROLL &&
             selection != SELECT_INCREASE_AUTOSCROLL &&
             selection != SELECT_DECREASE_AUTOSCROLL &&
             selection != SELECT_FULL_PAGE_UP &&
             selection != SELECT_FULL_PAGE_DOWN &&
             selection != SELECT_HALF_PAGE_UP &&
             selection != SELECT_HALF_PAGE_DOWN &&
             selection != SELECT_ONE_LINE_UP &&
             selection != SELECT_ONE_LINE_DOWN ) {
            DoAutoscrollToggle( AUTOSCROLL_OFF );
        }
        switch ( selection ) {
            case SELECT_NONE:
                break;
                
#ifdef SUPPORT_WORD_LOOKUP
            case SELECT_WORD_LOOKUP:
                isSelectWordTapMode = ! isSelectWordTapMode;
                ShowSelectWordTapIcon();
                break;
#endif

            case SELECT_SAVE_POSITION:
                AddToHistory( GetCurrentRecordId() );
                break;

            case SELECT_BRIGHTNESS_ADJUST:
                EmitKey( vchrBrightness, commandKeyMask );
                break;

            case SELECT_TOGGLE_BACKLIGHT:
                EmitKey( vchrBacklight, commandKeyMask );
                break;
                
            case SELECT_CONTRAST_ADJUST:
                EmitKey( vchrContrast, commandKeyMask );
                break;

            case SELECT_ONE_LINE_UP:
                DoPageMove( GetDefaultMainStyleHeight() );
                break;

            case SELECT_ONE_LINE_DOWN:
                DoPageMove( -GetDefaultMainStyleHeight() );
                break;

            case SELECT_FULL_PAGE_UP:
                if ( Prefs()->autoscrollEnabled &&
                    Prefs()->autoscrollDir == AUTOSCROLL_UP )
                    DoAutoscrollIncr();
                else if ( Prefs()->autoscrollEnabled )
                    DoAutoscrollDecr();
                else if ( Prefs()->pageControlsLink && OnFirstScreen() &&
                          NO_RECORD == GetSequentialRecordId( DIRECTION_UP ) )
                    GoToFirstAnchor();
                else
                    DoPageMove( RotGetScrollValue() );
                break;

            case SELECT_HALF_PAGE_UP:
                if ( Prefs()->autoscrollEnabled &&
                    Prefs()->autoscrollDir == AUTOSCROLL_UP )
                    DoAutoscrollIncr();
                else if ( Prefs()->autoscrollEnabled )
                    DoAutoscrollDecr();
                else if ( Prefs()->pageControlsLink && OnFirstScreen() &&
                          NO_RECORD == GetSequentialRecordId( DIRECTION_UP ) )
                    GoToFirstAnchor();
                else
                    DoPageMove( RotGetScrollValue() / 2 );
                break;

            case SELECT_FULL_PAGE_DOWN:
                if ( Prefs()->autoscrollEnabled &&
                    Prefs()->autoscrollDir == AUTOSCROLL_UP )
                    DoAutoscrollDecr();
                else if ( Prefs()->autoscrollEnabled )
                    DoAutoscrollIncr();
                else if ( Prefs()->pageControlsLink && OnLastScreen() )
                    GoToLastAnchor();
                else
                    DoPageMove( -RotGetScrollValue() );
                break;

            case SELECT_HALF_PAGE_DOWN:
                if ( Prefs()->autoscrollEnabled &&
                    Prefs()->autoscrollDir == AUTOSCROLL_UP )
                    DoAutoscrollDecr();
                else if ( Prefs()->autoscrollEnabled )
                    DoAutoscrollIncr();
                else if ( Prefs()->pageControlsLink && OnLastScreen() )
                    GoToLastAnchor();
                else
                    DoPageMove( -RotGetScrollValue() / 2 );
                break;

            case SELECT_GO_BACK:
                if ( IsFullscreenformActive() )
                    FsDoControlAction( -( LEFTCONTROL + 1 ) );
                else
                    DoControlAction( -( LEFTCONTROL + 1 ) );
                break;

            case SELECT_GO_FORWARD:
                if ( IsFullscreenformActive() )
                    FsDoControlAction( -( RIGHTCONTROL + 1 ) );
                else
                    DoControlAction( -( RIGHTCONTROL + 1 ) );
                break;

            case SELECT_GO_HOME:
                if ( IsFullscreenformActive() )
                    FsDoControlAction( -( HOMECONTROL + 1 ) );
                else
                    DoControlAction( -( HOMECONTROL + 1 ) );
                break;

            case SELECT_GO_TO_TOP:
                if ( IsFullscreenformActive() )
                    FsAdjustVerticalOffset( FULLSCREEN_TOP );
                else
                    GotoLocation( TOP );
                break;

            case SELECT_GO_TO_BOTTOM:
                if ( IsFullscreenformActive() )
                    FsAdjustVerticalOffset( FULLSCREEN_BOTTOM );
                else
                    GotoLocation( BOTTOM );
                break;

            case SELECT_FIND:
                FrmPopupForm( frmSearch );
                break;

            case SELECT_FIND_AGAIN:
                SearchAgain();
                break;

            case SELECT_ADD_BOOKMARK:
                FrmPopupForm( frmAddBookmark );
                break;

            case SELECT_VIEW_BOOKMARKS:
                FrmPopupForm( frmBookmarks );
                break;

            case SELECT_OPEN_LIBRARY:
                CloseDocument();
                FrmGotoForm( GetValidForm( frmLibrary ) );
                break;

            case SELECT_DETAILS:
                FrmPopupForm( frmDetails );
                break;

            case SELECT_PREFS:
                FrmGotoForm( frmPrefs );
                break;

            case SELECT_BUTTON_ACTION:
                PrefsButtonShowFirst();
                FrmGotoForm( frmPrefs );
                break;

            case SELECT_TAP_ACTION:
                PrefsControlShowFirst();
                FrmGotoForm( frmPrefs );
                break;

            case SELECT_GESTURE_ACTION:
                PrefsGestureShowFirst();
                FrmGotoForm( frmPrefs );
                break;

            case SELECT_TOGGLE_AUTOSCROLL:
                DoAutoscrollToggle( AUTOSCROLL_TOGGLE );
                break;

            case SELECT_INCREASE_AUTOSCROLL:
                DoAutoscrollIncr();
                break;
                
            case SELECT_DECREASE_AUTOSCROLL:
                DoAutoscrollDecr();
                break;

            case SELECT_NEXT_ANCHOR:
                HandleNextControl();
                break;

            case SELECT_PREV_ANCHOR:
                HandlePrevControl();
                break;

            case SELECT_GO_TO_LINK:
                if ( IsFullscreenformActive() ) {
                    if ( 0 < actualAnchor )
                        FsDoControlAction( actualAnchor );
                } else {
                    DoControlAction( actualAnchor );
                }
                break;

            case SELECT_COPY_TO_MEMO:
                DoHardcopy();
                break;

            case SELECT_DELETE_DOCUMENT:
                DoDeleteDocument();
                break;

            case SELECT_FONT:
                FrmPopupForm( GetValidForm( frmFont ) );
                break;

            case SELECT_COMMAND_STROKE:
                EmitKey( vchrCommand, commandKeyMask );
                break;

            case SELECT_MENU:
                EmitKey( vchrMenu, commandKeyMask );
                break;

#ifdef HAVE_ROTATE
            case SELECT_ROTATE_LEFT: {
                RotateType angle;
                angle = Prefs()->rotate;
                switch ( angle ) {
                    case ROTATE_ZERO:
                        angle = ROTATE_PLUS90;
                        break;
                    case ROTATE_PLUS90:
                        angle = ROTATE_MINUS90;
                        break;
                    case ROTATE_MINUS90:
                        angle = ROTATE_ZERO;
                }
                if ( Prefs()->individualDocumentFonts )
                    GetHistoryPtr()->rotate = angle;
                else
                    Prefs()->rotate = angle;
                FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
                break;
            }

            case SELECT_ROTATE_RIGHT: {
                RotateType angle;
                angle = Prefs()->rotate;
                switch ( angle ) {
                    case ROTATE_PLUS90:
                        angle = ROTATE_ZERO;
                        break;
                    case ROTATE_MINUS90:
                        angle = ROTATE_PLUS90;
                        break;
                    case ROTATE_ZERO:
                        angle = ROTATE_MINUS90;
                        break;
                }
                if ( Prefs()->individualDocumentFonts )
                    GetHistoryPtr()->rotate = angle;
                else
                    Prefs()->rotate = angle;
                FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
                break;
            }
#endif

            case SELECT_TOGGLE_TOOLBAR:
                if ( ! IsFormMain( Prefs()->lastForm ) )
                    break;
                if ( Prefs()->toolbar != TOOLBAR_NONE ) {
                    Prefs()->savedToolbar = Prefs()->toolbar;
                    Prefs()->toolbar      = TOOLBAR_NONE;
                }
                else {
                    if ( Prefs()->savedToolbar == TOOLBAR_NONE )
                        Prefs()->toolbar = TOOLBAR_TOP;
                    else
                        Prefs()->toolbar = Prefs()->savedToolbar;
                }
#ifdef HAVE_SILKSCREEN
                if ( GetDIAState() == DIA_STATE_NO_STATUS_BAR &&
                   Prefs()->toolbar != TOOLBAR_NONE )
                    SetDIAState( Prefs()->savedSilkscreen );
#endif
                FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
                break;

            case SELECT_TOGGLE_FULLSCREEN: {
                UInt16       newToolbarState;
                UInt16       newScrollbarState;
                DIAStateType newDIAState;
                if ( Prefs()->toolbar == TOOLBAR_NONE &&
                    Prefs()->scrollbar == SCROLLBAR_NONE
                   ) {
                    /* From fullscreen to non-fullscreen */
                    if ( Prefs()->savedToolbar == TOOLBAR_NONE &&
                         Prefs()->savedScrollbar == SCROLLBAR_NONE ) {
                        Prefs()->savedToolbar   = TOOLBAR_TOP;
                        Prefs()->savedScrollbar = SCROLLBAR_RIGHT;
                    }
                    if ( Prefs()->savedSilkscreen == DIA_STATE_NO_STATUS_BAR ) {
                        Prefs()->savedSilkscreen = DIA_STATE_MIN;
                    }
                    newToolbarState   = Prefs()->savedToolbar;
                    newScrollbarState = Prefs()->savedScrollbar;
                    newDIAState       = Prefs()->savedSilkscreen;
                }
                else {
                    /* From non-fullscreen to fullscreen */
                    newToolbarState   = TOOLBAR_NONE;
                    newScrollbarState = SCROLLBAR_NONE;
                    newDIAState       = DIA_STATE_NO_STATUS_BAR;
                }
                Prefs()->savedToolbar    = Prefs()->toolbar;
                Prefs()->savedScrollbar  = Prefs()->scrollbar;
                Prefs()->savedSilkscreen = GetDIAState();
                Prefs()->toolbar         = newToolbarState;
                Prefs()->scrollbar       = newScrollbarState;
                SetDIAState( newDIAState );
                FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
                break;
            }

            default:
                break;
        }
    }
}



/* Respond to pen tap */
Boolean HandlePenDown
    (
    const Coord x,
    const Coord y
    )
{
    Boolean handled = false;

    PenData.penDownX = PenData.moveFromX = x;
    PenData.penDownY = PenData.moveFromY = y;

#ifdef HAVE_SILKSCREEN
    if ( DIA_STATE_NO_STATUS_BAR == GetDIAState() ) {
        RectangleType bounds;
        GetHiddenStatusBarArea( &bounds );
        HiResAdjustBounds( &bounds, sonyHiRes );
        if ( RctPtInRectangle( x, y, &bounds ) ) {
            if ( Prefs()->toolbar == TOOLBAR_NONE &&
                 Prefs()->scrollbar == SCROLLBAR_NONE ) {
                DoSelectTypeAction( SELECT_TOGGLE_FULLSCREEN );
            }
            else {
                if ( Prefs()->savedSilkscreen == DIA_STATE_NO_STATUS_BAR ) {
                    Prefs()->savedSilkscreen = DIA_STATE_MIN;
                }
                SetDIAState( Prefs()->savedSilkscreen );
            }
            handled = true;
            return handled;
        }
    }
#endif

#ifdef SUPPORT_WORD_LOOKUP
    if ( isSelectWordTapMode ) {
        PenData.selectedControl = FindSelectedWord( x, y );
        isSelectWordTapMode     = false;
        ShowSelectWordTapIcon();
    }
    else if ( Prefs()->selectWordTap == SELECT_WORD_TAP_ONCE ) {
        PenData.selectedControl = FindControlObject( x, y );
        if ( PenData.selectedControl == 0 )
            PenData.selectedControl = FindSelectedWord( x, y );
    }
    else
#endif
        PenData.selectedControl = FindControlObject( x, y );

    if ( PenData.selectedControl != 0 ) {
        HighlightControl( PenData.selectedControl, ANCHOR_SELECTED );
        handled = true;
    }
    else if ( Prefs()->controlMode != MODE3 && TopLeftY() < y &&
              y < TopLeftY() + ExtentY() ) {
        RegionType region;
        Coord      rotX;
        Coord      rotY;

        rotX = x;
        rotY = y;

        RotFromScreenXY( &rotX, &rotY );

        if ( Prefs()->controlMode == MODE1 ) {
            if ( rotY <= ( RotTopLeftY() + RotExtentY() / 4 ) )
                region = REGION1;
            else if ( rotY <= ( RotTopLeftY() + RotExtentY() / 2 ) )
                region = REGION2;
            else if ( rotY <= ( RotTopLeftY() + 3 * RotExtentY() / 4 ) )
                region = REGION3;
            else
                region = REGION4;
        }
        else {
            if ( rotY <= ( RotTopLeftY() + RotExtentY() / 2 ) ) {
                if ( rotX <= ( RotTopLeftX() + RotExtentX() / 2 ) )
                    region = REGION1;
                else
                    region = REGION2;
            }
            else {
                if ( rotX <= ( RotTopLeftX() + RotExtentX() / 2 ) )
                    region = REGION3;
                else
                    region = REGION4;
            }
        }
        DoScreenAction( region );
        handled = true;
    }
    return handled;
}



/* Respond to pen movement */
Boolean HandlePenMove
    (
    const Coord x,
    const Coord y
    )
{
    Int16 mouseDelta;
    Coord rotDownX;
    Coord rotDownY;
    Coord rotMoveX;
    Coord rotMoveY;
    Coord rotMoveFromX;
    Coord rotMoveFromY;
    Boolean handled = false;

    PenData.penMoveY    = y;
    rotMoveX = x;
    rotMoveY = y;
    rotDownX = PenData.penDownX;
    rotDownY = PenData.penDownY;
    rotMoveFromX = PenData.moveFromX;
    rotMoveFromY = PenData.moveFromY;

    RotFromScreenXY( &rotMoveX, &rotMoveY );
    RotFromScreenXY( &rotDownX, &rotDownY );
    RotFromScreenXY( &rotMoveFromX, &rotMoveFromY );

    mouseDelta = rotDownY - rotMoveY;
    if ( PenData.selectedControl != 0 &&
         ( ( mouseDelta < -3 ) || ( 3 < mouseDelta ) ) ) {
        HighlightControl( PenData.selectedControl, ANCHOR_UNSELECTED );
        PenData.selectedControl = 0;
        handled = true;
    }
    if ( PenData.selectedControl == 0 ) {
        DoPageMove( rotMoveY - rotMoveFromY );
        PenData.moveFromX = x;
        PenData.moveFromY = y;
        handled           = true;
    }
    return handled;
}



/* Respond to pen release */
Boolean HandlePenUp
    (
    const Coord x,
    const Coord y
    )
{
    if ( PenData.selectedControl != 0 )
        HighlightControl( PenData.selectedControl, ANCHOR_UNSELECTED );

    if (
#ifdef SUPPORT_WORD_LOOKUP
          ( PenData.selectedControl == -( SELECTEDWORDCONTROL + 1 ) &&
            InSelectedWord( x, y ) ) ||
#endif
         PenData.selectedControl == FindControlObject( x, y ) ) {
        DoControlAction( PenData.selectedControl );
    }

    ClearPenData();
    PenData.moveFromX = -1;
    PenData.moveFromY = -1;

    return true; /* always handled */
}



/* Retrieve the bounds of an object */
void GetControlBounds
    (
    const FormType* form,       /* pointer to memory block that contains
                                   the form */
    const UInt16    objectId,   /* ID of an object in the form */
    const Int16     controlID   /* control ID of the object */
    )
{
    FrmGetObjectBounds( form, FrmGetObjectIndex( form, objectId ),
        &controls[ controlID ].bounds );
    HiResAdjustBounds( &controls[ controlID ].bounds, palmHiRes );
}



/* Go to location in record */
void GotoLocation
    (
    const Int16 location    /* location in record ( percentage from top
                               measured in steps of 10% ) */
    )
{
    YOffset     seqOffset;
    YOffset     extentY;
    YOffset     seqHeight;
    YOffset     delta;

    seqOffset = GetSequenceOffset();
    extentY = ExtentY();
    seqHeight = GetSequenceHeight();
    delta = seqOffset + extentY - location * ( seqHeight / 10 + 1 );

    if ( NO_RECORD == GetSequentialRecordId( DIRECTION_DOWN ) &&
         GetSequenceHeight() <= GetSequenceOffset() + ExtentY() - delta )
        /* delta would put us too far down */
        delta = GetSequenceOffset() + ExtentY() - GetSequenceHeight();

    DoPageMove( delta );
}



/* Toggle Autoscroll */
void DoAutoscrollToggle
    (
    AutoscrollType toggle
    )
{
    FormType* mainForm;
    UInt16    prevCoordSys;

    if ( toggle == AUTOSCROLL_OFF &&
         ! Prefs()->autoscrollEnabled )
        return;

    mainForm = FrmGetFormPtr( GetMainFormId() );
    prevCoordSys = PalmSetCoordinateSystem( STANDARD );

    if ( toggle == AUTOSCROLL_TOGGLE )
        Prefs()->autoscrollEnabled = ! Prefs()->autoscrollEnabled;
    else if ( toggle == AUTOSCROLL_ON )
        Prefs()->autoscrollEnabled = true;
    else
        Prefs()->autoscrollEnabled = false;

    if ( Prefs()->autoscrollEnabled )
        Prefs()->autoscrollLastScrollTime = TimGetTicks();

    if ( Prefs()->autoscrollEnabled &&
         Prefs()->autoscrollDir == AUTOSCROLL_DOWN &&
         Prefs()->autoscrollMode == AUTOSCROLL_PIXELS ) {
        LineCacheActivate();
    }
    else {
        LineCacheRefreshCurrentScreenData();
        LineCacheDeactivate();
    }

    if ( Prefs()->toolbar != TOOLBAR_NONE && IsMainFormWinActive() ) {
        if ( Prefs()->autoscrollEnabled ) {
            if ( Prefs()->toolbar == TOOLBAR_SILK ) {
                /* FIXME: figure this out */
            }
            else {
                FrmHideObject( mainForm,
                    FrmGetObjectIndex( mainForm, bmpAutoscrollStart ) );
                FrmShowObject( mainForm,
                    FrmGetObjectIndex( mainForm, bmpAutoscrollStop ) );
            }
        }
        else {
            if ( Prefs()->toolbar == TOOLBAR_SILK ) {
                /* FIXME: figure this out */
            }
            else {
                FrmHideObject( mainForm,
                    FrmGetObjectIndex( mainForm, bmpAutoscrollStop ) );
                FrmShowObject( mainForm,
                    FrmGetObjectIndex( mainForm, bmpAutoscrollStart ) );
            }
        }
    }

    PalmSetCoordinateSystem( prevCoordSys );
}



/* Increase Autoscroll rate */
void DoAutoscrollIncr( void )
{
    if ( Prefs()->autoscrollInterval == AUTOSCROLL_MIN_INTERVAL_VALUE )
        Prefs()->autoscrollJump += AUTOSCROLL_INCR_JUMP_VALUE;
    else
    {
        if ( Prefs()->autoscrollInterval <=
           MILLISECONDS_TO_TICKS( AUTOSCROLL_FINE_TRANSITION ) )
            Prefs()->autoscrollInterval -=
                MILLISECONDS_TO_TICKS( AUTOSCROLL_FINE_INCR_INTERVAL_VALUE );
        else
            Prefs()->autoscrollInterval -=
                MILLISECONDS_TO_TICKS( AUTOSCROLL_INCR_INTERVAL_VALUE );
    }
}



/* Decrease Autoscroll rate */
void DoAutoscrollDecr( void )
{
    if ( Prefs()->autoscrollJump == AUTOSCROLL_MIN_JUMP_VALUE )
    {
        if ( Prefs()->autoscrollInterval <
             MILLISECONDS_TO_TICKS( AUTOSCROLL_FINE_TRANSITION ) )
            Prefs()->autoscrollInterval +=
                MILLISECONDS_TO_TICKS( AUTOSCROLL_FINE_INCR_INTERVAL_VALUE );
        else if ( Prefs()->autoscrollInterval <
                  MILLISECONDS_TO_TICKS( AUTOSCROLL_MAX_INTERVAL_VALUE ) )
            Prefs()->autoscrollInterval +=
                MILLISECONDS_TO_TICKS( AUTOSCROLL_INCR_INTERVAL_VALUE );
    }
    else
        Prefs()->autoscrollJump -= AUTOSCROLL_INCR_JUMP_VALUE;
}



/* Returns whether MainForm's Window is active and OK to draw toolbars on,
   or not (e.g. obscured by menu, etc.) */
Boolean IsMainFormWinActive( void )
{
    FormType*   mainForm;
    WinHandle   mainWindow;
    WinHandle   activeWindow;

    mainForm        = FrmGetFormPtr( GetMainFormId() );
    mainWindow      = FrmGetWindowHandle( mainForm );
    activeWindow    = WinGetActiveWindow();
    if ( mainWindow == activeWindow )
        return true;
    else
        return false;
}

