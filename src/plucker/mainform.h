/*
 * $Id: mainform.h,v 1.17 2003/12/31 16:24:59 prussar Exp $
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

#ifndef PLUCKER_MAINFORM_H
#define PLUCKER_MAINFORM_H

#include "viewer.h"


/* Initialize the main form */
extern void MainFormInit( void ) MAINFORM_SECTION;

/* Event handler for the main form. */
extern Boolean MainFormHandleEvent( EventType* event );

/* Handle common menu items */
extern Boolean HandleCommonMenuItems( UInt16 itemID ) MAINFORM_SECTION;

/* Main form needs updating when it becomes active. */
extern void SetMainFormUpdate( void ) MAINFORM_SECTION;
#endif

