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
 *      Cheat code checking.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __M_CHEAT__
#define __M_CHEAT__

#include "d_event.h"

#define CHEAT(cheat, deh_cheat, when, func, arg, repeatable) \
  { cheat, deh_cheat, when, func, arg, repeatable, 0, 0, 0, 0, 0, "" }

#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

/* killough 4/16/98: Cheat table structure */

typedef enum {
  cht_always = 0,
  not_demo = 1,
  not_menu = 2,
  not_classic_demo = 4, // allowed in dsda demo format
} cheat_when_t;

typedef struct cheatseq_s {
  const char *	cheat;
  const char *const deh_cheat;
  const cheat_when_t when;
  void (*const func)();
  const int arg;
  const int repeatable;
  uint64_t code, mask;
  size_t sequence_len;

  // state used during the game
  size_t chars_read;
  int param_chars_read;
  char parameter_buf[CHEAT_ARGS_MAX];
} cheatseq_t;

extern cheatseq_t cheat[];

void M_CheatGod(void);
void M_CheatNoClip(void);
void M_CheatIDDT(void);
dboolean M_CheatResponder(event_t *ev);
dboolean M_CheatEntered(const char* element, const char* value);

#endif
