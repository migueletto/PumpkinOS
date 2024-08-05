/*
 * $Id: viewer.c,v 1.135 2004/05/12 01:49:10 prussar Exp $
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
 *
 * Originally based on code by Andrew Howlett from his Tex2Hex demo. (Thanks!)
 */

#define PLUCKER_GLOBALS_HERE

#include <PalmOS.h>
#include <VFSMgr.h>

#include "allforms.h"
#include "genericfile.h"
#include "ramfile.h"
#include "cache.h"
#include "const.h"
#include "debug.h"
#include "image.h"
#include "os.h"
#include "paragraph.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "screen.h"
#include "search.h"
#include "session.h"
#include "DIA.h"
#include "timeout.h"
#include "uncompress.h"
#include "util.h"
#include "control.h"
#include "doclist.h"
#include "anchor.h"
#include "font.h"
#include "hires.h"
#include "keyboard.h"
#include "keyboardform.h"
#include "grayfont.h"
#include "xlit.h"
#include "statusform.h"

#include "viewer.h"
#include "language.h"
#include "../libpit/debug.h"

/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define MAX_ERROR_CODE_LEN  11 /* 32-bit ulong */

#define REGISTER    true
#define UNREGISTER  false

/* MIME type for Plucker documents */
#define ViewerMIMEType  "application/prs.plucker"

/* min supported ROM version */
static const UInt32 appMinRomVersion = sysMakeROMVersion( 2, 0, 0,
                                        sysROMStageDevelopment, 0 );


/***********************************************************************
 *
 *      Internal types
 *
 ***********************************************************************/
typedef struct {
    UInt16                  formID;
    FormEventHandlerType*   handler;
} EventHandler;


/***********************************************************************
 *
 *      Local Functions
 *
 ***********************************************************************/
static void EnableNotification ( Boolean registering ) VIEWER_SECTION;
static void HandleNotification ( SysNotifyParamType* cmdPBP ) VIEWER_SECTION;
static Boolean HandleVChrs(EventType *event) VIEWER_SECTION;
static Boolean HandleShiftGestures(void) VIEWER_SECTION;
static void HandleFormLoad(EventType *event) VIEWER_SECTION;
static UInt16 CalculateAutoscrollTimeout(void) VIEWER_SECTION;
static Err StartApplication(MemPtr cmdPBP) VIEWER_SECTION;
static void EventLoop(void) VIEWER_SECTION;
static void StopApplication(void) VIEWER_SECTION;

static language_t *old_lang = NULL;
static language_t *utf8_lang = NULL;

/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
/* list of forms and their event handlers */
static EventHandler ListOfEventHandlers[] = {
    { frmMainTop, MainFormHandleEvent },
    { frmMainBottom, MainFormHandleEvent },
    { frmMainNone, MainFormHandleEvent },
    { frmMainTopHandera, MainFormHandleEvent },
    { frmMainBottomHandera, MainFormHandleEvent },
    { frmMainBottomHanderaLow, MainFormHandleEvent },
    { frmMainNoneHandera, MainFormHandleEvent },
    { frmFont, FontFormHandleEvent },
    { frmFontOS2, FontFormHandleEvent },
    { frmFontHandera, FontFormHandleEvent },
    { frmKeyboard, KeyboardFormHandleEvent },
    { frmPrefs, PrefsFormHandleEvent },
    { frmLibrary, LibraryFormHandleEvent },
    { frmAbout, StatusFormHandleEvent },
    { frmLibraryHandera, LibraryFormHandleEvent },
    { frmLibraryHanderaLow, LibraryFormHandleEvent },
    { frmDetails, DetailsFormHandleEvent },
    { frmHardcopy, HardcopyFormHandleEvent },
    { frmEmail, EmailFormHandleEvent },
    { frmSearch, SearchFormHandleEvent },
    { frmResult, ResultFormHandleEvent },
    { frmAddBookmark, AddBookmarkFormHandleEvent },
    { frmBookmarks, BookmarksFormHandleEvent },
    { frmRenameDoc, RenameDocFormHandleEvent },
    { frmExternalLinks, ExternalLinksFormHandleEvent },
    { frmCategory, CategoryFormHandleEvent },
    { frmNewCategory, CategoryNameFormHandleEvent },
    { frmFullscreen, FsFormHandleEvent },
    { 0x00, NULL }
};



