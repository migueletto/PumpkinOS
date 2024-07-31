/*
 * $Id: fullscreenform.c,v 1.48 2004/05/08 09:04:54 nordstrom Exp $
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
#include <BmpGlue.h>
#include <CtlGlue.h>

#include "anchor.h"
#include "control.h"
#include "debug.h"
#include "genericfile.h"
#include "hires.h"
#include "image.h"
#include "jogdial.h"
#include "mainform.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "rotate.h"
#include "screen.h"
#include "table.h"
#include "util.h"
#include "handera.h"

#include "fullscreenform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define FULLSCREEN_TOP      10000
#define FULLSCREEN_BOTTOM  -10000
#define MAX_FRAME_SIZE      50


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
 typedef struct {
    MemHandle bmpH;
    BitmapPtr bmp;
    Coord     x;
    Coord     y;
} ArrowType;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FormType*     fsForm;
static RectangleType ctlBounds;
static RectangleType screenBounds;
static RectangleType bufferBounds;
static RectangleType imageBounds;
static RectangleType frameBounds;
static WinHandle     screenWindow;
static WinHandle     bufferWindow;
static ImageType*    image;
static Coord         moveFromX = 0;
static Coord         moveFromY = 0;
static Int16         anchor = NOT_FOUND;
static Int16         tableX;
static Int16         tableY;
static Boolean       penDownInFrame;
static Boolean       fullscreenformActive = false;
static RectangleType fullBounds;
static Boolean       fastEnough;    /* SuperVZ or better */
static Coord         originalBackButtonHeight = 0;
static Coord         originalBackButtonWidth = 0;


/***********************************************************************
 *
 *      Private functions
 *
 ***********************************************************************/
static Boolean FsFormInit( void ) FULLSCREENFORM_SECTION;
static void MoveImage( Coord x, Coord y ) FULLSCREENFORM_SECTION;
static void DrawImage( ImageType* image, Coord drawX, Coord drawY, 
                RectangleType* bounds ) FULLSCREENFORM_SECTION;
static void DrawScanner( void ) FULLSCREENFORM_SECTION;
static void DrawButton( void ) FULLSCREENFORM_SECTION;
static void FreeMemory( void ) FULLSCREENFORM_SECTION;
static void FsGoBackToPreviousPage(void) FULLSCREENFORM_SECTION;
static Boolean HandleFsControl(const EventType* event) FULLSCREENFORM_SECTION;
static Boolean HandleFsKeyDown(const EventType* event) FULLSCREENFORM_SECTION;
static Boolean HandleFsPenUp( const Coord x, const Coord y) FULLSCREENFORM_SECTION;
static Boolean HandleFsPenMove( const Coord x, const Coord y) FULLSCREENFORM_SECTION;
static Boolean HandleFsPenDown( const Coord x, const Coord y) FULLSCREENFORM_SECTION;
static Boolean FsHandleMenuEvent ( UInt16 itemID ) FULLSCREENFORM_SECTION;


