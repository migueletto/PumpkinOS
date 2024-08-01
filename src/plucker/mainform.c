/*
 * $Id: mainform.c,v 1.147 2004/05/08 09:04:54 nordstrom Exp $
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

#include "bookmark.h"
#include "bookmarkform.h"
#include "control.h"
#include "debug.h"
#include "document.h"
#include "emailform.h"
#include "fiveway.h"
#include "fullscreenform.h"
#include "hardcopyform.h"
#include "hires.h"
#include "history.h"
#include "image.h"
#include "jogdial.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "prefsbrowsing.h"
#include "prefslibrary.h"
#include "prefsbutton.h"
#include "prefscontrol.h"
#include "resourceids.h"
#include "search.h"
#include "DIA.h"
#include "util.h"
#include "screen.h"
#include "genericfile.h"
#include "os.h"
#include "libraryform.h"
#include "link.h"
#include "table.h"
#include "keyboard.h"
#include "font.h"
#include "rotate.h"
#include "dimensions.h"
#include "metadocument.h"
#include "handera.h"

#include "mainform.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    UInt16 bitmap;
    UInt16 command;
} ToolbarButton;


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define TOP             0
#define BOTTOM          10


static const ToolbarButton ListOfToolbarButtons[] = {
    { 0, 0 },
    { bmpBookmark, mGoAddBookmark },
    { BarInfoBitmap, mViewDetails },
    { bmpSettings, mOptionsPref },
    { bmpBtnAction, mOptionsButton },
    { bmpTapAction, mOptionsControl },
    { bmpTop, mGoTop },
    { bmpBottom, mGoBottom },
    { bmpCopyToMemo, mViewCopyToMemo },
    { BarDeleteBitmap, mGoDeleteDocument }
};


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void SetScrollbarLocation( FormType* mainForm ) MAINFORM_SECTION;
static Boolean HandleScrollbar( EventType* event ) MAINFORM_SECTION;
static Boolean MainFormWrong( void ) MAINFORM_SECTION;
static Boolean HandleMenuEvent(UInt16 itemID) MAINFORM_SECTION;
static Boolean HandleKeyboardAction(WChar ch) MAINFORM_SECTION;



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean forceMainFormUpdate = false;




/* Main form needs updating when it becomes active. */
void SetMainFormUpdate( void )
{
    forceMainFormUpdate = true;
}




/* Handle a keyboard press via the keyboard map */
Boolean HandleKeyboardAction
    (
    WChar ch    /* character to handle */
    )
{
    SelectType action;
    action = GetKeyboardAction( ch, 0 );
    if ( action == KEYBOARD_ACTION_NONE )
        return false;
    DoSelectTypeAction( action );
    return true;
}



/* Handle common menu items */
Boolean HandleCommonMenuItems
    (
    UInt16 itemID   /* ID for selected menu item */
    )
{
    Boolean     handled;

    handled = false;
    switch ( itemID ) {
        case mOptionsAbout:
            FrmPopupForm( frmAbout );
            handled = true;
            break;

        case mOptionsContact:
            SetMailto( 0 );
            if ( IsFullscreenformActive() )
                FsFrmGotoForm( frmEmail );
            else
                FrmGotoForm( frmEmail );
            handled = true;
            break;

        case mOptionsFont:
            FrmPopupForm( GetValidForm( frmFont ) );
            handled = true;
            break;
            
        case mOptionsKeyboard:
            FrmPopupForm( frmKeyboard );
            handled = true;
            break;

        case mOptionsPref:
            if ( IsFormLibrary( Prefs()->lastForm ) )
                PrefsLibraryShowFirst();
            else if ( IsFormMain( Prefs()->lastForm ) )
                PrefsBrowsingShowFirst();

            if ( IsFullscreenformActive() )
                FsFrmGotoForm( frmPrefs );
            else
                FrmGotoForm( frmPrefs );
            handled = true;
            break;

        case mOptionsButton:
            PrefsButtonShowFirst();
            FrmGotoForm( frmPrefs );
            handled = true;
            break;

        case mOptionsControl:
            PrefsControlShowFirst();
            FrmGotoForm( frmPrefs );
            handled = true;
            break;



        default:
            break;
    }
    return handled;
}