/* This routine opens the document, loads the saved-state information
   and initializes global variables */
static Err StartApplication
    (
    MemPtr cmdPBP   /* pointer to command parameter block */
    )
{
    volatile UInt16 formId;
    Err             err;

    OS_Init();

    err = RomVersionCompatible( appMinRomVersion );
    if ( err != errNone )
        return err;

    err = CreateInternalMemoDB();
    if ( err != errNone )
        return err;

    GrayFntStart();
    OpenTransliterations();
    InitializeLanguageSupport();
    InitializeCache();
    ImageInit();

    ReadPrefs();

    OpenKeyboardMap();
    LoadUserFont( GetUserFontNumber( Prefs()->mainUserFontName, true,
                      fontCacheDoc ) );

    InitializeViewportBoundaries();

    SetStandardFunctions();
    SetScreenMode();
    SetFontFunctions();

    InitializeResizeSupport( resizeIndex );
    LoadResizePrefs( ( UInt32 )ViewerAppID, ( UInt16 )ViewerPrefDIA );

    if ( cmdPBP != NULL ) {
        Char    name[ dmDBNameLength ];
        UInt32  type;
        UInt32  creator;
        UInt16  cardNo;
        LocalID dbID;

        cardNo  = ( (SysAppLaunchCmdOpenDBType*) cmdPBP )->cardNo;
        dbID    = ( (SysAppLaunchCmdOpenDBType*) cmdPBP )->dbID;

        DmDatabaseInfo( cardNo, dbID, name, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, &type, &creator );

        if ( creator == ViewerAppID && type == ViewerDocumentType ) {
            ShowSyncMessage();

            StrCopy( Prefs()->docName, name );
            Prefs()->lastForm = GetMainFormId();
            InitializeDocInfoList();

            HideMessage();
        }
    }

    formId = GetValidForm( frmLibrary );

    err = CreateUncompressBuffer();
    if ( err == dmErrCantOpen ) {
        return err;
    }
    else if ( err == errNone ) {
        if ( GetValidForm( Prefs()->lastForm ) != formId ) {
            err = OpenLastDocument();
            if ( err == errNone )
                formId = GetMainFormId();
            else
                formId = frmLibrary;
        }
    }
    InitSessionData();
    ClearFindPatternData();

    FrmGotoForm( formId );

    EnableNotification( REGISTER );

    return errNone;
}



/* This procedure either registers us or unregisters us from notification
   from special system events. */
static void EnableNotification
    (
    Boolean registering /* are we registering or unregistering? */
    )
{
    if ( SupportNotification() ) {
        UInt16 cardNo;
        LocalID dbID;
        Err err;

        /* Find out what cardID and Local DB ID we're using */
        err = SysCurAppDatabase( &cardNo, &dbID );
        if ( err != errNone )
            return;

        if ( registering ) {
#ifdef HAVE_SONY_SDK
            if ( IsSony() ) {
                SysNotifyRegister( cardNo, dbID,
                    sonySysNotifyHoldStatusChangeEvent, NULL,
                    sysNotifyNormalPriority, 0 );
            }
#endif
            if ( SupportVFS() ) {
                SysNotifyRegister( cardNo, dbID, sysNotifyVolumeMountedEvent,
                                   NULL, sysNotifyNormalPriority, 0 );
                SysNotifyRegister( cardNo, dbID, sysNotifyVolumeUnmountedEvent,
                                   NULL, sysNotifyNormalPriority, 0 );
            }
        } else {
#ifdef HAVE_SONY_SDK
            if ( IsSony() ) {
                SysNotifyUnregister( cardNo, dbID,
                    sonySysNotifyHoldStatusChangeEvent,
                    sysNotifyNormalPriority );
            }
#endif
            if ( SupportVFS() ) {
                SysNotifyUnregister( cardNo, dbID, sysNotifyVolumeMountedEvent,
                                     sysNotifyNormalPriority );
                SysNotifyUnregister( cardNo, dbID,
                                     sysNotifyVolumeUnmountedEvent,
                                     sysNotifyNormalPriority );
            }
        }
    }
}



