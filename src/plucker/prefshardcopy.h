/*
 * $Id: prefshardcopy.h,v 1.1 2003/02/03 22:46:17 adamm Exp $
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

#ifndef PLUCKER_PREFSHARDCOPY_H
#define PLUCKER_PREFSHARDCOPY_H

#include "prefsform.h"

#include "viewer.h"

/* Handle the Hardcopy preferences */
extern Boolean PrefsHardcopyPreferenceEvent( ActionType action ) PREFSFORM_SECTION;

/* Nominate the Hardcopy section to be shown first when the prefsform loads */
extern void PrefsHardcopyShowFirst( void ) PREFSFORM_SECTION;

/* Event handler for the Hardcopy preferences */
extern Boolean PrefsHardcopyPalmEvent( EventType* event ) PREFSFORM_SECTION;

#endif