/* Handle a menu event */
static Boolean HandleMenuEvent
    (
    UInt16 itemID   /* ID for selected menu item */
    )
{
    Boolean handled;

    handled = false;
    if ( HandleCommonMenuItems( itemID ) )
        return true;
    else {
        switch ( itemID ) {
            case mGoHome:
            case mGoBack:
            case mGoForward:
                DoControlAction( mGoHome - itemID - 1 );
                handled = true;
                break;

            case mGoTop:
                GotoLocation( TOP );
                handled = true;
                break;

            case mGoBottom:
                GotoLocation( BOTTOM );
                handled = true;
                break;

            case mGoSearch:
                FrmPopupForm( frmSearch );
                handled = true;
                break;

            case mGoSearchAgain:
                SearchAgain();
                handled = true;
                break;

            case mGoAddBookmark:
                FrmPopupForm( frmAddBookmark );
                handled = true;
                break;

            case mGoBookmarks:
                FrmPopupForm( frmBookmarks );
                handled = true;
                break;
                
#ifdef SUPPORT_WORD_LOOKUP
            case mGoLookup:
                DoSelectTypeAction( SELECT_WORD_LOOKUP );
                handled = true;
                break;
#endif

            case mViewAutoscrollStart:
                DoAutoscrollToggle( AUTOSCROLL_ON );
                break;

            case mViewAutoscrollStop:
                DoAutoscrollToggle( AUTOSCROLL_OFF );
                break;
                
            case mViewToggleFullscreen:
                DoSelectTypeAction( SELECT_TOGGLE_FULLSCREEN );
                break;

            case mViewAutoscrollIncr:
                DoAutoscrollIncr();
                break;
                
            case mViewAutoscrollDecr:
                DoAutoscrollDecr();
                break;

            case mGoLibrary:
                FrmGotoForm( GetValidForm( frmLibrary ) );
                ReleaseFsTableHandle();
                ReleaseFsImageHandle();
                handled = true;
                break;

            case mViewCopyToMemo:
                DoHardcopy();
                handled = true;
                break;

            case mViewDetails:
                FrmPopupForm( frmDetails );
                handled = true;
                break;

            case mGoDeleteDocument:
                DoDeleteDocument();
                handled = true;
                break;

            default:
                break;
        }
    }
    return handled;
}



/* Initialize the main form */
void MainFormInit( void )
{
    FormType*               mainForm;

    UpdateMargins();
    ReRenderAllIfNeeded();
    InitializeViewportBoundaries();
    ResetActualAnchor();

    mainForm        = FrmGetFormPtr( GetMainFormId() );

    SetScrollbarLocation( mainForm );

    FrmDrawForm( mainForm );
    /* WinSetActiveWindow must be called or nothing from control.c
       will work until after the user touches something on the form */
    WinSetActiveWindow( FrmGetWindowHandle( mainForm ) );

    ClearControlBounds();
    if ( Prefs()->toolbar == TOOLBAR_SILK ) {
        HanderaSetSilkScreen();
    }
    else if ( Prefs()->toolbar != TOOLBAR_NONE ) {
        GetControlBounds( mainForm, bmpMenu, MENUCONTROL );
        GetControlBounds( mainForm, bmpBookmark, BOOKMARKCONTROL );
        GetControlBounds( mainForm, frmMainPercentPopup, OFFSETCONTROL );
        GetControlBounds( mainForm, bmpDbase, LIBRARYCONTROL );
        GetControlBounds( mainForm, bmpFind, FINDCONTROL );
        GetControlBounds( mainForm, bmpAgain, AGAINCONTROL );
        GetControlBounds( mainForm, bmpLeft, LEFTCONTROL );
        GetControlBounds( mainForm, bmpHome, HOMECONTROL );
        GetControlBounds( mainForm, bmpRight, RIGHTCONTROL );
        GetControlBounds( mainForm, bmpAutoscrollStart,
            AUTOSCROLLSTARTCONTROL );
        GetControlBounds( mainForm, bmpAutoscrollStop, AUTOSCROLLSTOPCONTROL );
        GetControlBounds( mainForm, bmpAutoscrollIncr, AUTOSCROLLINCRCONTROL );
        GetControlBounds( mainForm, bmpAutoscrollDecr, AUTOSCROLLDECRCONTROL );

        UpdateVerticalOffset( 0 );
    }
    Prefs()->lastForm = GetMainFormId();
    DoAutoscrollToggle( AUTOSCROLL_OFF );
}



