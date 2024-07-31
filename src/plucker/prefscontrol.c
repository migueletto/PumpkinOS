/*
 * $Id: prefscontrol.c,v 1.3 2003/02/27 12:19:24 nordstrom Exp $
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
#include "resourceids.h"
#include "util.h"

#include "prefscontrol.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean            showFirst = false;
static UInt16             prevControlMode;
static SelectType         controlSelect[ NUM_OF_CONTROL_MODES ]
                                       [ NUM_OF_CONTROL_LISTS ];
static ModeType           controlMode;
static UInt16             objectList[][ MAX_OBJECTS ] = {
    { bmpMode1, frmPrefsControlPopup1, frmPrefsControlPopup2,
      frmPrefsControlPopup3, frmPrefsControlPopup4, frmPrefsControlLabel1,
      frmPrefsControlLabel2, frmPrefsControlLabel3, frmPrefsControlLabel4,
      EOL },
    { bmpMode2, frmPrefsControlPopup1, frmPrefsControlPopup2,
      frmPrefsControlPopup3, frmPrefsControlPopup4, frmPrefsControlLabel1,
      frmPrefsControlLabel2, frmPrefsControlLabel3, frmPrefsControlLabel4,
      EOL },
    { bmpMode3, frmPrefsControlMsg1, frmPrefsControlMsg2, frmPrefsControlMsg3,
      frmPrefsControlMsg4, EOL }
};



/* Handle the Control preferences */
Boolean PrefsControlPreferenceEvent
    (
    ActionType action
    )
{
    Boolean handled;
    UInt16  i;

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
            MemMove( controlSelect, Prefs()->select, sizeof( controlSelect ) );
            controlMode = Prefs()->controlMode;

            CtlSetValue( GetObjectPtr( frmPrefsControlMode1 ),
                controlMode == MODE1 );
            CtlSetValue( GetObjectPtr( frmPrefsControlMode2 ),
                controlMode == MODE2 );
            CtlSetValue( GetObjectPtr( frmPrefsControlMode3 ),
                controlMode == MODE3 );

            handled = true;
            break;

        case DISPLAY:
            PrefsShowSection( objectList, controlMode );
            prevControlMode = controlMode;

            if ( controlMode != MODE3 ) {
                for ( i = 0; i < NUM_OF_CONTROL_LISTS; i++ ) {
                    InitializeActionList( frmPrefsControlList1 + i );
                    LstSetHeight( GetObjectPtr( frmPrefsControlList1 + i ),
                        14 );
                    SetListToSelection( frmPrefsControlList1 + i,
                        frmPrefsControlPopup1 + i,
                        controlSelect[ controlMode ][ i ] );
                }
            }
            handled = true;
            break;

        case CLEAR:
            PrefsHideSection( objectList, prevControlMode );
            handled = true;
            break;

        case SAVE:
            MemMove( Prefs()->select, controlSelect, sizeof( controlSelect ) );
            Prefs()->controlMode = controlMode;
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Nominate the Control section to be shown first when the prefsform loads */
void PrefsControlShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Control preference */
Boolean PrefsControlPalmEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled     = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmPrefsControlMode1:
                case frmPrefsControlMode2:
                case frmPrefsControlMode3:
                    controlMode = event->data.ctlEnter.controlID -
                        frmPrefsControlMode1;
                    /* We need to hide then redraw our current section because
                       HandleControlPrefs() has the code to display the proper
                       objects to match our recently updated controlMode */
                    PrefsHideSection( objectList, prevControlMode );
                    PrefsOpenSection( GetCurrentSection() );
                    handled = true;
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
                    case frmPrefsControlPopup1:
                    case frmPrefsControlPopup2:
                    case frmPrefsControlPopup3:
                    case frmPrefsControlPopup4:
                    {
                        UInt8 area;

                        area = event->data.popSelect.listID -
                            frmPrefsControlList1;
                        controlSelect[ controlMode ][ area ] =
                            LstGetSelection( list );
                        handled = true;
                        break;
                    }

                    default:
                        handled = false;
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

