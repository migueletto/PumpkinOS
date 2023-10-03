/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:  Head up display
 *
 *-----------------------------------------------------------------------------*/

#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"
#include "d_think.h"
#include "p_mobj.h"
#include "r_defs.h"

#define HU_MSGTIMEOUT   (4*TICRATE)

#define HU_CROSSHAIRS	8

void HU_Start(void);

dboolean HU_Responder(event_t* ev);

void HU_Ticker(void);
void HU_Drawer(void);

mobj_t *HU_Target(void);

int SetCustomMessage(int plr, const char *msg, int ticks, int sfx);

extern int hud_health_red;    // health amount less than which status is red
extern int hud_health_yellow; // health amount less than which status is yellow
extern int hud_health_green;  // health amount above is blue, below is green
extern int hud_ammo_red;      // ammo percent less than which status is red
extern int hud_ammo_yellow;   // ammo percent less is yellow more green

#endif
