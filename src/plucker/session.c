/*
 * $Id: session.c,v 1.35 2004/01/25 18:46:05 prussar Exp $
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
#include "history.h"
#include "metadocument.h"
#include "search.h"
#include "document.h"

#include "session.h"


/***********************************************************************
 *
 *      Local functions
 *
 ***********************************************************************/
static void HandleSessionData( Boolean init ) SESSION_SECTION;



/* Handle the session data */
static void HandleSessionData
    (
    Boolean init    /* if true initialize session data, otherwise
                       store session data */
    )
{
    MemHandle   handle;
    History*    sessionPtr;

    handle = ReturnMetaHandle( SESSION_DATA_ID, 0 );
    if ( handle != NULL ) {

        if ( init ) {
            sessionPtr = MemHandleLock( handle );
            ReadHistory( sessionPtr, MemHandleSize( handle ) );
        }
        else {
            MetaCheckByteSize( SESSION_DATA_ID, sizeof( History ),
                handle );
            sessionPtr = MemHandleLock( handle );
            SaveHistory( sessionPtr );
        }

        MemHandleUnlock( handle );
    }
    else if ( init ) {
        InitHistory();
    }
}



/* Initialize the session data */
void InitSessionData( void )
{
    HandleSessionData( true );
    SetSearchPosition( 0 );
    ResetLastUncompressedPHTML();
}



/* Save the session data */
void SaveSessionData( void )
{
    HandleSessionData( false );
}

