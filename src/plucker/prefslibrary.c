/*
 * $Id: prefslibrary.c,v 1.7 2004/03/07 05:29:07 prussar Exp $
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

#include "debug.h"
#include "prefsdata.h"
#include "util.h"

#include "prefslibrary.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Boolean              showFirst = false;
static SyncPolicyType       syncPolicy;
static CategoryStyleType    categoryStyle;
static SortType             sortBy;
static SortOrderType        sortOrder;



/* Handle the Library preferences */
Boolean PrefsLibraryPreferenceEvent
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
            syncPolicy = Prefs()->syncPolicy;
            SetListToSelection( frmPrefsLibrarySyncPolicyList,
                frmPrefsLibrarySyncPolicyPopup, syncPolicy );

            categoryStyle = Prefs()->categoryStyle;
            SetListToSelection( frmPrefsLibraryCategoryStyleList,
                frmPrefsLibraryCategoryStylePopup, categoryStyle );

            sortBy = Prefs()->sortType;
            /* We must decrease the sortBy value when setting the default
               selection, since the list doesn't include the first value
               from the SortType (the SORT_INVALID value) */
            SetListToSelection( frmPrefsLibrarySortByList,
                frmPrefsLibrarySortByPopup, sortBy - 1 );

            sortOrder = Prefs()->sortOrder;
            SetListToSelection( frmPrefsLibrarySortOrderList,
                frmPrefsLibrarySortOrderPopup, sortOrder );

            CtlSetValue( GetObjectPtr( frmPrefsLibraryDateTime ),
                Prefs()->useDateTime );
            CtlSetValue( GetObjectPtr( frmPrefsLibraryShowType ),
                Prefs()->column.type );
            CtlSetValue( GetObjectPtr( frmPrefsLibraryShowDate ),
                Prefs()->column.date );
            CtlSetValue( GetObjectPtr( frmPrefsLibraryShowSize ),
                Prefs()->column.size );
            CtlSetValue( GetObjectPtr( frmPrefsLibraryIndicateOpened ),
                Prefs()->indicateOpened );
            handled = true;
            break;

        case SAVE:
            Prefs()->syncPolicy     = syncPolicy;
            Prefs()->categoryStyle  = categoryStyle;
            Prefs()->sortType       = sortBy;
            Prefs()->sortOrder      = sortOrder;
            Prefs()->useDateTime    = CtlGetValue( GetObjectPtr(
                                        frmPrefsLibraryDateTime ) );
            Prefs()->column.type    = CtlGetValue( GetObjectPtr(
                                        frmPrefsLibraryShowType ) );
            Prefs()->column.date    = CtlGetValue( GetObjectPtr(
                                        frmPrefsLibraryShowDate ) );
            Prefs()->column.size    = CtlGetValue( GetObjectPtr(
                                        frmPrefsLibraryShowSize ) );
            Prefs()->indicateOpened = CtlGetValue( GetObjectPtr(
                                        frmPrefsLibraryIndicateOpened ) );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}



/* Nominate the Library section to be shown first when the prefsform loads */
void PrefsLibraryShowFirst( void )
{
    showFirst = true;
}



/* Event handler for the Library preference */
Boolean PrefsLibraryPalmEvent
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
                    case frmPrefsLibrarySyncPolicyPopup:
                        syncPolicy = (SyncPolicyType) selection;
                        handled = true;
                        break;

                    case frmPrefsLibraryCategoryStylePopup:
                        categoryStyle = (CategoryStyleType) selection;
                        handled = true;
                        break;

                    case frmPrefsLibrarySortByPopup:
                        /* We must increase the selection value when setting
                           the sortBy, since the list doesn't include the first
                           value from the SortType (the SORT_INVALID value) */
                        sortBy = (SortType) ( selection + 1 );
                        handled = true;
                        break;

                    case frmPrefsLibrarySortOrderPopup:
                        sortOrder = (SortOrderType) selection;
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

