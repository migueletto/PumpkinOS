/*
 * $Id: renamedocform.c,v 1.9 2004/05/08 09:04:54 nordstrom Exp $
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
#include <VFSMgr.h>

#include "debug.h"
#include "doclist.h"
#include "genericfile.h"
#include "libraryform.h"
#include "resourceids.h"
#include "util.h"
#include "vfsfile.h"
#include "DIA.h"

#include "renamedocform.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FieldType*    fldPtr;
static DocumentInfo* docInfo;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void RenameDocFormInit( void ) RENAMEDOCFORM_SECTION;


/* Initialize the rename document form */
static void RenameDocFormInit( void )
{
    FormType* form;

    docInfo = DocInfo( ReturnLastIndex() );
    fldPtr = GetObjectPtr( frmRenameDocField );
    FldSetSelection( fldPtr, 0, 0 );
    FldInsert( fldPtr, docInfo->name, StrLen( docInfo->name ) );
    FldSetSelection( fldPtr, 0, StrLen( docInfo->name ) );
    FldSetDirty( fldPtr, false );

    form = FrmGetFormPtr( frmRenameDoc );
    FrmDrawForm( form );
    FrmSetFocus( form, FrmGetObjectIndex( form, frmRenameDocField ) );
}



/* Event handler for the rename document form */
Boolean RenameDocFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;
    UInt16  libraryFormId;
    Char    filename[ PATH_LEN ];

    handled = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            if ( event->data.ctlEnter.controlID == frmRenameDocOK ) {

                if ( FldDirty( fldPtr ) ) {
                    Char* docName;

                    docName = FldGetTextPtr( fldPtr );
                    ErrTry {
                        RenameDocument( docName, docInfo, filename );
                        UpdateDocumentName( ReturnLastIndex(), docName, 
                            filename );
                        ReleaseDocInfoList();
                        InitializeDocInfoList();
                        FrmUpdateForm( GetValidForm( frmLibrary ), 
                            frmRedrawUpdateCode);
                    }
                    ErrCatch( err ) {
                        switch ( err ) {
                            case errNoDocumentName:
                                break;

                            case vfsErrFilePermissionDenied:
                            case dmErrReadOnly:
                                FrmAlert( errReadOnly );
                                break;

                            case vfsErrFileAlreadyExists:
                            case dmErrAlreadyExists:
                                FrmCustomAlert( errAlreadyExists, docName, NULL, NULL );
                                FldSetSelection( fldPtr, 0, StrLen( docName ) );
                                break;

                            default:
                                FrmCustomAlert( errCannotRenameDoc, docName, NULL, NULL );
                                break;
                        }
                        break;
                    } ErrEndCatch
                }
            }
            else if ( event->data.ctlEnter.controlID != frmRenameDocCancel ) {
                break;
            }
            libraryFormId = GetValidForm( frmLibrary );
            FrmReturnToForm( libraryFormId );
            if ( event->data.ctlEnter.controlID != frmRenameDocCancel )
                FrmUpdateForm( libraryFormId, frmUpdateList );
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
            RenameDocFormInit();
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
