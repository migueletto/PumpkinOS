/*
 * $Id: prefsform.c,v 1.72 2004/05/08 09:04:54 nordstrom Exp $
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
#include "jogdial.h"
#include "prefsdata.h"
#include "prefsgeneral.h"
#include "prefsbrowsing.h"
#include "prefslibrary.h"
#include "prefsautoscroll.h"
#include "prefshardcopy.h"
#include "prefsbutton.h"
#include "prefscontrol.h"
#include "prefsgesture.h"
#include "prefsjogdial.h"
#include "prefslookup.h"
#include "resourceids.h"
#include "util.h"
#include "os.h"
#include "DIA.h"

#include "prefsform.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define START_SECTION    0
#define NUM_OF_SECTIONS  10


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef struct {
    Boolean (*PreferenceEvent)( ActionType action );
    Boolean (*PalmEvent)( EventType* event );
    UInt16  helpID;
} HandlerType;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void PrefsFormInit( void ) PREFSFORM_SECTION;
static void PrefsClearSection( UInt16 section ) PREFSFORM_SECTION;
static void PrefsSaveSections( void ) PREFSFORM_SECTION;
static void PrefsReleaseSections( void ) PREFSFORM_SECTION;
static void PrefsSetSection( Int16 setSection ) PREFSFORM_SECTION;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FormType*        prefsForm;
static UInt16           openedPrefs;
static UInt16           handlerMap[ NUM_OF_SECTIONS ];
static Int16            totalSections;
static Int16            currentSection;
static MemHandle        sectionHandle;
static Char             sectionList[ 255 ];
static UInt16           mainObjectList[][ MAX_OBJECTS ] = {
    { frmPrefsGeneralMenuToolbarLabel, frmPrefsGeneralMenuToolbarPopup,
      frmPrefsGeneralToolbarModeLabel, frmPrefsGeneralToolbarModePopup,
      frmPrefsGeneralScreenDepthLabel,
      frmPrefsGeneralScreenDepthPopup,
      frmPrefsGeneralScrollbarLabel, frmPrefsGeneralScrollbarPopup, EOL },
    { frmPrefsBrowsingStrikethrough, frmPrefsBrowsingUnderline, 
      frmPrefsBrowsingPageControlsLink, frmPrefsBrowsingEnableSoftHyphens,
      frmPrefsBrowsingForceDefaultColors, frmPrefsBrowsingAlignLabel,
      frmPrefsBrowsingAlignPopup, frmPrefsBrowsingAlignList, EOL },
    { frmPrefsLibrarySyncPolicyLabel, frmPrefsLibrarySyncPolicyPopup,
      frmPrefsLibrarySyncPolicyList, frmPrefsLibraryCategoryStyleLabel,
      frmPrefsLibraryCategoryStylePopup, frmPrefsLibrarySortByLabel,
      frmPrefsLibrarySortByPopup, frmPrefsLibrarySortOrderLabel,
      frmPrefsLibrarySortOrderPopup, frmPrefsLibraryDateTime,
      frmPrefsLibraryShowType, frmPrefsLibraryShowDate,
      frmPrefsLibraryShowSize, frmPrefsLibraryIndicateOpened, EOL },
    { frmPrefsAutoscrollJumpLabel, frmPrefsAutoscrollJumpButton,
      frmPrefsAutoscrollJumpUpButton, frmPrefsAutoscrollJumpDownButton,
      frmPrefsAutoscrollModePopup, frmPrefsAutoscrollDirLabel,
      frmPrefsAutoscrollDirPopup, frmPrefsAutoscrollIntervalLabel,
      frmPrefsAutoscrollIntervalButton, frmPrefsAutoscrollIntervalUpButton,
      frmPrefsAutoscrollIntervalDownButton, frmPrefsAutoscrollMillisecondsLabel,
      frmPrefsAutoscrollStayOn, EOL },
    { frmPrefsHardcopyActionLabel, frmPrefsHardcopyActionPopup,
      frmPrefsHardcopyRangeLabel, frmPrefsHardcopyRangePopup,
      frmPrefsHardcopyLinkLabel, frmPrefsHardcopyLinkPopup,
      frmPrefsHardcopyTitle,
      EOL },
    { frmPrefsButtonHardKeys, frmPrefsButtonArrowKeys,
      frmPrefsButtonSelectAction, EOL },
    { frmPrefsControlMode1, frmPrefsControlMode2, frmPrefsControlMode3,
      frmPrefsControlRegionLabel, EOL },
    { frmPrefsGestureGestures, frmPrefsGestureSelectAction, frmPrefsGestureUp,
      bmpGestureUp, frmPrefsGestureLeft, bmpGestureLeft, frmPrefsGestureTap,
      bmpGestureTap, frmPrefsGestureRight, bmpGestureRight,
      frmPrefsGestureDown, bmpGestureDown, EOL },
    { frmPrefsJogdialJogEnabled, frmPrefsJogdialSelectAction, EOL },
    { frmPrefsLookupAlwaysActive, frmPrefsLookupActionList,
      frmPrefsLookupActionPopup, frmPrefsLookupActionLabel, EOL }
};
static HandlerType      handlerList[ NUM_OF_SECTIONS ] = {
    { PrefsGeneralPreferenceEvent, PrefsGeneralPalmEvent, strPrefsGeneralHelp },
    { PrefsBrowsingPreferenceEvent, PrefsBrowsingPalmEvent,
          strPrefsBrowsingHelp },
    { PrefsLibraryPreferenceEvent, PrefsLibraryPalmEvent, strPrefsLibraryHelp },
    { PrefsAutoscrollPreferenceEvent, PrefsAutoscrollPalmEvent,
          strPrefsAutoscrollHelp },
    { PrefsHardcopyPreferenceEvent, PrefsHardcopyPalmEvent,
          strPrefsHardcopyHelp },
    { PrefsButtonPreferenceEvent, PrefsButtonPalmEvent, strPrefsButtonHelp },
    { PrefsControlPreferenceEvent, PrefsControlPalmEvent, strPrefsControlHelp },
    { PrefsGesturePreferenceEvent, PrefsGesturePalmEvent, strPrefsGestureHelp },
    { PrefsJogdialPreferenceEvent, PrefsJogdialPalmEvent, strPrefsJogdialHelp },
    { PrefsLookupPreferenceEvent, PrefsLookupPalmEvent, strPrefsLookupHelp },
};



/* Initialize the preference form */
static void PrefsFormInit( void )
{
    ListType*       list;
    ControlType*    ctl;
    Char**          sections;
    UInt16          i;
    Char*           temp;
    Char            formTitle[ 30 ];
#ifdef HAVE_HANDSPRING_SDK
    FrmNavStateFlagsType navStateFlags;
    Err                  navErr;
#endif

    prefsForm = FrmGetFormPtr( frmPrefs );

#ifdef HAVE_HANDSPRING_SDK
    if ( HaveHsNav() )
    {
        /* Get the current nav state flags for the form and
           enable object focus mode */
        navErr = FrmGetNavState ( prefsForm, &navStateFlags );
        if ( navErr == errNone )
        {
            navStateFlags |= kFrmNavStateFlagsObjectFocusMode;
            FrmSetNavState ( prefsForm, navStateFlags );
        }
    }
#endif
    
    currentSection = START_SECTION;

    MemSet( &sectionList, sizeof( sectionList ), 0 );
    totalSections = 0;
    temp = sectionList;
    for ( i = 0; i < NUM_OF_SECTIONS; i++ ) {
        if ( handlerList[ i ].PreferenceEvent( AVAILABLE ) ) {
            if ( handlerList[ i ].PreferenceEvent( SHOWFIRST ) )
                currentSection = totalSections;
            SysCopyStringResource( temp, strPrefsGeneral + i );
            temp += StrLen( temp ) + 1;
            handlerMap[ totalSections++ ] = i;
        }
    }

    list        = GetObjectPtr( frmPrefsSectionList );
    ctl         = GetObjectPtr( frmPrefsSectionPopup );

    sectionHandle = SysFormPointerArrayToStrings( sectionList,
        totalSections );
    sections      = MemHandleLock( sectionHandle );

    LstSetListChoices( list, sections, totalSections );
    LstSetHeight( list, totalSections );
    LstSetSelection( list, currentSection );
    CtlSetLabel( ctl, LstGetSelectionText( list, currentSection ) );

    PrefsShowSection( mainObjectList, currentSection );

    FrmDrawForm( prefsForm );

    /* Move the title to the right, to make room for the help icon */
    StrPrintF( formTitle, "      %s", FrmGetTitle( prefsForm ) );
    FrmSetTitle( prefsForm, formTitle );

    DrawHelpIcon( prefsForm, frmPrefsHelp );

    openedPrefs = 0;
    PrefsOpenSection( currentSection );
}



