/*
 * $Id: prefsbrowsing.c,v 1.11 2004/04/18 15:34:48 prussar Exp $
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

#include "prefsbrowsing.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean        showFirst   = false;
static ForceAlignType forceAlign;
static Boolean        forceDefaultColors;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void LoadBrowsingPrefs( void ) PREFSFORM_SECTION;



/* Handle the Browsing preferences */
Boolean PrefsBrowsingPreferenceEvent
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
            LoadBrowsingPrefs();
            handled = true;
            break;

        case SAVE:
            Prefs()->strikethrough = CtlGetValue( GetObjectPtr(
                                        frmPrefsBrowsingStrikethrough ) );
            Prefs()->underlineMode = CtlGetValue( GetObjectPtr(
                                        frmPrefsBrowsingUnderline ) );
            Prefs()->pageControlsLink = CtlGetValue( GetObjectPtr(
                                        frmPrefsBrowsingPageControlsLink ) );
            Prefs()->enableSoftHyphens = CtlGetValue( GetObjectPtr(
                                        frmPrefsBrowsingEnableSoftHyphens ) );
            Prefs()->forceAlign = forceAlign;
            forceDefaultColors = CtlGetValue( GetObjectPtr(
                                     frmPrefsBrowsingForceDefaultColors ) );
            Prefs()->forceDefaultColors = forceDefaultColors;
/*            if ( forceDefaultColors != Prefs()->forceDefaultColors ) {

                if ( IsFormMain( Prefs()->lastForm ) )
                    FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
            } */

            handled = true;
            break;

        case RELEASE:
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Load the browsing section into memory */
static void LoadBrowsingPrefs( void )
{
    CtlSetValue( GetObjectPtr( frmPrefsBrowsingStrikethrough ),
        Prefs()->strikethrough );
    CtlSetValue( GetObjectPtr( frmPrefsBrowsingUnderline ),
        Prefs()->underlineMode );
    CtlSetValue( GetObjectPtr( frmPrefsBrowsingPageControlsLink ),
        Prefs()->pageControlsLink );
    CtlSetValue( GetObjectPtr( frmPrefsBrowsingEnableSoftHyphens ),
        Prefs()->enableSoftHyphens );
    CtlSetValue( GetObjectPtr( frmPrefsBrowsingForceDefaultColors ),
        Prefs()->forceDefaultColors );
    forceAlign = Prefs()->forceAlign;
    SetListToSelection( frmPrefsBrowsingAlignList,
        frmPrefsBrowsingAlignPopup, forceAlign );
}



/* Nominate the Browsing section to be shown first when the prefsform loads */
void PrefsBrowsingShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Browsing preferences */
Boolean PrefsBrowsingPalmEvent
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
                    case frmPrefsBrowsingAlignPopup:
                        forceAlign = (ForceAlignType) selection;
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

