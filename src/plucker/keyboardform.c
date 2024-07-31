/*
 * $Id: keyboardform.c,v 1.12 2004/05/08 09:04:54 nordstrom Exp $
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
#include "keyboard.h"
#include "prefsform.h"
#include "util.h"
#include "os.h"
#include "DIA.h"

#include "keyboardform.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void AddToKeyList(WChar key, UInt16 modifier, KeyActionType action)
    KEYBOARDFORM_SECTION;
static KeyActionType LookupInKeyList(WChar key, UInt16 modifier)
    KEYBOARDFORM_SECTION;
static void KeyboardFormInit( void ) KEYBOARDFORM_SECTION;
static void ShowSelections( void ) KEYBOARDFORM_SECTION;
static void SetSelections( void ) KEYBOARDFORM_SECTION;
static void ClearMap( void ) KEYBOARDFORM_SECTION;



/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static WChar specialKeyList[] = {
    chrLineFeed,
    chrHorizontalTabulation,
    chrUpArrow,
    chrDownArrow,
    chrLeftArrow,
    chrRightArrow,
    chrBackspace
};
#define KEY_LIST_LENGTH ( sizeof( keyList ) / sizeof( keyList[ 0 ] )

static KeyMapType*   keyboardMap = NULL;
static UInt16        length      = 0;

static KeyActionType specialKeyAction;
static KeyActionType stdKeyAction;
static UInt16        specialKeyIndex;
static FieldType*    fldPtr;
static WChar         stdKey;



/* set a keyboard action */
static void AddToKeyList
    (
    WChar         key,      /* key to set */
    UInt16        modifier, /* currently should be 0 */
    KeyActionType action    /* action to assign */
    )
{
    KeyMapType* newKeyboardMap;
    KeyMapType  entry;
    UInt16      i;

    if ( key == 0 )
        return;

    for ( i = 0 ; i < length ; i ++ )
        if ( keyboardMap[ i ].key == key &&
           keyboardMap[ i ].modifier == modifier )
            break;

    entry.key      = key;
    entry.modifier = modifier;
    entry.action   = action;

    if ( i == length ) {
        if ( action == KEYBOARD_ACTION_NONE )
            return;
        newKeyboardMap = SafeMemPtrNew( ( length + 1 ) * sizeof( KeyMapType ) );
        if ( length != 0 )
            MemMove( newKeyboardMap, keyboardMap, length * sizeof( KeyMapType ) );
        newKeyboardMap[ length ] = entry;
        length++;
        if ( keyboardMap != NULL )
            SafeMemPtrFree( keyboardMap );
        keyboardMap = newKeyboardMap;
    }
    else if ( action != KEYBOARD_ACTION_NONE )
        keyboardMap[ i ] = entry;
    else {
        /* take out entry, and shift down */
        for ( ; i < length - 1 ; i ++ )
            keyboardMap[ i ] = keyboardMap[ i + 1 ];
        length--;
        if ( length == 0 ) {
            keyboardMap = NULL;
            return;
        }
        newKeyboardMap = SafeMemPtrNew( length * sizeof( KeyMapType ) );
        MemMove( newKeyboardMap, keyboardMap, length * sizeof( KeyMapType ) );
        SafeMemPtrFree( keyboardMap );
        keyboardMap = newKeyboardMap;
    }
}



/* get a keyboard action */
static KeyActionType LookupInKeyList
                     (
                WChar         key,      /* key to get */
                UInt16        modifier  /* currently should be 0 */
                     )
{
   int i;

   for ( i = 0 ; i < length ; i++ )
        if ( keyboardMap[ i ].key == key &&
           keyboardMap[ i ].modifier == modifier )
            break;
   if ( i == length )
       return KEYBOARD_ACTION_NONE;
    return keyboardMap[ i ].action;
}



/* Show everything that has been selected, looking things up in the
   lists, and setting stdKey */
