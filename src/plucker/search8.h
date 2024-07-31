/*
 * $Id: search8.h,v 1.7 2003/12/30 17:46:27 nordstrom Exp $
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

#ifndef PLUCKER_SEARCH8_H
#define PLUCKER_SEARCH8_H

#include <PalmTypes.h>

#ifndef ARM_RESOURCE
# include "viewer.h"
#else
# define SEARCH8_SECTION
#endif

Char DoSearch8BitText(Char* text, UInt16 size, WChar* wpattern,
     UInt16 patternLen, Char* xlat, UInt16* currentOffsetInTextPtr,
     Int16* currentOffsetInPatternPtr, Int16* offsetOfPlaceFoundPtr,
     Boolean haveDepth
    ) SEARCH8_SECTION;
#endif     /* PLUCKER_SEARCH8_H */

