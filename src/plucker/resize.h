/*
 * $Id: resize.h,v 1.10 2004/05/07 03:36:22 prussar Exp $
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


#ifndef PLUCKER_RESIZE_H
#define PLUCKER_RESIZE_H

#include "viewer.h"

#if ! defined( SUPPORT_DIA ) && defined( HAVE_SILKSCREEN )
# define SUPPORT_DIA
#endif

#if ! defined( SUPPORT_DIA_SONY ) && defined( HAVE_SONY_SDK )
# define SUPPORT_DIA_SONY
#endif

#if ! defined( SUPPORT_DIA_HANDERA ) && defined( HAVE_HANDERA_SDK )
# define SUPPORT_DIA_HANDERA
#endif

#ifndef winDisplayChangedEvent
# define winDisplayChangedEvent 0x4101
#else
# define HAVE_PALM_DIA_SDK
#endif

#include "resizeconsts.h"

#define WORD_LIST_TYPE 'wrdl'

typedef struct {
    UInt16 objectID;
    UInt16 flags;
    UInt16 reserved;
} DIAConfigEntryType;

typedef UInt8 DIAStateType;

typedef struct {
    UInt16             sizeInWords;
    UInt16             flags;
    UInt16             bin;
    UInt16             preferredState;
    DIAConfigEntryType objectList[0];
} DIAConfigType;

typedef struct {
    UInt16 from;
    UInt16 to;
} DIAIndexEntryType;

typedef struct {
    UInt16 count;
    DIAIndexEntryType mapList[ 0 ];
} DIAIndexType;

typedef struct {
    UInt16         formID;
    UInt16         numObjects;
    Boolean        open;
    MemHandle      configHandle;
    Coord          lastExtentX;
    Coord          lastExtentY;
    RectangleType  lastBounds;
    Boolean        forceRedraw;
    DIAConfigType* config;
    RectangleType* originalBounds;
} DIAFormEntryType;

#ifdef SUPPORT_DIA
extern void InitializeResizeSupport( UInt16 formMapId ) RESIZE_SECTION;
extern void TerminateResizeSupport( void ) RESIZE_SECTION;
extern void SetResizePolicy( UInt16 formID ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmOpenEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmCloseEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinDisplayChangedEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinExitEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleWinEnterEvent( void ) RESIZE_SECTION;
extern Boolean ResizeHandleFrmRedrawUpdateCode( void ) RESIZE_SECTION;
extern void LoadResizePrefs( UInt32 appID, UInt16 prefID ) RESIZE_SECTION;
extern void SaveResizePrefs( UInt32 appID, UInt16 prefID, Int16 version )
    RESIZE_SECTION;
extern void ResizeRefreshCurrentForm( void ) RESIZE_SECTION;
extern void SetHaveWinDisplayChangedEvent( Boolean value ) RESIZE_SECTION;
extern Boolean ResizeHandleEvent( EventType* event ) RESIZE_SECTION;

#else

#define InitializeResizeSupport( x )
#define TerminateResizeSupport()
#define SetResizePolicy( x )
#define ResizeHandleFrmOpenEvent()  true
#define ResizeHandleFrmCloseEvent() true
#define ResizeHandleWinDisplayChangedEvent() true
#define ResizeHandleWinExitEvent()  true
#define ResizeHandleWinEnterEvent() true
#define LoadResizePrefs( a, b )
#define SaveResizePrefs( a, b, c )
#define ResizeRefreshCurrentForm()
#define SetHaveWinDisplayChangedEvent( x )
#define ResizeHandleFrmRedrawUpdateCode() true
#define ResizeHandleEvent( e ) false

#endif

#endif /* _ARP_RESIZE_H */