/* Open a specific section onto the form */
void PrefsOpenSection
    (
    UInt16 section
    )
{
    UInt16 mappedSection;

    mappedSection = handlerMap[ section ];

    /* LOADing a section should only be run the first time a preference section
       is opened. We should only need to initialize the controls and the
       parameters once ... */
    if ( ! ( openedPrefs & 1 << section ) ) {
        openedPrefs |= 1 << section;
        handlerList[ mappedSection ].PreferenceEvent( LOAD );
    }

    /* ... but run DISPLAY every time */
    handlerList[ mappedSection ].PreferenceEvent( DISPLAY );
}



/* Run the CLEAR action on this section */
static void PrefsClearSection
    (
    UInt16 section
    )
{
    UInt16 mappedSection;

    mappedSection = handlerMap[ section ];
    handlerList[ mappedSection ].PreferenceEvent( CLEAR );
}



/* Run the SAVE action, but only on the sections that have been viewed */
static void PrefsSaveSections( void )
{
    UInt16 i;
    UInt16 mappedSection;

    /* Doing this prevents accidently saving parameters that were never
       properly initialized to begin with */
    for ( i = 0; i < NUM_OF_SECTIONS; i++ ) {
        mappedSection = handlerMap[ i ];
        if ( openedPrefs & 1 << i )
            handlerList[ mappedSection ].PreferenceEvent( SAVE );
    }
}



