/*
 * $Id: fiveway.h,v 1.8 2004/05/08 08:57:55 nordstrom Exp $
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

#ifndef PLUCKER_FIVEWAY_H
#define PLUCKER_FIVEWAY_H

#include "viewer.h"

#if defined( HAVE_FIVEWAY_SDK ) && ! defined( HAVE_HANDSPRING_SDK )

#define FiveWayCenterPressed(eventP) \
( \
  NavSelectPressed(eventP) \
)

#define FiveWayDirectionPressed(eventP, nav) \
( \
  NavDirectionPressed(eventP, nav) \
)

/* only to be used with Left, Right, Up, and Down; use
   FiveWayCenterPressed for Select/Center */
#define FiveWayKeyPressed(eventP, nav) \
( \
  NavKeyPressed(eventP, nav) \
)

#define IsFiveWayEvent(eventP) \
( \
  IsFiveWayNavEvent(eventP) \
)

extern void FiveWayResetValues( void ) FIVEWAY_SECTION;
extern void FiveWaySetRow( Int16 row ) FIVEWAY_SECTION;
extern Int16 FiveWayGetRow( void ) FIVEWAY_SECTION;
extern void FiveWayHighlightRow( Boolean enable ) FIVEWAY_SECTION;
extern Boolean FiveWayLibraryHandler( EventType* event ) FIVEWAY_SECTION;
extern Boolean FiveWayMainHandler( EventType* event ) FIVEWAY_SECTION;

#elif ! defined( HAVE_FIVEWAY_SDK) && defined( HAVE_HANDSPRING_SDK )

#define HsNavCenterPressed(eventP) \
( \
  IsHsFiveWayNavEvent(eventP) && \
  ((eventP)->data.keyDown.chr == vchrRockerCenter) && \
  (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0) \
)

#define HsNavDirectionPressed(eventP, nav) \
( \
  IsHsFiveWayNavEvent(eventP) && \
  ( vchrRocker ## nav == vchrRockerUp) ? \
   (((eventP)->data.keyDown.chr == vchrPageUp) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
   (vchrRocker ## nav == vchrRockerDown) ? \
   (((eventP)->data.keyDown.chr == vchrPageDown) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav) \
)

#define HsNavKeyPressed(eventP, nav) \
( \
  ( vchrRocker ## nav == vchrRockerCenter ) ? \
  HsNavCenterPressed(eventP) : \
  HsNavDirectionPressed(eventP, nav) \
)

#define IsHsFiveWayNavEvent(eventP) \
( \
    HaveHsNav() && ((eventP)->eType == keyDownEvent) && \
    ( \
        ((((eventP)->data.keyDown.chr == vchrPageUp) || \
          ((eventP)->data.keyDown.chr == vchrPageDown)) && \
         (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0)) \
        || \
        (TxtCharIsRockerKey((eventP)->data.keyDown.modifiers, \
                            (eventP)->data.keyDown.chr)) \
    ) \
)

#define FiveWayCenterPressed(eventP) \
( \
  HsNavCenterPressed(eventP) \
)

#define FiveWayDirectionPressed(eventP, nav) \
( \
  HsNavDirectionPressed(eventP, nav) \
)

/* only to be used with Left, Right, Up, and Down; use
   FiveWayCenterPressed for Select/Center */
#define FiveWayKeyPressed(eventP, nav) \
( \
  HsNavKeyPressed(eventP, nav) \
)

#define IsFiveWayEvent(eventP) \
( \
  IsHsFiveWayNavEvent(eventP) \
)

extern void FiveWayResetValues( void ) FIVEWAY_SECTION;
extern void FiveWaySetRow( Int16 row ) FIVEWAY_SECTION;
extern Int16 FiveWayGetRow( void ) FIVEWAY_SECTION;
extern void FiveWayHighlightRow( Boolean enable ) FIVEWAY_SECTION;
extern Boolean FiveWayLibraryHandler( EventType* event ) FIVEWAY_SECTION;
extern Boolean FiveWayMainHandler( EventType* event ) FIVEWAY_SECTION;

#elif defined( HAVE_FIVEWAY_SDK ) && defined( HAVE_HANDSPRING_SDK )

#define HsNavCenterPressed(eventP) \
( \
  IsHsFiveWayNavEvent(eventP) && \
  ((eventP)->data.keyDown.chr == vchrRockerCenter) && \
  (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0) \
)

#define HsNavDirectionPressed(eventP, nav) \
( \
  IsHsFiveWayNavEvent(eventP) && \
  ( vchrRocker ## nav == vchrRockerUp) ? \
   (((eventP)->data.keyDown.chr == vchrPageUp) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
   (vchrRocker ## nav == vchrRockerDown) ? \
   (((eventP)->data.keyDown.chr == vchrPageDown) || \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav)) : \
    ((eventP)->data.keyDown.chr == vchrRocker ## nav) \
)

#define HsNavKeyPressed(eventP, nav) \
( \
  ( vchrRocker ## nav == vchrRockerCenter ) ? \
  HsNavCenterPressed(eventP) : \
  HsNavDirectionPressed(eventP, nav) \
)

#define IsHsFiveWayNavEvent(eventP) \
( \
    HaveHsNav() && ((eventP)->eType == keyDownEvent) && \
    ( \
        ((((eventP)->data.keyDown.chr == vchrPageUp) || \
          ((eventP)->data.keyDown.chr == vchrPageDown)) && \
         (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0)) \
        || \
        (TxtCharIsRockerKey((eventP)->data.keyDown.modifiers, \
                            (eventP)->data.keyDown.chr)) \
    ) \
)

#define FiveWayCenterPressed(eventP) \
( \
  NavSelectPressed(eventP) || \
  HsNavCenterPressed(eventP) \
)

#define FiveWayDirectionPressed(eventP, nav) \
( \
  NavDirectionPressed(eventP, nav) || \
  HsNavDirectionPressed(eventP, nav) \
)

/* only to be used with Left, Right, Up, and Down; use
   FiveWayCenterPressed for Select/Center */
#define FiveWayKeyPressed(eventP, nav) \
( \
  NavKeyPressed(eventP, nav) || \
  HsNavKeyPressed(eventP, nav) \
)

#define IsFiveWayEvent(eventP) \
( \
  HaveHsNav() ? IsHsFiveWayNavEvent(eventP) : IsFiveWayNavEvent(eventP) \
)

extern Boolean FiveWayLibraryHandler( EventType* event ) FIVEWAY_SECTION;
extern Boolean FiveWayMainHandler( EventType* event ) FIVEWAY_SECTION;

#else

#define IsFiveWayEvent(eventP)         false
#define IsHsFiveWayNavEvent(eventP)    false
#define FiveWayCenterPressed(eventP)   false
#define FiveWayDirectionPressed(eventP, nav) false
#define FiveWayKeyPressed(eventP, nav) false
#define FiveWayLibraryHandler( event ) false
#define FiveWayMainHandler( event )    false

#endif

#endif