/* Initialize the image form */
static Boolean FsFormInit( void )
{
    MemHandle       imageHandle;
    Err             err;
    Coord           maxFrameSize;
    UInt32          processorType;
    UInt16          backButtonIndex;
    RectangleType   newBounds;

    err = FtrGet( sysFileCSystem, sysFtrNumProcessorID, &processorType );
    if ( err == errNone && 0x00040000 <= processorType )
        fastEnough = true;
    else
        fastEnough = false;

    if ( IsLargeTable() )
        imageHandle = GetFullscreenTableHandle();
    else
        imageHandle = GetFullscreenImageHandle();
    image = LockImage( imageHandle );
    if ( image == NULL || image->err != errNone )
        return false;

    fullscreenformActive = true;

    fsForm = FrmGetFormPtr( GetValidForm( frmFullscreen ) );

    backButtonIndex = FrmGetObjectIndex( fsForm, frmFullscreenBack );

    FrmHideObject( fsForm, backButtonIndex );

    FrmDrawForm( fsForm );

    screenWindow = WinGetDrawWindow();

    screenBounds.topLeft.x = 0;
    screenBounds.topLeft.y = 0;
    screenBounds.extent.x  = RotMaxExtentX();
    screenBounds.extent.y  = RotMaxExtentY();
    newBounds              = screenBounds;
    if ( fastEnough )
        RotateRectangleInPlace( &newBounds );
    WinSetBounds( screenWindow, &newBounds );
    HiResAdjustBounds( &screenBounds, palmHiRes );

    bufferWindow = NULL;
    if ( Support35() ) { /* 3.0 (at least) mis-behaves terribly */
        bufferWindow = WinCreateOffscreenWindow( screenBounds.extent.x,
            screenBounds.extent.y, nativeFormat, &err );
        bufferBounds = screenBounds;
        if ( err != errNone ) {
            MSG( _( "Couldn't create offscreen window: 0x%x\n", err ) );
            bufferWindow = NULL;
        }
    }

    imageBounds.topLeft.x = 0;
    imageBounds.topLeft.y = 0;
    imageBounds.extent.x = image->width;
    imageBounds.extent.y = image->height;

    /* Establish the dimensions of our scanner frame. Double its size if we're
       on a non-OS5 sony device. */
    maxFrameSize = MAX_FRAME_SIZE;
    HiResAdjust( &maxFrameSize, sonyHiRes | handeraHiRes );
    if ( imageBounds.extent.x < imageBounds.extent.y ) {
        frameBounds.extent.y = maxFrameSize;
        frameBounds.extent.x = (UInt32) imageBounds.extent.x *
            (UInt32) frameBounds.extent.y / (UInt32) imageBounds.extent.y;
    }
    else {
        frameBounds.extent.x = maxFrameSize;
        frameBounds.extent.y = (UInt32) imageBounds.extent.y *
            (UInt32) frameBounds.extent.x / (UInt32) imageBounds.extent.x;
    }
    frameBounds.topLeft.x = RotMaxExtentX() - frameBounds.extent.x - 1;
    frameBounds.topLeft.y = RotMaxExtentY() - frameBounds.extent.y - 1;

    /* adjust position of back button for displays taller than 160 */
    if ( originalBackButtonHeight == 0 ) {
        FrmGetObjectBounds( fsForm, backButtonIndex, &ctlBounds );
        HiResAdjustBounds( &ctlBounds, sonyHiRes );
        originalBackButtonWidth  = ctlBounds.extent.x;
        originalBackButtonHeight = ctlBounds.extent.y;
    }
    ctlBounds.topLeft.x = 1;
    ctlBounds.topLeft.y = RotMaxExtentY() - originalBackButtonHeight - 1;
    ctlBounds.extent.x  = originalBackButtonWidth;
    ctlBounds.extent.y  = originalBackButtonHeight;
    
    if ( fastEnough ) {
        ControlType* backButton;

        backButton = GetObjectPtr( frmFullscreenBack );
        CtlGlueSetFont( backButton, symbolFont );
        switch ( Prefs()->rotate ) {
            case ROTATE_PLUS90:
                CtlSetLabel( backButton, "\006" );
                break;

            case ROTATE_MINUS90:
                CtlSetLabel( backButton, "\005" );
                break;

            case ROTATE_ZERO:
            default:
                CtlSetLabel( backButton, "\003" );
                break;
        }
        RotateRectangleInPlace( &ctlBounds );
    }
    if ( IsHiResTypeSony( HiResType() ) ) {
        ctlBounds.topLeft.x /= 2;
        ctlBounds.topLeft.y /= 2;
        ctlBounds.extent.x  /= 2;
        ctlBounds.extent.y  /= 2;
    }
    FrmSetObjectBounds( fsForm, backButtonIndex, &ctlBounds );

    FrmShowObject( fsForm, backButtonIndex );

    ctlBounds.topLeft.x--;
    ctlBounds.topLeft.y--;
    ctlBounds.extent.x += 2;
    ctlBounds.extent.y += 2;
    HiResAdjustBounds( &ctlBounds, sonyHiRes );

    /* Pre-lock any subimages */
    if ( image->type == MULTIIMAGE ) {
        MultiImageNodeType* node;

        node = ListFirst( image->data.multiList );
        while ( node != NULL ) {
            node->image = LockImage( node->imageHandle );
            node = ListNext( image->data.multiList, node );
        }
    }

    fullBounds.topLeft.x = 0;
    fullBounds.topLeft.y = 0;
    fullBounds.extent.x = image->width;
    fullBounds.extent.y = image->height;

    /* Screen locking isn't necessary since this is the initial
       display of the image */
    DrawImage( image, 0, 0, &fullBounds );

    if ( IsLargeTable() )
        CopyTableAnchors( image->reference, 0, 0 );

    return true;
}



