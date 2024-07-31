/*
 * $Id: anchor.c,v 1.58 2004/02/08 21:50:38 prussar Exp $
 *
 * Viewer - a part of Plucker, the free off-line HTML viewer for PalmOS
 * Copyright (c) 1998-2001, Mark Ian Lillywhite and Michael Nordstrom
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

#include "debug.h"
#include "prefsdata.h"
#include "util.h"
#include "list.h"
#include "link.h"
#include "hires.h"
#include "table.h"
#include "rotate.h"
#include "fullscreenform.h"
#include "paragraph.h"

#include "anchor.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define SQUARE_CORNERS  0
#define ANCHORS_MATCH   0

#define NO_IMAGE        0


/***********************************************************************
 *
 *      Internal Types
 *
 ***********************************************************************/
/* Anchors define regions on the screen that people can tap on to
   link to other records */

typedef struct {
    RectangleType   bounds;
    UInt16          reference;          /* The record to link to */
    UInt16          image;              /* The image to link to */
    Int16           paragraphOffset;    /* The offset to named anchor
                                           paragraph */
    AnchorStateType state;
    Int16           anchorId;
    Boolean         underline;
} AnchorType;


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
/* To link VisibleAnchors together */
static Int16        anchorId    = 0;
static LinkedList   anchorList  = NULL;

static AnchorType*  currentAnchor = NULL;


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static Boolean AnchorInViewport(AnchorType *anchor) ANCHOR_SECTION;
static void AnchorCreate(const TextContext *tContext,
                const UInt16 reference, const UInt16 image,
                const Int16 offset, const Int16 anchorId) ANCHOR_SECTION;


/* initialize new list of anchors */
void AnchorListInit( void )
{
    if ( anchorList != NULL ) {
        if ( IsLineCacheActive() ) {
            /* If the line cache is active, for continuation
               purposes we must preserve the last anchor */
            AnchorType* lastAnchorPtr;
            lastAnchorPtr = ListLast( anchorList );
            if ( lastAnchorPtr != NULL ) {
                AnchorType lastAnchorData;
                lastAnchorData = *lastAnchorPtr;
                AnchorListRelease();
                anchorList     = ListCreate();
                lastAnchorPtr  = SafeMemPtrNew( sizeof( AnchorType ) );
                *lastAnchorPtr = lastAnchorData;
                ListAppend( anchorList, lastAnchorPtr );
                return;
            }
        }
        AnchorListRelease();
    }

    anchorList = ListCreate();
}



/* release list of anchors */
void AnchorListRelease( void )
{
    ListRelease( anchorList );
    currentAnchor   = NULL;
    anchorList      = NULL;
    anchorId        = 0;

    MSG( "Anchors removed\n" );
}



/* Check if given anchor is within viewport's boundaries */
static Boolean AnchorInViewport
    (
    AnchorType* anchor   /* pointer to visible anchor */
    )
{
    UInt16          prevCoordSys;
    RectangleType*  bounds;
    Int16           anchorTop;
    Int16           anchorBottom;
    Boolean         inViewport;

    bounds          = &anchor->bounds;
    anchorTop       = bounds->topLeft.y;
    anchorBottom    = bounds->topLeft.y + bounds->extent.y;

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    if ( anchorBottom < TopLeftY() || TopLeftY() + ExtentY() < anchorTop ||
         bounds->topLeft.x + bounds->extent.x < TopLeftX() ||
         TopLeftX() + ExtentX() < bounds->topLeft.x )
        inViewport = false;
    else
        inViewport = true;

    PalmSetCoordinateSystem( prevCoordSys );

    return inViewport;
}



