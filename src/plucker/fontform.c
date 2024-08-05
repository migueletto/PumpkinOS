/*
 * $Id: fontform.c,v 1.41 2004/05/08 09:04:53 nordstrom Exp $
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
#include "font.h"
#include "fullscreenform.h"
#include "hires.h"
#include "jogdial.h"
#include "prefsdata.h"
#include "resourceids.h"
#include "os.h"
#include "util.h"
#include "history.h"
#include "grayfont.h"
#include "metadocument.h"
#include "mainform.h"

#include "fontform.h"
#include "../libpit/debug.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define FONT_BUTTONS        1

static const Char letter  = 'A';
#define BUTTON_COUNT         6
#define BUTTON_COUNT_OS2     2
#define BUTTON_COUNT_HANDERA 5

#define NO_USER_FONT_BUTTON  -1

/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
typedef enum {
    LASTFORM_MAIN = 0,
    LASTFORM_LIBRARY
} LastFormType;

typedef struct {
    FontModeType fontMode;
    FontID       fontID;
    UInt16       objID;
    ControlType* control;
} ButtonType;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static FormType*    form;
static UInt16       fontFormID;
static ButtonType*  buttonList;
static UInt8        buttonCount;
static UInt8        selected;
static FontModeType fontModePref;
static Int16        userFontSelection;
static Int16        lineSpacing;
static Int16        paragraphSpacing;
static Int16        userFontButtonNumber;
static Int16        oldUserFontNumber;
static Char*        prefsUserFontName;
static Boolean      individualDocumentFonts;
static Boolean      cameFromLibrary;
#ifdef HAVE_ROTATE
static RotateType   rotate;
#endif


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void FontFormInit( void ) FONTFORM_SECTION;

static void HideFontFormObject( UInt16 id, Boolean isControl ) FONTFORM_SECTION;

static void ShowFontFormObject( UInt16 id, Boolean isControl ) FONTFORM_SECTION;

UInt8 FontGetSelected(void)
{
    return selected;
}


UInt8 FontGetButtonCount(void)
{
    return buttonCount;
}


void FontSelectNextFont(void)
{
    FrmSetControlGroupSelection( form, 1,
        buttonList[ ++selected ].objID );
}


void FontSelectPreviousFont(void)
{
    FrmSetControlGroupSelection( form, 1,
        buttonList[ --selected ].objID );
}

/* Draw or redraw the font selection buttons */
static void DrawButtons
    (
    Boolean redraw   /* is this a first drawing or a redrawing? */
    )
{
    FontID          prevFont;
    UInt16          i;

    if ( redraw && userFontButtonNumber == NO_USER_FONT_BUTTON )
        return;

    prevFont     = FntGetFont();

    if ( userFontButtonNumber != NO_USER_FONT_BUTTON ) {
        LoadUserFont( userFontSelection );
        RefreshCustomFonts();
    }

    /* Display the letters in the middle of each pushbutton */
    for ( i = 0; i < buttonCount; i++ ) {
        buttonList[ i ].control = GetObjectPtr( buttonList[ i ].objID );
        if ( redraw ) {
            CtlSetValue( buttonList[ i ].control, false );
            CtlEraseControl( buttonList[ i ].control );
            CtlDrawControl( buttonList[ i ].control );
        }
        DisplayChar( buttonList[ i ].fontID, letter, form,
            buttonList[ i ].objID );
    }

    if ( userFontButtonNumber != NO_USER_FONT_BUTTON ) {
        LoadUserFont( oldUserFontNumber );
        RefreshCustomFonts();
    }

    FntSetFont( prevFont );

    FrmSetControlGroupSelection( form, FONT_BUTTONS,
        buttonList[ selected ].objID );
}



/* Hide the object */
static void HideFontFormObject
    (
    UInt16 id,    /* object id */
    Boolean isControl /* is this a control to be disabled? */
    )
{
    FormType *form;

    form = FrmGetFormPtr( fontFormID );
    FrmHideObject( form, FrmGetObjectIndex( form, id ) );
    if ( isControl ) CtlSetUsable( GetObjectPtr( id ), false );
}



