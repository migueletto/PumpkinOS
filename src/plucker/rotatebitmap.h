/*
 * $Id: rotatebitmap.h,v 1.6 2003/12/30 17:46:26 nordstrom Exp $
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

#ifndef PLUCKER_ROTATEBITMAP_H
#define PLUCKER_ROTATEBITMAP_H

#include <PalmTypes.h>
#ifndef ARM_RESOURCE
# include "viewer.h"
#endif

extern void RotateBitmap(Int16  width, Int16  height, UInt8* dest,
    UInt16 destRowBytes, UInt8* src, UInt16 srcRowBytes, Boolean clockwise,
    UInt8 bitDepth)
#ifndef ARM_RESOURCE
    ROTATE_SECTION
#endif
    ;


#endif     /* PLUCKER_ROTATEBITMAP_H */