/* Move image up/down replaces AdjustVerticalOffset() in document.c */
void FsAdjustVerticalOffset
    (
    Int16   adjustment  /* adjustment in pixels */
    )
{
    Coord x;
    Coord y;

    x = screenBounds.topLeft.x;
    y = screenBounds.topLeft.y - adjustment;

    y = min( imageBounds.extent.y - screenBounds.extent.y, y );
    y = max( 0, y );

    imageBounds.topLeft.x = screenBounds.topLeft.x = x;
    imageBounds.topLeft.y = screenBounds.topLeft.y = y;

    DrawImage( image, -x, -y, &fullBounds );

    if ( IsLargeTable() )
        CopyTableAnchors( image->reference, x, y );
}



/* Move image up/down/left/right */
static void MoveImage
    (
    Coord x,
    Coord y
    )
{
    screenBounds.topLeft.x = x;
    screenBounds.topLeft.y = y;

    if ( bufferWindow != NULL )
        screenWindow = WinSetDrawWindow( bufferWindow );

    DrawImage( image, -x, -y, &fullBounds );

    if ( bufferWindow != NULL ) {
        WinSetDrawWindow( screenWindow );
        WinCopyRectangle( bufferWindow, screenWindow, &bufferBounds, 0, 0,
            winPaint );
    }
    DrawButton();
    DrawScanner();
}



static void DrawImage
    (
    ImageType*    thisImage,
    Coord         drawX,
    Coord         drawY,
    RectangleType* bounds
    )
{
    if ( thisImage->type == MULTIIMAGE ) {
        MultiImageNodeType* node;

        node = ListFirst( thisImage->data.multiList );
        while ( node != NULL ) {
            if ( node->image->type == WINDOW_HANDLE ) {
                RectangleType fakeBounds;

                fakeBounds.topLeft.x = 0;
                fakeBounds.topLeft.y = 0;
                fakeBounds.extent.x = node->position.extent.x;
                fakeBounds.extent.y = node->position.extent.y;
                WinCopyRectangle( node->image->data.window, screenWindow,
                                  &fakeBounds,
                                  drawX + node->position.topLeft.x,
                                  drawY + node->position.topLeft.y, winPaint );
            } else {
                DrawImage( node->image, drawX + node->position.topLeft.x,
                           drawY + node->position.topLeft.y, &node->position );
            }
            node = ListNext( thisImage->data.multiList, node );
        }
    } else {
        UInt16        prevCoordSys;
        RectangleType whitespace;

        prevCoordSys = PalmSetCoordinateSystem( NATIVE );
        if ( thisImage->type == WINDOW_HANDLE )
            WinCopyRectangle( thisImage->data.window, screenWindow,
                              &screenBounds, 0, 0, winPaint );
        else {
            WinHandle       screenWindow;
            WinHandle       drawWindow;
            Err             error;
            BitmapType*     bitmap;
            RectangleType   useMe;
            Coord           offsetX;
            Coord           offsetY;

            RctGetIntersection( bounds, &screenBounds, &useMe);
            if ( 0 < useMe.extent.x && 0 < useMe.extent.y ) {
                if ( ! Support35() ) {
                    WinDrawBitmap( thisImage->bitmap, drawX, drawY );
                }
                else {
                    /* crop the bitmap so that the rotation code only
                       rotates the visible part */
                    offsetX = useMe.topLeft.x - screenBounds.topLeft.x;
                    offsetY = useMe.topLeft.y - screenBounds.topLeft.y;
                    drawWindow = WinCreateOffscreenWindow( useMe.extent.x,
                                     useMe.extent.y, screenFormat, &error );
                    if ( error == errNone ) {
                        screenWindow = WinSetDrawWindow( drawWindow );
                        WinDrawBitmap( thisImage->bitmap, drawX - offsetX,
                                       drawY - offsetY );
                        bitmap = WinGetBitmap( drawWindow );
                        WinSetDrawWindow( screenWindow );
                        if ( fastEnough )
                            RotDrawBitmap( bitmap, offsetX, offsetY );
                        else
                            GeneralWinDrawBitmap( bitmap, offsetX, offsetY );
                        WinDeleteWindow( drawWindow, false );
                    }
                    else {
                        if ( fastEnough )
                            RotDrawBitmap( thisImage->bitmap, drawX, drawY );
                        else
                            GeneralWinDrawBitmap( thisImage->bitmap, drawX, drawY );
                    }
                }
            }
        }
        /* Specifically erase any whitespace beyond the image's bounds */
        if ( imageBounds.extent.x < screenBounds.extent.x ) {
            whitespace.topLeft.y = 0;
            whitespace.topLeft.x = imageBounds.extent.x;
            whitespace.extent.y  = screenBounds.extent.y;
            whitespace.extent.x  = screenBounds.extent.x - imageBounds.extent.x;

            if ( fastEnough )
                RotEraseRectangle( &whitespace, 0 );
            else
                WinEraseRectangle( &whitespace, 0 );
        }
        if ( imageBounds.extent.y < screenBounds.extent.y ) {
            whitespace.topLeft.x = 0;
            whitespace.topLeft.y = imageBounds.extent.y;
            whitespace.extent.x  = screenBounds.extent.x;
            whitespace.extent.y  = screenBounds.extent.y - imageBounds.extent.y;

            if ( fastEnough )
                RotEraseRectangle( &whitespace, 0 );
            else
                WinEraseRectangle( &whitespace, 0 );
        }
        PalmSetCoordinateSystem( prevCoordSys );
    }
}



