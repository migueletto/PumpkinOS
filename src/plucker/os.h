/*
 * $Id: os.h,v 1.46 2003/12/30 14:23:21 nordstrom Exp $
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

#ifndef PLUCKER_OS_H
#define PLUCKER_OS_H

#include "viewer.h"

extern Boolean SupportCompressionType( BitmapCompressionType type );
extern Boolean Support20( void );
extern Boolean Support30( void );
extern Boolean Support31( void );
extern Boolean Support33( void );
extern Boolean Support35( void );
extern Boolean Support40( void );
extern Boolean Support50( void );

extern Boolean SupportNotification(void);

/* Do we support the high density feature set? */
extern Boolean SupportHighDensity( void );

/* Return status for beam support */
extern Boolean SupportBeam( void );

/* Return status for ZLib support */
extern Boolean SupportZLib( void );

/* Return status for VFS support */
extern Boolean SupportVFS( void );

/* Return status for Graffiti 2 support */
extern Boolean SupportGraffiti2( void );

/* Return status for Armlet support */
extern Boolean SupportArmlets( void );

/* Do we have a 68K processor? */
extern Boolean Have68K( void );

/* Return max bit depth for this OS */
extern UInt32 GetMaxBitDepth( void );

/* Return the device's character encoding */
extern UInt32 GetCharEncoding( void );

/* Do we have an 8-bit character set? */
extern Boolean DeviceUses8BitChars( void );

/* Is this a Sony? */
extern Boolean IsSony( void );

/* Do we have a FiveWay controller? */
extern Boolean HaveFiveWay( void );

/* Do we have a Jogdial controller? */
extern Boolean HaveJogdial( void );

/* Do we have a Jogdial controller? */
extern Boolean HaveHsNav( void );

extern Err RomVersionCompatible( UInt32 reqVersion );

/* Initialize OS specific features */
extern void OS_Init( void );

/* Release OS specific features */
extern void OS_Release( void );

/* Check to see if we have a Palm character for the given Unicode char */
extern UInt16 FindPalmCharForUnicodeChar ( UInt32 unicodePoint ) OS_SECTION;

/* Do we have a white background? */
extern Boolean HaveWhiteBackground( void ) OS_SECTION;

#endif

