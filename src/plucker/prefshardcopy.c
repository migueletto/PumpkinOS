/*
 * $Id: prefshardcopy.c,v 1.4 2003/10/09 01:51:43 matto Exp $
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

#include "prefshardcopy.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean            showFirst = false;
static HardcopyActionType hardcopyAction;
static HardcopyRangeType  hardcopyRange;
static HardcopyLinkType   hardcopyLink;


/* Handle the Hardcopy preferences */
Boolean PrefsHardcopyPreferenceEvent
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
            hardcopyAction = Prefs()->hardcopyAction;
            SetListToSelection( frmPrefsHardcopyActionList,
                frmPrefsHardcopyActionPopup, hardcopyAction );

            hardcopyRange  = Prefs()->hardcopyRange;
            SetListToSelection( frmPrefsHardcopyRangeList,
                frmPrefsHardcopyRangePopup, hardcopyRange );

            hardcopyLink   = Prefs()->hardcopyLink;
            SetListToSelection( frmPrefsHardcopyLinkList,
                frmPrefsHardcopyLinkPopup, hardcopyLink );
            handled = true;
            break;

        case SAVE:
            Prefs()->hardcopyAction = hardcopyAction;
            Prefs()->hardcopyRange  = hardcopyRange;
            Prefs()->hardcopyLink   = hardcopyLink;
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Nominate the Hardcopy section to be shown first when the prefsform loads */
void PrefsHardcopyShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Hardcopy preference */
Boolean PrefsHardcopyPalmEvent
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
                    case frmPrefsHardcopyActionPopup:
                        hardcopyAction = selection;
                        handled = true;
                        break;

                    case frmPrefsHardcopyRangePopup:
                        hardcopyRange = selection;
                        handled = true;
                        break;

                    case frmPrefsHardcopyLinkPopup:
                        hardcopyLink = selection;
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