/* Show the object */
static void ShowFontFormObject
    (
    UInt16 id,    /* object id */
    Boolean isControl /* is this a control to be enabled? */
    )
{
    FormType *form;

    form = FrmGetFormPtr( fontFormID );
    FrmShowObject( form, FrmGetObjectIndex( form, id ) );
    if ( isControl ) CtlSetUsable( GetObjectPtr( id ), true );
}


/* Initialize the font form */
static void FontFormInit( void )
{
    UInt16          i;
    ControlType*    ctl;
    ListType*       list;

    if ( IsFormMain( Prefs()->lastForm ) ) {
        cameFromLibrary   = false;
        fontModePref      = Prefs()->fontModeMain;
        prefsUserFontName = Prefs()->mainUserFontName;
    }
    else if ( IsFormLibrary( Prefs()->lastForm ) ) {
        cameFromLibrary      = true;
        fontModePref         = Prefs()->fontModeLibrary;
        prefsUserFontName    = Prefs()->libraryUserFontName;
    }
    else {
        FontDoCancel();
        return;
    }

    fontFormID = GetValidForm( frmFont );
    if ( fontFormID == frmFont )
        buttonCount = BUTTON_COUNT;
#ifdef HAVE_HANDERA_SDK
    else if ( fontFormID == frmFontHandera )
        buttonCount = BUTTON_COUNT_HANDERA;
#endif
    else
        buttonCount = BUTTON_COUNT_OS2;
        
#ifdef SUPPORT_VFS_FONTS
    ScanVFSFonts();
#endif

    form        = FrmGetFormPtr( fontFormID );
    buttonList  = SafeMemPtrNew( buttonCount * sizeof *buttonList );

    if ( fontFormID == frmFontOS2 ) {
        buttonList[0].fontMode = FONT_DEFAULT;
        buttonList[1].fontMode = FONT_BOLD;
        buttonList[0].fontID   = stdFont;
        buttonList[1].fontID   = boldFont;
        buttonList[0].objID    = frmFont1;
        buttonList[1].objID    = frmFont2;
        userFontButtonNumber   = NO_USER_FONT_BUTTON;
#ifdef HAVE_HANDERA_SDK
    }
    else if ( fontFormID == frmFontHandera ) {
        buttonList[0].fontMode = FONT_NARROW;
        buttonList[1].fontMode = FONT_TINY;
        buttonList[2].fontMode = FONT_SMALL;
        buttonList[3].fontMode = FONT_DEFAULT;
        buttonList[4].fontMode = FONT_LARGE;
        buttonList[0].fontID   = narrowFont;
        buttonList[1].fontID   = stdFont;
        buttonList[2].fontID   = largeFont;
        buttonList[3].fontID   = VgaBaseToVgaFont( stdFont );
        buttonList[4].fontID   = VgaBaseToVgaFont( largeFont );
        buttonList[0].objID    = frmFont1;
        buttonList[1].objID    = frmFont2;
        buttonList[2].objID    = frmFont3;
        buttonList[3].objID    = frmFont4;
        buttonList[4].objID    = frmFont5;
        userFontButtonNumber   = NO_USER_FONT_BUTTON;
#endif
#ifdef HAVE_HIRES
    }
    else if ( IsHiResTypePalm( HiResType() ) ) {
        buttonList[0].fontMode = FONT_TINY;
        buttonList[1].fontMode = FONT_SMALL;
        buttonList[2].fontMode = FONT_NARROW;
        buttonList[3].fontMode = FONT_DEFAULT;
        buttonList[4].fontMode = FONT_LARGE;
        buttonList[5].fontMode = FONT_USER;
        buttonList[0].fontID   = tinyFont_palm;
        buttonList[1].fontID   = smallFont_palm;
        buttonList[2].fontID   = narrowFont;
        buttonList[3].fontID   = stdFont;
        buttonList[4].fontID   = largeFont;
        buttonList[5].fontID   = userStdFont_palm;
        buttonList[0].objID    = frmFont1;
        buttonList[1].objID    = frmFont2;
        buttonList[2].objID    = frmFont3;
        buttonList[3].objID    = frmFont4;
        buttonList[4].objID    = frmFont5;
        buttonList[5].objID    = frmFontUser;
        userFontButtonNumber   = 5;
#endif
#ifdef HAVE_SONY_SDK
    }
    else if ( IsHiResTypeSony( HiResType() ) ) {
        buttonList[0].fontMode = FONT_NARROW;
        buttonList[1].fontMode = FONT_TINY;
        buttonList[2].fontMode = FONT_SMALL;
        buttonList[3].fontMode = FONT_DEFAULT;
        buttonList[4].fontMode = FONT_LARGE;
        buttonList[5].fontMode = FONT_USER;
        buttonList[0].fontID   = narrowFont;
        buttonList[1].fontID   = hrTinyFont;
        buttonList[2].fontID   = hrSmallFont;
        buttonList[3].fontID   = hrStdFont;
        buttonList[4].fontID   = hrLargeFont;
        buttonList[5].fontID   = userStdFont_sony;
        buttonList[0].objID    = frmFont1;
        buttonList[1].objID    = frmFont2;
        buttonList[2].objID    = frmFont3;
        buttonList[3].objID    = frmFont4;
        buttonList[4].objID    = frmFont5;
        buttonList[5].objID    = frmFontUser;
        userFontButtonNumber   = 5;
#endif
    }
    else {
        buttonList[0].fontMode = FONT_NARROW;
        buttonList[1].fontMode = FONT_DEFAULT;
        buttonList[2].fontMode = FONT_BOLD;
        buttonList[3].fontMode = FONT_LARGE;
        buttonList[4].fontMode = FONT_LARGEBOLD;
        buttonList[5].fontMode = FONT_USER;
        buttonList[0].fontID   = narrowFont;
        buttonList[1].fontID   = stdFont;
        buttonList[2].fontID   = boldFont;
        buttonList[3].fontID   = largeFont;
        buttonList[4].fontID   = largeBoldFont;
        buttonList[5].fontID   = userStdFont;
        buttonList[0].objID    = frmFont1;
        buttonList[1].objID    = frmFont2;
        buttonList[2].objID    = frmFont3;
        buttonList[3].objID    = frmFont4;
        buttonList[4].objID    = frmFont5;
        buttonList[5].objID    = frmFontUser;
        userFontButtonNumber   = 5;
    }

    if ( cameFromLibrary ) {
        HideFontFormObject( frmFontLineSpacingLabel, false );
        HideFontFormObject( frmFontLineSpacingPopup, true );
        HideFontFormObject( frmFontLineSpacingList, false );
        HideFontFormObject( frmFontParagraphSpacingLabel, false );
        HideFontFormObject( frmFontParagraphSpacingPopup, true );
        HideFontFormObject( frmFontParagraphSpacingList, false );
        HideFontFormObject( frmFontIndividualFonts, true );
        HideFontFormObject( frmFontAsDefault, true );
#ifdef HAVE_ROTATE
        HideFontFormObject( frmFontRotateLabel, false );
        HideFontFormObject( frmFontRotateList, false );
        HideFontFormObject( frmFontRotatePopup, true );
#endif
    }
    else {
        ShowFontFormObject( frmFontLineSpacingLabel, false );
        ShowFontFormObject( frmFontLineSpacingPopup, true );
        ShowFontFormObject( frmFontLineSpacingList, false );
        ShowFontFormObject( frmFontParagraphSpacingLabel, false );
        ShowFontFormObject( frmFontParagraphSpacingPopup, true );
        ShowFontFormObject( frmFontParagraphSpacingList, false );
        ShowFontFormObject( frmFontIndividualFonts, true );
        ShowFontFormObject( frmFontAsDefault, true );
#ifdef HAVE_ROTATE
        ShowFontFormObject( frmFontRotateLabel, false );
        ShowFontFormObject( frmFontRotateList, false );
        ShowFontFormObject( frmFontRotatePopup, true );
#endif
    }

    if ( userFontButtonNumber != NO_USER_FONT_BUTTON ) {
        InitializeUserFontList( frmFontUserFontList );
        oldUserFontNumber = GetCurrentUserFontNumber();
    }

    FrmDrawForm( form );

    /* Highlight the pushbutton that reflects the currently set font */
    for ( i = 0; i < buttonCount; i++ ) {
       if ( fontModePref == buttonList[ i ].fontMode ) {
           selected = i;
           break;
       }
    }

    if ( userFontButtonNumber != NO_USER_FONT_BUTTON ) {
        userFontSelection = GetUserFontNumber( prefsUserFontName, true, 
                                fontCacheOff );
        if ( userFontSelection == NO_SUCH_USER_FONT )
            userFontSelection = 0;
        list        = GetObjectPtr( frmFontUserFontList );
        ctl         = GetObjectPtr( frmFontUserFontPopup );
        LstSetSelection( list, userFontSelection );
        CtlSetLabel( ctl, LstGetSelectionText( list, userFontSelection ) );
    }

    DrawButtons( false );

    if ( ! cameFromLibrary ) {
        lineSpacing = Prefs()->lineSpacing - MINIMAL_LINE_SPACING;
        list        = GetObjectPtr( frmFontLineSpacingList );
        ctl         = GetObjectPtr( frmFontLineSpacingPopup );
        LstSetSelection( list, lineSpacing );
        CtlSetLabel( ctl, LstGetSelectionText( list, lineSpacing ) );

        paragraphSpacing = Prefs()->paragraphSpacing;
        list        = GetObjectPtr( frmFontParagraphSpacingList );
        ctl         = GetObjectPtr( frmFontParagraphSpacingPopup );
        LstSetSelection( list, paragraphSpacing );
        CtlSetLabel( ctl, LstGetSelectionText( list, paragraphSpacing ) );

        individualDocumentFonts = Prefs()->individualDocumentFonts;
        ctl         = GetObjectPtr( frmFontIndividualFonts );
        CtlSetValue( ctl, individualDocumentFonts );

#ifdef HAVE_ROTATE
        rotate      = Prefs()->rotate;
        list        = GetObjectPtr( frmFontRotateList );
        ctl         = GetObjectPtr( frmFontRotatePopup );
        LstSetSelection( list, rotate );
        CtlSetLabel( ctl, LstGetSelectionText( list, rotate ) );
#endif

        ctl         = GetObjectPtr( frmFontAsDefault );
        CtlSetValue( ctl, false );
        if ( ! individualDocumentFonts )
            HideFontFormObject( frmFontAsDefault, true );
    }
}