/* Draw the image pan 'n scanner */
static void DrawScanner( void )
{
    RectangleType viewAreaBounds;

    if ( imageBounds.extent.x <= screenBounds.extent.x &&
         imageBounds.extent.y <= screenBounds.extent.y )
        return;

    viewAreaBounds.topLeft.x = (UInt32) screenBounds.topLeft.x *
                               (UInt32) frameBounds.extent.x /
                               (UInt32) imageBounds.extent.x +
                               (UInt32) frameBounds.topLeft.x;
    viewAreaBounds.topLeft.y = (UInt32) screenBounds.topLeft.y *
                               (UInt32) frameBounds.extent.y /
                               (UInt32) imageBounds.extent.y +
                               (UInt32) frameBounds.topLeft.y;
    viewAreaBounds.extent.x  = (UInt32) screenBounds.extent.x *
                               (UInt32) frameBounds.extent.x /
                               (UInt32) imageBounds.extent.x;
    viewAreaBounds.extent.y  = (UInt32) screenBounds.extent.y *
                               (UInt32) frameBounds.extent.y /
                               (UInt32) imageBounds.extent.y;

    if ( fastEnough ) {
        RotDrawGrayRectangleFrame( simpleFrame, &frameBounds );
        RotDrawGrayRectangleFrame( simpleFrame, &viewAreaBounds );
    }
    else {
        WinDrawGrayRectangleFrame( simpleFrame, &frameBounds );
        WinDrawGrayRectangleFrame( simpleFrame, &viewAreaBounds );
    }
}



/* Draw the button(s) */
static void DrawButton( void )
{
    CtlDrawControl( GetObjectPtr( frmFullscreenBack ) );
}



Boolean FsDoSelectTypeAction
    (
    SelectType selection
    )
{
    Boolean handled;

    handled = false;

    switch ( selection ) {
        case SELECT_GO_FORWARD:
            /* Currently you cannot go forward to anything from here */
            handled = true;
            break;

        case SELECT_GO_BACK:
        case SELECT_GO_HOME:
            FreeMemory();
            FrmReturnToForm( PREVIOUS_FORM );
            /* Not handled because we do want to run their normal functions
               from within DoSelectTypeAction() */
            handled = false;
            break;

        case SELECT_GO_TO_TOP:
            FsAdjustVerticalOffset( FULLSCREEN_TOP );
            handled = true;
            break;

        case SELECT_GO_TO_BOTTOM:
            FsAdjustVerticalOffset( FULLSCREEN_BOTTOM );
            handled = true;
            break;

        case SELECT_GO_TO_LINK:
            /* Not implemented yet */
            handled = true;
            break;

        default:
            break;
    }

    return handled;
}



