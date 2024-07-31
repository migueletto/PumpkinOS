/*
 * $Id: handera.h,v 1.2 2004/05/08 09:13:08 nordstrom Exp $
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

#ifndef PLUCKER_HANDERA_H
#define PLUCKER_HANDERA_H

#include "viewer.h"

/* These may one day break PalmOS because they are 'technically'
   supposed to be reserved for licensees. We need to allocate a range
   so that we can properly catch toolbar events sitting on the silk
   screen. If the silkscreen breaks later, try changing these :)

   For more information, see Chars.h */
#define vchrPluckerMin             0x2000
#define vchrPluckerMax             0x200F

#define vchrPluckerDbase           ( vchrPluckerMin + LIBRARYCONTROL )
#define vchrPluckerOffset          ( vchrPluckerMin + OFFSETCONTROL )
#define vchrPluckerBookmark        ( vchrPluckerMin + BOOKMARKCONTROL )
#define vchrPluckerFind            ( vchrPluckerMin + FINDCONTROL )
#define vchrPluckerAgain           ( vchrPluckerMin + AGAINCONTROL )
#define vchrPluckerAutoscrollDecr  ( vchrPluckerMin + AUTOSCROLLDECRCONTROL )
#define vchrPluckerAutoscrollStop  ( vchrPluckerMin + AUTOSCROLLSTOPCONTROL )
#define vchrPluckerAutoscrollStart ( vchrPluckerMin + AUTOSCROLLSTARTCONTROL )
#define vchrPluckerAutoscrollIncr  ( vchrPluckerMin + AUTOSCROLLINCRCONTROL )
#define vchrPluckerLeft            ( vchrPluckerMin + LEFTCONTROL )
#define vchrPluckerHome            ( vchrPluckerMin + HOMECONTROL )
#define vchrPluckerRight           ( vchrPluckerMin + RIGHTCONTROL )

#ifdef HAVE_HANDERA_SDK
void HanderaSetSilkScreen( void ) HANDERA_SECTION;
void HanderaResetSilkScreen( void ) HANDERA_SECTION;
void HanderaUpdateSilkVerticalOffset(
                const Char* offText ) HANDERA_SECTION;
#else
#define HanderaSetSilkScreen()
#define HanderaResetSilkScreen()
#define HanderaUpdateSilkVerticalOffset(x)
#endif

#endif