static void SetScrollbarLocation
    (
    FormType* mainForm
    )
{
    //ScrollBarType*  scrollbar;
    UInt16          scrollbarIndex;
    RectangleType   location;
    UInt16          prevCoordSys;

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );

    scrollbarIndex = FrmGetObjectIndex( mainForm, frmMainScrollBar );
    //scrollbar      = FrmGetObjectPtr( mainForm, scrollbarIndex );

    InitializeViewportBoundaries();
    UpdateMargins();

    if ( Prefs()->scrollbar == SCROLLBAR_NONE ) {
        FrmHideObject( mainForm, scrollbarIndex );
    }
    else {
        Int16 adjustFactor;
        if ( IsHiResTypeSony( HiResType() ) )
            adjustFactor = 2;
        else
            adjustFactor = 1;
#ifdef HAVE_ROTATE
        switch ( Prefs()->rotate ) {
            case ROTATE_ZERO:
#endif
                if ( Prefs()->scrollbar == SCROLLBAR_LEFT ) {
                    location.topLeft.x = 0;
                    location.topLeft.y = TopLeftY();
                }
                else { /* SCROLLBAR_RIGHT */
                    location.topLeft.x = TopLeftX() + ExtentX();
                    location.topLeft.y = TopLeftY();
                }
                location.topLeft.x /= adjustFactor;
                location.topLeft.y /= adjustFactor;
                location.extent.y   = ExtentY() / adjustFactor - 1;
                if ( location.topLeft.x != 0 )
                    location.topLeft.x++;
                location.topLeft.y++;
                location.extent.x = SCROLLBAR_WIDTH;
#ifdef HAVE_ROTATE
                SetScrollbarDirection( DIRECTION_DOWN );
                break;
            case ROTATE_MINUS90:
            case ROTATE_PLUS90:
                if ( ( Prefs()->rotate == ROTATE_MINUS90 &&
                       Prefs()->scrollbar == SCROLLBAR_LEFT ) ||
                     ( Prefs()->rotate == ROTATE_PLUS90 &&
                       Prefs()->scrollbar == SCROLLBAR_RIGHT ) ) {
                    location.topLeft.y = TopLeftY()
                        - adjustFactor * ( SCROLLBAR_WIDTH + 1 ) ;
                }
                else {
                    location.topLeft.y = TopLeftY() + ExtentY() + adjustFactor;
                }
                location.extent.y   = SCROLLBAR_WIDTH;
                location.topLeft.x  = TopLeftX() / adjustFactor;
                location.topLeft.y /= adjustFactor;
                location.extent.x   = ExtentX() / adjustFactor;
                if ( Prefs()->rotate == ROTATE_MINUS90 )
                    SetScrollbarDirection( DIRECTION_UP );
                else
                    SetScrollbarDirection( DIRECTION_DOWN );
                break;
        }
#endif

        FrmSetObjectBounds( mainForm, scrollbarIndex, &location );
        FrmShowObject( mainForm, scrollbarIndex );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}