/* Wrapper for FrmGotoForm() when called from frmFullscreen */
void FsFrmGotoForm
    (
    UInt16 form
    )
{
    FreeMemory();
    FrmReturnToForm( Prefs()->lastForm );
    FrmGotoForm( form );
}



void FsDoControlAction
    (
    Int16 control /* control value of the object */
    )
{
    FreeMemory();
    FrmReturnToForm( PREVIOUS_FORM );
    DoControlAction( control );
}



static void FsGoBackToPreviousPage(void)
{
    FsDoControlAction( -( LEFTCONTROL + 1 ) );
}



static Boolean HandleFsControl(const EventType* event)
{
    if ( event->data.ctlEnter.controlID == frmFullscreenBack) {
        FsGoBackToPreviousPage();
        return true;
    }
    return false;
}



/* Handle a menu event */
static Boolean FsHandleMenuEvent
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
         /* Cannot go forward from here yet */
         /* case mGoForward: */
                FsDoControlAction( mGoHome - itemID - 1 );
                handled = true;
                break;

            case mGoTop:
                FsAdjustVerticalOffset( FULLSCREEN_TOP );
                handled = true;
                break;

            case mGoBottom:
                FsAdjustVerticalOffset( FULLSCREEN_BOTTOM );
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

            case mGoLibrary:
                FreeMemory();
                FrmReturnToForm( PREVIOUS_FORM );
                FrmGotoForm( GetValidForm( frmLibrary ) );
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



static Boolean HandleFsKeyDown(const EventType* event)
{
    Boolean handled = false;

    if ( IsJogdialBack( event->data.keyDown.chr ) ) {
        FsGoBackToPreviousPage();
        return true;
    }

    switch ( event->data.keyDown.chr ) {
        case pageUpChr:
            if ( Prefs()->arrowMode[ UP_ARROW ] != SELECT_NONE &&
                 Prefs()->arrowKeys )
                DoSelectTypeAction( Prefs()->arrowMode[ UP_ARROW ] );
            else
                DoPageMove( RotGetScrollValue() );
            handled = true;
            break;

        case pageDownChr:
            if ( Prefs()->arrowMode[ DOWN_ARROW ] != SELECT_NONE &&
                 Prefs()->arrowKeys )
                DoSelectTypeAction( Prefs()->arrowMode[ DOWN_ARROW ] );
            else
                DoPageMove( -RotGetScrollValue() );
            handled = true;
            break;

#ifdef HAVE_FIVEWAY_SDK
        case vchrNavChange:
            if ( NavKeyPressed( event, Left ) &&
                 Prefs()->arrowMode[ LEFT_ARROW ] != SELECT_NONE &&
                 Prefs()->arrowKeys ) {
                DoSelectTypeAction( Prefs()->arrowMode[ LEFT_ARROW ] );
                handled = true;
            }
            else if ( NavKeyPressed( event, Right ) &&
                 Prefs()->arrowMode[ RIGHT_ARROW ] != SELECT_NONE &&
                 Prefs()->arrowKeys ) {
                DoSelectTypeAction( Prefs()->arrowMode[ RIGHT_ARROW ] );
                handled = true;
            }
            else if ( NavKeyPressed( event, Select ) &&
                 Prefs()->arrowMode[ SELECT_ARROW ] != SELECT_NONE &&
                 Prefs()->arrowKeys ) {
                DoSelectTypeAction( Prefs()->arrowMode[ SELECT_ARROW ]);
                handled = true;
            }
            break;
#endif

        case spaceChr:
            if ( Prefs()->gestMode[ GESTURES_RIGHT ] != SELECT_NONE &&
                 Prefs()->gestures )
                DoSelectTypeAction( Prefs()->gestMode[ GESTURES_RIGHT ] );
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
                DoSelectTypeAction( Prefs()->gestMode[ GESTURES_DOWN ] );
            handled = true;
            break;

        case '.':
            /* The same argument can be given for '.' except that this
               is only really here to benefit Graffiti 2 users. Normal
               Graffiti users won't notice any difference */
            if ( Prefs()->gestMode[ GESTURES_TAP ] != SELECT_NONE )
                DoSelectTypeAction( Prefs()->gestMode[ GESTURES_TAP ] );
            handled = true;
            break;

        case backspaceChr:
            if ( Prefs()->gestMode[ GESTURES_LEFT ] != SELECT_NONE &&
                 Prefs()->gestures )
                DoSelectTypeAction( Prefs()->gestMode[ GESTURES_LEFT ] );
            handled = true;
            break;

        default:
            break;
    }

    if ( ! handled ) {
        /* handle plucker specific key events */
        if ( vchrPluckerMin <= event->data.keyDown.chr &&
              event->data.keyDown.chr <= vchrPluckerMax ) {
            DoControlAction( -( event->data.keyDown.chr -
                vchrPluckerMin + 1 ) );
            handled = true;
        }
    }

    return handled;
}



