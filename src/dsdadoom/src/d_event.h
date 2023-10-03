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
 * DESCRIPTION:
 *  Event information structures.
 *
 *-----------------------------------------------------------------------------*/


#ifndef __D_EVENT__
#define __D_EVENT__


#include "doomtype.h"


//
// Event handling.
//

// Input event types.
typedef enum
{
  ev_keydown,
  ev_keyup,
  ev_mouse,
  ev_mousemotion,
  ev_joystick,
  ev_move_analog,
  ev_look_analog,
  ev_trigger,
  ev_text,
  ev_quit,
} evtype_t;

typedef union
{
  int i;
  float f;
} event_data_t;

typedef struct
{
  evtype_t type;
  event_data_t data1;
  event_data_t data2;
  char *text;
} event_t;


typedef enum
{
  ga_nothing,
  ga_loadlevel,
  ga_newgame,
  ga_loadgame,
  ga_playdemo,
  ga_completed,
  ga_victory,
  ga_worlddone,
  ga_screenshot,

  // hexen
  ga_leavemap
} gameaction_t;



//
// Button/action code definitions.
//
typedef enum
{
  BT_ATTACK = 1, // Press "Fire".
  BT_USE    = 2, // Use button, to open doors, activate switches.

  // Flag, weapon change pending.
  // If true, the next 4 bits hold weapon num.
  BT_CHANGE = 4,

  // The 4bit weapon mask and shift, convenience.
  BT_WEAPONMASK_OLD = (8 + 16 + 32),      // e6y
  BT_WEAPONMASK     = (8 + 16 + 32 + 64), // extended to pick up SSG // phares
  BT_WEAPONSHIFT    = 3,

  // Special events
  BT_SPECIAL     = 128,
  BT_SPECIALMASK = 3,
  BT_PAUSE = 1,  // Pause the game.
  BT_JOIN  = 64, // Demo joined.
} buttoncode_t;


//
// GLOBAL VARIABLES
//

extern gameaction_t gameaction;

#endif