/* Handle any scrollbar events */
static Boolean HandleScrollbar
    (
    EventType* event /* pointer to an EventType structure */
    )
{
    static YOffset  trueStartingValue = 0;
    Int16      startingValue;
    FormType*       mainForm;
    UInt16          scrollbarIndex;
    ScrollBarType*  scrollbar;
    YOffset         delta;
    YOffset    newPosition;
    Int16            min;  /* dummy */
    Int16            max;
    Int16            size; /* dummy */

    mainForm        = FrmGetFormPtr( FrmGetActiveFormID() );
    scrollbarIndex  = FrmGetObjectIndex( mainForm, frmMainScrollBar );
    scrollbar       = FrmGetObjectPtr( mainForm, scrollbarIndex );
    delta           = 0;

    if ( event->eType == sclEnterEvent ) {
        SclGetScrollBar( scrollbar, &startingValue, &min, &max, &size );
        trueStartingValue = ScrollbarScale( startingValue );
    }
    else if ( event->eType == sclExitEvent ) {
        newPosition = ScrollbarScale( event->data.sclExit.newValue );
        delta = trueStartingValue - newPosition;
    }
    else {
        return SclHandleEvent( scrollbar, event );
    }

    if ( delta < 0 && -GetDefaultMainStyleHeight() < delta )
        delta = -GetDefaultMainStyleHeight();
    if ( 0 < delta && delta < GetDefaultMainStyleHeight() )
        delta = GetDefaultMainStyleHeight();
    if ( delta != 0 )
        DoPageMove( delta );

    return false;
}



/* Watcher function to verify that the actual formId matches the intended one */
static Boolean MainFormWrong( void )
{
    UInt16 properMainFormId;
    UInt16 activeMainFormId;

    properMainFormId = GetMainFormId();
    activeMainFormId = FrmGetActiveFormID();

    if ( properMainFormId != activeMainFormId ) {
        FrmGotoForm( properMainFormId );
        return true;
    }
    Prefs()->lastForm = GetMainFormId();
    DoAutoscrollToggle( AUTOSCROLL_OFF );
    return false;
}




#ifdef SUPPORT_WORD_LOOKUP
static void GoSelectedWord( void )
{
     Char* word;

     word = GetSelectedWord( NULL );
     if ( word != NULL ) {
        switch ( Prefs()->selectedWordAction ) {
            case SELECT_WORD_SEARCH_FORM:
                AddSearchString( word );
                FrmPopupForm( frmSearch );
                break;

#ifdef SUPPORT_PPI
            case SELECT_WORD_PPI:
            {
                LocalID dbID;

                dbID = DmFindDatabase( 0, "PPI" );
                if ( dbID != 0 ) {
                    UInt32 outputValue;

                    SysAppLaunch( 0, dbID, 0, sysAppLaunchCmdCustomBase,
                        word, &outputValue );
                }
                break;
            }
#endif
            case SELECT_WORD_TO_CLIPBOARD:
                ClipboardAddItem( clipboardText, word, StrLen( word ) );
                break;

            default:
                return;
        }
    }
}
#endif



/* Event handler for the main form. */
Boolean MainFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean successful;
    Boolean handled;
    UInt16  prevCoordSys;

    successful = false;
    handled = false;

    if ( HandleScrollbar( event ) ) {
        handled = true;
        return handled;
    }
#if defined( HAVE_FIVEWAY_SDK ) || defined( HAVE_HANDSPRING_SDK )
    if ( FiveWayMainHandler( event ) ) {
        handled = true;
        return handled;
    }
