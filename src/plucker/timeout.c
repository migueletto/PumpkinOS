/*
 * $Id: timeout.c,v 1.3 2003/02/23 17:48:20 nordstrom Exp $
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

#include "timeout.h"


/***********************************************************************
 *
 *      Private variables
 *
 ***********************************************************************/
static UInt32  lastTick = 0;
static UInt32  minTicks = 0;

static void (*TimeoutCallbackHandler)( void );



/* Get the status of pending timeout settings */
Boolean TimeoutGet( UInt32* ticks )
{
    *ticks = minTicks;
    return ( minTicks != 0 );
}



/* Set a new Timeout value and callback function*/
void TimeoutSet
    (
    UInt32 ticks,
    void*  callbackFunc
    )
{
    lastTick = TimGetTicks();
    minTicks = ticks;
    TimeoutCallbackHandler = callbackFunc;
}



/* Reset the timer */
void TimeoutReset( void )
{
    lastTick = TimGetTicks();
}



/* Release the timer so its no longer active */
void TimeoutRelease( void )
{
    TimeoutCallbackHandler = NULL;
    minTicks = 0;
}



/* Handler to decide if our minTicks waiting time has passed */
Boolean TimeoutEventHandler( void )
{
    Boolean handled;
    UInt32  now;

    handled = false;

    if ( minTicks == 0 || lastTick == 0 )
        return handled;

    /* Sometimes while waiting for a nilEvent, we may receive one before the
       minTicks number of ticks has passed. Be sure to check */
    now = TimGetTicks();
    if ( lastTick <= ( now - minTicks ) ) {
        TimeoutCallbackHandler();
        handled = true;
        lastTick = now;
    }

    return handled;
}