static void ShowSelections( void )
{
    ControlType*    ctl;
    ListType*       list;

    stdKey           = 0;
    TxtGlueGetNextChar( FldGetTextPtr( fldPtr ), 0, &stdKey );
    stdKeyAction     = LookupInKeyList( stdKey, 0 );
    specialKeyAction = LookupInKeyList( specialKeyList[ specialKeyIndex ], 0 );

    list             = GetObjectPtr( frmKeyboardStdKeyActionList );
    ctl              = GetObjectPtr( frmKeyboardStdKeyActionPopup );
    LstSetSelection( list, stdKeyAction );
    CtlSetLabel( ctl, LstGetSelectionText( list,
                          stdKeyAction ) );

    list             = GetObjectPtr( frmKeyboardSpecialKeyActionList );
    ctl              = GetObjectPtr( frmKeyboardSpecialKeyActionPopup );
    LstSetSelection( list, specialKeyAction );
    CtlSetLabel( ctl, LstGetSelectionText( list,
                          specialKeyAction ) );

    list             = GetObjectPtr( frmKeyboardSpecialKeyList );
    ctl              = GetObjectPtr( frmKeyboardSpecialKeyPopup );
    LstSetSelection( list, specialKeyIndex );
    CtlSetLabel( ctl, LstGetSelectionText( list,
                          specialKeyIndex ) );

}



/* stdKeyAction or specialKeyAction have changed: put the changes into the
   key list */
static void SetSelections( void )
{
    stdKey = 0;
    TxtGlueGetNextChar( FldGetTextPtr( fldPtr ), 0, &stdKey );
    AddToKeyList( stdKey, 0, stdKeyAction );
    AddToKeyList( specialKeyList[ specialKeyIndex ], 0, specialKeyAction );
}



/* initialize the keyboard customization form */
static void KeyboardFormInit( void )
{
    FormType* keyboardForm;

    keyboardForm = FrmGetFormPtr( frmKeyboard );

    length = GetKeyboardMapLength();
    if ( 0 < length ) {
        keyboardMap = SafeMemPtrNew( length * sizeof( KeyMapType ) );
        GetKeyboardMap( keyboardMap );
    }
    else
        keyboardMap = NULL;

    InitializeActionList( frmKeyboardStdKeyActionList );
    InitializeActionList( frmKeyboardSpecialKeyActionList );
    InitializeKeyList( frmKeyboardSpecialKeyList );

    specialKeyIndex  = 0;

    fldPtr = GetObjectPtr( frmKeyboardStdKey );

    FldInsert( fldPtr, " ", 1 );
    FldSetSelection( fldPtr, 0, 1 );

    ShowSelections();

    FrmDrawForm( keyboardForm );
}




/* empty the keyboard map */
static void ClearMap( void )
{
    if ( length != 0 ) {
        SafeMemPtrFree( keyboardMap );
        keyboardMap = NULL;
        length      = 0;
    }
}



/* handler for keyboard customization form */
Boolean KeyboardFormHandleEvent
    (
    EventType* event  /* pointer to an EventType structure */
    )
{
    Boolean   handled;

    handled = false;

    switch ( event->eType ) {
        case ctlSelectEvent:
            switch ( event->data.ctlEnter.controlID ) {
                case frmKeyboardOK:
                    SetKeyboardMap( keyboardMap, length );
                    FrmReturnToForm( Prefs()->lastForm );
                    ClearMap();
                    handled = true;
                    break;

                case frmKeyboardCancel:
                    FrmReturnToForm( Prefs()->lastForm );
                    ClearMap();
                    handled = true;
                    break;

                case frmKeyboardDefault:
                    ClearMap();
                    length = GetDefaultKeyboardMapLength();
                    if ( length != 0 ) {
                        keyboardMap = SafeMemPtrNew( length * sizeof( KeyMapType ) );
                        GetDefaultKeyboardMap( keyboardMap );
                    }
                    ShowSelections();
                    handled = true;
                    break;

                case frmKeyboardClear:
                    ClearMap();
                    ShowSelections();
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
                    case frmKeyboardSpecialKeyPopup:
                        specialKeyIndex = selection;
                        ShowSelections();
                        break;

                    case frmKeyboardSpecialKeyActionPopup:
                        specialKeyAction = selection;
                        SetSelections();
                        ShowSelections();
                        break;

                    case frmKeyboardStdKeyActionPopup:
                        stdKeyAction = selection;
                        SetSelections();
                        ShowSelections();
                        break;

                    default:
                        break;
                }
                handled = true;
            }
            break;
        }

        case keyDownEvent:
            handled = FldHandleEvent( fldPtr, event );
            ShowSelections();
            if ( stdKey != 0 )
                FldSetSelection( fldPtr, 0, 1 );
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
            KeyboardFormInit();
            handled = true;
            break;

        case frmCloseEvent:
#ifdef HAVE_SILKSCREEN
            ResizeHandleFrmCloseEvent();
#endif
            ClearMap();
            handled = false;
            break;

        default:
            handled = false;
    }
    return handled;
}

