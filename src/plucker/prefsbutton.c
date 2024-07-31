/*
 * $Id: prefsbutton.c,v 1.9 2003/10/07 20:57:15 nordstrom Exp $
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

#include "os.h"
#include "debug.h"
#include "font.h"
#include "hires.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "util.h"

#include "prefsbutton.h"


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef enum {
    NORMAL = 0,
    FIVEWAY
} ButtonLayoutType;


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define STANDARD_ICON    1000
#define HANDERA_ICON     2000
#define UP_CHAR          0x01
#define DOWN_CHAR        0x02
#define LEFT_CHAR        0x02
#define RIGHT_CHAR       0x03
#define SELECT_CHAR      0x13
#define UP_CHAR_FONT     symbol7Font
#define DOWN_CHAR_FONT   symbol7Font
#define LEFT_CHAR_FONT   symbol11Font
#define RIGHT_CHAR_FONT  symbol11Font
#define SELECT_CHAR_FONT symbolFont


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean            showFirst = false;
static SelectType         buttonMode[ NUM_OF_HW_BUTTONS+NUM_OF_ARROW_BUTTONS ];
static Buttons            selected;
static UInt16             objectList[][ MAX_OBJECTS ] = {
    { frmPrefsButtonDatebook, frmPrefsButtonAddress, frmPrefsButtonUp,
      frmPrefsButtonDown, frmPrefsButtonTodo, frmPrefsButtonMemo, EOL },
    { frmPrefsButtonDatebook, frmPrefsButtonAddress, frmPrefsButtonUp,
      frmPrefsButtonDown, frmPrefsButtonLeft, frmPrefsButtonRight,
      frmPrefsButtonSelect, frmPrefsButtonTodo, frmPrefsButtonMemo, EOL },
};


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void DrawButtonIcon( UInt32 creator,
                UInt16 objectID ) PREFSFORM_SECTION;



/* Handle the Button preferences */
Boolean PrefsButtonPreferenceEvent
    (
    ActionType action
    )
{
    Boolean                 handled;
    static ButtonLayoutType layout;

    handled = false;

    switch ( action ) {
        case AVAILABLE:
            /* Always available */
            handled = true;
            break;

        case SHOWFIRST:
            handled = showFirst;
            showFirst = false;
            break;

        case LOAD:
        {
            if ( HaveFiveWay() )
                layout = FIVEWAY;
            else
                layout = NORMAL;

            selected = DATEBOOK_BUTTON;

            InitializeActionList(frmPrefsButtonActionList);

            CtlSetValue( GetObjectPtr( frmPrefsButtonHardKeys ),
                Prefs()->hardKeys );
            CtlSetValue( GetObjectPtr( frmPrefsButtonArrowKeys ),
                Prefs()->arrowKeys );

            /* To save space, we've included both the definitions for the
               normal and the fiveway button layouts within a 'normal' layout.
               If we need to draw the fiveway layout, we need to move the
               normal layout buttons around a bit to make way. */
            if ( layout == FIVEWAY ) {
                FormType* form;

                form = FrmGetActiveForm();
                MoveObjectRelatively( form, frmPrefsButtonDatebook, -7, 0 );
                MoveObjectRelatively( form, frmPrefsButtonAddress, -11, 0 );
                MoveObjectRelatively( form, frmPrefsButtonTodo, 11, 0 );
                MoveObjectRelatively( form, frmPrefsButtonMemo, 7, 0 );
                MoveObjectRelatively( form, frmPrefsButtonUp, 0, -7 );
                MoveObjectRelatively( form, frmPrefsButtonDown, 0, 7 );
            }

            MemMove( buttonMode, Prefs()->hwMode, NUM_OF_HW_BUTTONS );
            MemMove( buttonMode + NUM_OF_HW_BUTTONS,
                Prefs()->arrowMode, NUM_OF_ARROW_BUTTONS );
            handled = true;
            break;
        }

        case DISPLAY:
            PrefsShowSection( objectList, layout );

            /* We would have just used AffirmControlImage() for this, but
               instead of simply displaying a bitmap stored as a resource,
               we have to run DrawButtonIcon() several times */
            CtlSetValue( GetObjectPtr( selected + frmPrefsButtonDatebook ),
                false );

            DrawButtonIcon( sysFileCDefaultButton1App, frmPrefsButtonDatebook);
            DrawButtonIcon( sysFileCDefaultButton2App, frmPrefsButtonAddress );
            DrawButtonIcon( sysFileCDefaultButton3App, frmPrefsButtonTodo );
            DrawButtonIcon( sysFileCDefaultButton4App, frmPrefsButtonMemo );

            DisplayChar( UP_CHAR_FONT, UP_CHAR, FrmGetActiveForm(), frmPrefsButtonUp );
            DisplayChar( DOWN_CHAR_FONT, DOWN_CHAR, FrmGetActiveForm(),
                frmPrefsButtonDown );
            if ( layout == FIVEWAY ) {
                DisplayChar( LEFT_CHAR_FONT, LEFT_CHAR, FrmGetActiveForm(),
                    frmPrefsButtonLeft );
                DisplayChar( SELECT_CHAR_FONT, SELECT_CHAR, FrmGetActiveForm(),
                    frmPrefsButtonSelect );
                DisplayChar( RIGHT_CHAR_FONT, RIGHT_CHAR, FrmGetActiveForm(),
                    frmPrefsButtonRight );
            }

            CtlSetValue( GetObjectPtr( selected + frmPrefsButtonDatebook ),
                true );

            SetListToSelection( frmPrefsButtonActionList,
                frmPrefsButtonSelectAction, buttonMode[ selected ] );
            handled = true;
            break;

        case CLEAR:
            PrefsHideSection( objectList, layout );
            handled = true;
            break;

        case SAVE:
            Prefs()->hardKeys = CtlGetValue( GetObjectPtr(
                frmPrefsButtonHardKeys ) );
            Prefs()->arrowKeys = CtlGetValue( GetObjectPtr(
                frmPrefsButtonArrowKeys ) );
            MemMove( Prefs()->hwMode, buttonMode, NUM_OF_HW_BUTTONS );
            MemMove( Prefs()->arrowMode, buttonMode + NUM_OF_HW_BUTTONS,
                NUM_OF_ARROW_BUTTONS );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Draw this creator's icon on top of this objectID's coordinates */
static void DrawButtonIcon
    (
    UInt32 creator,
    UInt16 objectID
    )
{
    DmOpenRef        db;
    DmResID          dmResID;
    MemHandle        bitmapH;
    BitmapType*      bitmap;
    Coord            iconWidth;
    Coord            iconHeight;
    Coord            adjustX;
    Coord            adjustY;
    RectangleType    bounds;

    db = DmOpenDatabaseByTypeCreator( sysFileTApplication, creator,
        dmModeReadOnly );

    /* Get the larger Handera icon if we're on such a device. If not,
       default to the standard icon */
    if ( IsHiResTypeHandera( HiResType() ) )
        dmResID = HANDERA_ICON;
    else
        dmResID = STANDARD_ICON;

    bitmapH = DmGetResource( iconType, dmResID );
    bitmap  = (BitmapType*) MemHandleLock( bitmapH );

    BmpGlueGetDimensions( bitmap, &iconWidth, &iconHeight, NULL );

    FrmGetObjectBounds( FrmGetActiveForm(),
        FrmGetObjectIndex( FrmGetActiveForm(), objectID ),
        &bounds );

    HiResAdjustBounds( &bounds, sonyHiRes );
    WinSetClip( &bounds );

    /* Centre the icon within the pushbuttons */
    adjustX = ( iconWidth / 2 ) - ( bounds.extent.x / 2 );
    adjustY = ( iconHeight / 2 ) - ( bounds.extent.y / 2 );
    if ( ! Support35() ) {
        /* For some strange reason, OS 3.1 crashes when the bitmap starts
           outside the clipping region */
        if ( 0 < adjustX )
            adjustX = 0;
        if ( 0 < adjustY )
            adjustY = 0;
    }

    /* draw the image to the icon */
    WinDrawBitmap( bitmap, bounds.topLeft.x - adjustX,
        bounds.topLeft.y - adjustY);
    WinResetClip();

    /* free the image from memory */
    MemPtrUnlock( bitmap );
    DmReleaseResource( bitmapH );
    DmCloseDatabase( db );
}



/* Nominate the Button section to be shown first when the prefsform loads */
void PrefsButtonShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Button preference */
Boolean PrefsButtonPalmEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled     = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmPrefsButtonDatebook:
                case frmPrefsButtonAddress:
                case frmPrefsButtonTodo:
                case frmPrefsButtonMemo:
                case frmPrefsButtonUp:
                case frmPrefsButtonDown:
                case frmPrefsButtonLeft:
                case frmPrefsButtonRight:
                case frmPrefsButtonSelect:
                    selected = event->data.ctlEnter.controlID -
                        frmPrefsButtonDatebook;
                    SetListToSelection( frmPrefsButtonActionList,
                        frmPrefsButtonSelectAction, buttonMode[ selected ] );
                    handled = true;
                    break;

                case frmPrefsButtonSelectAction:
                    buttonMode[ selected ] =
                        SetActionForSelectedButton( frmPrefsButtonActionList,
                            event->data.ctlEnter.controlID );
                    handled = true;
                    break;

                default:
                    break;
            }
            break;

        default:
            handled = false;
    }

    return handled;
}