static Boolean HandleFsPenUp
    (
    const Coord x,
    const Coord y
    )
{
    if ( IsLargeTable() && anchor != NOT_FOUND ) {
        HighlightAnchor( anchor, ANCHOR_UNSELECTED );
        if ( ( Abs(x - tableX) < 4 ) && ( Abs(y - tableY ) < 4 ) ) {
            FsDoControlAction( anchor + 1 );
            return true;
        }
    }

    anchor = NOT_FOUND;

    DrawImage( image, -imageBounds.topLeft.x, -imageBounds.topLeft.y,
               &fullBounds );

    if ( IsLargeTable() )
        CopyTableAnchors( image->reference, imageBounds.topLeft.x,
                          imageBounds.topLeft.y );

    penDownInFrame = false;

    return true;
}



static Boolean HandleFsPenMove
    (
    const Coord x,
    const Coord y
    )
{
    static RectangleType lastImageBounds = { { 0, 0 }, { 0, 0 } };
    Coord                penDownX;
    Coord                penDownY;

    if ( penDownInFrame ) {
        PointType viewFrameCentre;

        /* Find the middle of the viewFrame so we can centre the user's pen
           event within the reference area. */
        viewFrameCentre.x = (UInt32) screenBounds.extent.x *
                            (UInt32) frameBounds.extent.x /
                            (UInt32) imageBounds.extent.x / 2;
        viewFrameCentre.y = (UInt32) screenBounds.extent.y *
                            (UInt32) frameBounds.extent.y /
                            (UInt32) imageBounds.extent.y / 2;

        /* The max() is necessary here to prevent the frame from focusing on
           and area that precedes the original image. If the user pens-down
           within the scanner frame but drags that pen beyond the top or left
           scope of that frame (a negative coordinate) then just use zero */
        penDownX = (UInt32) max( 0, x - frameBounds.topLeft.x -
                                        viewFrameCentre.x ) *
                   (UInt32) imageBounds.extent.x /
                   (UInt32) frameBounds.extent.x;
        penDownY = (UInt32) max( 0, y - frameBounds.topLeft.y -
                                        viewFrameCentre.y ) *
                   (UInt32) imageBounds.extent.y /
                   (UInt32) frameBounds.extent.y;

        imageBounds.topLeft.x = penDownX;
        imageBounds.topLeft.y = penDownY;
    }
    else {
        penDownX = x;
        penDownY = y;

        imageBounds.topLeft.x += 2 * ( moveFromX - penDownX );
        imageBounds.topLeft.y += 2 * ( moveFromY - penDownY );
    }

    /* Limit the image's topLeft bounds to be no higher than the image extent
       minus the screen extent, and no lower than 0 */
    imageBounds.topLeft.x = min( imageBounds.extent.x - screenBounds.extent.x,
                                 imageBounds.topLeft.x );
    imageBounds.topLeft.x = max( 0, imageBounds.topLeft.x );

    imageBounds.topLeft.y = min( imageBounds.extent.y - screenBounds.extent.y,
                                 imageBounds.topLeft.y );
    imageBounds.topLeft.y = max( 0, imageBounds.topLeft.y );

    moveFromX = penDownX;
    moveFromY = penDownY;

    /* if the newly calculated imageBounds match the previous imageBounds,
       don't bother updating the screen. Otherwise the image/arrows will start
       to flicker just by holding down the pen */
    if ( lastImageBounds.topLeft.x == imageBounds.topLeft.x &&
         lastImageBounds.topLeft.y == imageBounds.topLeft.y )
        return false;

    lastImageBounds = imageBounds;

    MoveImage( imageBounds.topLeft.x, imageBounds.topLeft.y );

    return true;
}