#endif    

    switch ( event->eType ) {
        case penDownEvent:
            prevCoordSys = PalmSetCoordinateSystem( NATIVE );
            TranslateHiResPenEvents( event );
            /* Tap in text area ( i.e. not in header or on scrollbar ) or
               tap in active area of header ( i.e. title or graphical
               buttons ) */
            if ( TopLeftY() < event->screenY &&
                 event->screenY < ( TopLeftY() + ExtentY() ) ) {
                switch ( Prefs()->scrollbar ) {
                    case SCROLLBAR_LEFT:
                        if ( TopLeftX() < event->screenX ) {
                            HandlePenDown( event->screenX, event->screenY );
                            handled = true;
                        }
                        else {
                            DoAutoscrollToggle( AUTOSCROLL_OFF );
                        }
                        break;

                    default:
                        if ( event->screenX < ExtentX() ) {
                            HandlePenDown( event->screenX, event->screenY );
                            handled = true;
                        }
                        else {
                            DoAutoscrollToggle( AUTOSCROLL_OFF );  
                        }
                        break;
                }
            }
            else {
                HandlePenDown( event->screenX, event->screenY );
                handled = true;
            }
            PalmSetCoordinateSystem( prevCoordSys );
            break;

        case penMoveEvent:
            prevCoordSys = PalmSetCoordinateSystem( NATIVE );
            TranslateHiResPenEvents( event );
            if ( Prefs()->controlMode == MODE3 )
                HandlePenMove( event->screenX, event->screenY );
            handled = true;
            PalmSetCoordinateSystem( prevCoordSys );
            break;

        case penUpEvent:
            prevCoordSys = PalmSetCoordinateSystem( NATIVE );
            TranslateHiResPenEvents( event );
            HandlePenUp( event->screenX, event->screenY );
            PalmSetCoordinateSystem( prevCoordSys );
            handled = true;
            break;

        case keyDownEvent:
            RotateArrowKeys( &( event->data.keyDown.chr ) );
            switch ( event->data.keyDown.chr ) {
                case pageUpChr:
#ifdef HAVE_HANDSPRING_SDK
                  if ( ( JogdialType() == handspringJogdial ) &&
                       HsEvtMetaEvent ( event ) )
                      break;
#endif
                    if ( Prefs()->arrowMode[ UP_ARROW ] != SELECT_NONE &&
                         Prefs()->arrowKeys )
                        DoSelectTypeAction( Prefs()->arrowMode[ UP_ARROW ] );
                    else
                        DoPageMove( RotGetScrollValue() );
                    handled = true;
                    break;

                case pageDownChr:
#ifdef HAVE_HANDSPRING_SDK
                    if ( ( JogdialType() == handspringJogdial ) &&
                       HsEvtMetaEvent ( event ) )
                      break;
#endif
                    if ( Prefs()->arrowMode[ DOWN_ARROW ] != SELECT_NONE &&
                         Prefs()->arrowKeys )
                        DoSelectTypeAction( Prefs()->arrowMode[ DOWN_ARROW ] );
                    else
                        DoPageMove( -RotGetScrollValue() );
                    handled = true;
                    break;

                case spaceChr:
                    if ( Prefs()->gestMode[ GESTURES_RIGHT ] != SELECT_NONE &&
                         Prefs()->gestures )
                        DoSelectTypeAction(
                            Prefs()->gestMode[ GESTURES_RIGHT ] );
                    else
                        HandleKeyboardAction( event->data.keyDown.chr );
                    handled = true;
                    break;

                case 'i':
                case 'I':
                case '1':
                case 'l':
                case 'L':
                    /* Although 'i' and 'I' are received under normal Graffiti
                       while 'l' and 'L' are under Graffiti 2, there is not
                       much use in differentiating between the two because
                       there is no harm in honouring both methods under both
                       intrepretations of Graffiti. However, '1' is valid
                       under both versions. */
                    if ( Prefs()->gestMode[ GESTURES_DOWN ] != SELECT_NONE &&
                         Prefs()->gestures )
                        DoSelectTypeAction(
                            Prefs()->gestMode[ GESTURES_DOWN ] );
                    else
                        HandleKeyboardAction( event->data.keyDown.chr );
                    handled = true;
                    break;

                case '.':
                    /* The same argument can be given for '.' except that this
                       is only really here to benefit Graffiti 2 users. Normal
                       Graffiti users won't notice any difference */
                    if ( Prefs()->gestMode[ GESTURES_TAP ] != SELECT_NONE &&
                         Prefs()->gestures )
                        DoSelectTypeAction( Prefs()->gestMode[ GESTURES_TAP ] );
                    else
                        HandleKeyboardAction( event->data.keyDown.chr );
                    handled = true;
                    break;

                case backspaceChr:
                    if ( Prefs()->gestMode[ GESTURES_LEFT ] != SELECT_NONE &&
                         Prefs()->gestures )
                        DoSelectTypeAction(
                            Prefs()->gestMode[ GESTURES_LEFT ] );
                    else
                        HandleKeyboardAction( event->data.keyDown.chr );
                    handled = true;
                    break;

                default:
                    handled = HandleKeyboardAction( event->data.keyDown.chr );
                    break;
            }
            if ( ! handled )
                handled = JogdialMainHandler( event );

            if ( ! handled ) {
                /* handle plucker specific key events */
                if ( vchrPluckerMin <= event->data.keyDown.chr &&
                     event->data.keyDown.chr <= vchrPluckerMax ) {
                    DoControlAction( -( event->data.keyDown.chr -
                        vchrPluckerMin + 1 ) );
                    handled = true;
                }
            }
            /* Some devices (ie Zire 71) don't reset their auto-off timer
               upon some evtKeyDown events. Let's remind them to. */
            EvtResetAutoOffTimer();
            break;

        case menuCmdBarOpenEvent:
            MenuCmdBarAddButton( menuCmdBarOnLeft, bmpRight, 
                menuCmdBarResultMenuItem, mGoForward, NULL );
            MenuCmdBarAddButton( menuCmdBarOnLeft, bmpHome,
                menuCmdBarResultMenuItem, mGoHome, NULL );
            MenuCmdBarAddButton( menuCmdBarOnLeft, bmpLeft, 
                menuCmdBarResultMenuItem, mGoBack, NULL );
            if ( Prefs()->toolbarButton != 0 ) {
                MenuCmdBarAddButton( menuCmdBarOnLeft,
                    ListOfToolbarButtons[ Prefs()->toolbarButton ].bitmap,
                    menuCmdBarResultMenuItem,
                    ListOfToolbarButtons[ Prefs()->toolbarButton ].command,
                    NULL );
            }
            MenuCmdBarAddButton( menuCmdBarOnLeft, bmpDbase,
                menuCmdBarResultMenuItem, mGoLibrary, NULL );

            handled = false;
            break;

        case menuEvent:
            /* Refresh display to get rid of command text in bottom left
               corner */
            MenuEraseStatus( NULL );
            ErrTry {
                handled = HandleMenuEvent( event->data.menu.itemID );
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                handled = false;
            } ErrEndCatch
            break;

        case frmGotoEvent:
            SetFindPatternData( event->data.frmGoto.matchPos,
                event->data.frmGoto.matchLen );
            SetLastSearchedRecordId( event->data.frmGoto.recordNum );
            JumpToRecord( event->data.frmGoto.recordNum, NO_OFFSET,
                event->data.frmGoto.matchPos );

            ClearFindPatternData();

            handled = true;
            break;


#ifdef SUPPORT_WORD_LOOKUP
        case pluckerSelectedWordEvent:
            GoSelectedWord();
            handled = true;
            break;
#endif

        case frmUpdateEvent:
            if ( MainFormWrong() ) {
                handled = true;
                break;
            }
            SetScreenMode();
            LineCacheClear();
            UpdateMargins();
            ReRenderAllIfNeeded();
            SetScrollbarLocation( FrmGetFormPtr( GetMainFormId() ) );
            if ( event->data.frmUpdate.updateCode == frmRedrawUpdateCode ||
                 event->data.frmUpdate.updateCode == frmUpdateAnchors ) {
                //FormType* mainForm;
                WriteModeType  mode;

                if ( event->data.frmUpdate.updateCode == frmUpdateAnchors ) {
                    mode = WRITEMODE_LAYOUT_ANCHORS;
                }
                else {
                    mode                = WRITEMODE_DRAW_CHAR;
                    forceMainFormUpdate = false;
#ifdef HAVE_SILKSCREEN
                    ResizeHandleFrmRedrawUpdateCode();
#endif
                }

                //mainForm = FrmGetFormPtr( GetMainFormId() );
                if ( mode == WRITEMODE_DRAW_CHAR ) {
                    MainFormInit();
                    /* If we are just coming back from frmFullscreen,
                       don't reload GetHistoryCurrent().
                       There is all ready a DoControlAction() pending. */
                    if ( FrmGetFormId( FrmGetFirstForm() ) == frmFullscreen ) {
                        handled = true;
                        break;
                    }
                }

                successful = ViewRecord( GetHistoryCurrent(),
                                ! GetAddHistory(), NO_OFFSET, NO_OFFSET,
                                mode );
                if ( ! successful ) {
                    FrmAlert( warnInsufficientMemory );
                    FrmGotoForm( GetValidForm( frmLibrary ) );
                }
                handled = true;
            }
            else if ( event->data.frmUpdate.updateCode == frmViewRecord ) {
                UInt16 reference;

                reference = GetBookmarkRecordID();
                SetVisitedLink( reference );
                successful = ViewRecord( reference, ! GetAddHistory(),
                                NO_OFFSET, NO_OFFSET, WRITEMODE_DRAW_CHAR );
                if ( ! successful ) {
                    FrmAlert( warnInsufficientMemory );
                    FrmGotoForm( GetValidForm( frmLibrary ) );
                }
                handled = true;
            }
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case winEnterEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleWinEnterEvent();
#endif
            if ( forceMainFormUpdate ) {
                forceMainFormUpdate = false;
                if ( ! MainFormWrong() ) {
                    EventType e;
                    MemSet( &e, sizeof(EventType), 0 );
                    e.eType                     = frmUpdateEvent;
                    e.data.frmUpdate.formID     = GetMainFormId();
                    e.data.frmUpdate.updateCode = frmRedrawUpdateCode;
                    EvtAddUniqueEventToQueue( &e, 0, true );
                }
            }
            handled = true;
            break;

        case frmOpenEvent:
        {
            UInt16 reference;
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            if ( MainFormWrong() ) {
                handled = true;
                break;
            }
            forceMainFormUpdate = false;
            SetScreenMode();
            MainFormInit();
            LineCacheClear();

            /* Start with last visited page or home page at the very first
               activation */
            reference = GetHistoryCurrent();
            SetVisitedLink( reference );
            successful = ViewRecord( reference, ! GetAddHistory(), NO_OFFSET,
                             NO_OFFSET, WRITEMODE_DRAW_CHAR );
            if ( ! successful ) {
                FrmAlert( warnInsufficientMemory );
                ReleaseFsTableHandle();
                ReleaseFsImageHandle();
                FrmGotoForm( GetValidForm( frmLibrary ) );
            }

            handled = true;
            break;
        }

        case frmCloseEvent:
        {
            MemHandle handle;

            HanderaResetSilkScreen();

            ReleaseFsTableHandle();

            ReleaseFsImageHandle();

            handle = GetCurrentRecord();
            FreeRecordHandle( &handle );
            ResetRecordReferences();
            
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif

            handled = false;
            break;
        }

        default: {
            if ( event->eType == nilEvent && Prefs()->autoscrollEnabled &&
                ( Int16 )( Prefs()->autoscrollLastScrollTime + 
                           Prefs()->autoscrollInterval -
                           ( UInt16 )TimGetTicks() ) <= 0
               ) {
                Int16       jump;

                jump = Prefs()->autoscrollJump;
                switch ( Prefs()->autoscrollDir ) {
                    case AUTOSCROLL_DOWN:
                        jump = -jump;
                        if ( OnLastScreen() ) {
                            DoAutoscrollToggle( AUTOSCROLL_OFF );
                        }
                        break;

                    case AUTOSCROLL_UP:
                        if ( OnFirstScreen() ) {
                            DoAutoscrollToggle( AUTOSCROLL_OFF );
                        }
                        break;
                }

                if ( Prefs()->autoscrollStayOn ) {
                    EvtResetAutoOffTimer();
                }

                switch ( Prefs()->autoscrollMode ) {
                    case AUTOSCROLL_PIXELS:
                        DoPageMove ( jump );
                        break;

                    case AUTOSCROLL_PAGES:
                        DoPageMove( RotGetScrollValue() * jump );
                        break;
                }
                Prefs()->autoscrollLastScrollTime = TimGetTicks();

            }
            handled = false;
            break;
        }
    }

    return handled;
}

