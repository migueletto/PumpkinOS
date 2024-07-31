/*
 * $Id: statusform.c,v 1.6 2004/05/12 01:49:10 prussar Exp $
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

#include "resourceids.h"
#include "util.h"
#include "hires.h"
#include "DIA.h"

#include "statusform.h"

/* Simple event handler for frmAbout (used to be used for other forms) */

Boolean StatusFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean     handled;
    FormType*   form;

    form    = FrmGetActiveForm();
    handled = false;

    switch ( (Int32)event->eType ) {
        case frmOpenEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmOpenEvent();
#endif
            FrmDrawForm( form );
            handled = true;
            break;

        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmAboutOK ) {
                FrmReturnToForm( PREVIOUS_FORM );
                handled = true;
            }
            break;

        case winDisplayChangedEvent:
            handled = ResizeHandleWinDisplayChangedEvent();
            break;

        case winExitEvent:
            handled = ResizeHandleWinExitEvent();
            break;

        case winEnterEvent:
            handled = ResizeHandleWinEnterEvent();
            break;

        case frmUpdateEvent:
            FrmDrawForm( FrmGetActiveForm() );
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        case pluckerClosePopupEvent:
            FrmReturnToForm( PREVIOUS_FORM );
            handled = true;
            break;

        case pluckerDeleteFormEvent:
            FrmEraseForm( form );
            FrmDeleteForm( form );
            handled = true;
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}