/* Handle notification is ran each time a registered event is cought
   and passed back to anyone who is listening. */
static void HandleNotification
    (
    SysNotifyParamType* cmdPBP   /* pointer to command parameter block */
    )
{
    if ( HandleResizeNotification( cmdPBP->notifyType ) )
        return;
    switch ( cmdPBP->notifyType ) {
#ifdef HAVE_SONY_SDK
        case sonySysNotifyHoldStatusChangeEvent:
            DoAutoscrollToggle( AUTOSCROLL_OFF );
            break;
#endif
        case sysNotifyVolumeMountedEvent:
        case sysNotifyVolumeUnmountedEvent:
            /* Clear the Doc Cache, Rebuild the List, then open the Library */
            CloseDocList();
            ReleaseDocInfoList();
            FrmGotoForm( GetValidForm( frmLibrary ) );

            /* We don't want it to load up any other new programs,
               Stay right here... */
            cmdPBP->handled |= vfsHandledUIAppSwitch;
            cmdPBP->handled |= vfsHandledStartPrc;
            break;

        default:
            break;
    }
}



/* This routine handles the VChrs: HW buttons, power off, low battery
   dialogs, etc */
static Boolean HandleVChrs
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;
    UInt32  oldMask;

    handled = false;

    /* Hardware keys are useful while plucker is running, but if you turn
       on a device via its hardware key, the plucker related function may
       run rather than the intended button. That can be confusing. Instead,
       lets honour our own command keys only if the device is already on. */
    if ((event->data.keyDown.modifiers & poweredOnKeyMask) != 0 &&
        (event->data.keyDown.chr == vchrHard1 ||
         event->data.keyDown.chr == vchrHard2 ||
         event->data.keyDown.chr == vchrHard3 ||
         event->data.keyDown.chr == vchrHard4))
        return handled;

    oldMask = KeySetMask( commandKeyMask );
    if ( IsMainFormWinActive() || IsFullscreenformActive() ) {
        switch ( event->data.keyDown.chr ) {
            /* Disable Japanese henkan-keys and keyboard. */
            case vchrTsm1:
            case vchrTsm2:
            case vchrTsm3:
            case vchrTsm4:
            case vchrKeyboard:
            case vchrKeyboardAlpha:
            case vchrKeyboardNumeric:
                if ( Prefs()->gestMode[ GESTURES_TAP ] != SELECT_NONE )
                    DoSelectTypeAction( Prefs()->gestMode[ GESTURES_TAP ] );
                handled = true;
                break;

            case vchrHard1:
                if ( Prefs()->hardKeys &&
                     Prefs()->hwMode[ DATEBOOK_BUTTON ] != SELECT_NONE ) {
                    DoSelectTypeAction( Prefs()->hwMode[ DATEBOOK_BUTTON ] );
                    handled = true;
                }
                break;

            case vchrHard2:
                if ( Prefs()->hardKeys &&
                     Prefs()->hwMode[ ADDRESS_BUTTON ] != SELECT_NONE ) {
                    DoSelectTypeAction( Prefs()->hwMode[ ADDRESS_BUTTON ] );
                    handled = true;
                }
                break;

            case vchrHard3:
                if ( Prefs()->hardKeys &&
                     Prefs()->hwMode[ TODO_BUTTON ] != SELECT_NONE ) {
                    DoSelectTypeAction( Prefs()->hwMode[ TODO_BUTTON ] );
                    handled = true;
                }
                break;

            case vchrHard4:
                if ( Prefs()->hardKeys &&
                     Prefs()->hwMode[ MEMO_BUTTON ] != SELECT_NONE ) {
                    DoSelectTypeAction( Prefs()->hwMode[ MEMO_BUTTON ] );
                    handled = true;
                }
                break;

            case vchrLowBattery:
            {
                /* OS 3.3 seems to be sending events to itself for
                   internal purposes -- make sure it really is a
                   low battery situation */
                if ( ! HasEnoughPower() ) {
                    DoAutoscrollToggle( AUTOSCROLL_OFF );
                }
                break;
            }

            case vchrMenu:
            case vchrCommand:
            case vchrHardPower:
            case vchrAutoOff:
            case vchrLock:
                DoAutoscrollToggle( AUTOSCROLL_OFF );
                break;

#ifdef RETURN_TO_PREV_APP
            case vchrLaunch:
                DoAutoscrollToggle( AUTOSCROLL_OFF );
                SendAppStopEvent();
                handled = true;
                break;
#endif

            default:
                break;
        }
    }
    else if ( FrmGetActiveFormID() == frmExternalLinks && Prefs()->hardKeys ) {
        /* Convert HW button go back action into go back event for
           external links form */
        switch ( event->data.keyDown.chr ) {
            case vchrHard1:
                if ( Prefs()->hwMode[ DATEBOOK_BUTTON ] == SELECT_GO_BACK ) {
                    handled = true;
                }
                break;

            case vchrHard2:
                if ( Prefs()->hwMode[ ADDRESS_BUTTON ] == SELECT_GO_BACK ) {
                    handled = true;
                }
                break;

            case vchrHard3:
                if ( Prefs()->hwMode[ TODO_BUTTON ] == SELECT_GO_BACK ) {
                    handled = true;
                }
                break;

            case vchrHard4:
                if ( Prefs()->hwMode[ MEMO_BUTTON ] == SELECT_GO_BACK ) {
                    handled = true;
                }
                break;

            default:
                handled = false;
                break;
        }
        if ( handled ) {
            event->eType                    = ctlSelectEvent;
            event->data.ctlEnter.controlID  = frmExternalLinksBack;
            handled                         = false;
        }
    }
    oldMask = KeySetMask( oldMask );
    return handled;
}