/* Run when the user hits OK or pushes in the Jogdial */
void FontDoOK( void )
{
    Boolean updateDoc;
    Boolean changeUserFont;

    Prefs()->individualDocumentFonts =
        CtlGetValue( GetObjectPtr( frmFontIndividualFonts ) );

    updateDoc    = false;
    lineSpacing += MINIMAL_LINE_SPACING;

    if ( userFontButtonNumber != NO_USER_FONT_BUTTON && 
        GetNumberOfUserFonts() != 0 &&
        userFontSelection != GetUserFontNumber( prefsUserFontName, true, 
                                 fontCacheOff )
        )
        changeUserFont = true;
    else
        changeUserFont = false;

    if ( ( fontModePref     != buttonList[ selected ].fontMode ) ||
         ( lineSpacing      != Prefs()->lineSpacing            ) ||
         ( paragraphSpacing != Prefs()->paragraphSpacing       ) ||
#ifdef HAVE_ROTATE
         ( rotate           != Prefs()->rotate                 ) ||
#endif
         ( changeUserFont )
        ) {
        if ( ! IsFullscreenformActive() ) {
            if ( ! cameFromLibrary ) {
                if ( Prefs()->individualDocumentFonts ) {
                    GetHistoryPtr()->font = buttonList[ selected ].fontMode;
                }
                else {
                    Prefs()->fontModeMain = buttonList[ selected ].fontMode;
                    Prefs()->defaultFontModeMain = Prefs()->fontModeMain;
                }
            }
            else if ( cameFromLibrary )
                Prefs()->fontModeLibrary = buttonList[ selected ].fontMode;

            if ( changeUserFont ) {
                LoadUserFont( userFontSelection );
                RefreshCustomFonts();
            }

        }
        if ( ! cameFromLibrary ) {
            if ( Prefs()->individualDocumentFonts ) {
                GetHistoryPtr()->lineSpacing      = lineSpacing;
                GetHistoryPtr()->paragraphSpacing = paragraphSpacing;
#ifdef HAVE_ROTATE
                GetHistoryPtr()->rotate           = rotate;
#endif
            }
            else {
                Prefs()->lineSpacing             = lineSpacing;
                Prefs()->paragraphSpacing        = paragraphSpacing;
#ifdef HAVE_ROTATE
                Prefs()->rotate                  = rotate;
#endif
                Prefs()->defaultLineSpacing      = lineSpacing;
                Prefs()->defaultParagraphSpacing = paragraphSpacing;
#ifdef HAVE_ROTATE
                Prefs()->defaultRotate           = rotate;
#endif
            }
        }
        SetFontStyles();
        if ( changeUserFont ) {
            if ( Prefs()->individualDocumentFonts &&
                IsFormMain( Prefs()->lastForm )
              )
                StrCopy( GetHistoryPtr()->userFontName,
                    GetUserFontName( userFontSelection ) );
            else {
                StrCopy( prefsUserFontName,
                    GetUserFontName( userFontSelection ) );
                if ( ! cameFromLibrary )
                    StrCopy( Prefs()->defaultMainUserFontName,
                        GetUserFontName( userFontSelection ) );
            }
        }
        updateDoc = true;
    }

    if ( IsFullscreenformActive() ) {
        FrmUpdateForm( frmFullscreen, frmRedrawUpdateCode );
        ReRenderAllIfNeeded();
        FrmReturnToForm( frmFullscreen );
    } else {
        if ( ! cameFromLibrary && Prefs()->individualDocumentFonts &&
             CtlGetValue( GetObjectPtr( frmFontAsDefault ) ) ) {
            Prefs()->defaultFontModeMain     = GetHistoryPtr()->font;
            Prefs()->defaultLineSpacing      = GetHistoryPtr()->lineSpacing;
            Prefs()->defaultParagraphSpacing =
                GetHistoryPtr()->paragraphSpacing;
            StrCopy( Prefs()->defaultMainUserFontName,
                GetHistoryPtr()->userFontName );
#ifdef HAVE_ROTATE
            Prefs()->defaultRotate           = GetHistoryPtr()->rotate;
#endif            
        }

        FrmReturnToForm( Prefs()->lastForm );
        if ( updateDoc ) {
            if ( IsFormMain( Prefs()->lastForm ) )
                SetMainFormUpdate();
            else
                FrmUpdateForm( Prefs()->lastForm, frmRedrawUpdateCode );
        }
    }

    SafeMemPtrFree( buttonList );
}



