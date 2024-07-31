/*
 * $Id: prefslookup.h,v 1.1 2003/12/31 14:49:40 prussar Exp $
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

#ifndef PLUCKER_PREFSLOOKUP_H
#define PLUCKER_PREFSLOOKUP_H

#include "prefsform.h"

#include "viewer.h"

#ifdef SUPPORT_WORD_LOOKUP

/* Handle the Jogdial preferences */
extern Boolean PrefsLookupPreferenceEvent( ActionType action ) PREFSFORM_SECTION;

/* Nominate the Jogdial section to be shown first when the prefsform loads */
extern void PrefsLookupShowFirst( void ) PREFSFORM_SECTION;

/* Event handler for the Jogdial preferences */
extern Boolean PrefsLookupPalmEvent( EventType* event ) PREFSFORM_SECTION;

#else

#define PrefsLookupPreferenceEvent PrefsNULLPreferenceEvent
#define PrefsLookupShowFirst()
#define PrefsLookupPalmEvent       PrefsNULLPalmEvent

#endif

#endif


