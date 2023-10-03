//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

// SB_bar.h

#ifndef __SB_BAR__
#define __SB_BAR__

#include "d_event.h"

#define STARTREDPALS     1
#define STARTBONUSPALS   9
#define STARTPOISONPALS 13
#define STARTICEPAL     21
#define STARTHOLYPAL    22
#define STARTSCOURGEPAL 25
#define NUMREDPALS       8
#define NUMBONUSPALS     4
#define NUMPOISONPALS    8

void SB_Start(void);
void SB_Init(void);
void SB_Ticker(void);
void SB_Drawer(dboolean statusbaron, dboolean refresh, dboolean fullmenu);
void SB_PaletteFlash(dboolean forceChange);

// hexen

void SB_SetClassData(void);

#endif
