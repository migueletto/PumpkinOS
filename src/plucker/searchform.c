/*
 * $Id: searchform.c,v 1.45 2004/05/08 09:04:54 nordstrom Exp $
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
#include "hires.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "search.h"
#include "util.h"
#include "const.h"
#include "xlit.h"

#include "searchform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define START_AT_TOP true


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void SearchFormInit( void ) SEARCHFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FieldType*   fldPtr;
static SearchModeType    searchMode;
static UInt16            xlitMode = 0;



/* Initialize the search form */
static void SearchFormInit( void )
{
    FormType*       searchForm;
    ListType*       list;
    ControlType*    ctl;
    Char            text[ MAX_PATTERN_LEN+1 ];

    SetupXlitPopup();

    fldPtr = GetObjectPtr( frmSearchField );
    FldSetSelection( fldPtr, 0, 0 );
    GetSearchString( text );
    if ( *text != '\0' ) {
        FldInsert( fldPtr, text, StrLen( text ) );
        FldSetSelection( fldPtr, 0, StrLen( text ) );
    }

    searchForm = FrmGetFormPtr( frmSearch );

    FrmDrawForm( searchForm );

    FrmSetFocus( searchForm, FrmGetObjectIndex( searchForm, frmSearchField ) );

    searchMode = Prefs()->searchMode;

    list    = GetObjectPtr( frmSearchList );
    ctl     = GetObjectPtr( frmSearchPopup );

    LstSetSelection( list, searchMode );
    CtlSetLabel( ctl, LstGetSelectionText( list, searchMode ) );

#ifdef SUPPORT_TRANSLITERATION
    list    = GetObjectPtr( frmSearchXlitList );
    ctl     = GetObjectPtr( frmSearchXlitPopup );

    xlitMode = Prefs()->searchXlit;
    
    if ( LstGetNumberOfItems( list ) <= xlitMode ) {
        xlitMode = 0;
    }

    LstSetSelection( list, xlitMode );
    CtlSetLabel( ctl, LstGetSelectionText( list, xlitMode ) );
#endif

    CtlSetValue( GetObjectPtr( frmSearchCasesensitive ),
        ( Prefs()->searchFlags & SEARCH_CASESENSITIVE ) );
    CtlSetValue( GetObjectPtr( frmSearchPhrase ),
        ! ( Prefs()->searchFlags & SEARCH_MULTIPLE ) );
}



/* Event handler for the search form */
Boolean SearchFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean   handled;
    Boolean   found;
    Char*     pattern;

    handled = false;
    found   = false;
    pattern = NULL;

    switch ( event->eType ) {
        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmSearchOK ) {
                Prefs()->searchFlags = 0;
                if ( CtlGetValue( GetObjectPtr( frmSearchCasesensitive ) ) )
                    Prefs()->searchFlags |= SEARCH_CASESENSITIVE;
                if ( ! CtlGetValue( GetObjectPtr( frmSearchPhrase ) ) )
                    Prefs()->searchFlags |= SEARCH_MULTIPLE;
                Prefs()->searchMode   = searchMode;
                Prefs()->searchXlit   = xlitMode;

                pattern = FldGetTextPtr( fldPtr );
                if ( pattern != NULL ) {
                    AddSearchString( pattern );

                    if ( searchMode == SEARCH_IN_ONE_PAGE ) {
                        found = SearchDocument( pattern, true, NULL,
                                    searchMode );
                    }
                    else {
                        FrmGotoForm( GetValidForm( frmResult ) );
                        handled = true;
                        break;
                    }
                }
            }
            else if ( event->data.ctlEnter.controlID == frmSearchHistory ) {
                MemHandle   nameListHandle;
                ListType*   list;
                Char**      nameList;
                Char*       patternList;
                Int16       selection;
                Int16       entries;

                entries = Prefs()->searchEntries;
                if ( entries == 0 ) {
                    FrmAlert( infoEmptyPatterns );
                    break;
                }

                patternList     = GetSearchPatterns();
                nameListHandle  = SysFormPointerArrayToStrings( patternList,
                                    entries );
                nameList        = MemHandleLock( nameListHandle );
                list            = GetObjectPtr( frmSearchHistoryList );

                LstSetListChoices( list, nameList, entries );
                LstSetHeight( list, entries );

                selection = LstPopupList( list );
                if ( selection != noListSelection ) {
                    Char* pattern;
                    Char* label;

                    label   = LstGetSelectionText( list, selection );
                    pattern = FldGetTextPtr( fldPtr );
                    if ( pattern != NULL )
                        FldSetSelection( fldPtr, 0, StrLen( pattern ) );
                    FldInsert( fldPtr, label, StrLen( label ) );
                    FldSetSelection( fldPtr, 0, StrLen( label ) );
                }
                ReleaseSearchPatterns();
                MemHandleUnlock( nameListHandle );
                MemHandleFree( nameListHandle );
                handled = true;
                break;
            }
            else if ( event->data.ctlEnter.controlID != frmSearchCancel ) {
                break;
            }
            FrmReturnToForm( PREVIOUS_FORM );
            if ( found )
                GoToSearchResult();
            handled = true;
            break;

        case popSelectEvent:
            if ( event->data.popSelect.controlID == frmSearchPopup ) {
                Int16 selection;

                selection = event->data.popSelect.selection;
                if ( selection != noListSelection ) {
                    ListType*       list;
                    ControlType*    ctl;
                    Char*           label;

                    list    = event->data.popSelect.listP;
                    ctl     = GetObjectPtr( frmSearchPopup );
                    label   = LstGetSelectionText( list, selection );

                    CtlSetLabel( ctl, label );
                    LstSetSelection( list, selection );
                    searchMode = selection;
                }
                handled = true;
            }
#ifdef SUPPORT_TRANSLITERATION
            else if ( event->data.popSelect.controlID == frmSearchXlitPopup ) {
                Int16 selection;

                selection = event->data.popSelect.selection;
                if ( selection != noListSelection ) {
                    ListType*       list;
                    ControlType*    ctl;
                    Char*           label;

                    list    = event->data.popSelect.listP;
                    ctl     = GetObjectPtr( frmSearchXlitPopup );
                    label   = LstGetSelectionText( list, selection );

                    CtlSetLabel( ctl, label );
                    LstSetSelection( list, selection );
                    xlitMode = selection;
                }
                handled = true;
            }
#endif
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            SearchFormInit();
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        default:
            handled = false;
    }

    return handled;
}

