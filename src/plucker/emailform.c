/*
 * $Id: emailform.c,v 1.37 2004/05/08 09:04:53 nordstrom Exp $
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
#include "hires.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "util.h"
#include "DIA.h"

#include "emailform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
static Char toAddress[] = "plucker-team@rubberchicken.org";


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    Int16 to_off;
    Int16 cc_off;
    Int16 subject_off;
    Int16 message_off;
} MailtoData;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void SetScrollLabel( FieldPtr field, Int16 id ) EMAILFORM_SECTION;
static void UpdateScrollers( void ) EMAILFORM_SECTION;
static void GetMailto( void ) EMAILFORM_SECTION;
static void EmailFormInit( void ) EMAILFORM_SECTION;
static void DoMail( void ) EMAILFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FieldType*       toField;
static FieldType*       ccField;
static FieldType*       subjectField;
static FieldType*       messageField;
static FieldType*       currentField;
static ScrollBarType*   scrollBar;
static Int16            mailIdx;



/* Set label/image for scroll arrows */
static void SetScrollLabel
    (
    FieldPtr    field,  /* field pointer */
    Int16       id      /* object ID */
    )
{
    static Char direction[ 2 ];

    if ( FldScrollable( field, winUp ) )
        StrCopy( direction, "\010" );   /* UP */
    else if ( FldScrollable( field, winDown ) )
        StrCopy( direction, "\007" );   /* DOWN */
    else
        StrCopy( direction, "\022" );   /* NONE */

    CtlSetLabel( GetObjectPtr( id ), direction );
}


/* Updates the scrollbar and scroll arrows */
static void UpdateScrollers( void )
{
    SetScrollLabel( toField, frmEmailToArrow );
    SetScrollLabel( ccField, frmEmailCcArrow );
    SetScrollLabel( subjectField, frmEmailSubjectArrow );

    UpdateFieldScrollbar( messageField, scrollBar );
}



/* Get pointer into the Mailto record */
static void GetMailto( void )
{
    MemHandle   handle;
    MailtoData* mail;
    Char*       pointer;

    /* Special case for dev-list */
    if ( mailIdx == 0 ) {
        InsertText( toField, toAddress );
        return;
    }

    handle = ReturnRecordHandle( mailIdx );
    if ( handle == NULL ) {
        InsertText( toField, toAddress );
        FrmAlert( errBadMailto );
        return;
    }

    pointer  = MemHandleLock( handle );
    pointer += sizeof( Header );

    mail = (MailtoData*) pointer;
    if ( mail->to_off != 0 )
        InsertText( toField, pointer + mail->to_off );
    if ( mail->cc_off != 0 )
        InsertText( ccField, pointer + mail->cc_off );
    if ( mail->subject_off != 0 )
        InsertText( subjectField, pointer + mail->subject_off );
    if ( mail->message_off != 0 )
        InsertText( messageField, pointer + mail->message_off );

    MemHandleUnlock( handle );
    FreeRecordHandle( &handle );
}



/* Initialize the email form */
static void EmailFormInit( void )
{
    FormType* emailForm;

    toField         = GetObjectPtr( frmEmailTo );
    ccField         = GetObjectPtr( frmEmailCc );
    subjectField    = GetObjectPtr( frmEmailSubject );
    messageField    = GetObjectPtr( frmEmailMessage );
    scrollBar       = GetObjectPtr( frmEmailScrollBar );

    GetMailto();

    emailForm = FrmGetFormPtr( frmEmail );
    if ( Prefs()->scrollbar == SCROLLBAR_LEFT ) {
        SetObjectPosition( emailForm, frmEmailToLabel, false );
        SetObjectPosition( emailForm, frmEmailTo, false );
        SetObjectPosition( emailForm, frmEmailToArrow, true );
        SetObjectPosition( emailForm, frmEmailCcLabel, false );
        SetObjectPosition( emailForm, frmEmailCc, false );
        SetObjectPosition( emailForm, frmEmailCcArrow, true );
        SetObjectPosition( emailForm, frmEmailSubjectLabel, false );
        SetObjectPosition( emailForm, frmEmailSubject, false );
        SetObjectPosition( emailForm, frmEmailSubjectArrow, true );
        SetObjectPosition( emailForm, frmEmailMessage, false );
        SetObjectPosition( emailForm, frmEmailScrollBar, true );
    }
    FrmDrawForm( emailForm );
    UpdateScrollers();
}



