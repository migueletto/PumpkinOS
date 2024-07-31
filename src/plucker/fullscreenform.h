/*
 * $Id: fullscreenform.h,v 1.10 2003/09/08 02:18:47 chrish Exp $
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

#ifndef PLUCKER_FULLSCREENFORM_H
#define PLUCKER_FULLSCREENFORM_H

#include "prefsdata.h"

#include "viewer.h"

extern Boolean FsFormHandleEvent( EventType* event );

/* Move image up/down replaces AdjustVerticalOffset() in document.c */
extern void FsAdjustVerticalOffset( Int16 adjustment ) FULLSCREENFORM_SECTION;

extern Boolean FsDoSelectTypeAction( SelectType selection ) FULLSCREENFORM_SECTION;

extern void FsDoControlAction( Int16 control ) FULLSCREENFORM_SECTION;

extern void FsFrmGotoForm( UInt16 form ) FULLSCREENFORM_SECTION;

/* Check if fullscreenform is active */
extern Boolean IsFullscreenformActive( void ) FULLSCREENFORM_SECTION;

#endif