static void PrefsReleaseSections( void )
{
    UInt16 i;

    if ( sectionHandle != NULL ) {
        MemHandleUnlock( sectionHandle );
        MemHandleFree( sectionHandle );
        sectionHandle = NULL;
    }

    for ( i = 0; i < NUM_OF_SECTIONS; i++ )
        handlerList[ handlerMap[ i ] ].PreferenceEvent( RELEASE );
}



/* Hide specific objects from the display based upon provided list */
void PrefsHideSection
    (
    UInt16 list[][ MAX_OBJECTS ],
    UInt16 section
    )
{
    UInt16 i;
    UInt16 mappedSection;

    mappedSection = handlerMap[ section ];
    for ( i = 0; list[ mappedSection ][ i ] != EOL; i++ )
        FrmHideObject( prefsForm, FrmGetObjectIndex( prefsForm,
            list[ mappedSection ][ i ] ) );

}



/* Show specific objects from the display based upon provided list */
void PrefsShowSection
    (
    UInt16 list[][ MAX_OBJECTS ],
    UInt16 section
    )
{
    UInt16 i;
    UInt16 mappedSection;

    mappedSection = handlerMap[ section ];
    for ( i = 0; list[ mappedSection ][ i ] != EOL; i++ )
        FrmShowObject( prefsForm, FrmGetObjectIndex( prefsForm,
            list[ mappedSection ][ i ] ) );

}



/* Set the display to match the list/popup selection */
void SetListToSelection
    (
    UInt16 listID,
    UInt16 popupID,
    UInt16 selection
    )
{
    ListType*    list;
    ControlType* ctl;

    list = GetObjectPtr( listID );
    ctl  = GetObjectPtr( popupID );
    LstSetSelection( list, selection );
    CtlSetLabel( ctl, LstGetSelectionText( list, selection ) );
}



/* Open the popup for the SelectType lists, and return the users' new value */
SelectType SetActionForSelectedButton
    (
    UInt16    listObjectID,
    UInt16    controlID
    )
{
    ListType*     list;
    ControlType*  ctl;
    UInt16        selection;

    list        = GetObjectPtr( listObjectID );
    selection   = LstPopupList( list );
    if ( selection != noListSelection ) {
        Char* label;

        ctl   = GetObjectPtr( controlID );
        label = LstGetSelectionText( list, selection );
        CtlSetLabel( ctl, label );
        LstSetSelection( list, selection );
        return selection;
    }
    return LstGetSelection( list );
}



/* Ensure everything looks as it should */
void AffirmControlImage
    (
    UInt16 selected,
    UInt16 firstCtlObjectID,
    UInt16 firstImageObjectID
    )
{
    UInt16 ctlObjectID;
    UInt16 imageObjectID;

    ctlObjectID   = selected + firstCtlObjectID;
    imageObjectID = selected + firstImageObjectID;

    /* When dealing with images being displayed overtop of pushbuttons,
       sometimes the pushbutton itself will be drawn overtop the image. Usually
       this doesn't bother us except when the pushbutton is already set as
       selected.

       The end result is a pushbutton that is already inverted, with an
       un-inverted image overtop. Then when the user clicks another pushbutton,
       you inadvertenly end up with two inverted image. The following code
       fixes the ordering of things. */
    CtlSetValue( GetObjectPtr( ctlObjectID ), false );
    FrmShowObject( prefsForm, FrmGetObjectIndex( prefsForm, imageObjectID ) );
    CtlSetValue( GetObjectPtr( ctlObjectID ), true );
}