/* Set current address in the Mailto record */
void SetMailto
    (
    Int16 index /* index of record */
    )
{
    mailIdx = index;
}



/* Event handler for the email form. */
Boolean EmailFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;

    handled = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmEmailSend:
                    if ( StrLen( FldGetTextPtr( toField ) ) == 0 ) {
                        FrmAlert( warnNoTo );
                        handled = true;
                        break;
                    }
                    DoMail();
                    FrmGotoForm( Prefs()->lastForm );
                    handled = true;
                    break;

                case frmEmailCancel:
                    FrmGotoForm( Prefs()->lastForm );
                    handled = true;
                    break;

                case frmEmailToArrow:
                    if ( FldScrollable( toField, winUp ) )
                        FldScrollField( toField, 1, winUp );
                    else if ( FldScrollable( toField, winDown ) )
                        FldScrollField( toField, 1, winDown );
                    UpdateScrollers();
                    break;

                case frmEmailCcArrow:
                    if ( FldScrollable( ccField, winUp ) )
                        FldScrollField( ccField, 1, winUp );
                    else if ( FldScrollable( ccField, winDown ) )
                        FldScrollField( ccField, 1, winDown );
                    UpdateScrollers();
                    break;

                case frmEmailSubjectArrow:
                    if ( FldScrollable( subjectField, winUp ) )
                        FldScrollField( subjectField, 1, winUp );
                    else if ( FldScrollable( subjectField, winDown ) )
                        FldScrollField( subjectField, 1, winDown );
                    UpdateScrollers();
                    break;
            }
            break;

        case fldEnterEvent:
            currentField = event->data.fldEnter.pField;
            break;

        case fldChangedEvent:
            UpdateScrollers();
            break;

        case keyDownEvent:
            switch ( event->data.keyDown.chr ) {
                case pageUpChr:
                    if ( FldScrollable( currentField, winUp ) )
                        FldScrollField( currentField, 1, winUp );
                    UpdateScrollers();
                    break;

                case pageDownChr:
                    if ( FldScrollable( currentField, winDown ) )
                        FldScrollField( currentField, 1, winDown );
                    UpdateScrollers();
                    break;

                case chrLineFeed:
                    /* no <CR> in other fields */
                    handled = ( currentField != messageField );
                    break;
            }
            break;

        case sclRepeatEvent:
            ScrollMessage( messageField, event->data.sclRepeat.newValue -
                                         event->data.sclRepeat.value );
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
            CheckMem(warnLowEmailMem);
            EmailFormInit();
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



/* Send mail specified in the email form */
static void DoMail( void )
{
    //Err                     err;
    LocalID                 dbID;
    UInt32                  result;
    UInt16                  cardNo;
    DmSearchStateType       searchState;
    MailAddRecordParamsType message;

    MemSet( &message, sizeof( message ), 0 );

    message.secret          = false;
    message.signature       = true;
    message.confirmRead     = false;
    message.confirmDelivery = false;
    message.priority        = mailPriorityNormal;
    message.to              = FldGetTextPtr( toField );
    message.cc              = FldGetTextPtr( ccField );
    message.subject         = FldGetTextPtr( subjectField );
    message.body            = FldGetTextPtr( messageField );

    dbID = 0;
    DmGetNextDatabaseByTypeCreator( true, &searchState, sysFileTApplication, 
        'mail', true, &cardNo, &dbID );
    ErrNonFatalDisplayIf( dbID == 0, "Could not find Mail Application" );

    /* Mail app not found. Must be m100! */
    if ( dbID == 0 )
        return;

    /*err =*/ SysAppLaunch( cardNo, dbID, 0, sysAppLaunchCmdAddRecord, &message,
            &result );
    //ErrNonFatalDisplayIf( err != errNone, "Could not launch Mail Application" );
}