/* This routine handles the tap and upstroke button actions */
static Boolean HandleShiftGestures( void )
{
    Boolean handled;
    Boolean empty;
    UInt16  tempShift;
    MenuBarType* activeMenu;

    handled     = false;
    empty       = false;
    tempShift   = false;

    /* if the user has entered a command stroke for a menu
       shortcut then we skip the gestures handling */
    activeMenu = MenuGetActiveMenu();
    if ( activeMenu != NULL )
        return false;

    /* Most of the functions we use here have been deprecated in Graffiti 2 */
    if ( SupportGraffiti2() )
        /* FIXME: there's no way yet to handle the Graffiti 2 version of
           grfTempShiftPunctuation (for GESTURES_UP calls)... yet :( */
        return false;

    if ( IsMainFormWinActive() || IsFullscreenformActive() ) {
        GrfGetState( &empty, &empty, &tempShift, &empty );
        switch ( tempShift ) {
            case 0:
                /* speed gain if evaluate no shift state first and don't
                   clean graffiti state? */
                break;

            case grfTempShiftUpper:
                if ( Prefs()->gestMode[ GESTURES_UP ] != SELECT_NONE )
                    DoSelectTypeAction( Prefs()->gestMode[ GESTURES_UP ] );
                GrfSetState( 0, 0, 0 );
                handled = true;
                break;

            case grfTempShiftPunctuation:
                if ( Prefs()->gestMode[ GESTURES_TAP ] != SELECT_NONE )
                    DoSelectTypeAction( Prefs()->gestMode[ GESTURES_TAP ] );
                GrfSetState( 0, 0, 0 );
                handled = true;
                break;

            default:
                /* one of the other current or future shift states */
                GrfSetState( 0, 0, 0 );
                handled = true;
                break;
        }
    }
    return handled;
}



