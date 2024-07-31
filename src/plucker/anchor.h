/*
 * $Id: anchor.h,v 1.29 2003/07/12 14:57:02 prussar Exp $
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

#ifndef PLUCKER_ANCHOR_H
#define PLUCKER_ANCHOR_H

#include "viewer.h"
#include "util.h"


typedef enum {
    ANCHOR_UNUSED = 0,
    ANCHOR_SELECTED,
    ANCHOR_UNSELECTED,
    ANCHOR_LIMBO,
} AnchorStateType;


/* initialize new list of anchors */
extern void AnchorListInit(void) ANCHOR_SECTION;

/* release list of anchors */
extern void AnchorListRelease(void) ANCHOR_SECTION;

/* Return the image reference for a visible anchor */
extern UInt16 GetVisibleImage(const Int16 index) ANCHOR_SECTION;

/* Return the paragraph offset of a visible anchor */
extern Int16 GetVisibleOffset(const Int16 index) ANCHOR_SECTION;

/* Return the reference of a visible anchor */
extern UInt16 GetVisibleReference(const Int16 index) ANCHOR_SECTION;

/* Return the image position for a visible anchor */
extern RectangleType GetVisibleImagePosition(const Int16 index) ANCHOR_SECTION;

/* Append image to current anchor */
extern void AnchorAppendImage(const TextContext *tContext,
                              const Int16 height,
                              const UInt16 image) ANCHOR_SECTION;

/* Adjust all of the visible anchors by the given amount */
extern void AdjustVisibleAnchors(const Int16 adjustment) ANCHOR_SECTION;

/* Handle multi-line anchors */
extern void AnchorContinue(const TextContext *tContext) ANCHOR_SECTION;

/* Set highlight status for the anchor */
extern void HighlightAnchor(const Int16 control,
                            const AnchorStateType state) ANCHOR_SECTION;

/* Restart anchor */
extern void RestartAnchor(const TextContext *tContext,
                          const Int16 height) ANCHOR_SECTION;

/* Initialize a new visible anchor */
extern void AnchorStart(const TextContext *tContext,
                        const UInt16 reference,
                        const Int16 offset) ANCHOR_SECTION;

/* Mark the end of a visible anchor */
extern void AnchorStop(const TextContext *tContext,
                       const Int16 height) ANCHOR_SECTION;

/* Mark the end of a visible image anchor */
extern void AnchorStopImage(TextContext *tContext,
                            const Int16 height,
                            const Int16 width) ANCHOR_SECTION;

/* return index for anchor at given coordinate, or 0 if not found */
extern Int16 AnchorIndex(Int16 x, Int16 y) ANCHOR_SECTION;

/* Return index for last visible anchor or NOT_FOUND */
extern Int16 FindFirstVisibleAnchor(void) ANCHOR_SECTION;

/* Return index for last visible anchor or NOT_FOUND
   if somethig's wrong */
extern Int16 FindLastVisibleAnchor(void) ANCHOR_SECTION;

/* Return index for next visible anchor or NOT_FOUND
   if no anchor was found */
extern Int16 FindNextVisibleAnchor(void) ANCHOR_SECTION;

/* Return index for prev visible anchor or NOT_FOUND
   if no anchor was found */
extern Int16 FindPrevVisibleAnchor(void) ANCHOR_SECTION;

/* Delete anchors outside the visible area */
extern void DeleteUnusedAnchors(void) ANCHOR_SECTION;

#endif