/* create new anchor and add it to list of anchors */
static void AnchorCreate
    ( 
    const TextContext*  tContext,   /* pointer to text context */
    const UInt16        reference,  /* record reference */
    const UInt16        image,      /* image reference */
    const Int16         offset,     /* offset to first paragraph */
    const Int16         anchorId    /* unique ID for anchor */
    )
{
    AnchorType*  anchor;

    if ( anchorList == NULL )
        AnchorListInit();

    anchor = SafeMemPtrNew( sizeof *anchor );
    anchor->bounds.topLeft.x    = tContext->cursorX;
    anchor->bounds.topLeft.y    = 0;
    anchor->bounds.extent.x     = 0;
    anchor->bounds.extent.y     = 0;
    anchor->anchorId            = anchorId;
    anchor->reference           = reference;
    anchor->image               = image;
    anchor->paragraphOffset     = offset;
    anchor->underline           = true;
    anchor->state               = ANCHOR_LIMBO;

    ListAppend( anchorList, anchor );
    MSG( _( "Anchor id = %d added\n", anchor->anchorId ) );
}



/* Initialize a new visible anchor */
void AnchorStart
    (
    const TextContext* tContext,    /* pointer to text context */
    const UInt16 reference,         /* record reference */
    const Int16 offset              /* offset to first paragraph */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    AnchorCreate( tContext, reference, NO_IMAGE, offset, anchorId++ );
}



/* Handle multi-line anchors */
void AnchorContinue
    (
    const TextContext* tContext     /* pointer to text context */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    if ( anchorList != NULL ) {
        AnchorType* lastAnchor;

        lastAnchor = ListLast( anchorList );

        AnchorCreate( tContext, lastAnchor->reference, lastAnchor->image,
            lastAnchor->paragraphOffset, lastAnchor->anchorId );
    }
}



/* Restart anchor */
void RestartAnchor
    (
    const TextContext*  tContext,   /* pointer to text context */
    const Int16         height      /* height of line */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    AnchorStop( tContext, height );
    AnchorContinue( tContext );
}



/* Append image to current anchor */
void AnchorAppendImage
    (
    const TextContext*  tContext,   /* pointer to text context */
    const Int16         height,     /* height of line */
    const UInt16        image       /* image reference */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    AnchorStop( tContext, height );
    if ( anchorList != NULL ) {
        AnchorType* lastAnchor;

        lastAnchor = ListLast( anchorList );

        AnchorCreate( tContext, lastAnchor->reference, image,
            lastAnchor->paragraphOffset, lastAnchor->anchorId );
    }
}



/* Mark the end of a visible image anchor */
void AnchorStopImage
    (
    TextContext*    tContext,   /* pointer to text context */
    const Int16     height,     /* height of image */
    const Int16     width       /* width of image */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    if ( anchorList != NULL ) {
        AnchorType* lastAnchor;

        lastAnchor = ListLast( anchorList );

        lastAnchor->underline   = false;
        tContext->cursorX      += width;

        RestartAnchor( tContext, height );

        tContext->cursorX      -= width;
        lastAnchor->underline   = true;
    }
}



void DeleteUnusedAnchors( void )
{
    AnchorType* anchor;

    anchor  = ListFirst( anchorList );
    while ( anchor != NULL ) {
        if ( ! AnchorInViewport( anchor ) ) {
            ListTakeOut( anchorList, anchor );
            SafeMemPtrFree( anchor );
        }
        anchor = ListNext( anchorList, anchor );
    }
}



