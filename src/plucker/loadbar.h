/*
 * $Id: loadbar.h,v 1.2 2003/09/07 14:00:05 chrish Exp $
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

#include "viewer.h"

#ifndef PLUCKER_LOADBAR_H
#define PLUCKER_LOADBAR_H

typedef struct {
    UInt16        totalSteps;
    UInt16        currentStep;
    WinHandle     saveBits;
    RectangleType saveArea;
    RectangleType frame;
    RectangleType bar;
    RectangleType prevClipArea;
} LoadBarType;

extern void LoadBarNextStep( LoadBarType* loaderPtr );
extern LoadBarType* LoadBarNew( UInt16 steps );
extern void LoadBarFree( LoadBarType* loaderPtr );

#endif
