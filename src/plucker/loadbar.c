/*
 * $Id: loadbar.c,v 1.3 2004/01/03 13:32:43 nordstrom Exp $
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

#include "debug.h"
#include "hires.h"
#include "os.h"
#include "util.h"

#include "loadbar.h"


/***********************************************************************
 *
 *      Internal Constants
 *
 ***********************************************************************/
#define LOADBAR_WIDTH    50
#define LOADBAR_HEIGHT   10
#define FRAME_PADDING    1


void LoadBarNextStep
    (
    LoadBarType* loadBar
    )
{
    RectangleType partialBar;
    UInt16        prevCoordSys;

    if ( loadBar == NULL )
        return;

    partialBar              = loadBar->bar;
    partialBar.extent.x    /= loadBar->totalSteps;
    partialBar.extent.x    *= ++loadBar->currentStep;

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    if ( Support35() ) {
        WinPushDrawState();
        WinSetForeColor( UIColorGetTableEntryIndex( UIObjectSelectedFill ) );
        WinPaintRectangle( &partialBar, 0 );
        WinPopDrawState();
    }
    else {
        WinDrawRectangle( &partialBar, 0 );
    }
    PalmSetCoordinateSystem( prevCoordSys );
}



LoadBarType* LoadBarNew
    (
    UInt16 steps
    )
{
    UInt16       prevCoordSys;
    LoadBarType* loadBar;
    Err          err;

    loadBar = SafeMemPtrNew( sizeof( LoadBarType ) );
    MemSet( loadBar, sizeof( LoadBarType ), 0 );

    loadBar->totalSteps = steps;
    WinGetClip( &loadBar->prevClipArea );

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );

    /* Define the total area of the loadbar as the saveArea size */
    loadBar->frame.extent.x = LOADBAR_WIDTH;
    loadBar->frame.extent.y = LOADBAR_HEIGHT;
    HiResAdjust( &loadBar->frame.extent.x, sonyHiRes | handeraHiRes );
    HiResAdjust( &loadBar->frame.extent.y, sonyHiRes | handeraHiRes );
    loadBar->frame.topLeft.x = MaxExtentX() / 2 -
                               loadBar->frame.extent.x / 2;
    loadBar->frame.topLeft.y = MaxExtentY() / 2 -
                               loadBar->frame.extent.y / 2;

    /* Define the area that we want to save. -1 constitutes 1 pixel larger on
       each size, necessary to save the frame itself within saveBits */
    loadBar->saveArea = loadBar->frame;
    RctInsetRectangle( &loadBar->saveArea, -1 );
    loadBar->saveBits = WinSaveBits( &loadBar->saveArea, &err );

    /* Define the bar which is FRAME_PADDING pixels smaller on each size than
     * the original frame bounds */
    loadBar->bar = loadBar->frame;
    RctInsetRectangle( &loadBar->bar, FRAME_PADDING );

    if ( err == errNone && loadBar->saveBits != NULL ) {
        WinEraseRectangle( &loadBar->frame, 0 );
        WinDrawRectangleFrame( simpleFrame, &loadBar->frame );
    }
    else {
        LoadBarFree( loadBar );
        loadBar = NULL;
    }

    PalmSetCoordinateSystem( prevCoordSys );

    return loadBar;
}



void LoadBarFree
    (
    LoadBarType* loadBar
    )
{
    UInt16 prevCoordSys;

    if ( loadBar == NULL )
        return;

    prevCoordSys = PalmSetCoordinateSystem( STANDARD );
    if ( loadBar->saveBits != NULL )
        WinRestoreBits( loadBar->saveBits, loadBar->saveArea.topLeft.x,
            loadBar->saveArea.topLeft.y );
    PalmSetCoordinateSystem( prevCoordSys );

    WinSetClip( &loadBar->prevClipArea );

    SafeMemPtrFree( loadBar );
}