/* This routine handles the loading of forms */
static void HandleFormLoad
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    FormType*       form;
    UInt16          formID;
    EventHandler*   list;

    formID  = event->data.frmLoad.formID;
    form    = FrmInitForm( formID );
    if ( form != NULL ) {
        FrmSetActiveForm( form );

        for ( list = ListOfEventHandlers; list->formID != 0; list++ ) {
            if ( list->formID == formID ) {
                SetResizePolicy( formID );
                FrmSetEventHandler( form, list->handler );
                return;
            }
        }
    }
    ErrFatalDisplay( "Unknown form ID" );
}



/* This routine handles timeout calculation for the autoscroll */
static UInt16 CalculateAutoscrollTimeout( void )
{
    UInt16 now;
    UInt16 interval;
    UInt16 elapsed;
    UInt16 timeToWait;
    UInt16 lastScrollTime;

    now             = TimGetTicks();
    interval        = Prefs()->autoscrollInterval;
    lastScrollTime  = Prefs()->autoscrollLastScrollTime;
    
    if ( lastScrollTime < now )
        elapsed = now - lastScrollTime;
    else
        elapsed = 0;

    if ( elapsed < interval )
        timeToWait = interval - elapsed;
    else
        timeToWait = 0;

    return timeToWait;
}



/* This routine is the event loop for the viewer application */
static void EventLoop( void )
{
    EventType   event;
    UInt16      err;
    UInt32      waitTicks;

    do {
        if ( TimeoutGet( &waitTicks ) ) {
            if (waitTicks < 1) waitTicks = 1;
            EvtGetEvent( &event, waitTicks );
        }
        /* FIXME: Move autoscroll into Timeout API */
        else if ( Prefs()->autoscrollEnabled ) {
            UInt32 w = CalculateAutoscrollTimeout();
            if (w < 1) w = 1;
            EvtGetEvent( &event, w );
        } else
            EvtGetEvent( &event, evtWaitForever );

        if ( event.eType == keyDownEvent && HandleVChrs( &event ) )
            continue;

#if 0
        /* This pauses autoscroll drawing when mainform window
           is obscured (eg menu). Remove if already comfortably handled
           with the vchrs handling for menu silkscreen, etc. Don't seem
           to work though. */
        if (event.eType == winExitEvent) {
            if ( event.data.winExit.exitWindow ==
                 (WinHandle) FrmGetFormPtr( GetMainFormId() ) )
                DoAutoscrollToggle( AUTOSCROLL_OFF );
            continue;
        }
#endif

        /* SysHandleEvent returns true if system handled events: keys
           open/close events, key up downs, graffiti strokes */
        if ( SysHandleEvent( &event ) ) {
            if ( event.eType == penUpEvent && Prefs()->gestures ) {
                if ( HandleShiftGestures() )
                    continue;
            }
            else {
                continue;
            }
        }

        if ( MenuHandleEvent( NULL, &event, &err ) )
            continue;

        if ( event.eType == frmLoadEvent )
            HandleFormLoad( &event );

        ErrTry {
            if ( waitTicks != 0 )
                TimeoutEventHandler();
            FrmDispatchEvent( &event );
        }
        ErrCatch( err ) {
            switch ( err ) {
                case memErrNotEnoughSpace :
                case dmErrMemError :
                    FrmAlert( warnInsufficientMemory );
                    event.eType = appStopEvent;
                    break;

                case exgMemError:
                case exgErrNoReceiver:
                case exgErrUserCancel:
                case exgErrDeviceFull:
                    /* the OS will display an alert for these "errors" */
                    /* we continue running, though */
                    break;

                default:
                {
                    Char errorCode[ MAX_ERROR_CODE_LEN ];

                    StrIToA( errorCode, err );
                    FrmCustomAlert( errUnhandledException, errorCode, NULL,
                        NULL );
                    event.eType = appStopEvent;
                    break;
                }
            }
        } ErrEndCatch
    } while ( event.eType != appStopEvent );
}