static Boolean HandleFsPenDown
    (
    const Coord x,
    const Coord y
    )
{
    /* Since we don't want to move the image if we're clicking on
       any of the control buttons, we'll break early */
    RectangleType rotXY = { { x, y }, { 0, 0 } };

    if ( fastEnough )
        RotateRectangleInPlace( &rotXY );
    if ( RctPtInRectangle( rotXY.topLeft.x, rotXY.topLeft.y, &ctlBounds ) )
            return false;

    penDownInFrame = RctPtInRectangle( x, y, &frameBounds );

    if ( IsLargeTable() ) {
        anchor = AnchorIndex( x, y );
        if ( anchor != NOT_FOUND ) {
            HighlightAnchor( anchor, ANCHOR_SELECTED );
            tableX = x;
            tableY = y;
            penDownInFrame = false;
        }
    }

    moveFromX = x;
    moveFromY = y;

    /* If we initially penDown'd within frameBounds, move the image immediatly
       but only if the viewing location the user wants to see has changed.
       (Ie, HandleFsPenMove() true only if the pen has *actually* moved) */
    if ( penDownInFrame && HandleFsPenMove( x, y ) )
        return true;

    DrawButton();
    DrawScanner();

    return true;
}



/* Event handler for the image form */
Boolean FsFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    static Boolean menuActive = false;
    Boolean handled;
    Coord   penX;
    Coord   penY;

    handled = false;

    penX = event->screenX;
    penY = event->screenY;
    HiResAdjust( &penX, sonyHiRes );
    HiResAdjust( &penY, sonyHiRes );
    if ( fastEnough )
        RotFromScreenXY( &penX, &penY );

    switch ( event->eType ) {
        case ctlSelectEvent:
            handled = HandleFsControl(event);
            break;

        case keyDownEvent:
            handled = HandleFsKeyDown( event );
            break;

        case penDownEvent:
            handled = HandleFsPenDown( penX, penY );
            break;

        case penMoveEvent:
            handled = HandleFsPenMove( penX, penY );
            break;

        case penUpEvent:
            /* Apparently the OS sends a penUp when changing Menus! */
            if ( ! menuActive )
                handled = HandleFsPenUp( penX, penY );
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            if ( event->data.winEnter.enterWindow == (WinHandle) fsForm &&
                 event->data.winEnter.enterWindow ==
                                             (WinHandle) FrmGetFirstForm() )
                menuActive = false;
            break;

        case winExitEvent:
            if ( event->data.winExit.exitWindow == (WinHandle) fsForm )
                menuActive = true;
            handled = ResizeHandleWinExitEvent();
            break;

        case frmUpdateEvent:
            FreeMemory();
            UpdateDisplayExtent();
            menuActive = false;
            if ( ! FsFormInit() ) {
                /* Do something */
            }
            handled = true;
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            menuActive = false;
            if ( ! FsFormInit() ) {
                /* Do something */
            }
            handled = true;
            break;

        case frmCloseEvent:
            FreeMemory();
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        case menuEvent:
            /* Refresh display to get rid of command text in bottom left
               corner */
            MenuEraseStatus( NULL );
            ErrTry {
                handled = FsHandleMenuEvent( event->data.menu.itemID );
            }
            ErrCatch( UNUSED_PARAM( err ) ) {
                handled = false;
            } ErrEndCatch
            break;

        default:
            handled = false;
    }

    return handled;
}



static void FreeMemory( void )
{
    if ( bufferWindow != NULL ) {
        WinDeleteWindow( bufferWindow, false );
        bufferWindow = NULL;
    }

    if ( image->type == MULTIIMAGE ) {
        MultiImageNodeType* node;

        node = ListFirst( image->data.multiList );
        while ( node != NULL ) {
            UnlockImage( node->image );
            node = ListNext( image->data.multiList, node );
        }
    }

    UnlockImage( image );
    image = NULL;
    fullscreenformActive = false;
}



/* Check if fullscreenform is active */
Boolean IsFullscreenformActive( void )
{
    return fullscreenformActive;
}

