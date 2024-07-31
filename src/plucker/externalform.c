/*
 * $Id: externalform.c,v 1.47 2004/05/08 09:04:53 nordstrom Exp $
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
#include "document.h"
#include "genericfile.h"
#include "jogdial.h"
#include "fiveway.h"
#include "keyboard.h"
#include "rotate.h"
#include "paragraph.h"
#include "resourceids.h"
#include "uncompress.h"
#include "util.h"
#include "os.h"
#include "DIA.h"

#include "externalform.h"

#ifdef HAVE_PALMCUNIT
#include "mock_Header.h"
#endif


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void ExternalLinksFormInit( void ) EXTERNALFORM_SECTION;
static Boolean IsGoBackEvent( EventType* event ) EXTERNALFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static Int16 linkIdx;



/* Set current link in the ExternalLinks record */
void SetLinkIndex
    (
    Int16 index /* index of record */
    )
{
    linkIdx = index;
}



/* Initialize field with URL string */
Boolean AddURLToField
    (
    FieldType*  fldPtr, /* pointer to field */
    Int16       index   /* index in URL record */
    )
{
    Char*   url;
    Boolean status;

    url = SafeMemPtrNew( 3000 );
    SetCopyBuffer( url, 3000 );
    status = AddURL( index );
    if ( status )
        FldInsert( fldPtr, url, StrLen( url ) );
    SafeMemPtrFree( url );
    return status;
}



/* Initialize the externallinks form */
static void ExternalLinksFormInit( void )
{
    FormType*   externallinksForm;
    FieldType*  field;

    field = GetObjectPtr( frmExternalLinksLink );
    externallinksForm = FrmGetFormPtr( frmExternalLinks );
    if ( AddURLToField( field, linkIdx ) )
    {
        FrmShowObject( externallinksForm, FrmGetObjectIndex( externallinksForm,
                                            frmExternalLinksCopy) );
#ifdef HAVE_HANDSPRING_SDK
        if ( HaveHsNav() )
            FrmShowObject( externallinksForm, FrmGetObjectIndex( externallinksForm,
                                                frmExternalLinksBrowse) );
        else
            FrmHideObject( externallinksForm, FrmGetObjectIndex( externallinksForm,
                                                frmExternalLinksBrowse) );
#endif
    }
    else
    {
        FrmHideObject( externallinksForm, FrmGetObjectIndex( externallinksForm,
                                            frmExternalLinksCopy) );
#ifdef HAVE_HANDSPRING_SDK
        FrmHideObject( externallinksForm, FrmGetObjectIndex( externallinksForm,
                                            frmExternalLinksBrowse) );
#endif
    }
    FrmDrawForm( externallinksForm );
}



/* Write the text from a TextField to a Memo */
void WriteTextFieldToMemo
    (
    FieldType* field    /* field to get text from */
    )
{
    UInt16 length;

    length = FldGetTextLength( field );
    if ( length == 0 )
        return;

    WriteTextToMemo( FldGetTextPtr( field ) );
}



/* Write given text to a Memo */
void WriteTextToMemo
    (
    Char* textPtr    /* text to add to memo */
    )
{
    static UInt16 index     = dmMaxRecordIndex;
    static UInt32 offset    = 0;
    static UInt32 secs      = 0;

    if ( secs == 0 ) {
        DateTimeType dateTime;

        secs = TimGetSeconds();
        TimSecondsToDateTime( secs, &dateTime );
        WriteMemoEntry( &index, &offset, "%s %4d-%02d-%02d %02d:%02d\n",
            "Plucker URLs", dateTime.year, dateTime.month, dateTime.day,
            dateTime.hour, dateTime.minute );
    }
    WriteMemoEntry( &index, &offset, "%s\n", textPtr );
}



/* Check to see if it is a "go back" event */
static Boolean IsGoBackEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean status;

    /* Check form button and jogdial */
    if ( event->data.ctlEnter.controlID == frmExternalLinksBack ||
         IsJogdialBack( event->data.keyDown.chr ) ) {
        return true;
    }

    /* Check fiveway */
    if ( Prefs()->arrowKeys ) {
        Boolean didLeft;
        Boolean didUp;
        Boolean didDown;

        didLeft = FiveWayKeyPressed( event, Left );
        didUp   = FiveWayKeyPressed( event, Up );
        didDown = FiveWayKeyPressed( event, Down );
        if ( RotSelect( didLeft, didDown, didUp ) &&
             Prefs()->arrowMode[ LEFT_ARROW ] == SELECT_GO_BACK ) {
            return true;
        }
    }

    /* Check gestures and keyboard */
    status = false;
    switch ( event->data.keyDown.chr ) {
        case backspaceChr:
            if ( Prefs()->gestures &&
                 Prefs()->gestMode[ GESTURES_LEFT ] == SELECT_GO_BACK ) {
                status = true;
            }
            else if ( GetKeyboardAction( event->data.keyDown.chr, 0 ) ==
                      SELECT_GO_BACK ) {
                status = true;
            }
            break;

        default:
            if ( GetKeyboardAction( event->data.keyDown.chr, 0 ) ==
                 SELECT_GO_BACK ) {
                status = true;
            }
            break;
    }
    return status;
}
    


/* Event handler for the externallinks form */
Boolean ExternalLinksFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean     handled;
    FieldType*  field;

    handled = false;
    field   = GetObjectPtr( frmExternalLinksLink );

    switch ( event->eType ) {
        case ctlSelectEvent:
        case keyDownEvent:
            if ( event->data.ctlEnter.controlID == frmExternalLinksCopy ||
                 IsJogdialRelease( event->data.keyDown.chr ) )
                WriteTextFieldToMemo( field );
#ifdef HAVE_HANDSPRING_SDK
            else if ( event->data.ctlEnter.controlID == frmExternalLinksBrowse )
                HsBrowseUrl( FldGetTextPtr( field ) );
#endif
            else if ( ! IsGoBackEvent( event ) )
                break;

            FrmGotoForm( GetMainFormId() );
            handled = true;
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
            ExternalLinksFormInit();
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

