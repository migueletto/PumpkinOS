/*
 * $Id: keyboard.h,v 1.3 2003/08/03 14:07:02 nordstrom Exp $
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

#ifndef PLUCKER_KEYBOARD_H
#define PLUCKER_KEYBOARD_H

#include "viewer.h"

#define KEYBOARD_ACTION_NONE      0

typedef UInt16  KeyActionType;

typedef struct {
    WChar          key;
    UInt16         modifier;
    KeyActionType  action;
} KeyMapType;

/* look up the key and get the action */
extern KeyActionType GetKeyboardAction(WChar key, UInt16 modifier)
    KEYBOARD_SECTION;

/* load in the keyboard map database */
extern void OpenKeyboardMap( void ) KEYBOARD_SECTION;

/* close the keyboard map database */
extern void CloseKeyboardMap( void ) KEYBOARD_SECTION;

/* Get length of keyboard data */
extern UInt16 GetKeyboardMapLength( void ) KEYBOARD_SECTION;

/* Get all keyboard map data */
extern void GetKeyboardMap( KeyMapType* dest ) KEYBOARD_SECTION;

/* Set all keyboard map data */
extern void SetKeyboardMap( KeyMapType* src, UInt16 size ) KEYBOARD_SECTION;

/* Get length of default keyboard data */
extern UInt16 GetDefaultKeyboardMapLength( void ) KEYBOARD_SECTION;

/* Get default keyboard map data */
extern void GetDefaultKeyboardMap( KeyMapType* dest )
    KEYBOARD_SECTION;

#endif