/* Return the value for currentSection */
UInt16 GetCurrentSection( void )
{
    return currentSection;
}



/* Set a new valie for currentSection */
void SetCurrentSection
    (
    UInt16 thisSection
    )
{
    currentSection = thisSection;
}



/* Goto the previous section in the preferences */
void PrefsPreviousSection( void )
{
    PrefsSetSection( currentSection - 1 );
}



/* Goto the next section in the preferences */
void PrefsNextSection( void )
{
    PrefsSetSection( currentSection + 1 );
}



/* Goto the specified preference section */
static void PrefsSetSection
    (
    Int16 setSection
    )
{
    ListType*     list;
    ControlType*  ctl;

    if ( totalSections - 1 < setSection )
        setSection = 0;
    else if ( setSection < 0 )
        setSection = totalSections - 1;

    /* Clear off the currently open section */
    PrefsClearSection( currentSection );
    PrefsHideSection( mainObjectList, currentSection );

    /* Open the newly requested section */
    PrefsShowSection( mainObjectList, setSection );
    PrefsOpenSection( setSection );

    list = GetObjectPtr( frmPrefsSectionList );
    ctl  = GetObjectPtr( frmPrefsSectionPopup );
    LstSetSelection( list, setSection );
    CtlSetLabel( ctl, LstGetSelectionText( list, setSection ) );

    currentSection = setSection;
}



/* Event handler for the preference form */
Boolean PrefsFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;
    UInt16  mappedSection;

    handled     = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmPrefsOK:
                    PrefsSaveSections();
                    /* FALLTHROUGH */

                case frmPrefsCancel:
                    PrefsReleaseSections();
                    FrmGotoForm( Prefs()->lastForm );
                    handled = true;
                    break;

                case frmPrefsHelp:
                    mappedSection = handlerMap[ currentSection ];
                    FrmHelp( handlerList[ mappedSection ].helpID );
                    handled = true;
                    break;

                default:
                    break;
            }
            break;

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
                    case frmPrefsSectionPopup:
                        PrefsSetSection( selection );
                        handled = true;
                        break;

                    default:
                        break;
                }
            }
            break;
        }
        
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
            PrefsFormInit();
            handled = true;
            break;

        case frmCloseEvent:
            PrefsReleaseSections();
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            handled = false;
            break;

        default:
            handled = false;
    }

    if ( ! handled )
        handled = JogdialPrefHandler( event );

    /* If the event still hasn't been handled, dispatch it
       to the individual sections' event handlers */
    if ( ! handled ) {
        mappedSection = handlerMap[ currentSection ];
        handled = handlerList[ mappedSection ].PalmEvent( event );
    }

    return handled;
}



/* The last two functions are here so that we can build the preferences safely
   regardless of the fact that we have any required third-party SDKs. One such
   example that uses this is the jogdial section.

   Since we may or may not be building prefsjogdial.c, prefsjogdial.h still
   needs to identify if HAVE_JOGDIAL is enabled or not. If it is then
   everything is fine, but if not we redefine the 'event functions' to point to
   the 'NULL event functions' as detailed below. Doing so simply returns the
   equivilant of 'handled = false' and everything continues happily.

   Mostly this is useful during the line:

       if ( handlerList[ i ].PreferenceEvent( AVAILABLE ) ) { ... }

   Since that we don't have the SDK, the PrefsNULLPreferenceEvent takes over
   and simply returns false. It is then kept out of the list and everything
   continues along happily.

   Another way of doing the same thing may be to simply comment out the
   references to the jogdial functions in mainObjectList and handlerList. This
   may not be such a bad idea because the jogdial is at the bottom of each
   list; but if it were somewhere in the middle and/or we were commenting more
   than one section out because of a lack of SDKs, bad things may happen
   because of the mis-alignment.

   Doing it this way is a better long-term solution, and is easy to extend to
   other future sections. See prefsjogdial.h for on how its actually done.
 */



/* Failover function to handle missing sections */
Boolean PrefsNULLPreferenceEvent
    (
    ActionType action
    )
{
    return false;
}



/* Failover function to handle missing sections */
Boolean PrefsNULLPalmEvent
    (
    EventType* event
    )
{
    return false;
}