/* Mark the end of a visible anchor */
void AnchorStop
    (
    const TextContext*  tContext,   /* pointer to text context */
    const Int16         height      /* height of line */
    )
{
    if ( ! DO_ANCHORS( tContext->writeMode ) )
        return;

    if ( anchorList != NULL ) {
        AnchorType*   lastAnchor;
        RectangleType unrotated;

        lastAnchor = ListLast( anchorList );


        /* set anchor boundaries */
        lastAnchor->bounds.topLeft.y   = tContext->cursorY - height;
        lastAnchor->bounds.extent.y    = height;
        lastAnchor->bounds.extent.x    = tContext->cursorX -
                                         lastAnchor->bounds.topLeft.x;

        unrotated                      = lastAnchor->bounds;

        RotateRectangleInPlace( &( lastAnchor->bounds ) );


        /* Only underline visible anchors */
        if ( tContext->writeMode == WRITEMODE_DRAW_CHAR &&
             AnchorInViewport( lastAnchor ) ) {
            /* No underline for images */
            if ( lastAnchor->underline && lastAnchor->bounds.extent.x != 0 ) {
                if ( LinkVisited( lastAnchor->reference ) ) {
                    if ( Prefs()->strikethrough )
                        StrikeThrough( &unrotated );

                    RotDrawGrayLine( unrotated.topLeft.x,
                        tContext->cursorY - 1, tContext->cursorX,
                        tContext->cursorY - 1 );

                }
                else if ( Prefs()->underlineMode ) {
                    RotDrawGrayLine( unrotated.topLeft.x,
                        tContext->cursorY - 1, tContext->cursorX,
                        tContext->cursorY - 1 );
                }
                else {
                    RotDrawLine( unrotated.topLeft.x,
                        tContext->cursorY - 1, tContext->cursorX,
                        tContext->cursorY - 1 );
                }
            }
            lastAnchor->state = ANCHOR_UNSELECTED;
        }
        else if ( ! AnchorInViewport( lastAnchor ) ) {
            /* TODO: fix problem with "missing" anchors */
/*            ListTakeOut( anchorList, lastAnchor );  */  /* oooooh :( */
/*            SafeMemPtrFree( lastAnchor ); */
        }
    }
}



/* return index for anchor at given coordinate, or NOT_FOUND if not found */
Int16 AnchorIndex
    (
    Int16 x,
    Int16 y
    )
{
    AnchorType* anchor;
    Int16       index;

    index   = 1;
    anchor  = ListFirst( anchorList );
    while ( anchor != NULL && ! RctPtInRectangle( x, y, &anchor->bounds ) ) {
        index  += 1;
        anchor = ListNext( anchorList, anchor );
    }
    if ( anchor == NULL )
        return NOT_FOUND;
    else
        return index;
}



/* Return the reference of a visible anchor */
UInt16 GetVisibleReference
    (
    const Int16 index   /* index of visible anchor */
    )
{
    AnchorType* anchor;

    anchor = ListGet( anchorList, index );

    return anchor->reference;
}



/* Return the offset of a visible anchor */
Int16 GetVisibleOffset
    (
    const Int16 index   /* index of visible anchor */
    )
{
    AnchorType* anchor;

    anchor = ListGet( anchorList, index );

    return anchor->paragraphOffset;
}



/* Return the image reference for a visible anchor */
UInt16 GetVisibleImage
    (
    const Int16 index   /* index of visible anchor */
    )
{
    AnchorType* anchor;

    anchor = ListGet( anchorList, index );

    return anchor->image;
}



/* Return the image position for a visible anchor */
RectangleType GetVisibleImagePosition
    (
    const Int16 index   /* index of visible anchor */
    )
{
    AnchorType* anchor;

    anchor = ListGet( anchorList, index );

    return anchor->bounds;
}



