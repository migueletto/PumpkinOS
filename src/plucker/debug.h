/*
 * $Id: debug.h,v 1.20 2003/01/02 07:49:37 adamm Exp $
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

#ifndef PLUCKER_DEBUG_H
#define PLUCKER_DEBUG_H

#if defined( DEBUG ) && defined( DEBUG_LOG )

#include "viewer.h"

/* Create formatted debug message */
extern Char* _( Char* fmt, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );

/* write debug message to log file */
extern void DebugMsg( Char* fname, Int16 line, Char* msg );

/* write debug message to memo when running on PalmOS device */
extern void MemoDebugMsg( Char* msg );

/* write debug message to log file on desktop when running on POSE */
extern void POSEDebugMsg( Char* fname, Int16 line, Char* msg );

/* write debug message to FrmCustomAlert() */
extern void FormDebugMsg( Char* fname, Int16 line, Char* msg );

#define MSG( s )            DebugMsg( __FILE__, __LINE__, s )
#define MSG_IF( expr,s )    if ( ( expr ) ) { MSG( s ); }
#define FORMMSG( s )        FormDebugMsg( __FILE__, __LINE__, s )

#else /* ! DEBUG_LOG */

#define MSG( s )            /* none */
#define MSG_IF( expr,s )    /* none */
#define FORMMSG( s )        /* none */

#endif /* defined( DEBUG ) && defined( DEBUG_LOG ) */

#endif /* PLUCKER_DEBUG_H */

