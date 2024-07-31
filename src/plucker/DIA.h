/*
 * $Id: DIA.h,v 1.4 2004/05/13 00:12:17 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2004, Mark Ian Lillywhite and Michael Nordstrom
 * and Alexander R. Pruss.
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


#ifndef PLUCKER_DIA_H
#define PLUCKER_DIA_H

#include "viewer.h"

#include "resize.h"

typedef enum {
    DIA_HARDWARE_NONE = 0,
    DIA_HARDWARE_HANDERA,
    DIA_HARDWARE_SONY1,
    DIA_HARDWARE_SONY2,
    DIA_HARDWARE_PALM10,
    DIA_HARDWARE_PALM11,
    DIA_HARDWARE_UNKNOWN
} DIAHardwareType;

#if defined( SUPPORT_DIA ) || defined( HAVE_SILKSCREEN )

extern DIAHardwareType InitializeDIA( void ) DIA_SECTION;
extern void TerminateDIA( void ) DIA_SECTION;
extern void SetDIAState( DIAStateType state ) DIA_SECTION;
extern void SetDIAAllowResize( Boolean allow ) DIA_SECTION;
extern Boolean HandleResizeNotification( UInt32 notificationType )
    DIA_SECTION;
extern DIAStateType GetDIAState( void ) DIA_SECTION;
extern void SetCustomDIAPolicy( UInt16 formID ) DIA_SECTION;
extern DIAHardwareType GetDIAHardware( void ) DIA_SECTION;
extern void SetDIAConstraints( WinHandle winH, Boolean defaultBig,
    Boolean allowBig ) DIA_SECTION;
/* Check which DIA state covers more screen space */
extern Int16 CompareDIAState( DIAStateType x, DIAStateType y )
    DIA_SECTION;
extern void GetHiddenStatusBarArea( RectangleType* area ) DIA_SECTION;
/* These two functions should NOT be used by code other than that in
   resize.c and DIA.c. */
extern void PushCoordinateSystemToStandard( void ) DIA_SECTION;
extern void PopCoordinateSystem( void ) DIA_SECTION;

#else

#define InitializeDIA()      DIA_HARDWARE_NONE
#define TerminateDIA()
#define SetDIAState( s )
#define SetDIAAllowResize( a )
#define HandleResizeNotification( n ) false
#define GetDIAState()        DIA_STATE_MAX
#define SetCustomDIAPolicy( f )
#define GetDIAHardware()     DIA_HARDWARE_NONE
#define SetDIAConstraints( w, d, a )
#define CompareDIAState( x, y )    0
#define GetHiddenStatusBarArea( a )  MemSet( a, sizeof( RectangleType ), 0 )

#endif

#endif /* _ARP_DIA_H */
