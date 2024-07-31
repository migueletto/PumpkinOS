/*
 * $Id: prefsform.h,v 1.16 2004/03/02 03:23:00 prussar Exp $
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

#ifndef PLUCKER_PREFSFORM_H
#define PLUCKER_PREFSFORM_H

#include "prefsdata.h"

#include "viewer.h"

#define MAX_OBJECTS  16
#define EOL          0xFFFF

/* The ActionType combines two forms of responses from the sections into one
   general definition. Specifically, AVAILABLE and SHOWFIRST will return either
   true or false depending on the circumstances. Everything else will just
   return true if something happened during that procedural call. */
typedef enum {
    AVAILABLE = 0,   /* Is this section available to the user? */
    SHOWFIRST,       /* Does this section want to be shown first? */
    LOAD,            /* Load this section into memory. */
    DISPLAY,         /* Display this section on the device. */
    CLEAR,           /* Clear the display. */
    SAVE,            /* Save the data into the preferences. */
    RELEASE          /* Release any used MemHandles or MemPtrs */
} ActionType;


/* Open a specific section onto the form */
extern void PrefsOpenSection( UInt16 thisRow ) PREFSFORM_SECTION;

/* Hide specific objects from the display based upon the provided list */
extern void PrefsHideSection( UInt16 list[][ MAX_OBJECTS ],
                UInt16 thisRow ) PREFSFORM_SECTION;

/* Show specific objects from the display based upon the provided list */
extern void PrefsShowSection( UInt16 list[][ MAX_OBJECTS ],
                UInt16 thisRow ) PREFSFORM_SECTION;

/* Set the display to match the list/popup selection */
extern void SetListToSelection( UInt16 listID, UInt16 popupID,
                UInt16 selection ) PREFSFORM_SECTION;

/* Ensure everything looks as it should */
extern void AffirmControlImage( UInt16 selected, UInt16 firstCtlObjectID,
                UInt16 firstImageObjectID) PREFSFORM_SECTION;

/* Open the popup for the SelectType lists, and return the users' new value */
extern SelectType SetActionForSelectedButton( UInt16 listObjectID,
                UInt16 controlID ) PREFSFORM_SECTION;

/* Return the value for currentSection */
extern UInt16 GetCurrentSection( void ) PREFSFORM_SECTION;

/* Set a new value for currentSection */
extern void SetCurrentSection( UInt16 thisSection ) PREFSFORM_SECTION;

/* Goto the previous section in the preferences */
extern void PrefsPreviousSection( void ) PREFSFORM_SECTION;

/* Goto the next section in the preferences */
extern void PrefsNextSection( void ) PREFSFORM_SECTION;

/* Event handler for the preference form */
extern Boolean PrefsFormHandleEvent( EventType* event );

/* Failover function to handle missing sections */
extern Boolean PrefsNULLPreferenceEvent( ActionType action ) PREFSFORM_SECTION;

/* Failover function to handle missing sections */
extern Boolean PrefsNULLPalmEvent( EventType* event ) PREFSFORM_SECTION;

#endif

