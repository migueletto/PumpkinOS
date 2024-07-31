/*
 * $Id: bookmarkform.c,v 1.44 2004/05/08 09:04:53 nordstrom Exp $
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
#include <TxtGlue.h>

#include "bookmark.h"
#include "debug.h"
#include "document.h"
#include "fullscreenform.h"
#include "hires.h"
#include "image.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "table.h"
#include "util.h"
#include "bookmarkform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define DELETE_BOOKMARK         0
#define MAX_BOOKMARK_LIST_LEN   22


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void BookmarksFormInit( void ) BOOKMARKFORM_SECTION;
static void AddBookmarkFormInit( void ) BOOKMARKFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FieldType*   fldPtr;
static UInt16       recordID  = NO_RECORD;



/* Initialize the add bookmark form */
static void AddBookmarkFormInit( void )
{
    FormType*       addBookmarkForm;
    Int16           length;
    Char            name[ MAX_BOOKMARK_LEN + 1 ];
    Int16           i;

    addBookmarkForm = FrmGetFormPtr( frmAddBookmark );
    fldPtr          = GetObjectPtr( frmAddBookmarkName );

    TextifyRecord( name, MAX_BOOKMARK_LEN );
    length = StrLen( name );
    /*eliminate raw control chars*/
    for ( i = 0; i < length; i++ ) {
        if ( TxtGlueCharIsCntrl( name[ i ] ) ) {
            name[ i ] = ' ';
        }
    }
    
    FldInsert( fldPtr, name, length );
    FldSetSelection( fldPtr, 0, length );

    FrmDrawForm( addBookmarkForm );
    FrmSetFocus( addBookmarkForm, FrmGetObjectIndex( addBookmarkForm,
        frmAddBookmarkName ) );
}



/* Event handler for the add bookmark form */
Boolean AddBookmarkFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled = false;

    switch ( event->eType ) {
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
            AddBookmarkFormInit();
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmAddBookmarkAdd ) {
                if ( FldDirty( fldPtr ) ) {
                    Char* name;

                    name = FldGetTextPtr( fldPtr );
                    ErrTry {
                        AddBookmark( name );
                    }
                    ErrCatch( err ) {
                        if ( err == errNoBookmarkName )
                            break;
                        else
                            FrmCustomAlert( errCannotAddBookmark, name, NULL,
                                NULL );
                    } ErrEndCatch
                }
                else {
                    break;
                }
            }
            else if ( event->data.ctlEnter.controlID != frmAddBookmarkCancel )
                break;

            FrmReturnToForm( PREVIOUS_FORM );
            handled = true;
            break;

        default:
            handled = false;
    }

    return handled;
}



/* Return record ID for selected bookmark */
UInt16 GetBookmarkRecordID( void )
{
    return recordID;
}



/* Initialize the bookmark form */
static void BookmarksFormInit( void )
{
    FormType* bookmarksForm;
    ListType* list;

    bookmarksForm = FrmGetFormPtr( frmBookmarks );

    list = GetObjectPtr( frmBookmarksList );
    CreateBookmarkList( list );
//    LstSetHeight( list, MAX_BOOKMARK_LIST_LEN );

    FrmDrawForm( bookmarksForm );
}



/* Event handler for the bookmark form */
Boolean BookmarksFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    UInt16  extEntries = 0;
    //Int16   extIndex   = NULL;
    Int16   extIndex   = 0;
    Boolean handled    = false;

    recordID   = NO_RECORD;
    extEntries = CountExtBookmarks();

    switch ( event->eType ) {
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
            BookmarksFormInit();
            handled = true;
            break;

        case frmCloseEvent:
            ReleaseBookmarkList();
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif            
            handled = false;
            break;

        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmBookmarksDelete ) {
                ListType*   list;
                Int16       selection;

                list        = GetObjectPtr( frmBookmarksList );
                selection   = LstGetSelection( list );
                if ( extEntries <= selection &&
                     selection != noListSelection ) {

                    UInt16 choice;

                    choice = FrmCustomAlert( confirmDelete,
                                LstGetSelectionText( list, selection ), NULL,
                                    NULL );
                    if ( choice == DELETE_BOOKMARK ) {
                        ErrTry {
                            DeleteBookmark( selection - extEntries );
                            ReleaseBookmarkList();
                            CreateBookmarkList( list );
//                            LstSetHeight( list, MAX_BOOKMARK_LIST_LEN );
                            LstDrawList( list );
                        }
                        ErrCatch( UNUSED_PARAM( err ) ) {
                            FrmCustomAlert( errCannotDeleteBookmark,
                                LstGetSelectionText( list, selection ), NULL,
                                NULL );
                        } ErrEndCatch
                    }
                }
                handled = true;
                break;
            }
            else if ( event->data.ctlEnter.controlID == frmBookmarksGo ) {
                ListType*   list;
                Int16       selection;

                list        = GetObjectPtr( frmBookmarksList );
                selection   = LstGetSelection( list );
                if ( selection != noListSelection ) {
                    if ( selection >= extEntries )
                        recordID = RestoreBookmarkData( selection -
                            extEntries );
                    else
                        extIndex = selection;
                }
            }
            else if ( event->data.ctlEnter.controlID != frmBookmarksDone ) {
                extEntries = 0;
                break;
            }

            ReleaseBookmarkList();
            FrmReturnToForm( PREVIOUS_FORM );

            if ( recordID != NO_RECORD ) {
                if ( IsFullscreenformActive() )
                    FsFrmGotoForm ( GetMainFormId() );
                else
                    FrmUpdateForm( GetMainFormId(), frmViewRecord );
            }
            //else if ( extIndex != NULL ) {
            else if ( extIndex != 0 ) {
                GotoExtBookmark( extIndex );
            }

            handled = true;
            break;

        default:
            handled = false;
    }

    return handled;
}