/* Set highlight status for the anchor */
void HighlightAnchor
    (
    const Int16             index,  /* anchor index */
    const AnchorStateType   state   /* anchor state ( ANCHOR_SELECTED or 
                                       ANCHOR_UNSELECTED ) */
    )
{
    UInt16      prevCoordSys;
    AnchorType* anchor;
    Int16       anchorId;
    Int16       topY;
    Int16       bottomY;

    MSG_IF( !( state == ANCHOR_SELECTED || state == ANCHOR_UNSELECTED ),
        _( "invalid anchor state, %d\n", state ) );

    prevCoordSys = PalmSetCoordinateSystem( NATIVE );

    if ( IsLargeTable() ) {
        topY        = 0;
        bottomY     = topY + MaxExtentY();
    } else {
        topY        = TopLeftY();
        bottomY     = topY + ExtentY();
    }
    anchor      = ListGet( anchorList, index );
    anchorId    = anchor->anchorId;

    anchor      = ListFirst( anchorList );
    while ( anchor != NULL ) {
        /* only toggle a valid anchor if it's not already in the given state */
        if ( anchor->anchorId == anchorId &&
             anchor->state != state ) {
            /* update boundaries to be within visible screen area */
            if ( anchor->bounds.topLeft.y < topY ) {
                anchor->bounds.extent.y    -= topY - anchor->bounds.topLeft.y;
                anchor->bounds.topLeft.y    = topY;
            }
            if ( bottomY < ( anchor->bounds.topLeft.y +
                             anchor->bounds.extent.y ) ) {
                anchor->bounds.extent.y = bottomY - anchor->bounds.topLeft.y;
            }
            if ( IsFullscreenformActive() ) {
                RectangleType rotated = anchor->bounds;

                RotateRectangleInPlace( &rotated );
                HighlightRectangle( &rotated, SQUARE_CORNERS,
                    ( anchor->state == ANCHOR_UNSELECTED ) ? true : false,
                    BUTTON );
            }
            else {
                HighlightRectangle( &anchor->bounds, SQUARE_CORNERS,
                    ( anchor->state == ANCHOR_UNSELECTED ) ? true : false,
                    BUTTON );
            }
            anchor->state = state;
        }
        anchor = ListNext( anchorList, anchor );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}



/* Adjust all of the visible anchors by the given amount */
void AdjustVisibleAnchors
    (
    const Int16 adjustment  /* adjustment in pixels */
    )
{
    AnchorType* anchor;

    anchor  = ListFirst( anchorList );
    while ( anchor != NULL ) {
        anchor->bounds.topLeft.y += adjustment;
        if ( ! AnchorInViewport( anchor ) ) {
            MSG( _( "Anchor id = %d removed\n", anchor->anchorId ) );
            ListTakeOut( anchorList, anchor );
            SafeMemPtrFree( anchor );
        }
        anchor = ListNext( anchorList, anchor );
    }
}



Int16 FindFirstVisibleAnchor( void )
{
    DeleteUnusedAnchors();

    currentAnchor = ListFirst( anchorList );

    if ( currentAnchor != NULL )
        return ListIndex( anchorList, currentAnchor ) + 1;
    else
        return NOT_FOUND;
}



Int16 FindLastVisibleAnchor( void )
{
    DeleteUnusedAnchors();

    currentAnchor = ListLast( anchorList );

    if ( currentAnchor != NULL )
        return ListIndex( anchorList, currentAnchor ) + 1;
    else
        return NOT_FOUND;
}


Int16 FindNextVisibleAnchor( void )
{
    AnchorType* nextAnchor;

    /* unselect current anchor (if any) */
    if ( currentAnchor != NULL ) {
        HighlightAnchor( ListIndex( anchorList, currentAnchor ) + 1,
            ANCHOR_UNSELECTED );

        nextAnchor = currentAnchor;
        do {
            nextAnchor = ListNext( anchorList, nextAnchor );
        } while ( nextAnchor != NULL &&
                  nextAnchor->anchorId == currentAnchor->anchorId );
    }
    else {
        currentAnchor   = ListFirst( anchorList );
        nextAnchor      = currentAnchor;
    }

    /* highlight next anchor and return ID */
    if ( nextAnchor != NULL ) {
        Int16 index;

        currentAnchor   = nextAnchor;
        index           = ListIndex( anchorList, currentAnchor ) + 1;
        HighlightAnchor( index, ANCHOR_SELECTED );
        return index;
    }
    else {
        return NOT_FOUND;
    }
}


Int16 FindPrevVisibleAnchor( void )
{
    AnchorType* prevAnchor;

    /* unselect current anchor (if any) */
    if ( currentAnchor != NULL ) {
        HighlightAnchor( ListIndex( anchorList, currentAnchor ) + 1,
            ANCHOR_UNSELECTED );

        prevAnchor = currentAnchor;
        do {
            prevAnchor = ListPrev( anchorList, prevAnchor );
        } while ( prevAnchor != NULL &&
                  prevAnchor->anchorId == currentAnchor->anchorId );
    }
    else {
        currentAnchor   = ListFirst( anchorList );
        prevAnchor      = currentAnchor;
    }

    /* highlight next anchor and return ID */
    if ( prevAnchor != NULL ) {
        Int16 index;

        currentAnchor   = prevAnchor;
        index           = ListIndex( anchorList, currentAnchor ) + 1;
        HighlightAnchor( index, ANCHOR_SELECTED );
        return index;
    }
    else {
        return NOT_FOUND;
    }
}


