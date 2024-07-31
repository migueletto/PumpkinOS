/*
 * $Id: control.h,v 1.39 2004/01/25 19:11:40 prussar Exp $
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

#ifndef PLUCKER_CONTROL_H
#define PLUCKER_CONTROL_H

#include "viewer.h"
#include "prefsdata.h"


typedef enum {
    AUTOSCROLL_OFF = 0,
    AUTOSCROLL_ON,
    AUTOSCROLL_TOGGLE
} AutoscrollType;

#define AUTOSCROLL_MIN_JUMP_VALUE       1
#define AUTOSCROLL_MAX_JUMP_VALUE       40
#define AUTOSCROLL_INCR_JUMP_VALUE      1
#define AUTOSCROLL_MIN_INTERVAL_VALUE   0
#define AUTOSCROLL_MAX_INTERVAL_VALUE   500
#define AUTOSCROLL_INCR_INTERVAL_VALUE  50
#define AUTOSCROLL_FINE_TRANSITION      100
#define AUTOSCROLL_FINE_INCR_INTERVAL_VALUE 10
#define TICKS_TO_MILLISECONDS( ticks )  ( ( ticks ) * 10 )
#define MILLISECONDS_TO_TICKS( ms )     ( ( ms ) / 10 )


/* Reset the prev/next anchor */
void ResetActualAnchor( void ) CONTROL_SECTION;

/* Unhighlight and reset the anchor for the prev/next controls */
void UnselectActualAnchor( void ) CONTROL_SECTION;

/* Clear the structure holding pen data between events */
extern void ClearControlBounds(void) CONTROL_SECTION;

/* Perform action assigned to given control object */
extern void DoControlAction(const Int16 control) CONTROL_SECTION;

/* Emit a keystroke */
extern void EmitKey(UInt16 key, UInt16 modifiers) CONTROL_SECTION;

/* Perform action for specified select type */
extern void DoSelectTypeAction(SelectType selection) CONTROL_SECTION;

/* Retrieve the bounds of an object */
extern void GetControlBounds(const FormType *form,
                             const UInt16 objectId,
                             const Int16 control) CONTROL_SECTION;

/* Go to location in record */
extern void GotoLocation(const Int16 percent) CONTROL_SECTION;

/* Respond to pen tap */
extern Boolean HandlePenDown(const Coord x, const Coord y) CONTROL_SECTION;

/* Respond to pen movement */
extern Boolean HandlePenMove(const Coord x, const Coord y) CONTROL_SECTION;

/* Respond to pen release */
extern Boolean HandlePenUp(const Coord x, const Coord y) CONTROL_SECTION;

/* Toggle Autoscroll */
extern void DoAutoscrollToggle(AutoscrollType toggle) CONTROL_SECTION;

/* Increase Autoscroll rate */
extern void DoAutoscrollIncr(void) CONTROL_SECTION;

/* Decrease Autoscroll rate */
extern void DoAutoscrollDecr(void) CONTROL_SECTION;

/* Jump to requested record */
extern void JumpToRecord(const UInt16 record, const Int16 pOffset,
    const Int16 cOffset) CONTROL_SECTION;

/* Returns whether MainForm's Window is active and OK to draw toolbars on,
   or not (e.g. obscured by menu, etc.) */
extern Boolean IsMainFormWinActive( void ) CONTROL_SECTION;

#endif