/* This routine closes the document and saves the current state of the
   application */
static void StopApplication( void )
{
    EnableNotification( UNREGISTER );

    /* Save in the prefs what our silkscreen is set to, so its status is
       restored whenever plucker's loaded again */

    FrmSaveAllForms();
    FrmCloseAllForms();

    ImageFinish();
    RemoveCache();
    CloseDocument();
    ReleaseLastDocInfo();
    ReleaseDocInfoList();
    CloseDocList();
    ReleaseSearchQueue();
    DeleteUncompressBuffer();
    WritePrefs();
    SaveResizePrefs( ( UInt32 )ViewerAppID, ( UInt16 )ViewerPrefDIA,
        ( Int16 )ViewerVersion );
    TerminateResizeSupport();
    CloseKeyboardMap();
    AnchorListRelease();
    ReleaseActionList();
    ReleaseKeyList();
    CloseTransliterations();
    GrayFntStop();

    DeleteInternalMemoDB();

    OS_Release();
}

void select_utf8(int on) {
  language_t *lang;

  if (on) {
    lang = LanguageSelect(utf8_lang);
    if (old_lang == NULL) old_lang = lang;
  } else {
    LanguageSelect(old_lang);
  }
}


#ifndef HAVE_PALMCUNIT
/* This is the main entry point for the viewer application */
UInt32 PilotMain
    (
    UInt16 cmd,         /* SysAppLaunch Command */
    MemPtr cmdPBP,      /* pointer to command parameter block */
    UInt16 launchFlags  /* launch flag ( see SystemMgr.h for details ) */
    )
{
    Err err;

    err = errNone;
    if ( cmd == sysAppLaunchCmdNormalLaunch ) {
        err = StartApplication( NULL );
        if ( err != errNone )
            return err;

        utf8_lang = LanguageInit(langUTF8);
        EventLoop();
        LanguageSelect(NULL);
        LanguageFinish(utf8_lang);
        utf8_lang = NULL;
        StopApplication();
    }
    else if ( cmd == sysAppLaunchCmdOpenDB ) {
        err = StartApplication( cmdPBP );
        if ( err != errNone )
            return err;

        EventLoop();
        StopApplication();
    }
    else if ( cmd == sysAppLaunchCmdSyncNotify ) {
        UInt32 romVersion;

        FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
        if ( 0x04000000 <= romVersion )
            ExgRegisterDatatype( ViewerAppID, exgRegTypeID, ViewerMIMEType, NULL, 0);
        else if ( 0x03000000 <= romVersion )
            ExgRegisterData( ViewerAppID, exgRegTypeID, ViewerMIMEType);
    }
    else if ( cmd == sysAppLaunchCmdExgAskUser ) {
        /* use default behavior, i.e. always ask whether to accept */
    }
    else if ( cmd == sysAppLaunchCmdExgReceiveData ) {
        Boolean globalsAvailable;

        if ( ( launchFlags & sysAppLaunchFlagSubCall ) != 0 )
            globalsAvailable = true;
        else
            globalsAvailable = false;

        err = ReceiveRAMDocument( (ExgSocketPtr)cmdPBP, globalsAvailable );
    }
    else if ( cmd == sysAppLaunchCmdNotify ) {
        HandleNotification( (SysNotifyParamType*) cmdPBP );
    }

    return err;
}
#endif

