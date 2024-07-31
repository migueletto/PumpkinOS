/*
 * $Id: prefsgesture.c,v 1.3 2003/02/27 12:19:24 nordstrom Exp $
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

#include "prefsdata.h"
#include "util.h"

#include "prefsgesture.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean     showFirst = false;
static SelectType  gestureMode[ NUM_OF_GESTURES ];
static Gestures    gestureSelected;



/* Handle the Gesture preferences */
Boolean PrefsGesturePreferenceEvent
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
            gestureSelected = GESTURES_UP;
            MemMove( gestureMode, Prefs()->gestMode, sizeof( gestureMode ) );
            InitializeActionList( frmPrefsGestureActionList );
            CtlSetValue( GetObjectPtr( frmPrefsGestureGestures ),
                Prefs()->gestures );
            SetListToSelection( frmPrefsGestureActionList,
                frmPrefsGestureSelectAction, gestureMode[ gestureSelected ] );
            handled = true;
            break;

        case DISPLAY:
            AffirmControlImage( gestureSelected, frmPrefsGestureUp,
                bmpGestureUp );
            handled = true;
            break;

        case SAVE:
            Prefs()->gestures = CtlGetValue( GetObjectPtr(
                frmPrefsGestureGestures ) );
            MemMove( Prefs()->gestMode, gestureMode, sizeof( gestureMode ) );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Nominate the Gesture section to be shown first when the prefsform loads */
void PrefsGestureShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Gesture preference */
Boolean PrefsGesturePalmEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled     = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmPrefsGestureUp:
                case frmPrefsGestureRight:
                case frmPrefsGestureDown:
                case frmPrefsGestureLeft:
                case frmPrefsGestureTap:
                    gestureSelected = event->data.ctlEnter.controlID -
                        frmPrefsGestureUp;
                    SetListToSelection( frmPrefsGestureActionList,
                        frmPrefsGestureSelectAction,
                        gestureMode[ gestureSelected ] );
                    handled = true;
                    break;

                case frmPrefsGestureSelectAction:
                    gestureMode[ gestureSelected ] =
                        SetActionForSelectedButton( frmPrefsGestureActionList,
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

