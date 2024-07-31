/*
 * $Id: detailsform.c,v 1.41 2004/05/12 02:21:26 prussar Exp $
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
#include "externalform.h"
#include "history.h"
#include "hires.h"
#include "image.h"
#include "link.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "table.h"
#include "util.h"
#include "DIA.h"

#include "detailsform.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void AddDocNameTitle( Char* name ) DETAILSFORM_SECTION;
static void DetailsFormInit( void ) DETAILSFORM_SECTION;



/* write document name in details form */
static void AddDocNameTitle
    (
    Char* name
    )
{
    FontID oldFont;
    Coord  x;
    Coord  y;
    
    UInt16 oldCoordSys;

    oldCoordSys = PalmSetCoordinateSystem( STANDARD );

    oldFont = FntSetFont( HiResFont( boldFont ) );
    x       = ( MaxExtentX() - FntCharsWidth( name, StrLen( name ) ) ) / 2;
    y       = 20;
    HiResAdjust( &y, sonyHiRes | handeraHiRes );
    WinDrawChars( name, StrLen( name ), x, y );
    FntSetFont( oldFont );

    PalmSetCoordinateSystem( oldCoordSys );
}



/* Initialize the details form */
static void DetailsFormInit( void )
{
    FormType*   detailsForm;
    FieldType*  urlField;
    UInt16      reference;

    detailsForm = FrmGetFormPtr( frmDetails );
    urlField    = GetObjectPtr( frmDetailsLink );
    reference   = GetHistoryCurrent();

    if ( AddURLToField( urlField, reference ) )
        FrmShowObject( detailsForm, FrmGetObjectIndex( detailsForm,
                                        frmDetailsCopy ) );
    else
        FrmHideObject( detailsForm, FrmGetObjectIndex( detailsForm,
                                        frmDetailsCopy ) );
    FrmDrawForm( detailsForm );

    AddDocNameTitle( Prefs()->docName );

    CtlSetValue( GetObjectPtr( frmDetailsStatusRead ),
        LinkVisited( reference ) );
    CtlSetValue( GetObjectPtr( frmDetailsStatusUnread ),
        ! LinkVisited( reference ) );
    CtlSetValue( GetObjectPtr( frmDetailsShowImages ),
        ShowImages( reference ) );
}



/* Event handler for the details form */
Boolean DetailsFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;
    Boolean update;

    handled     = false;
    update      = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmDetailsOK ) {
                UInt16  reference;
                Boolean oldStatus;
                Boolean newStatus;

                reference = GetHistoryCurrent();
                oldStatus = ShowImages( reference );
                newStatus = CtlGetValue( GetObjectPtr( frmDetailsShowImages ) );
                update    = oldStatus ^ newStatus;

                if ( newStatus )
                    ShowImagesOn( reference );
                else
                    ShowImagesOff( reference );

                if ( CtlGetValue( GetObjectPtr( frmDetailsStatusRead ) ) )
                    SetVisitedLink( reference );
                else
                    UnsetVisitedLink( reference );

            }
            else if ( event->data.ctlEnter.controlID == frmDetailsCopy ) {
                FieldType* field;

                field = GetObjectPtr( frmDetailsLink );
                WriteTextFieldToMemo( field );
            }
            else if ( event->data.ctlEnter.controlID != frmDetailsCancel ) {
                break;
            }
            FrmReturnToForm( PREVIOUS_FORM );
            if ( update ) {
                ResetHeight();
                FrmUpdateForm( GetMainFormId(), frmRedrawUpdateCode );
            }
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
            DetailsFormInit();
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

