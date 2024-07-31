/*
 * $Id: pluckererror.h,v 1.7 2003/11/30 15:50:27 prussar Exp $
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

#ifndef PLUCKER_ERROR_H
#define PLUCKER_ERROR_H


#define errNoZLibSupport        ( appErrorClass | 1 )
#define errZLibMemError         ( appErrorClass | 2 )
#define errNoBookmarkName       ( appErrorClass | 3 )
#define errNoDocumentName       ( appErrorClass | 4 )
#define errInvalidOwner         ( appErrorClass | 5 )
#define errNoHiRes              ( appErrorClass | 6 )
#define errNoSilkScreen         ( appErrorClass | 7 )
#define errBadImageType         ( appErrorClass | 8 )
#define errImageTooHighBitDepth ( appErrorClass | 9 )
#define errBadImageCompression  ( appErrorClass | 10 )
#define errOSVersionTooLow      ( appErrorClass | 11 )


#endif

