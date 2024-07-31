/*
 * $Id: prefsgeneral.c,v 1.11 2004/04/18 15:34:48 prussar Exp $
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

#include "history.h"
#include "metadocument.h"
#include "os.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "DIA.h"
#include "util.h"

#include "prefsgeneral.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean        showFirst = false;
static MemHandle      depthHandle;
static UInt16         screenDepth;
static Char           depthList[ 20 ];
static ScrollbarType  scrollbar;
static UInt16         toolbarButton;
static MemHandle      toolbarHandle;
static ToolbarType    toolbar;
static Char           toolbarList[ 60 ];


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void LoadGeneralPrefs( void ) PREFSFORM_SECTION;



/* Handle the General preferences */
Boolean PrefsGeneralPreferenceEvent
    (
    ActionType action
    )
{
    Boolean handled;

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
            LoadGeneralPrefs();
            handled = true;
            break;

        case SAVE:
            Prefs()->toolbarButton = toolbarButton;

            if ( screenDepth != Prefs()->screenDepth ) {
                Prefs()->screenDepth = screenDepth;
                /* FIXME: aren't we supposed to do something here? */
            }
            if ( toolbar != Prefs()->toolbar ) {
                Prefs()->toolbar = toolbar;

                    /* fix incompatible settings */
                if ( toolbar != TOOLBAR_NONE &&
                     GetDIAState() == DIA_STATE_NO_STATUS_BAR )
                    SetDIAState( Prefs()->savedSilkscreen );
                    
                if ( IsFormMain( Prefs()->lastForm ) ) {
                    Prefs()->lastForm = GetMainFormId();
                }
            }
            if ( scrollbar != Prefs()->scrollbar ) {
                Prefs()->scrollbar = scrollbar;
                if ( IsFormMain( Prefs()->lastForm ) )
                    ReRenderAll();
            }
            handled = true;
            break;

        case RELEASE:
            if ( toolbarHandle != NULL ) {
                MemHandleUnlock( toolbarHandle );
                MemHandleFree( toolbarHandle );
                toolbarHandle = NULL;
            }
            if ( depthHandle != NULL ) {
                MemHandleUnlock( depthHandle );
                MemHandleFree( depthHandle );
                depthHandle = NULL;
            }
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Load the general section into memory */
static void LoadGeneralPrefs( void )
{
    ListType*       list;
    ControlType*    ctl;
    UInt16          entries;
    UInt16          selection;
    UInt16          i;
    Char            depth[ 3 ];
    Char*           temp;
    Char**          depths;
    UInt32          supportedDepths;
    Char**          toolbarOptions;


    toolbarButton = Prefs()->toolbarButton;
    SetListToSelection( frmPrefsGeneralMenuToolbarList,
        frmPrefsGeneralMenuToolbarPopup, toolbarButton );

    screenDepth = Prefs()->screenDepth;

    if ( Support30() )
        WinScreenMode( winScreenModeGetSupportedDepths, NULL, NULL,
            &supportedDepths, NULL );
    else
        supportedDepths = 1;

    MemSet( &depthList, sizeof( depthList ), 0 );
    entries     = 0;
    selection   = 0;
    temp        = depthList;
    for ( i = 1; supportedDepths; i++ ) {
        if ( ( supportedDepths & 0x01 ) == 0x01 ) {
            StrIToA( depth, i );
            StrCat( temp, depth );
            temp += StrLen( depth ) + 1;
            if ( screenDepth == i )
                selection = entries;
            entries++;
        }
        supportedDepths >>= 1;
    }

    list        = GetObjectPtr( frmPrefsGeneralScreenDepthList );
    ctl         = GetObjectPtr( frmPrefsGeneralScreenDepthPopup );
    depthHandle = SysFormPointerArrayToStrings(
        depthList, entries );
    depths      = MemHandleLock( depthHandle );

    LstSetListChoices( list, depths, entries );
    LstSetHeight( list, entries );
    LstSetSelection( list, selection );
    CtlSetLabel( ctl, LstGetSelectionText( list, selection ) );

    list      = GetObjectPtr( frmPrefsGeneralToolbarModeList );
    ctl       = GetObjectPtr( frmPrefsGeneralToolbarModePopup );
    toolbar   = Prefs()->toolbar;
    selection = toolbar;

    MemSet( &toolbarList, sizeof( toolbarList ), 0 );
    temp    = toolbarList;
    SysCopyStringResource( temp, strPrefsTopToolbar );
    temp   += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strPrefsBottomToolbar );
    temp   += StrLen( temp ) + 1;
    SysCopyStringResource( temp, strPrefsNoToolbar );
    temp   += StrLen( temp ) + 1;
    if ( GetDIAHardware() == DIA_HARDWARE_HANDERA ) {
        SysCopyStringResource( temp, strPrefsSilkToolbar );
        entries = 4;
    }
    else {
        entries = 3;
    }

    toolbarHandle = SysFormPointerArrayToStrings(
        toolbarList, entries );
    toolbarOptions = MemHandleLock( toolbarHandle );
    LstSetListChoices( list, toolbarOptions, entries );
    LstSetHeight( list, entries );

    LstSetSelection( list, selection );
    CtlSetLabel( ctl, LstGetSelectionText( list, selection ) );

    scrollbar = Prefs()->scrollbar;
    SetListToSelection( frmPrefsGeneralScrollbarList,
        frmPrefsGeneralScrollbarPopup, scrollbar );
}



/* Nominate the General section to be shown first when the prefsform loads */
void PrefsGeneralShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the General preferences */
Boolean PrefsGeneralPalmEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled     = false;

    switch ( event->eType ) {
        case popSelectEvent:
        {
            Int16       selection;

            selection = event->data.popSelect.selection;
            if ( selection != noListSelection ) {
                ControlType*    ctl;
                ListType*       list;
                Char*           label;
                UInt16          controlID;

                list        = event->data.popSelect.listP;
                controlID   = event->data.popSelect.controlID;
                ctl         = GetObjectPtr( controlID );
                label       = LstGetSelectionText( list, selection );

                CtlSetLabel( ctl, label );
                LstSetSelection( list, selection );

                switch ( controlID ) {
                    case frmPrefsGeneralScreenDepthPopup:
                        screenDepth = StrAToI( label );
                        handled = true;
                        break;

                    case frmPrefsGeneralScrollbarPopup:
                        scrollbar = (ScrollbarType) selection;
                        handled = true;
                        break;

                    case frmPrefsGeneralMenuToolbarPopup:
                        toolbarButton = selection;
                        handled = true;
                        break;

                    case frmPrefsGeneralToolbarModePopup:
                        toolbar = (ToolbarType) selection;
                        handled = true;
                        break;
                        
                    default:
                        break;
                }
            }
            break;
        }

        default:
            handled = false;
    }

    return handled;
}

