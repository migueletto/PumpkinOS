/*
 * $Id: prefsautoscroll.c,v 1.5 2003/07/05 20:46:37 prussar Exp $
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

#include "control.h"
#include "prefsdata.h"
#include "util.h"

#include "prefsautoscroll.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static UInt16 HandleSpinner( UInt16 direction, UInt16 variable,
                UInt16 minValue, UInt16 maxValue, UInt16 controlID,
                UInt16 increment, UInt16 fineTransition, UInt16 fineIncrement ) 
                PREFSFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean            showFirst;
static AutoscrollModeType autoscrollMode;
static AutoscrollDirType  autoscrollDir;
static UInt16             autoscrollInterval;
static UInt16             autoscrollJump;



/* Handle the Autoscroll preferences */
Boolean PrefsAutoscrollPreferenceEvent
    (
    ActionType action
    )
{
    Boolean         handled;
    ControlType*    ctl;
    static Char     jumpLabel[ maxStrIToALen ];
    static Char     intervalLabel[ maxStrIToALen ];

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
            ctl             = GetObjectPtr( frmPrefsAutoscrollJumpButton );
            autoscrollJump  = Prefs()->autoscrollJump;
            StrIToA( jumpLabel, autoscrollJump );
            CtlSetLabel( ctl, jumpLabel );

            autoscrollMode  = Prefs()->autoscrollMode; SetListToSelection(
                frmPrefsAutoscrollModeList, frmPrefsAutoscrollModePopup,
                autoscrollMode );

            autoscrollDir   = Prefs()->autoscrollDir;
            SetListToSelection( frmPrefsAutoscrollDirList,
                frmPrefsAutoscrollDirPopup, autoscrollDir ^ 1 );

            ctl             = GetObjectPtr( frmPrefsAutoscrollIntervalButton );
            autoscrollInterval = TICKS_TO_MILLISECONDS(
                Prefs()->autoscrollInterval );
            StrIToA( intervalLabel, autoscrollInterval );
            CtlSetLabel( ctl, intervalLabel );

            CtlSetValue( GetObjectPtr( frmPrefsAutoscrollStayOn ),
                Prefs()->autoscrollStayOn );
            handled = true;
            break;

        case SAVE:
            Prefs()->autoscrollJump = autoscrollJump;
            Prefs()->autoscrollMode = autoscrollMode;
            Prefs()->autoscrollDir  = autoscrollDir;
            /* pref in ticks, label in milliseconds */
            Prefs()->autoscrollInterval = MILLISECONDS_TO_TICKS(
                                              autoscrollInterval );
            Prefs()->autoscrollStayOn  = CtlGetValue(
                GetObjectPtr( frmPrefsAutoscrollStayOn ) );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Handles behaviour of spinners for the Autoscroll section */
static UInt16 HandleSpinner
    (
    UInt16 direction,   /* whether variable should increase or decrease */
    UInt16 variable,    /* variable to modify with the spinner input */
    UInt16 minValue,    /* minimum value allowed */
    UInt16 maxValue,    /* maximum value allowed */
    UInt16 controlID,   /* resourceID of the control to show the variable's
                           current value */
    UInt16 increment,   /* how much to increase by */
    UInt16 fineTransition, /* from 0 to this, use the fineIncrement instead */
    UInt16 fineIncrement   /* use this from 0 to fineTransition */
    )
{
    ControlType*    ctl;
    static Char     spinnerLabel[ maxStrIToALen ];

    ctl = GetObjectPtr( controlID );
    if ( direction == AUTOSCROLL_UP && ( variable + increment ) <= maxValue )
    {
        if ( variable < fineTransition )
            variable += fineIncrement;
        else
            variable += increment;
    }
    else if ( direction == AUTOSCROLL_DOWN && minValue < variable )
    {
        if ( variable <= fineTransition )
            variable -= fineIncrement;
        else
            variable -= increment;
    }    
    else
        SndPlaySystemSound( sndWarning );

    StrIToA( spinnerLabel, variable );
    CtlSetLabel( ctl, spinnerLabel );

    return variable;
}



/* Nominate the Autoscroll section to be shown first when the prefsform loads */
void PrefsAutoscrollShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Autoscroll preference */
Boolean PrefsAutoscrollPalmEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled     = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmPrefsAutoscrollStayOn:
                    if ( CtlGetValue( GetObjectPtr(
                             frmPrefsAutoscrollStayOn ) ) )
                        FrmAlert( warnStayOn );
                    handled = true;
                    break;

                default:
                    break;
            }
            break;

        case ctlRepeatEvent:
            switch ( event->data.ctlRepeat.controlID ) {
                /* leave unhandled so the buttons can repeat */
                case frmPrefsAutoscrollJumpUpButton:
                    autoscrollJump = HandleSpinner( AUTOSCROLL_UP,
                        autoscrollJump, AUTOSCROLL_MIN_JUMP_VALUE,
                        AUTOSCROLL_MAX_JUMP_VALUE,
                        frmPrefsAutoscrollJumpButton,
                        AUTOSCROLL_INCR_JUMP_VALUE, 0, 0 );
                    break;

                case frmPrefsAutoscrollJumpDownButton:
                    autoscrollJump = HandleSpinner( AUTOSCROLL_DOWN,
                        autoscrollJump, AUTOSCROLL_MIN_JUMP_VALUE,
                        AUTOSCROLL_MAX_JUMP_VALUE,
                        frmPrefsAutoscrollJumpButton,
                        AUTOSCROLL_INCR_JUMP_VALUE, 0, 0 );
                    break;

                case frmPrefsAutoscrollIntervalUpButton:
                    autoscrollInterval = HandleSpinner( AUTOSCROLL_UP,
                        autoscrollInterval, AUTOSCROLL_MIN_INTERVAL_VALUE,
                        AUTOSCROLL_MAX_INTERVAL_VALUE,
                        frmPrefsAutoscrollIntervalButton,
                        AUTOSCROLL_INCR_INTERVAL_VALUE,
                        AUTOSCROLL_FINE_TRANSITION,
                        AUTOSCROLL_FINE_INCR_INTERVAL_VALUE );
                    break;

                case frmPrefsAutoscrollIntervalDownButton:
                    autoscrollInterval = HandleSpinner( AUTOSCROLL_DOWN,
                        autoscrollInterval, AUTOSCROLL_MIN_INTERVAL_VALUE,
                        AUTOSCROLL_MAX_INTERVAL_VALUE,
                        frmPrefsAutoscrollIntervalButton,
                        AUTOSCROLL_INCR_INTERVAL_VALUE,
                        AUTOSCROLL_FINE_TRANSITION,
                        AUTOSCROLL_FINE_INCR_INTERVAL_VALUE );
                    break;

                default:
                    break;
            }
            break;

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
                    case frmPrefsAutoscrollModePopup:
                        autoscrollMode = (AutoscrollModeType) selection;
                        handled = true;
                        break;

                    case frmPrefsAutoscrollDirPopup:
                        autoscrollDir = (AutoscrollDirType) selection ^ 1;
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