/* Run when the user hits Cancel or pushes back on the Jogdial */
void FontDoCancel( void )
{
    if ( IsFullscreenformActive() )
        FrmReturnToForm( frmFullscreen );
    else
        FrmReturnToForm( Prefs()->lastForm );
    SafeMemPtrFree( buttonList );
}



/* Event handler for the font form */
Boolean FontFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean handled;
    UInt16  i;

    handled = false;

    switch ( event->eType ) {
        case keyDownEvent:
            handled = JogdialFontHandler( event );
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
                    case frmFontLineSpacingPopup:
                        lineSpacing = selection;
                        break;

                    case frmFontParagraphSpacingPopup:
                        paragraphSpacing = selection;
                        break;

#ifdef HAVE_ROTATE
                    case frmFontRotatePopup:
                        rotate = selection;
                        break;
#endif

                    case frmFontUserFontPopup:
                        if ( userFontButtonNumber != NO_USER_FONT_BUTTON ) {
                            selected = userFontButtonNumber;
                            userFontSelection = selection;
                            DrawButtons( true );
                        }
                        break;

                    default:
                        break;
                }
                handled = true;
            }
            break;
        }

        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmFont1:
                case frmFont2:
                case frmFont3:
                case frmFont4:
                case frmFont5:
                case frmFontUser:
                    for ( i = 0; i < buttonCount; i++ ) {
                        if ( buttonList[ i ].objID ==
                             event->data.ctlEnter.controlID ) {
                            selected = i;
                            break;
                        }
                    }
                    handled = true;
                    break;

                case frmFontIndividualFonts:
                    if ( ! CtlGetValue( GetObjectPtr(
                              frmFontIndividualFonts ) )
                      ) {
                        HideFontFormObject( frmFontAsDefault, true );
                    }
                    else
                        ShowFontFormObject( frmFontAsDefault, true );
                    handled = true;
                    break;

                case frmFontAsDefault:
                    handled = true;
                    break;

                case frmFontOK:
                    FontDoOK();
                    handled = true;
                    break;

                case frmFontCancel:
                    FontDoCancel();
                    handled = true;
                    break;

                default:
                    break;
            }
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
            FontFormInit();
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            SafeMemPtrFree( buttonList );
            break;

        default:
            handled = false;
    }

    return handled;
}

